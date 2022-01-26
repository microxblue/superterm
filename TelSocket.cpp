// CTelnetSocket.cpp : implementation file
//

#include "stdafx.h"
#include "TelSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTelnetSocket

CTelnetSocket::CTelnetSocket()
{	
}

CTelnetSocket::~CTelnetSocket()
{
}

void	CTelnetSocket::OnReceive(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	/*
	BYTE	RecvBuffer[1024];		
	BYTE	*RecvPtr = RecvBuffer;
	INT		BytesToHandle;

	BytesToHandle = Receive(RecvPtr,sizeof(RecvBuffer));

	if (m_NegociationInProgress) {
		if (BytesToHandle) {
			BytesHandled   = HandleTelnet (RecvBuffer, BytesToHandle);
			BytesToHandle -= BytesHandled;
		}
	}

	if (BytesToHandle) {
		if (BytesToHandle != m_quFifo.Put (pRecvBuffer,  BytesToHandle)) {
			ASSERT ();
		}
		if (IsQuadFull || IsTimeout) {
			::PostMessage (WM_0);
		}
	}
	*/
	CSocket::OnReceive(nErrorCode);
}

void CTelnetSocket::OnClose(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	CSocket::OnClose(nErrorCode);
}

/*
void CTelnetSocket::YesReply(BYTE verb, BYTE option)
{
  BYTE   buf[3];
  buf[0] = IAC;
  buf[1] = (verb==DO)?WILL:(verb==DONT)?WONT:(verb==WILL)?DO:DONT;
  buf[2] = option;
  send (buf,3);
}

void CTelnetSocket::NoReply(BYTE verb,BYTE option)
{
  BYTE buf[3];
  buf[0] = IAC;
  buf[1] = (verb==DO)?WONT:(verb==DONT)?WILL:(verb==WILL)?DONT:DO;
  buf[2] = option;
  send (buf,3);
}

void CTelnetSocket::AskFor(BYTE verb,BYTE option)
{
  BYTE buf[3];
  buf[0] = IAC;
  buf[1] = verb;
  buf[2] = option;
  send (buf,3);
}
*/