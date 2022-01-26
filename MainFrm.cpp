// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "SuperTerm.h"
#include "MenuEx.h"
#include "MainFrm.h"
#include "IniFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)
IMPLEMENT_MENUEX(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MENUEX_MESSAGES()
	ON_MESSAGE(WM_USER, OnUserMessage) 
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_EXT, OnUpdateConnectStatusLed)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_REC, OnUpdateServerStatusLed)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCRL, OnUpdateVtConsoleType)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_REC,
	ID_INDICATOR_EXT
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_dwConnectStatus = 0;
	m_hPrevConnectIcon = NULL;
	m_hPrevServerIcon  = NULL;
	m_pMainMenu        = NULL;

}

CMainFrame::~CMainFrame()
{
	::DestroyIcon(m_hPrevConnectIcon);
	::DestroyIcon(m_hPrevServerIcon);

	if (m_pMainMenu)
		delete m_pMainMenu;

}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndToolBar.LoadTrueColorToolBar(
        16,
        IDB_TOOLBAR
		);
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(4,ID_INDICATOR_REC, SBPS_DISABLED , 15);
	m_wndStatusBar.SetPaneInfo(5,ID_INDICATOR_EXT, SBPS_DISABLED , 15);

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);		
	CBitmap bm; 
	bm.LoadBitmap(IDB_STATUS); 
	m_imgList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 8, 8); 
	m_imgList.Add(&bm, RGB(255,255,255)); 

	m_hLedIconOn = (HICON)LoadImage(AfxGetApp()->m_hInstance,
                                  MAKEINTRESOURCE(IDR_LED_ON),
                                  IMAGE_ICON,16,16,
                                  LR_DEFAULTCOLOR
								  );

	m_hLedIconOff = (HICON)LoadImage(AfxGetApp()->m_hInstance,
                                  MAKEINTRESOURCE(IDR_LED_OFF),
                                  IMAGE_ICON,16,16,
								  LR_DEFAULTCOLOR
								  );	

	m_pMainMenu = new CMenuEx(this);
    if(m_pMainMenu->LoadMenu(IDR_MAINFRAME))
    {
        /*
            Custom color & size settings
            for startup should go here
        */
		m_pMainMenu->UseToolBarImages(&m_wndToolBar);
        ::DestroyMenu(m_hMenuDefault);
        SetMenu(m_pMainMenu);
        m_hMenuDefault = m_pMainMenu->GetSafeHmenu();
    }

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_MINIMIZEBOX | WS_SYSMENU;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnUpdateConnectStatusLed(CCmdUI* pCmdUI)
{		
	CStatusBarCtrl *pBar = &m_wndStatusBar.GetStatusBarCtrl();		
	static DWORD  dwOldState = -1;
	DWORD dwState = m_dwConnectStatus & 0x0F;
	HICON hIcon;

	if (dwState == dwOldState) {
		hIcon = m_hPrevConnectIcon;
	} else {
		switch (dwState) {
		case THREAD_ID_SERIAL:  // Serial
			hIcon = m_imgList.ExtractIconW(4);
			break;
		case THREAD_ID_PIPE:  // Pipe
			hIcon = m_imgList.ExtractIconW(5);
			break;
		case THREAD_ID_USBBULK: // USB
			hIcon = m_imgList.ExtractIconW(7);
			break;
		case THREAD_ID_TELNET:  // Telnet
			hIcon = m_imgList.ExtractIconW(3);
			break;
		default: // No connect
			hIcon = m_imgList.ExtractIconW(6);
			break;
		}
		::DestroyIcon(m_hPrevConnectIcon);
		m_hPrevConnectIcon = hIcon;
		dwOldState = dwState;
	}
	pBar->SetIcon(pCmdUI->m_nIndex, hIcon);
	
}

void CMainFrame::OnUpdateVtConsoleType(CCmdUI* pCmdUI)
{
	BYTE  Mode = (BYTE)((m_dwConnectStatus>>16) & 0x0F);

	if (Mode == VT_UTF8) {
		pCmdUI->SetText(_T("UTF8"));
	} else if (Mode == VT_ANSI) {
		pCmdUI->SetText(_T("ANSI"));
	} else {
		pCmdUI->SetText(_T("????"));
	}	
}

