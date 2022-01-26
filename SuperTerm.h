// SuperTerm.h : main header file for the SuperTerm application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CSuperTermApp:
// See SuperTerm.cpp for the implementation of this class
//

class CSuperTermApp : public CWinApp
{
public:
	CSuperTermApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnViewOptions();
};

extern CSuperTermApp theApp;