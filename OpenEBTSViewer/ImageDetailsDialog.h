#pragma once
#include "afxcmn.h"


// CImageDetailsDialog dialog

class CImageDetailsDialog : public CDialog
{
	DECLARE_DYNAMIC(CImageDetailsDialog)

public:
	CImageDetailsDialog(CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_IMAGEDETAILS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CListCtrl m_ctlList;

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void UpdateList(CStringArray* pStrings);
};
