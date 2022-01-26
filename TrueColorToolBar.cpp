#include "stdafx.h"
#include "TrueColorToolBar.h"
#include "GuiLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrueColorToolBar

CTrueColorToolBar::CTrueColorToolBar()
{
	m_bDropDown     = FALSE;
	m_clrBtnShadow  = GetSysColor(COLOR_3DSHADOW);
	m_clrBtnHilight = GetSysColor(COLOR_3DHILIGHT);
	m_clrBtnFace	= GetSysColor(COLOR_3DFACE);

}

CTrueColorToolBar::~CTrueColorToolBar()
{
}


BEGIN_MESSAGE_MAP(CTrueColorToolBar, CToolBar)
	//{{AFX_MSG_MAP(CTrueColorToolBar)
	ON_NOTIFY_REFLECT(TBN_DROPDOWN, &CTrueColorToolBar::OnToolbarDropDown)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()	
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CTrueColorToolBar::OnNMCustomdraw)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrueColorToolBar message handlers
BOOL CTrueColorToolBar::LoadTrueColorToolBar(int  nBtnWidth,
											 UINT uToolBar,
											 UINT uToolBarHot,
											 UINT uToolBarDisabled)
{
	if (!uToolBarHot && !uToolBarDisabled) {
		if (uToolBar) {
			if (!SetTrueColorToolBar(IMAGE_TYPE_ALL, uToolBar, nBtnWidth))
				return FALSE;	
		}
	} else {
		if (uToolBar) {
			if (!SetTrueColorToolBar(IMAGE_TYPE_NORMAL, uToolBar, nBtnWidth))
				return FALSE;
		}
		
		if (uToolBarHot) {
			if (!SetTrueColorToolBar(IMAGE_TYPE_HOT, uToolBarHot, nBtnWidth))
				return FALSE;
		}

		if (uToolBarDisabled) {
			if (!SetTrueColorToolBar(IMAGE_TYPE_DISABLED, uToolBarDisabled, nBtnWidth))
				return FALSE;
		}
	}
	return TRUE;
}

COLORREF CTrueColorToolBar::ProcessBitmap(UINT uToolType, CBitmap *pSrcBitmap, CBitmap *pDstBitmap)
{
	BITMAP bmInfo;
	VERIFY(pSrcBitmap->GetBitmap(&bmInfo));	
	VERIFY(bmInfo.bmBitsPixel == sizeof(RGBX) * 8);	

	DIBSECTION ds;
	UINT nPixels = bmInfo.bmHeight * bmInfo.bmWidth;    
	VERIFY(pSrcBitmap->GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));
	RGBX *pRGBSrc = reinterpret_cast<RGBX*>(ds.dsBm.bmBits);
	VERIFY(pRGBSrc);

	COLORREF rgbMask;
	RGBX     RGBBgr;
	rgbMask = RGB(pRGBSrc->rgbtRed,pRGBSrc->rgbtGreen, pRGBSrc->rgbtBlue);
	if (!pDstBitmap) {
		return rgbMask;
	}
	memcpy(&RGBBgr, &rgbMask, sizeof(RGBX));

	VERIFY(pDstBitmap->GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));
	RGBX *pRGBDst = reinterpret_cast<RGBX*>(ds.dsBm.bmBits);
	VERIFY(pRGBDst);

	COLORREF	colPixel = 0;
	switch (uToolType) {
	case IMAGE_TYPE_HOT:				
		DrawHotBitmap (pSrcBitmap, pDstBitmap);
		/*
		for (UINT nIdx = 0; nIdx < nPixels; nIdx++) {
			if (!memcmp(pRGBDst, &rgbMask, sizeof(RGBX))) {				
				*pRGBDst =  *(RGBX *)&colPixel;
			}
			pRGBSrc++;
			pRGBDst++;
		}
		*/
		break;

	case IMAGE_TYPE_DISABLED:
		for (UINT nIdx = 0; nIdx < nPixels; nIdx++) {
			if (memcmp(pRGBSrc, &rgbMask, sizeof(RGBX))) {
				*(RGBX *)&colPixel = *pRGBSrc;
				colPixel = CGuiLib::GrayScale(colPixel);
				colPixel = CGuiLib::LightenColor(colPixel, .5);
				*pRGBDst = *(RGBX *)&colPixel;
			} else {
				*pRGBDst = RGBBgr;
			}
			pRGBSrc++;
			pRGBDst++;
		}
		break;

	default: //IMAGE_TYPE_NORMAL
		memcpy (pRGBDst, pRGBSrc, nPixels * sizeof(RGBX));
		break;
	}

	return rgbMask;
}

