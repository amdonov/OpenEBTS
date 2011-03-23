#include "Includes.h"
#include "OpenEBTSErrors.h"
#include "TransactionDef.h"
#include "IWTransaction.h"
#include "Common.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "IWVerification.h"
#include "OpenEBTS.h"
#include "RuleObj.h"


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

	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;

	for (unsigned int i = 0; i < nSize; i++)
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

		m_nIDCSequence = 0; // reset the IDC sequence

#ifdef UNICODE
		// In the UNICODE version we specify that we will be writing UTF-8 chars into fields
		Set(L"T1_DCS_CSI", L"003", 1, 1);	// Character Set Index is "003" for UTF-8
		Set(L"T1_DCS_CSN", L"UTF-8", 1, 1);	// Character Set Name
#endif
	}

	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSNEW, nRet);
		LogMessage(sLogMessage);
	}

	return nRet;
}

int CIWTransaction::ReadTransactionFile(CStdStringPath sFilePath)
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

	f = _tfopenpath(sFilePath, _TPATH("rb"));

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

	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSREAD, sFilePath, nRet);
		LogMessage(sLogMessage);
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

	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSREADMEM, nRet);
		LogMessage(sLogMessage);
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
		m_dNativeResolutionPPMM = _tcstod(sData, NULL);

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

			if (g_bLogToFile)
			{
				CStdString sLogMessage;
				sLogMessage.Format(IDS_LOGTRANSRECORDSINFILE, nRecords);
				LogMessage(sLogMessage);
			}

			for (nSubField=1;nSubField <= nRecords && nRet == IW_SUCCESS;nSubField++)
			{
				// first get the expected record type
				if (FindItem(RECORD_TYPE1, 1, TYPE1_CNT, nSubField, ITEM_RECORDTYPE, sData) == IW_SUCCESS)
				{
					nRecordType = (int)_tcstol(sData, NULL, 10);

					// then get the record index
					if (FindItem(RECORD_TYPE1, 1, TYPE1_CNT, nSubField, ITEM_RECORDINDEX, sData) == IW_SUCCESS)
					{
						m_nIDCDigits = sData.GetLength();
						nRecordIndex = (int)_tcstol(sData, NULL, 10);

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

									pRec->SetRecordOffset((int)(pRecordData - m_pTransactionData));
									nRet = pRec->ReadBinaryRecord(pRecordData, nRecordType);

									// Types 3 to 8 need to know the Native Scanning Resolution
									pRec->SetNativeScanningResolution(m_dNativeResolutionPPMM);

									// set pointer to next record
									pRecordData += pRec->GetLogicalRecordLen();

									if (nRet == IW_SUCCESS)
										m_RecordAry.push_back(pRec);
									else
										delete pRec;

									if (g_bLogToFile)
									{
										CStdString sLogMessage;
										sLogMessage.Format(IDS_LOGTRANSREADRECORDTYPE, nRecordType, nRet);
										LogMessage(sLogMessage);
									}
									if (nRet != IW_SUCCESS)
									{
										CStdString sLogMessage;
										sLogMessage.Format(IDS_LOGTRANSREADRECORDFAILED, m_sFilePath, nRecordType, nRet);
										LogMessage(sLogMessage);
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

									if (g_bLogToFile)
									{
										CStdString sLogMessage;
										sLogMessage.Format(IDS_LOGTRANSREADRECORDTYPE, nRecordType, nRet);
										LogMessage(sLogMessage);
									}
									if (nRet != IW_SUCCESS)
									{
										CStdString sLogMessage;
										sLogMessage.Format(IDS_LOGTRANSREADRECORDFAILED, m_sFilePath, nRecordType, nRet);
										LogMessage(sLogMessage);
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
	CNISTRecord *pRec = 0;

	DebugOutRecords(_T("AddRecord start"));

	pRec = new CNISTRecord;

	if (pRec)
	{
		int nIDC = 0;
		CStdString sIDC;
		int recordIndex = 0;

		// According to the NIST spec the IDC is a sequentially incrementing value,
		// starting with the type 2 record which has a IDC of '00'. 
		// Just like Aware, record deletes will blow a hole in the seq and 
		// it is the responsiblity of the user to reorder the IDC sequentially. 
		// Similarly, if 2 or more records share a relationship it is the users 
		// responsiblity to ensure they share the same IDC value.

		if (RecordType > 1)
		{
			nIDC = m_nIDCSequence;
			m_nIDCSequence++;
		}

		// return the record index within this record type
		GetRecordTypeMaxIndex(RecordType, &recordIndex);

		recordIndex++;
		sIDC.Format(_T("%02d"), nIDC);

		pRec->InitializeNewRecord(RecordType);

		// by default set the IDC field to the next ID, application can modify
		pRec->SetItem(sIDC, REC_TAG_IDC, 1, 1);
		m_RecordAry.push_back(pRec);

		*pRecordIndex = recordIndex;

		Type1AddRecordIDC(pRec, RecordType, nIDC);
	}
/*

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
*/
	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSADDRECORD, RecordType, nRet);
		LogMessage(sLogMessage);
	}

	DebugOutRecords(_T("AddRecord end"));

	return nRet;
}

void CIWTransaction::DebugOutRecords(CStdString sContext)
{
#ifdef _DEBUG
	if (!g_bLogToFile) return;

	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	CNISTRecord *pType1Rec = GetRecord(RECORD_TYPE1, 1);
	CStdString sData;
	CStdString sData1;
	CStdString sData2;
	CStdString sFoo;

	sFoo.Format(IDS_LOGTRANSDEBUGSTART, sContext);
	LogMessage(sFoo);

	if (pType1Rec->FindItem(TYPE1_CNT, 1, 2, sData) == IW_SUCCESS)
	{
		int nCount = (int)_tcstol(sData, NULL, 10);

		sFoo.Format(IDS_LOGTRANSDEBUGCONTENTS1, nCount);
		LogMessage(sFoo);

		for (int i=2; i<=nCount+1; i++)
		{
			if (pType1Rec->FindItem(TYPE1_CNT, i, 1, sData1) != IW_SUCCESS)
			{
				sFoo = IDS_LOGTRANSDEBUGERROR;
				LogMessage(sFoo);
				continue;
			}
			if (pType1Rec->FindItem(TYPE1_CNT, i, 2, sData2) != IW_SUCCESS)
			{
				sFoo = IDS_LOGTRANSDEBUGERROR;
				LogMessage(sFoo);
				continue;
			}

			sFoo.Format(IDS_LOGTRANSDEBUGCONTENTS2, i, sData1, sData2);
			LogMessage(sFoo);
		}
	}

	for (unsigned int i = 0; i < nSize; i++)
	{
		pRec = m_RecordAry.at(i);
		sFoo.Format(IDS_LOGTRANSDEBUGRECORD, i, pRec->GetRecordType());
		LogMessage(sFoo);
	}
	sFoo = IDS_LOGTRANSDEBUGEND;
	LogMessage(sFoo);
#endif
}

int CIWTransaction::DeleteRecord(int RecordType, int RecordIndex)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	CNISTRecord *pRecTmp = 0;

	pRec = GetRecord(RecordType, RecordIndex);

	for (unsigned int i = 0; i < nSize; i++)
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
		CStdString sData;
		CStdString sCount;
		CStdString sIDC;

		if (pRec->FindItem(TYPE1_CNT, 1, 2, sData) == IW_SUCCESS)
		{
			int nCount = (int)_tcstol(sData, NULL, 10);

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
		int nRecPos = 0;
		size_t nSize = m_RecordAry.size();
		CNISTRecord *pRec = 0;

		for (unsigned int i = 0; i < nSize; i++)
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
		size_t nSize = m_RecordAry.size();
		CNISTRecord *pRec = 0;
		CStdString sCount;

		for (unsigned int i = 0; i < nSize; i++)
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
				nCount = (int)_tcstol(sData, NULL, 10);

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
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRet = 0;
	CNISTRecord *pRec = 0;
	int nIndex = 0;

	for (unsigned int i = 0; i < nSize; i++)
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

int CIWTransaction::GetNumFields(int RecordType, int RecordIndex, int *pCount)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetNumFields(pCount);

	return nRet;
}

int CIWTransaction::GetNextField(int RecordType, int RecordIndex, int Field, int *pNextField)
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
			int nIDC = (int)_tcstol(sData, NULL, 10);
			sData.Format(_T("%02d"), nIDC);
		}

		if ((nRet = pRecord->SetItem(sData, Field, Subfield, Item)) == IW_SUCCESS)
		{
			if (Field == REC_TAG_IDC && RecordType != RECORD_TYPE1)
			{
				int nIDC = (int)_tcstol(sData, NULL, 10);
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

	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSREMOVEITEM, RecordType, RecordIndex, Field, Subfield, Item);
		LogMessage(sLogMessage);
	}

	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->RemoveItem(Field,Subfield,Item);

	return nRet;
}

