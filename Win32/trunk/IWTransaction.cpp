#include "stdafx.h"
#include "OpenEBTSErrors.h"
#include "TransactionDef.h"
#include "IWTransaction.h"
#include "Common.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "IWVerification.h"
#include "OpenEBTS.h"


#define ITEM_RECORDTYPE		1
#define ITEM_RECORDINDEX	2

CIWTransaction::CIWTransaction()
{
	m_pTransactionData = NULL;
	m_bTransactionLoaded = false;
	m_nIDCDigits = 2;
	m_pVerification = NULL;
	m_dNativeResolutionPPMM = 0.0;
}

CIWTransaction::~CIWTransaction()
{
	if (m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = NULL;
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

int CIWTransaction::New(CStdString sTransactionType, CIWVerification *pIWVer)
{
	int nRet = IW_SUCCESS;

	// Every transaction has exactly one Type-1 record, so we add it automatically.
	CNISTRecord *pRec1 = new CNISTRecord;

	if (pRec1)
	{
		pRec1->InitializeNewRecord(RECORD_TYPE1);
		pRec1->SetRecordType(RECORD_TYPE1);
		pRec1->SetItem(sTransactionType, TYPE1_TOT, 1, 1);

		m_RecordAry.push_back(pRec1);

		m_pVerification = pIWVer;
		m_bTransactionLoaded = true;

#ifdef UNICODE
		// In the UNICODE version we specify that we will be writing UTF-8 chars into fields
		Set(L"T1_DCS_CSI", L"003", 1, 1);	// Character Set Index is "003" for UTF-8
		Set(L"T1_DCS_CSN", L"UTF-8", 1, 1);	// Character Set Name
#endif
	}

	if (g_bTraceOn)
	{
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::New] Result %d"), nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::ReadTransactionFile(CStdString sFilePath)
{
	int nRet = IW_ERR_READING_FILE;
	FILE *f;

	m_sFilePath = sFilePath;

	IWS_BEGIN_EXCEPTION_METHOD("CIWTransaction::ReadTransactionFile")

	if (m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = NULL;
	}

	m_bTransactionLoaded = false;

	f = _tfopen(sFilePath, _T("rb"));
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		long lSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		m_pTransactionData = new BYTE[lSize];

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
		m_pTransactionData = NULL;
	}

	if (g_bTraceOn)
	{
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::ReadTransactionFile] Path %s, Result %d"), sFilePath, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::ReadTransactionFileMem(const BYTE *pMemFile, int MemFileSize)
{
	int nRet = IW_ERR_READING_FILE;

	m_sFilePath = _T("");

	IWS_BEGIN_EXCEPTION_METHOD("CIWTransaction::ReadTransactionFileMem")

	if (m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = 0;
	}

	m_bTransactionLoaded = false;

	m_pTransactionData = new BYTE[MemFileSize];

	if (m_pTransactionData)
	{
		IWS_BEGIN_CATCHEXCEPTION_BLOCK()

		memcpy(m_pTransactionData, pMemFile, MemFileSize);
		nRet = IW_SUCCESS;

		IWS_END_CATCHEXCEPTION_BLOCK()
	}		
	else
		nRet = IW_ERR_OUT_OF_MEMORY;

	if (nRet != IW_SUCCESS && m_pTransactionData)
	{
		delete [] m_pTransactionData;
		m_pTransactionData = 0;
	}

	if (g_bTraceOn)
	{
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::ReadTransactionFileMem] Result %d"), nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::GetRecords()
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pRec1 = new CNISTRecord;
	BYTE* pRecordData = m_pTransactionData;
	CNISTRecord *pRec;
	CStdString sData;

	nRet = pRec1->ReadRecord(pRecordData, RECORD_TYPE1);

	// Get and store the native resolution, this may be useful later
	if (pRec1->FindItem(TYPE1_NSR, 1, 1, sData) == IW_SUCCESS)
		m_dNativeResolutionPPMM = _tstof(sData);

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

			if (g_bTraceOn)
			{
				CStdString sTraceMsg;
				sTraceMsg.Format(_T("[CIWTransaction::GetRecords] Records in file %d"), nRecords);
				TraceMsg(sTraceMsg);
			}

			for (nSubField=1;nSubField <= nRecords && nRet == IW_SUCCESS;nSubField++)
			{
				// first get the expected record type
				if (FindItem(RECORD_TYPE1, 1, TYPE1_CNT, nSubField, ITEM_RECORDTYPE, sData) == IW_SUCCESS)
				{
					nRecordType = _ttoi(sData);

					// then get the record index
					if (FindItem(RECORD_TYPE1, 1, TYPE1_CNT, nSubField, ITEM_RECORDINDEX, sData) == IW_SUCCESS)
					{
						m_nIDCDigits = sData.GetLength();
						nRecordIndex = _ttoi(sData);

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
										CStdString sTraceMsg;
										sTraceMsg.Format(_T("[CIWTransaction::GetRecords] Read record type %d, result %d"), nRecordType, nRet);
										TraceMsg(sTraceMsg);
									}
									if (nRet != IW_SUCCESS)
									{
										CStdString sTraceMsg;
										sTraceMsg.Format(_T("[CIWTransaction::GetRecords] Read record FAILED. File %s, type %d, result %d"), m_sFilePath, nRecordType, nRet);
										LogFile(sTraceMsg);
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
										CStdString sTraceMsg;
										sTraceMsg.Format(_T("[CIWTransaction::GetRecords] Read record type %d, result %d"), nRecordType, nRet);
										TraceMsg(sTraceMsg);
									}
									if (nRet != IW_SUCCESS)
									{
										CStdString sTraceMsg;
										sTraceMsg.Format(_T("[CIWTransaction::GetRecords] Read record FAILED. File %s, type %d, result %d"), m_sFilePath, nRecordType, nRet);
										LogFile(sTraceMsg);
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
		m_bTransactionLoaded = true;

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
		CStdString sIDC;

		GetRecordTypeMaxIndex(RecordType, &nIDC);

		nIDC++;
		sIDC.Format(_T("%02d"), nIDC);

		pRec->InitializeNewRecord(RecordType);

		// by default set the IDC field to the next ID, application can modify
		pRec->SetItem(sIDC, REC_TAG_IDC, 1, 1);
		m_RecordAry.push_back(pRec);

		*pRecordIndex = nIDC;

		Type1AddRecordIDC(pRec, RecordType, nIDC);
	}

	if (g_bTraceOn)
	{
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::AddRecord] Type %d, Index %d"), RecordType, nRet);
		TraceMsg(sTraceMsg);
	}

	//DebugOutRecords("AddRecord end");

	return nRet;
}

int CIWTransaction::DebugOutRecords(CStdString sContext)
{
#ifdef _DEBUG
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);
	CStdString sData;
	CStdString sData1;
	CStdString sData2;
	CStdString sFoo;

	sFoo.Format(_T("\n------------------------------- %s\n"), sContext);
	OutputDebugString(sFoo);

	if (pType1Rec->FindItem(TYPE1_CNT, 1, 2, sData) == IW_SUCCESS)
	{
		int nCount = _ttoi(sData);

		sFoo.Format(_T("CONTENTS: (01) %02d items\n"), nCount);
		OutputDebugString(sFoo);

		for (int i=2; i<=nCount+1; i++)
		{
			if (pType1Rec->FindItem(TYPE1_CNT, i, 1, sData1) != IW_SUCCESS)
			{
				OutputDebugString(_T("error\n"));
				continue;
			}
			if (pType1Rec->FindItem(TYPE1_CNT, i, 2, sData2) != IW_SUCCESS)
			{
				OutputDebugString(_T("error\n"));
				continue;
			}

			sFoo.Format(_T("CONTENTS: (%02d) %02s %02s\n"), i, sData1, sData2);
			OutputDebugString(sFoo);
		}
	}

	for (int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);
		sFoo.Format(_T("Record %02d, Type %02d\n"), i, pRec->GetRecordType());
		OutputDebugString(sFoo);
	}
	OutputDebugString(_T("-------------------------------\n"));
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

	return nRet;
}

int CIWTransaction::Type1AddRecordIDC(CNISTRecord *pRecord, int nRecordType, int nIDC)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pRec = GetRecord(RECORD_TYPE1, 1);

	if (pRec)
	{
		int nCount = 0;
		CStdString sData;
		CStdString sCount;
		CStdString sIDC;

		if (pRec->FindItem(TYPE1_CNT, 1, 2, sData) == IW_SUCCESS)
		{
			int nCount = _ttoi(sData);

			nCount++;
			sCount.Format(_T("%02d"), nCount);

			// update the logical record count
			pRec->SetItem(sCount, TYPE1_CNT, 1, 2);

			// add record to contents field in type1 record
			sIDC.Format(_T("%d"), nRecordType);
			nRet = pRec->SetItem(sIDC, TYPE1_CNT, nCount+1, 1);

			// only required to use 1 digit, but try to be consistent
			if (m_nIDCDigits == 1)
				sIDC.Format(_T("%d"), nIDC);
			else if (m_nIDCDigits == 2)
				sIDC.Format(_T("%02d"), nIDC);
			else if (m_nIDCDigits == 3)
				sIDC.Format(_T("%03d"), nIDC);

			nRet = pRec->SetItem(sIDC, TYPE1_CNT, nCount+1, 2);
		}
	}

	return nRet;
}

int CIWTransaction::Type1UpdateIDC(CNISTRecord *pRecord, int nIDC)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);

	if (pType1Rec)
	{
		int nCount = 0;
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

		CStdString sIDC;

		// only required to use 1 digit, but try to be consistent
		if (m_nIDCDigits == 1)
			sIDC.Format(_T("%d"), nIDC);
		else if (m_nIDCDigits == 2)
			sIDC.Format(_T("%02d"), nIDC);
		else if (m_nIDCDigits == 3)
			sIDC.Format(_T("%03d"), nIDC);

		nRet = pType1Rec->SetItem(sIDC, TYPE1_CNT, nRecPos+1, 2);
	}

	return nRet;
}

