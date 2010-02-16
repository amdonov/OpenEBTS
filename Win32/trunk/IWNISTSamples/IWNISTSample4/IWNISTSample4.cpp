// IWNISTSample4.cpp
//

#include "stdafx.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "..\Helpers.h"

//
// This sample demonstrates the image format capabilities of IWNIST by creating an EBTS file
// containing images of all possible formats and subsequently re-opening it an exporting all
// the images within to disk to offer the greatest possibiliy to verify the successful
// functioning.
//

int _tmain(int argc, TCHAR* argv[])
{
	CIWTransaction	*pTrans = NULL;
	CIWVerification *pVer = NULL;
	long			lRet;
	TCHAR			szParseError[IW_MAX_PARSE_ERROR];
	int				iIndex;
	int				iNumErrors;
	int				iErr;
	int				nErrCode;
	const TCHAR		*szErrDesc;
	TCHAR			szDrive[_MAX_DRIVE];
	TCHAR			szDir[_MAX_DIR];
	TCHAR			szFilename[_MAX_PATH];
	TCHAR			szFolderSamples[_MAX_PATH];

	// Find Samples folder relative to EXE folder
	GetModuleFileName(NULL, szFilename, _MAX_PATH);
	_tsplitpath_s(szFilename, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
	_tcscpy_s(szFolderSamples, _MAX_PATH, szDrive);
	_tcscat_s(szFolderSamples, _MAX_PATH, szDir);
	_tcscat_s(szFolderSamples, _MAX_PATH, _T("..\\samples\\"));

	// Read verification file
	_tcscpy_s(szFilename, _MAX_PATH, szFolderSamples);
	_tcscat_s(szFilename, _MAX_PATH, _T("ebts1_2_extra.txt"));
	lRet = IWReadVerification(szFilename, &pVer, IW_MAX_PARSE_ERROR, szParseError);
	if (!CheckRet(_T("IWReadVerification"), lRet))
	{
		_tprintf(_T("Error parsing verification file '%s': %s\n"), szFilename, szParseError);
		goto done;
	}

	// Create new transaction, with verification file loaded to enable mnemonic
	// usage for field referral. In this sample we create a transaction of type
	// DPRS: DOD Flat-Print Rap Sheet Search
	lRet = IWNew(_T("IWTST"), pVer, &pTrans);
	if (!CheckRet(_T("IWNew"), lRet)) goto done;

	//
	// Set Type-1 fields. We don't need to add a Type-1 record, all EBTS files have one and exactly one.
	//
	lRet = IWSet(pTrans, _T("T1_DAT"), _T("20080709"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_PRY"), _T("4"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_DAI"), _T("TEST00001"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_ORI"), _T("TEST00001"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_TCN"), _T("TEST-20080528173758-SOFT-0001-1C629"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_NSR"), _T("19.69"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_NTR"), _T("19.69"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add Type-2 record, we don't need to add anything for this TOT
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);	if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	//
	// Add Type-13 images using all possible input and output formats
	//
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger1.bmp"), _T("bmp"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger1.bmp"), _T("bmp"), _T("fx4"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.bmp"), _T("bmp"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.bmp"), _T("bmp"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.bmp"), _T("bmp"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.bmp"), _T("bmp"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.bmp"), _T("bmp"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.bmp"), _T("bmp"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.bmp"), _T("bmp"), _T("jp2"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jpg"), _T("jpg"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jpg"), _T("jpg"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jpg"), _T("jpg"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jpg"), _T("jpg"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.jpg"), _T("jpg"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.jpg"), _T("jpg"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.jpg"), _T("jpg"), _T("jp2"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jp2"), _T("jp2"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jp2"), _T("jp2"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jp2"), _T("jp2"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.jp2"), _T("jp2"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.jp2"), _T("jp2"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.jp2"), _T("jp2"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger24.jp2"), _T("jp2"), _T("jp2"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.wsq"), _T("wsq"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.wsq"), _T("wsq"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.wsq"), _T("wsq"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger8.wsq"), _T("wsq"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger1.fx4"), _T("fx4"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger1.fx4"), _T("fx4"), _T("fx4"), 0, 0)) goto done;

	//
	// Verify entire transaction file
	//
	lRet = IWVerify(pTrans);
	if (lRet == IW_WARN_TRANSACTION_FAILED_VERIFICATION)
	{
		// Output all warnings
		iNumErrors = IWGetErrorCount(pTrans);
		for (iErr = 0; iErr < iNumErrors; iErr++)
		{
			lRet = IWGetError(pTrans, iErr, &nErrCode, &szErrDesc);
			if (!CheckRet(_T("IWGetError"), lRet)) goto done;
			_tprintf(_T("Verification warning: %s (#%d)\n"), szErrDesc, nErrCode);
		}
		// NOTE: We still write out the file if verification fails
		// because of bugs in the verification schema (e.g. tenprint image
		// field 14.999 not defined).
		//goto done;	// No point writing the file if it isn't right
	}
	else if (!CheckRet(_T("IWVerify"), lRet))
	{
		goto done;
	}

	// Write it
	_tcscpy_s(szFilename, _MAX_PATH, szFolderSamples);
	_tcscat_s(szFilename, _MAX_PATH, _T("out4.ebts"));
	lRet = IWWrite(pTrans, szFilename);
	if (!CheckRet(_T("IWWrite"), lRet)) goto done;

	_tprintf(_T("Successful creation of %s\n"), szFilename);

	// Close-off transaction
	IWClose(&pTrans);
	pTrans = NULL;

	// Now let's read the file back and save out the images
	lRet = IWRead(szFilename, pVer, &pTrans);
	if (!CheckRet(_T("IWRead"), lRet)) goto done;

	// Save out the files as they are
	int			nRecs;
	const TCHAR	*szFmt;
	long		lLength;
	long		lH;
	long		lV;
	int			iBpp;
	const void *pData;
	TCHAR		szFile[_MAX_FNAME];

	lRet = IWGetRecordTypeCount(pTrans, 13, &nRecs);
	if (!CheckRet(_T("IWGetRecordTypeCount"), lRet)) goto done;
	for (int i=1; i <= nRecs; i++)
	{
		// Call IWGetImageInfo just to get bpp
		lRet = IWGetImageInfo(pTrans, 13, i, &szFmt, &lLength, &lH, &lV, &iBpp);
		if (!CheckRet(_T("IWGetImageInfo"), lRet)) goto done;

		// Get the image bits
		lRet = IWGetImage(pTrans, 13, i, &szFmt, &lLength, &pData);
		if (!CheckRet(_T("IWGetImage"), lRet)) goto done;

		_stprintf_s(szFile, _MAX_FNAME, _T("out4_%02d_%dx%d_%dbpp.%s"), i, lH, lV, iBpp, szFmt);
		_tcscpy_s(szFilename, _MAX_PATH, szFolderSamples);
		_tcscat_s(szFilename, _MAX_PATH, szFile);

		WriteBLOBToFile(szFilename, (BYTE*)pData, lLength);
	}

	_tprintf(_T("Successful extraction of images within c:\\out4.ebts\n"));

done:
	if (pTrans != NULL)
	{
		IWClose(&pTrans);
		pTrans = NULL;
	}

	if (pVer != NULL)
	{
		IWCloseVerification(&pVer);
		pVer = NULL;
	}

	_tprintf(_T("\nPress <ENTER> to exit."));
	getchar();

	return 0;
}
