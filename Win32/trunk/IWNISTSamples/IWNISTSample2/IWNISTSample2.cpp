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
	strcat_s(szFilename, _MAX_PATH, "ebts1_2.txt");
	lRet = IWReadVerification(szFilename, &pVer, IW_MAX_PARSE_ERROR, szParseError);
	if (!CheckRet("IWReadVerification", lRet)) {
		printf("Error parsing verification file '%s': %s\n", szFilename, szParseError);
		goto done;
	}

	// Create new transaction, with verification file loaded to enable mnemonic
	// usage for field referral. In this sample we create a transaction of type LFIS
	lRet = IWNew("LFIS", pVer, &pTrans);
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
	lRet = IWSet(pTrans, "T1_DOM_IMP", "EBTSNAVY", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_DOM_VER", "1.2", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;

	//
	// Add Type-2 record and set important fields
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);					if (!CheckRet("IWAddRecord", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_ATN", "DET J Q PUBLIC", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_CIX", "42", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_HTR", "400711", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_GEO", "AK", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_PRI", "3", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_LATENT_TECN", "DOE, JOHN", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_LATENT_SUB", "SMITH, JOE", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_CIN_PRE", "12345", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_CIN_ID", "12345", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_IMA_MAK", "Make", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_IMA_MAK", "Make", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_IMA_MODL", "Model", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_IMA_SERNO", "SerialNumber", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_RCD2_FGP", "02", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_RCD2_RCN2", "10", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;

	//
	// Add some fingerprints of various types
	//
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, "finger1.bmp", "bmp", "wsq", 0, 2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans,  7, szFolderSamples, "finger2.bmp", "bmp", "wsq", 0, 7)) goto done; //0=livescan, 7=L-index

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
		goto done;	// No point writing the file if it isn't right
	} else if (!CheckRet("IWVerify", lRet)) {
		goto done;
	}

	// Write it
	strcpy_s(szFilename, _MAX_PATH, szFolderSamples);
	strcat_s(szFilename, _MAX_PATH, "out2.ebts");
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
