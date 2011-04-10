#include "Config.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "OpenEBTSJNIHelpers.h"
#include "OpenEBTSJNI.h"


#define RETVAL			int ret = 0;
#define PACKAGERETVAL	PackageReturnValue(env, ret, joRet);

/*
 * Class:     OpenEBTS
 * Method:    IWNew
 * Signature: (Ljava/lang/String;LNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWNew
  (JNIEnv *env, jobject obj, jstring jsTOT, jobject joRet)
{
	RETVAL
	CIWTransaction 	*pTra = NULL;
	const TCHAR		*szTOT = NULL;
	jint 			nTransaction = 0;

	szTOT = JNIGetString(env, jsTOT);

	ret = IWNew(szTOT, NULL, &pTra);
	
	if (ret == IW_SUCCESS)
	{
		nTransaction = (jint)pTra;
	}

	JNIReleaseString(env, jsTOT, szTOT);
	PACKAGERETVAL

	return nTransaction;
}

/*
 * Class:     OpenEBTS
 * Method:    IWReadFromFile
 * Signature: (Ljava/lang/String;LNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWReadFromFile
  (JNIEnv *env, jobject obj, jstring jsPath, jint nVerification, jobject joRet)
{
	RETVAL
	const TCHARPATH		*szPath = NULL;
	CIWVerification		*pVer = (CIWVerification*)nVerification;
	CIWTransaction		*pTra = NULL;
	jint				nTransaction = 0;

	szPath = JNIGetStringPath(env, jsPath);

	ret = IWRead(szPath, pVer, &pTra);

	if (ret == IW_SUCCESS)
	{
		nTransaction = (jint)pTra;
	}

	JNIReleaseStringPath(env, jsPath, szPath);
	PACKAGERETVAL

	return nTransaction;
}

/*
 * Class:     OpenEBTS
 * Method:    IWWriteToFile
 * Signature: (ILjava/lang/String;LNISTFile/NISTFileFormat;LNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWWriteToFile
  (JNIEnv *env, jobject obj, jint nTransaction, jstring jsPath, jobject joFmt, jobject joRet)
{
	RETVAL
	const TCHARPATH	*szPath = NULL;
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;

	szPath = JNIGetStringPath(env, jsPath);

	ret = IWWrite(pTra, szPath);

	if (ret == IW_SUCCESS)
	{
		nTransaction = (jint)pTra;
	}

	JNIReleaseStringPath(env, jsPath, szPath);
	PACKAGERETVAL
} 

/*
 * Class:     OpenEBTS
 * Method:    IWClose
 * Signature: (ILOpenEBTS/NISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWClose
  (JNIEnv *env, jobject obj, jint nTransaction, jobject joRet)
 {
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;

	ret = IWClose(&pTra);

	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWAddRecord
 * Signature: (IILNISTReturn;)V
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWAddRecord
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	jint			nRecordIndex;

	ret = IWAddRecord(pTra, nRecordType, &nRecordIndex);

	PACKAGERETVAL
	return nRecordIndex;
}

/*
 * Class:     OpenEBTS
 * Method:    IWDeleteRecord
 * Signature: (IIILNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWDeleteRecord
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;

	ret = IWDeleteRecord(pTra, nRecordType, nRecordIndex);

	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetRecordTypeCount
 * Signature: (IILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetRecordTypeCount
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				count = 0;	

	ret = IWGetRecordTypeCount(pTra, nRecordType, &count);

	PACKAGERETVAL
	return count;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetNumRecords
 * Signature: (ILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetNumRecords
  (JNIEnv *env, jobject obj, jint nTransaction, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	jint			nCount;

	ret = IWGetNumRecords(pTra, &nCount);

	PACKAGERETVAL
	return nCount;
}

/*
 * Class:     OpenEBTS
 * Method:    IWSetDataViaMnemonic
 * Signature: (ILjava/lang/String;IILjava/lang/String;LNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWSetDataViaMnemonic
  (JNIEnv *env, jobject obj, jint nTransaction, jstring jsMNU, jint nRecordIndex, jint nSecondaryIndex,
  jstring jsData, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szMNU = NULL;
	const TCHAR		*szData = NULL;

	szMNU = JNIGetString(env, jsMNU);
	szData = JNIGetString(env, jsData);

	ret = IWSet(pTra, szMNU, szData, nSecondaryIndex, nRecordIndex);

	JNIReleaseString(env, jsMNU, szMNU);
	JNIReleaseString(env, jsData, szData);
	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetDataViaMnemonic
 * Signature: (ILjava/lang/String;IILNISTReturn;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_obi_OpenEBTS_IWGetDataViaMnemonic
  (JNIEnv *env, jobject obj, jint nTransaction, jstring jsMNU, jint nRecordIndex, jint nSecondaryIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szMNU = NULL;
	const TCHAR		*szData = NULL;
	jstring			jsData = NULL;

	szMNU = JNIGetString(env, jsMNU);

	ret = IWGet(pTra, szMNU, &szData, nSecondaryIndex, nRecordIndex);

	if (ret == IW_SUCCESS)
	{
		jsData = JNINewString(env, szData);
	}

	JNIReleaseString(env, jsMNU, szMNU);
	PACKAGERETVAL

	return jsData;
}

/*
 * Class:     OpenEBTS
 * Method:    IWSetImage
 * Signature: (III[BILNISTFile/NISTImageFormat;LNISTFile/NISTImageFormat;ILNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWSetImage
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jbyteArray jbaImage,
  jint nFmtIn, jint nFmtOut, jint nCompression, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	jboolean		jzIsCopy = 0;
	int				cbImage = 0;
	jbyte			*pImage = NULL;

	cbImage = (*env)->GetArrayLength(env, jbaImage);
	if (cbImage == 0) goto done;

	// Get Java byte array as pointer to bytes
	pImage = (*env)->GetByteArrayElements(env, jbaImage, &jzIsCopy);
	if (pImage == NULL) goto done;

	ret = IWSetImage(pTra, nRecordType, nRecordIndex, ToFormatString(nFmtIn), cbImage, pImage,
					 ToFormatString(nFmtOut), (float)nCompression);

done:
	if (jzIsCopy) (*env)->ReleaseByteArrayElements(env, jbaImage, pImage, 0);

	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetImage
 * Signature: (IIILNISTReturn;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_obi_OpenEBTS_IWGetImage
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szFmt = NULL;
	int				cbImage = 0;
	const void		*pImage = NULL;
	jbyteArray		jbaImage = NULL;

	ret = IWGetImage(pTra, nRecordType, nRecordIndex, &szFmt, &cbImage, &pImage);

	if (ret == IW_SUCCESS && cbImage > 0)
	{
		jbaImage = (*env)->NewByteArray(env, cbImage);
		(*env)->SetByteArrayRegion(env, jbaImage, 0, cbImage, pImage);
	}

	PACKAGERETVAL
	return jbaImage;
}

ImageInfo IWGetImageFormatHelper
(JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, int *pret)
//
// Unfortunately there aren't separate methods for these properties so this function
// may get called up to 4 times per image.
//
{
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szFmt = NULL;
	int				cbImage;
	int				nW;
	int				nH;
	int				nBPP;
	ImageInfo		ii;

	*pret = IWGetImageInfo(pTra, nRecordType, nRecordIndex, &szFmt, &cbImage, &nW, &nH, &nBPP);

	ii.nWidth = nW;
	ii.nHeight = nH;
	ii.nDepth = nBPP;
	ii.nFormat = FromFormatString(szFmt);

	return ii;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetImageFormat
 * Signature: (IIILNISTReturn;)LNISTFile/NISTImageFormat;
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetImageFormat
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	ImageInfo	ii;

	ii = IWGetImageFormatHelper(env, obj, nTransaction, nRecordType, nRecordIndex, &ret);

	PACKAGERETVAL
	return ii.nFormat;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetImageWidth
 * Signature: (IIILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetImageWidth
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	ImageInfo ii;

	ii = IWGetImageFormatHelper(env, obj, nTransaction, nRecordType, nRecordIndex, &ret);

	PACKAGERETVAL
	return ii.nWidth;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetImageHeight
 * Signature: (IIILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetImageHeight
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	ImageInfo ii;

	ii = IWGetImageFormatHelper(env, obj, nTransaction, nRecordType, nRecordIndex, &ret);

	PACKAGERETVAL
	return ii.nHeight;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetImageDepth
 * Signature: (IIILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetImageDepth
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	ImageInfo ii;

	ii = IWGetImageFormatHelper(env, obj, nTransaction, nRecordType, nRecordIndex, &ret);

	PACKAGERETVAL
	return ii.nDepth;
}

/*
 * Class:     OpenEBTS
 * Method:    IWSetImageFromFile
 * Signature: (IIILjava/lang/String;LNISTFile/NISTImageFormat;LNISTFile/NISTImageFormat;ILNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWSetImageFromFile
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex,
  jstring jsPath, jint nFmtIn, jint nFmtOut, jint nCompression, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHARPATH	*szPath = NULL;

	szPath = JNIGetStringPath(env, jsPath);

	ret = IWImportImage(pTra, nRecordType, nRecordIndex, szPath, ToFormatString(nFmtOut),
						(float)nCompression, ToFormatString(nFmtIn));

	JNIReleaseStringPath(env, jsPath, szPath);
	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetImageToFile
 * Signature: (IIILjava/lang/String;LNISTFile/NISTImageFormat;LNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWGetImageToFile
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jstring jsPath, jint nFmt, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHARPATH	*szPath = NULL;

	szPath = JNIGetStringPath(env, jsPath);

	ret = IWExportImage(pTra, nRecordType, nRecordIndex, szPath, ToFormatString(nFmt));

	JNIReleaseStringPath(env, jsPath, szPath);
	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWReadVerification
 * Signature: (Ljava/lang/String;LNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWReadVerification
  (JNIEnv *env, jobject obj, jstring jsPath, jobject jobsbParseErrorOut, jobject joRet)
{
	RETVAL
	const TCHARPATH		*szPath = NULL;
	CIWVerification		*pVer = NULL;
	jint				nVerification = 0;
	TCHAR				szParseError[1024];

	szPath = JNIGetStringPath(env, jsPath);

	ret = IWReadVerification(szPath, &pVer, 1024, szParseError);

	if (ret == IW_SUCCESS)
	{
		nVerification = (jint)pVer;
	}
	else
	{
		if (_tcslen(szParseError))
		{
			JNIAppendStringtoStringBuffer(env, jobsbParseErrorOut, szParseError);
		}
	}

	JNIReleaseStringPath(env, jsPath, szPath);
	PACKAGERETVAL

	return nVerification;
}

/*
 * Class:     OpenEBTS
 * Method:    IWCloseVerification
 * Signature: (ILNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWCloseVerification
  (JNIEnv *env, jobject obj, jint nVerification, jobject joRet)
{
	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;

	ret = IWCloseVerification(&pVer);

	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWSetVerification
 * Signature: (IILNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWSetVerification
  (JNIEnv *env, jobject obj, jint nTransaction, jint nVerification, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	CIWVerification	*pVer = (CIWVerification*)nVerification;

	ret = IWSetVerification(pTra, pVer);

	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWVerify
 * Signature: (ILNISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWVerify
  (JNIEnv *env, jobject obj, jint nTransaction, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;

	ret = IWVerify(pTra);

	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetErrorCount
 * Signature: (ILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetErrorCount
  (JNIEnv *env, jobject obj, jint nTransaction, jobject joRet)
// Note, in this function ret is st explicitly to IW_SUCCESS since
// IWGetErrorCount doesn't return an error code.
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				nCount = 0;

	nCount = IWGetErrorCount(pTra);
	ret = IW_SUCCESS;

	PACKAGERETVAL
	return nCount;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetErrorString
 * Signature: (IILOpenEBTS/NISTReturn;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_obi_OpenEBTS_IWGetErrorString
 (JNIEnv *env, jobject obj, jint nTransaction, jint nIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				nCode = 0;
	const TCHAR		*szError = NULL;
	jstring			jsError = NULL;

	ret = IWGetError(pTra, nIndex, &nCode, &szError);

	if (ret == IW_SUCCESS && _tcslen(szError))
	{
		jsError = JNINewString(env, szError);
	}

	PACKAGERETVAL
	return jsError;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetErrorCode
 * Signature: (IILOpenEBTS/NISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetErrorCode
  (JNIEnv *env, jobject obj, jint nTransaction, jint nIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szError = NULL;
	int				nCode = 0;

	ret = IWGetError(pTra, nIndex, &nCode, &szError);

	PACKAGERETVAL
	return nCode;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetFieldCount
 * Signature: (IIILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetFieldCount
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				count = 0;	

	ret = IWGetNumFields(pTra, nRecordType, nRecordIndex, &count);

	PACKAGERETVAL
	return count;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetNextField
 * Signature: (IIIILNISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWGetNextField
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jint nFieldIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				nFieldIndexNext = 0;	

	ret = IWGetNextField(pTra, nRecordType, nRecordIndex, nFieldIndex, &nFieldIndexNext);

	PACKAGERETVAL
	return nFieldIndexNext;
}

/*
 * Class:     OpenEBTS
 * Method:    IWNumSubfields
 * Signature: (IIIILOpenEBTS/NISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWNumSubfields
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jint nFieldIndex,
  jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				count = 0;

	ret = IWNumSubfields(pTra, nRecordType, nRecordIndex, nFieldIndex, &count);

	PACKAGERETVAL
	return count;
}

/*
 * Class:     OpenEBTS
 * Method:    IWNumItems
 * Signature: (IIIIILOpenEBTS/NISTReturn;)I
 */
