//////////////////////////////////////////////////
// Outlook2Ctrl.cpp : implementation file
//

#include "stdafx.h"
#include "Outlook2Ctrl.h"
#include <gdiplus.h>

// COutlook2Ctrl

	

void COutlook2Ctrl::DrawGradientRect(CDC * pDC, CRect &rect, COLORREF cr1, COLORREF cr2, DWORD dwStyle)
{
#if(WINVER >= 0x0500)
	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = rect.left;
	vert [0] .y      = rect.top;
	vert [0] .Red    = GetRValue(cr1)*0xFF00/255;
	vert [0] .Green  = GetGValue(cr1)*0xFF00/255;
	vert [0] .Blue   = GetBValue(cr1)*0xFF00/255;
	vert [0] .Alpha  = 0;

	vert [1] .x      = rect.right;
	vert [1] .y      = rect.bottom; 
	vert [1] .Red    = GetRValue(cr2)*0xFF00/255;
	vert [1] .Green  = GetGValue(cr2)*0xFF00/255;
	vert [1] .Blue   = GetBValue(cr2)*0xFF00/255;
	vert [1] .Alpha  = 0x0000;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;

	GradientFill(pDC->GetSafeHdc(),vert,2,&gRect,1,dwStyle);
#else
	pDC->FillSolidRect(rect, cr1);
#endif
}


IMPLEMENT_DYNAMIC(COutlook2Ctrl, OLCTRL2_BASE_CLASS)
COutlook2Ctrl::COutlook2Ctrl()
{
	m_iSize = AfxGetApp()->GetProfileInt(_T("Settings"),_T("OutbarSize"),200);
	m_iDragging = 0;
	m_iDragoffset = 0;
	hDragCur = AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR); // sometime fails .. 
	if (!hDragCur) hDragCur = AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(IDC_SIZEWE)); 

#if(WINVER >= 0x0500)
	hHandCur = LoadCursor(NULL, IDC_HAND);
#else
	hHandCur = LoadCursor(NULL, IDC_ARROW);
#endif

	LOGFONT lf;
	HFONT hf = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	CFont * gf = CFont::FromHandle(hf);
	gf->GetLogFont(&lf);
	strcpy_s(lf.lfFaceName, _T("Tahoma"));
	lf.lfUnderline = TRUE;
	ftHotItems.CreateFontIndirect(&lf);
	lf.lfUnderline = FALSE;

	ftItems.CreateFontIndirect(&lf);

	lf.lfWeight = FW_BOLD;
	ftFolders.CreateFontIndirect(&lf);

	lf.lfHeight = 20;
	ftCaption.CreateFontIndirect(&lf);


	m_csCaption = "";

	 
	SetupColors();

	m_iNumFoldersDisplayed = -1;
	m_iFolderHeight = 31;
	m_iItemHeight = 18;
	m_iSelectedFolder = 0;
	m_iSubItemHeight = 17;

	iHiFolder = iHiLink = -1;
	pLastHilink = NULL;
}

COutlook2Ctrl::~COutlook2Ctrl()
{
	for (int t = 0; t < m_Folders.GetSize(); t++)
	{
		COL2Folder * p = (COL2Folder *) m_Folders.GetAt(t);
		delete p;
	}
	m_Folders.RemoveAll();
	AfxGetApp()->WriteProfileInt(_T("Settings"),_T("OutbarSize"),m_iSize);
}

void COutlook2Ctrl::SetupColors()
{
	// classical 
	m_crBackCaption = GetSysColor(COLOR_BTNSHADOW);
	m_crTextCaption = GetSysColor(COLOR_WINDOW);
	m_crCmdOther = GetSysColor(COLOR_WINDOW);
	m_crDisabled = GetSysColor(COLOR_GRAYTEXT);

	m_crBackground = GetSysColor(COLOR_BTNSHADOW);
	m_crCmdLink = GetSysColor(COLOR_CAPTIONTEXT);


	// modern

/*	m_crBackCaption = GetSysColor(COLOR_BTNSHADOW);
	m_crTextCaption = GetSysColor(COLOR_WINDOW);
	m_crCmdOther = GetSysColor(COLOR_BTNTEXT);
	m_crDisabled = GetSysColor(COLOR_GRAYTEXT);

	m_crBackground = GetSysColor(COLOR_WINDOW);
	m_crCmdLink = GetSysColor(COLOR_HOTLIGHT);
*/
}


BEGIN_MESSAGE_MAP(COutlook2Ctrl, OLCTRL2_BASE_CLASS)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// COutlook2Ctrl message handlers


