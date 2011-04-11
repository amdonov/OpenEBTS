#include "Includes.h"
#include "Common.h"
#include "TransactionDef.h"
#include "IWVerification.h"
#include "IWTransaction.h"
#include "TransactionDef.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "NISTField.h"
#include "RuleObj.h"

// If FAIL_ON_ERROR not defined functions will return IW_SUCCESS 
// unless we're unable to open or read file. Parsing errors will
// not fail initialization unless FAIL_ON_ERROR is defined.
// The thinking is that even if an error occurs in parsing there
// may be enough valid definitions and rules to process the 
// validation request.

// #define FAIL_ON_ERROR

#define STR_TRANSACTION_DEF_TAG _T("TRANSACTIONS")
#define STR_TRANSACTION_FIELD_TAG _T("FIELDS")

enum TOTStateEnum
{
	enStError,
	enStPending,
	enStClassify,
	enStTOTCategory,
	enStTOTComment,
	enStTOTDefinition,
	enStComment,
	enStComplete
};

enum RuleStateEnum
{
	enRuleError,
	enRulePending,
	enRuleTransactionList,
	enRuleLocationIndex,
	enRuleMneumonic,
	enRuleCharType,
	enRuleFieldLength,
	enRuleOccurrence,
	enRuleTags,
	enRuleComplete
};

CIWVerification::CIWVerification()
{
	m_bVerificationLoaded = false;
}

CIWVerification::~CIWVerification()
{
}

int CIWVerification::ReadVerificationFile(CStdStringPath sPath, CStdString& sParseError)
{
	int nRet = IW_ERR_READING_FILE;
	BYTE  *pFile = NULL;
	FILE  *f;

	m_bVerificationLoaded = false;
	sParseError.Empty();

	f = _tfopenpath(sPath, _TPATH("rb"));
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		long lSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		pFile = new BYTE[lSize + 1];
		fread(pFile, 1, lSize, f);
		fclose(f);
		pFile[lSize] = '\0';	// Null terminate file, treating it as a big string

#ifdef UNICODE
		// In the UNICODE version of OpenEBTS, the Verification Files are in UTF-8,
		// hence we convert the buffer in-place to wide chars so we can proceed
		// with simple generic _T code.
		wchar_t *pFileNew;

		if (!UTF8toUCS((const char*)pFile, &pFileNew))
		{
			return IW_ERR_READING_FILE;	// Error decoding UTF-8
		}

		delete pFile;
		pFile = (BYTE*)pFileNew;
#endif

		if ((nRet = LoadTOTDefinitions((TCHAR*)pFile, sPath)) == IW_SUCCESS)
		{
			if ((nRet = LoadRules((TCHAR*)pFile, sPath, sParseError)) == IW_SUCCESS)
			{
				m_bVerificationLoaded = true;
			}
		}
	}
	else
	{
		nRet = IW_ERR_OPENING_FILE_FOR_READING;
	}

	if (pFile != NULL) delete pFile;

	return nRet;
}

int CIWVerification::LoadRules(TCHAR *pFile, CStdString sPath, CStdString& sErr)
{
	CStdString sLine;
	CStdString sTemp;
	bool bFound = false;

	sErr.Empty();

	sLine = ReadLine(&pFile);

	while (pFile != NULL)
	{
		sLine.Trim();
		sLine.MakeUpper();
		
		// find the start of the rules section
		if (sLine.GetLength() >= 6 && !sLine.Left(12).CompareNoCase(STR_TRANSACTION_FIELD_TAG))
		{
			bFound = true;
			break;	// got it
		}				
		sLine = ReadLine(&pFile);
	}

	TCHAR *pRule;
	CStdString sLocationIndex, sMneumonic, sCharType, sFieldSize, sOccurrence;
	CStdString sDescription, sLongDescription, sSpecialChars, sDateFormat, sAdvancedRule, sTags;
	CStdString sTransactionList;
	CStdString sMMap, sOMap;

	if (bFound)
	{
		pRule = GetRule(&pFile);

		while (pRule)
		{
			SkipComments(&pRule);

			// kas, 17Nov2009
			// the entire rule may be commented out, if
			// nothing is left get next rule
			if (_tcslen(pRule) == 0)
			{
				pRule = GetRule(&pFile);
				continue;
			}
			sTemp = GetTransactionList(&pRule);
			
			if (sTemp != _T(""))
				sTransactionList = sTemp;

			if (pRule)
			{
				sLocationIndex = GetLocationIndex(&pRule);
			}
			if (pRule)
			{
				sMneumonic = GetMneumonic(&pRule);
			}
			if (pRule)
			{
				sCharType = GetCharType(&pRule);
			}
			if (pRule)
			{
				sFieldSize = GetFieldSize(&pRule);
			}
			if (pRule)
			{
				sOccurrence = GetOccurrences(&pRule);
			}
			if (pRule)
			{
				sDescription = GetOptionalDescription(&pRule);
			}
			if (pRule)
			{
				sSpecialChars = GetOptionalSpecialChars(&pRule);
			}
			if (pRule)
			{
				sDateFormat = GetOptionalDateFormat(&pRule);
			}
			if (pRule)
			{
				sAdvancedRule = GetOptionalAdvancedRule(&pRule);
			}
			if (pRule)
			{
				sMMap = GetOptionalMMap(&pRule);
			}
			if (pRule)
			{
				sOMap = GetOptionalOMap(&pRule);
			}
			if (pRule)
			{
				sLongDescription = GetOptionalLongDescription(&pRule);
			}
#ifdef _DEBUG
			if (g_bLogToFile)
			{
				sTemp.Format(IDS_LOGVERLOADRULES, sLocationIndex, sMneumonic, sCharType, sFieldSize, sOccurrence);
				LogMessage(sTemp);
			}
#endif
			CRuleObj ruleObj;
			if (ruleObj.SetData(sPath, sTransactionList, sLocationIndex, sMneumonic, sCharType, sFieldSize,
								sOccurrence, sDescription, sLongDescription, sSpecialChars, sDateFormat,
								sAdvancedRule, sMMap, sOMap, sTags, sErr))
			{
				m_rulesAry.push_back(ruleObj);
			}
			else
			{
				return IW_ERR_LOADING_VERICATION;
			}

			pRule = GetRule(&pFile);
		}
	}

	return (m_rulesAry.size() > 0 ? IW_SUCCESS : IW_ERR_LOADING_VERICATION);
}

CStdString CIWVerification::ReadLine(TCHAR **ppFile)
{
	TCHAR *pTemp = *ppFile;
	CStdString sRet;

	if (!pTemp)
	{
		*ppFile = NULL;
		return sRet;
	}

	try
	{
		while (*pTemp && *pTemp != '\n')
			sRet += *pTemp++;

		if (*pTemp && *pTemp == '\n')
		{
			pTemp++;
			*ppFile = pTemp;
		}
		else
			*ppFile = NULL; 
	}
	catch (...)
	{
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERREADLINE);
			LogMessage(sMsg);
		}
	}

	return sRet;
}

TCHAR *CIWVerification::GetRule(TCHAR **ppFile)
{
	TCHAR *pRet = NULL;
	TCHAR *pTemp = *ppFile;
	TCHAR *pStart = pTemp;
	CStdString sRet;
	int ch;
	bool bInQuote = false;

	if (!pTemp)
	{
		*ppFile = NULL;
		return pRet;
	}

	try
	{
		while (*pTemp)
		{
			ch = *pTemp;
			
			if (ch == '"')
			{
				if (!bInQuote)
					bInQuote = true;
				else
					bInQuote = false;
			}
			else if (ch == ';')
			{
				if (!bInQuote)
				{
					pRet = pStart;
					*pTemp = '\0';
					pTemp++;
					*ppFile = pTemp;
					break;
				}
			}
			pTemp++;
		}
		if (*pTemp == '\0')
			*ppFile = NULL;
	}
	catch (...)
	{
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERGETRULE);
			LogMessage(sMsg);
		}
	}

	return pRet;
}

void CIWVerification::SkipComments(TCHAR **ppRule)
{
	TCHAR *pTemp = *ppRule;

	try
	{
		while (1)
		{
			while (*pTemp && isspace(*pTemp))
				pTemp++;
		
			if (*pTemp && *pTemp == '#')
			{
				while (*pTemp && *pTemp != '\n')
					pTemp++;
			}
			else
				break;
		}
	}
	catch (...)
	{
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERSKIPCOMMENTS);
			LogMessage(sMsg);
		}
	}

	*ppRule = pTemp;
}

CStdString CIWVerification::GetTransactionList(TCHAR **ppRule)
{
	CStdString sRet;
	TCHAR *pTemp = *ppRule;

	if (!pTemp)
		return sRet;

	try
	{
		while (*pTemp && isspace(*pTemp))
			pTemp++;

		if (*pTemp)
		{
			bool bInComment = false;
			if (*pTemp == '[')
			{
				pTemp++;
				while (pTemp && *pTemp != ']')
				{
					if (*pTemp == '#')
						bInComment = true;
					
					if (!bInComment)
						sRet += *pTemp++;

					if (*pTemp == '\n')
						bInComment = false;
				}
				if (*pTemp)
					pTemp++; 
				*ppRule = pTemp;
			}
		}
	}
	catch (...)
	{
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERGETTRANSLIST);
			LogMessage(sMsg);
		}
	}

	return sRet;
}

CStdString CIWVerification::GetLocationIndex(TCHAR **ppRule)
{
	return GetNextToken(ppRule);
}

CStdString CIWVerification::GetMneumonic(TCHAR **ppRule)
{
	return GetNextToken(ppRule);
}

CStdString CIWVerification::GetCharType(TCHAR **ppRule)
{
	return GetNextToken(ppRule);
}

CStdString CIWVerification::GetFieldSize(TCHAR **ppRule)
{
	return GetRangeToken(ppRule);
}

CStdString CIWVerification::GetOccurrences(TCHAR **ppRule)
{
	return GetRangeToken(ppRule);
}

CStdString CIWVerification::GetOptionalDescription(TCHAR **ppRule)
// Look for 'desc="The Description"'
{
	return ExtractTagValue(ppRule, _T("desc"));
}

CStdString CIWVerification::GetOptionalSpecialChars(TCHAR **ppRule)
// Look for optional 'sca="-"'
{
	return ExtractTagValue(ppRule, _T("sca"));
}

