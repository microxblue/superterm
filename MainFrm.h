// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "ChildView.h"
#include "TrueColorToolBar.h"
#include "MenuEx.h"

class CMainFrame : public CFrameWnd
{
private:
	// BIT0:1   00  Connect OFF
	// BIT0:1   01  Connect Serial
	// BIT0:1   10  Connect Pipe
	// BIT0:1   11  Connect Telnet

    // BIT4:5   00  No service
	// BIT4:5   01  Listening
	// BIT4:5   02  Connected
	DWORD		m_dwConnectStatus;
	CImageList  m_imgList;

public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);	

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;	
	CChildView  m_wndView;
	HICON		m_hLedIconOn;
	HICON		m_hLedIconOff;
	CTrueColorToolBar  m_wndToolBar;	
	CMenuEx*    m_pMainMenu;

	HICON		m_hPrevConnectIcon;
	HICON		m_hPrevServerIcon;

	HICON       GetIconForItem(UINT itemID);

// Generated message map functions
protected:	
	afx_msg LRESULT OnUserMessage(WPARAM wParam, LPARAM lParam);
	afx_msg	void OnUpdateConnectStatusLed(CCmdUI* pCmdUI);
	afx_msg	void OnUpdateServerStatusLed(CCmdUI* pCmdUI);
	afx_msg	void OnUpdateVtConsoleType(CCmdUI* pCmdUI);	
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpmis);
           
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnViewOptions();
	afx_msg void OnClose();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
};


