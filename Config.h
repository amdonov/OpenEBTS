/*
 * Config.h
 *
 * Special definitions to allow compilation under Win32 and UNIX
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_


#ifdef WIN32

#define _WIN32_IE 0x0400

#include <tchar.h>

// On Win32 filenames can be referred to by char* or wide chars
#define TCHARPATH			TCHAR
#define CStdStringPath		CStdString
#define _tfopenpath(a, b)	_tfopen(a, b)
#define _tremovepath(a)		_tremove(a)
#define _trenamepath(a, b)	_trename(a, b)
#define _tcscpypath			_tcscpy
#define _tcscatpath			_tcscat
#define _sntprintfpath		_sntprintf
#define _tgetenvpath		_tgetenv
#define _TPATH(x)			_T(x)

#else

// On *nix filenames can be referred to only by char* (the filesystem is UTF-8),
// so here we set these *path definitions to point to the char* counterparts
// regardless of the UNICODE flag
#define TCHARPATH			char
#define CStdStringPath		CStdStringA
#define _tfopenpath(a, b)	fopen(a, b)
#define _tremovepath(a)		remove(a)
#define _trenamepath(a, b)	rename(a, b)
#define _tcscpypath			strcpy
#define _tcscatpath			strcat
#define _sntprintfpath		snprintf
#define _tgetenvpath		getenv
#define _TPATH(x)			(x)

#define WCHAR		wchar_t

// These few definitions make up for the lack of tchar.h on *nix
#ifdef UNICODE

#define _T(x)		L##x
#define _tcstol		wcstol
#define _tcstod		wcstod
#define _vstprintf	vswprintf
#define _tprintf	wprintf
#define _tcscpy		wcscpy
#define _tcscat		wcscat
#define _tcslen		wcslen
#define _tcsncpy	wcsncpy
#define	_tcsncat	wcsncat
#define _sntprintf	swprintf
#define _istalnum	iswalnum
#define _istalpha	iswalpha
#define _istdigit	iswdigit
#define _istprint	iswprint
#define _istprint	iswprint
#define _istspace	iswspace
#define _tcsrchr	wcsrchr

#else

#define _T(x)		(x)
#define _tcstol		strtol
#define _tcstod		strtod
#define _vstprintf	vsnprintf
#define _tprintf	printf
#define _tcscpy		strcpy
#define _tcscat		strcat
#define _tcslen		strlen
#define _tcsncpy	strncpy
#define	_tcsncat	strncat
#define _sntprintf	snprintf
#define _istalnum	isalnum
#define _istalpha	isalpha
#define _istdigit	isdigit
#define _istprint	isprint
#define _istprint	isprint
#define _istspace	isspace
#define _tcsrchr	strrchr

#endif

#define _MAX_PATH 	PATH_MAX
#define _MAX_DIR	NAME_MAX
#define _MAX_DRIVE

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))

#include <inttypes.h>
typedef unsigned char	BYTE;
typedef uint16_t		WORD;
typedef uint32_t		UINT;
typedef int32_t			LONG;
typedef uint32_t		DWORD;
typedef int				BOOL;
typedef DWORD			DWORD_PTR;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

// Make sure this struct gets packed on a 2-byte bounday, since it's 14 bytes long
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

#endif

#endif /* CONFIG_H_ */