CStdString CIWVerification::GetOptionalDateFormat(TCHAR **ppRule)
// Look for optional 'date="CCYYMMDD"' date format tag
{
	return ExtractTagValue(ppRule, _T("date"));
}

CStdString CIWVerification::GetOptionalAdvancedRule(TCHAR **ppRule)
// Catchall function to return any other advanced rule, such as:
//
// and(2.030)
// and("T2_WGT")
// mustexist(2.030)
// mustexist("T2_WGT")
// greater(2.030)
// greater("T2_DOA")
// greatereq(2.030)
// greatereq("T2_DOA")
// if(10.043..2)="HUMAN"
// if("T10_SMD_CLASS")="HUMAN"
// nif(2.499..2)="USA"
// nif("T2_POB")="MA"
// supportedif(10.043..2)="SCAR|MARK|TATTOO"
// supportedif("T10_IMT")="SCAR|MARK|TATTOO"
// less(2.030)
// less("T2_DOA")
// lesseq(2.030)
// lesseq("T2_DOA")
// xor(2.030)
// xor("T2_WGT")
// regex="[0-9][0-9]\.[0-9]+" 
// nregex=" .*" 
//
// We only support one advanced rule, so as soon as we find one we return it.
{
	CStdString sRule;

	sRule = ExtractAdvancedTagValue(ppRule, _T("and"));
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("mustexist"));
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("greater"));
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("greatereq"));
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("supportedif"), true);
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("if"), true);
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("nif"), true);
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("less"));
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("lesseq"));
	if (!sRule.IsEmpty()) goto done;
	sRule = ExtractAdvancedTagValue(ppRule, _T("xor"));
	if (!sRule.IsEmpty()) goto done;

	// regex and nregex follow the '[tag]=' syntaz
	sRule = ExtractTagValue(ppRule, _T("regex"));
	if (!sRule.IsEmpty())
	{
		// tack on tag name again
		sRule = _T("regex=\"") + sRule + _T("\"");
		goto done;
	}
	sRule = ExtractTagValue(ppRule, _T("nregex"));
	if (!sRule.IsEmpty())
	{
		// tack on tag name again
		sRule = _T("nregex=\"") + sRule + _T("\"");
		goto done;
	}

done:
	return sRule;
}

CStdString CIWVerification::GetOptionalMMap(TCHAR **ppRule)
// Look for optional 'mmap="<Value>[:<Desc>]|<Value>[:<Desc]|..."' Mandatory Mapping tag
{
	return ExtractTagValue(ppRule, _T("mmap"));
}

CStdString CIWVerification::GetOptionalOMap(TCHAR **ppRule)
// Look for optional 'omap="<Value>[:<Desc>]|<Value>[:<Desc]|..."' Optional Mapping tag
{
	return ExtractTagValue(ppRule, _T("omap"));
}

CStdString CIWVerification::GetOptionalLongDescription(TCHAR **ppRule)
// Look for optional 'long_desc="This field does blah blah blah"'
{
	CStdString sLongDesc;
	CStdString sLongDescRet;
	int		   iPosStart;
	int		   iPosEnd;
	TCHAR	   c;

	sLongDesc = ExtractTagValue(ppRule, _T("long_desc"));

	// Clean-up string by removing end-of-line backslashes. This means
	// removing all back-slash + whitespace + CR + LF + whitespace sequences.
	// To make things a bit simpler, we look for the backslash, and then jump
	// over any sequence containing whitespace, CR or LF in any order.

	for (int i = 0; i < sLongDesc.GetLength(); i++)
	{
		// Look for backslash
		if (sLongDesc.at(i) == '\\')
		{
			iPosStart = i;
			iPosEnd = 0;
			i += 1;	// skip over backslash
			for (int j = i; j < sLongDesc.GetLength(); j++)
			{
				c = sLongDesc.at(j);
				if (!isspace(c) && (c != 0xd) && (c != 0xa))
				{
					iPosEnd = j-1;
					break;
				}
			}
			if (iPosEnd != 0)
			{
				// remove all chars between iPosStart and iPosEnd
				sLongDesc = sLongDesc.Left(iPosStart) + sLongDesc.Right(sLongDesc.GetLength() - iPosEnd -1);
			}
		}
	}

	return sLongDesc;
}

CStdString CIWVerification::ExtractAdvancedTagValue(TCHAR **ppRule, const TCHAR *szTag, bool bHasValue/*=false*/)
// Look for '[szTag](X)' or if bHasValue is set, '[szTag](X)="Y"'
{
	CStdString sFullTag;
	CStdString sRet;
	CStdString sString;
	long	   lPosStart;
	long	   lPosCurr;
	long	   lPosEnd;

	sRet = "";

	sString = *ppRule;												// Copy string into a CStdString

	sFullTag = szTag;
	sFullTag += _T("(");
	lPosStart = sString.Find(sFullTag);								// Find beginning of tag

	// Note that it's possible that there is no such tag, which is fine, we just return ""
	if (lPosStart != -1)
	{
		lPosCurr = lPosStart + sFullTag.GetLength();				// Jump to actual tag contents
		lPosEnd = sString.Find(_T(")"), lPosCurr);					// Find closing bracket

		if (lPosEnd != -1)
		{
			if (bHasValue)
			{
				// Also extract ="Y" portion
				lPosEnd = sString.Find(_T("\""), lPosEnd);			// Find first double-quote
				lPosCurr = lPosEnd + 1;								// Jump to just after first double-quote
				lPosEnd = sString.Find(_T("\""), lPosCurr);			// Find second double-quote
			}

			sRet = sString.Mid(lPosStart, lPosEnd - lPosStart + 1);		// Extract entire tag and content
		}
	}

	return sRet;
}

CStdString CIWVerification::ExtractTagValue(TCHAR **ppRule, const TCHAR *szTag)
// Look for tag '[tagname]="[tagvalue]"'
{
	CStdString sFullTag;
	CStdString sString;
	CStdString sRet;
	long	   lPosStart;
	long	   lPosEnd;

	sRet = "";

	sString = *ppRule;												// Copy string into a CStdString

	sFullTag = szTag;
	sFullTag += _T("=\"");												// e.g. desc --> desc="
	lPosStart = sString.Find(sFullTag);								// Find beginning of tag

	// Note that it's possible that there is no such tag, which is fine, we just return ""
	if (lPosStart != -1)
	{
		lPosStart += sFullTag.GetLength();							// Jump to actual tag value
		lPosEnd = sString.Find(_T("\""), lPosStart);				// Find closing double-quote
		if (lPosEnd != -1)
		{
			sRet = sString.Mid(lPosStart, lPosEnd - lPosStart);		// Extract tag value
		}
	}

	return sRet;
}

CStdString CIWVerification::GetRangeToken(TCHAR **ppRule)
{
	CStdString sRet;
	TCHAR *pTemp = *ppRule;

	if (!pTemp)
		return sRet;

	try
	{
		while (*pTemp && isspace(*pTemp))
			pTemp++;

		if (*pTemp)
		{	
			CStdString sField1,sField2;

			if (toupper(*pTemp) == 'X')
				sField1 = *pTemp;
			else
			{
				while (isdigit(*pTemp))
					sField1 += *pTemp++;
			}
			
			if (sField1 != _T(""))
			{
				TCHAR *pTemp1 = pTemp;
				bool bHyphen = false;

				while (pTemp1 && *pTemp1)
				{
					if (isdigit(*pTemp1))
					{
						if (!bHyphen)
						{
							// ran into next field
							sRet = sField1;
							*ppRule = pTemp;
							break;
						}
						else
						{
							while (isdigit(*pTemp1))
								sField2 += *pTemp1++;

							sRet = sField1 + _T('-') + sField2;
							*ppRule = pTemp1;
							break;
						}
					}
					else if (toupper(*pTemp1) == 'X')
					{
						if (bHyphen)
						{
							sRet = sField1 + _T('-') + *pTemp1;
							*ppRule = ++pTemp1;
							break;
						}						
						else
						{
							// ran into next field
							sRet = sField1;
							*ppRule = pTemp;
							break;
						}
					}
					else if (*pTemp1 == '-')
						bHyphen = true;
					else if (isalpha(*pTemp1))
					{
						// ran into next field
						sRet = sField1;
						*ppRule = pTemp;
						break;					
					}

					pTemp1++;
				}
			}
		}
	}
	catch (...)
	{
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERGETRANGETOKEN);
			LogMessage(sMsg);
		}
	}

	return sRet;
}

CStdString CIWVerification::GetNextToken(TCHAR **ppRule)
{
	CStdString sRet;
	TCHAR *pTemp = *ppRule;

	if (!pTemp)
		return sRet;

	try
	{
		while (1)
		{
			while (*pTemp && isspace(*pTemp))
				pTemp++;
		
			if (*pTemp == '#')
			{
				while (*pTemp != '\0' && *pTemp != '\n')
					pTemp++;
			}
			else
				break;
		}

		if (*pTemp)
		{	
			// go until space
			while (*pTemp != '\0' && !isspace(*pTemp))
				sRet += *pTemp++;

			if (*pTemp)
				pTemp++; 
			*ppRule = pTemp;
		}
	}
	catch (...)
	{
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERGETNEXTTOKEN);
			LogMessage(sMsg);
		}
	}

	return sRet;
}

