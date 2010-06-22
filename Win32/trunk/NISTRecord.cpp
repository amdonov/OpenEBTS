
#include "stdafx.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "OpenEBTS.h"


CNISTRecord::CNISTRecord()
{
	m_nRecordType = -1;
	m_dNativeResolutionPPMM = 0.0;
}

CNISTRecord::~CNISTRecord()
{
	int nSize = m_FieldList.size();
	CNISTField *pField;

	for (int i = 0; i < nSize; i++)
	{
		pField = m_FieldList.at(i);
		if (pField) delete pField;
	}
	m_FieldList.clear();
}

void CNISTRecord::InitializeNewRecord(int nRecordType)
{
	CNISTField *pField = new CNISTField;
	CStdString	sData;
	int nRet;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::InitializeNewRecord");
		CStdString sTraceMsg;

		sTraceMsg.Format(_T("[%s] RecordType %d"), sTraceFrom, nRecordType);
		TraceMsg(sTraceMsg);
	}

	m_nRecordType = nRecordType;

	// 2 fields must appear in every record, and they
	// must be the first 2 fields in the record
	// by default always add the .001 and .002 records

	sData = _T("0");	// Set length to 0
	if ((nRet = pField->AddItem(nRecordType, REC_TAG_LEN, 1/*Subfield*/, 1/*Item*/, sData)) == IW_SUCCESS)
	{
		AddField(pField);

		pField = new CNISTField;

		if (nRecordType == RECORD_TYPE1)
			sData = _T("0400");		// default version
		else
			sData = _T("00");		// default IDC value, should be correctly set by calling application?

		if ((nRet = pField->AddItem(nRecordType, (nRecordType == RECORD_TYPE1 ? REC_TAG_VER : REC_TAG_IDC), 1/*Subfield*/, 1/*Item*/, sData)) == IW_SUCCESS)
			AddField(pField);

		if (nRecordType == RECORD_TYPE1) // initialize an empty contents field
		{
			pField = new CNISTField;
			sData = _T("1");
			if ((nRet = pField->AddItem(nRecordType, TYPE1_CNT, 1/*Subfield*/, 1/*Item*/, sData)) == IW_SUCCESS)
			{
				sData = _T("0"); 
				if ((nRet = pField->AddItem(nRecordType, TYPE1_CNT, 1/*Subfield*/, 2/*Item*/, sData)) == IW_SUCCESS)
					AddField(pField);
			}
		}
	}
}

void CNISTRecord::SetNativeScanningResolution(double dNativeResolutionPPMM)
{
	m_dNativeResolutionPPMM = dNativeResolutionPPMM;
}

int CNISTRecord::ReadLogicalRecordLen(BYTE* pTransactionData, int nRecordType, int nRecordIndex)
{
	char szTemp[120];
	char *pTemp;
	int nRet = 0;

	// grab the first few bytes, the len field is always first and should be in here
	memcpy(szTemp, pTransactionData, sizeof(szTemp));

	pTemp = IWStrTok(szTemp, CHAR_PERIOD);

	if (pTemp)
	{
		if (atoi(pTemp) == nRecordType)
		{
			pTemp = IWStrTok(NULL, CHAR_COLON);

			if (pTemp)
			{
				if (atoi(pTemp) == REC_TAG_LEN)
				{
					pTemp = IWStrTok(NULL, CHAR_GS);

					if (pTemp)
						nRet = atoi(pTemp);
				}
			}
		}
	}

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::ReadLogicalRecordLen");
		CStdString sTraceMsg;
		
		sTraceMsg.Format(_T("[%s] RecordType %d, Index %d, Length %ld"), sTraceFrom, nRecordType, nRecordIndex, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CNISTRecord::ReadRecord(BYTE* pTransactionData, int nRecordType)
{
	int nRet = IW_SUCCESS;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::ReadRecord");
		CStdString sTraceMsg;
		
		sTraceMsg.Format(_T("[%s] RecordType %d"), sTraceFrom, nRecordType);
		TraceMsg(sTraceMsg);
	}

	m_nRecordType = nRecordType;
	m_bGetImage = false;

	m_nRecordLen = ReadLogicalRecordLen(pTransactionData, nRecordType); // get size of this record

	if (m_nRecordLen > 0)
	{
		BYTE* pRecord = new BYTE[m_nRecordLen];

		if (pRecord)
		{
			int			nField;
			BYTE		*pFieldData;
			BYTE		*pContent = NULL;
			CStdString	sErr;
			CNISTField	*pField;
			BYTE		*pTemp;
			bool		bEndofRecord = false;
			int			nFieldPos = 1;

			memcpy(pRecord, pTransactionData, m_nRecordLen * sizeof(BYTE));

			pTemp = (BYTE*)IWStrTok((char*)pRecord, CHAR_PERIOD);
	
			while (pTemp && !bEndofRecord)
			{
				nField = atoi((char*)pTemp);

				if (nField == nRecordType) // make sure we're in sync
				{
					pTemp = (BYTE*)IWStrTok(NULL, CHAR_COLON, &bEndofRecord); // get the field

					if (pTemp)
					{
						nField = atoi((char*)pTemp);

						if (IsDATField(m_nRecordType, nField))
						{
							// take advantage of the fact that I know
							// the pointer is sitting at the beginning of the image bytes
							m_bGetImage = true;
							pFieldData = (BYTE*)IWStrTok(NULL, CHAR_GS, &bEndofRecord); // get the field	data	
							m_bGetImage = false;
							// Explicitly set bEndofRecord because we know that DAT fields are
							// always the last fields
							bEndofRecord = true;
						}
						else
							pFieldData = (BYTE*)IWStrTok(NULL, CHAR_GS, &bEndofRecord); // get the field	data	
						
						pField = new CNISTField;
						pField->m_nRecordType = nRecordType;
						pField->m_nField = nField;

						if (nField == REC_TAG_LEN || nField == REC_TAG_IDC)
						// these don't need to be handled special 
						// but it helps for debugging
						{
							// LEN and IDC are always in ASCII, no conversion necessary here
							CStdString sFieldData((char*)pFieldData);
							pField->SetSubField(1, 1, sFieldData);
						}
						else if (IsDATField(m_nRecordType, nField))
						{
							if (pFieldData)
							{
								int nOffset = pFieldData - pRecord;
								BYTE *pImage = (BYTE*)pTransactionData + nOffset;
								int nImageLen = m_nRecordLen - nOffset - 1;

								pField->SetImageData(pImage, nImageLen);
							}
						}
						else
						{
							if (strchr((char*)pFieldData, CHAR_US) || strchr((char*)pFieldData, CHAR_RS))
							{
								AddSubItems(pField, (char*)pFieldData);
							}
							else
							{
								// Field may be in UTF-8, convert if using the UNICODE version of OpenEBTS
								CStdString sFieldData;
#ifdef UNICODE
								// Field data may be in UTF-8, let's get it properly into wide char land
								wchar_t *wNew;

								if (!UTF8toUCS2((char*)pFieldData, &wNew))
								{
									return IW_ERR_READING_FILE;	// Error deconding UTF-8
								}
								sFieldData = wNew;
								delete wNew;
#else
								sFieldData = (char*)pFieldData;
#endif
								pField->SetSubField(1, 1, sFieldData);
							}
						}
						AddField(pField);
					}
				}
				// get next field
				if (!bEndofRecord)
				{
					pTemp = (BYTE*)IWStrTok(NULL, CHAR_PERIOD, &bEndofRecord);
				}
			}
			delete [] pRecord;
		}
	}
	else
		nRet = IW_ERR_READING_FILE;

	return nRet;
}

/*********************************************************/
/* IWStrTok is NOT thread safe!
/*********************************************************/

char* CNISTRecord::IWStrTok(char *pInStr, char cDelim, bool *pbEndofRecord)
{
	static char *pCurPos = 0;
	static char *pString = 0;
	static char *pEndString = 0;
	static int nCurPos = 0;
	char *pTemp;
	char *pRet = 0;

	if (m_bGetImage)
		return pCurPos; 

	if (pInStr)
	{
		pCurPos = pString = pInStr;
		nCurPos = 0;
		int nLen = strlen(pInStr);
		pEndString = pString+nLen;
	}
	
	while (pCurPos <= pEndString)
	{
		if (*pCurPos == cDelim || *pCurPos == CHAR_FS)
		{
			if (*pCurPos == CHAR_FS && pbEndofRecord)
				*pbEndofRecord = true;

			if (pCurPos > pString)
			{
				pTemp = pCurPos-1;
				
				*pCurPos = '\0'; // break the string at the delimiter
				pCurPos++; // move off null
				
				// move to begin of previous field
				while (pTemp > pString && *pTemp != '\0')
					pTemp--;

				// move off null
				if (*pTemp == '\0')
					pTemp++;
					
				pRet = pTemp;
			}
			else
			{
				*pCurPos = '\0'; // return empty string
				pRet = pCurPos;
			}

			break;
		}
		else
			pCurPos++;
	}
	
	return pRet;
}

/*********************************************************/
/* Binary Record methods
/*********************************************************/

int CNISTRecord::ReadBinaryRecord(BYTE* pTransactionData, int nRecordType)
{
	int nRet = IW_SUCCESS;
	BYTE bIDC;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::ReadBinaryRecord");
		CStdString sTraceMsg;
		
		sTraceMsg.Format(_T("[%s] RecordType %d"), sTraceFrom, nRecordType);
		TraceMsg(sTraceMsg);
	}

	m_nRecordType = nRecordType;
	m_bGetImage = false;

	if (nRecordType != RECORD_TYPE8)
	{
		FINGERPRINT_HEADER *pHdr = (FINGERPRINT_HEADER*)pTransactionData;
		bIDC = pHdr->bIDC;
	}
	else
	{
		SIGNATURE_HEADER *pHdr = (SIGNATURE_HEADER*)pTransactionData;
		bIDC = pHdr->bIDC;
	}

	CStdString sFieldData;

	IWS_BEGIN_EXCEPTION_METHOD("CNISTRecord::ReadBinaryRecord")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CNISTField *pField = new CNISTField(nRecordType, REC_TAG_LEN);

	// the first 2 fields are the same for all binary records, LEN and IDC
	sFieldData.Format(_T("%d"), GetDecimalValue(pTransactionData, 4));
	pField->SetSubField(1, REC_TAG_LEN, sFieldData);
	AddField(pField);
	m_nRecordLen = _ttoi(sFieldData);

	pField = new CNISTField(nRecordType, REC_TAG_IDC);
	sFieldData.Format(_T("%d"), bIDC);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	if (CNISTRecord::IsBinaryFingerType(nRecordType))
	{
		nRet = GetFingerprintInfo(nRecordType, pTransactionData);
		pField = new CNISTField(nRecordType, TYPE4_DAT); //9 in all cases
	}
	else if (nRecordType == RECORD_TYPE8)
	{
		nRet = GetSignatureInfo(pTransactionData);
		pField = new CNISTField(nRecordType, TYPE8_DAT); //8 in this case
	}
	pField->SetImageData((BYTE*)pTransactionData, m_nRecordLen);
	AddField(pField);

	IWS_END_CATCHEXCEPTION_BLOCK()

	// If we wanted to write out NIST records with fingerprint info we
	// would need to store the fingerprint data as well.
	// We would need to be careful to write fields correctly especially
	// the hex fields, 1, 4, 6 and 7. But at the moment we only care
	// about reading NIST files that contain fingerprint info.

	return nRet;
}

