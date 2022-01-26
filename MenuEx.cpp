/////////////////////////////////////////////
// CMenuEx - Version 1.1 - August 30, 2004 //
// --------------------------------------- //
// MFC-derived class for owner-drawn and   //
// Office XP style menus.                  //
// --------------------------------------- //
// Copyright (C) 2004 by C. Buegenburg     //
// All rights reserved.                    //
// --------------------------------------- //
// You are permitted to freely use and     //
// distribute this class, as long as you   //
// a) give proper credit to me and         //
// b) do not remove my copyright notes and //
// c) report any code changes to me.       //
/////////////////////////////////////////////

#include "stdafx.h"
#include "MenuEx.h"
#include "GuiLib.h"

////////////////////////////
// Statics Implementation //
////////////////////////////

COLORREF	CMenuEx::m_colHead;			// Default: COLOR_MENUBAR
COLORREF	CMenuEx::m_colBackground;	// Default: COLOR_MENU
COLORREF	CMenuEx::m_colHorzBar;		// Default: COLOR_MENUBAR
COLORREF	CMenuEx::m_colText;			// Default: COLOR_MENUTEXT
COLORREF	CMenuEx::m_colGrayText;		// Default: COLOR_GRAYTEXT
COLORREF	CMenuEx::m_colSeparator;	// Default: COLOR_MENUBAR
COLORREF	CMenuEx::m_colHilightBorder;// Default: COLOR_3DSHADOW
COLORREF	CMenuEx::m_colHilight;		// Default: COLOR_MENUHILIGHT
COLORREF	CMenuEx::m_colMenuGray;	    // Default: COLOR_MENU
COLORREF	CMenuEx::m_colMenuHilight;	// Default: COLOR_MENU
COLORREF	CMenuEx::m_colMenuHilightBorder;
BOOL		CMenuEx::m_bHighlightGray;	// Default: FALSE
CSize		CMenuEx::m_szImage;			// Default: 16x16
CSize		CMenuEx::m_szImagePadding;	// Default: 3x3
CSize		CMenuEx::m_szTextPadding;	// Default: 3x3

////////////////////////////////
// Construction & Destruction //
////////////////////////////////

CMenuEx::CMenuEx(CWnd* pWnd)
{
	CMenu::CMenu();
	m_pWnd = pWnd;

	// Defaults
	// DWORD dwVersion		= GetVersion();

	m_colHead			= GetSysColor(COLOR_3DFACE		);
	m_colHilight		= GetSysColor(COLOR_3DHIGHLIGHT	);
	m_colMenuHilightBorder = GetSysColor(COLOR_HIGHLIGHT);
	m_colMenuHilight	= CGuiLib::LightenColor(m_colMenuHilightBorder, .7);
	m_colBackground		= GetSysColor(COLOR_MENU		);
	m_colHorzBar		= GetSysColor(COLOR_MENUBAR		);
	m_colText			= GetSysColor(COLOR_MENUTEXT	);
	m_colGrayText		= GetSysColor(COLOR_GRAYTEXT	);
	m_colSeparator		= GetSysColor(COLOR_MENUBAR		);
	m_colHilightBorder	= GetSysColor(COLOR_3DSHADOW	);
	m_colMenuGray    	= GetSysColor(COLOR_MENU		);
	m_bHighlightGray	= FALSE;
	m_szImage			= CSize(16, 16);
	m_szImagePadding	= CSize(3, 3);
	m_szTextPadding		= CSize(5, 3);
}

CMenuEx::~CMenuEx()
{
	CMenu::~CMenu();

	for(INT32 a=0; a<m_TempMenus.GetSize(); a++)
	{
		delete (CMenuEx*) m_TempMenus[a];
	}
	for(INT32 b=0; b<m_TempItems.GetSize(); b++)
	{
		delete (MENUITEMEX*) m_TempItems[b];
	}
	for(INT32 c=0; c<m_ImageLists.GetSize(); c++)
	{
		delete (CImageList*) m_ImageLists[c];
	}
}

////////////////////
// Public Methods //
////////////////////

BOOL CMenuEx::LoadMenu(UINT nIDResource)
{
	BOOL bResult = CMenu::LoadMenu(nIDResource);
	if(bResult) MakeOwnerDrawn(TRUE);
	return bResult;
}

BOOL CMenuEx::LoadMenu(LPCTSTR lpszResourceName)
{
	BOOL bResult = CMenu::LoadMenu(lpszResourceName);
	if(bResult) MakeOwnerDrawn(TRUE);
	return bResult;
}

