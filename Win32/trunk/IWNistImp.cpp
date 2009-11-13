
#include "stdafx.h"
#include "OpenEBTS.h"
#include "TransactionDef.h"
#include "IWTransaction.h"
#include "IWVerification.h"
#include "OpenEBTSErrors.h"
#include "Common.h"


/************************************************************/
/*                                                          */
/*               Transaction Management                     */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWRead(const char *pszPath, 
					  CIWVerification *pIWVer, CIWTransaction **ppIWTrans)    
{
	SetLogFlags();

	if (!pszPath)
		return IW_ERR_READING_FILE;

	if (ppIWTrans == NULL)
		return IW_ERR_NULL_TRANSACTION_POINTER;

	int nErrCode = IW_ERR_READING_FILE;

	IWS_BEGIN_EXCEPTION_METHOD("IWRead")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWTransaction *pTrans = new CIWTransaction;

	if (pTrans)
	{
		if ((nErrCode = pTrans->ReadTransactionFile(pszPath)) == IW_SUCCESS)
		{
			nErrCode = pTrans->GetRecords();

			if (nErrCode == IW_SUCCESS || nErrCode == IW_WARN_INVALID_FIELD_NUM)
			{
				*ppIWTrans = pTrans;
				pTrans->SetVerification(pIWVer);
			}
			else
				delete pTrans;
		}
		else
			nErrCode = IW_ERR_READING_FILE;
	}
	else
		nErrCode = IW_ERR_OUT_OF_MEMORY;

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nErrCode;
}

IWNIST_API int WINAPI IWReadMem(unsigned char *pBuffer, int BufferSize, 
																	CIWVerification *pIWVer, CIWTransaction **ppIWTrans)
{
	SetLogFlags();

	if (!pBuffer)
		return IW_ERR_NULL_POINTER;

	if (ppIWTrans == NULL)
		return IW_ERR_NULL_TRANSACTION_POINTER;

	int nErrCode = IW_ERR_READING_FILE;

	IWS_BEGIN_EXCEPTION_METHOD("IWReadMem")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWTransaction *pTrans = new CIWTransaction;

	if (pTrans)
	{
		if ((nErrCode = pTrans->ReadTransactionFileMem((const unsigned char *)pBuffer,BufferSize)) == IW_SUCCESS)
		{
			nErrCode = pTrans->GetRecords();

			if (nErrCode == IW_SUCCESS || nErrCode == IW_WARN_INVALID_FIELD_NUM)
			{
				*ppIWTrans = pTrans;
				pTrans->SetVerification(pIWVer);
			}
			else
				delete pTrans;
		}
		else
			nErrCode = IW_ERR_READING_FILE;
	}
	else
		nErrCode = IW_ERR_OUT_OF_MEMORY;

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nErrCode;
}

IWNIST_API int WINAPI IWReadVerification(const char* pszPath, CIWVerification** ppIWVer, int MaxParseError, char* ParseError)
{
	SetLogFlags();

	if (!pszPath)
		return IW_ERR_OPENING_FILE_FOR_READING;

	if (ppIWVer == NULL)
		return IW_ERR_NULL_POINTER;

	int nErrCode = IW_ERR_OPENING_FILE_FOR_READING;

	IWS_BEGIN_EXCEPTION_METHOD("IWReadVerification")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWVerification *pVerification = new CIWVerification;

	if (pVerification)
	{
		if ((nErrCode = pVerification->ReadVerificationFile(pszPath, MaxParseError, ParseError)) == IW_SUCCESS)
		{
			if (nErrCode == IW_SUCCESS)
				*ppIWVer = pVerification;
			else
				delete pVerification;
		}
	}
	else
		nErrCode = IW_ERR_OUT_OF_MEMORY;

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nErrCode;
}

IWNIST_API int WINAPI IWSetVerification(CIWTransaction *pIWTrans, CIWVerification *pIWVer)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans != NULL)
		nRet = pIWTrans->SetVerification(pIWVer);

	return nRet;
}

