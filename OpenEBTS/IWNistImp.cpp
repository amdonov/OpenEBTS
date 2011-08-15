
#include "Includes.h"
#include "OpenEBTS.h"
#include "TransactionDef.h"
#include "IWTransaction.h"
#include "IWVerification.h"
#include "OpenEBTSErrors.h"
#include "Common.h"
extern "C"
{
#include <wsq.h>
int debug = 0;
}

//************************************************************/
//                                                            /
//                  Transaction Management                    /
//                                                            /
//************************************************************//

OPENEBTS_API int WINAPI IWRead(const TCHARPATH *szPath, CIWVerification *pIWVer, CIWTransaction **ppIWTrans)
{
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

OPENEBTS_API int WINAPI IWReadVerification(const TCHARPATH* szPath, CIWVerification** ppIWVer, int nMaxParseError, TCHAR* szParseError)
{
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
		CStdStringPath sPath(szPath);

		nErrCode = pVerification->ReadVerificationFile(sPath, sParseError);
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

OPENEBTS_API int WINAPI IWWrite(CIWTransaction* pIWTrans, const TCHARPATH* szPath)
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

OPENEBTS_API int WINAPI IWWriteXML(CIWTransaction* pIWTrans, const TCHARPATH* szPath, int bValidate)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWWriteXML")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWTrans->WriteXML(szPath, bValidate != 0);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

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

OPENEBTS_API int WINAPI IWGetNumFields(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, int* pnCount)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWGetNumFields")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->GetNumFields(nRecordType, nRecordIndex, pnCount);

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
											  int* pnOccMin, int* pnOccMax, int* pnOffset, int* pbAutomaticallySet, int* pbMandatory)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetRuleRestrictions")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetRuleRestrictions(szTransactionType, szMnemonic, pnRecordType, pnField, pnSubfield, pnItem, pszDesc, pszLongDesc,
										   pszCharType, pszSpecialChars, pszDateFormat, pszAdvancedRule, pnSizeMin, pnSizeMax, pnOccMin,
										   pnOccMax, pnOffset, (bool*)pbAutomaticallySet, (bool*)pbMandatory);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWGetLocationIndex(CIWVerification* pIWVer, const TCHAR* szTransactionType, const TCHAR* szMnemonic,
											const TCHAR** pszLocationIndex)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetLocationIndex")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetLocationIndex(szTransactionType, szMnemonic, pszLocationIndex);

		IWS_END_CATCHEXCEPTION_BLOCK()
	}

	return nRet;
}

OPENEBTS_API int WINAPI IWGetValueList(CIWVerification* pIWVer, const TCHAR* szTransactionType, const TCHAR* szMnemonic, int *pbMandatory,
									   int nDataArraySize, const TCHAR** rgszDataArray, const TCHAR** rgszDescArray, int *pnEntries)
{
	int nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	if (pIWVer && pIWVer->IsLoaded())
	{
		IWS_BEGIN_EXCEPTION_METHOD("IWGetValueList")
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		nRet = pIWVer->GetValueList(szTransactionType, szMnemonic, (bool*)pbMandatory, nDataArraySize, rgszDataArray, rgszDescArray, pnEntries);

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
								   const TCHAR** pszStorageFormat, int* pnLength, const void** pData)
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
								   int nLength, void* pData, const TCHAR* szStorageFormat, float fCompression)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWSetImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->SetImage(nRecordType, nRecordIndex, CStdString(szInputFormat), nLength, (BYTE*)pData, CStdString(szStorageFormat), fCompression);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWImportImage(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex, const TCHARPATH* szPath,
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
									  const TCHARPATH* szPath, const TCHAR* szOutputFormat)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWExportImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->ExportImage(nRecordType, nRecordIndex, szPath, szOutputFormat);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWExportImageMem(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex,
										 BYTE** ppBuffer, int *pSize, const TCHAR* szOutputFormat)
{
	int nRet = IW_ERR_TRANSACTION_NOT_LOADED;

	IWS_BEGIN_EXCEPTION_METHOD("IWExportImageMem")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (pIWTrans && pIWTrans->IsTransactionLoaded())
		nRet = pIWTrans->ExportImageMem(nRecordType, nRecordIndex, ppBuffer, pSize, szOutputFormat);

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;	
}

