// OpenEBTSEditorView.cpp : implementation of the COpenEBTSEditorView class
//

#include "stdafx.h"
#include "OpenEBTSEditor.h"

#include "OpenEBTSEditorDoc.h"
#include "OpenEBTSEditorView.h"
#include "IWOutlookBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COpenEBTSEditorView

IMPLEMENT_DYNCREATE(COpenEBTSEditorView, CScrollView)

BEGIN_MESSAGE_MAP(COpenEBTSEditorView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_MESSAGE(WM_UPDATE_IMAGE, &COpenEBTSEditorView::OnUpdateImage)
	ON_MESSAGE(WM_UPDATE_FIELD, &COpenEBTSEditorView::OnUpdateField)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_ZOOM_100, &COpenEBTSEditorView::OnZoom100)
	ON_COMMAND(ID_ZOOM_TOFIT, &COpenEBTSEditorView::OnZoomTofit)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_100, &COpenEBTSEditorView::OnUpdateZoom100)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_TOFIT, &COpenEBTSEditorView::OnUpdateZoomTofit)
	ON_COMMAND(ID_VIEW_IMAGEDETAILS, &COpenEBTSEditorView::OnViewImagedetails)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMAGEDETAILS, &COpenEBTSEditorView::OnUpdateViewImagedetails)
//	ON_WM_DESTROY()
ON_WM_SIZE()
END_MESSAGE_MAP()

// COpenEBTSEditorView construction/destruction

COpenEBTSEditorView::COpenEBTSEditorView()
{
	m_pRecord = NULL;
	m_bStretchToFit = FALSE;
	m_pImageDetailsDialog = NULL;
	m_pGridCtrl = NULL;
}

COpenEBTSEditorView::~COpenEBTSEditorView()
{
	if(m_pGridCtrl) delete m_pGridCtrl;

	if (m_pImageDetailsDialog)
	{
		m_pImageDetailsDialog->DestroyWindow();
		delete m_pImageDetailsDialog;
		m_pImageDetailsDialog = NULL;
	}
}

BOOL COpenEBTSEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CScrollView::PreCreateWindow(cs);
}

// COpenEBTSEditorView drawing

void COpenEBTSEditorView::OnDraw(CDC* pDC)
{
	COpenEBTSEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (!pDoc) return;

	CRect rc;
	GetClientRect(&rc);

	// draw image
	if(m_pRecord)
	{
		HGLOBAL hBMP = (HGLOBAL) m_pRecord->m_hDIB;
		if(hBMP)
		{
			LPBITMAPFILEHEADER lpBMIFile = (LPBITMAPFILEHEADER) GlobalLock(hBMP);
			if(!lpBMIFile) return;

			LPBITMAPINFOHEADER lpBMI = (LPBITMAPINFOHEADER)((LPBYTE)lpBMIFile + sizeof(BITMAPFILEHEADER)); 
			if(!lpBMI) return;

			LPBYTE lpBits = (LPBYTE)lpBMIFile + lpBMIFile->bfOffBits;

			CRect rcImage;
			
			// draw background
			rcImage.CopyRect(&rc);
			rcImage.right = max(rc.Width(), lpBMI->biWidth);
			rcImage.bottom = max(rc.Height(), lpBMI->biHeight);
			pDC->FillSolidRect(&rcImage, RGB(238,242,255));

			if(m_bStretchToFit)
			{
				rcImage.CopyRect(&rc);
				double ratio = (double) lpBMI->biWidth / (double)(abs(lpBMI->biHeight));
				if(lpBMI->biHeight <= lpBMI->biWidth)
				{
					rcImage.bottom = rcImage.top + (int)(rcImage.Width() / ratio);
					if(rcImage.bottom > rc.bottom)
					{
						rcImage.bottom = rc.bottom;
						rcImage.right = rcImage.left + (int)(rcImage.Height() * ratio);
					}
				}
				else
				{
					rcImage.right = rcImage.left + (int)(rcImage.Height() * ratio);
				}
				rcImage.DeflateRect(5,5,5,5);
			}
			else
			{
				rcImage.SetRect(0, 0, lpBMI->biWidth, lpBMI->biHeight);
			}

			::SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
			::StretchDIBits(pDC->GetSafeHdc(), rcImage.left, rcImage.top, rcImage.Width(), rcImage.Height(),
				0, 0, lpBMI->biWidth, lpBMI->biHeight, lpBits, (BITMAPINFO *)lpBMI, DIB_RGB_COLORS, SRCCOPY);

			pDC->SelectStockObject(NULL_BRUSH);
			pDC->Rectangle(&rcImage);

			SetScrollSizes(MM_TEXT, CSize(rcImage.Width(), rcImage.Height()));

			GlobalUnlock(hBMP);
		}
	}
	else
	{
		// draw background
		pDC->FillSolidRect(&rc, RGB(238,242,255));

		SetScrollSizes(MM_TEXT, CSize(0,0));
	}
}


