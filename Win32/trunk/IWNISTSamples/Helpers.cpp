#include "stdafx.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "Helpers.h"

bool AddPrintHelper(CIWTransaction *pTrans, long lType, char *szFolder, char *szFile, char *szFmtIn, char *szFmtOut, int iImpressionType, int iFingerIndex)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szNum[10];
	char		szMNUImp[10];
	char		szMNUFgp[10];
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	if (lType !=  3 && lType !=  4 && lType !=  5 && lType != 6 && lType != 7 &&
		lType != 13 && lType != 14 && lType != 15)
	{
		printf("AddPrintHelper is for Type-3, Type-4, Type-5, Type-6, Type-7, Type-13, Type-14, and Type-15 records only");
		goto done;
	}

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	// Add Type-lType record and retrieve new index
	lRet = IWAddRecord(pTrans, lType, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set image data (let IWNIST convert the input to WSQ with compression factor 15)
	lRet = IWSetImage(pTrans, lType, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 15.0f);
	if (!CheckRet("IWSetImage", lRet)) goto done;

	// Set mandatory impression type
	_ltoa_s(iImpressionType, szNum, 10, 10);
	switch (lType)
	{
		case 3:  strcpy_s(szMNUImp, 10, "T3_IMP"); break;
		case 4:  strcpy_s(szMNUImp, 10, "T4_IMP"); break;
		case 5:  strcpy_s(szMNUImp, 10, "T5_IMP"); break;
		case 6:  strcpy_s(szMNUImp, 10, "T6_IMP"); break;
		case 7:  strcpy_s(szMNUImp, 10, "T7_IMP"); break;
		case 13: strcpy_s(szMNUImp, 10, "T13_IMP"); break;
		case 14: strcpy_s(szMNUImp, 10, "T14_IMP"); break;
		case 15: strcpy_s(szMNUImp, 10, "T15_IMP"); break;
		default: szMNUImp[0] = '\0';
	}
	lRet = IWSet(pTrans, szMNUImp, szNum, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory NIST fingerprint index (or palmprint position for Type-15)
	sprintf_s(szNum, lType == 15 ? "%03d" : "%d", iFingerIndex);
	switch (lType)
	{
		case 3:  strcpy_s(szMNUFgp, 10, "T3_FGP"); break;
		case 4:  strcpy_s(szMNUFgp, 10, "T4_FGP"); break;
		case 5:  strcpy_s(szMNUFgp, 10, "T5_FGP"); break;
		case 6:  strcpy_s(szMNUFgp, 10, "T6_FGP"); break;
		case 7:  strcpy_s(szMNUFgp, 10, "T7_FGP"); break;
		case 13: strcpy_s(szMNUFgp, 10, "T13_FGP"); break;
		case 14: strcpy_s(szMNUFgp, 10, "T14_FGP"); break;
		case 15: strcpy_s(szMNUFgp, 10, "T15_PLP"); break;
			default: szMNUFgp[0] = '\0';
	}
	lRet = IWSet(pTrans, szMNUFgp, szNum, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	if (lType == 13)
	{
		// Set mandatory source agency
		lRet = IWSet(pTrans, "T13_SRC", "TEST00001", 1, iIndex);
		if (!CheckRet("IWSet", lRet)) goto done;

		// Set mandatory latent capture date
		lRet = IWSet(pTrans, "T13_LCD", "20080709", 1, iIndex);
		if (!CheckRet("IWSet", lRet)) goto done;
	}
	else if (lType == 14)
	{
		// Set mandatory source agency
		lRet = IWSet(pTrans, "T14_SRC", "TEST00001", 1, iIndex);
		if (!CheckRet("IWSet", lRet)) goto done;

		// Set mandatory latent capture date
		lRet = IWSet(pTrans, "T14_LCD", "20080709", 1, iIndex);
		if (!CheckRet("IWSet", lRet)) goto done;
	}
	else if (lType == 15)
	{
		// Set mandatory source agency
		lRet = IWSet(pTrans, "T15_SRC", "TEST00001", 1, iIndex);
		if (!CheckRet("IWSet", lRet)) goto done;

		// Set mandatory palm capture date
		lRet = IWSet(pTrans, "T15_PCD", "20080709", 1, iIndex);
		if (!CheckRet("IWSet", lRet)) goto done;
	}

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddMinutiaeHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, int iImpressionType, int iFingerIndex)
//TODO: not sure how to proceed
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szNum[10];
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	// Add Type-9 record and retrieve new index
	lRet = IWAddRecord(pTrans, 9, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set mandatory impression type
	_ltoa_s(iImpressionType, szNum, 10, 10);
	lRet = IWSet(pTrans, "T9_IMP", szNum, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory NIST fingerprint index
	sprintf_s(szNum, 10, "%02d", iFingerIndex);
	lRet = IWSet(pTrans, "T9_FGP", szNum, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory minutiae format, S=Standard or U=User-defined
	lRet = IWSet(pTrans, "T9_FMT", "U", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddMugshotHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmtIn, char *szFmtOut)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 10, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 10, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 15.0f);
	if (!CheckRet("IWSetImage", lRet)) goto done;

	// Set mandatory image type to FACE
	lRet = IWSet(pTrans, "T10_IMT", "FACE", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory image source agency
	lRet = IWSet(pTrans, "T10_SRC", "TEST00001", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory image date of photo
	lRet = IWSet(pTrans, "T10_PHD", "20080220", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddSignatureHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmt)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 8, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 8, iIndex, szFmt, lSize, pBlob, szFmt, 0.0f);
	if (!CheckRet("IWSetImage", lRet)) goto done;

	// Set mandatory Signature type (0 = subject, 1 = processing official)
	lRet = IWSet(pTrans, "T8_SIG", "0", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddType16Helper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmtIn, char *szFmtOut, int iFeature)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szNum[10];
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 16, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 16, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 15.0f);
	if (!CheckRet("IWSetImage", lRet)) goto done;

	// Set mandatory feature identifier
	_ltoa_s(iFeature, szNum, 10, 10);
	lRet = IWSet(pTrans, "T16_FID", szNum, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddIrisHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmt, int iEye)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szNum[10];
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 17, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 17, iIndex, szFmt, lSize, pBlob, "jpg", 0.0f);
	if (!CheckRet("IWSetImage", lRet)) goto done;

	// Set mandatory feature identifier
	_ltoa_s(iEye, szNum, 10, 10);
	lRet = IWSet(pTrans, "T17_FID", szNum, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory source agency
	lRet = IWSet(pTrans, "T17_SRC", "TEST00001", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory iris capture date
	lRet = IWSet(pTrans, "T17_ICD", "20080709", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddCBEFFHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szBiometricType, char *szBDBOwner, char *szBDBType)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	char		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 99, &iIndex);
	if (!CheckRet("IWAddRecord", lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 99, iIndex, "cbeff", lSize, pBlob, "cbeff", 0.0f);
	if (!CheckRet("IWSetImage", lRet)) goto done;

	// Set mandatory source agency
	lRet = IWSet(pTrans, "T99_SRC", "TEST00001", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory biometric creation date
	lRet = IWSet(pTrans, "T99_BCD", "20080709145300Z", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory CBEFF header version
	lRet = IWSet(pTrans, "T99_HDV", "0101", 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory biometric type
	lRet = IWSet(pTrans, "T99_BTY", szBiometricType, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory BDB format owner
	lRet = IWSet(pTrans, "T99_BFO", szBDBOwner, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	// Set mandatory BDB format type
	lRet = IWSet(pTrans, "T99_BFT", szBDBType, 1, iIndex);
	if (!CheckRet("IWSet", lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

BYTE* ReadBLOBFromFile(char *szFilename, long *plFileSize)
{
	HANDLE	hFile;
	DWORD	cbFile;
	DWORD	cbRead;
	BYTE	*pBuf = NULL;

	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		cbFile = ::GetFileSize(hFile, NULL);
		pBuf = new BYTE [cbFile];
		ReadFile(hFile, pBuf, cbFile, &cbRead, NULL);
		CloseHandle(hFile);
		if (cbRead != cbFile) {
			delete [] pBuf;
			pBuf = NULL;
			_tprintf(_T("Failed to read file \"%s\"\n"), szFilename);
		} else {
			if (plFileSize != NULL) *plFileSize = cbFile;
		}
	} else {
		_tprintf(_T("Failed to open file \"%s\"\n"), szFilename);
	}

	return pBuf;
}

bool WriteBLOBToFile(char *szFilename, char* pBlob, long lBlobLen)
{
	HANDLE	hFile;
	DWORD	cbWritten;

	hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error: failed to open \"%s\", error %ld\n"), szFilename, GetLastError());
		return false;
	}
	if (!WriteFile(hFile, pBlob, lBlobLen, &cbWritten, NULL) || (cbWritten != (DWORD)lBlobLen)) {
		_tprintf(_T("Error: failed to write to \"%s\", error %ld\n"), szFilename, GetLastError());
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CheckRet(char *szContext, long lRetVal)
{
	char szErrMsg[256];

	if (lRetVal == IW_SUCCESS) return true;

	switch (lRetVal) {
		case IW_ERR_LOADING_VERICATION:
			sprintf_s(szErrMsg, 256, "%s: error loading verification file\n", szContext); break;
		case IW_ERR_OPENING_FILE_FOR_READING:
			sprintf_s(szErrMsg, 256, "%s: error opening file for reading\n", szContext); break;
		case IW_ERR_OPENING_FILE_FOR_WRITING:
			sprintf_s(szErrMsg, 256, "%s: error opening file for writing\n", szContext); break;
		case IW_ERR_READING_FILE:
			sprintf_s(szErrMsg, 256, "%s: error reading file\n", szContext); break;
		case IW_ERR_WRITING_FILE:
			sprintf_s(szErrMsg, 256, "%s: error writing file\n", szContext); break;
		case IW_ERR_RECORD_NOT_FOUND:
			sprintf_s(szErrMsg, 256, "%s: record not found error\n", szContext); break;
		case IW_ERR_TRANSACTION_NOT_LOADED:
			sprintf_s(szErrMsg, 256, "%s: transaction not loaded error\n", szContext); break;
		case IW_ERR_VERIFICATION_NOT_LOADED:
			sprintf_s(szErrMsg, 256, "%s: verification file not loaded error\n", szContext); break;
		case IW_ERR_NULL_POINTER:
			sprintf_s(szErrMsg, 256, "%s: null pointer error\n", szContext); break;
		case IW_ERR_NULL_TRANSACTION_POINTER:
			sprintf_s(szErrMsg, 256, "%s: null transaction error\n", szContext); break;
		case IW_ERR_UNSUPPORTED_IMAGE_FORMAT:
			sprintf_s(szErrMsg, 256, "%s: unsupported image format error\n", szContext); break;
		case IW_ERR_UNSUPPORTED_RECORD_TYPE:
			sprintf_s(szErrMsg, 256, "%s: unsupported record type\n", szContext); break;
		case IW_ERR_INDEX_OUT_OF_RANGE:
			sprintf_s(szErrMsg, 256, "%s: index out of range error\n", szContext); break;
		case IW_ERR_INVALID_SUBFIELD_NUM:
			sprintf_s(szErrMsg, 256, "%s: invalid subfield number error\n", szContext); break;
		case IW_ERR_MNEMONIC_NOT_FOUND:
			sprintf_s(szErrMsg, 256, "%s: mnemonic not found error\n", szContext); break;
		case IW_ERR_OUT_OF_MEMORY:
			sprintf_s(szErrMsg, 256, "%s: out of memory error\n", szContext); break;
		case IW_ERR_WSQ_COMPRESS:
			sprintf_s(szErrMsg, 256, "%s: wsq compression error\n", szContext); break;
		case IW_ERR_WSQ_DECOMPRESS:
			sprintf_s(szErrMsg, 256, "%s: wsq decompression error\n", szContext); break;
		case IW_ERR_IMAGE_CONVERSION:
			sprintf_s(szErrMsg, 256, "%s: error converting image\n", szContext); break;
		default:
			sprintf_s(szErrMsg, 256, "%s: unknown error %ld\n", szContext, lRetVal); break;
	}

	printf(szErrMsg);

	return false;
}
