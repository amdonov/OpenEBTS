// IWNISTEditorDoc.cpp : implementation of the CIWNISTEditorDoc class
//

#include "stdafx.h"
#include "IWNISTEditor.h"
#include "IWNISTEditorDoc.h"
#include "IWNISTEditorView.h"
#include "IWOutlookBar.h"
#include "MainFrm.h"
#include "ProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIWNISTEditorDoc

IMPLEMENT_DYNCREATE(CIWNISTEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CIWNISTEditorDoc, CDocument)
END_MESSAGE_MAP()


// CIWNISTEditorDoc construction/destruction

CIWNISTEditorDoc::CIWNISTEditorDoc()
{
	m_pIWTrans = NULL;
	m_nCurrentRecord = -1;
}

CIWNISTEditorDoc::~CIWNISTEditorDoc()
{
}

BOOL CIWNISTEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	IWNew("", NULL, &m_pIWTrans);

	// Set the version of NIST file we are creating
	// For EBTS 8.1, this is 0400
	IWSetItem(m_pIWTrans, (LPCSTR)"0400", 1, 1, 2, 1, 1);

	ReadType1Record();

	IWClose(&m_pIWTrans);

	SetModifiedFlag(FALSE);
	
	return TRUE;
}


BOOL CIWNISTEditorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	BOOL bRet = FALSE;

	if (CDocument::OnOpenDocument(lpszPathName))
	{
		m_strPathName = lpszPathName;

		if(ReadNIST() == ERROR_SUCCESS)
		{
			SetModifiedFlag(FALSE);
			bRet = TRUE;
		}
	}
	else
	{
		CString csMessage;
		csMessage.Format("%s is not a valid NIST file.", lpszPathName);
		AfxMessageBox(csMessage);
	}

	return bRet;
}

// CIWNISTEditorDoc serialization

void CIWNISTEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CIWNISTEditorDoc diagnostics

#ifdef _DEBUG
void CIWNISTEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CIWNISTEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CIWNISTEditorDoc commands

int CIWNISTEditorDoc::ReadNIST(void)
{
	CWaitCursor cursor;
	int			nRet;

	nRet  = IWRead(m_strPathName, NULL, &m_pIWTrans);

	if (nRet == ERROR_SUCCESS)
	{
		ReadType1Record();
		ReadType2Record();

		for(int i = 3; i <= 6; i++)
			ReadFingerprintRecords(i);

		for(int i = 13; i <= 14; i++)
			ReadFingerprintRecords(i);
		
		ReadType7Records();
		ReadType8Records();
		ReadType9Records();
		ReadType10Records();

		// 11, 12 reserved for future use

		ReadType15Records();
		ReadType16Records();
		ReadType17Records();
		ReadType99Records();
	}

	IWClose(&m_pIWTrans);

	CWnd* pWnd = theApp.m_pStatusWnd;
	if(pWnd->GetSafeHwnd()) pWnd->SendMessage(WM_UPDATE_STATUS, NULL, (LPARAM) "Ready");
	
	return nRet;
}

