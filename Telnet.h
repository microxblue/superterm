#ifndef         __TELNET_CLIENTCLASS__
#define         __TELNET_CLIENTCLASS__

#include   "DeviceIo.h"

const	unsigned char IAC		= 255;
const	unsigned char DO		= 253; // 0xfd
const	unsigned char DONT		= 254;
const	unsigned char WILL		= 251; // 0xfb
const	unsigned char WONT		= 252;
const	unsigned char SB		= 250;
const	unsigned char SE		= 240;
const	unsigned char IS		= '0';
const	unsigned char SEND		= '1';
const	unsigned char INFO		= '2';
const	unsigned char VAR		= '0';
const	unsigned char VALUE		= '1';
const	unsigned char ESC		= '2';
const	unsigned char USERVAR	= '3';

class CTelnetClient : public CDeviceIo, public CAsyncSocket
{   
	
	#define CONNECT_INTERVAL   2

private :                                                       

	typedef enum {
		EnumTypeIdle,
		EnumTypeConnected,
		EnumTypeConnecting,				
	} CONNECTSTATE;

	int				HandleOptions	(int  data);
	CString			ArrangeReply	(CString strOption);
	void			RespondToOptions();
	void            HandleMessage(BYTE* rxBuffer, int nRead);
	static UINT		CallBackThread	(LPVOID pParam );
	void			CleanUp ();

	int				m_nCount;
	int				m_nPort;
	CString			m_csServer;
	CONNECTSTATE	m_ConnectState;
	BOOL			m_bNegociation;
	CStringList		m_ListOptions;
	HWND			m_hWndNotify;

public :
	CTelnetClient                             ( void );
	~CTelnetClient                            ( void );

	// Public operations            
	BOOL        IoOpen              ( CWnd *pWnd, TCHAR *pCfgStr);
	int			IoWrite    			( BYTE *Buffer , int nNumberOfBytesToWrite );
	int			IoRead    			( BYTE *Buffer , int nNumberOfBytesToRead );
	void		IoClose				( void );
	void		IoReset				( void );
	BOOL        IoIsValid           ( void );

public:
	//virtual void OnReceive(int nErrorCode);
	//virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};

#endif