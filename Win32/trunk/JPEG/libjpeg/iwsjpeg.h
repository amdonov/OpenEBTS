

#ifndef _JPG_INTERFACE_H
#define _JPG_INTERFACE_H

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif // __cplusplus

#include <windows.h>

/* 
 * set mode and type of logging using SetLogMode() 
 * JPG_LOGMODE_FILE and JPG_LOGMODE_DIALOG can be combined
 * JPG_LOGMODE_NONE is exclusive
 *
 *
 * Decide whether to emit a trace or warning message.
 * msg_level is one of:
 *   -1: recoverable corrupt-data warning, may want to abort.
 *    0: important advisory messages (always display to user).
 *    1: first level of tracing detail.
 *    2,3,...: successively more detailed tracing messages.
 * An application might override this method if it wanted to abort on warnings
 * or change the policy about which messages to display.
 */

#define JPG_LOGMODE_NONE	1			/* default, no output to file or display */
#define JPG_LOGMODE_FILE	2			/* output to file */
#define JPG_LOGMODE_DIALOG	4		/* output to display */

#ifdef	WIN32
#pragma pack(push, 1)
#endif

#define CS_UNKNOWN		0		/* error/unspecified */
#define CS_GRAYSCALE	1		/* monochrome */
#define CS_RGB				2		/* red/green/blue */
#define CS_YCbCr			3		/* Y/Cb/Cr (also known as YUV) */
#define CS_CMYK				4		/* C/M/Y/K */
#define CS_YCCK				5		/* Y/Cb/Cr/K */

typedef struct IWJPGINFO_TAG
{
	long lWidth;
	long lHeight;
	long lScaleUnits; // 1 == pixels/inch, 2 == pixels/centimeter
	long lHPS; // horizontal pixel scale
	long lVPS; // vertical pixel scale
	long lCompression; // 1 indicates baseline jpg(JPEGB), 2 indicates lossless compression (JPEGL)
	long lColorSpace; // one of the above defines, CS_YCbCr == NIST YCC
} IWJPGINFO;

#define COMMENT_SEPERATOR "C||C"

void SetLogMode(int nMode, int nMsg_level);
BOOL SetLogFile(char *pDestFile); /* complete path to log file */

/* BMP_to_JPG() - creates a jpg file from a .bmp file, returns 0 if fails 
 *							- pass 0 to use default compresion factor of 85
 *							- valid compression values are 26 - 100
 */

int BMP_to_JPG(char *pSourcePath, char *pDestPath, int nCompression);

/* DIB_to_JPG() - creates a jpg file from a memory dib, returns 0 if fails 
 *							- pass 0 to use default compresion factor of 85
 *							- valid compression values are 26 - 100
 *							- bBottomUp, describes orientation of incoming DIB, usually DIBS are bottomup
 *		  			  - note: 32 bit dibs are converted to 24 bit dibs, our jpb library 
 *							  doesnt grok 32 bit dibs
 */

int DIB_to_JPG(HGLOBAL hSourceDIB, char *pDestPath, int nCompression, BOOL bBottomUp);

/* ConvertDIBtoJPG() - returns a handle to a jpg, created from hSourceDIB, returns null if fails
 *									 - pass 0 to use default compresion factor of 85
 *									 - valid compression values are 26 - 100
 *									 - bBottomUp, describes orientation of incoming DIB, usually DIBS are bottomup
 *									 - note: 32 bit dibs are converted to 24 bit dibs, our jpb library 
 *										 doesnt grok 32 bit dibs
 */

HGLOBAL ConvertDIBtoJPG(HGLOBAL hSourceDIB, int nCompression, BOOL bBottomUp);

/* AddComment() - to clear comments in jpg pass a 0 length string or a null pComment
 *								- otherwise the comment will be added to jpg
 *								- note: AddComment does not replace existing comments, it adds a new comment
 */

BOOL AddComment(char *pJPGPath, char *pComment);

/* GetComment() - comment(s) returned in pbuffer, multiple comments seperated by 'C||C'
 *								- (optional) jpg info returned in pjpginfo buffer
 */

BOOL GetComment(char *pJPGPath, char *pBuffer, int nBufferLen, char *pjpginfo, int ninfolen);

BOOL ReplaceComment(char *pJPGPath, char *pComment);

/* JPG_to_DIB() - creates a bmp file from jpg file input
 *							- returns 0 if error occurs
 *							
 */

int JPG_to_BMP(char *pJPGSrcPath, char *pBMPDestPath);

/* JPGMemory_to_BMP() - creates a bmp file from jpg memory handle input
 *							- returns 0 if error occurs
 *							
 */

int JPGMemory_to_BMP(HGLOBAL hJPG, char *pBMPDestPath);

/* JPGMemory_to_BMPMemory() - creates a bmp memory handle output from 
 *														jpg memory handle input
 *													- returns 0 if error occurs
 *							
 */

int JPGMemory_to_BMPMemory(HGLOBAL hJPG, HGLOBAL *phBMP);

int BMPMemory_to_JPGMemory(HGLOBAL hBMP, HGLOBAL *phJPG, int nCompression);

/* JPG_GetInfo() - extracts information about image 
 *													- returns 0 if error occurs
 *							
 */

int JPG_GetInfo(HGLOBAL hJPG, IWJPGINFO *jpgInfo);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef	WIN32
#pragma pack(pop)
#endif

#endif // _JPG_INTERFACE_H
