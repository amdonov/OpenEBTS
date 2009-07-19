#include "stdafx.h"
#include "OpenEBTSErrors.h"
#include "TransactionDef.h"
#include "IWTransaction.h"
#include "Common.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "IWVerification.h"
#include "OpenEBTS.h"


#define ITEM_RECORDTYPE 1
#define ITEM_RECORDINDEX	2


CIWTransaction::CIWTransaction()
{
	m_pTransactionData = 0;
	m_bTransactionLoaded = FALSE;
	m_nIDCDigits = 2;
	m_pVerification = NULL;
	m_dNativeResolutionPPMM = 0.0;
}

CIWTransaction::~CIWTransaction()
{
	if (m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = 0;
	}

	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;

	for (int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);
		if (pRec) delete pRec;
	}
	m_RecordAry.clear();

	FreeErrors();
}

int CIWTransaction::New(const char *pszTransactionType, CIWVerification *pIWVer)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pRec1 = new CNISTRecord;

	if (pRec1)
	{
		pRec1->InitializeNewRecord(RECORD_TYPE1);
		pRec1->SetRecordType(RECORD_TYPE1);

		if (pszTransactionType && *pszTransactionType)
			pRec1->SetItem(pszTransactionType, TYPE1_TOT, 1, 1);

		m_RecordAry.push_back(pRec1);

		m_pVerification = pIWVer;
		m_bTransactionLoaded = TRUE;
	}

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CIWTransaction::New");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Result %d", sTraceFrom, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::ReadTransactionFile(const char *pFilePath)
{
	int nRet = IW_ERR_READING_FILE;
	FILE *f;

	m_sFilePath = pFilePath;

	IWS_BEGIN_EXCEPTION_METHOD("CIWTransaction::ReadTransactionFile")

	if (m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = 0;
	}

	m_bTransactionLoaded = FALSE;

	f = fopen(pFilePath, "rb");
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		long lSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		m_pTransactionData = new char[lSize];

		if (m_pTransactionData)
		{
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			if (fread(m_pTransactionData, 1, lSize, f) == (unsigned long)lSize)
				nRet = IW_SUCCESS;

			IWS_END_CATCHEXCEPTION_BLOCK()
		}		
		else
			nRet = IW_ERR_OUT_OF_MEMORY;

		IWS_BEGIN_CATCHEXCEPTION_BLOCK()
		fclose(f);
		IWS_END_CATCHEXCEPTION_BLOCK()
	}
	else
		nRet = IW_ERR_OPENING_FILE_FOR_READING;

	if (nRet != IW_SUCCESS && m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = 0;
	}

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CIWTransaction::ReadTransactionFile");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Path %s, Result %d", sTraceFrom, pFilePath, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::GetRecords()
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pRec1 = new CNISTRecord;
	char *pRecordData = m_pTransactionData;
	CNISTRecord *pRec;
	char *pData;

	nRet = pRec1->ReadRecord(pRecordData, RECORD_TYPE1);

	// Get and store the native resolution, this may be useful later
	if (pRec1->FindItem(TYPE1_NSR, 1, 1, (const char**) &pData) == IW_SUCCESS)
		m_dNativeResolutionPPMM = atof(pData);

	if (nRet == IW_SUCCESS)
	{
		int nRecords = 0;

		m_RecordAry.push_back(pRec1);

		// move to start of next record, type 2 I assume...
		pRecordData += pRec1->GetLogicalRecordLen();

		// Use the contents field of the type 1 record to process 
		// the rest of the file.
		
		if (GetNumSubfields(RECORD_TYPE1, 1, TYPE1_CNT, &nRecords) == IW_SUCCESS)
		{
			int nRecordType, nRecordIndex;
			int nSubField;
			const char *pData;

			if (g_bTraceOn)
			{
				CStdString sTraceFrom("CIWTransaction::GetRecords");
				CStdString sTraceMsg;
				
				sTraceMsg.Format("[%s] Records in file %d", sTraceFrom, nRecords);
				TraceMsg(sTraceMsg);
			}

			for (nSubField=1;nSubField <= nRecords && nRet == IW_SUCCESS;nSubField++)
			{
				// first get the expected record type
				if (FindItem(RECORD_TYPE1, 1, TYPE1_CNT, 
												nSubField, ITEM_RECORDTYPE, &pData) == IW_SUCCESS)
				{
					nRecordType = atoi(pData);

					// then get the record index
					if (FindItem(RECORD_TYPE1, 1, TYPE1_CNT, 
													nSubField, ITEM_RECORDINDEX, &pData) == IW_SUCCESS)
					{
						if (pData)
						{
							m_nIDCDigits = strlen(pData);
							nRecordIndex = atoi(pData);
						}
									
						switch (nRecordType)
						{
							case RECORD_TYPE1: // already read this one
								break;

							case RECORD_TYPE3:	// binary fields, get as much header info as we can
							case RECORD_TYPE4:	// types even though we cant do anything 
							case RECORD_TYPE5:	// with the data.
							case RECORD_TYPE6:
							case RECORD_TYPE7:
							case RECORD_TYPE8:
								{
									pRec = new CNISTRecord;

									pRec->SetRecordOffset(pRecordData - m_pTransactionData);
									nRet = pRec->ReadBinaryRecord(pRecordData, nRecordType);

									// Types 3 to 8 need to know the Native Scanning Resolution
									pRec->SetNativeScanningResolution(m_dNativeResolutionPPMM);

									// set pointer to next record
									pRecordData += pRec->GetLogicalRecordLen();

									if (nRet == IW_SUCCESS)
										m_RecordAry.push_back(pRec);
									else
										delete pRec;

									if (g_bTraceOn)
									{
										CStdString sTraceFrom("CIWTransaction::GetRecords");
										CStdString sTraceMsg;
										
										sTraceMsg.Format("[%s] Read record type %d, result %d", sTraceFrom, nRecordType, nRet);
										TraceMsg(sTraceMsg);
									}
									if (nRet != IW_SUCCESS)
									{
										CStdString sTraceFrom("CIWTransaction::GetRecords");
										CStdString sTraceMsg;
										
										sTraceMsg.Format("[%s] Read record FAILED. File %s, type %d, result %d", sTraceFrom, m_sFilePath, nRecordType, nRet);
										LogFile(NULL, sTraceMsg);
									}
									
								}
								break;

							case RECORD_TYPE2:
							case RECORD_TYPE9:
							case RECORD_TYPE10:
							case RECORD_TYPE11:
							case RECORD_TYPE12:
							case RECORD_TYPE13:
							case RECORD_TYPE14:
							case RECORD_TYPE15:
							case RECORD_TYPE16:
							case RECORD_TYPE17:
								{
									pRec = new CNISTRecord;

									nRet = pRec->ReadRecord(pRecordData, nRecordType);

									// set pointer to next record
									pRecordData += pRec->GetLogicalRecordLen();

									if (nRet == IW_SUCCESS)
										m_RecordAry.push_back(pRec);
									else
										delete pRec;

									if (g_bTraceOn)
									{
										CStdString sTraceFrom("CIWTransaction::GetRecords");
										CStdString sTraceMsg;
										
										sTraceMsg.Format("[%s] Read record type %d, result %d", sTraceFrom, nRecordType, nRet);
										TraceMsg(sTraceMsg);
									}
									if (nRet != IW_SUCCESS)
									{
										CStdString sTraceFrom("CIWTransaction::GetRecords");
										CStdString sTraceMsg;
										
										sTraceMsg.Format("[%s] Read record FAILED. File %s, type %d, result %d", sTraceFrom, m_sFilePath, nRecordType, nRet);
										LogFile(NULL, sTraceMsg);
									}
									
								}
								break;

							default:
								break;
						}
					}
				}
			}
		}								
	}
	else
	{
		if (pRec1)
			delete pRec1;
	}

	// no longer need file data, its all in memory structures now...
	if (m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = 0;
	}

	if (nRet == IW_SUCCESS)
		m_bTransactionLoaded = TRUE;

	return nRet;
}

