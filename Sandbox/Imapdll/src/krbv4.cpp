// krb4.cpp - Client authenticator for kerberos V4.
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "MyTypes.h"

#include <string.h>
#include <stdio.h>

#include "resource.h"
#include "exports.h"
#include "ImapDefs.h"
#include "osdep.h"
#include "Proto.h"

#include "krbv4.h"


// Imported::
extern unsigned long imap_maxlogintrials;


BOOL (CALLBACK* SetUserName)(LPSTR) = NULL;
BOOL (CALLBACK* GetTicketForService)(LPSTR, LPSTR, LPDWORD) = NULL;
BOOL (CALLBACK* SendTicketForService)(LPSTR service, LPSTR version, int fd) = NULL;

// Internal functions
//
unsigned long	ntohl (unsigned long ulNet);
unsigned short	ntohs (unsigned short usNet);
unsigned long	htonl (unsigned long ulHost);
unsigned short	htons (unsigned short usHost);


/////////////////////////////////////////////////////////////////////////
// CramMD5Authenticator 
//
// Implement the CRAM-MD5 authenticator. This is called by imap_auth.
// It uses "challenger" to get the latest challenge the challenger
// ( the IMAP server in the case of IMAP), and "responnder" to send the 
// response.
//
/////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4100)
#pragma warning (disable : 4101)


long KrbV4Authenticator (authchallenge_t challenger,
			      authrespond_t responder, void *s,
			      unsigned long *trial, char *user)
{
	long lResult = 0;

	// Sanity:
	//
	if (! (challenger && responder && s && user) )
	{
		return 0;
	}

	CProtocol *pProto = (CProtocol *) s;
	char tmp[MAILTMPLEN];

	//
	// JOK - Call back to upper layers. to get user and password.
	//
	if (pProto)
	{
		pProto->mm_login (user, tmp, *trial);

		CString szHost; pProto->GetCanonicalHostname (szHost);

		// Instantiate a CKrb4 to do all the work.
		//
		CKrb4 krb4Auth (challenger, responder, szHost, pProto, user);

		lResult =  krb4Auth.Authenticate();
	}

	return lResult;
}



// =========== class CKrb4 implementation ==============================//

CKrb4::CKrb4 (authchallenge_t challenger,
			  authrespond_t responder, LPCSTR pFQDN, CProtocol* s,
			  LPCSTR pUser)

			  : m_pChallenger (challenger),
			    m_pResponder (responder),
				m_szServerFQDN (pFQDN),
				m_pProtocol (s),
				m_szUser (pUser)
{

	m_hInstKerberosLib = NULL;

	m_hDesLib = NULL;

	//

	m_szService = CRString (IDS_IMAP_SERVICE);

	// DLL function pointers.
	//
	m_pfnKrbMkPriv	= NULL;

	m_pfnKrbMkPriv	= NULL;
	m_pfnKrbRdPriv	= NULL;

	m_pfnKrbMkReq	= NULL;
	m_pfnKrbRdReq	= NULL;

	m_pfnKrbRealmofhost = NULL;

	m_pfnKrbGetCred	= NULL;

	m_pfnKrbGetErrText = NULL;


	// DES:
	m_pfnDesKeySched	= NULL;
	m_pfnDesEcbEncrypt	= NULL;
	m_pfnDesPcbcEncrypt	= NULL;
}


CKrb4::~CKrb4()
{
	// Unload library every time:
	//
	if (m_hInstKerberosLib)
	{
		FreeLibrary (m_hInstKerberosLib);
		m_hInstKerberosLib = NULL;
	}

	// DES
	if (m_hDesLib)
	{
		FreeLibrary (m_hDesLib);
		m_hDesLib = NULL;
	}
}



// Wrappers around the kerberos DLL imported functions
//
char* CKrb4::krb_realmofhost (char *host)
{
	return m_pfnKrbRealmofhost ? (*m_pfnKrbRealmofhost) (host) : NULL;
}


long CKrb4::krb_mk_priv  (unsigned char* in, unsigned char* out,
				unsigned long in_length , Key_schedule schedule,
				C_Block *key, struct sockaddr_in *sender,
				struct sockaddr_in *receiver)
{
	return m_pfnKrbMkPriv ? 
		(*m_pfnKrbMkPriv)
			 (in, out, in_length, schedule, key,
			  sender, receiver) : 1;
}



