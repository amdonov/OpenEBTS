// IWOutlookBar.cpp : implementation file
//

#include "stdafx.h"
#include "IWNISTEditor.h"
#include "IWOutlookBar.h"
#include "ListDialog.h"
#include "ImageListDialog.h"

// CIWOutlookBar

IMPLEMENT_DYNAMIC(CIWOutlookBar, COutlook2Ctrl)

CIWOutlookBar::CIWOutlookBar()
{

}

CIWOutlookBar::~CIWOutlookBar()
{
}


BEGIN_MESSAGE_MAP(CIWOutlookBar, COutlook2Ctrl)
END_MESSAGE_MAP()



// CIWOutlookBar message handlers

void CIWOutlookBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}


int CIWOutlookBar::AddRecord(int nType, CIWNistRecord* pRecord)
{
	int nFolder = -1;

	switch(nType)
	{
		case 1:
		case 2:
		{
			CListDialog* pDlg;
			nFolder = GetFolderIndex(nType);
			if(nFolder >= 0)
			{
				COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
				COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
				pDlg = (CListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
			}
			else
			{
				pDlg = new CListDialog;
				pDlg->Create(IDD_TEXT_LIST, this);
				pDlg->m_imageList.Create(10, 16, 0, 0, 0);
				pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_SMALL);
				pDlg->m_ctlList.InsertColumn(0, _T("Field"));
				//pDlg->m_ctlList.InsertColumn(1, _T("Value"));
				pDlg->m_ctlList.SetExtendedStyle(pDlg->m_ctlList.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

				nFolder = COutlook2Ctrl::AddFolder(nType == 1 ? "Transaction Details" : "Descriptive Record", IDI_FOLDER_CLOSED, nType);
				AddFolderItem("", 0, nFolder);
				AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
			}
			
			pDlg->m_ctlList.DeleteAllItems();

			for(int nIndex = 0; nIndex < pRecord->m_arrFields.GetSize(); nIndex++)
			{
				CIWNistField* pField = pRecord->m_arrFields.GetAt(nIndex);
				
				CString csString;
				csString.Format(_T("Field %d"), pField->m_ID);
				
				pDlg->m_ctlList.InsertItem(nIndex, csString);
				pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR) pField);
			}

			pDlg->m_ctlList.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			break;
		}
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 13:
		case 14:
		{
			CImageListDialog* pDlg;
			nFolder = GetFolderIndex(nType);
			if(nFolder >= 0)
			{
				COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
				COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
				pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
			}
			else
			{
				pDlg = new CImageListDialog(AfxGetMainWnd());
				pDlg->Create(IDD_IMAGE_LIST, this);

				CRect rc;
				pDlg->m_ctlList.GetClientRect(&rc);
				pDlg->m_imageList.Create(rc.Width()-60, rc.Width()-60, 0, 0, 0);
				pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_NORMAL);

				CString csLabel;
				switch(nType)
				{
					case 7:  csLabel = "User Defined"; break;
					case 13: csLabel = "Latent Prints"; break;
					default: csLabel = "Fingerprints";  break;
				}

#ifdef UNICODE
				char szLabel[80];
				size_t ret;
				wcstombs_s(&ret, szLabel, 80, csLabel, 80);
				nFolder = COutlook2Ctrl::AddFolder(szLabel, IDI_FOLDER_CLOSED, nType);
#else
				nFolder = COutlook2Ctrl::AddFolder(csLabel, IDI_FOLDER_CLOSED, nType);
#endif
				AddFolderItem("", 0, nFolder);
				AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
			}

			int nIndex = pDlg->m_ctlList.GetItemCount();
			pDlg->m_ctlList.InsertItem(nIndex, _T(""));
			pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR)pRecord);

			break;
		}
		case 15:
		{
			CImageListDialog* pDlg;
			nFolder = GetFolderIndex(nType);
			if(nFolder >= 0)
			{
				COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
				COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
				pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
			}
			else
			{
				pDlg = new CImageListDialog(AfxGetMainWnd());
				pDlg->Create(IDD_IMAGE_LIST, this);

				CRect rc;
				pDlg->m_ctlList.GetClientRect(&rc);
				pDlg->m_imageList.Create(rc.Width()-60, rc.Width()-60, 0, 0, 0);
				pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_NORMAL);

				nFolder = COutlook2Ctrl::AddFolder("Palm Prints", IDI_FOLDER_CLOSED, nType);
				AddFolderItem("", 0, nFolder);
				AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
			}
			
			int nIndex = pDlg->m_ctlList.GetItemCount();
			pDlg->m_ctlList.InsertItem(nIndex, _T(""));
			pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR)pRecord);

			break;
		}
		case 8:
		{
			CImageListDialog* pDlg;
			nFolder = GetFolderIndex(nType);
			if(nFolder >= 0)
			{
				COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
				COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
				pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
			}
			else
			{
				pDlg = new CImageListDialog(AfxGetMainWnd());
				pDlg->Create(IDD_IMAGE_LIST, this);

				CRect rc;
				pDlg->m_ctlList.GetClientRect(&rc);
				pDlg->m_imageList.Create(rc.Width()-60, rc.Width()-60, 0, 0, 0);
				pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_NORMAL);

				nFolder = COutlook2Ctrl::AddFolder("Signatures", IDI_FOLDER_CLOSED, nType);
				AddFolderItem("", 0, nFolder);
				AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
			}
			
			int nIndex = pDlg->m_ctlList.GetItemCount();
			pDlg->m_ctlList.InsertItem(nIndex, _T(""));
			pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR) pRecord);

			break;
		}
		case 10:
		{
			if(pRecord->m_nImageType == 1)
			{			
				CImageListDialog* pDlg;
				nFolder = GetFolderIndex(nType);
				if(nFolder >= 0)
				{
					COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
					COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
					pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
				}
				else
				{
					pDlg = new CImageListDialog(AfxGetMainWnd());
					pDlg->Create(IDD_IMAGE_LIST, this);

					CRect rc;
					pDlg->m_ctlList.GetClientRect(&rc);
					pDlg->m_imageList.Create(rc.Width()-60, rc.Width()-60, 0, 0, 0);
					pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_NORMAL);

					nFolder = COutlook2Ctrl::AddFolder("Faces/SMTs", IDI_FOLDER_CLOSED, nType);
					AddFolderItem("", 0, nFolder);
					AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
				}
				
				int nIndex = pDlg->m_ctlList.GetItemCount();
				pDlg->m_ctlList.InsertItem(nIndex, _T(""));
				pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR) pRecord);
			}
			else if(pRecord->m_nImageType == -1)
			{			
				CImageListDialog* pDlg;
				nFolder = GetFolderIndex(-1);
				if(nFolder >= 0)
				{
					COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
					COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
					pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
				}
				else
				{
					pDlg = new CImageListDialog(AfxGetMainWnd());
					pDlg->Create(IDD_IMAGE_LIST, this);

					CRect rc;
					pDlg->m_ctlList.GetClientRect(&rc);
					pDlg->m_imageList.Create(rc.Width()-60, rc.Width()-60, 0, 0, 0);
					pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_NORMAL);

					nFolder = COutlook2Ctrl::AddFolder("Other", IDI_FOLDER_CLOSED, -1);
					AddFolderItem("", 0, nFolder);
					AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
				}
				
				int nIndex = pDlg->m_ctlList.GetItemCount();
				pDlg->m_ctlList.InsertItem(nIndex, _T(""));
				pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR) pRecord);
			}
			break;
		}
		case 16:
		case 17:
		{
			CImageListDialog* pDlg;
			nFolder = GetFolderIndex(nType);
			if(nFolder >= 0)
			{
				COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
				COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);
				pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);			
			}
			else
			{
				pDlg = new CImageListDialog(AfxGetMainWnd());
				pDlg->Create(IDD_IMAGE_LIST, this);

				CRect rc;
				pDlg->m_ctlList.GetClientRect(&rc);
				pDlg->m_imageList.Create(rc.Width()-60, rc.Width()-60, 0, 0, 0);
				pDlg->m_ctlList.SetImageList(&pDlg->m_imageList, LVSIL_NORMAL);

				nFolder = COutlook2Ctrl::AddFolder("Irises", IDI_FOLDER_CLOSED, nType);
				AddFolderItem("", 0, nFolder);
				AddSubItem(pDlg->GetSafeHwnd(), true, nFolder);
			}
			
			int nIndex = pDlg->m_ctlList.GetItemCount();
			pDlg->m_ctlList.InsertItem(nIndex, _T(""));
			pDlg->m_ctlList.SetItemData(nIndex, (DWORD_PTR) pRecord);

			break;
		}
	}

	return nFolder;
}


