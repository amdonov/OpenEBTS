#pragma once
#include "afxcmn.h"

// CImageListDialog dialog

class CImageListDialog : public CDialog
{
	DECLARE_DYNAMIC(CImageListDialog)

public:
	CImageListDialog(CWnd* pNotifyWnd);   // standard constructor
	virtual ~CImageListDialog();

// Dialog Data
	enum { IDD = IDD_IMAGE_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);

public:
	CListCtrl m_ctlList;
	CImageList m_imageList;
	CWnd* m_pNotifyWnd;

public:
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