int CKrb4::krb_mk_req (KTEXT authent, char *service, char *instance,
					char *realm, unsigned long checksum)
{
	return m_pfnKrbMkReq ? (*m_pfnKrbMkReq) 
				(authent, service, instance,
					realm, checksum) : 1;
}


int CKrb4::krb_rd_req (KTEXT authent, char *service, char *instance, unsigned long from_addr,
					AUTH_DAT *ad, char *fn)
{
	return m_pfnKrbRdReq ? (*m_pfnKrbRdReq) 
				(authent, service, instance, from_addr,
					ad, fn) : 1;
}


int CKrb4::krb_get_cred (char *service, char *instance, char *realm,
			CREDENTIALS *c)
{
	return m_pfnKrbGetCred ? (*m_pfnKrbGetCred) (service, instance, realm, c) : -1;
}



const char* CKrb4::krb_get_err_text (KRB_INT result)
{
	return m_pfnKrbGetErrText ? (*m_pfnKrbGetErrText) (result) : NULL;
}




// DES:

KRB_INT CKrb4::des_key_sched (des_cblock block, des_key_schedule keyshed)
{
	return m_pfnDesKeySched ? (*m_pfnDesKeySched) (block, keyshed) : (unsigned short)-1;
}

KRB_INT CKrb4::des_ecb_encrypt (des_cblock *block1, des_cblock *block2, des_key_schedule keyshed, KRB_INT encrypt)
{
	return m_pfnDesEcbEncrypt ? (*m_pfnDesEcbEncrypt) (block1, block2,keyshed, encrypt) : (unsigned short) -1;
}


KRB_INT CKrb4::des_pcbc_encrypt (des_cblock *block1, des_cblock *block2, long uLen,
			     des_key_schedule keyshed, des_cblock *block3, KRB_INT encrypt)
{

	return m_pfnDesPcbcEncrypt ? (*m_pfnDesPcbcEncrypt) 
				( block1, block2, uLen, keyshed, block3, encrypt) : (unsigned short)-1;
}



//========= PUBLIC  CKrb4   FUNCTIONS ==============//

long CKrb4::Authenticate ()
{
	if ( !Initialize() )
		return 0;

	// AUTHENTICATE command has already been sent.
	// Get the initial random number from IMAP server:
	// Note: Stage0Challenge return the number in network byte order.
	//

	unsigned long ulRndNum = 0;

	if ( !Stage0Challenge (&ulRndNum) )
		return 0;

	if ( !Stage1Response (ulRndNum) )
		return FALSE;

	if ( ! Stage2Challenge (ulRndNum) )
		return FALSE;
	
	return 1;
}


// =============== PRIVATE CKrb4 METHODS ================//

