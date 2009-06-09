
#include "stdafx.h"
#include "NISTField.h"
#include "NISTRecord.h"
#include "IWNIST.h"


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
	char szLen[6] = { '0', '\0' };
	int nRet;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::InitializeNewRecord");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] RecordType %d", sTraceFrom, nRecordType);
		TraceMsg(sTraceMsg);
	}

	m_nRecordType = nRecordType;

	/***********************************************/
	/* 2 fields must appear in every record, and they
	/* must be the first 2 fields in the record
	/************************************************/

	// by default always add the .001 and .002 records
	if ((nRet = pField->AddItem(nRecordType, REC_TAG_LEN, 1/*Subfield*/, 1/*Item*/, szLen)) == IW_SUCCESS)
	{
		AddField(pField);

		pField = new CNISTField;

		if (nRecordType == RECORD_TYPE1)
			wsprintf(szLen, "%s", "0201"); // default version
		else
			wsprintf(szLen, "%s", "00"); // default IDC value, should be correctly set by calling application?

		if ((nRet = pField->AddItem(nRecordType, (nRecordType == RECORD_TYPE1 ? REC_TAG_VER : REC_TAG_IDC), 1/*Subfield*/, 1/*Item*/, szLen)) == IW_SUCCESS)
			AddField(pField);

		if (nRecordType == RECORD_TYPE1) // initialize an empty contents field
		{
			pField = new CNISTField;
			wsprintf(szLen, "%s", "1"); 
			if ((nRet = pField->AddItem(nRecordType, TYPE1_CNT, 1/*Subfield*/, 1/*Item*/, szLen)) == IW_SUCCESS)
			{
				wsprintf(szLen, "%s", "0"); 
				if ((nRet = pField->AddItem(nRecordType, TYPE1_CNT, 1/*Subfield*/, 2/*Item*/, szLen)) == IW_SUCCESS)
					AddField(pField);
			}
		}
	}
}

void CNISTRecord::SetNativeScanningResolution(double dNativeResolutionPPMM)
{
	m_dNativeResolutionPPMM = dNativeResolutionPPMM;
}

int CNISTRecord::ReadLogicalRecordLen(char *pTransactionData, int nRecordType, int nRecordIndex)
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
		
		sTraceMsg.Format("[%s] RecordType %d, Index %d, Length %ld", sTraceFrom, nRecordType, nRecordIndex, nRet);
		TraceMsg(sTraceMsg);
	}

	return nRet;
}