int CIWVerification::LoadTOTDefinitions(TCHAR *pFile, CStdStringPath sPath)
{
	CTransactionDefinition *pTransDef = NULL;
	int			nRet = IW_ERR_READING_FILE;
	CStdString	sLine;
	CStdString	sTOT;
	CStdString	sTOTDef;
	CStdString	sComment;
	int			nLen;
	int			i;
	TCHAR		ch;
	bool		bInComment = false;
	CStdString	sCategory;

	nRet = IW_SUCCESS;

	// read the transaction types and requirements supported in this file
	TOTStateEnum enState = enStPending;

	while (pFile != NULL)
	{
		sLine = ReadLine(&pFile);
		if (pFile == NULL) break;

		sLine.Trim();

		// found a transaction definition, run it through the state machine
		if (sLine.GetLength() >= 12 && !sLine.Left(12).CompareNoCase(STR_TRANSACTION_DEF_TAG))
		{
			// the last object may be empty
			if (pTransDef)
			{
				if (!pTransDef->IsEmpty())
				{
					if (!pTransDef->IsValid())
					{
						// No occurrences limitations were specified, flag the error
						delete pTransDef;
						pTransDef = NULL;
						nRet = IW_ERR_LOADING_VERICATION;
						break;
					}
				}
			}
			if (pTransDef)
			{
				delete pTransDef;
				pTransDef = NULL;
			}
			pTransDef = new CTransactionDefinition;

			sCategory = sLine.Mid(13);
			sCategory.Replace('"',' ');
			sCategory.Trim();
			
			pTransDef->m_sCategory = sCategory;

			enState = enStClassify; 
			continue; // start processing transaction beginning with next line
		}
		else if (!sLine.CompareNoCase(STR_TRANSACTION_FIELD_TAG))
			break; // hit the rules section
		else if (enState == enStPending)
			continue;
		else if (sLine.empty() || sLine.Left(1) == _T("#"))
			continue; // skip empty lines and comments

		sTOT = _T("");
		sTOTDef = _T("");
		sComment = _T("");

		nLen = sLine.GetLength();
		for (i = 0; i < nLen && enState != enStError && enState != enStComplete; i++)
		{
			ch = sLine.at(i);
			
			switch (enState)
			{
				case enStClassify:
					{
						if (isalpha(ch)) // TOT must begin with alpha char, can be alnum after 1st char
						{
							enState = enStTOTCategory; // transition to reading the TOT, eg: CAR "This is a CAR transaction"
							sTOT += ch;
						}
						else if (isdigit(ch))
						{
							enState = enStTOTDefinition; // transition to reading the TOT requirements, eg: 1:1 2:1 4:0-14 10:0-4
							sTOTDef += ch;
						}
					}
					break;

				case enStTOTCategory:
					{
						if (isalnum(ch))
							sTOT += ch;
						else
						{
							pTransDef->m_TOTArray.push_back(sTOT);
							enState = enStTOTComment;	// transition to comment which must follow TOT label
						}
					}
					break;

				case enStTOTComment:
					{
						if (ch == '"')
						{
							if (bInComment)
							{
								bInComment = false;
								enState = enStClassify; // end of comment, can't wait to see what's next!
								sComment.Replace('"',' ');
								sComment.Trim();
								pTransDef->m_TOTLabelArray.push_back(sComment);
								bInComment = false;
							}
							else if (!bInComment)
								bInComment = true;

							sComment += ch;
						}
						else if (_istspace(ch))
						{
							sComment += ch; // continue in enStTOTComment state
						}
						else if (!bInComment)
						{
							enState = enStError; // shouldn't be here
#ifdef FAIL_ON_ERROR
							nRet = IW_ERR_LOADING_VERICATION;
#endif
						}
						else
							sComment += ch;
					}
					break;

				case enStTOTDefinition:
					{
						if (ch == ';')
						{
							if (pTransDef->SetRuleString(sTOTDef))
							{
								// we should have a complete definition
								enState = enStComplete;
							}
							else
							{
								enState = enStError;
							}
						}
						else
						{
							sTOTDef += ch;
						}
					}
					break;

				default:
					;
			}
		}

		if (enState == enStComplete)
		{
			// add to the list
			if (pTransDef->IsValid())
			{
				if (pTransDef->m_sCategory == _T(""))	
					pTransDef->m_sCategory = sCategory; // use the same category as previous

				m_transactionDefAry.push_back(*pTransDef);
			}

			delete pTransDef;
			pTransDef = NULL;

			// There may be multiple definitions with a TRANSACTIONS section, create
			// a new object for the next TOT def, if there is only 1 definition 
			// this object will be deleted 
			pTransDef = new CTransactionDefinition; 
			enState = enStClassify;
		}
		else if (enState == enStError)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERREADVERFILE, sPath);
			LogMessage(sMsg);

			delete pTransDef;
			pTransDef = NULL;

#ifdef FAIL_ON_ERROR
			nRet = IW_ERR_LOADING_VERICATION;
#endif
		}
	}

	if (pTransDef)
	{
		delete pTransDef;
		pTransDef = NULL;
	}


	if (g_bLogToFile)
	{
		CStdString sMsg;
		sMsg.Format(IDS_LOGVERLOADTOTDEFS, m_transactionDefAry.size());
		LogMessage(sMsg);

#ifdef _DEBUG
		for (UINT i = 0; i < m_transactionDefAry.size(); i++)
		{
			CTransactionDefinition *pTrans = &m_transactionDefAry.at(i);
			sMsg.Format(IDS_LOGVERLOADTOTDEFSDBG, pTrans->m_sCategory, pTrans->GetRuleString());
			LogMessage(sMsg);
		}
#endif
	}

	return nRet;
}

void CIWVerification::DebugOutputVerification()
// This function summarizes the entire contents of the Verification file as loaded into our structures.
{
	CStdString sMsg;
	CStdString sTmp;
	size_t nCount;
	CTransactionDefinition *pTrans;
	std::vector<CRecordTypeCount> recTypeCountAry;
	CRuleObj *pRule;
	CStdString sMin;
	CStdString sMax;
	unsigned int i;
	unsigned int j;
	sMsg = IDS_LOGVERDBGOUTDELIM;
	LogMessage(sMsg);

	nCount = m_transactionDefAry.size();

	// TRANSACTIONS sections of Verification file
	for (i = 0; i < nCount; i++)
	{
		pTrans = &m_transactionDefAry.at(i);

		// Output all TOTs for this transaction list
		sMsg.Empty();
		for (j = 0; j < (unsigned int)pTrans->m_TOTArray.size(); j++)
		{
			sMsg += pTrans->m_TOTArray[j];
			if (j != (unsigned int)pTrans->m_TOTArray.size()-1)
			{
				sMsg += _T(", ");
			}
		}
		if (!sMsg.IsEmpty()) LogMessage(sMsg);


		// Output record type counts for this transaction list
		recTypeCountAry = pTrans->GetRecTypeCountAry();
		sMsg.Empty();
		for (j = 0; j < recTypeCountAry.size(); j++)
		{
			CRecordTypeCount* pRecTypeCount = &recTypeCountAry.at(j);
			if (pRecTypeCount)
			{
				if (pRecTypeCount->nMin == RANGE_NOTSPECIFIED) sMin = _T("X");
				else sMin.Format(_T("%d"), pRecTypeCount->nMin);
				if (pRecTypeCount->nMax == RANGE_NOTSPECIFIED) sMax = _T("X");
				else sMax.Format(_T("%d"), pRecTypeCount->nMax);

				if (pRecTypeCount->nMin == pRecTypeCount->nMax)
				{
					sTmp.Format(IDS_LOGVERDBGOUTMIN, pRecTypeCount->nRecordType, sMin);
					sMsg += sTmp;
				}
				else
				{
					sTmp.Format(IDS_LOGVERDBGOUTMINMAX, pRecTypeCount->nRecordType, sMin, sMax);
					sMsg += sTmp;
				}
			}
		}
		if (!sMsg.IsEmpty()) LogMessage(sMsg);
	}

	sMsg = IDS_LOGVERDBGOUTDELIM;
	LogMessage(sMsg);

	// FIELDS section of Verification file
	nCount = m_rulesAry.size();
	for (unsigned i = 0; i < nCount; i++)
	{
		pRule = &m_rulesAry.at(i);
		sMsg.Format(IDS_LOGVERDBGOUTFIELD,
					pRule->GetMNU(), pRule->GetLocation(), pRule->GetCharType(),
					pRule->GetMinFieldSize(), pRule->GetMaxFieldSize(), pRule->GetMinOccurrences(),
					pRule->GetMaxOccurrences(), pRule->GetDescription(), pRule->GetSpecialChars(),
					pRule->GetDateFormat(), pRule->GetMMap(), pRule->GetTransactionListString());
		LogMessage(sMsg);
	}

	sMsg = IDS_LOGVERDBGOUTDELIM;
	LogMessage(sMsg);
}

int CIWVerification::GetNumRulesPerMNU(CStdString sMNU)
{
	int			nRules = 0;
	CRuleObj	*pRule;

	for (int i=0; i < (int)m_rulesAry.size(); i++)
	{
		pRule = &m_rulesAry.at(i);
		if (pRule->GetMNU() == sMNU) nRules++;
	}

	return nRules;
}