bool COutlook2Ctrl::Create(CWnd * pParent, int iId)
{
#ifdef __BCGCB_H
	if (OLCTRL2_BASE_CLASS::Create(pParent,WS_VISIBLE|WS_CHILD|CBRS_LEFT,iId))
#else
	if (CControlBar::Create(NULL, _T(""), WS_VISIBLE|WS_CHILD, CRect(0,0,0,0), pParent, iId))
#endif
	{
		SetOwner(pParent);
		return true;
	}
	return false;
}

void COutlook2Ctrl::OnPaint()
{
	ClearRects();
	CPaintDC pdc(this);
	COMemDC dc(&pdc);

	CRect rc;
	GetClientRect(rc);
	CRect rcSide(rc.right-4, rc.top, rc.right, rc.bottom);
	dc.FillSolidRect(rcSide, GetSysColor(COLOR_3DFACE));

	// if closed all the way, we're done
	if (m_iSize <= 4) return;

	rc.right -= 4;
	DrawCaption(&dc, rc);

	rc.top += 35;
	dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));

	CRect rcBdr(rc);
	rc.InflateRect(-1,-1);
	DrawButtons(&dc, rc);
	dc.Draw3dRect(rcBdr, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
	rcInnerRect = rc;
	if (m_iSelectedFolder >= 0 && m_iSelectedFolder < m_Folders.GetSize())
	{
		COL2Folder * o = (COL2Folder *) m_Folders.GetAt(m_iSelectedFolder);
		DrawItems(&dc, o, rc);
	}
}

void COutlook2Ctrl::AnimateToFolder(int f)
{
	m_iSelectedFolder = f;
 	pLastHilink = NULL;
	m_csCaption = GetFolder(f)->csName;

	if (m_iSize > 4)
	{

		CRect rc;
		GetClientRect(rc);

		CDC dc;
		CClientDC cdc(this);
		dc.CreateCompatibleDC(&cdc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&cdc, rc.Width(), rc.Height());
		CBitmap * ob = (CBitmap *) dc.SelectObject(&bmp);

		DrawCaption(&dc, rc);
		CRect rcBdr(rc);
		rc.InflateRect(-1,-1);
		DrawButtons(&dc, rc);
		int iBottomInner = rc.bottom;
		dc.Draw3dRect(rcBdr, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
		rcInnerRect = rc;
		dc.FillSolidRect(rc, m_crBackground);
		if (m_iSelectedFolder >= 0 && m_iSelectedFolder < m_Folders.GetSize())
		{
			COL2Folder * o = (COL2Folder *) m_Folders.GetAt(m_iSelectedFolder);
			DrawItems(&dc, o, rc);
		}

		for (int t = iBottomInner - 10; t > 0; t -= (iBottomInner/8))
		{
			cdc.BitBlt(0,t,rc.Width(), iBottomInner - t, &dc, 0,0, SRCCOPY);
			Sleep(30);
		}
		dc.SelectObject(ob);
	}

	Invalidate();
}

BOOL COutlook2Ctrl::OnEraseBkgnd(CDC* pDC)
{
#ifdef __BCGCB_H
	return TRUE;
#else
	return OLCTRL2_BASE_CLASS::OnEraseBkgnd(pDC);
#endif
}

int COutlook2Ctrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (OLCTRL2_BASE_CLASS::OnCreate(lpCreateStruct) == -1) return -1;
	SetBarStyle (CBRS_LEFT|CBRS_HIDE_INPLACE & ~(CBRS_BORDER_ANY | CBRS_GRIPPER));
	return 0;
}

CSize COutlook2Ctrl::CalcFixedLayout (BOOL /*bStretch*/, BOOL bHorz)
{
	CSize size = CSize (m_iSize, 32767);
	return size;
}

void COutlook2Ctrl::SetCurFolder(int f, bool bAnimation)
{
	// hide all subwindows
	if (m_iSelectedFolder >= 0 && m_iSelectedFolder != f)
	{
		for (int f = 0; f < m_Folders.GetSize(); f++)
		{
			COL2Folder * pf = (COL2Folder *) m_Folders.GetAt(f);
			for (int i = 0; i < pf->m_Items.GetSize(); i++)
			{
				COL2Item * pi = (COL2Item *) pf->m_Items.GetAt(i);
				for (int s = 0; s < pi->m_SubItems.GetSize(); s++)
				{
					COL2SubItem * ps = (COL2SubItem *) pi->m_SubItems.GetAt(s);
					if (ps->hHostedWnd) ::ShowWindow(ps->hHostedWnd, SW_HIDE);
					ps->rcItem.SetRectEmpty();
				}
			}
		}
	}

	if (m_iSelectedFolder != f)
	{
		if (bAnimation) AnimateToFolder(f);
		else
		{
			m_iSelectedFolder = f;
 			pLastHilink = NULL;
			m_csCaption = GetFolder(f)->csName;
		}
		Invalidate();
	}
}

