
#ifndef _OPENEBTS_H_
#define _OPENEBTS_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OPENEBTS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OPENEBTS_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef OPENEBTS_EXPORTS
#define OPENEBTS_API __declspec(dllexport)
#else
#define OPENEBTS_API __declspec(dllimport)
#endif

#ifdef  __cplusplus
  class CIWTransaction;
  class CIWVerification;
#else
  typedef struct CIWTransaction CIWTransaction;
  typedef struct CIWVerification CIWVerification;
#endif

#define IW_MAX_PARSE_ERROR 1023

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _FINGERPRINT_HEADER_
{
	BYTE bFieldLen[4];
	BYTE bIDC;
	BYTE bImp;		// finger impression type
	BYTE bFGP[6];	// finger position
	BYTE bISR;		// scanning resolution
	BYTE bHLL[2];	// Horiz line length
	BYTE bVLL[2];	// Vert line length
	BYTE bComp;		// compression algorithm used
} FINGERPRINT_HEADER;

typedef struct _SIGNATURE_HEADER_
{
	BYTE bFieldLen[4];
	BYTE bIDC;
	BYTE bSIG;		// signature type
	BYTE bSRT;		// signature representation type
	BYTE bISR;		// scanning resolution
	BYTE bHLL[2];	// Horiz line length
	BYTE bVLL[2];	// Vert line length
} SIGNATURE_HEADER;

//************************************************************/
//                                                            /
//                  Transaction Management                    /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWNew(
	const TCHAR* szTransactionType,			// Type of transaction
	CIWVerification* pIWVer,				// Verification pointer
	CIWTransaction** ppIWTrans				// Pointer to transaction pointer
);

OPENEBTS_API int WINAPI IWRead(
	const TCHAR *szPath,					// Path of transaction file
	CIWVerification *pIWVer,				// Verification pointer
	CIWTransaction **ppIWTrans				// Pointer to transaction pointer
);

OPENEBTS_API int WINAPI IWReadMem(
	BYTE *pBuffer,							// Pointer to transaction buffer
	int nBufferSize,						// Size of the transaction buffer
	CIWVerification *pIWVer,				// Verification pointer
	CIWTransaction **ppIWTrans				// Pointer to transaction pointer
);

OPENEBTS_API int WINAPI IWClose(
	CIWTransaction **ppIWTrans				// Pointer to transaction pointer
);

OPENEBTS_API int WINAPI IWWriteMem(
   CIWTransaction* pIWTrans,				// Transaction pointer
   BYTE** ppBuffer,							// Pointer to Transaction buffer pointer
   int *pSize								// Size of buffer
);											

OPENEBTS_API int WINAPI IWAddRecord(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int* pnRecordIndex						// New record index
);

OPENEBTS_API int WINAPI IWGetRecordTypeCount(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int* pnRecordTypeCount					// Number of records returned
);

OPENEBTS_API int WINAPI IWWrite(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szPath						// Path of transaction file to create
);

OPENEBTS_API int WINAPI IWWriteXML(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szPath,					// Path of transaction file to create
	bool bValidate							// Validate produced XML against NIST schemas
);

OPENEBTS_API int WINAPI IWGetNumRecords(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int* pnRecords							// Number of records returned
);

OPENEBTS_API int WINAPI IWGetTransactionCategories(
	CIWVerification* pIWVer,				// Verification pointer
	int nDataArraySize,						// Number of string pointers in array
	const TCHAR** rgszDataArray,			// Block of string pointers
	int* pnEntries							// Number of entries entered into array
);

//************************************************************/
//                                                            /
//							Verification					  /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWGetTransactionTypes(
	CIWVerification* pIWVer,				// Verification pointer
	int nDataArraySize,						// Number of string pointers in arrays
	const TCHAR** rgszDataArray,			// Block of string pointers to TOTs
	const TCHAR** rgszDescArray,			// Block of string pointers to TOT descriptions
	int* pnEntries,							// Number of entries entered into arrays
	const TCHAR *szCategory					// Transaction category input
);

OPENEBTS_API int WINAPI IWGetRecordTypeOccurrences(
	CIWVerification* pIWVer,				// Verification pointer
	int nDataArraySize,						// Number of ints in piRecordType & piMinOccurrences & piMaxOccurrences
	int* rgnRecordType,						// Block of record types
	int* rgnMinOccurences,					// Block of minimum occurrences of a record type
	int* rgnMaxOccurences,					// Block of maximum occurrences of a record type
	int* pnEntries,							// Number of entries returned
	const TCHAR *szTOT						// TOT input
);

