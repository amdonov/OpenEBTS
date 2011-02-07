#include "stdafx.h"
#include <mbstring.h>
#include "common.h"

#define MAX_LOGFILE_SIZE (long)500000

bool g_bTraceOn = false;
bool g_bLogErrors = false;

TCHAR g_LogFilePath[_MAX_PATH+1] = { '\0', };
CStdString BuildDateTimeString();
void InitializePath();
void LogMessage(CStdString& str);
int GetRegInt(TCHAR* szKey, int nDefault);
CStdString GetRegString(TCHAR* szKey, TCHAR* szValue, bool bSetValue = false);
CStdString WriteRegString(TCHAR* szKey, TCHAR* szValue);

void SetLogFlags()
{
	g_bTraceOn = (GetRegInt(_T("TraceMessages"), 0) != 0);
	g_bLogErrors = (GetRegInt(_T("LogMessages"), 1) != 0);
}

void TraceMsg(CStdString& sTraceMsg)
{
	if (g_bTraceOn)
		LogMessage(sTraceMsg);
}

void LogFile(CStdString& sException)
{
	if (g_bLogErrors)
		LogMessage(sException);
}

void LogMessage(CStdString& str)
{
	FILE* logFile;
	CStdString csString = str;

	InitializePath();

	CStdString csPath = g_LogFilePath; // g_csAppPath;

	if (csPath == _T(""))
		return;

	//Prepend the date and time
	CStdString csDateTime = BuildDateTimeString();
	CStdString csOutput = csDateTime + " " + csString;

	if (csOutput.GetLength())
	{
		csOutput += _T("\r\n");

		logFile = _tfopen(csPath, _T("a+t"));
		if (logFile != NULL)
		{
			try
			{
				fseek(logFile, 0, SEEK_END);
				int nFileSize = ftell(logFile);

				if (nFileSize > MAX_LOGFILE_SIZE)
				{
					/* LogName.txt is too big.  Delete backup file LogNameOld.txt,
						rename LogName.txt to LogNameOld.txt and re-open LogName.txt */
					fclose(logFile);

					CStdString csPathBackup = csPath.Left(csPath.ReverseFind('.')) + "Old.log";
					
					//Open the old backup file in case it needs to be created
					logFile = _tfopen(csPathBackup, _T("a+t"));
					fclose(logFile);

					//Delete the backup file
					DeleteFile(csPathBackup);

					//Rename the log file to the backup name
					MoveFile(csPath, csPathBackup);

					//Re-open the log file
					logFile = _tfopen(csPath, _T("a+t"));
				}

				fwrite(csOutput, 1, csOutput.GetLength(), logFile);
			}
			catch (...)
			{	
				;
			}
			fclose(logFile);
		}
	}
}

void InitializePath()
{
	TCHAR szPath[_MAX_PATH+1];

	if (g_LogFilePath[0] == '\0')
	{
		DWORD dwLen = GetTempPath(sizeof(szPath)/sizeof(TCHAR), szPath);

		// I can't live without a log file....
		if (dwLen)
		{
			TCHAR *pFName = _tcsrchr(szPath, _T('\\'));

			*(pFName+1) = '\0';

			_tcscpy_s(g_LogFilePath, _MAX_PATH+1, szPath);
			_tcscat_s(g_LogFilePath, _MAX_PATH+1, LF_OPENEBTS);
		}
		else
		{
			g_LogFilePath[0] = '\0';
		}
	}
}

CStdString BuildDateTimeString()
{
	struct tm when;
	time_t now;

	time(&now);
	localtime_s(&when,&now);

	char szTime[80];
	asctime_s(szTime,sizeof(szTime),&when);

	if (szTime[strlen(szTime)-1] == '\n')
		szTime[strlen(szTime)-1] = '\0';

	return CStdString(szTime);
}