BOOL CMenuEx::Attach(HMENU hMenu)
{
	BOOL bResult = CMenu::Attach(hMenu);
	if(bResult) MakeOwnerDrawn(FALSE);
	return bResult;
}

void CMenuEx::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	MENUITEMEX* pItem = (MENUITEMEX*) lpDIS->itemData;

	// Define Rectangles
	CRect rectBar	(lpDIS->rcItem);
	CRect rectImage	(lpDIS->rcItem);
	CRect rectItem	(lpDIS->rcItem);
	CRect rectText	(lpDIS->rcItem);
	if(pItem->m_bIsTopLevel)
	{
		rectBar	= CRect(0, 0, 0, 0);
		rectText.left	+= 2;
		rectText.top	+= 3;
	}
	else
	{
		rectBar	.right	 = rectBar.left+m_szImagePadding.cx+m_szImage.cx+m_szImagePadding.cx;
		rectImage.left	+= m_szImagePadding.cx;
		rectImage.top	+= m_szImagePadding.cy;
		rectImage.right	 = rectImage.left+m_szImage.cx;
		rectImage.bottom = rectImage.top +m_szImage.cy;
		rectItem.left	+= m_szImagePadding.cx+m_szImage.cx+m_szImagePadding.cx;
		rectText.left	+= m_szImagePadding.cx+m_szImage.cx+m_szImagePadding.cx+m_szTextPadding.cx;
		rectText.top	+= m_szTextPadding.cy;
	}

	// Color Bar
	CGuiLib::GradientFill(pDC, rectBar, m_colHilight, m_colHilightBorder, FALSE);


	// Item
	if(pItem->m_uiID == 0)
	{
		// Separator
		pDC->FillSolidRect(&rectItem, m_colBackground);
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, m_colSeparator);
		CPen* pOldPen = pDC->SelectObject(&pen);
		pDC->MoveTo(rectItem.left+m_szTextPadding.cx,  rectItem.top+(rectItem.bottom-rectItem.top)/2);
		pDC->LineTo(rectItem.right, rectItem.top+(rectItem.bottom-rectItem.top)/2);
		pDC->SelectObject(pOldPen);
		DeleteObject(pen);
	}
	else if((lpDIS->itemState	& ODS_SELECTED)						&&
			(!(lpDIS->itemState	& ODS_GRAYED) || m_bHighlightGray)	&&						
			(lpDIS->itemAction	& (ODA_SELECT|ODA_DRAWENTIRE)))
	{
		// Selected Item
		CPen	pen;
		CBrush	brush;
		if(lpDIS->itemState & ODS_GRAYED)
		{
			pDC->SetTextColor(m_colGrayText);
			pen		.CreatePen			(PS_SOLID, 1, m_colHilightBorder);
			brush	.CreateSolidBrush	(m_colMenuGray);
		}
		else
		{
			pDC->SetTextColor(m_colText);
			pen		.CreatePen			(PS_SOLID, 1,  m_colMenuHilightBorder);
			brush	.CreateSolidBrush	(m_colMenuHilight);
		}

		CPen*	pOldPen		= pDC->SelectObject(&pen  );
		CBrush*	pOldBrush	= pDC->SelectObject(&brush);
		pDC->Rectangle(&lpDIS->rcItem);
		pDC->SelectObject(pOldPen  );
		pDC->SelectObject(pOldBrush);
		DeleteObject(pen);
		DeleteObject(brush);
	}
	else
	{
		// Grayed or Normal Item
		if(lpDIS->itemState & ODS_GRAYED)
			pDC->SetTextColor(m_colGrayText);
		else
			pDC->SetTextColor(m_colText);
		if(pItem->m_bIsTopLevel)
			pDC->FillSolidRect(&rectItem, m_colHead);
		else
			pDC->FillSolidRect(&rectItem, m_colBackground);
	}

	if(lpDIS->itemState & ODS_CHECKED)
	{
		// Checked Item
		CPen		pen;
		CPen*		pOldPen;
		CBrush		brush;
		CBrush*		pOldBrush;
		HBITMAP		hBmp;
		CBitmap*	pBmp;
		BITMAP		bmp;
		CSize		szBmp;
		CPoint		ptBmp;
		ZeroMemory(&bmp, sizeof(BITMAP));
		
		pen			.CreatePen(PS_SOLID, 1, m_colHilightBorder);
		brush		.CreateSolidBrush(m_colHorzBar);
		pOldPen		= pDC->SelectObject(&pen  );
		pOldBrush	= pDC->SelectObject(&brush);
		hBmp		= LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
		pBmp		= CBitmap::FromHandle(hBmp);
		pBmp		->GetBitmap(&bmp);
		szBmp		= CSize(bmp.bmWidth, bmp.bmHeight);
		ptBmp		= CPoint(rectImage.left+(m_szImage.cx-szBmp.cx)/2+1, rectImage.top+(m_szImage.cy-szBmp.cy)/2);
		pDC->SetBkColor		(m_colHorzBar);
		pDC->Rectangle		(rectImage.left, rectImage.top, rectImage.right, rectImage.bottom);
		pDC->DrawState		(ptBmp, szBmp, hBmp, DSS_NORMAL);
		pDC->SelectObject	(pOldPen  );
		pDC->SelectObject	(pOldBrush);
		DeleteObject(pen  );
		DeleteObject(brush);
		DeleteObject(hBmp );
	}
	else if((pItem->m_nImageEnabled >= 0) || (pItem->m_nImageDisabled >= 0))
	{
		// Item with Image
		if((lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemState & ODS_GRAYED) && m_bHighlightGray)
			pDC->SetBkColor(m_colMenuGray);
		else
			pDC->SetBkColor(m_colHorzBar);

		CImageList* pilEnabled	= (CImageList*) m_ImageLists[pItem->m_nListEnabled];
		CImageList* pilDisabled	= (CImageList*) m_ImageLists[pItem->m_nListDisabled];
		if(lpDIS->itemState & ODS_GRAYED)
		{
			// Grayed Item
			if(pilDisabled)
			{
				// Use Disabled ImageList
				pilDisabled->Draw(pDC, pItem->m_nImageDisabled, CPoint(rectImage.left, rectImage.top), ILD_TRANSPARENT);
			}
			else
			{
				// Use Enabled ImageList
				pilEnabled->Draw(pDC, pItem->m_nImageEnabled, CPoint(rectImage.left, rectImage.top), ILD_MASK);
			}
		}
		else
		{
			if (lpDIS->itemState	& ODS_SELECTED)	{
				// Normal Item
				pilDisabled->Draw(pDC, pItem->m_nImageEnabled, CPoint(rectImage.left, rectImage.top), ILD_TRANSPARENT);	
				pilEnabled->Draw (pDC, pItem->m_nImageEnabled, CPoint(rectImage.left - 2, rectImage.top - 2), ILD_TRANSPARENT);	
			} else { // Normal Item				
				pilEnabled->Draw(pDC, pItem->m_nImageEnabled, CPoint(rectImage.left, rectImage.top), ILD_TRANSPARENT);	
			}
		}
	}

	// Text
	pDC->SetBkMode(TRANSPARENT);
	if(pItem->m_bIsTopLevel)
		pDC->DrawText(pItem->m_sCaption, &rectText, DT_EXPANDTABS|DT_CENTER|DT_VCENTER);
	else
		pDC->DrawText(pItem->m_sCaption, &rectText, DT_EXPANDTABS|DT_LEFT|DT_VCENTER);
}

