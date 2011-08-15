#include "Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <limits.h>
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "Helpers.h"


bool AddPrintHelper(CIWTransaction *pTrans, long lType, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmtIn, const TCHAR *szFmtOut, int iImpressionType, int iFingerIndex)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHAR		szMNUImp[10];
	TCHAR		szMNUFgp[10];
	TCHARPATH	szFilename[_MAX_PATH];

	_tcscpypath(szFilename, szFolder);
	_tcscatpath(szFilename, _TPATH("/"));
	_tcscatpath(szFilename, szFile);

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

	// Set image data (let OpenEBTS convert the input to WSQ/JPG/JP2 with compression factor of 15)
	lRet = IWSetImage(pTrans, lType, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 15.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory impression type
	_sntprintf(szNum, 10, _T("%d"), iImpressionType);
	switch (lType)
	{
		case 3:  _tcscpy(szMNUImp, _T("T3_IMP")); break;
		case 4:  _tcscpy(szMNUImp, _T("T4_IMP")); break;
		case 5:  _tcscpy(szMNUImp, _T("T5_IMP")); break;
		case 6:  _tcscpy(szMNUImp, _T("T6_IMP")); break;
		case 7:  _tcscpy(szMNUImp, _T("T7_IMP")); break;
		case 13: _tcscpy(szMNUImp, _T("T13_IMP")); break;
		case 14: _tcscpy(szMNUImp, _T("T14_IMP")); break;
		case 15: _tcscpy(szMNUImp, _T("T15_IMP")); break;
		default: szMNUImp[0] = '\0';
	}
	lRet = IWSet(pTrans, szMNUImp, szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory NIST fingerprint index (or palmprint position for Type-15)
	_sntprintf(szNum, 10, lType == 15 ? _T("%03d") : _T("%d"), iFingerIndex);
	switch (lType)
	{
		case 3:  _tcscpy(szMNUFgp, _T("T3_FGP")); break;
		case 4:  _tcscpy(szMNUFgp, _T("T4_FGP")); break;
		case 5:  _tcscpy(szMNUFgp, _T("T5_FGP")); break;
		case 6:  _tcscpy(szMNUFgp, _T("T6_FGP")); break;
		case 7:  _tcscpy(szMNUFgp, _T("T7_FGP")); break;
		case 13: _tcscpy(szMNUFgp, _T("T13_FGP")); break;
		case 14: _tcscpy(szMNUFgp, _T("T14_FGP")); break;
		case 15: _tcscpy(szMNUFgp, _T("T15_PLP")); break;
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
		lRet = IWSet(pTrans, _T("T14_TCD"), _T("20080709"), 1, iIndex);
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

bool AddMinutiaeHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, int iImpressionType, int iFingerIndex)
//TODO: not sure how to proceed
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHARPATH	szFilename[_MAX_PATH];

	_tcscpypath(szFilename, szFolder);
	_tcscatpath(szFilename, _TPATH("/"));
	_tcscatpath(szFilename, szFile);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	// Add Type-9 record and retrieve new index
	lRet = IWAddRecord(pTrans, 9, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set mandatory impression type
	_sntprintf(szNum, 10, _T("%d"), iImpressionType);
	lRet = IWSet(pTrans, _T("T9_IMP"), szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	// Set mandatory NIST fingerprint index
	_sntprintf(szNum, 10, _T("%02d"), iFingerIndex);
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

bool AddMugshotHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmtIn, const TCHAR *szFmtOut)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHARPATH	szFilename[_MAX_PATH];

	_tcscpypath(szFilename, szFolder);
	_tcscatpath(szFilename, _TPATH("/"));
	_tcscatpath(szFilename, szFile);

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

bool AddSignatureHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmt)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHARPATH	szFilename[_MAX_PATH];

	_tcscpypath(szFilename, szFolder);
	_tcscatpath(szFilename, _TPATH("/"));
	_tcscatpath(szFilename, szFile);

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

bool AddType16Helper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmtIn, const TCHAR *szFmtOut, int iFeature)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHARPATH		szFilename[_MAX_PATH];

	_tcscpypath(szFilename, szFolder);
	_tcscatpath(szFilename, _TPATH("/"));
	_tcscatpath(szFilename, szFile);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 16, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 16, iIndex, szFmtIn, lSize, pBlob, szFmtOut, 90.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory feature identifier
	_sntprintf(szNum, 10, _T("%d"), iFeature);
	lRet = IWSet(pTrans, _T("T16_FID"), szNum, 1, iIndex);
	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	bRet = true;

done:
	if (pBlob) delete [] pBlob;

	return bRet;
}

bool AddIrisHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmt, int iEye)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHAR		szNum[10];
	TCHARPATH	szFilename[_MAX_PATH];

	_tcscpypath(szFilename, szFolder);
	_tcscatpath(szFilename, _TPATH("/"));
	_tcscatpath(szFilename, szFile);

	pBlob = ReadBLOBFromFile(szFilename, &lSize);
	if (!pBlob) goto done;

	lRet = IWAddRecord(pTrans, 17, &iIndex);
	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	// Set image data
	lRet = IWSetImage(pTrans, 17, iIndex, szFmt, lSize, pBlob, _T("raw"), 0.0f);
	if (!CheckRet(_T("IWSetImage"), lRet)) goto done;

	// Set mandatory feature identifier
	_sntprintf(szNum, 10, _T("%d"), iEye);
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