int CIWTransaction::AddRecord(int RecordType, int *pRecordIndex)
{
	int nRet = IW_SUCCESS;
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;

	//DebugOutRecords("AddRecord start");

	pRec = new CNISTRecord;

	if (pRec)
	{
		int nIDC = 0;
		char szIDC[10];

		GetRecordTypeMaxIndex(RecordType, &nIDC);

		nIDC++;
		_stprintf_s(szIDC, 10, "%02d", nIDC);

		pRec->InitializeNewRecord(RecordType);

		// by default set the IDC field to the next ID, application can modify
		pRec->SetItem((LPCSTR)szIDC, REC_TAG_IDC, 1, 1);

		m_RecordAry.push_back(pRec);

		*pRecordIndex = nIDC;

		Type1AddRecordIDC(pRec, RecordType, nIDC);
	}

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CIWTransaction::AddRecord");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Type %d, Index %d", sTraceFrom, RecordType, nRet);
		TraceMsg(sTraceMsg);
	}

	//DebugOutRecords("AddRecord end");

	return nRet;
}

int CIWTransaction::DebugOutRecords(const char *szContext)
{
#ifdef _DEBUG
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);
	const char *pData = 0;
	const char *pData1 = 0;
	const char *pData2 = 0;
	char szFoo[256];

	wsprintf(szFoo, "\n------------------------------- %s\n", szContext);
	OutputDebugString(szFoo);

	if (pType1Rec->FindItem(TYPE1_CNT, 1, 2, &pData) == IW_SUCCESS && pData)
	{
		int nCount = atoi(pData);

		wsprintf(szFoo, "CONTENTS: (01) %02d items\n", nCount);
		OutputDebugString(szFoo);

		for (int i=2; i<=nCount+1; i++)
		{
			if (pType1Rec->FindItem(TYPE1_CNT, i, 1, &pData1))
			{
				OutputDebugString("error\n");
				continue;
			}
			if (pType1Rec->FindItem(TYPE1_CNT, i, 2, &pData2))
			{
				OutputDebugString("error\n");
				continue;
			}
			wsprintf(szFoo, "CONTENTS: (%02d) %02s %02s\n", i, pData1, pData2);
			OutputDebugString(szFoo);
		}
	}

	for (int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);
		wsprintf(szFoo, "Record %02d, Type %02d\n", i, pRec->GetRecordType());
		OutputDebugString(szFoo);
	}
	OutputDebugString("-------------------------------\n");
