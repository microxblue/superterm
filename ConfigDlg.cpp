// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include ".\configdlg.h"
#include <algorithm>

//#include "PipeCaptureView.h"

// CConfigDlg dialog

IMPLEMENT_DYNAMIC(CConfigDlg, CDialog)
CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)	
{
	m_pView = (CPipeCaptureView *)pParent;
}

CConfigDlg::~CConfigDlg()
{
	DestroyWindow();
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BAUDRATE, m_BaudRate);
	DDX_Control(pDX, IDC_COMPORT, m_ComPort);
	DDX_Control(pDX, IDC_USBDEVICE, m_UsbDevice);
	DDX_Control(pDX, IDC_EDIT1, m_NamePipe);
	DDX_Control(pDX, IDC_EDIT2, m_EditTabSize);
	DDX_Control(pDX, IDC_EDIT3, m_TermRow);
	DDX_Control(pDX, IDC_EDIT4, m_TermCol);
	DDX_Control(pDX, IDC_IP_ADDRESS, m_ServerName);
	DDX_Control(pDX, IDC_EDIT6, m_Port);
	DDX_Control(pDX, IDC_EDIT7, m_ServerPort);
	DDX_Control(pDX, IDC_RADIO1, m_ComEnable);
	DDX_Control(pDX, IDC_RADIO2, m_PipeEnable);
	DDX_Control(pDX, IDC_RADIO3, m_TelnetEnable);
	DDX_Control(pDX, IDC_RADIO5, m_UsbBulkEnable);
	DDX_Control(pDX, IDC_USB_EPIN, m_UsbEpIn);
	DDX_Control(pDX, IDC_USB_INTERFACE, m_UsbInterface);
	DDX_Control(pDX, IDC_USB_EPOUT, m_UsbEpOut);
	DDX_Control(pDX, IDC_CHECK3, m_ServerEnable);
	DDX_Control(pDX, IDC_LOG_DOS, m_LogDos);
	DDX_Control(pDX, IDC_LOG_UNIX, m_LogUnix);
	DDX_Control(pDX, IDC_TERM_UTF8, m_TermUtf8);
	DDX_Control(pDX, IDC_TERM_ANSI, m_TermAnsi);
	DDX_Control(pDX, IDC_TERM_KEY_BS1, m_TermKeyBs1);
	DDX_Control(pDX, IDC_TERM_KEY_BS2, m_TermKeyBs2);
	DDX_Control(pDX, IDC_DTR, m_SerialDtr);
	DDX_Control(pDX, IDC_RTS, m_SerialRts);
	DDX_Control(pDX, IDC_UART_NAME, m_UartName);
	DDX_Control(pDX, IDC_PID, m_UsbPidFilter);	
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)	
	ON_BN_CLICKED(IDOK, OnBnClickedOk)	
	ON_BN_CLICKED(IDC_LOG_UNIX, OnBnClickedLogUnix)
	ON_CBN_SELCHANGE(IDC_USBDEVICE, &CConfigDlg::OnCbnSelchangeUsbdevice)
	ON_BN_CLICKED(IDC_RADIO5, &CConfigDlg::OnBnClickedUsbEnable)
END_MESSAGE_MAP()

CString CConfigDlg::GetPortName(int PortNum)
{
	CString  Result;
	CString  NameStr;	
	CString  Token = _T("");
	
	int i = 0; // substring index to extract
	m_UartName.GetWindowText(NameStr);
	while (AfxExtractSubString(Token, NameStr, i, ';'))
	{
		int Pos = 0;		
		Pos = Token.Find(':');
		if (Pos > 0) {
			if (PortNum == _ttoi(Token.Left(Pos))) {
				Result = Token.Mid(Pos+1);
				break;
			}			
		}
		i++;
	}
	return Result;
}

