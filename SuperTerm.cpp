// SuperTerm.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SuperTerm.h"
#include "MainFrm.h"
#include "ConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSuperTermApp

BEGIN_MESSAGE_MAP(CSuperTermApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CSuperTermApp::OnAppAbout)
//	ON_COMMAND(ID_VIEW_OPTIONS, &CSuperTermApp::OnViewOptions)
END_MESSAGE_MAP()


// CSuperTermApp construction

CSuperTermApp::CSuperTermApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CSuperTermApp object

CSuperTermApp theApp;


// CSuperTermApp initialization

BOOL CSuperTermApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// Generate INI file
	CConfigDlg  dlg;	
	dlg.Create(IDD_CONFIG); 
	//dlg.LoadValuesFromIni(); 
	dlg.SaveValuesToIni();
	dlg.CloseWindow();


	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications0"));

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// ***** THE FOLLOWING CODE IS THE PART YOU NEED *****
	// force a title-bar icon (MFC Wizard doesn't set it up correctly)
	HICON hIcon = LoadIcon(IDR_MAINFRAME);
	HICON hPrevIcon = pFrame->SetIcon(hIcon,FALSE);
	if (hPrevIcon && hPrevIcon != hIcon) DestroyIcon(hPrevIcon);


	return TRUE;
}


// CSuperTermApp message handlers




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CSuperTermApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CSuperTermApp message handlers
