

#include "stdafx.h"
#include "common.h"
#include "TransactionDef.h"
#include "IWVerification.h"
#include "IWTransaction.h"
#include "transactiondef.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "NISTField.h"
#include "ruleobj.h"


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
	m_bVerificationLoaded = FALSE;
}

CIWVerification::~CIWVerification()
{
}

int CIWVerification::ReadVerificationFile(const char* Path, int MaxParseError, char* ParseError)
{
	int nRet = IW_ERR_READING_FILE;
	CStdString sFilePath = Path;
	CStdString sErr;

	m_bVerificationLoaded = FALSE;

	if ((nRet = LoadTOTDefinitions(sFilePath)) == IW_SUCCESS)
	{
		if ((nRet = LoadRules(sFilePath, sErr)) == IW_SUCCESS)
		{
			m_bVerificationLoaded = TRUE;
		}
		else
		{
			strncpy(ParseError, sErr, MaxParseError-1);
			ParseError[MaxParseError-1] = '\0';
		}
	}

	return nRet;
}

int CIWVerification::LoadRules(CStdString& sFilePath, CStdString& sErr)
{
	int nRet = IW_ERR_READING_FILE;
	FILE *f;
	char *pFileSave = NULL;

	f = fopen(sFilePath, "rb");
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		long lSize = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *pFile = new char[(UINT)lSize+2];

		pFileSave = pFile;	// to be released at the end

		memset(pFile,'\0',lSize+2);
		fread(pFile, 1, lSize, f);
		fclose(f);

		char szDelimsLine[] = "\n";
		CStdString sLine;
		CStdString sTemp;
		BOOL bFound = FALSE;

		sLine = ReadLine(&pFile);

		while (pFile != NULL)
		{
			sLine.Trim();
			sLine.MakeUpper();
			
			// find the start of the rules section
			if (sLine.GetLength() >= 6 && !sLine.Left(12).CompareNoCase(STR_TRANSACTION_FIELD_TAG))
			{
				bFound = TRUE;
				break;	// got it
			}				
			sLine = ReadLine(&pFile);
		}

		char szDelimsRule[] = ";";
		char *pRule;
		CStdString sLocationIndex, sMneumonic, sCharType, sFieldSize, sOccurrence;
		CStdString sDescription, sLongDescription, sSpecialChars, sDateFormat, sTags;
		CStdString sTransactionList;
		CStdString sMMap;

		if (bFound)
		{
			pRule = GetRule(&pFile);

			while (pRule)
			{
				SkipComments(&pRule);

				// kas, 17Nov2009
				// the entire rule may be commented out, if
				// nothing is left get next rule
				if (strlen(pRule) == 0)
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
					sMMap = GetOptionalMMap(&pRule);
				}
				if (pRule)
				{
					sLongDescription = GetOptionalLongDescription(&pRule);
				}
				if (pRule)
				{
					sTags = GetTags(&pRule);
				}
#ifdef _DEBUG
				sTemp.Format("Loc: %s, MNU: %s, Type: %s, Size: %s, Occ: %s\n", sLocationIndex,
							 sMneumonic, sCharType, sFieldSize, sOccurrence);
				OutputDebugString(sTemp);
#endif
				CRuleObj ruleObj;

				if (ruleObj.SetData(sFilePath, sTransactionList, sLocationIndex, sMneumonic, sCharType, sFieldSize,
									sOccurrence, sDescription, sLongDescription, sSpecialChars, sDateFormat, sMMap,
									sTags, sErr))
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
		if (pFileSave) delete pFileSave;
	}
	else
		nRet = IW_ERR_OPENING_FILE_FOR_READING;

	return (m_rulesAry.size() > 0 ? IW_SUCCESS : IW_ERR_LOADING_VERICATION);
}

CStdString CIWVerification::ReadLine(char **ppFile)
{
	char *pTemp = *ppFile;
	CStdString sRet;
	CStdString sErr;

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
		sErr.Format("[CIWVerification::ReadLine] exception thrown");
		LogFile(NULL,sErr);
	}

	return sRet;
}

char *CIWVerification::GetRule(char **ppFile)
{
	char *pRet = NULL;
	char *pTemp = *ppFile;
	char *pStart = pTemp;
	CStdString sRet;
	int ch;
	BOOL bInQuote = FALSE;
	CStdString sErr;

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
					bInQuote = TRUE;
				else
					bInQuote = FALSE;
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
		sErr.Format("[CIWVerification::GetRule] exception thrown");
		LogFile(NULL,sErr);
	}

	return pRet;
}

void CIWVerification::SkipComments(char **ppRule)
{
	char *pTemp = *ppRule;
	CStdString sErr;

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
		sErr.Format("[CIWVerification::SkipComment] exception thrown");
		LogFile(NULL,sErr);
	}

	*ppRule = pTemp;
}

CStdString CIWVerification::GetTransactionList(char **ppRule)
{
	CStdString sRet;
	char *pTemp = *ppRule;
	CStdString sErr;

	if (!pTemp)
		return sRet;

	try
	{
		while (*pTemp && isspace(*pTemp))
			pTemp++;

		if (*pTemp)
		{
			BOOL bInComment = FALSE;
			if (*pTemp == '[')
			{
				pTemp++;
				while (pTemp && *pTemp != ']')
				{
					if (*pTemp == '#')
						bInComment = TRUE;
					
					if (!bInComment)
						sRet += *pTemp++;

					if (*pTemp == '\n')
						bInComment = FALSE;
				}
				if (*pTemp)
					pTemp++; 
				*ppRule = pTemp;
			}
		}
	}
	catch (...)
	{
		sErr.Format("[CIWVerification::GetTransactionList] exception thrown");
		LogFile(NULL,sErr);
	}

	return sRet;
}

CStdString CIWVerification::GetLocationIndex(char **ppRule)
{
	return GetNextToken(ppRule);
}

CStdString CIWVerification::GetMneumonic(char **ppRule)
{
	return GetNextToken(ppRule);
}

CStdString CIWVerification::GetCharType(char **ppRule)
{
	return GetNextToken(ppRule);
}

CStdString CIWVerification::GetFieldSize(char **ppRule)
{
	return GetRangeToken(ppRule);
}

CStdString CIWVerification::GetOccurrences(char **ppRule)
{
	return GetRangeToken(ppRule);
}

CStdString CIWVerification::GetOptionalDescription(char **ppRule)
// Look for 'desc="The Description"'
{
	return ExtractTagValue(ppRule, "desc");
}

CStdString CIWVerification::GetOptionalSpecialChars(char **ppRule)
// Look for optional 'sca="-"'
{
	return ExtractTagValue(ppRule, "sca");
}

CStdString CIWVerification::GetOptionalDateFormat(char **ppRule)
// Look for optional 'date="CCYYMMDD"' date format tag
{
	return ExtractTagValue(ppRule, "date");
}

