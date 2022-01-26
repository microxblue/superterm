#include "stdafx.h"
#include "Queue.h"
#include "Serial.h"
#include ".\serial.h"

CSerial::CSerial ( void )
{    
	m_Type  = EnumTypeSerialPort;
}

CSerial::~CSerial ( void )
{
	IoClose();
}

BOOL CSerial::EnableDtrRts (int EnableDtr, int EnableRts)
{
	DCB dcb;
	if (!::GetCommState(m_hDevice, &dcb)) {
		return FALSE;
	}
	if (EnableDtr) {
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
	}
	else {
		dcb.fDtrControl = DTR_CONTROL_DISABLE;
	}
	if (EnableRts) {
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
	}
	else {
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
	}
	if (!::SetCommState(m_hDevice, &dcb)) {
		return FALSE;
	}
	return TRUE;
}

BOOL CSerial::SetBaudRate (DWORD Baud)
{
	DCB dcb;

	memset(&dcb,0,sizeof(dcb));
	dcb.DCBlength=sizeof(dcb);
	if (!::GetCommState(m_hDevice,&dcb))
		return FALSE;
	dcb.BaudRate=Baud;
	dcb.ByteSize=8;
	dcb.Parity=0;	
	dcb.StopBits=ONESTOPBIT;	
	if (!::SetCommState(m_hDevice,&dcb))
		return FALSE;

	COMMTIMEOUTS commTimeout;				
	if(!::GetCommTimeouts(m_hDevice, &commTimeout)) /* Configuring Read & Write Time Outs */
		return FALSE;

	commTimeout.ReadIntervalTimeout = MAXWORD;
	commTimeout.ReadTotalTimeoutConstant =  0;
	commTimeout.ReadTotalTimeoutMultiplier = 0;
	commTimeout.WriteTotalTimeoutConstant = 0;
	commTimeout.WriteTotalTimeoutMultiplier = 0;

	if(!::SetCommTimeouts(m_hDevice, &commTimeout))
		return FALSE;

	if(!::SetupComm(m_hDevice, 8192, 8192))
		return FALSE;
		
	if(!::SetCommMask(m_hDevice, EV_RXCHAR | EV_ERR /* EV_TXEMPTY */)) /* Setting Event Type */
		return FALSE;

	return TRUE;
}


BOOL CSerial::IoOpen( CWnd *pWnd, TCHAR *pCfgStr)
{          
	DWORD    BaudRate;
	int		 BaudRateValue;
	TCHAR	 seps[] = _T(";");
	TCHAR    *ptr;
	TCHAR    *token;
	TCHAR    *argstr;

	if (m_hDevice != INVALID_HANDLE_VALUE) {
		return FALSE;
	}	

	int Dtr, Rts;

	Dtr = 0;
	Rts = 0;
	/* Establish string and get the first token: */		
	argstr = _tcsdup(pCfgStr);
	token  = _tcstok(argstr, seps );
	while( token != NULL )
	{
		if ((ptr = _tcschr(token, _T('='))) > 0) {
			*ptr = 0;
			if (!_tcsicmp(_T("Device"), token))
				m_csName.Format(_T("\\\\.\\%s"), ptr + 1); 
			else if (!_tcsicmp(_T("Dtr"), token))
				Dtr = _ttoi(ptr + 1);
			else if (!_tcsicmp(_T("Rts"), token))
				Rts = _ttoi(ptr + 1);
			else if (!_tcsicmp(_T("BaudRate"), token))
				BaudRateValue = _ttoi (ptr + 1);
			*ptr = _T('=');
		}
		token  = _tcstok (NULL, seps);
	}
	free (argstr);

	if (m_csName.IsEmpty()) {
		return FALSE;
	}

	m_hDevice = CreateFile (  
		m_csName, 
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED,
		NULL
		);  

	if (m_hDevice != INVALID_HANDLE_VALUE) {      
		::PurgeComm(m_hDevice, PURGE_RXABORT|PURGE_RXABORT|PURGE_TXABORT|PURGE_TXCLEAR);
		::ClearCommError(m_hDevice, NULL, NULL);

		m_hEventExit        = CreateEvent(NULL, TRUE,  FALSE, NULL);
		m_hEventWrite       = CreateEvent(NULL, TRUE,  FALSE, NULL);

		// 300;600;900;1200;2400;4800;9600;19200;38400;57600;115200;230440;460800;921600
		switch (BaudRateValue) {
			case 300:
				BaudRate = CBR_300;
        break;
      case 600:
				BaudRate = CBR_600;
        break;
      case 1200:
				BaudRate = CBR_1200;
        break;
      case 2400:
				BaudRate = CBR_2400;
        break;
      case 4800:
				BaudRate = CBR_4800;      
        break;
      case 9600:
				BaudRate = CBR_9600;
				break;
			case 19200:
				BaudRate = CBR_19200;
				break;
			case 38400:
				BaudRate = CBR_38400;
				break;
			case 57600:
				BaudRate = CBR_57600;
				break;
			case 115200:
				BaudRate = CBR_115200;
				break;
			case 230400:
				BaudRate = 230400;
        break;
			case 460800:
				BaudRate = 460800;
        break;
			case 921600:
				BaudRate = 921600;
        break;
                        default:
        BaudRate = BaudRateValue;
				break;
		}

		if (SetBaudRate(BaudRate)) {
			EnableDtrRts (Dtr, Rts);
		}

		IoReset (); 

		// Start a thread for data polling
		THREAD_PARAM *pTheadParam = new THREAD_PARAM;
		memset (pTheadParam, 0, sizeof(THREAD_PARAM));
		pTheadParam->hDevice     =  m_hDevice;
		pTheadParam->hWnd        =  pWnd->GetSafeHwnd(); 
		pTheadParam->hEventQuit  =  m_hEventExit;
		pTheadParam->hEventWrite =  m_hEventWrite;
		pTheadParam->pQueueRx    =  &m_QueueRx; 
		pTheadParam->pQueueTx    =  &m_QueueTx; 
		pTheadParam->pWriteReady =  &m_bWriteReady;
		m_pThead = AfxBeginThread ( (AFX_THREADPROC)CallBackThread , (void *)pTheadParam);
		if (m_pThead == NULL) {			
			delete pTheadParam;
			IoClose ();
			ASSERT (0);
			return FALSE;
		}		
	} 	
	return  FALSE;           
}


