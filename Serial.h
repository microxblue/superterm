#ifndef		__SERIAL_CLASS__
#define		__SERIAL_CLASS__

#include   "DeviceIo.h"

class CSerial : public CDeviceIo
{   
private :					
	HANDLE              m_hEventWait;				

	BOOL				EnableDtrRts(int EnableDtr, int EnableRts);
	BOOL				SetBaudRate			(DWORD Baud);
	static UINT		    CallBackThread		(LPVOID pParam );

public :
	CSerial				( void );
	~CSerial			( void );

	// Public operations		
	BOOL        IoOpen              ( CWnd *pWnd, TCHAR *pCfgStr);
	int			IoWrite    			( BYTE *Buffer , int nNumberOfBytesToWrite );
	int			IoRead    			( BYTE *Buffer , int nNumberOfBytesToRead );
	void		IoClose				( void );
	void		IoReset				( void );
	BOOL        IoIsValid           ( void );

};

#endif