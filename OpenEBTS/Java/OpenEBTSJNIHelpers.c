#include "Config.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "OpenEBTSJNIHelpers.h"
#include "stdlib.h"


TCHAR* JNIGetString(JNIEnv *env, jstring js)
// Use correct string functions depending on whether we're using OpenEBTS UNICODE or MBCS
{
	TCHAR	*sz = NULL;

#ifdef UNICODE

#ifdef WIN32
		// UNICODE/WIN32: GetStringChars returns 16-bit wide chars so we're good to go
		sz = (TCHAR*)(*env)->GetStringChars(env, js, 0);
#else
		// UNICODE/*NIX: GetStringChars returns 16-bit wide chars but *NIX wchar_t is 4-bytes
		// so we need to reallocate
		const jchar	*jsz = NULL;
		jint		nLen = 0;
		int			i = 0;

		jsz = (*env)->GetStringChars(env, js, 0);
		nLen = (*env)->GetStringLength(env, js);

		// Allocate return buffer as 4-bytes * length (including null terminator)
		sz = malloc((nLen + 1) * sizeof(TCHAR));
		for (i = 0; i < nLen; i++)
		{
			sz[i] = jsz[i];
		}

		sz[nLen] = 0;

		// Might as well release the chars now, later we will need to release sz with free()
		(*env)->ReleaseStringChars(env, js, jsz);
#endif

#else
		// MBCS/WIN32 and *NIX: GetStringUTFChars returns 8-bit chars so we're good to go
		sz = (TCHAR*)(*env)->GetStringUTFChars(env, js, 0);
#endif

	return sz;
}

void JNIReleaseString(JNIEnv *env, jstring js, const TCHAR *sz)
{
#ifdef UNICODE

#ifdef WIN32
		// UNICODE/WIN32, regular release with ReleaseStringChars
		if (sz) (*env)->ReleaseStringChars(env, js, (jchar*)sz);
#else
		// UNICODE/*NIX: we release with free() since we used malloc() for this custom conversion
		// Might as well release the chars now, later we will need to release sz with free()
		if (sz) free((void*)sz);
#endif

#else
		// MBCS/WIN32 and *NIX, regular release with ReleaseStringUTFChars
		if (sz) (*env)->ReleaseStringUTFChars(env, js, (char*)sz);
#endif
}


TCHARPATH* JNIGetStringPath(JNIEnv *env, jstring js)
// Just like JNIGetString...except that on *NIX paths are always regular char*s
{
#ifdef WIN32
	return JNIGetString(env, js);
#else
	return (TCHARPATH*)(*env)->GetStringUTFChars(env, js, 0);
#endif
}

void JNIReleaseStringPath(JNIEnv *env, jstring js, const TCHARPATH *sz)
// Just like JNIGetString...except that on *NIX paths are always regular char*s
{
#ifdef WIN32
	JNIReleaseString(env, js, sz);
#else
	// *NIX is always char* for paths, be it MBCS or UTF-8
	if (sz) (*env)->ReleaseStringUTFChars(env, js, (char*)sz);
#endif
}

jstring JNINewString(JNIEnv *env, const TCHAR *sz)
// Does the work of NewString*, taking into account platform and character
{
	jstring	js = NULL;

#ifdef UNICODE

	int 	nLen = (int)_tcslen(sz);

#ifdef WIN32
	// UNICODE/WIN32: NewString takes 16-bit wide chars so we're good to go
	js = (*env)->NewString(env, (jchar*)sz, nLen);
#else
	// UNICODE/*NIX: TCHAR is wchar_t is 4-bytes, so we need to create a temporary 2-byte buffer
	int			i = 0;
	jchar 		*jsz = NULL;

	// Allocate temporary char buffer as 2-bytes * length (including null terminator)
	jsz = malloc((nLen + 1) * sizeof(jchar));

	// Allocate return buffer as 4-bytes * length (including null terminator)
	for (i = 0; i < nLen; i++)
	{
		// Note: we're dropping the upper 2 bytes of the 4-byte *NIX wchar_t, but
		// these should really be 0.
		jsz[i] = sz[i];
	}

	js = (*env)->NewString(env, (jchar*)jsz, nLen);

	free(jsz);
#endif

#else
	// MBCS/WIN32 and *NIX: NewStringUTF takes 8-bit chars so we're good to go
	js = (*env)->NewStringUTF(env, (char*)sz);
#endif

	return js;
}