// COpenEBTSEditorView printing

BOOL COpenEBTSEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void COpenEBTSEditorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void COpenEBTSEditorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// COpenEBTSEditorView diagnostics

#ifdef _DEBUG
void COpenEBTSEditorView::AssertValid() const
{
	CScrollView::AssertValid();
}

void COpenEBTSEditorView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

COpenEBTSEditorDoc* COpenEBTSEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COpenEBTSEditorDoc)));
	return (COpenEBTSEditorDoc*)m_pDocument;
}
#endif //_DEBUG

// COpenEBTSEditorView message handlers

void COpenEBTSEditorView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT, CSize(0,0));

	COpenEBTSEditorDoc* pDoc = GetDocument();
	CIWOutlookBar* pOutBar = (CIWOutlookBar*) GetParent()->SendMessage(WM_GET_OUTLOOKBAR);
	pOutBar->Empty();

	if(m_pImageDetailsDialog)
	{
		delete m_pImageDetailsDialog;
		m_pImageDetailsDialog = NULL;
	}

	if (m_pGridCtrl == NULL)
	{
		// Create the Gridctrl object
		m_pGridCtrl = new CGridCtrl;
		if (!m_pGridCtrl) return;

		// Create the Gridctrl window
		CRect rect;
		GetClientRect(rect);
		m_pGridCtrl->Create(CRect(0, 0, 100, 100), this, IDC_GRID, WS_CHILD | WS_VISIBLE);
		m_pGridCtrl->SetBkColor(RGB(238,242,255));

		// fill it up with stuff
		m_pGridCtrl->SetEditable(FALSE);
		m_pGridCtrl->EnableDragAndDrop(FALSE);

		m_pGridCtrl->SetRowResize(FALSE);
		m_pGridCtrl->SetColumnResize(FALSE);

		try {
			m_pGridCtrl->SetRowCount(1);
			m_pGridCtrl->SetColumnCount(1);
			m_pGridCtrl->SetFixedRowCount(1);
			m_pGridCtrl->SetFixedColumnCount(1);
		}
		catch (CMemoryException* e)
		{
			e->ReportError();
			e->Delete();
			return;
		}
	}

	for(int i = 0; i < pDoc->m_arrRecords.GetSize(); i++)
	{
		COpenEBTSRecord* pRecord = (COpenEBTSRecord*) pDoc->m_arrRecords.GetAt(i);
		pOutBar->AddRecord(pRecord->m_nRecordType, pRecord);
	}
	
	pOutBar->SetCurFolder(0);
	pOutBar->Invalidate();
	Invalidate();
}

