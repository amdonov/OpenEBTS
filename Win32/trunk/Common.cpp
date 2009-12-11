

#include "stdafx.h"
#include "common.h"

#define MAX_LOGFILE_SIZE (long)500000

BOOL g_bTraceOn = FALSE;
BOOL g_bLogErrors = FALSE;

TCHAR g_LogFilePath[_MAX_PATH+1] = { '\0', };
CStdString BuildDateTimeString();
void InitializePath();
void LogMessage(CStdString& str);
int GetRegInt(char *pszKey, int nDefault);
CStdString GetRegString(char *pszKey, char *pszValue, BOOL bSetValue = FALSE);
CStdString WriteRegString(char *pszKey, char *pszValue);

void SetLogFlags()
{
	g_bTraceOn = GetRegInt("TraceMessages", 0);
	g_bLogErrors = GetRegInt("LogMessages", 1);
}

void TraceMsg(CStdString& sTraceMsg)
{
	if (g_bTraceOn)
		LogMessage(sTraceMsg);
}

void LogFile(char *pLogFile,CStdString& sException)
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

	if (csPath == "")
		return;

	//Prepend the date and time
	CStdString csDateTime = BuildDateTimeString();
	CStdString csOutput = csDateTime + " " + csString;

	if (csOutput.GetLength())
	{
		csOutput += "\r\n";

		logFile = fopen(csPath, "a+t");
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
					logFile = fopen(csPathBackup, "a+t");
					fclose(logFile);

					//Delete the backup file
					DeleteFile(csPathBackup);

					//Rename the log file to the backup name
					MoveFile(csPath, csPathBackup);

					//Re-open the log file
					logFile = fopen(csPath, "a+t");
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
		DWORD dwLen = GetTempPath(sizeof(szPath),szPath);

		// I can't live without a log file....
		if (dwLen)
		{
			char *pFName = strrchr(szPath,'\\');

			*(pFName+1) = '\0';

			strcpy_s(g_LogFilePath,_MAX_PATH+1,szPath);
			strcat_s(g_LogFilePath,_MAX_PATH+1,LF_OPENEBTS);
		}
		else
			g_LogFilePath[0] = '\0';
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

CStdString GetRegString(char *pszKey, char *pszValue, BOOL bSetValue)
{
	DWORD	dwType = REG_SZ;
	unsigned char szValue[80] = { '\0', };
	DWORD dwValueLength = sizeof(szValue);
	HKEY	hKey = 0;
	unsigned long lDisposition = 0;
	LONG lResult;	// result of registry operation
	CStdString sRet;

	try
	{
		CStdString csKey = "SOFTWARE\\ImageWare Systems\\OpenEBTS";

		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, csKey, 0, 
				NULL,REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &lDisposition);

		if (lDisposition == REG_OPENED_EXISTING_KEY || lDisposition == REG_CREATED_NEW_KEY)  //if opened get value
		{
			lResult = RegQueryValueEx(hKey, pszKey, NULL, &dwType,
					szValue,&dwValueLength); //Try to get value

			if (lResult == ERROR_SUCCESS && bSetValue)
			{
				//Make value and set it to default argument
				lResult = RegSetValueEx(hKey, pszKey, 0, REG_SZ, (unsigned char *)pszValue, strlen(pszValue));
				sRet = pszValue;
			}
			else if (lResult != ERROR_SUCCESS)  //No value by that name, create it
			{
				if (1) //pszDefault)
				{
					char szDefaultValue[2] = { '\0', };
					char *pDefValue = (pszValue ? pszValue : szDefaultValue);
					//Make value and set it to default argument or empty
					lResult = RegSetValueEx(hKey, pszKey, 0, REG_SZ, (unsigned char *)pDefValue, strlen(pDefValue)+1);

					sRet = pDefValue;
				}
			}
			else
				sRet = (char*)szValue; // just return value
		}
		else
		{
			CStdString sErr;

//			sErr.Format("Error reading registry entry for HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageWare Systems\\OpenEBTS\\%s", pszKey);
//			AfxMessageBox(sErr,MB_ICONEXCLAMATION|MB_OK);
		}
 
		if (sRet == "" && pszValue && strlen(pszValue))
			sRet = pszValue;

		if (hKey)
			RegCloseKey(hKey);
	}
	catch (...)
	{
		;
	}

	return sRet;
}

int GetRegInt(char *pszKey, int nDefault)
{
	int nRet = -1;
	char szValue[20] = { '\0', };
	char szDefault[20] = { '\0', };

	if (nDefault != -1)
		wsprintf(szValue,"%d",nDefault);

	CStdString sRet = GetRegString(pszKey,szValue);

	if (sRet != "")
		nRet = atoi((LPCSTR)sRet);
	
	return nRet;
}

CStdString WriteRegString(char *pszKey, char *pszValue)
{
	return GetRegString(pszKey, pszValue, TRUE);
}