CStdString CIWVerification::GetOptionalMMap(char **ppRule)
// Look for optional 'mmap="<Value>[:<Desc>]|<Value>[:<Desc]|..."' Mandatory Mapping tag
{
	return ExtractTagValue(ppRule, "mmap");
}

CStdString CIWVerification::GetOptionalLongDescription(char **ppRule)
// Look for optional 'long_desc="This field does blah blah blah"'
{
	return ExtractTagValue(ppRule, "long_desc");
}

CStdString CIWVerification::ExtractTagValue(char **ppRule, const char *szTag)
// // Look for tage '[tagname]="[tagvalue]"'
{
	CStdString sFullTag;
	CStdString sString;
	CStdString sRet;
	long	   lPosStart;
	long	   lPosEnd;

	sRet = "";

	sString = *ppRule;												// Copy string into a CStdString

	sFullTag = szTag;
	sFullTag += "=\"";												// e.g. desc --> desc="
	lPosStart = sString.Find(sFullTag);								// Find beginning of tag

	// Note that it's possible that there is no such tag, which is fine, we just return ""
	if (lPosStart != -1)
	{
		lPosStart += sFullTag.GetLength();							// Jump to actual tag value
		lPosEnd = sString.Find("\"", lPosStart);					// Find closing double-quote
		if (lPosEnd != -1)
		{
			sRet = sString.Mid(lPosStart, lPosEnd - lPosStart);		// Extract tag value
		}
	}

	return sRet;
}

CStdString CIWVerification::GetTags(char **ppRule)
{
	CStdString sRet;
	char *pTemp = *ppRule;
	CStdString sErr;

	if (!pTemp)
		return sRet;
	
	try
	{
		while (*pTemp && isspace(*pTemp))
			pTemp++;

		if (*pTemp)
		{
			sRet = pTemp;	
			*ppRule = pTemp;
		}
	}
	catch (...)
	{
		sErr.Format("[CIWVerification::GetTransactionList] exception thrown");
		LogFile(NULL,sErr);
	}

	return sRet;
}

CStdString CIWVerification::GetRangeToken(char **ppRule)
{
	CStdString sRet;
	char *pTemp = *ppRule;
	CStdString sErr;

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
				char *pTemp1 = pTemp;
				BOOL bHyphen = FALSE;

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

							sRet = sField1+'-'+sField2;
							*ppRule = pTemp1;
							break;
						}
					}
					else if (toupper(*pTemp1) == 'X')
					{
						if (bHyphen)
						{
							sRet = sField1+'-'+*pTemp1;
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
						bHyphen = TRUE;
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
		sErr.Format("[CIWVerification::GetRangeToken] exception thrown");
		LogFile(NULL,sErr);
	}

	return sRet;
}

CStdString CIWVerification::GetNextToken(char **ppRule)
{
	CStdString sRet;
	char *pTemp = *ppRule;
	CStdString sErr;

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
		sErr.Format("[CIWVerification::GetNextToken] exception thrown");
		LogFile(NULL,sErr);
	}

	return sRet;
}

int CIWVerification::LoadTOTDefinitions(CStdString& sFilePath)
{
	int nRet = IW_ERR_READING_FILE;
	FILE *f;
	CTransactionDefinition *pTransDef = NULL;

	f = fopen(sFilePath, "r+t");
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		long lSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		CStdString sTemp;
		CStdString sTOT;
		CStdString sTOTDef;
		CStdString sComment;
		int nLen;
		int i;
		_TCHAR ch;
		BOOL bInComment = FALSE;
		CStdString sCategory;

		nRet = IW_SUCCESS;

		// read the transaction types and requirements supported in this file
		TOTStateEnum enState = enStPending;

		while (fgets(sTemp.GetBuffer(1024), 1024, f) && nRet == IW_SUCCESS)
		{
			sTemp.ReleaseBuffer();
			sTemp.MakeUpper();
			sTemp.Trim();

			// found a transaction definition, run it through the state machine
			if (sTemp.GetLength() >= 12 && !sTemp.Left(12).CompareNoCase(STR_TRANSACTION_DEF_TAG))
			{
				// the last object may be empty
				if (pTransDef)
				{
					if (!pTransDef->IsEmpty())
					{
						if (!pTransDef->IsValid())
						{
							delete pTransDef;
							pTransDef = NULL;
#ifdef FAIL_ON_ERROR
							nRet = IW_ERR_LOADING_VERICATION;
							continue;
#endif
						}
					}
				}
				if (pTransDef)
				{
					delete pTransDef;
					pTransDef = NULL;
				}
				pTransDef = new CTransactionDefinition;

				sCategory = sTemp.Mid(13);
				sCategory.Replace('"',' ');
				sCategory.Trim();
				
				pTransDef->m_sCategory = sCategory;

				enState = enStClassify; 
				continue; // start processing transaction beginning with next line
			}
			else if (!sTemp.CompareNoCase(STR_TRANSACTION_FIELD_TAG))
				break; // hit the rules section
			else if (enState == enStPending)
				continue;
			else if (sTemp.empty() || sTemp.Left(1) == _T("#"))
				continue; // skip empty lines and comments

			sTOT = _T("");
			sTOTDef = _T("");
			sComment = _T("");

			nLen = sTemp.GetLength();
			for (i = 0; i < nLen && enState != enStError && enState != enStComplete; i++)
			{
				ch = sTemp.at(i);
				
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
									bInComment = FALSE;
									enState = enStClassify; // end of comment, can't wait to see what's next!
									sComment.Replace('"',' ');
									sComment.Trim();
									pTransDef->m_TOTLabelArray.push_back(sComment);
									bInComment = FALSE;
								}
								else if (!bInComment)
									bInComment = TRUE;

								sComment += ch;
							}
							else if (isspace(ch))
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

				// There may be multiple definitions with a TRANSACTIONS setion, create 
				// a new object for the next TOT def, if there is only 1 definition 
				// this object will be deleted 
				pTransDef = new CTransactionDefinition; 
				enState = enStClassify;
			}
			else if (enState == enStError)
			{
				CStdString sTraceMsg;
				
				sTraceMsg.Format("[CIWVerification::ReadVerificationFile] Error reading verification file %s", sFilePath);
				LogFile(NULL,sTraceMsg);

				delete pTransDef;
				pTransDef = NULL;

#ifdef FAIL_ON_ERROR
				nRet = IW_ERR_LOADING_VERICATION;
#endif
			}
		}
		fclose(f);
	}
	else
	{
		nRet = IW_ERR_OPENING_FILE_FOR_READING;
	}

	if (pTransDef)
	{
		delete pTransDef;
		pTransDef = NULL;
	}

	CStdString sTraceMsg;
	sTraceMsg.Format("[CIWVerification::LoadTOTDefinitions] %ld TOT definitions in file", m_transactionDefAry.size());
	TraceMsg(sTraceMsg);

	return nRet;
}

