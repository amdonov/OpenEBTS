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
	if (!CheckRet("IWReadVerification", lRet)) {
		printf("Error parsing verification file '%s': %s\n", szFilename, szParseError);
		goto done;
	}

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
	// Add Type-13 images using all possible input and output formats
	//
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger1.bmp", "bmp", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger1.bmp", "bmp", "fx4", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.bmp", "bmp", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.bmp", "bmp", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.bmp", "bmp", "jp2", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.bmp", "bmp", "wsq", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.bmp", "bmp", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.bmp", "bmp", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.bmp", "bmp", "jp2", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jpg", "jpg", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jpg", "jpg", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jpg", "jpg", "jp2", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jpg", "jpg", "wsq", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.jpg", "jpg", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.jpg", "jpg", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.jpg", "jpg", "jp2", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jp2", "jp2", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jp2", "jp2", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jp2", "jp2", "jp2", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.jp2", "jp2", "wsq", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.jp2", "jp2", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.jp2", "jp2", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger24.jp2", "jp2", "jp2", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.wsq", "wsq", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.wsq", "wsq", "jpg", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.wsq", "wsq", "jp2", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger8.wsq", "wsq", "wsq", 0, 0)) goto done;

	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger1.fx4", "fx4", "raw", 0, 0)) goto done;
	if (!AddPrintHelper(pTrans, 13, szFolderSamples, "finger1.fx4", "fx4", "fx4", 0, 0)) goto done;

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
	strcat_s(szFilename, _MAX_PATH, "out4.ebts");
	lRet = IWWrite(pTrans, szFilename);
	if (!CheckRet("IWWrite", lRet)) goto done;

	printf("Successful creation of %s\n", szFilename);

	// Close-off transaction
	IWClose(&pTrans);
	pTrans = NULL;

	// Now let's read the file back and save out the images
	lRet = IWRead(szFilename, pVer, &pTrans);
	if (!CheckRet("IWRead", lRet)) goto done;

	// Save out the files as they are
	int			nRecs;
	const char	*szFmt;
	long		lLength;
	long		lH;
	long		lV;
	int			iBpp;
	const void	*pData;
	char		szFile[_MAX_FNAME];

	lRet = IWGetRecordTypeCount(pTrans, 13, &nRecs);
	if (!CheckRet("IWGetRecordTypeCount", lRet)) goto done;
	for (int i=1; i <= nRecs; i++)
	{
		// Call IWGetImageInfo just to get bpp
		lRet = IWGetImageInfo(pTrans, 13, i, &szFmt, &lLength, &lH, &lV, &iBpp);
		if (!CheckRet("IWGetImageInfo", lRet)) goto done;

		// Get the image bits
		lRet = IWGetImage(pTrans, 13, i, &szFmt, &lLength, &pData);
		if (!CheckRet("IWGetImage", lRet)) goto done;

		sprintf_s(szFile, _MAX_FNAME, "out4_%02d_%dx%d_%dbpp.%s", i, lH, lV, iBpp, szFmt);
		strcpy_s(szFilename, _MAX_PATH, szFolderSamples);
		strcat_s(szFilename, _MAX_PATH, szFile);

		WriteBLOBToFile(szFilename, (char*)pData, lLength);
	}

	printf("Successful extraction of images within c:\\out4.ebts\n");

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
