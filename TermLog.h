#pragma once

#include   "Serial.h"
#include   "Pipe.h"
#include   "Telnet.h"
#include   "UsbBulk.h"
#include   "IniFile.h"

#define	   CONNECT_CHECK_TIMER_TID		0x100
#define	   SEND_FILE_TIMER_TID   		0x101
#define	   MOUSEREP_TIMER_TID			0x500
#define	   MOUSEREP_TIMER_ELAPSE	 	0x5

#define	   FILE_LOG_TYPE_TEXT			0
#define	   FILE_LOG_TYPE_BINARY 		1

#define    VT_ANSI   0
#define    VT_UTF8   1


#define	   LINE_ATTR_SOFT_CR            (1<<0)

class CTermLog : public CWnd
{
	DECLARE_DYNAMIC(CTermLog)

	typedef struct 
	{
		int			iPort;
		HWND		hWnd;	
		SOCKET		hTelnetSck;
		HANDLE		hEventQuit;		;
		CQueue		*pQueueRx;
		CQueue		*pQueueTx;
	} SERVER_THREAD_PARAM;

	typedef struct _tagTermCfg
	{
		CString		ComPort;
		int			BaudRate;
		int			EnableDtr;
		int			EnableRts;
		int			ComEnable;
		int			PipeEnable;
		int		    UsbEnable;
		int			TelnetEnable;
		int			ServerEnable;
		CString		PipeName;		
		int			TabSize;
		int			TermRow;
		int			TermCol;
		int			LogFormat;
		int         TermType;
		CString		ServerIp;
		int			Port;
		int			ServerPort;
		CString		UsbDeviceName;
		int			Interface;
		int			EpIn;
		int			EpInSize;
		int			EpOut;
		int			EpOutSize;
	} TERMCFG;
	TERMCFG			m_TermCfg;

	typedef struct _tagLineData
	{
		DWORD	 dwFlags;	    /* flags */		
		TCHAR	 *pszLineText;  /* buffer pointer to the line text */
		BYTE	 *pdbLineAttr;	/* buffer pointer to the line attribute */	
		int		 nLineLen;	    /* length of the text in the line */
	} *LPLINEDATA, LINEDATA;

	CIni		 m_Ini;

// Construction
public:
	CTermLog(CPoint sz);
	void SetTerminalType (int Type);

// Attributes
public:

// Operations
public:

// Overrides

// Implementation
public:	
	FILE        *m_hBinFile;
	FILE        *m_hTxtFile;
	HANDLE       m_hEventAppQuit;
	HANDLE       m_hEventServerQuit;
	SOCKET		 m_hTelnetSck;
	CQueue		 m_TelnetQueueRx;
	CQueue		 m_TelnetQueueTx;

	virtual ~CTermLog();
	BOOL		Create(DWORD dwStyle, RECT& rect, CWnd* pParentWnd, UINT nID);
	void		LogData(int nType, BOOL bEnable);
	BOOL		IsSelected () ;	
	void		DeSelect ();
	void		ConfigInProgress(BOOL bFlag);
	int			SendFile(LPCTSTR FilePath,  int LineInterval, int Protocol);
	int			ReceiveFile(LPCTSTR FilePath,  int LineInterval, int Protocol);
	UINT		ReadData(BYTE *pData, UINT  nLen);
	UINT		WriteData(BYTE *pData, UINT  nLen);
	UINT		WriteNetDataToDevice();
	int			StartTelnetListen ();
	int			ContinueSendFile();

	static		UINT	ListenThread(LPVOID pParam);
	static		UINT	RequestHandler(LPVOID pParam);

private:
	//static const WORD  m_extAscTable[128];
	CWinThread	*m_pThread;
	CString		 m_csSendFile;
	BOOL		 m_bClose;
	CStringA     m_strFileStringPendingToSend;

	CSerial		  m_SerialPort;
	CPipe		  m_Pipe;
	CTelnetClient m_Telnet;	
	CUsbBulk      m_UsbBulk;
	CDeviceIo	 *m_pDevice;

	CList<LINEDATA, LINEDATA&> m_stlLine;
    
	COLORREF	 m_colTable[16];
	COLORREF	 m_colDefBack;
	COLORREF	 m_colDefFore;
	COLORREF	 m_colCurrBack;
	COLORREF	 m_colCurrFore;
	COLORREF	 m_colSelBack;
	COLORREF	 m_colSelFore;