void CIWVerification::DebugOutputVerification()
// This function summarizes the entire contents of the Verification file as loaded into our structures.
{
	CStdString sTraceMsg;
	long nCount;
	CTransactionDefinition *pTrans;
	std::vector<CRecordTypeCount> recTypeCountAry;
	CRuleObj *pRule;
	long i;
	long j;
	char szMin[10];
	char szMax[10];

	sTraceMsg = _T("%%%%%%%%%%%%%%%%%%%%\n");
	OutputDebugString(sTraceMsg);

	nCount = m_transactionDefAry.size();

	// TRANSACTIONS sections of Verification file
	for (i = 0; i < nCount; i++)
	{
		pTrans = &m_transactionDefAry.at(i);

		// Output all TOTs for this transation list
		for (j = 0; j < (int)pTrans->m_TOTArray.size(); j++)
		{
			sTraceMsg.Format("%s", pTrans->m_TOTArray[j]);
			OutputDebugString(sTraceMsg);
			if (j != pTrans->m_TOTArray.size()-1)
			{
				sTraceMsg = _T(", ");
				OutputDebugString(sTraceMsg);
			}
		}

		OutputDebugString(" --> ");

		// Output record type counts for this transaction list
		recTypeCountAry = pTrans->GetRecTypeCountAry();
		for (j=0; j<(int)recTypeCountAry.size(); j++)
		{
			CRecordTypeCount* pRecTypeCount = &recTypeCountAry.at(j);
			if (pRecTypeCount)
			{
				if (pRecTypeCount->nMin == RANGE_NOTSPECIFIED) strcpy_s(szMin, 10, "X");
				else _ltoa_s(pRecTypeCount->nMin, szMin, 10, 10);
				if (pRecTypeCount->nMax == RANGE_NOTSPECIFIED) strcpy_s(szMax, 10, "X");
				else _ltoa_s(pRecTypeCount->nMax, szMax, 10, 10);

				if (pRecTypeCount->nMin == pRecTypeCount->nMax)
				{
					sTraceMsg.Format("%ld:%s ", pRecTypeCount->nRecordType, szMin);
				}
				else
				{
					sTraceMsg.Format("%ld:%s-%s ", pRecTypeCount->nRecordType, szMin, szMax);
				}
				OutputDebugString(sTraceMsg);
			}
		}
		OutputDebugString("\n");
	}

	sTraceMsg = _T("%%%%%%%%%%%%%%%%%%%%\n");
	OutputDebugString(sTraceMsg);

	// FIELDS section of Verification file
	nCount = m_rulesAry.size();
	for (i = 0; i < nCount; i++)
	{
		pRule = &m_rulesAry.at(i);
		sTraceMsg.Format("%s\t%s\t%3s Len(%2ld..%2ld) Occ(%2ld..%2ld) Desc(%s) sca(%s) date(%s) map(%s) Trans %s\n",
						 pRule->GetMNU(), pRule->GetLocation(), pRule->GetCharType(),
						 pRule->GetMinFieldSize(), pRule->GetMaxFieldSize(), pRule->GetMinOccurrences(),
						 pRule->GetMaxOccurrences(), pRule->GetDescription(), pRule->GetSpecialChars(),
						 pRule->GetDateFormat(), pRule->GetMap(), pRule->GetTransactionListString());
		OutputDebugString(sTraceMsg);
	}

	sTraceMsg = _T("%%%%%%%%%%%%%%%%%%%%\n");
	OutputDebugString(sTraceMsg);
}

int CIWVerification::GetNumRulesPerMNU(CStdString &sMNU)
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
	const char						*pData;
	BOOL							bFound;
	BOOL							bCountFound;
	std::vector<CRecordTypeCount>	recTypeCountAry;
	CTransactionDefinition			*pTransDef;
	int								iRecType;
	int								nRecTypeCount;
	char							szErr[512];
	CStdString						sTraceMsg;
	CRuleObj						*pRule;
	BOOL							bFieldsOK;
	BOOL							bMandatory;
	BOOL							bOptional;

#ifdef _DEBUG
	DebugOutputVerification();
#endif

	if (pTrans == NULL) return IW_ERR_TRANSACTION_NOT_LOADED;
	if (!pTrans->IsTransactionLoaded()) return IW_ERR_TRANSACTION_NOT_LOADED;
	if (!pTrans->IsVerificationLoaded()) return IW_ERR_VERIFICATION_NOT_LOADED;

	nRetTmp = pTrans->FindItem(RECORD_TYPE1, 1, TYPE1_TOT, 1, 1, &pData);

	if (nRetTmp == IW_SUCCESS)
	{
		sTOT = pData; // transaction type

		// Locate transaction type in transaction definitions array
		bFound = FALSE;
		for (i = 0; i < (int)m_transactionDefAry.size() && !bFound; i++)
		{
			pTransDef = &m_transactionDefAry.at(i);

			for (j = 0; j < (int)pTransDef->m_TOTArray.size() && !bFound; j++)
			{
				if (!sTOT.CompareNoCase(pTransDef->m_TOTArray.at(j)))
				{
					bFound = TRUE;
					recTypeCountAry = pTransDef->GetRecTypeCountAry();
				}
			}
		}

		if (bFound)
		{
			// Next scan all possible record types, and check the allowed-record-counts array
			for (iRecType = 1; iRecType <= 99; iRecType++)	// 1-based
			{
				nRecTypeCount = 0;
				pTrans->GetRecordTypeCount(iRecType, &nRecTypeCount);

				bCountFound = FALSE;
	
				for (i=0; i<(int)recTypeCountAry.size(); i++)
				{
					CRecordTypeCount* pRecTypeCount = &recTypeCountAry.at(i);
					if (pRecTypeCount && pRecTypeCount->nRecordType == iRecType)
					{
						bCountFound = TRUE;

						// Verify record type ranges
						pTrans->GetRecordTypeCount(pRecTypeCount->nRecordType, &nRecTypeCount);

						if (pRecTypeCount->nMin != RANGE_NOTSPECIFIED)
						{
							if (nRecTypeCount < pRecTypeCount->nMin)
							{
								wsprintf(szErr, "Transaction Type %s must contain at least %ld Type %ld records: it only contains %ld.",
										 sTOT.c_str(), pRecTypeCount->nMin, pRecTypeCount->nRecordType, nRecTypeCount);
								pTrans->AddError(szErr, 0);
								nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
							}
						}
						if (pRecTypeCount->nMax != RANGE_NOTSPECIFIED)
						{
							if (nRecTypeCount > pRecTypeCount->nMax)
							{
								wsprintf(szErr, "Transaction Type %s may contain at most %ld Type %ld records: it contains %ld.",
										 sTOT.c_str(), pRecTypeCount->nMax, pRecTypeCount->nRecordType, nRecTypeCount);
								pTrans->AddError(szErr, 0);
								nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
							}
						}
					}
				}

				if (nRecTypeCount != 0 && !bCountFound)
				// We have a record of an unsupported Record-Type
				{
					wsprintf(szErr, "Transaction Type %s may not contain Type %ld records: it contains %ld of them.",
							 sTOT.c_str(), iRecType, nRecTypeCount);
					pTrans->AddError(szErr, 0);
					nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
				}
			}

			// Now loop through all the rules
			for (i=0; i < (int)m_rulesAry.size(); i++)
			{
				pRule = &m_rulesAry.at(i);

#ifdef _DEBUG
				sTraceMsg.Format("applying %s\t%s\t%3s Len(%2ld..%2ld) Occ(%2ld..%2ld) Trans %s\n",
								 pRule->GetMNU(), pRule->GetLocation(), pRule->GetCharType(),
								 pRule->GetMinFieldSize(), pRule->GetMaxFieldSize(),
								 pRule->GetMinOccurrences(), pRule->GetMaxOccurrences(),
								 pRule->GetTransactionListString());
				OutputDebugString(sTraceMsg);
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
					default:		 sErr = _T("VerifyTransaction: invalid Location Form Type");
									 LogFile(NULL,sErr);
				}
				if (!bFieldsOK) nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;

				// Also check to see if any mandatory fields are missing
				nRetTmp = pTrans->GetRecordTypeCount(pRule->GetRecordType(), &nRecTypeCount);
				if (nRetTmp != IW_SUCCESS) nRecTypeCount = 0; // just in case something went wrong

				// Note: a field is only mandatory if we actually have a record of the same RecordType
				if (nRecTypeCount > 0)
				{
					nRetTmp = pTrans->Get(pRule->GetMNU(), &pData, 1, 1);

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
							FlagFieldError(pTrans, pRule, IW_WARN_REQ_FIELD_MISSING, "Mandatory field not present");
							nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
						}

						// Check for present non-mandatory/non-optional field
						if (nRetTmp == IW_SUCCESS && !bMandatory && !bOptional)
						{
							FlagFieldError(pTrans, pRule, IW_WARN_UNSUPPORT_FIELD_PRESENT, "Unsupported field present");
							nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
						}
					}
				}
			}
		}
		else
		{
			wsprintf(szErr, "Verification file does not contain Transaction Type %s", sTOT.c_str());
			pTrans->AddError(szErr, 0);
			nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
		}
	}
	else
	{
		pTrans->AddError("Failed to find TOT field in Record 1", 0);
		nRet = IW_WARN_TRANSACTION_FAILED_VERIFICATION;
	}

	return nRet;
}