void COpenEBTSEditorView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
/*

	pOutBar->AddFolder("Descriptive Record", IDI_ICON2);
    pOutBar->AddFolder("Faces", IDI_ICON3);
	pOutBar->AddFolder("Fingerprints", IDI_ICON3);
	pOutBar->AddFolder("Irises", IDI_ICON3);

        pOutBar->AddFolderItem("Categories");
            pOutBar->AddSubItem("Friends", hIco1, 
                 COutlook2Ctrl::OCL_SELECT, ID_CATEGORIES_FRIEND);
            pOutBar->AddSubItem("Private", hIco2, 
                 COutlook2Ctrl::OCL_SELECT, ID_CATEGORIES_PRIVATE);
            pOutBar->AddSubItem("Work", hIco3, 
                 COutlook2Ctrl::OCL_SELECT, ID_CATEGORIES_WORK);

        pOutBar->AddFolderItem("View Style");
            pOutBar->AddSubItem("Grid", NULL, 
                 COutlook2Ctrl::OCL_RADIO, ID_VIEW_GRID);
            pOutBar->AddSubItem("List", NULL, 
                 COutlook2Ctrl::OCL_RADIO, ID_VIEW_LIST);
            pOutBar->AddSubItem("Cards", NULL, 
                 COutlook2Ctrl::OCL_RADIO, ID_VIEW_CARDS);

        pOutBar->AddFolderItem("");
            pOutBar->AddSubItem("Name", 
                 NULL, COutlook2Ctrl::OCL_CHECK, ID_FIELDS_NAME);
            pOutBar->AddSubItem("Phone", 
                 NULL, COutlook2Ctrl::OCL_CHECK, ID_FIELDS_PHONE);
            pOutBar->AddSubItem("Address", 
                 NULL, COutlook2Ctrl::OCL_CHECK, ID_FIELDS_ADDRESS);

        pOutBar->AddFolderItem("");
            pOutBar->AddSubItem("Create new contact ..", 
                 NULL, COutlook2Ctrl::OCL_COMMAND, ID_TEST_CREATENEWCONTACT);
            pOutBar->AddSubItem("Import contacts ..", 
                 NULL, COutlook2Ctrl::OCL_COMMAND, ID_TEST_IMPORTCONTACT);
*/
}

LRESULT COpenEBTSEditorView::OnUpdateField(WPARAM wParam, LPARAM lParam)
{
	COpenEBTSField* pField = (COpenEBTSField*)lParam;
	
	m_pRecord = NULL;
	Invalidate();
	ScrollToPosition(CPoint(0,0));

	if(!m_pGridCtrl) return 0;
	
	m_pGridCtrl->ShowWindow(SW_SHOW);

	if (m_pImageDetailsDialog)
		m_pImageDetailsDialog->ShowWindow(SW_HIDE);


	if(pField)
	{
		GV_ITEM Item;
		Item.mask = GVIF_TEXT|GVIF_FORMAT;
		Item.row = Item.col = 0;
		Item.strText.Format(_T("Field %d"), pField->m_ID);
		Item.nFormat = DT_LEFT|DT_WORDBREAK;
		m_pGridCtrl->SetItem(&Item);

		LOGFONT lf;
		memcpy(&lf, m_pGridCtrl->GetItemFont(0,0), sizeof(LOGFONT));
		lf.lfWeight = 700;
		m_pGridCtrl->SetItemFont(0, 0, &lf);

		m_pGridCtrl->SetRowCount(1);
		m_pGridCtrl->SetColumnCount(1);
		m_pGridCtrl->SetColumnWidth(0, 72);
		m_pGridCtrl->SetRowHeight(0, CELL_HEIGHT);

		for(int nSubfield = 0; nSubfield < pField->m_SubFields.GetCount(); nSubfield++)
		{
			COpenEBTSSubfield* pSubfield = pField->m_SubFields.GetAt(nSubfield);

			if(m_pGridCtrl->GetRowCount() <= nSubfield + 1)
			{
				m_pGridCtrl->SetRowCount(m_pGridCtrl->GetRowCount() + 1);
				m_pGridCtrl->SetRowHeight(nSubfield + 1, CELL_HEIGHT);

				Item.row = nSubfield + 1;
				Item.col = 0;
				Item.strText.Format(_T("Subfield %d"), nSubfield + 1);
				m_pGridCtrl->SetItem(&Item);
			}

			for(int nItem = 0; nItem < pSubfield->m_Items.GetCount(); nItem++)
			{
				if(m_pGridCtrl->GetColumnCount() <= nItem + 1)
				{
					m_pGridCtrl->SetColumnCount(m_pGridCtrl->GetColumnCount() + 1);
					Item.row = 0;
					Item.col = nItem + 1;
					Item.strText.Format(_T("Item %d"), nItem + 1);
					m_pGridCtrl->SetItem(&Item);			
					m_pGridCtrl->SetColumnWidth(Item.col, 144);
				}
		
				m_pGridCtrl->SetItemText(nSubfield + 1, nItem + 1, pSubfield->m_Items.GetAt(nItem));
				m_pGridCtrl->SetItemState(nSubfield + 1, nItem + 1, m_pGridCtrl->GetItemState(nSubfield + 1, nItem + 1) | GVIS_READONLY);
			}
		}

		CRect rc;
		GetClientRect(&rc);
		m_pGridCtrl->MoveWindow(10, 10, min(m_pGridCtrl->GetColumnCount() * 144 - 68, rc.Width()-20), m_pGridCtrl->GetRowCount() * CELL_HEIGHT + 4);

		for(int i = 0; i < m_pGridCtrl->GetColumnCount(); i++)
		{
			if(!m_pGridCtrl->IsCellVisible(0, i))
			{
				m_pGridCtrl->MoveWindow(10, 10, min(m_pGridCtrl->GetColumnCount() * 144 - 68,
					rc.Width()-20), m_pGridCtrl->GetRowCount() * CELL_HEIGHT + 4	+ GetSystemMetrics(SM_CYVSCROLL));
				break;
			}
		}
	}

	return ERROR_SUCCESS;
}