IWNIST_API int WINAPI IWClose(CIWTransaction **ppIWTrans)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (ppIWTrans != NULL)
	{
		CIWTransaction *pIWTrans = *ppIWTrans;

		if (pIWTrans)
		{
			IWS_BEGIN_EXCEPTION_METHOD("IWClose")
			
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			delete pIWTrans;
			*ppIWTrans = 0;
			nRet = IW_SUCCESS;

			IWS_END_CATCHEXCEPTION_BLOCK()
		}
	}

	return nRet;
}

IWNIST_API int WINAPI IWCloseVerification(CIWVerification **ppIWVer)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (ppIWVer != NULL)
	{
		CIWVerification *pIWVer = *ppIWVer;
		if (pIWVer)
		{
			IWS_BEGIN_EXCEPTION_METHOD("IWCloseVerification")
			
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			delete pIWVer;
			*ppIWVer = 0;
			nRet = IW_SUCCESS;

			IWS_END_CATCHEXCEPTION_BLOCK()
		}
	}

	return nRet;
}

IWNIST_API int WINAPI IWGet(CIWTransaction* IWTrans, const char* Mnemonic, const char** Data, 
															int Index, int RecordIndex)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGet")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Get(Mnemonic, Data, Index, RecordIndex);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWSet(CIWTransaction* IWTrans, const char* Mnemonic, const char* Data, int StartIndex, int RecordIndex)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSet")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Set(Mnemonic, Data, StartIndex, RecordIndex);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWOccurrences(CIWTransaction* IWTrans, const char* Mnemonic, int* Occurrences, int RecordIndex)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWOccurrencesEx")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Occurrences(Mnemonic, Occurrences, RecordIndex);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWVerify(CIWTransaction* IWTrans)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWVerify")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Verify();
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWAddRecord(CIWTransaction* IWTrans, int RecordType, int* RecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	// Support all types supported by the FBI: "Only Type-1, Type-2, Type-4, Type-7, Type-9, 
	// Type-10, Type-13, Type-14, Type-15, Type-16 Type-17, and Type-99 records will be
	// accepted by the FBI", from EBTS Specification, April 1 2008, Appendix B.
	// (Type 1 gets added on creation because it always has to be there)
	// Also supports Type-8, by request from DH.
	if (RecordType == RECORD_TYPE2  || RecordType == RECORD_TYPE4  ||
		RecordType == RECORD_TYPE7  || RecordType == RECORD_TYPE8  ||
		RecordType == RECORD_TYPE9  || RecordType == RECORD_TYPE10 ||
		RecordType == RECORD_TYPE13 || RecordType == RECORD_TYPE14 ||
		RecordType == RECORD_TYPE15 || RecordType == RECORD_TYPE16 ||
		RecordType == RECORD_TYPE17 || RecordType == RECORD_TYPE99)
	{
		CIWTransaction *pIWTrans = IWTrans;

		if (pIWTrans)
		{
			IWS_BEGIN_EXCEPTION_METHOD("IWAddRecord")
			
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			nRet = pIWTrans->AddRecord(RecordType, RecordIndex);

			IWS_END_CATCHEXCEPTION_BLOCK()
		}
	}
	else
		nRet = IW_ERR_UNSUPPORTED_RECORD_TYPE;

	return nRet;
}

IWNIST_API int WINAPI IWGetRecordTypeCount(CIWTransaction* IWTrans, 
									int RecordType, int* RecordTypeCount)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetRecordTypeCount")
		
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		pIWTrans->GetRecordTypeCount(RecordType, RecordTypeCount);
		nRet = IW_SUCCESS;

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

IWNIST_API int WINAPI IWWrite(CIWTransaction* IWTrans, const char* Path)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWWrite")
		
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWTrans->Write(Path);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

IWNIST_API int WINAPI IWWriteXML(CIWTransaction* IWTrans, const char* Path, bool bValidate)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWWriteXML")
		
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWTrans->WriteXML(Path, bValidate);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

