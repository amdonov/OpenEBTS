
#include "stdafx.h"
#include "OpenEBTS.h"
#include "TransactionDef.h"
#include "IWTransaction.h"
#include "IWVerification.h"
#include "OpenEBTSErrors.h"
#include "Common.h"

//************************************************************/
//                                                            /
//                  Transaction Management                    /
//                                                            /
//************************************************************//

OPENEBTS_API int WINAPI IWRead(const TCHAR *szPath, CIWVerification *pIWVer, CIWTransaction **ppIWTrans)
{
	SetLogFlags();

	if (!szPath)
		return IW_ERR_READING_FILE;

	if (ppIWTrans == NULL)
		return IW_ERR_NULL_TRANSACTION_POINTER;

	int nErrCode = IW_ERR_READING_FILE;

	IWS_BEGIN_EXCEPTION_METHOD("IWRead")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWTransaction *pTrans = new CIWTransaction;

	if (pTrans)
	{
		if ((nErrCode = pTrans->ReadTransactionFile(CStdString(szPath))) == IW_SUCCESS)
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

OPENEBTS_API int WINAPI IWReadMem(BYTE *pBuffer, int nBufferSize, CIWVerification *pIWVer, CIWTransaction **ppIWTrans)
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
		if ((nErrCode = pTrans->ReadTransactionFileMem((const BYTE*)pBuffer, nBufferSize)) == IW_SUCCESS)
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

OPENEBTS_API int WINAPI IWWriteMem(CIWTransaction* pIWTrans, BYTE** ppBuffer, int *pSize)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWWriteMem")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWTrans->WriteMem(ppBuffer, pSize);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWReadVerification(const TCHAR* szPath, CIWVerification** ppIWVer, int nMaxParseError, TCHAR* szParseError)
{
	SetLogFlags();

	if (!szPath)
		return IW_ERR_OPENING_FILE_FOR_READING;

	if (ppIWVer == NULL)
		return IW_ERR_NULL_POINTER;

	int nErrCode = IW_ERR_OPENING_FILE_FOR_READING;

	IWS_BEGIN_EXCEPTION_METHOD("IWReadVerification")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWVerification *pVerification = new CIWVerification;

	if (nMaxParseError > 0)
	{
		szParseError[0] = '\0';
	}

	if (pVerification)
	{
		CStdString sParseError;

		nErrCode = pVerification->ReadVerificationFile(CStdString(szPath), sParseError);
		if (nErrCode == IW_SUCCESS)
		{
			*ppIWVer = pVerification;
		}
		else
		{
			delete pVerification;
			if (!sParseError.IsEmpty())
			{
				_tcsncpy(szParseError, sParseError, nMaxParseError-1);
				szParseError[nMaxParseError-1] = '\0';
			}
		}
	}
	else
	{
		nErrCode = IW_ERR_OUT_OF_MEMORY;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nErrCode;
}

OPENEBTS_API int WINAPI IWSetVerification(CIWTransaction *pIWTrans, CIWVerification *pIWVer)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans != NULL)
		nRet = pIWTrans->SetVerification(pIWVer);

	return nRet;
}

OPENEBTS_API int WINAPI IWMemFree(BYTE** ppBuffer)
{
	int nRet = IW_ERR_NULL_POINTER;

	if (ppBuffer != NULL)
	{
		nRet = MemFree(*ppBuffer);
	}
	return nRet;
}


OPENEBTS_API int WINAPI IWClose(CIWTransaction **ppIWTrans)
{
	int nRet = IW_ERR_NULL_POINTER;

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

OPENEBTS_API int WINAPI IWCloseVerification(CIWVerification **ppIWVer)
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

OPENEBTS_API int WINAPI IWGet(CIWTransaction* pIWTrans, const TCHAR* szMnemonic, const TCHAR** pszData, 
							  int nIndex, int nRecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGet")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		CStdString sData;
		nRet = pIWTrans->Get(CStdString(szMnemonic), sData, nIndex, nRecordIndex);
		*pszData = pIWTrans->CreateNewStringSlot(sData);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWSet(CIWTransaction* pIWTrans, const TCHAR* szMnemonic, const TCHAR* szData,
							  int nStartIndex, int nRecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSet")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Set(CStdString(szMnemonic), CStdString(szData), nStartIndex, nRecordIndex);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWRemove(CIWTransaction* pIWTrans, const TCHAR* szMnemonic, 
																		int nIndex, int nRecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWRemove")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Remove(CStdString(szMnemonic), nIndex, nRecordIndex);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWOccurrences(CIWTransaction* pIWTrans, const TCHAR* szMnemonic, int* pnOccurrences, int nRecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWOccurrencesEx")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		nRet = pIWTrans->Occurrences(szMnemonic, pnOccurrences, nRecordIndex);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWVerify(CIWTransaction* pIWTrans)
{
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

OPENEBTS_API int WINAPI IWAddRecord(CIWTransaction* pIWTrans, int nRecordType, int* pnRecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	// Support all types supported by the FBI: "Only Type-1, Type-2, Type-4, Type-7, Type-9, 
	// Type-10, Type-13, Type-14, Type-15, Type-16 Type-17, and Type-99 records will be
	// accepted by the FBI", from EBTS Specification, April 1 2008, Appendix B.
	// (Type 1 gets added on creation because it always has to be there)
	// Also supports Type-8, by request from DH.
	if (nRecordType == RECORD_TYPE2  || nRecordType == RECORD_TYPE4  ||
		nRecordType == RECORD_TYPE7  || nRecordType == RECORD_TYPE8  ||
		nRecordType == RECORD_TYPE9  || nRecordType == RECORD_TYPE10 ||
		nRecordType == RECORD_TYPE13 || nRecordType == RECORD_TYPE14 ||
		nRecordType == RECORD_TYPE15 || nRecordType == RECORD_TYPE16 ||
		nRecordType == RECORD_TYPE17 || nRecordType == RECORD_TYPE99)
	{
		if (pIWTrans)
		{
			IWS_BEGIN_EXCEPTION_METHOD("IWAddRecord")
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			nRet = pIWTrans->AddRecord(nRecordType, pnRecordIndex);

			IWS_END_CATCHEXCEPTION_BLOCK()
		}
	}
	else
		nRet = IW_ERR_UNSUPPORTED_RECORD_TYPE;

	return nRet;
}

OPENEBTS_API int WINAPI IWGetRecordTypeCount(CIWTransaction* pIWTrans, int nRecordType, int* pnRecordTypeCount)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetRecordTypeCount")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		pIWTrans->GetRecordTypeCount(nRecordType, pnRecordTypeCount);
		nRet = IW_SUCCESS;

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWWrite(CIWTransaction* pIWTrans, const TCHAR* szPath)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWWrite")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWTrans->Write(szPath);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWWriteXML(CIWTransaction* pIWTrans, const TCHAR* szPath, bool bValidate)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWWriteXML")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWTrans->WriteXML(szPath, bValidate);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

// following fns not required, but may be simple to implement
OPENEBTS_API int WINAPI IWGetNumRecords(CIWTransaction* pIWTrans, int* pnRecords)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetNumRecords")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		pIWTrans->GetNumRecords(pnRecords);
		nRet = IW_SUCCESS;

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWNew(const TCHAR* szTransactionType, CIWVerification* pIWVer, CIWTransaction** ppIWTrans)
{
	SetLogFlags();

	if (ppIWTrans == NULL)
		return IW_ERR_NULL_TRANSACTION_POINTER;

	int nRet = IW_ERR_OUT_OF_MEMORY;

	IWS_BEGIN_EXCEPTION_METHOD("IWNew")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CIWTransaction *pIWTrans = new CIWTransaction;

	nRet = pIWTrans->New(szTransactionType, pIWVer);

	*ppIWTrans = pIWTrans;

	nRet = IW_SUCCESS;

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

//************************************************************/
//                                                            /
//						Data Access                           /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWNumSubfields(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, int nField, int* pnCount)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWNumSubfields")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNumSubfields(nRecordType, nRecordIndex, nField, pnCount);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWNumItems(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex,
								   int nField, int nSubfield, int* pnCount)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWNumItems")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNumItems(nRecordType, nRecordIndex, nField, nSubfield, pnCount);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWFindItem(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex,
								   int nField, int nSubfield, int nItem, const TCHAR** pszData)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWFindItem")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		CStdString sData;
		nRet = pIWTrans->FindItem(nRecordType, nRecordIndex, nField, nSubfield, nItem, sData);
		*pszData = pIWTrans->CreateNewStringSlot(sData);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWSetItem(CIWTransaction* pIWTrans, const TCHAR* szData, int nRecordType, 
								  int nRecordIndex, int nField, int nSubfield, int nItem)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSetItem")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->SetItem(CStdString(szData), nRecordType, nRecordIndex, nField, nSubfield, nItem);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWRemoveItem(CIWTransaction* pIWTrans, int nRecordType, 
								  int nRecordIndex, int nField, int nSubfield, int nItem)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWRemoveItem")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->RemoveItem(nRecordType, nRecordIndex, nField, nSubfield, nItem);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWGetNextField(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, int nField, int* pnNextField)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetNextField")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNextField(nRecordType, nRecordIndex, nField, pnNextField);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
} 