int CIWTransaction::GetRecordTypeCount(int RecordType, int *pRecordTypeCount)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nCount = 0;

	*pRecordTypeCount = 0;

	for (unsigned int i = 0; i < nSize; i++)
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
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nIndex = 0;

	for (unsigned int i = 0; i < nSize; i++)
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
	*pRecords = (int)m_RecordAry.size();

	return IW_SUCCESS;
}

int CIWTransaction::GetImage(int RecordType,int RecordIndex, CStdString& sStorageFormat, int *pLength, const BYTE **ppData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetImage(sStorageFormat, pLength, ppData);

	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSGETIMAGE, RecordType, RecordIndex, nRet);
		LogMessage(sLogMessage);
	}

	return nRet;
}

int CIWTransaction::GetImageInfo(int RecordType, int RecordIndex, CStdString& sStorageFormat, int *pnLength,
								 int *pnHLL, int *pnVLL, int *pnBitsPerPixel)
{
	int	nRet = IW_ERR_RECORD_NOT_FOUND;
	CNISTRecord *pRecord = GetRecord(RecordType, RecordIndex);

	if (pRecord)
		nRet = pRecord->GetImageInfo(sStorageFormat, pnLength, pnHLL, pnVLL, pnBitsPerPixel);

	return nRet;
}