void COutlook2Ctrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	
	if (m_iSize <= 4 || m_dragRect.PtInRect(point))
	{
		m_iDragoffset = m_iSize - point.x;

		m_iDragging = 1;
		SetCursor(hDragCur);
		SetCapture();
		SetFocus();
		CRect rc;
		GetClientRect(rc);
		OnInvertTracker(m_dragRect);
		return;
	}
	int f,i,s;
	int r = HitTest(f,i,s,point);
	if (r == 1)
	{
		SetCurFolder(f);
	}
	if (r == 4)
	{
		COL2SubItem	* ps = GetSubItem(f,i,s);
		if (ps->dwStyle == 2 || ps->dwStyle == 3) // item is hotlinked or checkbox
		{
			AfxGetMainWnd()->SendMessage(WM_COMMAND, MAKELONG(ps->lParam, 0), (LPARAM) GetSafeHwnd());
			InvalidateRect(rcInnerRect, FALSE);
		}
		if (ps->dwStyle == 0 || ps->dwStyle == 1) // item is radio or singlelist .. check before sending
		{
			COL2CCmdUI pui;
			pui.pSI = ps;
			pui.m_nID = ps->lParam; 
			AfxGetMainWnd()->OnCmdMsg(pui.m_nID, CN_UPDATE_COMMAND_UI, &pui, NULL);
			if (!(pui.iRes & 2)) // not checked yet .. so let's do it
				AfxGetMainWnd()->SendMessage(WM_COMMAND, MAKELONG(ps->lParam, 0), (LPARAM) GetSafeHwnd());
			InvalidateRect(rcInnerRect, FALSE);
		}
	}
	OLCTRL2_BASE_CLASS::OnLButtonDown(nFlags, point);
}

void COutlook2Ctrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_iDragging)
	{
		ReleaseCapture();
		OnInvertTracker(m_dragRect);
		m_iSize = point.x + m_iDragoffset;
		if (m_iSize < 4) m_iSize = 4;
		CFrameWnd* pParentFrame = GetParentFrame ();
		pParentFrame->RecalcLayout ();
	}
	m_iDragging = 0;

	OLCTRL2_BASE_CLASS::OnLButtonUp(nFlags, point);
}

void COutlook2Ctrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_iDragging == 1)
	{
		CRect rc1(m_dragRect);
		m_dragRect.SetRect(point.x-5 + m_iDragoffset, rc1.top, point.x + m_iDragoffset, rc1.bottom);
		if (rc1 != m_dragRect)
		{
			OnInvertTracker(rc1);
			OnInvertTracker(m_dragRect);
		}
	}
	else
	{
		int f,i,s;
		int r = HitTest(f,i,s,point);
		if (r == 1)
		{
			CClientDC dc(this); 
			if (iHiFolder >= 0 && iHiFolder != f)
			{
				DrawButton(&dc, (COL2Folder *) m_Folders.GetAt(iHiFolder), iHiFolder == m_iSelectedFolder, false);
			}
			if (iHiFolder != f)
			{
				iHiFolder = f;
				DrawButton(&dc, (COL2Folder *) m_Folders.GetAt(iHiFolder), iHiFolder == m_iSelectedFolder, true);
			}
		}
		if (r == 4)
		{
			COL2SubItem	* ps = GetSubItem(f,i,s);
			CClientDC dc(this); 
			if (pLastHilink && pLastHilink != ps)
			{
				DrawSubItem(&dc, GetFolder(iHilinkFolder), GetItem(iHilinkFolder, iHilinkItem), pLastHilink, false);
			}
			if (pLastHilink != ps)
			{
				pLastHilink = ps;
				DrawSubItem(&dc, GetFolder(f), GetItem(f,i), ps, true);
				iHilinkFolder = f;
				iHilinkItem = i;
			}
		}

		if (r == 0)
		{
			if (iHiFolder >= 0 && iHiFolder < m_Folders.GetSize())
			{
				CClientDC dc(this); 
				DrawButton(&dc, (COL2Folder *) m_Folders.GetAt(iHiFolder), iHiFolder == m_iSelectedFolder, false);
				iHiFolder = -1;
			}
			if (pLastHilink != NULL)
			{
				CClientDC dc(this); 
				DrawSubItem(&dc, GetFolder(iHilinkFolder), GetItem(iHilinkFolder, iHilinkItem), pLastHilink, false);
				pLastHilink = NULL;
			}
		}
	}

	OLCTRL2_BASE_CLASS::OnMouseMove(nFlags, point);
}

