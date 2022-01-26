// SendFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SuperTerm.h"
#include "SendFileDlg.h"
#include "HistoryComboBox.h"
#include ".\sendfiledlg.h"

// CSendFileDlg dialog

IMPLEMENT_DYNAMIC(CSendFileDlg, CDialog)
CSendFileDlg::CSendFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendFileDlg::IDD, pParent)
{
	m_LineInterval = 100;  //100ms
}

CSendFileDlg::~CSendFileDlg()
{
}

void CSendFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSendFileDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnFileBrowse)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,LPARAM lParam,LPARAM lpData)
{    
    if(uMsg == BFFM_INITIALIZED) {
      SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}


// CSendFileDlg message handlers
void CSendFileDlg::OnFileBrowse() 
{
	CString     InitFolderPath;
	CFileDialog fOpenDlg(TRUE, _T(""), _T("*.*"), OFN_FILEMUSTEXIST,  _T("*.*"), this);


	fOpenDlg.m_pOFN->lpstrTitle = _T("Select Send File");
	fOpenDlg.m_pOFN->lpstrInitialDir = InitFolderPath;
	if	(fOpenDlg.DoModal()==IDOK) {
		((CComboBox*)GetDlgItem(IDC_COMBO_FILE))->SetWindowText(fOpenDlg.GetPathName());
	}	
}


BOOL CSendFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here	
	m_IniFile = m_Ini.GetIniFilePath();		
	((CHistoryComboBox*)GetDlgItem(IDC_COMBO_FILE))->LoadHistoryList(m_IniFile, _T("Send"), _T("File"));

    TCHAR    Buffer[1024];	
	m_Ini.GetString (_T("SendFile"), _T("LineInterval"), Buffer, sizeof(Buffer), _T("100")); 			
	((CEdit *)GetDlgItem(IDC_INTERVAL))->SetWindowText(Buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSendFileDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here	
	CString  strLineInterval;
	((CEdit *)GetDlgItem(IDC_INTERVAL))->GetWindowText(strLineInterval);
	
	m_LineInterval = _tstol(strLineInterval);
	if (m_LineInterval <= 0 && m_LineInterval >= 10000) {
		AfxMessageBox(_T("Invalue Line Interfavlue value!"));
		return;
	} else {		
		m_Ini.WriteInt (_T("SendFile"), _T("LineInterval"), m_LineInterval);	
	}

	((CHistoryComboBox*)GetDlgItem(IDC_COMBO_FILE))->SaveHistoryList(m_IniFile,  _T("Send"), _T("File"));	
	((CComboBox*)GetDlgItem(IDC_COMBO_FILE))->GetWindowText(m_FilePath);
	if (GetFileAttributes(m_FilePath) != INVALID_FILE_ATTRIBUTES) {		
		OnOK();
	} else {
		m_FilePath.Empty(); 
		AfxMessageBox (_T("Invalid File Path!"));
	}
}