int CIWTransaction::Type1DeleteRecordIDC(CNISTRecord *pRecord, int nIDC)
{
	int nRet = IW_SUCCESS;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);

	if (pType1Rec)
	{
		int nCount = 0;
		CStdString sData;
		int nRecPos = 0;
		int nSize = m_RecordAry.size();
		CNISTRecord *pRec = 0;
		CStdString sCount;

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
			if (pType1Rec->FindItem(TYPE1_CNT, 1, 2, sData) == IW_SUCCESS)
			{
				nCount = _ttoi(sData);

				nCount--;
				sCount.Format(_T("%02d"), nCount);

				pType1Rec->SetItem(sCount, TYPE1_CNT, 1, 2);
			}
		}
	}

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

int CIWTransaction::FindItem(int RecordType, int RecordIndex, int Field, int Subfield, int Item, CStdString& sData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->FindItem(Field, Subfield, Item, sData);

	return nRet;
}

int CIWTransaction::SetItem(CStdString sData, int RecordType, int RecordIndex, int Field, int Subfield, int Item)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
	{
		// although it doesn't matter, use 2 digit IDC fields
		if (Field == REC_TAG_IDC && RecordType != RECORD_TYPE1)
		{
			int nIDC = _ttoi(sData);
			sData.Format(_T("%02d"), nIDC);
		}

		if ((nRet = pRecord->SetItem(sData, Field, Subfield, Item)) == IW_SUCCESS)
		{
			if (Field == REC_TAG_IDC && RecordType != RECORD_TYPE1)
			{
				int nIDC = _ttoi(sData);
				// modifying the IDC field, so update the contents field in the type1 record
				Type1UpdateIDC(pRecord, nIDC);
			}
		}
	}

	return nRet;
}