#endif

	return IW_SUCCESS;
}

int CIWTransaction::DeleteRecord(int RecordType, int RecordIndex)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	CNISTRecord *pRecTmp = 0;
	int nRecordIndex = 0;

	//DebugOutRecords("DeleteRecord start");

	pRec = GetRecord(RecordType, RecordIndex);

	for (int i = 0; i < nSize; i++)
	{
		pRecTmp = m_RecordAry.at(i);

		if (pRec == pRecTmp)
		{
			// Adjust contents table
			nRet = Type1DeleteRecordIDC(pRec, RecordIndex);
			// Note: we delete the record after Type1DeleteRecordIDC
			m_RecordAry.erase(m_RecordAry.begin() + i);
			delete pRec;
			break;
		}
	}

	//DebugOutRecords("DeleteRecord end");

	return nRet;
}

int CIWTransaction::Type1AddRecordIDC(CNISTRecord *pRecord, int nRecordType, int nIDC)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pRec = GetRecord(RECORD_TYPE1, 1);

	//DebugOutRecords("Type1AddRecordIDC start");

	if (pRec)
	{
		int nCount = 0;
		const char *pData = 0;

		if (pRec->FindItem(TYPE1_CNT, 1, 2, &pData) == IW_SUCCESS && pData)
		{
			int nCount = atoi(pData);
			char szCount[10];
			char szIDC[10];

			nCount++;
			wsprintf(szCount, "%02d", nCount);

			// update the logical record count
			pRec->SetItem((LPCSTR)szCount, TYPE1_CNT, 1, 2);

			// add record to contents field in type1 record
			wsprintf(szIDC, "%d", nRecordType);
			nRet = pRec->SetItem((LPCSTR)szIDC, TYPE1_CNT, nCount+1, 1);

			// only required to use 1 digit, but try to be consistent
			if (m_nIDCDigits == 1)
				wsprintf(szIDC, "%d", nIDC);
			else if (m_nIDCDigits == 2)
				wsprintf(szIDC, "%02d", nIDC);
			else if (m_nIDCDigits == 3)
				wsprintf(szIDC, "%03d", nIDC);

			nRet = pRec->SetItem((LPCSTR)szIDC, TYPE1_CNT, nCount+1, 2);
		}
	}

	//DebugOutRecords("Type1AddRecordIDC end");

	return nRet;
}