JNIEXPORT jint JNICALL Java_com_obi_OpenEBTS_IWNumItems
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jint nFieldIndex,
  jint nSubfieldIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	int				count = 0;

	ret = IWNumItems(pTra, nRecordType, nRecordIndex, nFieldIndex, nSubfieldIndex, &count);

	PACKAGERETVAL
	return count;
}

/*
 * Class:     OpenEBTS
 * Method:    IWFindItem
 * Signature: (IIIIIILOpenEBTS/NISTReturn;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_obi_OpenEBTS_IWFindItem
  (JNIEnv *env, jobject obj, jint nTransaction, jint nRecordType, jint nRecordIndex, jint nFieldIndex,
  jint nSubfieldIndex, jint nItemIndex, jobject joRet)
 {
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szData = NULL;
	jstring			jsData = NULL;

	ret = IWFindItem(pTra, nRecordType, nRecordIndex, nFieldIndex, nSubfieldIndex, nItemIndex, &szData);

	if (ret == IW_SUCCESS)
	{
		jsData = JNINewString(env, szData);
	}

	PACKAGERETVAL
	return jsData;
 }

/*
 * Class:     OpenEBTS
 * Method:    IWSetItem
 * Signature: (ILjava/lang/String;IIIIILOpenEBTS/NISTReturn;)V
 */