void CTrueColorToolBar::DrawHotBitmap(CBitmap *pSrcBitmap, CBitmap *pDstBitmap)
{	
     CDC dcSrc; 
	 CDC dcDst; 
     dcSrc.CreateCompatibleDC(NULL); 
	 dcDst.CreateCompatibleDC(NULL); 
	 CBitmap *pOldSrcBitmap = dcSrc.SelectObject(pSrcBitmap);
	 CBitmap *pOldDstBitmap = dcDst.SelectObject(pDstBitmap);

	 BITMAP bmInfo;
	 pSrcBitmap->GetBitmap(&bmInfo); 
	 UINT nBtnWidth = bmInfo.bmHeight;
	 COLORREF colBack = dcSrc.GetPixel(0,0);	 
	 CRect  rc(0,0,bmInfo.bmWidth,bmInfo.bmHeight);
	 dcDst.FillSolidRect(rc, colBack); 	 	
	 dcDst.BitBlt(rc.top,rc.left,rc.right,rc.bottom, &dcSrc, 1 , 1, SRCCOPY);

	 dcSrc.SelectObject(pOldSrcBitmap);
	 dcDst.SelectObject(pOldDstBitmap);
	 dcSrc.DeleteDC();           
	 dcDst.DeleteDC();           
}


HBITMAP CTrueColorToolBar::CopyBitmap(HBITMAP hSourceHbitmap)
{
     CDC sourceDC; 
     CDC destDC; 
     sourceDC.CreateCompatibleDC(NULL); 
     destDC.CreateCompatibleDC(NULL); 
     //The bitmap information. 
     BITMAP bm = {0}; 
     //Get the bitmap information. 
     ::GetObject(hSourceHbitmap, sizeof(bm), &bm); 
     // Create a bitmap to hold the result 
     HBITMAP hbmResult = ::CreateCompatibleBitmap(CClientDC(NULL), bm.bmWidth, bm.bmHeight); 

     HBITMAP hbmOldSource = (HBITMAP)::SelectObject(sourceDC.m_hDC, hSourceHbitmap); 
     HBITMAP hbmOldDest = (HBITMAP)::SelectObject(destDC.m_hDC, hbmResult); 
     destDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &sourceDC, 0, 0, SRCCOPY ); 

     // Restore DCs 
     ::SelectObject(sourceDC.m_hDC, hbmOldSource); 
     ::SelectObject(destDC.m_hDC, hbmOldDest); 
     ::DeleteObject(sourceDC.m_hDC); 
     ::DeleteObject(destDC.m_hDC); 

     return hbmResult; 
}


BOOL CTrueColorToolBar::SetTrueColorToolBar(UINT uToolType, 
							     	        UINT uToolBar,
										    int  nBtnWidth)
{
	CImageList	cImageList;
	CBitmap		cBitmap;	
	BITMAP		bmBitmap;	
	UINT		uToolBarType;
	//UINT		nSize;
	UINT		nNbBtn;
	COLORREF	rgbMask;

	HANDLE      hImg = LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(uToolBar), 
		                         IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
	if (!cBitmap.Attach(hImg) ||  !cBitmap.GetBitmap(&bmBitmap))
		return FALSE;
	nNbBtn	= bmBitmap.bmWidth / nBtnWidth;

	if (uToolType == IMAGE_TYPE_ALL) {
		CBitmap		cNewBitmap;
		CImageList	cImageList[IMAGE_TYPE_ALL];
		HANDLE      hNewImg = LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(uToolBar), 
		                         IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
		if (!cNewBitmap.Attach(hNewImg))	return FALSE;

		for (uToolType = IMAGE_TYPE_NORMAL ; uToolType < IMAGE_TYPE_ALL; uToolType++) {
			rgbMask = ProcessBitmap (uToolType,   &cBitmap, &cNewBitmap);			
			cImageList[uToolType].Create(nBtnWidth, bmBitmap.bmHeight, ILC_COLOR24|ILC_MASK, nNbBtn, 0);
			cImageList[uToolType].Add(&cNewBitmap, rgbMask);
			if (uToolType == IMAGE_TYPE_HOT) 
				uToolBarType = TB_SETHOTIMAGELIST;
			else if (uToolType == IMAGE_TYPE_DISABLED) 
				uToolBarType = TB_SETDISABLEDIMAGELIST;
			else 
				uToolBarType = TB_SETIMAGELIST;
			SendMessage(uToolBarType,  0, (LPARAM)cImageList[uToolType].m_hImageList);				
			cImageList[uToolType].Detach();
		}							
		cNewBitmap.DeleteObject(); 
	} else {
		rgbMask = ProcessBitmap (IMAGE_TYPE_NORMAL,   &cBitmap, NULL);			
		if (!cImageList.Create(nBtnWidth,  bmBitmap.bmHeight, ILC_COLOR24|ILC_MASK, nNbBtn, 0))
			return FALSE;
	
		if (cImageList.Add(&cBitmap, rgbMask) == -1)
			return FALSE;

		if (uToolType == IMAGE_TYPE_HOT) 
			uToolBarType = TB_SETHOTIMAGELIST;
		else if (uToolType == IMAGE_TYPE_DISABLED) 
			uToolBarType = TB_SETDISABLEDIMAGELIST;
		else 
			uToolBarType = TB_SETIMAGELIST;
		SendMessage(uToolBarType, 0, (LPARAM)cImageList.m_hImageList);	
		cImageList.Detach(); 		
	}	
	cBitmap.DeleteObject();
	return TRUE;
}