BOOL CIWVerification::VerifyFieldsForm1(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>
// This means that the rule applies to item 1 of all the subfields in the field.
// Each subfield can only have one item.
// There can only be as many subfields as specified by the occurrences value.
{
	BOOL		bRet = TRUE;
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
			bRet &= VerifyfieldContents(pTrans, pRule, NULL);
		}
		else
		{
			nRet = pTrans->GetNumSubfields(iRecType, iRecord, iField, &nSubfieldCount);

			// Note: VerifySubfieldOccurences will add any pertinent errors
			VerifySubfieldOccurences(pTrans, pRule, nSubfieldCount);

			for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
			{
				nRet = pTrans->GetNumItems(iRecType, iRecord, iField, iSubfield, &nItemCount);

				if (nItemCount != 1)
				{
					// Only 1 item allowed per subfield for Location Form 1
					FlagFieldError(pTrans, pRule, IW_WARN_INCORRECT_ITEM_COUNT, "Incorrect item count (%ld), must be 1", nItemCount);
					bRet = FALSE;
				}

				// Check all items, even if there are more than 1
				for (iItem = 1; iItem <= nItemCount; iItem++)	// 1-based
				{
					nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, &pData);

					if (nRet != IW_SUCCESS)
					{
						// TODO: this should be considered a bad problem (?)
					}

					bRet &= VerifyfieldContents(pTrans, pRule, pData);
				}
			}
		}
	}

	return bRet;
}

