// ImageDetailsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "OpenEBTSEditor.h"
#include "ImageDetailsDialog.h"


// CImageDetailsDialog dialog

IMPLEMENT_DYNAMIC(CImageDetailsDialog, CDialog)

CImageDetailsDialog::CImageDetailsDialog(CWnd* pParent)
	: CDialog(CImageDetailsDialog::IDD, pParent)
{
	Create(IDD, pParent);
}


void CImageDetailsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctlList);
}


BEGIN_MESSAGE_MAP(CImageDetailsDialog, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CImageDetailsDialog message handlers


BOOL CImageDetailsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	CRect rc;
	m_ctlList.GetClientRect(&rc);
	m_ctlList.InsertColumn(0, _T("Field"), LVCFMT_LEFT, 40);
	m_ctlList.InsertColumn(1, _T("Value"), LVCFMT_LEFT, (int)(rc.Width()-40));

	return TRUE;
}


void CImageDetailsDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (m_ctlList.GetSafeHwnd())
	{
		m_ctlList.MoveWindow(5, 5, cx-10, cy-10);

		CRect rc;
		m_ctlList.GetClientRect(&rc);
		m_ctlList.SetColumnWidth(1, (rc.Width() - 
			m_ctlList.GetColumnWidth(0)));
	}
	Invalidate();
}


void CImageDetailsDialog::UpdateList(CStringArray* pStrings)
{
	m_ctlList.DeleteAllItems();
	
	for(int i = 0; pStrings && i < pStrings->GetSize(); i++)
	{
		CString cs, csString = pStrings->GetAt(i);

		int nPos = csString.Find(_T("\t"));
		int nIndex = m_ctlList.GetItemCount();
		int nTokens = 0; 

		if(nPos == -1)
		{
			m_ctlList.InsertItem(nIndex, csString);
			continue;
		}				
		else
		{
			cs = csString.Left(nPos);
			m_ctlList.InsertItem(nIndex, cs);
			csString.Delete(0, nPos+1);
			nPos = 0;
		}

		cs = csString.Tokenize(_T("\n"), nPos);
		while(!cs.IsEmpty())
		{
			if(nTokens)
			{
				nIndex = m_ctlList.GetItemCount();
				m_ctlList.InsertItem(nIndex, _T(""));
			}

			m_ctlList.SetItemText(nIndex, 1, cs);

			cs = csString.Tokenize(_T("\n"), nPos);
			nTokens++;
		}
	}

	//for(int i = 0; pStrings && i < pStrings->GetSize(); i++)
	//{
	//	csTemp = pStrings->GetAt(i);
	//	nPos = csTemp.Find('\t');
	//	nIndex = m_ctlList.InsertItem(i, csTemp.Left(nPos));
	//	m_ctlList.SetItemText(nIndex, 1, csTemp.Mid(nPos+1, csTemp.GetLength()-nPos-2));
	//}
}