int CIWVerification::VerifyTransaction(CIWTransaction *pTrans)
{
	int								nRet = IW_SUCCESS;
	int								nRetTmp = IW_SUCCESS;
	CStdString						sErr;
	CStdString						sTOT;
	int								i;
	int								j;
	CStdString						sData;
	bool							bFound;
	std::vector<CRecordTypeCount>	recTypeCountAry;
	CTransactionDefinition			*pTransDef;
	int								nRecTypeCount;
	CStdString						sTraceMsg;
	CRuleObj						*pRule;
	bool							bFieldsOK;
	bool							bMandatory;
	bool							bOptional;
	bool							bApplySubItemRules;
	int								iFieldSave;
	CStdString						sRes;
	CStdString						sRecordType;
	std::vector<int>				rgiRecordTypes;

#ifdef _DEBUG
	DebugOutputVerification();
#endif

	if (pTrans == NULL) return IW_ERR_TRANSACTION_NOT_LOADED;
	if (!pTrans->IsTransactionLoaded()) return IW_ERR_TRANSACTION_NOT_LOADED;
	if (!pTrans->IsVerificationLoaded()) return IW_ERR_VERIFICATION_NOT_LOADED;

	nRetTmp = pTrans->FindItem(RECORD_TYPE1, 1, TYPE1_TOT, 1, 1, sData);

	if (nRetTmp == IW_SUCCESS)
	{
		sTOT = sData; // transaction type

		// Locate transaction type in transaction definitions array
		bFound = false;
		for (i = 0; i < (int)m_transactionDefAry.size() && !bFound; i++)
		{
			pTransDef = &m_transactionDefAry.at(i);

			for (j = 0; j < (int)pTransDef->m_TOTArray.size() && !bFound; j++)
			{
				if (!sTOT.CompareNoCase(pTransDef->m_TOTArray.at(j)))
				{
					bFound = true;
					recTypeCountAry = pTransDef->GetRecTypeCountAry();
				}
			}
		}

		if (bFound)
		{
			// Scan the allowed-record-counts array and check for compliance
			for (i = 0; i < (int)recTypeCountAry.size(); i++)
			{
				CRecordTypeCount* pRecTypeCount = &recTypeCountAry.at(i);

				// If record type is 1 or 2 digits, it's really a record type,
				// if it's 3 or 4 digits, it's 2 separate record types, and
				// the record count restrictions then apply to the sum of the
				// records of those 2 record types.
				sRecordType.Format(_T("%d"), pRecTypeCount->nRecordType);

				if (sRecordType.GetLength() <= 2)
				{
					// Verify record type ranges
					pTrans->GetRecordTypeCount(pRecTypeCount->nRecordType, &nRecTypeCount);

					if (pRecTypeCount->nMin != RANGE_NOTSPECIFIED)
					{
						if (nRecTypeCount < pRecTypeCount->nMin)
						{
							// Transaction Type %s must contain at least %ld Type %ld records: it only contains %ld.
							sErr.Format(IDS_TOTRECORDSMIN, sTOT, pRecTypeCount->nMin, pRecTypeCount->nRecordType, nRecTypeCount);
							pTrans->AddError(sErr, IW_WARN_REQ_RECORD_MISSING);
							nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
						}
					}
					if (pRecTypeCount->nMax != RANGE_NOTSPECIFIED)
					{
						if (nRecTypeCount > pRecTypeCount->nMax)
						{
							// Transaction Type %s may contain at most %ld Type %ld records: it contains %ld
							sErr.Format(IDS_TOTRECORDSMAX, sTOT, pRecTypeCount->nMax, pRecTypeCount->nRecordType, nRecTypeCount);
							pTrans->AddError(sErr, IW_WARN_REQ_RECORD_MISSING);
							nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
						}
					}

					// Populate our array of allowed record types. Later we will use this
					// to ensure there are no stray records of disallowed record types.
					rgiRecordTypes.push_back(pRecTypeCount->nRecordType);
				}
				else if (sRecordType.GetLength() == 3 || sRecordType.GetLength() == 4)
				{
					CStdString sRecordType1;
					CStdString sRecordType2;
					int		   iRecType1;
					int		   iRecType2;
					int		   nRecTypeCount1;
					int		   nRecTypeCount2;
					
					sRecordType2 = sRecordType.Right(2);							// The right 2 digits
					sRecordType1 = sRecordType.Left(sRecordType.GetLength() - 2);	// The left 1 or 2 digits

					iRecType1 = (int)_tcstol(sRecordType1, NULL, 10);
					iRecType2 = (int)_tcstol(sRecordType2, NULL, 10);

					// Now we check for compliance against the *sum* of the record types
					pTrans->GetRecordTypeCount(iRecType1, &nRecTypeCount1);
					pTrans->GetRecordTypeCount(iRecType2, &nRecTypeCount2);

					if (pRecTypeCount->nMin != RANGE_NOTSPECIFIED)
					{
						if (nRecTypeCount1 + nRecTypeCount2 < pRecTypeCount->nMin)
						{
							// Transaction Type %s must contain at least %ld Type %ld and Type %ld records: it only contains %ld.
							sErr.Format(IDS_TOTRECORDSMIN2, sTOT, pRecTypeCount->nMin, iRecType1, iRecType2, nRecTypeCount1 + nRecTypeCount2);
							pTrans->AddError(sErr, IW_WARN_REQ_RECORD_MISSING);
							nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
						}
					}
					if (pRecTypeCount->nMax != RANGE_NOTSPECIFIED)
					{
						if (nRecTypeCount1 + nRecTypeCount2 > pRecTypeCount->nMax)
						{
							// Transaction Type %s may contain at most %ld Type %ld and Type %ld records: it contains %ld
							sErr.Format(IDS_TOTRECORDSMAX2, sTOT, pRecTypeCount->nMax, iRecType1, iRecType2, nRecTypeCount1 + nRecTypeCount2);
							pTrans->AddError(sErr, IW_WARN_REQ_RECORD_MISSING);
							nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
						}
					}

					// Populate our array of allowed record types. Later we will use this
					// to ensure there are no stray records of disallowed record types.
					rgiRecordTypes.push_back(iRecType1);
					rgiRecordTypes.push_back(iRecType2);
				}
				else
				{
					// just plain ignore 5 or more digit record type descriptors
				}
			}

			// Next scan all possible record types, and check for disallowed record types
			for (int iRecType = 1; iRecType <= 99; iRecType++)	// 1-based
			{
				bool bRecTypeFound = false;
	
				nRecTypeCount = 0;
				pTrans->GetRecordTypeCount(iRecType, &nRecTypeCount);

				if (nRecTypeCount != 0)
				{
					// Scan array of allowed Record Types we created above
					for (UINT i = 0; i < rgiRecordTypes.size(); i++)
					{
						if (rgiRecordTypes.at(i) == iRecType)
						{
							bRecTypeFound = true;
							break;
						}
					}

					if (!bRecTypeFound)
					// We have a record of an unsupported Record-Type
					{
						// Transaction Type %s may not contain Type %ld records: it contains %ld of them.
						sErr.Format(IDS_TOTRECORDUNSUPPORTED, sTOT, iRecType, nRecTypeCount);
						pTrans->AddError(sErr, IW_WARN_UNSUPPORT_RECORD_PRESENT);
						nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
					}
				}
			}

			iFieldSave = 0;
			bApplySubItemRules = false;

			// Now loop through all the rules
			for (i=0; i < (int)m_rulesAry.size(); i++)
			{
				pRule = &m_rulesAry.at(i);

#ifdef _DEBUG
				if (g_bLogToFile)
				{
					sTraceMsg.Format(IDS_LOGVERVERIFYTRANS,
									 pRule->GetMNU(), pRule->GetLocation(), pRule->GetCharType(),
									 pRule->GetMinFieldSize(), pRule->GetMaxFieldSize(),
									 pRule->GetMinOccurrences(), pRule->GetMaxOccurrences(),
									 pRule->GetTransactionListString());
					LogMessage(sTraceMsg);
				}
#endif

				bFieldsOK = true;
				switch (pRule->GetLocFormType())
				{
					case LOC_FORM_1: bFieldsOK = VerifyFieldsForm1(sTOT, pTrans, pRule); break;
					case LOC_FORM_2: bFieldsOK = VerifyFieldsForm2(sTOT, pTrans, pRule); break;
					case LOC_FORM_3: bFieldsOK = VerifyFieldsForm3(sTOT, pTrans, pRule); break;
					case LOC_FORM_4: bFieldsOK = VerifyFieldsForm4(sTOT, pTrans, pRule); break;
					case LOC_FORM_5: bFieldsOK = VerifyFieldsForm5(sTOT, pTrans, pRule); break;
					case LOC_FORM_6: bFieldsOK = VerifyFieldsForm6(sTOT, pTrans, pRule); break;
					case LOC_FORM_7: bFieldsOK = VerifyFieldsForm7(sTOT, pTrans, pRule); break;
					default:		 sErr = IDS_LOGVERVERIFYTRANSERR;
									 LogMessage(sErr);
				}
				if (!bFieldsOK) nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;

				// Also check to see if any mandatory fields are missing
				nRetTmp = pTrans->GetRecordTypeCount(pRule->GetRecordType(), &nRecTypeCount);
				if (nRetTmp != IW_SUCCESS) nRecTypeCount = 0; // just in case something went wrong

				// Note: a field is only mandatory if we actually have a record of the same RecordType
				if (nRecTypeCount > 0)
				{
					// We're done with subitem rules
					if (iFieldSave != pRule->GetField())
					{
						iFieldSave = 0;
						bApplySubItemRules = false;
					}

					// If we're on a subitem of an existing field we apply the rule, otherwise we just skip it
					if (pRule->GetLocFormType() == LOC_FORM_5 && !bApplySubItemRules) continue;

					nRetTmp = pTrans->Get(pRule->GetMNU(), sData, 1, 1);

					// Header items are empty, but getting IW_ERR_HEADER_ITEM back value implies the field does exist
					if (nRetTmp != IW_ERR_HEADER_ITEM)
					{
						// Here we have another limitation, and that is we can't accurately treat multiple
						// rules per MNU with the current structure (e.g., ebts1_2.txt with T2_CIX, which
						// appears twice)
						if (GetNumRulesPerMNU(pRule->GetMNU()) == 1)
						{
							bMandatory = pRule->IsMandatory(sTOT);
							bOptional = pRule->IsOptional(sTOT);

							// Check for missing mandatory field
							if (nRetTmp == IW_ERR_RECORD_NOT_FOUND && bMandatory)
							{
								// Mandatory field not present
								FlagFieldError(pTrans, pRule, IW_WARN_REQ_FIELD_MISSING, IDS_MANDATORYFIELD);
								nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
							}

							// Check for present non-mandatory/non-optional field
							if (nRetTmp == IW_SUCCESS && !bMandatory && !bOptional)
							{
								// Unsupported field present
								FlagFieldError(pTrans, pRule, IW_WARN_UNSUPPORT_FIELD_PRESENT, IDS_UNSUPPORTEDFIELD);
								nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
							}
						}
					}
					else
					{
						// From now on any rule with the same field# will get applied, as the field exist.
						// This is necessary because the Optional/Mandatory tags only apply if and only if the
						// field exists.
						iFieldSave = pRule->GetField();
						bApplySubItemRules = true;
					}
				}
			}
		}
		else
		{
			// Verification file does not contain Transaction Type %s
			sErr.Format(IDS_VERTOTUNK, sTOT);
			pTrans->AddError(sErr, IW_WARN_UNKNOWN_TOT);
			nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
		}
	}
	else
	{
		// Failed to find TOT field in Record 1
		pTrans->AddError(IDS_NOTOTFIELD, IW_WARN_REQ_FIELD_MISSING);
		nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
	}

	return nRet;
}