void PackageReturnValue(JNIEnv *env, int ret, jobject joRet)
// Place OpenEBTS integer return value into the return value class' nRet member
{
	jclass		cls;
	jfieldID	fid;

	if (joRet == NULL) return; // caller passed in null object, no return value for you!

	cls = (*env)->GetObjectClass(env, joRet);
	if (cls == 0) return;

	fid = (*env)->GetFieldID(env, cls, "nRet", "I");
	if (fid == 0) return;

	(*env)->SetIntField(env, joRet, fid, ret);
}

/*
Due to Android Issue 13832, CallVoidMethod throws a java.lang.CloneNotSupportedException
(January 2011 - Future Releases will fix this). Hence we use a StringBuffer for returning
the possible IWReadVerification parse error.

void JNISetStringViaConstructor(JNIEnv* env, jstring js, TCHAR* sz)
// Do all the black magic required to populate the Java String js
// with the string sz.
{
	jstring		jsNew = NULL;
	jclass		cls;
	jmethodID	mid;

	jsNew = JNINewString(env, sz);

	cls = (*env)->GetObjectClass(env, js);
	if (cls == 0) return;

	mid = (*env)->GetMethodID(env, cls, "<init>", "(Ljava/lang/String;)V");
	if (mid == 0) return;

	(*env)->CallVoidMethod(env, js, mid, jsNew);

	if ((*env)->ExceptionOccurred(env))
	{
         (*env)->ExceptionDescribe(env);
         (*env)->ExceptionClear(env);
	}
}
*/

