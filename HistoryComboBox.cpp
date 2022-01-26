// HistoryComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "HistoryComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryComboBox

CHistoryComboBox::CHistoryComboBox()
{
}

CHistoryComboBox::~CHistoryComboBox()
{
}

int CHistoryComboBox::AddNewString(LPCTSTR lpszString)
{
  int nRet = -1;
  
  CString TextString(lpszString);
  TextString.TrimLeft (_T(" "));
  TextString.TrimRight(_T(" "));

  //if (TextString.IsEmpty()) return 0; 

  nRet = InsertString(0, TextString);
  int nIndex = FindStringExact(0, TextString);
  if (nIndex != -1 && nIndex != 0) DeleteString(nIndex);

  int nNumItems = GetCount();
  for (nIndex = HISTORY_NUM; nIndex < nNumItems; nIndex++)  DeleteString(nIndex);
  SetCurSel(nRet);

  return nRet;
}

void CHistoryComboBox::LoadHistoryList (LPCTSTR IniName, LPCTSTR Section, LPCTSTR Key)
{
	CString InKey;	
	TCHAR   Buffer[ITEM_MAX_LEN];

	LimitText (ITEM_MAX_LEN); 
	for (int Index = 0; Index < HISTORY_NUM;Index++) {		
		InKey.Format(_T("%s%d"), Key, Index);
		GetPrivateProfileString (Section, InKey.GetBuffer(0), _T(""), Buffer, sizeof(Buffer), IniName);		
		InKey.ReleaseBuffer();
		if (Buffer[0])
			AddString(Buffer);		 
	}
	SetCurSel(0);	
}

void CHistoryComboBox::SaveHistoryList (LPCTSTR IniName, LPCTSTR Section, LPCTSTR Key)
{
	CString InKey;    
	int     Ret;	
	TCHAR   Buffer[ITEM_MAX_LEN];
	
    GetWindowText(Buffer, sizeof(Buffer));
	AddNewString (Buffer);
	
	for (int Index = 0; Index < HISTORY_NUM;Index++) {		
		InKey.Format(_T("%s%d"), Key, Index);		
		Buffer[0] = 0;
		Ret = GetLBText(Index, Buffer);
		if (Ret != CB_ERR /*&& Buffer[0]*/) {
		    Buffer[ITEM_MAX_LEN-1] = 0;
			if (Buffer[0]) {
				WritePrivateProfileString (Section, InKey.GetBuffer(0), Buffer, IniName);			
				InKey.ReleaseBuffer(); 
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CHistoryComboBox, CComboBox)
	//{{AFX_MSG_MAP(CHistoryComboBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryComboBox message handlers