int CNISTRecord::ReadRecord(char *pTransactionData, int nRecordType)
{
	int nRet = IW_SUCCESS;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::ReadRecord");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] RecordType %d", sTraceFrom, nRecordType);
		TraceMsg(sTraceMsg);
	}

	m_nRecordType = nRecordType;
	m_bGetImage = FALSE;

	m_nRecordLen = ReadLogicalRecordLen(pTransactionData, nRecordType); // get size of this record

	if (m_nRecordLen > 0)
	{
		char *pRecord = new char[m_nRecordLen];

		if (pRecord)
		{
			int nField;
			char *pszFieldData;
			char *pContent = 0;
			CStdString sErr;
			CNISTField *pField;
			char *pTemp;
			BOOL bEndofRecord = FALSE;
			int nFieldPos = 1;

			memcpy(pRecord, pTransactionData, m_nRecordLen*sizeof(char));

			pTemp = IWStrTok(pRecord, CHAR_PERIOD);
	
			while (pTemp && !bEndofRecord)
			{
				nField = atoi(pTemp);

				if (nField == nRecordType) // make sure we're in sync
				{
					pTemp = IWStrTok(NULL, CHAR_COLON, &bEndofRecord); // get the field

					if (pTemp)
					{
						nField = atoi(pTemp);

						if (IsDATField(m_nRecordType, nField))
						{
							// take advantage of the fact that I know
							// the pointer is sitting at the beginning of the image bytes
							m_bGetImage = TRUE;
							pszFieldData = IWStrTok(NULL, CHAR_GS, &bEndofRecord); // get the field	data	
							m_bGetImage = FALSE;
						}
						else
							pszFieldData = IWStrTok(NULL, CHAR_GS, &bEndofRecord); // get the field	data	
						
						pField = new CNISTField;
						pField->m_nRecordType = nRecordType;
						pField->m_nField = nField;

						if (nField == REC_TAG_LEN || nField == REC_TAG_IDC)
						// these don't need to be handled special 
						// but it helps for debugging
						{
							pField->SetSubField(1, 1, pszFieldData);
						}
						else if (IsDATField(m_nRecordType, nField))
						{
							if (pszFieldData)
							{
								int nOffset = pszFieldData-pRecord;
								BYTE *pImage = (BYTE*)pTransactionData+nOffset;
								int nImageLen = (m_nRecordLen-nOffset)-1;

								pField->SetImageData(pImage, nImageLen);	
							}
						}
						else
						{
							if (strchr(pszFieldData, CHAR_US) || strchr(pszFieldData, CHAR_RS))
							{
								AddSubItems(pField, pszFieldData);
							}
							else
								pField->SetSubField(1, 1, pszFieldData);
						}
						AddField(pField);
					}
				}
				// get next field
				pTemp = IWStrTok(NULL, CHAR_PERIOD, &bEndofRecord);
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

char *CNISTRecord::IWStrTok(char *pInStr, char cDelim, BOOL *pbEndofRecord)
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
				*pbEndofRecord = TRUE;

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

int CNISTRecord::ReadBinaryRecord(char *pTransactionData, int nRecordType)
{
	int nRet = IW_SUCCESS;
	BYTE bIDC;

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::ReadBinaryRecord");
		CStdString sTraceMsg;
		
		sTraceMsg.Format("[%s] RecordType %d", sTraceFrom, nRecordType);
		TraceMsg(sTraceMsg);
	}

	m_nRecordType = nRecordType;
	m_bGetImage = FALSE;

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

	char szFieldData[40] = { '\0', };

	IWS_BEGIN_EXCEPTION_METHOD("CNISTRecord::ReadBinaryRecord")
	IWS_BEGIN_CATCHEXCEPTION_BLOCK()

	CNISTField *pField = new CNISTField(nRecordType, REC_TAG_LEN);

	// the first 2 fields are the same for all binary records, LEN and IDC
	wsprintf(szFieldData, "%ld", GetDecimalValue(pTransactionData, 4));
	pField->SetSubField(1, REC_TAG_LEN, szFieldData);
	AddField(pField);
	m_nRecordLen = atoi(szFieldData);

	pField = new CNISTField(nRecordType, REC_TAG_IDC);
	wsprintf(szFieldData, "%d", bIDC);
	pField->SetSubField(1, 1, szFieldData);
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

int CNISTRecord::GetFingerprintInfo(int nRecordType, char *pTransactionData)
{
	int nRet = IW_SUCCESS;
	FINGERPRINT_HEADER *pFPrintHdr = (FINGERPRINT_HEADER *)pTransactionData;
	CNISTField *pField;
	char szFieldData[40] = { '\0', };

	pField = new CNISTField(nRecordType, 3);	//Impression Type

	wsprintf(szFieldData, "%d", pFPrintHdr->bImp);
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 4); //finger position
	wsprintf(szFieldData, "%d", GetDecimalValue((char *)pFPrintHdr->bFGP, 1));
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 5); //scanning resolution
	wsprintf(szFieldData, "%d", pFPrintHdr->bISR);
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 6); //Horiz line length
	wsprintf(szFieldData, "%d", GetDecimalValue((char *)pFPrintHdr->bHLL, 2));
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 7); //Vert line length
	wsprintf(szFieldData, "%d", GetDecimalValue((char *)pFPrintHdr->bVLL, 2));
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 8); //compression algorithm
	wsprintf(szFieldData, "%d", pFPrintHdr->bComp);
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	return nRet;
}

int CNISTRecord::GetSignatureInfo(char *pTransactionData)
{
	int nRet = IW_SUCCESS;
	SIGNATURE_HEADER *pSig = (SIGNATURE_HEADER *)pTransactionData;
	CNISTField *pField;
	int nRecordType = RECORD_TYPE8;
	char szFieldData[40] = { '\0', };

	pField = new CNISTField(nRecordType, 3/*Signature type*/);
	wsprintf(szFieldData, "%d", pSig->bSIG);
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 4/*signature representation type*/);
	wsprintf(szFieldData, "%d", pSig->bSRT);
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 5/*scanning resolution*/);
	wsprintf(szFieldData, "%d", pSig->bISR);
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 6/*Horiz line length*/);
	wsprintf(szFieldData, "%d", GetDecimalValue((char *)pSig->bHLL, 2));
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	pField = new CNISTField(nRecordType, 7/*Vert line length*/);
	wsprintf(szFieldData, "%d", GetDecimalValue((char *)pSig->bVLL, 2));
	pField->SetSubField(1, 1, szFieldData);
	AddField(pField);

	return nRet;
}

