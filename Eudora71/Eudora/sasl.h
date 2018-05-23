// sasl.h - Class representing the SASL machanism
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

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
	int GetAuthIndex(LPCSTR pName);


private:
	// Keep a list of authenticators (CAuthenticator objects) that both the client and server can
	// handle.
	// Note: These are pointers to static functions so don't attempt to free the 
	// contained objects.
	//
	CPtrList m_authenticators;

};