OPENEBTS_API int WINAPI IWGetTransactionCategories(CIWVerification *pIWVer, int nDataArraySize, const TCHAR** rgszDataArray, int *pnEntries)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetTransactionCategories")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetTransactionCategories(nDataArraySize, rgszDataArray, pnEntries);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWGetTransactionTypes(CIWVerification* pIWVer, int nDataArraySize, const TCHAR** rgszDataArray, 
											  const TCHAR** rgszDescArray, int *pnEntries, const TCHAR *szCategory)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetTransactionTypes")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetTransactionTypes(nDataArraySize, rgszDataArray, rgszDescArray, pnEntries, szCategory);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWGetRecordTypeOccurrences(CIWVerification* pIWVer, int nDataArraySize, int *pnRecordType,
												   int *pnMinOccurrences, int *pnMaxOccurrences, int *pnEntries, const TCHAR *szTOT)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetRecordTypeOccurrences")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetRecordTypeOccurrences(nDataArraySize, pnRecordType, pnMinOccurrences, pnMaxOccurrences, pnEntries, szTOT);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}


OPENEBTS_API int WINAPI IWGetMnemonics(CIWVerification* pIWVer, const TCHAR* szTransactionType, int nDataArraySize,
									   const TCHAR** rgszDataArray, const TCHAR** rgszDescArray, int* pnEntries)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetMnemonics")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetMnemonics(szTransactionType, nDataArraySize, rgszDataArray, rgszDescArray, pnEntries);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWGetRuleRestrictions(CIWVerification* pIWVer, const TCHAR* szTransactionType, const TCHAR* szMnemonic,
											  int* pnRecordType, int* pnField, int* pnSubfield, int* pnItem, const TCHAR** pszDesc,
											  const TCHAR** pszLongDesc, const TCHAR** pszCharType, const TCHAR** pszSpecialChars,
											  const TCHAR** pszDateFormat, const TCHAR** pszAdvancedRule, int* pnSizeMin, int* pnSizeMax,
											  int* pnOccMin, int* pnOccMax, int* pnOffset, bool* pbAutomaticallySet, bool* pbMandatory)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetRuleRestrictions")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetRuleRestrictions(szTransactionType, szMnemonic, pnRecordType, pnField, pnSubfield, pnItem, pszDesc, pszLongDesc,
										   pszCharType, pszSpecialChars, pszDateFormat, pszAdvancedRule, pnSizeMin, pnSizeMax, pnOccMin,
										   pnOccMax, pnOffset, pbAutomaticallySet, pbMandatory);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWGetValueList(CIWVerification* pIWVer, const TCHAR* szTransactionType, const TCHAR* szMnemonic, bool *pbMandatory,
									   int nDataArraySize, const TCHAR** rgszDataArray, const TCHAR** rgszDescArray, int *pnEntries)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetValueList")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetValueList(szTransactionType, szMnemonic, pbMandatory, nDataArraySize, rgszDataArray, rgszDescArray, pnEntries);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