BOOL COutlook2Ctrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		CPoint point;
		::GetCursorPos (&point);
		ScreenToClient (&point);

		if (m_iDragging || m_dragRect.PtInRect(point)) 
		{
			SetCursor(hDragCur);
			return TRUE;
		}
		int f,i,s;
		int r = HitTest(f,i,s,point);
		if (r == 4)
		{
			COL2SubItem	* ps = GetSubItem(f,i,s);
			if (ps->dwStyle == 2) // item is hotlinked
			{
				SetCursor(hHandCur);
				return TRUE;
			}
		}
		if (r == 1)
		{
			SetCursor(hHandCur);
			return TRUE;
		}
	}

	return OLCTRL2_BASE_CLASS::OnSetCursor(pWnd, nHitTest, message);
}

void COutlook2Ctrl::OnInvertTracker(const CRect& rc)
{
	CFrameWnd* pParentFrame = GetParentFrame ();
	CDC* pDC = pParentFrame->GetDC();
	CRect rect(rc);
    ClientToScreen(rect);
	pParentFrame->ScreenToClient(rect);

	CBrush br;
	br.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
	HBRUSH hOldBrush = NULL;
	hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, br.m_hObject);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), DSTINVERT);
	if (hOldBrush != NULL) SelectObject(pDC->m_hDC, hOldBrush);
	ReleaseDC(pDC);

}

void COutlook2Ctrl::DrawCaption(CDC * pDC, CRect & rect)
{
	CRect rc(rect);
	pDC->Draw3dRect(rc.left, rc.top, rc.right, rc.top+2, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DDKSHADOW));

	rc.bottom = rc.top + 35;
	rc.top += 2;

	COLORREF cr1 = RGB(89,135,214); //GetSysColor(COLOR_GRADIENTACTIVECAPTION);
	COLORREF cr2 = RGB(3, 56, 147); //GetSysColor(COLOR_ACTIVECAPTION);
	DrawGradientRect(pDC, rc, cr1, cr2);

	rc.bottom -=2;
	COLORREF crSave = pDC->SetTextColor(m_crTextCaption);
	pDC->SetBkMode(TRANSPARENT);
	CFont * of = (CFont *) pDC->SelectObject(&ftCaption);
	rc.left += 8;
    pDC->DrawText(m_csCaption, rc, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);
	pDC->SelectObject(of);
	pDC->SetTextColor(crSave);
}


COL2Folder::COL2Folder()
{
	hIcon = NULL;
	dwStyle = 0;
	rcItem.SetRectEmpty();
}

COL2Folder::~COL2Folder()
{
	for (int t = 0; t < m_Items.GetSize(); t++)
	{
		COL2Item * p = (COL2Item *) m_Items.GetAt(t);
		delete p;
	}
	m_Items.RemoveAll();
}


COL2Item::COL2Item()
{
	dwStyle = 0;
	rcItem.SetRectEmpty();
}

COL2Item::~COL2Item()
{
	for (int t = 0; t < m_SubItems.GetSize(); t++)
	{
		COL2SubItem * p = (COL2SubItem *) m_SubItems.GetAt(t);
		delete p;
	}
	m_SubItems.RemoveAll();
}



COL2SubItem::COL2SubItem()
{
	hIcon = NULL;
	hHostedWnd = NULL;
	dwStyle = 0;
	rcItem.SetRectEmpty();
	iLastStatus = 0;
}

COL2SubItem::~COL2SubItem()
{
}

int COutlook2Ctrl::AddFolder(const char * m_strName, UINT iIcoID, DWORD lParam)
{
	HICON hIco = (HICON) LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(iIcoID),IMAGE_ICON,16,16,0);
	if(m_csCaption.IsEmpty()) m_csCaption = m_strName;

	COL2Folder * o = new COL2Folder;
	o->csName = m_strName;
	o->hIcon = hIco;
	o->lParam = lParam;
	return (int) m_Folders.Add(o);
}

int COutlook2Ctrl::AddFolderItem(const char * m_strItemName, DWORD dwStyle, int iFolder)
{
	if (iFolder < 0) iFolder = (int) m_Folders.GetSize() - 1;

	COL2Folder * o = (COL2Folder *) m_Folders.GetAt(iFolder);
	COL2Item * i = new COL2Item;

	i->csName = m_strItemName;
	i->dwStyle = dwStyle;
	return (int) o->m_Items.Add(i);
}

