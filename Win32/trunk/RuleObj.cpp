
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

bool CRuleObj::SetData(CStdString sFilePath, CStdString& sTransactionList, CStdString& sLocation, CStdString& sMNU, CStdString& sCharType,
					   CStdString& sFieldSize, CStdString& sOccurrences, CStdString& sDescription, CStdString& sLongDescription,
					   CStdString& sSpecialChars, CStdString& sDateFormat, CStdString& sMMap, CStdString& sOMap,
					   CStdString& sTags, CStdString& sErr)
{
	CStdString sTemp;
	bool bRet = false;

	sErr.Empty();

	// extract the TOT's this rule applies to
	if (!SetTransactions(sTransactionList))
	{
		sErr = "SetTransaction Failed";
		goto done;
	}
	if (!SetLocation(sLocation))
	{
		sErr.Format(_T("Invalid location: %s"), sLocation);
		goto done;
	}
	if (!SetMNU(sMNU))
	{
		sErr.Format(_T("Invalid MNU: %s"), sMNU);
		goto done;
	}
	if (!SetCharType(sCharType))
	{
		sErr.Format(_T("%s, invalid char type: %s"), m_sMNU, sCharType);
		goto done;
	}
	if (!SetFieldSize(sFieldSize))
	{
		sErr.Format(_T("%s, invalid field size: %s"), m_sMNU, sFieldSize);
		goto done;
	}
	if (!SetOccurrences(sOccurrences))
	{
		sErr.Format(_T("%s, invalid occurrence value: %s"), m_sMNU, sOccurrences);
		goto done;
	}
	if (!SetOptionalDescription(sDescription))
	{
		sErr.Format(_T("%s, invalid description value: %s"), m_sMNU, sDescription);
		goto done;
	}
	if (!SetOptionalLongDescription(sLongDescription))
	{
		sErr.Format(_T("%s, invalid long description value: %s"), m_sMNU, sDescription);
		goto done;
	}
	if (!SetOptionalSpecialChars(sSpecialChars))
	{
		sErr.Format(_T("%s, invalid sca value: %s"), m_sMNU, sSpecialChars);
		goto done;
	}
	if (!SetOptionalDateFormat(sDateFormat))
	{
		sErr.Format(_T("%s, invalid date format value: %s"), m_sMNU, sDateFormat);
		goto done;
	}
	if (!SetOptionalMMap(sMMap, sFilePath))
	{
		sErr.Format(_T("%s, invalid mmap value: %s"), m_sMNU, sMMap);
		goto done;
	}
	if (!SetOptionalOMap(sOMap, sFilePath))
	{
		sErr.Format(_T("%s, invalid omap value: %s"), m_sMNU, sMMap);
		goto done;
	}
	if (!SetTags(sTags))
	{
		sErr.Format(_T("%s, invalid tag value: %s"), m_sMNU, sTags.Left(60));
		goto done;
	}

	bRet = true;

#ifdef _DEBUG
	DumpObject();
#endif // _DEBUG

done:
	if (!sErr.IsEmpty())
	{
		LogFile(sErr);
	}

	return bRet;
}