void CTrueColorToolBar::AddDropDownButton(CWnd* pParent, UINT uButtonID, UINT uMenuID)
{
	if (!m_bDropDown) {
		GetToolBarCtrl().SendMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_DRAWDDARROWS);
		m_bDropDown = TRUE;
	}

	SetButtonStyle(CommandToIndex(uButtonID), TBSTYLE_DROPDOWN);

	stDropDownInfo DropDownInfo;
	DropDownInfo.pParent	= pParent;
	DropDownInfo.uButtonID	= uButtonID;
	DropDownInfo.uMenuID	= uMenuID;
	m_lstDropDownButton.Add(DropDownInfo);
}

void CTrueColorToolBar::OnToolbarDropDown(NMHDR* pnmhdr, LRESULT *plr)
{	
	NMTOOLBAR *pnmtb = (NMTOOLBAR *)pnmhdr;

	for (int i = 0; i < m_lstDropDownButton.GetSize(); i++) {
		
		stDropDownInfo DropDownInfo = m_lstDropDownButton.GetAt(i);

		if (DropDownInfo.uButtonID == UINT(pnmtb->iItem)) {

			CMenu menu;
			menu.LoadMenu(DropDownInfo.uMenuID);
			CMenu* pPopup = menu.GetSubMenu(0);
			
			CRect rc;
			SendMessage(TB_GETRECT, (WPARAM)pnmtb->iItem, (LPARAM)&rc);
			ClientToScreen(&rc);
			
			pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,
				                   rc.left, rc.bottom, DropDownInfo.pParent, &rc);
			break;
		}
	}
}
int CTrueColorToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	 GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS );

	return 0;
}

void CTrueColorToolBar::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	CDC      dc;
	dc.Attach(pNMCD->hdc);
	CRect	rc(pNMCD->rc);
	CImageList *pImgList;
	*pResult = 0;
	
	
	int  nItem = CommandToIndex((UINT)pNMCD->dwItemSpec);

	UINT uID = 0, uStyle = 0;
	int	 nBitmap = 0;
	GetButtonInfo(nItem, uID, uStyle, nBitmap);			
	
	switch (pNMCD->dwDrawStage)  {
	case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;

	case CDDS_ITEMPREPAINT:   
		if ((pNMCD->uItemState & CDIS_HOT) && (pNMCD->uItemState & CDIS_CHECKED)) { 						
			pImgList = GetToolBarCtrl().GetImageList();
			dc.FillSolidRect(&rc, 0xFCDDC0); 
			dc.Draw3dRect(&rc, 0xFEB064, 0xFEB064);
			rc.InflateRect (-1,-1); 
			dc.Draw3dRect(&rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DHILIGHT)); 
		} else if (pNMCD->uItemState & CDIS_HOT) {				
			pImgList = GetToolBarCtrl().GetHotImageList();
			dc.FillSolidRect(&rc, 0xFCDDC0); 
			dc.Draw3dRect(&rc, 0xFEB064, 0xFEB064); 					
		} else if (pNMCD->uItemState & CDIS_CHECKED) { 			
			pImgList = GetToolBarCtrl().GetImageList(); 
			rc.InflateRect (-1,-1); 
			dc.Draw3dRect(&rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DHILIGHT)); 
		} else if (pNMCD->uItemState & CDIS_DISABLED) { 			
			pImgList = GetToolBarCtrl().GetDisabledImageList(); 
		} else {
			pImgList = GetToolBarCtrl().GetImageList(); 
		}

		int cx = 0, cy = 0;
		::ImageList_GetIconSize(pImgList->GetSafeHandle(),&cx,&cy);
		CPoint ptImage((pNMCD->rc.left + pNMCD->rc.right - cx) / 2 ,
                 (pNMCD->rc.top + pNMCD->rc.bottom - cy) / 2);

		// Draw Icon
		pImgList->Draw(&dc, nBitmap, ptImage, ILD_TRANSPARENT);
		//DrawTransparentBitmap(&dc,	0, CPoint(0,0));

		//*pResult = TBCDRF_NOBACKGROUND | TBCDRF_NOEDGES; //CDRF_SKIPDEFAULT //TBCDRF_NOBACKGROUND
		*pResult = CDRF_SKIPDEFAULT;
		
		break;
	}

	dc.Detach();
   
}

