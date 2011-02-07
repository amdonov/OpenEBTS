// StatusDialog.cpp : implementation file
//

#include "stdafx.h"
#include "OpenEBTSEditor.h"
#include "StatusDialog.h"


// CStatusDialog dialog

IMPLEMENT_DYNAMIC(CStatusDialog, CDialog)

CStatusDialog::CStatusDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStatusDialog::IDD, pParent)
{

}

CStatusDialog::~CStatusDialog()
{
}

void CStatusDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_ctlProgress);
	DDX_Control(pDX, IDC_ACTION, m_ctlAction);
}


BEGIN_MESSAGE_MAP(CStatusDialog, CDialog)
	ON_MESSAGE(WM_UPDATE_STATUS, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CStatusDialog message handlers

LRESULT CStatusDialog::OnUpdateStatus(WPARAM wParam, LPARAM lParam)
{
	int nPos = (int) wParam;
	LPCTSTR szText = (LPCTSTR) lParam;

	m_ctlProgress.SetPos(nPos);
	m_ctlAction.SetWindowText(szText);

	return ERROR_SUCCESS;
}

BOOL CStatusDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ctlProgress.SetRange(0, 100);

	return TRUE;
}

HBRUSH CStatusDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_PROMPT)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}

	return hbr;
}