int CIWTransaction::Type1UpdateIDC(CNISTRecord *pRecord, int nIDC)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);

	//DebugOutRecords("Type1UpdateIDC start");

	if (pType1Rec)
	{
		int nCount = 0;
		const char *pData = 0;
		int nRecPos = 0;
		int nSize = m_RecordAry.size();
		CNISTRecord *pRec = 0;

		for (int i = 0; i < nSize; i++)
		{
			pRec = m_RecordAry.at(i);

			if (pRec && pRec == pRecord)
			{
				nRecPos = i;
				break;
			}
		}
		
		char szIDC[10];

		// only required to use 1 digit, but try to be consistent
		if (m_nIDCDigits == 1)
			wsprintf(szIDC, "%d", nIDC);
		else if (m_nIDCDigits == 2)
			wsprintf(szIDC, "%02d", nIDC);
		else if (m_nIDCDigits == 3)
			wsprintf(szIDC, "%03d", nIDC);

		nRet = pType1Rec->SetItem(szIDC, TYPE1_CNT, nRecPos+1, 2);
	}

	//DebugOutRecords("Type1UpdateIDC end");

	return nRet;
}

int CIWTransaction::Type1DeleteRecordIDC(CNISTRecord *pRecord, int nIDC)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);

	//DebugOutRecords("Type1DeleteRecordIDC start");

	if (pType1Rec)
	{
		int nCount = 0;
		const char *pData = 0;
		int nRecPos = 0;
		int nSize = m_RecordAry.size();
		CNISTRecord *pRec = 0;
		char szCount[10];

		for (int i = 0; i < nSize; i++)
		{
			pRec = m_RecordAry.at(i);

			if (pRec && pRec == pRecord)
			{
				nRecPos = i;
				break;
			}
		}

		nRet = pType1Rec->DeleteSubfield(TYPE1_CNT, nRecPos+1);

		// update the logical record count, if delete was successful
		if (nRet == IW_SUCCESS)
		{
			if (pType1Rec->FindItem(TYPE1_CNT, 1, 2, &pData) == IW_SUCCESS && pData)
			{
				nCount = atoi(pData);

				nCount--;
				wsprintf(szCount, "%02d", nCount);

				pType1Rec->SetItem((LPCSTR)szCount, TYPE1_CNT, 1, 2);
			}
		}
	}

	//DebugOutRecords("Type1DeleteRecordIDC end");

	return nRet;
}

CNISTRecord *CIWTransaction::GetRecord(int nRecordType, int nRecordIndex)
{
	int nSize = m_RecordAry.size();
	CNISTRecord *pRet = 0;
	CNISTRecord *pRec = 0;
	int nTypeCount = 0;
	int nIndex = 0;

	for (int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);

		if (pRec)
		{
			if (pRec->GetRecordType() == nRecordType)
			{
				nIndex++;
				if (nIndex == nRecordIndex)
				{
					pRet = pRec;
					break;
				}
			}
		}
	}

	return pRet;
}

int CIWTransaction::GetNumSubfields(int RecordType, int RecordIndex, int Field, int *pCount)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetNumSubfields(Field, pCount);

	return nRet;
}

int CIWTransaction::GetNumItems(int RecordType, int RecordIndex, int Field, int Subfield, int *pCount)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetNumItems(Field, Subfield, pCount);

	return nRet;
}