int CIWNISTEditorDoc::ReadType1Record()
{
	if (!m_pIWTrans) return -1;

	CIWNistRecord* pRec = new CIWNistRecord;
	pRec->m_nRecordType = 1;

	int nRecordType = 1;
	int nRecordTypeCount = 0;

	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;
	int nType2 = 0, nType3 = 0, nType4 = 0, nType5 = 0, 
		nType6 = 0, nType7 = 0, nType8 = 0, nType9 = 0, 
		nType10 = 0, nType13 = 0, nType14 = 0, nType15 = 0, 
		nType16 = 0, nType17 = 0, nType99 = 0;
	int nFace = 0, nSMT = 0, nIris = 0;

///	CString sSubField;
///	CString sItemData, sData, sLBText;
	const char *Data;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);
		
	// for each record of this type
	for (int index = 1; index <= nRecordTypeCount; index++)
	{
		IWGetNextField(m_pIWTrans, nRecordType, index, FieldNum, &FieldNum);

		while (FieldNum)
		{
			CIWNistField* pField = new CIWNistField;
			pField->m_ID = FieldNum;

			IWNumSubfields(m_pIWTrans, nRecordType, index, FieldNum, &NumSubfields);

			for (Subfield=1; Subfield <= NumSubfields; Subfield++)
			{
				CIWNistSubfield* pSubfield = new CIWNistSubfield;

///				sSubField.Format(NumSubfields > 1 ? _T("Subfield %d") : _T(""), Subfield);

				IWNumItems(m_pIWTrans, nRecordType, index, FieldNum, Subfield, &NumItems);

///				sData = "";
				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, index, FieldNum, Subfield, Item, &Data);

///					if(FieldNum == 3)
///					{
///						if(Subfield == 1 && Item == 2)
///						{
///							sItemData.Format(_T("%d\t"), FieldNum);
///							sData += sItemData;
///						}
///						
///						if(Subfield > 1 && Item == 1)
///						{
///							switch(atoi(Data))
///							{
///								case 2: nType2++; break;
///								case 3: nType3++; break;
///								case 4: nType4++; break;
///								case 5: nType5++; break;
///								case 6: nType6++; break;
///								case 7: nType7++; break;
///								case 8: nType8++; break;
///								case 9: nType9++; break;
///								case 10: nType10++; break;
///								case 13: nType13++; break;
///								case 14: nType14++; break;
///								case 15: nType15++; break;
///								case 16: nType16++; break;
///								case 17: nType17++; break;
///								case 99: nType99++; break;
///							}
//							continue;
///						}
//						continue;
///					}
///					else if(NumItems == 1)
///					{
///						sItemData = Data;
///						sData += sItemData;
///					}
///					else
///					{
///						sItemData.Format(_T("Item %d ="), Item);
///						sItemData += CString(Data) + _T("\n");
///						sData += sItemData;
///					}

					pSubfield->m_Items.Add(Data);
				}

				pField->m_SubFields.Add(pSubfield);
			}
