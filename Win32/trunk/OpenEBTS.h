
#ifndef _IWNIST_H_
#define _IWNIST_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IWNIST_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IWNIST_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef IWNIST_EXPORTS
#define IWNIST_API __declspec(dllexport)
#else
#define IWNIST_API __declspec(dllimport)
#endif

#ifdef  __cplusplus
  class CIWTransaction;
  class CIWVerification;
#else
  typedef
   struct CIWTransaction
  CIWTransaction;
  typedef
   struct CIWVerification
  CIWVerification;
#endif

#define IW_MAX_PARSE_ERROR 1023

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif // __cplusplus

typedef struct _FINGERPRINT_HEADER_
{
	BYTE bFieldLen[4];
	BYTE bIDC;
	BYTE bImp; // finger impression type
	BYTE bFGP[6]; // finger position
	BYTE bISR; // scanning resolution
	BYTE bHLL[2]; // Horiz line length
	BYTE bVLL[2]; // Vert line length
	BYTE bComp; // compression algorithm used
} FINGERPRINT_HEADER;

typedef struct _SIGNATURE_HEADER_
{
	BYTE bFieldLen[4];
	BYTE bIDC;
	BYTE bSIG; // signature type
	BYTE bSRT; // signature representation type
	BYTE bISR; // scanning resolution
	BYTE bHLL[2]; // Horiz line length
	BYTE bVLL[2]; // Vert line length
} SIGNATURE_HEADER;

/*************************************************************/
/*                                                           */
/*               Transaction Management                      */
/*                                                           */
/*************************************************************/

IWNIST_API int WINAPI IWNew(
  const char* TransactionType,  /* Type of transaction */
  CIWVerification* IWVer,     /* Verification Pointer */
  CIWTransaction** IWTrans    /* Pointer to a Transaction 
                                 Pointer */
);

IWNIST_API int WINAPI IWRead(
  const char *pszPath,           /* Path name to file */
  CIWVerification *pIWVer,     /* Verification Pointer */
  CIWTransaction **ppIWTrans    /* Pointer to a Transaction 
                                 Pointer */
);

IWNIST_API int WINAPI IWReadMem(
	unsigned char *pBuffer, /* Pointer to Transaction Buffer */
	int BufferSize, /* Size of the transaction */
	CIWVerification *pIWVer, /* Verification Pointer */
	CIWTransaction **ppIWTrans /* Pointer to a Transaction Pointer */
);

IWNIST_API int WINAPI IWClose(
  CIWTransaction **ppIWTrans   /* Pointer to Transaction Pointer */
);

IWNIST_API int WINAPI IWAddRecord(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int* RecordIndex            /* Record Index */
);

IWNIST_API int WINAPI IWGetRecordTypeCount(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int* RecordTypeCount        /* Number of Records */
);

IWNIST_API int WINAPI IWWrite(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  const char* Path            /* Path name to file */
);

IWNIST_API int WINAPI IWWriteXML(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  const char* Path,           /* Path name to file */
  bool bValidate			  /* Validae produces XML against NIST schemas */
);

// following fns not required, but may be simple to implement
IWNIST_API int WINAPI IWGetNumRecords(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int* Records                /* The number of records */
);

IWNIST_API int WINAPI IWGetTransactionCategories(
	CIWVerification* IWVer, /* Verification Pointer */
	int DataArraySize, /* Number of string pointers	in DataArray */
	const char** DataArray, /* Block of string pointers */
	int* Entries /* Number of entries entered into DataArray */
);

IWNIST_API int WINAPI IWGetTransactionTypes(
	CIWVerification* IWVer, /* Verification Pointer */
	int DataArraySize, /* Number of string pointers in DataArray & DescArray */
	const char** DataArray, /* Block of string pointers */
	const char** DescArray, /* Block of string pointers to descriptions */
	int* Entries, /* Number of entries entered into DataArray */
	const char * Category /* Transaction Category */
);

IWNIST_API int WINAPI IWGetRecordTypeOccurences(
	CIWVerification* pIWVer, /* Verification Pointer */
	int DataArraySize, /* Number of ints in piRecordType & piMinOccurences & piMaxOccurences */
	int *piRecordType, /* Block of Record Types */
	int *piMinOccurences, /* Block of minimum occurences of a Record Type */
	int *piMaxOccurences, /* Block of maximum occurences of a Record Type */
	int *pEntries, /* Number of entries returned */
	const char *pTOT /* TOT input parameter */
);