int CNISTRecord::GetFingerprintInfo(int nRecordType, BYTE* pTransactionData)
{
	int nRet = IW_SUCCESS;
	FINGERPRINT_HEADER *pFPrintHdr = (FINGERPRINT_HEADER *)pTransactionData;
	CNISTField *pField;
	CStdString sFieldData;

	pField = new CNISTField(nRecordType, 3);	// Impression Type
	sFieldData.Format(_T("%d"), pFPrintHdr->bImp);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 4);	// Finger position
	sFieldData.Format(_T("%d"), GetDecimalValue((BYTE*)pFPrintHdr->bFGP, 1));
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 5);	// Scanning resolution
	sFieldData.Format(_T("%d"), pFPrintHdr->bISR);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 6);	// Horiz line length
	sFieldData.Format(_T("%d"), GetDecimalValue((BYTE*)pFPrintHdr->bHLL, 2));
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 7);	// Vert line length
	sFieldData.Format(_T("%d"), GetDecimalValue((BYTE*)pFPrintHdr->bVLL, 2));
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 8);	// Compression algorithm
	sFieldData.Format(_T("%d"), pFPrintHdr->bComp);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	return nRet;
}

int CNISTRecord::GetSignatureInfo(BYTE* pTransactionData)
{
	int nRet = IW_SUCCESS;
	SIGNATURE_HEADER *pSig = (SIGNATURE_HEADER *)pTransactionData;
	CNISTField *pField;
	int nRecordType = RECORD_TYPE8;
	CStdString sFieldData;

	pField = new CNISTField(nRecordType, 3);	// Signature type
	sFieldData.Format(_T("%d"), pSig->bSIG);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 4);	// Signature representation type
	sFieldData.Format(_T("%d"), pSig->bSRT);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 5);	// Scanning resolution
	sFieldData.Format(_T("%d"), pSig->bISR);
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 6);	// Horiz line length
	sFieldData.Format(_T("%d"), GetDecimalValue((BYTE*)pSig->bHLL, 2));
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 7);	// Vert line length
	sFieldData.Format(_T("%d"), GetDecimalValue((BYTE*)pSig->bVLL, 2));
	pField->SetSubField(1, 1, sFieldData);
	AddField(pField);

	return nRet;
}

int CNISTRecord::GetDecimalValue(BYTE* pInStr, int nStrLen)
{
	char szHexStr[20] = { '\0', };
	char szTemp[5];

	for (int i = 0; i < nStrLen; i++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcat_s(szHexStr, sizeof(szHexStr), BytetoHexString(pInStr[i], szTemp));
	}

	return (int)strtol(szHexStr, NULL, 16);
}

char *CNISTRecord::BytetoHexString(BYTE lDecimalValue, char *pszHexString)
{
	char szHex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	
	pszHexString[0] = szHex[lDecimalValue/16];
	pszHexString[1] = szHex[lDecimalValue%16];

	return pszHexString;	
}

/*********************************************************/
/* End Binary Record methods
/*********************************************************/

int CNISTRecord::GetNextField(int nField, int *pNextField)
{
	int nSize = m_FieldList.size();
	CNISTField *pField;
	int nRet = IW_ERR_RECORD_NOT_FOUND;

	*pNextField = 0;

	for (int i = 0; i < nSize; i++)
	{
		pField = m_FieldList.at(i);

		if (pField->m_nField > nField)
		{
			*pNextField = pField->m_nField;
			nRet = IW_SUCCESS;
			break;
		}
	}

	return nRet;
}

int CNISTRecord::GetNumSubfields(int nField, int *pCount)
{
	int nRet = IW_ERR_INDEX_OUT_OF_RANGE;
	CNISTField *pField = GetNISTField(nField);

	*pCount = 0;

	if (pField)
	{
		*pCount = pField->GetNumSubfields();
		nRet = IW_SUCCESS;
	}

	return nRet;
}

CNISTField *CNISTRecord::GetNISTField(int nField)
{
	int nSize = m_FieldList.size();
	CNISTField *pRetField = 0;
	CNISTField *pField = 0;

	for (int i = 0; i < nSize; i++)
	{
		pField = m_FieldList.at(i);

		if (pField->m_nField == nField)
		{
			pRetField = pField;
			break;
		}
	}

	return pRetField;
}