int CIWTransaction::GetNextField(int RecordType, int RecordIndex, 
													int Field, int *pNextField)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetNextField(Field, pNextField);

	return nRet;
}

int CIWTransaction::FindItem(int RecordType, int RecordIndex, int Field, 
									int Subfield, int Item, const char **ppData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->FindItem(Field, Subfield, Item, ppData);

	return nRet;
}

int CIWTransaction::SetItem(const char *pData, int RecordType, int RecordIndex, 
									int Field, int Subfield, int Item)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
	{
		char szIDC[10];
		const char *pItemData = pData;

		// although it doesn't matter, use 2 digit IDC fields
		if (Field == REC_TAG_IDC && RecordType != RECORD_TYPE1)
		{
			int nIDC = atoi(pData);

			_stprintf_s(szIDC, 10, "%02d", nIDC);
			pItemData = szIDC;
		}

		if ((nRet = pRecord->SetItem(pItemData/*pData*/, Field, Subfield, Item)) == IW_SUCCESS)
		{
			if (Field == REC_TAG_IDC && RecordType != RECORD_TYPE1)
			{
				int nIDC = atoi(pData);
				// modifying the IDC field, so update the contents field
				// in the type1 record
				Type1UpdateIDC(pRecord, nIDC);
			}
		}
	}

	return nRet;
}

int CIWTransaction::GetRecordTypeCount(int RecordType, int *pRecordTypeCount)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nCount = 0;

	*pRecordTypeCount = 0;

	for (int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);

		if (pRec)
		{
			if (pRec->GetRecordType() == RecordType)
				nCount++;
		}
	}

	if (nCount)
	{
		*pRecordTypeCount = nCount;
		nRet = IW_SUCCESS;
	}

	return nRet;
}

int CIWTransaction::GetRecordTypeMaxIndex(int RecordType, int *pIndex)
{
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nIndex = 0;

	for (int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);

		if (pRec)
		{
			if (pRec->GetRecordType() == RecordType)
			{
				nIndex++;
			}
		}
	}

	*pIndex = nIndex;

	return IW_SUCCESS;
}

int CIWTransaction::GetNumRecords(int *pRecords)
{
	*pRecords = m_RecordAry.size();

	return IW_SUCCESS;
}

int CIWTransaction::GetImage(int RecordType, int RecordIndex, const char **ppStorageFormat, 
							 long *pLength, const void **ppData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
	nRet = pRecord->GetImage(ppStorageFormat, pLength, ppData);

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CIWTransaction::GetImage");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Type %d, Index %d, Result %d", sTraceFrom, RecordType, RecordIndex, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::GetImageInfo(int RecordType, int RecordIndex, const char **ppStorageFormat, 
								 long *pLength, long *phll, long *pvll, int *pBitsPerPixel)
{
	int	nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
	nRet = pRecord->GetImageInfo(ppStorageFormat, pLength, phll, pvll, pBitsPerPixel);

	return nRet;
}

int CIWTransaction::SetImage(int RecordType, int RecordIndex, const char *pInputFormat, 
							 long Length, void *pData, const char *pStorageFormat, 
							 float Compression)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = 0;

	if (RecordIndex == 0) // flag to add new image
	{
		pRecord = new CNISTRecord;

		if (pRecord)
		{
			int nIndex;
			
			GetRecordTypeCount(RecordType, &nIndex);
			nIndex++;

			pRecord->SetRecordType(RecordType);
			m_RecordAry.push_back(pRecord);
		}
	}
	else
		pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->SetImage(pInputFormat, RecordIndex, Length, pData, pStorageFormat, Compression);

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CIWTransaction::SetImage");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Type %d, Index %d, Input format %s, Result %d", sTraceFrom, RecordType, RecordIndex, (pInputFormat ? pInputFormat : ""), nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::ImportImage(int RecordType, int RecordIndex, const char* Path, 
								const char* pStorageFormat, float Compression, 
								const char* pInputFormat)
// Same as IWSetImage, but from a file.
{
	int		nRet;
	long	Length;
	void	*pData = NULL;
	CStdString csExt;
	FILE	*f;

	IWS_BEGIN_EXCEPTION_METHOD("CIWTransaction::ImportImage")

	f = fopen(Path, "rb");
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		Length = ftell(f);
		fseek(f, 0, SEEK_SET);

		pData = new char[Length];

		if (pData)
		{
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			if (fread(pData, 1, Length, f) == (unsigned long)Length)
				nRet = IW_SUCCESS;

			IWS_END_CATCHEXCEPTION_BLOCK()
		}		
		else
			nRet = IW_ERR_OUT_OF_MEMORY;

		IWS_BEGIN_CATCHEXCEPTION_BLOCK()
		fclose(f);
		IWS_END_CATCHEXCEPTION_BLOCK()

		// If pInputFormat is NULL, we determine the input format via the file extension
		if (!pInputFormat)
		{
			CStdString csPath = Path;

			csExt = csPath.Right(csPath.GetLength() - csPath.ReverseFind('.') - 1);
		}
		else
		{
			csExt = pInputFormat;
		}

		if (nRet == NO_ERROR)
			nRet = SetImage(RecordType, RecordIndex, csExt, Length, pData, pStorageFormat, Compression);

	}
	else
		nRet = IW_ERR_OPENING_FILE_FOR_READING;

	if (pData)
	{
		delete [] pData;
		pData = NULL;
	}

	return nRet;
}

