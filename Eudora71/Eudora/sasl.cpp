// sasl.cpp - Implement the SASL wrapper.
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

#include "stdafx.h"

#include <qcutils.h>
#include <afxmt.h>

#include "sasl.h"
#include "SmtpAuth.h"


#include "DebugNewHelpers.h"


#define IsWhite(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')



// Note: If no authenticators, set this to {NULL}.
typedef struct
{
	// Name of the authentication mechanism.
	LPCSTR pName;

	PAuthClient pClient;
} MailAuthenticator;

static MailAuthenticator mailAuthenticators[] = {
							{"GSSAPI",		GSSAPIAuthenticator},
							{"CRAM-MD5",	CramMD5Authenticator},
							{"LOGIN",		LoginAuthenticator},
							{"PLAIN",		PlainAuthenticator},
						    NULL
						  };




//=================== Implementation of CAuthenticator ========================//

CAuthenticator::CAuthenticator(LPCSTR pName, PAuthClient pC)
	: szName (pName), pClient (pC)
{
}




CAuthenticator::~CAuthenticator()
{

}





//==================== Implementation of CSasl ===========================//

CSasl::CSasl()
{
}


CSasl::~CSasl()
{
	DeleteList();
}



void CSasl::ParseAuthenticators (LPCSTR pszBuf)
{
	LPCSTR start = pszBuf;
	LPCSTR end;
	CString szAuthType;

	while (start)
	{
		for (end = start; *end && IsWhite(*end); end++);

		start = end;
		for (end = start; *end && !IsWhite(*end); end++);

		if (end <= start)
		{
			start = NULL;
		}
		else
		{
			CString szAuthType (start, end - start);
			AddAuthenticator (szAuthType);

			start = end;
		}
	}
}

// If the given name is an authentication scheme we understand, return the
// index of it in the list.  If not found (or entry is bad), return -1.
//
int CSasl::GetAuthIndex(LPCSTR pName)
{
	for (int i = 0; mailAuthenticators[i].pName; i++)
	{
		if (stricmp(mailAuthenticators[i].pName, pName) == 0)
		{
			if (mailAuthenticators[i].pClient)
				return i;

			// How'w we get an entry with no code to run?
			ASSERT(0);
			return -1;
		}
	}

	return -1;
}

// If the given authenticator type is one we can handle, add it to our internal list.
//
BOOL CSasl::AddAuthenticator (LPCSTR pName)
{
	if (!pName)
		return FALSE;

	int NewIndex = GetAuthIndex(pName);
	if (NewIndex < 0)
		return FALSE;

	POSITION pos = m_authenticators.GetHeadPosition();
	while (pos)
	{
		POSITION CurrentPos = pos;
		CAuthenticator* pAuth = (CAuthenticator *) m_authenticators.GetNext(pos);
		if (pAuth)
		{
			int CurrentIndex = GetAuthIndex(pAuth->GetAuthenticatorType());
			if (NewIndex == CurrentIndex)
			{
				// It's in the list already
				return TRUE;
			}
			if (NewIndex < CurrentIndex)
			{
				// This new authenticator has higher precedence than the current one, so add it before the current one
				CAuthenticator* NewAuth = DEBUG_NEW CAuthenticator(mailAuthenticators[NewIndex].pName, mailAuthenticators[NewIndex].pClient);
				m_authenticators.InsertBefore(CurrentPos, NewAuth);
				return TRUE;
			}
		}
	}

	// Least precedence, so add to the end of the list
	CAuthenticator* NewAuth = DEBUG_NEW CAuthenticator(mailAuthenticators[NewIndex].pName, mailAuthenticators[NewIndex].pClient);
	m_authenticators.AddTail (NewAuth);

	return TRUE;
}



CAuthenticator* CSasl::GetNextAuthenticator(PAuthClient pClient /*= NULL*/)
{
	POSITION pos = m_authenticators.GetHeadPosition();

	while (pos)
	{
		CAuthenticator *p = (CAuthenticator *) m_authenticators.GetNext(pos);
		if (p)
		{
			if (!pClient)
			{
				// No previous authenticator: return the first one.
				return p;
			}
			else if (p->GetAuthClient() == pClient)
			{
				// Found pClient in the list.
				if (pos)
				{
					// pClient is not the last authenticator in the list:
					// return the next authenticator.
					return (CAuthenticator *) m_authenticators.GetNext(pos);
				}
				else
				{
					// pClient is the last authenticator in the list: return NULL.
					return NULL;
				}
			}
		}
	}

	return NULL;
}




// DeleteList [PRIVATE]
//
// Free our list of authenticators.
//
void CSasl::DeleteList ()
{
	POSITION pos = m_authenticators.GetHeadPosition();

	while (pos)
	{
		CAuthenticator* pAuth = (CAuthenticator *) m_authenticators.GetNext(pos);

		delete pAuth;
	}

	m_authenticators.RemoveAll();
}