/*
			// add an entry to the record list
			if(FieldNum == 3)
			{
				sLBText = sItemData;
				if(nType2 > 0)
				{
					sItemData.Format(_T("Descriptive Text Records : %d\n"), nType2);
					sLBText += sItemData;
				}
				if(nType3 > 0)
				{
					sItemData.Format(_T("Fingerprint Records (Low-Resolution Grayscale): %d\n"), nType3);
					sLBText += sItemData;
				}
				if(nType4 > 0)
				{
					sItemData.Format(_T("Fingerprint Records (High-Resolution Grayscale): %d\n"), nType4);
					sLBText += sItemData;
				}
				if(nType5 > 0)
				{
					sItemData.Format(_T("Fingerprint Records (Low-Resolution Binary): %d\n"), nType5);
					sLBText += sItemData;
				}
				if(nType6 > 0)
				{
					sItemData.Format(_T("Fingerprint Records (High-Resolution Binary): %d\n"), nType6);
					sLBText += sItemData;
				}

				if(nType7 > 0)
				{
					sItemData.Format(_T("User-Defined Image Records : %d\n"), nType7);
					sLBText += sItemData;
				}
				if(nType8 > 0)
				{
					sItemData.Format(_T("Signature Records : %d\n"), nType8);
					sLBText += sItemData;
				}
				if(nType9 > 0)
				{
					sItemData.Format(_T("Minutiae Data Records : %d\n"), nType9);
					sLBText += sItemData;
				}

				if(nType10 > 0)
				{
					for(int i = 1; i <= nType10; i++)
					{
						IWFindItem(m_pIWTrans, 10, i, 3, 1, 1, &Data);
						switch(GetImageType(Data))
						{
							case 1:
								nFace++;
								break;
							case 2:
								nSMT++;
								break;
							case 3:
								nIris++;
								break;
						}

					}
					if(nFace)
					{
						sItemData.Format(_T("Facial Records : %d\n"), nFace);
						sLBText += sItemData;
					}
					if(nSMT)
					{
						sItemData.Format(_T("SMT Records : %d\n"), nFace);
						sLBText += sItemData;
					}
				}

				if(nType13 > 0)
				{
					sItemData.Format(_T("Variable-Resolution Latent Image Records: %d\n"), nType13);
					sLBText += sItemData;
				}
				if(nType14 > 0)
				{
					sItemData.Format(_T("Fingerprint Records (High-Resolution Binary): %d\n"), nType14);
					sLBText += sItemData;
				}
				if(nType15 > 0)
				{
					sItemData.Format(_T("Fingerprint Records (High-Resolution Binary): %d\n"), nType15);
					sLBText += sItemData;
				}


				if(nType16 || nIris)
				{
					sItemData.Format(_T("Iris Records : %d\n"), nType16 + nIris);
					sLBText += sItemData;
				}
			}
			else
			{
				sLBText.Format(_T("%d\t%s%s"), FieldNum, sSubField, sData);
			}

			//TRACE(sLBText + _T("\n"));

			pRec->m_arrStrings.Add(sLBText);
*/
			pRec->m_arrFields.Add(pField);

			IWGetNextField(m_pIWTrans, nRecordType, index, FieldNum, &FieldNum);
		}
	}

	m_arrRecords.Add(pRec);

	return ERROR_SUCCESS;
}
int CIWNISTEditorDoc::ReadType2Record()
{
	if (!m_pIWTrans) return -1;

	int nRecordType = 2;
	int nRecordTypeCount = 0;

	CIWNistRecord* pRec = new CIWNistRecord;
	pRec->m_nRecordType = nRecordType;

	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;

	CString sSubField, sItemData, sData, sLBText;
	const char *Data;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);

	if(!nRecordTypeCount) return NO_ERROR;

	// for each record of this type
	for (int index = 1; index <= nRecordTypeCount; index++)
	{
		IWGetNextField(m_pIWTrans, nRecordType, index, FieldNum, &FieldNum);

		while (FieldNum)
		{
			CIWNistField* pField = new CIWNistField;
			pField->m_ID = FieldNum;

			IWNumSubfields(m_pIWTrans, nRecordType, index, FieldNum, &NumSubfields);

			sData.Empty();

			for (Subfield=1, sData = ""; Subfield <= NumSubfields; Subfield++)
			{
				CIWNistSubfield* pSubfield = new CIWNistSubfield;

				sSubField.Format("Subfield %d%c", Subfield, US);

				sData += sSubField;

				IWNumItems(m_pIWTrans, nRecordType, index, FieldNum, Subfield, &NumItems);

				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, index, FieldNum, Subfield, Item, &Data);

					sItemData.Format(_T("Item %d = %s"), Item, CString(Data));
					
					sData += sItemData;
					
					if(Item < NumItems) sData += RS;

					pSubfield->m_Items.Add(Data);
				}

				pField->m_SubFields.Add(pSubfield);

				sData += "\t";

				sLBText.Format(_T("Field %d\n%s"), FieldNum, sData);

				//TRACE(sLBText + _T("\n"));

				pRec->m_arrStrings.Add(sLBText);
			}

			pRec->m_arrFields.Add(pField);

			IWGetNextField(m_pIWTrans, nRecordType, index, FieldNum, &FieldNum);
		}
	}

	m_arrRecords.Add(pRec);

	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadFingerprintRecords(int nRecordType, int nPos, BOOL bIsCompressed)
{
	if (!m_pIWTrans) return -1;

	int nRecordTypeCount = 0;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);

	// anything to do?
	if(!nRecordTypeCount) return ERROR_SUCCESS;
	
	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;

	CString sSubField, sItemData, sData, sLBText;
	const char *Data;

	char* fld_name[] = { "Unknown", "Record Length","Image Designator","Impression Type",
	                     "Finger Position","Image Resolution","Image Width","Image Height","Compression Alogrithm" };
	char* imp_type[] = { "Live-scan slap", "Live-scan roll",
		                 "Non-live-scan slap","Non-live-scan roll",
						 "Captured Directly","Mark tracing","Mark photo","Mark lift" };
	char* print_pos[]= { "Unknown","Right thumb","Right index finger",
		                 "Right middle finger","Right ring finger","Right little finger",
						 "Left thumb","Left index finger","Left middle finger",
						 "Left ring finger","Left little finger","Right thumb slap",
						 "Left thumb slap","Right four slap","Left four slap"};

	int nFirstToRead = (nPos == READ_ALL_RECORDS ? 1 : nPos);
	int nLastToRead  = (nPos == READ_ALL_RECORDS ? nRecordTypeCount : nPos);

	CWnd* pWnd = theApp.m_pStatusWnd;
	CProgressBar* pBar = NULL;

	if(pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
		pBar = new CProgressBar("", 100, 100);

	for (int i = nFirstToRead; i <= nLastToRead; i++)
	{
		char* szText = new char[80];
		sprintf_s(szText, 80, "Reading fingerprint record %d of %d...", i, nRecordTypeCount);

		if(pBar)
		{
			pBar->SetText(szText);
			pBar->SetPos((int)(((double)i/(double)nRecordTypeCount)*100.00));
		}

		if(pWnd->GetSafeHwnd()) pWnd->SendMessage(WM_UPDATE_STATUS, (int)(((double)i/(double)nRecordTypeCount)*100.00), (LPARAM) szText);

		delete szText;

		CIWNistRecord* pRec = new CIWNistRecord;
		pRec->m_nRecordType = nRecordType;

		sLBText.Empty();
		
		IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		
		while (FieldNum)
		{
			// skip image data (field 999)
			if(FieldNum == 999)
			{
				IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
				continue;
			}

			sLBText.Format("%d\t", FieldNum);

			IWNumSubfields(m_pIWTrans, nRecordType, i, FieldNum, &NumSubfields);
			
			for (Subfield = 1; Subfield <= NumSubfields; Subfield++)
			{
				sSubField.Format(NumSubfields > 1 ? _T("Subfield %d") : _T(""), Subfield);

				IWNumItems(m_pIWTrans, nRecordType, i, FieldNum, Subfield, &NumItems);
				
				sData = "";
				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, i, FieldNum, Subfield, Item,&Data);

					sItemData.Format(_T("Item %d = %s"), Item, CString(Data));
					sData += sItemData;
				}

				int nData = atoi(Data);
				switch(FieldNum)
				{
					case 3:
						if(nData >= 0 && nData < sizeof(imp_type)/sizeof(imp_type[0]))
							sLBText += imp_type[atoi(Data)];
						else
							sLBText += Data;
						break;
					case 4:
						if(nData >= 0 && nData < sizeof(print_pos)/sizeof(print_pos[0]))
							sLBText += print_pos[atoi(Data)];
						else
							sLBText += Data;
						break;
					default:
						sLBText += Data;
						break;
				}

				sLBText += "\n";
				
			}
			
			pRec->m_arrStrings.Add(sLBText);

			IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		}
		
		// make thumbnail
		HGLOBAL hBMP = GetThumbnail(nRecordType, i);	

		pRec->m_hDIB = hBMP;

		m_arrRecords.Add(pRec);
	}

	if (pBar) delete pBar;

	if (pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)) && pWnd->GetSafeHwnd())
		pWnd->SendMessage(WM_UPDATE_STATUS, 100, (LPARAM) "Ready");

	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadType7Records()
{
	return ReadFingerprintRecords(7);
}

