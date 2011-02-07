
#ifndef _IWTRANSACTION_H_
#define _IWTRANSACTION_H_

class CNISTRecord;
class CIWVerification;

class CNISTErr
{
public:
	CStdString	m_sErr;
	int			m_nCode;
};

class CIWTransaction
{
private:
	BYTE			*m_pTransactionData;
	CStdStringPath	m_sFilePath;
	int				m_nIDCDigits;
	double			m_dNativeResolutionPPMM;
	CIWVerification *m_pVerification;
	bool			m_bTransactionLoaded;		// Indicates the transaction file was successfully read

	CStringSlots	m_stringSlots;

	// Array of records contained by transaction
	std::vector<CNISTRecord*> m_RecordAry;

	// Array of possible errors during Verify
	std::vector<CNISTErr> m_ErrAry;

	CNISTRecord *GetRecord(int nRecordType, int nRecordIndex);
	int Type1AddRecordIDC(CNISTRecord *pRecord, int nRecordType, int nIDC);
	int Type1UpdateIDC(CNISTRecord *pRecord, int nIDC);
	int Type1DeleteRecordIDC(CNISTRecord *pRecord, int nIDC);
	void DebugOutRecords(CStdString sContext);
	int SetRecordLengths();
	int GetRecordLengths();

public:
	CIWTransaction();
	virtual ~CIWTransaction();

	TCHAR* CreateNewStringSlot(CStdString s) { return m_stringSlots.AddNew(s); };

	int ReadTransactionFile(CStdStringPath sFilePath);
	int ReadTransactionFileMem(const BYTE* pMemFile, int MemFileSize);
	int GetRecords();
	bool IsTransactionLoaded() { return m_bTransactionLoaded; } 
	bool IsVerificationLoaded(); 

	/************************************************************/
	/*                                                          */
	/*               Transaction Management                      */
	/*                                                          */
	/************************************************************/

	int GetNumRecords(int *pRecords);
	int GetRecordTypeCount(int RecordType, int *pRecordTypeCount);
	int GetRecordTypeMaxIndex(int RecordType, int *pIndex);
	int New(CStdString sTransactionType, CIWVerification *pIWVer);
	int AddRecord(int RecordType, int *pRecordIndex);
	int DeleteRecord(int RecordType, int RecordIndex);

	int Write(CStdStringPath sPath);
	int WriteBinary(CStdStringPath sPath);
	int WriteXML(CStdStringPath sPath, bool bValidate);
    int WriteMem(BYTE** ppBuffer, int *pSize);

	/************************************************************/
	/*                   XML Specific (2-2008)                  */
	/************************************************************/

	int  GetXML(BYTE **ppXML, bool bValidate, int *pnLengthXML, CStdString& sErr);
	/*

	For future use

	int  GetXMLForType1Record(auto_ptr <PackageInformationRecordType> &packageInformationRecord, char **pErr);
	int  GetXMLForType2Record(auto_ptr <PackageDescriptiveTextRecordType> &packageDescriptiveTextRecord, char **pErr);
	int	 GetXMLForImageTypeRecords(vector <PackageImageRecordType> &packageImageRecords, char **pErr);
	int  AddType3Type4Type5Type6XML(vector <PackageImageRecordType> &packageImageRecords, int iRecordType, int iRecordIndex,
									::ansinist::RTCCodeType &recordCategoryCode, ::niemcore::IdentificationType  &imageReferenceIdentification);
	int  AddType7XML(vector <PackageImageRecordType> &packageImageRecords, int iRecordType, int iRecordIndex,
					 ::ansinist::RTCCodeType &recordCategoryCode, ::niemcore::IdentificationType  &imageReferenceIdentification);
	int  AddType10XML(vector <PackageImageRecordType> &packageImageRecords, int iRecordType, int iRecordIndex,
					  ::ansinist::RTCCodeType &recordCategoryCode, ::niemcore::IdentificationType  &imageReferenceIdentification);

	void SerializeDOMtoXMLFormatTarget(xercesc::XMLFormatTarget& target, const xercesc::DOMDocument& doc, const std::string& encoding = "UTF-8");
	void RemoveEmptyElementsFromDOM(DOMElement* e);
	void SetProperPackageImageRecordSubstitutions(::xercesc::DOMDocument& d, DOMElement* e);
	bool HandleXMLSchemaException(char **pErr);

	bool DecomposeYYYYMMDD(const char *szDate, int *py, unsigned short *pm, unsigned short *pd);
	bool DecomposeT1_GMT(const char *szDate, int *py, unsigned short *pm, unsigned short *pd,
						 unsigned short *pgh, unsigned short *pgm, unsigned short *pgs);
	bool DecomposeNAME(const char *szFullName, char szGivenName[256], char szMiddleName[256], char szSurName[256]);
	bool DecomposeXXYY(const char *szAgeRange, int *pmin, int *pmax);
	bool DecomposeXXXYYY(const char *szHeightRange, int *pmin, int *pmax);
	bool DecomposeTABSEP3(const char *sz, char sz1[256], char sz2[256], char sz3[256]);

	int	 ReturnError(const char *szErr, char **pErr);
	bool GetSchemaPath(char szSchema[_MAX_PATH]);
	*/

	/************************************************************/
	/*                   NIST Data Access                       */
	/************************************************************/

	int GetNextField(int RecordType, int RecordIndex, int Field, int *pNextField);
	int GetNumSubfields(int RecordType, int RecordIndex, int Field, int *pCount);
	int GetNumItems(int RecordType, int RecordIndex, int Field, int Subfield, int *pCount);
	int FindItem(int RecordType, int RecordIndex, int Field, int Subfield, int Item, CStdString& sData);
	int SetItem(CStdString sData, int RecordType, int RecordIndex, int Field, int Subfield, int Item);
	int RemoveItem(int RecordType, int RecordIndex, int Field, int Subfield, int Item);

	/************************************************************/
	/*        Verification based NIST Data Access               */
	/************************************************************/

	int SetVerification(CIWVerification *pIWVer);
	int Get(CStdString sMnemonic, CStdString& sData, int Index, int RecordIndex);
	int Set(CStdString sMnemonic, CStdString sData, int StartIndex, int RecordIndex);
	int Occurrences(CStdString sMnemonic, int *pnOccurrences, int RecordIndex);
	int Verify();
	int GetErrorCount();
	int GetError(int Index, int* Code, CStdString& sDesc);
	int Remove(CStdString sMnemonic, int Index, int RecordIndex);

	void FreeErrors();
	void AddError(CStdString sErr, int nCode);

	/************************************************************/
	/*                                                          */
	/*                        Images                            */
	/*                                                          */
	/************************************************************/

	int GetImage(int RecordType,int RecordIndex, CStdString& sStorageFormat, int *pLength, const BYTE **ppData);
	int SetImage(int RecordType, int RecordIndex, CStdString sInputFormat, int nLength, BYTE *pData, CStdString sStorageFormat, float Compression);
	int	ImportImage(int RecordType, int RecordIndex, CStdStringPath sPath, CStdString sStorageFormat, float Compression, CStdString sInputFormat);
	int ExportImage(int RecordType, int RecordIndex, CStdStringPath sPath, CStdString sOutputFormat);
	int GetImageInfo(int RecordType, int RecordIndex, CStdString& sStorageFormat, int *pnLength, int *pnHLL, int *pnVLL, int *pnBitsPerPixel);
};

#endif // _IWTRANSACTION_H_
