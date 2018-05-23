// SmtpAuth.cpp
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

#include "sasl.h"
#include "md5.h"
#include "SMTPGssapi.h"
#include "SMTPSession.h"
#include "QCSMTPSettings.h"


#include "DebugNewHelpers.h"


LPCSTR GetPassword(void* pStream)
{
	QCSMTPThreadMT* pSmtp = (QCSMTPThreadMT *) pStream;

	return pSmtp ? pSmtp->GetPassword () : NULL;
}


/////////////////////////////////////////////////////////////////////////
// CramMD5Authenticator 
//
// Implement the CRAM-MD5 authenticator. This is called by imap_auth.
// It uses "challenger" to get the latest challenge the challenger
// ( the IMAP server in the case of IMAP), and "responnder" to send the 
// response.
//
/////////////////////////////////////////////////////////////////////////

BOOL CramMD5Authenticator (PAuthChallenger pChallenger, PAuthResponder pResponder,
							void* pStream, const char* pUser)
{
	BOOL bResult = FALSE;

	char tmp[1024];
	unsigned char szDigest[1024];
	unsigned long len;

	// Sanity:
	if (! (pChallenger && pResponder && pStream && pUser) )
	{
		ASSERT (0);
		return FALSE;
	}

	LPCSTR p = GetPassword(pStream);
	if (p)
		strcpy (tmp, p);
	else
		return FALSE;

	//
	// Get the first ready response.
	//
	char buf [1024];
	*buf = 0;
	len = (*pChallenger) (pStream, buf, 1020);

	// Must have a challenge string.
	if (len <= 0)
		return FALSE;

	//
	// Formulate our response. The password returned in "tmp"
	// is used as the key.
	// This returns the response in szDigest, starting at szDigest[1].
	// The size of the digest in returned in szDigest[0].
	//
	memset(szDigest, 0, sizeof(szDigest));

    hmac_md5( (unsigned char* )buf, (unsigned char* )tmp, (char *)szDigest);

	if ( !*szDigest )
	{
		ASSERT (0);
		goto end;
	}

	// Formulate response in the for: user<sp>digest. 
	sprintf (tmp, "%s ",pUser);

	// So far we've just got text in "tmp".
	len = strlen (tmp);

	// szDigest is not necessarily text:
	int dlen;
	int i;

	dlen = (int) (unsigned char)szDigest[0];

	for (i = 0 ; i < dlen; i++)
		tmp[len + i] = szDigest[i + 1];

	// Total length.		
	len += dlen;

	// Ok. Send the response.
	if (! (*pResponder) (pStream, tmp, len ) )
	{
		bResult = FALSE;
		goto end;
	}

	// If we get here, we've done our bit.
	bResult = TRUE;

end:
	// Cleanup!!


	return bResult;
}

BOOL LoginAuthenticator (PAuthChallenger pChallenger, PAuthResponder pResponder,
							void* pStream, const char* pUser)
{
	char buf [1024];

	// Sanity:
	if (! (pChallenger && pResponder && pStream && pUser) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Expecting "Username:" challenge
	*buf = 0;
	if ((*pChallenger) (pStream, buf, sizeof(buf) - 4) < 0)
		return FALSE;
	ASSERT(stricmp(buf, "Username:") == 0);

	// Sending user name
	strcpy(buf, pUser);
	if (!(*pResponder) (pStream, buf, strlen(buf)))
		return FALSE;

	// Expecting "Password:" challenge
	*buf = 0;
	if ((*pChallenger) (pStream, buf, sizeof(buf) - 4) < 0)
		return FALSE;
	ASSERT(stricmp(buf, "Password:") == 0);

	// Sending password
	strcpy(buf, GetPassword(pStream));
	if (!(*pResponder) (pStream, buf, strlen(buf)))
		return FALSE;

	return TRUE;
}

BOOL PlainAuthenticator (PAuthChallenger pChallenger, PAuthResponder pResponder,
							void* pStream, const char* pUser)
{
	char buf [1024];

	// Sanity:
	if (! (pChallenger && pResponder && pStream && pUser) )
	{
		ASSERT (0);
		return FALSE;
	}

	*buf = 0;
	if ((*pChallenger) (pStream, buf, sizeof(buf) - 4) < 0)
		return FALSE;

	// Sending <NULL>username<NULL>password
	buf[0] = 0;
	strcpy(buf + 1, pUser);
	LPCSTR password = GetPassword(pStream);
	strcpy(buf + strlen(pUser) + 2, password);
	if (!(*pResponder) (pStream, buf, strlen(pUser) + strlen(password) + 2))
		return FALSE;

	return TRUE;
}

BOOL GSSAPIAuthenticator(PAuthChallenger pChallenger, PAuthResponder pResponder,
						 void* pStream, const char* pUser)
{
	if (!pChallenger || !pResponder || !pStream || !pUser)
	{
		return FALSE;
	}

	QCSMTPThreadMT	*pSmtp = (QCSMTPThreadMT*)pStream;
	QCSMTPSettings	*pSmtpSettings = pSmtp->GetSettings();

	if (pSmtpSettings)
	{
		// If an SMTP server is specified use that as the host, otherwise
		// go with the POP server.
		CString		 strHost = pSmtpSettings->GetSMTPServer();
		if (strHost.IsEmpty())
		{
			strHost = pSmtpSettings->GetPOPAccount();
			int			iAtPos = strHost.Find('@');
			if (iAtPos >= 0)
			{
				strHost = strHost.Right(strHost.GetLength() - iAtPos - 1);
			}
		}

		// Attempt authentication.
		return SMTPGssapiAuthenticator(pChallenger, pResponder,
										pSmtp, (char*)pUser, strHost);
	}

	return FALSE;
}