// following fns not required, but may be simple to implement
IWNIST_API int WINAPI IWGetNumRecords(CIWTransaction* IWTrans, int* Records)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetNumRecords")
		
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		pIWTrans->GetNumRecords(Records);
		nRet = IW_SUCCESS;

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

IWNIST_API int WINAPI IWNew(const char* TransactionType, 
					 CIWVerification* IWVer, CIWTransaction** IWTrans)
{
	SetLogFlags();

	if (IWTrans == NULL)
		return IW_ERR_NULL_TRANSACTION_POINTER;

	int nRet = IW_ERR_OUT_OF_MEMORY;

	IWS_BEGIN_EXCEPTION_METHOD("IWNew")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWTransaction *pTrans = new CIWTransaction;

	nRet = pTrans->New(TransactionType, IWVer);

	*IWTrans = pTrans;

	nRet = IW_SUCCESS;

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

/************************************************************/
/*                                                          */
/*                   NIST Data Access                       */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWNumSubfields(CIWTransaction* IWTrans, int RecordType, 
							  int RecordIndex, int Field, int* Count)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWNumSubfields")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNumSubfields(RecordType, RecordIndex, Field, Count);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWNumItems(CIWTransaction* IWTrans, int RecordType, 
						  int RecordIndex, int Field, int Subfield, int* Count)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWNumItems")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNumItems(RecordType, RecordIndex, Field, Subfield, Count);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWFindItem(CIWTransaction* IWTrans, int RecordType, int RecordIndex, 
						  int Field, int Subfield, int Item, const char** Data)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWFindItem")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->FindItem(RecordType, RecordIndex, Field, Subfield, Item, Data);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWSetItem(CIWTransaction* IWTrans, const char* Data, int RecordType, 
						 int RecordIndex, int Field, int Subfield, int Item)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSetItem")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->SetItem(Data, RecordType, RecordIndex, Field, Subfield, Item);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWGetNextField(CIWTransaction* IWTrans, int RecordType, 
							  int RecordIndex, int Field, int* NextField)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetNextField")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNextField(RecordType, RecordIndex, Field, NextField);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
} 

IWNIST_API int WINAPI IWGetTransactionCategories(CIWVerification *pIWVer, int DataArraySize, const char **ppDataArray, int *Entries)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetTransactionCategories")
		
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetTransactionCategories(DataArraySize, ppDataArray, Entries);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

IWNIST_API int WINAPI IWGetTransactionTypes(CIWVerification* pIWVer, int DataArraySize, const char **ppDataArray, 
																								const char **ppDescArray, int *pEntries, const char *pCategory)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetTransactionTypes")
		
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetTransactionTypes(DataArraySize, ppDataArray, ppDescArray, pEntries, pCategory);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

/************************************************************/
/*                                                          */
/*                        Images                            */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWGetImage(CIWTransaction* IWTrans, int RecordType, int RecordIndex, 
						  const char** StorageFormat, long* Length, const void** Data)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetImage")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetImage(RecordType, RecordIndex, StorageFormat, Length, Data);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

IWNIST_API int WINAPI IWSetImage(CIWTransaction* IWTrans, int RecordType, int RecordIndex, const char* InputFormat, 
						  long Length, void* Data, const char* StorageFormat, float Compression)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSetImage")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->SetImage(RecordType, RecordIndex, InputFormat, 
																Length, Data, StorageFormat, Compression);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

IWNIST_API int WINAPI IWImportImage(CIWTransaction* IWTrans, int RecordType, int RecordIndex, 
							const char* Path, const char* StorageFormat, float Compression, 
							const char* InputFormat)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWImportImage")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->ImportImage(RecordType, RecordIndex, Path, StorageFormat, Compression, InputFormat);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

IWNIST_API int WINAPI IWExportImage(CIWTransaction* IWTrans, int RecordType, int RecordIndex, 
							const char* Path, const char* OutputFormat)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWExportImage")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->ExportImage(RecordType, RecordIndex, Path, OutputFormat);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

