// IWNISTSample3.cpp
//

#include "stdafx.h"
#include "IWNIST.h"
#include "IWNISTErrors.h"
#include "..\Helpers.h"

//
// This sample demonstrates the creation of a sample based on an "invented"
// IWTST type inserted into ebts1_2.txt (ebts1_2_extra.txt). The changes in
// this verification file consist of the addition of Type-13, Type-15 and
// Type-17 definition.
//

int _tmain(int argc, _TCHAR* argv[])
{
	CIWTransaction	*pTrans = NULL;
	CIWVerification *pVer = NULL;
	long			lRet;
	char			szParseError[IW_MAX_PARSE_ERROR];
	int				iIndex;
	int				iNumErrors;
	int				iErr;
	int				nErrCode;
	const char		*szErrDesc;
	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFilename[_MAX_PATH];
	char			szFolderSamples[_MAX_PATH];

	// Find Samples folder relative to EXE folder
	GetModuleFileName(NULL, szFilename, _MAX_PATH);
	_splitpath_s(szFilename, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
	strcpy_s(szFolderSamples, _MAX_PATH, szDrive);
	strcat_s(szFolderSamples, _MAX_PATH, szDir);
	strcat_s(szFolderSamples, _MAX_PATH, "..\\samples\\");

	// Read verification file
	strcpy_s(szFilename, _MAX_PATH, szFolderSamples);
	strcat_s(szFilename, _MAX_PATH, "ebts1_2_extra.txt");
	lRet = IWReadVerification(szFilename, &pVer, IW_MAX_PARSE_ERROR, szParseError);
	if (!CheckRet("IWReadVerification", lRet)) goto done;

	// Create new transaction, with verification file loaded to enable mnemonic
	// usage for field referral. In this sample we create a transaction of type
	// DPRS: DOD Flat-Print Rap Sheet Search
	lRet = IWNew("IWTST", pVer, &pTrans);
	if (!CheckRet("IWNew", lRet)) goto done;

	//
	// Set Type-1 fields. We don't need to add a Type-1 record, all EBTS files have one and exactly one.
	//
	lRet = IWSet(pTrans, "T1_DAT", "20080709", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_PRY", "4", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_DAI", "TEST00001", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_ORI", "TEST00001", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_TCN", "TEST-20080528173758-SOFT-0001-1C629", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_NSR", "19.69", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_NTR", "19.69", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;

	//
	// Add Type-2 record, we don't need to add anything for this TOT
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);					if (!CheckRet("IWAddRecord", lRet)) goto done;

	//
	// Add a Type-8 signature
	//
	if (!AddSignatureHelper(pTrans, szFolderSamples, "signature.jpg", "jpg")) goto done;

	//
	// Add some fingerprints of various types, 13, 14 and 15
	//
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger1.wsq", "wsq", "wsq", 0,  2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans, 14, szFolderSamples, "finger2.wsq", "wsq", "wsq", 0,  2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans, 15, szFolderSamples, "slap.wsq", "wsq", "wsq", 0, 30)) goto done; //0=livescan,

	//
	// Add a Type-17 iris
	//
	if (!AddIrisHelper(pTrans, szFolderSamples, "iris1.jpg", "jpg", 1)) goto done; //1=R-eye
	if (!AddIrisHelper(pTrans, szFolderSamples, "iris2.jpg", "jpg", 2)) goto done; //2=L-eye

	//
	// Add a Type-99 CBEFF
	//
	if (!AddCBEFFHelper(pTrans, szFolderSamples, "cbeff.bin", "00000008", "001B", "0201")) goto done;

	//
	// Verify entire transaction file
	//
	lRet = IWVerify(pTrans);
	if (lRet == IW_WARN_TRANSACTION_FAILED_VERIFICATION) {
		// Output all warnings
		iNumErrors = IWGetErrorCount(pTrans);
		for (iErr = 0; iErr < iNumErrors; iErr++) {
			lRet = IWGetError(pTrans, iErr, &nErrCode, &szErrDesc);
			if (!CheckRet("IWGetError", lRet)) goto done;
			printf("Verification warning: %s (#%d)\n", szErrDesc, nErrCode);
		}
		// NOTE: We still write out the file if verification fails
		// because of bugs in the verification schema (e.g. tenprint image
		// field 14.999 not defined).
		//goto done;	// No point writing the file if it isn't right
	} else if (!CheckRet("IWVerify", lRet)) {
		goto done;
	}

	// Write it
	strcpy_s(szFilename, _MAX_PATH, szFolderSamples);
	strcat_s(szFilename, _MAX_PATH, "out3.ebts");
	lRet = IWWrite(pTrans, szFilename);
	if (!CheckRet("IWWrite", lRet)) goto done;

	printf("Successful creation of %s\n", szFilename);

done:

	if (pTrans != NULL) {
		IWClose(&pTrans);
		pTrans = NULL;
	}

	if (pVer != NULL) {
		IWCloseVerification(&pVer);
		pVer = NULL;
	}

	printf("\nPress <ENTER> to exit.");
	getchar();

	return 0;
}