BOOL CConfigDlg::LoadValuesFromIni()
{
	EnumerateSerialPorts(m_ComPortList);  		
	
	CString  NameStr;
	CString  ComStr;
  CString  ComStrAlt;
	CString  PortStr;
	TCHAR    Buffer[1024];
	int      PortIndex = 0;
	int      PortNum;
	
	m_Ini.GetString(_T("ComPort"), _T("NameMap"), Buffer, sizeof(Buffer), _T(""));
	m_UartName.SetWindowText(Buffer);

	m_Ini.GetString (_T("ComPort"),_T("PortName"), Buffer, sizeof(Buffer), _T("")); 		
	NameStr = Buffer;
	for (int i=0; i<m_ComPortList.GetSize(); i++) {
	  PortNum = m_ComPortList.ElementAt(i);
	  PortStr = GetPortName(PortNum);
	  if (PortStr.IsEmpty()) {
		  ComStr.Format(_T("COM%d"), PortNum);
	  }
	  else {
      ComStr.Format(_T("COM%d (%s)"), PortNum, PortStr);
	  }    
	  if (!i && NameStr.IsEmpty()) NameStr =  ComStr;
	  m_ComPort.InsertString(i, ComStr); 	  
	  if (!NameStr.Compare(ComStr)) {
		  PortIndex = i;      
	  }   
    if (ComStr.Compare(NameStr) == 0) {
      PortIndex = i;    
    }
    if (ComStr.Find(NameStr + _T(" ")) == 0) {
      PortIndex = i;
    }

  }	
	m_ComPort.SetCurSel   (PortIndex); 

	PortIndex = 0;
	m_Ini.GetString (_T("ComPort"),_T("BaudRate"), Buffer, sizeof(Buffer), _T("115200")); 	
	NameStr = Buffer;
	for (int i=0; i<m_BaudRate.GetCount(); i++) {
		m_BaudRate.GetLBText (i, ComStr);
		if (!ComStr.Compare(NameStr)) {
			PortIndex = i;
			break;
		}		
    }
	m_BaudRate.SetCurSel   (PortIndex); 

	int nValue;
	nValue = m_Ini.GetInt(_T("ComPort"), _T("EnableDtr"), 0);
	m_SerialDtr.SetCheck(nValue);
	nValue = m_Ini.GetInt(_T("ComPort"), _T("EnableRts"), 0);
	m_SerialRts.SetCheck(nValue);

	m_Ini.GetString (_T("SoftSdv"), _T("PipeName"), Buffer, sizeof(Buffer), _T("\\\\.\\PIPE\\SOFTSDV1")); 		
	m_NamePipe.SetWindowText (Buffer);

  m_ServerName.LoadHistoryList(m_Ini.GetIniFilePath(), _T("Telnet"), _T("ServerList"));
	//m_Ini.GetString (_T("Telnet"), _T("ServerName"), Buffer, sizeof(Buffer), _T("192.168.0.10")); 		
	//m_ServerName.SetWindowText (Buffer);

	m_Ini.GetString (_T("Telnet"), _T("Port"), Buffer, sizeof(Buffer), _T("23")); 		
	m_Port.SetWindowText (Buffer);

	m_Ini.GetString (_T("Server"), _T("Port"), Buffer, sizeof(Buffer), _T("8888")); 		
	m_ServerPort.SetWindowText (Buffer);
	
	nValue = m_Ini.GetInt (_T("ComPort"),_T("Enable") , 1);
	m_ComEnable.SetCheck(nValue);	

	nValue = m_Ini.GetInt (_T("SoftSdv"),_T("Enable") , 0);
	m_PipeEnable.SetCheck(nValue);	
	
	nValue = m_Ini.GetInt (_T("UsbBulk"),_T("Enable") , 0);
	m_UsbBulkEnable.SetCheck(nValue);

	if (nValue) {
		m_UsbDevice.EnableWindow(0);
		OnBnClickedUsbEnable ();		
	} else { 
    m_UsbPidFilter.EnableWindow(0);
		m_UsbDevice.EnableWindow(0);
		m_UsbEpOut.EnableWindow(0);
		m_UsbEpIn.EnableWindow(0);
	}
	
	nValue = m_Ini.GetInt (_T("Telnet"),_T("Enable") ,  0);
	m_TelnetEnable.SetCheck(nValue);	

	nValue = m_Ini.GetInt (_T("Server"),_T("Enable") ,  0);
	m_ServerEnable.SetCheck(nValue);

	m_Ini.GetString (_T("Console"), _T("TabSize"),  Buffer, sizeof(Buffer), _T("4"));
	m_EditTabSize.SetWindowText (Buffer);
	m_Ini.GetString (_T("Console"), _T("Row"),  Buffer, sizeof(Buffer), _T("25"));
	m_TermRow.SetWindowText (Buffer);
	m_ptScreen.y = _ttoi(Buffer);
	m_Ini.GetString (_T("Console"), _T("Col"),  Buffer, sizeof(Buffer), _T("80"));
	m_ptScreen.x = _ttoi(Buffer);
	m_TermCol.SetWindowText (Buffer);
	
	nValue = m_Ini.GetInt (_T("Logging"),_T("Format"), 0);
	if (nValue)  nValue = 0;
	else		 nValue = 1;
	m_LogDos.SetCheck(nValue);	
	m_LogUnix.SetCheck(1 - nValue);	
		  
	nValue = m_Ini.GetInt (_T("Terminal"),_T("Type"), 0);	
	if (nValue == 1)  m_TermUtf8.SetCheck(1);
	else m_TermAnsi.SetCheck(1);
	
	nValue = m_Ini.GetInt(_T("Terminal"), _T("BsKeyMapping"), 0);
	if (nValue == 1)  m_TermKeyBs2.SetCheck(1);
	else m_TermKeyBs1.SetCheck(1);

	return TRUE;
}


