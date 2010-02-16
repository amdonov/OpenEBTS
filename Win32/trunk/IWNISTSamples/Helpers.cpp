#include "stdafx.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "Helpers.h"

bool AddPrintHelper(CIWTransaction *pTrans, long lType, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmtIn, TCHAR *szFmtOut, int iImpressionType, int iFingerIndex)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHAR		szMNUImp[10];
	TCHAR		szMNUFgp[10];
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	if (lType !=  3 && lType !=  4 && lType !=  5 && lType != 6 && lType != 7 &&
		lType != 13 && lType != 14 && lType != 15)
	{
		_tprintf(_T("AddPrintHelper is for Type-3, Type-4, Type-5, Type-6, Type-7, Type-13, Type-14, and Type-15 records only"));
		goto done;
	}

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	// Add Type-lType record and retrieve new index
	lRet = IWAddRecord(pTrans, lType, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data (let IWNIST convert the input to WSQ with compression factor of 15)
	lRet = IWSetImage(pTrans, lType, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 15.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory impression type
	_ltot_s(iImpressionType, szNum, 10, 10);
	switch (lType)
	{
		case 3:  _tcscpy_s(szMNUImp, 10, _T("T3_IMP")); break;
		case 4:  _tcscpy_s(szMNUImp, 10, _T("T4_IMP")); break;
		case 5:  _tcscpy_s(szMNUImp, 10, _T("T5_IMP")); break;
		case 6:  _tcscpy_s(szMNUImp, 10, _T("T6_IMP")); break;
		case 7:  _tcscpy_s(szMNUImp, 10, _T("T7_IMP")); break;
		case 13: _tcscpy_s(szMNUImp, 10, _T("T13_IMP")); break;
		case 14: _tcscpy_s(szMNUImp, 10, _T("T14_IMP")); break;
		case 15: _tcscpy_s(szMNUImp, 10, _T("T15_IMP")); break;
		default: szMNUImp[0] = '\0';
	}
	lRet = IWSet(pTrans, szMNUImp, szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory NIST fingerprint index (or palmprint position for Type-15)
	_stprintf_s(szNum, lType == 15 ? _T("%03d") : _T("%d"), iFingerIndex);
	switch (lType)
	{
		case 3:  _tcscpy_s(szMNUFgp, 10, _T("T3_FGP")); break;
		case 4:  _tcscpy_s(szMNUFgp, 10, _T("T4_FGP")); break;
		case 5:  _tcscpy_s(szMNUFgp, 10, _T("T5_FGP")); break;
		case 6:  _tcscpy_s(szMNUFgp, 10, _T("T6_FGP")); break;
		case 7:  _tcscpy_s(szMNUFgp, 10, _T("T7_FGP")); break;
		case 13: _tcscpy_s(szMNUFgp, 10, _T("T13_FGP")); break;
		case 14: _tcscpy_s(szMNUFgp, 10, _T("T14_FGP")); break;
		case 15: _tcscpy_s(szMNUFgp, 10, _T("T15_PLP")); break;
			default: szMNUFgp[0] = '\0';
	}
	lRet = IWSet(pTrans, szMNUFgp, szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	if (lType == 13)
	{
		// Set mandatory source agency
		lRet = IWSet(pTrans, _T("T13_SRC"), _T("TEST00001"), 1, iIndex);
		if (!CheckRet(_T("IWSet"), lRet)) goto done;

		// Set mandatory latent capture date
		lRet = IWSet(pTrans, _T("T13_LCD"), _T("20080709"), 1, iIndex);
		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	}
	else if (lType == 14)
	{
		// Set mandatory source agency
		lRet = IWSet(pTrans, _T("T14_SRC"), _T("TEST00001"), 1, iIndex);
		if (!CheckRet(_T("IWSet"), lRet)) goto done;

		// Set mandatory latent capture date
		lRet = IWSet(pTrans, _T("T14_LCD"), _T("20080709"), 1, iIndex);
		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	}
	else if (lType == 15)
	{
		// Set mandatory source agency
		lRet = IWSet(pTrans, _T("T15_SRC"), _T("TEST00001"), 1, iIndex);
		if (!CheckRet(_T("IWSet"), lRet)) goto done;

		// Set mandatory palm capture date
		lRet = IWSet(pTrans, _T("T15_PCD"), _T("20080709"), 1, iIndex);
		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	}

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddMinutiaeHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, int iImpressionType, int iFingerIndex)
//TODO: not sure how to proceed
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	// Add Type-9 record and retrieve new index
	lRet = IWAddRecord(pTrans, 9, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set mandatory impression type
	_ltot_s(iImpressionType, szNum, 10, 10);
	lRet = IWSet(pTrans, _T("T9_IMP"), szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory NIST fingerprint index
	_stprintf_s(szNum, 10, _T("%02d"), iFingerIndex);
	lRet = IWSet(pTrans, _T("T9_FGP"), szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory minutiae format, S=Standard or U=User-defined
	lRet = IWSet(pTrans, _T("T9_FMT"), _T("U"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddMugshotHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmtIn, TCHAR *szFmtOut)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 10, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 10, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 90.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory image type to FACE
	lRet = IWSet(pTrans, _T("T10_IMT"), _T("FACE"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory image source agency
	lRet = IWSet(pTrans, _T("T10_SRC"), _T("TEST00001"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory image date of photo
	lRet = IWSet(pTrans, _T("T10_PHD"), _T("20080220"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddSignatureHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmt)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 8, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 8, iIndex, szFmt, lSize, pBlob, szFmt, 0.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory Signature type (0 = subject, 1 = processing official)
	lRet = IWSet(pTrans, _T("T8_SIG"), _T("0"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddType16Helper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmtIn, TCHAR *szFmtOut, int iFeature)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 16, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 16, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 90.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory feature identifier
	_ltot_s(iFeature, szNum, 10, 10);
	lRet = IWSet(pTrans, _T("T16_FID"), szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddIrisHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmt, int iEye)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 17, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 17, iIndex, szFmt, lSize, pBlob, _T("raw"), 0.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory feature identifier
	_ltot_s(iEye, szNum, 10, 10);
	lRet = IWSet(pTrans, _T("T17_FID"), szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory source agency
	lRet = IWSet(pTrans, _T("T17_SRC"), _T("TEST00001"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory iris capture date
	lRet = IWSet(pTrans, _T("T17_ICD"), _T("20080709"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddCBEFFHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szBiometricType, TCHAR *szBDBOwner, TCHAR *szBDBType)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szFilename[_MAX_PATH];

	_tmakepath_s(szFilename, _MAX_PATH, NULL, szFolder, szFile, NULL);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 99, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 99, iIndex, _T("cbeff"), lSize, pBlob, _T("cbeff"), 0.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory source agency
	lRet = IWSet(pTrans, _T("T99_SRC"), _T("TEST00001"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory biometric creation date
	lRet = IWSet(pTrans, _T("T99_BCD"), _T("20080709145300Z"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory CBEFF header version
	lRet = IWSet(pTrans, _T("T99_HDV"), _T("0101"), 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory biometric type
	lRet = IWSet(pTrans, _T("T99_BTY"), szBiometricType, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory BDB format owner
	lRet = IWSet(pTrans, _T("T99_BFO"), szBDBOwner, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory BDB format type
	lRet = IWSet(pTrans, _T("T99_BFT"), szBDBType, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

BYTE* ReadBLOBFromFile(TCHAR *szFilename, long *plFileSize)
{
	HANDLE	hFile;
	DWORD	cbFile;
	DWORD	cbRead;
	BYTE	*pBuf = NULL;

	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		cbFile = ::GetFileSize(hFile, NULL);
		pBuf = new BYTE [cbFile];
		ReadFile(hFile, pBuf, cbFile, &cbRead, NULL);
		CloseHandle(hFile);
		if (cbRead != cbFile)
		{
			delete [] pBuf;
			pBuf = NULL;
			_tprintf(_T("Failed to read file \"%s\"\n"), szFilename);
		}
		else
		{
			if (plFileSize != NULL) *plFileSize = cbFile;
		}
	}
	else
	{
		_tprintf(_T("Failed to open file \"%s\"\n"), szFilename);
	}

	return pBuf;
}

bool WriteBLOBToFile(TCHAR *szFilename, BYTE* pBlob, long lBlobLen)
{
	HANDLE	hFile;
	DWORD	cbWritten;

	hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("Error: failed to open \"%s\", error %ld\n"), szFilename, GetLastError());
		return false;
	}
	if (!WriteFile(hFile, pBlob, lBlobLen, &cbWritten, NULL) || (cbWritten != (DWORD)lBlobLen))
	{
		_tprintf(_T("Error: failed to write to \"%s\", error %ld\n"), szFilename, GetLastError());
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CheckRet(TCHAR *szContext, long lRetVal)
{
	TCHAR szErrMsg[256];

	if (lRetVal == IW_SUCCESS) return true;

	switch (lRetVal)
	{
		case IW_ERR_LOADING_VERICATION:
			_stprintf_s(szErrMsg, 256, _T("%s: error loading verification file\n"), szContext); break;
		case IW_ERR_OPENING_FILE_FOR_READING:
			_stprintf_s(szErrMsg, 256, _T("%s: error opening file for reading\n"), szContext); break;
		case IW_ERR_OPENING_FILE_FOR_WRITING:
			_stprintf_s(szErrMsg, 256, _T("%s: error opening file for writing\n"), szContext); break;
		case IW_ERR_READING_FILE:
			_stprintf_s(szErrMsg, 256, _T("%s: error reading file\n"), szContext); break;
		case IW_ERR_WRITING_FILE:
			_stprintf_s(szErrMsg, 256, _T("%s: error writing file\n"), szContext); break;
		case IW_ERR_RECORD_NOT_FOUND:
			_stprintf_s(szErrMsg, 256, _T("%s: record not found error\n"), szContext); break;
		case IW_ERR_TRANSACTION_NOT_LOADED:
			_stprintf_s(szErrMsg, 256, _T("%s: transaction not loaded error\n"), szContext); break;
		case IW_ERR_VERIFICATION_NOT_LOADED:
			_stprintf_s(szErrMsg, 256, _T("%s: verification file not loaded error\n"), szContext); break;
		case IW_ERR_NULL_POINTER:
			_stprintf_s(szErrMsg, 256, _T("%s: null pointer error\n"), szContext); break;
		case IW_ERR_NULL_TRANSACTION_POINTER:
			_stprintf_s(szErrMsg, 256, _T("%s: null transaction error\n"), szContext); break;
		case IW_ERR_UNSUPPORTED_IMAGE_FORMAT:
			_stprintf_s(szErrMsg, 256, _T("%s: unsupported image format error\n"), szContext); break;
		case IW_ERR_UNSUPPORTED_RECORD_TYPE:
			_stprintf_s(szErrMsg, 256, _T("%s: unsupported record type\n"), szContext); break;
		case IW_ERR_INDEX_OUT_OF_RANGE:
			_stprintf_s(szErrMsg, 256, _T("%s: index out of range error\n"), szContext); break;
		case IW_ERR_INVALID_SUBFIELD_NUM:
			_stprintf_s(szErrMsg, 256, _T("%s: invalid subfield number error\n"), szContext); break;
		case IW_ERR_MNEMONIC_NOT_FOUND:
			_stprintf_s(szErrMsg, 256, _T("%s: mnemonic not found error\n"), szContext); break;
		case IW_ERR_OUT_OF_MEMORY:
			_stprintf_s(szErrMsg, 256, _T("%s: out of memory error\n"), szContext); break;
		case IW_ERR_WSQ_COMPRESS:
			_stprintf_s(szErrMsg, 256, _T("%s: wsq compression error\n"), szContext); break;
		case IW_ERR_WSQ_DECOMPRESS:
			_stprintf_s(szErrMsg, 256, _T("%s: wsq decompression error\n"), szContext); break;
		case IW_ERR_IMAGE_CONVERSION:
			_stprintf_s(szErrMsg, 256, _T("%s: error converting image\n"), szContext); break;
		default:
			_stprintf_s(szErrMsg, 256, _T("%s: unknown error %ld\n"), szContext, lRetVal); break;
	}

	_tprintf(szErrMsg);

	return false;
}
