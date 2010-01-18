
#include "stdafx.h"
#include "RuleObj.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "Common.h"

#pragma warning(disable : 4786)
#include <boost/regex.hpp>


CRuleObj::CRuleObj()
{
	m_transactionDefault = TRANS_UNKNOWN;
	m_nMinFieldSize = m_nMaxFieldSize = m_nMinOccurrences = m_nMaxOccurrences = -1;
	m_nLocFormType = LOC_FORM_UNKOWN;
	m_nRecordType = m_nField = m_nSubField = m_nItem = m_nOffset = 1;
}

CRuleObj::~CRuleObj()
{
}

BOOL CRuleObj::SetData(CStdString sFilePath, CStdString& sTransactionList, CStdString& sLocation, CStdString& sMNU, CStdString& sCharType,
					   CStdString& sFieldSize, CStdString& sOccurrences, CStdString& sDescription, CStdString& sLongDescription,
					   CStdString& sSpecialChars, CStdString& sDateFormat, CStdString& sMMap, CStdString& sOMap,
					   CStdString& sTags, CStdString& sErr)
{
	CStdString sTemp;
	BOOL bRet = FALSE;

	sErr.Empty();

	// extract the TOT's this rule applies to
	if (SetTransactions(sTransactionList))
	{
		if (SetLocation(sLocation))
		{
			if (SetMNU(sMNU))
			{
				if (SetCharType(sCharType))
				{
					if (SetFieldSize(sFieldSize))
					{
						if (SetOccurrences(sOccurrences))
						{
							if (SetOptionalDescription(sDescription))
							{
								if (SetOptionalLongDescription(sLongDescription))
								{
									if (SetOptionalSpecialChars(sSpecialChars))
									{
										if (SetOptionalDateFormat(sDateFormat))
										{
											if (SetOptionalMMap(sMMap, sFilePath))
											{
												if (SetOptionalOMap(sOMap, sFilePath))
												{
													if (SetTags(sTags))
													{
														bRet = TRUE;
													}
													else
													{
														sErr.Format("%s, invalid tag value: %s", m_sMNU, sTags.Left(60));
														LogFile(NULL,sErr);
													}
												}
												else
												{
													sErr.Format("%s, invalid mmap value: %s", m_sMNU, sMMap);
													LogFile(NULL,sErr);
												}
											}
										}
										else
										{
											sErr.Format("%s, invalid date format value: %s", m_sMNU, sDateFormat);
											LogFile(NULL,sErr);
										}
									}
									else
									{
										sErr.Format("%s, invalid sca value: %s", m_sMNU, sSpecialChars);
										LogFile(NULL,sErr);
									}
								}
							}
							else
							{
								sErr.Format("%s, invalid description value: %s", m_sMNU, sDescription);
								LogFile(NULL,sErr);
							}
						}
						else
						{
							sErr.Format("%s, invalid occurrence value: %s", m_sMNU, sOccurrences);
							LogFile(NULL,sErr);
						}
					}
					else
					{
						sErr.Format("%s, invalid field size: %s", m_sMNU, sFieldSize);
						LogFile(NULL,sErr);
					}
				}
				else
				{
					sErr.Format("%s, invalid char type: %s", m_sMNU, sCharType);
					LogFile(NULL,sErr);
				}
			}
			else
			{
				sErr.Format("Invalid MNU: %s", sMNU);
				LogFile(NULL,sErr);
			}
		}
		else
		{
			sErr.Format("Invalid location: %s", sLocation);
			LogFile(NULL,sErr);
		}
	}

#ifdef _DEBUG
	DumpObject();
#endif // _DEBUG

	return bRet;
}

void CRuleObj::DumpObject()
{
	CStdString sMsg;

	sMsg.Format("[DumpObject] ==> MNU: %s, Location: %s, chartype %s, len min %ld, max %ld, occ min %ld, max %ld desc(%s) sca(%s) date(%s) map(%s)",
				m_sMNU, m_sLocation, m_sCharType, m_nMinFieldSize, m_nMaxFieldSize, m_nMinOccurrences, m_nMaxOccurrences,
				m_sDescription, m_sSpecialChars, m_sDateFormat, GetMMap());
	LogFile(NULL,sMsg);

	UINT							nKey;
	CStdString						sTemp, sMNU;
	int								count = 0;
	map<CStdString, UINT>::iterator it;

	for (it = m_transactionList.begin(); it != m_transactionList.end(); it++)
	{
		sMNU = (*it).first;
		nKey = (*it).second;

		if (count++ < 10)
			sTemp += sMNU + ", ";
		else
			break;
	}

	sMsg.Format("[DumpObject] %s, TOT's: %s", m_sMNU, sTemp);
	LogFile(NULL, sMsg);
}