int CIWNISTEditorDoc::ReadType8Records()
{
	if (!m_pIWTrans) return -1;

	int nRecordType = 8;
	int nRecordTypeCount = 0;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);

	// anything to do?
	if(!nRecordTypeCount) return ERROR_SUCCESS;
	
	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;

	CString sSubField, sItemData, sData, sLBText;
	const char *Data;

	for (int i = 1; i <= nRecordTypeCount; i++)
	{
		CIWNistRecord* pRec = new CIWNistRecord;
		pRec->m_nRecordType = nRecordType;

		sLBText.Empty();
		
		IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		
		while (FieldNum)
		{
			// skip image data 
			if(FieldNum == 8)
			{
				IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
				continue;
			}

			sLBText.Format("%d\t", FieldNum);

			IWNumSubfields(m_pIWTrans, nRecordType, i, FieldNum, &NumSubfields);
			
			for (Subfield = 1; Subfield <= NumSubfields; Subfield++)
			{
				sSubField.Format(NumSubfields > 1 ? _T("Subfield %d") : _T(""), Subfield);

				IWNumItems(m_pIWTrans, nRecordType, i, FieldNum, Subfield, &NumItems);
				
				sData = "";
				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, i, FieldNum, Subfield, Item,&Data);

					sItemData.Format(_T("Item %d = %s"), Item, CString(Data));
					sData += sItemData;
				}
				
				sLBText += Data;
				sLBText += "\n";
				
			}
			
			pRec->m_arrStrings.Add(sLBText);

			IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		}
		
		// make thumbnail
		HGLOBAL hBMP = GetThumbnail(nRecordType, i);	

		pRec->m_hDIB = hBMP;

		m_arrRecords.Add(pRec);
	}

	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadType9Records()
{
	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadType10Records()
{
	if (!m_pIWTrans) return -1;

	int nRecordType = 10;
	int nRecordTypeCount = 0;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);

	// anything to do?
	if(!nRecordTypeCount) return ERROR_SUCCESS;
	
	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;

	CString sSubField;
	CString sItemData, sData, sLBText;
	const char *Data;

	for (int i = 1; i <= nRecordTypeCount; i++)
	{
		CIWNistRecord* pRec = new CIWNistRecord;
		pRec->m_nRecordType = nRecordType;

		sLBText.Empty();

		IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);

		while (FieldNum)
		{
			// skip image data (field 999)
			if(FieldNum == 999)
			{
				IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
				continue;
			}

			IWNumSubfields(m_pIWTrans, nRecordType, i, FieldNum, &NumSubfields);

			for (Subfield = 1; Subfield <= NumSubfields; Subfield++)
			{
				sSubField.Format(NumSubfields > 1 ? _T("Subfield %d") : _T(""), Subfield);

				IWNumItems(m_pIWTrans, nRecordType, i, FieldNum, Subfield, &NumItems);

				sData = "";
				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, i, FieldNum, Subfield, Item, &Data);

					if(FieldNum == 3)
					{
						pRec->m_nImageType = GetImageType(Data);
					}

					if(NumItems == 1)
					{
						sItemData = Data;
						sData += sItemData;
					}
					else
					{
						sItemData.Format(_T("Item %d = %s "), Item, Data);
						sData += sItemData;
					}
				}

				sLBText.Format(_T("%d\t%s%s%s\n"), FieldNum, sSubField, NumItems > 1 ? ": " : "", sData);
				pRec->m_arrStrings.Add(sLBText);
			}


			IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		}

		// remove trailing newline character as needed
		if (sLBText.Right(1).Compare(_T("\n")) == 0)
			sLBText.Delete(sLBText.GetLength()-1);

		// get image
		HGLOBAL hBMP = GetThumbnail(nRecordType, i);

		pRec->m_hDIB = hBMP;

		m_arrRecords.Add(pRec);
	}

	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadType15Records()
{
	if (!m_pIWTrans) return -1;

	int nRecordType = 15;
	int nRecordTypeCount = 0;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);

	// anything to do?
	if(!nRecordTypeCount) return ERROR_SUCCESS;
	
	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;

	CString sSubField;
	CString sItemData, sData, sLBText;
	const char *Data;

	for (int i = 1; i <= nRecordTypeCount; i++)
	{
		CIWNistRecord* pRec = new CIWNistRecord;
		pRec->m_nRecordType = nRecordType;

		sLBText.Empty();

		IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);

		while (FieldNum)
		{
			// skip image data (field 999)
			if(FieldNum == 999)
			{
				IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
				continue;
			}

			IWNumSubfields(m_pIWTrans, nRecordType, i, FieldNum, &NumSubfields);

			for (Subfield = 1; Subfield <= NumSubfields; Subfield++)
			{
				sSubField.Format(NumSubfields > 1 ? _T("Subfield %d") : _T(""), Subfield);

				IWNumItems(m_pIWTrans, nRecordType, i, FieldNum, Subfield, &NumItems);

				sData = "";
				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, i, FieldNum, Subfield, Item, &Data);

					if(FieldNum == 3)
					{
						pRec->m_nImageType = GetImageType(Data);
					}

					if(NumItems == 1)
					{
						sItemData = Data;
						sData += sItemData;
					}
					else
					{
						sItemData.Format(_T("Item %d = %s "), Item, Data);
						sData += sItemData;
					}
				}

				sLBText.Format(_T("%d\t%s%s%s\n"), FieldNum, sSubField, NumItems > 1 ? ": " : "", sData);
				pRec->m_arrStrings.Add(sLBText);
			}


			IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		}

		// remove trailing newline character as needed
		if (sLBText.Right(1).Compare(_T("\n")) == 0)
			sLBText.Delete(sLBText.GetLength()-1);

		// get image
		HGLOBAL hBMP = GetThumbnail(nRecordType, i);

		pRec->m_hDIB = hBMP;

		m_arrRecords.Add(pRec);
	}

	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadType16Records()
{
	return ReadType1617Records(16);
}

