#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CStatusDialog dialog

class CStatusDialog : public CDialog
{
	DECLARE_DYNAMIC(CStatusDialog)

public:
	CStatusDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatusDialog();

// Dialog Data
	enum { IDD = IDD_STATUS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnUpdateStatus(WPARAM wParam, LPARAM lParam);
	CProgressCtrl m_ctlProgress;
	CStatic m_ctlAction;
	virtual BOOL OnInitDialog();
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
