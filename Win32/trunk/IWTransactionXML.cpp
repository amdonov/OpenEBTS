#include "stdafx.h"
#include "IWNISTErrors.h"
#include "IWTransaction.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "IWNIST.h"

int CIWTransaction::GetXML(BYTE **ppXML, BOOL bValidate, long *plLengthXML, char **pErr)
//
// Convert our mnemonic-based ANSI/NIST ITL 1-2007 format to the new ANSI/NIST 2-2008
// XML format using CodeSynthesis generated schema wrappers.
//
// Reference documents used:
//
// ANSI/NIST-ITL 1-2007:
// Data Format for the Interchange of Fingerprint, Facial, & Other Biometric Information - Part 1
// http://fingerprint.nist.gov/standard/
// http://fingerprint.nist.gov/standard/Approved-Std-20070427.pdf
//
// ANSI/NIST ITL 2-2008:
// XML Data Format for the Interchange of Fingerprint Facial, & Other Biometric Information – Part 2: XML Version
// http://fingerprint.nist.gov/standard/xml/index.html
// http://fingerprint.nist.gov/standard/Approved-XML-Std-20080828.pdf
//
//
// EBTS Ver 8.1
// http://www.fbibiospecs.org/fbibiometric/documents/EBTS_v8.1_11-24-08.pdf
// 
// EBTS 8.001 XML Information Exchange Package (Draft)
// http://www.fbibiospecs.org/fbibiometric/docs/EBTS-8001-XML_Draft1.zip
//
//
// CodeSynthesis C++/Tree Mapping User Manual
// http://codesynthesis.com/projects/xsd/documentation/cxx/tree/manual/
//
{
	int ret;
	bool bXMLPlatformInit = false;

	*pErr = NULL;

	try
	{
		// First initialize Xerces
		XMLPlatformUtils::Initialize(); bXMLPlatformInit = true;

		//**********************************************************
		//******* Type-1 Record ==> packageInformationRecord *******
		//**********************************************************
		auto_ptr <PackageInformationRecordType> packageInformationRecord;
		ret = GetXMLForType1Record(packageInformationRecord, pErr);
		if (ret != IW_SUCCESS) return ret;

		//**************************************************************
		//******* Type-2 Record ==> packageDescriptiveTextRecord *******
		//**************************************************************
		auto_ptr <PackageDescriptiveTextRecordType> packageDescriptiveTextRecord;
		ret = GetXMLForType2Record(packageDescriptiveTextRecord, pErr);
		if (ret != IW_SUCCESS) return ret;

		//**********************************************************************************
		//******* Type-3, Type-4, Type-5, Type-6, Type-7, Type-10, Type-13, Type-14, *******
		//******* Type-15, Type-16 and Type-17 Records ==> packageImageRecords       *******
		//**********************************************************************************
		vector <PackageImageRecordType> packageImageRecords;
		ret = GetXMLForImageTypeRecords(packageImageRecords, pErr);
		if (ret != IW_SUCCESS) return ret;

		//*******************************************************************************
		//******* The whole package ==> <NISTBiometricInformationExchangePackage> *******
		//*******************************************************************************
		NISTBiometricInformationExchangePackageType package(packageInformationRecord);	// Create package with Type-1 Record
		package.PackageDescriptiveTextRecord().push_back(packageDescriptiveTextRecord);	// Add Type-2 Record
		for (unsigned int i=0; i<packageImageRecords.size(); i++)
		{
			package.PackageDataRecord().push_back(packageImageRecords.at(i));			// Add image type record
		}

		//***********************************************
		//******* Now we output the XML to memory *******
		//***********************************************

		// Setup all namespace mappings
		xml_schema::namespace_infomap map;
		map["ebts"].schema = "fbi_ebts/fbi_ebts_beta_1.0.0.xsd";
		map["ebts"].name = "http://cjis.fbi.gov/fbi_ebts/beta_1.0.0";
		map["itl"].name = "http://biometrics.nist.gov/standard/2-2008";
		map["ansi-nist"].name = "http://niem.gov/niem/ansi-nist/2.0";
		map["j"].name = "http://niem.gov/niem/domains/jxdm/4.0";
		map["nc"].name = "http://niem.gov/niem/niem-core/2.0";
		map["s"].name = "http://niem.gov/niem/structures/2.0";
		map["xsd"].name = "http://www.w3.org/2001/XMLSchema";

		// Serialize XML to DOM, so we can traverse it to remove empty elements
		::xml_schema::dom::auto_ptr <xercesc::DOMDocument> packageDOM(NISTBiometricInformationExchangePackage(package,  map));

		// Now remove the empty elements
		RemoveEmptyElementsFromDOM(packageDOM->getDocumentElement());

		// Due to a problem with the way the FBI defined all the various PackageImageRecordType substitution groups,
		// PackageCBEFFBiometricDataRecord, all PackageImageRecord elements come out with PackageCBEFFBiometricDataRecord
		// tags (this is the last substitution defined in the set of 13 substitutions). Since Code Synthesis does not let
		// us pick and choose which one we want (since by schema definition they are equivalent!) we need to replace them
		// manually, based on the RecordCategoryCode tag (=Record Type) within that element.
		SetProperPackageImageRecordSubstitutions(*packageDOM, packageDOM->getDocumentElement());

		if (bValidate)
		{
			// If there are any validation issues we will catch the exception below and return the set of warnings
			TCHAR					szSchemaPath[_MAX_PATH];
			TCHAR					szSchemaDescriptor[_MAX_PATH+10] = "file:///";
			xml_schema::properties	props;

			GetSchemaPath(szSchemaPath);
			strcat_s(szSchemaDescriptor, _MAX_PATH+10, szSchemaPath);
			props.schema_location("http://cjis.fbi.gov/fbi_ebts/beta_1.0.0", szSchemaDescriptor);

			// Validation happens automatically when converting from DOM to the CodeSynthesis objects
			auto_ptr <NISTBiometricInformationExchangePackageType> packageValidation(NISTBiometricInformationExchangePackage(packageDOM, 0, props));
		}

		// Now serialize from DOM to a memory buffer, via Xerces' FormatTarget wrapper
		MemBufFormatTarget ft;
		SerializeDOMtoXMLFormatTarget(ft, *packageDOM);

		// Finally allocate our output buffer and copy the XML into it
		*plLengthXML = ft.getLen();
		*ppXML = (BYTE*)malloc(*plLengthXML);
		memcpy(*ppXML, ft.getRawBuffer(), *plLengthXML);
	}
	catch (xml_schema::exception&)
	{
		HandleXMLSchemaException(pErr);
		if (bXMLPlatformInit) XMLPlatformUtils::Terminate();
		return IW_ERR_WRITING_FILE;
	}

	if (bXMLPlatformInit) XMLPlatformUtils::Terminate();

	return IW_SUCCESS;
}

void CIWTransaction::SerializeDOMtoXMLFormatTarget(xercesc::XMLFormatTarget& target, const xercesc::DOMDocument& doc,
												   const std::string& encoding/*= "UTF-8"*/)
//
// Based on http://wiki.codesynthesis.com/Tree/FAQ, but adapted to work with the
// newer DOMLSSerializer instead of the no-longer-existing DomWriter.
//
{
	using namespace xercesc;
	namespace xml = xsd::cxx::xml;
	namespace tree = xsd::cxx::tree;

	const XMLCh ls_id [] = {chLatin_L, chLatin_S, chNull};

	// Get an implementation of the Load-Store (LS) interface.
	//
	DOMImplementation* impl(DOMImplementationRegistry::getDOMImplementation(ls_id));

	// Create a DOMLSSerializer.
	//
	xml::dom::auto_ptr<DOMLSSerializer> writer(impl->createLSSerializer ());

	DOMConfiguration* conf(writer->getDomConfig ());

	// Set error handler.
	//
	tree::error_handler<char> eh;
	xml::dom::bits::error_handler_proxy<char> ehp (eh);
	conf->setParameter(XMLUni::fgDOMErrorHandler, &ehp);

	// Set some nice features if the serializer supports them.
	//
	if (conf->canSetParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true))
		conf->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true);
	if (conf->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
		conf->setParameter (XMLUni::fgDOMWRTFormatPrettyPrint, true);
	//if (conf->canSetParameter(XMLUni::fgDOMXMLDeclaration, true))
	//	conf->setParameter(XMLUni::fgDOMXMLDeclaration, true);

	xml::dom::auto_ptr<DOMLSOutput> out(impl->createLSOutput());
	out->setEncoding(xml::string (encoding).c_str());
	out->setByteStream(&target);

	bool ret = writer->write(&doc, out.get());

	eh.throw_if_failed<tree::serialization<char> > ();
}

void CIWTransaction::RemoveEmptyElementsFromDOM(DOMElement* e)
// Performs one pass of DOM to recursively locate and remove empty elements.
{

	for (DOMNode* n = e->getFirstChild(); n != 0; )
	{
		if (n->getNodeType () == DOMNode::ELEMENT_NODE)
		{
			DOMElement* ne = static_cast<DOMElement*> (n);
			n = n->getNextSibling();

			if (ne->getFirstChild () != 0)
			{
				RemoveEmptyElementsFromDOM(ne);
			}

			if (ne->getFirstChild () == 0 && ne->getAttributes()->getLength () == 0)
			{
				e->removeChild(ne);
				ne->release ();
			}
		}
		else
		{
			n = n->getNextSibling ();
		}
	}
}

void CIWTransaction::SetProperPackageImageRecordSubstitutions(::xercesc::DOMDocument& d, DOMElement* e)
// Performs one pass of DOM to recursively locate and replace itl:PackageCBEFFBiometricDataRecord tags
// with the appropriate tag from the substitution group as defined by FBI, for that record type.
// The necessity for this arrises from the fact that NIST-ITL have defined a variety of substitution groups
// for the type PackageImageRecordType, so that any one substitution is valid according to XML/XSD. But
// the authors of ANSI/NIST-ITL 2-2008 intended there to be *specific* tags based on the Record-Type.
{
	for (DOMNode* n = e->getFirstChild(); n != 0; )
	{
		if (n->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			DOMElement* ne = static_cast<DOMElement*> (n);

			if (wcscmp(ne->getNodeName(),  L"itl:PackageCBEFFBiometricDataRecord") == 0)
			{
				OutputDebugString("found itl:PackageCBEFFBiometricDataRecord");
				// To know what we need to replace this with, we need to know the Record Type
				// represented, which is in the <ansi-nist:RecordCategoryCode> element.
				DOMNodeList* nlist = ne->getChildNodes();
				long recordType = -1;
				for (unsigned int i = 0; i < nlist->getLength(); i++)
				{
					DOMNode* node = nlist->item(i);
					DOMElement* elem = static_cast<DOMElement*>(node);
					if (wcscmp(elem->getNodeName(),  L"ansi-nist:RecordCategoryCode") == 0)
					{
						// Current node is <ansi-nist:RecordCategoryCode>, we want it's child note
						// (which will the text-node containing the data).
						node = node->getFirstChild();
						elem = static_cast<DOMElement*>(node);
						const XMLCh* val = elem->getNodeValue();
						recordType = _wtol(val);
						break;
					}
				}
				switch (recordType)
				{
					case  3: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageLowResolutionGrayscaleImageRecord");
							break;
					case  4: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageHighResolutionGrayscaleImageRecord");
							break;
					case  5: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageLowResolutionBinaryImageRecord");
							break;
					case  6: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageHighResolutionBinaryImageRecord");
							break;
					case  7: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageUserDefinedImageRecord");
							break;
					case  8: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageSignatureImageRecord");
							break;
					case 10: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageFacialAndSMTImageRecord");
							break;
					case 13: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageLatentImageRecord");
							break;
					case 14: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageFingerprintImageRecord");
							break;
					case 15: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackagePalmprintImageRecord");
							break;
					case 16: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageUserDefinedTestingImageRecord");
							break;
					case 17: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageIrisImageRecord");
							break;
					case 99: d.renameNode(n, ne->getNamespaceURI(), L"itl:PackageCBEFFBiometricDataRecord");
							break;
					default:
							;// undefined, so we have an error case: let's just leave the element un-renamed
				}
			}
			else
			{
				SetProperPackageImageRecordSubstitutions(d, ne);
			}
		}
		n = n->getNextSibling();
	}
}

int CIWTransaction::ReturnError(const char *szErr, char **pErr)
{
	long lLen;

	lLen = _tcslen(szErr)+1;
	*pErr = (char*)malloc(lLen);
	_tcscpy_s(*pErr, lLen, *pErr);
	return IW_ERR_WRITING_FILE;
}

BOOL CIWTransaction::DecomposeYYYYMMDD(const char *szDate, int *py, unsigned short *pm, unsigned short *pd)
// Convert YYYYMMDD into 3 numbers
{
	char szy[5];
	char szm[3];
	char szd[3];
	if (_tcslen(szDate) != 8) return FALSE;

	_tcsncpy_s(szy, 5, szDate, 4);   szy[4] = '\0';
	_tcsncpy_s(szm, 3, szDate+4, 2); szm[2] = '\0';
	_tcsncpy_s(szd, 3, szDate+6, 2); szd[2] = '\0';

	*py = _ttol(szy);
	*pm = (unsigned short)_ttol(szm);
	*pd = (unsigned short)_ttol(szd);

	return TRUE;
}

BOOL CIWTransaction::DecomposeT1_GMT(const char *szDate, int *py, unsigned short *pm, unsigned short *pd,
									 unsigned short *pzh, unsigned short *pzm, unsigned short *pzs)
// Convert YYYYMMDDHHMMSSZ into 6 numbers
{
	char szy[5];
	char szm[3];
	char szd[3];
	char szzh[3];
	char szzm[3];
	char szzs[3];
	if (_tcslen(szDate) != 15) return FALSE;

	_tcsncpy_s(szy, 5, szDate, 4);
	_tcsncpy_s(szm, 3, szDate+4, 2);
	_tcsncpy_s(szd, 3, szDate+6, 2);
	_tcsncpy_s(szzh, 3, szDate+8, 2);
	_tcsncpy_s(szzm, 3, szDate+10, 2);
	_tcsncpy_s(szzs, 3, szDate+12, 2);

	*py = _ttol(szy);
	*pm = (unsigned short)_ttol(szm);
	*pd = (unsigned short)_ttol(szd);
	*pzh = (unsigned short)_ttol(szzh);
	*pzm = (unsigned short)_ttol(szzm);
	*pzs = (unsigned short)_ttol(szzs);

	return TRUE;
}

BOOL CIWTransaction::DecomposeNAME(const char *szFullName, char szGivenName[256], char szMiddleName[256], char szSurName[256])
// Convert "Public, John Queue" into "John", "Queue", "Public"
{
	char *s;
	char *e;

	szGivenName[0] = '\0';
	szMiddleName[0] = '\0';
	szSurName[0] = '\0';

	s = (char*)szFullName;	// start
	e = strstr((char*)szFullName, ","); // end is at first comma
	if (e == NULL)
	{
		// unexpected, full name must have a comma
		return FALSE;
	}
	*e = _T('\0'); // null-terminate where comma was
	_tcscpy_s(szSurName, 256, s); // copy surname

	s = _tcsinc(e); // new start is one char beyond last end
	while (*s == ' ') s = _tcsinc(s); // skip over any space
	e = strstr((char*)s, " "); // end is at first space
	if (e != NULL)
	{
		*e = _T('\0'); // null-terminate where space was
		_tcscpy_s(szGivenName, 256, s); // copy given name

		s = _tcsinc(e); // new start is one char beyond last end
		while (*s == ' ') s = _tcsinc(s); // skip over any space
		_tcscpy_s(szMiddleName, 256, s); // copy middle name
	}
	else
	{
		// No other names, just given
		_tcscpy_s(szGivenName, 256, s); // copy given name
	}

	return TRUE;
}

BOOL CIWTransaction::DecomposeXXYY(const char *szAgeRange, int *pmin, int *pmax)
// Convert "XXYY" into XX and YY
{
	char szmin[3];
	char szmax[3];

	if (_tcslen(szAgeRange) != 4) return FALSE;

	_tcsncpy_s(szmin, 3, szAgeRange, 2);
	_tcsncpy_s(szmax, 3, szAgeRange+2, 2);

	*pmin = _ttol(szmin);
	*pmax = _ttol(szmax);

	return TRUE;
}

BOOL CIWTransaction::DecomposeXXXYYY(const char *szHeightRange, int *pmin, int *pmax)
// Convert "XXXYYY" into XXX and YYY
{
	char szmin[4];
	char szmax[4];

	if (_tcslen(szHeightRange) != 6) return FALSE;

	_tcsncpy_s(szmin, 4, szHeightRange, 3);
	_tcsncpy_s(szmax, 4, szHeightRange+3, 3);

	*pmin = _ttol(szmin);
	*pmax = _ttol(szmax);

	return TRUE;
}

