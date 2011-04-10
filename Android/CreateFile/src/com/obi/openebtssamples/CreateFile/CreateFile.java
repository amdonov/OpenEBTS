package com.obi.openebtssamples.CreateFile;

import com.obi.OpenEBTS;
import java.io.*;
import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;

public class CreateFile extends Activity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		int						ret;
		OpenEBTS				ebts;
		OpenEBTS.NISTFile 		nist;
		String 					sFolderSamples;
		String 					sPathTransaction;
		String 					sPathVerification;
		String					sPathFingerprint;
		StringBuffer			sbParseError;
		String					sErrorOut;
		int						nErrorOut;
		int 					nIndex;
		byte[]					baFingerprint;

        super.onCreate(savedInstanceState);
        //setContentView(R.layout.main);

		// Get Android's SD card folder, add "OpenEBTSSamles" and use that as the root of
		// all our test files
        File sdDir = Environment.getExternalStorageDirectory();
        sFolderSamples =  sdDir + "/OpenEBTSSamples";

        // Set up all file locations up front
		sPathVerification = sFolderSamples + "/EBTS9.1_ENUS.txt"; 
		sPathFingerprint = sFolderSamples + "/finger8.bmp"; 
		sPathTransaction = sFolderSamples + "/SampleCreateFile_out.ebts";

		// Create an OpenEBTS.NISTFile to manage our transaction file.
		// Set it up for a "CAR" transaction. 
		ebts = new OpenEBTS();
		nist = ebts.new NISTFile("CAR");

		// First load and associate Verification file with our transaction object: this
		// will allow us to refer to fields using their mnemonics.
		sbParseError = new StringBuffer(""); // prepare out parameter
		ret = nist.associateVerificationFile(sPathVerification, sbParseError);
		if (ret != OpenEBTS.IW_SUCCESS)
		{
			showError(ret, "associateVerificationFile");
			if (sbParseError.length() != 0)
			{
				Out_println("Parsing error: " + sbParseError);
			}
			return;
		}


		//
		// Set Type-1 fields. We don't need to add a Type-1 record, all EBTS files have one and exactly one.
		//
		ret = nist.setDataViaMnemonic("T1_DAT", 1, 1, "20080709");		if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_PRY", 1, 1, "4");				if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_DAI", 1, 1, "TEST00001");		if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_ORI", 1, 1, "TEST00001");		if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_TCN", 1, 1, "TEST-20080528173758-SOFT-0001-1C629");	if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_NSR", 1, 1, "19.69");			if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_NTR", 1, 1, "19.69");			if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_DOM_IMP",  1, 1,"EBTSTEST");	if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T1_DOM_VER",  1, 1,"NORAM");		if (showError(ret, "setDataViaMnemonic")) return;

		//
		// Add Type-2 record and set important fields
		//
		nIndex = nist.addRecord(2);											if (showError(ret, "addRecord")) return;
		ret = nist.setDataViaMnemonic("T2_RET", nIndex, 1, "Y");					if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_NAM", nIndex, 1, "LISA, MONA J");		if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_POB", nIndex, 1, "IT");				if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_DOB", nIndex, 1, "19750620");			if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_SEX", nIndex, 1, "F");					if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_RAC", nIndex, 1, "W");					if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_HGT", nIndex, 1, "510");				if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_WGT", nIndex, 1, "100");				if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_EYE", nIndex, 1, "BRO");				if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_HAI", nIndex, 1, "BLK");				if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_CRI", nIndex, 1, "DDNMI0039");			if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_DOA", nIndex, 1, "20080709");			if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_ASL_DOO", nIndex, 1, "20080709");		if (showError(ret, "setDataViaMnemonic")) return;
		ret = nist.setDataViaMnemonic("T2_ASL_AOL", nIndex, 1, "Armed Robbery"); if (showError(ret, "setDataViaMnemonic")) return;


		// Add Type-4 record
		baFingerprint = ReadByteArrayFromFile(sPathFingerprint);
		if (baFingerprint == null) return;

		nIndex = nist.addRecord(4);
		// Set image data (let OpenEBTS convert the input to WSQ with compression factor of 15)
		ret = nist.setImage(4, nIndex, baFingerprint, OpenEBTS.IMAGEFORMAT_BMP, OpenEBTS.IMAGEFORMAT_WSQ, 15);
		if (showError(ret, "setImage")) return;

		// Set mandatory impression Type, 0 = livescan
		ret = nist.setDataViaMnemonic("T4_IMP", nIndex, 1, "0");
 
		// Set mandatory NIST fingerprint index, 2 = R index
		ret = nist.setDataViaMnemonic("T4_FGP", nIndex, 1, "2");

		// Now verify entire transaction file
		ret = nist.verify();
		if (ret == OpenEBTS.IW_WARN_TRANSACTION_FAILED_VERIFICATION)
		{
			// Output all warnings
			for (int i= 0; i < nist.getErrorCount(); i++)
			{
				nErrorOut = nist.getErrorCode(i);
				sErrorOut = nist.getErrorString(i);

				Out_println("Verification warning: " + sErrorOut + " (code " + nErrorOut + ")");
			}
			Out_println("Aborting NIST file creation.");
			return;
		}

		ret = nist.writeToFile(sPathTransaction);
		if (showError(ret, "writeToFile")) return;

		Out_println("Successful creation of " + sPathTransaction);
    }

    private static byte[] ReadByteArrayFromFile(String sFile)
	// Populates and returns byte array from file. Returns null on failure.
	{
		File file = new File(sFile);
		byte[] ba = null;

		try
		{
			//	create FileInputStream object
			FileInputStream stream = new FileInputStream(file);
			ba = new byte[(int)file.length()];

			stream.read(ba);
		}
		catch(FileNotFoundException e)
		{
			Out_println("Exception encountered while attempting to read " + sFile);
			Out_println("File not found:" + e);
			ba = null;
		}
		catch(IOException ioe)
		{
			Out_println("Exception encountered while attempting to read " + sFile);
			Out_println(ioe.toString());
			ba = null;
		}

		return ba;
	}

	private static void Out_println(String s)
	// Output to System and to file
	{
		System.out.println(s);
	}

	private static boolean showError(int ret, String sContext)
	{
		if (ret == OpenEBTS.IW_SUCCESS) return false; // no error
		Out_println(sContext + " error " + ret);
		return true;
	}
}