BOOL CKrb4::Initialize ()
{
	// Validate attributes.
	//
	if ( m_szUser.IsEmpty() || m_szServerFQDN.IsEmpty()  )
	{
		return FALSE;
	}

	if (!m_pProtocol)
		return FALSE;

	// Library names.
	//
	CString szLibName; m_pProtocol->GetKrb4LibraryName(szLibName);

	CString szDesLibName; m_pProtocol->GetDesLibraryName (szDesLibName);

	// If already loaded, return:
	//
	if (m_hInstKerberosLib && m_hDesLib)
		return TRUE;

	m_hInstKerberosLib = ::LoadLibrary(szLibName);

	if (!m_hInstKerberosLib)
	{
		m_pProtocol->AddLastError(IMAPERR_LOCAL_ERROR, IDS_ERR_LOADING_LIBRARY, szLibName);
		return FALSE;
	}

	m_hDesLib = ::LoadLibrary ( szDesLibName );

	if (!m_hDesLib)
	{
		m_pProtocol->AddLastError(IMAPERR_LOCAL_ERROR, IDS_ERR_LOADING_LIBRARY, szDesLibName);
		return FALSE;
	}

	m_pfnKrbMkPriv	= (KRB_MK_PRIV *) GetProcAddress (m_hInstKerberosLib, "krb_mk_priv");

	m_pfnKrbRdPriv	= (KRB_RD_PRIV *) GetProcAddress (m_hInstKerberosLib, "krb_rd_priv");

	m_pfnKrbMkReq	= (KRB_MK_REQ  *) GetProcAddress (m_hInstKerberosLib, "krb_mk_req");
	m_pfnKrbRdReq	= (KRB_RD_REQ  *) GetProcAddress (m_hInstKerberosLib, "krb_rd_req");

	m_pfnKrbRealmofhost = (KRB_REALMOFHOST *) GetProcAddress (m_hInstKerberosLib, "krb_realmofhost");

	m_pfnKrbGetCred = (KRB_GET_CRED *) GetProcAddress (m_hInstKerberosLib, "krb_get_cred" );

	// Don't fail if this isn't in the DLL:
	m_pfnKrbGetErrText = (KRB_GET_ERR_TEXT *) GetProcAddress (m_hInstKerberosLib, "krb_get_err_text" );

	// DES:
	//
	m_pfnDesKeySched	= (DES_KEY_SCHED *) GetProcAddress (m_hDesLib, "des_key_sched" );
	m_pfnDesEcbEncrypt	= (DES_ECB_ENCRYPT *) GetProcAddress (m_hDesLib, "des_ecb_encrypt" );

	m_pfnDesPcbcEncrypt = (DES_PCBC_ENCRYPT *) GetProcAddress (m_hDesLib, "des_pcbc_encrypt" );


	// Verify that these are OK
	//
	if ( ! ( m_pfnKrbMkPriv		&&
	         m_pfnKrbRdPriv		&&
			 m_pfnKrbMkReq		&&
			 m_pfnKrbRealmofhost &&
			 m_pfnKrbGetCred	&&
			 m_pfnDesKeySched &&
			 m_pfnDesEcbEncrypt &&
			 m_pfnDesPcbcEncrypt
		 ) )
	{
		return FALSE;
	}

	return TRUE;
}




// Stage0Challenge [PRIVATE]
//
// Preliminary stage:
// The "AUTHENTICATE" command has already been sent. 
// Go grab the first ready response and return it.
// The first ready response should be a random 4-byte integer in network byte
// order that becomes the CRC check.
// NOTE: Make sure we return the number in network byte order!!
//
// Return TRUE if we got the number, and put the number in "ulRndNum".
//
BOOL CKrb4::Stage0Challenge (unsigned long *ulRndNum)
{
	if (!ulRndNum)
		return FALSE;

	// Must have had a CProtocol and challenger:
	//
	if (! (m_pProtocol && m_pChallenger) )
		return FALSE;

	// Get the first ready response.
	//
	unsigned long len = 0;
	unsigned char* pChallenge = (unsigned char *)(*m_pChallenger) (m_pProtocol, &len);

	// Must have a challenge string.
	if ( NULL == pChallenge || len != 4)
		return FALSE;

	// Grab the data:
	//
	memcpy ((void *)ulRndNum, pChallenge, 4);

	{
		CString str; str.Format ("random number in network byte order is %lu", *ulRndNum);
//		AfxMessageBox (str);
	}

    // NOTE: ulRndNum is now in local byte order:
	//
	*ulRndNum = ntohl(*ulRndNum);

	// Can now free it:
	//
	fs_give ( (void **) &pChallenge);

	// Convert to local byte order:
	// 

	return TRUE;
}