void CRuleObj::DumpObject()
{
	CStdString sMsg;

	sMsg.Format(_T("[DumpObject] ==> MNU: %s, Location: %s, chartype %s, len min %ld, max %ld, occ min %ld, max %ld desc(%s) sca(%s) date(%s) map(%s)"),
				m_sMNU, m_sLocation, m_sCharType, m_nMinFieldSize, m_nMaxFieldSize, m_nMinOccurrences, m_nMaxOccurrences,
				m_sDescription, m_sSpecialChars, m_sDateFormat, GetMMap());
	LogFile(sMsg);

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

	sMsg.Format(_T("[DumpObject] %s, TOT's: %s"), m_sMNU, sTemp);
	LogFile(sMsg);
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

bool CRuleObj::SetOptionalDescription(CStdString& sDescription)
{
	m_sDescription = sDescription;

	return true;
}

bool CRuleObj::SetOptionalLongDescription(CStdString& sLongDescription)
{
	m_sLongDescription = sLongDescription;

	return true;
}

bool CRuleObj::SetOptionalSpecialChars(CStdString& sSpecialChars)
// Note: we assume that m_sSpecialChars is already set
{
	m_sSpecialChars = sSpecialChars;

	// Adds to m_sCharType if m_sSpecialChars refers to printable or control characters.
	// "PRINT" is a special code for all printable characters.
	// "PRINTCTRL" is a special code for all printable characters plus control characters.
	if (!m_sSpecialChars.CompareNoCase(_T("PRINTCTRL")))
	{
		m_sCharType += "PC";
		m_sSpecialChars.Empty();
	}
	else if (!m_sSpecialChars.CompareNoCase(_T("PRINT")))
	{
		m_sCharType += "P";
		m_sSpecialChars.Empty();
	}

	return true;
}

bool CRuleObj::SetOptionalDateFormat(CStdString& sDateFormat)
// Do a brief format verification. For now we support the "Z" prefix and the abbreviations "CCYY" (or "YYYY"),
// "MM" and "DD.
{
	TCHAR		c;
	CStdString	sAllowed = "CYMD";

	for (int i = 0; i < sDateFormat.GetLength(); i++)
	{
		c = sDateFormat.GetAt(i);

		if (sAllowed.Find(c) == -1)
		{
			// Invalid char found, unless it's the first char and it's a "Z"
			if (!(i == 0 && c == 'Z'))
			{
				return false;
			}
		}
	}

	if (!sDateFormat.IsEmpty())
	{
		// Make sure we have a year, month and day, otherwise the date can't be valid
		if (sDateFormat.Find(_T("YYYY")) == -1 && sDateFormat.Find(_T("CCYY")) == -1) return false;
		if (sDateFormat.Find(_T("MM")) == -1) return false;
		if (sDateFormat.Find(_T("DD")) == -1) return false;
	}

	m_sDateFormat = sDateFormat;

	return true;
}

bool CRuleObj::SetOptionalMMap(CStdString& sMMap, CStdString& sFilePath)
{
	return SetOptionalMap(sMMap, sFilePath, m_mmapName, m_mmapDesc);
}

bool CRuleObj::SetOptionalOMap(CStdString& sOMap, CStdString& sFilePath)
{
	return SetOptionalMap(sOMap, sFilePath, m_omapName, m_omapDesc);
}

bool CRuleObj::SetOptionalMap(CStdString& sMap, CStdString& sFilePath,
							  std::vector<CStdString>& mapValNames,
							  std::vector<CStdString>& mapValDescriptions)
// Populate map from the values inside mmap or omap tag, such as mmap="<Value>[:<Desc>]|<Value>[:<Desc]|..."
// Support external file as well, as in mmap="file://<Path to external file>"
{
	TCHAR		c;
	bool		bInsideValue;
	CStdString	sName;
	CStdString	sDesc;
	CStdString	sFilename;
	CStdString	sFilePrefix = "file://";
	int			i;
	int			j;

	if (sMap.IsEmpty()) return true;	// no map, nothing to copy

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
	if (ncolons != npipes+1) return false;
	*/

	// First check if the info is in a file
	if (sMap.Find(sFilePrefix) == 0) // (starts at position 0)
	{
		TCHAR		szDrive[_MAX_DRIVE];
		TCHAR		szDir[_MAX_DIR];
		TCHAR		szPath[_MAX_DIR];
		FILE		*f;
		BYTE		*pFile;
		long		lSize;
		TCHAR		*szFile;

		// Open file in same folder as main verification file and read-in contents

		sFilename = sMap.Right(sMap.GetLength() - sFilePrefix.GetLength());

		_tsplitpath(sFilePath, szDrive, szDir, NULL, NULL);
		_tcscpy(szPath, szDrive);
		_tcscat(szPath, szDir);
		_tcscat(szPath, sFilename);

		f = _tfopen(szPath, _T("rb"));
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			lSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			pFile = new BYTE[lSize + 2];
			memset(pFile, '\0', lSize + 2);
			fread(pFile, 1, lSize, f);
			pFile[lSize] =  0x0A; // end file with carriage return so parser can include last element easily
			pFile[lSize+1] = '\0';
			fclose(f);

#ifdef UNICODE
			// In the UNICODE version of OpenEBTS, the Verification Files are in UTF-8
			// (as are any external map files) hence we convert the buffer in-place to
			// wide chars so we can proceed with simple generic _T code.

			if (!UTF8toUCS2((const char*)pFile, &szFile))
			{
				return false;	// Error deconding UTF-8
			}

			delete pFile;
#else
			szFile = (char*)pFile;
#endif

			bInsideValue = true; // the first char of the string is the first char of the first value

			for (i = 0; i < (int)_tcslen(szFile); i++)
			{
				c = szFile[i];
				if (c == 0x09)
				{
					bInsideValue = false;	// end of value, about to enter description

					// Save current value name in array and reset it
					sName.Trim();
					if (!sName.IsEmpty()) mapValNames.push_back(sName);
					sName.Empty();
				}
				else if (c == 0x0A)
				{
					bInsideValue = true;	// end of description, about to enter value

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

			delete szFile;
		}
		else
		{
			return false;
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
				bInsideValue = false;	// end of value, about to enter description

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

				bInsideValue = true;	// end of description, about to enter value

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

	return true;
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

bool CRuleObj::SetTags(CStdString& sTags)
{
#pragma message(" ===> CRuleObj, Tag support not implemented")
	return true;
}

bool CRuleObj::SetFieldSize(CStdString& sFieldSize)
{
	bool bRet = false;

	int nMin = RANGE_INVALID, nMax = RANGE_INVALID;

	if (SetRange(sFieldSize,&nMin,&nMax))
	{
		m_nMinFieldSize = nMin;
		m_nMaxFieldSize = nMax;
		bRet = true;
	}

	return bRet;
}

bool CRuleObj::SetOccurrences(CStdString& sOccurence)
{
	bool bRet = false;

	int nMin = RANGE_INVALID, nMax = RANGE_INVALID;

	if (SetRange(sOccurence,&nMin,&nMax))
	{
		m_nMinOccurrences = nMin;
		m_nMaxOccurrences = nMax;
		bRet = true;
	}

	return bRet;
}

bool CRuleObj::SetRange(CStdString& sRange, int *pMin, int *pMax)
{
	bool bRet = false;
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
					bRet = true;
				}
			}
		}
		else
		{
			if (GetRangeValue(sTemp,&nMin))
			{
				*pMin = *pMax = nMin;
				bRet = true;
			}
		}
	}

	//("%s range min %ld, max %ld\n", m_sMNU, *pMin, *pMax);

	return bRet;
}

bool CRuleObj::GetRangeValue(CStdString& sToken, int *pValue)
{
	bool		bRet = false;
	CStdString	sTemp(sToken);
	bool		bErr = false;
	TCHAR		ch;
	bool		bNotSpecified = false;

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
					bNotSpecified = true;
				else if (!isdigit(ch))
					bErr = true;
				break;
			default:
				if (bNotSpecified)
					bErr = true; // shouldn't be here
				else if (!isdigit(ch))
					bErr = true; // or here
				break;
		}
	}
	
	if (!bErr)
	{
		if (bNotSpecified)
			*pValue = RANGE_NOTSPECIFIED;
		else
			*pValue = _ttol(sTemp);
		bRet = true;
	}

	return bRet;
}

