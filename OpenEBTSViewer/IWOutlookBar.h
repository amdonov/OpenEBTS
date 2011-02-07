#pragma once
#include "Outlook2Ctrl.h"
#include "OpenEBTSEditorDoc.h"

// CIWOutlookBar

class CIWOutlookBar : public COutlook2Ctrl
{
	DECLARE_DYNAMIC(CIWOutlookBar)

public:
	CIWOutlookBar();
	virtual ~CIWOutlookBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void Empty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	int AddRecord(int nType, COpenEBTSRecord* pRecord);
public:
	int GetFolderIndex(int nType);
	int GetFolderCount() { return (int) m_Folders.GetSize(); }
	int GetCurFolder() { return m_iSelectedFolder; }
	void SetCurFolder(int f, bool bAnimation = false);
};