BOOL CConfigDlg::SaveValuesToIni()
{
	CString  NameStr;

	m_UartName.GetWindowText(NameStr);
	m_Ini.WriteString(_T("ComPort"), _T("NameMap"), NameStr);	
	m_ComPort.GetWindowText (NameStr);
	m_Ini.WriteString (_T("ComPort"),_T("PortName"), NameStr); 
	m_BaudRate.GetWindowText (NameStr);
	m_Ini.WriteString (_T("ComPort"),_T("BaudRate"), NameStr); 
    		
	m_Ini.WriteInt(_T("ComPort"), _T("EnableDtr"), m_SerialDtr.GetCheck());	
	m_Ini.WriteInt(_T("ComPort"), _T("EnableRts"), m_SerialRts.GetCheck());

	m_NamePipe.GetWindowText (NameStr);
	m_Ini.WriteString (_T("SoftSdv"), _T("PipeName"), NameStr);

  m_ServerName.SaveHistoryList(m_Ini.GetIniFilePath(), _T("Telnet"), _T("ServerList"));

	m_ServerName.GetWindowText (NameStr);
	m_Ini.WriteString (_T("Telnet"), _T("ServerName"), NameStr);

	m_Port.GetWindowText (NameStr);
	m_Ini.WriteString (_T("Telnet"), _T("Port"), NameStr);

	m_ServerPort.GetWindowText (NameStr);
	m_Ini.WriteString (_T("Server"), _T("Port"), NameStr); 			

	m_Ini.WriteInt (_T("ComPort"),_T("Enable") , m_ComEnable.GetCheck ());
	m_Ini.WriteInt (_T("SoftSdv"),_T("Enable"),  m_PipeEnable.GetCheck ());
	m_Ini.WriteInt (_T("Telnet"),_T("Enable"),   m_TelnetEnable.GetCheck ());
	m_Ini.WriteInt (_T("Server"),_T("Enable"),   m_ServerEnable.GetCheck ());

	int nValue = m_UsbBulkEnable.GetCheck ();
	m_Ini.WriteInt (_T("UsbBulk"),_T("Enable"),  nValue);
	if (nValue)	{
		m_UsbDevice.GetWindowText (NameStr);
		m_Ini.WriteString (_T("UsbBulk"),_T("DeviceName"), NameStr); 

		m_UsbInterface.GetWindowText (NameStr);
		m_Ini.WriteString (_T("UsbBulk"),_T("Interface"), NameStr); 

		m_UsbEpIn.GetWindowText (NameStr);
		m_Ini.WriteString (_T("UsbBulk"),_T("EpIn"), NameStr); 

		m_UsbEpOut.GetWindowText (NameStr);
		m_Ini.WriteString (_T("UsbBulk"),_T("EpOut"), NameStr); 

    m_UsbPidFilter.GetWindowText(NameStr);
    m_Ini.WriteString(_T("UsbBulk"), _T("PidFilter"), NameStr);
	}
	
	m_EditTabSize.GetWindowText (NameStr);	
	int TabSize    = _ttoi (NameStr);	
	if (TabSize <= 0 || TabSize >= 32) {
		TabSize = 4;
	}
	m_Ini.WriteInt (_T("Console"), _T("TabSize"), TabSize);

	m_TermRow.GetWindowText (NameStr);	
	int Row    = _ttoi (NameStr);	
	if (Row <= 0 || Row > 50) {
		Row = 25;
	}
	m_Ini.WriteInt (_T("Console"), _T("Row"), Row);
	m_ptScreen.y = Row;

	m_TermCol.GetWindowText (NameStr);	
	int Col    = _ttoi (NameStr);	
	if (Col <= 0 || Col >= 160) {
		Col = 80;
	}
	m_Ini.WriteInt (_T("Console"), _T("Col"), Col);
	m_ptScreen.x = Col;

	if (m_LogDos.GetCheck()) {
	  m_Ini.WriteInt (_T("Logging"),_T("Format"), 0);      
	} else {
      m_Ini.WriteInt (_T("Logging"),_T("Format"), 1);	  
	}	

	if (m_TermUtf8.GetCheck()) {
	  m_Ini.WriteInt (_T("Terminal"),_T("Type"), 1);      
	} else {
      m_Ini.WriteInt (_T("Terminal"),_T("Type"), 0);	  
	}	

	if (m_TermKeyBs1.GetCheck()) {
		m_Ini.WriteInt(_T("Terminal"), _T("BsKeyMapping"), 0);
	}
	else {
		m_Ini.WriteInt(_T("Terminal"), _T("BsKeyMapping"), 1);
	}

	return TRUE;
}


