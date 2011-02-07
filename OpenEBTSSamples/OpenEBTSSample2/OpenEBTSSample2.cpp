// OpenEBTSSample1.cpp
//

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

//
// This sample demonstrates the image format capabilities of OpenEBTS by creating an EBTS file
// containing images of all possible formats and subsequently re-opening it an exporting all
// the images within the EBTS file to disk to test the successful functioning.
//

int main()
{
	CIWTransaction	*pTrans = NULL;
	CIWVerification *pVer = NULL;
	int				iRet;
	TCHAR			szParseError[IW_MAX_PARSE_ERROR];
	int				iIndex;
	int				iNumErrors;
	int				iErr;
	int				nErrCode;
	const TCHAR		*szErrDesc;
	TCHARPATH		szFilename[_MAX_PATH];
	TCHARPATH		*szFolderSamples;

	// Get the environment variable for the 'samples' folder
	szFolderSamples = _tgetenvpath(_TPATH("OPENEBTSSAMPLESFOLDER"));
	if (szFolderSamples == NULL)
	{
		_tprintf(_T("The environment variable OPENEBTSSAMPLESFOLDER must be set to the folder containing the OpenEBTS samples.\n"));
		goto done;
	}

	// Read verification file
	// We use the modified version "EBTS9 .1_ENUS_ImageTest.txt" that allows for having only
	// many images as Type-13 records, using the fictional TOT "TEST".
	_tcscpypath(szFilename, szFolderSamples);
	_tcscatpath(szFilename, _TPATH("/EBTS9.1_ENUS_ImageTest.txt"));

	iRet = IWReadVerification(szFilename, &pVer, IW_MAX_PARSE_ERROR, szParseError);
	if (!CheckRet(_T("IWReadVerification"), iRet))
	{
		if (_tcslen(szParseError))
		{
#ifdef UNICODE
			_tprintf(_T("Error parsing verification file '%s': %ls\n"), szFilename, szParseError);
#else
			_tprintf(_T("Error parsing verification file '%s': %s\n"), szFilename, szParseError);
#endif
		}
		goto done;
	}

	iRet = IWNew(_T("TEST"), pVer, &pTrans);
	if (!CheckRet(_T("IWNew"), iRet)) goto done;

	//
	// Set Type-1 fields. We don't need to add a Type-1 record, all EBTS files have one and exactly one.
	//
	iRet = IWSet(pTrans, _T("T1_DAT"), _T("20080709"), 1, 1);		if (!CheckRet(_T("IWSet"), iRet)) goto done;
	iRet = IWSet(pTrans, _T("T1_PRY"), _T("4"), 1, 1);				if (!CheckRet(_T("IWSet"), iRet)) goto done;
	iRet = IWSet(pTrans, _T("T1_DAI"), _T("TEST00001"), 1, 1);		if (!CheckRet(_T("IWSet"), iRet)) goto done;
	iRet = IWSet(pTrans, _T("T1_ORI"), _T("TEST00001"), 1, 1);		if (!CheckRet(_T("IWSet"), iRet)) goto done;
	iRet = IWSet(pTrans, _T("T1_TCN"), _T("TEST-20080528173758-SOFT-0001-1C629"), 1, 1);	if (!CheckRet(_T("IWSet"), iRet)) goto done;
	iRet = IWSet(pTrans, _T("T1_NSR"), _T("19.69"), 1, 1);			if (!CheckRet(_T("IWSet"), iRet)) goto done;
	iRet = IWSet(pTrans, _T("T1_NTR"), _T("19.69"), 1, 1);			if (!CheckRet(_T("IWSet"), iRet)) goto done;

	//
	// Add Type-2 record, we don't need to add anything for this TOT
	//
	iRet = IWAddRecord(pTrans, 2, &iIndex);	if (!CheckRet(_T("IWAddRecord"), iRet)) goto done;

	//
	// Add Type-13 images using all possible input and output formats
	//
	// 1 bit per pixel formats

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.bmp"), _T("bmp"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.bmp"), _T("bmp"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.bmp"), _T("bmp"), _T("fx4"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.png"), _T("png"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.png"), _T("png"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.png"), _T("png"), _T("fx4"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.fx4"), _T("fx4"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.fx4"), _T("fx4"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger1.fx4"), _T("fx4"), _T("fx4"), 0, 0)) goto done;

	// 8 bits per pixel formats

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.bmp"), _T("bmp"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.bmp"), _T("bmp"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.bmp"), _T("bmp"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.bmp"), _T("bmp"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.bmp"), _T("bmp"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jpg"), _T("jpg"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jpg"), _T("jpg"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jpg"), _T("jpg"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jpg"), _T("jpg"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jpg"), _T("jpg"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jp2"), _T("jp2"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jp2"), _T("jp2"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jp2"), _T("jp2"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jp2"), _T("jp2"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.jp2"), _T("jp2"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.png"), _T("png"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.png"), _T("png"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.png"), _T("png"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.png"), _T("png"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.png"), _T("png"), _T("wsq"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.wsq"), _T("wsq"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.wsq"), _T("wsq"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.wsq"), _T("wsq"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.wsq"), _T("wsq"), _T("png"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger8.wsq"), _T("wsq"), _T("wsq"), 0, 0)) goto done;

	// 24 bits per pixel formats

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.bmp"), _T("bmp"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.bmp"), _T("bmp"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.bmp"), _T("bmp"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.bmp"), _T("bmp"), _T("png"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jpg"), _T("jpg"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jpg"), _T("jpg"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jpg"), _T("jpg"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jpg"), _T("jpg"), _T("png"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jp2"), _T("jp2"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jp2"), _T("jp2"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jp2"), _T("jp2"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.jp2"), _T("jp2"), _T("png"), 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.png"), _T("png"), _T("raw"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.png"), _T("png"), _T("jpg"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.png"), _T("png"), _T("jp2"), 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _TPATH("finger24.png"), _T("png"), _T("png"), 0, 0)) goto done;

	//
	// Verify entire transaction file
	//
	iRet = IWVerify(pTrans);
	if (iRet == IW_WARN_TRANSACTION_FAILED_VERIFICATION)
	{
		// Output all warnings
		iNumErrors = IWGetErrorCount(pTrans);
		for (iErr = 0; iErr < iNumErrors; iErr++)
		{
			iRet = IWGetError(pTrans, iErr, &nErrCode, &szErrDesc);
			if (!CheckRet(_T("IWGetError"), iRet)) goto done;

#ifdef UNICODE
			_tprintf(_T("Verification warning: %ls (#%d)\n"), szErrDesc, nErrCode);
#else
			_tprintf(_T("Verification warning: %s (#%d)\n"), szErrDesc, nErrCode);
#endif
		}
		// NOTE: We still write out the file if verification fails
		// because it might be interesting to analyze
		_tprintf(_T("Proceeding with writing file despite warnings.\n"));
	}
	else if (!CheckRet(_T("IWVerify"), iRet))
	{
		goto done;
	}

	// Write it
	_tcscpypath(szFilename, szFolderSamples);
	_tcscatpath(szFilename, _TPATH("/OpenEBTSSample2_out.ebts"));
	iRet = IWWrite(pTrans, szFilename);
	if (!CheckRet(_T("IWWrite"), iRet)) goto done;

	_tprintf(_T("Successful creation of OpenEBTSSample2_out.ebts\n"));

	// Close-off transaction
	IWClose(&pTrans);
	pTrans = NULL;

	// Now let's read the file back and save out the images
	iRet = IWRead(szFilename, pVer, &pTrans);
	if (!CheckRet(_T("IWRead"), iRet)) goto done;

	// Save out the files as they are
	int			nRecs;
	const TCHAR	*szFmt;
	int			cbSize;
	int			width;
	int			heigth;
	int			bpp;
	const void	*pData;
	TCHARPATH	szFile[_MAX_PATH];
	TCHARPATH	szFmtPath[10];

	iRet = IWGetRecordTypeCount(pTrans, 13, &nRecs);
	if (!CheckRet(_T("IWGetRecordTypeCount"), iRet)) goto done;
	for (int i=1; i <= nRecs; i++)
	{
		// Call IWGetImageInfo just to get dimensions and bpp
		iRet = IWGetImageInfo(pTrans, 13, i, &szFmt, &cbSize, &width, &heigth, &bpp);
		if (!CheckRet(_T("IWGetImageInfo"), iRet)) goto done;

		// Get the image bits
		iRet = IWGetImage(pTrans, 13, i, &szFmt, &cbSize, &pData);
		if (!CheckRet(_T("IWGetImage"), iRet)) goto done;

		// Only in *nix, in the UNICODE version of this sample, do we need to convert the wide format
		// into an mbcs string
#if defined(UNICODE) && !defined(WIN32)
		wcstombs(szFmtPath, szFmt, 10);
#else
		_tcscpy(szFmtPath, szFmt);
#endif

		_sntprintfpath(szFile, _MAX_PATH, _TPATH("//OpenEBTSSample2_out_%02d_%dx%d_%dbpp.%s"), i, width, heigth, bpp, szFmtPath);
		_tcscpypath(szFilename, szFolderSamples);
		_tcscatpath(szFilename, szFile);

		WriteBLOBToFile(szFilename, (BYTE*)pData, cbSize);
	}

	_tprintf(_T("Successful extraction of images.\n"));

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

	_tprintf(_T("OpenEBTSSample2 has completed.\n"));

	return 0;
}
