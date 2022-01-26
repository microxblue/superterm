#pragma once

class CTelnetSocket : public CSocket
{
// Attributes
public:

// Operations
public:
	CTelnetSocket();
	virtual ~CTelnetSocket();

private:

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TelSocket)
	public:
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(TelSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};

