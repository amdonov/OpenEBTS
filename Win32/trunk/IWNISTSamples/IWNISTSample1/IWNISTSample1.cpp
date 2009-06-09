// IWNISTSample1.cpp
//

#include "stdafx.h"
#include "IWNIST.h"
#include "IWNISTErrors.h"
#include "..\Helpers.h"

//
// This sample demonstrates the creation of a sample FBI CAR request, containing
// records of Type-1, Type-2, Type-4, Type-10, Type-14 and Type-16. The contents
// pass through Verification before being written to file.
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
	if (!CheckRet("IWReadVerification", lRet)) goto done;

	// Create new transaction, with verification file loaded to enable mnemonic
	// usage for field referral. In this sample we create a transaction of type CAR
	lRet = IWNew("CAR", pVer, &pTrans);
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
	lRet = IWSet(pTrans, "T1_DOM_IMP", "EBTSTEST", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T1_DOM_VER", "1.2", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;

	//
	// Add Type-2 record and set important fields
	//
	lRet = IWAddRecord(pTrans, 2, &iIndex);					if (!CheckRet("IWAddRecord", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_RET", "Y", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_NAM", "LISA, MONA J", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_NAM1", "LISA", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_NAM2", "MONA", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_NAM3", "J", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_POB", "IT", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_DOB", "19750620", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_SEX", "F", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_RAC", "W", 1, 1);				if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_HGT", "510", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_WGT", "100", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_EYE", "BRO", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_HAI", "BLK", 1, 1);			if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_CRI", "DDNMI0039", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_PER_TYPE", "MIO", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_DOA", "20080709", 1, 1);		if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_ASL_DOO", "20080709", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_ASL_AOL", "Armed Robbery", 1, 1); if (!CheckRet("IWSet", lRet)) goto done;
	lRet = IWSet(pTrans, "T2_ISN", "1234567890123", 1, 1);	if (!CheckRet("IWSet", lRet)) goto done;

	//
	// Add some Type-4 fingerprints and a Type-14 4-finger slap
	//
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, "finger1.bmp", "bmp", "wsq", 0, 2)) goto done; //0=livescan, 2=R-index
	if (!AddPrintHelper(pTrans,  4, szFolderSamples, "finger2.bmp", "bmp", "wsq", 0, 7)) goto done; //0=livescan, 7=L-index
	if (!AddPrintHelper(pTrans, 14, szFolderSamples, "slap.bmp",   "bmp", "wsq", 0, 13)) goto done; //0=livescan, 13=R-4 fingers

	//
	// Add some Type-10 mugshots
	//
	if (!AddMugshotHelper(pTrans, szFolderSamples, "face1.jpg", "jpg", "jpg")) goto done;
	if (!AddMugshotHelper(pTrans, szFolderSamples, "face2.jpg", "jpg", "jpg")) goto done;

	//
	// Add some Type-16 iris images
	//
	if (!AddType16Helper(pTrans, szFolderSamples, "iris1.jpg", "jpg", "jpg", 1)) goto done; //1=R-eye
	if (!AddType16Helper(pTrans, szFolderSamples, "iris2.jpg", "jpg", "jpg", 2)) goto done; //2=L-eye

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
	strcat_s(szFilename, _MAX_PATH, "out1.ebts");
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