CStdString CRuleObj::GetTransactionListString()
// Useful for debug output/logging, not much else
{
	map<CStdString, UINT>::iterator it;
	UINT nMandatory;
	CStdString sTemp;
	CStdString sMNU;

	if (m_transactionList.empty())
	// No specifics, at least output default, mandatory or optional
	{
		sTemp += (m_transactionDefault == TRANS_MANDATORY) ? "(M)" : "(O)";
	}
	else
	{
		for (it = m_transactionList.begin(); it != m_transactionList.end(); it++)
		{
			sMNU = (*it).first;
			nMandatory = (*it).second;

			sTemp += sMNU;
			sTemp += (nMandatory == TRANS_MANDATORY) ? "(M)" : "(O)";
			sTemp += ", ";
		}
	}

	return sTemp;
}

BOOL CRuleObj::SetOptionalDescription(CStdString& sDescription)
{
	m_sDescription = sDescription;

	return TRUE;
}

BOOL CRuleObj::SetOptionalLongDescription(CStdString& sLongDescription)
{
	m_sLongDescription = sLongDescription;

	return TRUE;
}

BOOL CRuleObj::SetOptionalSpecialChars(CStdString& sSpecialChars)
{
	m_sSpecialChars = sSpecialChars;

	return TRUE;
}

BOOL CRuleObj::SetOptionalDateFormat(CStdString& sDateFormat)
// Do a brief format verification. For now we support the "Z" prefix and the abbreviations "CCYY" (or "YYYY"),
// "MM" and "DD.
{
	char		c;
	CStdString	sAllowed = "CYMD";

	for (int i = 0; i < sDateFormat.GetLength(); i++)
	{
		c = sDateFormat.GetAt(i);

		if (sAllowed.Find(c) == -1)
		{
			// Invalid char found, unless it's the first char and it's a "Z"
			if (!(i == 0 && c == 'Z'))
			{
				return FALSE;
			}
		}
	}

	if (sDateFormat != "")
	{
		// Make sure we have a year, month and day, otherwise the date can't be valid
		if (sDateFormat.Find("YYYY") == -1 && sDateFormat.Find("CCYY") == -1) return FALSE;
		if (sDateFormat.Find("MM") == -1) return FALSE;
		if (sDateFormat.Find("DD") == -1) return FALSE;
	}

	m_sDateFormat = sDateFormat;

	return TRUE;
}

BOOL CRuleObj::SetOptionalMMap(CStdString& sMMap, CStdString& sFilePath)
{
	return SetOptionalMap(sMMap, sFilePath, m_mmapName, m_mmapDesc);
}

BOOL CRuleObj::SetOptionalOMap(CStdString& sOMap, CStdString& sFilePath)
{
	return SetOptionalMap(sOMap, sFilePath, m_omapName, m_omapDesc);
}

BOOL CRuleObj::SetOptionalMap(CStdString& sMap, CStdString& sFilePath,
							  std::vector<CStdString>& mapValNames,
							  std::vector<CStdString>& mapValDescriptions)