int CIWTransaction::SetImage(int RecordType, int RecordIndex, CStdString sInputFormat, int nLength, BYTE *pData,
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

	if (g_bLogToFile)
	{
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSSETIMAGE, RecordType, RecordIndex, (sInputFormat ? sInputFormat : ""), nRet);
		LogMessage(sLogMessage);
	}

	return nRet;
}

int CIWTransaction::ImportImage(int RecordType, int RecordIndex, CStdStringPath sPath, CStdString sStorageFormat, float fCompression,
								CStdString sInputFormat)
// Same as IWSetImage, but from a file.
{
	int			nRet = IW_ERR_OPENING_FILE_FOR_READING;
	long		nLength;
	BYTE		*pData = NULL;
	CStdString	sExt;
	FILE		*f;

	IWS_BEGIN_EXCEPTION_METHOD("CIWTransaction::ImportImage")

	f = _tfopenpath(sPath, _TPATH("rb"));
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

		if (nRet == IW_SUCCESS)
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

int CIWTransaction::ExportImage(int nRecordType, int nRecordIndex, CStdStringPath sPath, CStdString sOutputFormat)
// Currently only supports RAW to BMP, since NistImageExport uses this functionality, 
// but eventually we could easily support all format combinations using the existing
// XXXtoYYY functions.
{
	int			nRet = IW_ERR_RECORD_NOT_FOUND;
	CStdString  sFmt;
	const BYTE	*pData;
	int			nLength;
	int			w;
	int			h;
	int			bpp;
	int			nLengthNew;
	double		dPPMM;	// pixels per millimeter
	int			DPI;
	BYTE		*pNew = NULL;
	FILE		*f;

	CNISTRecord *pRecord = GetRecord(nRecordType, nRecordIndex);
	if (!pRecord) goto done;

	nRet = pRecord->GetImageInfo(sFmt, &nLength, &w, &h, (int*)&bpp);
	if (nRet != IW_SUCCESS) goto done;

	if (sFmt == _T("raw") && sOutputFormat == _T("bmp"))
	{
		nRet = pRecord->GetImage(sFmt, &nLength, &pData);
		if (nRet != IW_SUCCESS) goto done;

		nRet = pRecord->GetImageResolution(&dPPMM);
		if (nRet != IW_SUCCESS) goto done;
		DPI = (int)(dPPMM * 25.4);

		nRet = RAWtoBMP(w, h, DPI, bpp, (BYTE*)pData, &pNew, &nLengthNew);

		if (nRet != IW_SUCCESS) goto done;

		f = _tfopenpath(sPath, _TPATH("wb"));
		if (f != NULL)
		{
			fwrite(pNew, 1, nLengthNew, f);
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
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;

	for (unsigned int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
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
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;
	int nRecordLengths = 0;

	for (unsigned int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
	{
		pRec = m_RecordAry.at(i);

		if (pRec)
		{
			nRecordLengths += pRec->GetRecordLen();
		}
	}

	return nRecordLengths; 
}

int CIWTransaction::Write(CStdStringPath sPath)
{
	size_t nSize = m_RecordAry.size();
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;
	FILE *f = NULL;

	SetRecordLengths();

	f = _tfopenpath(sPath, _TPATH("wb"));
	if (f != NULL)
	{
		for (unsigned int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
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
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSWRITEBINARY, sPath, nRet);
		LogMessage(sLogMessage);
	}

	if (f != NULL) fclose(f);

	return nRet;
}

int CIWTransaction::WriteMem(BYTE** ppBuffer, int *pSize)
{
	size_t nSize = m_RecordAry.size();
	int nCurrentSize = 0;
	CNISTRecord *pRec = 0;
	int nRet = IW_SUCCESS;

	SetRecordLengths();
	int nRecordLengths = GetRecordLengths();
	*ppBuffer = new BYTE[nRecordLengths];

	if (ppBuffer != NULL)
	{
		for (unsigned int i = 0; i < nSize && nRet == IW_SUCCESS; i++)
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
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSWRITEMEM, nRet);
		LogMessage(sLogMessage);
	}
	else
	{
		*pSize = nRecordLengths;
	}

	return nRet;
}


int CIWTransaction::WriteXML(CStdStringPath sPath, bool bValidate)
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

	f = _tfopenpath(sPath, _TPATH("wb"));
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
		CStdString sLogMessage;
		sLogMessage.Format(IDS_LOGTRANSWRITEXML, sPath, nRet);
		LogMessage(sLogMessage);
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
		{
			nRet = SetItem(sData, recordType, recordIndex, field, subField, item);

			// Add any non-existent items as blank string, if they have the same field number. This will ensure
			// that multi-item fields will include the non-provided items as empty items which will be used by
			// any NIST file reader as a spacer between the multiple items. We only bother doing this for items
			// up to and not-including the final item, hence item must be greater than 1, otherwise no spacing
			// is required.
			if (nRet == IW_SUCCESS && item > 1)
			{
				const std::vector<CRuleObj>* parr = m_pVerification->GetRuleArray();
				CRuleObj *pRule;
				int recordType2 = 0;
				int recordIndex2 = 0;
				int field2 = 0;
				int subField2 = 0;
				int item2 = 0;

				// Loop throught them all and get the location indices, and compare with 'field' (and 'subfield')
				for (unsigned int i = 0; i < parr->size(); i++)
				{
					pRule = (CRuleObj*)&parr->at(i);

					if (pRule->GetLocation(nStartIndex, nRecordIndex, &recordType2, &recordIndex2, &field2, &subField2, &item2))
					{
						if (recordType2 == recordType && recordIndex2 == recordIndex &&
							field2 == field && subField2 == subField) // we have a mnemonic for one of the sibling items
						{
							// Only set to a blank string items *before* the one this function was asked to set
							if (item2 < item)
							{
								// And of course, only set it if it doesn't exist
								if (FindItem(recordType2, recordIndex2, field2, subField2, item2, sData) == IW_ERR_RECORD_NOT_FOUND)
								{
									nRet = SetItem(_T(""), recordType2, recordIndex2, field2, subField2, item2);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		nRet = IW_ERR_VERIFICATION_NOT_LOADED;
	}

	return nRet;
}

int CIWTransaction::Remove(CStdString sMnemonic, int nIndex, int nRecordIndex)
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
	return (int)m_ErrAry.size();
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
