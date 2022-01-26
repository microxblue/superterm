#pragma once
#include "IniFile.h"

// CSendFileDlg dialog

class CSendFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CSendFileDlg)
	
public:
	CIni	    m_Ini;
	CString		m_IniFile;
	CString		m_FilePath;
	int			m_LineInterval;

	CSendFileDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSendFileDlg();

// Dialog Data
	enum { IDD = IDD_SENDFILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