void CMenuEx::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	OnMeasureItem(lpMIS, m_pWnd);
}

void CMenuEx::OnMeasureItem(LPMEASUREITEMSTRUCT lpMIS, CWnd* pWnd)
{
	MENUITEMEX* pItem = (MENUITEMEX*) lpMIS->itemData;

	CDC* pDC = pWnd->GetDC();
	CFont* pOldFont = pDC->SelectObject(pWnd->GetFont());
	if(pItem->m_bIsTopLevel)
	{
		lpMIS->itemWidth	= pItem->m_sCaption.GetLength()*5;
		lpMIS->itemHeight	= 20;
	}
	else
	{
		lpMIS->itemWidth	= m_szImagePadding.cx+m_szImage.cx+m_szImagePadding.cx+m_szTextPadding.cx+pDC->GetTextExtent(pItem->m_sCaption).cx+m_szTextPadding.cx;
		lpMIS->itemHeight	= m_szTextPadding.cy+pDC->GetTextExtent(pItem->m_sCaption).cy+m_szTextPadding.cy;
		if(pItem->m_uiID == 0)
			lpMIS->itemHeight = m_szTextPadding.cy+1+m_szTextPadding.cy;
		else if(lpMIS->itemHeight < (UINT)(m_szImagePadding.cy+m_szImage.cy+m_szImagePadding.cy))
		{
			UINT nNewHeight = (UINT)(m_szImagePadding.cy+m_szImage.cy+m_szImagePadding.cy);
			UINT nDiff		= nNewHeight-lpMIS->itemHeight;
			m_szTextPadding.cy += nDiff/2;
			lpMIS->itemHeight = nNewHeight;
		}
	}
	pDC->SelectObject(pOldFont);
	pWnd->ReleaseDC(pDC);
}

