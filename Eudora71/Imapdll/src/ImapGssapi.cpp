// ImapGssapi.cpp: Implementation file for Imap GSSAPI authenticator class.
//
// Copyright (c) 2002 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include "ImapGssapi.h"
#include "rs.h"

#include "resource.h"


#include "DebugNewHelpers.h"

// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


CImapGssapi::CImapGssapi(void *challenger,
						 void *responder,
						 void *s,
						 LPCSTR pHost,
						 LPCSTR pUser)
			  : CGssapi(challenger,
						responder,
						s,
						pHost,
						pUser)
{
	m_szService = GetServiceName();
}


CImapGssapi::~CImapGssapi()
{
}


//
//	CImapGssapi::GetDLLName()
//
//	Returns the name of the GSSAPI DLL file.
//
//	Return:
//		A CString containing the DLL file name.
//
CString CImapGssapi::GetDLLName()
{
	CString			 strDLLName;

	// Get the DLL name from the protocol.  This will handle checking if the
	// user overrode the default name and if no override is provided it will
	// use the default name.
	CProtocol		*pProtocol = (CProtocol*)m_pProtocol;
	if (pProtocol)
	{
		pProtocol->GetGssLibraryName(strDLLName);
	}

	return strDLLName;
}

//
//	CImapGssapi::GetServiceName()
//
//	Returns the service name for this protocol (SMTP).
//
//	Return:
//		A CRString containing the service name.
//
CString CImapGssapi::GetServiceName()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	CString		strServiceName;

	strServiceName.LoadString(IDS_IMAP_SERVICE);

	return strServiceName;
}

//
//	CImapGssapi::GetChallenge()
//
//	Obtains the challenge from the m_pChallenger routine.
//
//	Parameters:
//		lSize[out] - Length of challenge data returned.
//
//	Return:
//		The challenge data or NULL if the read failed.
//
void *CImapGssapi::GetChallenge(unsigned long *lLength)
{
	authchallenge_t			pChallenger = (authchallenge_t)m_pChallenger;
	return pChallenger(m_pProtocol, lLength);
}

//
//	CImapGssapi::SendResponse()
//
//	Sends the specified response to the m_pResponder routine.
//
//	Parameters:
//		szResp[in] - Buffer to send.  If this is NULL the NULL will be passed
//			along causing the authentication to be aborted.  If this is an empty
//			string the appropriate "end of data" string will be passed along.
//			In all other cases this data is simply passed to the server.
//		lSize[in] - Length of data in szResp.
//
//	Return:
//		The return value of the responder routine (TRUE on success, FALSE on failure).
//
BOOL CImapGssapi::SendResponse(char *szResp, unsigned long lSize)
{
	authrespond_t			pResponder = (authrespond_t)m_pResponder;
	return pResponder(m_pProtocol, szResp, lSize);
}

//
//	CImapGssapi::AddLastErrorString()
//
//	Displays to the user and logs the specified error.
//
//	Parameters:
//		szError[in] - Error string to display and log.
//
//	Return:
//		The return value of the responder routine (TRUE on success, FALSE on failure).
//
void CImapGssapi::AddLastErrorString(const char *szString)
{
	CProtocol		*pProtocol = (CProtocol*)m_pProtocol;
	if (pProtocol)
	{
		pProtocol->AddLastErrorString(IMAPERR_LOCAL_ERROR, szString);
	}
	mm_log ((char*)szString,ERROR);
}


//
//	IMAPGssapiAuthenticator()
//
//	Client authenticator routine.  Creates an instance of the appropriate authenticator
//	class and calls its Authenticate() method.
//
//	Parameters:
//		challenger[in] - Challenger function.
//		responder[in] - Responder function.
//		stream[in] - Stream argument for functions.
//		trail[in/out] - Pointer to current trial count.
//		user[in] - User name.
//
//	Returns:
//		T if success, NIL otherwise, number of trials incremented if retry.
//
long IMAPGssapiAuthenticator(authchallenge_t challenger,authrespond_t responder,
			void *stream, unsigned long *trial,
			char *user)
{
	// Sanity:
	if (! (challenger && responder && stream && user) )
	{
		return 0;
	}

	CProtocol	*pProtocol = (CProtocol*)stream;
	CString		 szHostName;
	pProtocol->GetCanonicalHostname(szHostName);

	// Never retry.
	*trial = 0;

	TCHAR tmp[MAILTMPLEN];
	pProtocol->mm_login (user, tmp, *trial);

	// Instantiate a CGssapi to do all the work.
	CImapGssapi		gssAuth(challenger, responder, stream, szHostName, user);

	return gssAuth.Authenticate();
}