int CIWOutlookBar::GetFolderIndex(int nType)
{
	for(int i = 0; i < m_Folders.GetSize(); i++)
	{
		if(GetFolder(i)->lParam == nType) return i;
	}

	// didn't find it
	return -1;
}

void CIWOutlookBar::SetCurFolder(int nFolder, bool bAnimation)
{
	if(!m_Folders.GetCount()) return;

	//COL2Folder* pPrevSelFolder = GetFolder(m_iSelectedFolder);
	//if(pPrevSelFolder)
	//{
	//	HICON hIco = (HICON) LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_FOLDER_CLOSED),IMAGE_ICON,16,16,0);
	//	pPrevSelFolder->hIcon = hIco;
	//}

	COutlook2Ctrl::SetCurFolder(nFolder, bAnimation);

	COL2Folder* pFolder = GetFolder(nFolder);
	COL2Item* pItem = (COL2Item*) pFolder->m_Items.GetAt(0);
	COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);

	switch(pFolder->lParam)
	{
		case 1:
		case 2:
		{
			CListDialog* pDlg = (CListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);
			
			int nSelectedIndex = pDlg->m_ctlList.GetNextItem(-1, LVNI_SELECTED);
			if (nSelectedIndex < 0)
			{
				nSelectedIndex = 0;
				pDlg->m_ctlList.SetItemState(nSelectedIndex, 
					LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			}

			CIWNistField* pField = (CIWNistField*) pDlg->m_ctlList.GetItemData(nSelectedIndex);
			if(pField) AfxGetMainWnd()->SendMessage(WM_UPDATE_FIELD, pFolder->lParam, (LPARAM) pField);
			
			//HICON hIco = (HICON) LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_FOLDER_CLOSED),IMAGE_ICON,16,16,0);
			//pFolder->hIcon = hIco;
			
			pDlg->m_ctlList.SetFocus();

			break;
		}
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 10:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		{
			CImageListDialog* pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);
			
			int nSelectedIndex = pDlg->m_ctlList.GetNextItem(-1, LVNI_SELECTED);
			if( nSelectedIndex < 0)
			{
				nSelectedIndex = 0;
				pDlg->m_ctlList.SetFocus();
				pDlg->m_ctlList.SetItemState(nSelectedIndex, 
					LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			}

			CIWNistRecord* pRecord = (CIWNistRecord*) pDlg->m_ctlList.GetItemData(nSelectedIndex);
			if(pRecord) AfxGetMainWnd()->SendMessage(WM_UPDATE_IMAGE, pRecord->m_nRecordType, (LPARAM) pRecord);
			
			pDlg->m_ctlList.SetFocus();
			
			break;
		}
	}
}


void CIWOutlookBar::Empty()
{
	int nTypes[] = { 1, 2, 4, 10, 16 };
	int nTypeCount = sizeof(nTypes)/sizeof(nTypes[0]);

	for (int i = 0; i < nTypeCount; i++)
	{
		int nFolder = GetFolderIndex(nTypes[i]);
		if (nFolder < 0) continue;

		COL2Item* pItem = (COL2Item*)GetFolder(nFolder)->m_Items.GetAt(0);
		COL2SubItem* pSubItem = (COL2SubItem*)pItem->m_SubItems.GetAt(0);

		switch(nTypes[i])
		{
			case 1:
			case 2:
			{
				CListDialog* pDlg = (CListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);
				pDlg->m_ctlList.DeleteAllItems();
				pDlg->DestroyWindow();
				delete pDlg;
				break;
			}
			case 4:
			case 10:
			case 16:
			{
				CImageListDialog* pDlg = (CImageListDialog*) CWnd::FromHandle(pSubItem->hHostedWnd);
				pDlg->m_ctlList.DeleteAllItems();
				pDlg->DestroyWindow();
				delete pDlg;
				break;
			}
		}
	}

	COutlook2Ctrl::Empty();
}