void CTrueColorToolBar::OnPaint() 
{	
	CToolBar::OnPaint();

	// Redraw Separator
	//EraseNonClient();

	int nBitmap; 
	UINT uID, uStyleState;
	CClientDC dc(this);
	CRect	  rc;

	register const int nBtn = GetToolBarCtrl().GetButtonCount();
	for( register int i = 0; i < nBtn; ++i ) {
		GetButtonInfo(i, uID, uStyleState, nBitmap);			
		WORD wStyle = LOWORD(uStyleState);
		WORD wState = HIWORD(uStyleState);
		if( wState & TBSTATE_HIDDEN )
			continue;
		if( wStyle == TBSTYLE_SEP ) {					
			GetItemRect(i, rc);
			DrawSeparator(&dc, &rc);
		}
	}
}

void CTrueColorToolBar :: DrawSeparator( CDC *dc, CRect *rcSep ) const 
{
	CRect rc;
    BOOL bHorz = ((m_dwStyle & CBRS_ORIENT_HORZ) != 0) ? TRUE : FALSE;
	// make sure, this separator is not a placeholder for
	// another control.
	rc.CopyRect(rcSep); 
	if( rc.Width() <= 8 ) {
		if( bHorz ) {
			// draw the separator bar in the middle
			int x = (rc.left + rc.right) / 2;
			rc.left = x-1; rc.right = x+1;
			rc.top += 2;
			dc->Draw3dRect(
				rc,
				m_clrBtnShadow,
				m_clrBtnHilight
			);
		} else {
			// draw the separator bar in the middle
			rc.left = rc.left - m_sizeButton.cx;
			rc.right = rc.left + m_sizeButton.cx;
			rc.top = rc.bottom+1;
			rc.bottom = rc.top+3;
			int y = (rc.top+rc.bottom)/2;
			rc.top = y-1; rc.bottom = y+1;
			dc->Draw3dRect(
				rc,
				m_clrBtnShadow,
				m_clrBtnHilight
			);
		}
	}
}

void CTrueColorToolBar :: DrawGripper( CDC *dc, CRect *rectWindow) const 
{ 

    if( m_dwStyle & CBRS_FLOATING )
		return;		// no gripper if floating

	CRect gripper;
	if (rectWindow == NULL) {
		GetWindowRect(gripper);
		ScreenToClient(gripper);	
		gripper.OffsetRect(-gripper.left, -gripper.top);
	} else {
		gripper.CopyRect(rectWindow);
	}

	if( m_dwStyle & CBRS_ORIENT_HORZ ) {
		// gripper at left
		gripper.DeflateRect(4, 4);
		gripper.right = gripper.left+3;
        dc->Draw3dRect(
			gripper,
			m_clrBtnHilight,
            m_clrBtnShadow
		);
		gripper.OffsetRect(3, 0);
        dc->Draw3dRect(
			gripper,
			m_clrBtnHilight,
            m_clrBtnShadow
		);
	} else {
		// gripper at top
		gripper.DeflateRect(4, 4);
		gripper.bottom = gripper.top+3;
		dc->Draw3dRect(
			gripper,
			m_clrBtnHilight,
            m_clrBtnShadow
		);
		gripper.OffsetRect(0, 3);
        dc->Draw3dRect(
			gripper,
			m_clrBtnHilight,
            m_clrBtnShadow
		);
	}
}

void CTrueColorToolBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CToolBar::OnNcCalcSize(bCalcValidRects, lpncsp);
	// adjust non-client area for gripper at left or top
	if (m_dwStyle & CBRS_FLOATING) {
		;
	} else if( m_dwStyle & CBRS_ORIENT_HORZ ) {
		lpncsp->rgrc[0].left += 4;
		lpncsp->rgrc[0].right += 4;
	} else {
		lpncsp->rgrc[0].top += 6;
		lpncsp->rgrc[0].bottom += 6;
	}
}

// Erase the non-client area (borders) - copied from MFC implementation
void CTrueColorToolBar::EraseNonClient() 
{
	// get window DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// draw borders in non-client area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	// erase parts not drawn
	dc.IntersectClipRect(rectWindow);
		
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);

	DrawGripper(&dc, &rectWindow);	
}

void CTrueColorToolBar::OnNcPaint() 
{	
	EraseNonClient();
}


BOOL CTrueColorToolBar::OnEraseBkgnd(CDC*pDC)
{
	//CToolBar::OnEraseBkgnd(pDC);
	
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);    
	CGuiLib::GradientFill(pDC, rectWindow, m_clrBtnHilight, m_clrBtnShadow, TRUE);	
	//pDC->FillSolidRect(rectWindow, RGB(0,0,0));
	return TRUE;
}