BOOL CKrb4::Stage1Response (unsigned long ulRndNum)
{
	BOOL	bRet = FALSE;

	// Must have a responder.

	if (!m_pResponder)
		return FALSE;

	/* We should've just recieved a 32-bit number in network byte order.
     * We want to reply with an authenticator. */

    int result;

    KTEXT_ST ticket;

    memset(&ticket, 0, sizeof(ticket));	// very important to zero this out for the hack below -jdboyd 8.16.99

    ticket.length=MAX_KTXT_LEN;

	// These cannot be CStrings:
	//
    char    service[ANAME_SZ];				// "imap" 
    char    instance[INST_SZ];				// canonical hostname of server.
    char    realm[REALM_SZ];				// KERBEROS REALM.
	char	serverFQDN [MAX_K_NAME_SZ];

	if ( (m_szService.GetLength()  >= sizeof (service)  )	||
		 (m_szServerFQDN.GetLength() >= sizeof (serverFQDN)	) )
	{
		ASSERT (0);

		return FALSE;
	}

	strcpy (serverFQDN, m_szServerFQDN);

    char *pRealm = krb_realmofhost (serverFQDN);

	if (!pRealm)
		return FALSE;

	if ( strlen (pRealm) >= sizeof (realm) )
		return FALSE;

	// Save this:
	m_szRealm = pRealm;

	// Extract instance (hostname part of serverFDQN)
	//
	char *p = strchr (serverFQDN, '.');
	size_t len = 0;

	if (p)
	{
		len = (size_t) (p - serverFQDN);
	}
	else
	{
		len = strlen (serverFQDN);
	}
	
	if ( len >= sizeof (instance) )
	{
		ASSERT (0);
		return FALSE;
	}

	strncpy (instance, serverFQDN, len);
	instance[len] = '\0';

	// Save this:
	//
	m_szInstance = instance;

	// Make copies:
	//
	strcpy (service, m_szService);
	strcpy (realm, pRealm);

	
	// Kerberos Header File Hack	- jdboyd 8/16/99
	//
	// PC-LeLand expects a slightly different ticket structure than KClient.  In particular, the length
	// field is a short for PC-LeLand, and a long for KClient.  We try sending an older format ticket
	// structure that PC-LeLand understands.  If we notice the first two bytes of the actual ticket data are null,
	// then we can assume KClient or an updated PC-LeLand client is being used.  Knowing this, we try to get the
	// ticket again with the newer structure.  We put the results back in the older structure, since it gets used
	// later.

    result = krb_mk_req (&ticket, service, instance, realm, ulRndNum );
	if (result || (!ticket.dat[0] && !ticket.dat[1]))
	{
		NEWKTEXT_ST newticket;	// maybe try to get the ticket with the new structure
		
		if (!result) 
		{	
			// krb_mk_req didn't fail, but the data looks suspicious.  Try again with a newer ticket structure.
			result = krb_mk_req ((struct ktext *)(&newticket), service, instance, realm, ulRndNum );

			// put the ticket into the older structure, if we got it successfully this time
			if (!result)
			{
				ticket.length = (short) newticket.length;
				memcpy(ticket.dat, newticket.dat, newticket.length);
				ticket.mbz = 0;
			}
		}

		// Did krb_mk_req fail with some error?
		if (result)
		{
			CString szErr = krb_get_err_text ((KRB_INT)result);

			if ( !szErr.IsEmpty() )
			{
				m_pProtocol->AddLastErrorString (IMAPERR_LOCAL_ERROR, szErr);
			}

			return FALSE;
		}
	}

    void*	pClientOut = fs_get (ticket.length);

	if (pClientOut)
	{
		memcpy((char *) (pClientOut), ticket.dat, ticket.length);

		// Send the response to the server:
		//
		bRet = (*m_pResponder) ((void *)m_pProtocol, (char *)pClientOut, ticket.length) != 0;

		// Can now free this:
		//
		fs_give ( (void **)&pClientOut);
	}

	return bRet;
}


