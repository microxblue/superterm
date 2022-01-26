#include "stdafx.h"
#include "UsbBulk.h"

#define MY_VID 0x04B4
#define MY_PID 0x8613
#define MY_MIF 0x01

#define EP_IN  0x81
#define EP_OUT 0x01

#define BUF_SIZE   64

#define TEST_ASYNC
#define USB_TIMEOUT 100

CUsbBulk::CUsbBulk ( void )
{	
	m_Type  = EnumTypeUsbBulk;
	m_hUsbDevice = NULL;
}

CUsbBulk::~CUsbBulk ( void )
{
	IoClose();	
}

VOID CUsbBulk::ReleaseDevice(USB_HANDLE UsbHandle)
{
    if (UsbHandle) {
      usb_release_interface(UsbHandle, m_nInterface);
      usb_close(UsbHandle);	  
    }
}

USB_DEVICE *CUsbBulk::FindDevice(CString &DeviceName, int Interface)
{
  struct usb_bus *bus;
  USB_DEVICE     *dev;
  USB_DEVICE     *retdev = NULL;  
  CString         FileName;
  int			  pos;

  USES_CONVERSION;
  for(bus = usb_get_busses(); bus; bus = bus->next) {
      for(dev = bus->devices; dev; dev = dev->next) {
		  FileName = A2T(dev->filename);		  	
		  pos = FileName.Find(_T("--"));
	      if (pos >=0 )  
		    FileName = FileName.Mid(pos+2);
		    if (!FileName.Compare(DeviceName)) {
			  if (dev->config->interface->altsetting->bInterfaceNumber == Interface) {
				retdev  = dev;
				break;
			  }
          }
      }
  }  
  return retdev;  
}

BOOL CUsbBulk::IoOpen( CWnd *pWnd, TCHAR *pCfgStr)
{      
	if (m_hDevice != INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	TCHAR	 seps[] = _T(";");
	TCHAR    *ptr;
	TCHAR    *token;
	TCHAR    *argstr;
	int	      argcnt = 0;
	argstr = _tcsdup(pCfgStr);
	token  = _tcstok(argstr, seps );
	while( token != NULL )
	{
		if ((ptr = _tcschr(token, _T('='))) > 0) {
			*ptr = 0;
			if (!_tcsicmp(_T("DeviceName"), token)) {
				m_EndPoint.strDeviceName.Format(_T("%s"), ptr + 1); 	
			    argcnt ++;
			} else if (!_tcsicmp(_T("Interface"), token)) {
				m_EndPoint.nInterface = _tstoi(ptr + 1) & 0xFF;
				argcnt ++;
			} else if (!_tcsicmp(_T("EpIn"), token)) {
				m_EndPoint.nEpIn = _tstoi(ptr + 1) & 0xFF;
				argcnt ++;
			} else if (!_tcsicmp(_T("EpInSize"), token)) {
				m_EndPoint.nEpInSize = _tstoi(ptr + 1);
				if (m_EndPoint.nEpInSize > 1024)
					m_EndPoint.nEpInSize = 1024;
				argcnt ++;
			} else if (!_tcsicmp(_T("EpOut"), token)) {
				m_EndPoint.nEpOut = _tstoi(ptr + 1) & 0xFF;
				argcnt ++;
			} else if (!_tcsicmp(_T("EpOutSize"), token)) {
				m_EndPoint.nEpOutSize = _tstoi(ptr + 1);
				if (m_EndPoint.nEpOutSize > 1024)
					m_EndPoint.nEpOutSize = 1024;
				argcnt ++;
			}
			*ptr = _T('=');
		}
		token  = _tcstok (NULL, seps);
	}
	free (argstr);

	m_csName = m_EndPoint.strDeviceName;
	if (argcnt != 6)
		return FALSE;

	if (m_EndPoint.strDeviceName.GetLength() < 5)
		return FALSE;

	int  error = 0;
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */

	USB_DEVICE *UsbDevcie;
	CString     UsbVid (m_EndPoint.strDeviceName);
	int  pos = UsbVid.Find(_T("--"));
	if (pos >=0 )  
		UsbVid = UsbVid.Mid(pos+2);
    UsbDevcie = FindDevice(UsbVid, m_EndPoint.nInterface);
	if (!UsbDevcie)
		return FALSE;

	USB_HANDLE UsbHandle;
	UsbHandle = usb_open(UsbDevcie);
	if (!UsbHandle) {
		return FALSE;
	}
	m_nInterface =UsbDevcie->config->interface->altsetting->bInterfaceNumber;
	
    if (usb_set_configuration(UsbHandle, 1) < 0) {      
      error  = 6;
      ReleaseDevice (UsbHandle);
	  return FALSE;
    }

    if (usb_claim_interface(UsbHandle, m_nInterface) < 0) {      
      error  = 7;
      ReleaseDevice (UsbHandle);
	  return FALSE;
    }

    int altidx;
	for (altidx = 0 ; altidx < UsbDevcie->config->interface->num_altsetting; altidx++) {
		if (UsbDevcie->config->interface->altsetting[altidx].bNumEndpoints) break;
	}

    if (usb_set_altinterface(UsbHandle, altidx)) {	    
	  error  = 8;
      ReleaseDevice (UsbHandle);
	  return FALSE;
    }

	m_hDevice = (HANDLE)UsbHandle;

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
		pTheadParam->pContext    =  &m_EndPoint;
		m_pThead = AfxBeginThread ( (AFX_THREADPROC)CallBackThread , (void *)pTheadParam);
		if (m_pThead == NULL) {			
			delete pTheadParam;
			IoClose ();
			ASSERT (0);
			return FALSE;
		}			
	} 

	return  TRUE;           
}