// Populate map from the values inside mmap or omap tag, such as mmap="<Value>[:<Desc>]|<Value>[:<Desc]|..."
// Support external file as well, as in mmap="file://<Path to external file>"
{
	char		c;
	bool		bInsideValue;
	CStdString	sName;
	CStdString	sDesc;
	CStdString	sFilename;
	CStdString	sFilePrefix = "file://";
	int			i;
	int			j;

	if (sMap.IsEmpty()) return TRUE;	// no map, nothing to copy

	/*
	// We no longer do this verficiation because many lines in the original verification files
	// forget to include '|' but contain a carriage return instead.

	int			npipes = 0;
	int			ncolons = 0;

	// For a quick verification, we count the number of ':'s and '|'s. There should be one more ':' than '|'
	for (i = 0; i < sMMap.GetLength(); i++)
	{
		c = sMMap.GetAt(i);

		if (c == ':') ncolons++;
		else if (c == '|') npipes++;
	}
	if (ncolons != npipes+1) return FALSE;
	*/

	// First check if the info is in a file
	if (sMap.Find(sFilePrefix) == 0) // (starts at position 0)
	{
		char		szDrive[_MAX_DRIVE];
		char		szDir[_MAX_DIR];
		char		szPath[_MAX_DIR];
		FILE		*f;
		char		*pFile;
		char		*pFileSave;
		long		lSize;

		// Open file in same folder as main verification file and read-in contents

		sFilename = sMap.Right(sMap.GetLength() - sFilePrefix.GetLength());

		_splitpath(sFilePath, szDrive, szDir, NULL, NULL);
		strcpy(szPath, szDrive);
		strcat(szPath, szDir);
		strcat(szPath, sFilename);

		f = fopen(szPath, "rb");
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			lSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			pFile = new char [lSize+2];
			memset(pFile, '\0', lSize+2);
			pFileSave = pFile;
			fread(pFile, 1, lSize, f);
			fclose(f);

			pFile[lSize] =  0x0A; // end file with carriage return so parser can include last element easily

			bInsideValue = true; // the first char of the string is the first char of the first value

			for (i = 0; i < lSize+1; i++)
			{
				c = pFile[i];
				if (c == 0x09)
				{
					bInsideValue = FALSE;	// end of value, about to enter description

					// Save current value name in array and reset it
					sName.Trim();
					if (!sName.IsEmpty()) mapValNames.push_back(sName);
					sName.Empty();
				}
				else if (c == 0x0A)
				{
					bInsideValue = TRUE;	// end of description, about to enter value

					// save current value descriptions is array and reset it
					sDesc.Trim();
					if (!sDesc.IsEmpty()) mapValDescriptions.push_back(sDesc);
					sDesc.Empty();
				}
				else
				{
					if (bInsideValue)
					{
						sName += c;		// add new char to present name
					}
					else
					{
						sDesc += c;		// add new char to present description
					}
				}
			}

			delete [] pFileSave;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		// Values and descriptions are inside the verification file

		bInsideValue = true; // the first char of the string is the first char of the first value

		sMap += "|";	// succeed string with marker to allow easy parsing of final element

		for (i = 0; i < sMap.GetLength(); i++)
		{
			c = sMap.GetAt(i);

			if (c == ':')
			{
				bInsideValue = FALSE;	// end of value, about to enter description

				// Save current name value in array and reset it
				sName.Trim();
				mapValNames.push_back(sName);
				sName.Empty();
			}
			else if (c == '|')
			{
				// Skip over all carriage returns and white space and back slashes
				for (j = i+1; j < sMap.GetLength(); j++)
				{
					c = sMap.GetAt(j);
					if (c != 0x0D && c != 0x0A && c != 0x09 && c != ' ' && c != '\\')
					{
						break;
					}
				}
				i = j-1;	// skip to new position, backtracking to just before

				bInsideValue = TRUE;	// end of description, about to enter value

				// save current value descriptions is array and reset it
				sDesc.Trim();
				mapValDescriptions.push_back(sDesc);
				sDesc.Empty();
			}
			else
			{
				if (bInsideValue)
				{
					sName += c;		// add new char to present name
				}
				else
				{
					sDesc += c;		// add new char to present description
				}
			}
		}
	}

	return TRUE;
}

CStdString CRuleObj::GetMMap()
// Just a debug-outputable string that contains the map values
{
	CStdString sRet;

	for (unsigned int i = 0; i < m_mmapName.size(); i++)
	{
		sRet += m_mmapName.at(i);
		if (i != m_mmapName.size()-1) sRet += ",";
	}

	return sRet;
}

BOOL CRuleObj::SetTags(CStdString& sTags)
{
#pragma message(" ===> CRuleObj, Tag support not implemented")
	return TRUE;
}

BOOL CRuleObj::SetFieldSize(CStdString& sFieldSize)
{
	BOOL bRet = FALSE;

	int nMin = RANGE_INVALID, nMax = RANGE_INVALID;

	if (SetRange(sFieldSize,&nMin,&nMax))
	{
		m_nMinFieldSize = nMin;
		m_nMaxFieldSize = nMax;
		bRet = TRUE;
	}

	return bRet;
}

BOOL CRuleObj::SetOccurrences(CStdString& sOccurence)
{
	BOOL bRet = FALSE;

	int nMin = RANGE_INVALID, nMax = RANGE_INVALID;

	if (SetRange(sOccurence,&nMin,&nMax))
	{
		m_nMinOccurrences = nMin;
		m_nMaxOccurrences = nMax;
		bRet = TRUE;
	}

	return bRet;
}

