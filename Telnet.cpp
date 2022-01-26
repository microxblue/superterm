#include "stdafx.h"
#include "Telnet.h"
#include ".\telnet.h"

CTelnetClient::CTelnetClient ( void )
{	
	m_Type          = EnumTypeTelnet;

	m_ConnectState  = EnumTypeIdle;	
	m_bNegociation  = FALSE;
	m_nCount        = 0;
	m_hWndNotify    = NULL;
}

CTelnetClient::~CTelnetClient ( void )
{
	Close();	
}

BOOL CTelnetClient::IoOpen( CWnd *pWnd, TCHAR *pCfgStr)
{      	
	CString  csServer;
	DWORD	 Port;
	TCHAR	 seps[] = _T(";");
	TCHAR    *ptr;
	TCHAR    *token;
	TCHAR    *argstr;

	if (m_ConnectState == EnumTypeConnecting)
		return TRUE;

	if (m_ConnectState == EnumTypeConnected) {		
		return TRUE;
	}	

	if (m_nCount) {
		m_nCount --;
		// Don't connect too soon
		return FALSE;
	}
	
	m_nCount   = CONNECT_INTERVAL;
	Port = 23;

	/* Establish string and get the first token: */		
	csServer.Empty();
	argstr = _tcsdup(pCfgStr);
	token  = _tcstok(argstr, seps );
	while( token != NULL )
	{
		if ((ptr = _tcschr(token, _T('='))) > 0) {
			*ptr = 0;
			if (!_tcsicmp(_T("ServerName"), token))
				csServer.Format(_T("%s"), ptr + 1); 
			else if (!_tcsicmp(_T("Port"), token))
				Port = _ttoi (ptr + 1);
			*ptr = _T('=');
		}
		token  = _tcstok (NULL, seps);
	}
	free (argstr);

	m_nPort    = Port;
	m_csServer = csServer;
	m_ConnectState  = EnumTypeConnecting;
	m_QueueTx.Reset (); 
	m_QueueRx.Reset (); 		
	m_hWndNotify = pWnd->GetSafeHwnd() ;
	m_bNegociation = TRUE;
    m_csName = m_csServer;

	// Reset internal state machine
	HandleOptions (-1);

	if ((HANDLE)m_hSocket == INVALID_HANDLE_VALUE) {
		Create ();
	}

	BOOL  bRes  = Connect (m_csServer, m_nPort);	
	DWORD dwErr = GetLastError ();

	if (WSAEWOULDBLOCK != dwErr) {
		IoClose ();		
		return FALSE;
	}

	return  FALSE;
}

VOID CTelnetClient::CleanUp()
{	
	m_ConnectState = EnumTypeIdle;
	m_hDevice    = INVALID_HANDLE_VALUE;	
	m_hWndNotify = NULL;
}

VOID CTelnetClient::IoClose ()
{	
	CleanUp ();
	Close ();	
	m_nCount  = 0;
}

BOOL CTelnetClient::IoIsValid ()
{
	return (m_hDevice != INVALID_HANDLE_VALUE);
}

int CTelnetClient::IoRead ( BYTE *Buffer , int nNumberOfBytesToRead)
{
	return m_QueueRx.Get((BYTE *)Buffer, nNumberOfBytesToRead);
}

int CTelnetClient::IoWrite ( BYTE *Buffer , int nNumberOfBytesToWrite)
{
	int	nLen;	
	
	nLen = (int)m_QueueTx.Put((BYTE *)Buffer, nNumberOfBytesToWrite);
	if (nLen != nNumberOfBytesToWrite) {
		TRACE ("Telnet TX FIFO is full !\n");
	}

	// Signal to send
	OnSend (-1);

	return 0;    
}

void CTelnetClient::IoReset ()
{
	m_QueueRx.Reset();
	m_QueueTx.Reset();
}

UINT CTelnetClient::CallBackThread ( LPVOID pParam )
{
	return 0;
}

CString CTelnetClient::ArrangeReply(CString strOption)
{

	TCHAR Verb;
	TCHAR Option;
	TCHAR Modifier;
	TCHAR ch;
	BOOL bDefined = FALSE;
	CString  strResp;

	strResp.Empty();

	if(strOption.GetLength() < 3) 
		return strResp;

	Verb = strOption.GetAt(1);
	Option = strOption.GetAt(2);

	switch(Option)
	{
	case 1:	// Echo
	case 3: // Suppress Go-Ahead
		bDefined = TRUE;
		break;
	}

	strResp += IAC;

	if(bDefined == TRUE)
	{
		switch(Verb)
		{
		case DO:
			ch = WILL;
			strResp += ch;
			strResp += Option;
			break;
		case DONT:
			ch = WONT;
			strResp += ch;
			strResp += Option;
			break;
		case WILL:
			ch = DO;
			strResp += ch;
			strResp += Option;
			break;
		case WONT:
			ch = DONT;
			strResp += ch;
			strResp += Option;
			break;
		case SB:
			Modifier = strOption.GetAt(3);
			if(Modifier == SEND)
			{
				ch = SB;
				strResp += ch;
				strResp += Option;
				strResp += IS;
				strResp += IAC;
				strResp += SE;
			}
			break;
		}
	}

	else
	{
		switch(Verb)
		{
		case DO:
			ch = WONT;
			strResp += ch;
			strResp += Option;
			break;
		case DONT:
			ch = WONT;
			strResp += ch;
			strResp += Option;
			break;
		case WILL:
			ch = DONT;
			strResp += ch;
			strResp += Option;
			break;
		case WONT:
			ch = DONT;
			strResp += ch;
			strResp += Option;
			break;
		}
	}

	return strResp;
}