int CIWNISTEditorDoc::ReadType17Records()
{
	return ReadType1617Records(17);
}

int CIWNISTEditorDoc::ReadType1617Records(int nRecordType)
{
	if (!m_pIWTrans) return -1;

	int nRecordTypeCount = 0;
	int nImageType = 3;

	// establish the number of records to read
	IWGetRecordTypeCount(m_pIWTrans, nRecordType, &nRecordTypeCount);

	// if there are no records of this kind
	// make sure we don't have a page for them
	if(!nRecordTypeCount) return ERROR_SUCCESS;
	
	int FieldNum = 0;
	int NumSubfields, Subfield;
	int NumItems, Item;

	CString sSubField, sItemData, sData, sLBText;
	const char *Data;

//	IWFindItem(m_pIWTrans, nRecordType, 3, FieldNum, Subfield, Item, &Data);

	for (int i = 1; i <= nRecordTypeCount; i++)
	{
		CIWNistRecord* pRec = new CIWNistRecord;
		pRec->m_nRecordType = nRecordType;

		sLBText.Empty();

		IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);

		while (FieldNum)
		{
			IWNumSubfields(m_pIWTrans, nRecordType, i, FieldNum, &NumSubfields);

			for (Subfield = 1; Subfield <= NumSubfields; Subfield++)
			{
				sSubField.Format(NumSubfields > 1 ? _T("Subfield %d") : _T(""), Subfield);

				IWNumItems(m_pIWTrans, nRecordType, i, FieldNum, Subfield, &NumItems);

				sData = "";
				for (Item = 1; Item <= NumItems; Item++)
				{
					IWFindItem(m_pIWTrans, nRecordType, i, FieldNum, Subfield, Item, &Data);

					if(NumItems == 1)
					{
						sItemData = Data;
						sData += sItemData;
					}
					else
					{
						sItemData.Format(_T("Item %d = %s\n"), Item, CString(Data));
						sData += sItemData;
					}
				}

				sLBText.Format(_T("%d\t%s%s"), FieldNum, sSubField, sData);
				pRec->m_arrStrings.Add(sLBText);
			}

			IWGetNextField(m_pIWTrans, nRecordType, i, FieldNum, &FieldNum);
		}

		// remove trailing newline character as needed
		if (sLBText.Right(1).Compare(_T("\n")) == 0)
			sLBText.Delete(sLBText.GetLength()-1);

		// get image
		HGLOBAL hBMP = GetThumbnail(nRecordType, i);

		pRec->m_hDIB = hBMP;

		m_arrRecords.Add(pRec);

/*		// see if there is already a page for this image type
		CIWNistRecordPage* pPage = GetPageByImageType(nImageType);
		if(!pPage)
		{
			pPage = new CIWNistRecordPage();
			pPage->m_nRecordType = nRecordType;
			AddPage(pPage);

			TC_ITEM item;
			item.mask = TCIF_TEXT;
			switch(nImageType)
			{
				case 1:
					item.pszText = "Facial Records";
					break;
				case 2:
					item.pszText = "SMT Records";
					break;
				case 3:
					item.pszText = "Iris Records";
					break;
				default:
					item.pszText = "Other Records";
					break;
			}
			pPage->m_nImageType = nImageType;
			
			GetTabControl()->SetItem(GetPageIndex(pPage), &item);
		}

		int nPos = pPage->m_ctlList.m_Strings.Add(sLBText);
		pPage->m_ctlList.m_DIBs.Add(hBMP);

		nPos = pPage->m_ctlMugList.m_Strings.Add(sLBText);
		pPage->m_ctlMugList.m_DIBs.Add(hBMP);

		if(pPage->GetSafeHwnd()) pPage->FillListControl();
*/	}

	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::ReadType99Records()
{
	return ERROR_SUCCESS;
}

