// ChildView.h : interface of the CChildView class
//


#pragma once

#include "TermLog.h"

#define  IDC_TERMLOG    1

// CChildView window
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:	

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
  void ConfigInProgress(bool bFlag);
private:
	CTermLog   *m_pwndTermLog;
	CPoint		m_ptScreen;
	CPoint		m_ptMargin;

	int			CreateTermCtrl();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	afx_msg void OnViewOptions();
public:
	afx_msg void OnFileLogRawData();
public:
	afx_msg void OnViewLogOutput();
public:
	afx_msg void OnUpdateViewLogoutput(CCmdUI *pCmdUI);
public:
	afx_msg void OnUpdateFileLograwdata(CCmdUI *pCmdUI);
public:
	afx_msg void OnUpdateViewShowOutput(CCmdUI *pCmdUI);
public:
	afx_msg void OnUpdateViewShowBinary(CCmdUI *pCmdUI);
public:
	afx_msg void OnViewShowOutput();
public:
	afx_msg void OnViewShowBinary();
public:	
	afx_msg void OnCopyAll();
	afx_msg void OnCopySelected();
	afx_msg void OnClearScreen();
	afx_msg void OnClearHistory();
	afx_msg void OnPaste();
	afx_msg void OnSendFile();
	afx_msg void OnUpdateCommandCopySelected(CCmdUI *pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnFileReceivefile();
};