IWNIST_API int WINAPI IWGetMnemonics(
	CIWVerification* pIWVer, /* Verification Pointer */
	const char* TransactionType, /* Type of transaction */
	int DataArraySize, /* Number of string pointers in DataArray & DescArray*/
	const char** DataArray, /* Block of string pointers */
	const char** DescArray, /* Block of string pointers to descriptions */
	int* pEntries /* Number of entries entered into DataArray */
);

IWNIST_API int WINAPI IWGetRuleRestrictions(
	CIWVerification* pIWVer, /* Verification Pointer */
	const char* TransactionType, /* Type of transaction */
	const char* Mnemonic, /* Field Identifier */
	int* RecordType, /* Record Type */
	int* Field, /* Field Number */
	int* Subfield, /* Subfield number */
	int* Item, /* Item Number */
	const char** Desc, /* Description string */
	const char** LongDesc, /* Long Description */
	const char** CharType, /* Character Types allowed */
	const char** DateFormat, /* e.g."CCYYMMDD", or "" when not a date */
	int* SizeMin, /* Minimum field size */
	int* SizeMax, /* Maximum field size */
	int* OccMin, /* Minimum number of occurrences */
	int* OccMax, /* Maximum number of occurrences */
	int* Offset, /* Item offset from record/field start*/
	bool* AutomaticallySet /* true if OpenEBTS handles this field */
);

IWNIST_API int WINAPI IWGetValueList(
	CIWVerification* pIWVer, /*Verification Pointer */
	const char* TransactionType, /*Type of transaction */
	const char* Mnemonic, /* Field Identifier */
	int *Mandatory, /* Field is Mandatory */
	int DataArraySize, /* Number of string pointers in DataArray & DescArray */
	const char** DataArray, /* Block of string pointers */
	const char** DescArray, /* Block of string pointers to descriptions */
	int *Entries /* Number of entries entered into DataArray */
);

/************************************************************/
/*                                                          */
/*                   Verification based                     */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWReadVerification(
	const char* Path, /* Path name to file */
	CIWVerification** IWVer, /* Pointer to a Verification Pointer */
	int MaxParseError, /* Size of ParseError */
	char* ParseError
);

IWNIST_API int WINAPI IWCloseVerification(
	CIWVerification **ppIWVer /* Pointer to Verification Pointer */
);

IWNIST_API int WINAPI IWSetVerification(
	CIWTransaction *pIWTrans, /* Transaction Pointer */
	CIWVerification *pIWVer /* Verification Pointer */
);

IWNIST_API int WINAPI IWGet(
	CIWTransaction* IWTrans, /* Transaction Pointer */
	const char* Mnemonic, /* Field Identifier */
	const char** Data, /* Pointer to a string of data */
	int Index, /* Index value */
	int RecordIndex /* Record Index */
);

IWNIST_API int WINAPI IWSet(
	CIWTransaction* IWTrans, /* Transaction Pointer */
	const char* Mnemonic, /* Field Identifier */
	const char* Data, /* String of data */
	int StartIndex, /* Where to start inserting data */
	int RecordIndex /* Record Index */
);

IWNIST_API int WINAPI IWOccurrences(
	CIWTransaction* IWTrans, /* Transaction Pointer */
	const char* Mnemonic, /* Field Identifier */
	int* Occurrences, /* Number of Occurrences */
	int RecordIndex /* Record Index */
);

IWNIST_API int WINAPI IWVerify(
	CIWTransaction* IWTrans     /* Transaction Pointer */ 
);

/************************************************************/
/*                                                          */
/*                   NIST Data Access                       */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWNumSubfields(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  int Field,                  /* Field Number */
  int* Count                  /* The number of 
                                 subfields/items present */
);

IWNIST_API int WINAPI IWNumItems(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  int Field,                  /* Field Number */
  int Subfield,               /* Subfield number */
  int* Count                  /* The number of subfields/items 
                                 present */
);

IWNIST_API int WINAPI IWFindItem(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  int Field,                  /* Field Number */
  int Subfield,               /* Subfield number */
  int Item,                   /* Item Number */
  const char** Data           /* Pointer to a string of data */
);