IWNIST_API int WINAPI IWGetImageInfo(CIWTransaction* IWTrans, int RecordType, int RecordIndex, const char** StorageFormat, 
									 long* Length, long* hll, long* vll, int* BitsPerPixel)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetImageInfo")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetImageInfo(RecordType, RecordIndex, StorageFormat, 
									  Length, hll, vll, BitsPerPixel);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWDeleteRecord(CIWTransaction* IWTrans, int RecordType, int RecordIndex)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWDeleteRecord")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->DeleteRecord(RecordType, RecordIndex);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI RAWtoWSQ(BYTE* pImageIn, long lWidth, long lHeight, long lDPI, float fRate, BYTE** ppImageOut, long *plLengthOut)
// Input assumed to be 8 bpp
{
	int ret = IW_ERR_WSQ_COMPRESS;
	HGLOBAL hWSQ;
	BYTE *pWSQ;

	IWS_BEGIN_EXCEPTION_METHOD("RAWtoWSQ")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (NISTRAWtoWSQ((char*)pImageIn, lWidth, lHeight, lDPI, &hWSQ, fRate))
	{
		pWSQ = (BYTE*)GlobalLock(hWSQ);
		if (pWSQ != NULL)
		{
			*plLengthOut = GlobalSize(hWSQ);
			*ppImageOut = new BYTE[*plLengthOut];
			memcpy((void*)*ppImageOut, pWSQ, *plLengthOut);
			ret = IW_SUCCESS;
		}

		GlobalUnlock(hWSQ);
		GlobalFree(hWSQ);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return ret;
}

IWNIST_API int WINAPI BMPtoWSQ(BYTE* pImageIn, long lLengthIn, float fRate, BYTE** ppImageOut, long *plLengthOut)
// Input assumed to be 8 bpp (NISTBMPtoWSQ will return 0 otherwise and IW_ERR_WSQ_COMPRESS gets returned here)
{
	int ret = IW_ERR_WSQ_COMPRESS;
	HGLOBAL hWSQ;
	BYTE *pWSQ;

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoWSQ")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (NISTBMPtoWSQ((char*)pImageIn, lLengthIn, &hWSQ, fRate))
	{
		pWSQ = (BYTE*)GlobalLock(hWSQ);
		if (pWSQ != NULL)
		{
			*plLengthOut = GlobalSize(hWSQ);
			*ppImageOut = new BYTE[*plLengthOut];
			memcpy((void*)*ppImageOut, pWSQ, *plLengthOut);
			ret = IW_SUCCESS;
		}

		GlobalUnlock(hWSQ);
		GlobalFree(hWSQ);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return ret;
}

IWNIST_API int WINAPI WSQtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI)
{
	int ret = IW_ERR_WSQ_DECOMPRESS;
	HGLOBAL hRAW;
	BYTE *pRAW;

	IWS_BEGIN_EXCEPTION_METHOD("WSQtoRAW")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (NISTWSQtoRAW((char*)pImageIn, lLengthIn, &hRAW, plWidth, plHeight, plDPI))
	{
		pRAW = (BYTE*)GlobalLock(hRAW);
		if (pRAW != NULL)
		{
			*plLengthOut = GlobalSize(hRAW);
			*ppImageOut = new BYTE[*plLengthOut];
			memcpy((void*)*ppImageOut, pRAW, *plLengthOut);
			ret = IW_SUCCESS;
		}

		GlobalUnlock(hRAW);
		GlobalFree(hRAW);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return ret;
}

IWNIST_API int WINAPI WSQtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut)
{
	int ret = IW_ERR_WSQ_DECOMPRESS;
	HGLOBAL hBMP;
	BYTE *pBMP;

	IWS_BEGIN_EXCEPTION_METHOD("WSQtoBMP")

	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (NISTWSQtoBMP((char*)pImageIn, lLengthIn, &hBMP))
	{
		pBMP = (BYTE*)GlobalLock(hBMP);
		if (pBMP != NULL)
		{
			*plLengthOut = GlobalSize(hBMP);
			*ppImageOut = new BYTE[*plLengthOut];
			memcpy((void*)*ppImageOut, pBMP, *plLengthOut);
			ret = IW_SUCCESS;
		}

		GlobalUnlock(hBMP);
		GlobalFree(hBMP);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return ret;
}

IWNIST_API int WINAPI RAWtoBMP(int nWidth, int nHeight, int nDPI, int nDepth, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut)
// Supports 1bpp, 8bpp and 24bpp
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	long	lBytesPerRowBMP;
	long	lBytesPerRowRAW;
	long	nBMPDataLen;

	if (nDepth != 1 && nDepth != 8 && nDepth != 24) goto done;

	if (pImageIn)
	{
		if (nDepth == 1)
		{
			lBytesPerRowRAW = (nWidth+7)/8;
			lBytesPerRowBMP = (lBytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
		}
		else
		{
			lBytesPerRowRAW = nWidth * nDepth/8;
			lBytesPerRowBMP = (lBytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
		}
		nBMPDataLen = lBytesPerRowBMP * nHeight;

		// Define and initialize default color table, be it grayscale or two-tone
		DWORD color_count = 0;
		LPRGBQUAD color_table = NULL;
		DWORD table_size = 0;

		if (nDepth != 24)
		{
			color_count = 1 << nDepth;
			color_table = new RGBQUAD[color_count];
			table_size = color_count * sizeof(RGBQUAD);

			if (nDepth == 1)
			{
				RGBQUAD rgbBlack = { 0, 0, 0, 0 };
				RGBQUAD rgbWhite = { 0xff, 0xff, 0xff, 0 };

				memcpy(&color_table[0], &rgbBlack, sizeof(RGBQUAD));
				memcpy(&color_table[1], &rgbWhite, sizeof(RGBQUAD));
			}
			else
			{
				for (int i = 0; i < (int) color_count; i++)
				{
					RGBQUAD rgbTemp = { i, i, i, 0 };
					memcpy(&color_table[i], &rgbTemp, sizeof(RGBQUAD));
				}
			}
		}

		// Calc the size of the entire file
		DWORD dwBMPSize = (DWORD)(sizeof(BITMAPINFOHEADER) + table_size + nBMPDataLen);
		DWORD dwFileSize = (DWORD)(sizeof(BITMAPFILEHEADER) + dwBMPSize);

		*ppImageOut = new BYTE[dwFileSize];
		LPBYTE lpData = (LPBYTE) *ppImageOut;
		memset(lpData, NULL, dwFileSize);

		LPBITMAPFILEHEADER lpbmfh = (LPBITMAPFILEHEADER) lpData;
		lpbmfh->bfType = 0x4D42;
		lpbmfh->bfSize = dwFileSize;
		lpbmfh->bfReserved1 = lpbmfh->bfReserved2 = 0;
		lpbmfh->bfOffBits = dwFileSize - nBMPDataLen;

		LPBITMAPINFO lpbi = (LPBITMAPINFO)(lpData + sizeof(BITMAPFILEHEADER));

		lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
		lpbi->bmiHeader.biWidth = nWidth; 
		lpbi->bmiHeader.biHeight = nHeight; // switch sign on biHeight convert to top-down
		lpbi->bmiHeader.biPlanes = 1;
		lpbi->bmiHeader.biBitCount = nDepth;
		lpbi->bmiHeader.biCompression = BI_RGB;
        lpbi->bmiHeader.biXPelsPerMeter = (long)(nDPI * 10000.0 / 254.0 + 0.5);
        lpbi->bmiHeader.biYPelsPerMeter = lpbi->bmiHeader.biXPelsPerMeter;
		lpbi->bmiHeader.biSizeImage = nBMPDataLen;

		// no color table for 24-bit, otherwise use default
		if (lpbi->bmiHeader.biBitCount != 24) 
			lpbi->bmiHeader.biClrUsed = (DWORD)(1 << lpbi->bmiHeader.biBitCount); 

		// write the color table
		memcpy(lpbi->bmiColors, color_table, table_size);

		// write the pixel data
		if (nDepth != 24)
		{
			for(int i = 0; i < nHeight; i++)
			{
				memcpy(lpData + lpbmfh->bfOffBits + (i * lBytesPerRowBMP), 
					   pImageIn + (nHeight - i - 1) * lBytesPerRowRAW,
					   lBytesPerRowRAW);
			}
		}
		else
		{
			// BGR not RGB!
			BYTE *ptrSrc = pImageIn + (nHeight-1)*lBytesPerRowRAW;
			BYTE *ptrDst = lpData + lpbmfh->bfOffBits;
			for(int i = 0; i < nHeight; i++)
			{
				for(int j = 0; j < nWidth; j++)
				{
					memcpy(ptrDst + j*3 + 0, ptrSrc + j*3 + 2, 1);
					memcpy(ptrDst + j*3 + 1, ptrSrc + j*3 + 1, 1);
					memcpy(ptrDst + j*3 + 2, ptrSrc + j*3 + 0, 1);
				}
				ptrSrc -= lBytesPerRowRAW;
				ptrDst += lBytesPerRowBMP;
			}
		}

		*plLengthOut = dwFileSize;

		if (color_table) delete color_table;

		ret = IW_SUCCESS;
	}

done:
	return ret;
}

IWNIST_API int WINAPI BMPtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI)
//
// Supports 1bpp, 8bpp and 24bpp BMP inputs.
//
{
	int					ret = IW_ERR_IMAGE_CONVERSION;
	BITMAPFILEHEADER	*pbfh;
	BITMAPINFOHEADER	*pbih;
	long				lBytesPerRowBMP;
	long				lBytesPerRowRAW;
	long				i;
	long				lLenPalette;
	BYTE				*pBMPPtr;
	BYTE				*pRawPtr;

	if (!pImageIn) goto done;

	// Support both file BMPs and DIBs, why not
	pbfh = (BITMAPFILEHEADER*)pImageIn;
	if (pbfh->bfType == 0x4d42) { //"BM"
		pbih = (BITMAPINFOHEADER*)(pImageIn + sizeof(BITMAPFILEHEADER));
	} else {
		pbih = (BITMAPINFOHEADER*)pImageIn;
	}

	if (pbih->biCompression != BI_RGB) goto done;	// only uncompressed supported
	if (pbih->biBitCount != 1 && pbih->biBitCount != 8 && pbih->biBitCount != 24) goto done;

	// Set pBMPPtr to where image bits start.
	if (pbih->biBitCount != 24)
		lLenPalette = (pbih->biClrUsed == 0 ? 2^pbih->biBitCount :  pbih->biClrUsed) * sizeof(RGBQUAD);
	else
		lLenPalette = 0;
	pBMPPtr = (BYTE*)pbih + sizeof(BITMAPINFOHEADER) + lLenPalette;
	if (pbih->biBitCount == 1)
	{
		lBytesPerRowRAW = (pbih->biWidth+7)/8;
		lBytesPerRowBMP = (lBytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
	}
	else
	{
		lBytesPerRowRAW = pbih->biWidth * pbih->biBitCount/8;
		lBytesPerRowBMP = (lBytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
	}
	// Since the BMP is upside down, move the pointer to the last row
	pBMPPtr += lBytesPerRowBMP*(pbih->biHeight-1);

	*plLengthOut = lBytesPerRowRAW * pbih->biHeight;
	*ppImageOut = new BYTE[*plLengthOut];

	pRawPtr = (BYTE*)*ppImageOut;

	// We transfer each row into the new array, with 4-byte alignment
	// and with the rows reversed
	for (i=0; i<pbih->biHeight; i++) {
		memcpy(pRawPtr, pBMPPtr, lBytesPerRowRAW);
		pRawPtr += lBytesPerRowRAW;		// move down one raw row
		pBMPPtr -= lBytesPerRowBMP;		// move up one BMP row
	}

	*plWidth = pbih->biWidth;
	*plHeight = pbih->biHeight;
	*plDPI = (long)(pbih->biXPelsPerMeter * 254 / 10000.0 + 0.5);

	ret = IW_SUCCESS;

done:
	return ret;
}

IWNIST_API int WINAPI JPGtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut)
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	HGLOBAL	hIn = NULL;
	HGLOBAL	hOut = NULL;
	void	*pIn = NULL;
	void	*pOut = NULL;

	IWS_BEGIN_EXCEPTION_METHOD("JPGtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	hIn = ::GlobalAlloc(GMEM_MOVEABLE, lLengthIn);
	if (hIn == NULL) goto done;
	pIn = GlobalLock(hIn);
	if (pIn == NULL) goto done;
	memcpy(pIn, pImageIn, lLengthIn);
	GlobalUnlock(hIn);
	pIn = NULL;
	if (JPGMemory_to_BMPMemory(hIn, &hOut) != 1) goto done;
	pOut = GlobalLock(hOut);
	if (pOut == NULL) goto done;

	*plLengthOut = ::GlobalSize(hOut);
	*ppImageOut = new BYTE[*plLengthOut];
	memcpy(*ppImageOut, pOut, *plLengthOut);

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

done:
	if (pIn != NULL) GlobalUnlock(hIn);
	if (pOut != NULL) GlobalUnlock(hOut);
	if (hIn != NULL) GlobalFree(hIn);
	if (hOut != NULL) GlobalFree(hOut);

	return ret;
}

IWNIST_API int WINAPI BMPtoJPG(BYTE* pImageIn, long lLengthIn, long nCompression, BYTE **ppImageOut, long *plLengthOut)
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	HGLOBAL	hIn = NULL;
	HGLOBAL	hOut = NULL;
	void	*pIn = NULL;
	void	*pOut = NULL;

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoJPG")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	hIn = ::GlobalAlloc(GMEM_MOVEABLE, lLengthIn);
	if (hIn == NULL) goto done;
	pIn = GlobalLock(hIn);
	if (pIn == NULL) goto done;
	memcpy(pIn, pImageIn, lLengthIn);
	GlobalUnlock(hIn);
	pIn = NULL;
	if (BMPMemory_to_JPGMemory(hIn, &hOut, nCompression) != 1) goto done;
	pOut = GlobalLock(hOut);
	if (pOut == NULL) goto done;

	*plLengthOut = ::GlobalSize(hOut);
	*ppImageOut = new BYTE[*plLengthOut];
	memcpy(*ppImageOut, pOut, *plLengthOut);

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

done:
	if (pIn != NULL) GlobalUnlock(hIn);
	if (pOut != NULL) GlobalUnlock(hOut);
	if (hIn != NULL) GlobalFree(hIn);
	if (hOut != NULL) GlobalFree(hOut);

	return ret;
}

int WINAPI JP2toBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut)
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	HGLOBAL	hOut = NULL;
	void	*pOut = NULL;

	IWS_BEGIN_EXCEPTION_METHOD("JPGtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (!JasperJP2toBMP((char*)pImageIn, lLengthIn, &hOut)) goto done;

	pOut = GlobalLock(hOut);
	if (pOut == NULL) goto done;

	*plLengthOut = ::GlobalSize(hOut);
	*ppImageOut = new BYTE[*plLengthOut];
	memcpy(*ppImageOut, pOut, *plLengthOut);

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

done:
	if (pOut != NULL) GlobalUnlock(hOut);
	if (hOut != NULL) GlobalFree(hOut);

	return ret;
}

int WINAPI BMPtoJP2(BYTE* pImageIn, long lLengthIn, float fRate, BYTE **ppImageOut, long *plLengthOut)
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	HGLOBAL	hOut = NULL;
	void	*pOut = NULL;

	IWS_BEGIN_EXCEPTION_METHOD("JPGtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (!JasperBMPtoJP2((char*)pImageIn, lLengthIn, &hOut, fRate)) goto done;

	pOut = GlobalLock(hOut);
	if (pOut == NULL) goto done;

	*plLengthOut = ::GlobalSize(hOut);
	*ppImageOut = new BYTE[*plLengthOut];
	memcpy(*ppImageOut, pOut, *plLengthOut);

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

done:
	if (pOut != NULL) GlobalUnlock(hOut);
	if (hOut != NULL) GlobalFree(hOut);

	return ret;
}

int WINAPI RAWtoFX4(int nWidth, int nHeight, int nDPI, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut)
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	HGLOBAL	hOut = NULL;
	void	*pOut = NULL;

	IWS_BEGIN_EXCEPTION_METHOD("RAWtoFX4")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (!LIBTIFFRAWtoGroup4Fax((char*)pImageIn, nWidth, nHeight, nDPI, &hOut)) goto done;

	pOut = GlobalLock(hOut);
	if (pOut == NULL) goto done;

	*plLengthOut = ::GlobalSize(hOut);
	*ppImageOut = new BYTE[*plLengthOut];
	memcpy(*ppImageOut, pOut, *plLengthOut);

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

done:
	if (pOut != NULL) GlobalUnlock(hOut);
	if (hOut != NULL) GlobalFree(hOut);

	return ret;
}

