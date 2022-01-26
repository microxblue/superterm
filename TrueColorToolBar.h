
#ifndef TRUECOLORTOOLBAR_H_
#define TRUECOLORTOOLBAR_H_

#pragma once

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CTrueColorToolBar

class CTrueColorToolBar : public CToolBar
{	
	#define IMAGE_TYPE_NORMAL    0
	#define IMAGE_TYPE_DISABLED  1
	#define IMAGE_TYPE_HOT       2
	#define IMAGE_TYPE_ALL       3

// Construction
public:
	CTrueColorToolBar();

	typedef struct {
		BYTE	rgbtRed;
		BYTE	rgbtGreen;
		BYTE	rgbtBlue;
		//BYTE    rgbtResv;
	} RGBX;

// Attributes
private:
	BOOL m_bDropDown;
	COLORREF	m_clrBtnShadow;
	COLORREF	m_clrBtnHilight;
	COLORREF	m_clrBtnFace;

	struct stDropDownInfo {
	public:
		UINT  uButtonID;
		UINT  uMenuID;
		CWnd* pParent;
	};
	
	CArray <stDropDownInfo, stDropDownInfo&> m_lstDropDownButton;
	
// Operations
public:
	COLORREF    ProcessBitmap(UINT uToolType, CBitmap *pSrcBitmap, CBitmap *pDstBitmap);
	HBITMAP		CopyBitmap(HBITMAP hSourceHbitmap);
	COLORREF	DarkenColor(COLORREF col,double factor);
	COLORREF	LightenColor(COLORREF col,double factor);
	COLORREF	GrayScale(COLORREF col);
	void		DrawHotBitmap(CBitmap *pSrcBitmap, CBitmap *pDstBitmap);
	void		GradientFill (CDC *pDC, CRect rectBar, COLORREF cLeft, COLORREF cRight, BOOL bVertical);
	void		DrawTransparentBitmap (CDC *pDC, HICON hIcon, CPoint pt, CSize sz = CSize(0,0));

	BOOL LoadTrueColorToolBar(int  nBtnWidth,
							  UINT uToolBar,
							  UINT uToolBarHot		= 0,
							  UINT uToolBarDisabled = 0);

	void AddDropDownButton(CWnd* pParent, UINT uButtonID, UINT uMenuID);

private:
	BOOL SetTrueColorToolBar(UINT uToolBarType,
		                     UINT uToolBar,
						     int  nBtnWidth);

	void DrawSeparator(CDC *dc, CRect *rcSep) const;
	void DrawGripper( CDC *dc, CRect *rectWindow = NULL) const;

// Implementation
public:
	virtual ~CTrueColorToolBar();

	// Generated message map functions
protected:
	void EraseNonClient();
	afx_msg void OnToolbarDropDown(NMHDR* pnmh, LRESULT* plRes);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);	
	afx_msg void OnNcPaint();
	afx_msg	BOOL OnEraseBkgnd(CDC*pDC);
};


#endif // TRUECOLORTOOLBAR_H_
