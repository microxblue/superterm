#if !defined(AFX_HISTORYCOMBOBOX_H__C655E8C6_BD69_4CD9_9414_A351482E636A__INCLUDED_)
#define AFX_HISTORYCOMBOBOX_H__C655E8C6_BD69_4CD9_9414_A351482E636A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryComboBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryComboBox window

#define HISTORY_NUM   6
#define ITEM_MAX_LEN  256

class CHistoryComboBox : public CComboBox
{
// Construction
public:
	CHistoryComboBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryComboBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHistoryComboBox();
    int     AddNewString(LPCTSTR lpszString);
	
	void	LoadHistoryList (LPCTSTR IniName, LPCTSTR Section, LPCTSTR Key);
	void	SaveHistoryList (LPCTSTR IniName, LPCTSTR Section, LPCTSTR Key);

	// Generated message map functions
protected:
	//{{AFX_MSG(CHistoryComboBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYCOMBOBOX_H__C655E8C6_BD69_4CD9_9414_A351482E636A__INCLUDED_)