int WINAPI BMPtoFX4(BYTE* pImageIn, long lLengthIn, BYTE** ppImageOut, long *plLengthOut)
//
// 2-step process, via RAW
//
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	BYTE	*pImageTemp = NULL;
	long	lLengthTemp;
	long	lWidth;
	long	lHeight;
	long	lDPI;

	ret = BMPtoRAW(pImageIn, lLengthIn, &pImageTemp, &lLengthTemp, &lWidth, &lHeight, &lDPI);
	if (ret != IW_SUCCESS) goto done;

	ret = RAWtoFX4(lWidth, lHeight, lDPI, pImageTemp, ppImageOut, plLengthOut);

done:
	if (pImageTemp != NULL) delete [] pImageTemp;

	return ret;
}

int WINAPI FX4toRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI)
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	HGLOBAL	hOut = NULL;
	void	*pOut = NULL;

	IWS_BEGIN_EXCEPTION_METHOD("FX4toRAW")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (!LIBTIFFGroup4FaxtoRAW((char*)pImageIn, lLengthIn, &hOut, plWidth, plHeight, plDPI)) goto done;

	pOut = GlobalLock(hOut);
	if (pOut == NULL) goto done;

	*plLengthOut = ::GlobalSize(hOut);
	*ppImageOut = new BYTE[*plLengthOut];
	memcpy(*ppImageOut, pOut, *plLengthOut);

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

