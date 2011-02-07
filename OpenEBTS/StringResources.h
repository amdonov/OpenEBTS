//
// String table
//
// Note that the Unicode versions use %ls vs. %s, other than that they are identical.
//


#ifdef UNICODE

// Verification file errors
#define IDS_SETTRANSFAILED			_T("SetTransaction Failed")
#define IDS_INVALIDLOCATION			_T("Invalid location: %ls")
#define IDS_INVALIDMNU				_T("Invalid MNU: %ls")
#define IDS_INVALIDCHARTYPE			_T("%ls, invalid char type: %ls")
#define IDS_INVALIDFIELDSIZE		_T("%ls, invalid field size: %ls")
#define IDS_INVALIDOCCURRENCES		_T("%ls, invalid occurrence value: %ls")
#define IDS_INVALIDDESCRIPTION		_T("%ls, invalid description value: %ls")
#define IDS_INVALIDLONGDESCRIPTION	_T("%ls, invalid long description value: %ls")
#define IDS_INVALIDSCA				_T("%ls, invalid sca value: %ls")
#define IDS_INVALIDDATEFORMAT		_T("%ls, invalid date format value: %ls")
#define IDS_INVALIDADVANCEDRULE		_T("%ls, invalid advanced rule: %ls")
#define IDS_INVALIDMMAP				_T("%ls, invalid mmap value: %ls")
#define IDS_INVALIDOMAP				_T("%ls, invalid omap value: %ls")
// NIST file validation errors
#define IDS_EXCEPTIONUNK        	_T("[%ls] Unknown exception occurred.")
#define IDS_TOTRECORDSMAX       	_T("Transaction Type %ls may contain at most %ld Type %ld records: it contains %ld")
#define IDS_TOTRECORDSMIN       	_T("Transaction Type %ls must contain at least %ld Type %ld records: it only contains %ld.")
#define IDS_TOTRECORDUNSUPPORTED 	_T("Transaction Type %ls may not contain Type %ld records: it contains %ld of them.")
#define IDS_MANDATORYFIELD      	_T("Mandatory field not present")
#define IDS_UNSUPPORTEDFIELD    	_T("Unsupported field present")
#define IDS_VERTOTUNK           	_T("Verification file does not contain Transaction Type %ls")
#define IDS_NOTOTFIELD          	_T("Failed to find TOT field in Record 1")
#define IDS_INCORRECTITEMCOUNT  	_T("Incorrect item count (%ld), must be 1")
#define IDS_INVALIDLENGTH       	_T("Invalid length of %ld, should be %ld")
#define IDS_INVALIDLENGTHMIN    	_T("Invalid length of %ld, minimum allowed is %ld")
#define IDS_INVALIDLENGTHMAX    	_T("Invalid length of %ld, maximum allowed is %ld")
#define IDS_INVALIDTOTALLENGTH  	_T("Invalid total subfield length of %ld, should be %ld")
#define IDS_INVALIDTOTALLENGTHMIN  	_T("Invalid total subfield length of %ld, minimum allowed is %ld")
#define IDS_INVALIDTOTALLENGTHMAX  	_T("Invalid total subfield length of %ld, maximum allowed is %ld")
#define IDS_CHARINVALIDVALUE    	_T("Invalid value '%ls' for CharType '%ls'")
#define IDS_CHARINVALIDVALUESPECIAL _T("Invalid value '%ls' for CharType '%ls', Special Chars '%ls'")
#define IDS_TOOMANYDATACHARS    	_T("Invalid value %ld for CharType '%ls'")
#define IDS_INVALIDYEAR         	_T("Invalid year %ld")
#define IDS_INVALIDMONTH        	_T("Invalid month %ld")
#define IDS_INVALIDDAY          	_T("Invalid day %ld")
#define IDS_INVALIDMMAPDATA         _T("'%ls': invalid data from mandatory map")
#define IDS_TOOFEWSUBFIELDS     	_T("Too few subfields (%ld), must be %ld")
#define IDS_TOOMANYSUBFIELDS    	_T("Too many subfields (%ld), must be %ld")
#define IDS_TOOFEWSUBFIELDSMIN  	_T("Too few subfields (%ld), minimum allowed is %ld")
#define IDS_TOOFEWSUBFIELDSMAX  	_T("Too many subfields (%ld), maximum allowed is %ld")
#define IDS_TOTRECORDSMAX2      	_T("Transaction Type %ls may contain at most %ld Type %ld and Type %ld records: it contains %ld")
#define IDS_TOTRECORDSMIN2      	_T("Transaction Type %ls must contain at least %ld Type %ld and Type %ld records: it only contains %ld.")
#define IDS_FLAGFIELDERROR			_T("Field %ls (%ls): %ls")
// Logfile entries
#define IDS_LOGTRANSNEW				_T("[CIWTransaction::New] Result %d")
#define IDS_LOGTRANSREAD			_T("[CIWTransaction::ReadTransactionFile] Path %ls, Result %d")
#define IDS_LOGTRANSREADMEM			_T("[CIWTransaction::ReadTransactionFileMem] Result %d")
#define IDS_LOGTRANSRECORDSINFILE	_T("[CIWTransaction::GetRecords] Records in file %d")
#define IDS_LOGTRANSREADRECORDTYPE	_T("[CIWTransaction::GetRecords] Read record type %d, result %d")
#define	IDS_LOGTRANSREADRECORDFAILED _T("[CIWTransaction::GetRecords] Read record FAILED. File %ls, type %d, result %d")
#define	IDS_LOGTRANSADDRECORD		_T("[CIWTransaction::AddRecord] Type %d, Index %d")
#define IDS_LOGTRANSREMOVEITEM		_T("[CIWTransaction::RemoveItem] (%ld, %ld, %ld, %ld, %ld)")
#define IDS_LOGTRANSGETIMAGE		_T("[CIWTransaction::GetImage] Type %d, Index %d, Result %d")
#define IDS_LOGTRANSSETIMAGE		_T("[CIWTransaction::SetImage] Type %d, Index %d, Input format %ls, Result %d")
#define IDS_LOGTRANSWRITEBINARY		_T("[CIWTransaction::WriteBinary] Write record FAILED. File %ls, result %d")
#define IDS_LOGTRANSWRITEMEM		_T("[CIWTransaction::WriteMem] Write record to memory FAILED. Result %d")
#define IDS_LOGTRANSWRITEXML		_T("[CIWTransaction::WriteXML] WriteXML FAILED. File %ls, result %d")
#define IDS_LOGTRANSDEBUGSTART		_T("------------------------------- %ls")
#define IDS_LOGTRANSDEBUGEND		_T("-------------------------------")
#define IDS_LOGTRANSDEBUGCONTENTS1	_T("CONTENTS: (01) %02d items")
#define IDS_LOGTRANSDEBUGCONTENTS2	_T("CONTENTS: (%02d) %02s %02s")
#define IDS_LOGTRANSDEBUGRECORD		_T("Record %02d, Type %02d")
#define IDS_LOGTRANSDEBUGERROR		_T("error")
#define IDS_LOGFIELDGETWRITELEN		_T("[CNISTField::GetWriteLen] RecordType %d, Field %d, Len %ld")
#define IDS_LOGFIELDWRITE			_T("[CNISTField::Write] RecordType %d, Field %d")
#define	IDS_LOGRECINITNEW			_T("[CNISTRecord::InitializeNewRecord] RecordType %d")
#define IDS_LOGRECREADRECLEN		_T("[CNISTRecord::ReadLogicalRecordLen] RecordType %d, Index %d, Length %ld")
#define IDS_LOGRECREADREC			_T("[CNISTRecord::ReadRecord] RecordType %d")
#define IDS_LOGRECREADRECBIN		_T("[CNISTRecord::ReadBinaryRecord] RecordType %d")
#define IDS_LOGRECSETITEM			_T("[CNISTRecord::SetItem] (%ld, %ld, %ld): %ls")
#define IDS_LOGRECGETRECLEN			_T("[CNISTRecord::GetRecordLen] Recordtype %d, Len %ld")
#define IDS_LOGRULESETDATA			_T("[CRuleObj::SetData] Error: %ls")
#define IDS_LOGRULEDUMPOBJ1			_T("[CRuleObj::DumpObject] ==> MNU: %ls, Location: %ls, chartype %ls, len min %ld, max %ld, occ min %ld, max %ld desc(%ls) sca(%ls) date(%ls) map(%ls)")
#define IDS_LOGRULEDUMPOBJ2			_T("[CRuleObj::DumpObject] %ls, TOT's: %ls")
#define IDS_LOGRULESETTRANS			_T("[CRuleObj::SetTransactions] TOT = %ls")
#define IDS_LOGRULETESTREGEX		_T("[CRuleObj::TestRegEx] Exception thrown. %ls. Maybe an invalid mask defined.")
#define IDS_LOGVERLOADRULES			_T("[CIWVerification::LoadRules] Loc: %ls, MNU: %ls, Type: %ls, Size: %ls, Occ: %ls")
#define IDS_LOGVERREADLINE			_T("[CIWVerification::ReadLine] exception thrown")
#define IDS_LOGVERGETRULE			_T("[CIWVerification::GetRule] exception thrown")
#define IDS_LOGVERSKIPCOMMENTS		_T("[CIWVerification::SkipComments] exception thrown")
#define IDS_LOGVERGETTRANSLIST		_T("[CIWVerification::GetTransactionList] exception thrown")
#define IDS_LOGVERGETRANGETOKEN		_T("[CIWVerification::GetRangeToken] exception thrown")
#define IDS_LOGVERGETNEXTTOKEN		_T("[CIWVerification::GetNextToken] exception thrown")
#define IDS_LOGVERREADVERFILE		_T("[CIWVerification::ReadVerificationFile] Error reading verification file %s")
#define IDS_LOGVERLOADTOTDEFS		_T("[CIWVerification::LoadTOTDefinitions] %ld TOT definitions in file")
#define IDS_LOGVERLOADTOTDEFSDBG	_T("[CIWVerification::LoadTOTDefinitions] (%ls, %ls)")
#define IDS_LOGVERDBGOUTDELIM		_T("%%%%%%%%%%%%%%%%%%%%")
#define IDS_LOGVERDBGOUTMIN			_T("%ld:%ls ")
#define IDS_LOGVERDBGOUTMINMAX		_T("%ld:%ls-%ls ")
#define IDS_LOGVERDBGOUTFIELD		_T("%ls\t%ls\t%3s Len(%2ld..%2ld) Occ(%2ld..%2ld) Desc(%ls) sca(%ls) date(%ls) map(%ls) Trans %ls\n")
#define IDS_LOGVERVERIFYTRANS		_T("[CIWVerification::VerifyTransaction] Applying %ls\t%ls\t%3s Len(%2ld..%2ld) Occ(%2ld..%2ld) Trans %ls")
#define IDS_LOGVERVERIFYTRANSERR	_T("[CIWVerification::VerifyTransaction] Invalid Location Form Type")
#define IDS_LOGVERVERIFYFIELDCHARS	_T("[CIWVerification::VerifyFieldChars] %ls, unknown CharType: %ls\n")