OPENEBTS_API int WINAPI IWGetMnemonics(
	CIWVerification* pIWVer,				// Verification pointer
	const TCHAR* szTransactionType,			// Type of transaction
	int nDataArraySize,						// Number of string pointers in arrays
	const TCHAR** rgszDataArray,			// Block of string pointers to mnemonics
	const TCHAR** rgszDescArray,			// Block of string pointers to descriptions
	int* pnEntries							// Number of entries entered into arrays
);

OPENEBTS_API int WINAPI IWGetRuleRestrictions(
	CIWVerification* pIWVer,				// Verification pointer
	const TCHAR* szTransactionType,			// Type of transaction
	const TCHAR* szMnemonic,				// Field identifier
	int* pnRecordType,						// Record type
	int* pnField,							// Field number
	int* pnSubfield,						// Subfield number
	int* pnItem,							// Item number
	const TCHAR** pszDesc,					// Description string
	const TCHAR** pszLongDesc,				// Long description
	const TCHAR** pszCharType,				// Character types allowed
	const TCHAR** pszSpecialChars,			// List of additional characters allowed
	const TCHAR** pszDateFormat,			// e.g."CCYYMMDD", or "" when not a date
	const TCHAR** pszAdvancedRule,			// e.g."greater("T2_DOA")"
	int* pnSizeMin,							// Minimum field size
	int* pnSizeMax,							// Maximum field size
	int* pnOccMin,							// Minimum number of occurrences
	int* pnOccMax,							// Maximum number of occurrences
	int* pnOffset,							// Item offset from record/field start
	bool* pbAutomaticallySet,				// True if OpenEBTS handles this field
	bool* pbMandatory						// True if field is mandatory, false if optional
);

OPENEBTS_API int WINAPI IWGetValueList(
	CIWVerification* pIWVer,				// Verification Pointer
	const TCHAR* szTransactionType,			// Type of transaction
	const TCHAR* szMnemonic,				// Field identifier
	bool *pbMandatory,						// Field is mandatory
	int nDataArraySize,						// Number of string pointers in DataArray & DescArray
	const TCHAR** rgszDataArray,			// Block of string pointers
	const TCHAR** rgszDescArray,			// Block of string pointers to descriptions
	int *pnEntries							// Number of entries entered into DataArray
);

OPENEBTS_API int WINAPI IWReadVerification(
	const TCHAR* szPath,					// Path name to file
	CIWVerification** ppIWVer,				// Pointer to a verification pointer
	int nMaxParseError,						// Size of szParseError
	TCHAR* szParseError						// Potential errors encountered in verification file
);

OPENEBTS_API int WINAPI IWCloseVerification(
	CIWVerification **ppIWVer				// Pointer to verification pointer
);

OPENEBTS_API int WINAPI IWSetVerification(
	CIWTransaction *pIWTrans,				// Transaction pointer
	CIWVerification *pIWVer					// Verification pointer
);

OPENEBTS_API int WINAPI IWGet(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szMnemonic,				// Field identifier
	const TCHAR** pszData,					// Pointer to a string of data
	int nIndex,								// Index value
	int nRecordIndex						// Record index
);

OPENEBTS_API int WINAPI IWSet(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szMnemonic,				// Field identifier
	const TCHAR* szData,					// String of data
	int nStartIndex,						// Where to start inserting data
	int nRecordIndex						// Record index
);

OPENEBTS_API int WINAPI IWOccurrences(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szMnemonic,				// Field identifier
	int* pnOccurrences,						// Number of occurrences
	int nRecordIndex						// Record index
);

OPENEBTS_API int WINAPI IWVerify(
	CIWTransaction* pIWTrans				// Transaction pointer
);

//************************************************************/
//                                                            /
//						Data Access                           /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWNumSubfields(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	int nField,								// Field number
	int* pnCount							// The number of subfields/items present
);

OPENEBTS_API int WINAPI IWNumItems(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	int nField,								// Field number
	int nSubfield,							// Subfield number
	int* pnCount							// The number of subfields/items present
);

