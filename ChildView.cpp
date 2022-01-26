// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "SuperTerm.h"
#include "ChildView.h"
#include "ConfigDlg.h"
#include "SendFileDlg.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	m_pwndTermLog = NULL;
	m_ptScreen.x = 0;
	m_ptScreen.y = 0;
	m_ptMargin.x = 1;
	m_ptMargin.y = 1;
}

CChildView::~CChildView()
{
	if (m_pwndTermLog)
		delete m_pwndTermLog;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_OPTIONS, &CChildView::OnViewOptions)
	ON_COMMAND(ID_FILE_LOGRAWDATA, &CChildView::OnFileLogRawData)
	ON_COMMAND(ID_VIEW_LOGOUTPUT, &CChildView::OnViewLogOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOGOUTPUT, &CChildView::OnUpdateViewLogoutput)
	ON_UPDATE_COMMAND_UI(ID_FILE_LOGRAWDATA, &CChildView::OnUpdateFileLograwdata)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_OUTPUT, &CChildView::OnUpdateViewShowOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_BINARY, &CChildView::OnUpdateViewShowBinary)
	ON_COMMAND(ID_VIEW_SHOW_OUTPUT, &CChildView::OnViewShowOutput)
	ON_COMMAND(ID_VIEW_SHOW_BINARY, &CChildView::OnViewShowBinary)
	ON_UPDATE_COMMAND_UI(ID__COPY_SELECTED, &CChildView::OnUpdateCommandCopySelected)
	ON_COMMAND(ID__COPY_SELECTED, &CChildView::OnCopySelected)
	ON_COMMAND(ID__COPY_ALL, &CChildView::OnCopyAll)	
	ON_COMMAND(ID__PASTE, &CChildView::OnPaste)	
	ON_COMMAND(ID__CLEAR_SCREEN, &CChildView::OnClearScreen)
	ON_COMMAND(ID__CLEAR_HISTORY, &CChildView::OnClearHistory)
	ON_COMMAND(ID_FILE_SENDFILE, &CChildView::OnSendFile)

	ON_WM_CLOSE()
	//ON_COMMAND(ID_FILE_RECEIVEFILE, &CChildView::OnFileReceivefile)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}


void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here	
	CRect rc;
	GetClientRect(rc);	
	
	if (m_pwndTermLog && ::IsWindow(m_pwndTermLog->m_hWnd)) { 
		m_pwndTermLog->MoveWindow(rc);
	}
}

int CChildView::CreateTermCtrl()
{
	DWORD dwViewStyle =	WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE;	
	ASSERT(!m_pwndTermLog);
	m_pwndTermLog = new CTermLog(m_ptScreen);	
	if(!m_pwndTermLog->Create( dwViewStyle, CRect(0, 0, 0, 0), this, IDC_TERMLOG)) {
		return -1;
	}	

	m_ptScreen.x = m_pwndTermLog->m_TermCfg.TermCol;
	m_ptScreen.y = m_pwndTermLog->m_TermCfg.TermRow;

	// Resize the MainWindows	
	CREATESTRUCT cs;	
	cs.cx = (m_ptScreen.x + m_ptMargin.x * 2) * LOG_FONT_WIDTH  + ::GetSystemMetrics(SM_CXVSCROLL) + 5;
	cs.cy = (m_ptScreen.y + m_ptMargin.y * 2) * LOG_FONT_HEIGHT + ::GetSystemMetrics(SM_CYHSCROLL) + 48;	

	CRect  rcClient;
	rcClient.left   = 0;
	rcClient.top    = 0;
	rcClient.right  = cs.cx;
	rcClient.bottom = cs.cy;
	AdjustWindowRect(rcClient, WS_OVERLAPPED | WS_CAPTION, TRUE);
	
	CRect  rect;
	CWnd   *pWnd = AfxGetMainWnd();
	CIni	Ini;
	
	rect.left = (::GetSystemMetrics(SM_CXSCREEN) - rcClient.Width())/2; 
	rect.left = Ini.GetInt(_T("Window"), _T("Left"), rect.left); 
	
	rect.top  = (::GetSystemMetrics(SM_CYSCREEN) - rcClient.Height())/2;
	rect.top  = Ini.GetInt(_T("Window"), _T("Top"), rect.top); 

	rect.right  = rect.left + rcClient.Width();
	rect.bottom = rect.top  + rcClient.Height();
	pWnd->MoveWindow (rect);

	return 0;
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    CreateTermCtrl ();

	return 0;
}

