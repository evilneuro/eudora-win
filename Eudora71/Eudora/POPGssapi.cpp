// POPGssapi.cpp: Implementation file for POP GSSAPI authenticator class.
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

#include "POPSession.h"
#include "sasl.h"

#include "POPGssapi.h"

#include "rs.h"
#include "resource.h"

#include "guiutils.h"

// Include the files in the correct order to allow leak checking with malloc.
// CRTDBG_MAP_ALLOC already defined in stdafx.h
#include <stdlib.h>
#include <crtdbg.h>

#include "DebugNewHelpers.h"

// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


CPOPGssapi::CPOPGssapi (void *challenger,
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


CPOPGssapi::~CPOPGssapi()
{
}

//
//	CPOPGssapi::GetDLLName()
//
//	Returns the name of the GSSAPI DLL file.
//
//	Return:
//		A CString containing the DLL file name.
//
CString CPOPGssapi::GetDLLName()
{
	CString			 strDLLName;

	if (m_pProtocol)
	{
		CPOPSession			*pPOPSession = (CPOPSession*)m_pProtocol;
		strDLLName = pPOPSession->GetGSSLibraryName();
	}

	return strDLLName;
}


//
//	CPOPGssapi::GetServiceName()
//
//	Returns the service name for this protocol (POP).
//
//	Return:
//		A CString containing the service name.
//
CString CPOPGssapi::GetServiceName()
{
	CString			 strServiceName;

	strServiceName.LoadString(IDS_POP_SERVICE_A);

	return strServiceName;
}

//
//	CPOPGssapi::GetChallenge()
//
//	Obtains the challenge from the m_pChallenger routine.
//
//	Parameters:
//		lSize[out] - Length of challenge data returned.
//
//	Return:
//		The challenge data or NULL if the read failed.
//
void *CPOPGssapi::GetChallenge(unsigned long *lLength)
{
	*lLength = MAILTMPLEN;
	char			*szOut = (char*)malloc(*lLength);
	if (szOut)
	{
		memset(szOut, 0, *lLength);

		PAuthChallenger		pChallenger = (PAuthChallenger)m_pChallenger;
		*lLength = pChallenger(m_pProtocol, szOut, *lLength);
		if (*lLength == 0)
		{
			free(szOut);
			szOut = NULL;
		}
	}
	return szOut;
}

//
//	CPOPGssapi::SendResponse()
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
BOOL CPOPGssapi::SendResponse(char *szResp, unsigned long lSize)
{
	PAuthResponder			pResponder = (PAuthResponder)m_pResponder;
	return pResponder(m_pProtocol, szResp, lSize);
}

//
//	CPOPGssapi::AddLastErrorString()
//
//	Displays to the user and logs the specified error.
//
//	Parameters:
//		szError[in] - Error string to display and log.
//
void CPOPGssapi::AddLastErrorString(const char *szError)
{
	if (m_pProtocol)
	{
		CPOPSession			*pPOPSession = (CPOPSession*)m_pProtocol;
		CTaskInfoMT			*pInfo = pPOPSession->GetTaskInfo();
		if (pInfo)
		{
			pInfo->CreateError(szError, TERR_POPAUTH);
		}
	}
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
//		user[in] - User name.
//		szHost[in] - Host name.
//
//	Returns:
//		T if success, NIL otherwise, number of trials incremented if retry.
//
long POPGssapiAuthenticator(void *challenger,
							void *responder,
							void *s,
							char *szUser,
							CString szHost)
{
	// Sanity:
	if (! (challenger && responder && s && szUser && !szHost.IsEmpty()) )
	{
		return 0;
	}

	// Instantiate a CGssapi to do all the work.
	CPOPGssapi		gssAuth(challenger, responder, s, szHost, szUser);

	return gssAuth.Authenticate();
}