bool CIWVerification::VerifyFieldsForm1(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>
// This means that the rule applies to item 1 of all the subfields in the field.
// Each subfield can only have one item.
// There can only be as many subfields as specified by the occurrences value.
{
	bool		bRet = true;
	int			nRet;
	int			iRecType;
	int			iRecord;
	int			iField;
	int			iSubfield;
	int			iItem;
	int			nSubfieldCount;
	int			nRecTypeCount;
	int			nItemCount;
	CStdString	sData;

	iRecType = pRule->GetRecordType();
	iField = pRule->GetField();

	nRet = pTrans->GetRecordTypeCount(iRecType, &nRecTypeCount);
	if (nRet != IW_SUCCESS) nRecTypeCount = 0;

	for (iRecord = 1; iRecord <= nRecTypeCount; iRecord++)	// 1-based
	{
		if (CNISTRecord::IsDATField(iRecType, iField))
		{
			// Nothing to do
		}
		else
		{
			nRet = pTrans->GetNumSubfields(iRecType, iRecord, iField, &nSubfieldCount);

			// Note: VerifySubfieldOccurrences will add any pertinent errors
			VerifySubfieldOccurrences(pTrans, pRule, nSubfieldCount);

			for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
			{
				nRet = pTrans->GetNumItems(iRecType, iRecord, iField, iSubfield, &nItemCount);

				if (nItemCount != 1)
				{
					// Only 1 item allowed per subfield for Location Form 1
					// Incorrect item count (%ld), must be 1
					FlagFieldError(pTrans, pRule, IW_WARN_INCORRECT_ITEM_COUNT, IDS_INCORRECTITEMCOUNT, nItemCount);
					bRet = false;
				}

				// Check all items, even if there are more than 1
				for (iItem = 1; iItem <= nItemCount; iItem++)	// 1-based
				{
					nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, sData);

					if (nRet != IW_SUCCESS)
					{
						// TODO: this should be considered a bad problem (?)
					}

					bRet &= VerifyfieldContents(pTrans, pRule, sData);
				}
			}
		}
	}

	return bRet;
}

bool CIWVerification::VerifyFieldsForm2(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>.<SUBFIELD>
// This means that the rule applies to all the items in the specified subfield.
// There can only be as many items as specified by the occurrences value.
//
// 
//
{
	bool bRet = true;

	return bRet;
}

bool CIWVerification::VerifyFieldsForm3(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>.<SUBFIELD NUMBER>.<ITEM NUMBER>
// This means that rule applies to the specified item in the specified subfield.
// The �occurrences� value is meaningless in this context.
{
	bool bRet = true;

	return bRet;
}

bool CIWVerification::VerifyFieldsForm4(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>:
// This means that the rule applies to everything in the field.
// There can only be as many subfields as specified by the occurrences value.
// For each subfield, the sum of data lengths is checked against the field size value.
{
	bool		bRet = true;
	int			nRet;
	int			iRecType;
	int			iRecord;
	int			iField;
	int			iSubfield;
	int			iItem;
	int			nSubfieldCount;
	int			nRecTypeCount;
	int			nItemCount;
	int			nTotalLength;
	CStdString	sData;

	iRecType = pRule->GetRecordType();
	iField = pRule->GetField();

	nRet = pTrans->GetRecordTypeCount(iRecType, &nRecTypeCount);
	if (nRet != IW_SUCCESS) nRecTypeCount = 0;

	for (iRecord = 1; iRecord <= nRecTypeCount; iRecord++)	// 1-based
	{
		if (CNISTRecord::IsDATField(iRecType, iField))
		{
			// Is this even possible?

			/*
			bRet &= VerifyFieldChars(pTrans, pRule, pData);

			const char* szStorageFormat;
			long		Length;
			long		hll;
			long		vll;
			int			BitsPerPixel;

			nRet = pTrans->GetImageInfo(iRecType, iRecord, &szStorageFormat, &Length, &hll, &vll, &BitsPerPixel);
			if (nRet == IW_SUCCESS)
			{
				nTotalLength += Length;
			}
			*/
		}
		else
		{
			nRet = pTrans->GetNumSubfields(iRecType, iRecord, iField, &nSubfieldCount);

			// Note: VerifySubfieldOccurrences will add any pertinent errors
			VerifySubfieldOccurrences(pTrans, pRule, nSubfieldCount);

			for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
			{
				nRet = pTrans->GetNumItems(iRecType, iRecord, iField, iSubfield, &nItemCount);

				nTotalLength = 0;

				for (iItem = 1; iItem <= nItemCount; iItem++)	// 1-based
				{
					nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, sData);

					if (nRet == IW_SUCCESS)
					{
						bRet &= VerifyFieldChars(pTrans, pRule, sData);
						nTotalLength += sData.GetLength();
					}
					else
					{
						// TODO: this should be considered a bad problem (?)
					}
				}

				// We never call VerifyFieldLength in the item loop, because for Location Form 4
				// the length limits refer to the total of all items, for each subfield.
				bRet &= VerifyFieldLengthTotal(pTrans, pRule, nTotalLength);
			}
		}
	}

	return bRet;
}

bool CIWVerification::VerifyFieldsForm5(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER >..<ITEM NUMBER>
// This means that the rule applies to all items numbered by {ITEM NUMBER} in all the subfields.
// The occurrence value is meaningless in this context.
// This type of location index is usually used in conjunction with the location form type of 4
{
	bool bRet = true;
	int			nRet;
	int			iRecType;
	int			iRecord;
	int			iField;
	int			iSubfield;
	int			iItem;
	int			nSubfieldCount;
	int			nRecTypeCount;
	CStdString	sData;

	iRecType = pRule->GetRecordType();
	iField = pRule->GetField();
	iItem = pRule->GetItem();

	nRet = pTrans->GetRecordTypeCount(iRecType, &nRecTypeCount);
	if (nRet != IW_SUCCESS) nRecTypeCount = 0;

	for (iRecord = 1; iRecord <= nRecTypeCount; iRecord++)	// 1-based
	{
		nRet = pTrans->GetNumSubfields(iRecType, iRecord, iField, &nSubfieldCount);

		for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
		{
			nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, sData);

			if (nRet == IW_SUCCESS)
			// Note that an error may occur, because it's not a given that item 'iItem' exists
			{
				// If the data is empty then we just ignore any rule, because all sub-items
				// defined in the verification file must always be provided for a header-item,
				// and if the item is not specified than an empty string serves as a placeholder.
				if (!sData.IsEmpty())
				{
					bRet &= VerifyfieldContents(pTrans, pRule, sData);
				}
			}
		}
	}

	return bRet;
}

bool CIWVerification::VerifyFieldsForm6(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER >...<OFFSET>
// For binary records (Types 4, 7 and 8), the <OFFSET> value specifies where the item begins relative to the start of the record.
// The size of the binary item is determined by the data type, 1, 2 and 4 byte unsigned integers are supported.
// Text records (Types 1, 2 and 10) can also use this form for fields with a fixed layout.
// For text records, the <OFFSET> value specifies where the item begins with respect to the start of the field.
// The number of subfields is determined by the occurrence value.
// The space reserved for the field is based on the maximum value in the occurrence range.
{
	bool		bRet = true;
	
	// TODO

/*
	int			nRet;
	int			iRecType;
	int			iRecord;
	int			iField;
	int			iSubfield;
	int			iItem;
	int			nSubfieldCount;
	int			nRecTypeCount;
	int			nItemCount;
	const char	*pData;

	iRecType = pRule->GetRecordType();
	iField = pRule->GetField();

	nRet = pTrans->GetRecordTypeCount(iRecType, &nRecTypeCount);
	if (nRet != IW_SUCCESS) nRecTypeCount = 0;

	for (iRecord = 1; iRecord <= nRecTypeCount; iRecord++)	// 1-based
	{
		if (CNISTRecord::IsDATField(iRecType, iField))
		{
			bRet &= VerifyFieldLength(pTrans, pRule, NULL);
			bRet &= VerifyFieldChars(pTrans, pRule, NULL);
		}
		else
		{
			nRet = pTrans->GetNumSubfields(iRecType, iRecord, iField, &nSubfieldCount);

			// Note: VerifySubfieldOccurrences will add any pertinent errors
			VerifySubfieldOccurrences(pTrans, pRule, nSubfieldCount);

			for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
			{
				nRet = pTrans->GetNumItems(iRecType, iRecord, iField, iSubfield, &nItemCount);

				if (nItemCount != 1)
				{
					// Only 1 item allowed per subfield for Location Form 1
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_ITEMS, "Too many items (%ld), must be 1", nItemCount);
					bRet = false;
				}

				// Check all items, even if there are more than 1
				for (iItem = 1; iItem <= nItemCount; iItem++)	// 1-based
				{
					nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, &pData);

					if (nRet != IW_SUCCESS)
					{
						// TODO: this should be considered a bad problem (?)
					}

					bRet &= VerifyFieldLength(pTrans, pRule, pData);
					bRet &= VerifyFieldChars(pTrans, pRule, pData);
				}
			}
		}
	}
*/

	return bRet;
}

bool CIWVerification::VerifyFieldsForm7(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUM>.<SUBFIELD>.<ITEM>.<OFFSET>
// This format is similar to Form 6, except that it allows a specific subfield and item number to be associated with the rule.
// It is useful for fields (or subfields) which contain different types of items.
{
	bool bRet = true;

	return bRet;
}

bool CIWVerification::VerifyfieldContents(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData)
{
	bool bRet = true;

	bRet &= VerifyFieldLength(pTrans, pRule, sData);
	bRet &= VerifyFieldChars(pTrans, pRule, sData);
	bRet &= VerifyFieldDateFormat(pTrans, pRule, sData);
	bRet &= VerifyFieldValue(pTrans, pRule, sData);

	return bRet;
}

bool CIWVerification::VerifyFieldLength(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData)
//
// Apply min/max field length rules
//
{
	bool		bRet = true;
	int			iRecType;
	int			iField;
	int			nLen;
	int			nMin;
	int			nMax;

	iRecType = pRule->GetRecordType();
	iField = pRule->GetField();

	if (!CNISTRecord::IsDATField(iRecType, iField))
	{
		// Get length of field
		nLen = sData.GetLength();

		nMin = pRule->GetMinFieldSize();
		nMax = pRule->GetMaxFieldSize();

		if (nMin == nMax)	// Provide cleaner error message if min is the same as max
		{
			if (nMin != RANGE_NOTSPECIFIED) // Check !=
			{
				if (nLen != nMin)
				{
					if (nLen > nMin)
						// Invalid length of %ld, should be %ld
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, IDS_INVALIDLENGTH, nLen, nMin);
					else
						// Invalid length of %ld, should be %ld
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, IDS_INVALIDLENGTH, nLen, nMin);
					bRet = false;
				}
			}
		}
		else
		{
			if (nMin != RANGE_NOTSPECIFIED)	// Check Min
			{
				if (nLen < nMin)
				{
					// Invalid length of %ld, minimum allowed is %ld
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, IDS_INVALIDLENGTHMIN, nLen, nMin);
					bRet = false;
				}
			}
			if (nMax != RANGE_NOTSPECIFIED)	// Check Max
			{
				if (nLen > nMax)
				{
					// Invalid length of %ld, maximum allowed is %ld
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, IDS_INVALIDLENGTHMAX, nLen, nMax);
					bRet = false;
				}
			}
		}
	}
	else
	{
		// Binary DAT field, nothing to check really (?)
	}

	return bRet;
}

