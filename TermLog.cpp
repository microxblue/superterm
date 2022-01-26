#include "stdafx.h"
#include "TermLog.h"
#include "MenuEx.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(CTermLog, CWnd)

/*
const WORD CTermLog::m_extAscTable[128] =
{
    0x00C7,  0x00FC,  0x00E9,  0x00E2,  0x00E4,  0x00E0,  0x00E5,  0x00E7,
    0x00EA,  0x00EB,  0x00E8,  0x00EF,  0x00EE,  0x00EC,  0x00C4,  0x00C5,
    0x00C9,  0x00E6,  0x00C6,  0x00F4,  0x00F6,  0x00F2,  0x00FB,  0x00F9,
    0x00FF,  0x00D6,  0x00DC,  0x00A2,  0x00A3,  0x00A5,  0x20A7,  0x0192,
    0x00E1,  0x00ED,  0x00F3,  0x00FA,  0x00F1,  0x00D1,  0x00AA,  0x00BA,
    0x00BF,  0x2310,  0x00AC,  0x00BD,  0x00BC,  0x00A1,  0x00AB,  0x00BB,
    0x2591,  0x2592,  0x2593,  0x2502,  0x2524,  0x2561,  0x2562,  0x2556,
    0x2555,  0x2563,  0x2551,  0x2557,  0x255D,  0x255C,  0x255B,  0x2510,
    0x2514,  0x2534,  0x252C,  0x251C,  0x2500,  0x253C,  0x255E,  0x255F,
    0x255A,  0x2554,  0x2569,  0x2566,  0x2560,  0x2550,  0x256C,  0x2567,
    0x2568,  0x2564,  0x2565,  0x2559,  0x2558,  0x2552,  0x2553,  0x256B,
    0x256A,  0x2518,  0x250C,  0x2588,  0x2584,  0x258C,  0x2590,  0x2580,
    0x03B1,  0x00DF,  0x0393,  0x03C0,  0x03A3,  0x03C3,  0x00B5,  0x03C4,
    0x03A6,  0x0398,  0x03A9,  0x03B4,  0x221E,  0x03C6,  0x03B5,  0x2229,
    0x2261,  0x00B1,  0x2265,  0x2264,  0x2320,  0x2321,  0x00F7,  0x2248,
    0x00B0,  0x2219,  0x00B7,  0x221A,  0x207F,  0x00B2,  0x25A0,  0x00A0
};
*/
int xtoi (LPCTSTR str, int *pos)
{
  int     u;
  int     n;
  int     h;
  TCHAR   c;
  LPCTSTR p;

  h = 0;
  p = str;

  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }

  if (*str == '-') {
	n = 1;
  } else {
	n = 0;
  }

  //
  // skip preceeding zeros
  //
  while (*str && *str == '0') {
	u = 0;
    str += 1;
  }

  //
  // skip preceeding white space
  //
  if (*str && (*str == 'x' || *str == 'X')) {
    h    = 1;
    str += 1;
  }

  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a' && c <= 'f') {
      c -= 'a' - 'A';
    }

    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      u = u << 4 | c - (c >= 'A' ? 'A' - 10 : '0');
	  if (h == 0 && c >= 'A')
		  break;
    } else {
      break;
    }

    c = *(str++);
  }


  if (n == 1) {
    if (pos) {
	  *pos = 0;
    }
	return -1;
  } else {
	if (pos) {
	  *pos = (int)(str - p);
    }
    if (h == 1) return u;
	else return _tstoi(p);
  }

}

CTermLog::CTermLog(CPoint sz)
{
	//m_pDevice      = &m_SerialPort;
	//m_pDevice      = &m_Pipe;
	m_pDevice      = &m_Telnet;
	m_TermCfg.TelnetEnable = 1;

	m_bClose       = FALSE;
	m_FirstAttempt = 0;

	m_pCacheBitmap = NULL;

	m_ptMargin.y = 1;
	m_ptMargin.x = 1;
	m_ptChar.y   = LOG_FONT_HEIGHT;
	m_ptChar.x   = LOG_FONT_WIDTH;
	m_ptScreen   = sz;

	m_colTable[0]  = RGB(  0,    0,   0);
	m_colTable[1]  = RGB(128,    0,   0);
	m_colTable[2]  = RGB(  0,  128,   0);
	m_colTable[3]  = RGB(128,  128,   0);
	m_colTable[4]  = RGB(  0,    0, 128);
	m_colTable[5]  = RGB(128,    0, 128);
	m_colTable[6]  = RGB(  0,  128, 128);
	m_colTable[7]  = RGB(192,  192, 192);

	m_colTable[8]  = RGB(128,  128, 128);
	m_colTable[9]  = RGB(255,    0,   0);
	m_colTable[10] = RGB(  0,  255,   0);
	m_colTable[11] = RGB(255,  255,   0);
	m_colTable[12] = RGB(  0,    0, 255);
	m_colTable[13] = RGB(255,    0, 255);
	m_colTable[14] = RGB(  0,  255, 255);
	m_colTable[15] = RGB(255,  255, 255);

	m_dbDefColor   = 0x07;

	m_nHistoryLine = 100000;

	m_IdleTimer    = 0;
	m_strPendingToSend.Empty();

	Initialize ();
}

CTermLog::~CTermLog()
{
	if (m_pCacheBitmap != NULL)
	{
		m_pCacheBitmap->DeleteObject();
		delete m_pCacheBitmap;
	}
	m_Font.DeleteObject();
	ClearBuffer();

	if (m_hBinFile) {
		fclose(m_hBinFile);
		m_hBinFile = NULL;
	}

	if (m_hTxtFile) {
		fclose(m_hTxtFile);
		m_hTxtFile = NULL;
	}
}

BEGIN_MESSAGE_MAP(CTermLog, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_WM_CHAR()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_USER, OnUserMessage)
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID__COPY_SELECTED, OnCopySelected)
	ON_COMMAND(ID__COPY_ALL, OnCopyAll)
	ON_COMMAND(ID__CLEAR_SCREEN, OnClearScreen)
	ON_COMMAND(ID__CLEAR_HISTORY, OnClearHistory)
	ON_COMMAND(ID__PASTE, OnPaste)
	ON_COMMAND(ID_SAVE_PASTE, OnSavePaste)
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CTermLog::Initialize()
{
	m_bSelecting  = FALSE;
	m_bLineLastChar = FALSE;
	m_nTotalLines = 0;
	m_ptFirstVisible.x = 0;
	m_ptFirstVisible.y = 0;
	m_ptCaret.x = 0;
	m_ptCaret.y = 0;

	m_posDispTopLine  = NULL;
	m_posTermCurrLine = NULL;
	m_nCurrentMaxLineWidth = 0;

	m_ptScrollRaw.x = 0;
	m_ptScrollRaw.y = 0;
	m_ptStartSelection.x = -1;
	m_ptStartSelection.y = -1;
	m_ptEndSelection.x   = -1;
	m_ptEndSelection.y   = -1;
	m_ptCurrent.x =0;
	m_ptCurrent.y =0;
	m_iDeltaPerLine     = 0;
	m_iAccumDelta       = 0;
	m_nSetVScrollPos = 0;
	m_nLastTopLine = 0;
	m_posCheckLine = NULL;
	m_TabSize     = 8;
	m_colDefFore  = RGB(192,  192,  192);
	m_colDefBack  = RGB(0,      0,    0);
	m_colSelFore  = RGB(0,      0,    0);
	m_colSelBack  = RGB(255,  255,  255);
	m_colCurrFore = m_colDefFore;
	m_colCurrBack = m_colDefBack;

	m_bRawLog     = FALSE;
	m_bConfigInProgress = FALSE;

	m_dbCurrColor = m_dbDefColor;

	m_hBinFile = NULL;
	m_hTxtFile = NULL;

	m_pThread  = NULL;
	
	m_hEventAppQuit    = CreateEvent(NULL, TRUE,  FALSE, NULL);
	m_hEventServerQuit = CreateEvent(NULL, TRUE,  FALSE, NULL);
  
  m_AnsiMode = VT_UTF8;
}

// CLogCtrl message handlers
BOOL CTermLog::Create(DWORD dwStyle, RECT& rect, CWnd* pParentWnd, UINT nID)
{
	// Create window
	return CWnd::CreateEx(0, NULL, NULL, dwStyle, rect, pParentWnd, nID, NULL);
}

BOOL CTermLog::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CTermLog::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	GetClientRect( &m_rcClientRect );

	if (m_rcClientRect.Height() &&  m_rcClientRect.Width()) {
		m_ptMargin.y = (m_rcClientRect.Height() - m_ptScreen.y * m_ptChar.y) / 2;
		m_ptMargin.x = (m_rcClientRect.Width()  - m_ptScreen.x * m_ptChar.x)  / 2;
		//TRACE ("CX=%d  CY=%d  MX=%d  MY=%d\n",m_ptScreen.x, m_ptScreen.y, m_ptMargin.x, m_ptMargin.y);
	}

	if (m_pCacheBitmap)
		delete m_pCacheBitmap;

	m_pCacheBitmap = new CBitmap;
	VERIFY(m_pCacheBitmap->CreateCompatibleBitmap(GetDC(), m_rcClientRect.Width(), m_rcClientRect.Height()));

	Invalidate ();
}

int CTermLog::StartTelnetListen ()
{
	PostMessage (WM_USER, CODE_THREAD_EXIT, THREAD_ID_CONNECT);
	return 0;
}

