#ifndef		__DEVICE_IO_CLASS__
#define		__DEVICE_IO_CLASS__

#include   "Queue.h"

class CDeviceIo
{
	#define		MAX_READ_BUFFER_SIZE	  0x400
	#define		MAX_WRITE_BUFFER_SIZE     0x400
		
	#define		CODE_RECV_FROM_DEVICE     0x0000	
	#define		CODE_RECV_FROM_NETWORK    0x0001	
	#define		CODE_THREAD_EXIT		  0x0002
	#define			 THREAD_ID_SERIAL     0x0001
	#define			 THREAD_ID_PIPE       0x0002
	#define			 THREAD_ID_USBBULK    0x0003
	#define			 THREAD_ID_TELNET     0x0004
	#define			 THREAD_ID_SERVER     0x0005
	#define			 THREAD_ID_LISTEN     0x0006
	#define			 THREAD_ID_CONNECT    0x0007
	#define		CODE_CONNECT_FAIL   	  0x00FF

	protected :				
		CWnd			   *m_pWnd;
		CString				m_csName;
		HANDLE				m_hDevice;				
		BOOL                m_bWriteReady;
		CQueue				m_QueueRx;
		CQueue				m_QueueTx;
		HANDLE              m_hEventRead;
		HANDLE              m_hEventWrite;
		HANDLE				m_hEventExit;
		CWinThread		   *m_pThead;

	public :
		typedef struct {
			HWND		hWnd;
			HANDLE		hDevice;
			HANDLE		hEventQuit;
			HANDLE		hEventWrite;
			BOOL		*pWriteReady;
			CQueue		*pQueueRx;
			CQueue		*pQueueTx;
			VOID		*pContext;
		} THREAD_PARAM;

		typedef enum {
			EnumTypeSerialPort = 1,
			EnumTypeNamedPipe,
			EnumTypeUsbBulk,
			EnumTypeTelnet,
		} DEVTYPE;

		DEVTYPE				m_Type;

		CDeviceIo			( void )
		{
			m_hDevice               = INVALID_HANDLE_VALUE; 	    			
			m_hEventRead            = INVALID_HANDLE_VALUE;
			m_hEventWrite           = INVALID_HANDLE_VALUE;
			m_hEventExit            = INVALID_HANDLE_VALUE;    			
			m_bWriteReady           = TRUE;
			m_pWnd					= NULL;
			m_pThead                = NULL;
		};

		~CDeviceIo			( void )
		{
		};

		// Public operations		
		virtual BOOL        IoOpen              ( CWnd *pWnd, TCHAR *pCfgStr) = 0;
		virtual int			IoWrite    			( BYTE *Buffer , int nNumberOfBytesToWrite ) = 0;
		virtual int			IoRead    			( BYTE *Buffer , int nNumberOfBytesToRead ) = 0;
		virtual void		IoClose				( void ) = 0;
		virtual void		IoReset				( void ) = 0;
		virtual BOOL        IoIsValid           ( void ) = 0;
		CString&            IoGetName           ( void ) {return m_csName;}
};

#endif