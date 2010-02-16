// IWNISTSample3.cpp
//

#include "stdafx.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "..\Helpers.h"

//
// This sample demonstrates the creation of a sample based on an _T("invented")
// IWTST type inserted into ebts1_2.txt (ebts1_2_extra.txt). The changes in
// this verification file consist of the addition of Type-13, Type-15 and
// Type-17 definition.
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
	lRet = IWAddRecord(pTrans, 2, &iIndex);					if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;

	//
	// Add a Type-8 signature
	//
	if (!AddSignatureHelper(pTrans, szFolderSamples, _T("signature.jpg"), _T("jpg"))) goto done;

	//
	// Add some fingerprints of various types, 13, 14 and 15
	//
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, _T("finger1.wsq"), _T("wsq"), _T("wsq"), 0,  2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans, 14, szFolderSamples, _T("finger2.wsq"), _T("wsq"), _T("wsq"), 0,  2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans, 15, szFolderSamples, _T("slap.wsq"), _T("wsq"), _T("wsq"), 0, 30)) goto done; //0=livescan,

	//
	// Add a couple of Type-17 irises
	//
	if (!AddIrisHelper(pTrans, szFolderSamples, _T("iris1.jpg"), _T("jpg"), 1)) goto done; //1=R-eye
	if (!AddIrisHelper(pTrans, szFolderSamples, _T("iris2.jpg"), _T("jpg"), 2)) goto done; //2=L-eye

	//
	// Add a Type-99 CBEFF
	//
	if (!AddCBEFFHelper(pTrans, szFolderSamples, _T("cbeff.bin"), _T("00000008"), _T("001B"), _T("0201"))) goto done;

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
	_tcscat_s(szFilename, _MAX_PATH, _T("out3.ebts"));
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
