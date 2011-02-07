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
// This sample demonstrates the creation of a sample FBI CAR request, containing
// records of Type-1, Type-2, Type-4, Type-10, Type-14 and Type-16. The contents
// pass through Verification before being written to file.
//

int main()
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
	_tcscpypath(szFilename, szFolderSamples);
	_tcscatpath(szFilename, _TPATH("/EBTS9.1_ENUS.txt"));

	lRet = IWReadVerification(szFilename, &pVer, IW_MAX_PARSE_ERROR, szParseError);
	if (!CheckRet(_T("IWReadVerification"), lRet))
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

	// Create new transaction, with verification file loaded to enable mnemonic
	// usage for field referral. In this sample we create a transaction of type CAR
	lRet = IWNew(_T("CAR"), pVer, &pTrans);
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
	lRet = IWSet(pTrans, _T("T1_DOM_IMP"), _T("EBTSTEST"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_DOM_VER"), _T("NORAM"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add Type-2 record and set important fields
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);							if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RET"), _T("Y"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_NAM"), _T("LISA, MONA J"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_POB"), _T("IT"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_DOB"), _T("19750620"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_SEX"), _T("F"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RAC"), _T("W"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_HGT"), _T("510"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_WGT"), _T("100"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_EYE"), _T("BRO"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_HAI"), _T("BLK"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_CRI"), _T("DDNMI0039"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_DOA"), _T("20080709"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_ASL_DOO"), _T("20080709"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_ASL_AOL"), _T("Armed Robbery"), 1, 1); if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add a Type-4 fingerprint and a Type-14 4-finger slap
	//
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, _TPATH("finger8.bmp"), _T("bmp"), _T("wsq"), 0, 2)) goto done;	 //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans, 14, szFolderSamples, _TPATH("slap8.png"),   _T("png"), _T("wsq"), 0, 13)) goto done; //0=livescan, 13=R-4 fingers

	//
	// Add a Type-10 mugshot
	//
	if (!AddMugshotHelper(pTrans, szFolderSamples, _TPATH("face24.jpg"), _T("jpg"), _T("jpg"))) goto done;

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
	else if (!CheckRet(_T("IWVerify"), lRet))
	{
		goto done;
	}

	// Write it
	_tcscpypath(szFilename, szFolderSamples);
	_tcscatpath(szFilename, _TPATH("/OpenEBTSSample1_out.ebts"));
	lRet = IWWrite(pTrans, szFilename);
	if (!CheckRet(_T("IWWrite"), lRet)) goto done;

	_tprintf(_T("Successful creation of OpenEBTSSample1_out.ebts\n"));

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

	_tprintf(_T("OpenEBTSSample1 has completed.\n"));

	return 0;
}