	CBitmap		*m_pCacheBitmap;
	CRect		 m_rcClientRect;
	CFont		 m_Font;
	CPoint		 m_ptScreen;
	CPoint		 m_ptMargin;
	CPoint		 m_ptChar;
	CPoint		 m_ptCaret;
	CPoint		 m_ptStartSelection;
	CPoint		 m_ptEndSelection;
	BOOL		 m_bSelecting;

	CPoint	 	 m_cMouseRepPoint;
	int			 m_iMouseRepDelta;
	WORD		 m_nMouseRepSpeed;
	WORD		 m_nMouseRepCounter;
	bool		 m_bIsMouseRepActive;
	int			 m_iDeltaPerLine;
	int			 m_iAccumDelta;     // for mouse wheel logic
	int			 m_nHistoryLine;
	int			 m_nLastTopLine;     // the last top screen      
	int			 m_nCurrentMaxLineWidth;
	int			 m_nTotalLines;
	int			 m_nSetVScrollPos;
	CPoint		 m_ptFirstVisible;
	POSITION     m_posDispTopLine;
	POSITION     m_posTermTopLine;
	POSITION	 m_posTermCurrLine;
	POSITION     m_posCheckLine;
    BYTE         m_AnsiMode;
	int          m_FirstAttempt;

	void		UpdateParam();
	void		Initialize ();	
	void		ClearBuffer ();
	void		DrawClient (CDC * pDC);
	int			GetLineBlockWithSameAttribute(LPLINEDATA pLineData, int nPos);
	BOOL		AdjustSelection(int nCurrentLine, int *pPos, int *nLen);
	int			CharPositionFromPoint(CPoint pt, CPoint &pos);
	int			GetLineLength(int nLine);
	void		StartMouseRepeat(const CPoint& cPoint, int iDelta, WORD nSpeed);
	void		StopMouseRepeat();
	void		Refresh();
	void		UpdateVScroll();
	void		UpdateHScroll();
	void		HScroll(UINT nSBCode, UINT nPos);
	void		VScroll(UINT nSBCode, UINT nPos);
	void		SetTopLine (int nLine);
	void		UpdateSelection (CPoint ptsCursor, bool bSelect);
	void		AddBlankLine();
	void		SetLineAttr(int nLineDirection, DWORD dwAttrToAdd, DWORD dwAttrToRemove = 0);
	void		TrimLine(POSITION pos, BOOL bChangeLen);	
	int			GetTextBuffer(TCHAR *pBuffer);	
	int			GetSelected(TCHAR *pSelect);
	void		CopyToClipboard(BOOL bSelected);	
	BOOL		TryConnection();
	bool		LogBinary (BYTE *pBuffer, int nLen = 1);
	bool		LogText   (TCHAR Char);
	UINT		SendData();
	int			GetParamList (TCHAR *Input, int *Param, int BufferLen);

	POINT      m_ptCurrent;
	POINT      m_ptSavedCursor;
	POINT      m_ptScrollRaw;
	BYTE       m_dbCurrColor;
	BYTE       m_HightLight;
	POINT      m_Cursor;
	BYTE	   m_dbDefColor;
	int		   m_TabSize;
	BOOL	   m_bRawLog;
	BOOL       m_bLineLastChar;
	BOOL	   m_bConfigInProgress;
	int		   m_IdleTimer;
	CString    m_strPendingToSend;

	void		GotoXy		(int x, int y);
	void		EraseChar	(int StartPos, int EndPos);
	void		SetAttribute(int *Param, int Number);
	void		SetCurrPos (int x, int y);
	int			DoAnsiCtrl (TCHAR *AnsiBuf, int Len);
	bool		CharHandler (TCHAR Char);
	void		OutputChar (TCHAR Char);
	void		ScrollUp (int Lines);
	void		EraseLine (int StartLine, int EndLine, int Color = -1);
	void		ClearScreen(int StartLine, int EndLine, int Color = -1);
	void		AdjustDisplayPos ();
	void		MoveToNextPos();
	void		IntializeScreen();
	void		SetCaretPosition();
	

	void		Test ();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:


public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnUserMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
//	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnCopyAll();
	afx_msg void OnCopySelected();
	afx_msg void OnPaste();
	afx_msg void OnSavePaste();	
	afx_msg void OnClearScreen();
public:
	afx_msg void OnClearHistory();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