int CNISTRecord::AddSubItems(CNISTField *pNISTField, char *szFieldData)
// Note that this function always operates in the regular ASCII character space
{
	int nRet = IW_SUCCESS;
	char szRS[2];

	szRS[0] = CHAR_RS;
	szRS[1] = '\0';

	if (szFieldData && *szFieldData)
	{
		int nLen = strlen(szFieldData);

		if (nLen)
		{
			char *pTemp;
			char *pTempSubItem;
			char *pTempSub;
			int nSubField = 1;
			int nSubFieldLen;
			int nSubFieldItem = 1;
			int nSubFieldItemLen = 0;
			int i;
			CSubFieldItem *pSubItem;

			pTemp = strtok(szFieldData, szRS);
			while (pTemp)
			{
				pTempSubItem = pTempSub = pTemp;
				
				// get the list of subfield items
				if (strchr(pTempSub, CHAR_US))
				{
					nSubFieldItem = 1;
					nSubFieldLen = strlen(pTempSub);
					for (i = 0; i <= nSubFieldLen; i++)
					{
						// add the subfield items
						if (*pTempSub == CHAR_US || *pTempSub == NULL) 
						{
							pSubItem = new CSubFieldItem;
							pSubItem->m_nSubField = nSubField;
							pSubItem->m_nSubFieldItem = nSubFieldItem++;
							nSubFieldItemLen = pTempSub-pTempSubItem;

							if (nSubFieldItemLen)
							{
								*pTempSub = '\0'; // chop off our string
								pSubItem->m_sData = pTempSubItem;
							}
							pTempSubItem = pTempSub+1; // get past null

							pNISTField->m_SubFieldAry.push_back(pSubItem);
						}
						pTempSub++;
					}
				}
				else // only 1 subfield item
				{
					pSubItem = new CSubFieldItem;
					pSubItem->m_nSubField = nSubField;
					pSubItem->m_nSubFieldItem = 1;
					nSubFieldItemLen = strlen(pTemp);
					if (nSubFieldItemLen)
					{
#ifdef UNICODE
						// Field data may be in UTF-8, let's get it properly into wide char land
						wchar_t *wNew;

						if (!UTF8toUCS2(pTempSubItem, &wNew))
						{
							return IW_ERR_READING_FILE;	// Error deconding UTF-8
						}

						pSubItem->m_sData = wNew;
						delete wNew;
#else
						pSubItem->m_sData = pTempSubItem;
#endif
					}
					pNISTField->m_SubFieldAry.push_back(pSubItem);
				}
				nSubField++;

				if (pTempSub >= szFieldData + nLen)
					pTemp = NULL;
				else
					pTemp = strtok(NULL, szRS);
			}
		}
	}

	return nRet;
}

int CNISTRecord::GetNumItems(int Field, int Subfield, int* Count)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	CNISTField *pField = GetNISTField(Field);

	if (pField)
		nRet = pField->GetNumItems(Subfield, Count);

	return nRet;
}

void CNISTRecord::AddField(CNISTField *pField)
{
	int nSize = m_FieldList.size();
	CNISTField *pTemp = 0;
	CNISTField *pInsertAfter = 0;
	int nPos = 0;

	for (int i = 0; i < nSize; i++)
	{
		pTemp = m_FieldList.at(i);

		if (pTemp && pTemp->m_nField > pField->m_nField)
		{
			nPos = i;
			break;
		}
	}

	if (!nPos)
		m_FieldList.push_back(pField);
	else
		m_FieldList.insert(m_FieldList.begin() + nPos, pField);
}

int CNISTRecord::RemoveItem(int Field, int Subfield, int Item)
{
	int nRet = IW_SUCCESS;

	int nSize = m_FieldList.size();
	int nPos = -1;
	CNISTField *pTemp;

	// KAS 6/22/10
	// For now we simply remove the entire field and
	// ignore the subfield and item values
	for (int i = 0; i < nSize; i++)
	{
		pTemp = m_FieldList.at(i);

		if (pTemp && pTemp->m_nField == Field)
		{
			nPos = i;
			break;
		}
	}

	if (nPos > -1 && nPos < nSize)
		m_FieldList.erase(m_FieldList.begin()+nPos);

	return nRet;
}

int CNISTRecord::FindItem(int Field, int Subfield, int Item, CStdString& sData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;

	CNISTField *pField = GetNISTField(Field);

	if (pField)
	{
		if (IsDATField(m_nRecordType, Field))
		{
			// Don't return anything for images, but do return OK if it exists
			sData = "";
			nRet = IW_SUCCESS;
		}
		else
		{
			if (Item == 0)
			{
				// Field exists but this position refers to the header item defined by
				// a Location Form 4, and hence itself does not contain any data
				sData = "";
				nRet = IW_ERR_HEADER_ITEM;
			}
			else
			{
				nRet = pField->FindItem(Subfield, Item, sData);
			}
		}
	}

	return nRet;
}

int CNISTRecord::DeleteSubfield(int Field, int Subfield)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	int nSize = m_FieldList.size();
	CNISTField *pField = 0;

	for (int i = 0; i < nSize; i++)
	{
		pField = m_FieldList.at(i);

		if (pField->m_nField == Field)
		{
			pField->RemoveSubField(Subfield);
			nRet = IW_SUCCESS;
			break;
		}
	}

	return nRet;
}

int CNISTRecord::SetItem(CStdString sData, int Field, int Subfield, int Item)
{
	int nRet = IW_SUCCESS;
	CNISTField *pField = GetNISTField(Field);

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::SetItem");
		CStdString sTraceMsg;

		sTraceMsg.Format(_T("[%s] (%ld, %ld, %ld): %s"), sTraceFrom, Field, Subfield, Item, sData);
		TraceMsg(sTraceMsg);
	}

	if (pField)
	{
		nRet = pField->SetSubField(Subfield, Item, sData);
	}
	else
	{
		pField = new CNISTField;
		if ((nRet = pField->AddItem(m_nRecordType, Field, Subfield, Item, sData)) == IW_SUCCESS)
			AddField(pField);
		else
			delete pField;
	}

	if (CNISTRecord::IsBinaryFingerType(m_nRecordType))
	{
		// For fingerprints, also update the binary FINGERPRINT_HEADER
		pField = GetNISTField(GetDATField(m_nRecordType));
		if(pField)
		{
			int iVal = _ttoi(sData);
			FINGERPRINT_HEADER* pHdr = (FINGERPRINT_HEADER*)pField->m_pImageData;
			switch(Field)
			{
				case 1: pHdr->bFieldLen[0] = HIBYTE(HIWORD(iVal));
						pHdr->bFieldLen[1] = LOBYTE(HIWORD(iVal));
						pHdr->bFieldLen[2] = HIBYTE(LOWORD(iVal));
						pHdr->bFieldLen[3] = LOBYTE(LOWORD(iVal));
						break;
				case 2: pHdr->bIDC = iVal; break;		// IDC
				case 3: pHdr->bImp = iVal; break;		// finger impression type
				case 4: if (Subfield >= 1 && Subfield <= 6)
						{
							pHdr->bFGP[Subfield-1] = iVal;	// finger position
						}
						break;
				case 5: pHdr->bISR = iVal; break;		// scanning resolution
				case 6: pHdr->bHLL[1] = LOBYTE(iVal);	// horz line length
						pHdr->bHLL[0] = HIBYTE(iVal);	// horz line length
						break;
				case 7: pHdr->bVLL[1] = LOBYTE(iVal);	// vert line length
						pHdr->bVLL[0] = HIBYTE(iVal);	// vert line length	
						break;
				case 8: pHdr->bComp = iVal;				// compression algorithm used
			}
			long l=42;
		}
	}
	else if (m_nRecordType == RECORD_TYPE8)
	{
		// For signatures, also update the binary SIGNATURE_HEADER
		pField = GetNISTField(GetDATField(m_nRecordType));
		if(pField)
		{
			int iVal = _ttoi(sData);
			SIGNATURE_HEADER* pHdr = (SIGNATURE_HEADER*)pField->m_pImageData;
			switch(Field)
			{
				case 1: pHdr->bFieldLen[0] = HIBYTE(HIWORD(iVal));
						pHdr->bFieldLen[1] = LOBYTE(HIWORD(iVal));
						pHdr->bFieldLen[2] = HIBYTE(LOWORD(iVal));
						pHdr->bFieldLen[3] = LOBYTE(LOWORD(iVal));
						break;
				case 2: pHdr->bIDC = iVal; break;		// IDC
				case 3: pHdr->bSIG = iVal; break;		// signature type
				case 4: pHdr->bSRT = iVal; break;		// signature representation type
				case 5: pHdr->bISR = iVal; break;		// scanning resolution
				case 6: pHdr->bHLL[1] = LOBYTE(iVal);	// horz line length
						pHdr->bHLL[0] = HIBYTE(iVal);	// horz line length
						break;
				case 7: pHdr->bVLL[1] = LOBYTE(iVal);	// vert line length
						pHdr->bVLL[0] = HIBYTE(iVal);	// vert line length	
			}
		}
	}

	return nRet;
}

