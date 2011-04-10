import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import com.obi.OpenEBTS;

public class SampleReadFile
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
		int				nFields;
		int				nFieldIndex;
		int				nSubfields;
		int				nSubfieldIndex;
		int				nItems;
		int				nItemIndex;
		OpenEBTS				ebts;
		OpenEBTS.NISTFile 		nist;
		OpenEBTS.NISTRecord		record;
		OpenEBTS.NISTField		field;
		OpenEBTS.NISTSubfield	subfield;
		OpenEBTS.NISTItem		item;

		sFolderSamples = System.getenv("OPENEBTSSAMPLESFOLDER");

		sPathTransaction = sFolderSamples + "/NISTFile.ebts";
		sPathOutput = sFolderSamples + "/SampleReadFile_out.txt"; 

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

		// Enumerate all the textual data in NIST file by navigating to the records,
		// the field, the subfields and finally the items.
		for (nRecordType = 1; nRecordType <= 99; nRecordType++)
		{
			nRecords = nist.getRecordCountofType(nRecordType);
			if (nRecords == 0) continue;

			for (nRecordIndex = 1; nRecordIndex <= nRecords; nRecordIndex++)
			{
				record = nist.getRecordOfType(nRecordType, nRecordIndex);

				nFields = record.getFieldCount();
				for (nFieldIndex = 1; nFieldIndex <= nFields; nFieldIndex++)
				{
					field = record.getField(nFieldIndex);

					nSubfields = field.getSubfieldCount();
					for (nSubfieldIndex = 1; nSubfieldIndex <= nSubfields; nSubfieldIndex++)
					{
						subfield = field.getSubfield(nSubfieldIndex);

						nItems = subfield.getItemCount();
						for (nItemIndex = 1; nItemIndex <= nItems; nItemIndex++)
						{
							item = subfield.getItem(nItemIndex);

							OutputItem(item);
						}
					}
				}
			}

			_out.flush();
		}
	}

	private static void OutputItem(OpenEBTS.NISTItem item)
	{
		int		nRecordType;
		int		nRecordIndex;
		int		nFieldIndex;
		int		nSubfieldIndex;
		int		nItemIndex;
		String	sData;

		nRecordType = item.getRecordType();
		nRecordIndex = item.getRecordIndex();
		nFieldIndex = item.getFieldIndex();
		nSubfieldIndex = item.getSubfieldIndex();
		nItemIndex = item.getItemIndex();
		sData = item.getData();

		Out_println("(#" + nRecordIndex + ") "  + nRecordType + "." + nFieldIndex + "." + nSubfieldIndex + "." + nItemIndex + ": \"" + sData + "\"");
	}

	private static void Out_println(String s)
	// Output to System and to file
	{
		_out.println(s);
		System.out.println(s);
	}

	private static boolean showError(int ret, String sContext)
	{
		if (ret == OpenEBTS.IW_SUCCESS) return false; // no error
		Out_println(sContext + " error " + ret);
		return true;
	}
}
