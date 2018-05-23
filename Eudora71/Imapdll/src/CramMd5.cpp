// CramMd5.cpp - Client authenticator for CRAM-MD5.
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "MyTypes.h"

#include "md5.h"
#include "resource.h"
#include "exports.h"
#include "ImapDefs.h"
#include "osdep.h"
#include "mm.h"
#include "Proto.h"		// Our class defs.

#include "CramMd5.h"

#include "DebugNewHelpers.h"


// Imported::
extern unsigned long imap_maxlogintrials;

// INTERNAL 
int hmac_md5(unsigned char* text, unsigned char* key, char* digest);



/////////////////////////////////////////////////////////////////////////
// CramMD5Authenticator 
//
// Implement the CRAM-MD5 authenticator. This is called by imap_auth.
// It uses "challenger" to get the latest challenge the challenger
// ( the IMAP server in the case of IMAP), and "responnder" to send the 
// response.
//
/////////////////////////////////////////////////////////////////////////

long CramMD5Authenticator (authchallenge_t challenger,
			      authrespond_t responder, void *s,
			      unsigned long *trial, char *user)
{
	CProtocol *pProto = (CProtocol *) s;
	char tmp[MAILTMPLEN];
	unsigned char szDigest[1024];
	unsigned long len;
	unsigned char *pChallenge = NULL;
	long ret = NIL;

	// Sanity:
	if (! (challenger && responder && s && trial && user) )
	{
		ASSERT (0);
		return NIL;
	}

	//
	// This authenticator uses a login password. The caller manages the
	// number of allowed trials. Here, we simply, (and always), get the password
	// and do the CRAM-MD5 thing.
	//
	tmp[0] = 0;			/* get password in this */

	//
	// JOK - Call back to upper layers. to get user and password.
	//
	pProto->mm_login (user, tmp, *trial);

	//
	// Must have both a user and a password.
	//
	if (! (tmp[0] && user[0]) )
	{
		//
		// user refused to give a password
		//

		// Don't allow any more trials.

		*trial = imap_maxlogintrials + 1;

		mm_log ("CRAM-MD5 authenticator aborted", IMAPERROR);

		pProto->AddLastError(IMAPERR_USER_CANCELLED, IDS_ERR_USER_ABORTED);

		return NIL;
	}

	//
	// Get the first ready response.
	//
	len = 0;
	pChallenge = (unsigned char *)(*challenger) (pProto, &len);

	// Must have a challenge string.
	if ( NULL == pChallenge || len == 0)
		return NIL;

	//
	// Formulate our response. The password returned in "tmp"
	// is used as the key.
	// This returns the response in szDigest, starting at szDigest[1].
	// The size of the digest in returned in szDigest[0].
	//
	memset(szDigest, 0, sizeof(szDigest));

    hmac_md5( (unsigned char* )pChallenge, (unsigned char* )tmp, (char *)szDigest);

	if ( !*szDigest )
	{
		ASSERT (0);
		goto end;
	}

	// Formulate response in the for: user<sp>digest. 
	sprintf (tmp, "%s ",user);

	// So far we've just got text in "tmp".
	len = strlen (tmp);

	// szDigest is not necessarily text:
	unsigned char dlen;
	unsigned char i;

	dlen = szDigest[0];

	for (i = 0 ; i < dlen; i++)
		tmp[len + i] = szDigest[i + 1];

	// Total length.		
	len += dlen;

	// Increment trial
	(*trial)++;

	// Ok. Send the response.
	if (! (*responder) (pProto, tmp, len ) )
	{
		ret = NIL;
		goto end;
	}

	// If we get here, we've done our bit.
	ret = 1;

end:
	// Cleanup!!

	if (pChallenge)
		fs_give ((void **) &pChallenge);
		
	return ret;
}




/************************************************************************
 * GetStamp - grab the timestamp out of a CRAM banner
 ************************************************************************/
unsigned char * 
GetStamp(unsigned char * stamp,unsigned char * banner)
{
	unsigned char * cp1,*cp2;
	int		len = 0;
	//*stamp = 0;
	
	//banner[*banner+1] = 0;
	
	cp1 = (unsigned char *) strchr((char *)banner,'<');

	if (cp1)
	{
		cp2=(unsigned char *)strchr((char *)cp1+1,'>');

		if (cp2)
		{
			len = cp2-cp1+1;
			strncpy((char *)stamp,(char *)cp1,len);
		} 
	}
	
	return (stamp);
}  



//
// produce hmac using md5 - based on the acap stuff.
//
// The result is returned in "digest", starting at byte 1. The size of the digest 
// is returned in digets[0] - yuck. This should change.
//

int hmac_md5(unsigned char* text, unsigned char* key, char* digest)
{
	MD5_CTX context;
    unsigned char stamp[255];

    unsigned char k_ipad[64];   
    unsigned char k_opad[64];    /* outer padding -
                                      * key XORd with opad
                                      */
    memset(stamp,0,sizeof(stamp));

    if (!*GetStamp((unsigned char *)stamp,(unsigned char *)text))
    {
	   	// Error_Notify(IDS_APOP_NO_BANNER_ERROR, -1);
	     return (FALSE);
	}

    unsigned char tk[16];
    static char hex[]="0123456789abcdef";
        
    text=stamp;
        
    int i;
    int ntext=strlen((const char* )text);
    int nkey=strlen((const char* )key);
        
    if (nkey > 64) 
    {
         MD5_CTX      tctx;
         MD5Init(&tctx);
         MD5Update(&tctx,(unsigned char*) key, nkey);
         MD5Final(&tctx);
          
         key  = (unsigned char* )tk;
         nkey = 16;
    }

    memset( k_ipad,0, sizeof(k_ipad));
    memset( k_opad,0, sizeof(k_opad));
    memcpy( k_ipad,key, nkey);
    memcpy( k_opad,key,nkey);

        
    for (i=0; i<64; i++) 
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5C;
    }
            
    MD5Init(&context);                
    MD5Update(&context, k_ipad, 64);  
    MD5Update(&context, text, ntext); 
    MD5Final(&context);          
        

    MD5Init(&context);                
    MD5Update(&context, k_opad, 64);  
    MD5Update(&context, (unsigned char* )context.digest, 16); 
    MD5Final(&context);
        
  	for (i = 0; i < sizeof(context.digest); i++)
  	{
    	digest[2*i+1] = hex[(context.digest[i]>>4)&0xf];
	    digest[2*i+2] = hex[context.digest[i]&0xf];
    }

    digest[0] = 2 * sizeof(context.digest);
    
	return (TRUE);
}


#endif / IMAP4