bool AddCBEFFHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szBiometricType, const TCHAR *szBDBOwner, const TCHAR *szBDBType)
{
	bool		bRet = false;
	long		lRet;
	int			iIndex;
	BYTE		*pBlob = NULL;
	long		lSize;
	TCHARPATH	szFilename[_MAX_PATH];

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

BYTE* ReadBLOBFromFile(const TCHARPATH *szFilename, long *plFileSize)
{
	FILE	*f;
	size_t	cbFile;
	size_t	cbRead;
	BYTE	*pBuf = NULL;

	f = _tfopenpath(szFilename, _TPATH("rb"));
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		cbFile = ftell(f);
		fseek(f, 0, SEEK_SET);

		pBuf = new BYTE [cbFile];

		cbRead = fread(pBuf, 1, cbFile, f);
		fclose(f);

		if (cbRead != cbFile)
		{
			delete [] pBuf;
			pBuf = NULL;
			_tprintf(_T("Failed to read file \"%s\"\n"), szFilename);
		}
		else
		{
			if (plFileSize != NULL) *plFileSize = (long)cbFile;
		}
	}
	else
	{
		_tprintf(_T("Failed to open file \"%s\"\n"), szFilename);
	}

	return pBuf;
}

bool WriteBLOBToFile(const TCHARPATH *szFilename, BYTE* pBlob, long lBlobLen)
{
	bool	bRet = false;
	FILE	*f;
	size_t	cbWritten;

	f = _tfopenpath(szFilename, _TPATH("wb"));
	if (f != NULL)
	{
		cbWritten = fwrite(pBlob, 1, lBlobLen, f);
		fclose(f);

		if ((long)cbWritten == lBlobLen)
		{
			bRet = true;
		}
		else
		{
			_tprintf(_T("Failed to write \"%s\"\n"), szFilename);
		}
	}
	else
	{
		_tprintf(_T("Error: failed to open \"%s\", error %d\n"), szFilename, errno);
	}

	return bRet;
}

bool CheckRet(const TCHAR *szContext, long lRetVal)
{
	TCHAR szErrFmt[256];

	if (lRetVal == IW_SUCCESS) return true;

#ifdef UNICODE
	_tcsncpy(szErrFmt, _T("%ls: "), 256);
#else
	_tcsncpy(szErrFmt, _T("%s: "), 256);
#endif

	switch (lRetVal)
	{
		case IW_ERR_LOADING_VERICATION:
			_tcsncat(szErrFmt, _T("error loading verification file\n"), 256); break;
		case IW_ERR_OPENING_FILE_FOR_READING:
			_tcsncat(szErrFmt, _T("error opening file for reading\n"), 256); break;
		case IW_ERR_OPENING_FILE_FOR_WRITING:
			_tcsncat(szErrFmt, _T("error opening file for writing\n"), 256); break;
		case IW_ERR_READING_FILE:
			_tcsncat(szErrFmt, _T("error reading file\n"), 256); break;
		case IW_ERR_WRITING_FILE:
			_tcsncat(szErrFmt, _T("error writing file\n"), 256); break;
		case IW_ERR_RECORD_NOT_FOUND:
			_tcsncat(szErrFmt, _T("record not found error\n"), 256); break;
		case IW_ERR_TRANSACTION_NOT_LOADED:
			_tcsncat(szErrFmt, _T("transaction not loaded error\n"), 256); break;
		case IW_ERR_VERIFICATION_NOT_LOADED:
			_tcsncat(szErrFmt, _T("verification file not loaded error\n"), 256); break;
		case IW_ERR_NULL_POINTER:
			_tcsncat(szErrFmt, _T("null pointer error\n"), 256); break;
		case IW_ERR_NULL_TRANSACTION_POINTER:
			_tcsncat(szErrFmt, _T("null transaction error\n"), 256); break;
		case IW_ERR_UNSUPPORTED_IMAGE_FORMAT:
			_tcsncat(szErrFmt, _T("unsupported image format error\n"), 256); break;
		case IW_ERR_UNSUPPORTED_RECORD_TYPE:
			_tcsncat(szErrFmt, _T("unsupported record type\n"), 256); break;
		case IW_ERR_INDEX_OUT_OF_RANGE:
			_tcsncat(szErrFmt, _T("index out of range error\n"), 256); break;
		case IW_ERR_INVALID_SUBFIELD_NUM:
			_tcsncat(szErrFmt, _T("invalid subfield number error\n"), 256); break;
		case IW_ERR_MNEMONIC_NOT_FOUND:
			_tcsncat(szErrFmt, _T("mnemonic not found error\n"), 256); break;
		case IW_ERR_OUT_OF_MEMORY:
			_tcsncat(szErrFmt, _T("out of memory error\n"), 256); break;
		case IW_ERR_WSQ_COMPRESS:
			_tcsncat(szErrFmt, _T("wsq compression error\n"), 256); break;
		case IW_ERR_WSQ_DECOMPRESS:
			_tcsncat(szErrFmt, _T("wsq decompression error\n"), 256); break;
		case IW_ERR_IMAGE_CONVERSION:
			_tcsncat(szErrFmt, _T("error converting image\n"), 256); break;
		default:
			_tprintf(_T("Unknown error %ld - "), lRetVal);
	}

	_tprintf(szErrFmt, szContext);

	return false;
}