//************************************************************/
//                                                            /
//                        Images                              /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI IWGetImage(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, 
								   const TCHAR** pszStorageFormat, long* pnLength, const void** pData)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		CStdString sStorageFormat;
		nRet = pIWTrans->GetImage(nRecordType, nRecordIndex, sStorageFormat, pnLength, (const BYTE**)pData);
		*pszStorageFormat = pIWTrans->CreateNewStringSlot(sStorageFormat);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWSetImage(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, const TCHAR* szInputFormat,
								   long nLength, void* pData, const TCHAR* szStorageFormat, float fCompression)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSetImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->SetImage(nRecordType, nRecordIndex, CStdString(szInputFormat), nLength, (BYTE*)pData, CStdString(szStorageFormat), fCompression);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWImportImage(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, const TCHAR* szPath,
									  const TCHAR* szStorageFormat, float fCompression, const TCHAR* szInputFormat)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWImportImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->ImportImage(nRecordType, nRecordIndex, CStdString(szPath), CStdString(szStorageFormat), fCompression, szInputFormat);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWExportImage(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex,
									  const TCHAR* szPath, const TCHAR* szOutputFormat)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWExportImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->ExportImage(nRecordType, nRecordIndex, szPath, szOutputFormat);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWGetImageInfo(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex,
									   const TCHAR** pszStorageFormat, long* pnLength, long* pnHLL, long* pnVLL, int* pnBitsPerPixel)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetImageInfo")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		CStdString sStorageFormat;
		nRet = pIWTrans->GetImageInfo(nRecordType, nRecordIndex, sStorageFormat, pnLength, pnHLL, pnVLL, pnBitsPerPixel);
		*pszStorageFormat = pIWTrans->CreateNewStringSlot(sStorageFormat);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWDeleteRecord(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWDeleteRecord")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->DeleteRecord(nRecordType, nRecordIndex);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

//************************************************************/
//                                                            /
//                   Image format conversion			      /
//                                                            /
//************************************************************/

OPENEBTS_API int WINAPI RAWtoWSQ(BYTE* pImageIn, long lWidth, long lHeight, long lDPI, float fRate, BYTE** ppImageOut, long *plLengthOut)
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

OPENEBTS_API int WINAPI BMPtoWSQ(BYTE* pImageIn, long lLengthIn, float fRate, BYTE** ppImageOut, long *plLengthOut)
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

OPENEBTS_API int WINAPI WSQtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI)
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