int CTermLog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	UpdateParam ();
	m_ptScreen.x = m_TermCfg.TermCol;
	m_ptScreen.y = m_TermCfg.TermRow;
	m_AnsiMode   = m_TermCfg.TermType ? VT_UTF8 : VT_ANSI;

	LOGFONT lfBaseFont;
	memset(&lfBaseFont, 0, sizeof(lfBaseFont));
	lfBaseFont.lfWidth  = 10;
	lfBaseFont.lfHeight = 12;
	lfBaseFont.lfCharSet = DEFAULT_CHARSET;
	lfBaseFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lfBaseFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfBaseFont.lfQuality = DEFAULT_QUALITY;
	lfBaseFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	_tcscpy (lfBaseFont.lfFaceName, _T("Terminal"));
	m_Font.CreateFontIndirect( &lfBaseFont );

	ULONG       ulScrollLines ;
	SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0) ;

	// ulScrollLines usually equals 3 or 0 (for no scrolling)
	// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
	if (ulScrollLines)
		m_iDeltaPerLine = WHEEL_DELTA / ulScrollLines ;
	else
		m_iDeltaPerLine = 0 ;

	CreateSolidCaret(2, m_ptChar.y);

	IntializeScreen ();

	StartTelnetListen ();

	TryConnection();

	SetTimer(CONNECT_CHECK_TIMER_TID, 100, 0);

	return 0;
}

void CTermLog::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	DrawClient (&dc);

	SetCaretPosition ();
}

void CTermLog::ClearBuffer()
{
	LPLINEDATA  pLineData;

	for(POSITION nPos = m_stlLine.GetHeadPosition(); nPos;)
	{
		pLineData = &(m_stlLine.GetNext(nPos));
		delete (pLineData->pszLineText);
		delete (pLineData->pdbLineAttr);
	}
	m_stlLine.RemoveAll ();

	Initialize ();

	Refresh ();
}

int CTermLog::GetLineBlockWithSameAttribute(LPLINEDATA pLineData, int nPos)
{
	int	 nIdx;
	BYTE dbAttribute;

	if (pLineData->pdbLineAttr == NULL) {
		return pLineData->nLineLen;
	} else {
		dbAttribute = pLineData->pdbLineAttr[nPos];
		for (nIdx = nPos + 1; nIdx < pLineData->nLineLen; nIdx++) {
			if (!(dbAttribute ==  pLineData->pdbLineAttr[nIdx])) {
				break;
			}
		}
		return nIdx - nPos;
	}
}

BOOL CTermLog::AdjustSelection(int nCurrentLine, int *pPos, int *pLen)
{
	int   nOffset = *pPos;
	int	  nLen    = *pLen;
	BOOL  bFlag = FALSE;

	if (m_ptEndSelection == m_ptStartSelection)
		return bFlag;

	nCurrentLine += m_ptFirstVisible.y;

	CPoint  ptStart (m_ptStartSelection);
	CPoint  ptEnd   (m_ptEndSelection);

	if (ptStart.y > ptEnd.y) {
		ptEnd   = m_ptStartSelection;
		ptStart = m_ptEndSelection;
	} else if  (ptStart.y == ptEnd.y) {
		if (ptStart.x > ptEnd.x) {
			ptEnd.x    = m_ptStartSelection.x;
			ptStart.x  = m_ptEndSelection.x;
		}
	}

	if (nCurrentLine == ptStart.y && nCurrentLine == ptEnd.y) { // Same line
		bFlag   = TRUE;
		if (ptEnd.x > nLen) {
			ptEnd.x = nLen;
		}
		if (ptStart.x > nLen) {
			ptStart.x = nLen ;
		}
		nOffset = ptStart.x;
		nLen    = ptEnd.x - ptStart.x;
	} else if (nCurrentLine == ptStart.y) {  // Header line
		if (ptStart.x > nLen) {
			ptStart.x = nLen;
		} else {
			bFlag   = TRUE;
		}
		nLen    = nOffset + nLen - ptStart.x;
		nOffset = ptStart.x;
	} else if (nCurrentLine == ptEnd.y) {    // Tail line
		bFlag   = TRUE;
		if (ptEnd.x <= nLen) {
			nLen = ptEnd.x;
		}
		nOffset = 0;
	} else if (nCurrentLine > ptStart.y && nCurrentLine < ptEnd.y) {
		bFlag   = TRUE;       // Whole line
	}

	*pPos = nOffset;
	*pLen = nLen;

	return bFlag;
}

void CTermLog::DrawClient(CDC * pDC)
{
	CDC memDC;
	VERIFY(memDC.CreateCompatibleDC(pDC));


	CBitmap *pOldBitmap = memDC.SelectObject (m_pCacheBitmap);
	CFont   *pOldFont   = memDC.SelectObject (&m_Font);

	memDC.FillSolidRect(&m_rcClientRect, m_colDefBack);

	int nDrawLines;
	if (m_nTotalLines - m_ptFirstVisible.y < m_ptScreen.y) {
		nDrawLines = m_nTotalLines - m_ptFirstVisible.y;
	} else {
		nDrawLines = m_ptScreen.y;
	}

	CRect      rc;
	LPLINEDATA  pLineData;
	POSITION nPos  = m_posDispTopLine;
	int y =  m_ptMargin.y;
	int x;
	memDC.SetBkColor  ( m_colCurrBack);
	memDC.SetTextColor( m_colCurrFore);

	for(int nLines = 0; nLines < nDrawLines; nLines++)
	{
		pLineData = &(m_stlLine.GetNext(nPos));
		rc.top    = y;
		rc.bottom = rc.top + m_ptChar.y;
		rc.right   = m_ptMargin.x;
		x = m_ptMargin.x - m_ptFirstVisible.x * m_ptChar.x;

		int		 nSelPos  = 0;
		int      nSelLen  = pLineData->nLineLen;
		int		 nSelFlag = 0;

		if (AdjustSelection(nLines, &nSelPos, &nSelLen)) {
			nSelFlag |= 1; // Mark line selected
			if ((nSelPos == 0) && (nSelLen == pLineData->nLineLen)) {
				// Whole line is selected, skip the normal print
				nSelFlag |= 2;
			}
		}

		if (!(nSelFlag & 2)) {  // Not selected for the while line
			int		 nPrint;
			int      nIdx = 0;
			COLORREF colNewBack, colNewFore;
			TCHAR   *pChar = pLineData->pszLineText;
			int		 nLen = pLineData->nLineLen;
			while (nIdx < nLen)
			{
				nPrint     = GetLineBlockWithSameAttribute(pLineData, nIdx);
				if (pLineData->pdbLineAttr) {
					colNewFore = m_colTable[pLineData->pdbLineAttr[nIdx] & 0x0F];
					colNewBack = m_colTable[(pLineData->pdbLineAttr[nIdx]>>4) & 0x0F];
				} else {
					colNewFore = m_colDefFore;
					colNewBack = m_colDefBack;
				}
				if (colNewBack != m_colCurrBack) {
					m_colCurrBack = colNewBack;
					memDC.SetBkColor  (m_colCurrBack);
				}
				if (colNewFore != m_colCurrFore) {
					m_colCurrFore = colNewFore;
					memDC.SetTextColor  (m_colCurrFore);
				}
				rc.left  = rc.right;
				rc.right = rc.left + nPrint * m_ptChar.x;
				memDC.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, rc, pChar, nPrint, NULL);
				pChar += nPrint;
				x	  += nPrint * m_ptChar.x;
				nIdx  += nPrint;
			}
		}

		if (nSelFlag) { // Selected in the line
			// Selection
			if (m_colSelBack != m_colCurrBack)
			{
				m_colCurrBack = m_colSelBack;
				memDC.SetBkColor  (m_colCurrBack);
			}
			if (m_colSelFore != m_colCurrFore)
			{
				m_colCurrFore = m_colSelFore;
				memDC.SetTextColor  (m_colCurrFore);
			}

			x = m_ptMargin.x + (nSelPos - m_ptFirstVisible.x) * m_ptChar.x;
			rc.left  = x;
			rc.right = rc.left + nSelLen * m_ptChar.x;
			memDC.ExtTextOut (x, y, ETO_CLIPPED | ETO_OPAQUE, rc, pLineData->pszLineText + nSelPos, nSelLen, NULL);
		}

		y += m_ptChar.y;
	}

	// Copy it from memory to screen
	pDC->BitBlt(m_rcClientRect.left, m_rcClientRect.top, m_rcClientRect.Width(), m_rcClientRect.Height(), &memDC,
		m_rcClientRect.left, m_rcClientRect.top, SRCCOPY);
	memDC.SelectObject(pOldFont);
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
}

void CTermLog::TrimLine(POSITION pos, BOOL bChangeLen)
{
	LPLINEDATA   lpLineData;

	lpLineData    = &(m_stlLine.GetAt(pos));
	// Trim the line
	int nIdx;
	for (nIdx = lpLineData->nLineLen - 1; nIdx >= 0; nIdx--) {
		if (lpLineData->pszLineText[nIdx] != _T(' ')) {
			break;
		}
	}
	if (bChangeLen)
		lpLineData->nLineLen = nIdx + 1;
}

void CTermLog::AddBlankLine()
{
	LINEDATA     LineData;
	LPLINEDATA   lpLineData;
	static int a=0;
	int	nLineMaxLen = m_ptScreen.x;
	memset(&LineData, 0, sizeof(LineData));

	m_nTotalLines = (int)m_stlLine.GetCount();
	if (m_nTotalLines  >= m_ptScreen.y) {
		if (m_posCheckLine == NULL) {
			m_posCheckLine = m_stlLine.GetHeadPosition();
		}

		TrimLine (m_posCheckLine, TRUE);

		lpLineData    = &(m_stlLine.GetNext(m_posCheckLine));
		if (lpLineData->pdbLineAttr){
			delete (lpLineData->pdbLineAttr);
			lpLineData->pdbLineAttr = NULL;
		}
	}

	if (m_nTotalLines  > m_nHistoryLine) {
		lpLineData = &m_stlLine.GetHead();
		delete (lpLineData->pszLineText);
		delete (lpLineData->pdbLineAttr);
		m_stlLine.RemoveHead ();
		if (m_ptFirstVisible.y > 0)
			m_ptFirstVisible.y --;
	}

	LineData.pszLineText = new TCHAR [nLineMaxLen + 1];
	LineData.pdbLineAttr = new BYTE  [nLineMaxLen];
	for (int nIdx = 0; nIdx < nLineMaxLen; nIdx++) {
		LineData.pszLineText[nIdx] = _T(' ');
		LineData.pdbLineAttr[nIdx] = 0;
	}
	LineData.pszLineText[nLineMaxLen] = 0;
	LineData.nLineLen    = nLineMaxLen;
	m_stlLine.AddTail (LineData);

	m_nTotalLines = (int)m_stlLine.GetCount();

	if (m_nTotalLines  > m_ptScreen.y) {
		m_nSetVScrollPos =  1;
		SetTopLine (m_nTotalLines - m_ptScreen.y);
	} else {
		m_nSetVScrollPos = -1;
		SetTopLine (0);
	}
}

