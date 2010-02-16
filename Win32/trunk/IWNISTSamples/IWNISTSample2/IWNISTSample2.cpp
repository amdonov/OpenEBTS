// IWNISTSample2.cpp
//

#include "stdafx.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "..\Helpers.h"

//
// This sample demonstrates the creation of a sample FBI LFIS request containing
// records of Type-1, Type-2, Type-4, and Type-7. The contents pass through
// Verification before being written to file.
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
	_tcscat_s(szFilename, _MAX_PATH, _T("ebts1_2.txt"));
	lRet = IWReadVerification(szFilename, &pVer, IW_MAX_PARSE_ERROR, szParseError);
	if (!CheckRet(_T("IWReadVerification"), lRet))
	{
		_tprintf(_T("Error parsing verification file '%s': %s\n"), szFilename, szParseError);
		goto done;
	}

	// Create new transaction, with verification file loaded to enable mnemonic
	// usage for field referral. In this sample we create a transaction of type LFIS
	lRet = IWNew(_T("LFIS"), pVer, &pTrans);
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
	lRet = IWSet(pTrans, _T("T1_DOM_IMP"), _T("EBTSNAVY"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T1_DOM_VER"), _T("1.2"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add Type-2 record and set important fields
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);					if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_ATN"), _T("DET J Q PUBLIC"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_CIX"), _T("42"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_HTR"), _T("400711"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_GEO"), _T("AK"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_PRI"), _T("3"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_LATENT_TECN"), _T("DOE, JOHN"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_LATENT_SUB"), _T("SMITH, JOE"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_CIN_PRE"), _T("12345"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_CIN_ID"), _T("12345"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_IMA_MAK"), _T("Make"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_IMA_MAK"), _T("Make"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_IMA_MODL"), _T("Model"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_IMA_SERNO"), _T("SerialNumber"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RCD2_FGP"), _T("02"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RCD2_RCN2"), _T("10"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add some fingerprints of various types
	//
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, _T("finger1.bmp"), _T("bmp"), _T("wsq"), 0, 2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans,  7, szFolderSamples, _T("finger2.bmp"), _T("bmp"), _T("wsq"), 0, 7)) goto done; //0=livescan, 7=L-index

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
		goto done;	// No point writing the file if it isn't right
	}
	else if (!CheckRet(_T("IWVerify"), lRet))
	{
		goto done;
	}

	// Write it
	_tcscpy_s(szFilename, _MAX_PATH, szFolderSamples);
	_tcscat_s(szFilename, _MAX_PATH, _T("out2.ebts"));
	lRet = IWWrite(pTrans, szFilename);
	if (!CheckRet(_T("IWWrite"), lRet)) goto done;

	_tprintf(_T("Successful creation of %s\n"), szFilename);

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