long CNISTRecord::GetDecimalValue(char *pInStr, int nStrLen)
{
	char szHexStr[20] = { '\0', };
	char szTemp[5];
	char *pTemp = pInStr;

	for (int i = 0; i < nStrLen; i++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcat_s(szHexStr, sizeof(szHexStr), BytetoHexString(pTemp[i], szTemp));
	}
	
	return strtol(szHexStr, NULL, 16);
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
	int nRet = IW_ERR_RECORD_NOT_FOUND;
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

int CNISTRecord::AddSubItems(CNISTField *pNISTField, char *pszFieldData)
{
	int nRet = IW_SUCCESS;
	char szRS[2];

	szRS[0] = CHAR_RS;
	szRS[1] = '\0';

	if (pszFieldData && *pszFieldData)
	{
		int nLen = strlen(pszFieldData);

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

			pTemp = strtok(pszFieldData, szRS);
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
								pSubItem->m_pszData = new char[nSubFieldItemLen+1];
								strcpy_s(pSubItem->m_pszData, nSubFieldItemLen+1, pTempSubItem);
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
						pSubItem->m_pszData = new char[nSubFieldItemLen+1];
						strcpy_s(pSubItem->m_pszData, nSubFieldItemLen+1, pTempSubItem);
					}
					pNISTField->m_SubFieldAry.push_back(pSubItem);
				}
				nSubField++;

				if (pTempSub >= pszFieldData + nLen)
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

int CNISTRecord::FindItem(int Field, int Subfield, int Item, const char** ppData)
{
	int nRet = IW_ERR_RECORD_NOT_FOUND;

	CNISTField *pField = GetNISTField(Field);

	if (pField)
	{
		if (IsDATField(m_nRecordType, Field))
		{
			*ppData = (char*)pField->m_pImageData;
			nRet = IW_SUCCESS;
		}
		else
		{
			nRet = pField->FindItem(Subfield, Item, ppData);
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

int CNISTRecord::SetItem(const char *pData, int Field, int Subfield, int Item)
{
	int nRet = IW_SUCCESS;
	CNISTField *pField = GetNISTField(Field);

	if (g_bTraceOn)
	{
		CStdString sTraceFrom("CNISTRecord::SetItem");
		CStdString sTraceMsg;

		sTraceMsg.Format("[%s] (%ld, %ld, %ld): %s", sTraceFrom, Field, Subfield, Item, pData);
		TraceMsg(sTraceMsg);
	}

	if (pField)
		nRet = pField->SetSubField(Subfield, Item, pData);
	else
	{
		pField = new CNISTField;
		if ((nRet = pField->AddItem(m_nRecordType, Field, Subfield, Item, pData)) == IW_SUCCESS)
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
			int iVal = atoi(pData);
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
			int iVal = atoi(pData);
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

int CNISTRecord::GetImage(const char **ppStorageFormat, long *pLength, const void **ppData)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	CNISTField *pField =  GetNISTField(GetDATField(m_nRecordType));
	*ppStorageFormat = 0;
	*pLength = 0;
	*ppData = 0;

	if (pField == NULL) goto done;

	*pLength = pField->m_nImageLen;
	*ppStorageFormat = CNISTField::ImageExtFromImageFormat(pField->m_ImageFormat);		

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

int CNISTRecord::GetImageInfo(const char **ppStorageFormat, long *pLength, long *phll, long *pvll, int *pBitsPerPixel)
{
	int nRet = IW_ERR_INVALID_SUBFIELD_NUM;
	CNISTField *pField =  GetNISTField(GetDATField(m_nRecordType));

	if (pField)
	{
		*ppStorageFormat = CNISTField::ImageExtFromImageFormat(pField->m_ImageFormat);
		*pLength = pField->m_nImageLen;

		char* pData;

		FindItem(TYPE4_HLL, 1, 1, (const char**) &pData); // note: *_HLL are all the same
		*phll = atoi(pData);
		FindItem(TYPE4_VLL, 1, 1, (const char**) &pData); // note: *_VLL are all the same
		*pvll = atoi(pData);

		if (m_nRecordType >= RECORD_TYPE3 && m_nRecordType <= RECORD_TYPE7)
		// These are always all grayscale
		{
			*pBitsPerPixel = 8;
			nRet = IW_SUCCESS;
		}
		else if (m_nRecordType == 8)
		// signature is always binary
		{
			*pBitsPerPixel = 1;
			nRet = IW_SUCCESS;
		}
		else if (m_nRecordType == 10)
		// could be color or grayscale
		{
			FindItem(TYPE10_CSP, 1, 1, (const char**) &pData);
			*pBitsPerPixel = _stricmp(pData, (const char*)"GRAY") ? 24 : 8;
			nRet = IW_SUCCESS;
		}
		else if (m_nRecordType >= RECORD_TYPE13 && m_nRecordType <= RECORD_TYPE17)
		// Field has bits per pixel (*_BPX)
		{
			FindItem(TYPE13_BPX, 1, 1, (const char**) &pData);  // note: *_BPX are all the same
			*pBitsPerPixel = atoi(pData);
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
		char	*pData1;
		char	*pData2;
		long	lResCode;
		long	lScaleUnits;
		long	lHPixelScale;

		if (m_nRecordType >= RECORD_TYPE3 && m_nRecordType <= RECORD_TYPE8)
		{
			FindItem(TYPE4_ISR, 1, 1, (const char**) &pData1); // note: *_ISR are all the same
			lResCode = atoi(pData1);

			if (lResCode == 1)
				*pdPPMM = m_dNativeResolutionPPMM;
			else
				*pdPPMM = m_dNativeResolutionPPMM/2; // minimum is 1/2 of native (?)
		}
		else
		{
			// For all other record types we look at the Scale and Horizontal resolution.
			// Currently we do not support differing horizontal and vertical resolutions
			// but this would be easy to do in the future if needed
			// (note: *_SLC and *_HPS are all the same)
			if ((FindItem(TYPE10_SLC, 1, 1, (const char**) &pData1) == IW_SUCCESS) &&
				(FindItem(TYPE10_HPS, 1, 1, (const char**) &pData2) == IW_SUCCESS))
			{
				lScaleUnits = atoi(pData1);
				lHPixelScale = atoi(pData2);
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

BOOL CNISTRecord::FormatSupportedInOut(const char *pFormat)
// Currently RAW is not supported for import, because we don't have an easy way to specify
// image properties.
{
	IWImageFormat fmt;

	fmt = CNISTField::ImageFormatFromImageExt(pFormat);
	if (fmt == fmtBMP || fmt == fmtJPG || fmt == fmtJP2 || fmt == fmtWSQ || fmt == fmtFX4 || fmt == fmtCBEFF)
		return TRUE;
	else
		return FALSE;
}

BOOL CNISTRecord::FormatSupportedEBTS(const char *pFormat)
// This differs from the input/output formats: the EBTS file internally can support RAW, 
// but not BMP.
{
	IWImageFormat fmt;

	fmt = CNISTField::ImageFormatFromImageExt(pFormat);
	if (fmt == fmtRAW || fmt == fmtJPG || fmt == fmtJP2 || fmt == fmtWSQ || fmt == fmtFX4 || fmt == fmtCBEFF)
		return TRUE;
	else
		return FALSE;
}

float CNISTRecord::CompressionToRate(float Compression)
// Do something similar to EB to convert the 1..100 Compression factor
// to a WSQ compression "rate" parameter. We want 15 to be approximately
// 1:15, which maps to FBI's default rate of 0.75, so we use 11.25/Q, but
// we we make sure the rate never exceed 4 because this gives unpredictable
// result (and crashes).
// In the end, this implies that Compression values of 1, 2 and 3 are equivalent.
{
	int	  QFactor;
	float fRate;

	QFactor = (int)Compression;
	QFactor = __min(__max(QFactor, 1), 100);
	fRate =  11.25f/QFactor;
	fRate = __min(fRate, 4.0f);

	return fRate;
}

int CNISTRecord::SetImage(const char *pInputFormat, int RecordIndex, long Length, void *pData, 
						  const char *pStorageFormat, float Compression)
{
	int				nRet = IW_ERR_IMAGE_CONVERSION;
	CNISTField		*pField =  GetNISTField(GetDATField(m_nRecordType));
	BYTE			*pDataTmp = NULL;
	long			LengthTmp;
	BYTE			*pDataOut = NULL;
	long			LengthOut;
	IWNISTImageInfo info;
	long			lWidth;
	long			lHeight;
	long			lDPI;

	// Check for invalid inputs
	if (!pInputFormat || !pStorageFormat) return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	if (!pData || Length == 0) return IW_ERR_NULL_POINTER;
	// Check for supported formats
	if (!CNISTRecord::FormatSupportedInOut(pInputFormat))    return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	if (!CNISTRecord::FormatSupportedEBTS(pStorageFormat)) return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
	// Check for format congruency on "cbeff": this is not a real image, so it can't be converted
	if ((!_stricmp(pInputFormat, "cbeff") &&  _stricmp(pStorageFormat, "cbeff")) ||
		( _stricmp(pInputFormat, "cbeff") && !_stricmp(pStorageFormat, "cbeff")))
		return IW_ERR_UNSUPPORTED_IMAGE_FORMAT;

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

	// Convert image if required, after this  pDataOut should own the output image
	if (_stricmp(pInputFormat, pStorageFormat))
	{
		// Possible combinations at this point are:
		// BMP -> RAW, FX4, JPG, JP2 or WSQ
		// JPG -> RAW, JP2 or WSQ
		// JP2 -> RAW, JPG or WSQ
		// WSQ -> RAW, JPG or JP2
		// FX4 -> RAW

		if (!_stricmp(pInputFormat, "bmp"))
		{
			if (!_stricmp(pStorageFormat, "raw"))
			{
				if (BMPtoRAW((BYTE*)pData, Length, &pDataOut, &LengthOut, &lWidth, &lHeight, &lDPI)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "jpg"))
			{
				if (BMPtoJPG((BYTE*)pData, Length, (long)Compression, &pDataOut, &LengthOut)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "jp2"))
			{	
				if (BMPtoJP2((BYTE*)pData, Length, Compression, &pDataOut, &LengthOut)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "wsq"))
			{
				if (BMPtoWSQ((BYTE*)pData, Length, CompressionToRate(Compression), &pDataOut, &LengthOut)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "fx4"))
			{
				if (BMPtoFX4((BYTE*)pData, Length, &pDataOut, &LengthOut)) goto done;
			}
			else // (shouldn't happen)
			{
				nRet = IW_ERR_UNSUPPORTED_IMAGE_FORMAT;
				goto done;
			}
		}
		else if (!_stricmp(pInputFormat, "jpg"))
		{
			// Format must be converted to BMP first
			if (JPGtoBMP((BYTE*)pData, Length, &pDataTmp, &LengthTmp)) goto done;
			if (!_stricmp(pStorageFormat, "raw"))
			{
				if (BMPtoRAW(pDataTmp, LengthTmp, &pDataOut, &LengthOut, &lWidth, &lHeight, &lDPI)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;	
			}
			else if (!_stricmp(pStorageFormat, "jp2"))
			{	
				if (BMPtoJP2(pDataTmp, LengthTmp, Compression, &pDataOut, &LengthOut)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "wsq"))
			{
				if (BMPtoWSQ(pDataTmp, LengthTmp, CompressionToRate(Compression), &pDataOut, &LengthOut)) goto done;
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
		else if (!_stricmp(pInputFormat, "jp2"))
		{
			// Format must be converted to BMP first
			if (JP2toBMP((BYTE*)pData, Length, &pDataTmp, &LengthTmp)) goto done;
			if (!_stricmp(pStorageFormat, "raw"))
			{
				if (BMPtoRAW(pDataTmp, LengthTmp, &pDataOut, &LengthOut, &lWidth, &lHeight, &lDPI)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;	
			}
			else if (!_stricmp(pStorageFormat, "jpg"))
			{	
				if (BMPtoJPG(pDataTmp, LengthTmp, (long)Compression, &pDataOut, &LengthOut)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "wsq"))
			{
				if (BMPtoWSQ(pDataTmp, LengthTmp, CompressionToRate(Compression), &pDataOut, &LengthOut)) goto done;
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
		else if (!_stricmp(pInputFormat, "wsq"))
		{
			if (!_stricmp(pStorageFormat, "raw"))
			{
				if (WSQtoRAW((BYTE*)pData, Length, &pDataOut, &LengthOut, &lWidth, &lHeight, &lDPI)) goto done;
			}
			else if (!_stricmp(pStorageFormat, "jpg"))
			{
				if (WSQtoBMP((BYTE*)pData, Length, &pDataTmp, &LengthTmp)) goto done;
				if (BMPtoJPG(pDataTmp, LengthTmp, (long)Compression, &pDataOut, &LengthOut)) goto done;
				// ditch the temp image, pDataOut now owns the output image
				delete [] pDataTmp;
				pDataTmp = NULL;
			}
			else if (!_stricmp(pStorageFormat, "jp2"))
			{
				if (WSQtoBMP((BYTE*)pData, Length, &pDataTmp, &LengthTmp)) goto done;
				if (BMPtoJP2(pDataTmp, LengthTmp, Compression, &pDataOut, &LengthOut)) goto done;
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
		else if (!_stricmp(pInputFormat, "fx4"))
		{
			if (!_stricmp(pStorageFormat, "raw"))
			{
				if (FX4toRAW((BYTE*)pData, Length, &pDataOut, &LengthOut, &lWidth, &lHeight, &lDPI)) goto done;
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
		pDataOut = new BYTE[Length];
		LengthOut = Length;
		memcpy(pDataOut, pData, Length);
	}

	// If it's a "cbeff" there's nothing else to do
	if (!_stricmp(pInputFormat, "cbeff"))
	{
		pField->m_ImageFormat = fmtCBEFF;
		pField->m_pImageData = pDataOut;
		pField->m_nImageLen = LengthOut;
		pDataOut = NULL; // now belongs to m_pImageData
		nRet = IW_SUCCESS;
		goto done;
	}

	// Set field's image format constant based on extension
	pField->m_ImageFormat = CNISTField::ImageFormatFromImageExt(pStorageFormat);

	// We fetch the pertinent image information that will later be used to populate some
	// of the EBTS record's mandatory fields.
	// If the target format is RAW, we do this on the input image and then mod it slightly, 
	// otherwise directly on the output image.
	if (pField->m_ImageFormat == fmtRAW)
	{
		nRet = GetInfoFromImage((BYTE*)pData, Length, pInputFormat, &info);
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
		nRet = GetInfoFromImage((BYTE*)pDataOut, LengthOut, pStorageFormat, &info);
		if (nRet != IW_SUCCESS) goto done;
	}

	if (!CNISTRecord::IsBinaryType(m_nRecordType))
	{
		// We're done, we just set the image in pField
		pField->m_pImageData = pDataOut;
		pField->m_nImageLen = LengthOut;
		pDataOut = NULL; // now belongs to m_pImageData
	}
	else
	{
		// We must precede the image with the appropriate header
		// for fingerprint or signature
		if (CNISTRecord::IsBinaryFingerType(m_nRecordType))
		{
			pField->m_pImageData = new BYTE [LengthOut + sizeof(FINGERPRINT_HEADER)];
			memcpy(pField->m_pImageData + sizeof(FINGERPRINT_HEADER), pDataOut, LengthOut);
			pField->m_nImageLen = LengthOut;
			// For binary records we also set the total record length, since we know it
			LengthOut += sizeof(FINGERPRINT_HEADER);
			pField->m_nRecordLen = LengthOut;

			// ditch pDataOut, since we reallocated the whole thing
			delete [] pDataOut;
			pDataOut = NULL;
		}
		else if (m_nRecordType == RECORD_TYPE8)
		{
			pField->m_pImageData = new BYTE [LengthOut + sizeof(SIGNATURE_HEADER)];
			memcpy(pField->m_pImageData + sizeof(SIGNATURE_HEADER), pDataOut, LengthOut);
			pField->m_nImageLen = LengthOut;
			// For binary records we also set the total record length, since we know it
			LengthOut += sizeof(SIGNATURE_HEADER);
			pField->m_nRecordLen = LengthOut;

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
	SetMandatoryImageFields(pField, info, RecordIndex, LengthOut);

	nRet = IW_SUCCESS;

	IWS_END_CATCHEXCEPTION_BLOCK()

done:

	if (pDataTmp != NULL) delete [] pDataTmp;
	if (pDataOut != NULL) delete [] pDataOut;

	return nRet;
}

int CNISTRecord::GetInfoFromImage(BYTE *pImage, long lImageLength, const char* szFmt, IWNISTImageInfo *pInfo)
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

	if (!_stricmp(szFmt, "bmp"))
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
	else if (!_stricmp(szFmt, "jpg"))
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
	else if (!_stricmp(szFmt, "jp2"))
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
	else if (!_stricmp(szFmt, "wsq"))
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
	else if (!_stricmp(szFmt, "fx4"))
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

	char szData[20];

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

		wsprintf(szData, "%ld", lFieldLength);
		SetItem(szData, TYPE4_LEN, 1, 1);

		wsprintf(szData, "%ld", lRecordIndex);
		SetItem(szData, TYPE4_IDC, 1, 1);

		SetItem("0", TYPE4_IMP, 1, 1); // default impression to 0="Live-scan Plain"

		// Set all 6 fingers to "unknown"
		for (int i=1; i<=6; i++) SetItem("255", TYPE4_FGP, i, 1);

		// For now if the compression if 500dpi or more, we use 1 (Native Scanning Resolution), 
		// 0 (Minimum Scanning Resolution) otherwise
		wsprintf(szData, "%ld", (info.lHPS >= 500 ? 1 : 0));
		SetItem(szData, TYPE4_ISR, 1, 1);

		wsprintf(szData, "%ld", info.lWidth);
		SetItem(szData, TYPE4_HLL, 1, 1);

		wsprintf(szData, "%ld", info.lHeight);
		SetItem(szData, TYPE4_VLL, 1, 1);

		wsprintf(szData, "%ld", info.lCompressionAlgo);
		SetItem(szData, TYPE4_GCA, 1, 1);
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

		wsprintf(szData, "%ld", lFieldLength);
		SetItem(szData, TYPE8_LEN, 1, 1);

		wsprintf(szData, "%ld", lRecordIndex);
		SetItem(szData, TYPE8_IDC, 1, 1);

		SetItem("0", TYPE8_SIG, 1, 1); // default signature type, 0="subject's signature" 

		// TYPE8_SRT: “0” if the image is scanned and not compressed, a binary “1” if the
		// image is scanned and compressed, and the binary equivalent of “2” if the image
		// is vector data. We don't support vector data yet, so it's either 0 or 1.
		wsprintf(szData, "%ld", info.lCompressionAlgo);
		SetItem(szData, TYPE8_SRT, 1, 1);

		// For now if the compression if 500dpi or more, we use 1 (Native Scanning Resolution), 
		// 0 (Minimum Scanning Resolution) otherwise
		wsprintf(szData, "%ld", (info.lHPS >= 500 ? 1 : 0));
		SetItem(szData, TYPE8_ISR, 1, 1);

		wsprintf(szData, "%ld", info.lWidth);
		SetItem(szData, TYPE8_HLL, 1, 1);

		wsprintf(szData, "%ld", info.lHeight);
		SetItem(szData, TYPE8_VLL, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE10)
	{
		wsprintf(szData, "%ld", info.lWidth);
		SetItem(szData, TYPE10_HLL, 1, 1);

		wsprintf(szData, "%ld", info.lHeight);
		SetItem(szData, TYPE10_VLL, 1, 1);

		wsprintf(szData, "%ld", info.lScaleUnits);
		SetItem(szData, TYPE10_SLC, 1, 1);

		wsprintf(szData, "%ld", info.lHPS);
		SetItem(szData, TYPE10_HPS, 1, 1);

		wsprintf(szData, "%ld", info.lVPS);
		SetItem(szData, TYPE10_VPS, 1, 1);

		SetItem(info.szCompression, TYPE10_CGA, 1, 1);

		wsprintf(szData, "%s", info.szColorSpace);
		SetItem(szData, TYPE10_CSP, 1, 1);
	}
	else if (pNISTField->m_nRecordType >= RECORD_TYPE13 && pNISTField->m_nRecordType <= RECORD_TYPE15)
	{
		// Note, we use the Type-14 constants, but they are the same for Type-13 and Type-15

		wsprintf(szData, "%ld", info.lWidth);
		SetItem(szData, TYPE14_HLL, 1, 1);

		wsprintf(szData, "%ld", info.lHeight);
		SetItem(szData, TYPE14_VLL, 1, 1);

		wsprintf(szData, "%ld", info.lScaleUnits);
		SetItem(szData, TYPE14_SLC, 1, 1);

		wsprintf(szData, "%ld", info.lHPS);
		SetItem(szData, TYPE14_HPS, 1, 1);

		wsprintf(szData, "%ld", info.lVPS);
		SetItem(szData, TYPE14_VPS, 1, 1);

		SetItem(info.szCompression, TYPE14_CGA, 1, 1);

		wsprintf(szData, "%ld", info.lBitsPerPixel);
		SetItem(szData, TYPE14_BPX, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE16)
	{
		wsprintf(szData, "%ld", info.lWidth);
		SetItem(szData, TYPE16_HLL, 1, 1);

		wsprintf(szData, "%ld", info.lHeight);
		SetItem(szData, TYPE16_VLL, 1, 1);

		wsprintf(szData, "%ld", info.lScaleUnits);
		SetItem(szData, TYPE16_SLC, 1, 1);

		wsprintf(szData, "%ld", info.lHPS);
		SetItem(szData, TYPE16_HPS, 1, 1);

		wsprintf(szData, "%ld", info.lVPS);
		SetItem(szData, TYPE16_VPS, 1, 1);

		SetItem(info.szCompression, TYPE16_CGA, 1, 1);

		wsprintf(szData, "%ld", info.lBitsPerPixel);
		SetItem(szData, TYPE16_BPX, 1, 1);

		wsprintf(szData, "%ld", info.lImageFormat);
		SetItem(szData, TYPE16_IFM, 1, 1);
	}
	else if (pNISTField->m_nRecordType == RECORD_TYPE17)
	{
		wsprintf(szData, "%ld", info.lWidth);
		SetItem(szData, TYPE17_HLL, 1, 1);

		wsprintf(szData, "%ld", info.lHeight);
		SetItem(szData, TYPE17_VLL, 1, 1);

		wsprintf(szData, "%ld", info.lScaleUnits);
		SetItem(szData, TYPE17_SLC, 1, 1);

		wsprintf(szData, "%ld", info.lHPS);
		SetItem(szData, TYPE17_HPS, 1, 1);

		wsprintf(szData, "%ld", info.lVPS);
		SetItem(szData, TYPE17_VPS, 1, 1);

		SetItem(info.szCompression, TYPE17_CGA, 1, 1);

		wsprintf(szData, "%ld", info.lBitsPerPixel);
		SetItem(szData, TYPE17_BPX, 1, 1);

		wsprintf(szData, "%s", info.szColorSpace);
		SetItem(szData, TYPE17_CSP, 1, 1);
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

BOOL CNISTRecord::IsDATField(int nRecordType, int nField)
{
	return (nField == GetDATField(nRecordType));
}

BOOL CNISTRecord::IsBinaryFingerType(int nRecordType)
{
	if (nRecordType >= RECORD_TYPE3 && nRecordType <= RECORD_TYPE7)
		return TRUE;
	else
		return FALSE;	
}

BOOL CNISTRecord::IsBinaryType(int nRecordType)
// Binary fingerprint types plus signature type
{
	if (CNISTRecord::IsBinaryFingerType(nRecordType) || nRecordType == RECORD_TYPE8)
		return TRUE;
	else
		return FALSE;	
}

int CNISTRecord::GetRecordLen()
{
	int nFields = m_FieldList.size();
	CNISTField *pField;
	int nLen = 0;
	int nImagePos = 0;
	BOOL bImage = FALSE;

	// get length of all fields except for .01 field
	for (int i = 0; i < nFields; i++)
	{
		pField = m_FieldList.at(i);

		if (pField && pField->m_nField != 1)
			nLen += pField->GetWriteLen();

		if (IsDATField(pField->m_nRecordType, pField->m_nField))
		{
			bImage = TRUE;
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

	wsprintf(szLen, "%d", nLen); // get the length of the len field
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

		sTraceMsg.Format("[%s] Recordtype %d, Len %ld", sTraceFrom, m_nRecordType, nLen);
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
			pField->m_bWriteRecordSeperator = FALSE;
			if (i+1 == nFields) 
				pField->m_bWriteRecordSeperator = TRUE;

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
