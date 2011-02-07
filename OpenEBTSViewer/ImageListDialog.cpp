// ImageListDialog.cpp : implementation file
//

#include "stdafx.h"
#include "OpenEBTSEditor.h"
#include "ImageListDialog.h"
#include "OpenEBTSEditorDoc.h"


// CImageListDialog dialog

IMPLEMENT_DYNAMIC(CImageListDialog, CDialog)

CImageListDialog::CImageListDialog(CWnd* pNotifyWnd)
	: CDialog(CImageListDialog::IDD, pNotifyWnd)
{
	m_pNotifyWnd = pNotifyWnd;
}

CImageListDialog::~CImageListDialog()
{
}

void CImageListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctlList);
}


BEGIN_MESSAGE_MAP(CImageListDialog, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, OnCustomdrawList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CImageListDialog::OnLvnItemchangedList1)
END_MESSAGE_MAP()


// CImageListDialog message handlers

void CImageListDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (m_ctlList.GetSafeHwnd())
	{
		m_ctlList.MoveWindow(4, 0, cx-8, cy-8);
	}

	Invalidate();
}

void CImageListDialog::OnCustomdrawList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*) pNMHDR;

	*pResult = 0;

	switch(pLVCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		}	
		case CDDS_ITEMPREPAINT:
		{
			*pResult = CDRF_NOTIFYPOSTPAINT;
			break;
		}
		case CDDS_ITEMPOSTPAINT:
		{
			LVITEM rItem;
			int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.mask  = LVIF_IMAGE | LVIF_STATE;
			rItem.iItem = nItem;
			rItem.stateMask = LVIS_SELECTED;
			m_ctlList.GetItem ( &rItem );

			CDC*  pDC = CDC::FromHandle ( pLVCD->nmcd.hdc );

			// Get the rect that holds the item's icon.
			CRect rcItem;
			m_ctlList.GetItemRect ( nItem, &rcItem, LVIR_BOUNDS );

			// Draw selection rect as needed
			BOOL bSelected = m_ctlList.GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED;
			CBrush br(GetSysColor(bSelected ? COLOR_HIGHLIGHT : COLOR_WINDOW));
			
			rcItem.DeflateRect(4,4,4,4);
			pDC->FrameRect(&rcItem, &br);
			rcItem.DeflateRect(1,1,1,1);
			pDC->FrameRect(&rcItem, &br);
			rcItem.DeflateRect(5,5,5,5);

			// Get the image
			HGLOBAL hBMP = (HGLOBAL)((COpenEBTSRecord*)pLVCD->nmcd.lItemlParam)->m_hDIB;
			if(hBMP)
			{
				LPBITMAPFILEHEADER lpBMIFile = (LPBITMAPFILEHEADER) GlobalLock(hBMP);
//				ASSERT(lpBMIFile);
				if(!lpBMIFile) return;

				LPBITMAPINFOHEADER lpBMI = (LPBITMAPINFOHEADER)((LPBYTE)lpBMIFile + sizeof(BITMAPFILEHEADER)); 
//				ASSERT(lpBMI);
				if(!lpBMI) return;

				LPBYTE lpBits = ((LPBYTE)lpBMIFile) + lpBMIFile->bfOffBits; //((LPBYTE)lpBMI) + lpBMI->biSize + 
					//(lpBMI->biBitCount > 8 ? 0 : lpBMI->biClrUsed * sizeof(RGBQUAD));
					
				double ratio = (double) lpBMI->biWidth / (double)(abs(lpBMI->biHeight));

				CRect rcImage(rcItem);

				if(lpBMI->biHeight <= lpBMI->biWidth)
				{
					rcImage.bottom = min(rcImage.top + (int)(rcImage.Width() / ratio), rcItem.bottom);
				}
				else
				{
					rcImage.right = rcImage.left + (int)(rcImage.Height() * ratio);
				}

				rcImage.DeflateRect(5,5,5,5);
//				rcImage.OffsetRect((rcItem.Width()-rcImage.Width())/2, (rcItem.Height()-rcImage.Height())/2);

				::SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
				::StretchDIBits(pDC->GetSafeHdc(), rcImage.left, rcImage.top, rcImage.Width(), rcImage.Height(),
					0, 0, lpBMI->biWidth, lpBMI->biHeight, lpBits, (BITMAPINFO *)lpBMI, DIB_RGB_COLORS, SRCCOPY);

				GlobalUnlock(hBMP);
			}

			*pResult = CDRF_SKIPDEFAULT;
			break;
		}
	}
}

void CImageListDialog::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if(pNMLV->uNewState & LVIS_SELECTED)
	{
		if(m_pNotifyWnd)
		{
			COpenEBTSRecord* pRecord = (COpenEBTSRecord*) m_ctlList.GetItemData(pNMLV->iItem);
			m_pNotifyWnd->SendMessage(WM_UPDATE_IMAGE, pRecord->m_nRecordType, (LPARAM) pRecord);
			m_ctlList.SetFocus();
		}
	}
	*pResult = 0;
}

BOOL CImageListDialog::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		{
			if(pMsg->wParam == VK_TAB)
			{
				CWnd* pWnd = theApp.m_pMainWnd;
				if(pWnd && pWnd->GetSafeHwnd())
				{
					pWnd->SendMessage(WM_COMMAND, ID_NEXT_FOLDER, 0);
					m_ctlList.SetFocus();
				}
			}
			break;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