bool CRuleObj::SetCharType(CStdString& sCharType)
{
	bool bRet = false;
	CStdString sErr;

	if (sCharType != _T(""))
	{
		int nPos = 0;
		int nLen = sCharType.GetLength();
		bool bErr = false;
		CStdString sTemp(sCharType);
		TCHAR ch, prevChar;

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
						bErr = true;
					break;
				case 1:
					if (ch == 'N' || ch == 'S')
					{
						if (prevChar == 'B')
							bErr = true;
						else
							prevChar = ch;
					}
					else if ((ch == '1' || ch == '2' || ch == '4') && prevChar != 'B')
						bErr = true;
					break;
				case 2:
					if (ch != 'S')
						bErr = true;
					break;

				default:
					bErr = true; // too many chars
			}
		}
		bRet = (bErr == false);
	}

	if (bRet)
		m_sCharType = sCharType;

	return bRet;
}

/* No longer used because this approch makes little sense with UNICODE charsets

void CRuleObj::SetAllowedChars()
// Sets m_sAllowedChars based on m_sCharType and m_sSpecialChars.
// It's just a list of all alowed characters.
{
	char c;
	bool bA;		// Alpha
	bool bN;		// Numeric
	bool bS;		// Special
	bool bB;		// Binary
	bool bP;		// Printable
	bool bC;		// Control Chars
	bool bCharOK;

	m_sAllowedChars.Empty();

	// Simplify m_sCharType into 4 bools
	bA = (m_sCharType.Find('A') != -1);
	bN = (m_sCharType.Find('N') != -1);
	bS = (m_sCharType.Find('S') != -1);
	bB = (m_sCharType.Find('B') != -1);

	// Simplify m_sSpecialChars into 2 bools
	bC = !m_sSpecialChars.CompareNoCase(_T("PRINTCTRL"));
	bP = bC || (!m_sSpecialChars.CompareNoCase(_T("PRINT")));

	// Check all ASCII chars for conformance
	for (int i = 1; i < 256; i++)
	{
		c = (BYTE)i;
		bCharOK = false;	// default to "don't add"

		// To minimize function calls and comparisons we compare
		// with the largest sets before the smaller ones.
		if (bB)
		{
			bCharOK = true;
		}
		else if (bP && _istprint(c))
		{
			bCharOK = true;
		}
		else if (bA && _istalpha(c))
		{
			bCharOK = true;
		}
		else if (bN && _istdigit(c))
		{
			bCharOK = true;
		}
		else if (bC && (c == 0x09 || c == 0x0A || c == 0x0D))	// TAB, LF, CR
		{
			bCharOK = true;
		}
		else if (bS && !bP)
		{
			// m_sSpecialChars is neither "PRINT" nor "PRINTCTRL" so m_sSpecialChars
			// contains the extra allowed characters.
			if (m_sSpecialChars.Find(c) != -1)
			{
				bCharOK = true;
			}
		}

		if (bCharOK)
		{
			m_sAllowedChars += c;
		}
	}
}
*/