BOOL CRuleObj::SetRange(CStdString& sRange, int *pMin, int *pMax)
{
	BOOL bRet = FALSE;
	CStdString sErr;
	int nMin = RANGE_INVALID, nMax = RANGE_INVALID;

	if (sRange != _T(""))
	{
		CStdString sTemp(sRange);
		int nPos;
		
		if ((nPos = sTemp.Find('-')) != -1)
		{
			CStdString sMin, sMax;
			
			sMin = sTemp.Left(nPos);
			sMax = sTemp.Mid(nPos+1);

			if (GetRangeValue(sMin,&nMin))
			{				
				if (GetRangeValue(sMax,&nMax))
				{
					*pMin = nMin;
					*pMax = nMax;
					bRet = TRUE;
				}
			}
		}
		else
		{
			if (GetRangeValue(sTemp,&nMin))
			{
				*pMin = *pMax = nMin;
				bRet = TRUE;
			}
		}
	}

	//("%s range min %ld, max %ld\n", m_sMNU, *pMin, *pMax);

	return bRet;
}

BOOL CRuleObj::GetRangeValue(CStdString& sToken, int *pValue)
{
	BOOL bRet = FALSE;
	CStdString sTemp(sToken);
	BOOL bErr = FALSE;
	char ch;
	BOOL bNotSpecified = FALSE;

	sTemp.TrimLeft();
	sTemp.TrimRight();
	sTemp.MakeUpper();

	int nLen = sTemp.GetLength();
	for (int i = 0; i < nLen && !bErr; i++)
	{
		ch = sTemp.GetAt(i);
		switch (i)
		{
			case 0:
				if (ch == 'X')
					bNotSpecified = TRUE;
				else if (!isdigit(ch))
					bErr = TRUE;
				break;
			default:
				if (bNotSpecified)
					bErr = TRUE; // shouldn't be here
				else if (!isdigit(ch))
					bErr = TRUE; // or here
				break;
		}
	}
	
	if (!bErr)
	{
		if (bNotSpecified)
			*pValue = RANGE_NOTSPECIFIED;
		else
			*pValue = atol(sTemp);
		bRet = TRUE;
	}

	return bRet;
}

BOOL CRuleObj::SetCharType(CStdString& sCharType)
{
	BOOL bRet = FALSE;
	CStdString sErr;

	if (sCharType != _T(""))
	{
		int nPos = 0;
		int nLen = sCharType.GetLength();
		BOOL bErr = FALSE;
		CStdString sTemp(sCharType);
		char ch, prevChar;

		sTemp.MakeUpper();
		for (int i = 0; i < nLen && !bErr; i++)
		{
			ch = sTemp.GetAt(i);
			switch (i)
			{
				case 0:
					if (ch == 'A' || ch == 'N' || ch == 'B')
						prevChar = ch;
					else
						bErr = TRUE;
					break;
				case 1:
					if (ch == 'N' || ch == 'S')
					{
						if (prevChar == 'B')
							bErr = TRUE;
						else
							prevChar = ch;
					}
					else if ((ch == '1' || ch == '2' || ch == '4') && prevChar != 'B')
						bErr = TRUE;
					break;
				case 2:
					if (ch != 'S')
						bErr = TRUE;
					break;

				default:
					bErr = TRUE; // too many chars
			}
		}
		bRet = (bErr == FALSE);
	}

	if (bRet)
		m_sCharType = sCharType;

	return bRet;
}

BOOL CRuleObj::SetMNU(CStdString& sMNU)
{
	BOOL bRet = FALSE;

	if (sMNU != _T(""))
	{
		if (isalpha(sMNU.GetAt(0)))
		{
			int nLen = sMNU.GetLength();
			BOOL bErr = FALSE;

			for (int i = 0; i < nLen && !bErr; i++)
			{
				if (!isalnum(sMNU.GetAt(i)) && !(sMNU.GetAt(i) == '_'))
					bErr = TRUE;
			}

			bRet = (bErr == FALSE);
		}
	}

	if (bRet)
		m_sMNU = sMNU;

	return bRet;
}