void CTermLog::IntializeScreen()
{
	for (int i=0; i<m_ptScreen.y; i++) {
		AddBlankLine ();
	}
	m_posTermTopLine  = m_stlLine.GetHeadPosition();
	m_posTermCurrLine = m_stlLine.GetHeadPosition();
}

void CTermLog::Test ()
{

/*
	TCHAR c;
	for (int i=0 ;i < 0x10cec+0x2000; i++) {
		c = fgetc(m_fp);
		CharHandler (c);
	}
	for (int i=0; i<1000; i++) {
	LineData.pszLineText = new TCHAR [nLineMaxLen + 1];
	LineData.pdbLineAttr = new BYTE  [nLineMaxLen];
	for (int j = 0; j < nLineMaxLen; j++) {
	LineData.pszLineText[j] = _T('0') + (j % 10) + (i%10);
	LineData.pdbLineAttr[j] = j+1;
	}
	LineData.pszLineText[nLineMaxLen] = 0;
	LineData.nLineLen    = nLineMaxLen;
	m_stlLine.AddTail (LineData);
	}
	m_nTotalLines = (int)m_stlLine.GetCount();
	SetTopLine (m_ptFirstVisible.y);
	*/
	/*
	m_ptStartSelection.x = 0;
	m_ptStartSelection.y = 10;
	m_ptEndSelection.x   = 100;
	m_ptEndSelection.y   = 11;
	*/
}

int CTermLog::GetLineLength(int nLine)
{
	POSITION nPos = m_stlLine.FindIndex (nLine);
	if (!nPos)
		return 0;
	LPLINEDATA  pLineData = &(m_stlLine.GetAt(nPos));

	return pLineData->nLineLen;
}

int CTermLog::CharPositionFromPoint(CPoint pt, CPoint &pos)
{
	int		nResult  = 0;

	pos.x = m_ptFirstVisible.x +  (pt.x - m_ptMargin.x) / m_ptChar.x;
	pos.y = m_ptFirstVisible.y +  (pt.y - m_ptMargin.y) / m_ptChar.y;

	if (pos.y <  0) {
		pos.y = 0;
		nResult |= 1;
	}

	if (pos.y >= m_nTotalLines) {
		pos.y = m_nTotalLines - 1;
		nResult |= 2;
	}
	if (pos.x <  0) {
		pos.x = 0;
		nResult |= 4;
	}

	int nLength = GetLineLength (pos.y);
	if (pos.x > nLength) {
		pos.x = nLength;
		nResult |= 8;
	}

	if (!m_rcClientRect.PtInRect (pt))
		return -1;
	else
		return nResult;
}

void CTermLog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint  pos;

	SetFocus();

	// Capture mouse
	SetCapture();

	// Check for valid selection
	if (!m_bSelecting)	{
		// Set selection start position
		CharPositionFromPoint (point, m_ptStartSelection);
		m_ptEndSelection = m_ptStartSelection;

		// Set caret position
		m_ptCaret = m_ptStartSelection;

	}	else	{
		// Set selection end position
		CharPositionFromPoint (point, m_ptEndSelection);

		// Set caret position
		m_ptCaret = m_ptEndSelection;
	}

	// Update screen
	Refresh ();

	CWnd::OnLButtonDown(nFlags, point);
}

void CTermLog::DeSelect()
{
	m_ptStartSelection.x = -1;
	m_ptEndSelection.x = -1;
	m_ptStartSelection.y = -1;
	m_ptEndSelection.y = -1;
	// Update screen
	Invalidate ();
}

void CTermLog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();

	StopMouseRepeat ();

	if (m_ptStartSelection == m_ptEndSelection) {
		// Clear selection
		DeSelect();
	} else {
		// Update screen
		Invalidate ();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CTermLog::Refresh()
{
	if( GetSafeHwnd() )
	{
		UpdateVScroll ();
		UpdateHScroll ();
		SetCaretPosition ();
		Invalidate ();
	}
}

void CTermLog::StartMouseRepeat(const CPoint& cPoint, int iDelta, WORD nSpeed)
{
	m_cMouseRepPoint = cPoint;
	m_nMouseRepSpeed = nSpeed;
	m_iMouseRepDelta = iDelta;
	if(!m_bIsMouseRepActive) {
		m_bIsMouseRepActive = true;
		m_nMouseRepCounter = nSpeed;
		SetTimer(MOUSEREP_TIMER_TID, MOUSEREP_TIMER_ELAPSE, NULL);
	}
}

void CTermLog::StopMouseRepeat()
{
	if(m_bIsMouseRepActive) {
		m_bIsMouseRepActive = false;
		KillTimer(MOUSEREP_TIMER_TID);
	}
}

void CTermLog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint  pos;
	int nResult;

	if ((nFlags & MK_LBUTTON) && (m_ptStartSelection.y != -1) && (m_ptStartSelection.x != -1))	{

		// Get mouse position
		nResult = CharPositionFromPoint (point, m_ptEndSelection);
		if (nResult == -1) {
			int iRepSpeed = 0;
			int iDelta = 0;
			if(point.y < m_rcClientRect.top) {
				iDelta = -1;
				iRepSpeed = (int)m_rcClientRect.top + 1 - (int)point.y;
			} else if(point.y > m_rcClientRect.bottom ) {
				iDelta = 1;
				iRepSpeed = (int)point.y - (int)m_rcClientRect.bottom + 1;
			}
			if(iDelta != 0) {
				iRepSpeed /= 15;
				if(iRepSpeed > 5) {
					iRepSpeed = 6;
				}
				StartMouseRepeat(point, iDelta, (short)(7 - iRepSpeed));
			}
			m_cMouseRepPoint = point; // make sure we always have the latest point
		} else {
			StopMouseRepeat ();

			// Set caret position
			//m_ptCaret = m_ptEndSelection;
		}
		m_ptCaret = m_ptEndSelection;
		Refresh ();
	} else {
		StopMouseRepeat ();
	}

	//TRACE("(%d,%d)-(%d,%d)\n", m_ptStartSelection.y, m_ptStartSelection.x, m_ptEndSelection.y, m_ptEndSelection.x) ;
	CWnd::OnMouseMove(nFlags, point);
}

void CTermLog::UpdateHScroll()
{
	SCROLLINFO  si ;

	si.cbSize = sizeof (si) ;
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
	si.nMin   = 0 ;
	if (m_nCurrentMaxLineWidth > m_ptScreen.x)
		si.nMax   = m_nCurrentMaxLineWidth - 1;
	else
		si.nMax   = 1;
	si.nPage  = m_ptScreen.x;
	SetScrollInfo (SB_HORZ, &si, TRUE);
}

void CTermLog::UpdateVScroll()
{
	SCROLLINFO  si ;

	si.cbSize = sizeof (si) ;
	si.fMask  = SIF_RANGE | SIF_PAGE |SIF_DISABLENOSCROLL;
	si.nMin   = 0;
	if (m_nTotalLines > m_ptScreen.y) {
		si.nMax   = m_nTotalLines - 1;
	} else {
		si.nMax   = 1;
	}
	si.nPage  = m_ptScreen.y;
	if (m_nSetVScrollPos) {
		if (m_nSetVScrollPos < 0)
			si.nPos = 0;
		else
			si.nPos = si.nMax;
		m_nSetVScrollPos = 0;
		si.fMask |= SIF_POS;
	}
	SetScrollInfo (SB_VERT , &si, TRUE);
}

void CTermLog::HScroll(UINT nSBCode, UINT nPos)
{
	SCROLLINFO  si ;
	// Get all the vertical scroll bar information

	si.cbSize = sizeof (si) ;
	si.fMask  = SIF_ALL ;

	// Save the position for comparison later on

	GetScrollInfo(SB_HORZ,&si);
	int iHorzPos = si.nPos ;

	switch (LOWORD (nSBCode))
	{
	case SB_TOP:
		si.nPos = si.nMin ;
		break ;

	case SB_BOTTOM:
		si.nPos = si.nMax ;
		break ;

	case SB_LINELEFT:
		si.nPos -= 1 ;
		break ;

	case SB_LINERIGHT:
		si.nPos += 1 ;
		break ;

	case SB_PAGELEFT:
		si.nPos -= si.nPage ;
		break ;

	case SB_PAGERIGHT:
		si.nPos += si.nPage ;
		break ;

	case SB_THUMBPOSITION:
		si.nPos = si.nTrackPos ;
		break ;

	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos ;
		break ;
	default :
		break ;
	}
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.

	si.fMask = SIF_POS ;
	SetScrollInfo (SB_HORZ, &si, FALSE);
	GetScrollInfo (SB_HORZ, &si);

	//if (si.nPos != iHorzPos) {
	m_ptFirstVisible.x = si.nPos;
	Refresh ();
	//}
}

void CTermLog::SetTopLine (int nLine)
{
	int nDelta = nLine - m_ptFirstVisible.y;
	m_ptFirstVisible.y = nLine;

	if (m_posDispTopLine == NULL) {
		m_posDispTopLine = m_stlLine.GetHeadPosition();
	}

	if (nDelta)	{
		if (abs(nDelta) < m_ptScreen.y * 2) {
			if (nDelta < 0) {
				while (nDelta++) {
					m_stlLine.GetPrev(m_posDispTopLine);
				}
			} else {
				while (nDelta--) {
					m_stlLine.GetNext(m_posDispTopLine);
				}
			}
		} else {
			m_posDispTopLine = m_stlLine.FindIndex(nLine);
		}
	}
	Refresh ();
}