LRESULT COpenEBTSEditorView::OnUpdateImage(WPARAM wParam, LPARAM lParam)
{
	m_pRecord = (COpenEBTSRecord*) lParam;

	if(m_pRecord)
	{
		if(!m_pImageDetailsDialog)
		{
			m_pImageDetailsDialog = new CImageDetailsDialog(this);
			CRect rcWnd, rcClient;
			AfxGetMainWnd()->GetClientRect(&rcClient);
			m_pImageDetailsDialog->GetWindowRect(&rcWnd);

			m_pImageDetailsDialog->SetWindowPos(&CWnd::wndBottom, 
				rcClient.right/* - rcWnd.Width() */, rcClient.bottom/* - rcWnd.Height()*/,
				0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		}

		if(m_pImageDetailsDialog)
		{
			m_pImageDetailsDialog->ShowWindow(SW_SHOW);
			m_pImageDetailsDialog->UpdateList(&m_pRecord->m_arrStrings);
			theApp.m_pMainWnd->SetWindowPos(&CWnd::wndTop, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}

	if (m_pGridCtrl)
		m_pGridCtrl->ShowWindow(SW_HIDE);
	
	Invalidate();
	
	return 0;
}
void COpenEBTSEditorView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
#if 0
	CFileDialog dlg(TRUE);

	if(dlg.DoModal() == IDCANCEL) return;

	Graphics g(GetSafeHwnd());
	Bitmap bitmap(dlg.GetPathName());
	
	g.DrawImage(&bitmap, 10, 10);
#endif 

	CScrollView::OnLButtonDblClk(nFlags, point);
}

void COpenEBTSEditorView::OnZoom100()
{
	m_bStretchToFit = FALSE;
	Invalidate();
}

void COpenEBTSEditorView::OnUpdateZoom100(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pRecord != NULL);
	pCmdUI->SetCheck(m_bStretchToFit ==  FALSE);
}

void COpenEBTSEditorView::OnZoomTofit()
{
	m_bStretchToFit = TRUE;
	Invalidate();
}


void COpenEBTSEditorView::OnUpdateZoomTofit(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pRecord != NULL);
	pCmdUI->SetCheck(m_bStretchToFit ==  TRUE);
}

void COpenEBTSEditorView::OnViewImagedetails()
{
	BOOL bVisible = m_pImageDetailsDialog &&
		m_pImageDetailsDialog->IsWindowVisible();

	m_pImageDetailsDialog->ShowWindow(bVisible ? SW_HIDE : SW_SHOW);
}

void COpenEBTSEditorView::OnUpdateViewImagedetails(CCmdUI *pCmdUI)
{
	if(m_pRecord)
	{
		BOOL bVisible = m_pImageDetailsDialog &&
			m_pImageDetailsDialog->IsWindowVisible();

		pCmdUI->SetCheck(bVisible);
	}

	pCmdUI->Enable(m_pRecord != NULL);
}


void COpenEBTSEditorView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	for(int i = 0; m_pGridCtrl && i < m_pGridCtrl->GetColumnCount(); i++)
	{
		if(!m_pGridCtrl->IsCellVisible(0, i))
		{
			m_pGridCtrl->MoveWindow(10, 10, min(m_pGridCtrl->GetColumnCount() * 144 - 68,
				cx - 20), m_pGridCtrl->GetRowCount() * CELL_HEIGHT + 4	+ GetSystemMetrics(SM_CYVSCROLL));
			break;
		}
	}
}
