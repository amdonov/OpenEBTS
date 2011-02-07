// OpenEBTSEditor.h : main header file for the OpenEBTSEditor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#define WM_GET_OUTLOOKBAR (WM_USER + 1)
#define WM_UPDATE_IMAGE   (WM_USER + 2)
#define WM_UPDATE_FIELD   (WM_USER + 3)
#define WM_UPDATE_STATUS  (WM_USER + 4)

// COpenEBTSEditorApp:
// See OpenEBTSEditor.cpp for the implementation of this class
//

class COpenEBTSEditorApp : public CWinApp
{
public:
	COpenEBTSEditorApp();

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	CWnd* m_pStatusWnd;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern COpenEBTSEditorApp theApp;