OPENEBTS_API int WINAPI IWFindItem(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	int nField,								// Field number
	int nSubfield,							// Subfield number
	int nItem,								// Item Number
	const TCHAR** pszData					// Pointer to a string of data
);

OPENEBTS_API int WINAPI IWSetItem(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szData,					// String of data
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	int nField,								// Field number
	int nSubfield,							// Subfield number
	int nItem								// Item number
);

OPENEBTS_API int WINAPI IWGetNextField(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,           				// Record type
	int nRecordIndex,						// Record index
	int nField,								// Field number
	int* pnNextField						// Next field number after 'Field'
); 

//************************************************************/
//                                                            /
//                        Images                              /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWGetImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHAR** pszStorageFormat,			// Three character image format
	long* pnLength,							// Data length
	const void** Data						// Pointer to a pointer of binary data
);

OPENEBTS_API int WINAPI IWSetImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHAR* szInputFormat,				// Three character image format
	long nLength,							// Data length
	void* pData,							// Pointer to binary data
	const TCHAR* szStorageFormat,			// Three character extension of stored format type
	float fCompression						// Image compression value
);

OPENEBTS_API int WINAPI IWImportImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHAR* szPath,					// Path name to file
	const TCHAR* szStorageFormat,			// Three character extension of stored format type
	float fCompression,						// Image compression value
	const TCHAR* szInputFormat				// Three character extension for input image format
);

OPENEBTS_API int WINAPI IWExportImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHAR* szPath,					// Path name to file
	const TCHAR* szOutputFormat				// Three character extension for input image format
);

OPENEBTS_API int WINAPI IWGetImageInfo(
	CIWTransaction* pIWTrans,				// Transaction pointer 
	int nRecordType,						// Record type 
	int nRecordIndex,						// Record index 
	const TCHAR** pszStorageFormat,			// Three character extension of stored format type 
	long* pnLength,							// Data length 
	long* pnHLL,							// Horizontal line length 
	long* pnVLL,							// Vertical line length 
	int* pnBitsPerPixel						// Number of bits per image pixel 
);

OPENEBTS_API int WINAPI IWDeleteRecord(
	CIWTransaction* pIWTrans,				// Transaction pointer 
	int nRecordType,						// Record type 
	int nRecordIndex						// Record index 
);

//************************************************************/
//                                                            /
//                   Image format conversion			      /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI RAWtoWSQ(BYTE* pImageIn, long lWidth, long lHeight, long lDPI, float fRate, BYTE** ppImageOut, long *plLengthOut);
OPENEBTS_API int WINAPI BMPtoWSQ(BYTE* pImageIn, long lLengthIn, float fRate, BYTE** ppImageOut, long *plLengthOut);

OPENEBTS_API int WINAPI WSQtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI);
OPENEBTS_API int WINAPI WSQtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);

OPENEBTS_API int WINAPI RAWtoBMP(int nWidth, int nHeight, int nDPI, int nDepth, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut);
OPENEBTS_API int WINAPI BMPtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI);

OPENEBTS_API int WINAPI JPGtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);
OPENEBTS_API int WINAPI BMPtoJPG(BYTE* pImageIn, long lLengthIn, long nCompression, BYTE **ppImageOut, long *plLengthOut);

OPENEBTS_API int WINAPI JP2toBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);
OPENEBTS_API int WINAPI BMPtoJP2(BYTE* pImageIn, long lLengthIn, float fRate, BYTE **ppImageOut, long *plLengthOut);

OPENEBTS_API int WINAPI RAWtoFX4(int nWidth, int nHeight, int nDPI, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut);
OPENEBTS_API int WINAPI FX4toRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI);

OPENEBTS_API int WINAPI BMPtoFX4(BYTE* pImageIn, long lLengthIn, BYTE** ppImageOut, long *plLengthOut);
OPENEBTS_API int WINAPI FX4toBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);

OPENEBTS_API int WINAPI MemFree(BYTE* pImage);

//************************************************************/
//                                                            /
//                   Error Reporting                          /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWGetErrorCount(
	CIWTransaction* pIWTrans					// Transaction pointer
);

OPENEBTS_API int WINAPI IWGetError(
	  CIWTransaction* pIWTrans,				// Transaction pointer
	  int Index,							// Zero based index value
	  int* Code,							// Error code
	  const TCHAR** pszDesc					// Error description string
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _OPENEBTS_H_					 
