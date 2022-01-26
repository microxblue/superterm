
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CTrueColorToolBar

class CGuiLib
{	
public:
	static void DrawTransparent (CDC *pDC, HICON hIcon, CPoint pt, CSize sz);
	static void CGuiLib::GradientFill (CDC *pDC, CRect rectBar, COLORREF cLeft, COLORREF cRight, BOOL bVertical);
	static COLORREF GrayScale(COLORREF col);
	static COLORREF DarkenColor(COLORREF col,double factor);
	static COLORREF LightenColor(COLORREF col,double factor);
};

