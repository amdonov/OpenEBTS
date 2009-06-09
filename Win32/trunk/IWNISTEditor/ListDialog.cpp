// ListDialog.cpp : implementation file
//

#include "stdafx.h"
#include "IWNISTEditor.h"
#include "ListDialog.h"


// CListDialog dialog

IMPLEMENT_DYNAMIC(CListDialog, CDialog)

CListDialog::CListDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CListDialog::IDD, pParent)
{

}

CListDialog::~CListDialog()
{
}

void CListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctlList);
}


BEGIN_MESSAGE_MAP(CListDialog, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CListDialog::OnSelectionChanged)
END_MESSAGE_MAP()


// CListDialog message handlers

void CListDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (m_ctlList.GetSafeHwnd())
	{
		m_ctlList.MoveWindow(4, 0, cx-8, cy-8);

		CRect rc;
		m_ctlList.GetClientRect(&rc);
		m_ctlList.SetColumnWidth(0, (int) rc.Width());
	}

	Invalidate();
}

void CListDialog::OnSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (pNMLV->uNewState & LVIS_SELECTED)
		AfxGetMainWnd()->PostMessage(WM_UPDATE_FIELD, 1, pNMLV->lParam);

	*pResult = 0;
}

BOOL CListDialog::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		{
			if(pMsg->wParam == VK_TAB)
			{
				CWnd* pWnd = theApp.m_pMainWnd;
				if(pWnd && pWnd->GetSafeHwnd())
				{
					pWnd->SendMessage(WM_COMMAND, ID_NEXT_FOLDER, 0);
					m_ctlList.SetFocus();
				}
			}
			break;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