BOOL CConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here   
	//m_Ini.SetPathName ();

	LoadValuesFromIni ();

	return TRUE;  // return TRUE unless you set the focus to a control
	
}


BOOL CConfigDlg::CheckValues()
{
	// If server and client have same ports, only one of them can be enabled
	CString NameStr;
	if (m_ServerEnable.GetCheck () &&  m_TelnetEnable.GetCheck())  {
		m_ServerPort.GetWindowText(NameStr);
		int  nPort1 = _ttoi (NameStr);
		m_Port.GetWindowText(NameStr);
		int  nPort2 = _ttoi (NameStr);
		if (nPort1 == nPort2) {
			AfxMessageBox (_T("Both Client and Server are enabled, but they have same port number!"));
			return FALSE;
		}
		
		//m_TelnetEnable.GetWindowText (NameStr)

	}
	return TRUE;
}

void CConfigDlg::OnBnClickedOk()
{
	if (!CheckValues()) {
		return;
	}

	SaveValuesToIni ();
	OnOK();
}

bool CConfigDlg::MatchFilter(CString csDevStr, CString csFilter)
{
  int nTokenPos = 0;
  int nFilterLen;
  int nDevStrLen;

  if (csFilter.GetLength() == 0) {
    return true;
  }

  nDevStrLen = csDevStr.GetLength();
  CString strToken = csFilter.Tokenize(_T(";"), nTokenPos);
  while (!strToken.IsEmpty())
  {
    nFilterLen = strToken.GetLength();
    if (nDevStrLen > nFilterLen) {
      if (strToken.CompareNoCase(csDevStr.Mid(nDevStrLen - nFilterLen)) == 0) {
        return true;
      }
    }
    strToken = csFilter.Tokenize(_T("+"), nTokenPos);
  }
  return false;
}

