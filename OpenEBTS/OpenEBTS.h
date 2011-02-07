
#ifndef _OPENEBTS_H_
#define _OPENEBTS_H_

#ifdef UNICODE
	typedef wchar_t		TCHAR;
#else
	typedef char		TCHAR;
#endif


#ifdef WIN32

#ifdef OPENEBTS_EXPORTS
#define OPENEBTS_API __declspec(dllexport)
#else
#define OPENEBTS_API __declspec(dllimport)
#endif

#ifndef WINAPI
#define WINAPI      __stdcall
#endif

// On Win32 paths are Unicode so we can refer to them with wide or single chars
#define TCHARPATH	TCHAR

#else

#define OPENEBTS_API
#define WINAPI
// On *nix, the filesystem is UTF-8, so we always use char*'s to pass filenames around
#define TCHARPATH	char

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

#ifndef BYTE
typedef unsigned char BYTE;
#endif

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
	const TCHARPATH *szPath,				// Path of transaction file
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

OPENEBTS_API int WINAPI IWMemFree(
   BYTE** ppBuffer							// Pointer to Transaction buffer pointer
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
	const TCHARPATH* szPath					// Path of transaction file to create
);

OPENEBTS_API int WINAPI IWWriteXML(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHARPATH* szPath,				// Path of transaction file to create
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
	const TCHARPATH* szPath,				// Path name to file
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

// IWRemove and IWRemoveItem are partially implemented.
// They will only remove the entire field at present, at some
// point we may want to remove subfields and subitems within
// fields.
OPENEBTS_API int WINAPI IWRemove(
	CIWTransaction* pIWTrans,				// Transaction pointer
	const TCHAR* szMnemonic,				// Field identifier
	int nIndex,							// Index value, pass 0 to remove entire field
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

OPENEBTS_API int WINAPI IWRemoveItem(
	CIWTransaction* pIWTrans,				// Transaction pointer
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
	int* pnLength,							// Data length
	const void** Data						// Pointer to a pointer of binary data
);

OPENEBTS_API int WINAPI IWSetImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHAR* szInputFormat,				// Three character image format
	int nLength,							// Data length
	void* pData,							// Pointer to binary data
	const TCHAR* szStorageFormat,			// Three character extension of stored format type
	float fCompression						// Image compression value
);

OPENEBTS_API int WINAPI IWImportImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHARPATH* szPath,				// Path name to file
	const TCHAR* szStorageFormat,			// Three character extension of stored format type
	float fCompression,						// Image compression value
	const TCHAR* szInputFormat				// Three character extension for input image format
);

OPENEBTS_API int WINAPI IWExportImage(
	CIWTransaction* pIWTrans,				// Transaction pointer
	int nRecordType,						// Record type
	int nRecordIndex,						// Record index
	const TCHARPATH* szPath,				// Path name to file
	const TCHAR* szOutputFormat				// Three character extension for input image format
);

OPENEBTS_API int WINAPI IWGetImageInfo(
	CIWTransaction* pIWTrans,				// Transaction pointer 
	int nRecordType,						// Record type 
	int nRecordIndex,						// Record index 
	const TCHAR** pszStorageFormat,			// Three character extension of stored format type 
	int* pnLength,							// Data length
	int* pnHLL,								// Horizontal line length
	int* pnVLL,								// Vertical line length
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

OPENEBTS_API int WINAPI BMPtoRAW(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut, int *pWidth, int *pHeight, int *pDPI);
OPENEBTS_API int WINAPI RAWtoBMP(int width, int height, int DPI, int depth, BYTE* pImageIn, BYTE** ppImageOut, int *pcbOut);

OPENEBTS_API int WINAPI BMPtoWSQ(BYTE* pImageIn, int cbIn, float fRate, BYTE** ppImageOut, int *pcbOut);
OPENEBTS_API int WINAPI WSQtoBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut);

OPENEBTS_API int WINAPI BMPtoJPG(BYTE* pImageIn, int cbIn, int nCompression, BYTE **ppImageOut, int *pcbOut);
OPENEBTS_API int WINAPI JPGtoBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut);

OPENEBTS_API int WINAPI BMPtoJP2(BYTE* pImageIn, int cbIn, float fRate, BYTE **ppImageOut, int *pcbOut);
OPENEBTS_API int WINAPI JP2toBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut);

OPENEBTS_API int WINAPI BMPtoFX4(BYTE* pImageIn, int cbIn, BYTE** ppImageOut, int *pcbOut);
OPENEBTS_API int WINAPI FX4toBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut);

OPENEBTS_API int WINAPI PNGtoBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut);
OPENEBTS_API int WINAPI BMPtoPNG(BYTE* pImageIn, int cbIn, BYTE** ppImageOut, int *pcbOut);

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
