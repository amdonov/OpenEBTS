import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;

import com.obi.OpenEBTS;

public class SampleExportImages
{
	private static PrintWriter	_out;

	public static void main(String[] args)
	{
		int				ret;
		String 			sFolderSamples;
		String 			sPathTransaction;
		String			sPathOutput;
		File			fileOut;
		int 			nRecordType;
		int				nRecords;
		int				nRecordIndex;
		OpenEBTS				ebts;
		OpenEBTS.NISTFile 		nist;

		sFolderSamples = System.getenv("OPENEBTSSAMPLESFOLDER");

		sPathTransaction = sFolderSamples + "/NISTFile2.ebts";
		sPathOutput = sFolderSamples + "/SampleExportImages_out.txt"; 

		ebts = new OpenEBTS();
		nist = ebts.new NISTFile();

		// Create our output file to receive this program's output
		fileOut = new File(sPathOutput);
		try
		{
			_out = new PrintWriter(fileOut);
		}
		catch (FileNotFoundException e)
		{
			e.printStackTrace();
			return;
		}

		ret = nist.readFromFile(sPathTransaction);
		if (showError(ret, "readFromFile")) return;

		// Loop through Type-3 to Type-17 image records
		for (nRecordType = 3; nRecordType <= 17; nRecordType++)
		{
			nRecords = nist.getRecordCountofType(nRecordType);
	
			for (nRecordIndex = 1; nRecordIndex <= nRecords; nRecordIndex++)
			{
				nist.getImageFormat(nRecordType, nRecordIndex);
	
				outputImageFile(nist, sFolderSamples, nRecordType, nRecordIndex, "bmp"); 
				outputImageFile(nist, sFolderSamples, nRecordType, nRecordIndex, "jpg"); 
				outputImageFile(nist, sFolderSamples, nRecordType, nRecordIndex, "jp2"); 
				outputImageFile(nist, sFolderSamples, nRecordType, nRecordIndex, "png"); 
				outputImageFile(nist, sFolderSamples, nRecordType, nRecordIndex, "wsq"); 
				outputImageFile(nist, sFolderSamples, nRecordType, nRecordIndex, "fx4"); 
			}
		}

		out_println("Done.");
		_out.flush();
	}

	private static void outputImageFile(OpenEBTS.NISTFile nist, String sFolderSamples, int nRecordType, int nRecordIndex, String sFmt)
	{
		int				ret;
		String			sPathImageOutput;

		out_print("Exporting Type-" + nRecordType + " image #" + nRecordIndex + " as " + sFmt+ "...");

		sPathImageOutput = sFolderSamples + "/SampleExportImages" + nRecordType + "." + nRecordIndex + "_out." + sFmt;
		ret = nist.getImageAsToFile(nRecordType, nRecordIndex, formatFromString(sFmt), sPathImageOutput);

		// Trap special case when error is just caused by an incompatible bit-depth
		if (ret == OpenEBTS.IW_ERR_UNSUPPORTED_BIT_DEPTH)
		{
			out_println(" (skipping: incompatible bit depths)");
		}
		else
		{
			out_println("");
			if (showError(ret, "getImageAsToFile")) return;
		}
	}

	private static int formatFromString(String sFmt)
	{
		int fmt = 0;
		
		if (sFmt.equals("raw")) fmt = OpenEBTS.IMAGEFORMAT_RAW;
		else if (sFmt.equals("bmp")) fmt = OpenEBTS.IMAGEFORMAT_BMP;
		else if (sFmt.equals("wsq")) fmt = OpenEBTS.IMAGEFORMAT_WSQ;
		else if (sFmt.equals("jpg")) fmt = OpenEBTS.IMAGEFORMAT_JPG;
		else if (sFmt.equals("jp2")) fmt = OpenEBTS.IMAGEFORMAT_JP2;
		else if (sFmt.equals("fx4")) fmt = OpenEBTS.IMAGEFORMAT_FX4;
		else if (sFmt.equals("png")) fmt = OpenEBTS.IMAGEFORMAT_PNG;

		return fmt;
	}

	private static void out_print(String s)
	// Output to System and to file
	{
		_out.print(s);
		System.out.print(s);
	}

	private static void out_println(String s)
	// Output to System and to file
	{
		_out.println(s);
		System.out.println(s);
	}

	private static boolean showError(int ret, String sContext)
	{
		if (ret == OpenEBTS.IW_SUCCESS) return false; // no error
		out_println(sContext + " error " + ret);
		return true;
	}
}