void CConfigDlg::EnumerateUsbDevices (CStringArray& devices, CString csFilter)
{
  struct usb_bus *bus;
  struct usb_device *dev;
  struct usb_device *retdev = NULL;
  int                altidx;

  usb_init(); /* initialize the library */
  usb_find_busses(); /* find all busses */
  usb_find_devices(); /* find all connected devices  */

  CString DeviceString;
  CString Temp;
  devices.RemoveAll(); 
  USES_CONVERSION;
  for(bus = usb_get_busses(); bus; bus = bus->next) {
      for(dev = bus->devices; dev; dev = dev->next) {		  
		  DeviceString = A2T(dev->filename);	
      if (!MatchFilter(DeviceString, csFilter)) {
        continue;
      }
		  for (altidx = 0 ; altidx < dev->config->interface->num_altsetting; altidx++) {
			  if (dev->config->interface->altsetting[altidx].bNumEndpoints) break;
		  }
		  Temp.Format(_T(";%d"), dev->config->interface->altsetting[altidx].bInterfaceNumber);
		  DeviceString.Append(Temp); 
		  for (int j=0; j < dev->config->interface->altsetting[altidx].bNumEndpoints; j++) {
			  if (dev->config->interface->altsetting[altidx].endpoint[j].bmAttributes == 2) { //BULK
				  Temp.Format(_T(";0x%02X [%d]"),
					dev->config->interface->altsetting[altidx].endpoint[j].bEndpointAddress,
					dev->config->interface->altsetting[altidx].endpoint[j].wMaxPacketSize);
					DeviceString.Append(Temp); 
			  }		
		  }
		  DeviceString.Append(_T(";"));
		  devices.Add(DeviceString);
		  //TRACE(_T("%s\n"), DeviceString);
      }
  }  
}

bool SortDescendingInt(const int& x, const int& y)
{
   return x < y;
}

void CConfigDlg::EnumerateSerialPorts(CUIntArray& ports)
{
  //Make sure we clear out any elements which may already be in the array
  ports.RemoveAll();

  //Determine what OS we are running on
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  osvi.dwPlatformId = VER_PLATFORM_WIN32_NT;
  BOOL bGetVer = 1; // GetVersionEx(&osvi);
  
  //On NT use the QueryDosDevice API
  if (bGetVer && (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT))  
  {
    //Use QueryDosDevice to look for all devices of the form COMx. This is a better
    //solution as it means that no ports have to be opened at all.
    TCHAR szDevices[65535];
    DWORD dwChars = QueryDosDevice(NULL, szDevices, 65535);
    if (dwChars)
    {
      int i=0;
      while (1)  {
        //Get the current device name
        TCHAR*  pszCurrentDevice = &szDevices[i];
        int nLen = (int)_tcslen(pszCurrentDevice);
        if (nLen > 3 && _tcsnicmp(pszCurrentDevice, _T("COM"), 3) == 0)
        {
          //Work out the port number
			if (isdigit(pszCurrentDevice[3])) {
				int nPort = _ttoi(&pszCurrentDevice[3]);
				ports.Add(nPort);
			}
        }

        // Go to next NULL character
        while(szDevices[i] != _T('\0'))
          i++;

        // Bump pointer to the next string
        i++;

        // The list is double-NULL terminated, so if the character is
        // now NULL, we're at the end
        if (szDevices[i] == _T('\0'))
          break;
      }
    }    
  } else {
    // Otherwise, try to open device one by one
    for (UINT i=1; i<256; i++)
    {
      //Form the Raw device name
      CString sPort;
      sPort.Format(_T("\\\\.\\COM%d"), i);

      //Try to open the port
      BOOL bSuccess = FALSE;
      HANDLE hPort = ::CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
      if (hPort == INVALID_HANDLE_VALUE)
      {
        DWORD dwError = GetLastError();

        //Check to see if the error was because some other app had the port open or a general failure
        if (dwError == ERROR_ACCESS_DENIED || dwError == ERROR_GEN_FAILURE)
          bSuccess = TRUE;
      }
      else
      {       		
        bSuccess = TRUE;
        CloseHandle(hPort);
      }

      if (bSuccess)
        ports.Add(i);
    }
  }
  std::sort(ports.GetData(), ports.GetData() + ports.GetSize(), SortDescendingInt);
}

