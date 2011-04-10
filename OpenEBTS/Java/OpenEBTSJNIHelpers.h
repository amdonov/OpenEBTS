#include <jni.h>
#include <string.h>

// Smart version of GetString*Chars ReleaseString*Chars and New*String to
// handle Win32/Linux and UNICODE/MBCS.
TCHAR* JNIGetString(JNIEnv *env, jstring js);
void JNIReleaseString(JNIEnv *env, jstring js, const TCHAR *sz);
TCHARPATH* JNIGetStringPath(JNIEnv *env, jstring js);
void JNIReleaseStringPath(JNIEnv *env, jstring js, const TCHARPATH *sz);
jstring JNINewString(JNIEnv *env, const TCHAR *sz);

void PackageReturnValue(JNIEnv *env, int ret, jobject joRet);

//void JNISetStringViaConstructor(JNIEnv* env, jstring js, TCHAR* sz);
void JNIAppendStringtoStringBuffer(JNIEnv* env, jobject sb, TCHAR* sz);
jobjectArray JNICreateStringArray(JNIEnv *env, const TCHAR **sz, int n);
jobjectArray JNICreate3xNIntArray(JNIEnv *env, int *a, int *b, int *c, int n);
jobject JNICreateNISTFieldRules(JNIEnv *env, const TCHAR *jszMNU,
	int nRecordType, int nField, int nSubfield, int nItem, const TCHAR *szDesc, const TCHAR *szLongDesc, const TCHAR *szCharType,
	const TCHAR *szSpecialChars, const TCHAR *szDateFormat, const TCHAR *szAdvancedRule, int nSizeMin, int nSizeMax, int nOccMin,
	int nOccMax, int nOffset, int bAutomaticallySet, int bMandatory);
jobject JNICreateNISTValueList(JNIEnv *env, const TCHAR *szTOT, const TCHAR *szMNU, int bMandatory, const TCHAR **szName,
	const TCHAR **szValue, int nValues);

int JNISetObjectFieldToInt(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, int nValue);
int JNISetObjectFieldToBoolean(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, int nValue);
int JNISetObjectFieldToString(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, const TCHAR *szFieldValue);
int JNISetObjectFieldToStringArray(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, const TCHAR **sz, int n);


// Conversion of image format specifiers: OpenEBTS uses the image extension string
// to specify the format.
const TCHAR* ToFormatString(int fmt);
int FromFormatString(const TCHAR* szFmt);

enum NISTImageFormat
{
	imageFormatRAW,
	imageFormatBMP,
	imageFormatJPG,
	imageFormatWSQ,
	imageFormatJP2,
	imageFormatFX4,
	imageFormatCBEFF,
	imageFormatPNG
};

struct _ImageInfo
{
	int		nWidth;
	int		nHeight;
	int		nFormat;
	int		nDepth;
};
typedef struct _ImageInfo ImageInfo;