BOOL CRuleObj::SetLocation(CStdString& sLocation)
{
	CStdString sTemp(sLocation);
	CStdString sErr;
	BOOL bRet = TRUE;

	sTemp.TrimLeft();
	sTemp.TrimRight();

	// sTemp must be one of:
	// 1. digit . digit
	// 2. digit . digit . digit
	// 3. digit . digit . digit . digit
	// 4. digit . digit . digit . digit . digit
	// 5. digit . digit :
	// 6. digit . digit .. digit
	// 7. digit . digit ... digit

	std::vector<UINT>	numAry;
	CStdString			sRegEx;
	struct locstruct
	{
		int nForm;
		char szRegex[80];
	} formAry[] = {
		{ LOC_FORM_1, "^\\d+\\.\\d+$" },						// digit . digit
		{ LOC_FORM_2, "^\\d+\\.\\d+\\.\\d+$" },					// digit . digit . digit
		{ LOC_FORM_3, "^\\d+\\.\\d+\\.\\d+\\.\\d+$" },			// digit . digit . digit . digit
		{ LOC_FORM_4, "^\\d+\\.\\d+\\:$" },						// digit . digit :
		{ LOC_FORM_5, "^\\d+\\.\\d+\\.{2}\\d+$" },				// digit . digit .. digit
		{ LOC_FORM_6, "^\\d+\\.\\d+\\.{3}\\d+$" },				// digit . digit ... digit
		{ LOC_FORM_7, "^\\d+\\.\\d+\\.\\d+\\.\\d+\\.\\d+$" }	// digit . digit . digit . digit . digit
	};
	int nSize = sizeof(formAry)/sizeof(formAry[0]);

	// the form types are categorized from Aware rule documentation
	m_nLocFormType = LOC_FORM_UNKOWN;
	for (int i = 0; i < nSize; i++)
	{
		sRegEx = formAry[i].szRegex;
		if (TestRegEx(sTemp,sRegEx))
		{
			m_nLocFormType = formAry[i].nForm;
			
			if (m_nLocFormType == LOC_FORM_4)
				sTemp.Replace(':',' ');
			else if (m_nLocFormType == LOC_FORM_5)
				sTemp.Replace("..",".");
			else if (m_nLocFormType == LOC_FORM_6)
				sTemp.Replace("...",".");

			break;
		}
	}

	if (m_nLocFormType != LOC_FORM_UNKOWN && ExtractValues(sTemp ,&numAry))
	{
		int nCount = numAry.size();

		m_nRecordType = nCount > 0 ? numAry.at(0) : 1;
		m_nField = nCount > 1 ? numAry.at(1) : 1;

		m_nItem = nCount > 3 ? numAry.at(3) : 1;

		if (m_nLocFormType == LOC_FORM_5)
		{
			m_nItem = nCount > 2 ? numAry.at(2) : 1;
		}
		else if (m_nLocFormType == LOC_FORM_6)
		{
			m_nOffset = nCount > 2 ? numAry.at(2) : 1;
		}
		else if (m_nLocFormType == LOC_FORM_4)
		{
			// This is a heading for a section, so it doesn't represent a subfield or an item.
			// We use 0 to denote this fact.
			m_nSubField = 0;
			m_nItem = 0;
		}
		else
		{
			m_nSubField = nCount > 2 ? numAry.at(2) : 1;
		}

		bRet = TRUE;
	}

	sErr.Format("Loc: %s (form %d), rec: %ld, field: %ld, subfield: %ld, item: %ld, offset: %ld\n",sLocation,m_nLocFormType,m_nRecordType,m_nField,m_nSubField,m_nItem,m_nOffset);
	OutputDebugString(sErr);

	if (bRet)
		m_sLocation = sLocation;

	return bRet;
}