bool CIWVerification::VerifyFieldLengthTotal(CIWTransaction *pTrans, CRuleObj *pRule, int nTotalLen)
//
// Apply min/max field length rules, for total take across all items in a subfield
//
{
	bool		bRet = true;
	int			iRecType;
	int			iField;
	int			nMin;
	int			nMax;

	iRecType = pRule->GetRecordType();
	iField = pRule->GetField();

	if (!CNISTRecord::IsDATField(iRecType, iField))
	{
		nMin = pRule->GetMinFieldSize();
		nMax = pRule->GetMaxFieldSize();

		if (nMin == nMax)	// Provide cleaner error message if min is the same as max
		{
			if (nMin != RANGE_NOTSPECIFIED) // Check !=
			{
				if (nTotalLen != nMin)
				{
					if (nTotalLen > nMin)
						// Invalid total subfield length of %ld, should be %ld
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, IDS_INVALIDTOTALLENGTH, nTotalLen, nMin);
					else
						// Invalid total subfield length of %ld, should be %ld
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, IDS_INVALIDTOTALLENGTH, nTotalLen, nMin);
					bRet = false;
				}
			}
		}
		else
		{
			if (nMin != RANGE_NOTSPECIFIED)	// Check Min
			{
				if (nTotalLen < nMin)
				{
					// Invalid total subfield length of %ld, minimum allowed is %ld
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, IDS_INVALIDTOTALLENGTHMIN, nTotalLen, nMin);
					bRet = false;
				}
			}
			if (nMax != RANGE_NOTSPECIFIED)	// Check Max
			{
				if (nTotalLen > nMax)
				{
					// Invalid total subfield length of %ld, maximum allowed is %ld
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, IDS_INVALIDTOTALLENGTHMAX, nTotalLen, nMax);
					bRet = false;
				}
			}
		}
	}

	return bRet;
}

bool CIWVerification::VerifyFieldChars(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData)
//
// Apply char type rules
//
{
	bool		bRet = true;
	CStdString	sCharType;
	CStdString	sSpecialChars;
	DWORD		dwVal;
	DWORD		dwValMax;

	sCharType = pRule->GetCharType();
	sSpecialChars = pRule->GetSpecialChars();

	// "P" = Printable and "PC" = Control chars are supersets of all other chartypes so we check
	// for the presence of those char codes first, then we explicitly look for the other code types.

	if (sCharType.Find(_T("PC")) != -1)
	{
		if (!IsPrintable(sData, true))
		{
			// Invalid value '%s' for CharType '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_PRINT, IDS_CHARINVALIDVALUE, sData.c_str(), sCharType.c_str());
			bRet = false;
		}		
	}
	else if (sCharType.Find(_T("P")) != -1)
	{
		if (!IsPrintable(sData, false))
		{
			// Invalid value '%s' for CharType '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_PRINTCTRL, IDS_CHARINVALIDVALUE, sData.c_str(), sCharType.c_str());
			bRet = false;
		}		
	}
	else if (!sCharType.CompareNoCase(_T("A")))
	{
		if (!IsAlpha(sData))
		{
			// Invalid value '%s' for CharType '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHA, IDS_CHARINVALIDVALUE, sData.c_str(), sCharType.c_str());
			bRet = false;
		}
	}
	else if (!sCharType.CompareNoCase(_T("N")))
	{
		if (!IsNumeric(sData))
		{
			// Invalid value '%s' for CharType '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_NUMERIC, IDS_CHARINVALIDVALUE, sData.c_str(), sCharType.c_str());
			bRet = false;
		}
	}
	else if (!sCharType.CompareNoCase(_T("AN")))
	{
		if (!IsAlphaNumeric(sData))
		{
			// Invalid value '%s' for CharType '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHANUMERIC, IDS_CHARINVALIDVALUE, sData.c_str(), sCharType.c_str());
			bRet = false;
		}
	}
	else if (!sCharType.CompareNoCase(_T("AS")))
	{
		if (!IsAlphaSpecial(sData, sSpecialChars))
		{
			// Invalid value '%s' for CharType '%s', Special Chars '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHA_SPECIAL, IDS_CHARINVALIDVALUESPECIAL, sData.c_str(), sCharType.c_str(), sSpecialChars.c_str());
			bRet = false;
		}
	}
	else if (!sCharType.CompareNoCase(_T("NS")))
	{
		if (!IsNumericSpecial(sData, sSpecialChars))
		{
			// Invalid value '%s' for CharType '%s', Special Chars '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_NUMERIC_SPECIAL, IDS_CHARINVALIDVALUESPECIAL, sData.c_str(), sCharType.c_str(), sSpecialChars.c_str());
			bRet = false;
		}
	}
	else if (!sCharType.CompareNoCase(_T("ANS")))
	{
		if (!IsAlphaNumericSpecial(sData, sSpecialChars))
		{
			// Invalid value '%s' for CharType '%s', Special Chars '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHANUMERIC_SPECIAL,IDS_CHARINVALIDVALUESPECIAL, sData.c_str(), sCharType.c_str(), sSpecialChars.c_str());
			bRet = false;
		}
	}
	else if (!sCharType.CompareNoCase(_T("B1"))  || !sCharType.CompareNoCase(_T("B2")) ||
			 !sCharType.CompareNoCase(_T("B4")))
	{
		if (sCharType.GetAt(1) == _T('1')) dwValMax = 255;
		if (sCharType.GetAt(1) == _T('2')) dwValMax = 65535;
		if (sCharType.GetAt(1) == _T('4')) dwValMax = 2147483647;

		dwVal = _tcstol(sData, NULL, 10);
		if (dwVal > dwValMax)
		{
			// Invalid value %ld for CharType '%s'
			FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, IDS_TOOMANYDATACHARS, dwVal, sCharType.c_str());
			bRet = false;
		}
	}
	else
	{
		// Unknown CharType code
		if (g_bLogToFile)
		{
			CStdString sMsg;
			sMsg.Format(IDS_LOGVERVERIFYFIELDCHARS, pRule->GetMNU(), sCharType);
			LogMessage(sMsg);
		}
	}

	return bRet;
}

bool CIWVerification::VerifyFieldDateFormat(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData)
{
	bool		bRet = true;
	bool		bAllowNil = false;
	CStdString	sFmt;
	CStdString	sVal;
	CStdString	sY, sM, sD;
	long		iY, iM, iD;
	long		lY, lM, lD;

	sFmt = pRule->GetDateFormat();

	// If there is no date format specified there's nothing to check
	if (sFmt == _T("")) return true;

	// See if there's the "Z" prefix to indicate the whole date can be a string of '0's
	if (sFmt.GetAt(0) == _T('Z'))
	{
		if (sData == _T("00000000")) return true;	// 8 0's is fine with the "Z" prefix
		bAllowNil = true;
	}

	// Get indexes of individual items. These have been checked when reading-in
	// the verification file so none of them should be -1.
	iY = sFmt.Find(_T("CCYY"));
	if (iY == -1)
	{
		iY = sFmt.Find(_T("YYYY"));
	}
	iM = sFmt.Find(_T("MM"));
	iD = sFmt.Find(_T("DD"));

	if (bAllowNil)
	{
		iY--; iM--; iD--;	// Shift all index counters down 1
	}

	sY = sData.Mid(iY, 4);	// extract year string
	sM = sData.Mid(iM, 2);	// extract month string
	sD = sData.Mid(iD, 2);	// extract day string

	lY = _tcstol(sY.c_str(), NULL, 10);	// extract year number
	lM = _tcstol(sM.c_str(), NULL, 10);	// extract month number
	lD = _tcstol(sD.c_str(), NULL, 10);	// extract day number

	// Year must be anywhere from 1900 to 2099
	if (lY < 1900 || lY > 2099)
	{
		// Invalid year %ld
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATE, IDS_INVALIDYEAR, lY);
		bRet = false;
	}

	// Month between 1 and 12
	if (lM < 1 || lM > 12) 
	{
		// Invalid month %ld
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATE, IDS_INVALIDMONTH, lM);
		bRet = false;
	}

	// Day between 1 and ...
	if (lD < 1 || lD > DaysInMonth(lY, lM))
	{
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATE, IDS_INVALIDDAY, lD);
		bRet = false;
	}

	return bRet;
}

long CIWVerification::DaysInMonth(long y, long m)
{
	long numberOfDays;

	if (m == 4 || m == 6 || m == 9 || m == 11)
	{
		numberOfDays = 30;
	}
	else if (m == 2)
	{
		bool isLeapYear;
		isLeapYear = ((y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0)));
		if (isLeapYear)
			numberOfDays = 29;
		else
			numberOfDays = 28;
	}
	else
	{
		numberOfDays = 31;
	}

	return numberOfDays;
}

bool CIWVerification::VerifyFieldValue(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData)
// Check Mandatory Map contents, if applicable
{
	std::vector<CStdString> vals;
	CStdString				sVal;
	bool					bFound = false;

	vals = pRule->GetMMapValNames();

	if (vals.size() == 0) return true; // nothing to check, no mandatory map exists

	for (unsigned int i = 0; i < vals.size(); i++)
	{
		sVal = vals.at(i);
		if (sVal == sData)
		{	
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		// '%s': invalid data from mandatory map
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATA, IDS_INVALIDMMAPDATA, sData.c_str());
	}

	return bFound;
}