void JNIAppendStringtoStringBuffer(JNIEnv* env, jobject sb, TCHAR* sz)
// Call the append method of the input StringBuffer with the provided string
// sz as input.
{
	jstring		js = NULL;
	jclass		cls;
	jmethodID	mid;

    if (sb == NULL) return;

	js = JNINewString(env, sz);

	cls = (*env)->GetObjectClass(env, sb);
	if (cls == 0) return;

    mid = (*env)->GetMethodID(env, cls, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
    if (mid == 0) return;

    (*env)->CallObjectMethod(env, sb, mid, js);
}

jobjectArray JNICreateStringArray(JNIEnv* env, const TCHAR** sz, int n)
// More magic, this time to return a created string array.
{
	int				i;
	jclass			cls;
	jstring			js;
	jstring			jsEmpty;
	jobjectArray	jsa = NULL;

	cls = (*env)->FindClass(env, "java/lang/String");
	if (cls == 0) goto done;
	jsEmpty	=  JNINewString(env, _T(""));

	jsa = (*env)->NewObjectArray(env, n, cls, jsEmpty);

	for (i = 0; i < n; i++)
	{
		js = JNINewString(env, sz[i]);
		(*env)->SetObjectArrayElement(env, jsa, i, js);
		// These can take a lot of precious Java VM space,
		// so we explicitly delete them right away
		(*env)->DeleteLocalRef(env, js);
	}

done:
	return jsa;
}

jobjectArray JNICreate3xNIntArray(JNIEnv *env, int *a, int *b, int *c, int n)
// Creates a 3 x N Java array of integers
{
	jobjectArray	jnax3 = NULL;
	jintArray		jna1 = NULL;
	jintArray		jna2 = NULL;
	jintArray		jna3 = NULL;
	jclass			cls;

	cls = (*env)->FindClass(env, "[I");
	if (cls == 0) goto done;

	jnax3 = (*env)->NewObjectArray(env, 3, cls, NULL);

	// Allocate 3 integer arrays of length n
	jna1 = (*env)->NewIntArray(env, n); if (jna1 == NULL) goto done;
	jna2 = (*env)->NewIntArray(env, n); if (jna2 == NULL) goto done;
	jna3 = (*env)->NewIntArray(env, n); if (jna3 == NULL) goto done;

	// Set the function's input array to these three arrays
	(*env)->SetIntArrayRegion(env, jna1, 0, n, a);
	(*env)->SetIntArrayRegion(env, jna2, 0, n, b);
	(*env)->SetIntArrayRegion(env, jna3, 0, n, c);

	// Add the three array as elements of our object array
	(*env)->SetObjectArrayElement(env, jnax3, 0, jna1);
	(*env)->SetObjectArrayElement(env, jnax3, 1, jna2);
	(*env)->SetObjectArrayElement(env, jnax3, 2, jna3);

	// Free the allocated one dimensional integer arrays, so they don't linger
	// too long in the Java VM
	(*env)->DeleteLocalRef(env, jna1);
	(*env)->DeleteLocalRef(env, jna2);
	(*env)->DeleteLocalRef(env, jna3);

done:
	return 	jnax3;
}

jobject JNICreateNISTFieldRules(JNIEnv *env, const TCHAR *jszMNU,
	int nRecordType, int nField, int nSubfield, int nItem, const TCHAR *szDesc, const TCHAR *szLongDesc, const TCHAR *szCharType,
	const TCHAR *szSpecialChars, const TCHAR *szDateFormat, const TCHAR *szAdvancedRule, int nSizeMin, int nSizeMax, int nOccMin,
	int nOccMax, int nOffset, int bAutomaticallySet, int bMandatory)
// Creates a NISTFieldRules object
{
	jobject		jobjRules = NULL;
	jclass		cls;

	cls = (*env)->FindClass(env, "com/obi/OpenEBTS$NISTFieldRules");
	if (cls == 0) goto done;

	jobjRules = (*env)->AllocObject(env, cls);
	if (jobjRules == NULL) goto done;

	JNISetObjectFieldToString(env, cls, jobjRules, "_sMNU", jszMNU);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nRecordType", nRecordType);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nField", nField);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nSubfield", nSubfield);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nItem", nItem);
	// What the Verification File calls Descriptions is really the field's name, and what
	// it calls Long Description is really just a description.
	JNISetObjectFieldToString(env, cls, jobjRules, "_sName", szDesc);
	JNISetObjectFieldToString(env, cls, jobjRules, "_sDescription", szLongDesc);
	JNISetObjectFieldToString(env, cls, jobjRules, "_sCharType", szCharType);
	JNISetObjectFieldToString(env, cls, jobjRules, "_sSpecialChars", szSpecialChars);
	JNISetObjectFieldToString(env, cls, jobjRules, "_sDateFormat", szDateFormat);
	JNISetObjectFieldToString(env, cls, jobjRules, "_sAdvancedRule", szAdvancedRule);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nSizeMin", nSizeMin);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nSizeMax", nSizeMax);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nOccurrencesMin", nOccMin);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nOccurrencesMax", nOccMax);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nOffset", nOffset);
	JNISetObjectFieldToInt(env, cls, jobjRules, "_nSizeMax", nSizeMax);
	JNISetObjectFieldToBoolean(env, cls, jobjRules, "_bAutomaticallySet", bAutomaticallySet);
	JNISetObjectFieldToBoolean(env, cls, jobjRules, "_bMandatory", bMandatory);

done:
	return jobjRules;
}

int JNISetObjectFieldToInt(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, int nValue)
{
	jfieldID	fid;

	fid = (*env)->GetFieldID(env, cls, szFieldName, "I");
	if (fid == 0) return 0;

	(*env)->SetIntField(env, obj, fid, nValue);

	return 1;
}

int JNISetObjectFieldToBoolean(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, int nValue)
{
	jfieldID	fid;

	fid = (*env)->GetFieldID(env, cls, szFieldName, "Z");
	if (fid == 0) return 0;

	(*env)->SetBooleanField(env, obj, fid, nValue);

	return 1;
}

