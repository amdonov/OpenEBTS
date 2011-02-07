// OpenEBTSEditorView.h : interface of the COpenEBTSEditorView class
//


#pragma once
#include "ImageDetailsDialog.h"
#include "GridCtrl_src/GridCtrl.h"

#define IDC_GRID	100
#define CELL_HEIGHT	21	

class COpenEBTSEditorView : public CScrollView
{
protected: // create from serialization only
	COpenEBTSEditorView();
	DECLARE_DYNCREATE(COpenEBTSEditorView)

// Attributes
public:
	COpenEBTSEditorDoc* GetDocument() const;
	CGridCtrl* m_pGridCtrl;

	BOOL m_bStretchToFit;
	COpenEBTSRecord* m_pRecord;
	CImageDetailsDialog* m_pImageDetailsDialog;

private:

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~COpenEBTSEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnUpdateField(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateImage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnZoom100();
	afx_msg void OnZoomTofit();
	afx_msg void OnUpdateZoom100(CCmdUI *pCmdUI);
	afx_msg void OnUpdateZoomTofit(CCmdUI *pCmdUI);
	afx_msg void OnViewImagedetails();
	afx_msg void OnUpdateViewImagedetails(CCmdUI *pCmdUI);
public:
//	afx_msg void OnDestroy();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in OpenEBTSEditorView.cpp
inline COpenEBTSEditorDoc* COpenEBTSEditorView::GetDocument() const
   { return reinterpret_cast<COpenEBTSEditorDoc*>(m_pDocument); }
#endif