bool CIWVerification::VerifySubfieldOccurrences(CIWTransaction *pTrans, CRuleObj *pRule, int nSubfieldCount)
// Given a rule, make sure the number of subfields falls in the legal range. Note that we never consider
// 0 an error, since this just means the subfield is not present, and mandatory fields are checked elsewhere.
{
	bool		bRet = true;
	int			nMinOccurrences;
	int			nMaxOccurrences;

	if (nSubfieldCount == 0) return true;

	nMinOccurrences = pRule->GetMinOccurrences();
	nMaxOccurrences = pRule->GetMaxOccurrences();

	if (nMinOccurrences == nMaxOccurrences) // Custom error message for min=max
	{
		if (nMinOccurrences != RANGE_NOTSPECIFIED) // Check !=
		{
			if (nSubfieldCount != nMinOccurrences)
			{
				if (nSubfieldCount < nMinOccurrences)
					// Too few subfields (%ld), must be %ld
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_SUBFIELDS, IDS_TOOFEWSUBFIELDS, nSubfieldCount, nMinOccurrences);
				else
					// Too many subfields (%ld), must be %ld
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_SUBFIELDS, IDS_TOOMANYSUBFIELDS, nSubfieldCount, nMinOccurrences);
				bRet = false;
			}
		}
	}
	else
	{
		if (nMinOccurrences != RANGE_NOTSPECIFIED) // Check min
		{
			if (nSubfieldCount < nMinOccurrences)
			{
				// Too few subfields (%ld), minimum allowed is %ld
				FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_SUBFIELDS, IDS_TOOFEWSUBFIELDSMIN, nSubfieldCount, nMinOccurrences);
				bRet = false;
			}
		}
		if (nMaxOccurrences != RANGE_NOTSPECIFIED) // Check max
		{
			if (nSubfieldCount > nMaxOccurrences)
			{
				// Too many subfields (%ld), maximum allowed is %ld
				FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_SUBFIELDS, IDS_TOOFEWSUBFIELDSMAX, nSubfieldCount, nMaxOccurrences);
				bRet = false;
			}
		}
	}

	return bRet;
}

// Same as TCHAR for UNICODE, but *unsigned* char for non-UNICODE.
// This allows the _ist* functions to work properly on negative chars
// for inputs in the range 128..255.
#ifdef UNICODE
typedef WCHAR TUCHAR;
#else
typedef unsigned char TUCHAR;
#endif

bool CIWVerification::IsAlpha(CStdString& s)
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);

		if (!_istalpha(c))
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

bool CIWVerification::IsNumeric(CStdString& s)
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);

		if (!_istdigit(c))
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

bool CIWVerification::IsAlphaNumeric(CStdString& s)
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);

		if (!_istalnum(c))
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

bool CIWVerification::IsPrintable(CStdString& s, bool bAllowControlChars)
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);

		if (!_istprint(c))
		{
			if (bAllowControlChars)
			{
				if  (c != 0x09 && c != 0x0A && c != 0x0D) // TAB, LF, CR
				{
					bRet = false;
					break;
				}
			}
			else
			{
				bRet = false;
				break;
			}
		}
	}

	return bRet;
}

bool CIWVerification::IsAlphaSpecial(CStdString& s, CStdString& sSpecial)
// Note: sca="PRINT" is a special code for "all printable characters" and sca="PRINTCTRL"
// means all printable characters + CR + LF + TAB.
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);
		// If the char is not alphabetic and it can't be found in the set of
		// Special Characters the string fails the test.
		if (!_istalpha(c) && sSpecial.Find(c) == -1)
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

bool CIWVerification::IsNumericSpecial(CStdString& s, CStdString& sSpecial)
// Note: sca="PRINT" is a special code for "all printable characters" and sca="PRINTCTRL"
// means all printable characters + CR + LF + TAB.
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);
		// If the char is not numeric and it can't be found in the set of
		// Special Characters the string fails the test.
		if (!_istdigit(c) && sSpecial.Find(c) == -1)
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