int CIWTransaction::ExportImage(int RecordType, int RecordIndex, 
								const char* Path, const char* OutputFormat)
// Currently on supports RAW to BMP, since NistImageExport uses this functionality, 
// but eventually we could easily support all format combinations using the existing
// XXXtoYYY functions.
{
	int			nRet = IW_ERR_RECORD_NOT_FOUND;
	const char	*pFmt;
	const void	*pData;
	long		lLength;
	long		lW;
	long		lH;
	long		lBpp;
	long		lLengthNew;
	double		dPPMM;	// pixels per millimeter
	long		lDPI;
	BYTE		*pNew = NULL;
	FILE		*f;

	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);
	if (!pRecord) goto done;

	nRet = pRecord->GetImageInfo(&pFmt, &lLength, &lW, &lH, (int*)&lBpp);
	if (nRet != IW_SUCCESS) goto done;

	if (!_stricmp(pFmt, "raw") && !_stricmp(OutputFormat, "bmp"))
	{
		nRet = pRecord->GetImage(&pFmt, &lLength, &pData);
		if (nRet != IW_SUCCESS) goto done;

		nRet = pRecord->GetImageResolution(&dPPMM);
		if (nRet != IW_SUCCESS) goto done;
		lDPI = (long)(dPPMM * 25.4);

		nRet = RAWtoBMP(lW, lH, lDPI, lBpp, (BYTE*)pData, &pNew, &lLengthNew);
		if (nRet != IW_SUCCESS) goto done;

		f = fopen(Path, "wb");
		if (f != NULL)
		{
			fwrite(pNew, 1, lLengthNew, f);
			fclose(f);
			nRet = IW_SUCCESS;
		}
		else
		{
			nRet = IW_ERR_OPENING_FILE_FOR_WRITING;
		}
	}
	else
	{
		nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	}

done:
	if (pNew != NULL) delete [] pNew;

	return nRet;
}