void CTermLog::VScroll(UINT nSBCode, UINT nPos)
{
	SCROLLINFO  si ;
	// Get all the vertical scroll bar information

	si.cbSize = sizeof (si) ;
	si.fMask  = SIF_ALL ;
	GetScrollInfo (SB_VERT,&si);

	// Save the position for comparison later on

	int iVertPos = si.nPos ;

	switch (nSBCode)
	{
	case SB_TOP:
		si.nPos = si.nMin ;
		break ;

	case SB_BOTTOM:
		si.nPos = si.nMax ;
		break ;

	case SB_LINEUP:
		si.nPos -= 1 ;
		break ;

	case SB_LINEDOWN:
		si.nPos += 1 ;
		break ;

	case SB_PAGEUP:
		si.nPos -= si.nPage ;
		break ;

	case SB_PAGEDOWN:
		si.nPos += si.nPage ;
		break ;

	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos ;
		break ;

	default:
		break ;
	}

	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	if (m_nTotalLines <= m_ptScreen.y) {
		si.nPos = 0;
		SetScrollInfo (SB_VERT, &si, FALSE);
		SetTopLine (0);
	} else {
		si.fMask = SIF_POS ;
		SetScrollInfo (SB_VERT, &si, FALSE);
		GetScrollInfo (SB_VERT, &si);
		SetTopLine (si.nPos);
	}

}
void CTermLog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	VScroll (nSBCode, nPos);
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CTermLog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	HScroll (nSBCode, nPos);
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTermLog::UpdateSelection (CPoint ptsCursor, bool bSelect)
{
	CharPositionFromPoint (ptsCursor, m_ptEndSelection);
	m_ptCaret = m_ptEndSelection;
}

BOOL CTermLog::TryConnection()
{
	BOOL  bValid;

	bValid = m_pDevice->IoIsValid();
	if (!bValid)  {
		CString  csOpen;
		if (m_pDevice->m_Type == CDeviceIo::EnumTypeSerialPort)
			csOpen.Format(_T("Device=%s;BaudRate=%d;Dtr=%d;Rts=%d"), m_TermCfg.ComPort, m_TermCfg.BaudRate, m_TermCfg.EnableDtr, m_TermCfg.EnableRts);
		else if (m_pDevice->m_Type == CDeviceIo::EnumTypeNamedPipe)
			csOpen.Format(_T("PipeName=%s"),m_TermCfg.PipeName);
		else if (m_pDevice->m_Type == CDeviceIo::EnumTypeTelnet)
			csOpen.Format(_T("ServerName=%s;Port=%d"),m_TermCfg.ServerIp, m_TermCfg.Port);
		else if (m_pDevice->m_Type == CDeviceIo::EnumTypeUsbBulk)
			csOpen.Format(_T("DeviceName=%s;Interface=%d;EpIn=%d;EpInSize=%d;EpOut=%d;EpOutSize=%d"), m_TermCfg.UsbDeviceName, \
			    m_TermCfg.Interface, m_TermCfg.EpIn, m_TermCfg.EpInSize, m_TermCfg.EpOut, m_TermCfg.EpOutSize);
		else
			ASSERT(0);
		m_pDevice->IoOpen (this, csOpen.GetBuffer(0));
		csOpen.ReleaseBuffer();
		bValid = m_pDevice->IoIsValid();
	}
	DWORD Data;
	if (bValid) {
		Data = m_pDevice->m_Type;
	} else {
		Data = 0;
	}
	AfxGetMainWnd()->SendMessage(WM_USER, 0,  Data);
	return bValid;
}

#define  COMMAND_LINE_DELAY   1

void CTermLog::OnTimer(UINT nTimerID)
{
	// TODO: Add your message handler code here and/or call default
	if (nTimerID == SEND_FILE_TIMER_TID) {
		ContinueSendFile();
		return;
	}

	if (nTimerID == CONNECT_CHECK_TIMER_TID && !m_bConfigInProgress) {
		static  int  Counter;
		if (Counter ++ == 10) {
			BOOL  bRes = TryConnection();
			if (m_FirstAttempt <= 1) {
			    if (!bRes && (m_FirstAttempt == 1)) {
					PostMessage (WM_USER, CODE_CONNECT_FAIL, 0);
				}
				m_FirstAttempt++;
			}
			Counter  = 0;
		}
		PostMessage (WM_USER, CODE_RECV_FROM_DEVICE, 0);
		if (m_IdleTimer++ == COMMAND_LINE_DELAY) {
			SendData();
			m_IdleTimer = 0;
		}
	} else if (m_bIsMouseRepActive && (nTimerID == MOUSEREP_TIMER_TID) ) {
		if(m_nMouseRepCounter > 0) {
			m_nMouseRepCounter--;
		} else {
			UpdateSelection (m_cMouseRepPoint, true);
			if (m_iMouseRepDelta < 0) {
				VScroll ((m_nMouseRepSpeed == 1) ? SB_PAGEUP : SB_LINEUP, m_iMouseRepDelta);
			} else {
				VScroll ((m_nMouseRepSpeed == 1) ? SB_PAGEDOWN : SB_LINEDOWN, m_iMouseRepDelta);
			}
			if (m_nMouseRepSpeed == 1)  // Page mode
				m_nMouseRepCounter = 15;
			else
				m_nMouseRepCounter = m_nMouseRepSpeed;
		}
	}
}

BOOL CTermLog::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	if (m_iDeltaPerLine == 0)
		return TRUE; //CWnd::OnMouseWheel(nFlags, zDelta, pt);

	m_iAccumDelta += zDelta; //(short) HIWORD (wParam) ;     // 120 or -120
	while (m_iAccumDelta >= m_iDeltaPerLine)
	{
		SendMessage (WM_VSCROLL, SB_LINEUP, 0) ;
		m_iAccumDelta -= m_iDeltaPerLine ;
	}

	while (m_iAccumDelta <= -m_iDeltaPerLine)
	{
		SendMessage (WM_VSCROLL, SB_LINEDOWN, 0) ;
		m_iAccumDelta += m_iDeltaPerLine ;
	}

	return TRUE; //CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


void CTermLog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnRButtonDown(nFlags, point);
}


void CTermLog::EraseChar (int StartPos, int EndPos)
{
	if (StartPos < 0)             StartPos = 0;
	if (StartPos >= m_ptScreen.x) StartPos = m_ptScreen.x -1;

	if (EndPos < 0)               EndPos   = 0;
	if (EndPos >= m_ptScreen.x)   EndPos   = m_ptScreen.x -1;

	LPLINEDATA  pLineData = &(m_stlLine.GetAt(m_posTermCurrLine));
	for (int x=StartPos; x<=EndPos; x++) {
		pLineData->pszLineText[x] = _T(' ');
		pLineData->pdbLineAttr[x] = m_dbCurrColor;
	}
	Invalidate ();
}

void CTermLog::ClearScreen(int StartLine, int EndLine, int Color)
{
	EraseLine (0, m_ptScreen.y, Color);
	GotoXy (1,1);
	m_Cursor.x  = 0;
	m_Cursor.y = 0;
}


void CTermLog::EraseLine (int StartLine, int EndLine, int Color)
{
	if (Color == -1)			Color = m_dbCurrColor;
	if (StartLine < 0)          StartLine = 0;
	if (StartLine >= m_ptScreen.y) StartLine = m_ptScreen.y-1;

	if (EndLine < 0)            EndLine   = 0;
	if (EndLine >= m_ptScreen.y)   EndLine   = m_ptScreen.y-1;

	POSITION    posLine = m_posTermTopLine;
	int		    nCnt	= StartLine;
	while (nCnt--) {
		m_stlLine.GetNext(posLine);
	}

	LPLINEDATA  pLineData;
	for (int y=StartLine; y<=EndLine; y++) {
		pLineData = &(m_stlLine.GetNext(posLine));
		for (int x=0; x<m_ptScreen.x; x++) {
			pLineData->pszLineText[x] = _T(' ');
			pLineData->pdbLineAttr[x] = Color;
		}
		//pLineData->nLineLen = m_ptScreen.x;
	}
	Invalidate ();
}

void CTermLog::SetAttribute (int *Param, int Number)
{
	for (int Index=0; Index< Number; Index++) {
		if         (Param[Index] == 0)  {
			m_HightLight   = 0;
			m_dbCurrColor  = m_dbDefColor;
		} else  if   (Param[Index] == 1)  {
			m_HightLight = 1;
		} else  if   (Param[Index] >= 40 && Param[Index] < 48)  {
			m_dbCurrColor = m_dbCurrColor & 0x0F | (((Param[Index]-40) << 4));
		} else  if   (Param[Index] >= 30 && Param[Index] < 38)  {
			m_dbCurrColor = m_dbCurrColor & 0xF0 | ( ((Param[Index]-30) & 0x0F));		
    } else  if   (Param[Index] >= 100 && Param[Index] < 108)  {
			m_dbCurrColor = m_dbCurrColor & 0x0F | (((Param[Index]-100) << 4) + 0x80);
		} else  if   (Param[Index] >= 90 && Param[Index] < 98)  {
			m_dbCurrColor = m_dbCurrColor & 0xF0 | ( ((Param[Index]-90) & 0x0F) + 0x08);
		}
	}

}

void CTermLog::SetCurrPos (int x, int y)
{
	if (x >= 0 && m_ptCurrent.x != x)  {
		m_ptCurrent.x = x;
	}

	if (y >= 0 && m_ptCurrent.y != y)  {
		if (y >= m_ptScreen.y) {
			ScrollUp(y - m_ptScreen.y + 1);
			m_ptCurrent.y = m_ptScreen.y - 1;
		} else {
			int nDelta = y - m_ptCurrent.y;
			m_ptCurrent.y = y;
			if (nDelta > 0) {
				while (nDelta--) m_stlLine.GetNext(m_posTermCurrLine);
			} else {
				while (nDelta++) m_stlLine.GetPrev(m_posTermCurrLine);
			}
		}
	}

	Invalidate ();
}

void CTermLog::GotoXy (int x, int y)
{
	x--;
	y--;

	if (y < 0)               y   = 0;
	if (y >= m_ptScreen.y)   y   = m_ptScreen.y-1;

	if (x < 0)               x   = 0;
	if (x >= m_ptScreen.x)   x   = m_ptScreen.x-1;

	m_bLineLastChar = FALSE;

	SetCurrPos (x, y);

}

