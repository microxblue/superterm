#include "stdafx.h"
#include "TermLog.h"

UINT CTermLog::RequestHandler(LPVOID pParam)
{
	TRACE ("Connect Thread Start!\n");

	SERVER_THREAD_PARAM  paramThread = *(SERVER_THREAD_PARAM *)pParam;
	delete pParam;

	AfxSocketInit(NULL);
	CAsyncSocket  SrvSocket;		
	SrvSocket.Attach(paramThread.hTelnetSck);
	
	BYTE  rxBuffer[4096];
	BYTE  txBuffer[4096];
	
	BOOL bSleep;
	int  nRead;
	int	 nSent;
	int  nError;
	BOOL bLoop;

	bLoop = TRUE;
	while (bLoop) {		
		// Handle recv
		bSleep = TRUE;
		nRead = SrvSocket.Receive(rxBuffer, sizeof(rxBuffer));
		if (nRead == SOCKET_ERROR)	{			
			nError = SrvSocket.GetLastError();
			if (nError != WSAEWOULDBLOCK) {
				break;
			}					
		} else if (nRead > 0) {			
			paramThread.pQueueRx->Put (rxBuffer, nRead);
			::PostMessage (paramThread.hWnd, WM_USER, CODE_RECV_FROM_NETWORK, THREAD_ID_SERVER);
			bSleep = FALSE;
		} else {
			nError = SrvSocket.GetLastError();
			break;
		}

		nRead = paramThread.pQueueTx->Get (txBuffer, sizeof(txBuffer));
		if (nRead) {
			nSent = SrvSocket.Send(txBuffer, nRead);
			if (nSent == SOCKET_ERROR) {
				nError = SrvSocket.GetLastError();
				if (nError != WSAEWOULDBLOCK) {
					break;
				}					
			} else {
				bSleep = FALSE;
			}
		}
	
		if ( ::WaitForSingleObject(paramThread.hEventQuit, 0) == WAIT_OBJECT_0 ) {
			bLoop = FALSE;
			continue;
		}

		if (bSleep)
			Sleep (15);
	}		
	SrvSocket.Close(); 

	TRACE ("Connect Thread Exit!\n");
	::PostMessage (paramThread.hWnd, WM_USER, CODE_THREAD_EXIT, THREAD_ID_CONNECT); 
	
	return 0;
}


UINT CTermLog::ListenThread(LPVOID pParam)
{	
	TRACE ("Listen Thread Start!\n");	

	SERVER_THREAD_PARAM  paramThread = *(SERVER_THREAD_PARAM *)pParam;
	delete pParam;
	
	AfxSocketInit(NULL);

	CAsyncSocket	Listen;		
	Listen.Create(paramThread.iPort);

	WSAEVENT  evHandles[2];	
	evHandles[1] = paramThread.hEventQuit; 
	evHandles[0] = WSACreateEvent();
	int nRes     = WSAEventSelect(Listen.m_hSocket, evHandles[0],  FD_ACCEPT);			
	DWORD        dwWait = 0;

	if(Listen.Listen())	{					
		CAsyncSocket  tmpSock;
		BOOL		  bSuccess;
		while (1) {
			bSuccess = Listen.Accept(tmpSock);
			if (bSuccess) {
				goto AcceptConnect;
			} else {
				DWORD  dwErr = GetLastError();
				if (dwErr != WSAEWOULDBLOCK) {
					break;
				} else {
					dwWait =  WSAWaitForMultipleEvents (2, evHandles, FALSE, WSA_INFINITE, FALSE);						
					if (dwWait == WSA_WAIT_EVENT_0) {// Accepted
						if (!Listen.Accept(tmpSock)) {
							break;
						}
AcceptConnect:			
						paramThread.hTelnetSck = tmpSock.Detach(); 			
						SERVER_THREAD_PARAM *pNewParam  = new SERVER_THREAD_PARAM;
						*pNewParam = paramThread;
						CWinThread *pThread = AfxBeginThread(RequestHandler, pNewParam);			
						if (!pThread) {
							delete pNewParam;
						}
						break;
					} else { // Exit
						break;
					}					
				}				
				// Wait for accept or quit
			}
		}
	}
	
	WSACloseEvent (evHandles[0]);
	Listen.Close();		

	TRACE ("Listen Thread Exit!\n");	
	::PostMessage (paramThread.hWnd, WM_USER, CODE_THREAD_EXIT, THREAD_ID_LISTEN); 	
	if (dwWait == WSA_WAIT_EVENT_0 + 1) {
		::PostMessage (paramThread.hWnd, WM_USER, CODE_THREAD_EXIT, THREAD_ID_CONNECT); 	
	}
	return 0;
}