bool CRuleObj::SetMNU(CStdString& sMNU)
{
	bool bRet = false;

	if (sMNU != _T(""))
	{
		if (isalpha(sMNU.GetAt(0)))
		{
			int nLen = sMNU.GetLength();
			bool bErr = false;

			for (int i = 0; i < nLen && !bErr; i++)
			{
				if (!isalnum(sMNU.GetAt(i)) && !(sMNU.GetAt(i) == '_'))
					bErr = true;
			}

			bRet = (bErr == false);
		}
	}

	if (bRet)
		m_sMNU = sMNU;

	return bRet;
}

bool CRuleObj::SetLocation(CStdString& sLocation)
{
	CStdString sTemp(sLocation);
	CStdString sErr;
	bool bRet = true;

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
				sTemp.Replace(_T(':'), _T(' '));
			else if (m_nLocFormType == LOC_FORM_5)
				sTemp.Replace(_T(".."), _T("."));
			else if (m_nLocFormType == LOC_FORM_6)
				sTemp.Replace(_T("..."), _T("."));

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

		bRet = true;
	}

	sErr.Format(_T("Loc: %s (form %d), rec: %ld, field: %ld, subfield: %ld, item: %ld, offset: %ld\n"),
				sLocation, m_nLocFormType, m_nRecordType, m_nField, m_nSubField, m_nItem, m_nOffset);
	OutputDebugString(sErr);

	if (bRet)
		m_sLocation = sLocation;

	return bRet;
}