int CIWNISTEditorDoc::GetImageType(const char* szType)
{
	UINT nRet = -1;
	char *szImageType[] = { "FINGERPRINT", "FACE", "SMT", "IRIS" };
	int nTypeCount = sizeof(szImageType)/ sizeof(szImageType[0]);

	if (szType == NULL)return nRet;

	for(int i = 0; i < nTypeCount; i++)
	{
		if(_strcmpi(szType, szImageType[i]) == 0)
		{
			nRet = i;
			break;
		}
	}

	return nRet;
}


int CIWNISTEditorDoc::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

HGLOBAL CIWNISTEditorDoc::GetThumbnail(int nType, int nPos)
{
	if(!m_pIWTrans) return NULL;

	HGLOBAL hRet = NULL;
	char *pStorageFormat;
	char *pImageData = NULL;
	long lImageLen;
	char *pBMP;
	long nLength;

	if (IWGetImage(m_pIWTrans, nType, nPos, (const char**) &pStorageFormat, &lImageLen, (const void**) &pImageData) == 0) //IW_SUCCESS)
	{
		if (pImageData && lImageLen)
		{
			if (pStorageFormat && !strcmp(pStorageFormat, "raw"))
			{
				const char* Data;
				long nWidth, nHeight, nDepth;
				
				IWFindItem(m_pIWTrans, nType, nPos, 6, 1, 1, &Data);
				nWidth = atol(Data);
				
				IWFindItem(m_pIWTrans, nType, nPos, 7, 1, 1, &Data);
				nHeight = atol(Data);

				IWFindItem(m_pIWTrans, nType, nPos, 12, 1, 1, &Data);
				nDepth = atol(Data);

				if (RAWtoBMP(nWidth, nHeight, 500, nDepth, (BYTE*)pImageData, (BYTE**)&pBMP, &nLength) == 0)
				{
					return PackageAsHGLOBAL(pBMP, nLength);
				}
			}
			else if (pStorageFormat && !strcmp(pStorageFormat, "jpg"))
			{
				if (JPGtoBMP((BYTE*)pImageData, lImageLen, (BYTE**)&pBMP, &nLength) == 0)
				{
					return PackageAsHGLOBAL(pBMP, nLength);
				}
			}
			else if (pStorageFormat && !strcmp(pStorageFormat, "wsq"))
			{
				if (WSQtoBMP((BYTE*)pImageData, lImageLen, (BYTE**)&pBMP, &nLength) == 0)
				{
					return PackageAsHGLOBAL(pBMP, nLength);
				}
			}
			else if (pStorageFormat && !strcmp(pStorageFormat, "fx4"))
			{
				if (FX4toBMP((BYTE*)pImageData, lImageLen, (BYTE**)&pBMP, &nLength) == 0)
				{
					return PackageAsHGLOBAL(pBMP, nLength);
				}
			}
			else if (pStorageFormat && !strcmp(pStorageFormat, "jp2"))
			{
				if (JP2toBMP((BYTE*)pImageData, lImageLen, (BYTE**)&pBMP, &nLength) == 0)
				{
					return PackageAsHGLOBAL(pBMP, nLength);
				}
			}
		}
	}

	return NULL;
}

HGLOBAL CIWNISTEditorDoc::PackageAsHGLOBAL(char* p, long n)
{
	HGLOBAL hRet;
	hRet = GlobalAlloc(GPTR, n);
	char *ptr = (char*)GlobalLock(hRet);
	memcpy(ptr, p, n);
	GlobalUnlock(hRet);
	MemFree((BYTE*)p);

	return hRet;
}

CIWNistRecord* CIWNISTEditorDoc::GetRecord(int nIndex)
{
	// validate range
	if(nIndex < 0 || nIndex > m_arrRecords.GetUpperBound())
		return NULL;

	return (CIWNistRecord*) m_arrRecords.GetAt(nIndex);
}

void CIWNISTEditorDoc::DeleteContents()
{
	while(m_arrRecords.GetSize())
	{
		delete (CIWNistRecord*) m_arrRecords.GetAt(0);
		m_arrRecords.RemoveAt(0);
	}

	CDocument::DeleteContents();
}
