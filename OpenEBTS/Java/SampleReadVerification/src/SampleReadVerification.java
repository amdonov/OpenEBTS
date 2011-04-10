import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import com.obi.OpenEBTS;

public class SampleReadVerification 
{
	private static PrintWriter	_out;

	public static void main(String[] args)
	{
		int				ret;
		String 			sFolderSamples;
		String 			sPathVerification;
		String			sPathOutput;
		File			fileOut;
		OpenEBTS							ebts;		// OpenEBTS library
		OpenEBTS.NISTVerification			ver;		// Encapsulates Verification File
		OpenEBTS.NISTTransactionCategories	cats;		// All Transaction Categories
		OpenEBTS.NISTTransactionCategory	cat;		// One Transaction Category
		OpenEBTS.NISTTransactionList		tots;		// List of TOTs within a Transaction Category				
		OpenEBTS.NISTTransaction			tot;		// Encapsulates a TOT 				
		OpenEBTS.NISTRecordOccurrencesList	occs;		// Defines the min and max per-record-occurrences
		OpenEBTS.NISTRecordOccurrence		occ;		// Defines the min and max for one record type 
		OpenEBTS.NISTFieldDefinitionList	fields;		// Encapsulates all MNUs for a given TOT

		sFolderSamples = System.getenv("OPENEBTSSAMPLESFOLDER");
		sPathVerification = sFolderSamples + "/EBTS9.1_ENUS.txt"; 
		sPathOutput = sFolderSamples + "/SampleReadVerification_out.txt"; 

		ebts = new OpenEBTS();
		ver = ebts.new NISTVerification();

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

		ret = ver.loadFromFile(sPathVerification);
		if (ret != 0)
		{
			showError(ret, "loadFromFile");
			if (!ver.getParseError().isEmpty())
			{
				Out_println("Parsing error: " + ver.getParseError());
			}
			return;
		}

		// First we loop through the transaction categories and print out all the transactions
		// that fall under those categories. We also list the imposed record type occurrence limits.
		cats = ver.getTransactionCategories();

		for (int c = 0; c < cats.getTransactionCategoryCount(); c++)
		{
			cat = cats.getTransactionCategory(c);

			Out_println();
			Out_println("Category " + cat.getName());
			OutputUnderlines(9 + cat.getName().length());

			tots = cat.getTransactionList();

			for (int t = 0; t < tots.getTransactionCount(); t++)
			{
				tot = tots.getTransaction(t);

				Out_print("    TOT " + tot.getName() + " = " + tot.getDescription() + "   ");

				occs = tot.getRecordOccurrencesList();

				for (int o = 0; o < occs.getRecordOccurrenceCount(); o++)
				{
					occ = occs.getRecordOccurrence(o);

					Out_print(occ.getRecordType() + ":" + occ.getOccurrencesMin() + "-" + occ.getOccurrencesMax() + " ");
				}
				Out_println();
				_out.flush();

			}
		}

		Out_println();
		Out_println();

		// Now we restart the loop. This time, for each TOT we loop through all MNUs
		// and output all information about them that is available. 
		for (int c = 0; c < cats.getTransactionCategoryCount(); c++)
		{
			cat = cats.getTransactionCategory(c);
			tots = cat.getTransactionList();

			for (int t = 0; t < tots.getTransactionCount(); t++)
			{
				tot = tots.getTransaction(t);
				fields = tot.getFieldList();

				Out_println("Transaction Type: " + tot.getName());
				OutputUnderlines(18 + tot.getName().length());

				OutputFieldDefinitionList(fields, 1);
				Out_println();

				_out.flush();
			}
		}

		_out.flush();
	}