void CChildView::OnViewOptions()
{
	// TODO: Add your command handler code here
	CConfigDlg  dlg;

	CPoint ptScreen;
	ptScreen.x = m_pwndTermLog->m_TermCfg.TermCol;
	ptScreen.y = m_pwndTermLog->m_TermCfg.TermRow;
	m_pwndTermLog->ConfigInProgress(TRUE);
	dlg.DoModal();
		
	if (dlg.m_ptScreen == ptScreen) {
		m_pwndTermLog->ConfigInProgress(FALSE);
	} else {
		m_pwndTermLog->CloseWindow ();
		delete m_pwndTermLog;
		m_pwndTermLog = NULL;
		CreateTermCtrl ();
	}	
}

void CChildView::OnFileLogRawData()
{
	// TODO: Add your command handler code here
	if (m_pwndTermLog) {
		if (m_pwndTermLog->m_hBinFile)
			m_pwndTermLog->LogData (FILE_LOG_TYPE_BINARY, FALSE);
		else 
			m_pwndTermLog->LogData (FILE_LOG_TYPE_BINARY, TRUE);			
	}
}

void CChildView::OnViewLogOutput()
{
	// TODO: Add your command handler code here
	if (m_pwndTermLog) {
		if (m_pwndTermLog->m_hTxtFile)
			m_pwndTermLog->LogData (FILE_LOG_TYPE_TEXT, FALSE);
		else 
			m_pwndTermLog->LogData (FILE_LOG_TYPE_TEXT, TRUE);			
	}
}


void CChildView::OnUpdateViewLogoutput(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_pwndTermLog) {
		if (m_pwndTermLog->m_hTxtFile)
			pCmdUI->SetCheck (TRUE); 
		else 
			pCmdUI->SetCheck (FALSE); 
	}
}

void CChildView::OnUpdateFileLograwdata(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_pwndTermLog) {
		if (m_pwndTermLog->m_hBinFile)
			pCmdUI->SetCheck (TRUE); 
		else 
			pCmdUI->SetCheck (FALSE); 
	}
}

void CChildView::OnUpdateViewShowOutput(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_pwndTermLog) {
		if (m_pwndTermLog->m_hTxtFile) {
			pCmdUI->Enable (TRUE);
		} else {
			pCmdUI->Enable (FALSE);
		}
	}
}

void CChildView::OnUpdateViewShowBinary(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_pwndTermLog) {
		if (m_pwndTermLog->m_hBinFile) {
			pCmdUI->Enable (TRUE);
		} else {
			pCmdUI->Enable (FALSE);
		}
	}
}

void CChildView::OnViewShowOutput()
{
	// TODO: Add your command handler code here
	if (m_pwndTermLog) {
		m_pwndTermLog->LogData (FILE_LOG_TYPE_TEXT, FALSE); 
		ShellExecute (NULL, _T("OPEN"), _T("Debug.log"), NULL, _T("."), SW_SHOW);
	}
	  
}

void CChildView::OnViewShowBinary()
{
	// TODO: Add your command handler code here
	if (m_pwndTermLog) {
		m_pwndTermLog->LogData (FILE_LOG_TYPE_BINARY, FALSE); 
		ShellExecute (NULL, _T("OPEN"), _T("Debug.bin"), NULL, _T("."), SW_SHOW);
	}
}

void CChildView::OnUpdateCommandCopySelected(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_pwndTermLog)
		pCmdUI->Enable (m_pwndTermLog->IsSelected());
}

void CChildView::OnPaste()
{
	// TODO: Add your command handler code here	
	if (m_pwndTermLog) {
		m_pwndTermLog->OnPaste ();
	}
}

void CChildView::OnCopyAll()
{
	// TODO: Add your command handler code here	
	if (m_pwndTermLog) {
		m_pwndTermLog->OnCopyAll ();
	}
}


void CChildView::OnCopySelected()
{
	if (m_pwndTermLog) {
		m_pwndTermLog->OnCopySelected ();
	}
}

void CChildView::OnClearScreen()
{
	// TODO: Add your command handler code here
	if (m_pwndTermLog) {
		m_pwndTermLog->OnClearScreen ();
	}
}

void CChildView::OnClearHistory()
{
	// TODO: Add your command handler code here	
	if (m_pwndTermLog) {
		m_pwndTermLog->OnClearHistory ();
	}
}

void CChildView::OnSendFile()
{
	// TODO: Add your command handler code here	
	if (m_pwndTermLog) {
		CSendFileDlg  dlg;
		if (dlg.DoModal() == IDOK) { 		
			// Send File
			m_pwndTermLog->SendFile(dlg.m_FilePath, dlg.m_LineInterval, 0);
		}
	}
}

void CChildView::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (m_pwndTermLog) {
		m_pwndTermLog->OnClose(); 
	}
	CWnd::OnClose();
}

void CChildView::ConfigInProgress(bool bFlag)
{
  if (m_pwndTermLog) {
    m_pwndTermLog->ConfigInProgress(bFlag);
  }
}