VOID CUsbBulk::IoClose ()
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
		ReleaseDevice((USB_HANDLE)m_hDevice);	
		m_hDevice = INVALID_HANDLE_VALUE; 	
	}
}

BOOL CUsbBulk::IoIsValid ()
{
	return (m_hDevice != INVALID_HANDLE_VALUE);
}

int CUsbBulk::IoRead ( BYTE *Buffer , int nNumberOfBytesToRead)
{
	return m_QueueRx.Get((BYTE *)Buffer, nNumberOfBytesToRead);
}

int CUsbBulk::IoWrite ( BYTE *Buffer , int nNumberOfBytesToWrite)
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

void CUsbBulk::IoReset ()
{
	m_QueueRx.Reset();
	m_QueueTx.Reset();
}


/*
* Read/Write using async transfer functions.
*
* NOTE: This function waits for the transfer to complete essentially making
* it a sync transfer function so it only serves as an example of how one might
* implement async transfers into thier own code.
*/
static int transfer_bulk_async(usb_dev_handle *dev,
                               int ep,
                               char *bytes,
                               int size,
                               int timeout)
{
    // Each async transfer requires it's own context. A transfer
    // context can be re-used.  When no longer needed they must be
    // freed with usb_free_async().
    //
    void* async_context = NULL;
    int ret;

    // Setup the async transfer.  This only needs to be done once
    // for multiple submit/reaps. (more below)
    //
    ret = usb_bulk_setup_async(dev, &async_context, ep);
    if (ret < 0)
    {
        printf("error usb_bulk_setup_async:\n%s\n", usb_strerror());
        goto Done;
    }

    // Submit this transfer.  This function returns immediately and the
    // transfer is on it's way to the device.
    //
    ret = usb_submit_async(async_context, bytes, size);
    if (ret < 0)
    {
        printf("error usb_submit_async:\n%s\n", usb_strerror());
        usb_free_async(&async_context);
        goto Done;
    }

    // Wait for the transfer to complete.  If it doesn't complete in the
    // specified time it is cancelled.  see also usb_reap_async_nocancel().
    //
    ret = usb_reap_async(async_context, timeout);

    // Free the context.
    usb_free_async(&async_context);

Done:
    return ret;
}

UINT CUsbBulk::CallBackThread ( LPVOID pParam )
{
	THREAD_PARAM  paramThread = *(THREAD_PARAM *)pParam;	
	delete pParam;	

	ENDPOINT_PARAM *EpParam = (ENDPOINT_PARAM *)paramThread.pContext;

	int        EpIn      = EpParam->nEpIn;
	int        EpInSize  = 64;
	int        EpOut     = EpParam->nEpOut;
	int        EpOutSize = 64;

	int		   nStage  = 0;
	int        dwError = 0;
	DWORD      dwBytesInBuffer;
		
	//evtHandles[0]  = paramThread.hEventQuit;	
	ResetEvent (paramThread.hEventQuit);
	
	USB_HANDLE  UsbHandle = (USB_HANDLE)paramThread.hDevice;
	char        EpInBuf [512];
	char        EpOutBuf[512];

	// Clear the buffer
	// while (usb_bulk_read(UsbHandle, EpIn, EpInBuf, EpInSize, 50) > 0);

	DWORD dwByteWrite;
	int   ret;
    bool  bLoop = TRUE;

	while (bLoop) {

#ifdef TEST_ASYNC
      // Running an async read test
      ret = transfer_bulk_async(UsbHandle, EpIn, EpInBuf, EpInSize, USB_TIMEOUT);
#else
      // Running a sync read test
      ret = usb_bulk_read(UsbHandle, EpIn, EpInBuf, EpInSize, USB_TIMEOUT);	
#endif

	  if (ret >= 0) {
		  // We have data in buffer		  
		  if (ret) {
			  dwBytesInBuffer = ret;
			  paramThread.pQueueRx->Put((BYTE *)EpInBuf, dwBytesInBuffer);			  			  
			  if (IsWindow(paramThread.hWnd)) {
				PostMessage (paramThread.hWnd, WM_USER, CODE_RECV_FROM_DEVICE, THREAD_ID_USBBULK);
			  }	
		  }
	  } else if (ret == -116) {
		dwByteWrite =paramThread.pQueueTx->Get((BYTE *)EpOutBuf, EpOutSize);					
		if (dwByteWrite) {

#ifdef TEST_ASYNC
          // Running an async write test
          ret = transfer_bulk_async(UsbHandle, EpOut, EpOutBuf,dwByteWrite, USB_TIMEOUT);
#else
          // Running a sync write test          
		  ret = usb_bulk_write(UsbHandle, EpOut, EpOutBuf, dwByteWrite, USB_TIMEOUT);
#endif
		  
		  if (ret == -5) {
			  dwError = ret;
			  break;
		  }
		} else {
		  Sleep (100);
		}
	  }	else {
		dwError = ret;
	    break;
	  }  
      if ( ::WaitForSingleObject(paramThread.hEventQuit, 0) == WAIT_OBJECT_0 ) {
		bLoop = FALSE;
		continue;
	  }
	}

	TRACE ("UsbBulk Polling Thread Exit! Error = %d  Stage = %d\n", dwError, nStage);
	
	if (IsWindow(paramThread.hWnd)) {
		PostMessage (paramThread.hWnd, WM_USER, CODE_THREAD_EXIT,  THREAD_ID_USBBULK);
	}	

	return 0;
}