BOOL CIWTransaction::DecomposeTABSEP3(const char *sz, char sz1[256], char sz2[256], char sz3[256])
// Convert "aaa\tbbb\tccc" into "aaa", "bbb" and "ccc"
{
	char *s;
	char *e;

	sz1[0] = '\0';
	sz2[0] = '\0';
	sz3[0] = '\0';

	s = (char*)sz;	// start
	e = strstr(s, "\t"); // end is at first tab
	if (e == NULL)
	{
		// unexpected, must have a tab
		return FALSE;
	}
	*e = _T('\0'); // null-terminate where comma was
	_tcscpy_s(sz1, 256, s); // copy string #1

	s = _tcsinc(e); // new start is one char beyond last end

	e = strstr(s, "\t"); // end is at second tab

	*e = _T('\0'); // null-terminate where tab was
	_tcscpy_s(sz2, 256, s); // copy string #2

	s = _tcsinc(e); // new start is one char beyond last end
	_tcscpy_s(sz3, 256, s); // copy string #3

	return TRUE;
}

#define MAXERRSIZE 4096

BOOL CIWTransaction::HandleXMLSchemaException(char **pErr)
{
	char	szErr[MAXERRSIZE];
	long	lTotalLen;
	long	lLen;

	*pErr = NULL;

	try
	{
		throw;
	}
	catch (const xml_schema::parsing& e)
	{
		// Special case, we bundle multiple error messages into one long string
		lTotalLen = 0;
		for (unsigned int i=0; i<e.diagnostics().size(); i++)
		{
			xml_schema::error err(e.diagnostics().at(i));
			_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Parsing error: %s (id %s, severity %d, line %d, column %d)\n", 
						err.message().c_str(), err.id().c_str(), err.severity(), err.line(), err.column());
			OutputDebugString(szErr);

			lLen = strlen(szErr);
			lTotalLen += lLen;
			if (lTotalLen < MAXERRSIZE)
			{
				*pErr = (char*)realloc(*pErr, lTotalLen+1);
				if (i == 0) *pErr[0] = 0;
				strcat_s(*pErr, lTotalLen+1, szErr);
			}
			else
			{
				break; // no more errors can fit!
			}
		}
		return TRUE;
	}
 	catch (const xml_schema::expected_element& e)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Expected element \"%s\" in namespace \"%s\"\n", e.name().c_str(), e.namespace_().c_str());
	}
	catch (const xml_schema::unexpected_element& e)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Unexpected element \"%s\" in namespace \"%s\", expected element \"%s\" in namespace \"%s\"\n",
					e.expected_name().c_str(), e.expected_namespace().c_str(),
					e.encountered_name().c_str(), e.encountered_namespace().c_str());
	}
	catch (const xml_schema::expected_attribute& e)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Expected attribute \"%s\" in namespace \"%s\"\n", e.name().c_str(), e.namespace_().c_str());
	}
	catch (const xml_schema::unexpected_enumerator& e)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Unexpected enumerator \"%s\"\n", e.enumerator().c_str());
	}
	catch (const xml_schema::expected_text_content&)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Non-text content was encountered during parsing\n");
	}
	catch (const xml_schema::no_prefix_mapping& e)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "No prefix mapping for prefix \"%s\"\n", e.prefix());
	}
	catch (const xml_schema::no_type_info& e)
	{
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "No type info for element \"%s\" in namespace \"%s\"\n", e.type_name().c_str(), e.type_namespace().c_str());
	}	
	catch (const xml_schema::exception& e)
	{
		// Last chance "catchall"
		_snprintf_s(szErr, MAXERRSIZE, MAXERRSIZE, "Unknown expection caught; %s\n", e.what());
	}

	lLen = strlen(szErr);
	*pErr = (char*)malloc(lLen+1);
	strcpy_s(*pErr, lLen+1, szErr);
	
	return TRUE;
}

BOOL CIWTransaction::GetSchemaPath(char szSchema[_MAX_PATH])
// Return full path to main schame file. Ensure that there are no spaces by replacing them with '%20'.
{
	/*
	HKEY	hKey = NULL;
	DWORD	dwSize = _MAX_PATH;
	BOOL	bDone;
	int		iLen;
	char	szSchemaR[_MAX_PATH];

	// Obtain ProgramFilesDir Key Value
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
      return FALSE;
	}
	else
	{
		if(RegQueryValueEx(hKey, _T("ProgramFilesDir"), NULL, NULL, (LPBYTE)szSchema, &dwSize) != ERROR_SUCCESS)
		{
			return FALSE;
		}
		RegCloseKey(hKey);
	}

	strcat_s(szSchema, _MAX_PATH, "\\ImageWare Systems\\IWNIST\\fbi_ebts\\fbi_ebts_beta_1.0.0.xsd\"");

	//strcpy_s(szSchema, _MAX_PATH, "C:\\Program Files\\ImageWare Systems\\IWNIST\\fbi_ebts\\fbi_ebts_beta_1.0.0.xsd"); //DEBUG
	*/

	// Xerces has big problems when then path has spaces in it, so for now we force it in this folder.
	strcpy_s(szSchema, _MAX_PATH, "C:\\IWNIST\\fbi_ebts\\fbi_ebts_beta_1.0.0.xsd");

	return TRUE;
}

int CIWTransaction::GetXMLForType1Record(auto_ptr <PackageInformationRecordType> &packageInformationRecord, char **pErr)
{
	const char	*pData;
	double		dVal;
	CNISTRecord *pRec1 = GetRecord(RECORD_TYPE1, 1);

	//*************************************************
	//******* T1_DAT (DATE) ==> transactionDate *******
	//*************************************************
	//
	//<ansi-nist:TransactionDate>
	//    <nc:Date>2007-07-06</nc:Date>
    //</ansi-nist:TransactionDate>
    //
    int y;
	unsigned short m;
	unsigned short d;
	ansinist::TransactionType::TransactionDate_type transactionDate;
	if (pRec1->FindItem(TYPE1_DAT, 1, 1, &pData) == IW_SUCCESS)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date											transactionDateTempTemp(y, m, d);
			::proxyxsd::date											transactionDateTemp(transactionDateTempTemp);
			::niemcore::DateType::DateRepresentation_sequence			dateRepresentationSequence;

			dateRepresentationSequence.push_back(transactionDateTemp);
			transactionDate.DateRepresentation(dateRepresentationSequence);
		}
		else
		{
			//failed decomposition (todo)
		}
	}
	else
	{
		return ReturnError("Missing T1_DAT", pErr);
	}

	//********************************************************************************************
	//******* T1_DAI (DESTINATION AGENCY IDENTIFIER)==> transactionDestinationOrganization *******
	//********************************************************************************************
	//
	//<ansi-nist:TransactionDestinationOrganization>
	//    <nc:OrganizationIdentification>
	//        <nc:IdentificationID>WI013415Y</nc:IdentificationID>
	//    </nc:OrganizationIdentification>
	//  [ <nc:OrganizationName>WI Crime Information Bureau</nc:OrganizationName> ] <-- not supported in 1-2007
	//</ansi-nist:TransactionDestinationOrganization>
	//
	niemcore::OrganizationType transactionDestinationOrganization;
	if (pRec1->FindItem(T1_DAI, 1, 1, &pData) == IW_SUCCESS)
	{
		niemcore::IdentificationType::IdentificationID_type				identificationID;
		niemcore::IdentificationType::IdentificationID_sequence			identificationIDSequence;
		niemcore::OrganizationType::OrganizationIdentification_type		organizationIdentification;
		niemcore::OrganizationType::OrganizationIdentification_sequence organizationIdentificationSequence;

		identificationID = pData;
		identificationIDSequence.push_back(identificationID);
		organizationIdentification.IdentificationID(identificationIDSequence);
		organizationIdentificationSequence.push_back(organizationIdentification);
		transactionDestinationOrganization.OrganizationIdentification(organizationIdentificationSequence);
	}
	else
	{
		return ReturnError("Missing T1_DAI", pErr);
	}

	//********************************************************************************************
	//******* T1_ORI (ORIGINATING AGENCY IDENTIFIER) ==> transactionOriginatingOrganization ******
	//********************************************************************************************
	//
	//<ansi-nist:TransactionOriginatingOrganization>
	//    <nc:OrganizationIdentification>
	//        <nc:IdentificationID>WI013415Y</nc:IdentificationID>
	//    </nc:OrganizationIdentification>
	//  [ <nc:OrganizationName>WI Crime Information Bureau</nc:OrganizationName> ] <-- not supported in 1-2007
	//</ansi-nist:TransactionOriginatingOrganization>
	//
	ansinist::TransactionType::TransactionOriginatingOrganization_type transactionOriginatingOrganization;
	if (pRec1->FindItem(T1_ORI, 1, 1, &pData) == IW_SUCCESS)
	{
		niemcore::IdentificationType::IdentificationID_type				identificationID;
		niemcore::IdentificationType::IdentificationID_sequence			identificationIDSequence;
		niemcore::OrganizationType::OrganizationIdentification_type		organizationIdentification;
		niemcore::OrganizationType::OrganizationIdentification_sequence organizationIdentificationSequence;

		identificationID = pData;
		identificationIDSequence.push_back(identificationID);
		organizationIdentification.IdentificationID(identificationIDSequence);
		organizationIdentificationSequence.push_back(organizationIdentification);
		transactionOriginatingOrganization.OrganizationIdentification(organizationIdentificationSequence);
	}
	else
	{
		return ReturnError("Missing T1_ORI", pErr);
	}

	//****************************************************************************************
	//******* T1_TCN (TRANSACTION CONTROL NUMBER) ==> transactionControlIdentification *******
	//****************************************************************************************
	//
	//<ansi-nist:TransactionControlIdentification>
	//    <nc:IdentificationID>56839</nc:IdentificationID>
	//</ansi-nist:TransactionControlIdentification>
	//
	ansinist::TransactionType::TransactionControlIdentification_type transactionControlIdentification;
	if (pRec1->FindItem(T1_TCN, 1, 1, &pData) == IW_SUCCESS)
	{
		niemcore::IdentificationType::IdentificationID_type				identificationID;
		niemcore::IdentificationType::IdentificationID_sequence			identificationIDSequence;
		niemcore::OrganizationType::OrganizationIdentification_type		organizationIdentification;
		niemcore::OrganizationType::OrganizationIdentification_sequence organizationIdentificationSequence;

		identificationID = pData;
		identificationIDSequence.push_back(identificationID);
		transactionControlIdentification.IdentificationID(identificationIDSequence);
	}
	else
	{
		return ReturnError("Missing T1_TCN", pErr);
	}

	//**********************************************************************************************
	//******* T1_NSR (NATIVE SCANNING RESOLUTION) +                                          *******
	//******* T1_NTR (NOMINAL TRANSMITTING RESOLUTION) ==> transactionImageResolutionDetails *******
	//**********************************************************************************************
	//
	//<ansi-nist:TransactionImageResolutionDetails>
	//    <ansi-nist:NativeScanningResolutionValue>19.69</ansi-nist:NativeScanningResolutionValue>
	//    <ansi-nist:NominalTransmittingResolutionValue>19.69</ansi-nist:NominalTransmittingResolutionValue>
	//</ansi-nist:TransactionImageResolutionDetails>
	//
	ansinist::TransactionType::TransactionImageResolutionDetails_type transactionImageResolutionDetails(0,0);
	if (pRec1->FindItem(T1_NSR, 1, 1, &pData) == IW_SUCCESS)
	{
		dVal = atof(pData);
		transactionImageResolutionDetails.NativeScanningResolutionValue(dVal);
	}
	else
	{
		return ReturnError("Missing T1_NSR", pErr);
	}
	if (pRec1->FindItem(T1_NTR, 1, 1, &pData) == IW_SUCCESS)
	{
		dVal = atof(pData);
		transactionImageResolutionDetails.NominalTransmittingResolutionValue(dVal);
	}
	else
	{
		return ReturnError("Missing T1_NTR", pErr);
	}

	//************************************************************************
	//******* T1_VER (VERSION NUMBER) ==> transactionMajorVersionValue *******
	//*******							+ transactionMinorVersionValue *******
	//************************************************************************
	//
	//<ansi-nist:TransactionMajorVersionValue>04</ansi-nist:TransactionMajorVersionValue>
	//<ansi-nist:TransactionMinorVersionValue>00</ansi-nist:TransactionMinorVersionValue>
	//
	ansinist::TransactionType::TransactionMajorVersionValue_type transactionMajorVersionValue(0);
	ansinist::TransactionType::TransactionMinorVersionValue_type transactionMinorVersionValue(0);
	if (pRec1->FindItem(T1_VER, 1, 1, &pData) == IW_SUCCESS)
	{
		// Stored as "MMmm"
		TCHAR sz[5];
		long lMinor;
		long lMajor;

		_tcscpy_s(sz, 5, pData);
		lMinor = _ttol(sz+2);
		sz[2] = '\0';
		lMajor = _ttol(sz);
		transactionMajorVersionValue = lMajor;
		transactionMinorVersionValue = lMinor;
	}
	else
	{
		return ReturnError("Missing T1_VER", pErr);
	}

	//************************************************************************
	//******* T1_TOT (TYPE OF TRANSACTION) ==> transactionCategoryCode *******
	//************************************************************************
	//
	//<ansi-nist:TransactionCategoryCode>CAR</ansi-nist:TransactionCategoryCode>
	//
	ansinist::TransactionType::TransactionCategoryCode_type transactionCategoryCode("");
	if (pRec1->FindItem(T1_TOT, 1, 1, &pData) == IW_SUCCESS)
	{
		transactionCategoryCode = pData;
	}
	else
	{
		return ReturnError("Missing T1_TOT", pErr);
	}

	//*****************************************************************************
	//******* T1_GMT (GREENWICH MEAN TIME, OPTIONAL) ==> transactionUTCDate *******
	//*****************************************************************************
	//
	//<ansi-nist:TransactionUTCDate>
	//	<nc:DateTime>2005-11-05T05:25:00Z</nc:DateTime>
	//</ansi-nist:TransactionUTCDate>
	//
	unsigned short zh;
	unsigned short zm;
	unsigned short zs;
	ansinist::TransactionType::TransactionUTCDate_type	transactionUTCDate;
	if (pRec1->FindItem(TYPE1_GMT, 1, 1, &pData) == IW_SUCCESS &&
		DecomposeT1_GMT(pData, &y, &m, &d, &zh, &zm, &zs))
	{
		::xml_schema::date								transactionDateTempTemp(y, m, d, zh, zm);
		proxyxsd::date									transactionDateTemp(transactionDateTempTemp);
		niemcore::DateType::DateRepresentation_sequence dateRepresentationSequence;

		dateRepresentationSequence.push_back(transactionDateTemp);
		transactionUTCDate.DateRepresentation(dateRepresentationSequence);
	}
	else
	{
		// Not a problem, this field is optional
	}

	//********************************************************************************************************
	//******* T1_DCS (DIRECTORY CHARACTER SET, OPTIONAL) ==> transactionCharacterSetDirectory_sequence *******
	//********************************************************************************************************
	//
	//<ansi-nist:TransactionCharacterSetDirectory>
	//    <ansi-nist:CharacterSetCommonNameCode>ASCII 7-bit English</ansi-nist:CharacterSetCommonNameCode>
	//    <ansi-nist:CharacterSetIndexCode>000</ansi-nist:CharacterSetIndexCode>
	//    <ansi-nist:CharacterSetVersionIdentification>
	//        <nc:IdentificationID>000</nc:IdentificationID>
	//    </ansi-nist:CharacterSetVersionIdentification>
	//</ansi-nist:TransactionCharacterSetDirectory>
	//
	// TODO(?): support more than one <TransactionCharacterSetDirectory>
	//
	const char* pCSN = NULL;
	const char* pCSI = NULL;
	const char* pCSV = NULL;
	::ansinist::TransactionType::TransactionCharacterSetDirectory_sequence					transactionCharacterSetDirectory_sequence;
	pRec1->FindItem(TYPE1_DCS, 1, 1, &pCSI);
	pRec1->FindItem(TYPE1_DCS, 1, 2, &pCSN);
	pRec1->FindItem(TYPE1_DCS, 1, 3, &pCSV);
	if (pCSI && pCSN)
	{
		::ansinist::TransactionCharacterSetDirectoryType::CharacterSetIndexCode_type			characterSetIndexCode(pCSI);
		auto_ptr<::ansinist::CSNCodeType>														cSNCode;
 
		// Must convert "common name" string to "common name code" enum. We ignore the passed in
		// CSN altoghether and use the CSI directly, which is easier.
		if (_tcsicmp(pCSI, "000") == 0)
		{
			cSNCode.reset(new ::ansinist::CSNCodeType(::ansinist::CSNCodeType::ASCII_7_bit_English));
		}
		else if (_tcsicmp(pCSI, "001") == 0)
		{
			cSNCode.reset(new ::ansinist::CSNCodeType(::ansinist::CSNCodeType::ASCII_8_bit_Latin));
		}
		else if (_tcsicmp(pCSI, "002") == 0)
		{
			cSNCode.reset(new ::ansinist::CSNCodeType(::ansinist::CSNCodeType::UNICODE_16_bit));
		}
		else if (_tcsicmp(pCSI, "003") == 0)
		{
			cSNCode.reset(new ::ansinist::CSNCodeType(::ansinist::CSNCodeType::UTF_8_bit));
		}
		else
		{
			//We have to set it to something, so set it to the common name passed in
			cSNCode.reset(new ::ansinist::CSNCodeType(pCSN));
		}

		::ansinist::TransactionType::TransactionCharacterSetDirectory_type						transactionCharacterSetDirectory(
																									*cSNCode,
																									characterSetIndexCode);
		if (pCSV)
		{
			niemcore::IdentificationType::IdentificationID_type								identificationID(pCSV);
			niemcore::IdentificationType::IdentificationID_sequence							identificationIDSequence;
			niemcore::IdentificationType													identification;
			identificationIDSequence.push_back(identificationID);
			identification.IdentificationID(identificationIDSequence);
			ansinist::TransactionCharacterSetDirectoryType::CharacterSetVersionIdentification_type	characterSetVersionIdentification(identification);
			transactionCharacterSetDirectory.CharacterSetVersionIdentification(characterSetVersionIdentification);
		}
		transactionCharacterSetDirectory_sequence.push_back(transactionCharacterSetDirectory);
	}

	//******************************************************************
	//******* T1_CNT (FILE CONTENT)==> transactionContentSummary *******
	//******************************************************************
	//
	//<ansi-nist:TransactionContentSummary>
	//    <ansi-nist:ContentFirstRecordCategoryCode>1</ansi-nist:ContentFirstRecordCategoryCode>
	//    <ansi-nist:ContentRecordCount>21</ansi-nist:ContentRecordCount>
	//    <ansi-nist:ContentRecordSummary>                                     }
	//        <ansi-nist:ImageReferenceIdentification>                         }
	//            <nc:IdentificationID>00</nc:IdentificationID>                } x N
	//        </ansi-nist:ImageReferenceIdentification>                        }
	//        <ansi-nist:RecordCategoryCode>02</ansi-nist:RecordCategoryCode>  }
	//    </ansi-nist:ContentRecordSummary>                                    }
	//</ansi-nist:TransactionContentSummary>
	//
	ansinist::TransactionContentSummaryType::ContentFirstRecordCategoryCode_type contentFirstRecordCategoryCode("1"); // Note: always "1"
	ansinist::TransactionContentSummaryType::ContentRecordCount_type			 contentRecordCount(0);
	int		   nCount;
	const char *pData1;
	const char *pData2;

	if (pRec1->FindItem(TYPE1_CNT, 1, 2, &pData) == IW_SUCCESS && pData)
	{
		nCount = atoi(pData);
		contentRecordCount = nCount;
	}
	else
	{
		return ReturnError("Missing T1_CNT", pErr);
	}
	ansinist::TransactionType::TransactionContentSummary_type transactionContentSummary(contentFirstRecordCategoryCode,
																						contentRecordCount);
	for (int i = 2; i <= nCount+1; i++)
	{
		if (pRec1->FindItem(TYPE1_CNT, i, 1, &pData1))
		{
			return ReturnError("Invalid contents in TYPE1_CNT", pErr);
		}
		if (pRec1->FindItem(TYPE1_CNT, i, 2, &pData2))
		{
			return ReturnError("Invalid contents in TYPE1_CNT", pErr);
		}

		// <ImageReferenceIdentification><IdentificationID>
		niemcore::IdentificationType::IdentificationID_type			   identificationID(pData2);
		niemcore::IdentificationType::IdentificationID_sequence		   identificationIDSequence;
		niemcore::IdentificationType								   identification;
		identificationIDSequence.push_back(pData1);
		identification.IdentificationID(identificationIDSequence);
		ansinist::ContentRecordType::ImageReferenceIdentification_type imageReferenceIdentification(identification);
		// <RecordCategoryCode>
		ansinist::ContentRecordType::RecordCategoryCode_type		   recordCategoryCode(pData1);

		// Add another <ContentRecordSummary> to the sequence
		ansinist::TransactionContentSummaryType::ContentRecordSummary_type contentRecordSummary(imageReferenceIdentification,
																								recordCategoryCode);
		transactionContentSummary.ContentRecordSummary().push_back(contentRecordSummary);
	}

	//*********************************************************
	//******* Type1 Record ==> packageInformationRecord *******
	//*********************************************************
	//
	//<PackageInformationRecord>
	//    <RecordCategoryCode>01</RecordCategoryCode>
	//    <Transaction>
	//
	PackageInformationRecordType::Transaction_type transaction(transactionDate,
															   transactionDestinationOrganization,
															   transactionOriginatingOrganization,
															   transactionControlIdentification,
															   transactionImageResolutionDetails,
															   transactionMajorVersionValue,
															   transactionMinorVersionValue,
															   transactionCategoryCode,
															   transactionContentSummary);
	transaction.TransactionUTCDate(transactionUTCDate);	// optional transactionUTCDate
	transaction.TransactionCharacterSetDirectory(transactionCharacterSetDirectory_sequence); // optional transactionCharacterSetDirectory
	PackageInformationRecordType::RecordCategoryCode_type	recordCategoryCode("01");

	packageInformationRecord.reset(new PackageInformationRecordType(recordCategoryCode, transaction));

	return IW_SUCCESS;
}