int CTermLog::GetParamList (TCHAR *Input, int *Param, int BufferLen)
{
	int  Index;
	for (Index=0; *Input && (Index < BufferLen); Index++) {
		if (isdigit(*Input)) {
			Param[Index] = _ttoi(Input);
		} else {
		    if (*Input == ';') {
			  Input++;
		    }
		    Param[Index] = 1;
		}
		while (*Input != ';' && *Input) Input++;
		if (*Input == ';') Input++;
	}
	return Index;
}

int CTermLog::DoAnsiCtrl (TCHAR *AnsiBuf, int Len)
{
	TCHAR Handled;
	TCHAR CtrlChar;
	TCHAR *Ptr;
	int  Param[8];
	int  Index;

	Handled  = true;
	CtrlChar = AnsiBuf[Len-1];

	if (AnsiBuf[1]=='[') {
	  switch (CtrlChar) {

	  case 'K':
	  case 'J':
		  if (Len == 3) {
			  if (CtrlChar=='J') {
				  // Erases the screen from the current position down to the bottom of the screen.
				  EraseChar (m_ptCurrent.x, m_ptScreen.x);
				  if (m_ptCurrent.y != m_ptScreen.y - 1)
					  EraseLine (m_ptCurrent.y + 1, m_ptScreen.y);
			  } else {
				  // Erases from the current cursor position to the end of the current line.
				  //TRACE("Clear1 Y=%d X=%d\n", m_ptCurrent.y, m_ptCurrent.x);
				  EraseChar (m_ptCurrent.x, m_ptScreen.x);
			  }
		  } else if (Len == 4) {
			  if (AnsiBuf[2]=='0') {  // Same as[J
				  if (CtrlChar=='J') {
					EraseChar (m_ptCurrent.x, m_ptScreen.x);
					if (m_ptCurrent.y != m_ptScreen.y - 1)
						EraseLine (m_ptCurrent.y + 1, m_ptScreen.y);
				  } else { // [0K
					  //TRACE("Clear2 Y=%d X=%d\n", m_ptCurrent.y, m_ptCurrent.x);
					  EraseChar (m_ptCurrent.x, m_ptScreen.x);
				  }
			  } else if (AnsiBuf[2]=='1') {
				  if (CtrlChar=='J') {
					  // Erases the screen from the current line up to the top of the screen.
					  EraseLine (0, m_ptCurrent.y);
				  } else {
					  // Erases from the current cursor position to the start of the current line.
					  EraseChar (0, m_ptScreen.x);
				  }
			  } else if (AnsiBuf[2]=='2') {
				  if (CtrlChar=='J') {
					  // Erases the screen with the background colour and moves the cursor to home.
					  // EraseLine (0, m_ptScreen.y);
					  ClearScreen(0, m_ptCurrent.y);
				  } else {
					  // Erases the entire current line.
					  EraseChar (0, m_ptScreen.x);
				  }
			  } else {
				  Handled  = false;
			  }
		  } else {
			  Handled  = false;
		  }
		  break;


		  if (Len > 3) {
			  Ptr = &AnsiBuf[2];
			  Index = 0;
			  for (Index=0; Index<3; Index++) {
				  Param[Index] = _ttoi(Ptr);
				  while (*Ptr != ';' && *Ptr) Ptr++;
			  }
		  }
		  break;

	  // <ESC>[{COUNT} A|B|C|D
	  case 'A':
	  case 'B':
	  case 'C':
	  case 'D':
		  {
		    int nAdjust;
			if (isdigit(AnsiBuf[2])) {
				nAdjust = _ttoi(&AnsiBuf[2]);
			} else {
				nAdjust = 1;
			}
			if (CtrlChar == 'A')
				GotoXy (m_ptCurrent.x + 1, m_ptCurrent.y - nAdjust + 1);
			else if (CtrlChar == 'B')
				GotoXy (m_ptCurrent.x + 1, m_ptCurrent.y + nAdjust + 1);
			else if (CtrlChar == 'C')
				GotoXy (m_ptCurrent.x + 1 + nAdjust, m_ptCurrent.y + 1);
			else
				GotoXy (m_ptCurrent.x + 1 - nAdjust, m_ptCurrent.y + 1);
		  }
		  break;

	  case 's':
		  // Save current currsor pos
		  m_ptSavedCursor = m_ptCurrent;
		  break;

	  case 'u':
		  // Restore current currsor pos
		  GotoXy (m_ptSavedCursor.x + 1, m_ptSavedCursor.y + 1);
		  break;

	  case 'r':
		  if (Len == 3) {
			  m_ptScrollRaw.x = 1;
			  m_ptScrollRaw.y = m_ptScreen.y + 1;
		  } else {
			  Ptr = &AnsiBuf[2];
			  GetParamList(Ptr, Param, 8);
			  m_ptScrollRaw.x = Param[0];
			  m_ptScrollRaw.y = Param[1];
		  }
		  break;

	  case 'f':
	  case 'H':
	  case 'm':
		  if (Len == 3) {
			  // The cursor will move to the home position
			  if (CtrlChar == 'H'|| CtrlChar == 'f')
				  GotoXy (1, 1);
			  else {  // [m  ==>  [0m
				  Index    = 1;
				  Param[0] = 0;
				  SetAttribute (Param, Index);
			  }
		  } else {
			  // Sets the cursor position where subsequent text will begin
			  Ptr = &AnsiBuf[2];
			  Index = GetParamList(Ptr, Param, 8);
			  if (CtrlChar == 'H'|| CtrlChar == 'f') {
				  GotoXy (Param[1], Param[0]);
			  } else {
				  SetAttribute (Param, Index);
			  }
		  }
		  break;

    case 'h':
    case 'l':
      // last 
      if ((AnsiBuf[2] == '?') && (Len > 4)) {
        int Code = _ttoi (&AnsiBuf[3]);
        if (Code == 25) {
          if (CtrlChar == 'h') {
            // Hide Cursor
            HideCaret();
          }
          else {
            // Show Cursor
            ShowCaret();
          }
        }
      }        
      break;

	  default:
		  Handled  = false;
		  break;
	  }
	} else {
      switch (CtrlChar) {
	  case '7':
		  // Save current currsor pos
		  m_ptSavedCursor = m_ptCurrent;
		  break;

	  case '8':
		  // Restore current currsor pos
		  GotoXy (m_ptSavedCursor.x + 1, m_ptSavedCursor.y + 1);
		  break;

      case 'D':
		  // Scroll display down one line.
		  m_ptSavedCursor = m_ptCurrent;
		  break;

	  case 'M':
		  // Scroll display up one line
		  GotoXy (m_ptSavedCursor.x + 1, m_ptSavedCursor.y + 1);
		  break;

	  default:
		Handled  = false;
		break;
	  }
	}

	if (!Handled) {
		// Print the un-handled ANSI string
		TRACE ("Unsupported: %S\n", AnsiBuf);
	}

	return Handled;
}

void CTermLog::SetLineAttr(int nLineDirection, DWORD dwAttrToAdd, DWORD dwAttrToRemove)
{
	POSITION  pos = m_posTermCurrLine;
	if (nLineDirection < 0) {  // last line
		m_stlLine.GetPrev(pos);
	} else if (nLineDirection > 0) { // next line
		m_stlLine.GetNext(pos);
	}
	if (pos) {
		LPLINEDATA  pLineData   = &(m_stlLine.GetAt(pos));
		pLineData->dwFlags |=  dwAttrToAdd;
		pLineData->dwFlags &= ~dwAttrToRemove;
	}
}

void CTermLog::MoveToNextPos()
{
	SetCurrPos (m_ptCurrent.x + 1, -1);

	if (m_ptCurrent.x >=  m_ptScreen.x) {
		m_bLineLastChar = TRUE;
	} else {
		m_bLineLastChar = FALSE;
	}
}

void  CTermLog::OutputChar (TCHAR Char)
{
	if (m_ptCurrent.x > m_ptScreen.x)
		return;

	if (m_bLineLastChar) {
		// Soft line CR
		SetCurrPos (0, m_ptCurrent.y + 1);
		m_bLineLastChar = FALSE;
		SetLineAttr(-1, LINE_ATTR_SOFT_CR);
		EraseChar(0, m_ptScreen.x);
	}
	LPLINEDATA  pLineData = &(m_stlLine.GetAt(m_posTermCurrLine));

	pLineData->pszLineText[m_ptCurrent.x] = Char;
	pLineData->pdbLineAttr[m_ptCurrent.x] = m_dbCurrColor;
	if (pLineData->nLineLen <=  m_ptCurrent.x) {
		pLineData->nLineLen = m_ptCurrent.x + 1;
	}

	MoveToNextPos();

	Invalidate ();
}

