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

#pragma once

#define OBM_CHECK 32760

class CMenuEx : public CMenu
{
	// Construction & Destruction
public:
	struct MENUITEMEX
	{
		UINT		m_uiID;
		CString		m_sCaption;
		BOOL		m_bIsTopLevel;
		INT_PTR		m_nListEnabled;
		INT_PTR		m_nListDisabled;
		INT			m_nImageEnabled;
		INT			m_nImageDisabled;
	};
	CMenuEx(CWnd* pWnd);
	virtual	~CMenuEx();

	// Public methods
public:
			BOOL	LoadMenu		(UINT nIDResource);
			BOOL	LoadMenu		(LPCTSTR lpszResourceName);
			BOOL	Attach			(HMENU hMenu);
	virtual	void	DrawItem		(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual	void	MeasureItem		(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	static	void	OnMeasureItem	(LPMEASUREITEMSTRUCT lpMeasureItemStruct, CWnd* pWnd);
			void	UseToolBarImages(CToolBar* pToolBar);

	// Protected methods
protected:
			void	MakeOwnerDrawn	(BOOL bTopLevel);
			void	SetItemImages	(UINT uiID, INT nImage, CToolBar* pToolBar);

	// Public properties
public:
	static	COLORREF	m_colHead;			// Default: COLOR_MENUBAR
	static	COLORREF	m_colBackground;	// Default: COLOR_MENU
	static	COLORREF	m_colHorzBar;		// Default: COLOR_MENUBAR
	static	COLORREF	m_colText;			// Default: COLOR_MENUTEXT
	static	COLORREF	m_colGrayText;		// Default: COLOR_GRAYTEXT
	static	COLORREF	m_colSeparator;		// Default: COLOR_MENUBAR
	static	COLORREF	m_colHilightBorder;	// Default: COLOR_3DSHADOW	
	static	COLORREF	m_colMenuHilight;	// Default: COLOR_MENUHILIGHT
	static	COLORREF	m_colHilight;		// Default: COLOR_3DHILIGHT
	static	COLORREF	m_colMenuGray;      // Default: COLOR_MENU
	static	COLORREF	m_colMenuHilightBorder;
	static	BOOL		m_bHighlightGray;	// Default: FALSE
	static	CSize		m_szImage;			// Default: 16x16
	static	CSize		m_szImagePadding;	// Default: 3x3
	static	CSize		m_szTextPadding;	// Default: 5x3

	// Protected properties
protected:
	CWnd*		m_pWnd;
	CPtrArray	m_TempMenus;
	CPtrArray	m_TempItems;
	CPtrArray	m_ToolBars;
	CPtrArray	m_ImageLists;

	// Helper-Defines
#define DECLARE_MENUEX()																\
protected:																				\
	afx_msg void OnMeasureItem(int nIDControl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

#define ON_MENUEX_MESSAGES()	\
	ON_WM_MEASUREITEM()

#define IMPLEMENT_MENUEX(theClass, baseClass)												\
	void theClass::OnMeasureItem(int, LPMEASUREITEMSTRUCT lpMeasureItemStruct)	\
	{																						\
		CMenuEx::OnMeasureItem(lpMeasureItemStruct, this);									\
	}
};