void CConfigDlg::UpdateUsbDeviceName()
{
	TCHAR    Buffer[1024];
	CString  NameStr;
  CString  PidFilter;
	int		 DevIdx;
	int		 Intf;
	int		 pos;	
  

	Intf = m_Ini.GetInt (_T("UsbBulk"),_T("Interface"), 0); 		
	m_Ini.GetString (_T("UsbBulk"),_T("DeviceName"), Buffer, sizeof(Buffer), _T("")); 		
	NameStr = Buffer;
	pos = NameStr.Find(_T("--"));
  	if (pos)
		NameStr = NameStr.Mid(pos + 2);
	  
	CString UsbRemaing;
	CString UsbNodeStr;
	CString UsbIntfStr;
	DevIdx = 0;
	  
	for (int i = 0; i < m_UsbDeviceList.GetSize(); i++) {
	  UsbRemaing.Format (_T("%s"), m_UsbDeviceList.ElementAt(i));
	  pos =  UsbRemaing.Find(_T(";"));
	  if (pos < 0) pos = UsbRemaing.GetLength();	  
	  UsbNodeStr  = UsbRemaing.Left(pos).Mid(4);
	  UsbRemaing  = UsbRemaing.Mid(pos+1); 	  	
    
	  m_UsbDevice.InsertString(i, UsbNodeStr); 	  
	  m_UsbDevice.SetItemData(i, i);
	  
	  pos = UsbNodeStr.Find(_T("--"));
  	  if (pos)
		UsbNodeStr = UsbNodeStr.Mid(pos + 2);

	  int a=_ttoi(UsbRemaing);
	  if (!NameStr.Compare(UsbNodeStr) && (Intf == _ttoi(UsbRemaing)) ) {
      DevIdx = i;		  
	  }    
	}	
	m_UsbDevice.SetCurSel   (DevIdx);
}		

void CConfigDlg::UpdateInterfaces()
{
	// TODO: Add your control notification handler code here
	for (int nIndex = m_UsbEpIn.GetCount() - 1; nIndex >= 0; nIndex--) 
	  m_UsbInterface.DeleteString(nIndex);	

	int idx = m_UsbDevice.GetItemData(m_UsbDevice.GetCurSel());
	if (idx >= m_UsbDeviceList.GetSize() || idx < 0) 
		return;
	
	CString UsbRemaing;
	CString UsbNodeStr;
	int pos;
	int intf;
	UsbRemaing = m_UsbDeviceList.GetAt(idx);
	pos =  UsbRemaing.Find(_T(";"));
	if (pos < 0) {
	    UsbRemaing.Empty();
		intf = 0;
	} else {
		UsbRemaing = UsbRemaing.Mid(pos+1);
		intf = _ttoi(UsbRemaing);
	}

	UsbNodeStr.Format(_T("%d"), intf);
	m_UsbInterface.InsertString(0, UsbNodeStr);
	m_UsbInterface.SetCurSel(0);	

}