IWNIST_API int WINAPI IWSetItem(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  const char* Data,           /* String of data */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  int Field,                  /* Field Number */
  int Subfield,               /* Subfield number */
  int Item                    /* Item Number */
);

IWNIST_API int WINAPI IWGetNextField(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  int Field,                  /* Field Number */
  int* NextField              /* Next field number after 'Field' */
); 

/************************************************************/
/*                                                          */
/*                        Images                            */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWGetImage(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  const char** StorageFormat, /* Three character image format */
  long* Length,               /* Data Length */
  const void** Data           /* Pointer to a pointer of 
                                 binary data */
);

IWNIST_API int WINAPI IWSetImage(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  const char* InputFormat,    /* Three character image format */
  long Length,                /* Data Length */
  void* Data,                 /* Pointer to binary data */
  const char* StorageFormat,  /* Three character extension of
                                 stored format type */
  float Compression           /* Image compression value */
);

IWNIST_API int WINAPI IWImportImage(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  const char* Path,           /* Path name to file */
  const char* StorageFormat,  /* Three character extension of
                                 stored format type */
  float Compression,          /* Image compression value */
  const char* InputFormat     /* Three character extension 
                                 for input image format.*/
);

IWNIST_API int WINAPI IWExportImage(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int RecordType,             /* Record Type */
  int RecordIndex,            /* Record Index */
  const char* Path,           /* Path name to file */
  const char* OutputFormat    /* Three character extension 
                                 for input image format */
);

IWNIST_API int WINAPI IWGetImageInfo(
  CIWTransaction* IWTrans,    /* Transaction Pointer */ 
  int RecordType,             /* Record Type */ 
  int RecordIndex,            /* Record Index */ 
  const char** StorageFormat, /* Three character extension of stored format type */ 
  long* Length,               /* Data Length */ 
  long* hll,                  /* Horizontal line length */ 
  long* vll,                  /* Vertical line length */ 
  int* BitsPerPixel           /* Number of bits per image pixel */ 
);

IWNIST_API int WINAPI IWDeleteRecord(
  CIWTransaction* IWTrans,    /* Transaction Pointer */ 
  int RecordType,             /* Record Type */ 
  int RecordIndex             /* Record Index */ 
);

// Image conversion routines
IWNIST_API int WINAPI RAWtoWSQ(BYTE* pImageIn, long lWidth, long lHeight, long lDPI, float fRate, BYTE** ppImageOut, long *plLengthOut);
IWNIST_API int WINAPI BMPtoWSQ(BYTE* pImageIn, long lLengthIn, float fRate, BYTE** ppImageOut, long *plLengthOut);

IWNIST_API int WINAPI WSQtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI);
IWNIST_API int WINAPI WSQtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);

IWNIST_API int WINAPI RAWtoBMP(int nWidth, int nHeight, int nDPI, int nDepth, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut);
IWNIST_API int WINAPI BMPtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI);

IWNIST_API int WINAPI JPGtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);
IWNIST_API int WINAPI BMPtoJPG(BYTE* pImageIn, long lLengthIn, long nCompression, BYTE **ppImageOut, long *plLengthOut);

IWNIST_API int WINAPI JP2toBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);
IWNIST_API int WINAPI BMPtoJP2(BYTE* pImageIn, long lLengthIn, float fRate, BYTE **ppImageOut, long *plLengthOut);

IWNIST_API int WINAPI RAWtoFX4(int nWidth, int nHeight, int nDPI, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut);
IWNIST_API int WINAPI FX4toRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI);

IWNIST_API int WINAPI BMPtoFX4(BYTE* pImageIn, long lLengthIn, BYTE** ppImageOut, long *plLengthOut);
IWNIST_API int WINAPI FX4toBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut);

IWNIST_API int WINAPI MemFree(BYTE* pImage);

/************************************************************/
/*                                                          */
/*                   Error Reporting                        */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWGetErrorCount(
  CIWTransaction* IWTrans     /* Transaction Pointer */
);

IWNIST_API int WINAPI IWGetError(
  CIWTransaction* IWTrans,    /* Transaction Pointer */
  int Index,                  /* Index value  (zero based)*/
  int* Code,                  /* Error Code */
  const char** Desc           /* Description string */
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _IWNIST_H_					 