VOID CSerial::IoClose ()
{    
	// Wait for quit
	if (m_hEventExit != INVALID_HANDLE_VALUE) {
		if (m_pThead) {
			DWORD dwExitCode = 0;
			GetExitCodeThread(m_pThead->m_hThread, &dwExitCode);
			if (dwExitCode == STILL_ACTIVE) {
				SetEvent (m_hEventExit);
				Sleep (100);
			}			
		}		 		
		CloseHandle  (m_hEventExit);
		m_hEventExit = INVALID_HANDLE_VALUE;
	}

	if (m_hEventWrite != INVALID_HANDLE_VALUE) {
		CloseHandle  (m_hEventWrite);
		m_hEventWrite = INVALID_HANDLE_VALUE;
	}

	if (m_hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle  (m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE; 	
	}
}

BOOL CSerial::IoIsValid ()
{
	return (m_hDevice != INVALID_HANDLE_VALUE);
}

int CSerial::IoRead ( BYTE *Buffer , int nNumberOfBytesToRead)
{
	return m_QueueRx.Get((BYTE *)Buffer, nNumberOfBytesToRead);
}

int CSerial::IoWrite ( BYTE *Buffer , int nNumberOfBytesToWrite)
{
	int	nLen;	

	nLen = (int)m_QueueTx.Put((BYTE *)Buffer, nNumberOfBytesToWrite);
	if (nLen != nNumberOfBytesToWrite) {
		TRACE ("Serial TX FIFO is full !\n");
	}

	if (m_bWriteReady)
		SetEvent (m_hEventWrite);

	return nLen;    
}

void CSerial::IoReset ()
{
	m_QueueRx.Reset();
	m_QueueTx.Reset();
}


UINT CSerial::CallBackThread ( LPVOID pParam )
{
	THREAD_PARAM  paramThread = *(THREAD_PARAM *)pParam;	
	delete pParam;		

	DWORD      dwReadCnt;
	DWORD      dwWriteCnt;

	HANDLE hEventWait  = CreateEvent(NULL, TRUE,  FALSE, NULL);
	HANDLE hEventRead  = CreateEvent(NULL, TRUE,  FALSE, NULL);
	HANDLE hEventWrite = paramThread.hEventWrite;
	HANDLE     evtHandles[3]; 
	OVERLAPPED ovRead     = {0};
	OVERLAPPED ovWrite    = {0};
	OVERLAPPED ovWait     = {0};

	evtHandles[0]  = paramThread.hEventQuit;
	evtHandles[1]  = hEventWait;	
	evtHandles[2]  = hEventWrite;

	ovWrite.hEvent = hEventWrite;						
	ovRead.hEvent  = hEventRead;
	ovWait.hEvent  = hEventWait;

	ResetEvent (paramThread.hEventQuit);	

	// Keep looping	
	int		nStage;
	DWORD   dwError;
	DWORD   dwWait;
	DWORD   dwEventMask;
	BOOL    bCallWait = TRUE;
	BOOL	bContinue = TRUE;    
	while  (bContinue) {
		if (bCallWait)  {
			if (!WaitCommEvent(paramThread.hDevice, &dwEventMask, &ovWait))	{					
				dwError= GetLastError ();			
				if (dwError != ERROR_IO_PENDING) {	
					nStage = 0;
					break;
				}
			}
			bCallWait = FALSE;
		}

		dwWait = WaitForMultipleObjects (3, evtHandles, FALSE, INFINITE);
		switch (dwWait)
		{
		case WAIT_OBJECT_0:	// Quit thread
			dwError = 0;
			nStage  = 1;
			goto Quit;
			break;

		case WAIT_OBJECT_0 + 1: // Have data to read			
			if (dwEventMask & EV_RXCHAR) {	
				BYTE    rxBuffer[MAX_READ_BUFFER_SIZE];
				COMSTAT comStat; 
				DWORD   dwBytesInBuffer;
				DWORD   dwBytesToRead;
				DWORD   dwErrorFlags;
				DWORD   dwReadResult;
				ClearCommError(paramThread.hDevice, &dwErrorFlags, &comStat) ;
				dwBytesInBuffer = comStat.cbInQue;
				while (dwBytesInBuffer > 0) {
					if (dwBytesInBuffer > sizeof(rxBuffer) - 1) {
						dwBytesToRead = sizeof(rxBuffer) - 1;
					} else {
						dwBytesToRead = dwBytesInBuffer;
					}
					// Issue read
					if (!ReadFile(paramThread.hDevice, rxBuffer, dwBytesToRead, &dwReadResult, &ovRead)) {
						dwError = GetLastError();
						if (dwError != ERROR_IO_PENDING) {	
							nStage = 2;
							goto Quit;
						}
					}				
					// Wait the read to complete
					if (!GetOverlappedResult(paramThread.hDevice, &ovRead, &dwReadCnt, TRUE)) { 								
						dwError = GetLastError();
						nStage = 3;
						goto Quit;
					} else {
						ResetEvent(hEventRead);
						paramThread.pQueueRx->Put(rxBuffer, dwReadCnt);							
					}							
					dwBytesInBuffer -= dwBytesToRead;
				}	
				if (IsWindow(paramThread.hWnd)) {
					PostMessage (paramThread.hWnd, WM_USER, CODE_RECV_FROM_DEVICE, THREAD_ID_SERIAL); 						
				}									
			}	
			if (dwEventMask & EV_ERR) {
				DWORD  Error;				
				::ClearCommError(paramThread.hDevice, &Error, NULL);
				TRACE ("Detected error! 0x%08X\n", Error);
			}
			bCallWait = TRUE;
			break;

		case WAIT_OBJECT_0 + 2: // Have data to write
			{
				DWORD   dwByteWrite;
				BYTE    txBuffer[MAX_WRITE_BUFFER_SIZE];
				ResetEvent(hEventWrite);
				dwByteWrite = paramThread.pQueueTx->Get(txBuffer, sizeof(txBuffer));					
				if (dwByteWrite) {
					if (!WriteFile (paramThread.hDevice , txBuffer , dwByteWrite , &dwWriteCnt , &ovWrite)) {
						dwError = GetLastError();
						if (dwError != ERROR_IO_PENDING) {								
							nStage = 4;
							goto Quit;
						}						
					}
					*(paramThread.pWriteReady) = FALSE;
				} else {
					*(paramThread.pWriteReady) = TRUE;
				}
			}
			break;

Quit:
		default:		   				
			bContinue = FALSE;			
			break;
		}

	}

	TRACE ("Serial Polling Thread Exit! Error = %d  Stage = %d\n", dwError, nStage);

	::CancelIo(paramThread.hDevice);	

	CloseHandle  (hEventRead);
	CloseHandle  (hEventWait);	

	if (IsWindow(paramThread.hWnd)) {
		PostMessage (paramThread.hWnd, WM_USER, CODE_THREAD_EXIT,  THREAD_ID_SERIAL);
	}	

	return 0;
}