bool CTermLog::CharHandler (TCHAR Char)
{
	static int	 State;
	static int	 Index;
	static TCHAR LastChar;
	static TCHAR Buffer[64];

	switch (State) {
	  case 0:
			if (Char == '^') {
				State = 3;
			} else if (Char == 0x1B) {
			  Index = 0;
			  Buffer[0] = 0x1B;
			  State = 1;
		  } else {
			  bool   Show;

			  Show = false;
			  switch (Char) {

			  case 0x0A:
				  SetCurrPos (0,  m_ptCurrent.y + 1);
				  if (LastChar == 0x0A) {
					  LogText (0x0D);
					}
					LogText (0x0A);
				  break;

			  case 0x0D:
				  // EraseChar (m_ptCurrent.x, m_ptScreen.x);
				  GotoXy (0 + 1, m_ptCurrent.y + 1);
				  if (LastChar == 0x0D) {
					  LogText (0x0A);
				  }
				  break;
			  
			  case 0x03:
				  break;

				case 0x07:					
					// SetCurrPos(m_ptCurrent.x + 1, m_ptCurrent.y);
					break;

			  case 0x08: // Backspace
				  if (m_ptCurrent.x >= 1) {					  					  
						SetCurrPos (m_ptCurrent.x-1,  m_ptCurrent.y);
						//EraseChar(m_ptCurrent.x, m_ptCurrent.x + 1);
				  }
				  break;

			  case 0x09: //TAB
				  {
					int SpaceNum = ((m_ptCurrent.x + m_TabSize) / m_TabSize) * m_TabSize - m_ptCurrent.x;
					while (SpaceNum--)
					OutputChar (' ');
				  }
				  LogText (0x09);
				  break;


			  default:
				  Show = true;
				  break;
			  }

			  LastChar = Char;
			  if (Show) {
				  OutputChar (Char);
				  LogText    (Char);
			  }
		  }
		  break;

	  case 1:
		  if (Char == '[') {
			  Buffer[1] = '[';
			  Index = 2;
			  State = 2;
		  } else {
			  State = 0;
			  if (isalpha(Char)) {
				 Buffer[1] = Char;
				 Buffer[2] = 0;
			     Index = 2;
				 DoAnsiCtrl (Buffer, Index);
			  } else {
				 OutputChar (0x1B);
			     CharHandler (Char);
			  }
		  }
		  break;

	  case 2:
		  if (Index < 64) {
			  Buffer[Index++] = Char;
			  Buffer[Index] = 0;
		  }
		  if (isalpha(Char)) {
			  State = 0;
			  DoAnsiCtrl (Buffer, Index);
		  }
		  break;

		case 3: 
			if (Char == 'H') {
				if (m_ptCurrent.x >= 1) {
					EraseChar(m_ptCurrent.x - 1, m_ptCurrent.x);
					SetCurrPos(m_ptCurrent.x - 1, m_ptCurrent.y);
				}
			}
			else {
				;
			}
			State = 0;
			break;
	}
	return true;
}

/*
void CTermLog::AdjustDisplayPos ()
{
	int nTopLine;

	if (m_nTotalLines  > m_ptScreen.y) {
		m_posTermTopLine = m_stlLine.GetTailPosition();
		nTopLine = m_nTotalLines - m_ptScreen.y;
		int nCnt = m_ptScreen.y;
		while (nCnt--) {
			m_stlLine.GetPrev(m_posTermTopLine);
		}
	} else {
		nTopLine = 0;
		m_posTermTopLine = m_stlLine.GetHeadPosition();
	}
	SetTopLine (nTopLine);
	//Invalidate ();
}
*/

void CTermLog::ScrollUp (int Lines)
{
	if (Lines<=0)
		return;

	if (Lines > m_ptScreen.y)  {
		Lines = m_ptScreen.y;
	}

	while (Lines--) {
		AddBlankLine ();
		m_stlLine.GetNext(m_posTermCurrLine);
		m_stlLine.GetNext(m_posTermTopLine);
	}

	Invalidate ();
}

void CTermLog::SetTerminalType (int Type)
{
	m_AnsiMode = Type;
	AfxGetMainWnd()->SendMessage(WM_USER, 2, Type);
}

void CTermLog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	BYTE  code[3];
  UINT  Ch;
	int   Num;
	bool  NotifyParent;

	NotifyParent = (nFlags == 0xFF) ? false : true;
	Num = 0;

  if (nFlags == 0xFF) {
    Ch = tolower(nChar);
    if ((Ch >= 'a') && (Ch <= 'z')) { // Ctrl+X / Q
      Num = 1;
      code[0] = 0x01 + (Ch - 'a');
    }
  } else if (nChar == 0x91) { // Lock, change VT mode
		if (m_AnsiMode == VT_UTF8) {
	      SetTerminalType (VT_ANSI);
		} else if (m_AnsiMode == VT_ANSI) {;
		  SetTerminalType (VT_UTF8);
		}
	}
	else if ((nChar == 0x13) || (nChar == 0x03)) { // Break
		Num = 1;
		code[0] = 0x03;  // Ctrl+C
	} else if (nChar>36 && nChar<41 ) {//38 40 37 39
		/*
		| UP      | 0x01 | ESC [ A   | ESC [ A  | ESC [ A  |
		| DOWN    | 0x02 | ESC [ B   | ESC [ B  | ESC [ B  |
		| RIGHT   | 0x03 | ESC [ C   | ESC [ C  | ESC [ C  |
		| LEFT    | 0x04 | ESC [ D   | ESC [ D  | ESC [ D  |
		*/
		code[0]=0x1b;
 	  code[1]=0x5b;
		Num = 3;
		switch(nChar)
		{
		case 38:
			code[2]=0x41;
			break;
		case 40:
			code[2]=0x42;
			break;
		case 37:
			code[2]=0x44;
			break;
		case 39:
			code[2]=0x43;
			break;
		default:
			Num = 0;
			break;
		}

	} else if (nChar >= 112 && nChar <= 112+11) {
		// F1-F10
		/*
		| F1      | 0x0B | ESC [ M   | ESC 1    | ESC O P  |
		| F2      | 0x0C | ESC [ N   | ESC 2    | ESC O Q  |
		| F3      | 0x0D | ESC [ O   | ESC 3    | ESC O w  |
		| F4      | 0x0E | ESC [ P   | ESC 4    | ESC O x  |
		| F5      | 0x0F | ESC [ Q   | ESC 5    | ESC O t  |
		| F6      | 0x10 | ESC [ R   | ESC 6    | ESC O u  |
		| F7      | 0x11 | ESC [ S   | ESC 7    | ESC O q  |
		| F8      | 0x12 | ESC [ T   | ESC 8    | ESC O r  |
		| F9      | 0x13 | ESC [ U   | ESC 9    | ESC O p  |
		| F10     | 0x14 | ESC [ V   | ESC 0    | ESC O M  |
		*/
		code[0]=0x1b;
		if (m_AnsiMode == VT_UTF8) {
			code[1]='1'+ (nChar-112);
			if (nChar >= 112 + 9) {
				int nIdx = nChar - (112 + 9);
				switch (nIdx) {
				case 0: // F10
					code[1] = 'O';
					// F10
					NotifyParent = false;
					break;
				case 1: // F11
					code[1] = '!';
					break;
				default:// F12
					code[1] = '@';
					break;
				}
			}
			Num = 2;
		} else if (m_AnsiMode == VT_ANSI) {
			code[1]='[';
			code[2]='M' + (nChar-112);
			Num = 3;
		}
	} else  {

		/*
		| HOME    | 0x05 | ESC [ H   | ESC h    | ESC [ H  | 36
		| END     | 0x06 | ESC [ F   | ESC k    | ESC [ K  | 35
		| INSERT  | 0x07 | ESC [ @   | ESC +    | ESC [ @  | 45
		| DELETE  | 0x08 | ESC [ X   | ESC -    | ESC [ P  | 46
		| PG UP   | 0x09 | ESC [ I   | ESC ?    | ESC [ V  | 33
		| PG DOWN | 0x0A | ESC [ G   | ESC /    | ESC [ U  | 34
		*/

		BYTE Value = (BYTE)(nChar);
		if (m_AnsiMode == VT_UTF8) {
			Num = 2;
			switch (Value) {
				case 36:
				  code[1]='h';
				  break;
				case 35:
				  code[1]='k';
				  break;
				case 45:
				  code[1]='+';
				  break;
				case 46:
				  code[1]='-';
				  break;
				case 33:
				  code[1]='?';
				  break;
				case 34:
				  code[1]='/';
				  break;
				default:
					Num = 0;
			}
		} else  if (m_AnsiMode == VT_ANSI) {
            Num = 3;
			switch (Value) {
				case 36:
				  code[1]='H';
				  break;
				case 35:
				  code[1]='F';
				  break;
				case 45:
				  code[1]='@';
				  break;
				case 46:
				  code[1]='X';
				  break;
				case 33:
				  code[1]='I';
				  break;
				case 34:
				  code[1]='G';
				  break;
				default:
					Num = 0;
			}
		}
		code[0]=0x1b;
	}

	if (Num) {
		WriteData (code, Num);
	}

	if (NotifyParent)  CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTermLog::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	int  nLen = 1;
	if (0 && (nChar == 0x0D)) {
		if (IsSelected()) {
			OnCopySelected ();
			DeSelect ();
			nLen = 0;
		}
	}
	if (nLen)
		WriteData ((BYTE *)&nChar, nLen);
  TRACE(_T("%x\n"), nChar);
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CTermLog::OnClose()
{
	m_bClose = TRUE;

	// TODO: Add your message handler code here and/or call default
    KillTimer(CONNECT_CHECK_TIMER_TID);

	// Signal Listening thread to exit
	SetEvent (m_hEventServerQuit);

	// Close IO device
	m_pDevice->IoClose ();

	CWnd::OnClose();
}

UINT CTermLog::WriteNetDataToDevice()
{
	BYTE       rxBuffer[4096];
	int		   nActualLen;
	nActualLen = m_TelnetQueueRx.Get (rxBuffer, sizeof(rxBuffer));
	if (nActualLen) {
		m_pDevice->IoWrite (rxBuffer, nActualLen);
	}
	return nActualLen;
}

UINT CTermLog::SendData()
{
	if (m_strPendingToSend.IsEmpty()) return 0;

	USES_CONVERSION;
	int  nPos = m_strPendingToSend.Find(_T("\n"));
	UINT nLen;
	if (nPos < 0) {
		nLen =  m_strPendingToSend.GetLength();
		WriteData((LPBYTE)W2A(m_strPendingToSend), nLen);
		m_strPendingToSend.Empty();
	} else {
		nLen = nPos + 1;
		WriteData((LPBYTE)W2A(m_strPendingToSend.Left(nPos + 1)), nLen);
		m_strPendingToSend = m_strPendingToSend.Mid(nPos+1);
	}
	return nLen;
}

UINT CTermLog::WriteData(BYTE *pData, UINT  nLen)
{
	if (nLen) {
		m_pDevice->IoWrite (pData, nLen);
	}
	return nLen;
}

UINT CTermLog::ReadData(BYTE *pData, UINT  nLen)
{
	UINT nActualLen;

	nActualLen = m_pDevice->IoRead (pData, nLen);
	if (nActualLen) {
		LogBinary(pData, nActualLen);
		m_TelnetQueueTx.Put (pData, nActualLen);
		m_IdleTimer = 0;
	}

	return nActualLen;
}