done:
	if (pOut != NULL) GlobalUnlock(hOut);
	if (hOut != NULL) GlobalFree(hOut);

	return ret;
}

int WINAPI FX4toBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut)
{
	int		ret;
	BYTE	*pRAW;
	long	lLengthRAW;
	long	lWidthRAW;
	long	lHeightRAW;
	long	lDPIRAW;

	ret = FX4toRAW(pImageIn, lLengthIn, &pRAW, &lLengthRAW, &lWidthRAW, &lHeightRAW, &lDPIRAW);
	if (ret == IW_SUCCESS)
	{
		ret = RAWtoBMP(lWidthRAW, lHeightRAW, lDPIRAW, 1, pRAW, ppImageOut, plLengthOut);
		MemFree(pRAW);	// free pRAW no matter what the result of RAWtoBMP
	}

	return ret;
}

int WINAPI MemFree(BYTE* pImage)
{
	int	ret = IW_ERR_NULL_POINTER;

	try
	{
		if (pImage)
		{
			delete [] pImage;
			ret = IW_SUCCESS;
		}
	}
	catch(...)
	{
	}

	return ret;
}

/************************************************************/
/*                                                          */
/*                   Error Reporting                        */
/*                                                          */
/************************************************************/

IWNIST_API int WINAPI IWGetErrorCount(CIWTransaction* IWTrans)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = 0;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetErrorCount")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetErrorCount();

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

IWNIST_API int WINAPI IWGetError(CIWTransaction* IWTrans, int Index, int* Code, const char** Desc)
{
	CIWTransaction *pIWTrans = IWTrans;
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetError")
	
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetError(Index, Code, Desc);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}