BOOL CRuleObj::SetTransactions(CStdString& sTransactionList)
{
	CStdString sTemp;
	CStdString sErr;
	BOOL bRet = TRUE;

	sTemp = sTransactionList;
	sTemp.Replace('\n',' ');
	sTemp.Replace('\r',' ');
	sTemp.Replace('[',' ');
	sTemp.Replace(']',' ');
	sTemp.TrimLeft();
	sTemp.TrimRight();
	sTemp.MakeUpper();
	
	UINT nMandatory = TRANS_MANDATORY;

	if (sTemp != _T(""))
	{
		sTemp += ',';
		
		/* parse code */
		CStdString sCopy(sTemp);
		CStdString sToken;
		CStdString sMNU;
		CStdString sMandatory;
		UINT nMandatoryOverride = 1;
		int nPos1;
		BOOL bCheckFirst = TRUE;
		BOOL bSkipField = FALSE;
		int nPos = sCopy.Find(',');
		
		while (nPos != -1)
		{
			sToken = sCopy.Left(nPos);
			
			sToken.TrimLeft();
			sToken.TrimRight();
			
			if (sToken != _T(""))
			{
				bSkipField = FALSE;
				// the first entry can be a 'O' or an 'M' to set the requirement for all following
				if (bCheckFirst)
				{
					bCheckFirst = FALSE;
					if (sToken.GetLength() == 1)
					{
						if (!sToken.Compare("O"))
							m_transactionDefault = nMandatory = TRANS_OPTIONAL;
						else if (!sToken.Compare("M"))
							m_transactionDefault = nMandatory = TRANS_MANDATORY;
						bSkipField = TRUE;
					}
				}

				if (!bSkipField)
				{
					nMandatoryOverride = nMandatory;
					if ((nPos1 = sToken.Find(' ')) != -1)
					{
						sMNU = sToken.Left(nPos1);
						sMandatory = sToken.Mid(nPos1+1);
						sMNU.TrimLeft();
						sMNU.TrimRight();
						sMandatory.TrimLeft();
						sMandatory.TrimRight();
						nMandatoryOverride = sMandatory.Compare("O") ? TRANS_MANDATORY : TRANS_OPTIONAL;
					}
					else
					{
						sMNU = sToken;
						sMNU.TrimLeft();
						sMNU.TrimRight();
					}
			
					if (isalpha(sMNU.GetAt(0)))	// MNU must begin with a char
						m_transactionList[sMNU]= nMandatoryOverride;
					else
					{
						sErr.Format("[CRuleObj::SetTransactions] Invalid MNU: %s", sMNU);
						LogFile(NULL,sErr);
						bRet = FALSE;
					}
				}
			}

			if (nPos+1 == sCopy.GetLength())
				break;

			sCopy = sCopy.Mid(nPos+1);
			nPos = sCopy.Find(',');
		}
	}		

/*
	int nCount = m_transactionList.GetCount();
	POSITION pos = m_transactionList.GetStartPosition();
	UINT nKey;
	CStdString sMNU;

	OutputDebugString("Trans count %ld\n",nCount);
	while (pos != NULL)
	{
		m_transactionList.GetNextAssoc( pos, sMNU, nKey );

		OutputDebugString("%s = %ld\n",sMNU,nKey);
	}
*/

	return bRet;
}

BOOL CRuleObj::TestRegEx(CStdString& sInput, CStdString& sRegEx)
{
	boost::regex e;
	std::string input;
	CStdString sErr;
	BOOL bRet = FALSE;

	input = sInput;
	
	try
	{
		e = sRegEx; // m_sRegEx;
	}
	catch (...)
	{
		sErr.Format("[CRuleObj::TestRegEx] Exception thrown. %s. Maybe an invalid mask defined.",sRegEx);
		LogFile(NULL,sErr);
	}

	try
	{
		// return false for partial match, true for full match, or throw for
		// impossible match based on what we have so far...
		boost::match_results<std::string::const_iterator> what;
		if(0 == boost::regex_match(input, what, e, boost::match_default | boost::match_partial))
		{
			bRet = FALSE;
			// the input so far could not possibly be valid so reject it:
	//		throw std::runtime_error("Invalid data entered - this could not possibly be a valid card number");
		}
		// OK so far so good, but have we finished?
		if(what[0].matched)
		{
			// excellent, we have a result:
			// return true;
			bRet = TRUE; // so far, so good
		}
	}
	catch (...)
	{
		sErr.Format("[CRuleObj::TestRegEx] Exception thrown 2. %s. Maybe an invalid mask defined.",sRegEx);
		LogFile(NULL,sErr);
	}

	return bRet;
}

BOOL CRuleObj::ExtractValues(CStdString& sLocation, std::vector<UINT> *pNumAry)
{
	BOOL bRet = TRUE;
	CStdString sTemp(sLocation);

	if (sTemp != _T(""))
	{
		sTemp += ".";

		CStdString sCopy(sTemp);
		CStdString sToken;
		int nPos = sCopy.Find('.');
		int nVal;

		while (nPos != -1)
		{
			sToken = sCopy.Left(nPos);
			
			nVal = atol(sToken);			
			pNumAry->push_back(nVal);

			if (nPos+1 == sCopy.GetLength())
				break;

			sCopy = sCopy.Mid(nPos+1);
			nPos = sCopy.Find('.');
		}
	}		

	return bRet;
}

BOOL CRuleObj::IsValid()
//TODO?
{
	BOOL bRet = FALSE;

	return bRet;
}