int COutlook2Ctrl::AddSubItem(const char * m_strName, int iIcoID, DWORD dwStyle, DWORD lParam, int iFolder, int iFolderItem)
{
	HICON hIco = (HICON) LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(iIcoID),IMAGE_ICON,16,16,0);
	return AddSubItem(m_strName, hIco, dwStyle, lParam, iFolder, iFolderItem);
}

int COutlook2Ctrl::AddSubItem(const char * m_strName, HICON hIcon, DWORD dwStyle, DWORD lParam, int iFolder, int iFolderItem)
{
	if (iFolder < 0) iFolder = (int) m_Folders.GetSize() - 1;
	COL2Folder * o = (COL2Folder *) m_Folders.GetAt(iFolder);

	if (iFolderItem < 0) iFolderItem = (int) o->m_Items.GetSize() - 1;
	COL2Item  * i = (COL2Item *) o->m_Items.GetAt(iFolderItem);

	COL2SubItem * s = new COL2SubItem;

	s->csName = m_strName;
	s->dwStyle = dwStyle;
	s->hIcon = hIcon;
	s->lParam = lParam;

	return (int) i->m_SubItems.Add(s);
}

int COutlook2Ctrl::AddSubItem(HWND hHosted, bool bStretch, int iFolder, int iFolderItem)
{
	ASSERT(hHosted);

	if (iFolder < 0) iFolder = (int) m_Folders.GetSize() - 1;
	COL2Folder * o = (COL2Folder *) m_Folders.GetAt(iFolder);

	if (iFolderItem < 0) iFolderItem = (int) o->m_Items.GetSize() - 1;
	COL2Item  * i = (COL2Item *) o->m_Items.GetAt(iFolderItem);

	COL2SubItem * s = new COL2SubItem;
	
	CRect rc;
	::GetWindowRect(hHosted, rc);

	s->dwStyle = OCL_HWND;
	s->lParam = rc.Height();
	s->hHostedWnd = hHosted;
	if (bStretch) s->lParam = 0xFFFFFFFF;

	return (int) i->m_SubItems.Add(s);
	
}


void COutlook2Ctrl::DrawButtons(CDC * pDC, CRect & rect)
{
	if (rect.bottom - m_iFolderHeight < rect.top) return;

	int by = rect.bottom;

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	for (int t = (int) m_Folders.GetSize() - 1; t >= 0; t--)
	{
		COL2Folder * o = (COL2Folder *) m_Folders.GetAt(t);
		o->rcItem.SetRect(rect.left+1,by - m_iFolderHeight, rect.right, by);
		pDC->FillSolidRect(o->rcItem.left,o->rcItem.top,o->rcItem.Width(), o->rcItem.Height() + 1, RGB(0, 45, 150));
		DrawButton(pDC, o, t == m_iSelectedFolder ? true : false, o->rcItem.PtInRect(pt) ? true : false);
		by -= (m_iFolderHeight + 1);
		rect.bottom -= (m_iFolderHeight + 1);
		if (rect.top >= rect.bottom - m_iFolderHeight) break;
	}
}

void COutlook2Ctrl::DrawButton(CDC * pDC, COL2Folder * o, bool bSel, bool bOver)
{
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	CFont * of = (CFont *) pDC->SelectObject(&ftFolders);

	CRect rect(o->rcItem);
	if (bOver)
	{
		//pDC->Draw3dRect(rect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
		//rect.InflateRect(-1,-1);
		//pDC->Draw3dRect(rect, GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW));
	}
	else
	{
		//rect.bottom --;
		//pDC->Draw3dRect(rect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
		//pDC->Draw3dRect(rect.left-1, rect.bottom,rect.Width()+1,1,GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
		//rect.InflateRect(-1,-1);
	}

	if (bSel) 
	{
#if(WINVER >= 0x0500)
		COLORREF cr1 = RGB(251,230,148); //GetSysColor(COLOR_GRADIENTACTIVECAPTION);
		COLORREF cr2 = RGB(238, 149, 21); //GetSysColor(COLOR_ACTIVECAPTION);
		DrawGradientRect(pDC, rect, cr1, cr2);
		pDC->SetTextColor(RGB(0, 0, 0)); //GetSysColor(COLOR_CAPTIONTEXT));
#else
		pDC->FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));//COLOR_ACTIVECAPTION));
		pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
