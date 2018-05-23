// sasl.h - Class representing the SASL machanism
//


// Supply a buffer in which to place the server's challenge, and
// the size of the buffer (bufsize).
// Return the length of the data in "outbuf".
//
typedef int (*PAuthChallenger) (void *protocol, char *outbuf, int bufsize);


// Pass the mechanism's response bac to the server in "response". "size" is
// the length of data in "response".
//
typedef BOOL (*PAuthResponder)   (void *protocol, char *response, unsigned long size);


typedef BOOL (*PAuthClient) (PAuthChallenger pChallenger,
			      PAuthResponder pResponder, void *pStream, const char* pUser);


// This class represents a single authentication mechanism.
//
class CAuthenticator : public CObject
{
public:
	CAuthenticator(LPCSTR pName, PAuthClient pC);
	virtual ~CAuthenticator();

	LPCSTR GetAuthenticatorType()
		{ return (LPCSTR)szName; }
	PAuthClient GetAuthClient ()
		{ return pClient; }

private:

	// Name of the authentication mechanism.
	CString szName;

	// The client authentication routine. This is called after the
	// first authenticate response is sent.
	//
	PAuthClient pClient;
};



class CSasl
{

public:

	CSasl();
	~CSasl();

	void ParseAuthenticators (LPCSTR pszBuf);

	// Iterator through the list of authenticators. 
	// Iterates from the strongest mechanism down to the weakest.
	//
	CAuthenticator* GetNextAuthenticator(PAuthClient pClient = NULL);

private:
	void CSasl::DeleteList ();

	// If "pName" represents and authenticator that we can handle, add it to
	// our list.
	// 
	BOOL AddAuthenticator (LPCSTR pName);


private:
	// Keep a list of authenticators (CAuthenticator objects) that both the client and server can
	// handle.
	// Note: These are pointers to static functions so don't attempt to free the 
	// contained objects.
	//
	CPtrList m_authenticators;

};