int CIWTransaction::SetRecordLengths()
{
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;

	for (int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
	{
		pRec = m_RecordAry.at(i);

		if (pRec)
		{
			pRec->GetRecordLen();
		}
	}

	return nRet;
}

int CIWTransaction::Write(const char *pPath)
{
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;
	FILE *f = NULL;

	SetRecordLengths();

	f = fopen(pPath, "wb");
	if (f != NULL)
	{
		for (int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
		{
			pRec = m_RecordAry.at(i);

			if (pRec)
			{
				if (CNISTRecord::IsBinaryType(pRec->GetRecordType()))
				{
					nRet = pRec->WriteBinary(f);
				}
				else
				{
					nRet = pRec->Write(f);
				}
			}
		}
	}
	else
		nRet = IW_ERR_OPENING_FILE_FOR_WRITING;

	if (nRet != IW_SUCCESS)
	{
		CStdString sTraceFrom("CIWTransaction::WriteBinary");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Write record FAILED. File %s, result %d", sTraceFrom, pPath, nRet);
		LogFile(NULL, sTraceMsg);
	}

	if (f != NULL) fclose(f);

	return nRet;
}

int CIWTransaction::WriteXML(const char *pPath, BOOL bValidate)
{
	int				nRet = IW_SUCCESS;
	FILE			*f;
	BYTE*			pXML;
	long			lLengthXML;
	char			*pErr;

	FreeErrors();

	SetRecordLengths();

	// Not yet supported
	//
	//nRet = GetXML(&pXML, bValidate, &lLengthXML, &pErr);
	//

	return IW_ERR_WRITING_FILE;

	if (nRet != IW_SUCCESS)
	{
		if (pErr)
		{
			AddError(pErr, 0);
			free(pErr);
		}
		return nRet;
	}

	f = fopen(pPath, "wb");
	if (f != NULL)
	{
		fwrite(pXML, 1, lLengthXML, f);
		fclose(f);
		free(pXML);
	}
	else
		nRet = IW_ERR_OPENING_FILE_FOR_WRITING;

	if (nRet != IW_SUCCESS)
	{
		CStdString sTraceFrom("CIWTransaction::WriteXML");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] Write record FAILED. File %s, result %d", sTraceFrom, pPath, nRet);
		LogFile(NULL, sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::SetVerification(CIWVerification *pIWVer)
{
	int nRet = IW_SUCCESS;

	if (m_bTransactionLoaded)
		m_pVerification = pIWVer;
	else
		nRet = IW_ERR_TRANSACTION_NOT_LOADED;
	
	return nRet;
}

BOOL CIWTransaction::IsVerificationLoaded() 
{ 
	BOOL bRet = FALSE; 
	
	if (m_pVerification != NULL) 
		bRet = m_pVerification->IsLoaded(); 
	
	return bRet; 
} 

int CIWTransaction::Get(const char *pMnemonic, const char **ppData, int Index, int RecordIndex)
{
	int nRet = IW_SUCCESS;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(pMnemonic, Index, RecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = FindItem(recordType, recordIndex, field, subField, item, ppData);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

int CIWTransaction::Set(const char *pMnemonic, const char *pData, int StartIndex, int RecordIndex)
{
	int nRet = IW_SUCCESS;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(pMnemonic, StartIndex, RecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = SetItem(pData, recordType, recordIndex, field, subField, item);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

int CIWTransaction::Occurrences(const char *pMnemonic, int *pOccurrences, int RecordIndex)
{
	int nRet = IW_SUCCESS;

	*pOccurrences = 0;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(pMnemonic, 1/*StartIndex*/, RecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = GetNumItems(recordType, recordIndex, field, subField, pOccurrences);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

int CIWTransaction::Verify()
{
	int nRet = IW_SUCCESS;

	if (IsVerificationLoaded())
	{
		FreeErrors();
		SetRecordLengths();
		nRet = m_pVerification->VerifyTransaction(this);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

void CIWTransaction::FreeErrors()
{
	m_ErrAry.clear();
}

void CIWTransaction::AddError(char* szErr, int nCode)
{
	CNISTErr *pErr = new CNISTErr;

	pErr->m_nCode = nCode;
	strcpy_s(pErr->m_szErr, MAXERRORLEN, szErr);

	m_ErrAry.push_back(*pErr);
}

int CIWTransaction::GetErrorCount()
{
	return m_ErrAry.size();
}

int CIWTransaction::GetError(int Index, int* Code, const char** Desc)
{
	int nRet = IW_ERR_INDEX_OUT_OF_RANGE;

	if (Index >= 0 && Index < (int)m_ErrAry.size())
	{
		CNISTErr *pErr = &m_ErrAry.at(Index);
		*Code = pErr->m_nCode;
		*Desc = pErr->m_szErr;
		nRet = IW_SUCCESS;
	}

	return nRet;
}