BOOL CIWVerification::VerifyFieldsForm2(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>.<SUBFIELD>
// This means that the rule applies to all the items in the specified subfield.
// There can only be as many items as specified by the occurrences value.
//
// 
//
{
	BOOL bRet = TRUE;

	return bRet;
}

BOOL CIWVerification::VerifyFieldsForm3(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>.<SUBFIELD NUMBER>.<ITEM NUMBER>
// This means that rule applies to the specified item in the specified subfield.
// The ‘occurrences’ value is meaningless in this context.
{
	BOOL bRet = TRUE;

	return bRet;
}

BOOL CIWVerification::VerifyFieldsForm4(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER>:
// This means that the rule applies to everything in the field.
// There can only be as many subfields as specified by the occurrences value.
// For each subfield, the sum of data lengths is checked against the field size value.
{
	BOOL		bRet = TRUE;
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
	int			nTotalLength;

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

			// Note: VerifySubfieldOccurences will add any pertinent errors
			VerifySubfieldOccurences(pTrans, pRule, nSubfieldCount);

			for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
			{
				nRet = pTrans->GetNumItems(iRecType, iRecord, iField, iSubfield, &nItemCount);

				nTotalLength = 0;

				for (iItem = 1; iItem <= nItemCount; iItem++)	// 1-based
				{
					nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, &pData);

					if (nRet == IW_SUCCESS)
					{
						bRet &= VerifyFieldChars(pTrans, pRule, pData);
						nTotalLength += strlen(pData);
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

BOOL CIWVerification::VerifyFieldsForm5(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER >..<ITEM NUMBER>
// This means that the rule applies to all items numbered by {ITEM NUMBER} in all the subfields.
// The occurrence value is meaningless in this context.
// This type of location index is usually used in conjunction with the location form type of 4
{
	BOOL bRet = TRUE;
	int			nRet;
	int			iRecType;
	int			iRecord;
	int			iField;
	int			iSubfield;
	int			iItem;
	int			nSubfieldCount;
	int			nRecTypeCount;
	const char	*pData;

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
			nRet = pTrans->FindItem(iRecType, iRecord, iField, iSubfield, iItem, &pData);

			if (nRet == IW_SUCCESS)
			// Note that an error may occur, because it's not a given that item 'iItem' exists
			{
				bRet &= VerifyfieldContents(pTrans, pRule, pData);
			}
		}
	}

	return bRet;
}

BOOL CIWVerification::VerifyFieldsForm6(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUMBER >...<OFFSET>
// For binary records (Types 4, 7 and 8), the <OFFSET> value specifies where the item begins relative to the start of the record.
// The size of the binary item is determined by the data type, 1, 2 and 4 byte unsigned integers are supported.
// Text records (Types 1, 2 and 10) can also use this form for fields with a fixed layout.
// For text records, the <OFFSET> value specifies where the item begins with respect to the start of the field.
// The number of subfields is determined by the occurrence value.
// The space reserved for the field is based on the maximum value in the occurrence range.
{
	BOOL		bRet = TRUE;
	
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

			// Note: VerifySubfieldOccurences will add any pertinent errors
			VerifySubfieldOccurences(pTrans, pRule, nSubfieldCount);

			for (iSubfield = 1; iSubfield <= nSubfieldCount; iSubfield++)	// 1-based
			{
				nRet = pTrans->GetNumItems(iRecType, iRecord, iField, iSubfield, &nItemCount);

				if (nItemCount != 1)
				{
					// Only 1 item allowed per subfield for Location Form 1
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_ITEMS, "Too many items (%ld), must be 1", nItemCount);
					bRet = FALSE;
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

BOOL CIWVerification::VerifyFieldsForm7(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule)
// <RECORD TYPE>.<FIELD NUM>.<SUBFIELD>.<ITEM>.<OFFSET>
// This format is similar to Form 6, except that it allows a specific subfield and item number to be associated with the rule.
// It is useful for fields (or subfields) which contain different types of items.
{
	BOOL bRet = TRUE;

	return bRet;
}

BOOL CIWVerification::VerifyfieldContents(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData)
{
	BOOL bRet = TRUE;

	bRet &= VerifyFieldLength(pTrans, pRule, pData);
	bRet &= VerifyFieldChars(pTrans, pRule, pData);
	bRet &= VerifyFieldDateFormat(pTrans, pRule, pData);
	bRet &= VerifyFieldValue(pTrans, pRule, pData);

	return bRet;
}

BOOL CIWVerification::VerifyFieldLength(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData)
//
// Apply min/max field length rules
//
{
	BOOL		bRet = TRUE;
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
		nLen = strlen(pData);

		nMin = pRule->GetMinFieldSize();
		nMax = pRule->GetMaxFieldSize();

		if (nMin == nMax)	// Provide cleaner error message if min is the same as max
		{
			if (nMin != RANGE_NOTSPECIFIED) // Check !=
			{
				if (nLen != nMin)
				{
					if (nLen > nMin)
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, "Invalid length of %ld, should be %ld", nLen, nMin);
					else
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, "Invalid length of %ld, should be %ld", nLen, nMin);
					bRet = FALSE;
				}
			}
		}
		else
		{
			if (nMin != RANGE_NOTSPECIFIED)	// Check Min
			{
				if (nLen < nMin)
				{
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, "Invalid length of %ld, minimum allowed is %ld", nLen, nMin);
					bRet = FALSE;
				}
			}
			if (nMax != RANGE_NOTSPECIFIED)	// Check Max
			{
				if (nLen > nMax)
				{
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, "Invalid length of %ld, maximum allowed is %ld", nLen, nMax);
					bRet = FALSE;
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

BOOL CIWVerification::VerifyFieldLengthTotal(CIWTransaction *pTrans, CRuleObj *pRule, int nTotalLen)
//
// Apply min/max field length rules, for total take across all items in a subfield
//
{
	BOOL		bRet = TRUE;
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
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, "Invalid total subfield length of %ld, should be %ld", nTotalLen, nMin);
					else
						FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, "Invalid total subfield  length of %ld, should be %ld", nTotalLen, nMin);
					bRet = FALSE;
				}
			}
		}
		else
		{
			if (nMin != RANGE_NOTSPECIFIED)	// Check Min
			{
				if (nTotalLen < nMin)
				{
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_DATA_CHARS, "Invalid total subfield length of %ld, minimum allowed is %ld", nTotalLen, nMin);
					bRet = FALSE;
				}
			}
			if (nMax != RANGE_NOTSPECIFIED)	// Check Max
			{
				if (nTotalLen > nMax)
				{
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, "Invalid total subfield length of %ld, maximum allowed is %ld", nTotalLen, nMax);
					bRet = FALSE;
				}
			}
		}
	}

	return bRet;
}

BOOL CIWVerification::VerifyFieldChars(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData)
//
// Apply char type rules
//
{
	BOOL		bRet = TRUE;
	CStdString	sCharType;
	CStdString	sSpecialChars;
	CStdString	sErr;
	CStdString	sVal;
	DWORD		dwVal;
	DWORD		dwValMax;

	sCharType = pRule->GetCharType();
	sSpecialChars = pRule->GetSpecialChars();
	sVal = pData;

	if (!sCharType.CompareNoCase(_T("A")))
	{
		if (!IsAlpha(sVal))
		{
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHA, "Invalid value '%s' for CharType '%s'", sVal.c_str(), sCharType.c_str());
			bRet = FALSE;
		}
	}
	else if (!sCharType.CompareNoCase(_T("N")))
	{
		if (!IsNumeric(sVal))
		{
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_NUMERIC, "Invalid value '%s' for CharType '%s'", sVal.c_str(), sCharType.c_str());
			bRet = FALSE;
		}
	}
	else if (!sCharType.CompareNoCase(_T("AN")))
	{
		if (!IsAlphaNumeric(sVal))
		{
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHANUMERIC, "Invalid value '%s' for CharType '%s'", sVal.c_str(), sCharType.c_str());
			bRet = FALSE;
		}
	}
	else if (!sCharType.CompareNoCase(_T("AS")))
	{
		if (!IsAlphaSpecial(sVal, sSpecialChars))
		{
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHA_SPECIAL, "Invalid value '%s' for CharType '%s', Special Chars '%s'", sVal.c_str(), sCharType.c_str(), sSpecialChars.c_str());
			bRet = FALSE;
		}
	}
	else if (!sCharType.CompareNoCase(_T("NS")))
	{
		if (!IsNumericSpecial(sVal, sSpecialChars))
		{
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_NUMERIC_SPECIAL, "Invalid value '%s' for CharType '%s', Special Chars '%s'", sVal.c_str(), sCharType.c_str(), sSpecialChars.c_str());
			bRet = FALSE;
		}
	}
	else if (!sCharType.CompareNoCase(_T("ANS")))
	{
		if (!IsAlphaNumericSpecial(sVal, sSpecialChars))
		{
			FlagFieldError(pTrans, pRule, IW_WARN_DATA_NOT_ALPHANUMERIC_SPECIAL, "Invalid value '%s' for CharType '%s', Special Chars '%s'", sVal.c_str(), sCharType.c_str(), sSpecialChars.c_str());
			bRet = FALSE;
		}
	}
	else if (!sCharType.CompareNoCase(_T("B1"))  || !sCharType.CompareNoCase(_T("B2")) ||
			 !sCharType.CompareNoCase(_T("B4")))
	{
		if (sCharType.GetAt(1) == _T('1')) dwValMax = 255;
		if (sCharType.GetAt(1) == _T('2')) dwValMax = 65535;
		if (sCharType.GetAt(1) == _T('4')) dwValMax = 4294967295;
		if (pData != NULL)
		{
			dwVal = _ttol(pData);
			if (dwVal > dwValMax)
			{
				FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_DATA_CHARS, "Invalid value %ld for CharType %s", dwVal, sCharType.c_str());
				bRet = FALSE;
			}
		}
	}
	else
	{
		// Unknown CharType code
		sErr.Format(_T("[CIWVerification::VerifyField] %s, unknown CharType: %s\n"), pRule->GetMNU(), sCharType);
		LogFile(NULL,sErr);
		OutputDebugString(sErr);
	}

	return bRet;
}