#endif
	}
	else 
	{
#if(WINVER >= 0x0500)
		COLORREF cr1 = RGB(203,225,252); //GetSysColor(COLOR_GRADIENTINACTIVECAPTION);
		COLORREF cr2 = RGB(125,166,224); //GetSysColor(COLOR_INACTIVECAPTION);
		DrawGradientRect(pDC, rect, cr1, cr2);
		pDC->SetTextColor(RGB(0, 0, 0)); //GetSysColor(COLOR_INACTIVECAPTIONTEXT));
#else
		pDC->FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
#endif
	}

	if (o->hIcon && rect.left+20 < rect.right)
	{
		DrawIconEx(pDC->GetSafeHdc(), rect.left + 4, 
			rect.top + (rect.Height()/2) - 8, o->hIcon, 16, 16, 0, NULL, DI_NORMAL);
	}

	CRect rc1(rect);
	rc1.left += 24;
	pDC->DrawText(o->csName, rc1, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);

	pDC->SelectObject(of);
}

void COutlook2Ctrl::DrawItems(CDC * pDC, COL2Folder * oFolder, CRect & rect)
{
	if (rect.bottom - m_iItemHeight < rect.top) return;

	CFont * of = (CFont *) pDC->SelectObject(&ftItems);

	for (int t = 0; t < oFolder->m_Items.GetSize(); t++)
	{
		if (t != 0) rect.top += 7;

		COL2Item * i = (COL2Item *) oFolder->m_Items.GetAt(t);
		if (i->csName != "")
		{
			i->rcItem.SetRect(rect.left+1,rect.top, rect.right-1, rect.top + m_iItemHeight);
			rect.top += m_iItemHeight;
		}
		else
		{
			i->rcItem.SetRect(rect.left+1,rect.top+1, rect.right-1, rect.top + 3);
			rect.top += 8;
		}

		DrawItem(pDC, oFolder, i);
		DrawSubItems(pDC, oFolder, i, rect);

		if (rect.top >= rect.bottom - m_iItemHeight) break;
	}
	pDC->SelectObject(of);

}

void COutlook2Ctrl::DrawItem(CDC * pDC, COL2Folder * o, COL2Item * i)
{
	CRect rect(i->rcItem);
	if (i->csName != "")
	{
#if(WINVER >= 0x0500)
		COLORREF cr1 = RGB(203,225,252); //GetSysColor(COLOR_GRADIENTACTIVECAPTION);
		COLORREF cr2 = RGB(125,166,224); //GetSysColor(COLOR_ACTIVECAPTION);
		DrawGradientRect(pDC, rect, cr1, cr2);
#else
		pDC->FillSolidRect(rect, GetSysColor(COLOR_ACTIVECAPTION));
#endif
		//pDC->SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
		CRect rc1(rect);
		rc1.left += 4;
		pDC->DrawText(i->csName, rc1, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);
	}
	else
	{
#if(WINVER >= 0x0500)
		COLORREF cr1 = RGB(203,225,252); //GetSysColor(COLOR_GRADIENTACTIVECAPTION);
		COLORREF cr2 = RGB(125,166,224); //GetSysColor(COLOR_ACTIVECAPTION);
		//DrawGradientRect(pDC, rect, cr1, cr2, GRADIENT_FILL_RECT_H);
#else
		pDC->FillSolidRect(rect, GetSysColor(COLOR_ACTIVECAPTION));
#endif
	}
}

void COutlook2Ctrl::DrawSubItems(CDC * pDC, COL2Folder * oFolder, COL2Item * iItem, CRect & rect)
{
	if (rect.bottom - m_iSubItemHeight < rect.top) return;

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	
	for (int t1 = 0; t1 < iItem->m_SubItems.GetSize(); t1++)
	{
		COL2SubItem * p = (COL2SubItem *) iItem->m_SubItems.GetAt(t1);
		if (p->hHostedWnd)
		{
			int dy;
			if (p->lParam == 0xFFFFFFFF) dy = rect.Height(); else dy = (int) p->lParam;

			p->rcItem.SetRect(rect.left+1,rect.top,rect.right-1,rect.top+dy);

			::SetWindowPos(p->hHostedWnd, NULL,p->rcItem.left, p->rcItem.top, p->rcItem.Width(), p->rcItem.Height(), SWP_NOZORDER);
			if (!::IsWindowVisible(p->hHostedWnd)) ::ShowWindow(p->hHostedWnd, SW_SHOW);
			rect.top += dy;
		}
		else
		{
			p->rcItem.SetRect(rect.left+1,rect.top,rect.right-1,rect.top+m_iSubItemHeight);
			DrawSubItem(pDC, oFolder, iItem, p, p->rcItem.PtInRect(pt) ? true : false);
			rect.top += m_iSubItemHeight;
		}
		
		if (rect.top >= rect.bottom - m_iSubItemHeight) break;
	}
}