void CTermLog::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
	//HideCaret();
}


void CTermLog::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
	ShowCaret();
}

void CTermLog::SetCaretPosition()
{
	CPoint ptCaret (m_ptCaret);

	CPoint pt;

	pt.x = m_ptMargin.x + (m_ptCurrent.x) * m_ptChar.x;
	pt.y = m_ptMargin.y + (m_ptCurrent.y) * m_ptChar.y;

	SetCaretPos (pt);
}

BOOL CTermLog::IsSelected ()
{
	if (m_ptEndSelection == m_ptStartSelection)
		return FALSE;
	else
		return TRUE;
}

int CTermLog::GetTextBuffer(TCHAR *pBuffer)
{
	LPLINEDATA  pLineData;
	int			nLen = 0;
	TCHAR       *pStart = pBuffer;

	for (POSITION nPos = m_stlLine.GetHeadPosition(); nPos;)
	{
		pLineData = &(m_stlLine.GetNext(nPos));
		if (pStart)
			memcpy (pBuffer, pLineData->pszLineText, pLineData->nLineLen * sizeof(TCHAR));
		pBuffer += pLineData->nLineLen;
		if (!(pLineData->dwFlags & LINE_ATTR_SOFT_CR) ) {
			if (pStart) {
				*pBuffer++ = _T('\r');
				*pBuffer++ = _T('\n');
			} else {
				pBuffer+=2;
			}
		}
	}
	if (pStart)
		*pBuffer = 0;
	return (int)((pBuffer - pStart + 1) * sizeof(TCHAR));
}


void CTermLog::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here
	CMenuEx menu(AfxGetMainWnd());
	menu.LoadMenu(IDR_POPUP_MENU);

	CMenu* pMenu = menu.GetSubMenu(0);
	if (IsSelected())
		pMenu->EnableMenuItem (ID__COPY_SELECTED, MF_BYCOMMAND | MF_ENABLED);
	else
		pMenu->EnableMenuItem (ID__COPY_SELECTED, MF_BYCOMMAND | MF_GRAYED);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON , point.x, point.y, this);

	return;
}


BOOL CTermLog::GetSelected(TCHAR *pBuffer)
{
	TCHAR       *pStart = pBuffer;
	int			 nLen;

	if (!IsSelected ()) {
		return FALSE;
	}

	CPoint  ptStart (m_ptStartSelection);
	CPoint  ptEnd   (m_ptEndSelection);

	if (ptStart.y > ptEnd.y) {
		ptEnd   = m_ptStartSelection;
		ptStart = m_ptEndSelection;
	} else if  (ptStart.y == ptEnd.y) {
		if (ptStart.x > ptEnd.x) {
			ptEnd.x    = m_ptStartSelection.x;
			ptStart.x  = m_ptEndSelection.x;
		}
	}

	POSITION    nPos      = m_stlLine.FindIndex (ptStart.y);
	LPLINEDATA  pLineData;
	for(int nLines = ptStart.y; nPos && (nLines <= ptEnd.y); nLines++) {
		pLineData = &(m_stlLine.GetNext(nPos));
		if (ptEnd.y == ptStart.y) {
			if (pStart)
				memcpy (pBuffer, pLineData->pszLineText + ptStart.x, (ptEnd.x - ptStart.x) * sizeof(TCHAR));
			pBuffer += (ptEnd.x - ptStart.x);
		} else if (nLines == ptStart.y) {
			nLen = (pLineData->nLineLen - ptStart.x);
			if (pStart)
				memcpy (pBuffer, pLineData->pszLineText + ptStart.x,  nLen * sizeof(TCHAR));
			pBuffer += nLen;
			if (!(pLineData->dwFlags & LINE_ATTR_SOFT_CR) ) {
				if (pStart) {
					*pBuffer++ = _T('\r');
					*pBuffer++ = _T('\n');
				} else {
					pBuffer+=2;
				}
			}
		} else if (nLines == ptEnd.y) {
			nLen = ptEnd.x;
			if (pStart)
				memcpy (pBuffer, pLineData->pszLineText, nLen * sizeof(TCHAR));
			pBuffer += nLen;
		} else {
			if (pStart)
				memcpy (pBuffer, pLineData->pszLineText, pLineData->nLineLen * sizeof(TCHAR));
			pBuffer += pLineData->nLineLen;
			if (!(pLineData->dwFlags & LINE_ATTR_SOFT_CR) ) {
				if (pStart) {
					*pBuffer++ = _T('\r');
					*pBuffer++ = _T('\n');
				} else {
					pBuffer+=2;
				}
			}
		}
	}
	if (pStart)
		*pBuffer = 0;
	return (int)((pBuffer - pStart + 1) * sizeof(TCHAR));
}

void CTermLog::CopyToClipboard(BOOL bSelected)
{
	CString  csSelection;
	LPTSTR   lptstrCopy;
	HGLOBAL  hglbCopy;

	if (!m_nTotalLines) {
		return;
	}

	OpenClipboard();
	EmptyClipboard();

	int nLen;

	if (bSelected)
		nLen = GetSelected(NULL);
	else
		nLen = GetTextBuffer(NULL);

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, nLen);
	if(hglbCopy == NULL)
		CloseClipboard();

	lptstrCopy = (TCHAR*)GlobalLock(hglbCopy);

	if (bSelected)
		GetSelected(lptstrCopy);
	else
		GetTextBuffer(lptstrCopy);


	GlobalUnlock(hglbCopy);
	SetClipboardData(CF_UNICODETEXT, hglbCopy);

	CloseClipboard();
}

void CTermLog::OnCopyAll()
{
	// TODO: Add your command handler code here
	CopyToClipboard (FALSE);
}


void CTermLog::OnCopySelected()
{
	CopyToClipboard (TRUE);
}

void CTermLog::OnClearScreen()
{
	// TODO: Add your command handler code here
	ClearScreen  (0, m_ptScreen.y, m_dbDefColor);
}

void CTermLog::OnClearHistory()
{
	// TODO: Add your command handler code here
	ClearBuffer ();
	IntializeScreen ();
}

void CTermLog::UpdateParam()
{
	TCHAR  Buffer[1024];
	int    Idx;

	//m_Ini.SetPathName();
	m_Ini.GetString (_T("ComPort"),_T("PortName"), Buffer, sizeof(Buffer), _T(""));
	for (Idx = 0; Idx < (int)_tcslen(Buffer); Idx++) {
		if (Buffer[Idx] == _T(' ')) {
			Buffer[Idx] = 0;
			break;
		}
	}
	m_TermCfg.ComPort  = Buffer;

	m_Ini.GetString (_T("ComPort"),_T("BaudRate"), Buffer, sizeof(Buffer), _T(""));
	m_TermCfg.BaudRate = _ttoi(Buffer);

	m_TermCfg.EnableDtr = m_Ini.GetInt(_T("ComPort"), _T("EnableDtr"), 0);
	m_TermCfg.EnableRts = m_Ini.GetInt(_T("ComPort"), _T("EnableRts"), 0);

	int nValue;
	nValue = m_Ini.GetInt (_T("ComPort"),_T("Enable") , 0);
	m_TermCfg.ComEnable = nValue;

	nValue = m_Ini.GetInt (_T("SoftSdv"),_T("Enable") , 0);
	m_TermCfg.PipeEnable = nValue;

	nValue = m_Ini.GetInt (_T("UsbBulk"),_T("Enable") , 0);
	m_TermCfg.UsbEnable = nValue;

	nValue = m_Ini.GetInt (_T("Telnet"),_T("Enable") , 0);
	m_TermCfg.TelnetEnable = nValue;

	nValue = m_Ini.GetInt (_T("Server"),_T("Enable") , 0);
	m_TermCfg.ServerEnable = nValue;

	m_Ini.GetString (_T("UsbBulk"),_T("DeviceName"), Buffer, sizeof(Buffer), _T(""));
	m_TermCfg.UsbDeviceName  = Buffer;

	m_TermCfg.Interface = m_Ini.GetInt (_T("UsbBulk"),_T("Interface"), 0);

	int pos;
	m_Ini.GetString (_T("UsbBulk"),_T("EpIn"), Buffer, sizeof(Buffer), _T(""));
	m_TermCfg.EpIn     = xtoi(Buffer, &pos);
	if ((int)_tcslen(Buffer) > pos+1)
	  m_TermCfg.EpInSize = xtoi(Buffer+pos+1, NULL);
	else
	  m_TermCfg.EpInSize = 0;

	m_Ini.GetString (_T("UsbBulk"),_T("EpOut"), Buffer, sizeof(Buffer), _T(""));
	m_TermCfg.EpOut     = xtoi(Buffer, &pos);
	if ((int)_tcslen(Buffer) > pos+1)
		m_TermCfg.EpOutSize = xtoi(Buffer+pos+1, NULL);
	else
		m_TermCfg.EpOutSize = 0;

	m_Ini.GetString (_T("Telnet"), _T("ServerName"), Buffer, sizeof(Buffer), _T(""));
	m_TermCfg.ServerIp  = Buffer;

	nValue = m_Ini.GetInt (_T("Telnet"), _T("Port"),  0);
	m_TermCfg.Port = nValue;

	nValue = m_Ini.GetInt (_T("Server"), _T("Port"),  0);
	m_TermCfg.ServerPort = nValue;

	m_Ini.GetString (_T("SoftSdv"), _T("PipeName"), Buffer, sizeof(Buffer), _T(""));
	m_TermCfg.PipeName = Buffer;

	nValue = m_Ini.GetInt (_T("Console"), _T("TabSize"),  0);
	m_TermCfg.TabSize = nValue;

	nValue = m_Ini.GetInt (_T("Console"), _T("Row"),  0);
	m_TermCfg.TermRow = nValue;

	nValue = m_Ini.GetInt (_T("Console"), _T("Col"), 0);
	m_TermCfg.TermCol = nValue;

	nValue = m_Ini.GetInt (_T("Logging"),_T("Format"), 0);
	m_TermCfg.LogFormat = nValue;

	nValue = m_Ini.GetInt (_T("Terminal"),_T("Type"), 0);
	m_TermCfg.TermType = nValue;
	SetTerminalType (m_TermCfg.TermType);

	if (m_TermCfg.TelnetEnable) {
		m_pDevice      = &m_Telnet;
	} else if (m_TermCfg.PipeEnable) {
		m_pDevice      = &m_Pipe;
	} else if (m_TermCfg.ComEnable) {
		m_pDevice      = &m_SerialPort;
	} else if (m_TermCfg.UsbEnable) {
		m_pDevice      = &m_UsbBulk;
	}

	m_FirstAttempt = 0;
}