	private static void OutputFieldDefinitionList(OpenEBTS.NISTFieldDefinitionList fields, int nIndentLevel)
	{
		String sAutomatic;	// A or ' '
		String sMandatory;	// M or O
		String sNumbers;	// Complete field index e.g. 2.1.42
		String sSizes;		// Field's byte size range e.g. 1-5
		String sOccs;		// Field's occurrences range e.g. 1-10
		String sDatatype;	// Field's datatype specifics
		OpenEBTS.NISTFieldDefinition		field;		// Encapsulates one MNU
		OpenEBTS.NISTFieldValueList         values;		// The list of suggested values for a field

		for (int f = 0; f < fields.getFieldDefinitionCount(); f++)
		{
			field = fields.getFieldDefinition(f);

			sAutomatic = field.getIsAutomaticallySet() ? "A" : " ";
			sMandatory = field.getIsMandatory() ? "M" : "O";
			sNumbers = field.getRecordType() + "." + field.getFieldIndex() + "." + field.getItemIndex();
			
			// Field length. If min = max just print one number. Also, a max of -1 mean
			// "no limit", let's use an 'x' to denote this
 			sSizes = String.valueOf(field.getFieldSizeMin());
			if (field.getFieldSizeMax() != field.getFieldSizeMin())
			{
				sSizes += "-";
				if (field.getFieldSizeMax() == -1)
				{
					sSizes += "x";
				}
				else
				{
					sSizes += field.getFieldSizeMax();
				}
			}

			// Number of occurrences. We proceed like in in the field length case.
			// In this case, the min occurrences can also be -1 (the max has to be -1 as well)
			if (field.getOccurrencesMin() == -1)
			{
				sOccs = "x";
			}
			else
			{
				sOccs = String.valueOf(field.getOccurrencesMin());
			}
			if (field.getOccurrencesMax() != field.getOccurrencesMin())
			{
				sOccs += "-";
				if (field.getOccurrencesMax() == -1)
				{
					sOccs += "x";
				}
				else
				{
					sOccs += field.getOccurrencesMax();
				}
			}

			// Get field's datatype information
			sDatatype= "";
			if (field.getIsDataTypeA()) sDatatype += "A";
			if (field.getIsDataTypeN()) sDatatype += "N";
			if (field.getIsDataTypeB()) sDatatype += "B";
			if (field.getIsDataTypeP()) sDatatype += "P";
			if (field.getIsDataTypeC()) sDatatype += "C";
			if (field.getIsDataTypeSET()) sDatatype += "S";
			if (!field.getSpecialChars().isEmpty())
			{
				sDatatype += "+{" + field.getSpecialChars() + "}";
			}
			if (field.getIsDate())
			{
				sDatatype += "=>" + field.getDateFormat();
			}

			for (int i = 0; i < nIndentLevel; i++) Out_print("    ");
			Out_print(sAutomatic + sMandatory + " " + field.getMnemonic() + " (" + sNumbers + ", " +  field.getIdentifier() + ", " + field.getName() + ") ");
			Out_print("typ(" + sDatatype + ") len(" + sSizes + ") occ(" + sOccs + ") ");
			// Only output advanced rule is there is one, since it's rare;
			if (!field.getAdvancedRule().isEmpty())
			{
				Out_print("adv(" + field.getAdvancedRule() + ")");
			}
			Out_println();

			// Output field value list, if any
			values = field.getValueList();

			if (values.getFieldValueCount() > 0)
			{
				// Indent this value list some more (+1)
				for (int i = 0; i < nIndentLevel + 1; i++) Out_print("    ");
				Out_print(values.getIsMandatory() ? "Mandatory" : "Optional");				
				Out_println(" field list:");				
	
				for (int v = 0; v < values.getFieldValueCount(); v++)
				{
					for (int i = 0; i < nIndentLevel + 1; i++) Out_print("    ");
	
					// Since there could be hundreds of these (for example, geographic locations) we
					// only show the first 5
					if (v < 5)
					{
						Out_println("    " + values.getFieldValueValue(v) + " {" +  values.getFieldValueName(v) + "}");
					}
					else
					{
						Out_println("etc... (" + values.getFieldValueCount() + " values in total)");
						break;
					}
				}
			}

			// If this field has subitems, list them too but indented, by recursing into this function
			if (field.getItemList() != null)
			{
				OutputFieldDefinitionList(field.getItemList(), nIndentLevel + 1);
			}
		}
	}

	private static void OutputUnderlines(int n)
	{
		for (int u = 0; u < n; u++) Out_print("-");
		Out_println();
	}

	private static boolean showError(int ret, String sContext)
	{
		if (ret == OpenEBTS.IW_SUCCESS) return false; // no error
		Out_println(sContext + " error " + ret);
		return true;
	}

	//
	// Out_* functions replace regular System.out functions to allow writing to file
	// and printing to System.out at the same time.
	//

	private static void Out_print(String s)
	// Output to System and to file
	{
		_out.print(s);
		System.out.print(s);
	}

	private static void Out_println(String s)
	// Output to System and to file
	{
		_out.println(s);
		System.out.println(s);
	}

	private static void Out_println()
	{
		_out.println();
		System.out.println();
	}
}