OPENEBTS_API int WINAPI WSQtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut)
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

OPENEBTS_API int WINAPI RAWtoBMP(int nWidth, int nHeight, int nDPI, int nDepth, BYTE* pImageIn, BYTE** ppImageOut, long *plLengthOut)
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

OPENEBTS_API int WINAPI BMPtoRAW(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut, long *plWidth, long *plHeight, long *plDPI)
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
		lLenPalette = (pbih->biClrUsed == 0 ? 1 << pbih->biBitCount :  pbih->biClrUsed) * sizeof(RGBQUAD);
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

OPENEBTS_API int WINAPI JPGtoBMP(BYTE* pImageIn, long lLengthIn, BYTE **ppImageOut, long *plLengthOut)
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

OPENEBTS_API int WINAPI BMPtoJPG(BYTE* pImageIn, long lLengthIn, long nCompression, BYTE **ppImageOut, long *plLengthOut)
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

OPENEBTS_API int WINAPI IWGetErrorCount(CIWTransaction* pIWTrans)
{
	int nRet = 0;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetErrorCount")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetErrorCount();

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

OPENEBTS_API int WINAPI IWGetError(CIWTransaction* pIWTrans, int nIndex, int* pnCode, const TCHAR** pszDesc)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetError")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		CStdString sDesc;
		nRet = pIWTrans->GetError(nIndex, pnCode, sDesc);
		*pszDesc = pIWTrans->CreateNewStringSlot(sDesc);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}