void COutlook2Ctrl::DrawSubItem(CDC * pDC, COL2Folder * oFolder, COL2Item * iItem, COL2SubItem * pSubItem, bool bOver)
{
	CRect rect(pSubItem->rcItem);
	CFont * of = (CFont *) pDC->SelectObject(&ftItems);
	pDC->SetBkColor(GetSysColor(COLOR_3DFACE));

	switch (pSubItem->dwStyle)
	{
	case OCL_SELECT:
		{
			//pDC->SetTextColor(m_crCmdOther);
			if (bOver)
			{
				pDC->SelectObject(&ftHotItems);
			}

			if (pSubItem->hIcon && rect.left+25 < rect.right)
			{
				if (pSubItem->hIcon) DrawIconEx(pDC->GetSafeHdc(), rect.left+8, rect.top+1, pSubItem->hIcon,16,16,0,NULL, DI_NORMAL);
			}
			rect.left += 28;
			
			if (pSubItem->lParam)
			{
				COL2CCmdUI pui;
				pui.pSI = pSubItem;
				pui.m_nID = pSubItem->lParam;
				GetOwner()->OnCmdMsg(pui.m_nID, CN_UPDATE_COMMAND_UI, &pui, NULL);
				if (pui.iRes&2) pDC->FillSolidRect(rect, GetSysColor(COLOR_3DFACE)); // checked
				pSubItem->iLastStatus = pui.iRes;
			}

			pDC->DrawText(pSubItem->csName, rect, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);
		}
		break;
	case OCL_RADIO:
		{
			//pDC->SetTextColor(m_crCmdOther);

			DWORD dwHotStyle = 0;

			if (bOver) 
			{
#if(WINVER >= 0x0500)
				//dwHotStyle = DFCS_HOT;
#endif
				pDC->SelectObject(&ftHotItems);
			}
			CRect rci(rect.left+8, rect.top+2, rect.left + 20, rect.top + 14);
			if (pSubItem->lParam)
			{
				COL2CCmdUI pui;
				pui.pSI = pSubItem;
				pui.m_nID = pSubItem->lParam; 
				GetOwner()->OnCmdMsg(pui.m_nID, CN_UPDATE_COMMAND_UI, &pui, NULL);
				if (!(pui.iRes&2)) pDC->DrawFrameControl(rci, DFC_BUTTON, DFCS_BUTTONRADIO|DFCS_FLAT|dwHotStyle);
				else pDC->DrawFrameControl(rci, DFC_BUTTON, DFCS_BUTTONRADIO|DFCS_FLAT|DFCS_CHECKED|dwHotStyle);
				pSubItem->iLastStatus = pui.iRes;
			}
			else pDC->DrawFrameControl(rci, DFC_BUTTON, DFCS_BUTTONRADIO|DFCS_FLAT|dwHotStyle);
			rect.left += 28;

			pDC->DrawText(pSubItem->csName, rect, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);
		}
		break;
	case OCL_COMMAND:
		{
			//pDC->SetTextColor(m_crCmdLink);
			rect.left += 28;

			if (bOver) 
				pDC->SelectObject(&ftHotItems);

			if (pSubItem->lParam)
			{
				COL2CCmdUI pui;
				pui.pSI = pSubItem;
				pui.m_nID = pSubItem->lParam;
				GetOwner()->OnCmdMsg(pui.m_nID, CN_UPDATE_COMMAND_UI, &pui, NULL);
				pSubItem->iLastStatus = pui.iRes;
			}

			pDC->DrawText(pSubItem->csName, rect, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);

		}
		break;
	case OCL_CHECK: // checkbox
		{
			//pDC->SetTextColor(m_crCmdOther);

			DWORD dwHotStyle = 0;
			if (bOver) 
			{
#if(WINVER >= 0x0500)
				//dwHotStyle = DFCS_HOT;
#endif
				pDC->SelectObject(&ftHotItems);
			}

			CRect rci(rect.left+8, rect.top+2, rect.left + 20, rect.top + 14);
			if (pSubItem->lParam)
			{
				COL2CCmdUI pui;
				pui.pSI = pSubItem;
				pui.m_nID = pSubItem->lParam; 
				GetOwner()->OnCmdMsg(pui.m_nID, CN_UPDATE_COMMAND_UI, &pui, NULL);
				if (!(pui.iRes&2)) pDC->DrawFrameControl(rci, DFC_BUTTON, DFCS_BUTTONCHECK|DFCS_FLAT|dwHotStyle);
				else pDC->DrawFrameControl(rci, DFC_BUTTON, DFCS_BUTTONCHECK|DFCS_FLAT|DFCS_CHECKED|dwHotStyle);
				pSubItem->iLastStatus = pui.iRes;
			}
			else pDC->DrawFrameControl(rci, DFC_BUTTON, DFCS_BUTTONCHECK|DFCS_FLAT|dwHotStyle);
			rect.left += 28;

			pDC->DrawText(pSubItem->csName, rect, DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);
		}
		break;
	}
	pDC->SelectObject(of);
}


