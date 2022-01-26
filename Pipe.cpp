#include "stdafx.h"
#include "Pipe.h"

CPipe::CPipe ( void )
{	
	m_Type  = EnumTypeNamedPipe;
}

CPipe::~CPipe ( void )
{
	IoClose();	
}

BOOL CPipe::IoOpen( CWnd *pWnd, TCHAR *pCfgStr)
{      
	CString  PipeName;
	TCHAR	 seps[] = _T(";");
	TCHAR    *ptr;
	TCHAR    *token;
	TCHAR    *argstr;

	if (m_hDevice != INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	/* Establish string and get the first token: */		
	argstr = _tcsdup(pCfgStr);
	token  = _tcstok(argstr, seps );
	while( token != NULL )
	{
		if ((ptr = _tcschr(token, _T('='))) > 0) {
			*ptr = 0;
			if (!_tcsicmp(_T("PipeName"), token))
				PipeName.Format(_T("%s"), ptr + 1); 			
			*ptr = _T('=');
		}
		token  = _tcstok (NULL, seps);
	}
	free (argstr);

	if (PipeName.IsEmpty()) {
		return FALSE;
	}

	m_csName  = PipeName;                     
	m_hDevice = CreateFile (  
		PipeName, 
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,
		NULL
		);    

	if (m_hDevice != INVALID_HANDLE_VALUE) {         
		
		m_hEventWrite       = CreateEvent(NULL, TRUE,  FALSE, NULL);
		m_hEventExit        = CreateEvent(NULL, TRUE,  FALSE, NULL);

		IoReset (); 

		// Start a new thread to monitor the pipe
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

VOID CPipe::IoClose ()
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

BOOL CPipe::IoIsValid ()
{
	return (m_hDevice != INVALID_HANDLE_VALUE);
}

int CPipe::IoRead ( BYTE *Buffer , int nNumberOfBytesToRead)
{
	return m_QueueRx.Get((BYTE *)Buffer, nNumberOfBytesToRead);
}

int CPipe::IoWrite ( BYTE *Buffer , int nNumberOfBytesToWrite)
{
	int	nLen;	

	nLen = (int)m_QueueTx.Put((BYTE *)Buffer, nNumberOfBytesToWrite);
	if (nLen != nNumberOfBytesToWrite) {
		TRACE ("Pipe TX FIFO is full !\n");
	}

	if (m_bWriteReady)
		SetEvent (m_hEventWrite);

	return nLen;      
}

void CPipe::IoReset ()
{
	m_QueueRx.Reset();
	m_QueueTx.Reset();
}

UINT CPipe::CallBackThread ( LPVOID pParam )
{
	THREAD_PARAM  paramThread = *(THREAD_PARAM *)pParam;	
	delete pParam;	

	int		   nStage;
	DWORD      dwError;
	DWORD      dwReadCnt;
	DWORD      dwWriteCnt;
	DWORD      dwBytesInBuffer;
	
	BYTE       rxBuffer[MAX_READ_BUFFER_SIZE];
	BYTE       txBuffer[MAX_WRITE_BUFFER_SIZE];

	HANDLE	   hEventRead  = CreateEvent(NULL, TRUE,  FALSE, NULL);
	HANDLE	   hEventWrite = paramThread.hEventWrite;
	HANDLE     evtHandles[3]; 
	OVERLAPPED ovRead     = {0};	
	OVERLAPPED ovWrite    = {0};

	evtHandles[0]  = paramThread.hEventQuit;
	evtHandles[1]  = hEventRead;	
	evtHandles[2]  = hEventWrite;

	ovWrite.hEvent = hEventWrite;						
	ovRead.hEvent  = hEventRead;

	ResetEvent (paramThread.hEventQuit);
	
	if (!ReadFile (paramThread.hDevice, rxBuffer, 1, &dwReadCnt, &ovRead)) {
		dwError= GetLastError ();			
		if (dwError != ERROR_IO_PENDING) {	
			nStage = 0;
		}
	}
	
	DWORD   dwWait;		
	BOOL	bContinue = TRUE;    
	while (bContinue) {		
		dwWait = WaitForMultipleObjects (3, evtHandles, FALSE, INFINITE);
		switch (dwWait)
		{
		case WAIT_OBJECT_0:	
			dwError = 0;
			nStage  = 1;
			bContinue = FALSE;
			break;

		case WAIT_OBJECT_0 + 1: // Has data to read
			{
				DWORD  dwReadResult;
				DWORD  dwBytesToRead;
				// Wait the read to complete
				if (!GetOverlappedResult(paramThread.hDevice, &ovRead, &dwReadResult, FALSE)) { 								
					dwError = GetLastError();
					nStage = 2;
					goto ErrorCode;
				}
				ResetEvent(hEventWrite);
				paramThread.pQueueRx->Put(rxBuffer, dwReadResult);						

				if (!PeekNamedPipe(paramThread.hDevice, NULL, 0, NULL, &dwBytesInBuffer, NULL)) {
					dwError = GetLastError();
					nStage = 3;
					goto ErrorCode;
				}

				while (dwBytesInBuffer > 0) {
					if (dwBytesInBuffer > sizeof(rxBuffer) - 1) {
						dwBytesToRead = sizeof(rxBuffer) - 1;
					} else {
						dwBytesToRead = dwBytesInBuffer;
					}
					// Issue read
					if (!ReadFile(paramThread.hDevice, rxBuffer, dwBytesToRead, &dwReadCnt, &ovRead)) {
						dwError = GetLastError();
						if (dwError != ERROR_IO_PENDING) {							
							nStage = 4;
							goto ErrorCode;
						}
					}				
					// Wait the read to complete
					if (!GetOverlappedResult(paramThread.hDevice, &ovRead, &dwReadResult, TRUE)) { 								
						dwError = GetLastError();
						nStage = 5;
						goto ErrorCode;					
					} else {
						ResetEvent(hEventRead);
						paramThread.pQueueRx->Put(rxBuffer, dwReadResult);
					}							
					dwBytesInBuffer -= dwBytesToRead;
				} 		

				if (IsWindow(paramThread.hWnd)) {
					PostMessage (paramThread.hWnd, WM_USER, CODE_RECV_FROM_DEVICE, THREAD_ID_PIPE); 						
				}	

				if (!ReadFile (paramThread.hDevice, rxBuffer, 1, &dwReadCnt, &ovRead)) {
					dwError = GetLastError();
					if (dwError != ERROR_IO_PENDING) {							
						nStage = 6;
						goto ErrorCode;
					}						
				}
			}				
			break;

		case WAIT_OBJECT_0 + 2: // Has data to write
			{
				DWORD   dwByteWrite;
				ResetEvent(hEventWrite);
				dwByteWrite =paramThread.pQueueTx->Get(txBuffer, sizeof(txBuffer)-1);					
				if (dwByteWrite) {
					if (!WriteFile ( paramThread.hDevice , txBuffer , dwByteWrite , &dwWriteCnt , &ovWrite)) {
						dwError = GetLastError();
						if (dwError != ERROR_IO_PENDING) {							
							nStage = 7;
							goto ErrorCode;
						}	
					}
					*(paramThread.pWriteReady) = FALSE;
				} else {
					*(paramThread.pWriteReady) = TRUE;
				}
			}
			break;

		default:
ErrorCode:
			bContinue = FALSE;		
			break;			
		}
	}

	TRACE ("Pipe Polling Thread Exit! Error = %d  Stage = %d\n", dwError, nStage);

	::CancelIo(paramThread.hDevice);

	CloseHandle  (hEventRead);	

	if (IsWindow(paramThread.hWnd)) {
		PostMessage (paramThread.hWnd, WM_USER, CODE_THREAD_EXIT,  THREAD_ID_PIPE);
	}	

	return 0;
}