void CMainFrame::OnUpdateServerStatusLed(CCmdUI* pCmdUI)
{	
	CStatusBarCtrl *pBar = &m_wndStatusBar.GetStatusBarCtrl();		
	static DWORD  dwOldState = -1;
	DWORD dwState = (m_dwConnectStatus >> 4) & 0x03;
	HICON hIcon;

	if (dwState == dwOldState) {
		hIcon = m_hPrevServerIcon;
	} else {
		switch (dwState) {
		case 1:  // Listen
			hIcon = m_imgList.ExtractIconW(0);
			break;
		case 2:  // Connected
			hIcon = m_imgList.ExtractIconW(2);
			break;
		default: // No service
			hIcon = m_imgList.ExtractIconW(1);
			break;
		}
		::DestroyIcon(m_hPrevServerIcon);
		m_hPrevServerIcon = hIcon;
		dwOldState = dwState;
	}
	pBar->SetIcon(pCmdUI->m_nIndex, hIcon);	
}

LRESULT CMainFrame::OnUserMessage(WPARAM wParam, LPARAM lParam)
{
	DWORD  dwCmd = (DWORD)wParam;

	if (wParam == 0) { //Change Connection State LED		
		m_dwConnectStatus = (m_dwConnectStatus & 0xFFFFFFF0) | ((DWORD)lParam & 0x0F);
	} else if (wParam == 1) { //Change Server State LED		
		m_dwConnectStatus = (m_dwConnectStatus & 0xFFFFFF0F) | (((DWORD)lParam & 0x0F)<<4);
	} else if (wParam == 2) { //Change Console Type		
		m_dwConnectStatus = (m_dwConnectStatus & 0xFFF0FFFF) | (((DWORD)lParam & 0x0F)<<16);
	}
	
  if (wParam == 0x1000) {
    CIni		 fIni;
    // Switch UART
    m_wndView.ConfigInProgress(TRUE);
        
    fIni.WriteInt(_T("ComPort"), _T("Enable"), 1);
    fIni.WriteInt(_T("SoftSdv"), _T("Enable"), 0);
    fIni.WriteInt(_T("Telnet"), _T("Enable"),  0);
    fIni.WriteInt(_T("Server"), _T("Enable"),  0);

    CString ComStr;
    ComStr.Format(_T("COM%d"), (BYTE)lParam);
    fIni.WriteString(_T("ComPort"), _T("PortName"), (LPCTSTR)ComStr);
    m_wndView.ConfigInProgress(FALSE);
  }

	if (wParam == 0x2000) {
		if ((BYTE)lParam == 0) {
			m_wndView.ConfigInProgress(FALSE);
    }
		if ((BYTE)lParam == 1) {
			m_wndView.ConfigInProgress(TRUE);
		}
	}

	return 0;
}

HICON CMainFrame::GetIconForItem(UINT itemID) 
{
  HICON hIcon = (HICON)0;
	CToolBarCtrl  *pToolBar = &m_wndToolBar.GetToolBarCtrl();
	
	TBBUTTON  tbButton;
	int i, j;
	int nCnt = pToolBar->GetButtonCount(); 
	for (i=0, j=0; i<nCnt; i++) {
		pToolBar->GetButton(i,&tbButton); 
		if (tbButton.idCommand == itemID)
			break;
		if (tbButton.fsStyle & BTNS_SEP) 
			continue;
		j++;
	}
	
	if (i != nCnt) {
		hIcon = (pToolBar->GetImageList())->ExtractIcon (j);   
	} else {
		hIcon = NULL;
	}

    return hIcon;
}

void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CIni  Ini;
	WINDOWPLACEMENT WndStatus;

	WndStatus.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&WndStatus);

	Ini.WriteInt (_T("Window"), _T("Left"), WndStatus.rcNormalPosition.left);
	Ini.WriteInt (_T("Window"), _T("Top"),  WndStatus.rcNormalPosition.top);

	m_wndView.OnClose(); 
	CFrameWnd::OnClose();

}