int CIWTransaction::RemoveItem(int RecordType, int RecordIndex, int Field, int Subfield, int Item)
{
	int nRet = IW_SUCCESS;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CIWTransaction::RemoveItem");
		CStdString sTraceMsg;

		sTraceMsg.Format(_T("[%s] (%ld, %ld, %ld, %ld, %ld)"), sTraceFrom, RecordType, RecordIndex, Field, Subfield, Item);
		TraceMsg(sTraceMsg);
	}

	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->RemoveItem(Field,Subfield,Item);

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

int CIWTransaction::GetImage(int RecordType,int RecordIndex, CStdString& sStorageFormat, long *pLength, const BYTE **ppData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetImage(sStorageFormat, pLength, ppData);

	if (g_bTraceOn)
	{
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::GetImage] Type %d, Index %d, Result %d"), RecordType, RecordIndex, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::GetImageInfo(int RecordType, int RecordIndex, CStdString& sStorageFormat, long *pnLength,
								 long *pnHLL, long *pnVLL, int *pnBitsPerPixel)
{
	int	nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetImageInfo(sStorageFormat, pnLength, pnHLL, pnVLL, pnBitsPerPixel);

	return nRet;
}

int CIWTransaction::SetImage(int RecordType, int RecordIndex, CStdString sInputFormat, long nLength, BYTE *pData,
							 CStdString sStorageFormat, float Compression)
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
		nRet = pRecord->SetImage(sInputFormat, RecordIndex, nLength, pData, sStorageFormat, Compression);

	if (g_bTraceOn)
	{
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::SetImage] Type %d, Index %d, Input format %s, Result %d"), RecordType, RecordIndex, (sInputFormat ? sInputFormat : ""), nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CIWTransaction::ImportImage(int RecordType, int RecordIndex, CStdString sPath, CStdString sStorageFormat, float fCompression,
								CStdString sInputFormat)