// Stage2Challenge [PRIVATE]
//
// Preliminary stage:
// The "AUTHENTICATE" command has already been sent. 
// Go grab the first ready response and return it.
// The first ready response should be a random 4-byte integer in network byte
// order that becomes the CRC check.
// NOTE: Make sure we return the number in network byte order!!
//
// Return TRUE if we got the number, and put the number in "ulRndNum".
//
BOOL CKrb4::Stage2Challenge (unsigned long ulRndNum)
{
	BOOL bRet = FALSE;

	// Must have had a CProtocol and challenger:
	//
	if (! (m_pProtocol && m_pChallenger) )
		return FALSE;

	// Get the second ready response. Length is now 8 bytes.
	//
	unsigned long len = 0;
	unsigned char* pChallenge = (unsigned char *)(*m_pChallenger) (m_pProtocol, &len);

	// Must have a challenge string.
	if ( NULL == pChallenge || len != 8)
		return FALSE;

	// Copy server's challenge:
	//
	int lup;
	unsigned char in [8];

    for (lup=0; lup<8; lup++)
      in[lup] = pChallenge[lup];

	// We can now free pChallenge:
	//
	fs_give ((void **)&pChallenge);
	pChallenge = NULL;

	// These cannot be CStrings:
	//
    char    service[ANAME_SZ];				// "imap" 
    char    instance[INST_SZ];				// canonical hostname of server.
    char    realm[REALM_SZ];				// KERBEROS REALM.

	// Make copies:
	//
	strcpy (service, m_szService);
	strcpy (realm, m_szRealm);
	strcpy (instance, m_szInstance);


	/* get credentials */
	CREDENTIALS credentials;;

    if ((krb_get_cred(service, instance, realm, &credentials)))
    {
		return FALSE;
    }

	// 
	des_key_schedule keysched;

    des_key_sched(credentials.session, keysched);

	/* verify data 1st 4 octets must be equal to chal+1 */
    des_ecb_encrypt((des_cblock *)in, (des_cblock *)in, keysched, DES_DECRYPT);

    unsigned long testnum = (unsigned long) in;

    testnum = (in[0]*256*256*256)+(in[1]*256*256)+(in[2]*256)+in[3];

//    testnum = ntohl(testnum);

    if (testnum != ulRndNum + 1)
      return FALSE;

    /* construct 8 octets
     * 1-4 - original checksum
     * 5 - bitmask of sec layer
     * 6-8 max buffer size
     */

    unsigned long nchal = htonl(ulRndNum);

	{
		CString str; str.Format ("random number in network byte order is %lu", nchal);
//		AfxMessageBox (str);
	}


	unsigned char sout [1024];

#if 0 // JOK     
	sout[0] = (unsigned char) nchal >> 24;
    sout[1] = (unsigned char) nchal >> 16;
    sout[2] = (unsigned char) nchal >> 8;
    sout[3] = (unsigned char) nchal;
#endif

	// Do a block copy (JOK)
	memcpy (sout, (void *)&nchal, 4);

    sout[4] = 1; /*4 */;     /*bitmask sec layers */
    sout[5] = 0x00;  /* max ciphertext buffer size */
    sout[6] = 0x04;
    sout[7] = 0x00;

	char userid [512];

	strcpy (userid, m_szUser);

#if 0 
	{
		CString str; str.Format ("Userid is %s", m_szUser);
		AfxMessageBox (str);
	}
#endif

    for (lup = 0; lup < (int) strlen(userid); lup++)
      sout[8+lup] = userid[lup];
    
    len = 8 + strlen(userid);	// was 9.  userid ends up after initial 8 characters, though ... -jdboyd Aug 6, 1999

    /* append 0 based octets so is multiple of 8 */
    while(len%8)
    {
		sout[len] = 0;
		len++;
    }
    sout[len] = 0;
    
    des_key_sched (credentials.session, keysched);

    des_pcbc_encrypt((des_cblock *)sout,
		     (des_cblock *)sout,
		     len - 1, keysched, (des_cblock *)credentials.session, DES_ENCRYPT);

    // Send this to IMAP. The caller will get the final response from IMAP
	// to determine if the user was authenticated.
	//
	
	void *pClientout = fs_get ( len );
	if (pClientout)
	{
		memcpy( (char *) pClientout, sout, len);
	
		// This gets sent finally to the server:
		//
		bRet = (*m_pResponder) ((void *)m_pProtocol, (char *)pClientout, len) != 0;

		// Can now free this:
		//
		fs_give ( (void **)&pClientout);
	}

	return bRet;
}




unsigned long ntohl (unsigned long ulNet)
{
	// 
	unsigned long b0 = (ulNet & 0xFF000000) >> 24;
	unsigned long b1 = (ulNet & 0x00FF0000) >> 8;

	unsigned long b2 = (ulNet & 0x000000FF) << 24;
	unsigned long b3 = (ulNet & 0x0000FF00) << 8;

	return b0 | b1 | b2 | b3;
}



unsigned short ntohs (unsigned short usNet)
{
	// 
	unsigned short b0 = (unsigned short) ( (usNet & 0xFF00) >> 8 );
	unsigned short b1 = (unsigned short) ( (usNet & 0x00FF) << 8 );

	return (unsigned short) (b0 | b1);
}


unsigned long htonl (unsigned long ulHost)
{
	return ntohl (ulHost);
}


unsigned short htons (unsigned short usHost)
{
	return ntohs (usHost);
}





#endif // IMAP4
