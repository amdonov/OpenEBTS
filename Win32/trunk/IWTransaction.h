

#ifndef _IWTRANSACTION_H_
#define _IWTRANSACTION_H_

class CNISTRecord;
class CIWVerification;

#define MAXERRORLEN 4096

class CNISTErr
{
public:
	char m_szErr[MAXERRORLEN];
	int  m_nCode;
};

class CIWTransaction
{
private:
	char *m_pTransactionData;
	CStdString m_sFilePath;
	int m_nIDCDigits;
	double m_dNativeResolutionPPMM;

	CIWVerification *m_pVerification;

	// Indicates the transaction file was successfully read
	BOOL m_bTransactionLoaded; 

	// Array of records contained by transaction
	std::vector<CNISTRecord*> m_RecordAry;

	// Array of possible errors during Verify
	std::vector<CNISTErr> m_ErrAry;

	CNISTRecord *GetRecord(int nRecordType, int nRecordIndex);
	int Type1AddRecordIDC(CNISTRecord *pRecord, int nRecordType, int nIDC);
	int Type1UpdateIDC(CNISTRecord *pRecord, int nIDC);
	int Type1DeleteRecordIDC(CNISTRecord *pRecord, int nIDC);
	int DebugOutRecords(const char *szContext);
	int SetRecordLengths();

public:
	CIWTransaction();
	virtual ~CIWTransaction();

	int ReadTransactionFile(const char *pFilePath);
	int GetRecords();
	BOOL IsTransactionLoaded() { return m_bTransactionLoaded; } 
	BOOL IsVerificationLoaded(); 

	/************************************************************/
	/*                                                          */
	/*               Transaction Management                      */
	/*                                                          */
	/************************************************************/

	int GetNumRecords(int *pRecords);
	int GetRecordTypeCount(int RecordType, int *pRecordTypeCount);
	int GetRecordTypeMaxIndex(int RecordType, int *pIndex);
	int New(const char *pszTransactionType, CIWVerification *pIWVer);
	int AddRecord(int RecordType, int *pRecordIndex);
	int DeleteRecord(int RecordType, int RecordIndex);

	int Write(const char *pPath);
	int WriteBinary(const char *pPath);
	int WriteXML(const char *pPath, BOOL bValidate);
	
	/************************************************************/
	/*                   XML Specific (2-2008)                  */
	/************************************************************/

	int  GetXML(BYTE **ppXML, BOOL bValidate, long *plLengthXML, char **pErr);
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
	BOOL HandleXMLSchemaException(char **pErr);

	BOOL DecomposeYYYYMMDD(const char *szDate, int *py, unsigned short *pm, unsigned short *pd);
	BOOL DecomposeT1_GMT(const char *szDate, int *py, unsigned short *pm, unsigned short *pd,
						 unsigned short *pgh, unsigned short *pgm, unsigned short *pgs);
	BOOL DecomposeNAME(const char *szFullName, char szGivenName[256], char szMiddleName[256], char szSurName[256]);
	BOOL DecomposeXXYY(const char *szAgeRange, int *pmin, int *pmax);
	BOOL DecomposeXXXYYY(const char *szHeightRange, int *pmin, int *pmax);
	BOOL DecomposeTABSEP3(const char *sz, char sz1[256], char sz2[256], char sz3[256]);

	int	 ReturnError(const char *szErr, char **pErr);
	BOOL GetSchemaPath(char szSchema[_MAX_PATH]);
	*/

	/************************************************************/
	/*                   NIST Data Access                       */
	/************************************************************/

	int GetNextField(int RecordType, int RecordIndex, int Field, int *pNextField);
	int GetNumSubfields(int RecordType, int RecordIndex, int Field, int *pCount);
	int GetNumItems(int RecordType, int RecordIndex, int Field, 
										int Subfield, int *pCount);
	int FindItem(int RecordType, int RecordIndex, int Field, 
									int Subfield, int Item, const char **ppData);
	int SetItem(const char* Data, int RecordType, int RecordIndex,
									int Field, int Subfield, int Item);

	/************************************************************/
	/*        Verification based NIST Data Access               */
	/************************************************************/

	int SetVerification(CIWVerification *pIWVer);
	int Get(const char *pMnemonic, const char **ppData, int Index, int RecordIndex);
	int Set(const char *pMnemonic, const char *pData, int StartIndex, int RecordIndex);
	int Occurrences(const char *pMnemonic, int *pOccurrences, int RecordIndex);
	int Verify();
	int GetErrorCount();
	int GetError(int Index, int* Code, const char** Desc);

	void FreeErrors();
	void AddError(char* szErr, int nCode);

	/************************************************************/
	/*                                                          */
	/*                        Images                            */
	/*                                                          */
	/************************************************************/

	int GetImage(int RecordType,int RecordIndex, const char **ppStorageFormat,
								long *pLength, const void **ppData);

	int SetImage(int RecordType, int RecordIndex, const char *pInputFormat, 
											long Length, void *pData, const char *pStorageFormat,
														float Compression);
	int	ImportImage(int RecordType, int RecordIndex, const char* Path,
								const char* StorageFormat, float Compression,
								const char* InputFormat);
	int ExportImage(int RecordType, int RecordIndex,
								const char* Path, const char* OutputFormat);

	int GetImageInfo(int RecordType, int RecordIndex, const char **ppStorageFormat,
								long *pLength, long *phll, long *pvll, int *pBitsPerPixel);
};

#endif // _IWTRANSACTION_H_
