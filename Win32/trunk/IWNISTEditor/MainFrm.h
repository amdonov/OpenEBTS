// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "IWOutlookBar.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	int CreateOutBar();
	void DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar m_wndStatusBar;
	CToolBar m_wndToolBar;
	CToolBar m_wndImageToolBar;
	CIWOutlookBar m_wndOutBar;
	CImageList *m_pImageList;
	CImageList *m_pDisabledImageList;
	CImageList *m_pImageList2;
	CImageList *m_pDisabledImageList2;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnGetOutlookBar(WPARAM wpUnused, LPARAM lpUnused);
	afx_msg LRESULT OnUpdateField(WPARAM nRecordType, LPARAM pRecord);
	afx_msg LRESULT OnUpdateImage(WPARAM nRecordType, LPARAM pRecord);
	afx_msg LRESULT OnUpdateStatus(WPARAM wpUnused, LPARAM lParam);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnViewImagetoolbar();
	afx_msg void OnUpdateViewImagetoolbar(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
public:
	afx_msg void OnNextFolder();
};