bool CIWVerification::IsAlphaNumericSpecial(CStdString& s, CStdString& sSpecial)
// Note: sca="PRINT" is a special code for "all printable characters" and sca="PRINTCTRL"
// means all printable characters + CR + LF + TAB.
{
	bool	bRet = true;
	TUCHAR	c;

	for (int i = 0; i < s.GetLength(); i++)
	{
		c = s.GetAt(i);
		// If the char is not alphabetic, not numeric and it can't be found in the set of
		// Special Characters the string fails the test.
		if (!_istalpha(c) && !_istdigit(c) && sSpecial.Find(c) == -1)
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

void CIWVerification::FlagFieldError(CIWTransaction *pTrans, CRuleObj* pRule, int nErrCode, CStdString sErr, ...)
{
	TCHAR szErr[1024];
	CStdString sMsg;

	va_list args;
	va_start(args, sErr);

#ifdef WIN32
	// Microsoft screwed up with the compiler under VS2008, see
	// https://connect.microsoft.com/VisualStudio/feedback/details/465591/vswprintf-yields-either-error-or-warning
	_vstprintf_s(szErr, 1024, sErr, args);
#else
	_vstprintf(szErr, 1024, sErr, args);
#endif

	va_end(args);

	sMsg.Format(IDS_FLAGFIELDERROR, pRule->GetMNU().c_str(), pRule->GetLocation().c_str(), szErr);
	pTrans->AddError(sMsg, nErrCode);

	if (g_bLogToFile)
	{
		LogMessage(sMsg);
	}
}

CRuleObj *CIWVerification::GetRule(CStdString sMNU)
{
	CRuleObj *pRet = NULL;
	size_t nCount = m_rulesAry.size();
	CRuleObj *pTemp;

	for (unsigned int i = 0; i < nCount; i++)
	{
		pTemp = &m_rulesAry.at(i);
		if (pTemp)
		{
			if (!sMNU.CompareNoCase(pTemp->GetMNU()))
			{
				pRet = pTemp;
				break;
			}
		}
	}
	
	return pRet;
}

int CIWVerification::GetMNULocation(CStdString sMNU, int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item)
{
	int nRet = IW_ERR_MNEMONIC_NOT_FOUND;
	CRuleObj *pRule = GetRule(sMNU);

	if (pRule)
	{
		if (pRule->GetLocation(inputIndex, inputRecordIndex, recordType, recordIndex, field, subField, item))
			nRet = IW_SUCCESS;
	}

	return nRet;
}

int CIWVerification::GetTransactionCategories(int DataArraySize, const TCHAR **ppDataArray, int *pEntries)
{
	int nRet = IW_SUCCESS;
	size_t nSize = m_transactionDefAry.size();
	CTransactionDefinition *pTransDef = NULL; 

	int nPos = 0;
	bool bCopy = DataArraySize > 0;
	std::vector<CStdString> sCategoryAry; // hack to workaround poor logic in parsing
	size_t nCount;
	unsigned int  j;
	bool bFound = false;

	for (unsigned int i = 0; i < nSize; i++)
	{
		pTransDef = &m_transactionDefAry.at(i);

		nCount = sCategoryAry.size();
		bFound = false;
		for (j = 0; j < nCount && !bFound; j++)
		{
			if (!sCategoryAry.at(j).CompareNoCase(pTransDef->m_sCategory))
				bFound = true; // we've already counted this category
		}

		if (!bFound)
		{
			if (bCopy)
			{
				if (nPos < DataArraySize)
				{
					ppDataArray[nPos] = pTransDef->m_sCategory;
					sCategoryAry.push_back(pTransDef->m_sCategory);
				}
				else
					break;
			}
			else
				sCategoryAry.push_back(pTransDef->m_sCategory);

			nPos++;
		}
	}

	*pEntries = nPos;

	return nRet;
}

int CIWVerification::GetTransactionTypes(int DataArraySize, const TCHAR **ppDataArray, 
										 const TCHAR **ppDescArray, int *pEntries, const TCHAR *pCategory)
{
	int						nRet = IW_SUCCESS;
	size_t					nSize = m_transactionDefAry.size();
	CTransactionDefinition	*pTransDef = NULL; 
	CStdString				sCategory(pCategory);
	int						nPos = 0;
	bool					bCopy = DataArraySize > 0;

	for (unsigned int i = 0; i < nSize; i++)
	{
		pTransDef = &m_transactionDefAry.at(i);

		if (sCategory == _T(""))
		{
			for (int j = 0; j < (int)pTransDef->m_TOTArray.size(); j++)
			{
				if (bCopy)
				{
					if (nPos < DataArraySize)
					{
						if (ppDataArray)
							ppDataArray[nPos] = CreateNewStringSlot(pTransDef->m_TOTArray.at(j));
						if (ppDescArray)
							ppDescArray[nPos] = CreateNewStringSlot(pTransDef->m_TOTLabelArray.at(j));
					}
					else
						break;
				}
				nPos++;
			}
		}
		else
		{
			if (!sCategory.CompareNoCase(pTransDef->m_sCategory)) 
			{
				for (int j = 0; j < (int)pTransDef->m_TOTArray.size(); j++)
				{
					if (bCopy)
					{
						if (nPos < DataArraySize)
						{
							if (ppDataArray)
								ppDataArray[nPos] = CreateNewStringSlot(pTransDef->m_TOTArray.at(j));
							if (ppDescArray)
								ppDescArray[nPos] = CreateNewStringSlot(pTransDef->m_TOTLabelArray.at(j));
						}
						else
							break;
					}
					nPos++;
				}
			}
		}
	}

	*pEntries = nPos;

	return nRet;
}

int CIWVerification::GetRecordTypeOccurrences(int DataArraySize, int *piRecordType, int *piMinOccurrences, int *piMaxOccurrences,
											 int *pEntries, const TCHAR *pTOT)
{
	int								nRet = IW_SUCCESS;
	size_t							nSize = m_transactionDefAry.size();
	CTransactionDefinition			*pTransDef = NULL; 
	std::vector<CRecordTypeCount>	recTypeCountAry;
	bool							bCopy = DataArraySize > 0;
	CStdString						sTOT(pTOT);

	if (!pEntries) return IW_ERR_NULL_POINTER;

	if (DataArraySize)
	{
		if (!piRecordType || !piMinOccurrences || !piMaxOccurrences)
		{
			return IW_ERR_NULL_POINTER;
		}
	}

	for (unsigned int i = 0; i < nSize; i++)
	{
		pTransDef = &m_transactionDefAry.at(i);

		for (UINT j = 0; j < pTransDef->m_TOTArray.size(); j++)
		{
			if (!sTOT.CompareNoCase(pTransDef->m_TOTArray.at(j))) 
			{
				recTypeCountAry = pTransDef->GetRecTypeCountAry();
				*pEntries = (int)recTypeCountAry.size();

				if (bCopy)
				{
					for (int j = 0; j < min(DataArraySize, *pEntries); j++)
					{
						CRecordTypeCount* pRecTypeCount = &recTypeCountAry.at(j);
						if (pRecTypeCount)
						{
							piRecordType[j] = pRecTypeCount->nRecordType;
							piMinOccurrences[j] = pRecTypeCount->nMin;
							piMaxOccurrences[j] = pRecTypeCount->nMax;
						}
					}
				}
			}
		}
	}

	return nRet;
}

int CIWVerification::GetMnemonics(const TCHAR* TransactionType, int DataArraySize, const TCHAR** ppDataArray, const TCHAR** ppDescArray, int* pEntries)
{
	int			nRet = IW_SUCCESS;
	CRuleObj	*pRule;
	int			nPos = 0;
	bool		bCopy = DataArraySize > 0;
	CStdString	sTOT(TransactionType);

	for (int i=0; i < (int)m_rulesAry.size(); i++)
	{
		pRule = &m_rulesAry.at(i);

		if (pRule->IsMandatory(sTOT) || pRule->IsOptional(sTOT))
		{
			if (bCopy)
			{
				if (nPos < DataArraySize)
				{
					if (ppDataArray) ppDataArray[nPos] = CreateNewStringSlot(pRule->GetMNU());
					if (ppDescArray) ppDescArray[nPos] = CreateNewStringSlot(pRule->GetDescription());
				}
				else
				{
					break;
				}
			}
			nPos++;
		}
	}

	*pEntries = nPos;

	return nRet;
}

static const TCHAR s_rgszAutomaticMNU[][10] =
{
 _T("T1_LEN"),  _T("T1_VER"),  _T("T1_CNT"),  _T("T1_TOT"), _T("T1_DCS"),
 _T("T2_LEN"),  _T("T2_IDC"),
 _T("T4_LEN"),  _T("T4_IDC"),  _T("T4_HLL"),  _T("T4_VLL"),  _T("T4_GCA"),  _T("T4_DAT"),
 _T("T7_LEN"),  _T("T7_IDC"),  _T("T7_HLL"),  _T("T7_VLL"),  _T("T7_GCA"),  _T("T7_DAT"),
 _T("T9_LEN"),  _T("T9_IDC"),
_T("T10_LEN"), _T("T10_IDC"), _T("T10_HLL"), _T("T10_VLL"), _T("T10_SLC"), _T("T10_HPS"), _T("T10_VPS"), _T("T10_CGA"), _T("T10_DAT"),
_T("T13_LEN"), _T("T13_IDC"), _T("T13_HLL"), _T("T13_VLL"), _T("T13_SLC"), _T("T13_HPS"), _T("T13_VPS"), _T("T13_CGA"), _T("T13_BPX"), _T("T13_DAT"),
_T("T14_LEN"), _T("T14_IDC"), _T("T14_HLL"), _T("T14_VLL"), _T("T14_SLC"), _T("T14_HPS"), _T("T14_VPS"), _T("T14_CGA"), _T("T14_BPX"), _T("T14_DAT"),
_T("T15_LEN"), _T("T15_IDC"), _T("T15_HLL"), _T("T15_VLL"), _T("T15_SLC"), _T("T15_HPS"), _T("T15_VPS"), _T("T15_CGA"), _T("T15_BPX"), _T("T15_DAT"),
_T("T16_LEN"), _T("T16_IDC"), _T("T16_HLL"), _T("T16_VLL"), _T("T16_SLC"), _T("T16_HPS"), _T("T16_VPS"), _T("T16_CGA"), _T("T16_BPX"), _T("T16_DAT"),
_T("T17_LEN"), _T("T17_IDC"), _T("T17_HLL"), _T("T17_VLL"), _T("T17_SLC"), _T("T17_HPS"), _T("T17_VPS"), _T("T17_CGA"), _T("T17_BPX"), _T("T17_DAT")
};

static int s_nAutomaticMNUs = sizeof(s_rgszAutomaticMNU)/sizeof(s_rgszAutomaticMNU[0]);

int CIWVerification::GetRuleRestrictions(const TCHAR* TransactionType, const TCHAR* pMnemonic, int* pRecordType, int* pField, int* pSubfield,
										 int* pItem, const TCHAR** ppDesc, const TCHAR** ppLongDesc, const TCHAR** ppCharType, const TCHAR** ppSpecialChars,
										 const TCHAR** ppDateFormat, const TCHAR** ppAdvancedRule, int* pSizeMin, int* pSizeMax, int* pOccMin, int* pOccMax,
										 int* pOffset, bool* pAutomaticallySet, bool* pMandatory)
{
	int			nRet = IW_ERR_MNEMONIC_NOT_FOUND;
	CRuleObj	*pRule;
	CStdString	sTOT(TransactionType);
	bool		bAuto;

	if (TransactionType == NULL) return IW_ERR_NULL_POINTER;
	if (pMnemonic == NULL) return IW_ERR_NULL_POINTER;
	if (pRecordType == NULL) return IW_ERR_NULL_POINTER;
	if (pField == NULL) return IW_ERR_NULL_POINTER;
	if (pSubfield == NULL) return IW_ERR_NULL_POINTER;
	if (pItem == NULL) return IW_ERR_NULL_POINTER;
	if (ppDesc == NULL) return IW_ERR_NULL_POINTER;
	if (ppLongDesc == NULL) return IW_ERR_NULL_POINTER;
	if (ppCharType == NULL) return IW_ERR_NULL_POINTER;
	if (ppSpecialChars == NULL) return IW_ERR_NULL_POINTER;
	if (ppDateFormat == NULL) return IW_ERR_NULL_POINTER;
	if (ppAdvancedRule == NULL) return IW_ERR_NULL_POINTER;
	if (pSizeMin == NULL) return IW_ERR_NULL_POINTER;
	if (pSizeMax == NULL) return IW_ERR_NULL_POINTER;
	if (pOccMin == NULL) return IW_ERR_NULL_POINTER;
	if (pOccMax == NULL) return IW_ERR_NULL_POINTER;
	if (pOffset == NULL) return IW_ERR_NULL_POINTER;
	if (pAutomaticallySet == NULL) return IW_ERR_NULL_POINTER;
	if (pMandatory == NULL) return IW_ERR_NULL_POINTER;

	for (int i = 0; i < (int)m_rulesAry.size(); i++)
	{
		pRule = &m_rulesAry.at(i);

		if (pRule->GetMNU().CompareNoCase(pMnemonic) == 0)
		{
			if (pRule->IsMandatory(sTOT) || pRule->IsOptional(sTOT))
			{
				// integral parameters
				*pRecordType = pRule->GetRecordType();
				*pField = pRule->GetField();
				*pSubfield = pRule->GetSubField();
				*pItem = pRule->GetItem();
				*pSizeMin = pRule->GetMinFieldSize();
				*pSizeMax = pRule->GetMaxFieldSize();
				*pOccMin = pRule->GetMinOccurrences();
				*pOccMax = pRule->GetMaxOccurrences();
				*pOffset = pRule->GetOffset();
				*pMandatory = pRule->IsMandatory(sTOT) ? true : false;

				// string parameters
				*ppDesc = CreateNewStringSlot(pRule->GetDescription());
				*ppLongDesc = CreateNewStringSlot(pRule->GetLongDescription());
				*ppCharType = CreateNewStringSlot(pRule->GetCharType());
				*ppSpecialChars = CreateNewStringSlot(pRule->GetSpecialChars());
				*ppDateFormat = CreateNewStringSlot(pRule->GetDateFormat());
				*ppAdvancedRule = CreateNewStringSlot(pRule->GetAdvancedRule());

				// Is field automatically managed by OpenEBTS? Scan array.
				bAuto = false;
				for (int j = 0; j < s_nAutomaticMNUs; j++)
				{
					if (pRule->GetMNU().CompareNoCase(s_rgszAutomaticMNU[j]) == 0)
					{
						bAuto = true;
						break;
					}
				}
				*pAutomaticallySet = bAuto;

				nRet = IW_SUCCESS;
			}
		}
	}

	return nRet;
}

int CIWVerification::GetValueList(const TCHAR* TransactionType, const TCHAR* pMnemonic, bool *pMandatory,
								  int DataArraySize, const TCHAR** ppDataArray, const TCHAR** ppDescArray, int *pEntries)
{
	int						nRet = IW_ERR_MNEMONIC_NOT_FOUND;
	CRuleObj				*pRule;
	CStdString				sTOT(TransactionType);
	std::vector<CStdString> names;
	std::vector<CStdString> descriptions;
	CStdString				sVal;
	bool					bCopy = DataArraySize > 0;

	if (TransactionType == NULL) return IW_ERR_NULL_POINTER;
	if (pMnemonic == NULL) return IW_ERR_NULL_POINTER;

	for (int i = 0; i < (int)m_rulesAry.size(); i++)
	{
		pRule = &m_rulesAry.at(i);

		if (pRule->GetMNU().CompareNoCase(pMnemonic) == 0)
		{
			if (pRule->IsMandatory(sTOT) || pRule->IsOptional(sTOT))
			{
				*pEntries = 0;
				// Only one of MMap or OMap can be specified, so both cannot be non-zero in length
				names = pRule->GetMMapValNames();
				if (names.size() == 0)
				{
					names = pRule->GetOMapValNames();
					descriptions = pRule->GetOMapValDescriptions();
					if (pMandatory != NULL) *pMandatory = 0;
				}
				else
				{
					descriptions = pRule->GetMMapValDescriptions();
					if (pMandatory != NULL) *pMandatory = 1;
				}
				// The names and descriptions arrays are parallel, so should have the
				// same size; we just do this for safety.
				*pEntries = (int)min(names.size(), descriptions.size());

				if (bCopy)
				{
					for (int j = 0; j < *pEntries; j++)
					{
						if (j < DataArraySize)
						{
							ppDataArray[j] = CreateNewStringSlot(names.at(j));
							ppDescArray[j] = CreateNewStringSlot(descriptions.at(j));
						}
						else
							break;
					}
				}

				nRet = IW_SUCCESS;
				break;
			}
		}
	}

	return nRet;
}