JNIEXPORT void JNICALL Java_com_obi_OpenEBTS_IWSetItem
  (JNIEnv *env, jobject obj, jint nTransaction, jstring jsData, jint nRecordType, jint nRecordIndex,
  jint nFieldIndex, jint nSubfieldIndex, jint nItemIndex, jobject joRet)
{
	RETVAL
	CIWTransaction	*pTra = (CIWTransaction*)nTransaction;
	const TCHAR		*szData = NULL;

	szData = JNIGetString(env, jsData);

	ret = IWSetItem(pTra, szData, nRecordType, nRecordIndex, nFieldIndex, nSubfieldIndex, nItemIndex);

	JNIReleaseString(env, jsData, szData);
	PACKAGERETVAL
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetTransactionCategories
 * Signature: (I[Ljava/lang/String;LOpenEBTS/NISTReturn;)V
 */
JNIEXPORT jobjectArray JNICALL Java_com_obi_OpenEBTS_IWGetTransactionCategories
  (JNIEnv *env, jobject obj, jint nVerification, jobject joRet)
{
	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	int				nCategories = 0;
	const TCHAR		*szCategories[100];	// a reasonable amount of categories
	jobjectArray	jsaCategories = NULL;

	ret = IWGetTransactionCategories(pVer, 100, (const TCHAR**)&szCategories, &nCategories);

	if (ret == IW_SUCCESS && nCategories > 0)
	{
		jsaCategories = JNICreateStringArray(env, szCategories, nCategories);
	}

	PACKAGERETVAL
	return jsaCategories;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetTransactionTypeNames
 * Signature: (ILjava/lang/String;LOpenEBTS/NISTReturn;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_obi_OpenEBTS_IWGetTransactionTypeNames
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsCategory, jobject joRet)
 {
 	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	int				nTOTs = 0;
	const TCHAR		*szNames[100];	// a reasonable amount of TOTs
	const TCHAR		*szCategory = NULL;
	jobjectArray	jsaNames = NULL;

	szCategory = JNIGetString(env, jsCategory);

	ret = IWGetTransactionTypes(pVer, 100, (const TCHAR**)&szNames, NULL, &nTOTs, szCategory);

	if (ret == IW_SUCCESS && nTOTs > 0)
	{
		jsaNames = JNICreateStringArray(env, szNames, nTOTs);
	}

	JNIReleaseString(env, jsCategory, szCategory);

	PACKAGERETVAL
	return jsaNames;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetTransactionTypeDescriptions
 * Signature: (ILjava/lang/String;LOpenEBTS/NISTReturn;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_obi_OpenEBTS_IWGetTransactionTypeDescriptions
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsCategory, jobject joRet)
{
 	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	int				nTOTs = 0;
	const TCHAR		*szDescriptions[100];	// a reasonable amount of TOTs
	const TCHAR		*szCategory = NULL;
	jobjectArray	jsaDescriptions = NULL;

	szCategory = JNIGetString(env, jsCategory);

	ret = IWGetTransactionTypes(pVer, 100, NULL, szDescriptions, &nTOTs, szCategory);

	if (ret == IW_SUCCESS && nTOTs > 0)
	{
		jsaDescriptions = JNICreateStringArray(env, szDescriptions, nTOTs);
	}

	JNIReleaseString(env, jsCategory, szCategory);

	PACKAGERETVAL
	return jsaDescriptions;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetRecordTypeOccurrences
 * Signature: (ILjava/lang/String;LOpenEBTS/NISTReturn;)[[I
 */
JNIEXPORT jobjectArray JNICALL Java_com_obi_OpenEBTS_IWGetRecordTypeOccurrences
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsTOT, jobject joRet)
// IWGetRecordTypeOccurrences returns a 2 dimensional array of ints of size
// 3 x n, where index 0 indexes the Record Type array, and index 1 and 2 the min
// and max allowed occurrences arrays, respectively.
{
 	RETVAL
 	jobjectArray	jnaOccurrences;
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	int				nOccurrences = 0;
	const TCHAR		*szTOT = NULL;
	int				rgnRecordType[100];
	int				rgnMin[100];
	int				rgnMax[100];

	szTOT = JNIGetString(env, jsTOT);

	ret = IWGetRecordTypeOccurrences(pVer, 100, rgnRecordType, rgnMin, rgnMax, &nOccurrences, szTOT);

	if (ret == IW_SUCCESS && nOccurrences > 0)
	{
		jnaOccurrences = JNICreate3xNIntArray(env, rgnRecordType, rgnMin, rgnMax, nOccurrences);
	}

	JNIReleaseString(env, jsTOT, szTOT);
	PACKAGERETVAL
	return jnaOccurrences;
}


/*
 * Class:     OpenEBTS
 * Method:    IWGetMnemonicsNames
 * Signature: (ILjava/lang/String;LOpenEBTS/NISTReturn;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_obi_OpenEBTS_IWGetMnemonicsNames
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsTOT, jobject joRet)
{
 	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	int				nMNUs = 0;
	const TCHAR		*szTOT = NULL;
	const TCHAR		*szNames[1000];	// a reasonable amount of MNUs
	jobjectArray	jsaNames = NULL;

	szTOT = JNIGetString(env, jsTOT);

	ret = IWGetMnemonics(pVer, szTOT, 1000, szNames, NULL, &nMNUs);

	if (ret == IW_SUCCESS && nMNUs > 0)
	{
		jsaNames = JNICreateStringArray(env, szNames, nMNUs);
	}

	JNIReleaseString(env, jsTOT, szTOT);

	PACKAGERETVAL
	return jsaNames;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetMnemonicsDescriptions
 * Signature: (ILjava/lang/String;LOpenEBTS/NISTReturn;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_obi_OpenEBTS_IWGetMnemonicsDescriptions
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsTOT, jobject joRet)
{
 	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	int				nMNUs = 0;
	const TCHAR		*szDescriptions[1000];	// a reasonable amount of MNUs
	jobjectArray	jsaDescriptions = NULL;
	const TCHAR		*szTOT = NULL;

	szTOT = JNIGetString(env, jsTOT);

	ret = IWGetMnemonics(pVer, szTOT, 1000, NULL, szDescriptions, &nMNUs);

	if (ret == IW_SUCCESS && nMNUs > 0)
	{
		jsaDescriptions = JNICreateStringArray(env, szDescriptions, nMNUs);
	}

	JNIReleaseString(env, jsTOT, szTOT);
	PACKAGERETVAL
	return jsaDescriptions;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetRuleRestrictions
 * Signature: (ILjava/lang/String;Ljava/lang/String;LOpenEBTS/NISTReturn;)LOpenEBTS/NISTFieldRules;
 */
JNIEXPORT jobject JNICALL Java_com_obi_OpenEBTS_IWGetRuleRestrictions
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsTOT, jstring jsMNU, jobject joRet)
// Because IWGetRuleRestrictions returns so many parameters in C, in Java we return an entire
// structure, NISTFieldRules.
{
 	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	const TCHAR		*szTOT = NULL;
	const TCHAR		*szMNU = NULL;
	jobject			jobjRules = NULL;
	int				nRecordType;
	int				nField;
	int				nSubfield;
	int				nItem;
	const TCHAR		*szDesc = NULL;
	const TCHAR		*szLongDesc = NULL;
	const TCHAR		*szCharType = NULL;
	const TCHAR		*szSpecialChars = NULL;
	const TCHAR		*szDateFormat = NULL;
	const TCHAR		*szAdvancedRule = NULL;
	int				nSizeMin;
	int				nSizeMax;
	int				nOccMin;
	int				nOccMax;
	int				nOffset;
	int				bAutomaticallySet;
	int				bMandatory;

	szTOT = JNIGetString(env, jsTOT);
	szMNU = JNIGetString(env, jsMNU);

	ret = IWGetRuleRestrictions(pVer, szTOT, szMNU,
								&nRecordType, &nField, &nSubfield, &nItem, &szDesc, &szLongDesc, &szCharType,
								&szSpecialChars, &szDateFormat, &szAdvancedRule, &nSizeMin, &nSizeMax, &nOccMin,
								&nOccMax, &nOffset, &bAutomaticallySet, &bMandatory);

	if (ret == IW_SUCCESS)
	{
		jobjRules = JNICreateNISTFieldRules(env, szMNU,
						nRecordType, nField, nSubfield, nItem, szDesc, szLongDesc, szCharType,
						szSpecialChars, szDateFormat, szAdvancedRule, nSizeMin, nSizeMax, nOccMin,
						nOccMax, nOffset, bAutomaticallySet, bMandatory);
	}

	JNIReleaseString(env, jsTOT, szTOT);
	JNIReleaseString(env, jsMNU, szMNU);
	PACKAGERETVAL
	return jobjRules;
}

/*
 * Class:     OpenEBTS
 * Method:    IWGetValueList
 * Signature: (ILjava/lang/String;Ljava/lang/String;LOpenEBTS/NISTReturn;)LOpenEBTS/NISTValueList;
 */
JNIEXPORT jobject JNICALL Java_com_obi_OpenEBTS_IWGetValueList
  (JNIEnv *env, jobject obj, jint nVerification, jstring jsTOT, jstring jsMNU, jobject joRet)
 {
 	RETVAL
	CIWVerification	*pVer = (CIWVerification*)nVerification;
	const TCHAR		*szTOT = NULL;
	const TCHAR		*szMNU = NULL;
	const TCHAR		*szName[1000];	// a reasonable amount of values
	const TCHAR		*szValue[1000];
	int				nValues;
	int				bMandatory;
	jobject			jobjValues = NULL;

	szTOT = JNIGetString(env, jsTOT);
	szMNU = JNIGetString(env, jsMNU);

	ret = IWGetValueList(pVer, szTOT, szMNU, &bMandatory, 1000, szValue, szName, &nValues);

	if (ret == IW_SUCCESS && nValues >  0)
	{
		jobjValues = JNICreateNISTValueList(env, szTOT, szMNU, bMandatory, szName, szValue, nValues);
	}

	JNIReleaseString(env, jsTOT, szTOT);
	JNIReleaseString(env, jsMNU, szMNU);
	PACKAGERETVAL
	return jobjValues;
 }
