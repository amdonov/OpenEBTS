#pragma once
#include "afxcmn.h"

// CListDialog dialog

class CListDialog : public CDialog
{
	DECLARE_DYNAMIC(CListDialog)

public:
	CListDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CListDialog();

// Dialog Data
	enum { IDD = IDD_TEXT_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	CListCtrl m_ctlList;
	CImageList m_imageList;
public:
	afx_msg void OnSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