int CNISTRecord::GetImage(CStdString& sStorageFormat, long *pLength, const BYTE **ppData)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	CNISTField *pField =  GetNISTField(GetDATField(m_nRecordType));

	sStorageFormat.Empty();
	*pLength = 0;
	*ppData = 0;

	if (pField == NULL) goto done;

	*pLength = pField->m_nImageLen;
	sStorageFormat = CNISTField::ImageExtFromImageFormat(pField->m_ImageFormat);		

	if (pField)
	{
		switch(pField->m_nRecordType)
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				// Strip fingerprint header
				*ppData = pField->m_pImageData + sizeof(FINGERPRINT_HEADER);
				nRet = IW_SUCCESS;
				break;

			case 8:
				// Strip signature header
				*ppData = pField->m_pImageData + sizeof(SIGNATURE_HEADER);
				nRet = IW_SUCCESS;
				break;

			default:
				*ppData = pField->m_pImageData;
				nRet = IW_SUCCESS;
				break;
		}
	}

done:
	return nRet;
}

int CNISTRecord::GetImageInfo(CStdString& sStorageFormat, long *pnLength, long *pnHLL, long *pnVLL, int *pnBitsPerPixel)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	CNISTField *pField =  GetNISTField(GetDATField(m_nRecordType));

	if (pField)
	{
		sStorageFormat = CNISTField::ImageExtFromImageFormat(pField->m_ImageFormat);
		*pnLength = pField->m_nImageLen;

		CStdString sData;

		FindItem(TYPE4_HLL, 1, 1, sData); // note: *_HLL are all the same
		*pnHLL = _ttol(sData);
		FindItem(TYPE4_VLL, 1, 1, sData); // note: *_VLL are all the same
		*pnVLL = _ttol(sData);

		if (m_nRecordType >= RECORD_TYPE3 && m_nRecordType <= RECORD_TYPE7)
		// These are always all grayscale
		{
			*pnBitsPerPixel = 8;
			nRet = IW_SUCCESS;
		}
		else if (m_nRecordType == 8)
		// signature is always binary
		{
			*pnBitsPerPixel = 1;
			nRet = IW_SUCCESS;
		}
		else if (m_nRecordType == 10)
		// could be color or grayscale
		{
			FindItem(TYPE10_CSP, 1, 1, sData);
			*pnBitsPerPixel = sData.CompareNoCase(_T("GRAY")) ? 24 : 8;
			nRet = IW_SUCCESS;
		}
		else if (m_nRecordType >= RECORD_TYPE13 && m_nRecordType <= RECORD_TYPE17)
		// Field has bits per pixel (*_BPX)
		{
			FindItem(TYPE13_BPX, 1, 1, sData);  // note: *_BPX are all the same
			*pnBitsPerPixel = _ttoi(sData);
			nRet = IW_SUCCESS;
		}
		else
		{
			// All cases have been dealt with
		}
	}

	return nRet;
}

int CNISTRecord::GetImageResolution(double *pdPPMM)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	CNISTField *pField =  GetNISTField(GetDATField(m_nRecordType));

	if (pField)
	{
		CStdString	sData1;
		CStdString	sData2;
		long		lResCode;
		long		lScaleUnits;
		long		lHPixelScale;

		if (m_nRecordType >= RECORD_TYPE3 && m_nRecordType <= RECORD_TYPE8)
		{
			FindItem(TYPE4_ISR, 1, 1, sData1);			// note: *_ISR are all the same
			lResCode = _ttoi(sData1);

			if (lResCode == 1)
				*pdPPMM = m_dNativeResolutionPPMM;
			else
				*pdPPMM = m_dNativeResolutionPPMM/2;	// minimum is 1/2 of native (?)
		}
		else
		{
			// For all other record types we look at the Scale and Horizontal resolution.
			// Currently we do not support differing horizontal and vertical resolutions
			// but this would be easy to do in the future if needed
			// (note: *_SLC and *_HPS are all the same)
			if ((FindItem(TYPE10_SLC, 1, 1, sData1) == IW_SUCCESS) &&
				(FindItem(TYPE10_HPS, 1, 1, sData2) == IW_SUCCESS))
			{
				lScaleUnits = _ttoi(sData1);
				lHPixelScale = _ttoi(sData2);
				if (lScaleUnits == 1) // pixels per inch
				{
					*pdPPMM = lHPixelScale / 25.4;
				}
				else if (lScaleUnits == 2) // pixels per cm
				{
					*pdPPMM = lHPixelScale * 10;
				}
				else
				{
					// We don't support 0 ("no scale units"), let's just assume 72dpi
					*pdPPMM = 72/25.4;
				}
			}
			else
			{
				// The mandatory fields are missing, let's assume a fixed resolution
				*pdPPMM = 72/25.4;
			}
		}
	}

	return IW_SUCCESS;
}

bool CNISTRecord::FormatSupportedInOut(CStdString sFormat)
// Currently RAW is not supported for import, because we don't have an easy way to specify
// image properties.
{
	IWImageFormat fmt;

	fmt = CNISTField::ImageFormatFromImageExt(sFormat);
	if (fmt == fmtBMP || fmt == fmtJPG || fmt == fmtJP2 || fmt == fmtWSQ || fmt == fmtFX4 || fmt == fmtCBEFF)
		return true;
	else
		return false;
}

bool CNISTRecord::FormatSupportedEBTS(CStdString sFormat)
// This differs from the input/output formats: the EBTS file internally can support RAW, 
// but not BMP.
{
	IWImageFormat fmt;

	fmt = CNISTField::ImageFormatFromImageExt(sFormat);
	if (fmt == fmtRAW || fmt == fmtJPG || fmt == fmtJP2 || fmt == fmtWSQ || fmt == fmtFX4 || fmt == fmtCBEFF)
		return true;
	else
		return false;
}

float CNISTRecord::CompressionToRate(float fCompression)
// Do something similar to EB to convert the 1..100 Compression factor
// to a WSQ compression "rate" parameter. We want 15 to be approximately
// 1:15, which maps to FBI's default rate of 0.75, so we use 11.25/Q, but
// we we make sure the rate never exceed 4 because this gives unpredictable
// result (and crashes).
// In the end, this implies that Compression values of 1, 2 and 3 are equivalent.
{
	int	  QFactor;
	float fRate;

	QFactor = (int)fCompression;
	QFactor = __min(__max(QFactor, 1), 100);
	fRate =  11.25f/QFactor;
	fRate = __min(fRate, 4.0f);

	return fRate;
}

