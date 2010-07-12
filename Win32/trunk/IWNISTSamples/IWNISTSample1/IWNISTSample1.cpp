// IWNISTSample1.cpp
//

#include "stdafx.h"
#include "OpenEBTS.h"
#include "OpenEBTSErrors.h"
#include "..\Helpers.h"

//
// This sample demonstrates the creation of a sample FBI CAR request, containing
// records of Type-1, Type-2, Type-4, Type-10, Type-14 and Type-16. The contents
// pass through Verification before being written to file.
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
	lRet = IWSet(pTrans, _T("T1_DOM_VER"), _T("1.2"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add Type-2 record and set important fields
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);					if (!CheckRet(_T("IWAddRecord"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RET"), _T("Y"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_NAM"), _T("LISA, MONA J"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_NAM1"), _T("LISA"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_NAM2"), _T("MONA"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_NAM3"), _T("J"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_POB"), _T("IT"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_DOB"), _T("19750620"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_SEX"), _T("F"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RAC"), _T("W"), 1, 1);				if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_HGT"), _T("510"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_WGT"), _T("100"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_EYE"), _T("BRO"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_HAI"), _T("BLK"), 1, 1);			if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_CRI"), _T("DDNMI0039"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_PER_TYPE"), _T("MIO"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_DOA"), _T("20080709"), 1, 1);		if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_ASL_DOO"), _T("20080709"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_ASL_AOL"), _T("Armed Robbery"), 1, 1); if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_ISN"), _T("1234567890123"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	lRet = IWSet(pTrans, _T("T2_RTA_ATT"), _T("Attention Me"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RTA_LINE1"), _T("Blab blah Line 1"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;
	lRet = IWSet(pTrans, _T("T2_RTA_CITY"), _T("Ottawa"), 1, 1);	if (!CheckRet(_T("IWSet"), lRet)) goto done;

	//
	// Add some Type-4 fingerprints and a Type-14 4-finger slap
	//
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, _T("finger1.bmp"), _T("bmp"), _T("wsq"), 0, 2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, _T("finger2.bmp"), _T("bmp"), _T("wsq"), 0, 7)) goto done; //0=livescan, 7=L-index
	if (!AddPrintHelper(pTrans, 14, szFolderSamples, _T("slap.bmp"),   _T("bmp"), _T("wsq"), 0, 13)) goto done; //0=livescan, 13=R-4 fingers

	//
	// Add some Type-10 mugshots
	//
	if (!AddMugshotHelper(pTrans, szFolderSamples, _T("face1.jpg"), _T("jpg"), _T("jpg"))) goto done;
	if (!AddMugshotHelper(pTrans, szFolderSamples, _T("face2.jpg"), _T("jpg"), _T("jpg"))) goto done;

	//
	// Add some Type-16 iris images
	//
	if (!AddType16Helper(pTrans, szFolderSamples, _T("iris1.jpg"), _T("jpg"), _T("jpg"), 1)) goto done; //1=R-eye
	if (!AddType16Helper(pTrans, szFolderSamples, _T("iris2.jpg"), _T("jpg"), _T("jpg"), 2)) goto done; //2=L-eye

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

	// the following lines need to be uncommented to test IWWriteMem()
	//BYTE* pBuffer;
	//int nSize = 0;
	//lRet = IWWriteMem(pTrans, &pBuffer, &nSize);
	//lRet = IWClose(&pTrans);
	//lRet = IWReadMem(pBuffer, nSize, pVer, &pTrans);
	//lRet = IWMemFree(&pBuffer);

	_tcscpy_s(szFilename, _MAX_PATH, szFolderSamples);
	_tcscat_s(szFilename, _MAX_PATH, _T("out1.ebts"));
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