int CTelnetClient::HandleOptions (int  data)
{
	static   int		nState;
	static   CString	strTemp;
	TCHAR				cBuffer[2];
	int					nRet = 0;

	if (data < 0) {
		m_ListOptions.RemoveAll();
		strTemp.Empty(); 
		nState = 0;
		return nRet;
	}

	cBuffer[1] = 0;
	cBuffer[0] = data;
	switch (nState) {
	case 0:
		if (data == IAC) {
			strTemp.Empty();					
			strTemp.Append(cBuffer);
			nState = 1;
			nRet   = 1;
		}
		break;

	case 1:
		nState = 3;		
		switch (data) {
		case DO:
		case DONT:
		case WILL:
		case WONT:								
			break;
		case IAC:				
			break;
		case SB:				
			nState = 2;												
			break;
		default:
			break;
		}
		strTemp.Append(cBuffer);
		nRet   = 1;
		break;

	case 2:		
		if (data == SE) {
			nState = 3;
		}		
		strTemp.Append(cBuffer);
		nRet   = 1;
		break;

	case 3:		
		strTemp.Append(cBuffer);		
		m_ListOptions.AddTail(strTemp);				
		strTemp.Empty(); 
		nState = 0;
		nRet = 1;
		break;

	default:
		ASSERT(0);		
		break;

	}

	return nRet;
}

void CTelnetClient::RespondToOptions()
{
	CString strOption;
	CString strResp;

	while(!m_ListOptions.IsEmpty()) {
		strOption = m_ListOptions.RemoveHead();		
		strResp   = strResp + ArrangeReply(strOption);
		/*
		for (int i = 0; i < strOption.GetLength(); i++) { 
		TRACE ("0x%02X ", strOption.GetAt(i));
		}
		TRACE ("\n");
		*/		
	}

	/*
	TRACE ("RESP: \n");
	for (int i = 0; i < strResp.GetLength(); i++) { 
	TRACE ("0x%02X ", strResp.GetAt(i));
	}
	TRACE ("\n");
	*/		
	USES_CONVERSION;
	LPSTR  pData = T2A(strResp);
	int    nLen = strResp.GetLength();
	int    nSent = Send(pData, nLen); 	
    if (nSent == SOCKET_ERROR) {
		DWORD nError = GetLastError();
		if (nError != WSAEWOULDBLOCK) {
			IoClose ();
			TRACE ("Telnet Send Error = %d  Stage = 0\n", nError);			
		}					
	}	
	m_ListOptions.RemoveAll();
}

void CTelnetClient::OnReceive(int nErrorCode)
{
	// TODO: Add your specialized code here and/or call the base class
	BYTE	rxBuffer[4096];		
	BYTE	*pBuffer = rxBuffer;		
	UINT	nRead;
	int		nStage;
	DWORD   dwError;

	nRead = Receive(rxBuffer, sizeof(rxBuffer)); 

	nStage  = 0;
	dwError = 0;
	switch (nRead) {
	   case 0:		
		   nStage = 0;
		   dwError = GetLastError();
		   IoClose();
		   break;

	   case SOCKET_ERROR:		   
		   dwError = GetLastError();
		   if (dwError != WSAEWOULDBLOCK) {			   
			   nStage = 1;
			   IoClose();
		   }
		   break;

	   default:
		   if (m_bNegociation) {			   
			   for (UINT i=0; i<nRead; i++) {				   
				   if (!HandleOptions((int)rxBuffer[i])) {
					   pBuffer += i;
					   nRead   -= i;
					   m_bNegociation = FALSE;					   
					   RespondToOptions();
					   break;
				   }				   
			   }			   
		   }

		   if (!m_bNegociation) {
			   int  nPut = m_QueueRx.Put(pBuffer, nRead);
			   if (nPut != nRead) {
				   TRACE ("Telnet RX FIFO is full!\n");
			   }
			   if (IsWindow(m_hWndNotify)) {
				   PostMessage (m_hWndNotify, WM_USER, CODE_RECV_FROM_DEVICE, THREAD_ID_TELNET); 				   
			   }
		   }
		   break;
	}

	if (dwError) {
		TRACE ("Telnet Receive Error = %d  Stage = %d\n", dwError, nStage);
	}

	__super::OnReceive(nErrorCode);
}

void CTelnetClient::OnConnect(int nErrorCode)
{
	// TODO: Add your specialized code here and/or call the base class
	if (nErrorCode) {	
		TRACE ("Telnet Connect Error = %d  Stage = 0\n", nErrorCode);	
		CleanUp ();		
	} else {
		m_ConnectState = EnumTypeConnected; 		
		m_hDevice = (HANDLE)m_hSocket;
		TRACE ("Client is connected ! \n");	
	}
	__super::OnConnect(nErrorCode);
}

void CTelnetClient::OnSend(int nErrorCode)
{
	// TODO: Add your specialized code here and/or call the base class
	DWORD   dwByteWrite;
	BYTE	txBuffer[4096];	
	int		nSent;
	int		nError;

	if (m_ConnectState != EnumTypeConnected)
		return;

	dwByteWrite = m_QueueTx.Get(txBuffer, sizeof(txBuffer)-1);					
	if (dwByteWrite) {
		nSent = Send (txBuffer, dwByteWrite);
		if (nSent == SOCKET_ERROR) {
			nError = GetLastError();
			if (nError != WSAEWOULDBLOCK) {
				IoClose ();
				TRACE ("Telnet Send Error = %d  Stage = 1\n", nError);
			}					
		}	
	}

	if (nErrorCode != -1) 
		__super::OnSend(nErrorCode);
}

void CTelnetClient::OnClose(int nErrorCode)
{
	// TODO: Add your specialized code here and/or call the base class
	CleanUp	();
	TRACE ("Telnet Client Disconnected!\n");
	__super::OnClose(nErrorCode);
}
