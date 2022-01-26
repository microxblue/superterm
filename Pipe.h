#ifndef		__PIPE_CLASS__
#define		__PIPE_CLASS__


#include   "DeviceIo.h"

class CPipe : public CDeviceIo
{   
private :							
	static UINT		    CallBackThread		(LPVOID pParam );        

public :
	CPipe				( void );
	~CPipe				( void );

	// Public operations		
	BOOL        IoOpen              ( CWnd *pWnd, TCHAR *pCfgStr);
	int			IoWrite    			( BYTE *Buffer , int nNumberOfBytesToWrite );
	int			IoRead    			( BYTE *Buffer , int nNumberOfBytesToRead );
	void		IoClose				( void );
	void		IoReset				( void );
	BOOL        IoIsValid           ( void );

};

#endif