void CConfigDlg::UpdateEndPoints()
{
	// TODO: Add your control notification handler code here
	for (int nIndex = m_UsbEpIn.GetCount() - 1; nIndex >= 0; nIndex--) 
	  m_UsbEpIn.DeleteString(nIndex);

	for (int nIndex = m_UsbEpOut.GetCount() - 1; nIndex >= 0; nIndex--)  
	  m_UsbEpOut.DeleteString(nIndex);

	int idx = m_UsbDevice.GetItemData(m_UsbDevice.GetCurSel());
	if (idx >= m_UsbDeviceList.GetSize() || idx < 0) 
		return;
	
	CString UsbRemaing;
	CString UsbNodeStr;
	int pos;
	UsbRemaing = m_UsbDeviceList.GetAt(idx);
	pos =  UsbRemaing.Find(_T(";"));
	if (pos < 0) 		
	    UsbRemaing.Empty();
	else {
		pos =  UsbRemaing.Find(_T(";"), pos + 1);
		if (pos < 0) 
			UsbRemaing.Empty();
		else   
			UsbRemaing  = UsbRemaing.Mid(pos+1);
	}


	TCHAR   Buffer[1024];
	m_Ini.GetString (_T("UsbBulk"),_T("EpIn"), Buffer, sizeof(Buffer), _T("")); 		
	CString EpInStr = Buffer;
	m_Ini.GetString (_T("UsbBulk"),_T("EpOut"), Buffer, sizeof(Buffer), _T("")); 		
	CString EpOutStr = Buffer;
	
	pos = UsbRemaing.Find(_T(";"));
	int i = 0, j = 0;
	int nSelIn = 0, nSelOut = 0;
	while (pos >= 0) {
		UsbNodeStr = UsbRemaing.Left(pos);
		UsbRemaing = UsbRemaing.Mid(pos+1);
		if (UsbNodeStr.GetLength() < 4) 
			break;
		if (UsbNodeStr.GetAt(2) == _T('0')) {
			if (!EpOutStr.Compare(UsbNodeStr)) {
				nSelOut = j;
			} 
			m_UsbEpOut.InsertString(j++, UsbNodeStr);
		} else {
			if (!EpInStr.Compare(UsbNodeStr)) {
				nSelIn = i;
			} 
			m_UsbEpIn.InsertString(i++, UsbNodeStr);
		}				
		pos = UsbRemaing.Find(_T(";"));
	}
		
	if (m_UsbEpIn.GetCount() > 0)	
		m_UsbEpIn.SetCurSel(nSelIn);

	if (m_UsbEpOut.GetCount() > 0)
		m_UsbEpOut.SetCurSel(nSelIn);		
}


void CConfigDlg::OnBnClickedLogUnix()
{
	// TODO: Add your control notification handler code here
}

void CConfigDlg::OnCbnSelchangeUsbdevice()
{
	UpdateInterfaces ();
	UpdateEndPoints ();
}

void CConfigDlg::OnBnClickedUsbEnable()
{
	// TODO: Add your control notification handler code here	
	if (m_UsbDevice.IsWindowEnabled())
		return;

  CString  csPidFilter;
  TCHAR    Buffer[1024];
  
  m_Ini.GetString(_T("UsbBulk"), _T("PidFilter"), Buffer, sizeof(Buffer), _T(""));
  m_UsbPidFilter.SetWindowText(Buffer);
  csPidFilter = Buffer;

	EnumerateUsbDevices(m_UsbDeviceList, csPidFilter);
	UpdateUsbDeviceName ();
	UpdateInterfaces ();
	UpdateEndPoints	();	

  m_UsbPidFilter.EnableWindow(1);
	m_UsbDevice.EnableWindow(1);
	m_UsbEpOut.EnableWindow(1);
	m_UsbEpIn.EnableWindow(1);	
}