void CMenuEx::UseToolBarImages(CToolBar* pToolBar)
{
	UINT	uiID;
	UINT	uiStyle;
	INT		nImage;
	for(INT nItem=0; nItem<pToolBar->GetCount(); nItem++)
	{
		pToolBar->GetButtonInfo(nItem, uiID, uiStyle, nImage);
		if(uiStyle != TBBS_BUTTON) continue;
		SetItemImages(uiID, nImage, pToolBar);
	}
}

///////////////////////
// Protected Methods //
///////////////////////

void CMenuEx::MakeOwnerDrawn(BOOL bTopLevel)
{
	CMenu*	pSubMenu;
	for(int ui=0; ui<GetMenuItemCount(); ui++)
	{
		MENUITEMEX* pItem = new MENUITEMEX();
		pItem->m_bIsTopLevel	= bTopLevel;
		pItem->m_nListEnabled	= -1;
		pItem->m_nListDisabled	= -1;
		pItem->m_nImageEnabled	= -1;
		pItem->m_nImageDisabled	= -1;
		GetMenuString(ui, pItem->m_sCaption, MF_BYPOSITION);
		pItem->m_uiID = GetMenuItemID(ui);
		ModifyMenu(ui, MF_BYPOSITION|MF_OWNERDRAW, pItem->m_uiID, (TCHAR*) pItem);
		m_TempItems.Add(pItem);

		if((pSubMenu = GetSubMenu(ui)) != NULL)
		{
			CMenuEx* pMenu = new CMenuEx(m_pWnd);
			m_TempMenus.Add(pMenu);
			pMenu->Attach(pSubMenu->GetSafeHmenu());
		}
	}
}

void CMenuEx::SetItemImages(UINT uiID, INT nImage, CToolBar* pToolBar)
{
	CMenuEx*	pMenu;
	MENUITEMEX* pItem;
	BOOL		bFound;
	INT_PTR		nListEnabled	= -1;
	INT_PTR		nListDisabled	= -1;
	CImageList*	pilEnabled;
	CImageList*	pilDisabled;
	for(INT nMenu=0; nMenu<m_TempMenus.GetSize(); nMenu++)
	{
		pMenu = (CMenuEx*) m_TempMenus[nMenu];
		for(INT nItem=0; nItem<pMenu->m_TempItems.GetSize(); nItem++)
		{
			pItem = (MENUITEMEX*) pMenu->m_TempItems[nItem];
			if(pItem->m_uiID == uiID)
			{
				// Check if ToolBar already included in this menu
				bFound = FALSE;
				for(INT nToolBar=0; nToolBar<pMenu->m_ToolBars.GetSize(); nToolBar++)
				{
					if(pToolBar == (CToolBar*) pMenu->m_ToolBars[nToolBar])
					{
						bFound			= TRUE;
						nListEnabled	= nToolBar;
						nListDisabled	= nToolBar+1;
						break;
					}
				}
				if(!bFound)
				{
					pMenu			->m_ToolBars.Add(pToolBar);
					nListEnabled	= pMenu->m_ImageLists.Add(new CImageList());
					nListDisabled	= pMenu->m_ImageLists.Add(new CImageList());
					pilEnabled		= pToolBar->GetToolBarCtrl().GetImageList();
					pilDisabled		= pToolBar->GetToolBarCtrl().GetDisabledImageList();
					if(pilEnabled )
						((CImageList*) pMenu->m_ImageLists[nListEnabled]) ->Create(pilEnabled );
					else
					{
						delete (CImageList*) pMenu->m_ImageLists[nListEnabled];
						pMenu->m_ImageLists[nListEnabled] = NULL;
					}
					if(pilDisabled)
						((CImageList*) pMenu->m_ImageLists[nListDisabled])->Create(pilDisabled);
					else
					{
						delete (CImageList*) pMenu->m_ImageLists[nListDisabled];
						pMenu->m_ImageLists[nListDisabled] = NULL;
					}
				}

				pItem->m_nListEnabled	= nListEnabled;
				pItem->m_nListDisabled	= nListDisabled;
				pItem->m_nImageEnabled	= nImage;
				pItem->m_nImageDisabled	= nImage;
			}
		}
	}
}
