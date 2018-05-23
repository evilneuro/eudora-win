// sasl.cpp - Implement the SASL wrapper.
//
#include "stdafx.h"

#include <qcutils.h>
#include <afxmt.h>

#include "sasl.h"



// AUTHENTICATORS that we can handle //


extern BOOL CramMD5Authenticator (PAuthChallenger pChallenger,
			      PAuthResponder pResponder, void *pStream, const char* pUser);



//====================================================================//

#define IsWhite(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')



// Note: If no authenticators, set this to {NULL}.
typedef struct
{
	// Name of the authentication mechanism.
	LPCSTR pName;

	PAuthClient pClient;
} MailAuthenticator;

static MailAuthenticator mailAuthenticators[] = {
							{"CRAM-MD5", CramMD5Authenticator},
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
	// Don't free contained objects.
	//
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


// If the given authenticator type is one we can handle, add it to our internal list.
//
BOOL CSasl::AddAuthenticator (LPCSTR pName)
{
	if (!pName)
		return FALSE;

	for (MailAuthenticator* p = mailAuthenticators; p->pName && p->pClient; p++)
	{
		if (strcmp (pName, p->pName) == 0)
		{
			CAuthenticator *pAuth = new CAuthenticator(p->pName, p->pClient);
			if (pAuth)
				m_authenticators.AddTail (pAuth);
		}
	}

	return FALSE;
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
				return p;
			else if (p->GetAuthClient() == pClient)
			{
				return (CAuthenticator *) m_authenticators.GetNext(pos);
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
	for (POSITION pos = m_authenticators.GetHeadPosition(); pos; )
	{
		CAuthenticator* pAuth = (CAuthenticator *) m_authenticators.GetNext(pos);

		delete pAuth;
	}

	m_authenticators.RemoveAll();
}