CStdString GetRegString(TCHAR *szKey, TCHAR *szValue, bool bSetValue)
{
	DWORD	dwType = REG_SZ;
	TCHAR	szRegValue[80];
	DWORD	dwValueLength = sizeof(szRegValue);
	HKEY	hKey = 0;
	unsigned long lDisposition = 0;
	LONG lResult;	// result of registry operation
	CStdString sRet;

	try
	{
		CStdString csKey = _T("SOFTWARE\\ImageWare Systems\\OpenEBTS");

		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, csKey, 0, 
				NULL,REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &lDisposition);

		if (lDisposition == REG_OPENED_EXISTING_KEY || lDisposition == REG_CREATED_NEW_KEY)  //if opened get value
		{
			lResult = RegQueryValueEx(hKey, szKey, NULL, &dwType, (LPBYTE)szRegValue, &dwValueLength); //Try to get value

			if (lResult == ERROR_SUCCESS && bSetValue)
			{
				//Make value and set it to default argument
				lResult = RegSetValueEx(hKey, szKey, 0, REG_SZ, (unsigned char *)szValue, _tcslen(szValue));
				sRet = szValue;
			}
			else if (lResult != ERROR_SUCCESS)  //No value by that name, create it
			{
				if (1) //pszDefault)
				{
					TCHAR szDefaultValue[2] = { '\0', };
					TCHAR *pDefValue = (szValue ? szValue : szDefaultValue);
					//Make value and set it to default argument or empty
					lResult = RegSetValueEx(hKey, szKey, 0, REG_SZ, (unsigned char *)pDefValue, _tcslen(pDefValue)+1);

					sRet = pDefValue;
				}
			}
			else
				sRet = (char*)szRegValue; // just return value
		}
		else
		{
			CStdString sErr;

//			sErr.Format("Error reading registry entry for HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageWare Systems\\OpenEBTS\\%s", pszKey);
//			AfxMessageBox(sErr,MB_ICONEXCLAMATION|MB_OK);
		}
 
		if (sRet == _T("") && szValue && _tcslen(szValue))
			sRet = szValue;

		if (hKey)
			RegCloseKey(hKey);
	}
	catch (...)
	{
		;
	}

	return sRet;
}

int GetRegInt(TCHAR* szKey, int nDefault)
{
	int nRet = -1;
	TCHAR szValue[20] = { '\0', };
	TCHAR szDefault[20] = { '\0', };

	if (nDefault != -1)
		_stprintf(szValue, _T("%d"), nDefault);

	CStdString sRet = GetRegString(szKey, szValue);

	if (sRet != _T(""))
		nRet = _ttoi(sRet);

	return nRet;
}

CStdString WriteRegString(TCHAR* szKey, TCHAR* szValue)
{
	return GetRegString(szKey, szValue, true);
}

bool UTF8toUCS2(const char *pIn, wchar_t **ppOut)
// Do a UTF-8 to wide char conversion. Caller must delete[] returned pointer
// if function returns true (based on the main UTF8-CPP sample).
{
	std::string							utf8string(pIn);
	vector<unsigned short>				utf16string;
	int									nChars;
	int									i;

	try
	{
		// Convert it to utf-16
		utf8::utf8to16(utf8string.begin(), utf8string.end(), back_inserter(utf16string));
	}
	catch (utf8::invalid_utf8)
	{
		return false;
	}

	nChars = utf16string.size();

	// Allocate space for new wchar_t*, including null-terminator
	*ppOut = new wchar_t[nChars + 1];

	// Copy UTF-16 (really a UCS2) into our wchar_t array
	for (i = 0; i < nChars; i++)
	{
		(*ppOut)[i] = utf16string.at(i);
	}
	(*ppOut)[nChars] = '\0';

	return true;
}

bool UCS2toUTF8(const wchar_t *wIn, char **ppOut, int *pnLength)
// Do a wide char to UTF-8 conversion. Caller must delete[] returned pointer
// if function returns true (based on the main UTF8-CPP sample).
{
	std::string							utf8string;
	vector<unsigned short>				utf16string;
	int									nChars;
	vector<unsigned short>::iterator	iter;
	int									i;

	// Form utf16string from wchat_t array
	nChars = wcslen(wIn);
	for (i = 0; i < nChars; i++)
	{
		utf16string.push_back(wIn[i]);
	}

	try
	{
		// Convert it to utf-8
		utf8::utf16to8(utf16string.begin(), utf16string.end(), back_inserter(utf8string));
	}
	catch (utf8::invalid_utf8)
	{
		return false;
	}

	// Allocate space for new char*, including null-terminator
	*ppOut = new char[utf8string.size() + 1];

	// Copy UTF-8 into our char array
	for (i = 0; i < (int)utf8string.size(); i++)
	{
		(*ppOut)[i] = utf8string.at(i);
	}
	(*ppOut)[utf8string.size()] = '\0';

	// Return length of all allocated *bytes*
	*pnLength = utf8string.size() + 1;

	return true;
}