int CIWTransaction::GetXMLForType2Record(auto_ptr <PackageDescriptiveTextRecordType> &packageDescriptiveTextRecord, char **pErr)
//<itl:PackageDescriptiveTextRecord>
//    <ansi-nist:RecordCategoryCode>02</ansi-nist:RecordCategoryCode>
//    <ansi-nist:ImageReferenceIdentification>
//        <nc:IdentificationID>00</nc:IdentificationID>
//    </ansi-nist:ImageReferenceIdentification>
//    <itl:UserDefinedDescriptiveText>
//        <itl:ExampleDomainDefinedDescriptiveText>
//            <!-- Well-formed XML goes here.  Users may define a substitute element. -->
//        </itl:ExampleDomainDefinedDescriptiveText>
//        <itl:ExampleOtherDescriptiveText>
//            <!-- Well-formed XML goes here.  Users may define a substitute element. -->
//        </itl:ExampleOtherDescriptiveText>
//    </itl:UserDefinedDescriptiveText>
//</itl:PackageDescriptiveTextRecord>
{
	// Generic data containers
	const char		*pData;
	const char		*pData1;
	const char		*pData2;
    int				y;
	unsigned short  m;
	unsigned short  d;
	int				min;
	int				max;
	char			sz1[256];
	char			sz2[256];
	char			sz3[256];
	long			lVal;
	double			dVal;
	bool			bVal;

	//<ansi-nist:RecordCategoryCode>02</ansi-nist:RecordCategoryCode>
	::PackageDataRecordType::RecordCategoryCode_type				recordCategoryCode("02");

	//<ansi-nist:ImageReferenceIdentification>
	//    <nc:IdentificationID>00</nc:IdentificationID>
	//</ansi-nist:ImageReferenceIdentification>
	::niemcore::IdentificationType::IdentificationID_sequence		identificationIDSequence;
	::niemcore::IdentificationType									identification;
	identificationIDSequence.push_back("00");
	identification.IdentificationID(identificationIDSequence);
	::PackageDataRecordType::ImageReferenceIdentification_type		imageReferenceIdentification(identification);

	//<ebts:DomainDefinedDescriptiveText>
	::fbi_ebts::DomainDefinedDescriptiveTextType					domainDefinedDescriptiveText;
	//<ebts:RecordActivity>
	::fbi_ebts::RecordActivityType									recordActivity;
	//<ebts:RecordActivity><j:Arrest>
	::jxdm::ArrestType												arrest;
	BOOL															bHasArrest = FALSE;	// Avoid empty <j:Arrest/> tag
	//<ebts:RecordTransactionData> (Note: this may change if/when the EBTS IEP "fbi_ebts_beta_1.0.0.xsd" gets updated)
	::fbi_ebts::RecordTransactionDataType							recordTransactionData;
	//<ebts:RecordTransactionData><ebts:TransactionResponseData>
	::fbi_ebts::TransactionResponseDataType							transactionResponseData;
	//<ebts:RecordTransactionData><ebts:TransactionResponseData><ebts:TransactionCandidateList>
	::fbi_ebts::TransactionCandidateListType						transactionCandidateList;
	//<ebts:RecordSubject>
	::fbi_ebts::RecordSubjectType									recordSubject;
	//<ebts:RecordSubject><ebts:PersonFingerprintSet>
	::fbi_ebts::PersonFingerprintSetType							personFingerprintSet;
	//<ebts:RecordSubject><ebts:PersonEmploymentAssociation>
	::fbi_ebts::PersonEmploymentAssociationType						personEmploymentAssociation;

	//***********************************************
	//******* 2.003: T2_FFN (FBI File Number) *******
	//***********************************************
	//
	//<ebts:RecordActivity>
    //    <ebts:FBIFileNumber>123456789</ebts:FBIFileNumber>
	//
	if (Get("T2_FFN", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		long lNum = _ttol(pData);
		recordActivity.FBIFileNumber(lNum);
	}

	//*******************************************************
	//******* 2.004: T2_QDD (Transaction Query Depth) *******
	//*******************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionQueryDepthCode>S</ebts:TransactionQueryDepthCode>
	//
	if (Get("T2_QDD", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordTransactionData.TransactionQueryDepthCode(pData);
	}

	//**********************************************
	//******* 2.005: T2_RET (Retention Code) *******
	//**********************************************
	//
	//<ansi-nist:RecordRetentionIndicator>true</ansi-nist:RecordRetentionIndicator>
	//
	if (Get("T2_RET", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		bVal = (_tcsicmp(pData, "Y") == 0);
		::ansinist::RecordType::RecordRetentionIndicator_type		recordRetentionIndicator(bVal);

		domainDefinedDescriptiveText.RecordRetentionIndicator().push_back(recordRetentionIndicator);
	}

	//***************************************************
	//******* 2.006: T2_ATN (Attention Indicator) *******
	//***************************************************
	//
	//<nc:CaveatText>DET J Q PUBLIC</nc:CaveatText>
	//
	if (Get("T2_ATN", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		domainDefinedDescriptiveText.CaveatText().push_back(pData);
	}

	//********************************************
	//******* 2.007: T2_SCO (Send Copy To) *******
	//********************************************
	//
	//<ansi-nist:RecordForwardOrganizations>
	//    <nc:OrganizationOtherIdentification>
	//        <nc:IdentificationID>ORI-56789</nc:IdentificationID>
	//    </nc:OrganizationOtherIdentification>
	//
	if (Get("T2_SCO", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType											identification;
		::niemcore::OrganizationType											organization;
		::niemcore::OrganizationType::OrganizationOtherIdentification_sequence	organizationOtherIdentification_sequence;

		identification.IdentificationID().push_back(pData);
		organizationOtherIdentification_sequence.push_back(identification);
		organization.OrganizationOtherIdentification(organizationOtherIdentification_sequence);
		domainDefinedDescriptiveText.RecordForwardOrganizations().push_back(organization);
	}

	//**************************************************************
	//******* 2.009: T2_OCA (Originating Agency Case Number) *******
	//**************************************************************
	//
	//<ebts:RecordActivity>
	//    <nc:CaseTrackingID>CASE-123456789</nc:CaseTrackingID>
	//
	if (Get("T2_OCA", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordActivity.CaseTrackingID().push_back(pData);
	}

	//******************************************************************
	//******* 2.010: T2_CIN (Contributor Case Identifier Number) *******
	//******************************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:ContributorCaseIdentificationNumber>
	//        <ebts:ContributorCasePrefixIdentification>
	//            <nc:IdentificationID>CONCASE-PRE-12345</nc:IdentificationID>
	//        </ebts:ContributorCasePrefixIdentification>
	//        <ebts:ContributorCaseIdentification>
	//            <nc:IdentificationID>CONCASE-ID-12345</nc:IdentificationID>
	//        </ebts:ContributorCaseIdentification>
	//    </ebts:ContributorCaseIdentificationNumber>
	//
	pData1 = pData2 = NULL;
	if (Get("T2_CIN_PRE", &pData1, 1, 1) == IW_SUCCESS && pData1)
	{
		::niemcore::IdentificationType								identificationPrefix;
		identificationPrefix.IdentificationID().push_back(pData1);
		::fbi_ebts::ContributorCaseIdentificationNumberType			contributorCaseIdentificationNumber(identificationPrefix);

		if (Get("T2_CIN_ID", &pData2, 1, 1) == IW_SUCCESS && pData1) //optional
		{
			::niemcore::IdentificationType							identificationID;
			identificationID.IdentificationID().push_back(pData2);
			contributorCaseIdentificationNumber.ContributorCaseIdentification(identificationID);
		}

		recordActivity.ContributorCaseIdentificationNumber().push_back(contributorCaseIdentificationNumber);
	}

	//*********************************************************************
	//******* 2.011: T2_CIX (Contributor Case Identifier Extension) *******
	//*********************************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:ContributorCaseIdentificationExtensionNumber>1</ebts:ContributorCaseIdentificationExtensionNumber>
	//
	if (Get("T2_CIX", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		recordActivity.ContributorCaseIdentificationExtensionNumber().push_back(lVal);
	}

	//******************************************************
	//******* 2.012: T2_LCN (FBI Latent Case Number) *******
	//******************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:FBILatentCaseIdentification>
	//         <nc:IdentificationID>L0123456789</nc:IdentificationID>
	//    </ebts:FBILatentCaseIdentification>
	//
	if (Get("T2_LCN", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;

		identification.IdentificationID().push_back(pData);
		recordActivity.FBILatentCaseIdentification(identification);
	}

	//*********************************************************************
	//******* 2.013: T2_LCX (Contributor Case Identifier Extension) *******
	//*********************************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:FBILatentCaseExtensionNumber>1</ebts:FBILatentCaseExtensionNumber>
	//
	if (Get("T2_LCX", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		recordActivity.FBILatentCaseExtensionNumber(lVal);
	}

	//******************************************
	//******* 2.014: T2_FBI (FBI Number) *******
	//******************************************
	//
	//<ebts:RecordSubject>
	//    <j:PersonFBIIdentification>
	//        <nc:IdentificationID>FBI123456</nc:IdentificationID>
	//    </j:PersonFBIIdentification>
	//
	if (Get("T2_FBI", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;

		identification.IdentificationID().push_back(pData);
		recordSubject.PersonFBIIdentification().push_back(identification);
	}

	//***********************************************************
	//******* 2.015: T2_SID (State Identification Number) *******
	//***********************************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonStateIdentification>
	//        <nc:IdentificationID>FL34567890</nc:IdentificationID>
	//    </nc:PersonStateIdentification>
	//
	if (Get("T2_SID", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;

		identification.IdentificationID().push_back(pData);
		recordSubject.PersonStateIdentification().push_back(identification);
	}

	//******************************************************
	//******* 2.016: T2_SOC (Social Security Number) *******
	//******************************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonSSNIdentification>
	//        <nc:IdentificationID>111222333</nc:IdentificationID>
	//    </nc:PersonSSNIdentification>
	//
	if (Get("T2_SOC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;

		identification.IdentificationID().push_back(pData);
		recordSubject.PersonSSNIdentification().push_back(identification);
	}

	//*******************************************************************
	//******* 2.017: T2_MNU (Miscellaneous Identification Number) *******
	//*******************************************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonOtherIdentification>
	//        <nc:IdentificationID>AF-123456789012</nc:IdentificationID>
	//    </nc:PersonOtherIdentification>
	//
	if (Get("T2_MNU", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;

		identification.IdentificationID().push_back(pData);
		recordSubject.PersonOtherIdentification().push_back(identification);
	}

	//************************************
	//******* 2.018: T2_NAM (Name) *******
	//************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonName>
	//        <nc:PersonGivenName>ANTHONY</nc:PersonGivenName>
	//        <nc:PersonMiddleName>PAUL</nc:PersonMiddleName>
	//        <nc:PersonSurName>JONES</nc:PersonSurName>
	//    </nc:PersonName>
	//
	if (Get("T2_NAM", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeNAME(pData, sz1, sz2, sz3))
		{
			::niemcore::PersonNameType								personName;
			::niemcore::PersonNameType::PersonGivenName_sequence	personGivenName_sequence;
			::niemcore::PersonNameType::PersonMiddleName_sequence	personMiddleName_sequence;
			::niemcore::PersonNameType::PersonSurName_sequence		personSurNameName_sequence;

			personGivenName_sequence.push_back(sz1);
			personMiddleName_sequence.push_back(sz2);
			personSurNameName_sequence.push_back(sz3);

			if (_tcslen(sz1)) personName.PersonGivenName(personGivenName_sequence);
			if (_tcslen(sz2)) personName.PersonMiddleName(personMiddleName_sequence);
			if (_tcslen(sz3)) personName.PersonSurName(personSurNameName_sequence);

			recordSubject.PersonName().push_back(personName);
		}
		else
		{
			// Error, name parsing function failed!
		}
	}

	//***************************************
	//******* 2.019: T2_AKA (Aliases) *******
	//***************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonAlternateName>
	//        <nc:PersonGivenName>Mickey</nc:PersonGivenName>
	//        <nc:PersonMiddleName>Bob</nc:PersonMiddleName>
	//        <nc:PersonSurName>Mouse</nc:PersonSurName>
	//    </nc:PersonAlternateName>
	//
	if (Get("T2_AKA", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeNAME(pData, sz1, sz2, sz3))
		{
			::niemcore::PersonNameType								personName;
			::niemcore::PersonNameType::PersonGivenName_sequence	personGivenName_sequence;
			::niemcore::PersonNameType::PersonMiddleName_sequence	personMiddleName_sequence;
			::niemcore::PersonNameType::PersonSurName_sequence		personSurNameName_sequence;

			personGivenName_sequence.push_back(sz1);
			personMiddleName_sequence.push_back(sz2);
			personSurNameName_sequence.push_back(sz3);

			if (_tcslen(sz1)) personName.PersonGivenName(personGivenName_sequence);
			if (_tcslen(sz2)) personName.PersonMiddleName(personMiddleName_sequence);
			if (_tcslen(sz3)) personName.PersonSurName(personSurNameName_sequence);

			recordSubject.PersonAlternateName().push_back(personName);
		}
		else
		{
			// Error, name parsing function failed!
		}
	}

	//**********************************************
	//******* 2.020: T2_POB (Place of Birth) *******
	//**********************************************
	//
	//<ebts:RecordSubject>
	//    <j:PersonBirthPlaceCode>IT</j:PersonBirthPlaceCode>
	//
	if (Get("T2_POB", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi::POBCodeType											pOBCode(pData);
		recordSubject.PersonBirthPlaceCode().push_back(pOBCode);
	}

	//******************************************************
	//******* 2.021: T2_CTZ (Country of Citizenship) *******
	//******************************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonCitizenshipText>US</nc:PersonCitizenshipText>
	//
	if (Get("T2_CTZ", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::TextType										personCitizenshipText(pData);
		recordSubject.PersonCitizenship().push_back(personCitizenshipText);
	}

	//*********************************************
	//******* 2.022: T2_DOB (Date of Birth) *******
	//*********************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonBirthDate>
	//        <nc:Date>1975-06-20</nc:Date>
	//    </nc:PersonBirthDate>
	//
	if (Get("T2_DOB", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date											personBirthDateTempTemp(y, m, d);
			::proxyxsd::date											personBirthDateTemp(personBirthDateTempTemp);
			::niemcore::DateType										personBirthDate;

			personBirthDate.DateRepresentation().push_back(personBirthDateTemp);
			recordSubject.PersonBirthDate().push_back(personBirthDate);
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	//*****************************************
	//******* 2.023: T2_AGR (Age Range) *******
	//*****************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonAgeMeasure>
	//        <nc:MeasureRangeValue>
	//            <nc:RangeMinimumValue>30</nc:RangeMinimumValue>
	//            <nc:RangeMaximumValue>35</nc:RangeMaximumValue>
	//        </nc:MeasureRangeValue>
	//    </nc:PersonAgeMeasure>
	//
	if (Get("T2_AGR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeXXYY(pData, &min, &max))
		{
			::niemcore::MeasureRangeValueType							measureRangeValue;
			::niemcore::TimeMeasureType									timeMeasure;

			measureRangeValue.RangeMinimumValue().push_back(min);
			measureRangeValue.RangeMaximumValue().push_back(max);
			timeMeasure.MeasureValue().push_back(measureRangeValue);

			recordSubject.PersonAgeMeasure().push_back(timeMeasure);
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	//**************************************
	//******* 2.024: T2_SEX (Gender) *******
	//**************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonSexCode>F</nc:PersonSexCode>
	//
	if (Get("T2_SEX", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi::SEXCodeType											sEXCode(pData);
		recordSubject.PersonSex().push_back(sEXCode);
	}

	//************************************
	//******* 2.025: T2_RAC (Race) *******
	//************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonRaceCode>W</nc:PersonRaceCode>
	//
	if (Get("T2_RAC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi::RACCodeType											rACCode(pData);
		recordSubject.PersonRace().push_back(rACCode);
	}

	//***********************************************************
	//******* 2.026: T2_SMT (Scars, Marks and Tattoos) *******
	//***********************************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonPhysicalFeature>
	//        <nc:PhysicalFeatureCategoryText>EXTR C NIP</nc:PhysicalFeatureCategoryText>
	//    </nc:PersonPhysicalFeature>
	//
	if (Get("T2_SMT", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::PhysicalFeatureType								physicalFeature;
		::niemcore::TextType										physicalFeatureCategoryText(pData);

		physicalFeature.PhysicalFeatureCategory().push_back(physicalFeatureCategoryText);
		recordSubject.PersonPhysicalFeature().push_back(physicalFeature);
	}

	//**************************************
	//******* 2.027: T2_HGT (Height) *******
	//**************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonHeightMeasure>
	//        <nc:MeasurePointValue>510</nc:MeasurePointValue>
	//
	::niemcore::LengthMeasureType									lengthMeasure;
	BOOL bHasLengthMeasure = FALSE;
	if (Get("T2_HGT", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		::niemcore::MeasurePointValueType							measurePointValue(lVal);

		lengthMeasure.MeasureValue().push_back(measurePointValue);
		bHasLengthMeasure = TRUE;
	}

	//********************************************
	//******* 2.028: T2_HTR (Height Range) *******
	//********************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonHeightMeasure>
	//        <nc:MeasureRangeValue>
	//            <nc:RangeMinimumValue>505</nc:RangeMinimumValue>
	//            <nc:RangeMaximumValue>600</nc:RangeMaximumValue>
	//        </nc:MeasureRangeValue>
	//
	if (Get("T2_HTR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeXXXYYY(pData, &min, &max))
		{
			::niemcore::MeasureRangeValueType						measureRangeValue;

			measureRangeValue.RangeMinimumValue().push_back(min);
			measureRangeValue.RangeMaximumValue().push_back(max);
			lengthMeasure.MeasureValue().push_back(measureRangeValue);
			bHasLengthMeasure = TRUE;
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	// Force T2_HGT and T2_HTR to fall under the same <nc:PersonHeightMeasure> tag
	// (although they probably shouldn't ever appear toghether anyway)
	if (bHasLengthMeasure) recordSubject.PersonHeightMeasure().push_back(lengthMeasure);

	//**************************************
	//******* 2.029: T2_WGT (Weight) *******
	//**************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonWeightMeasure>
	//        <nc:MeasurePointValue>170</nc:MeasurePointValue>
	//
	BOOL bHasWeightMeasure = FALSE;
	::niemcore::WeightMeasureType								weightMeasure;
	if (Get("T2_WGT", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		::niemcore::MeasurePointValueType							measurePointValue(lVal);

		weightMeasure.MeasureValue().push_back(measurePointValue);
		bHasWeightMeasure = TRUE;
	}

	//********************************************
	//******* 2.030: T2_WTR (Weight Range) *******
	//********************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonWeightMeasure>
	//        <nc:MeasureRangeValue>
	//            <nc:RangeMinimumValue>160</nc:RangeMinimumValue>
	//            <nc:RangeMaximumValue>180</nc:RangeMaximumValue>
	//        </nc:MeasureRangeValue>
	//
	if (Get("T2_WTR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeXXXYYY(pData, &min, &max))
		{
			::niemcore::MeasureRangeValueType						measureRangeValue;

			measureRangeValue.RangeMinimumValue().push_back(min);
			measureRangeValue.RangeMaximumValue().push_back(max);
			weightMeasure.MeasureValue().push_back(measureRangeValue);
			bHasWeightMeasure = TRUE;
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	// Force T2_WGT and T2_WTR to fall under the same <nc:PersonWeightMeasure> tag
	// (although they probably shouldn't ever appear toghether anyway)
	if (bHasWeightMeasure) recordSubject.PersonWeightMeasure().push_back(weightMeasure);

	//******************************************
	//******* 2.031: T2_EYE (Color Eyes) *******
	//******************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonEyeColorCode>BRO</nc:PersonEyeColorCode>
	//
	if (Get("T2_EYE", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi::EYECodeType											personEyeColorCode(pData);

		recordSubject.PersonEyeColor().push_back(personEyeColorCode);
	}

	//******************************************
	//******* 2.032: T2_HAI (Hair Color) *******
	//******************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonHairColorCode>BLK</nc:PersonHairColorCode>
	//
	if (Get("T2_HAI", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi::HAICodeType											personHairColorCode(pData);

		recordSubject.PersonHairColor().push_back(personHairColorCode);
	}

	//***************************************************************
	//******* 2.033: T2_FPC (NCIC Fingerprint Classification) *******
	//***************************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//        <ebts:FingerprintImageFinger>
	//            <ebts:NCICFingerprintClassificationText>12101116141109111713</ebts:NCICFingerprintClassificationText>
	//
	if (Get("T2_FPC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::FingerprintImageFingerType						fingerprintImageFinger;
		fingerprintImageFinger.NCICFingerprintClassificationText(pData);
		personFingerprintSet.FingerprintImageFinger().push_back(fingerprintImageFinger);
	}

	//*************************************************************
	//******* 2.034: T2_PAT (Pattern Level Classifications) *******
	//*************************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//        <ebts:FingerprintImageFinger>
	//            <ansi-nist:FingerPositionCode>1</ansi-nist:FingerPositionCode>
	//            <ebts:FingerprintPatternClassificationCode>WU</ebts:FingerprintPatternClassificationCode>
	//
	// Note: there doesn't seem to be any place to store the optional "reference pattern
	// classifications" described by T2_PAT_CL2 and T2_PAT_CL3. This may be have been
	// overlooked in the beta fbi ebts.
	//
	// TODO: is this supposed to allow for data on multiple fingers somehow?
	//
	if (Get("T2_PAT_FGP", &pData1, 1, 1) == IW_SUCCESS && pData1)
	{
		if (Get("T2_PAT_CL1", &pData2, 1, 1) == IW_SUCCESS && pData2)
		{
			::fbi_ebts::FingerprintImageFingerType					fingerprintImageFinger;

			fingerprintImageFinger.FingerPositionCode(pData1);
			fingerprintImageFinger.FingerprintPatternClassificationCode(pData2);
			personFingerprintSet.FingerprintImageFinger().push_back(fingerprintImageFinger);
		}
	}

	//**************************************************************
	//******* 2.035: T2_PPA (Palmprints Available Indicator) *******
	//**************************************************************
	//
	//<ebts:RecordSubject>
	//    <j:PersonPalmPrint>
	//        <nc:BiometricImage>
	//            <nc:BinaryAvailableIndicator>true</nc:BinaryAvailableIndicator>
	//        </nc:BiometricImage>
	//    </j:PersonPalmPrint>
	//
	if (Get("T2_PPA", &pData, 1, 1) == IW_SUCCESS && pData)
	// Note: T2_PPA is either "Y" or the field is omitted, so we don't even check pData
	{
		::niemcore::BiometricType									biometric;
		::niemcore::ImageType										image;

		image.BinaryAvailableIndicator().push_back(1);
		biometric.BiometricImage().push_back(image);
		recordSubject.PersonPalmPrint().push_back(biometric);
	}

	//*********************************************************
	//******* 2.036: T2_PHT (Photo Available Indicator) *******
	//*********************************************************
	//
	//<ebts:RecordSubject>
	//    <nc:PersonDigitalImage>
	//        <nc:BinaryAvailableIndicator>true</nc:BinaryAvailableIndicator>
	//    </nc:PersonDigitalImage>
	//
	if (Get("T2_PHT", &pData, 1, 1) == IW_SUCCESS && pData)
	// Note: T2_PHT is either "Y" or the field is omitted, so we don't even check pData
	{
		::niemcore::ImageType										image;

		image.BinaryAvailableIndicator().push_back(1);
		recordSubject.PersonDigitalImage().push_back(image);
	}

	//****************************************************
	//******* 2.037: T2_RFP (Reason Fingerprinted) *******
	//****************************************************
	//
	//<ebts:RecordActivity>
	//    <nc:ActivityReasonText>Gun Permit</nc:ActivityReasonText>
	//
	if (Get("T2_RFP", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordActivity.ActivityReasonText().push_back(pData);
	}

	//******************************************
	//******* 2.038: T2_DPR (Date Printed) *******
	//******************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//        <nc:BiometricCaptureDate>
	//            <nc:Date>2008-07-09</nc:Date>
	//        </nc:BiometricCaptureDate>
	//
	if (Get("T2_DPR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date											biometricCaptureDateTempTemp(y, m, d);
			::proxyxsd::date											biometricCaptureDateTemp(biometricCaptureDateTempTemp);
			::niemcore::DateType										biometricCaptureDate;

			biometricCaptureDate.DateRepresentation().push_back(biometricCaptureDateTemp);
			personFingerprintSet.BiometricCaptureDate(biometricCaptureDate);
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	//****************************************************
	//******* 2.039: T2_EAD (Employer and Address) *******
	//****************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonEmploymentAssociation>
	//        <nc:EmploymentLocation>
	//            <nc:LocationAddress>
	//                <nc:AddressFullText>2 Maple Lane, Columbus OHIO</nc:AddressFullText>
	//            </nc:LocationAddress>
	//        </nc:EmploymentLocation>
	//    </ebts:PersonEmploymentAssociation>
	//
	// Note that the spec would like the address to be like this:
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonEmploymentAssociation>
	//        <nc:Employer>
	//            <nc:EntityOrganization>
	//                <nc:OrganizationName/>
	//            </nc:EntityOrganization>
	//        </nc:Employer>
	//        <nc:EmploymentLocation>
	//            <nc:LocationAddress>
	//                <nc:StructuredAddress>
	//                    <nc:LocationStreet>
	//                        <nc:StreetNumberText/>
	//                        <nc:StreetName/>
	//                    </nc:LocationStreet>
	//                </nc:StructuredAddress>
	//            </nc:LocationAddress>
	//        </nc:EmploymentLocation>
	//	</ebts:PersonEmploymentAssociation>
	//
	// but since T2_EAD is free-text, there is no way to know how to parse out the
	// individual details of the address (!) so until further notice the conversion
	// places everything under <nc:AddressFullText>.
	//
	if (Get("T2_EAD", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::LocationType									location;
		::niemcore::AddressType										address;
		::niemcore::TextType										addressText(pData);

		address.AddressRepresentation().push_back(addressText);
		location.LocationAddress().push_back(address);
		personEmploymentAssociation.EmploymentLocation().push_back(location);
	}

	//******************************************
	//******* 2.040: T2_OCP (Occupation) *******
	//******************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonEmploymentAssociation>
	//        <nc:EmployeeOccupationText>Bank Manager</nc:EmployeeOccupationText>
	//
	if (Get("T2_OCP", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::TextType										occupationText(pData);
		personEmploymentAssociation.EmployeeOccupation().push_back(occupationText);
	}

	//*****************************************************************
	//******* 2.041: T2_RES (Residence of Person Fingerprinted) *******
	//*****************************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonResidenceAssociation>
	//        <nc:LocationAddress>
	//            <nc:AddressFullText>42 Apple Crescent, Columbus OHIIO</nc:AddressFullText>
	//        </nc:LocationAddress>
	//    </ebts:PersonResidenceAssociation>
	//
	// Same comment as for 2.039: T2_EAD. T2_RES is free-text and so there is no way
	// to decompose it into address atoms as the (beta) spec suggests:
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonResidenceAssociation>
	//        <nc:LocationAddress>
	//            <nc:StructuredAddress>
	//                <nc:LocationStreet>
	//                    <nc:StreetNumberText/>
	//                    <nc:StreetName/>
	//                </nc:LocationStreet>
	//             </nc:StructuredAddress>
	//        </nc:LocationAddress>
	//    </ebts:PersonResidenceAssociation>
	//
	if (Get("T2_RES", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::PersonResidenceAssociationType					personResidenceAssociation;
		::niemcore::AddressType										address;
		::niemcore::TextType										addressText(pData);

		address.AddressRepresentation().push_back(addressText);
		personResidenceAssociation.LocationAddress().push_back(address);
		recordSubject.PersonResidenceAssociation(personResidenceAssociation);
	}

	//*********************************************
	//******* 2.042: T2_MIL (Military Code) *******
	//*********************************************
	//
	//<ebts:RecordTransactionData>
	//    <ansi-nist:TransactionSubmissionMilitaryCode>G</ansi-nist:TransactionSubmissionMilitaryCode>
	//
	if (Get("T2_MIL", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::ansinist::MILCodeType										mILCodeType(pData);
		recordTransactionData.TransactionSubmissionMilitaryCode().push_back(mILCodeType);
	}

	//********************************************************
	//******* 2.043: T2_TSR (Type of Search Requested) *******
	//********************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ansi-nist:TransactionSearchRequestCategoryCode>V</ansi-nist:TransactionSearchRequestCategoryCode>
	//
	if (Get("T2_TSR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::ansinist::TSRCodeType										tSRCodeType(pData);
		recordTransactionData.TransactionSearchRequestCategoryCode().push_back(tSRCodeType);
	}

	//***********************************************************
	//******* 2.044: T2_GEO (Geographical Area of Search) *******
	//***********************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionSearchAreaCode>CA</ebts:TransactionSearchAreaCode>
	//
	if (Get("T2_GEO", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi::POBCodeType											pOBCode(pData);
		recordTransactionData.TransactionSearchAreaCode().push_back(pOBCode);
	}

	//**********************************************
	//******* 2.045: T2_DOA (Date of Arrest) *******
	//**********************************************
	//
	//<ebts:RecordActivity>
	//    <nc:ActivityDate>
	//        <nc:Date>2008-07-09</nc:Date>
	//    </nc:ActivityDate>
	//
	if (Get("T2_DOA", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date											activityDateTempTemp(y, m, d);
			::proxyxsd::date											activityDateTemp(activityDateTempTemp);
			::niemcore::DateType										activityDate;

			activityDate.DateRepresentation().push_back(activityDateTemp);
			recordActivity.ActivityDateRepresentation().push_back(activityDate);
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	//*******************************************************
	//******* 2.046: T2_DOS (Date of Arrest - Suffix) *******
	//*******************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:ArrestDateSuffixText>L</ebts:ArrestDateSuffixText>
	//
	if (Get("T2_DOS", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordActivity.ArrestDateSuffixText(pData);
	}

	//******************************************************
	//******* 2.047: T2_ASL (Arrest Segment Literal) *******
	//******************************************************
	//
	//<ebts:RecordActivity>
	//    <j:Arrest>
	//        <nc:ActivityDate>
	//            <nc:Date>2008-11-28</nc:Date>
	//        </nc:ActivityDate>
	//        <nc:ActivityDescriptionText>Armed Robbery</nc:ActivityDescriptionText>
	//    </j:Arrest>
	//
	if (Get("T2_ASL_DOO", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date										activityDateTempTemp(y, m, d);
			::proxyxsd::date										activityDateTemp(activityDateTempTemp);
			::niemcore::DateType									activityDate;

			activityDate.DateRepresentation().push_back(activityDateTemp);
			arrest.ActivityDateRepresentation().push_back(activityDate);
			bHasArrest = TRUE;
		}
		else
		{
			//failed decomposition (todo)
		}
	}
	if (Get("T2_ASL_AOL", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		arrest.ActivityDescriptionText().push_back(pData);
		bHasArrest = TRUE;
	}

	//****************************************************************
	//******* 2.048: T2_CSR (Civil Search Requested Indicator) *******
	//****************************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionCivilSearchRequestIndicator>true</ebts:TransactionCivilSearchRequestIndicator>
	//
	if (Get("T2_CSR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		bVal = (_tcsicmp(pData, "Y") == 0);
		// For a "No", the field is ommited
		if (bVal)
		{
			recordTransactionData.TransactionCivilSearchRequestIndicator(bVal);
		}
	}

	//*****************************************************
	//******* 2.051: T2_CSL (Court Segment Literal) *******
	//*****************************************************
	//
	//<ebts:RecordActivity>
	//    <j:Arrest>
	//        <j:ArrestCharge>
	//        <j:ChargeDisposition>
	//            <j:ChargeDispositionCondition>
	//                <nc:ConditionSetDate>
	//                    <nc:Date>2008-11-25</nc:Date>
	//                </nc:ConditionSetDate>
	//            </j:ChargeDispositionCondition>
	//            <j:ChargeDispositionOtherText>Sample Court Sentence Provision Literal</j:ChargeDispositionOtherText>
	//        </j:ChargeDisposition>
	//        <j:ChargeUCRCrimeCategoryText>Sample Court Offense Literal</j:ChargeUCRCrimeCategoryText>
	//    </j:ArrestCharge>
	//
	BOOL bHasCharge = FALSE;
	BOOL bHasChargeDisposition = FALSE;
	::jxdm::ChargeType												charge;
	::jxdm::ChargeDispositionType									chargeDisposition;
	if (Get("T2_CSL_CDD", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::niemcore::ConditionType								conditionType;
			::xml_schema::date										conditionSetDateTempTemp(y, m, d);
			::proxyxsd::date										conditionSetDateTemp(conditionSetDateTempTemp);
			::niemcore::DateType									conditionSetDate;

			conditionSetDate.DateRepresentation().push_back(conditionSetDateTemp);
			conditionType.ConditionSetDate().push_back(conditionSetDate);
			chargeDisposition.ChargeDispositionCondition().push_back(conditionType);
			bHasChargeDisposition = TRUE;
			bHasCharge = TRUE;
		}
		else
		{
			//failed decomposition (todo)
		}
	}
	if (Get("T2_CSL_COL", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		charge.ChargeUCRCrimeCategoryText().push_back(pData);
		bHasCharge = TRUE;
	}
	if (Get("T2_CSL_CPL", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		chargeDisposition.ChargeDispositionOtherText().push_back(pData);
		bHasCharge = TRUE;
		bHasChargeDisposition = TRUE;
	}
	if (bHasCharge)
	{
		if (bHasChargeDisposition) 
		{
			charge.ChargeDisposition().push_back(chargeDisposition);
		}
		arrest.ArrestCharge().push_back(charge);
		bHasArrest = TRUE;
	}

	//************************************************
	//******* 2.053: T2_OFC (Offense Category) *******
	//************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:OffenseCategoryCode>3</ebts:OffenseCategoryCode>
	//
	if (Get("T2_OFC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::OFCCodeType										oFCCodeType(pData);
		recordActivity.OffenseCategoryCode(oFCCodeType);
	}

	//**************************************************************************
	//******* 2.054: T2_SSD (Custody or Supervisory Status - Start Date) *******
	//**************************************************************************
	//
	//<ebts:RecordActivity>
	//    <j:Arrest>
	//        <j:ArrestRelease>
	//            <nc:ActivityDate>
	//                <nc:Date>2009-02-10</nc:Date>
	//            </nc:ActivityDate>
	//
	::niemcore::ReleaseType											release;
	if (Get("T2_SSD", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date										activityDateTempTemp(y, m, d);
			::proxyxsd::date										activityDateTemp(activityDateTempTemp);
			::niemcore::DateType									activityDate;

			activityDate.DateRepresentation().push_back(activityDateTemp);
			release.ActivityDateRepresentation().push_back(activityDate);
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	//*********************************************************************
	//******* 2.055: T2_SLE (Custody or Supervisory Status Literal) *******
	//*********************************************************************
	//
	//<ebts:RecordActivity>
	//    <j:Arrest>
	//        <j:ArrestRelease>
	//            <nc:ActivityDescriptionText>Sample Supervisory Status </nc:ActivityDescriptionText>
	//
	if (Get("T2_SLE", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		release.ActivityDescriptionText().push_back(pData);
	}
	arrest.ArrestRelease().push_back(release);

	//*******************************************************
	//******* 2.056: T2_ICO (Identification Comments) *******
	//*******************************************************
	//
	//<ebts:RecordActivity>
	//    <nc:ActivityDescriptionText>Sample Identification Comment</nc:ActivityDescriptionText>
	//
	if (Get("T2_ICO", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordActivity.ActivityDescriptionText().push_back(pData);
	}

	//**********************************************************
	//******* 2.057: T2_FNR (Finger number(s) Requested) *******
	//**********************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionFingerprintImagesRequestedText>00</ebts:TransactionFingerprintImagesRequestedText>
	//
	if (Get("T2_FNR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordTransactionData.TransactionFingerprintImagesRequestedText(pData);
	}

	//******************************************************
	//******* 2.059: T2_SRF (Search Results Finding) *******
	//******************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionSearchResultsCode>I</ebts:TransactionSearchResultsCode>
	//
	if (Get("T2_SRF", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::SRFCodeType										sRFCodeType(pData);

		transactionResponseData.TransactionSearchResultsCode(sRFCodeType);
	}

	//****************************************************
	//******* 2.060: T2_MSG (Status/Error Message) *******
	//****************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionStatusText>Sample Status/Error Message</ebts:TransactionStatusText>
	if (Get("T2_MSG", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		transactionResponseData.TransactionStatusText().push_back(pData);
	}

	//******************************************
	//******* 2.061: T2_CST (Case Title) *******
	//******************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:CaseTitleText>Sample Case Title</ebts:CaseTitleText>
	//
	if (Get("T2_CST", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordActivity.CaseTitleText(pData);
	}

	//******************************************
	//******* 2.062: T2_IMT (Image Type) *******
	//******************************************
	//
	//<ansi-nist:ImageCategoryCode>4</ansi-nist:ImageCategoryCode>
	//
	// Here we have a problem because EBTS 8.1 says we can use these values:
	//
	// Fingerprint 1
	// Lower Joint 2
	// Palmprint 3
	// Toe Print 4
	// Foot Print 5
	//
	// but the EBTS 8.001 XML IEP Draft schemas and sample XML files specify the
	// choices of FACE, MARK, SCAR, or TATTOO.
	//
	// Until this gets resolved we simpy use 'FACE', for testing purposes.
	//
	if (Get("T2_IMT", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		auto_ptr<::ansinist::IMTCodeType>							iMTCodeType;
 
		lVal = _ttol(pData);
		// Must convert the image type number to one of the enums
		//if (lVal == 0)
		//{
			iMTCodeType.reset(new ::ansinist::IMTCodeType(::ansinist::IMTCodeType::FACE));
		//}
		//else
		//{
		//	iMTCodeType.reset(new ::ansinist::IMTCodeType(pData)); // unknown
		//}

		domainDefinedDescriptiveText.ImageCategoryCode().push_back(iMTCodeType);
	}

	//******************************************
	//******* 2.063: T2_PTD (Image Type) *******
	//******************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonTypeDesignatorCode>S</ebts:PersonTypeDesignatorCode>
	//
	if (Get("T2_PTD", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::PTDCodeType										personTypeDesignatorCode(pData);
		recordSubject.PersonTypeDesignatorCode(personTypeDesignatorCode);
	}

	//**********************************************
	//******* 2.064: T2_CAN (Candidate List) *******
	//**********************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionCandidateList>
	//            <ebts:Candidate>
	//                <nc:PersonName>
	//                    <nc:PersonGivenName>MONA</nc:PersonGivenName>
	//                    <nc:PersonMiddleName>J</nc:PersonMiddleName>
	//                    <nc:PersonSurName>LISA</nc:PersonSurName>
	//                </nc:PersonName>
	//                <j:PersonFBIIdentification>
	//                    <nc:IdentificationID>FBINUM001</nc:IdentificationID>
	//                </j:PersonFBIIdentification>
	//            </ebts:Candidate>
	//
	if (Get("T2_CAN_FNU", &pData1, 1, 1) == IW_SUCCESS && pData1)
	{
		if (Get("T2_CAN_NAM", &pData2, 1, 1) == IW_SUCCESS && pData2)
		{
			if (DecomposeNAME(pData2, sz1, sz2, sz3))
			{
				::niemcore::PersonNameType								personName;
				::niemcore::PersonNameType::PersonGivenName_sequence	personGivenName_sequence;
				::niemcore::PersonNameType::PersonMiddleName_sequence	personMiddleName_sequence;
				::niemcore::PersonNameType::PersonSurName_sequence		personSurNameName_sequence;

				personGivenName_sequence.push_back(sz1);
				personMiddleName_sequence.push_back(sz2);
				personSurNameName_sequence.push_back(sz3);

				if (_tcslen(sz1)) personName.PersonGivenName(personGivenName_sequence);
				if (_tcslen(sz2)) personName.PersonMiddleName(personMiddleName_sequence);
				if (_tcslen(sz3)) personName.PersonSurName(personSurNameName_sequence);

				::niemcore::IdentificationType							identification;

				identification.IdentificationID().push_back(pData1);
				::fbi_ebts::CandidateType								candidate(personName, identification);

				transactionCandidateList.Candidate().push_back(candidate);
			}
			else
			{
				//decompose error (todo)
			}
		}
	}

	//**************************************************************
	//******* 2.065: T2_RSR (Repository Statistics Response) *******
	//**************************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionRepositoryResponse>
	//            <ebts:RepositoryParameterPercentage>0.321</ebts:RepositoryParameterPercentage>
	//            <ebts:RepositoryParameterText>EYE</ebts:RepositoryParameterText>
	//            <ebts:RepositoryParameterValueText>BLUE</ebts:RepositoryParameterValueText>
	//        </ebts:TransactionRepositoryResponse>
	//
	if (Get("T2_RSR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		// Tab separated parameter name, parameter value, and percentage
		if (DecomposeTABSEP3(pData, sz1, sz2, sz3))
		{
			double														dPercentage = atof(sz3);
			::niemcore::PercentageType									percentage(dPercentage );
			::fbi_ebts::TransactionRepositoryResponseType				transactionRepositoryResponse(percentage, sz1, sz2);

			transactionResponseData.TransactionRepositoryResponse().push_back(transactionRepositoryResponse);
		}
		else
		{
			// decomposition error (todo)
		}
	}

	//**********************************************************
	//******* 2.069: T2_ETC (Estimated Time to Complete) *******
	//**********************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionEstimatedCompletionTimeQuantity>6270</ebts:TransactionEstimatedCompletionTimeQuantity>
	//
	if (Get("T2_ETC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		recordTransactionData.TransactionEstimatedCompletionTimeQuantity(lVal);
	}

	//****************************************************************
	//******* 2.070: T2_RAP (Request for Electronic Rap Sheet) *******
	//****************************************************************
	//
	//<ansi-nist:RecordRapSheetRequestIndicator>true</ansi-nist:RecordRapSheetRequestIndicator>
	//
	if (Get("T2_RAP", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		bVal = (_tcsicmp(pData, "Y") == 0);
		domainDefinedDescriptiveText.RecordRapSheetRequestIndicator().push_back(bVal);
	}

	//*************************************************
	//******* 2.071: T2_ACN (ction to be Taken) *******
	//*************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionActionText>Sample Action to be Taken</ebts:TransactionActionText>
	//
	if (Get("T2_ACN", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		transactionResponseData.TransactionActionText(pData);
	}

	//************************************************************
	//******* 2.072: T2_FIU (Fingerprint Image(s) Updated) *******
	//************************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionFingerprintImagesUpdated>
	//        <ansi-nist:FingerPositionCode>3</ansi-nist:FingerPositionCode>
	//    </ebts:TransactionFingerprintImagesUpdated>
	//
	// TODO: handle fingerprint code "A" implying All fingers
	//
	if (Get("T2_FIU", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::TransactionFingerprintImagesUpdatedType			transactionFingerprintImagesUpdated;
		::ansinist::FPCCodeType										fPCCodeType(pData);

		transactionFingerprintImagesUpdated.FingerPositionCode().push_back(fPCCodeType);
		recordTransactionData.TransactionFingerprintImagesUpdated(transactionFingerprintImagesUpdated);
	}

	//*************************************************************
	//******* 2.073: T2_CRI (Controlling Agency Identifier) *******
	//*************************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:ActivityOrganizationAssociation>
	//        <nc:Organization>
	//            <nc:OrganizationOtherIdentification>
	//                <nc:IdentificationID>ORI456789</nc:IdentificationID>
	//            </nc:OrganizationOtherIdentification>
	//        </nc:Organization>
	//    </ebts:ActivityOrganizationAssociation>
	//
	if (Get("T2_CRI", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;
		::niemcore::OrganizationType								organization;
		::niemcore::OrganizationType::OrganizationOtherIdentification_sequence	organizationOtherIdentification_sequence;
		::fbi_ebts::ActivityOrganizationAssociationType				activityOrganizationAssociation;
		::structures::ReferenceType									reference;

		identification.IdentificationID().push_back(pData);
		organizationOtherIdentification_sequence.push_back(identification);
		organization.OrganizationOtherIdentification(organizationOtherIdentification_sequence);
		activityOrganizationAssociation.Organization().push_back(organization);
		recordActivity.ActivityOrganizationAssociation().push_back(activityOrganizationAssociation);
	}

	//***********************************************
	//******* 2.074: T2_FGP (Finger Position) *******
	//***********************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//        <ebts:FingerprintImageFinger>
	//            <ansi-nist:FingerPositionCode>7</ansi-nist:FingerPositionCode>
	//
	if (Get("T2_FGP", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::fbi_ebts::FingerprintImageFingerType						fingerprintImageFinger;
		::ansinist::FPCCodeType										fPCCodeType(pData);

		fingerprintImageFinger.FingerPositionCode(fPCCodeType);
		personFingerprintSet.FingerprintImageFinger().push_back(fingerprintImageFinger);
	}

	//*********************************************************
	//******* 2.077: T2_CFS (Cancel Fingerprint Search) *******
	//*********************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionCancelFingerprintSearchNumber>3124</ebts:TransactionCancelFingerprintSearchNumber>
	//
	if (Get("T2_CFS", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		recordTransactionData.TransactionCancelFingerprintSearchNumber(lVal);
	}

	//**********************************************************
	//******* 2.078: T2_PEN (Penetration Query Response) *******
	//**********************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionPenetrationQueryResponsePercentage>42</ebts:TransactionPenetrationQueryResponsePercentage>
	//
	if (Get("T2_PEN", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		dVal = atof(pData)/100; // percentage expressed as 0.0...1.0
		transactionResponseData.TransactionPenetrationQueryResponsePercentage(dVal);
	}

	//********************************************************************
	//******* 2.079: T2_NCR (Number of Candidates Images Returned) *******
	//********************************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionImagesRequestedQuantity>20</ebts:TransactionImagesRequestedQuantity>
	//
	if (Get("T2_NCR", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		recordTransactionData.TransactionImagesRequestedQuantity(lVal);
	}

	//****************************************************
	//******* 2.080: T2_EXP (Response Explanation) *******
	//****************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ansi-nist:TransactionReasonText>PHOTO NOT FOUND FOR SPECIFIED DOA DOS</ansi-nist:TransactionReasonText>
	//
	if (Get("T2_EXP", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordTransactionData.TransactionReasonText().push_back(pData);
	}

	//*********************************************
	//******* 2.082: T2_REC (Response Code) *******
	//*********************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionResponseIndicator>true</ebts:TransactionResponseIndicator>
	//
	if (Get("T2_REC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		bVal = (_tcsicmp(pData, "Y") == 0);
		transactionResponseData.TransactionResponseIndicator(bVal);
	}

	//****************************************************
	//******* 2.083: T2_ULF (Unsolved Latent File) *******
	//****************************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionUnsolvedLatentFileIndicator>true</ebts:TransactionUnsolvedLatentFileIndicator>
	//
	if (Get("T2_ULF", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		bVal = (_tcsicmp(pData, "Y") == 0);
		// For a "No", the field is ommited
		if (bVal)
		{
			recordTransactionData.TransactionUnsolvedLatentFileIndicator(bVal);
		}
	}

	//*****************************************************
	//******* 2.084: T2_AMP (Amputated or Bandaged) *******
	//*****************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//    <itl:FingerprintImageFingerMissing>
	//        <ansi-nist:FingerPositionCode>5</ansi-nist:FingerPositionCode>
	//        <itl:FingerMissingCode>XX</itl:FingerMissingCode>
	//    </itl:FingerprintImageFingerMissing>
	//
	if (Get("T2_AMP_FGP", &pData1, 1, 1) == IW_SUCCESS && pData1)
	{
		if (Get("T2_AMP_AMPCD", &pData2, 1, 1) == IW_SUCCESS && pData2)
		{
			::ansinist::FPCCodeType									fPCCode(pData1);
			::standard_22008::AMPCodeType							aMPCode(pData2);
			::standard_22008::FingerprintImageFingerMissingType		fingerprintImageFingerMissing(fPCCode, aMPCode);

			personFingerprintSet.FingerprintImageFingerMissing().push_back(fingerprintImageFingerMissing);
		}
	}

	//***************************************************
	//******* 2.085: T2_CRN (Civil Record Number) *******
	//***************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:CivilRecordIdentification>
	//        <nc:IdentificationID>CIV456789</nc:IdentificationID>
	//    <ebts:CivilRecordIdentification>
	//
	if (Get("T2_CRN", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType								identification;

		identification.IdentificationID().push_back(pData);
		recordActivity.CivilRecordIdentification(identification);
	}

	//************************************************************
	//******* 2.086: T2_SCNA (AFIS Segment Control Number) *******
	//************************************************************
	//
	//<ebts:RecordActivity>
	//    <ebts:AFISSegmentControlNumber>1234567890</ebts:AFISSegmentControlNumber>
	//
	if (Get("T2_SCNA", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		recordActivity.AFISSegmentControlNumber(lVal);
	}

	//**********************************************
	//******* 2.087: T2_TAA (Treat As Adult) *******
	//**********************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonAdultTreatmentIndicator>true</ebts:PersonAdultTreatmentIndicator>
	//
	if (Get("T2_TAA", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		bVal = (_tcsicmp(pData, "Y") == 0);
		// For a "No", the field is ommited
		if (bVal)
		{
			recordSubject.PersonAdultTreatmentIndicator(bVal);
		}
	}

	//******************************************
	//******* 2.088: T2_NOT (Note Field) *******
	//******************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionDescriptionText>This is a note</ebts:TransactionDescriptionText>
	//
	if (Get("T2_NOT", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		recordTransactionData.TransactionDescriptionText(pData);
	}

	//*******************************************
	//******* 2.089: T2_MSC (Match Score) *******
	//*******************************************
	//
	//<ebts:RecordTransactionData>
	//    <ebts:TransactionResponseData>
	//        <ebts:TransactionCandidateList>
	//            <ebts:CandidateListMatchScore>100000</ebts:CandidateListMatchScore>
	if (Get("T2_MSC", &pData, 1, 1) == IW_SUCCESS && pData)
	{
		lVal = _ttol(pData);
		transactionCandidateList.CandidateListMatchScore(lVal);
	}

	//***********************************************************************************
	//******* 2.091: T2_RCD1 (Ridge Core Delta One For Subpattern Classification) *******
	//***********************************************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//        <ebts:FingerprintImageFinger>
	//            <ansi-nist:FingerPositionCode>1</ansi-nist:FingerPositionCode>
	//            <ebts:FingerprintRidgeCountValue>21</ebts:FingerprintRidgeCountValue>
	//
	if (Get("T2_RCD1_FGP", &pData1, 1, 1) == IW_SUCCESS && pData1)
	{
		if (Get("T2_RCD1_RCN1", &pData2, 1, 1) == IW_SUCCESS && pData2)
		{
			::fbi_ebts::FingerprintImageFingerType					fingerprintImageFinger;
			::ansinist::FPCCodeType									fPCCode(pData1);

			lVal = _ttol(pData2);
			fingerprintImageFinger.FingerPositionCode(fPCCode);
			fingerprintImageFinger.FingerprintRidgeCountValue().push_back(lVal);
			personFingerprintSet.FingerprintImageFinger().push_back(fingerprintImageFinger);
		}
	}

	//***********************************************************************************
	//******* 2.092: T2_RCD2 (Ridge Core Delta Two For Subpattern Classification) *******
	//***********************************************************************************
	//
	//<ebts:RecordSubject>
	//    <ebts:PersonFingerprintSet>
	//        <ebts:FingerprintImageFinger>
	//            <ansi-nist:FingerPositionCode>1</ansi-nist:FingerPositionCode>
	//            <ebts:FingerprintRidgeCountValue>21</ebts:FingerprintRidgeCountValue>
	//  
	if (Get("T2_RCD2_FGP", &pData1, 1, 1) == IW_SUCCESS && pData1)
	{
		if (Get("T2_RCD2_RCN2", &pData2, 1, 1) == IW_SUCCESS && pData2)
		{
			::fbi_ebts::FingerprintImageFingerType					fingerprintImageFinger;
			::ansinist::FPCCodeType									fPCCode(pData1);

			lVal = _ttol(pData2);
			fingerprintImageFinger.FingerPositionCode(fPCCode);
			fingerprintImageFinger.FingerprintRidgeCountValue().push_back(lVal);
			personFingerprintSet.FingerprintImageFinger().push_back(fingerprintImageFinger);
		}
	}

	recordSubject.PersonEmploymentAssociation(personEmploymentAssociation);
	recordSubject.PersonFingerprintSet1(personFingerprintSet);
	domainDefinedDescriptiveText.RecordSubject(recordSubject);
	if (bHasArrest) recordActivity.Arrest().push_back(arrest);
	domainDefinedDescriptiveText.RecordActivity().push_back(recordActivity);
	transactionResponseData.TransactionCandidateList(transactionCandidateList);
	recordTransactionData.TransactionResponseData(transactionResponseData);
	domainDefinedDescriptiveText.RecordTransactionData1(recordTransactionData);

	//<itl:UserDefinedDescriptiveText>
	PackageDescriptiveTextRecordType::UserDefinedDescriptiveText_type	userDefinedDescriptiveText(domainDefinedDescriptiveText);

	//<itl:PackageDescriptiveTextRecord>
	packageDescriptiveTextRecord.reset(new PackageDescriptiveTextRecordType(recordCategoryCode,
																			imageReferenceIdentification,
																			userDefinedDescriptiveText));
	return IW_SUCCESS;
}

int CIWTransaction::GetXMLForImageTypeRecords(vector <PackageImageRecordType> &packageImageRecords, char **pErr)
/*
	<xsd:element name="FingerprintImage"  substitutionGroup="ansi-nist:RecordImage" type="itl:FingerprintImageType"/>
	<xsd:element name="PalmprintImage"  substitutionGroup="ansi-nist:RecordImage" type="itl:PalmprintImageType"/>
	<xsd:element name="IrisImage"  substitutionGroup="ansi-nist:RecordImage" type="itl:IrisImageType"/>
	<xsd:element name="TestImage"  substitutionGroup="ansi-nist:RecordImage" type="ansi-nist:NISTImageType"/>
	<xsd:element name="FaceImage"  substitutionGroup="ansi-nist:RecordImage" type="itl:FaceImageType"/>
	<!--  Here is a sample substitute for the Type07 user defined image.  Users should create their own in a different domain. -->
	<xsd:element name="ExampleRecordImage"  substitutionGroup="ansi-nist:RecordImage" type="xsd:anyType"/>
*/
{
	int				ret;
	int				iRecordType;
	int				iNumRecords;
	int				iRecordIndex;
	const char		*pData;
	char			szID[3];

	for (iRecordType = 3; iRecordType <= 17; iRecordType++)
	{
		// The image record types are 3, 4, 5, 6, 7, 10, 13, 14, 15, 16 and 17. Some of them
		// demonstrate differences with the others but these are adressed below. Naturally, it
		// is assumed that the proper set of fields has already been set for the image record
		// type (which is best assured by performing verification on the file).
		if (iRecordType == 8 || iRecordType == 9 || iRecordType == 11 || iRecordType == 12) continue;

		GetRecordTypeCount(iRecordType, &iNumRecords);
	
		for (iRecordIndex = 1; iRecordIndex <= iNumRecords; iRecordIndex++)
		{
			//**************************************
			//******* The record type itself *******
			//**************************************
			//
			//<ansi-nist:RecordCategoryCode>04</ansi-nist:RecordCategoryCode>
			//
			sprintf_s(szID, 3, "%02d", iRecordType);
			::ansinist::RTCCodeType											recordCategoryCode(szID);

			//***********************************************************
			//******* X.002: TX_IDC (Image Designation Character) *******
			//***********************************************************
			//
			//<ansi-nist:ImageReferenceIdentification>
			//    <nc:IdentificationID>01</nc:IdentificationID>
			//</ansi-nist:ImageReferenceIdentification>
			//
			::niemcore::IdentificationType::IdentificationID_sequence		identificationIDSequence;
			::niemcore::IdentificationType									imageReferenceIdentification;
			CNISTRecord *pRec = GetRecord(iRecordType, iRecordIndex);
			if (pRec && pRec->FindItem(REC_TAG_IDC, 1, 1, &pData) == IW_SUCCESS && pData)
			{
				sprintf_s(szID, 3, "%02d", _ttol(pData));
				identificationIDSequence.push_back(szID);

				imageReferenceIdentification.IdentificationID(identificationIDSequence);
			}

			if (iRecordType >= 3 && iRecordType <= 6)
			{
				ret = AddType3Type4Type5Type6XML(packageImageRecords, iRecordType, iRecordIndex, recordCategoryCode, imageReferenceIdentification);
				if (ret != IW_SUCCESS) return ret;
			}
			else if (iRecordType == 7)
			{
				ret = AddType7XML(packageImageRecords, iRecordType, iRecordIndex, recordCategoryCode, imageReferenceIdentification);
				if (ret != IW_SUCCESS) return ret;
			}
			else if (iRecordType == 10)
    		{
				ret = AddType10XML(packageImageRecords, iRecordType, iRecordIndex, recordCategoryCode, imageReferenceIdentification);
				if (ret != IW_SUCCESS) return ret;
			}
		}
	}

	return IW_SUCCESS;
}

int CIWTransaction::AddType3Type4Type5Type6XML(vector <PackageImageRecordType> &packageImageRecords,
											   int							   iRecordType,
											   int							   iRecordIndex,
											   ::ansinist::RTCCodeType		   &recordCategoryCode,
											   ::niemcore::IdentificationType  &imageReferenceIdentification)
//<itl:PackageHighResolutionGrayscaleImageRecord>
//    <ansi-nist:RecordCategoryCode>04</ansi-nist:RecordCategoryCode>
//    <ansi-nist:ImageReferenceIdentification>
//        <nc:IdentificationID>01</nc:IdentificationID>
//    </ansi-nist:ImageReferenceIdentification>
//    <ansi-nist:FingerprintImage>
//        <nc:BinaryBase64Object>mrHbPd....0H/+h</nc:BinaryBase64Object>
//        <ansi-nist:ImageCaptureDetail>
//            <ansi-nist:CaptureResolutionCode>1</ansi-nist:CaptureResolutionCode>
//        </ansi-nist:ImageCaptureDetail>
//        <ansi-nist:ImageCompressionAlgorithmCode>2</ansi-nist:ImageCompressionAlgorithmCode>
//        <ansi-nist:ImageHorizontalLineLengthPixelQuantity>80</ansi-nist:ImageHorizontalLineLengthPixelQuantity>
//        <ansi-nist:ImageVerticalLineLengthPixelQuantity>65</ansi-nist:ImageVerticalLineLengthPixelQuantity>
//        <ansi-nist:FingerprintImagePosition>
//            <ansi-nist:FingerPositionCode>1</ansi-nist:FingerPositionCode>
//        </ansi-nist:FingerprintImagePosition>
//        <ansi-nist:FingerprintImageImpressionCaptureCategoryCode>3</ansi-nist:FingerprintImageImpressionCaptureCategoryCode>
//    </ansi-nist:FingerprintImage>
//</itl:PackageHighResolutionGrayscaleImageRecord>
{
	const char *pData;

	::ansinist::FingerprintImageType							fingerprintImage;

	//***********************************************
	//******* X.003: TX_IMP (Impression Type) *******
	//***********************************************
	//
	//<ansi-nist:FingerprintImageImpressionCaptureCategoryCode>3</ansi-nist:FingerprintImageImpressionCaptureCategoryCode>
	//
	if (Get("T4_IMP", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		fingerprintImage.FingerprintImageImpressionCaptureCategory().push_back(::ansinist::IMPCodeType(pData));
	}

	//***********************************************
	//******* X.004: TX_FGP (Finger Position) *******
	//***********************************************
	//
	//<ansi-nist:FingerprintImagePosition>
	//    <ansi-nist:FingerPositionCode>1</ansi-nist:FingerPositionCode>
	//</ansi-nist:FingerprintImagePosition>
	//
	if (Get("T4_FGP", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		::ansinist::FingerprintType								fingerprintType;

		fingerprintType.FingerPositionCode().push_back(::ansinist::FPCCodeType(pData));
		fingerprintImage.FingerprintImagePosition().push_back(fingerprintType);
	}

	//*********************************************************
	//******* X.005: TX_ISR (Image Scanning Resolution) *******
	//*********************************************************
	//
	//<ansi-nist:ImageCaptureDetail>
	//    <ansi-nist:CaptureResolutionCode>1</ansi-nist:CaptureResolutionCode>
	//</ansi-nist:ImageCaptureDetail>
	//
	if (Get("T4_ISR", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		::ansinist::ImageCaptureType							imageCaptureType;

		imageCaptureType.CaptureResolution(::ansinist::ISRCodeType(pData));
		fingerprintImage.ImageCaptureDetail(imageCaptureType);
	}

	//******************************************************
	//******* X.006: TX_HLL (Horizontal Line Length) *******
	//******************************************************
	//
	//<ansi-nist:ImageHorizontalLineLengthPixelQuantity>80</ansi-nist:ImageHorizontalLineLengthPixelQuantity>
	//
	if (Get("T4_HLL", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		fingerprintImage.ImageHorizontalLineLengthPixelQuantity(_ttol(pData));
	}

	//****************************************************
	//******* X.007: TX_VLL (Vertical Line Length) *******
	//****************************************************
	//
	//<ansi-nist:ImageVerticalLineLengthPixelQuantity>65</ansi-nist:ImageVerticalLineLengthPixelQuantity>
	//
	if (Get("T4_VLL", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		fingerprintImage.ImageVerticalLineLengthPixelQuantity(_ttol(pData));
	}

	//***************************************************************
	//******* X.008: TX_GCA (Grayscale Compression Algorithm) *******
	//***************************************************************
	//
	//<ansi-nist:ImageCompressionAlgorithmCode>2</ansi-nist:ImageCompressionAlgorithmCode>
	//
	if (Get("T4_GCA", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		fingerprintImage.ImageCompressionAlgorithm(::ansinist::GCACodeType(pData));
	}

	//******************************************
	//******* X.009: TX_DAT (Image Data) *******
	//******************************************
	//
	//<nc:BinaryBase64Object>mrHbPd....0H/+h</nc:BinaryBase64Object>
	//
	const char	*szFmt;
	long		lLen;
	if (GetImage(iRecordType, iRecordIndex, &szFmt, &lLen, (const void**)&pData) == IW_SUCCESS)
	{
		::xml_schema::base64_binary								base64ImageTmp(pData, lLen);
		::proxyxsd::base64Binary								base64Image(base64ImageTmp);
		fingerprintImage.BinaryObject().push_back(base64Image);
	}

	PackageImageRecordType packageImageRecord(recordCategoryCode,
											  imageReferenceIdentification,
											  fingerprintImage);
	packageImageRecords.push_back(packageImageRecord);

	return IW_SUCCESS;
}

int CIWTransaction::AddType7XML(vector <PackageImageRecordType> &packageImageRecords,
							    int							    iRecordType,
							    int							    iRecordIndex,
							    ::ansinist::RTCCodeType		    &recordCategoryCode,
							    ::niemcore::IdentificationType  &imageReferenceIdentification)
//<itl:PackageUserDefinedImageRecord>
//    <ansi-nist:RecordCategoryCode>07</ansi-nist:RecordCategoryCode>
//    <ansi-nist:ImageReferenceIdentification>
//        <nc:IdentificationID>05</nc:IdentificationID>
//    </ansi-nist:ImageReferenceIdentification>
//    <itl:ExampleRecordImage>
//        <!-- Well-formed XML goes here.  Users may define a substitute element. -->
//    </itl:ExampleRecordImage>
//</itl:PackageUserDefinedImageRecord>
{
	return IW_SUCCESS;
}

int CIWTransaction::AddType10XML(vector <PackageImageRecordType> &packageImageRecords,
							     int							 iRecordType,
							     int							 iRecordIndex,
							     ::ansinist::RTCCodeType		 &recordCategoryCode,
							     ::niemcore::IdentificationType  &imageReferenceIdentification)
//
// FACE example:
//
//<itl:PackageFacialAndSMTImageRecord>
//    <ansi-nist:RecordCategoryCode>10</ansi-nist:RecordCategoryCode>
//    <ansi-nist:ImageReferenceIdentification>
//        <nc:IdentificationID>01</nc:IdentificationID>
//    </ansi-nist:ImageReferenceIdentification>
//    <itl:FaceImage>
//        <nc:BinaryBase64Object>/9j/4AAQSkZJRg...8ytec5T/2Q==</nc:BinaryBase64Object>
//        <ansi-nist:ImageCaptureDetail>
//            <ansi-nist:CaptureDate>
//                <nc:Date>2008-02-20</nc:Date>
//            </ansi-nist:CaptureDate>
//            <ansi-nist:CaptureOrganization>
//                <nc:OrganizationIdentification>
//                    <nc:IdentificationID>TEST00001</nc:IdentificationID>
//                </nc:OrganizationIdentification>
//            </ansi-nist:CaptureOrganization>
//            <ansi-nist:CaptureDeviceMonitoringModeText>ASSISTED</ansi-nist:CaptureDeviceMonitoringModeText>
//        </ansi-nist:ImageCaptureDetail>
//        <ansi-nist:ImageColorSpaceCode>YCC</ansi-nist:ImageColorSpaceCode>
//        <ansi-nist:ImageCompressionAlgorithmText>JPEGB</ansi-nist:ImageCompressionAlgorithmText>
//        <ansi-nist:ImageHorizontalLineLengthPixelQuantity>1024</ansi-nist:ImageHorizontalLineLengthPixelQuantity>
//        <ansi-nist:ImageHorizontalPixelDensityValue>1</ansi-nist:ImageHorizontalPixelDensityValue>
//        <ansi-nist:ImageQuality>
//            <ansi-nist:QualityAlgorithmProductIdentification>
//                <nc:IdentificationID>99</nc:IdentificationID>
//            </ansi-nist:QualityAlgorithmProductIdentification>
//            <ansi-nist:QualityValue>88</ansi-nist:QualityValue>
//            <ansi-nist:QualityMeasureVendorIdentification>
//                <nc:IdentificationID>000F</nc:IdentificationID>
//            </ansi-nist:QualityMeasureVendorIdentification>
//        </ansi-nist:ImageQuality>
//        <ansi-nist:ImageScaleUnitsCode>0</ansi-nist:ImageScaleUnitsCode>
//        <ansi-nist:ImageCategoryCode>FACE</ansi-nist:ImageCategoryCode>
//        <ansi-nist:ImageVerticalLineLengthPixelQuantity>768</ansi-nist:ImageVerticalLineLengthPixelQuantity>
//        <ansi-nist:ImageVerticalPixelDensityValue>1</ansi-nist:ImageVerticalPixelDensityValue>
//        <ansi-nist:FaceImage3DPoseAngle>
//            <ansi-nist:PosePitchAngleMeasure>-30</ansi-nist:PosePitchAngleMeasure>
//            <ansi-nist:PosePitchUncertaintyValue>10</ansi-nist:PosePitchUncertaintyValue>
//            <ansi-nist:PoseRollAngleMeasure>0</ansi-nist:PoseRollAngleMeasure>
//            <ansi-nist:PoseRollUncertaintyValue>10</ansi-nist:PoseRollUncertaintyValue>
//            <ansi-nist:PoseYawAngleMeasure>45</ansi-nist:PoseYawAngleMeasure>
//            <ansi-nist:PoseYawUncertaintyValue>10</ansi-nist:PoseYawUncertaintyValue>
//        </ansi-nist:FaceImage3DPoseAngle>
//        <ansi-nist:FaceImageAcquisitionProfileCode>10</ansi-nist:FaceImageAcquisitionProfileCode>
//        <ansi-nist:FaceImageAttribute>
//            <ansi-nist:FaceImageAttributeCode>SCARF</ansi-nist:FaceImageAttributeCode>
//        </ansi-nist:FaceImageAttribute>
//        <ansi-nist:FaceImageEyeColorAttributeCode>BLU</ansi-nist:FaceImageEyeColorAttributeCode>
//        <ansi-nist:FaceImageFeaturePoint>
//            <ansi-nist:FeaturePointHorizontalCoordinateValue>123</ansi-nist:FeaturePointHorizontalCoordinateValue>
//            <ansi-nist:FeaturePointIdentification>
//                <nc:IdentificationID>3.14</nc:IdentificationID>
//            </ansi-nist:FeaturePointIdentification>
//            <ansi-nist:FeaturePointCategoryCode>1</ansi-nist:FeaturePointCategoryCode>
//            <ansi-nist:FeaturePointVerticalCoordinateValue>97</ansi-nist:FeaturePointVerticalCoordinateValue>
//        </ansi-nist:FaceImageFeaturePoint>
//        <ansi-nist:FaceImageHairColorAttributeCode>BLK</ansi-nist:FaceImageHairColorAttributeCode>
//        <ansi-nist:FaceImagePoseOffsetAngleMeasure>45</ansi-nist:FaceImagePoseOffsetAngleMeasure>
//        <ansi-nist:FaceImageSubjectPoseCode>D</ansi-nist:FaceImageSubjectPoseCode>
//        <itl:FaceImageAcquisitionSource>
//            <ansi-nist:CaptureSourceCode>DIGITAL CAMERA</ansi-nist:CaptureSourceCode>
//        </itl:FaceImageAcquisitionSource>
//    </itl:FaceImage>
//</itl:PackageFacialAndSMTImageRecord>
//
// SMT example:
//
//<itl:PackageFacialAndSMTImageRecord>
//    <ansi-nist:RecordCategoryCode>10</ansi-nist:RecordCategoryCode>
//    <ansi-nist:ImageReferenceIdentification>
//        <nc:IdentificationID>02</nc:IdentificationID>
//    </ansi-nist:ImageReferenceIdentification>
//    <ansi-nist:PhysicalFeatureImage>
//        <nc:BinaryBase64Object>/9j/4AAQSkZJRgA...pWkj/2Q==</nc:BinaryBase64Object>
//        <ansi-nist:ImageCaptureDetail>
//            <ansi-nist:CaptureDate>
//                <nc:Date>2008-02-20</nc:Date>
//            </ansi-nist:CaptureDate>
//            <ansi-nist:CaptureOrganization>
//                <nc:OrganizationIdentification>
//                    <nc:IdentificationID>TEST00001</nc:IdentificationID>
//                </nc:OrganizationIdentification>
//            </ansi-nist:CaptureOrganization>
//        </ansi-nist:ImageCaptureDetail>
//        <ansi-nist:ImageColorSpaceCode>YCC</ansi-nist:ImageColorSpaceCode>
//        <ansi-nist:ImageCompressionAlgorithmText>JPEGB</ansi-nist:ImageCompressionAlgorithmText>
//        <ansi-nist:ImageHorizontalLineLengthPixelQuantity>170</ansi-nist:ImageHorizontalLineLengthPixelQuantity>
//        <ansi-nist:ImageHorizontalPixelDensityValue>96</ansi-nist:ImageHorizontalPixelDensityValue>
//        <ansi-nist:ImageScaleUnitsCode>1</ansi-nist:ImageScaleUnitsCode>
//        <ansi-nist:ImageCategoryCode>TATTOO</ansi-nist:ImageCategoryCode>
//        <ansi-nist:ImageVerticalLineLengthPixelQuantity>158</ansi-nist:ImageVerticalLineLengthPixelQuantity>
//        <ansi-nist:ImageVerticalPixelDensityValue>96</ansi-nist:ImageVerticalPixelDensityValue>
//        <ansi-nist:PhysicalFeatureDescriptionDetail>
//            <ansi-nist:PhysicalFeatureColorDetail>
//                <ansi-nist:PhysicalFeaturePrimaryColorCode>BLUE</ansi-nist:PhysicalFeaturePrimaryColorCode>
//            </ansi-nist:PhysicalFeatureColorDetail>
//            <ansi-nist:PhysicalFeatureCategoryCode>TATTOO</ansi-nist:PhysicalFeatureCategoryCode>
//            <ansi-nist:PhysicalFeatureClassCode>OBJECT</ansi-nist:PhysicalFeatureClassCode>
//            <ansi-nist:PhysicalFeatureDescriptionText>Anchor</ansi-nist:PhysicalFeatureDescriptionText>
//            <ansi-nist:PhysicalFeatureSubClassCode>MOBJECTS</ansi-nist:PhysicalFeatureSubClassCode>
//        </ansi-nist:PhysicalFeatureDescriptionDetail>
//        <ansi-nist:PhysicalFeatureNCICCode>ABDOM</ansi-nist:PhysicalFeatureNCICCode>
//        <ansi-nist:PhysicalFeatureSize>
//            <ansi-nist:PhysicalFeatureHeightMeasure>30</ansi-nist:PhysicalFeatureHeightMeasure>
//            <ansi-nist:PhysicalFeatureWidthMeasure>20</ansi-nist:PhysicalFeatureWidthMeasure>
//        </ansi-nist:PhysicalFeatureSize>
//    </ansi-nist:PhysicalFeatureImage>
//</itl:PackageFacialAndSMTImageRecord>
//
{
	::standard_22008::FaceImageType								faceImage((::xml_schema::string)"0");
	::ansinist::PhysicalFeatureImageType						physicalFeatureImage;
	::ansinist::ImageCaptureType								imageCaptureDetails;
	bool			bFace = true;
	const char		*pData;
	const char		*pData1;
	const char		*pData2;
	const char		*pData3;
	const char		*pData4;
	int				y;
	unsigned short	m;
	unsigned short	d;

	//********************************************
	//******* 10.003: T10_IMT (Image Type) *******
	//********************************************
	//
	//<ansi-nist:ImageCategoryCode>FACE</ansi-nist:ImageCategoryCode>
	//
	if (Get("T10_IMT", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	// Here we decide if we will proceed with a "FaceImage" or a "PhysicalFeatureImage"
	{
		::ansinist::IMTCodeType									iMTCodeType(pData);
		if (_tcsicmp(pData, "FACE") == 0)
		{
			faceImage.ImageCategoryCode(iMTCodeType);
			bFace = true;
		}
		else // ("SCAR", "MARK", or "TATTOO" assumed)
		{
			physicalFeatureImage.ImageCategoryCode(iMTCodeType);
			bFace = false;
		}
	}

	//*****************************************************
	//******* 10.004: T10_SRC (Source Agency / ORI) *******
	//*****************************************************
	//
	//<ansi-nist:CaptureOrganization>
	//    <nc:OrganizationIdentification>
	//        <nc:IdentificationID>WI013415Y</nc:IdentificationID>
	//    </nc:OrganizationIdentification>
	//    <nc:OrganizationName>WI Crime Information Bureau</nc:OrganizationName>
	//</ansi-nist:CaptureOrganization>
	//
	if (Get("T10_SRC", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		::niemcore::IdentificationType							organizationIdentification;
		::niemcore::OrganizationType							captureOrganization;

		organizationIdentification.IdentificationID().push_back(pData);
		captureOrganization.OrganizationIdentification().push_back(organizationIdentification);
		imageCaptureDetails.CaptureOrganization(captureOrganization);
	}

	//********************************************
	//******* 10.005: T10_PHD (Photo Date) *******
	//********************************************
	//
	//<ansi-nist:CaptureDate>
	//    <nc:Date>1953-04-23</nc:Date>
	//</ansi-nist:CaptureDate>
	//
	if (Get("T10_PHD", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (DecomposeYYYYMMDD(pData, &y, &m, &d))
		{
			::xml_schema::date											captureDateTempTemp(y, m, d);
			::proxyxsd::date											captureDateTemp(captureDateTempTemp);
			::niemcore::DateType										captureDate;

			captureDate.DateRepresentation().push_back(captureDateTemp);
			imageCaptureDetails.CaptureDate(captureDate);
		}
		else
		{
			//failed decomposition (todo)
		}
	}

	//********************************************************
	//******* 10.006: T10_HLL (Horizontal Line Length) *******
	//********************************************************
	//
	//<ansi-nist:ImageHorizontalLineLengthPixelQuantity>80</ansi-nist:ImageHorizontalLineLengthPixelQuantity>
	//
	if (Get("T10_HLL", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageHorizontalLineLengthPixelQuantity(_ttol(pData));
		else physicalFeatureImage.ImageHorizontalLineLengthPixelQuantity(_ttol(pData));
	}

	//******************************************************
	//******* 10.007: T10_VLL (Vertical Line Length) *******
	//******************************************************
	//
	//<ansi-nist:ImageVerticalLineLengthPixelQuantity>65</ansi-nist:ImageVerticalLineLengthPixelQuantity>
	//
	if (Get("T10_VLL", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageVerticalLineLengthPixelQuantity(_ttol(pData));
		else physicalFeatureImage.ImageVerticalLineLengthPixelQuantity(_ttol(pData));
	}

	//*********************************************
	//******* 10.008: T10_SLC (Scale Units) *******
	//*********************************************
	//
	//<ansi-nist:ImageScaleUnitsCode>0</ansi-nist:ImageScaleUnitsCode>
	//
	if (Get("T10_SLC", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageScaleUnitsCode(::ansinist::SLCCodeType(pData));
		else physicalFeatureImage.ImageScaleUnitsCode(::ansinist::SLCCodeType(pData));
	}

	//********************************************************
	//******* 10.009: T10_HPS (Horizontal Pixel Scale) *******
	//********************************************************
	//
	//<ansi-nist:ImageHorizontalPixelDensityValue>1</ansi-nist:ImageHorizontalPixelDensityValue>
	//
	if (Get("T10_HPS", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageHorizontalPixelDensityValue(_ttol(pData));
		else physicalFeatureImage.ImageHorizontalPixelDensityValue(_ttol(pData));
	}

	//******************************************************
	//******* 10.010: T10_VPS (Vertical Pixel Scale) *******
	//******************************************************
	//
	//<ansi-nist:ImageVerticalPixelDensityValue>1</ansi-nist:ImageVerticalPixelDensityValue>
	//
	if (Get("T10_VPS", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageVerticalPixelDensityValue(_ttol(pData));
		else physicalFeatureImage.ImageVerticalPixelDensityValue(_ttol(pData));
	}

	//*******************************************************
	//******* 10.011: T10_CGA (Compression Algorithm) *******
	//*******************************************************
	//
	//<ansi-nist:ImageCompressionAlgorithmText>JPEGB</ansi-nist:ImageCompressionAlgorithmText>
	//
	if (Get("T10_CGA", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageCompressionAlgorithm(::niemcore::TextType(pData));
		else physicalFeatureImage.ImageCompressionAlgorithm(::niemcore::TextType(pData));
	}

	//*********************************************
	//******* 10.012: T10_CSP (Color Space) *******
	//*********************************************
	//
	//<ansi-nist:ImageColorSpaceCode>YCC</ansi-nist:ImageColorSpaceCode>
	//
	if (Get("T10_CSP", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
	{
		if (bFace)      faceImage.ImageColorSpace(::ansinist::CSPCodeType(pData));
		else physicalFeatureImage.ImageColorSpace(::ansinist::CSPCodeType(pData));
	}	

	//********************************************
	//******* 10.999: T10_DAT (Image Data) *******
	//********************************************
	//
	//<nc:BinaryBase64Object>mrHbPd....0H/+h</nc:BinaryBase64Object>
	//
	const char	*szFmt;
	long		lLen;
	if (GetImage(iRecordType, iRecordIndex, &szFmt, &lLen, (const void**)&pData) == IW_SUCCESS)
	{
		::xml_schema::base64_binary								base64ImageTmp(pData, lLen);
		::proxyxsd::base64Binary								base64Image(base64ImageTmp);

		if (bFace)      faceImage.BinaryObject().push_back(base64Image);
		else physicalFeatureImage.BinaryObject().push_back(base64Image);
	}

	if (bFace)
	//
	// Fields specific to FaceImage
	//
	{
		//*************************************************************
		//******* 10.013: T10_SAP (Subject Acquisition Profile) *******
		//*************************************************************
		//
		//<ansi-nist:FaceImageAcquisitionProfileCode>10</ansi-nist:FaceImageAcquisitionProfileCode>
		//
		if (Get("T10_SAP", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			faceImage.FaceImageAcquisitionProfile(::ansinist::SAPCodeType(pData));
		}
		else
		{
			// Mandatory field! Error case.
			return IW_WARN_REQ_FIELD_MISSING;
		}

		//**********************************************
		//******* 10.020: T10_POS (Subject Pose) *******
		//**********************************************
		//
		//<ansi-nist:FaceImageSubjectPoseCode>F</ansi-nist:FaceImageSubjectPoseCode>
		//
		if (Get("T10_POS", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			faceImage.FaceImageSubjectPose(::ansinist::POSCodeType(pData));
		}

		//***************************************************
		//******* 10.021: T10_POA (Pose Offset Angle) *******
		//***************************************************
		//**********************************************
		//
		//<ansi-nist:FaceImagePoseOffsetAngleMeasure>45</ansi-nist:FaceImagePoseOffsetAngleMeasure>
		//
		if (Get("T10_POA", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			faceImage.FaceImagePoseOffsetAngleMeasure(_ttol(pData));
		}	

		//***************************************************
		//******* 10.022: T10_PXS (Photo Description) *******
		//***************************************************
		//
		//<ansi-nist:FaceImageAttribute>
		//    <ansi-nist:FaceImageAttributeCode>SCARF</ansi-nist:FaceImageAttributeCode>
		//</ansi-nist:FaceImageAttribute>
		//
		if (Get("T10_PXS", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			::ansinist::FaceImageAttributeType								faceImageAttribute;

			faceImageAttribute.FaceImageAttributeAbstract().push_back((::ansinist::PXSCodeType)pData);
			faceImage.FaceImageAttribute().push_back(faceImageAttribute);
		}

		//**********************************************************
		//******* 10.023: T10_PAS (Photo Acquisition Source) *******
		//**********************************************************
		//
		//<itl:FaceImageAcquisitionSource>
		//    <ansi-nist:CaptureSourceCode>DIGITAL CAMERA</ansi-nist:CaptureSourceCode>
		//</itl:FaceImageAcquisitionSource>
		//
		if (Get("T10_PAS", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			::standard_22008::FaceImageAcquisitionSourceType				faceImageAcquisitionSource((::ansinist::PASCodeType)pData);

			faceImage.FaceImageAcquisitionSource(faceImageAcquisitionSource);
		}

		//*******************************************************
		//******* 10.024: T10_SQS (Subject Quality Score) *******
		//*******************************************************
		//
		//<ansi-nist:ImageQuality>
		//    <ansi-nist:QualityAlgorithmProductIdentification>
		//        <nc:IdentificationID>99</nc:IdentificationID>
		//    </ansi-nist:QualityAlgorithmProductIdentification>
		//    <ansi-nist:QualityValue>88</ansi-nist:QualityValue>
		//    <ansi-nist:QualityMeasureVendorIdentification>
		//        <nc:IdentificationID>000F</nc:IdentificationID>
		//    </ansi-nist:QualityMeasureVendorIdentification>
		//</ansi-nist:ImageQuality>
		//
		::ansinist::ImageQualityType										imageQuality;
		if (Get("T10_SQS_QMV", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			imageQuality.QualityValue().push_back(_ttol(pData));
		}
		if (Get("T10_SQS_VID", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			::niemcore::IdentificationType									vendorIdentification;

			vendorIdentification.IdentificationID().push_back(pData);
			imageQuality.QualityMeasureVendorIdentification().push_back(vendorIdentification);
		}
		if (Get("T10_SQS_PID", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			::niemcore::IdentificationType									productIdentification;

			productIdentification.IdentificationID().push_back(pData);
			imageQuality.QualityAlgorithmProductIdentification().push_back(productIdentification);
		}
		faceImage.ImageQuality().push_back(imageQuality);

		//*******************************************************
		//******* 10.025: T10_SPA (Subject Quality Score) *******
		//*******************************************************
		//
		//<ansi-nist:FaceImage3DPoseAngle>
		//    <ansi-nist:PosePitchAngleMeasure>-30</ansi-nist:PosePitchAngleMeasure>
		//    <ansi-nist:PosePitchUncertaintyValue>10</ansi-nist:PosePitchUncertaintyValue>
		//    <ansi-nist:PoseRollAngleMeasure>0</ansi-nist:PoseRollAngleMeasure>
		//    <ansi-nist:PoseRollUncertaintyValue>10</ansi-nist:PoseRollUncertaintyValue>
		//    <ansi-nist:PoseYawAngleMeasure>45</ansi-nist:PoseYawAngleMeasure>
		//    <ansi-nist:PoseYawUncertaintyValue>10</ansi-nist:PoseYawUncertaintyValue>
		//</ansi-nist:FaceImage3DPoseAngle>
		//
		if (Get("T10_SPA_PIT", &pData1, 1, iRecordIndex) == IW_SUCCESS && pData1)
		{
			if (Get("T10_SPA_ROL", &pData2, 1, iRecordIndex) == IW_SUCCESS && pData2)
			{
				if (Get("T10_SPA_YAW", &pData3, 1, iRecordIndex) == IW_SUCCESS && pData3)
				{
					::ansinist::PoseAngleType											poseAngle(_ttol(pData1),
																								  _ttol(pData2),
																								  _ttol(pData3));
					if (Get("T10_SPA_PUN", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
					{
						poseAngle.PosePitchUncertaintyValue(_ttol(pData));
					}
					if (Get("T10_SPA_RUN", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
					{
						poseAngle.PoseRollUncertaintyValue(_ttol(pData));
					}
					if (Get("T10_SPA_YUN", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
					{
						poseAngle.PoseYawUncertaintyValue(_ttol(pData));
					}
					faceImage.FaceImage3DPoseAngle(poseAngle);
				}
			}
		}

		//************************************************************
		//******* 10.026: T10_SXS (Subject Facial Description) *******
		//************************************************************
		//
		//
		if (Get("T10_SPA_SXS", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			faceImage.FaceImageDescription().push_back((::ansinist::SXSCodeType)pData);
		}

		//***************************************************
		//******* 10.027: T10_SEC (Subject Eye Color) *******
		//***************************************************
		//
		//<ansi-nist:FaceImageEyeColorAttributeCode>BLU</ansi-nist:FaceImageEyeColorAttributeCode>
		//
		if (Get("T10_SEC", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			faceImage.FaceImageEyeColorAttribute((::ansinist::SECCodeType)pData);
		}

		//****************************************************
		//******* 10.028: T10_SHC (Subject Hair Color) *******
		//****************************************************
		//
		//<ansi-nist:FaceImageHairColorAttributeCode>BLK</ansi-nist:FaceImageHairColorAttributeCode>
		//
		if (Get("T10_SHC", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			faceImage.FaceImageHairColorAttribute().push_back((::ansinist::SHCCodeType)pData);
		}

		//*******************************************************
		//******* 10.029: T10_FFP (Facial Feature Points) *******
		//*******************************************************
		//
		//<ansi-nist:FaceImageFeaturePoint>
		//    <ansi-nist:FeaturePointHorizontalCoordinateValue>123</ansi-nist:FeaturePointHorizontalCoordinateValue>
		//    <ansi-nist:FeaturePointIdentification>
		//        <nc:IdentificationID>3.14</nc:IdentificationID>
		//    </ansi-nist:FeaturePointIdentification>
		//    <ansi-nist:FeaturePointCategoryCode>1</ansi-nist:FeaturePointCategoryCode>
		//    <ansi-nist:FeaturePointVerticalCoordinateValue>97</ansi-nist:FeaturePointVerticalCoordinateValue>
		//</ansi-nist:FaceImageFeaturePoint>
		//
		// Note: A typo in ANSI/NIST-ITL 1-2007 would have this mnemonic be 'SFT' at times.
		// This typo has even propogated to ANSI/NIST-ITL 2-2008.
		//
		if (Get("T10_FFP_X", &pData1, 1, iRecordIndex) == IW_SUCCESS && pData1)
		{
			if (Get("T10_FFP_FPC", &pData2, 1, iRecordIndex) == IW_SUCCESS && pData2)
			{
				if (Get("T10_FFP_FPT", &pData3, 1, iRecordIndex) == IW_SUCCESS && pData3)
				{
					if (Get("T10_FFP_Y", &pData4, 1, iRecordIndex) == IW_SUCCESS && pData4)
					{
						::niemcore::IdentificationType						featurePointIdentification;
						featurePointIdentification.IdentificationID().push_back(pData2);
						::ansinist::FaceImageFeaturePointType				faceImageFeaturePoint(atof(pData1),
																								  featurePointIdentification,
																								  (::ansinist::SFPCodeType)pData3,
																								  atof(pData4));
						faceImage.FaceImageFeaturePoint().push_back(faceImageFeaturePoint);
					}
				}
			}
		}

		//********************************************************
		//******* 10.030: T10_DMM (Device Monitoring Mode) *******
		//********************************************************
		//
		//<ansi-nist:CaptureDeviceMonitoringModeText>ASSISTED</ansi-nist:CaptureDeviceMonitoringModeText>
		//
		// Note that the EBTS 8.001 XML IEP Draft sample XML contains:
		//    <ansi-nist:CaptureDeviceMonitoringModeCode>ASSISTED</ansi-nist:CaptureDeviceMonitoringModeCode>
		// but in the ansi-nist_itl_constraint.xsd schema the type ansi-nist:DMMCodeType is (erroneously?)
		// mapped to the substitution group CaptureDescriptionCode, whereas  CaptureDeviceMonitoringModeCode
		// only get the type nc:TextType.
		//
		if (Get("T10_DMM", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			imageCaptureDetails.CaptureDeviceMonitoringMode((::niemcore::TextType)pData);	//::ansinist::DMMCodeType(?)
		}

		faceImage.ImageCaptureDetail(imageCaptureDetails);

		PackageImageRecordType packageImageRecord(recordCategoryCode,
												  imageReferenceIdentification,
												  faceImage);
		packageImageRecords.push_back(packageImageRecord);
	}
	else
	//
	// Fields specific to PhysicalFeatureImage
	//
	{
		//*******************************************************
		//******* 10.040: T10_SMT (NCIC Designation Code) *******
		//*******************************************************
		//
		// <ansi-nist:PhysicalFeatureNCICCode>ABDOM</ansi-nist:PhysicalFeatureNCICCode>
		//
		if (Get("T10_SMT", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
		{
			physicalFeatureImage.PhysicalFeatureNCICCode().push_back((::fbi::SMTCodeType)pData);
		}

		//*******************************************************
		//******* 10.041: T10_SMS (Scar/Mark/Tattoo Size) *******
		//*******************************************************
		//
		//<ansi-nist:PhysicalFeatureDescriptionDetail>
		//    <ansi-nist:PhysicalFeatureSize>
		//        <ansi-nist:PhysicalFeatureHeightMeasure>30</ansi-nist:PhysicalFeatureHeightMeasure>
		//        <ansi-nist:PhysicalFeatureWidthMeasure>20</ansi-nist:PhysicalFeatureWidthMeasure>
		//    </ansi-nist:PhysicalFeatureSize>
		//
		if (Get("T10_SMS_HEIGHT", &pData1, 1, iRecordIndex) == IW_SUCCESS && pData1)
		{
			if (Get("T10_SMS_WIDTH", &pData2, 1, iRecordIndex) == IW_SUCCESS && pData2)
			{
				::ansinist::PhysicalFeatureSizeType				physicalFeatureSize(_ttol(pData1), _ttol(pData2));

				physicalFeatureImage.PhysicalFeatureSize(physicalFeatureSize);
			}
		}

		//*************************************************
		//******* 10.042: T10_SMD (SMT Descriptors) *******
		//*************************************************
		//
		//<ansi-nist:PhysicalFeatureDescriptionDetail>
		//    <ansi-nist:PhysicalFeatureCategoryCode>TATTOO</ansi-nist:PhysicalFeatureCategoryCode>
		//    <ansi-nist:PhysicalFeatureClassCode>OBJECT</ansi-nist:PhysicalFeatureClassCode>
		//    <ansi-nist:PhysicalFeatureDescriptionText>Anchor</ansi-nist:PhysicalFeatureDescriptionText>
		//    <ansi-nist:PhysicalFeatureSubClassCode>MOBJECTS</ansi-nist:PhysicalFeatureSubClassCode>
		//
		if (Get("T10_SMD_SRC", &pData1, 1, iRecordIndex) == IW_SUCCESS && pData1)
		{
			if (Get("T10_SMD_CLASS", &pData2, 1, iRecordIndex) == IW_SUCCESS && pData2)
			{
				// Note: based on the CLASS field, the folowing field could be T10_SMD_HUMAN, T10_SMD_ANIMAL
				// T10_SMD_PLANT, T10_SMD_FLAG, T10_SMD_OBJECT, T10_SMD_ABSTRACT, T10_SMD_SYMBOL or T10_SMD_OTHER.
				// However, all the mnemonics have the same index value "10.042..3", so we just choose any one.
				if (Get("T10_SMD_HUMAN", &pData3, 1, iRecordIndex) == IW_SUCCESS && pData3)
				{
					// Mandatory 3 fields
					::ansinist::PhysicalFeatureDescriptionType				physicalFeatureDescriptionDetail((::ansinist::SMDCodeType)pData1,
																											 (::ansinist::PCCCodeType)pData2,
																											 (::ansinist::PSCCodeType)pData3);
					// Optional 4th field
					if (Get("T10_SMD_literal", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
					{
						physicalFeatureDescriptionDetail.PhysicalFeatureDescriptionText(pData);
					}

					// Note: T10_COL, if present, is associated with T10_SMD
					//************************************************
					//******* 10.043: T10_COL (Colors Present) *******
					//************************************************
					//
					//<ansi-nist:PhysicalFeatureDescriptionDetail>
					//    <ansi-nist:PhysicalFeatureColorDetail>
					//        <ansi-nist:PhysicalFeaturePrimaryColorCode>BLUE</ansi-nist:PhysicalFeaturePrimaryColorCode>
					//    </ansi-nist:PhysicalFeatureColorDetail>
					//
					if (Get("T10_COL", &pData, 1, iRecordIndex) == IW_SUCCESS && pData)
					{
						::ansinist::PhysicalFeatureColorDetailType			physicalFeatureColorDetail((::ansinist::COLCodeType)pData);

						physicalFeatureDescriptionDetail.PhysicalFeatureColorDetail(physicalFeatureColorDetail);
					}

					physicalFeatureImage.PhysicalFeatureDescriptionDetail().push_back(physicalFeatureDescriptionDetail);
				}
			}
		}

		physicalFeatureImage.ImageCaptureDetail(imageCaptureDetails);

		PackageImageRecordType packageImageRecord(recordCategoryCode,
												  imageReferenceIdentification,
												  physicalFeatureImage);
		packageImageRecords.push_back(packageImageRecord);
	}

	return IW_SUCCESS;
}
