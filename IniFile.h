#ifndef __INI_FILE_H__
#define __INI_FILE_H__

#include "stdafx.h"

class CIni
{
private:
	CString m_pszPathName;

public:		
	
	CIni() {
		SetPathName(_T(""));
	};
	virtual ~CIni() {};

	void SetPathName(CString PathName = _T("")) {
		if (PathName.IsEmpty()) {
			if (m_pszPathName.IsEmpty()) {
				CString IniFile = AfxGetApp()->m_pszHelpFilePath;
				int     Len = IniFile.GetLength();	
				IniFile = IniFile.Left(Len-3) + _T("ini");	
				m_pszPathName = IniFile;
			}
		} else {
			m_pszPathName = PathName;
		}
	}
	
	CString GetIniFilePath() {
		return m_pszPathName;
	}

	DWORD GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDefault = NULL) {
		return GetPrivateProfileString(lpSection, lpKey, lpDefault, lpBuffer, dwBufSize, m_pszPathName);
	}

	BOOL WriteString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue) {
		return WritePrivateProfileString (lpSection, lpKey, lpValue, m_pszPathName);
	}

	int  GetInt(LPCTSTR lpSection, LPCTSTR lpKey, int nDefault) {
		TCHAR buf[64] = _T("");
		GetString(lpSection, lpKey, buf, 32);
		if (!buf[0])  return nDefault;
		else		 return _tstoi(buf);		
	}

	BOOL WriteInt(LPCTSTR lpSection, LPCTSTR lpKey, int nValue) {		
		CString szValue;
		szValue.Format(_T("%d"), nValue);		
		return WriteString(lpSection, lpKey, szValue);
	}
};

#endif // #ifndef __INI_H__