BOOL CIWVerification::VerifyFieldDateFormat(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData)
{
	BOOL		bRet = TRUE;
	BOOL		bAllowNil = FALSE;
	CStdString	sFmt;
	CStdString	sVal;
	CStdString	sY, sM, sD;
	long		iY, iM, iD;
	long		lY, lM, lD;

	sFmt = pRule->GetDateFormat();
	sVal = pData;

	// If there is no date format specified there's nothing to check
	if (sFmt == "") return TRUE;

	// See if there's the "Z" prefix to indicate the whole date can be a string of '0's
	if (sFmt.GetAt(0) == 'Z')
	{
		if (sVal == "00000000") return TRUE;	// 8 0's is fine with the "Z" prefix
		bAllowNil = TRUE;
	}

	// Get indexes of individual items. These have been checked when reading-in
	// the verification file so none of them should be -1.
	iY = sFmt.Find("CCYY");
	if (iY == -1)
	{
		iY = sFmt.Find("YYYY");
	}
	iM = sFmt.Find("MM");
	iD = sFmt.Find("DD");

	if (bAllowNil)
	{
		iY--; iM--; iD--;	// Shift all index counters down 1
	}

	sY = sVal.Mid(iY, 4);	// extract year string
	sM = sVal.Mid(iM, 2);	// extract month string
	sD = sVal.Mid(iD, 2);	// extract day stting

	lY = atol(sY.c_str());	// extract year number
	lM = atol(sM.c_str());	// extract month number
	lD = atol(sD.c_str());	// extract day number

	// Year must be anywhere from 1900 to 2099
	if (lY < 1900 || lY > 2099)
	{
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATE, "Invalid year %ld", lY);
		bRet = FALSE;
	}

	// Month between 1 and 12
	if (lM < 1 || lM > 12) 
	{
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATE, "Invalid month %ld", lM);
		bRet = FALSE;
	}

	// Day between 1 and ...
	if (lD < 1 || lD > DaysInMonth(lY, lM))
	{
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATE, "Invalid day %ld", lD);
		bRet = FALSE;
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
		isLeapYear = (y % 4 == 0 && y % 100 != 0 || (y % 400 == 0));
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

BOOL CIWVerification::VerifyFieldValue(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData)
// Check Mandatory Map contents, if applicable
{
	std::vector<CStdString> vals;
	CStdString				sVal;
	BOOL					bFound = false;

	vals = pRule->GetMapVals();

	if (vals.size() == 0) return TRUE; // nothing to check, no mandatory map exists

	for (unsigned int i = 0; i < vals.size(); i++)
	{
		sVal = vals.at(i);
		if (strcmp(sVal, pData) == 0)
		{	
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		FlagFieldError(pTrans, pRule, IW_WARN_INVALID_DATA, "'%s': invalid data from mandatory map", pData);
	}

	return bFound;
}

BOOL CIWVerification::VerifySubfieldOccurences(CIWTransaction *pTrans, CRuleObj *pRule, int nSubfieldCount)
// Given a rule, make sure the number of subfields falls in the legal range. Note that we never consider
// 0 an error, since this just means the subfield is not present, and mandatory fields are checked elsewhere.
{
	BOOL		bRet = TRUE;
	int			nMinOccurences;
	int			nMaxOccurences;

	if (nSubfieldCount == 0) return TRUE;

	nMinOccurences = pRule->GetMinOccurrences();
	nMaxOccurences = pRule->GetMaxOccurrences();

	if (nMinOccurences == nMaxOccurences) // Custom error message for min=max
	{
		if (nMinOccurences != RANGE_NOTSPECIFIED) // Check !=
		{
			if (nSubfieldCount != nMinOccurences)
			{
				if (nSubfieldCount < nMinOccurences)
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_SUBFIELDS, "Too few subfields (%ld), must be %ld", nSubfieldCount, nMinOccurences);
				else
					FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_SUBFIELDS, "Too many subfields (%ld), must be %ld", nSubfieldCount, nMinOccurences);
				bRet = FALSE;
			}
		}
	}
	else
	{
		if (nMinOccurences != RANGE_NOTSPECIFIED) // Check min
		{
			if (nSubfieldCount < nMinOccurences)
			{
				FlagFieldError(pTrans, pRule, IW_WARN_TOO_FEW_SUBFIELDS, "Too few subfields (%ld), minimum allowed is %ld", nSubfieldCount, nMinOccurences);
				bRet = FALSE;
			}
		}
		if (nMaxOccurences != RANGE_NOTSPECIFIED) // Check max
		{
			if (nSubfieldCount > nMaxOccurences)
			{
				FlagFieldError(pTrans, pRule, IW_WARN_TOO_MANY_SUBFIELDS, "Too many subfields (%ld), maximum allowed is %ld", nSubfieldCount, nMaxOccurences);
				bRet = FALSE;
			}
		}
	}

	return bRet;
}