int JNISetObjectFieldToString(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, const TCHAR *szFieldValue)
{
	jfieldID	fid;
	jstring		jsFieldValue = NULL;

	fid = (*env)->GetFieldID(env, cls, szFieldName, "Ljava/lang/String;");
	if (fid == 0) return 0;

	jsFieldValue = JNINewString(env, szFieldValue);
	if (jsFieldValue == NULL) return 0;

	(*env)->SetObjectField(env, obj, fid, jsFieldValue);

	// We set the string to the class, now we can ditch it
	(*env)->DeleteLocalRef(env, jsFieldValue);

	return 1;
}

int JNISetObjectFieldToStringArray(JNIEnv *env, jclass cls, jobject obj, const char *szFieldName, const TCHAR **sz, int n)
{
	jfieldID		fid;
	jobjectArray	jsa = NULL;

	fid = (*env)->GetFieldID(env, cls, szFieldName, "[Ljava/lang/String;");
	if (fid == 0) return 0;

	jsa = JNICreateStringArray(env, sz, n);
	if (jsa == NULL) return 0;

	(*env)->SetObjectField(env, obj, fid, jsa);

	// We set the string array to the class, now we can ditch it
	(*env)->DeleteLocalRef(env, jsa);

	return 1;
}

jobject JNICreateNISTValueList(JNIEnv *env, const TCHAR *szTOT, const TCHAR *szMNU, int bMandatory, const TCHAR **szName, const TCHAR **szValue, int nValues)
// Create a NISTValueList that has these members:
//		boolean 		_bMandatory;
//		int				_nCount;
//		String[]		_saName;	
//		String[]		_saValue;	
{
	jobject			jobjValues = NULL;
	jclass			cls;

	cls = (*env)->FindClass(env, "com/obi/OpenEBTS$NISTValueList");
	if (cls == 0) goto done;

	jobjValues = (*env)->AllocObject(env, cls);
	if (jobjValues == NULL) goto done;

	if (!JNISetObjectFieldToBoolean(env, cls, jobjValues, "_bMandatory", bMandatory)) goto done;
	if (!JNISetObjectFieldToInt(env, cls, jobjValues, "_nCount", nValues)) goto done;
	if (!JNISetObjectFieldToStringArray(env, cls, jobjValues, "_saName", szName, nValues)) goto done;
	if (!JNISetObjectFieldToStringArray(env, cls, jobjValues, "_saValue", szValue, nValues)) goto done;

done:
	return jobjValues;
}

const TCHAR* ToFormatString(int fmt)
{
	switch (fmt)
	{
		case imageFormatRAW:	return _T("RAW"); break;
		case imageFormatBMP:	return _T("BMP"); break;
		case imageFormatJPG:	return _T("JPG"); break;
		case imageFormatWSQ:	return _T("WSQ"); break;
		case imageFormatJP2:	return _T("JP2"); break;
		case imageFormatFX4:	return _T("FX4"); break;
		case imageFormatCBEFF:	return _T("CBEFF"); break;
		case imageFormatPNG:	return _T("PNG"); break;
		default:				return _T("?"); break;	// shouldn't happen
	}
}

int FromFormatString(const TCHAR* szFmt)
{
	if (_tcscmp(szFmt, _T("RAW")) == 0)
	{
		return imageFormatRAW;
	}
	else if (_tcscmp(szFmt, _T("BMP")) == 0)
	{
		return imageFormatBMP;
	}
	else if (_tcscmp(szFmt, _T("JPG")) == 0)
	{
		return imageFormatJPG;
	}
	else if (_tcscmp(szFmt, _T("WSQ")) == 0)
	{
		return imageFormatWSQ;
	}
	else if (_tcscmp(szFmt, _T("JP2")) == 0)
	{
		return imageFormatJP2;
	}
	else if (_tcscmp(szFmt, _T("FX4")) == 0)
	{
		return imageFormatFX4;
	}
	else if (_tcscmp(szFmt, _T("CBEFF")) == 0)
	{
		return imageFormatCBEFF;
	}
	else if (_tcscmp(szFmt, _T("PNG")) == 0)
	{
		return imageFormatFX4;
	}
	else
	{
		return imageFormatRAW;	// shouldn't happen
	}
}