#else

// Verification file errors
#define IDS_SETTRANSFAILED			_T("SetTransaction Failed")
#define IDS_INVALIDLOCATION			_T("Invalid location: %s")
#define IDS_INVALIDMNU				_T("Invalid MNU: %s")
#define IDS_INVALIDCHARTYPE			_T("%s, invalid char type: %s")
#define IDS_INVALIDFIELDSIZE		_T("%s, invalid field size: %s")
#define IDS_INVALIDOCCURRENCES		_T("%s, invalid occurrence value: %s")
#define IDS_INVALIDDESCRIPTION		_T("%s, invalid description value: %s")
#define IDS_INVALIDLONGDESCRIPTION	_T("%s, invalid long description value: %s")
#define IDS_INVALIDSCA				_T("%s, invalid sca value: %s")
#define IDS_INVALIDDATEFORMAT		_T("%s, invalid date format value: %s")
#define IDS_INVALIDADVANCEDRULE		_T("%s, invalid advanced rule: %s")
#define IDS_INVALIDMMAP				_T("%s, invalid mmap value: %s")
#define IDS_INVALIDOMAP				_T("%s, invalid omap value: %s")
// NIST file validation errors
#define IDS_EXCEPTIONUNK        	_T("[%s] Unknown exception occurred.")
#define IDS_TOTRECORDSMAX       	_T("Transaction Type %s may contain at most %ld Type %ld records: it contains %ld")
#define IDS_TOTRECORDSMIN       	_T("Transaction Type %s must contain at least %ld Type %ld records: it only contains %ld.")
#define IDS_TOTRECORDUNSUPPORTED 	_T("Transaction Type %s may not contain Type %ld records: it contains %ld of them.")
#define IDS_MANDATORYFIELD      	_T("Mandatory field not present")
#define IDS_UNSUPPORTEDFIELD    	_T("Unsupported field present")
#define IDS_VERTOTUNK           	_T("Verification file does not contain Transaction Type %s")
#define IDS_NOTOTFIELD          	_T("Failed to find TOT field in Record 1")
#define IDS_INCORRECTITEMCOUNT  	_T("Incorrect item count (%ld), must be 1")
#define IDS_INVALIDLENGTH       	_T("Invalid length of %ld, should be %ld")
#define IDS_INVALIDLENGTHMIN    	_T("Invalid length of %ld, minimum allowed is %ld")
#define IDS_INVALIDLENGTHMAX    	_T("Invalid length of %ld, maximum allowed is %ld")
#define IDS_INVALIDTOTALLENGTH  	_T("Invalid total subfield length of %ld, should be %ld")
#define IDS_INVALIDTOTALLENGTHMIN  	_T("Invalid total subfield length of %ld, minimum allowed is %ld")
#define IDS_INVALIDTOTALLENGTHMAX  	_T("Invalid total subfield length of %ld, maximum allowed is %ld")
#define IDS_CHARINVALIDVALUE    	_T("Invalid value '%s' for CharType '%s'")
#define IDS_CHARINVALIDVALUESPECIAL _T("Invalid value '%s' for CharType '%s', Special Chars '%s'")
#define IDS_TOOMANYDATACHARS    	_T("Invalid value %ld for CharType '%s'")
#define IDS_INVALIDYEAR         	_T("Invalid year %ld")
#define IDS_INVALIDMONTH        	_T("Invalid month %ld")
#define IDS_INVALIDDAY          	_T("Invalid day %ld")
#define IDS_INVALIDMMAPDATA        	_T("'%s': invalid data from mandatory map")
#define IDS_TOOFEWSUBFIELDS     	_T("Too few subfields (%ld), must be %ld")
#define IDS_TOOMANYSUBFIELDS    	_T("Too many subfields (%ld), must be %ld")
#define IDS_TOOFEWSUBFIELDSMIN  	_T("Too few subfields (%ld), minimum allowed is %ld")
#define IDS_TOOFEWSUBFIELDSMAX  	_T("Too many subfields (%ld), maximum allowed is %ld")
#define IDS_TOTRECORDSMAX2      	_T("Transaction Type %s may contain at most %ld Type %ld and Type %ld records: it contains %ld")
#define IDS_TOTRECORDSMIN2      	_T("Transaction Type %s must contain at least %ld Type %ld and Type %ld records: it only contains %ld.")
#define IDS_FLAGFIELDERROR			_T("Field %s (%s): %s")
// Logfile entries
#define IDS_LOGTRANSNEW				_T("[CIWTransaction::New] Result %d")
#define IDS_LOGTRANSREAD			_T("[CIWTransaction::ReadTransactionFile] Path %s, Result %d")
#define IDS_LOGTRANSREADMEM			_T("[CIWTransaction::ReadTransactionFileMem] Result %d")
#define IDS_LOGTRANSRECORDSINFILE	_T("[CIWTransaction::GetRecords] Records in file %d")
#define IDS_LOGTRANSREADRECORDTYPE	_T("[CIWTransaction::GetRecords] Read record type %d, result %d")
#define	IDS_LOGTRANSREADRECORDFAILED _T("[CIWTransaction::GetRecords] Read record FAILED. File %s, type %d, result %d")
#define	IDS_LOGTRANSADDRECORD		_T("[CIWTransaction::AddRecord] Type %d, Index %d")
#define IDS_LOGTRANSREMOVEITEM		_T("[CIWTransaction::RemoveItem] (%ld, %ld, %ld, %ld, %ld)")
#define IDS_LOGTRANSGETIMAGE		_T("[CIWTransaction::GetImage] Type %d, Index %d, Result %d")
#define IDS_LOGTRANSSETIMAGE		_T("[CIWTransaction::SetImage] Type %d, Index %d, Input format %s, Result %d")
#define IDS_LOGTRANSWRITEBINARY		_T("[CIWTransaction::WriteBinary] Write record FAILED. File %s, result %d")
#define IDS_LOGTRANSWRITEMEM		_T("[CIWTransaction::WriteMem] Write record to memory FAILED. Result %d")
#define IDS_LOGTRANSWRITEXML		_T("[CIWTransaction::WriteXML] WriteXML FAILED. File %s, result %d")
#define IDS_LOGTRANSDEBUGSTART		_T("------------------------------- %s")
#define IDS_LOGTRANSDEBUGEND		_T("-------------------------------")
#define IDS_LOGTRANSDEBUGCONTENTS1	_T("CONTENTS: (01) %02d items")
#define IDS_LOGTRANSDEBUGCONTENTS2	_T("CONTENTS: (%02d) %02s %02s")
#define IDS_LOGTRANSDEBUGRECORD		_T("Record %02d, Type %02d")
#define IDS_LOGTRANSDEBUGERROR		_T("error")
#define IDS_LOGFIELDGETWRITELEN		_T("[CNISTField::GetWriteLen] RecordType %d, Field %d, Len %ld")
#define IDS_LOGFIELDWRITE			_T("[CNISTField::Write] RecordType %d, Field %d")
#define	IDS_LOGRECINITNEW			_T("[CNISTRecord::InitializeNewRecord] RecordType %d")
#define IDS_LOGRECREADRECLEN		_T("[CNISTRecord::ReadLogicalRecordLen] RecordType %d, Index %d, Length %ld")
#define IDS_LOGRECREADREC			_T("[CNISTRecord::ReadRecord] RecordType %d")
#define IDS_LOGRECREADRECBIN		_T("[CNISTRecord::ReadBinaryRecord] RecordType %d")
#define IDS_LOGRECSETITEM			_T("[CNISTRecord::SetItem] (%ld, %ld, %ld): %s")
#define IDS_LOGRECGETRECLEN			_T("[CNISTRecord::GetRecordLen] Recordtype %d, Len %ld")
#define IDS_LOGRULESETDATA			_T("[CRuleObj::SetData] Error: %s")
#define IDS_LOGRULEDUMPOBJ1			_T("[CRuleObj::DumpObject] ==> MNU: %s, Location: %s, chartype %s, len min %ld, max %ld, occ min %ld, max %ld desc(%s) sca(%s) date(%s) map(%s)")
#define IDS_LOGRULEDUMPOBJ2			_T("[CRuleObj::DumpObject] %s, TOT's: %s")
#define IDS_LOGRULESETTRANS			_T("[CRuleObj::SetTransactions] TOT = %s")
#define IDS_LOGRULETESTREGEX		_T("[CRuleObj::TestRegEx] Exception thrown. %s. Maybe an invalid mask defined.")
#define IDS_LOGVERLOADRULES			_T("[CIWVerification::LoadRules] Loc: %s, MNU: %s, Type: %s, Size: %s, Occ: %s")
#define IDS_LOGVERREADLINE			_T("[CIWVerification::ReadLine] exception thrown")
#define IDS_LOGVERGETRULE			_T("[CIWVerification::GetRule] exception thrown")
#define IDS_LOGVERSKIPCOMMENTS		_T("[CIWVerification::SkipComments] exception thrown")
#define IDS_LOGVERGETTRANSLIST		_T("[CIWVerification::GetTransactionList] exception thrown")
#define IDS_LOGVERGETRANGETOKEN		_T("[CIWVerification::GetRangeToken] exception thrown")
#define IDS_LOGVERGETNEXTTOKEN		_T("[CIWVerification::GetNextToken] exception thrown")
#define IDS_LOGVERREADVERFILE		_T("[CIWVerification::ReadVerificationFile] Error reading verification file %s")
#define IDS_LOGVERLOADTOTDEFS		_T("[CIWVerification::LoadTOTDefinitions] %ld TOT definitions in file")
#define IDS_LOGVERLOADTOTDEFSDBG	_T("[CIWVerification::LoadTOTDefinitions] (%s, %s)")
#define IDS_LOGVERDBGOUTDELIM		_T("%%%%%%%%%%%%%%%%%%%%")
#define IDS_LOGVERDBGOUTMIN			_T("%ld:%s ")
#define IDS_LOGVERDBGOUTMINMAX		_T("%ld:%s-%s ")
#define IDS_LOGVERDBGOUTFIELD		_T("%s\t%s\t%3s Len(%2ld..%2ld) Occ(%2ld..%2ld) Desc(%s) sca(%s) date(%s) map(%s) Trans %s\n")
#define IDS_LOGVERVERIFYTRANS		_T("[CIWVerification::VerifyTransaction] Applying %s\t%s\t%3s Len(%2ld..%2ld) Occ(%2ld..%2ld) Trans %s")
#define IDS_LOGVERVERIFYTRANSERR	_T("[CIWVerification::VerifyTransaction] Invalid Location Form Type")
#define IDS_LOGVERVERIFYFIELDCHARS	_T("[CIWVerification::VerifyFieldChars] %s, unknown CharType: %s\n")

#endif