void COutlook2Ctrl::OnSize(UINT nType, int cx, int cy)
{
	OLCTRL2_BASE_CLASS::OnSize(nType, cx, cy);
	m_dragRect.SetRect(cx-5, 0, cx, cy);
}

int COutlook2Ctrl::HitTest(int & iFolder, int & iItem, int & iSubItem, CPoint point)
{
	for (iFolder = 0; iFolder < m_Folders.GetSize(); iFolder++)
	{
		COL2Folder * pf = (COL2Folder *) m_Folders.GetAt(iFolder);
		if (pf->rcItem.PtInRect(point)) return 1;
		
		for (iItem = 0; iItem < pf->m_Items.GetSize(); iItem++)
		{
			COL2Item * pi = (COL2Item *) pf->m_Items.GetAt(iItem);
			if (pi->rcItem.PtInRect(point)) return 2;

			for (iSubItem = 0; iSubItem < pi->m_SubItems.GetSize(); iSubItem++)
			{
				COL2SubItem * ps = (COL2SubItem *) pi->m_SubItems.GetAt(iSubItem);
				if (ps->rcItem.PtInRect(point)) return 4;
			}
		}
	}
	return 0;
}

void COutlook2Ctrl::ClearRects(void)
{
	for (int f = 0; f < m_Folders.GetSize(); f++)
	{
		COL2Folder * pf = (COL2Folder *) m_Folders.GetAt(f);
		pf->rcItem.SetRectEmpty();
		for (int i = 0; i < pf->m_Items.GetSize(); i++)
		{
			COL2Item * pi = (COL2Item *) pf->m_Items.GetAt(i);
			pi->rcItem.SetRectEmpty();
			for (int s = 0; s < pi->m_SubItems.GetSize(); s++)
			{
				COL2SubItem * ps = (COL2SubItem *) pi->m_SubItems.GetAt(s);
				ps->rcItem.SetRectEmpty();
			}
		}
	}
}

COL2SubItem * COutlook2Ctrl::GetSubItem(int f, int i, int s)
{
	COL2Item   * pi = (COL2Item *)   GetFolder(f)->m_Items.GetAt(i);
    COL2SubItem *ps = (COL2SubItem *) pi->m_SubItems.GetAt(s);
	return ps;
}

COL2Folder * COutlook2Ctrl::GetFolder(int f)
{
	return (COL2Folder *) m_Folders.GetAt(f);
}

COL2Item * COutlook2Ctrl::GetItem(int f, int i)
{
	return (COL2Item *) GetFolder(f)->m_Items.GetAt(i);
}

void COutlook2Ctrl::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	if (m_iSelectedFolder < 0) return;

	COL2Folder * oFolder = GetFolder(m_iSelectedFolder);
	for (int i = 0; i < oFolder->m_Items.GetSize(); i++)
	{
		COL2Item * pi = (COL2Item *) oFolder->m_Items.GetAt(i);
		for (int s = 0; s < pi->m_SubItems.GetSize(); s++)
		{
			COL2SubItem * ps = (COL2SubItem *) pi->m_SubItems.GetAt(s);
			if (ps->dwStyle == OCL_SELECT || ps->dwStyle == OCL_RADIO || ps->dwStyle == OCL_CHECK)
			{
				COL2CCmdUI pui;
				pui.pSI = ps;
				pui.m_nID = ps->lParam; 
				GetOwner()->OnCmdMsg(pui.m_nID, CN_UPDATE_COMMAND_UI, &pui, NULL);
				if (pui.iRes != ps->iLastStatus && !ps->rcItem.IsRectEmpty())
				{
                    InvalidateRect(ps->rcItem);					
				}
			}
		}
	}

/*	iLastStatus = pui.iRes;

	TRACE1("%d\n", (int) GetTickCount());
	CToolBar b;
	b.OnUpdateCmdUI(*/
}


void COutlook2Ctrl::Empty()
{
	for (int t = 0; t < m_Folders.GetSize(); t++)
	{
		COL2Folder * p = (COL2Folder *) m_Folders.GetAt(t);
		delete p;
	}
	m_Folders.RemoveAll();
}