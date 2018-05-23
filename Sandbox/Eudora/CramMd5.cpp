// CramMd5.cpp - Client authenticator for CRAM-MD5.
//

#include "stdafx.h"

// #include "acapdlg.h"

#include "sasl.h"
#include "acapdlg.h"	// Contains the 

#include "md5.h"


// Must be supplied by caller:
//
extern LPCSTR GetCramPassword(void *pStream);

// These are already definged in acapdlg. SHOULD HAVE A GENERIC MECHANISM FOR THIS!!!
extern unsigned char * 
GetStamp(unsigned char * stamp,unsigned char * banner);

extern int hmac_md5(unsigned char* text, unsigned char* key, char* digest);



/////////////////////////////////////////////////////////////////////////
// CramMD5Authenticator 
//
// Implement the CRAM-MD5 authenticator. This is called by imap_auth.
// It uses "challenger" to get the latest challenge the challenger
// ( the IMAP server in the case of IMAP), and "responnder" to send the 
// response.
//
/////////////////////////////////////////////////////////////////////////

//BOOL CramMD5Authenticator (PAuthChallenger Challenger,
//			      PAuthResponder Responder, void *s, char *user, char *password)

BOOL CramMD5Authenticator (PAuthChallenger pChallenger,
			      PAuthResponder pResponder, void *pStream, const char* pUser)
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

	LPCSTR p = GetCramPassword(pStream);
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



