#include "Includes.h"
#include "Common.h"
#include "UTF8.h"

// Allow for 10MB logfiles
#define MAX_LOGFILE_SIZE (long)10240000

// Default logfile is empty, which means "don't log anything".
// If the environment variable OPENEBTSLOGPATH is set, logging
// will be turned on, and set to "normal" where only basic logging
// is performed. Setting the environment variable OPENEBTSLOGVERBOSE=1
// will log all the log messages.
// In general normal logging will trace the entry of important functions
// and error conditions, while verbose logging will also output detailed
// information on the contents of files being processed.

// Internally Log Level 0 == logging off, 1 == normal logging, 2 == verbose.
int g_nLogLevel = 0;
TCHARPATH g_LogFilePath[_MAX_PATH+1] = { '\0' };
CStdString BuildDateTimeString();


void LogMessageInit()
// Must be called before using LogMessage or LogMessageVerbose.
// Note that we use TCHARPATH/_tgetenvpath for the non-path string szVerbose
// because the circumstance is just like when dealing with a path: on *NIX
// getenv is always a char* function whereas on Win it's wchar under UNICODE.
{
	TCHARPATH *szLogFilePath = NULL;
	TCHARPATH* szVerbose = NULL;

	szLogFilePath = _tgetenvpath(_TPATH("OPENEBTSLOGPATH"));
	if (szLogFilePath != NULL)
	{
		// The environment variable is set, copy the path of the file
		_tcscpypath(g_LogFilePath, szLogFilePath);

		// Turn logging on
		g_nLogLevel = 1;

		// See if logging is verbose
		szVerbose = _tgetenvpath(_TPATH("OPENEBTSLOGVERBOSE"));
		if (szVerbose != NULL)
		{
			// Check value: ie., OPENEBTSLOGVERBOSE=1 toggles on, butr
			// OPENEBTSLOGVERBOSE=0 would toggle off.
#ifdef WIN32
			if (_tcstol(szVerbose, NULL, 10) > 0)
#else
			if (atol(szVerbose) > 0)
#endif
			{
				g_nLogLevel = 2;
			}
		}
	}
	else
	{
		g_nLogLevel = 0;
		g_LogFilePath[0] = '\0';
	}
}

// These macros can be used to avoid additional log-setting-dependant
// operations if we know logging is disabled.
bool IsLogging()		{ return g_nLogLevel == 1; }
bool IsLoggingVerbose()	{ return g_nLogLevel > 1; }

void LogMessageVerbose(CStdString& str)
{
	LogMessage(str, true);
}

void LogMessage(CStdString& str, bool bVerbose/*=false*/)
{
	if (g_nLogLevel == 0) return;
	if (bVerbose && g_nLogLevel != 2) return;

	FILE* logFile;
	CStdString csString = str;

	CStdStringPath csPath = g_LogFilePath;

	if (csPath.IsEmpty())
		return;

	//Prepend the date and time
	CStdString csDateTime = BuildDateTimeString();
	CStdString csOutput = csDateTime + " " + csString;

	if (csOutput.GetLength())
	{
		csOutput += _T("\n");

		logFile = _tfopenpath(csPath, _TPATH("a+t"));
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

					CStdStringPath csPathBackup = csPath.Left(csPath.ReverseFind('.')) + "Old.log";
					
					//Open the old backup file in case it needs to be created
					logFile = _tfopenpath(csPathBackup, _TPATH("a+t"));
					fclose(logFile);

					//Delete the backup file
					_tremovepath(csPathBackup);

					//Rename the log file to the backup name
					_trenamepath(csPath, csPathBackup);

					//Re-open the log file
					logFile = _tfopenpath(csPath, _TPATH("a+t"));
				}

				// In UNICODE, let's write the log file as UTF-8, otherwise let's use regular ASCII
#ifdef UNICODE
				int nLen = 0;
				char *szOutTemp;
				if (UCStoUTF8(csOutput, &szOutTemp, &nLen))
				{
					// (don't include the null-terminator)
					fwrite(szOutTemp, 1, nLen-1, logFile);
					delete szOutTemp;
				}
#else
				fwrite(csOutput, 1, csOutput.GetLength(), logFile);
#endif
			}
			catch (...)
			{	
				;
			}
			fclose(logFile);
		}
	}
}

void LogMessageVerbose(const TCHAR* sz)
{
	CStdString s(sz);
	LogMessage(s);
}

void LogMessage(const TCHAR* sz, bool bVerbose/*=false*/)
{
	CStdString s(sz);
	LogMessage(s, bVerbose);
}

CStdString BuildDateTimeString()
{
	time_t		timeNow;
	struct tm	*tmNow;
	char		szTime[80];

	time(&timeNow);
	tmNow = localtime(&timeNow);

	// Convert to, for example, "2011.04.17|14:02:14"
	strftime(szTime, 80, "%Y.%m.%d|%H:%M:%S", tmNow);

	return CStdString(szTime);
}

bool UTF8toUCS(const char *pIn, wchar_t **ppOut)
// Do a UTF-8 to wide char conversion. Caller must delete[] returned pointer
// if function returns true (based on the main UTF8-CPP sample).
// Note the returned wide chars will be 4-bytes on Unix.
{
	std::string							utf8string(pIn);
	vector<unsigned short>				utf16or32string;
	int									nChars;
	int									i;

	try
	{
#ifdef WIN32
		// Convert it to utf-16
		utf8::utf8to16(utf8string.begin(), utf8string.end(), back_inserter(utf16or32string));
#else
		// Convert it to utf-32
		utf8::utf8to32(utf8string.begin(), utf8string.end(), back_inserter(utf16or32string));

#endif
	}
	catch (utf8::invalid_utf8)
	{
		return false;
	}

	nChars = (int)utf16or32string.size();

	// Allocate space for new wchar_t*, including null-terminator
	*ppOut = new wchar_t[nChars + 1];

	// Copy UTF-16 (really a UCS2) into our wchar_t array
	for (i = 0; i < nChars; i++)
	{
		(*ppOut)[i] = utf16or32string.at(i);
	}
	(*ppOut)[nChars] = '\0';

	return true;
}

bool UCStoUTF8(const wchar_t *wIn, char **ppOut, int *pnLength)
// Do a wide char to UTF-8 conversion. Caller must delete[] returned pointer
// if function returns true (based on the main UTF8-CPP sample).
// Note that in windows wchar_t will be 16-bits and under Unix wchar_t will be 32-bits,
// but in both cases this function will convert down to UTF-8.
{
	std::string							utf8string;
	vector<unsigned short>				utf16or32string;
	int									nChars;
	vector<unsigned short>::iterator	iter;
	int									i;

	// Form utf16or32string from wchat_t array
	nChars = (int)wcslen(wIn);
	for (i = 0; i < nChars; i++)
	{
		utf16or32string.push_back(wIn[i]);
	}

	try
	{
		// Convert it to utf-8
#ifdef WIN32
		utf8::utf16to8(utf16or32string.begin(), utf16or32string.end(), back_inserter(utf8string));
#else
		utf8::utf32to8(utf16or32string.begin(), utf16or32string.end(), back_inserter(utf8string));
#endif
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
	*pnLength = (int)utf8string.size() + 1;

	return true;
}