int CNISTRecord::SetImage(CStdString sInputFormat, int nRecordIndex, long nLength, BYTE *pData, 
						  CStdString sStorageFormat, float fCompression)
{
	int				nRet = IW_ERR_IMAGE_CONVERSION;
	CNISTField		*pField =  GetNISTField(GetDATField(m_nRecordType));
	BYTE			*pDataTmp = NULL;
	long			nLengthTmp;
	BYTE			*pDataOut = NULL;
	long			nLengthOut;
	IWNISTImageInfo info;
	long			lWidth;
	long			lHeight;
	long			lDPI;
	IWImageFormat	fmtIn;
	IWImageFormat	fmtOut;

	// Check for invalid inputs
	if (!pData || nLength == 0) return IW_ERR_NULL_POINTER;
	// Check for supported formats
	if (!CNISTRecord::FormatSupportedInOut(sInputFormat))  return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	if (!CNISTRecord::FormatSupportedEBTS(sStorageFormat)) return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;

	fmtIn = CNISTField::ImageFormatFromImageExt(sInputFormat);
	fmtOut = CNISTField::ImageFormatFromImageExt(sStorageFormat);
	// Check for format congruency on "cbeff": this is not a real image, so it can't be converted
	if ((fmtIn == fmtCBEFF && fmtOut != fmtCBEFF) ||
		(fmtIn != fmtCBEFF && fmtOut == fmtCBEFF))
	{
		return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	}

	IWS_BEGIN_EXCEPTION_METHOD("CNISTRecord::SetImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	// Add image field if it doesn't exist. Image fields only have one DAT field.
	if (!pField)
	{
		pField = new CNISTField;
		pField->m_nRecordType = m_nRecordType;
		pField->m_nField = GetDATField(m_nRecordType);
		AddField(pField);
	}

	// Ditch existing image data, if any
	if (pField->m_pImageData)
	{
		delete [] pField->m_pImageData;
		pField->m_pImageData = 0;
		pField->m_nImageLen = 0;
	}

	// Convert image if required, after this pDataOut should own the output image
	if (fmtIn != fmtOut)
	{
		// Possible combinations at this point are:
		// BMP -> RAW, FX4, JPG, JP2 or WSQ
		// JPG -> RAW, JP2 or WSQ
		// JP2 -> RAW, JPG or WSQ
		// WSQ -> RAW, JPG or JP2
		// FX4 -> RAW

		if (fmtIn == fmtBMP)
		{
			if (fmtOut == fmtRAW)
			{
				if (BMPtoRAW((BYTE*)pData, nLength, &pDataOut, &nLengthOut, &lWidth, &lHeight, &lDPI)) goto done;
			}
			else if (fmtOut == fmtJPG)
			{
				if (BMPtoJPG((BYTE*)pData, nLength, (long)fCompression, &pDataOut, &nLengthOut)) goto done;
			}
			else if (fmtOut == fmtJP2)
			{	
				if (BMPtoJP2((BYTE*)pData, nLength, fCompression, &pDataOut, &nLengthOut)) goto done;
			}
			else if (fmtOut == fmtWSQ)
			{
				if (BMPtoWSQ((BYTE*)pData, nLength, CompressionToRate(fCompression), &pDataOut, &nLengthOut)) goto done;
			}
			else if (fmtOut == fmtFX4)
			{
				if (BMPtoFX4((BYTE*)pData, nLength, &pDataOut, &nLengthOut)) goto done;
			}
			else // (shouldn't happen)
			{
				nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
				goto done;
			}
		}
		else if (fmtIn == fmtJPG)
		{
			// Format must be converted to BMP first
			if (JPGtoBMP((BYTE*)pData, nLength, &pDataTmp, &nLengthTmp)) goto done;
			if (fmtOut == fmtRAW)
			{
				if (BMPtoRAW(pDataTmp, nLengthTmp, &pDataOut, &nLengthOut, &lWidth, &lHeight, &lDPI)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;	
			}
			else if (fmtOut == fmtJP2)
			{	
				if (BMPtoJP2(pDataTmp, nLengthTmp, fCompression, &pDataOut, &nLengthOut)) goto done;
			}
			else if (fmtOut == fmtWSQ)
			{
				if (BMPtoWSQ(pDataTmp, nLengthTmp, CompressionToRate(fCompression), &pDataOut, &nLengthOut)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;
			}
			else // (shouldn't happen)
			{
				nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
				goto done;
			}
		}
		else if (fmtIn == fmtJP2)
		{
			// Format must be converted to BMP first
			if (JP2toBMP((BYTE*)pData, nLength, &pDataTmp, &nLengthTmp)) goto done;
			if (fmtOut == fmtRAW)
			{
				if (BMPtoRAW(pDataTmp, nLengthTmp, &pDataOut, &nLengthOut, &lWidth, &lHeight, &lDPI)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;	
			}
			else if (fmtOut == fmtJPG)
			{	
				if (BMPtoJPG(pDataTmp, nLengthTmp, (long)fCompression, &pDataOut, &nLengthOut)) goto done;
			}
			else if (fmtOut == fmtWSQ)
			{
				if (BMPtoWSQ(pDataTmp, nLengthTmp, CompressionToRate(fCompression), &pDataOut, &nLengthOut)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;
			}
			else // (shouldn't happen)
			{
				nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
				goto done;
			}
		}
		else if (fmtIn == fmtWSQ)
		{
			if (fmtOut == fmtRAW)
			{
				if (WSQtoRAW((BYTE*)pData, nLength, &pDataOut, &nLengthOut, &lWidth, &lHeight, &lDPI)) goto done;
			}
			else if (fmtOut == fmtJPG)
			{
				if (WSQtoBMP((BYTE*)pData, nLength, &pDataTmp, &nLengthTmp)) goto done;
				if (BMPtoJPG(pDataTmp, nLengthTmp, (long)fCompression, &pDataOut, &nLengthOut)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;
			}
			else if (fmtOut == fmtJP2)
			{
				if (WSQtoBMP((BYTE*)pData, nLength, &pDataTmp, &nLengthTmp)) goto done;
				if (BMPtoJP2(pDataTmp, nLengthTmp, fCompression, &pDataOut, &nLengthOut)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;
			}
			else
			{
				nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
				goto done;
			}
		}
		else if  (fmtIn == fmtFX4)
		{
			if (fmtOut == fmtRAW)
			{
				if (FX4toRAW((BYTE*)pData, nLength, &pDataOut, &nLengthOut, &lWidth, &lHeight, &lDPI)) goto done;
			}
			else
			{
				nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
				goto done;
			}
		}
	}
	else
	{
		// Image in is the same as image out, or input is "cbeff":
		// just use the image blob directly
		pDataOut = new BYTE[nLength];
		nLengthOut = nLength;
		memcpy(pDataOut, pData, nLength);
	}

	// If it's a "cbeff" there's nothing else to do
	if (fmtIn == fmtCBEFF)
	{
		pField->m_ImageFormat = fmtCBEFF;
		pField->m_pImageData = pDataOut;
		pField->m_nImageLen = nLengthOut;
		pDataOut = NULL; // now belongs to m_pImageData
		nRet = IW_SUCCESS;
		goto done;
	}

	// Set field's image format constant based on extension
	pField->m_ImageFormat = CNISTField::ImageFormatFromImageExt(sStorageFormat);

	// We fetch the pertinent image information that will later be used to populate some
	// of the EBTS record's mandatory fields.
	// If the target format is RAW, we do this on the input image and then mod it slightly, 
	// otherwise directly on the output image.
	if (pField->m_ImageFormat == fmtRAW)
	{
		nRet = GetInfoFromImage((BYTE*)pData, nLength, sInputFormat, &info);
		if (nRet != IW_SUCCESS) goto done;
		// For RAW, we must change a few props
		strcpy_s(info.szCompression, 10, "NONE");
		strcpy_s(info.szColorSpace, 10, (info.lBitsPerPixel == 24 ? "RGB" : "GRAY"));
		// 2:Gray Scale (Mono) raw image format
		// 4:RGB (color) raw image format
		info.lImageFormat = (info.lBitsPerPixel == 8 ? 2 : 4);
		info.lCompressionAlgo = 0; // no compression
	}
	else
	{
		nRet = GetInfoFromImage((BYTE*)pDataOut, nLengthOut, sStorageFormat, &info);
		if (nRet != IW_SUCCESS) goto done;
	}

	if (!CNISTRecord::IsBinaryType(m_nRecordType))
	{
		// We're done, we just set the image in pField
		pField->m_pImageData = pDataOut;
		pField->m_nImageLen = nLengthOut;
		pDataOut = NULL; // now belongs to m_pImageData
	}
	else
	{
		// We must precede the image with the appropriate header
		// for fingerprint or signature
		if (CNISTRecord::IsBinaryFingerType(m_nRecordType))
		{
			pField->m_pImageData = new BYTE [nLengthOut + sizeof(FINGERPRINT_HEADER)];
			memcpy(pField->m_pImageData + sizeof(FINGERPRINT_HEADER), pDataOut, nLengthOut);
			pField->m_nImageLen = nLengthOut;
			// For binary records we also set the total record length, since we know it
			nLengthOut += sizeof(FINGERPRINT_HEADER);
			pField->m_nRecordLen = nLengthOut;

			// ditch pDataOut, since we reallocated the whole thing
			delete [] pDataOut;
			pDataOut = NULL;
		}
		else if (m_nRecordType == RECORD_TYPE8)
		{
			pField->m_pImageData = new BYTE [nLengthOut + sizeof(SIGNATURE_HEADER)];
			memcpy(pField->m_pImageData + sizeof(SIGNATURE_HEADER), pDataOut, nLengthOut);
			pField->m_nImageLen = nLengthOut;
			// For binary records we also set the total record length, since we know it
			nLengthOut += sizeof(SIGNATURE_HEADER);
			pField->m_nRecordLen = nLengthOut;

			// ditch pDataOut, since we reallocated the whole thing
			delete [] pDataOut;
			pDataOut = NULL;
		}
		else // (shouldn't happen)
		{
			goto done;
		}
	}

	// Finally, set any mandatory image fields we can based on the image data
	SetMandatoryImageFields(pField, info, nRecordIndex, nLengthOut);

	nRet = IW_SUCCESS;

	IWS_END_CATCHEXCEPTION_BLOCK()

done:

	if (pDataTmp != NULL) delete [] pDataTmp;
	if (pDataOut != NULL) delete [] pDataOut;

	return nRet;
}

int CNISTRecord::GetInfoFromImage(BYTE *pImage, long lImageLength, CStdString sFmt, IWNISTImageInfo *pInfo)
// Currently supports BMP, JPG and WSQ. Returns IW_ERR_UNSUPPORTED_IMAGE_FORMAT on most errors.
{
	int	 nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	BYTE jpgHdr[4] = { 0xFF, 0xD8, 0xFF, 0xE0 };
	BYTE exfHdr[4] = { 0xFF, 0xD8, 0xFF, 0xE1 };
	BYTE wsqHdr[2] = { 0xFF, 0xA0 };
	BYTE bmpHdr[2] = { 0x42, 0x4D };
	BYTE jp2Hdr[23] = { 0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A, 
						0x00, 0x00, 0x00, 0x14, 0x66, 0x74, 0x79, 0x70, 0x6A, 0x70, 0x32 };
	BYTE jp2IHDR[4] = { 0x69, 0x68, 0x64, 0x72 }; // "ihdr" precedes the jp2_ihdr_t struct

	IWS_BEGIN_EXCEPTION_METHOD("CNISTRecord::GetInfoFromImage")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	memset(pInfo, 0, sizeof(IWNISTImageInfo));

	sFmt.ToLower();

	if (sFmt == _T("bmp"))
	{
		// Check header to make sure format is really what caller claims it is
		if (!memcmp(pImage, bmpHdr, 2))
		{
			BITMAPINFOHEADER *pBMP = (BITMAPINFOHEADER*)(pImage + sizeof(BITMAPFILEHEADER));

			pInfo->lWidth = pBMP->biWidth;
			pInfo->lHeight = pBMP->biHeight;
			pInfo->lScaleUnits = 2; // = pixels/centimeter
			pInfo->lHPS = (long)((float)pBMP->biXPelsPerMeter/100+0.5);
			pInfo->lVPS = (long)((float)pBMP->biYPelsPerMeter/100+0.5);
			strcpy_s(pInfo->szCompression, 10, "NONE");
			strcpy_s(pInfo->szColorSpace, 10, (pBMP->biBitCount == 24 ? "RGB" : "GRAY"));
			pInfo->lBitsPerPixel = pBMP->biBitCount;
			// Looks like BMP isn't really supported: for now use "4", "RGB (color) raw image format"
			pInfo->lImageFormat = 4;
			pInfo->lCompressionAlgo = 0;

			nRet = IW_SUCCESS;
		}
	}
	else if (sFmt == _T("jpg"))
	{
		// Check header to make sure format is really what caller claims it is
		if ((!memcmp(pImage, jpgHdr, 4) && !memcmp(pImage+6, "JFIF", 4)) ||
			(!memcmp(pImage, exfHdr, 4) && !memcmp(pImage+6, "Exif", 4)))
		{
			IWJPGINFO jInfo = { 0, };
			HGLOBAL hJPG = GlobalAlloc(GPTR, lImageLength);

			if (hJPG)
			{
				BYTE *pJPG = (BYTE*)GlobalLock(hJPG);

				if (pJPG)
				{
					memcpy(pJPG, pImage, lImageLength);
					GlobalUnlock(hJPG);
					JPG_GetInfo(hJPG, &jInfo);
					GlobalFree(hJPG);

					pInfo->lWidth = jInfo.lWidth;
					pInfo->lHeight = jInfo.lHeight;
					pInfo->lScaleUnits = jInfo.lScaleUnits;
					pInfo->lHPS = jInfo.lHPS;
					pInfo->lVPS = jInfo.lVPS;
					strcpy_s(pInfo->szCompression, 10, (jInfo.lCompression == 1 ? "JPEGB" : "JPEGL"));
					strcpy_s(pInfo->szColorSpace, 10, (jInfo.lColorSpace == CS_YCbCr ? "YCC" : "UNK"));
					pInfo->lBitsPerPixel = (jInfo.lColorSpace == CS_GRAYSCALE ? 8 : 24);
					pInfo->lImageFormat = (jInfo.lColorSpace == CS_GRAYSCALE ? 6 : 8);
					pInfo->lCompressionAlgo = 1;

					nRet = IW_SUCCESS;
				}
				else
				{
					GlobalFree(hJPG);
					nRet = IW_ERR_OUT_OF_MEMORY;
				}
			}
			else
			{
				nRet = IW_ERR_OUT_OF_MEMORY;
			}
		}
	}
	else if (sFmt == _T("jp2"))
	{
		// Check header to make sure format is really what caller claims it is
		if (!memcmp(pImage, jp2Hdr, 23))
		{
			char* p = (char*)pImage;

			// Struct follows "ihdr" (=image header), seek to it
			while((p - (char*)pImage < lImageLength) && (memcmp(p, jp2IHDR, 4)))
				p++;

			p += sizeof(jp2IHDR);

			jp2_ihdr_t ihdr;

			ihdr.width = MAKELONG(MAKEWORD(*(p+3), *(p+2)), MAKEWORD(*(p+1), *p));
			p += sizeof(ihdr.width);
			ihdr.height = MAKELONG(MAKEWORD(*(p+3), *(p+2)), MAKEWORD(*(p+1), *p));
			p += sizeof(ihdr.height);
			ihdr.numcmpts = MAKEWORD(*(p+1), *p);
			p += sizeof(ihdr.numcmpts);

			pInfo->lWidth = ihdr.width;
			pInfo->lHeight = ihdr.height;
			// For now let's assume 500dpi, in the future we could search for the "res" header
			pInfo->lScaleUnits = 1; // = pixels/inch
			pInfo->lHPS = 500;
			pInfo->lVPS = 500;
			strcpy_s(pInfo->szCompression, 10, "JP2");
			strcpy_s(pInfo->szColorSpace, 10, (ihdr.numcmpts == 3 ? "RGB" : "GRAY"));
			pInfo->lBitsPerPixel = 8*ihdr.numcmpts;
			//Note: lCompressionAlgo does not apply
			//Note: lImageFormat not set since Type-16, which requires a "lImageFormat" can't have JP2

			nRet = IW_SUCCESS;
		}
	}
	else if (sFmt == _T("wsq"))
	{
		// Check header to make sure format is really what caller claims it is
		if (!memcmp(pImage, wsqHdr, 2))
		{
			char* p = (char*)pImage;

			// seek out the frame header (SOF)
			while((p - (char*)pImage < lImageLength) && (MAKEWORD(*(p+1), *p) != 0xFFA2))
				p++;

			// advance data pointer and verify position in buffer
			if((p += sizeof(WORD)) - (char*)pImage < lImageLength)
			{
				FRM_HEADER_WSQ frm_header_wsq;

				WORD hdr_len = MAKEWORD(*(p+1), *p);
				p += sizeof(hdr_len);

				frm_header_wsq.black = (BYTE) *p;
				p += sizeof(frm_header_wsq.black);
				frm_header_wsq.white = (BYTE) *p;
				p += sizeof(frm_header_wsq.white);
				frm_header_wsq.width = MAKEWORD(*(p+1), *p);
				p += sizeof(frm_header_wsq.width);
				frm_header_wsq.height = MAKEWORD(*(p+1), *p);

				pInfo->lWidth = frm_header_wsq.width;
				pInfo->lHeight = frm_header_wsq.height;
				// Assume 500 DPI for WSQ files for now
				pInfo->lScaleUnits = 1; // = pixels/inch
				pInfo->lHPS = 500;
				pInfo->lVPS = 500;
				strcpy_s(pInfo->szCompression, 10, "WSQ");
				strcpy_s(pInfo->szColorSpace, 10, "GRAY");
				pInfo->lBitsPerPixel = 8;
				pInfo->lCompressionAlgo = 1;
				//Note: lImageFormat not set since Type-16, which requires a "lImageFormat" can't have WSQ

				nRet = IW_SUCCESS;
			}
		}
	}
	else if (sFmt == _T("fx4"))
	{
		// Use our tifflib library to get dimensions ans resolution
		if (LIBTIFFGetInfo((char*)pImage, lImageLength, &pInfo->lWidth, &pInfo->lHeight, &pInfo->lHPS))
		{
			pInfo->lHPS = pInfo->lVPS;
			strcpy_s(pInfo->szCompression, 10, "FAX538"); // does not apply
			strcpy_s(pInfo->szColorSpace, 10, ""); // does not apply
			pInfo->lBitsPerPixel = 1;
			pInfo->lCompressionAlgo = 1;
			//Note: lImageFormat not set since Type-16, which requires a "lImageFormat" can't have FX4

			nRet = IW_SUCCESS;
		}
	}

	IWS_END_CATCHEXCEPTION_BLOCK()

	return nRet;
}

void CNISTRecord::SetMandatoryImageFields(CNISTField *pNISTField, IWNISTImageInfo& info, long lRecordIndex, long lFieldLength)
{
	BYTE jpgHdr[4] = { 0xFF, 0xD8, 0xFF, 0xE0 };
	BYTE exfHdr[4] = { 0xFF, 0xD8, 0xFF, 0xE1 };
	BYTE wsqHdr[2] = { 0xFF, 0xA0 };
	BYTE bmpHdr[2] = { 0x42, 0x4D };

	IWS_BEGIN_EXCEPTION_METHOD("CNISTRecord::SetMandatoryImageFields")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CStdString sData;

	if (CNISTRecord::IsBinaryFingerType(pNISTField->m_nRecordType))
	{
		// Note we use the Type4_* constants here, because the values are all the
		// same for Type-3 though Type-7

		// Here we should initialize the entire binary header:
		// BYTE bFieldLen[4];
		// BYTE bIDC;
		// BYTE bImp; // finger impression type
		// BYTE bFGP[6]; // finger position
		// BYTE bISR; // scanning resolution
		// BYTE bHLL[2]; // Horiz line length
		// BYTE bVLL[2]; // Vert line length
		// BYTE bComp; // compression algorithm used

		sData.Format(_T("%ld"), lFieldLength);
		SetItem(sData, TYPE4_LEN, 1, 1);

		sData.Format(_T("%ld"), lRecordIndex);
		SetItem(sData, TYPE4_IDC, 1, 1);

		SetItem(_T("0"), TYPE4_IMP, 1, 1); // default impression to 0="Live-scan Plain"

		// Set all 6 fingers to "unknown"
		for (int i=1; i<=6; i++) SetItem(_T("255"), TYPE4_FGP, i, 1);

		// For now if the compression if 500dpi or more, we use 1 (Native Scanning Resolution), 
		// 0 (Minimum Scanning Resolution) otherwise
		sData.Format(_T("%ld"), (info.lHPS >= 500 ? 1 : 0));
		SetItem(sData, TYPE4_ISR, 1, 1);

		sData.Format(_T("%ld"), info.lWidth);
		SetItem(sData, TYPE4_HLL, 1, 1);

		sData.Format(_T("%ld"), info.lHeight);
		SetItem(sData, TYPE4_VLL, 1, 1);

		sData.Format(_T("%ld"), info.lCompressionAlgo);
		SetItem(sData, TYPE4_GCA, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE8)
	{
		// Here we should initialize the entire binary header:
		// BYTE bFieldLen[4];
		// BYTE bIDC;
		// BYTE bSIG; // signature type
		// BYTE bSRT; // signature representation type
		// BYTE bISR; // scanning resolution
		// BYTE bHLL[2]; // Horiz line length
		// BYTE bVLL[2]; // Vert line length

		sData.Format(_T("%ld"), lFieldLength);
		SetItem(sData, TYPE8_LEN, 1, 1);

		sData.Format(_T("%ld"), lRecordIndex);
		SetItem(sData, TYPE8_IDC, 1, 1);

		SetItem(_T("0"), TYPE8_SIG, 1, 1); // default signature type, 0="subject's signature" 

		// TYPE8_SRT: “0” if the image is scanned and not compressed, a binary “1” if the
		// image is scanned and compressed, and the binary equivalent of “2” if the image
		// is vector data. We don't support vector data yet, so it's either 0 or 1.
		sData.Format(_T("%ld"), info.lCompressionAlgo);
		SetItem(sData, TYPE8_SRT, 1, 1);

		// For now if the compression if 500dpi or more, we use 1 (Native Scanning Resolution), 
		// 0 (Minimum Scanning Resolution) otherwise
		sData.Format(_T("%ld"), (info.lHPS >= 500 ? 1 : 0));
		SetItem(sData, TYPE8_ISR, 1, 1);

		sData.Format(_T("%ld"), info.lWidth);
		SetItem(sData, TYPE8_HLL, 1, 1);

		sData.Format(_T("%ld"), info.lHeight);
		SetItem(sData, TYPE8_VLL, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE10)
	{
		sData.Format(_T("%ld"), info.lWidth);
		SetItem(sData, TYPE10_HLL, 1, 1);

		sData.Format(_T("%ld"), info.lHeight);
		SetItem(sData, TYPE10_VLL, 1, 1);

		sData.Format(_T("%ld"), info.lScaleUnits);
		SetItem(sData, TYPE10_SLC, 1, 1);

		sData.Format(_T("%ld"), info.lHPS);
		SetItem(sData, TYPE10_HPS, 1, 1);

		sData.Format(_T("%ld"), info.lVPS);
		SetItem(sData, TYPE10_VPS, 1, 1);

		SetItem(CStdString(info.szCompression), TYPE10_CGA, 1, 1);
		SetItem(CStdString(info.szColorSpace), TYPE10_CSP, 1, 1);
	}
	else if (pNISTField->m_nRecordType >= RECORD_TYPE13 && pNISTField->m_nRecordType <= RECORD_TYPE15)
	{
		// Note, we use the Type-14 constants, but they are the same for Type-13 and Type-15

		sData.Format(_T("%ld"), info.lWidth);
		SetItem(sData, TYPE14_HLL, 1, 1);

		sData.Format(_T("%ld"), info.lHeight);
		SetItem(sData, TYPE14_VLL, 1, 1);

		sData.Format(_T("%ld"), info.lScaleUnits);
		SetItem(sData, TYPE14_SLC, 1, 1);

		sData.Format(_T("%ld"), info.lHPS);
		SetItem(sData, TYPE14_HPS, 1, 1);

		sData.Format(_T("%ld"), info.lVPS);
		SetItem(sData, TYPE14_VPS, 1, 1);

		SetItem(CStdString(info.szCompression), TYPE14_CGA, 1, 1);

		sData.Format(_T("%ld"), info.lBitsPerPixel);
		SetItem(sData, TYPE14_BPX, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE16)
	{
		sData.Format(_T("%ld"), info.lWidth);
		SetItem(sData, TYPE16_HLL, 1, 1);

		sData.Format(_T("%ld"), info.lHeight);
		SetItem(sData, TYPE16_VLL, 1, 1);

		sData.Format(_T("%ld"), info.lScaleUnits);
		SetItem(sData, TYPE16_SLC, 1, 1);

		sData.Format(_T("%ld"), info.lHPS);
		SetItem(sData, TYPE16_HPS, 1, 1);

		sData.Format(_T("%ld"), info.lVPS);
		SetItem(sData, TYPE16_VPS, 1, 1);

		SetItem(info.szCompression, TYPE16_CGA, 1, 1);

		sData.Format(_T("%ld"), info.lBitsPerPixel);
		SetItem(sData, TYPE16_BPX, 1, 1);

		sData.Format(_T("%ld"), info.lImageFormat);
		SetItem(sData, TYPE16_IFM, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE17)
	{
		sData.Format(_T("%ld"), info.lWidth);
		SetItem(sData, TYPE17_HLL, 1, 1);

		sData.Format(_T("%ld"), info.lHeight);
		SetItem(sData, TYPE17_VLL, 1, 1);

		sData.Format(_T("%ld"), info.lScaleUnits);
		SetItem(sData, TYPE17_SLC, 1, 1);

		sData.Format(_T("%ld"), info.lHPS);
		SetItem(sData, TYPE17_HPS, 1, 1);

		sData.Format(_T("%ld"), info.lVPS);
		SetItem(sData, TYPE17_VPS, 1, 1);

		SetItem(info.szCompression, TYPE17_CGA, 1, 1);

		sData.Format(_T("%ld"), info.lBitsPerPixel);
		SetItem(sData, TYPE17_BPX, 1, 1);

		SetItem(CStdString(info.szColorSpace), TYPE17_CSP, 1, 1);
	}

	IWS_END_CATCHEXCEPTION_BLOCK()
}

int CNISTRecord::GetDATField(int nRecordType)
{
	int iRet = -1;

	switch (nRecordType)
	{
		case 3:
		case 4:
		case 5:
		case 6:	 iRet = TYPE4_DAT;  break;
		case 7:  iRet = TYPE7_DAT;  break;
		case 8:  iRet = TYPE8_DAT;  break;
		case 10: iRet = TYPE10_DAT; break;
		case 13: iRet = TYPE13_DAT; break;
		case 14: iRet = TYPE14_DAT; break;
		case 15: iRet = TYPE15_DAT; break;
		case 16: iRet = TYPE16_DAT; break;
		case 17: iRet = TYPE17_DAT; break;
		case 99: iRet = TYPE99_DAT; break;
	}

	return iRet;
}

bool CNISTRecord::IsDATField(int nRecordType, int nField)
{
	return (nField == GetDATField(nRecordType));
}

bool CNISTRecord::IsBinaryFingerType(int nRecordType)
{
	if (nRecordType >= RECORD_TYPE3 && nRecordType <= RECORD_TYPE7)
		return true;
	else
		return false;	
}

bool CNISTRecord::IsBinaryType(int nRecordType)
// Binary fingerprint types plus signature type
{
	if (CNISTRecord::IsBinaryFingerType(nRecordType) || nRecordType == RECORD_TYPE8)
		return true;
	else
		return false;	
}

int CNISTRecord::GetRecordLen()
{
	int nFields = m_FieldList.size();
	CNISTField *pField;
	int nLen = 0;
	int nImagePos = 0;
	bool bImage = false;

	// get length of all fields except for .01 field
	for (int i = 0; i < nFields; i++)
	{
		pField = m_FieldList.at(i);

		if (pField && pField->m_nField != 1)
			nLen += pField->GetWriteLen();

		if (IsDATField(pField->m_nRecordType, pField->m_nField))
		{
			bImage = true;
			nImagePos = i; 
		}
	}

	// make sure image is at end of ary
	if (bImage)
	{
		pField = m_FieldList.at(nImagePos);
		m_FieldList.erase(m_FieldList.begin() + nImagePos);
		m_FieldList.push_back(pField);
	}
			
	nLen++; // for record seperator

	// now add .01 field len minus the len string
	if (m_nRecordType == 1)
		nLen += 5; // for "1.01:"
	else
		nLen += 6; // for "1.001:"

	char szLen[30];

	sprintf(szLen, "%d", nLen); // get the length of the len field
	nLen += strlen(szLen);

	pField = GetNISTField(REC_TAG_LEN);

	if (pField)
	{
		pField->m_nRecordLen = nLen; // set len field for writting
		pField->SetSubField(1, 1, szLen);
	}

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::GetRecordLen");
		CStdString sTraceMsg;

		sTraceMsg.Format(_T("[%s] Recordtype %d, Len %ld"), sTraceFrom, m_nRecordType, nLen);
		TraceMsg(sTraceMsg);
	}

	return nLen;
}

int CNISTRecord::Write(FILE *pFile)
{
	int nFields = m_FieldList.size();
	CNISTField *pField;
	int nRet = IW_SUCCESS;

	for (int i = 0; i < nFields && nRet == IW_SUCCESS; i++)
	{
		pField = m_FieldList.at(i);

		if (pField)
		{
			pField->m_bWriteRecordSeperator = false;
			if (i+1 == nFields) 
				pField->m_bWriteRecordSeperator = true;

			nRet = pField->Write(pFile);
		}
	}

	return nRet;
}

int CNISTRecord::WriteBinary(FILE *pFile)
{
	if (CNISTRecord::IsBinaryType(m_nRecordType))
	{
		CNISTField* pField = GetNISTField(GetDATField(m_nRecordType));
		if(pField)
		{
			fwrite(pField->m_pImageData, 1, pField->m_nRecordLen, pFile);
		}
	}

	return NO_ERROR;
}

int CNISTRecord::WriteBinary(BYTE **ppData, int *poffset)
{
	if (CNISTRecord::IsBinaryType(m_nRecordType))
	{
		CNISTField* pField = GetNISTField(GetDATField(m_nRecordType));
		if(pField)
		{
			memcpy(*ppData + *poffset, pField->m_pImageData, pField->m_nRecordLen);
			*poffset = *poffset + pField->m_nRecordLen;
		}
	}

	return NO_ERROR;
}

int CNISTRecord::Write(TCHAR **ppData, int *poffset)
{
	int nFields = m_FieldList.size();
	CNISTField *pField;
	int nRet = IW_SUCCESS;

	for (int i = 0; i < nFields && nRet == IW_SUCCESS; i++)
	{
		pField = m_FieldList.at(i);

		if (pField)
		{
			pField->m_bWriteRecordSeperator = false;
			if (i+1 == nFields) 
				pField->m_bWriteRecordSeperator = true;

			nRet = pField->Write(ppData, poffset);
		}
	}

	return nRet;
}