OPENEBTS_API int WINAPI IWGetImageInfo(CIWTransaction* pIWTrans, int nRecordType, int nRecordIndex,
									   const TCHAR** pszStorageFormat, int* pnLength, int* pnHLL, int* pnVLL, int* pnBitsPerPixel)
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

OPENEBTS_API int WINAPI RAWtoBMP(int width, int height, int DPI, int depth, BYTE* pImageIn, BYTE** ppImageOut, int *pcbOut)
// Supports 1bpp, 8bpp and 24bpp
{
	int		ret = IW_ERR_IMAGE_CONVERSION;
	int		bytesPerRowBMP;
	int		bytesPerRowRAW;
	int		nBMPDataLen;

	IWS_BEGIN_EXCEPTION_METHOD("RAWtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (depth != 1 && depth != 8 && depth != 24) goto done;

	if (pImageIn)
	{
		if (depth == 1)
		{
			bytesPerRowRAW = (width+7)/8;
			bytesPerRowBMP = (bytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
		}
		else
		{
			bytesPerRowRAW = width * depth/8;
			bytesPerRowBMP = (bytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
		}
		nBMPDataLen = bytesPerRowBMP * height;

		// Define and initialize default color table, be it grayscale or two-tone
		DWORD color_count = 0;
		RGBQUAD *color_table = NULL;
		DWORD table_size = 0;

		if (depth != 24)
		{
			color_count = 1 << depth;
			color_table = new RGBQUAD[color_count];
			table_size = color_count * sizeof(RGBQUAD);

			if (depth == 1)
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
		BYTE *pData = (BYTE*)*ppImageOut;
		memset(pData, 0, dwFileSize);

		BITMAPFILEHEADER *pbmfh = (BITMAPFILEHEADER*)pData;
		pbmfh->bfType = 0x4D42;
		pbmfh->bfSize = dwFileSize;
		pbmfh->bfReserved1 = pbmfh->bfReserved2 = 0;
		pbmfh->bfOffBits = dwFileSize - nBMPDataLen;

		BITMAPINFO *pbi = (BITMAPINFO*)(pData + sizeof(BITMAPFILEHEADER));

		pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbi->bmiHeader.biWidth = width;
		pbi->bmiHeader.biHeight = height; // switch sign on biHeight convert to top-down
		pbi->bmiHeader.biPlanes = 1;
		pbi->bmiHeader.biBitCount = depth;
		pbi->bmiHeader.biCompression = 0;	//BI_RGB
        pbi->bmiHeader.biXPelsPerMeter = (LONG)(DPI * 10000.0 / 254.0 + 0.5);
        pbi->bmiHeader.biYPelsPerMeter = pbi->bmiHeader.biXPelsPerMeter;
		pbi->bmiHeader.biSizeImage = nBMPDataLen;

		// no color table for 24-bit, otherwise use default
		if (pbi->bmiHeader.biBitCount != 24)
			pbi->bmiHeader.biClrUsed = (DWORD)(1 << pbi->bmiHeader.biBitCount);

		// write the color table
		memcpy(pbi->bmiColors, color_table, table_size);

		// write the pixel data
		if (depth != 24)
		{
			for(int i = 0; i < height; i++)
			{
				memcpy(pData + pbmfh->bfOffBits + (i * bytesPerRowBMP),
					   pImageIn + (height - i - 1) * bytesPerRowRAW,
					   bytesPerRowRAW);
			}
		}
		else
		{
			// BGR not RGB!
			BYTE *ptrSrc = pImageIn + (height-1)*bytesPerRowRAW;
			BYTE *ptrDst = pData + pbmfh->bfOffBits;
			for(int i = 0; i < height; i++)
			{
				for(int j = 0; j < width; j++)
				{
					memcpy(ptrDst + j*3 + 0, ptrSrc + j*3 + 2, 1);
					memcpy(ptrDst + j*3 + 1, ptrSrc + j*3 + 1, 1);
					memcpy(ptrDst + j*3 + 2, ptrSrc + j*3 + 0, 1);
				}
				ptrSrc -= bytesPerRowRAW;
				ptrDst += bytesPerRowBMP;
			}
		}

		*pcbOut = dwFileSize;

		if (color_table) delete color_table;

		ret = IW_SUCCESS;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

done:
	return ret;
}

OPENEBTS_API int WINAPI BMPtoRAW(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut, int *pWidth, int *pHeight, int *pDPI)
//
// Supports 1bpp, 8bpp and 24bpp BMP inputs.
//
{
	int					ret = IW_ERR_IMAGE_CONVERSION;
	BITMAPFILEHEADER	*pbfh;
	BITMAPINFOHEADER	*pbih;
	int					bytesPerRowBMP;
	int					bytesPerRowRAW;
	int					i;
	int					lLenPalette;
	BYTE				*pBMPPtr;
	BYTE				*pRawPtr;
	bool				bInverse = false;

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoRAW")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	if (!pImageIn) goto done;

	// Support both file BMPs and DIBs, why not
	pbfh = (BITMAPFILEHEADER*)pImageIn;
	if (pbfh->bfType == 0x4d42)	 //"BM"
	{
		pbih = (BITMAPINFOHEADER*)(pImageIn + sizeof(BITMAPFILEHEADER));
	} else {
		pbih = (BITMAPINFOHEADER*)pImageIn;
	}

	if (pbih->biCompression != 0) goto done;	// only uncompressed supported (BI_RGB = 0)
	if (pbih->biBitCount != 1 && pbih->biBitCount != 8 && pbih->biBitCount != 24) goto done;

	// Set pBMPPtr to where image bits start.
	if (pbih->biBitCount != 24)
		lLenPalette = (pbih->biClrUsed == 0 ? 1 << pbih->biBitCount :  pbih->biClrUsed) * sizeof(RGBQUAD);
	else
		lLenPalette = 0;
	pBMPPtr = (BYTE*)pbih + sizeof(BITMAPINFOHEADER) + lLenPalette;

	if (pbih->biBitCount == 1)
	{
		bytesPerRowRAW = (pbih->biWidth+7)/8;
		bytesPerRowBMP = (bytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs

		// For 1bpp images, the input BMP could have the 0 pixels as white, in which case
		// we'll need to invert the output since we need 0 as black. to determine this we
		// just check for 0xff in any one of the RGB components of the first palette entry.
		if (lLenPalette != 0)
		{
			RGBQUAD *pRGB = (RGBQUAD*)((BYTE*)pbih + sizeof(BITMAPINFOHEADER));
			if (pRGB->rgbRed == 0xff)
			{
				bInverse = true;	// yes, 0 is white in the input bmp
			}
		}
	}
	else
	{
		bytesPerRowRAW = pbih->biWidth * pbih->biBitCount/8;
		bytesPerRowBMP = (bytesPerRowRAW + 3)/4*4;	// 4-byte aligned BMPs
	}
	// Since the BMP is upside down, move the pointer to the last row
	pBMPPtr += bytesPerRowBMP*(pbih->biHeight-1);

	*pcbOut = bytesPerRowRAW * pbih->biHeight;
	*ppImageOut = new BYTE[*pcbOut];

	pRawPtr = (BYTE*)*ppImageOut;

	// We transfer each row into the new array, with 4-byte alignment
	// and with the rows reversed
	for (i=0; i<pbih->biHeight; i++)
	{
		if (!bInverse)
		{
			memcpy(pRawPtr, pBMPPtr, bytesPerRowRAW);
			pRawPtr += bytesPerRowRAW;		// move down one raw row
		}
		else
		{
			// We use pBMPPtrTmp here so as to not change pBMPPtr, which gets adjusted below
			// by the proper row amount, bytesPerRowBMP. On the other hand pRawPtr gets
			// incremented by bytesPerRowRAW in this loop.
			BYTE *pBMPPtrTmp = pBMPPtr;
			for (int j=0; j<bytesPerRowRAW; j++)
			{
				*pRawPtr++ = ~(*pBMPPtrTmp++);
			}
		}
		pBMPPtr -= bytesPerRowBMP;			// move up one BMP row
	}

	*pWidth = pbih->biWidth;
	*pHeight = pbih->biHeight;
	*pDPI = (LONG)(pbih->biXPelsPerMeter * 254 / 10000.0 + 0.5);

	ret = IW_SUCCESS;

	IWS_END_CATCHEXCEPTION_BLOCK()

done:
	return ret;
}

OPENEBTS_API int WINAPI JPGtoBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("JPGtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_JPEG, pImageIn, cbIn, FIF_BMP, 0, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[JPGtoBMP] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI BMPtoJPG(BYTE* pImageIn, int cbIn, int nCompression, BYTE **ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoJPG")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_BMP, pImageIn, cbIn, FIF_JPEG, nCompression, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[BMPtoJPG] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI JP2toBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("JP2toBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_JP2, pImageIn, cbIn, FIF_BMP, 0, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[JP2toBMP] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI BMPtoJP2(BYTE* pImageIn, int cbIn, float fRate, BYTE **ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoJP2")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_BMP, pImageIn, cbIn, FIF_JP2, (int)fRate, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[BMPtoJP2] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI BMPtoPNG(BYTE* pImageIn, int cbIn, BYTE** ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoPNG")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_BMP, pImageIn, cbIn, FIF_PNG, 0, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[BMPtoPNG] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI PNGtoBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("PNGtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_PNG, pImageIn, cbIn, FIF_BMP, 0, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[PNGtoBMP] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI BMPtoFX4(BYTE* pImageIn, int cbIn, BYTE** ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoFX4")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_BMP, pImageIn, cbIn, FIF_TIFF, TIFF_CCITTFAX4, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[BMPtoFX4] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI FX4toBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut)
{
	int			ret = IW_ERR_IMAGE_CONVERSION;
	char		szErr[FREEIMAGEERRSIZE];

	IWS_BEGIN_EXCEPTION_METHOD("FX4toBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	FreeImage_ConvertInMemory(FIF_TIFF, pImageIn, cbIn, FIF_BMP, 0, ppImageOut, pcbOut);
	if (FreeImageError(szErr))
	{
		if (IsLogging())
		{
			CStdString sMsg;
			sMsg.Format(_T("[FX4toBMP] Error: %s"), szErr);
			LogMessage(sMsg);
		}
		goto Exit;
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:

	return ret;
}

OPENEBTS_API int WINAPI BMPtoWSQ(BYTE* pImageIn, int cbIn, float fRate, BYTE** ppImageOut, int *pcbOut)
// Input assumed to be 8 bpp
{
	int 	ret = IW_ERR_WSQ_COMPRESS;
	BYTE	*pRAW = NULL;
	int		cbSizeRAW;
	int		width;
	int		height;
	int		DPI;
	BYTE	*pWSQ;
	int		cbSizeWSQ;
	int		retWSQ;

	IWS_BEGIN_EXCEPTION_METHOD("BMPtoWSQ")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	ret = BMPtoRAW(pImageIn, cbIn, &pRAW, &cbSizeRAW, &width, &height, &DPI);
	if (ret != IW_SUCCESS) goto Exit;

	retWSQ = wsq_encode_mem(&pWSQ, &cbSizeWSQ, fRate, pRAW, width, height, 8, DPI, NULL);
	if (retWSQ != 0) goto Exit;

	*pcbOut = cbSizeWSQ;
	*ppImageOut = new BYTE[cbSizeWSQ];
	memcpy(*ppImageOut, pWSQ, cbSizeWSQ);
	free(pWSQ);	// (allocated by wsq library via malloc)

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:
	if (pRAW != NULL) delete pRAW;

	return ret;
}

OPENEBTS_API int WINAPI WSQtoBMP(BYTE* pImageIn, int cbIn, BYTE **ppImageOut, int *pcbOut)
{
	int		ret = IW_ERR_WSQ_DECOMPRESS;
	BYTE	*pRAW = NULL;
	int		width;
	int		height;
	int		DPI;
	int		bpp;
	int		lossy;
	int		retWSQ;

	IWS_BEGIN_EXCEPTION_METHOD("WSQtoBMP")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	retWSQ = wsq_decode_mem(&pRAW, &width, &height, &bpp, &DPI, &lossy, pImageIn, cbIn);
	if (retWSQ != 0) goto Exit;

	ret = RAWtoBMP(width, height, DPI, bpp, pRAW, ppImageOut, pcbOut);
	if (ret != IW_SUCCESS) goto Exit;

	IWS_END_CATCHEXCEPTION_BLOCK()

	ret = IW_SUCCESS;

Exit:
	if (pRAW != NULL) free(pRAW);	// (allocated by wsq library via malloc)

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