// Same as IWSetImage, but from a file.
{
	int			nRet;
	long		nLength;
	BYTE		*pData = NULL;
	CStdString	sExt;
	FILE		*f;

	IWS_BEGIN_EXCEPTION_METHOD("CIWTransaction::ImportImage")

	f = _tfopen(sPath, _T("rb"));
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		nLength = ftell(f);
		fseek(f, 0, SEEK_SET);

		pData = new BYTE[nLength];

		if (pData)
		{
			IWS_BEGIN_CATCHEXCEPTION_BLOCK()

			if (fread(pData, 1, nLength, f) == (unsigned long)nLength)
				nRet = IW_SUCCESS;

			IWS_END_CATCHEXCEPTION_BLOCK()
		}		
		else
			nRet = IW_ERR_OUT_OF_MEMORY;

		IWS_BEGIN_CATCHEXCEPTION_BLOCK()
		fclose(f);
		IWS_END_CATCHEXCEPTION_BLOCK()

		// If sInputFormat is empty, we determine the input format via the file extension
		if (sInputFormat.IsEmpty())
		{
			sExt = sPath.Right(sPath.GetLength() - sPath.ReverseFind('.') - 1);
		}
		else
		{
			sExt = sInputFormat;
		}

		if (nRet == NO_ERROR)
			nRet = SetImage(RecordType, RecordIndex, sExt, nLength, pData, sStorageFormat, fCompression);

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

int CIWTransaction::ExportImage(int nRecordType, int nRecordIndex, CStdString sPath, CStdString sOutputFormat)
// Currently only supports RAW to BMP, since NistImageExport uses this functionality, 
// but eventually we could easily support all format combinations using the existing
// XXXtoYYY functions.
{
	int			nRet = IW_ERR_RECORD_NOT_FOUND;
	CStdString  sFmt;
	const BYTE	*pData;
	long		lLength;
	long		lW;
	long		lH;
	long		lBpp;
	long		lLengthNew;
	double		dPPMM;	// pixels per millimeter
	long		lDPI;
	BYTE		*pNew = NULL;
	FILE		*f;

	CNISTRecord *pRecord = GetRecord(nRecordType, nRecordIndex);
	if (!pRecord) goto done;

	nRet = pRecord->GetImageInfo(sFmt, &lLength, &lW, &lH, (int*)&lBpp);
	if (nRet != IW_SUCCESS) goto done;

	if (sFmt == _T("raw") && sOutputFormat == _T("bmp"))
	{
		nRet = pRecord->GetImage(sFmt, &lLength, &pData);
		if (nRet != IW_SUCCESS) goto done;

		nRet = pRecord->GetImageResolution(&dPPMM);
		if (nRet != IW_SUCCESS) goto done;
		lDPI = (long)(dPPMM * 25.4);

		nRet = RAWtoBMP(lW, lH, lDPI, lBpp, (BYTE*)pData, &pNew, &lLengthNew);
		if (nRet != IW_SUCCESS) goto done;

		f = _tfopen(sPath, _T("wb"));
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

int CIWTransaction::GetRecordLengths()
{
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;
	int nRecordLengths = 0;

	for (int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
	{
		pRec = m_RecordAry.at(i);

		if (pRec)
		{
			nRecordLengths += pRec->GetRecordLen();
		}
	}

	return nRecordLengths; 
}

int CIWTransaction::Write(CStdString sPath)
{
	int nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;
	FILE *f = NULL;

	SetRecordLengths();

	f = _tfopen(sPath, _T("wb"));
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
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::WriteBinary] Write record FAILED. File %s, result %d"), sPath, nRet);
		LogFile(sTraceMsg);
	}

	if (f != NULL) fclose(f);

	return nRet;
}

int CIWTransaction::WriteMem(BYTE** ppBuffer, int *pSize)
{
	int nSize = m_RecordAry.size();
	int nCurrentSize = 0;
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;

	SetRecordLengths();
	int nRecordLengths = GetRecordLengths();
	*ppBuffer = new BYTE[nRecordLengths];

	if (ppBuffer != NULL)
	{
		for (int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
		{
			pRec = m_RecordAry.at(i);

			if (pRec)
			{
				if (CNISTRecord::IsBinaryType(pRec->GetRecordType()))
				{
					nRet = pRec->WriteBinary( ppBuffer, &nCurrentSize);
				}
				else
				{
					nRet = pRec->Write((TCHAR **)ppBuffer, &nCurrentSize);
				}
			}
		}
	}
	else
	{
		nRet = IW_ERR_NULL_POINTER;
	}

	if (nRet != IW_SUCCESS)
	{
		CStdString sTraceFrom("CIWTransaction::WriteBinary");
		CStdString sTraceMsg;

		sTraceMsg.Format(_T(" Write record to memory FAILED. Result %d"), nRet);
		LogFile(sTraceMsg);
	}
	else
	{
		*pSize = nRecordLengths;
	}

	return nRet;
}


int CIWTransaction::WriteXML(CStdString sPath, bool bValidate)
{
	int				nRet = IW_SUCCESS;
	FILE			*f;
	BYTE*			pXML;
	long			lLengthXML;
	CStdString		sErr;

	FreeErrors();

	SetRecordLengths();

	// Not yet supported
	//
	//nRet = GetXML(&pXML, bValidate, &lLengthXML, &pErr);
	//

	return IW_ERR_WRITING_FILE;

	if (nRet != IW_SUCCESS)
	{
		AddError(sErr, 0);
		return nRet;
	}

	f = _tfopen(sPath, _T("wb"));
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
		CStdString sTraceMsg;
		sTraceMsg.Format(_T("[CIWTransaction::WriteXML] WriteXML FAILED. File %s, result %d"), sPath, nRet);
		LogFile(sTraceMsg);
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

bool CIWTransaction::IsVerificationLoaded() 
{ 
	bool bRet = false; 
	
	if (m_pVerification != NULL) 
		bRet = m_pVerification->IsLoaded(); 
	
	return bRet; 
} 

int CIWTransaction::Get(CStdString sMnemonic, CStdString& sData, int nIndex, int nRecordIndex)
{
	int nRet = IW_SUCCESS;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(sMnemonic, nIndex, nRecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = FindItem(recordType, recordIndex, field, subField, item, sData);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

int CIWTransaction::Set(CStdString sMnemonic, CStdString sData, int nStartIndex, int nRecordIndex)
{
	int nRet = IW_SUCCESS;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(sMnemonic, nStartIndex, nRecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = SetItem(sData, recordType, recordIndex, field, subField, item);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

int CIWTransaction::Remove(CStdString sMnemonic, int nRecordIndex, int nIndex)
{
	int nRet = IW_SUCCESS;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(sMnemonic, nIndex, nRecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = RemoveItem(recordType, recordIndex, field, subField, item);
	}
	else
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;

	return nRet;
}

int CIWTransaction::Occurrences(CStdString sMnemonic, int *pnOccurrences, int nRecordIndex)
{
	int nRet = IW_SUCCESS;

	*pnOccurrences = 0;

	if (IsVerificationLoaded())
	{	
		int recordType = 0;
		int recordIndex = 0;
		int field = 0;
		int subField = 0;
		int item = 0;

		if ((nRet = m_pVerification->GetMNULocation(sMnemonic, 1/*StartIndex*/, nRecordIndex, &recordType, &recordIndex, &field, &subField, &item)) == IW_SUCCESS)
			nRet = GetNumItems(recordType, recordIndex, field, subField, pnOccurrences);
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

void CIWTransaction::AddError(CStdString sErr, int nCode)
{
	CNISTErr err;

	err.m_nCode = nCode;
	err.m_sErr = sErr;

	m_ErrAry.push_back(err);
}

int CIWTransaction::GetErrorCount()
{
	return m_ErrAry.size();
}

int CIWTransaction::GetError(int Index, int* Code, CStdString& sDesc)
{
	int nRet = IW_ERR_INDEX_OUT_OF_RANGE;

	if (Index >= 0 && Index < (int)m_ErrAry.size())
	{
		CNISTErr *pErr = &m_ErrAry.at(Index);
		*Code = pErr->m_nCode;
		sDesc = pErr->m_sErr;
		nRet = IW_SUCCESS;
	}

	return nRet;
}