bool CRuleObj::SetTransactions(CStdString& sTransactionList)
{
	CStdString sTemp;
	CStdString sErr;
	bool bRet = true;

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
		bool bCheckFirst = true;
		bool bSkipField = false;
		int nPos = sCopy.Find(',');
		
		while (nPos != -1)
		{
			sToken = sCopy.Left(nPos);
			
			sToken.TrimLeft();
			sToken.TrimRight();
			
			if (sToken != _T(""))
			{
				bSkipField = false;
				// the first entry can be a 'O' or an 'M' to set the requirement for all following
				if (bCheckFirst)
				{
					bCheckFirst = false;
					if (sToken.GetLength() == 1)
					{
						if (!sToken.Compare(_T("O")))
							m_transactionDefault = nMandatory = TRANS_OPTIONAL;
						else if (!sToken.Compare(_T("M")))
							m_transactionDefault = nMandatory = TRANS_MANDATORY;
						bSkipField = true;
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
						nMandatoryOverride = sMandatory.Compare(_T("O")) ? TRANS_MANDATORY : TRANS_OPTIONAL;
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
						sErr.Format(_T("[CRuleObj::SetTransactions] Invalid MNU: %s"), sMNU);
						LogFile(sErr);
						bRet = false;
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

bool CRuleObj::TestRegEx(CStdString& sInput, CStdString& sRegEx)
{
#ifdef UNICODE
	typedef	std::wstring stdstring;
	boost::wregex	e;
#else
	typedef	std::string stdstring;
	boost::regex	e;
#endif
	CStdString		sErr;
	stdstring		input;
	bool			bRet = false;

	input = sInput.c_str();

	try
	{
		e = sRegEx.c_str();
	}
	catch (...)
	{
		sErr.Format(_T("[CRuleObj::TestRegEx] Exception thrown. %s. Maybe an invalid mask defined."), sRegEx);
		LogFile(sErr);
	}

	try
	{
		// return false for partial match, true for full match, or throw for
		// impossible match based on what we have so far...
		boost::match_results<stdstring::const_iterator> what;
		if(0 == boost::regex_match(input, what, e, boost::match_default | boost::match_partial))
		{
			bRet = false;
			// the input so far could not possibly be valid so reject it:
	//		throw std::runtime_error("Invalid data entered - this could not possibly be a valid card number");
		}
		// OK so far so good, but have we finished?
		if(what[0].matched)
		{
			// excellent, we have a result:
			// return true;
			bRet = true; // so far, so good
		}
	}
	catch (...)
	{
		sErr.Format(_T("[CRuleObj::TestRegEx] Exception thrown 2. %s. Maybe an invalid mask defined."), sRegEx);
		LogFile(sErr);
	}

	return bRet;
}

bool CRuleObj::ExtractValues(CStdString& sLocation, std::vector<UINT> *pNumAry)
{
	bool bRet = true;
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

			nVal = _ttol(sToken);
			pNumAry->push_back(nVal);

			if (nPos+1 == sCopy.GetLength())
				break;

			sCopy = sCopy.Mid(nPos+1);
			nPos = sCopy.Find('.');
		}
	}

	return bRet;
}

bool CRuleObj::IsMandatory(CStdString& sTOT)
// Given the Type Of Transaction, return is the field the rule refers to is mandatory for this type
// of record. Currently we ignore Location Form Type 5 rules, such as "2.051..2" for example, because
// these may be tagged as mandatory, but they are only mandatory if the associated field actually
// exists, and this field may be optional. This is known by thepresense of the associated Location
// Form Type 4 rule, for example "2.051:". In the future this will need to be taken into account,
// perhaps by having a smarter rule structure than just a flat array, i.e., a list of rules each
// containing related subrules.
{
	bool bRet = false;
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

bool CRuleObj::IsOptional(CStdString& sTOT)
{
	bool bRet = false;
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

bool CRuleObj::AppliesTo(int iRecType, int iRecord, int iField, int iSubfield, int iItem)
// TODO: study and support all Location Form Types (m_nLocFormType)
{
	bool	bRet = false;
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
			bRet = true;
		}
	}

	return bRet;
}

bool CRuleObj::GetLocation(int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item)
{
	bool bRet = false;

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
				bRet = true;
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
				bRet = true;
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
				bRet = true;
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
				bRet = true;
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
				bRet = true;
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
				bRet = true;
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
				bRet = true;
			}
			break;
	}

	return bRet;
}