BOOL CRuleObj::IsMandatory(CStdString& sTOT)
// Given the Type Of Transaction, return is the field the rule refers to is mandatory for this type
// of record. Currently we ignore Location Form Type 5 rules, such as "2.051..2" for example, because
// these may be tagged as mandatory, but they are only mandatory if the associated field actually
// exists, and this field may be optional. This is known by thepresense of the associated Location
// Form Type 4 rule, for example "2.051:". In the future this will need to be taken into account,
// perhaps by having a smarter rule structure than just a flat array, i.e., a list of rules each
// containing related subrules.
{
	BOOL bRet = false;
	UINT nMandatory;
	CStdString sTemp;
	CStdString sMNU;
	map<CStdString, UINT>::iterator it;

	if (m_nLocFormType == LOC_FORM_4) return false;
	if (m_nLocFormType == LOC_FORM_5) return false;

	if (m_transactionList.empty()) // No specifics, mandatory or optional for all TOTs
	{
		bRet = (m_transactionDefault == TRANS_MANDATORY);
	}
	else
	{
		for (it = m_transactionList.begin(); it != m_transactionList.end(); it++)
		{
			sMNU = (*it).first;
			nMandatory = (*it).second;

			if (!sTOT.CompareNoCase(sMNU))
			{
				bRet = (nMandatory == TRANS_MANDATORY);
			}
		}
	}

	return bRet;
}

BOOL CRuleObj::IsOptional(CStdString& sTOT)
{
	BOOL bRet = false;
	UINT nMandatory;
	CStdString sTemp;
	CStdString sMNU;
	map<CStdString, UINT>::iterator it;

	// We return true here for the same reason we returned false in IsMandatory
	if (m_nLocFormType == LOC_FORM_4) return true;
	if (m_nLocFormType == LOC_FORM_5) return true;

	// Kludge: In ebts1_2.txt, 10.999 is not marked for DPRS as it should be, but this
	// is not coherent as any binary record  is useless without it's DAT field. We simply
	// let all binary fields be optional for all records, at least as a temporary fix.
	if (CNISTRecord::IsDATField(m_nRecordType, m_nField)) return true;

	if (m_transactionList.empty()) // No specifics, mandatory or optional for all TOTs
	{
		bRet = (m_transactionDefault == TRANS_OPTIONAL);
	}
	else
	{
		for (it = m_transactionList.begin(); it != m_transactionList.end(); it++)
		{
			sMNU = (*it).first;
			nMandatory = (*it).second;

			if (!sTOT.CompareNoCase(sMNU))
			{
				bRet = (nMandatory == TRANS_OPTIONAL);
			}
		}
	}

	return bRet;
}