bool CTermLog::LogBinary (BYTE *pBuffer, int nLen)
{
	if (m_hBinFile) {
	  fwrite(pBuffer, 1, nLen, m_hBinFile);
	}
	return true;
}

bool CTermLog::LogText (TCHAR Char)
{
	if (m_hTxtFile) {
	  putc(Char, m_hTxtFile);
	}
	return true;
}

void CTermLog::ConfigInProgress(BOOL bFlag)
{
	TERMCFG     OldCfg = m_TermCfg;
	CDeviceIo   *pOld  = m_pDevice;

	if (bFlag) {
		m_bConfigInProgress = TRUE;
		m_pDevice->IoClose();
	} else {
		UpdateParam ();

		// Server changed
		BOOL bServerChanged = FALSE;
		if ((OldCfg.ServerIp != m_TermCfg.ServerIp) ||
			(OldCfg.ServerPort != m_TermCfg.ServerPort)) {
			bServerChanged = TRUE;
		}
		if ((OldCfg.ServerEnable != m_TermCfg.ServerEnable)) {
			if (m_TermCfg.ServerEnable) {
				// Need to start service
				StartTelnetListen ();
			} else {
				// Need to stop service
				SetEvent (m_hEventServerQuit);
			}
		} else {
			if (bServerChanged && m_TermCfg.ServerEnable) {
				SetEvent (m_hEventServerQuit);
			}
		}

		m_bConfigInProgress = FALSE;
		OnTimer(CONNECT_CHECK_TIMER_TID);

		// Device changed
		if ((pOld != m_pDevice) || (m_pDevice == &m_Telnet)) {
			OnClearScreen ();
		}
	}
}

void CTermLog::LogData(int nType, BOOL bEnable)
{
	if (nType == FILE_LOG_TYPE_TEXT) {
		if (bEnable) {
			if (!m_hTxtFile) {
				m_hTxtFile = fopen ("Debug.log", "w");
			}
		} else {
			if (m_hTxtFile) {
				fclose(m_hTxtFile);
				m_hTxtFile = NULL;
			}
		}
	} else if (nType == FILE_LOG_TYPE_BINARY) {
		if (bEnable) {
			if (!m_hBinFile) {
				m_hBinFile = fopen ("Debug.bin", "wb");
			}
		} else {
			if (m_hBinFile) {
				fclose(m_hBinFile);
				m_hBinFile = NULL;
			}
		}
	}

}

void CTermLog::OnSavePaste()
{

}

void CTermLog::OnPaste()
{
	USES_CONVERSION;
    LPSTR szClipboardText;

    if (::OpenClipboard(NULL))  {
        if (::IsClipboardFormatAvailable(CF_TEXT) ||
            ::IsClipboardFormatAvailable(CF_UNICODETEXT))  {
            // First try to ge UNICODE text.
            BOOL   bUnicode   = TRUE;
            HANDLE hClipboard = ::GetClipboardData(CF_UNICODETEXT);

            if (NULL == hClipboard) {
                // If UNICODE text was not available try to get ANSI text.
                bUnicode   = FALSE;
                hClipboard = ::GetClipboardData(CF_TEXT);
            }

            if (hClipboard != NULL) {
                LPCSTR szClipboardData = (LPCSTR)::GlobalLock(hClipboard);
                if (szClipboardData != NULL) {
                    if (bUnicode) {
                        szClipboardText = W2A((LPCWSTR)szClipboardData);
                    } else {
                        szClipboardText = (LPSTR)szClipboardData;
                    }
					//WriteData((BYTE *)szClipboardText, (int)strlen(szClipboardText));
					m_strPendingToSend = m_strPendingToSend + A2W(szClipboardText);
                    ::GlobalUnlock(hClipboard);
                }
            }
            hClipboard = NULL;
			SetFocus();
        }
        BOOL bRes = ::CloseClipboard();
    }
	return;
}

int CTermLog::ContinueSendFile()
{
	int    nPos;
	int	   nLen;

	if (m_strFileStringPendingToSend.IsEmpty()) {
		// Stop timer
		KillTimer(SEND_FILE_TIMER_TID);
		return 0;
	}

	if ((nPos = m_strFileStringPendingToSend.Find("\n")) >= 0 ) {
		nLen = nPos + 1;
		WriteData((LPBYTE)(LPCSTR)m_strFileStringPendingToSend.Left(nPos + 1), nLen);
		m_strFileStringPendingToSend = m_strFileStringPendingToSend.Mid(nPos+1);
	} else {
		nLen =  m_strFileStringPendingToSend.GetLength();
		if (nLen > 0) {
			WriteData((LPBYTE)(LPCSTR)(m_strFileStringPendingToSend), nLen);
		}
		m_strFileStringPendingToSend.Empty();
	}
	return 1;
}

int CTermLog::SendFile(LPCTSTR FilePath, int LineInterval, int Protocol)
{
	CStringA strPendingToSend;
	CFile   file;

	KillTimer (SEND_FILE_TIMER_TID);
	if (file.Open(FilePath, CFile::modeRead)) {
	  int len = (int)file.GetLength();
	  if (len > 0x10000) {
		file.Close();
		AfxMessageBox(_T("The file is too big to send!"));
		return -1;
	  } else {
		file.Read(strPendingToSend.GetBuffer(len), len);
		strPendingToSend.ReleaseBuffer();
	  }
	  file.Close();
	} else{
	  AfxMessageBox(_T("Could not open file!"));
	  return -2;
	}

	m_strFileStringPendingToSend = strPendingToSend;
	ContinueSendFile ();
	SetTimer (SEND_FILE_TIMER_TID, LineInterval, NULL);

	return 0;
}

LRESULT CTermLog::OnUserMessage(WPARAM wParam, LPARAM lParam)
{
	BYTE       rxBuffer[1024];
	int		   nLen = sizeof(rxBuffer);
	DWORD	   dwCode = (DWORD)wParam;
	DWORD	   dwData = (DWORD)lParam;

	if (m_bClose)
		return 0;

	switch (dwCode)
	{
	case CODE_CONNECT_FAIL:
		if (m_pDevice) {
			CString Msg;
			Msg.Format ( _T("Could not connect device %s!"), m_pDevice->IoGetName());
			// AfxMessageBox (Msg);
		}
		break;

	case CODE_RECV_FROM_DEVICE:
		if (m_csSendFile.IsEmpty()) {
			while (nLen == sizeof(rxBuffer)) {
				nLen = ReadData(rxBuffer, nLen);
				for (int nIdx = 0; nIdx < nLen; nIdx++) {
					CharHandler (rxBuffer[nIdx]);
				}
			}
		}
		break;

	case CODE_RECV_FROM_NETWORK:
		// Network received data, need to forward to device
		WriteNetDataToDevice();
		break;

	case CODE_THREAD_EXIT:
		if (dwData <= THREAD_ID_TELNET) {
			m_pDevice->IoClose();
		} else if (dwData == THREAD_ID_CONNECT) {
			if (m_TermCfg.ServerEnable) {
				SERVER_THREAD_PARAM *pParam = new SERVER_THREAD_PARAM;
				pParam->hWnd = GetSafeHwnd();
				pParam->pQueueRx   = &m_TelnetQueueRx;
				pParam->pQueueTx   = &m_TelnetQueueTx;
				pParam->hEventQuit = m_hEventServerQuit;
				pParam->iPort      = m_TermCfg.ServerPort;
				ResetEvent (m_hEventServerQuit);
				m_pThread = AfxBeginThread(ListenThread, pParam);
				if (!m_pThread) {
					delete pParam;
				}
				AfxGetMainWnd()->SendMessage(WM_USER, 1,  1);
			} else {
				AfxGetMainWnd()->SendMessage(WM_USER, 1,  0);
			}
		} else if (dwData == THREAD_ID_LISTEN) {
			m_pThread		= NULL;
			AfxGetMainWnd()->SendMessage(WM_USER, 1,  2);
		}
		break;

	default:
		ASSERT(0);
		break;
	}

    return 0;
}

BOOL CTermLog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL  bHandled = FALSE;

	if (pMsg->message == WM_SYSKEYDOWN && (pMsg->wParam == VK_F10) &&  GetAsyncKeyState(VK_F10) < 0) {
       // F10 handler
	   OnKeyDown (121, 1, 0);
	   bHandled = TRUE;
	} else if (pMsg->message == WM_KEYDOWN && GetAsyncKeyState(VK_CONTROL) < 0) {
		switch(pMsg-> wParam) {
    case   'x':
    case   'X':
    case   'c':
    case   'C':
    case   'v':
    case   'V':
    case   'z':
    case   'Z':
      OnKeyDown(pMsg->wParam, 1, 0xFF);
      bHandled = TRUE;
      break;
		case   VK_PRIOR   :
			VScroll(SB_PAGEUP, 0);
			bHandled = TRUE;
			break;
		case   VK_NEXT   :
			VScroll(SB_PAGEDOWN, 0);
			bHandled = TRUE;
			break;
		case   VK_HOME   :
			VScroll(SB_TOP, 0);
			bHandled = TRUE;
			break;
		case   VK_END   :
			VScroll(SB_BOTTOM, 0);
			bHandled = TRUE;
			break;
		case   VK_UP   :
			VScroll(SB_LINEUP, 0);
			bHandled = TRUE;
			break;
		case   VK_DOWN   :
			VScroll(SB_LINEDOWN, 0);
			bHandled = TRUE;
			break;
		default:
			break;
		}
	}

	if (bHandled)
		return TRUE;
	else
		return CWnd::PreTranslateMessage(pMsg);
}