#pragma once

#include "IniFile.h"
#include "resource.h"
#include "lusb0_usb.h"
#include "HistoryComboBox.h"

// CConfigDlg dialog
class CPipeCaptureView;

class CConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(CConfigDlg)

	CIni	m_Ini;
public:
	CConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigDlg();

// Dialog Data
	enum { IDD = IDD_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	
	CUIntArray   m_ComPortList;
	CStringArray m_UsbDeviceList;
	CComboBox m_BaudRate;
	CComboBox m_ComPort;
	CComboBox m_UsbDevice;
	CComboBox m_UsbInterface;
	CComboBox m_UsbEpIn;
	CComboBox m_UsbEpOut;
  CHistoryComboBox	m_ServerName;
  virtual BOOL OnInitDialog();
	CEdit   m_NamePipe;
	CEdit   m_TermRow;
	CEdit   m_TermCol;  
	CEdit	m_Port;
	CEdit	m_ServerPort;
        CEdit   m_UartName;
	CButton m_ServerEnable;
	CButton m_ComEnable;
	CButton m_PipeEnable;
	CButton m_TelnetEnable;
	CButton m_UsbBulkEnable;
	CButton m_LogDos;
	CButton m_TermAnsi;	
	CButton m_TermUtf8;	
	CButton m_TermKeyBs1;
	CButton m_TermKeyBs2;
	CButton m_LogUnix;
	CPoint	m_ptScreen;
	CButton m_SerialDtr;
	CButton m_SerialRts;
	CPipeCaptureView *m_pView;

	void EnumerateUsbDevices (CStringArray& devices, CString csFilter);
	void EnumerateSerialPorts(CUIntArray& ports);
	void UpdateUsbDeviceName();
	void UpdateInterfaces();
	void UpdateEndPoints();
  bool MatchFilter(CString csDevStr, CString csFilter);

	BOOL LoadValuesFromIni();
	BOOL SaveValuesToIni();
	BOOL CheckValues();
	CString GetPortName(int PortNum);

	afx_msg void OnBnClickedOk();		
	afx_msg void OnBnClickedLogUnix();
	CEdit m_EditTabSize;
	afx_msg void OnCbnSelchangeUsbdevice();
	afx_msg void OnBnClickedUsbEnable();
		
  CEdit m_UsbPidFilter;
  CButton mTermKeyBs2;
};