BOOL CRuleObj::AppliesTo(int iRecType, int iRecord, int iField, int iSubfield, int iItem)
// TODO: study and support all Location Form Types (m_nLocFormType)
{
	BOOL	bRet = FALSE;
	int		iRecTypeRule;
	int		iRecordRule;
	int		iFieldRule;
	int		iSubfieldRule;
	int		iItemRule;

	if (GetLocation(iSubfield, iRecord, &iRecTypeRule, &iRecordRule, &iFieldRule, &iSubfieldRule, &iItemRule))
	{
		if (iRecTypeRule == iRecType && iRecordRule == iRecord && iFieldRule == iField &&
			iSubfieldRule == iSubfield && iItemRule == iItem)
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL CRuleObj::GetLocation(int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item)
{
	BOOL bRet = FALSE;

	switch (m_nLocFormType)
	{
		case LOC_FORM_1:
			{
				// <RECORD TYPE>.<FIELD NUMBER>
				// This means that the rule applies to item 1 of all the subfields in the
				// field. Each subfield can only have one item. There can only be as
				// many subfields as specified by the occurrences value (see
				// occurrence value near the end of this section).
				// When a rule uses this form, the index parameter of the
				// Specification based data access functions (Index and
				// StartIndex) will refer to subfield number.

				// eg 2.007 T2_SCO AN 9-19 1-9;
				// This rule applies to field number 7 and item number 1 in all the
				// subfields in records of type 2. There can be between one and nine
				// subfields. The field must have between nine and 19 alphanumeric characters.
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = inputIndex;
				*item = 1; 
				bRet = TRUE;
			}
			break;

		case LOC_FORM_2:
			{
				// <RECORD TYPE>.<FIELD NUMBER>.<SUBFIELD>
				// This means that the rule applies to all the items in the specified
				// subfield. There can only be as many items as specified by the
				// occurrences value.
				// When a rule uses this form, the index parameter of the
				// Specification based data access functions (Index and
				// StartIndex) will refer to item number.

				// eg 2.011.1 T2_EXM N 1-4 3;
				// This rule applies to records of Type 2. In the 11th field, subfield one
				// must have three items composed of between 1 and 4 numeric characters.
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = m_nSubField;
				*item = inputIndex;
				bRet = TRUE;
			}
			break;

		case LOC_FORM_3:
			{
				// <RECORD TYPE>.<FIELD NUMBER>.<SUBFIELD NUMBER>.<ITEM NUMBER>
				// This means that rule applies to the specified item in the specified
				// subfield. The ‘occurrences’ value is meaningless in this context.
				// When a rule uses this form, the index parameter of the
				// Specification based data access functions (Index and
				// StartIndex) is ignored.

				// eg 1.03.2.4 T2_CNT N 2 1;
				// This rule applies to records of Type 1. In the 3rd field, subfield
				// two, item 4 must have data composed of 2 numeric characters. The
				// occurrence value is ignored when using this form of the location index.
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = m_nSubField;
				*item = m_nItem;
				bRet = TRUE;
			}
			break;

		case LOC_FORM_4:
			{
				// <RECORD TYPE>.<FIELD NUMBER>:
				// This means that the rule applies to everything in the field. There
				// can only be as many subfields as specified by the occurrences
				// value. For each subfield, the sum of data lengths is checked
				// against the field size value.
				// When a rule uses this form, the index parameter of the
				// Specification based data access functions (Index and
				// StartIndex) will refer to subfield number.
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = m_nSubField;
				*item = m_nItem;
				bRet = TRUE;
			}
			break;

		case LOC_FORM_5:
			{
				// <RECORD TYPE>.<FIELD NUMBER >..<ITEM NUMBER>
				// This means that the rule applies to all items numbered by {ITEM
				// NUMBER} in all the subfields. The occurrence value is
				// meaningless in this context. This type of location index is usually
				// used in conjunction with the location index of type “{RECORD TYPE}.{FIELD NUMBER}:”.
				// When a rule uses this form, the index parameter of the
				// Specification based data access functions (Index and
				// StartIndex) will refer to subfield number.

				// eg 4.03…5 T4_IMP B1 1 1;
				// This rule applies to records of Type 4. The third field is located at
				// an offset of 4 bytes from the start of the record. It will store a 1
				// byte (indicated by B1 data type) binary value there. One and only
				// one occurrence is allowed
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = inputIndex;
				*item = m_nItem;
				bRet = TRUE;
			}
			break;

		case LOC_FORM_6:
			{
				// <RECORD TYPE>.<FIELD NUMBER >...<OFFSET>
				// For binary records (Types 4, 7 and 8), the <OFFSET> value specifies
				// where the item begins relative to the start of the record. The size of
				// the binary item is determined by the data type, 1, 2 and 4 byte
				// unsigned integers are supported.
				// Text records (Types 1, 2 and 10) can also use this form for fields
				// with a fixed layout. For text records, the <OFFSET> value specifies
				// where the item begins with respect to the start of the field.
				// The number of subfields is determined by the occurrence value.
				// The space reserved for the field is based on the maximum value in
				// the occurrence range.
				// When a rule uses this form, the index parameter of the
				// Specification based data access functions (Index and
				// StartIndex) will refer to subfield number.
 
				// eg 4.03…5 T4_IMP B1 1 1;
				// This rule applies to records of Type 4. The third field is located at
				// an offset of 4 bytes from the start of the record. It will store a 1
				// byte (indicated by B1 data type) binary value there. One and only
				// one occurrence is allowed.
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = inputIndex;
				*item = m_nItem;
				bRet = TRUE;
			}
			break;

		case LOC_FORM_7:
			{
				// <RECORD TYPE>.<FIELD NUM>.<SUBFIELD>.<ITEM>.<OFFSET>
				// This format is similar to Form 6, except that it allows a specific
				// subfield and item number to be associated with the rule. It is
				// useful for fields (or subfields) which contain different types of
				// items.
 
				// eg 7.06: T7_IMR B1 11 1;
				// 7.06.1.1.17 T7_IMR_RES B2 1-5 1;
				// 7.06.1.2.19 T7_IMR_BPP B1 1 1;
				// 7.06.1.3.20 T7_IMR_WHITE B4 1-5 1;
				// 7.06.1.4.24 T7_IMR_BLACK B4 1-5 1;
				// This rule applies to records of Type 7. The sixth field is 11 bytes
				// long as indicated by the length value in the first line. This 11 byte
				// field is divided up into four items as specified by the following
				// lines. The first item is 2 bytes long (data type B2) and occurs at an
				// offset of 17 bytes from the start of the Type 7 record. When
				// entering the value for the first item as a text string, such as would
				// be done in the IWSet function, up to 5 digits are allowed, however
				// the number will be converted to binary by the function and stored
				// into 2 bytes.
				*recordType = m_nRecordType;
				*recordIndex = inputRecordIndex;
				*field = m_nField;
				*subField = m_nSubField;
				*item = m_nItem;
				bRet = TRUE;
			}
			break;
	}

	return bRet;
}

