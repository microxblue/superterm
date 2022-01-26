#include "StdAfx.h"
#include "GuiLib.h"

COLORREF CGuiLib::LightenColor(COLORREF col,double factor)
{
	if(factor>0.0&&factor<=1.0){
		BYTE red,green,blue,lightred,lightgreen,lightblue;
		red = GetRValue(col);
		green = GetGValue(col);
		blue = GetBValue(col);
		lightred = (BYTE)((factor*(255-red)) + red);
		lightgreen = (BYTE)((factor*(255-green)) + green);
		lightblue = (BYTE)((factor*(255-blue)) + blue);
		col = RGB(lightred,lightgreen,lightblue);
	}
	return(col);
}

COLORREF CGuiLib::DarkenColor(COLORREF col,double factor)
{
	if(factor>0.0&&factor<=1.0){
		BYTE red,green,blue,lightred,lightgreen,lightblue;
		red = GetRValue(col);
		green = GetGValue(col);
		blue = GetBValue(col);
		lightred = (BYTE)(red-(factor*red));
		lightgreen = (BYTE)(green-(factor*green));
		lightblue = (BYTE)(blue-(factor*blue));
		col = RGB(lightred,lightgreen,lightblue);
	}
	return(col);
}

COLORREF CGuiLib::GrayScale(COLORREF col)
{
	BYTE  dbGray;
	dbGray = (BYTE)( GetRValue(col) * (0.3) +  GetBValue(col) * (0.59) + GetGValue(col) * (0.11));				
	return (RGB(dbGray, dbGray, dbGray));
}

void CGuiLib::GradientFill (CDC *pDC, CRect rectBar, COLORREF cLeft, COLORREF cRight, BOOL bVertical)
{
        TRIVERTEX vert[2];	// for specifying range to gradient fill
        GRADIENT_RECT gRect;
        COLORREF FromCol =  cLeft;
        COLORREF ToCol   =  cRight;

        vert[0].Red   = (COLOR16)((FromCol&0x0000FF)<<8);
        vert[0].Green = (COLOR16)((FromCol&0x00FF00)<<0);
        vert[0].Blue  = (COLOR16)((FromCol&0xFF0000)>>8);
        vert[0].Alpha = 0;	// no fading/transparency
    				
        vert[1].Red   = (COLOR16)((ToCol&0x0000FF)<<8);
        vert[1].Green = (COLOR16)((ToCol&0x00FF00)<<0);
        vert[1].Blue  = (COLOR16)((ToCol&0xFF0000)>>8);
        vert[1].Alpha = 0;

        gRect.UpperLeft = 0;
        gRect.LowerRight = 1;
    				
        vert[0].x = rectBar.left;
        vert[0].y = rectBar.top;
        vert[1].x = rectBar.right;
        vert[1].y = rectBar.bottom;
        if (bVertical)	// vertically oriented?    
            pDC->GradientFill(vert,2,&gRect,1,GRADIENT_FILL_RECT_V);
        else 
            pDC->GradientFill(vert,2,&gRect,1,GRADIENT_FILL_RECT_H);
}

void CGuiLib::DrawTransparent (CDC *pDC, HICON hIcon, CPoint pt, CSize sz)
{
    CDC     dcMem;     
    CBitmap CacheBitmap;
    HBRUSH  hBrush = NULL;
    
    if (sz.cx == 0) {
        ICONINFO  IconInfo;
        GetIconInfo(hIcon, &IconInfo);
        sz.cx = IconInfo.xHotspot * 2; 
        sz.cy = IconInfo.xHotspot * 2;        
    }
     
    dcMem.CreateCompatibleDC (pDC); 
    CacheBitmap.CreateCompatibleBitmap(pDC, sz.cx, sz.cy);
    CBitmap* pBmpOld = dcMem.SelectObject (&CacheBitmap);          
    dcMem.DrawState(CPoint(0,0), sz, hIcon, DSS_NORMAL, hBrush);
    pDC->TransparentBlt (pt.x, pt.y, sz.cx, sz.cy, &dcMem, 0, 0, sz.cx, sz.cy, RGB(0,0,0));
    dcMem.SelectObject (pBmpOld);  
    CacheBitmap.DeleteObject(); 
    dcMem.DeleteDC();   
}