BOOL CIWVerification::IsAlpha(CStdString& s)
{
	BOOL bRet = TRUE;
	int  i;

	for (i=0; i<s.GetLength(); i++)
	{
		if (!_istalpha(s.GetAt(i)))
		{
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}

BOOL CIWVerification::IsNumeric(CStdString& s)
{
	BOOL bRet = TRUE;
	int  i;

	for (i=0; i<s.GetLength(); i++)
	{
		if (!_istdigit(s.GetAt(i)))
		{
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}

BOOL CIWVerification::IsAlphaNumeric(CStdString& s)
{
	BOOL bRet = TRUE;
	int  i;

	for (i=0; i<s.GetLength(); i++)
	{
		if (!_istdigit(s.GetAt(i)) && !_istalpha(s.GetAt(i)))
		{
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}

BOOL CIWVerification::IsPrintable(CStdString& s, BOOL bAllowControlChars)
{
	BOOL bRet = TRUE;
	char c;
	int  i;

	for (i=0; i<s.GetLength(); i++)
	{
		c = s.GetAt(i);
		if (!_istprint(c))
		{
			if (bAllowControlChars)
			{
				if  (c != 0x09 && c != 0x0A && c != 0x0D) // TAB, LF, CR
				{
					bRet = FALSE;
					break;
				}
			}
			else
			{
				bRet = FALSE;
				break;
			}
		}
	}

	return bRet;
}

BOOL CIWVerification::IsAlphaSpecial(CStdString& s, CStdString& sSpecial)
// Note: sca="PRINT" is a special code for "all printable characters" and sca="PRINTCTRL"
// means all printable characters + CR + LF + TAB.
{
	BOOL bRet = TRUE;
	char c;
	int	 i;

	if (!sSpecial.CompareNoCase("PRINT"))
	{
		return IsPrintable(s, false);
	}
	else if (!sSpecial.CompareNoCase("PRINTCTRL"))
	{
		return IsPrintable(s, true);
	}
	else
	{
		for (i=0; i<s.GetLength(); i++)
		{
			c = s.GetAt(i);
			// If the char is not alphabetic and it can't be found in the set of
			// Special Characters the string fails the test.
			if (!_istalpha(c) && sSpecial.Find(c) == -1)
			{
				bRet = FALSE;
				break;
			}
		}

		return bRet;
	}
}

BOOL CIWVerification::IsNumericSpecial(CStdString& s, CStdString& sSpecial)
// Note: sca="PRINT" is a special code for "all printable characters" and sca="PRINTCTRL"
// means all printable characters + CR + LF + TAB.
{
	BOOL bRet = TRUE;
	char c;
	int	 i;

	if (!sSpecial.CompareNoCase("PRINT"))
	{
		return IsPrintable(s, false);
	}
	else if (!sSpecial.CompareNoCase("PRINTCTRL"))
	{
		return IsPrintable(s, true);
	}
	else
	{
		for (i=0; i<s.GetLength(); i++)
		{
			c = s.GetAt(i);
			// If the char is not numeric and it can't be found in the set of
			// Special Characters the string fails the test.
			if (!_istdigit(c) && sSpecial.Find(c) == -1)
			{
				bRet = FALSE;
				break;
			}
		}

		return bRet;
	}
}

BOOL CIWVerification::IsAlphaNumericSpecial(CStdString& s, CStdString& sSpecial)
// Note: sca="PRINT" is a special code for "all printable characters" and sca="PRINTCTRL"
// means all printable characters + CR + LF + TAB.
{
	BOOL bRet = TRUE;
	char c;
	int	 i;

	if (!sSpecial.CompareNoCase("PRINT"))
	{
		return IsPrintable(s, false);
	}
	else if (!sSpecial.CompareNoCase("PRINTCTRL"))
	{
		return IsPrintable(s, true);
	}
	else
	{
		for (i=0; i<s.GetLength(); i++)
		{
			c = s.GetAt(i);
			// If the char is not alphabetic, not numeric and it can't be found in the set of
			// Special Characters the string fails the test.
			if (!_istalpha(c) && !_istdigit(c) && sSpecial.Find(c) == -1)
			{
				bRet = FALSE;
				break;
			}
		}

		return bRet;
	}
}

void CIWVerification::FlagFieldError(CIWTransaction *pTrans, CRuleObj* pRule, int nErrCode, const TCHAR *szFormat, ...)
{
	#define cchErrMax (MAXERRORLEN - 256)
	TCHAR szErr[cchErrMax];
	TCHAR szMsg[MAXERRORLEN];

	va_list args;
	va_start(args, szFormat);
	_vsntprintf_s(szErr, cchErrMax, cchErrMax, szFormat, args);
	va_end(args);

	_stprintf_s(szMsg, MAXERRORLEN, _T("Field %s (%s): %s"), pRule->GetMNU().c_str(), pRule->GetLocation().c_str(), szErr);

	pTrans->AddError(szMsg, nErrCode);
	OutputDebugString(szMsg);
}

CRuleObj *CIWVerification::GetRule(const char *pMNU)
{
	CRuleObj *pRet = NULL;
	int nCount = m_rulesAry.size();
	CRuleObj *pTemp;
	CStdString stMNU;

	for (int i = 0; i < nCount; i++)
	{
		pTemp = &m_rulesAry.at(i);

		stMNU = pTemp->GetMNU();
		if (!stMNU.CompareNoCase(pMNU))
		{
			pRet = pTemp;
			break;
		}
	}
	
	return pRet;
}

int CIWVerification::GetMNULocation(const char *pMNU, int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item)
{
	int nRet = IW_ERR_MNEMONIC_NOT_FOUND;
	CRuleObj *pRule = GetRule(pMNU);

	if (pRule)
	{
		if (pRule->GetLocation(inputIndex, inputRecordIndex, recordType, recordIndex, field, subField, item))
			nRet = IW_SUCCESS;
	}

	return nRet;
}

int CIWVerification::GetTransactionCategories(int DataArraySize, const char **ppDataArray, int *pEntries)
{
	int nRet = IW_SUCCESS;
	int nSize = m_transactionDefAry.size();
	CTransactionDefinition *pTransDef = NULL; 
	
	const char *pCat = ppDataArray ? *ppDataArray : NULL;
	int nPos = 0;
	BOOL bCopy = DataArraySize > 0;
	std::vector<CStdString> sCategoryAry; // hack to workaround poor logic in parsing
	int nCount, j;
	BOOL bFound = FALSE;
	for (int i = 0; i < nSize; i++)
	{
		pTransDef = &m_transactionDefAry.at(i);

		nCount = sCategoryAry.size();
		bFound = FALSE;
		for (j = 0; j < nCount && !bFound; j++)
		{
			if (!sCategoryAry.at(j).CompareNoCase(pTransDef->m_sCategory))
				bFound = TRUE; // we've already counted this category
		}

		if (!bFound)
		{
			if (bCopy)
			{
				if (nPos < DataArraySize)
				{
					ppDataArray[nPos] = (LPCSTR)pTransDef->m_sCategory;
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

int CIWVerification::GetTransactionTypes(int DataArraySize, const char **ppDataArray, 
																const char **ppDescArray, int *pEntries, const char *pCategory)
{
	int						nRet = IW_SUCCESS;
	int						nSize = m_transactionDefAry.size();
	CTransactionDefinition	*pTransDef = NULL; 
	CStdString				sCategory(pCategory);
	int						nPos = 0;
	BOOL					bCopy = DataArraySize > 0;

	for (int i = 0; i < nSize; i++)
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
						ppDataArray[nPos] = (LPCSTR)pTransDef->m_TOTArray.at(j);
						if (ppDescArray)
							ppDescArray[nPos] = (LPCSTR)pTransDef->m_TOTLabelArray.at(j);
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
							ppDataArray[nPos] = (LPCSTR)pTransDef->m_TOTArray.at(j);
							if (ppDescArray)
								ppDescArray[nPos] = (LPCSTR)pTransDef->m_TOTLabelArray.at(j);
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

int CIWVerification::GetRecordTypeOccurences(int DataArraySize, int *piRecordType, int *piMinOccurences, int *piMaxOccurences, int *pEntries, const char *pCategory)
{
	int								nRet = IW_SUCCESS;
	int								nSize = m_transactionDefAry.size();
	CTransactionDefinition			*pTransDef = NULL; 
	std::vector<CRecordTypeCount>	recTypeCountAry;
	BOOL							bCopy = DataArraySize > 0;
	CStdString						sCategory(pCategory);

	if (!pEntries) return IW_ERR_NULL_POINTER;

	if (DataArraySize)
	{
		if (!piRecordType || !piMinOccurences || !piMaxOccurences)
		{
			return IW_ERR_NULL_POINTER;
		}
	}

	for (int i = 0; i < nSize; i++)
	{
		pTransDef = &m_transactionDefAry.at(i);

		if (!sCategory.CompareNoCase(pTransDef->m_sCategory)) 
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
						piMinOccurences[j] = pRecTypeCount->nMin;
						piMaxOccurences[j] = pRecTypeCount->nMax;
					}
				}
			}
		}
	}

	return nRet;
}

#define MAXLEN_MNU			32
#define	MAXLEN_DESC			1024
#define	MAXLEN_LONGDESC		2048
#define MAXLEN_CHARTYPE		64
#define MAXLEN_DATEFORMAT	64

int CIWVerification::GetMnemonics(const char* TransactionType, int DataArraySize, const char** ppDataArray, const char** ppDescArray, int* pEntries)
{
	int			nRet = IW_SUCCESS;
	CRuleObj	*pRule;
	int			nPos = 0;
	BOOL		bCopy = DataArraySize > 0;
	CStdString	sTOT(TransactionType);
	// Kludge: We follow the same interface where strings get passed back as pre-allocated memory
	// hence we keep 500 static slots on hand.
	static char szMNU[500][MAXLEN_MNU];
	static char szMNUDescription[500][1024];

	for (int i=0; i < (int)m_rulesAry.size(); i++)
	{
		pRule = &m_rulesAry.at(i);

		if (pRule->IsMandatory(sTOT) || pRule->IsOptional(sTOT))
		{
			if (bCopy)
			{
				if (nPos < DataArraySize)
				{
					strncpy(szMNU[nPos], pRule->GetMNU(), MAXLEN_MNU); szMNU[nPos][MAXLEN_MNU-1] = '\0';
					ppDataArray[nPos] = szMNU[nPos];

					strncpy(szMNUDescription[nPos], pRule->GetDescription(), MAXLEN_DESC); szMNUDescription[nPos][MAXLEN_DESC-1] = '\0';
					ppDescArray[nPos] = szMNUDescription[nPos];
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

static const char s_rgszAutomaticMNU[][10] =
{
	 "T1_LEN",  "T1_VER",  "T1_CNT",  "T1_TOT",
	 "T2_LEN",  "T2_IDC",
	 "T4_LEN",  "T4_IDC",  "T4_HLL",  "T4_VLL",  "T4_GCA",  "T4_DAT",
	 "T7_LEN",  "T7_IDC",  "T7_HLL",  "T7_VLL",  "T7_GCA",  "T7_DAT",
	 "T9_LEN",  "T9_IDC",
	"T10_LEN", "T10_IDC", "T10_HLL", "T10_VLL", "T10_SLC", "T10_HPS", "T10_VPS", "T10_CGA", "T10_DAT",
	"T14_LEN", "T14_IDC", "T14_HLL", "T14_VLL", "T14_SLC", "T14_HPS", "T14_VPS", "T10_CGA", "T14_BPX", "T10_DAT",
	"T16_LEN", "T16_IDC", "T16_HLL", "T16_VLL", "T16_SLC", "T16_HPS", "T16_VPS", "T16_CGA", "T16_BPX", "T16_DAT"
};

static int s_nAutomaticMNUs = sizeof(s_rgszAutomaticMNU)/sizeof(s_rgszAutomaticMNU[0]);;
  
int CIWVerification::GetRuleRestrictions(const char* TransactionType, const char* pMnemonic, int* pRecordType, int* pField, int* pSubfield,
										 int* pItem, const char** ppDesc, const char** ppLongDesc, const char** ppCharType,
										 const char** ppDateFormat, int* pSizeMin, int* pSizeMax, int* pOccMin, int* pOccMax, int* pOffset,
										 bool* pAutomaticallySet)
{
	int			nRet = IW_ERR_MNEMONIC_NOT_FOUND;
	CRuleObj	*pRule;
	CStdString	sTOT(TransactionType);
	bool		bAuto;
	// We follow the same interface where strings get passed back as pre-allocated memory
	// hence we keep static slots on hand.
	static char szMNUDescription[MAXLEN_DESC];
	static char szMNULongDescription[MAXLEN_LONGDESC];
	static char szMNUCharType[64];
	static char szMNUDateFormat[64];

	if (TransactionType == NULL) return IW_ERR_NULL_POINTER;
	if (pMnemonic == NULL) return IW_ERR_NULL_POINTER;
	if (pRecordType == NULL) return IW_ERR_NULL_POINTER;
	if (pField == NULL) return IW_ERR_NULL_POINTER;
	if (pSubfield == NULL) return IW_ERR_NULL_POINTER;
	if (pItem == NULL) return IW_ERR_NULL_POINTER;
	if (ppDesc == NULL) return IW_ERR_NULL_POINTER;
	if (ppLongDesc == NULL) return IW_ERR_NULL_POINTER;
	if (ppCharType == NULL) return IW_ERR_NULL_POINTER;
	if (ppDateFormat == NULL) return IW_ERR_NULL_POINTER;
	if (pSizeMin == NULL) return IW_ERR_NULL_POINTER;
	if (pSizeMax == NULL) return IW_ERR_NULL_POINTER;
	if (pOccMin == NULL) return IW_ERR_NULL_POINTER;
	if (pOccMax == NULL) return IW_ERR_NULL_POINTER;
	if (pOffset == NULL) return IW_ERR_NULL_POINTER;
	if (pAutomaticallySet == NULL) return IW_ERR_NULL_POINTER;

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

				// string parameters
				strncpy(szMNUDescription, pRule->GetDescription(), MAXLEN_DESC); szMNUDescription[MAXLEN_DESC-1] = '\0';
				*ppDesc = szMNUDescription;
				strncpy(szMNULongDescription, pRule->GetLongDescription(), MAXLEN_LONGDESC); szMNULongDescription[MAXLEN_LONGDESC-1] = '\0';
				*ppLongDesc = szMNULongDescription;
				strncpy(szMNUCharType, pRule->GetCharType(), MAXLEN_CHARTYPE); szMNUCharType[MAXLEN_CHARTYPE-1] = '\0';
				*ppCharType = szMNUCharType;
				strncpy(szMNUDateFormat, pRule->GetDateFormat(), MAXLEN_DATEFORMAT); szMNUDateFormat[MAXLEN_DATEFORMAT-1] = '\0';
				*ppDateFormat = szMNUDateFormat;

				// Is field automatically managed by OpenEBTS? Scan array.
				bAuto = false;
				for (int j = 0; j < s_nAutomaticMNUs; j++)
				{
					if (pRule->GetMNU().CompareNoCase(s_rgszAutomaticMNU[j]) == 0)
					{
						bAuto = TRUE;
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
