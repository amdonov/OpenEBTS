// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "OpenEBTSEditor.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_MYOUTBAR 100

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_MESSAGE(WM_GET_OUTLOOKBAR, OnGetOutlookBar)
	ON_MESSAGE(WM_UPDATE_IMAGE, OnUpdateImage)
	ON_MESSAGE(WM_UPDATE_FIELD, OnUpdateField)
	ON_MESSAGE(WM_UPDATE_STATUS, OnUpdateStatus)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_VIEW_IMAGETOOLBAR, &CMainFrame::OnViewImagetoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMAGETOOLBAR, &CMainFrame::OnUpdateViewImagetoolbar)
	ON_WM_DESTROY()
	ON_COMMAND(ID_NEXT_FOLDER, &CMainFrame::OnNextFolder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_pImageList = NULL;
	m_pDisabledImageList = NULL;
	m_pImageList2 = NULL;
	m_pDisabledImageList2 = NULL;
}

CMainFrame::~CMainFrame()
{
	if(m_pDisabledImageList) delete m_pDisabledImageList;
	if(m_pDisabledImageList2) delete m_pDisabledImageList2;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// enable dockings only on the other 3 sides
	EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM|CBRS_ALIGN_RIGHT);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | 
		CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM|CBRS_ALIGN_RIGHT);
	DockControlBar(&m_wndToolBar, AFX_IDW_DOCKBAR_TOP);

	CBitmap bmpEnabled, bmpDisabled; 
	bmpEnabled.LoadBitmap(IDB_TB_ENABLED); 
	bmpDisabled.LoadBitmap (IDB_TB_DISABLED); 

	m_pImageList = m_wndToolBar.GetToolBarCtrl().GetImageList(); 
	m_pImageList->DeleteImageList(); 
	m_pImageList->Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 10); 
	m_pImageList->Add(&bmpEnabled, RGB(255,0,255)); 

	m_pDisabledImageList = new CImageList; 
	m_pDisabledImageList->Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 10); 
	m_pDisabledImageList->Add (&bmpDisabled, RGB(255,0,255)); 

	m_wndToolBar.GetToolBarCtrl().SetImageList(m_pImageList); 
	m_wndToolBar.GetToolBarCtrl().SetDisabledImageList (m_pDisabledImageList); 

	if (!m_wndImageToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | 
		CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndImageToolBar.LoadToolBar(IDR_IMAGETOOLS))
	{
		TRACE0("Failed to create image toolbar\n");
		return -1;      // fail to create
	}
	m_wndImageToolBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM|CBRS_ALIGN_RIGHT);
	DockControlBarLeftOf(&m_wndImageToolBar, &m_wndToolBar);

	CBitmap bmpEnabled2, bmpDisabled2; 
	bmpEnabled2.LoadBitmap(IDB_TB2_ENABLED); 
	bmpDisabled2.LoadBitmap (IDB_TB2_DISABLED); 

	m_pImageList2 = m_wndImageToolBar.GetToolBarCtrl().GetImageList(); 
	m_pImageList2->DeleteImageList(); 
	m_pImageList2->Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 10); 
	m_pImageList2->Add(&bmpEnabled2, RGB(255,0,255)); 

	m_pDisabledImageList2 = new CImageList; 
	m_pDisabledImageList2->Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 10); 
	m_pDisabledImageList2->Add (&bmpDisabled2, RGB(255,0,255)); 

	m_wndImageToolBar.GetToolBarCtrl().SetImageList(m_pImageList2); 
	m_wndImageToolBar.GetToolBarCtrl().SetDisabledImageList (m_pDisabledImageList2); 

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CreateOutBar();

	return 0;
}
void CMainFrame::DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf)
{
	CRect rect;
	DWORD dw;
	UINT n;

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=LeftOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}

int CMainFrame::CreateOutBar()
{
    // create the control
    m_wndOutBar.Create(this, ID_MYOUTBAR);

	return 0;
}

LRESULT CMainFrame::OnGetOutlookBar(WPARAM wpUnused, LPARAM lpUnused)
{
	return (LRESULT) &m_wndOutBar;
}
LRESULT CMainFrame::OnUpdateField(WPARAM nRecordType, LPARAM pRecord)
{
	GetActiveView()->PostMessage(WM_UPDATE_FIELD, nRecordType, pRecord);
	return 0;
}

LRESULT CMainFrame::OnUpdateImage(WPARAM nRecordType, LPARAM pRecord)
{
	GetActiveView()->PostMessage(WM_UPDATE_IMAGE, nRecordType, pRecord);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFrameWnd::PreCreateWindow(cs);
}

#ifdef _DEBUG

void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	for (UINT iFile = 0; iFile < nFiles; iFile++)
	{
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);
		pApp->OpenDocumentFile(szFileName);
	}
	::DragFinish(hDropInfo);

	//CFrameWnd::OnDropFiles(hDropInfo);
}

void CMainFrame::OnViewImagetoolbar()
{
	BOOL bVisible = m_wndImageToolBar.IsWindowVisible();
	m_wndImageToolBar.ShowWindow(bVisible ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewImagetoolbar(CCmdUI *pCmdUI)
{
	BOOL bVisible = m_wndImageToolBar.IsWindowVisible();
	pCmdUI->SetCheck(bVisible);
}

LRESULT CMainFrame::OnUpdateStatus(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR szText = (LPCTSTR) lParam;
	m_wndStatusBar.SetPaneText(0, szText);
	return ERROR_SUCCESS;
}


void CMainFrame::OnDestroy()
{
	m_wndOutBar.Empty();

	CFrameWnd::OnDestroy();
}

void CMainFrame::OnNextFolder()
{
	int nNewSel = m_wndOutBar.GetCurFolder() + 1;
	if (nNewSel > m_wndOutBar.GetFolderCount() - 1) nNewSel = 0;
}
