#ifndef		__USB_BULK_CLASS__
#define		__USB_BULK_CLASS__

#include   "DeviceIo.h"
#include   "lusb0_usb.h"

typedef struct {
	CString                  strDeviceName;
	int						 nInterface;
	int                      nEpIn;
	int		                 nEpInSize;	
	int		                 nEpOut;
	int		                 nEpOutSize;
} ENDPOINT_PARAM;

class CUsbBulk : public CDeviceIo
{   
    #define   USB_DEVICE  struct usb_device
	#define   USB_HANDLE  struct usb_dev_handle *
private :		
	struct usb_dev_handle   *m_hUsbDevice;

	ENDPOINT_PARAM           m_EndPoint;
	BYTE                     m_nInterface;

	static UINT		    CallBackThread		(LPVOID pParam );        
	VOID                ReleaseDevice(USB_HANDLE UsbHandle); 
	USB_DEVICE         *FindDevice(CString &DeviceName, int Interface);

public :
	CUsbBulk				( void );
	~CUsbBulk				( void );

	// Public operations		
	BOOL        IoOpen              ( CWnd *pWnd, TCHAR *pCfgStr);
	int			IoWrite    			( BYTE *Buffer , int nNumberOfBytesToWrite );
	int			IoRead    			( BYTE *Buffer , int nNumberOfBytesToRead );
	void		IoClose				( void );
	void		IoReset				( void );
	BOOL        IoIsValid           ( void );

};

#endif