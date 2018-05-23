// Proto.cpp - IMAP protocol implementation (class CProtocol).
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "MyTypes.h"

#include <afxmt.h>
#include <stdio.h>
#include <time.h>
#include "exports.h"
#include "ImapDefs.h"
#include "mm.h"
#include "Proto.h"		// Our class defs.

#include "resource.h"
#include "osdep.h"		// For fs_get and fs_give.
#include "rfc822.h"
#include "misc.h"
#include "ImapUtil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )

// Static functiosn: ============//
//
static void *imap_challenge (void *s,  // Set this to a CProtocol object.
					  unsigned long *len
					 );

static BOOL imap_response (void *s,	// Set this to a CProtocol object.
					char *response,
					unsigned long size);
static BOOL net_getbuffer (void *st, unsigned long size, char *buffer);
static BOOL str_getbuffer (void *st, unsigned long size, char *buffer);

// Global data:
//
unsigned long imap_maxlogintrials = 1;

// Static data:
//

// Used in several methods.
const char	Comma = ',';
const int MAX_UIDS_PER_COMMAND = 60;


/////////////////////////////////////////////////////////////////////////////////
// ===================== CProtocol implementation ===============================

CProtocol::	CProtocol(LPCSTR pServer, LPCSTR pPortnum)
{
	m_szHost = pServer;
	m_szPort = pPortnum;

	m_pCurrentElt = NULL;

	// Must be explicitly set.
	//
	m_bRecreatingMessageMap = FALSE;

	// Must clear this:
	//
	ClearMailStatus();
}

CProtocol::~CProtocol()
{
}



//============ Public maintenence things ==============//
void CProtocol::ClearMailStatus ()
{
	memset (&m_MailStatus, 0, sizeof(MAILSTATUS) );
}


// ========= Public interface functions ====================//

// FetchHeader [PUBLIC]
//
RCODE CProtocol::FetchHeader (unsigned long msgno, char *section,
				STRINGLIST *lines, long flags, ENVELOPE **ppEnvelope)
{
	char tmp[MAILTMPLEN];

	// Format fetch string.
	// Note that "HEADER" applies only to a top message or an rfc822
	// Assume that caller know what he's doing.
    if (section && *section)
		sprintf (tmp,"%s.HEADER",section);
    else
		strcpy (tmp,"HEADER");

	// Just pass the command to the driver.
	return Msgdata (msgno, tmp, 0, 0, lines, flags, ppEnvelope);
}




RCODE CProtocol::FetchMessage (unsigned long msgno, long flags)
{
	// Go do the driver thing.
    return Msgdata (msgno, "", 0, 0, NIL, flags, NULL);
}




RCODE CProtocol::FetchText (unsigned long msgno, char *section, long flags)
{
	char tmp[MAILTMPLEN];

	/* build IMAP-format section specifier */
	if (section && *section)
	{
	    sprintf (tmp,"%s.TEXT",section);
		flags &= ~FT_INTERNAL;	/* can't win with this set */
	}
	else
	{
		/* top-level message text wanted */
	    strcpy (tmp,"TEXT");
	}

	return Msgdata (msgno, tmp, 0, 0, NIL, flags, NULL);
}



RCODE CProtocol::PartialBody (unsigned long msgno, char *section,
			unsigned long first, unsigned long last, long flags)
{
	// Fetch the partial from the body.
	flags &= ~FT_INTERNAL;	/* bogus if this is set */

	// IF no section, fetch partial text of top-level message.
	if (!(section && *section))
	{
		// JOK modified!! Fetch  
		return Msgdata (msgno, "", first, last, NIL, flags, NULL);
	}
	else
	{
		return Msgdata (msgno, section, first, last, NIL, flags, NULL);
	}
}




RCODE CProtocol::FetchBody (unsigned long msgno, char *section, long flags)
{
	/* top-level text wanted? */
	if (!(section && *section))	
		return FetchMessage (msgno, flags);

	// Ok. We have a body part specifier. Go fetch that.
	flags &= ~FT_INTERNAL;	/* can't win with this set */

	return Msgdata (msgno, section, 0, 0, NIL, flags, NULL);
}


//	FetchMIME [PUBLIC]
//	jdboyd 8/24/99
RCODE CProtocol::FetchMIME (unsigned long msgno, char *section, long flags)
{
	char tmp[MAILTMPLEN];

	// Must have a section specifier for this to make sense
	if (!(section && *section)) return (NIL);

	flags &= ~FT_INTERNAL;

	// Format fetch string.
	sprintf (tmp,"%s.MIME",section);

	// call the driver.
	return Msgdata (msgno, tmp, 0, 0, NIL, flags, NIL);
}



MESSAGECACHE* CProtocol::MailElt ()
{
	// Does that stream we have one?
	if (!m_pCurrentElt)
		m_pCurrentElt = mail_new_cache_elt ();

	return m_pCurrentElt;
}





// Scan [ PUBLIC]
//
// IMAP scan mailboxes
//	    reference
//	    pattern to search
//	    string to scan
//

void CProtocol::Scan (char *ref, char *pat, char *contents)
{
	DoList (ref, pat, T, contents);
}



// LIST [PUBLIC]
//
// IMAP list mailboxes
// *	    reference
// *	    pattern to search
// 

void CProtocol::List (char *ref, char *pat)
{
	DoList (ref, pat, T, NIL);
}



// LSub [PUBLIC]
//
// IMAP list subscribed mailboxes
// *	    reference
// *	    pattern to search
//

void CProtocol::Lsub (char *ref, char *pat)
{
// JOK. Must have a stream.
//	ASSERT (stream != NULL);

	DoList (ref, pat, NIL, NIL);
}




// Subscribe [PUBLIC]
//
// IMAP subscribe to mailbox
// * mailbox to add to subscription list
// * Returns: T on success, NIL on failure
//
//
RCODE CProtocol::Subscribe (char *mailbox)
{
	return DoManage (mailbox, "SUBSCRIBE", NIL);
}



// Unsubscribe [PUBLIC]
//
// IMAP unsubscribe to mailbox
// *	    mailbox to delete from manage list
// * Returns: T on success, NIL on failure
//

RCODE CProtocol::Unsubscribe (char *mailbox)
{
	return DoManage (mailbox, "UNSUBSCRIBE", NIL);
}




// Create [PUBLIC]
//
// * IMAP create mailbox
// *	    mailbox name to create
// * Returns: T on success, NIL on failure
//
RCODE CProtocol::Create (char *mailbox)
{
	RCODE ret	= R_FAIL;

	// Sanity.
	//
	if (!mailbox)
		return R_FAIL;
	
	if (strlen (mailbox) >= MAILTMPLEN)
		return R_FAIL;

	IMAPPARSEDREPLY *reply;
	char mbx[MAILTMPLEN], mbx2[MAILTMPLEN];
	IMAPARG *args[3],ambx,amb2;
	ambx.type	= amb2.type = ASTRING; ambx.text = (void *) mbx;
	amb2.text	= (void *) mbx2;
	args[0]		= &ambx; args[1] = args[2] = NIL;

	//
	strcpy (mbx, mailbox);
		
	// Format command: 
	reply = Send ("Create", args);
	if (reply)
	{
		ret = ImapOK (reply) ? R_OK : R_FAIL;

		mm_log (reply->text, RCODE_SUCCEEDED (ret) ? (long) NIL : IMAPERROR);
	}

	return ret;
}



// Delete [PUBLIC]
//
// IMAP delete mailbox
//	    mailbox name to delete
// Returns: T on success, NIL on failure
//

RCODE CProtocol::Delete (char *mailbox)
{
	return DoManage (mailbox, "Delete", NIL);
}




// Rename [PUBLIC]
//
// * IMAP rename mailbox
// *	    old mailbox name
// *	    new mailbox name
// * Returns: T on success, NIL on failure
//

RCODE CProtocol::Rename (char *old, char *newname)
{
	return DoManage (old, "Rename",newname);
}





/* IMAP status
 * Accepts: mail stream
 *	    mailbox name
 *	    status flags
 * Returns: T on success, NIL on failure
 */

RCODE CProtocol::Status (LPCSTR pMailbox, long flags)
{
	IMAPARG *args[3],ambx,aflg;
	char tmp[MAILTMPLEN];
	RCODE ret = R_FAIL;

	// can't use stream if not IMAP4rev1, STATUS,
	if (!HasStatusCommand ())
	{
		// Should send error messages here$$$
		//
		return R_FAIL;
	}

	args[0] = &ambx; 
	args[1] = NIL;/* set up first argument as mailbox */

	ambx.type = ASTRING; ambx.text = (void *) pMailbox;

	/* have STATUS command? */
	aflg.type = FLAGS; aflg.text = (void *) tmp;
	args[1] = &aflg; args[2] = NIL;
	tmp[0] = tmp[1] = '\0';	/* build flag list */

	if (flags & SA_MESSAGES)
		 strcat (tmp," MESSAGES");

	if (flags & SA_RECENT)
		strcat (tmp," RECENT");

	if (flags & SA_UNSEEN)
		strcat (tmp," UNSEEN");

	if (flags & SA_UIDNEXT)
	{
		strcat (tmp, IsImap4Rev1 () ?  "UIDNEXT" : " UID-NEXT");
	}

	if (flags & SA_UIDVALIDITY)
	{
		strcat (tmp, IsImap4Rev1 () ? " UIDVALIDITY" : " UID-VALIDITY");
	}

	tmp[0] = '(';
	strcat (tmp,")");

	// send "STATUS mailbox flag".
	//
	ret = ImapOK (Send ("STATUS", args)) ? R_OK : R_FAIL;

	return ret;
}






// Open [PUBLIC]
//
// This is the entry point to establishing a connection with the server.
// If "bDoSelect" is TRUE, also SELECT the mailbox, otherwise just open a 
// "control" connection.
//
//	* NOTES (JOK)
//	* "stream" now must contain the host, and possibly the port and user name.
//	* END NOTES */

RCODE CProtocol::Open (LPCSTR pMailbox, unsigned long options)
{
	char *s, tmp[MAILTMPLEN],usr[MAILTMPLEN];
	IMAPPARSEDREPLY *reply = NULL;
	RCODE rcRet = R_FAIL;

	// Must have a hostname.
	//
	LPCSTR pszHostname = GetMachineName();

	if (! (pszHostname && *pszHostname) )
	{
		return R_FAIL;
	}

	// And it can't be too long:
	//
	if ( strlen ( pszHostname ) >= MAILTMPLEN )
	{
		return R_FAIL;
	}

	usr[0] = '\0';		/* initially no user name */

	// Cleanup in case we aborted last time. Will delete m_pNetStream.
	//
	Close (0);

	unsigned long prt = 0;

	// If a port is given, use that, otherwise use the default port.
	LPCSTR pszPort = GetPortStr ();
	if ( pszPort && *pszPort )
	{
		prt = atol ( pszPort );
	}
	else
	{
		prt = IMAPTCPPORT;
	}

	// Set up host.
	char host [MAILTMPLEN];
	strcpy (host, pszHostname );

	s = host;

	/* try to open ordinary (no PREAUTH) connection */

	if ( !NetOpen (s, "imap", m_ServiceID, prt) )
	{
		return R_FAIL;		
	}

	// Flag that we're connected:
	//
	SetConnected (TRUE);
	
	// Get and parse reply.
	if ( !ImapOK (reply = GetReply (NIL)) )
	{
		if (reply && reply->text)
		{
			AddLastErrorString ( IMAPERR_COMMAND_FAILED, reply->text);
			mm_log (reply->text, IMAPERROR);
		}

		return R_FAIL;
	}

	BOOL bAuthenticated = FALSE;
	
	// Get server capabilities.
	if ( IsConnected() )
	{
		// Get the canonical hostname for possible use in
		// imap_auth below:
		//
		CString szCanonHostname; GetCanonicalHostname (szCanonHostname);

		if ( (szCanonHostname.GetLength() > 0) &&
			 (szCanonHostname.GetLength() < sizeof (host) ) )
		{
			strcpy (host, szCanonHostname);
		}

		/* if have a connection */
		/* non-zero if not preauthenticated */


		if ( reply && (bAuthenticated = (strcmp (reply->key,"PREAUTH") == 0) ) )
		{
			usr[0] = '\0';
		}

		/* get server capabilities */
		reply = Send ("CAPABILITY",NIL);

		// Did user quit??
		//
		if (reply)
		{
			//
			// Authenticate:
			//

			if (!bAuthenticated)
			{ 
				/* need to authenticate? */
				//
				if (m_UseAuth && m_bAllowAuthenticate )
				{
					bAuthenticated = Authenticate (tmp, usr);
				}
				else
				{
					bAuthenticated = Login (tmp, usr);
				}
			}

			if (!bAuthenticated)
			{
				// Close the netstream if we failed to authenticate.
				NetClose ();
			}
		}
	}

	rcRet = bAuthenticated ? R_OK : R_LOGIN_FAILED;

	SetAuthenticated (bAuthenticated);

	// Set some state:
	//
	if ( IsAuthenticated() )
	{
	    m_bPermSeen = m_bPermDeleted = m_bPermAnswered =
		m_bPermDraft = IsLowerThanImap4() ? 1 : 0;

		m_PermUserFlags = IsLowerThanImap4() ? 0xffffffff : 0;

		// If we're still here, do the SELECT.
		//
		if ( pMailbox && !(options & OP_CONTROLONLY) )
		{
			rcRet = SelectMailbox (pMailbox);

			// Cache mailbox name if we succeeded:
			//
			if ( RCODE_SUCCEEDED (rcRet) )
				SetMailbox (pMailbox);
		}
	}
	
	return rcRet;
}




// SelectMailbox [PRIVATE]
//
RCODE CProtocol::SelectMailbox (LPCSTR pMailbox)
{
	RCODE ret = R_FAIL;
	IMAPPARSEDREPLY *reply = NULL;

	/* still have a connection? */
	if (! IsAuthenticated() )
	{
		return R_FAIL;
	}

	if (! (pMailbox && (strlen (pMailbox) < MAILTMPLEN) ) )
	{
		return R_FAIL;
	}

	char szMailbox [MAILTMPLEN];
	strcpy (szMailbox, pMailbox);

	IMAPARG *args[2];
	IMAPARG ambx;
	ambx.type = ASTRING;
	ambx.text = (void *) szMailbox;
	args[0] = &ambx; args[1] = NIL;

	// JOK 
	reply = Send (IsReadOnly() ? "EXAMINE": "SELECT", args);

	if ( reply && ImapOK (reply) )
	{
		// We've succeeded.
		SetSelected (TRUE);

		/* note if server said it was readonly */
		if (reply->text)
			SetReadOnly ( strncmp (ucase (reply->text),"[READ-ONLY]",11) == 0 );

		if (m_Nmsgs <= 0)
			mm_log ("Mailbox is empty",(long) NIL);

		ret = R_OK;
	}
	else
	{
		// Failed.
		// Make sure
		SetSelected (FALSE);

		if (reply && reply->text)
		{
			// JOK - Add server message.
			AddLastErrorString ( IMAPERR_COMMAND_FAILED, reply->text);
			mm_log (reply->text,IMAPERROR);
		}

		// If we fail, close the connection. Don't leave it half-open. (JOK).
		//
		NetClose ();

		ret = R_FAIL;
	}

	// If we're now connected, create msgno-to-uid mapping.
	//
	if ( IsSelected() )
	{
		RecreateMessageMapMT ();
	}

	return ret;
}





/* IMAP authenticate
 * Accepts: stream to authenticate
 *	    parsed network mailbox structure
 *	    scratch buffer
 *	    place to return user name
 * Returns: T on success, NIL on failure
 */

BOOL CProtocol::Authenticate (char *tmp, char *usr)
{
	unsigned long trial,ua;
	char tag[16];
	AUTHENTICATOR *at;
	IMAPPARSEDREPLY *reply = NIL;

	for (ua = m_UseAuth; IsConnected() && ua;)
	{
		if (!(at = mail_lookup_auth (find_rightmost_bit (&ua) + 1)))
		{
			fatal ("Authenticator logic bug!");
		}

		trial = 0;			/* initial trial count */
		do
		{			/* gensym a new tag */

			sprintf (tag,"%05ld", m_Gensym++);

				/* build command */
			sprintf (tmp, "%s AUTHENTICATE %s", tag, at->name);

			if (ImapSoutr (tmp) &&
				(*at->client) (imap_challenge, imap_response, (void *)this, &trial, usr))
			{
				/* abort if don't have tagged response */

				// THis replaces original (JOK).
				while (1)
				{
					reply = GetReply (tag);
					if (!reply)
						break;
	
					if ( strcmp (reply->tag, tag) )
						ImapSoutr ("*");
					else
						break;
				}

				/* done if got success response */
				if (reply && ImapOK (reply))
					return TRUE;

				if (reply && reply->text)
					mm_log (reply->text, WARN);
			}
		} while (IsConnected() && !m_bByeseen &&
			trial && (trial < imap_maxlogintrials));
	}

	return FALSE;			/* ran out of authenticators */
}





/* IMAP login
 * Accepts: 
 *	    scratch buffer
 *	    place to return user name
 * Returns: TRUE on success, FALSE on failure
 */

BOOL CProtocol::Login (char *tmp, char *usr)
{
	BOOL	bResult = FALSE;

	unsigned long trial = 0;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3];
	IMAPARG ausr,apwd;
	ausr.type = apwd.type = ASTRING;
	ausr.text = (void *) usr;
	apwd.text = (void *) tmp;
	args[0] = &ausr; args[1] = &apwd; args[2] = NIL;

	// 
	tmp[0] = 0;			/* get password */


	while (IsConnected() && !m_bByeseen && (trial < imap_maxlogintrials))
	{
		// JOK - Call back to upper layers..
		mm_login (usr, tmp, trial++);

		if (!tmp[0])
		{
			/* user refused to give a password */
			mm_log ("Login aborted",IMAPERROR);

			//
			// Signal that the user has aborted the login.
			//

			Notify (NETSTREAM_CLOSED, NULL);

			AddLastError(IMAPERR_USER_CANCELLED, IDS_ERR_USER_ABORTED);

			bResult = FALSE;
		}
		else
		{
			/* send "LOGIN usr tmp" */
			//
			reply = Send ("LOGIN", args);

			if (reply)
			{
				if ( ImapOK (reply) )
				{
					/* login successful, note if anonymous */
					SetAnonymous ( strcmp (usr, "anonymous") ? NIL : T );

					// If we have succeeded, tell that to the stream
					SetAuthenticated (TRUE);

					bResult = TRUE;			/* successful login */
				}
			}

			if (!bResult)
			{
				// Probably invalid password. In any case,
				// client should get new password from user.
				// (JOK - 8/18/98 - not any more).
				//

				// Must inform user:
				//
				if (reply && reply->text)
				{
					AddLastErrorString (IMAPERR_COMMAND_FAILED, reply->text);
				
					mm_log (reply->text,WARN);
				}
			}
		}
	}

	if (!bResult)
	{
		// Make sure the stream knows.
		SetAuthenticated (FALSE);
		SetSelected (FALSE);
	}

	return bResult;
}




// ImapChallenge [PUBLIC]
//
// NOTE: (JOK) - This function returns a "ready response" from the server, already
// base64 decoded. That is, the response from the server that follows the "+" sign.
// NOTE: This returns allocated memory that must be freed by the caller!!
// 
void* CProtocol::ImapChallenge (unsigned long *len)
{
	IMAPPARSEDREPLY *reply = NIL;
	char *p = Getline ();

	// JOK
	if (p)
	{
		reply = ParseReply (p);
	
		if (reply)
		{
			if ( strcmp (reply->tag, "+") )
				return NULL;
			else if (reply->text) 
				return rfc822_base64 ((unsigned char *) reply->text, strlen (reply->text), len);
		}
	}

	return NULL;
}



// ImapResponse [PUBLIC]
//
// NOTE (JOK): Use this to send our resonse back to the server. Pass the response text
// via "Response".
//
BOOL  CProtocol::ImapResponse (char *response, unsigned long size)
{
	unsigned long i,j;
	BOOL		ret = FALSE;
	char		*t, *u;

	if (size)
	{
  		/* make CRLFless BASE64 string */
		for (t = (char *) rfc822_binary ((void *) response,size,&i),u = t,j = 0;
				j < i; j++) if (t[j] > ' ')
		{
			 *u++ = t[j];
		}

		*u = '\0';			/* tie off string for mm_dlog() */

		mm_dlog (t);

		/* append CRLF */
		*u++ = '\015'; *u++ = '\012';

		ret = NetSout (t,i = u - t);

		fs_give ((void **) &t);
	}
	/* abort requested */
	else
		ret = ImapSoutr ("*");

	return ret;
}




void CProtocol::Stop ()
{
	// Call network parent's RequestStop() routine.
	//
	RequestStop();
}






//
// IMAP close
// Accepts: MAIL stream
//	    option flags
// NOTE: This closes the network connection and resets the stream flags,
// but does NOT delete the stream itselt.
//

void CProtocol::Close (long options)
{
	IMAPPARSEDREPLY *reply = NIL;

	/* send "LOGOUT" */
	if (IsSelected() && !m_bByeseen)
	{
		/* don't even think of doing it if saw a BYE */

		/* expunge silently if requested */
		if (options & CL_EXPUNGE)
			 Send ("EXPUNGE", NIL);

		if (IsSelected())
		{
			reply = Send ("LOGOUT",NIL);

			if (reply && reply->text)
				mm_log (reply->text, WARN);
		}
	}

	/* close NET connection if still open */
	NetClose ();

	// Reset stream status:
	SetConnected(FALSE);
	SetAuthenticated (FALSE);
	SetSelected (FALSE);

	/* free up memory */
	// 
	if (m_Reply.line)
		fs_give ((void **) &m_Reply.line);
}




// FetchFast [PUBLIC]
//
/* IMAP fetch fast information
 *	    sequence
 *	    option flags
 *
 * Generally, FetchStructure is preferred
 */

RCODE CProtocol::FetchFast (char *sequence, long flags)
{
	RCODE rResult = R_FAIL;

	char			*cmd		= NULL;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG			*args[3],aseq,aatt;

	// Format command. Send "FETCH sequence FAST" */
	// 
	cmd = (IsImap4()  && (flags & FT_UID)) ? "UID FETCH":"FETCH";

	aseq.type = SEQUENCE; aseq.text = (void *) sequence;

	aatt.type = ATOM; aatt.text = (IsImap4 ()) ?
			(void *) "(FLAGS INTERNALDATE RFC822.SIZE UID)" : (void *) "FAST";

	args[0] = &aseq; args[1] = &aatt; args[2] = NIL;

	reply = Send (cmd, args);
	if (reply)
	{
		if ( ImapOK (reply) )
			rResult = R_OK;

		if ( !RCODE_SUCCEEDED(rResult) && reply->text)
			mm_log (reply->text, IMAPERROR);
	}

	return rResult;
}




// FetchFlags [PUBLIC]
//
/* Accepts:
 *	    sequence
 *	    option flags
 */
// send "FETCH sequence FLAGS".

// HISTORY:
// END HISTORY.

RCODE CProtocol::FetchFlags (char *sequence, long flags)
{
	char *cmd = NULL;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;
	RCODE rResult = R_FAIL;

	if ( !IsSelected() )
		return R_FAIL;

	// FOrmat command.
	cmd = (IsImap4 () && (flags & FT_UID)) ? "UID FETCH":"FETCH";
	aseq.type = SEQUENCE; aseq.text = (void *) sequence;

#if 0 // No longer include RFC822.SIZE (6/3/98, JOK)
	aatt.type = ATOM; aatt.text = (void *) "(FLAGS RFC822.SIZE UID)";
#endif

	aatt.type = ATOM; aatt.text = (void *) "(FLAGS UID)";

	args[0] = &aseq; args[1] = &aatt; args[2] = NIL;

	// IF there is a CurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	MailElt ();

	// Now this:
	reply = Send (cmd, args);
	if (reply)
	{
		if ( ImapOK (reply) )
			rResult = R_OK;

		if ( !RCODE_SUCCEEDED (rResult) && reply->text)
			mm_log (reply->text,IMAPERROR);
	}

	return rResult;
}





// FetchEnvelope [PUBLIC]
//
// NOTE:
// JOK - This fetches just the message envelope.
// The parsing routine allocates an ENVELOPE structure and attaches it to the 
// stream's "current elt". We check for the envelope, detaches it from the stream and passes it to the caller.
// The caller MUST free the structure when done with it.
// END NOTE

ENVELOPE* CProtocol::FetchEnvelope (unsigned long msgno, long flags) 
{
	char seq[128],tmp[MAILTMPLEN];
	ENVELOPE *env;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;
	MESSAGECACHE	*elt = NULL;

	// Cannot have a zero msgno.
	if (msgno <= 0)
		return NULL;

	// Initialize: "env" is what's returned.
	env = NULL;

	args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
	aseq.type = SEQUENCE; aseq.text = (void *) seq;
	aatt.type = ATOM; aatt.text = NIL;

	// IF there is a CurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	elt = MailElt ();
	if (elt)
	{
		if (flags & FT_UID)
			elt->privat.uid = msgno;
		else
			elt->msgno = msgno;
	}

	// NOTE: "msgno" can be a UID or a message sequence number.
 	sprintf (seq,"%lu",msgno);	/* initial sequence (UID or msgno) */

	// Format command based on server capability.
	// NOTE: Can't handle any IMAP version older than imap2bis!!
	if (IsImap4() && (flags & FT_UID))
	{
		sprintf (tmp,"(UID ENVELOPE)");

		aatt.text = (void *) tmp;	/* do the built command */

		if (!ImapOK (reply = Send ("UID FETCH", args)))
		{
			if (reply && reply->text)
				mm_log (reply->text,IMAPERROR);
		}
    }
	else if (IsImap2bis ())
	{
		/* has non-extensive body and no UID. */
		sprintf (tmp,"(BODY)");

		aatt.text = (void *) tmp;	/* do the built command */

		if (!ImapOK (reply = Send ("FETCH",args)))
		{
			if (reply && reply->text)
				mm_log (reply->text,IMAPERROR);
		}
    }


	// "env" is what's returned.
	env = NULL;

	// Did we get anything?
	if (m_pCurrentElt)
	{
		// Make sure the UID's or msgno's matched.
		if (flags & FT_UID)
		{
			if (m_pCurrentElt->privat.uid == msgno)
				env = m_pCurrentElt->privat.msg.env;


		}
		else
		{
			if (m_pCurrentElt->msgno == msgno)
				env = m_pCurrentElt->privat.msg.env;
		}

		// Make sure we detach the body pointer if there was one.
		// If it's not our env, we'd want to delete it.
		if (env)
			m_pCurrentElt->privat.msg.env = NULL;

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return env;
}








// FetchStructure [PUBLIC]
//
// JOK - This fetches just the body structure.
// The parsing routine allocates a BODY and attaches it to the "current elt" in thye
// stream. We check for the body, detaches it from the elt and passes it to the caller.
// The caller MUST free the body when done with it.
// END NOTE
// Returns: Returns a BODY structure if successful.
//
BODY* CProtocol::FetchStructure (unsigned long msgno, long flags) 
{
	char seq[128],tmp[MAILTMPLEN];
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;
	BODY	*b;
	MESSAGECACHE *elt = NULL;

	// Cannot have a zero msgno.
	if (msgno <= 0)
		return NULL;

	// Initialize: "b" is what's returned.
	b = NULL;

	// Setup for IMAP call.
	args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
	aseq.type = SEQUENCE; aseq.text = (void *) seq;
	aatt.type = ATOM; aatt.text = NIL;

	// IF there is a CurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	elt = MailElt ();
	if (elt)
	{
		if (flags & FT_UID)
			elt->privat.uid = msgno;
		else
			elt->msgno = msgno;
	}

	// NOTE: "msgno" can be a UID or a message sequence number.
 	sprintf (seq,"%lu",msgno);	/* initial sequence (UID or msgno) */

	// Format command based on server capability.
	// NOTE: Can't handle any IMAP version older than imap2bis!!
	if (IsImap4() && (flags & FT_UID))
	{
		sprintf (tmp,"(UID BODYSTRUCTURE)");

		aatt.text = (void *) tmp;	/* do the built command */

		if (!ImapOK (reply = Send ("UID FETCH", args)))
		{
			if (reply && reply->text)
				mm_log (reply->text,IMAPERROR);
		}
    }
	else if (IsImap2bis ())
	{
		/* has non-extensive body and no UID. */
		sprintf (tmp,"(BODY)");

		aatt.text = (void *) tmp;	/* do the built command */

		if (!ImapOK (reply = Send ("FETCH", args)))
		{
			if (reply && reply->text)
				mm_log (reply->text,IMAPERROR);
		}
    }


	// "b" is what's returned.
	b = NULL;

	// Did we get anything?
	if (m_pCurrentElt)
	{
		// Make sure the UID's or msgno's matched.
		if (flags & FT_UID)
		{
			if (m_pCurrentElt->privat.uid == msgno)
				b = m_pCurrentElt->privat.msg.body;

		}
		else
		{
			if (m_pCurrentElt->msgno == msgno)
				b = m_pCurrentElt->privat.msg.body;
		}

		// Make sure we detach the body pointer if there was one.
		// If it's not our body, we'd want to delete it.
		if (b)
			m_pCurrentElt->privat.msg.body = NULL;

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return b;
}








// GetUid [PUBLIC]
//
// IMAP fetch UID
// Accepts:
// message number
// Returns: UID
//
unsigned long CProtocol::GetUid (unsigned long msgno)
{
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;
	char seq[MAILTMPLEN];
	unsigned long uid;
	MESSAGECACHE  *elt = NULL;

	/* IMAP2 didn't have UIDs */
	if (!IsImap4 ())
		return msgno;

	// Cannot have a zero msgno.
	if (msgno <= 0)
		return msgno;

	// Initialize:
	uid = 0;

	// IF there is a CurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	elt = MailElt ();
	if (elt)
		elt->msgno = msgno;

	// Setup for IMAP call.
 	sprintf (seq, "%lu", msgno);
    aseq.type = SEQUENCE;
	aseq.text = (void *) seq;
    aatt.type = ATOM;
	aatt.text = (void *) "UID";
    args[0] = &aseq;
	args[1] = &aatt;
	args[2] = NIL;

	/* send "FETCH msgno UID" */
    if (!ImapOK (reply = Send ("FETCH", args)))
	{
		if (reply && reply->text)
			mm_log (reply->text,IMAPERROR);
	}

	// Did we get anything?
	if (m_pCurrentElt)
	{
		// Make sure the msgno's matched.
		if (m_pCurrentElt->msgno == msgno)
			uid = m_pCurrentElt->privat.uid;

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return uid;
}




// GetMsgno [PUBLIC]
//

unsigned long CProtocol::GetMsgno (unsigned long uid)
{
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;	
	char seq[MAILTMPLEN];
	unsigned long msgno;
	MESSAGECACHE *elt = NULL;

	/* IMAP2 didn't have UIDs */
	if ( !IsImap4 () )
		return uid;

	// Initialize
	msgno = 0;

	/* have server hunt for UID */
	aseq.type = SEQUENCE; aseq.text = (void *) seq;
	aatt.type = ATOM; aatt.text = (void *) "UID";
	args[0] = &aseq; args[1] = &aatt; args[2] = NIL;
	sprintf (seq,"%lu",uid);

	// IF there is a m_pCurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	elt = MailElt ();
	if (elt)
		elt->privat.uid = uid;


	/* send "UID FETCH uid UID" */
	if (!ImapOK (reply = Send ("UID FETCH", args)))
	{
		if (reply && reply->text)
			mm_log (reply->text,IMAPERROR);
	}

	// Did we get anything?
	if (m_pCurrentElt)
	{
		// Make sure the uid's matched.
		if (m_pCurrentElt->privat.uid == uid)
			msgno = m_pCurrentElt->msgno;

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return msgno;			/* didn't find the UID anywhere */
}





// FetchRfc822size [PUBLIC]
// 

unsigned long CProtocol::FetchRfc822size (unsigned long msgno, long flags)
{
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;
	char *cmd = (IsImap4 () && (flags & FT_UID)) ? "UID FETCH":"FETCH";
	char seq[MAILTMPLEN];
	unsigned long rfc822size = 0;
	MESSAGECACHE  *elt = NULL;

	// Cannot have a zero msgno.
	if (msgno <= 0)
		return msgno;

	// Initialize:
	rfc822size = 0;

	// IF there is a CurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);
	m_pCurrentElt = NULL;

	// Allocate a new one.
	elt = MailElt ();
	if (elt)
	{
		if (flags & FT_UID)
			elt->privat.uid == msgno;
		else
			elt->msgno = msgno;
	}
	else
		return 0;

	// Setup for IMAP call.
 	sprintf (seq, "%lu", msgno);
    aseq.type = SEQUENCE;
	aseq.text = (void *) seq;
    aatt.type = ATOM;
	aatt.text = (void *) "RFC822.SIZE";
    args[0] = &aseq;
	args[1] = &aatt;
	args[2] = NIL;

	/* send "FETCH msgno UID" */
    if (!ImapOK (reply = Send (cmd, args)))
	{
		if (reply && reply->text)
			mm_log (reply->text,IMAPERROR);

			return 0;
	}

	// Did we get anything?
	if (m_pCurrentElt)
	{
		// Make sure the uid's matched.
		if (flags & FT_UID)
		{
			if (m_pCurrentElt->privat.uid == msgno)
				rfc822size = m_pCurrentElt->rfc822_size;
		}
		else
		{
			if (m_pCurrentElt->msgno == msgno)
				rfc822size = m_pCurrentElt->rfc822_size;
		}

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return rfc822size;
}






// SetFlags [PUBLIC]
//
// Do this in chunks.
//
//
RCODE CProtocol::SetFlags (LPCSTR pUidList, char* sFlagList, long flags)
{
	// Sanity:
	if (!(pUidList && sFlagList))
	{
		ASSERT (0);
		return R_FAIL;
	}

	// Copy Uidlist so we can modify it.
	char *pList = cpystr (pUidList);

	//
	// Note: If some got flagged, return TRUE.
	//
	BOOL bResult = FALSE;

	if (pList)
	{
		char *p  = pList;
		char *q;
		int nUids;
		BOOL	bThisResult;

		// Loop through pList and send chucks of uid's at a time.
		bThisResult = TRUE;

		while (p && bThisResult)
		{
			//
			// send this in smaller chunks!!
			//
			q = strchr (p, Comma);

			for (nUids = 0; q && nUids < MAX_UIDS_PER_COMMAND; nUids++)
			{
				q = strchr (q + 1, Comma);
			}

			// At this point, it q is non-NULL, it points to a comma.
			// Temporarily tie it off.
			// If q is NULL, then this is the last chunk.
			if (q)
			{
				*q = 0;
			}

			// Send this chunk.
			// Set the message flags.

			if (p && *p)
			{
				bThisResult = RCODE_SUCCEEDED (__SetFlags (p, sFlagList, flags));
			}

			if (!bThisResult)
				break;

			bResult = bResult || bThisResult;

			// Setup for next chunk.
			if (q)
			{
				// Put back comma.
				*q = Comma;
				p = q + 1;

				// In case have "3,4,5,"
				if (!*p)
					break;
			}
			else
			{
				// Stop loop.
				p = NULL;
			}
		}

		fs_give ((void **)&pList);
	}

	return bResult ? R_OK : R_FAIL;
}






// __SetFlags [PRIVATE]
//
// Do the real work.
//
/* IMAP modify flags
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 *	    option flags
 */

RCODE CProtocol::__SetFlags (char *sequence,char *flag,long flags)
{
	char *cmd;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[4],aseq,ascm,aflg;
	MESSAGECACHE *elt = NULL;
	BOOL	bResult = TRUE;		// JOK

	if ( !IsSelected() )
	{
		ASSERT(0);
		return R_FAIL;
	}

	cmd = (IsImap4 () && (flags & ST_UID)) ? "UID STORE":"STORE";

	aseq.type = SEQUENCE; aseq.text = (void *) sequence;
	ascm.type = ATOM; ascm.text = (void *)
	    ((flags & ST_SET) ?
			((IsImap4 () && (flags & ST_SILENT)) ?
			"+Flags.silent" : "+Flags") :
			((IsImap4 () && (flags & ST_SILENT)) ?
			"-Flags.silent" : "-Flags"));

	aflg.type = FLAGS; aflg.text = (void *) flag;
	args[0] = &aseq; args[1] = &ascm; args[2] = &aflg; args[3] = NIL;

	// IF there is a m_pCurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	elt = MailElt ();

	/* send "STORE sequence +Flags flag" */
	if (!ImapOK (reply = Send (cmd, args)))
	{
		// We failed.

		bResult = FALSE;

		if (reply && reply->text)
			mm_log (reply->text,IMAPERROR);
	}

	// Did we get anything?
	if (m_pCurrentElt)
	{
		// BUG: Is ST_SILENT is NOT set, we should check to see if the flags were
		// returned from the server correctly.

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return bResult ? R_OK : R_FAIL;
}





// Search [PUBLIC]
//
// This now just sends the command to the server. Untagged respponses will 
// be sent to a callback function.
//
void CProtocol::Search (char *charset, SEARCHPGM *pgm, long flags, CString& szResults)
{
	IMAPPARSEDREPLY *reply;
	IMAPARG *args[3],apgm,aseq,aatt;

	// Make sure these are empty:
	//
	m_szSearchResults.Empty();
	// Out parameter.
	szResults.Empty();

	args[1] = args[2] = NIL;
	apgm.type = SEARCHPROGRAM; apgm.text = (void *) pgm;
	aseq.type = SEQUENCE;
	aatt.type = ATOM;

	if (charset)
	{
		args[0] = &aatt; args[1] = &apgm;
		aatt.text = (void *) charset;
	}
	else
		args[0] = &apgm;

	/* do the SEARCH */
	if (!ImapOK (reply = Send ((flags & SE_UID) ? "UID SEARCH" : "SEARCH", args)))
	{
		if (reply && reply->text)
			mm_log (reply->text,ERROR);
	}

	// Copy results to out argument.
	//
	if ( !m_szSearchResults.IsEmpty() )
		szResults = m_szSearchResults;
}




// Ping [PUBLIC]
//
// Send "NOOP"
// Returns: T if stream still alive, else NIL
//
RCODE CProtocol::Ping ()
{
  return (IsSelected() &&
	  ImapOK (Send ("NOOP", NIL))) ? R_OK : R_FAIL;
}




// Check [PUBLIC]
//
// Send CHECK.
//
void CProtocol::Check ()
{
	IMAPPARSEDREPLY *reply = Send ("CHECK", NIL);

	if (reply)
		mm_log (reply->text, ImapOK (reply) ? (long) NIL : IMAPERROR);
}



// Expunge [PUBLIC]
//
// Send "EXPUNGE".
//
RCODE CProtocol::UIDExpunge (LPCSTR pUidList, CString& szUidsActuallyRemoved)
{
	RCODE rCode = R_OK;

	// Clear this.
	//
	m_szExpungeUidList.Empty();

	// Note: If pUidList is NULL, just expunge the mailbox.
	//
	if (pUidList)
	{
		// Need to do this - yuck.
		//
		char *pList = cpystr (pUidList);

		if (pList)
		{
			rCode = SetFlags (pList, "\\Deleted", ST_UID | ST_SET | ST_SILENT);

			fs_give ( (void **) &pList );
		}
	}

	if ( RCODE_SUCCEEDED (rCode) )
	{
		IMAPPARSEDREPLY *reply = Send ("EXPUNGE", NIL);

		if (reply)
		{
			if ( ImapOK (reply) )
			{
				rCode = S_OK;

				// Copy results to out parameter.
				//
				szUidsActuallyRemoved = m_szExpungeUidList;
			}
			
			if (reply->text)
				mm_log (reply->text, RCODE_SUCCEEDED (rCode) ? (long) NIL : IMAPERROR);
		}
	}

	return rCode;
}



// Expunge [PUBLIC]
//
// Send "EXPUNGE".
//
void CProtocol::Expunge ()
{

	IMAPPARSEDREPLY *reply = Send ("EXPUNGE", NIL);

	if (reply)
		mm_log (reply->text, ImapOK (reply) ? (long) NIL : IMAPERROR);
}






/* IMAP copy message(s)
 * Accepts: MAIL stream
 *	    sequence
 *	    destination mailbox
 *	    option flags
 * Returns: T if successful else NIL
 */

RCODE CProtocol::Copy (char *sequence, char *mailbox, long flags)
{
	char *cmd = (IsImap4 () && (flags & CP_UID)) ? "UID COPY" : "COPY";
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,ambx;
	aseq.type = SEQUENCE; aseq.text = (void *) sequence;
	ambx.type = ASTRING; ambx.text = (void *) mailbox;
	args[0] = &aseq; args[1] = &ambx; args[2] = NIL;

	/* send "COPY sequence mailbox" */
	if (!ImapOK (reply = Send (cmd, args)))
	{
		if (reply)
			mm_log (reply->text, IMAPERROR);

		return R_FAIL;
	}

	/* delete the messages if the user said to */
	if (flags & CP_MOVE)
		 SetFlags (sequence,"\\Deleted", ST_SET + ((flags & CP_UID) ? ST_UID : NIL));

  return R_OK;
}



/* IMAP append message string
 * Accepts: mail stream
 *	    destination mailbox
 *	    stringstruct of message to append
 * Returns: T on success, NIL on failure
 */

 /* Modified by JOK, May, 1997. */

RCODE CProtocol::Append (char *mailbox, char *flags, char *date, STRING *msg)
{
	char tmp[MAILTMPLEN];
	RCODE ret = R_FAIL;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[5],ambx,aflg,adat,amsg;

	// Must have a valid mailbox (JOK)
	if (!mailbox)
		return ret;

	// Copy mailbox into tmp.
	strcpy (tmp, mailbox);

	// Must have a valid and open stream. (JOK).
	//
	if ( IsSelected() )
	{
		ambx.type = ASTRING; ambx.text = (void *) tmp;
		aflg.type = FLAGS; aflg.text = (void *) flags;
		adat.type = ASTRING; adat.text = (void *) date;
		amsg.type = LITERAL; amsg.text = (void *) msg;

		if (flags || date)
		{	
			// IMAP4 form?
			//
			int i = 0;
			args[i++] = &ambx;
			if (flags) args[i++] = &aflg;
			if (date) args[i++] = &adat;
			args[i++] = &amsg;
			args[i++] = NIL;

			reply = Send ("APPEND", args);
			if ( reply && !strcmp (reply->key,"OK") )
				ret = R_OK;
		}

		// Try IMAP2bis form if IMAP4 form fails.
		//
		if ( !(RCODE_SUCCEEDED(ret) || (reply && strcmp (reply->key,"BAD"))))
		{
			args[0] = &ambx; args[1] = &amsg; args[2] = NIL;

			if (ImapOK (reply = Send ("APPEND", args)))
				ret = R_OK;
		}

		if ( !RCODE_SUCCEEDED (ret) )
		{
			if (reply && reply->text)
				mm_log (reply->text,IMAPERROR);
		}
	}
	else
		mm_log ("APPEND: No mailbox selected.", IMAPERROR);

  return ret;			/* return */
}



//========= Overrides of CNetStream functions ==================//

// NetNotify [PUBLIC]
//
/* virtual */
void CProtocol::NetNotify (NOTIFY_TYPE notify_type, void *data)
{
	// Just shunt this to the CStream's notifier function.
	//
	StreamNotify (notify_type, data);
}


// NetReportError [PUBLIC]
//
/* virtual */
void CProtocol::NetReportError (LPCSTR pStr, UINT Type)
{
	// Just pass this to the CStream.
	//
	ReportError (pStr, Type);
}



// NetLog [PUBLIC]
//
/* virtual */
void CProtocol::NetLog	(char *string, long errflg)
{
	// Just pass this to the CStream.
	//
	mm_log (string, errflg ? IMAPERROR : 0);
}




// RecreateMessageMap [PUBLIC]
//
// Recreate internal messagenumber-to-uid map.
RCODE CProtocol::RecreateMessageMapMT ()
{
	RCODE rc = R_OK;

	ClearMessageMap();

	if (m_Nmsgs > 0)
	{
		// If we fail for whatever reason, we must terminate the connection
		// so the message map gets rebuilt from scratch.
		//

		rc = FetchUids("1:*");

		if ( !RCODE_SUCCEEDED(rc) )
		{
			Close(0);
		}
	}

	return rc;
}




//========================= PRIVATE MEMBER FUNCTIONS ===============================//




// Msgdata [PRIVATE]
//
/* IMAP fetch message data
 * Accepts:
 *	    message number
 *	    section specifier
 *	    offset of first designated byte or 0 to start at beginning
 *	    maximum number of bytes or 0 for all bytes
 *	    lines to fetch if header
 *	    flags
 */

 // HISTORY:
 // Added by JOK, 3/20/987: ppEnvelope: If non-NULL, fetch envelopw as
 // well and return in ppEnvelope.
 //

RCODE CProtocol::Msgdata (unsigned long msgno,char *section,
		   unsigned long first, unsigned long last, STRINGLIST *lines,
		   long flags, ENVELOPE **ppEnvelope)
{
	char *t,tmp[MAILTMPLEN],part[40];
	char *cmd = (IsImap4() && (flags & FT_UID)) ? "UID FETCH":"FETCH";
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[7];
	IMAPARG aseq,aatt,alns,acls, aenvplus, aenvplusclose;
	char *s;

	// Initialize stuff.
	//
	aseq.type = NUMBER; aseq.text = (void *) msgno;
	aatt.type = ATOM;		/* assume atomic attribute */
	alns.type = LIST; alns.text = (void *) lines;
	acls.type = BODYCLOSE; acls.text = (void *) part;
	args[0] = &aseq; 
	args[1] = NIL;
	args[2] = args[3] = args[4] = args[5] = args[6] = NIL;

	part[0] = '\0';		/* initially no partial specifier */
	tmp[0] = '\0';

	// Must have an open mailbox:
	//
	if ( !IsSelected() )
		return R_FAIL;

	// Do we also fetch the envelope??
	// Note: We must put the ENVELOPE command args (in args[]), ahead of
	// BODYFETCH!!.
	//
	if (ppEnvelope)
	{
		MESSAGECACHE *elt = NULL;

		// Initialize:
		*ppEnvelope = NULL;

		// Set  up an argument for envelope:
		// Note: Must close this!!.

		aenvplus.type = ENVELOPEPLUS;
		aenvplus.text = "ENVELOPE";

		// Note: We must put the ENVELOPE command args (in args[]), ahead of
		// BODYFETCH!!.
		//
		args[1] = &aenvplus;
		args[2] = &aatt;

		// IF there is a m_pCurrentElt in the stream, delete it.
		if (m_pCurrentElt)
			mail_free_elt (&m_pCurrentElt);

		// Allocate a new one.
		elt	= MailElt ();
		if (elt)
		{
			if (flags & FT_UID)
				elt->privat.uid = msgno;
			else
				elt->msgno = msgno;
		}
	}
	else
	{
		args[1] = &aatt;
		args[2] = NULL;
	}

	// Format the rest of the command.
	//
	if ( IsImap4Rev1 () )
	{
		/* easy case if IMAP4rev1 server */
		aatt.type = (flags & FT_PEEK) ? BODYPEEK : BODYTEXT;

		if (lines)
		{
			/* want specific header lines? */
			sprintf (tmp,"%s.FIELDS%s",section,(flags & FT_NOT) ? ".NOT" : "");
			aatt.text = (void *) tmp;

			if (args[2])
			{
				args[3] = &alns;
				args[4] = &acls;
			}
			else
			{
				args[2] = &alns;
				args[3] = &acls;
			}
		}
		else
		{
			aatt.text = (void *) section;

			if (args[2])
			{
				args[3] = &acls;
			}
			else
			{
				args[2] = &acls;
			}
		}

		if (first || last)
			sprintf (part,"<%lu.%lu>",first,last ? last:-1);
	}
	/* BODY.PEEK[HEADER] becomes RFC822.HEADER */
	else if (!strcmp (section,"HEADER"))
	{
	    if (flags & FT_PEEK)
			aatt.text = (void *) "RFC822.HEADER";
		else
		{
			mm_notify ("[NOTIMAP4] Can't do non-peeking header fetch",WARN);
			return R_FAIL;
		}
	}
	/* other peeking was introduced in RFC-1730 */
	else if ((flags & FT_PEEK) && !IsImap4Only ())
	{
		mm_notify ("[NOTIMAP4] Can't do peeking fetch",WARN);
		return R_FAIL;
	}
	/* BODY[TEXT] becomes RFC822.TEXT */
	else if (!strcmp (section,"TEXT"))
	{
		aatt.text = (void *) ((flags & FT_PEEK) ? "RFC822.TEXT.PEEK" : "RFC822.TEXT");
	}
	/* BODY[] becomes RFC822 */
	else if (!section[0])
	{
		aatt.text = (void *) ((flags & FT_PEEK) ? "RFC822.PEEK" : "RFC822");
	}
	/* nested header */
	else if (t = strstr (section,".HEADER"))
	{
		if ( !IsImap4Only () )
		{	
			/* this was introduced in RFC-1730 */
			mm_notify ("[NOTIMAP4] Can't do nested header fetch",WARN);
			return R_FAIL;
		}

		aatt.type = (flags & FT_PEEK) ? BODYPEEK : BODYTEXT;

		if (args[2])
		{
			args[3] = &acls;		/* will need to close section */
		}
		else
		{
			args[2] = &acls;		/* will need to close section */
		}

		aatt.text = (void *) tmp;	/* convert .HEADER to .0 for RFC-1730 server */
		strncpy (tmp,section,t-section);
		strcpy (tmp+(t-section),".0");
	}
	/* extended nested text */
	else if (strstr (section,".MIME") || strstr (section,".TEXT"))
	{
		mm_notify ("[NOTIMAP4REV1] Can't do extended body part fetch",WARN);
		return R_FAIL;
	}
	/* nested message */
	else if ( IsImap2bis () )
	{
	    aatt.type = (flags & FT_PEEK) ? BODYPEEK : BODYTEXT;

		if (args[2])
		{
			args[3] = &acls;		/* will need to close section */
		}
		else
		{
			args[2] = &acls;		/* will need to close section */
		}

		aatt.text = (void *) section;
	}
	else if ( strtoul (section,&s,10) ) // Fetching the body section, non IMAP4r1 case -jdboyd 160699
	{
		aatt.text = tmp;

		if (flags & FT_PEEK)
			sprintf (tmp,"BODY.PEEK[%s]",section);
		else 
			sprintf (tmp,"BODY[%s]",section);
	}
	else
	{			/* ancient server */
		mm_notify ("[NOTIMAP2BIS] Can't do body part fetch",WARN);
		return R_FAIL;
	}


	if (ppEnvelope)
	{
		// Set up the closing argument:
		//
		aenvplusclose.type = ENVELOPEPLUSCLOSE;
		aenvplusclose.text = " ";

		// Find the last NULL argument.
		int i = 0;
		while (args[i])
		{
			i++;
		}
		args[i] = &aenvplusclose;
	}

	/* send the fetch command */
	if (!ImapOK (reply = Send (cmd, args)))
	{
		if (reply)
			mm_log (reply->text, IMAPERROR);

		return R_FAIL;			/* failure */
	}

	// Did we get an envelope?
	if (ppEnvelope && m_pCurrentElt)
	{
		ENVELOPE *pEnv = NULL;

		// Make sure the UID's or msgno's matched.
		if (flags & FT_UID)
		{
			if (m_pCurrentElt->privat.uid == msgno)
				pEnv = m_pCurrentElt->privat.msg.env;
		}
		else
		{
			if (m_pCurrentElt->msgno == msgno)
				pEnv = m_pCurrentElt->privat.msg.env;
		}

		// If we got an env, copy it to ppEnvelope.
		// Make sure we detach the body pointer if there was one.
		// If it's not our env, we'd want to delete it.
		if (pEnv)
		{
			*ppEnvelope = pEnv;
			m_pCurrentElt->privat.msg.env = NULL;
		}

		// Now free the elt.
		mail_free_elt (&m_pCurrentElt);
	}

	return R_OK;
}





// Send [PRIVATE]
//

IMAPPARSEDREPLY* CProtocol::Send (char *cmd, IMAPARG *args[])
{
	IMAPPARSEDREPLY *reply;
	IMAPARG *arg,**arglst;
	STRINGLIST *list;
	char c,*s,*t,tag[16];
	BOOL	bSeenEnvplus = FALSE;

	if (!IsConnected())
		return Fake (tag,"No-op dead stream");

	// Generate a new tag.
	//
	sprintf (tag, "%05ld", m_Gensym++);

	// write tag.
	//
	for (s = m_tmpbuf, t = tag; *t; *s++ = *t++);

	*s++ = ' ';			/* delimit and write command */

	// Copy command.
	for (t = cmd; *t; *s++ = *t++);

	if (arglst = args) while (arg = *arglst++)
	{
	    *s++ = ' ';			/* delimit argument with space */

	    switch (arg->type)
		{
			// Used for envelope along with other arguments, so we can
			// prepend an open brace.
			//
			case ENVELOPEPLUS:
				*s++ = '(';		/* wrap parens around string */
				for (t = (char *) arg->text; *t; *s++ = *t++);

				// Tell what follows that we've been here.
				bSeenEnvplus = TRUE;

				break;

			// Just append a closing brace:
			case ENVELOPEPLUSCLOSE:
				s[-1] = ')';		/* wrap parens around string */

				// reset.
				bSeenEnvplus = FALSE;

				break;

			case ATOM:			/* atom */
				for (t = (char *) arg->text; *t; *s++ = *t++);
				break;

			case NUMBER:		/* number */
				sprintf (s,"%lu",(unsigned long) arg->text);
				s += strlen (s);
				break;

			case FLAGS:			/* flag list as a single string */
				if (*(t = (char *) arg->text) != '(')
				{
					*s++ = '(';		/* wrap parens around string */
					while (*t) *s++ = *t++;

					*s++ = ')';		/* wrap parens around string */
				}
				else
					while (*t) *s++ = *t++;

				break;

			case ASTRING:		/* atom or string, must be literal? */
				if (reply = SendAstring (tag, &s, (char *)arg->text,
				     (unsigned long) strlen ((char *)arg->text),NIL))
				{
					return reply;
				}
				break;

			case LITERAL:		/* literal, as a stringstruct */
				if (reply = SendLiteral (tag, &s, (struct mailstring *) arg->text))
				{
					return reply;
				}

				break;

			case LIST:			/* list of strings */
				list = (STRINGLIST *) arg->text;

				c = '(';			/* open paren */

				do {			/* for each list item */
					*s++ = c;		/* write prefix character */
					if (reply = SendAstring (tag, &s, list->text.data,
				       list->text.size,NIL))
					{
						return reply;
					}

					c = ' ';		/* prefix character for subsequent strings */
				}
				while (list = list->next);

				*s++ = ')';		/* close list */

				break;

			case SEARCHPROGRAM:		/* search program */
				if (reply = SendSpgm (tag, &s, (struct search_program *)arg->text))
				{
					return reply;
				}

				break;

			case BODYTEXT:		/* body section */
				//
				// JOK - Now put "(" around BODY fetches (11/7/97).
				//
				// If we've been in an ENVELOPEPLUS, don't start with
				// a brace.
				if (!bSeenEnvplus)
					*s++ = '(';

				for (t = "BODY["; *t; *s++ = *t++);

				for (t = (char *) arg->text; *t; *s++ = *t++);

				break;

			case BODYPEEK:		/* body section */
				//
				// JOK - Now put "(" around BODY fetches (11/7/97).
				//
			
				// If we've been in an ENVELOPEPLUS, don't start with
				// a brace.
				if (!bSeenEnvplus)
					*s++ = '(';

				for (t = "BODY.PEEK["; *t; *s++ = *t++);

				for (t = (char *) arg->text; *t; *s++ = *t++);

				break;

			case BODYCLOSE:		/* close bracket and possible length */
				s[-1] = ']';		/* no leading space */

				// (JOK): Add any argument text before we put the closing ")".
				//
				for (t = (char *) arg->text; *t; *s++ = *t++);

				// If we've been in an ENVELOPEPLUS, don't end with
				// a brace.

				if (!bSeenEnvplus)
					*s++ = ')';

				break;

			case SEQUENCE:		/* sequence */
				// JOK - Don't recurse. Send the complete sequence!!

				/* falls through */

			case LISTMAILBOX:		/* astring with wildcards */
				if (reply = SendAstring (tag, &s, (char *)arg->text,
				     (unsigned long) strlen ((char *)arg->text),T))
				{
// JOK no onger 					mail_unlock(stream);
					return reply;
				}
				break;

			default:
				fatal ("Unknown argument type in imap_send()!");
		} // switch
	} // if.

	/* send the command */
	reply = ImapSout (tag, m_tmpbuf, &s);

	return reply;
}









// SendAstring [PRIVATE]
//
// Send atom-string.

IMAPPARSEDREPLY* CProtocol::SendAstring (char *tag,char **s,
				    char *t,unsigned long size,long wildok)
{
	unsigned long j;
	STRING st;
	int quoted = size ? NIL : T;	/* default to not quoted unless empty */

	for (j = 0; j < size; j++) switch (t[j])
	{
		case '\0':			/* not a CHAR */
		case '\012': case '\015':	/* not a TEXT-CHAR */
		case '"':
		case '\\':		/* quoted-specials (IMAP2 required this) */
			INIT (&st, mail_string, (void *) t,size);
			return SendLiteral (tag, s, &st);

		default:			/* all other characters */
			if (t[j] > ' ')
			break;	/* break if not a CTL */

		case '*': case '%':		/* list_wildcards */
			if (wildok)
				break;		/* allowed if doing the wild thing */

				/* atom_specials */
		case '(':
		case ')':
		case '{':
		case ' ':
		#if 0
		case '"': case '\\':		/* quoted-specials (could work in IMAP4) */
		#endif
			quoted = T;			/* must use quoted string format */
			break;
	}

	if (quoted) *(*s)++ = '"';	/* write open quote */

	while (size--)
	{
		 *(*s)++ = *t++;/* write the characters */
	}

	if (quoted)
		*(*s)++ = '"';	/* write close quote */

	return NIL;
}





// SendLiteral [PRIVATE]
//
//
// Accepts:
//	    reply tag
//	    pointer to current position pointer of output bigbuf
//	    literal to output as stringstruct
//
// Returns: error reply or NULL if success

IMAPPARSEDREPLY* CProtocol::SendLiteral (char *tag,char **s, STRING *st)
{
	IMAPPARSEDREPLY *reply;
	long i = SIZE (st);

	sprintf (*s,"{%ld}",i);	/* write literal count */
	*s += strlen (*s);		/* size of literal count */
				/* send the command */
	reply = ImapSout (tag, m_tmpbuf, s);

	if (reply && strcmp (reply->tag,"+"))
	{
		return reply;
	}

	while (i > 0)
	{			/* dump the text */
		/* JOK - If we don't have anything to output, flag an error. */
		if ( st->curpos == NULL || st->cursize <= 0)
		{
			return Fake (tag, "IMAP error reading data (data)");
		}
		
		if (!NetSout (st->curpos,st->cursize))
		{
			// JOK - mail_unlock (stream);
			return Fake (tag, "IMAP connection broken (data)");
		}

		i -= st->cursize;		/* note that we wrote out this much */
		st->curpos += (st->cursize - 1);
		st->cursize = 0;

		if (i > 0)
		{
			/* JOK - Go read more only if we need to */
			(*st->dtb->next) (st);	/* advance to next buffer's worth */
		}
	}

	return NIL;			/* success */
}







// SendSpgm  [PRIVATE]
//
// IMAP send search program
// Accepts:
//	    reply tag
//	    pointer to current position pointer of output bigbuf
//	    search program to output
// Returns: error reply or NIL if success
//
IMAPPARSEDREPLY* CProtocol::SendSpgm (char *tag, char **s, SEARCHPGM *pgm)
{
	IMAPPARSEDREPLY *reply;
	SEARCHHEADER *hdr;
	SEARCHOR *pgo;
	SEARCHPGMLIST *pgl;
//	char *t = "ALL";
	char *t = "";

	while (*t)
		 *(*s)++ = *t++;	/* default initial text */

	/* message sequences */
	if (pgm->msgno)
		SendSset (s, pgm->msgno);

	if (pgm->uid)
	{		/* UID sequence */
		for (t = "UID "; *t; *(*s)++ = *t++);

		SendSset (s, pgm->uid);
	}
				/* message sizes */
	if (pgm->larger)
	{
		sprintf (*s,"LARGER %lu",pgm->larger);
		*s += strlen (*s);
	}

	if (pgm->smaller)
	{
		sprintf (*s,"SMALLER %lu",pgm->smaller);
		*s += strlen (*s);
	}

	/* message flags */
	if (pgm->answered) for (t = "ANSWERED "; *t; *(*s)++ = *t++);
	if (pgm->unanswered) for (t ="UNANSWERED "; *t; *(*s)++ = *t++);
	if (pgm->deleted) for (t ="DELETED "; *t; *(*s)++ = *t++);
	if (pgm->undeleted) for (t ="UNDELETED "; *t; *(*s)++ = *t++);
	if (pgm->draft) for (t ="DRAFT "; *t; *(*s)++ = *t++);
	if (pgm->undraft) for (t ="UNDRAFT "; *t; *(*s)++ = *t++);
	if (pgm->flagged) for (t ="FLAGGED "; *t; *(*s)++ = *t++);
	if (pgm->unflagged) for (t ="UNFLAGGED "; *t; *(*s)++ = *t++);
	if (pgm->recent) for (t ="RECENT "; *t; *(*s)++ = *t++);
	if (pgm->old) for (t ="OLD "; *t; *(*s)++ = *t++);
	if (pgm->seen) for (t ="SEEN "; *t; *(*s)++ = *t++);
	if (pgm->unseen) for (t ="UNSEEN "; *t; *(*s)++ = *t++);
	if ((pgm->keyword &&		/* keywords */
		(reply = SendSlist (tag, s, "KEYWORD", pgm->keyword))) ||
      (pgm->unkeyword &&
		(reply = SendSlist (tag, s, "UNKEYWORD", pgm->unkeyword))))
		return reply;


	/* sent date ranges */
	if (pgm->sentbefore) SendSdate (s,"SENTBEFORE",pgm->sentbefore);
	if (pgm->senton) SendSdate (s,"SENTON",pgm->senton);
	if (pgm->sentsince) SendSdate (s,"SENTSINCE",pgm->sentsince);

	/* internal date ranges */
	if (pgm->before) SendSdate (s,"BEFORE",pgm->before);
	if (pgm->on) SendSdate (s,"ON",pgm->on);
	if (pgm->since) SendSdate (s,"SINCE",pgm->since);

	/* search texts */
	if ((pgm->bcc && (reply = SendSlist (tag, s, "BCC", pgm->bcc))) ||
		(pgm->cc && (reply = SendSlist (tag, s, "CC", pgm->cc))) ||
		(pgm->from && (reply = SendSlist(tag, s, "FROM", pgm->from)))||
		(pgm->to && (reply = SendSlist (tag, s, "TO", pgm->to))))
		return reply;

	if ((pgm->subject &&
		(reply = SendSlist (tag, s, "SUBJECT", pgm->subject))) ||
		(pgm->body && (reply = SendSlist(tag,s,"BODY",pgm->body)))||
		(pgm->text && (reply = SendSlist (tag, s, "TEXT", pgm->text))))
		return reply;

	// Make sure the next criteria doesn't get squished. -jdboyd
	// if we just added a criteria using SendSList ...
	if (pgm->bcc || pgm->cc || pgm->from || pgm->to || pgm->subject || pgm->body || pgm->text)
	{
		// and there are more to come ...
		if (pgm->header || pgm->or || pgm->not)
		{
			// add a space
			for (t = " "; *t; *(*s)++ = *t++);
		}
	}

	if (hdr = pgm->header) do
	{
		for (t = "HEADER "; *t; *(*s)++ = *t++);

		for (t = hdr->line; *t; *(*s)++ = *t++);

		// JOK - Add a space!!
		for (t = " "; *t; *(*s)++ = *t++);

		if (reply = SendAstring (tag,s,hdr->text,
				   (unsigned long) strlen (hdr->text),NIL))
			return reply;

		// JOK - Add a space if there are more headers to add.
		if (hdr->next)
		{
			for (t = " "; *t; *(*s)++ = *t++);
		}
	}
	while (hdr = hdr->next);

	if (pgo = pgm->or) do {
		for (t = "OR ("; *t; *(*s)++ = *t++);
		if (reply = SendSpgm (tag, s, pgo->first)) return reply;	// was pgm->or->first -jdboyd
		for (t = ") ("; *t; *(*s)++ = *t++);

		if (reply = SendSpgm (tag, s, pgo->second)) return reply;	// was pgm->or->second -jdboyd
		*(*s)++ = ')';
				
		// add a space if there is more to come -jdboyd
		if (pgo->next)
		{
			for (t = " "; *t; *(*s)++ = *t++);
		}

	} while (pgo = pgo->next);

	if (pgl = pgm->not) do {
		for (t = "NOT ("; *t; *(*s)++ = *t++);
		if (reply = SendSpgm (tag, s, pgl->pgm)) return reply;
		*(*s)++ = ')';
	}
	while (pgl = pgl->next);

	return NIL;			/* search program written OK */
}




// SendSset  [PRIVATE]
//
// IMAP send search set
// Accepts: pointer to current position pointer of output bigbuf
//	    search set to output
//

void CProtocol::SendSset (char **s,SEARCHSET *set)
{
	char c = 0;

	// Sanity: Must have at least these.
	if (!(set && set->first))
	{
		ASSERT (0);
		return;
	}

	do {				/* run down search set */
		if (c)
		{
			// If last is 0xFFFFFFFF, replace by "*". (JOK)
			//
			if (set->last == 0xFFFFFFFF)
			{
				sprintf (*s, "%c%lu:*", c, set->first);
			}
			else
			{
				sprintf (*s, set->last ? "%c%lu:%lu" : "%c%lu",c,set->first, set->last);
			}
		}
		else
		{
			if (set->last == 0xFFFFFFFF)
			{
				sprintf (*s, "%lu:*",set->first);
			}
			else
			{
				sprintf (*s, set->last ? "%lu:%lu" : "%lu",set->first,set->last);
			}
		}

		*s += strlen (*s);
		c = ',';			/* if there are any more */
	}
	while (set = set->next);

	// (JOK) Add a space after the list.
	for (char *t = " "; *t; *(*s)++ = *t++);
}





// SendSlist [PRIVATE]
//
// IMAP send search list
//  Accepts:
//	    reply tag
//	    pointer to current position pointer of output bigbuf
//	    name of search list
//	    search list to output
// Returns: NIL if success, error reply if error
//

IMAPPARSEDREPLY* CProtocol::SendSlist (char *tag, char **s, char *name, STRINGLIST *list)
{
	char *t;
	IMAPPARSEDREPLY *reply = NIL;

	do {
// (JOK) Space screws up some servers!!	    *(*s)++ = ' ';		/* output name of search list */
	    for (t = name; *t; *(*s)++ = *t++);

		*(*s)++ = ' ';
		reply = SendAstring (tag, s, list->text.data, list->text.size, NIL);

		// JOK - Add a space if there are more headers to add.
		if (list->next)
		{
			for (t = " "; *t; *(*s)++ = *t++);
		}
	}
	while (!reply && (list = list->next));

//	// JOK - Add a space so next criterion doesn't get squished.
//	for (t = " "; *t; *(*s)++ = *t++);

	return reply;
}


/* IMAP send search date
 * Accepts: pointer to current position pointer of output bigbuf
 *	    field name
 *	    search date to output
 */

void CProtocol::SendSdate (char **s,char *name,unsigned short date)
{
  sprintf (*s," %s %d-%s-%d",name,date & 0x1f,
	   months[((date >> 5) & 0xf) - 1],BASEYEAR + (date >> 9));
  *s += strlen (*s);
}



/* IMAP send buffered command to sender
 * Accepts: MAIL stream
 *	    reply tag
 *	    string
 *	    pointer to string tail pointer
 * Returns: reply
 */

IMAPPARSEDREPLY* CProtocol::ImapSout (char *tag, char *base, char **s)
{
	IMAPPARSEDREPLY *reply = NIL;

	// Make sure we have a netstream!!
	if ( !IsConnected() )
	{
		ASSERT (0);
		return NULL;
	}

	/* output debugging telemetry */
	**s = '\0';
	mm_dlog (base);

	*(*s)++ = '\015';		/* append CRLF */
	*(*s)++ = '\012';
	**s = '\0';

	// JOK:
	if (NetSout (base, *s - base))
	{
		reply = GetReply (tag);
	}

	*s = base;			/* restart buffer */

	// CAn be NULL.
	return reply;
}



/* IMAP send null-terminated string to sender
 * Accepts: MAIL stream
 *	    string
 * Returns: T if success, else NIL
 */

BOOL CProtocol::ImapSoutr (char *string)
{
	char tmp[MAILTMPLEN];
	
	if ( !IsConnected() )
		return FALSE;

	mm_dlog (string);

	return NetSoutr (strcat (strcpy (tmp,string),"\015\012"));
}





// GetReply {PRIVATE]
//
// IMAP get reply
// Accepts:
//	    tag to search or NIL if want a greeting
// Returns: parsed reply, never NIL
//

IMAPPARSEDREPLY* CProtocol::GetReply (char *tag)
{
	IMAPPARSEDREPLY *reply = NIL;

	while ( IsConnected() )
	{
		char *txt = NULL;

		// Fetch reply.
		txt = Getline ();

		if (NULL == txt)
		{
			if ( UserAborted () )
			{
				AddLastError (IMAPERR_USER_CANCELLED, IDS_ERR_USER_ABORTED);
			}
			else
			{
				AddLastError (IMAPERR_LOCAL_ERROR, IDS_ERR_CONNECTION_BROKEN, GetMachineName());
			}

			// Notify stream that conenction has been broken:
			//
			StreamNotify (NETSTREAM_CLOSED, NULL);
	
			return NULL;
		}

		/* parse reply from server */

		// NOTE: txt was ALLOCATED memory. imap_parse_reply takes care of freeing it.
		reply = ParseReply (txt);

		if (reply)
			{
			/* continuation ready? */
			if (!strcmp (reply->tag,"+"))
			{
				return reply;
			}
			/* untagged data? */
			else if (!strcmp (reply->tag,"*"))
			{
				ParseUnsolicited (reply);
				if (!tag)
					return reply;	/* return if just wanted greeting */
			}
			else
			{			/* tagged data */
				if (tag && !strcmp (tag,reply->tag))
				{
					// Blank last message.
					szSrvTRespBuf[0] = '\0';

					// Tags match. Copy this to the stream as well (JOK).
					if (reply->text)
					{
						long len =  min ( sizeof (szSrvTRespBuf) - 1, strlen ( reply->text ) );
						strncpy (szSrvTRespBuf, reply->text, len);
						szSrvTRespBuf[len] = '\0';
					}

					return reply;
				}
		
				if (reply->text && reply->tag && reply->key)
				{
					/* report bogon */
					AddLastError (IMAPERR_COMMAND_FAILED, IDS_ERR_UNEXPECTED_UNTAGGED_RESPONSE, 
								reply->tag, reply->key, reply->text);
				
					sprintf (m_tmpbuf, "Unexpected tagged response: %.80s %.80s %.80s",
							reply->tag, reply->key, reply->text);
					mm_log (m_tmpbuf, WARN);
				}
			}
		}
	}

#if 0 // TEST JOK
	return Fake (tag, "IMAP connection broken (server response)");
#endif

	return NULL;
}






// ParseReply [PRIVATE]
//
// IMAP parse reply
//  Accepts:
//	    text of reply
// Returns: parsed reply, or NIL if can't parse at least a tag and key
//
IMAPPARSEDREPLY* CProtocol::ParseReply (char *text)
{
	if (m_Reply.line)
		fs_give ((void **) &m_Reply.line);

	if (!(m_Reply.line = text))
	{
		/* NIL text means the stream died */
		NetClose ();

		return NIL;
	}

	mm_dlog (m_Reply.line);

	m_Reply.key = NIL;	/* init fields in case error */

	m_Reply.text = NIL;

	if (!(m_Reply.tag = (char *) strtok (m_Reply.line," ")))
	{
		mm_log ("IMAP server sent a blank line",WARN);
		return NIL;
	}

	/* non-continuation replies */
	if (strcmp (m_Reply.tag,"+"))
	{
				/* parse key */
		if (!(m_Reply.key = (char *) strtok (NIL," ")))
		{
			/* determine what is missing */
			sprintf (m_tmpbuf, "Missing IMAP reply key: %.80s", m_Reply.tag);

			mm_log (m_tmpbuf,WARN);	/* pass up the barfage */

			return NIL;		/* can't parse this text */
		}

		ucase (m_Reply.key);	/* make sure key is upper case */

		/* get text as well, allow empty text */
		if (!(m_Reply.text = (char *) strtok (NIL,"\n")))
		{
			m_Reply.text = m_Reply.key + strlen (m_Reply.key);
		}
	}
	else
	{
		/* special handling of continuation */
		m_Reply.key = "BAD";	/* so it barfs if not expecting continuation */

		if (!(m_Reply.text = (char *) strtok (NIL,"\n")))
		{
			m_Reply.text = "Ready for more command";
		}
	}

	return &m_Reply;		/* return parsed reply */
}




// Fake [PRIVATE]
//
// IMAP fake reply
// Accepts:
//	    tag
//	    text of fake reply
// Returns: parsed reply
//
IMAPPARSEDREPLY* CProtocol::Fake (char *tag, char *text)
{
	mm_notify (text, BYE);	/* send bye alert */

	NetClose ();

	/* build fake reply string */
	sprintf (m_tmpbuf, "%s NO [CLOSED] %s",tag ? tag : "*",text);

	// Add text as last error.
	AddLastErrorString ( IMAPERR_CONNECTION_CLOSED, m_tmpbuf );

	return NIL;
}







// ImapOK [PRIVATE]
//
// Parses the reply from IMAP.
//
BOOL CProtocol::ImapOK (IMAPPARSEDREPLY *reply)
{
	// Sanity;
	if (! reply )
		return FALSE;

		/* OK - operation succeeded */
	if (!strcmp (reply->key,"OK") || (!strcmp (reply->tag,"*") && !strcmp (reply->key,"PREAUTH")))
	{
		return TRUE;
	}
	/* BAD - operation rejected */
	else if (!strcmp (reply->key,"BAD"))
	{
		if (reply->key && reply->text)
		{
			sprintf (m_tmpbuf,"IMAP error: %.80s",reply->text);

			AddLastError (IMAPERR_COMMAND_FAILED, IDS_ERR_BAD_KEY, reply->key, reply->text);
		}
	}
	/* NO - operation failed */
	else
	{
		if (reply->key && reply->text)
		{
			sprintf (m_tmpbuf,"Unexpected IMAP response: %.80s %.80s",
				reply->key,reply->text);
			
			AddLastErrorString (IMAPERR_COMMAND_FAILED, reply->text);
		}
	}

	mm_log (m_tmpbuf,WARN);

	// If we get here, we failed:
	//
	return FALSE;
}





/* IMAP parse and act upon unsolicited reply
 * Accepts: MAIL stream
 *	    parsed reply
 */

// JOK - modified.

void CProtocol::ParseUnsolicited (IMAPPARSEDREPLY *reply)
{
	unsigned long i = 0;
	unsigned long msgno = 0;
	char *s,*t;
	char *keyptr,*txtptr;

	// Sanity:
	if (! reply )
		return;

	// Anytime we get here, make sure there is a m_pCurrentElt in the stream.
	if (!m_pCurrentElt)
	{
		// Allocate a new one.
		m_pCurrentElt = MailElt ();
	}

	/* see if key is a number */
	msgno = strtoul (reply->key,&s,10);

	// Keep track of it.
	//
	if (m_pCurrentElt && msgno)	// jdboyd - don't update the elt's msgno if this is a non-message specific message.
		m_pCurrentElt->msgno = msgno;

	/* if non-numeric */
	if (*s)
	{			
	    if (!strcmp (reply->key,"FLAGS"))
		{
			/* flush old user flags if any */
			while ((i < NUSERFLAGS) && m_UserFlags[i])
				fs_give ((void **) &m_UserFlags[i++]);

			i = 0;			/* add flags */
			if (s = (char *) strtok (reply->text+1," )"))
			{
				do if (*s != '\\') m_UserFlags[i++] = cpystr (s);
					while (s = (char *) strtok (NIL," )"));
			}
		}
		else if (!strcmp (reply->key,"SEARCH"))
		{
			/* only do something if have text */
			if (reply->text && (t = (char *) strtok (reply->text," "))) do 
			{
				i = atol (t);

				mm_searched (i);
			}
			while (t = (char *) strtok (NIL," "));
		}
	    else if (!strcmp (reply->key,"STATUS"))
		{
			char *txt;

			// JOK - Make sure of this:
			ClearMailStatus();

			/* mailbox is an astring */
			switch (*reply->text)
			{	
				case '"':			/* quoted string? */
				case '{':			/* literal? */
					txt = reply->text;	/* status data is in reply */
					t = ParseString (&txt,reply,NIL,NIL);
				break;
				default:			/* must be atom */
					t = cpystr (reply->text);
					if (txt = strchr (t,' ')) *txt++ = '\0';
				break;
			}

			if (t && txt && (*txt++ == '(') && (s = strchr (txt,')')) && (s - txt) &&
					!s[1])
			{
				*s = '\0';		/* tie off status data */

				/* initialize data block */
				m_MailStatus.flags = m_MailStatus.messages = m_MailStatus.recent = m_MailStatus.unseen =
				m_MailStatus.uidnext = m_MailStatus.uidvalidity = 0;

				ucase (txt);		/* do case-independent match */

				while (*txt && (s = strchr (txt,' ')))
				{
					*s++ = '\0';		/* tie off status attribute name */
					i = strtoul (s,&s,10);/* get attribute value */
					if (!strcmp (txt,"MESSAGES"))
					{
						m_MailStatus.flags |= SA_MESSAGES;
						m_MailStatus.messages = i;
					}
					else if (!strcmp (txt,"RECENT"))
					{
						m_MailStatus.flags |= SA_RECENT;
						m_MailStatus.recent = i;
					}
					else if (!strcmp (txt,"UNSEEN"))
					{
						m_MailStatus.flags |= SA_UNSEEN;
						m_MailStatus.unseen = i;
					}
					else if (!strcmp (txt,"UIDNEXT") || !strcmp (txt,"UID-NEXT"))
					{
						m_MailStatus.flags |= SA_UIDNEXT;
						m_MailStatus.uidnext = i;
					}
					else if (!strcmp (txt,"UIDVALIDITY")|| !strcmp (txt,"UID-VALIDITY"))
					{
						m_MailStatus.flags |= SA_UIDVALIDITY;
						m_MailStatus.uidvalidity = i;
					}

					/* next attribute */
					txt = (*s == ' ') ? s + 1 : s;
				}  // while

				strcpy (m_tmpbuf, t);

				/* pass status to main program */
				mm_status (m_tmpbuf, &m_MailStatus);

			} // if

			fs_give ((void **) &t);
		}

		else if ((!strcmp (reply->key,"LIST") || !strcmp (reply->key,"LSUB")) &&
			     (*reply->text == '(') && (s = strchr (reply->text,')')) &&
			 (s[1] == ' '))
		{
			char delimiter = '\0';
			*s++ = '\0';		/* tie off attribute list */

			/* parse attribute list */
			if (t = (char *) strtok (reply->text+1," "))
			do
			{
				if (!strcmp (ucase (t),"\\NOINFERIORS"))
					i |= LATT_NOINFERIORS;
				else if (!strcmp (t,"\\NOSELECT"))
					i |= LATT_NOSELECT;
				else if (!strcmp (t,"\\MARKED"))
					i |= LATT_MARKED;
				else if (!strcmp (t,"\\UNMARKED"))
					i |= LATT_UNMARKED;

				/* ignore extension flags */
			} while (t = (char *) strtok (NIL," "));

			switch (*++s)
			{		/* process delimiter */
				case 'N':			/* NIL */
				case 'n':
					s += 4;			/* skip over NIL<space> */
				break;
				case '"':			/* have a delimiter */
					delimiter = (*++s == '\\') ? *++s : *s;
					s += 3;			/* skip over <delimiter><quote><space> */
			}

			/* need to prepend a prefix? */
			if (m_Prefix)
				strcpy (m_tmpbuf, m_Prefix);
			else
				m_tmpbuf[0] = '\0';/* no prefix needed */

			/* need to do string parse? */
			if ((*s == '"') || (*s == '{'))
			{
				strcat (m_tmpbuf,t = ParseString (&s, reply, NIL, NIL));
				fs_give ((void **) &t);
			}
			else
				strcat(m_tmpbuf,s);/* atom is easy */

			if (reply->key[1] == 'S')
				mm_lsub (delimiter, m_tmpbuf, i);
			else
				mm_list (delimiter, m_tmpbuf, i);
		}
		else if (!strcmp (reply->key,"MAILBOX"))
		{
			if (m_Prefix)
				sprintf (t = m_tmpbuf, "%s%s", m_Prefix, reply->text);
			else
				t = reply->text;

			mm_list (NIL,t,NIL);
		}

	    else if (!strcmp (reply->key,"OK") || !strcmp (reply->key,"PREAUTH"))
		{
			if ((*reply->text == '[') && (t = strchr (s = reply->text + 1,']')) &&
				((i = t - s) < IMAPTMPLEN))
			{
				/* get text code */
				strncpy (m_tmpbuf, s, (size_t) i);
				m_tmpbuf[i] = '\0';	/* tie off text */

				if (!strcmp (ucase (m_tmpbuf),"READ-ONLY"))
					SetReadOnly (TRUE);
				else if (!strcmp (m_tmpbuf, "READ-WRITE"))
					SetReadOnly(FALSE);
				else if (!strncmp (m_tmpbuf,"UIDVALIDITY ",12))
				{
					m_UidValidity = strtoul (m_tmpbuf+12, NIL, 10);
					return;
				}
				else if (!strncmp (m_tmpbuf,"PERMANENTFLAGS (",16))
				{
					if (m_tmpbuf[i-1] == ')')
						m_tmpbuf[i-1] = '\0';

					m_bPermSeen = m_bPermDeleted = m_bPermAnswered =
					m_bPermDraft = m_bKwdCreate = NIL;

					m_PermUserFlags = 0;

					if (s = strtok (m_tmpbuf+16," "))
					do
					{
						if (!strcmp (s,"\\SEEN")) m_bPermSeen = T;
						else if (!strcmp (s,"\\DELETED")) m_bPermDeleted = T;
						else if (!strcmp (s,"\\FLAGGED")) m_bPermFlagged = T;
						else if (!strcmp (s,"\\ANSWERED")) m_bPermAnswered = T;
						else if (!strcmp (s,"\\DRAFT")) m_bPermDraft = T;
						else if (!strcmp (s,"\\*")) m_bKwdCreate = T;
						else m_PermUserFlags |= ParseUserFlag (s);
					} while (s = strtok (NIL," "));
					return;
				}
				// Handle alerts.
				else if (!strncmp (m_tmpbuf,"ALERT", 5))
				{
					mm_alert (reply->text);
				}
			}

			mm_notify (reply->text,(long) NIL);
		}
		else if (!strcmp (reply->key,"NO"))
		{
			mm_notify (reply->text,WARN);
		}
		else if (!strcmp (reply->key,"BYE"))
		{
			m_bByeseen = T;	/* note that a BYE seen */
			mm_notify (reply->text,BYE);
		}
		else if (!strcmp (reply->key,"BAD"))
		{
			mm_notify (reply->text,IMAPERROR);
		}
		else if (!strcmp (reply->key,"CAPABILITY"))
		{
			/* only do something if have text */
			if (reply->text && (t = (char *) strtok (ucase (reply->text)," ")))
			do 
			{
				if (!strcmp (t,"IMAP4"))
					m_bImap4 = T;
				else if (!strcmp (t,"IMAP4REV1"))
					m_bImap4rev1 = T;
				else if (!strcmp (t,"SCAN"))
					m_bUseScan = T;
				else if (!strncmp (t,"AUTH",4) && ((t[4] == '=') || (t[4] == '-')) &&
					(i = mail_lookup_auth_name(t+5)) && (--i < MAXAUTHENTICATORS))	
						m_UseAuth |= (1 << i);

				/* unsupported IMAP4 extension */
				else if (!strcmp (t,"STATUS")) m_bUseStatus = T;
				/* ignore other capabilities */
			}  while (t = (char *) strtok (NIL," "));
		}
		else
		{
			sprintf (m_tmpbuf , "Unexpected unsolicited message: %.80s", reply->key);
			mm_log (m_tmpbuf, WARN);
		}
	}

	else
	{	
		// if numeric, a keyword follows
		// deposit null at end of keyword.
		//
		keyptr = ucase ((char *) strtok (reply->text," "));

		// and locate the text after it.
		//
		txtptr = (char *) strtok (NIL,"\n");

		/* now take the action */
		/* change in size of mailbox */
		if (!strcmp (keyptr,"EXISTS"))
		{
			mm_exists (msgno);
		}
		else if (!strcmp (keyptr,"RECENT"))
		{
			mm_recent (msgno);
		}
		else if (!strcmp (keyptr,"EXPUNGE"))
		{
			mm_expunged (msgno);
		}
		else if (!strcmp (keyptr,"FETCH"))
		{
			ParseData (msgno,txtptr,reply);
		}
		/* obsolete alias for FETCH */
		else if (!strcmp (keyptr,"STORE"))
		{
			ParseData (msgno,txtptr,reply);
		}
				/* obsolete response to COPY */
		else if (strcmp (keyptr,"COPY"))
		{
			sprintf (m_tmpbuf, "Unknown message data: %lu %.80s", msgno, keyptr);
			mm_log (m_tmpbuf, WARN);
		}
	}
}





/* IMAP parse data
 * Accepts: MAIL stream
 *	    message #
 *	    text to parse
 *	    parsed reply
 *
 *  This code should probably be made a bit more paranoid about malformed
 * S-expressions.
 */

void CProtocol::ParseData (unsigned long msgno, char *text, IMAPPARSEDREPLY *reply)
{
	char *prop;
	MESSAGECACHE *elt = MailElt ();
	
	if (!elt)
		return;

	// msgno is unused::
	if (msgno);

	// Must have a reply.
	if (!reply)
		return;


	++text;			/* skip past open parenthesis */

	/* parse Lisp-form property list */
	//
	while (prop = (char *) strtok (text," )"))
	{
				/* point at value */
		text = (char *) strtok (NIL,"\n");

		/* parse the property and its value */
		//
		ParseProp (elt, ucase (prop), &text, reply);

		// Did we croak?
		if ( !IsConnected() )
			break;
	}
}




/* IMAP parse property
 * Accepts: MAIL stream
 *	    cache item
 *	    property name
 *	    property value text pointer
 *	    parsed reply
 */

void CProtocol::ParseProp (MESSAGECACHE *elt, char *prop, char **txtptr, IMAPPARSEDREPLY *reply)
{
	char *s;
	ENVELOPE **env;
	BODY **body;
	GETS_DATA md;

	// SAanity:
	if (!reply)
		return;

	INIT_GETS (md, this, elt->msgno, NIL, 0, 0);

	if (!strcmp (prop,"ENVELOPE"))
	{
		env = &elt->privat.msg.env;

		ParseEnvelope (env, txtptr, reply);
	}
	else if (!strcmp (prop,"FLAGS"))
	{
		ParseFlags (elt, txtptr);

		// Did we fail?
		if ( !IsConnected() )
			return;

		if ( elt->msgno && elt->privat.uid )
		{
			mm_uid (elt->msgno, elt->privat.uid);
		}

		// JOK (6/23/97) Pass the elt to top level to record the info.

		// Do this only if the uid has already been obtained and flags are valid.
		if (elt->privat.uid && elt->valid)
		{
			mm_elt_flags (elt);

			// After we have notified the upper layers, reset the elt's status
			// because it mey be re-used.
			elt->privat.uid = 0;
			elt->valid = NIL;
		}

	} 

	else if (!strcmp (prop,"INTERNALDATE"))
	{
		if (s = ParseString (txtptr, reply, NIL,NIL))
		{
			if (!mail_parse_date (elt,s))
			{
				sprintf (m_tmpbuf, "Bogus date: %.80s",s);
				mm_log (m_tmpbuf,WARN);
			}
			fs_give ((void **) &s);
		}
	}

	else if (!strcmp (prop,"UID")) /* unique identifier */
	{
		elt->privat.uid = strtoul (*txtptr,txtptr,10);

		// Update our msgno-to-uid map.
		//
		if ( elt->msgno && elt->privat.uid )
		{
			mm_uid (elt->msgno, elt->privat.uid);
		}

		// JOK (6/23/97) Pass the elt to top level to record the info.

		// Do this only if the uid has already been obtained and flags are valid.
		if (elt->privat.uid && elt->valid)
		{
			mm_elt_flags (elt);

			// After we have notified the upper layers, reset the elt's status
			// because it mey be re-used.
			elt->privat.uid = 0;
			elt->valid = NIL;
		}
	}

	else if (!strcmp (prop,"RFC822.HEADER") || !strcmp (prop,"BODY[HEADER]") ||
		!strcmp (prop,"BODY.PEEK[HEADER]")  || !strcmp (prop,"BODY[0]"))
	{
		if (elt->privat.msg.header.text.data)
			fs_give ((void **) &elt->privat.msg.header.text.data);

		md.what = "HEADER";

		// There must be a mailgets to slurp this out as well.
		ParseString (txtptr, reply, &md, &elt->privat.msg.header.text.size);
	}

	else if (!strcmp (prop, "RFC822.SIZE"))
	{
		elt->rfc822_size = strtoul (*txtptr,txtptr,10);
	}
	else if (!strcmp (prop,"RFC822.TEXT") || !strcmp (prop,"BODY[TEXT]") ||
			 !strcmp (prop,"BODY.PEEK[TEXT]"))
	{
		if (elt->privat.msg.text.text.data)
			fs_give ((void **) &elt->privat.msg.text.text.data);
		md.what = "TEXT";

		// This stuff gets slurped out via a mailgets.
		ParseString (txtptr, reply, &md, &elt->privat.msg.text.text.size);
	}

	else if (!strcmp (prop,"RFC822") || !strcmp (prop,"BODY[]") ||
			 !strcmp (prop,"BODY.PEEK[]"))
	{
	    if (elt->privat.msg.full.text.data)
			fs_give ((void **) &elt->privat.msg.full.text.data);

		md.what = "";

		// JOK - Again, this stuff gets slurped out via a mailgets.
		ParseString (txtptr, reply, &md, &elt->privat.msg.full.text.size);
	}

	else if (prop[0] == 'B' && prop[1] == 'O' && prop[2] == 'D' && prop[3] == 'Y')
	{
		s = cpystr (prop+4);	/* copy segment specifier */

		body = &elt->privat.msg.body;

		ParseBody ( &md, body, s, txtptr, reply);

		fs_give ((void **) &s);
	}

	else
	{
	    sprintf (m_tmpbuf, "Unknown message property: %.80s", prop);
		mm_log (m_tmpbuf, WARN);
	}
}






/* IMAP parse envelope
 * Accepts: MAIL stream
 *	    pointer to envelope pointer
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer
 */

void CProtocol::ParseEnvelope (ENVELOPE **env, char **txtptr, IMAPPARSEDREPLY *reply)
{
	ENVELOPE *oenv = *env;
	char c;

	// Make sure:
	if (! (env && reply) )
		return;

	if (! (txtptr && *txtptr) )
		return;

	c = *((*txtptr)++);	/* grab first character */

	/* ignore leading spaces */
	while (c == ' ') c = *((*txtptr)++);

	switch (c)
	{			/* dispatch on first character */
		case '(':			/* if envelope S-expression */
			*env = mail_newenvelope ();	/* parse the new envelope */
			(*env)->date = ParseString (txtptr, reply,NIL,NIL);
			(*env)->subject = ParseString (txtptr, reply, NIL, NIL);
			(*env)->from = ParseAdrlist (txtptr, reply);
			(*env)->sender = ParseAdrlist (txtptr, reply);
			(*env)->reply_to = ParseAdrlist (txtptr, reply);
			(*env)->to = ParseAdrlist (txtptr, reply);
			(*env)->cc = ParseAdrlist (txtptr, reply);
			(*env)->bcc = ParseAdrlist (txtptr, reply);
			(*env)->in_reply_to = ParseString (txtptr, reply, NIL, NIL);
			(*env)->message_id = ParseString (txtptr, reply, NIL, NIL);

			if (oenv)
			{			/* need to merge old envelope? */
				(*env)->newsgroups = oenv->newsgroups;
				oenv->newsgroups = NIL;
				(*env)->followup_to = oenv->followup_to;
				oenv->followup_to = NIL;
				(*env)->references = oenv->references;
				oenv->references = NIL;
				mail_free_envelope(&oenv);/* free old envelope */
			}

			if (**txtptr != ')')
			{
				sprintf (m_tmpbuf,"Junk at end of envelope: %.80s",*txtptr);
				mm_log (m_tmpbuf, WARN);
			}
			else
				++*txtptr;		/* skip past delimiter */

			break;

		case 'N':			/* if NIL */
		case 'n':
			++*txtptr;			/* bump past "I" */
			++*txtptr;			/* bump past "L" */
			break;

		default:
			sprintf (m_tmpbuf, "Not an envelope: %.80s",*txtptr);
			mm_log (m_tmpbuf,WARN);
			break;
	}
}







/* IMAP parse address list
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: address list, NIL on failure
 *
 * Updates text pointer
 */

ADDRESS* CProtocol::ParseAdrlist (char **txtptr, IMAPPARSEDREPLY *reply)
{
  ADDRESS *adr = NIL;
  char c = **txtptr;		/* sniff at first character */
				/* ignore leading spaces */
  while (c == ' ') c = *++*txtptr;
  ++*txtptr;			/* skip past open paren */
  switch (c) {
  case '(':			/* if envelope S-expression */
    adr = ParseAddress (txtptr, reply);
    if (**txtptr != ')') {
      sprintf (m_tmpbuf, "Junk at end of address list: %.80s",*txtptr);
      mm_log (m_tmpbuf, WARN);
    }
    else ++*txtptr;		/* skip past delimiter */
    break;
  case 'N':			/* if NIL */
  case 'n':
    ++*txtptr;			/* bump past "I" */
    ++*txtptr;			/* bump past "L" */
    break;
  default:
    sprintf (m_tmpbuf, "Not an address: %.80s",*txtptr);
    mm_log (m_tmpbuf,WARN);
    break;
  }
  return adr;
}







/* IMAP parse address
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: address, NIL on failure
 *
 * Updates text pointer
 */

ADDRESS* CProtocol::ParseAddress (char **txtptr, IMAPPARSEDREPLY *reply)
{
	ADDRESS *adr = NIL;
	ADDRESS *ret = NIL;
	ADDRESS *prev = NIL;
	char c = **txtptr;		/* sniff at first address character */

	switch (c)
	{
	case '(':			/* if envelope S-expression */
		while (c == '(')
		{		/* recursion dies on small stack machines */
			++*txtptr;		/* skip past open paren */
			if (adr) prev = adr;	/* note previous if any */
			adr = mail_newaddr ();	/* instantiate address and parse its fields */
			adr->personal = ParseString (txtptr, reply, NIL, NIL);
			adr->adl = ParseString (txtptr, reply, NIL, NIL);
			adr->mailbox = ParseString (txtptr, reply, NIL, NIL);
			adr->host = ParseString (txtptr, reply, NIL, NIL);

			if (**txtptr != ')')
			{	/* handle trailing paren */
				sprintf (m_tmpbuf,"Junk at end of address: %.80s",*txtptr);
				mm_log (m_tmpbuf,WARN);
			}
			else
				++*txtptr;		/* skip past close paren */

			c = **txtptr;		/* set up for while test */

				/* ignore leading spaces in front of next */
			while (c == ' ')
				c = *++*txtptr;

			if (!ret) ret = adr;	/* if first time note first adr */

				/* if previous link new block to it */
			if (prev) prev->next = adr;
		}
		break;

		case 'N':			/* if NIL */
		case 'n':
			*txtptr += 3;		/* bump past NIL */
		break;

		default:
			sprintf (m_tmpbuf, "Not an address: %.80s",*txtptr);
			mm_log (m_tmpbuf, WARN);
		break;
	}

	return ret;
}







// ParseFlags [PRIVATE]
//
// * IMAP parse flags
// * Accepts: current message cache
// *	    current text pointer
// *
// * Updates text pointer
//

void CProtocol::ParseFlags (MESSAGECACHE *elt, char **txtptr)
{
	char *flag;
	char c = '\0';

	elt->valid = T;		/* mark have valid flags now */

	elt->user_flags = NIL;	/* zap old flag values */
	elt->seen = elt->deleted = elt->flagged = elt->answered = elt->recent = NIL;

	while (c != ')')
	{
		/* parse list of flags */
		/* point at a flag */

		while (*(flag = ++*txtptr) == ' ');

		/* scan for end of flag */
		while (**txtptr != ' ' && **txtptr != ')')
			++*txtptr;

		c = **txtptr;		/* save delimiter */
		**txtptr = '\0';		/* tie off flag */

		if (!*flag)
		{
			break;		/* null flag */
		}
		/* if starts with \ must be sys flag */
		else if (*ucase (flag) == '\\')
		{
			if (!strcmp (flag,"\\SEEN")) elt->seen = T;
			else if (!strcmp (flag,"\\DELETED")) elt->deleted = T;
			else if (!strcmp (flag,"\\FLAGGED")) elt->flagged = T;
			else if (!strcmp (flag,"\\ANSWERED")) elt->answered = T;
			else if (!strcmp (flag,"\\RECENT")) elt->recent = T;
			else if (!strcmp (flag,"\\DRAFT")) elt->draft = T;
		}
		/* otherwise user flag */
		else
			elt->user_flags |= ParseUserFlag (flag);
	}

	++*txtptr;			/* bump past delimiter */

	mm_flags (elt->msgno);	/* make sure top level knows */
}





// ParseUserFlag [PRIVATE]
//
/* IMAP parse user flag
 * Accepts: MAIL stream
 *	    flag name
 * Returns: flag bit position
 */

unsigned long CProtocol::ParseUserFlag (char *flag)
{
  char tmp[MAILTMPLEN];
  long i;
				/* sniff through all user flags */
	for (i = 0; i < NUSERFLAGS; ++i)
	{
		if (m_UserFlags[i] &&
			!strcmp (flag,ucase (strcpy (tmp, m_UserFlags[i]))))
		{
			return (1 << i);		/* found it! */
		}
	}

	return (unsigned long) 0;	/* not found */
}






// ParseString [PRIVATE]
//
/* IMAP parse string
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 *	    mailgets data
 *	    returned string length
 * Returns: string
 *
 * Updates text pointer
 */

// NOTE: (JOK)
// If this returns a non-NULL value, the memory pointed to by that
// address MUST be freed by the caller.
// END NOTE.

char* CProtocol::ParseString (char **txtptr, IMAPPARSEDREPLY *reply, GETS_DATA *md, unsigned long *len)
{
	char *st = NULL;
	char *string = NULL;
	unsigned long i,j;
	char c;
	mailgets_t mg = NULL;

	// JOK :  Must have a stream 
	if (! (reply && txtptr && *txtptr) )
	{
		ASSERT (0);
		return NULL;
	}
	// END JOK

	// If the stream got disconnected for whatever reason, don't read anymore"
	//
	if ( !IsConnected() )
	{
		return NULL;
	}

	c = **txtptr;		/* sniff at first character */

	// NOTE: mg CAN be NIL. (JOK)
	//
	mg = GetMailGets ();
   
				/* ignore leading spaces */
	while (c == ' ') c = *++*txtptr;

	st = ++*txtptr;		/* remember start of string */

	switch (c)
	{
		case '"':			/* if quoted string */
			i = 0;			/* initial byte count */

			/* search for end of string */
			while (**txtptr != '"')
			{	
				if (**txtptr == '\\') ++*txtptr;
				++i;			/* bump count */

				++*txtptr;		/* bump pointer */
			}

			++*txtptr;			/* bump past delimiter */

			// JOK - If we have a mailgets, do that instead.
			if (md && mg)
			{		
				md->flags |= MG_COPY;/* otherwise flag need to copy */

				// Allocate a ParenStr data object to send data to caller.
				ParenStrData StrData;

				StrData.s = st;
				StrData.size = i;

				(*mg) (str_getbuffer, &StrData, i, md);

				// Doesn't return a value.
				string = NULL;
			}
			/* else must copy into free storage */
			else
			{			
				string = (char *) fs_get ((size_t) i + 1);

				/* copy the string */
				for (j = 0; j < i; j++)
				{	
					if (*st == '\\') ++st;	/* quoted character */
					string[j] = *st++;
				}
				string[j] = '\0';		/* tie off string */
			}

			if (len) *len = i;		/* set return value too */
		break;

		case 'N':			/* if NIL */
		case 'n':
			++*txtptr;			/* bump past "I" */
			++*txtptr;			/* bump past "L" */
			if (len) *len = 0;
		break;

		case '{':			/* if literal string */
				/* get size of string */
			i = strtoul (*txtptr,txtptr,10);
			if (len) *len = i;		/* set return value */

			/* have special routine to slurp string? */
			if (md && mg)
			{		
				/* partial fetch? */
				if (md->first)
				{		
					md->first--;		/* restore origin octet */
					md->last = i;		/* number of octets that we got */
				}
				else
					md->flags |= MG_COPY;/* otherwise flag need to copy */

				(*mg) (net_getbuffer, this, i, md);

				// Doesn't return a value.
				string = NULL;
			}
			/* must slurp into free storage */
			else
			{			
				string = (char *) fs_get ((size_t) i + 1);
				*string = '\0';		/* init in case getbuffer fails */

				/* get the literal */
				net_getbuffer (this, i, string);
			}

			fs_give ((void **) &reply->line);

			// If the stream got disconnected for whatever reason, don't read anymore"
			//
			if ( !IsConnected() )
				break;

			/* get new reply text line */
		    reply->line = Getline ();

			if (reply->line)
			{
				mm_dlog (reply->line);
			}

			*txtptr = reply->line;	/* set text pointer to point at it */
		break;

		default:
			sprintf (m_tmpbuf, "Not a string: %c%.80s",c,*txtptr);
			mm_log (m_tmpbuf, WARN);

			if (len)
				*len = 0;
		break;
	}

	return string;
}







// ParseBody [PRIVATE]
//
/* IMAP parse body structure or contents
 * Accepts: mailgets_data
 *	    pointer to body pointer
 *	    pointer to segment
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer, stores body
 */

void CProtocol::ParseBody (GETS_DATA *md, BODY **body, char *seg, char **txtptr,
		      IMAPPARSEDREPLY *reply)
{
	char *s;
	unsigned long size;
	STRINGLIST *stl = NIL;
	char *tmp = m_tmpbuf;
	MESSAGECACHE *elt;

	// Must have these:
	//
	if (! (md && body && seg && txtptr && *txtptr) )
		return;

	// Get the stream's "m_pCurrentElt" (JOK)
	// This will allocate one if necessary.
	elt = MailElt ();	
	if (!elt)
		return;

	/* dispatch based on type of data */
	switch (*seg++)
	{		
		case 'S':			/* extensible body structure */
			if (strcmp (seg,"TRUCTURE"))
			{
				sprintf (tmp, "Bad body fetch: %.80s",seg);
				mm_log (tmp, WARN);
				return;
			}

			/* falls through */
		case '\0':			/* body structure */
			mail_free_body (body);	/* flush any prior body */

				/* instantiate and parse a new body */
			ParseBodyStructure (*body = mail_newbody(), txtptr, reply);
			break;

		// JOK: 4/24/98 - Added .PEEK because the Novell groupwise server returns BODY.PEEK!!!
		case '.':
			ucase (seg);		/* make sure uppercase */

			// Better be this:
			if (strncmp (seg,"PEEK[", 5))
			{
				sprintf (tmp, "Bad body fetch: %.80s",seg);
				mm_log (tmp, WARN);
				return;
			}
			// Othersize, go pass PEEK[ and fall through:
			seg += 5;

		case '[':			/* body section text */
			ucase (seg);		/* make sure uppercase */

			/* header lines case? */
			if (!(s = strchr(seg,']')))
			{
				/* skip leading nesting */
				for (s = seg; *s && (isdigit (*s) || (*s == '.')); s++);

				/* better be one of these */
				if (strcmp (s, "HEADER.FIELDS") && strcmp (s,"HEADER.FIELDS.NOT"))
				{
					sprintf (tmp, "Unterminated section specifier: %.80s",seg);
					mm_log (tmp, WARN);
					return;
				}

				/* get list of headers */
				if (!(stl = ParseStringlist (txtptr,reply)))
				{
					sprintf (tmp, "Bogus header field list: %.80s",*txtptr);
					mm_log (tmp, WARN);
					return;
				}

				// JOK - We don't really need the string list!!!
				mail_free_stringlist (&stl);
				// END JOK 

				/* make sure terminated */
				if (**txtptr != ']')
				{	
					sprintf (tmp,"Unterminated header section specifier: %.80s",*txtptr);
					mm_log (tmp, WARN);
					mail_free_stringlist (&stl);
					return;
				}

				/* point after the text */
				if (*txtptr = strchr (s = *txtptr,' '))
					 *(*txtptr)++ = '\0';
			}


		    *s++ = '\0';		/* tie off section specifier */

			/* partial specifier? */
			if (*s == '<')
			{
				md->first = strtoul (s+1,&s,10) + 1;

				/* make sure properly terminated */
				if (*s++ != '>')
				{
					sprintf (tmp,"Unterminated partial data specifier: %.80s",s-1);
					mm_log (tmp,WARN);
					mail_free_stringlist (&stl);
					return;
				}
			}

			/* make sure no junk follows */
		    if (*s)
			{
				sprintf (tmp, "Junk after section specifier: %.80s",s);
				mm_log (tmp, WARN);
				mail_free_stringlist (&stl);
				return;
			}

			md->what = seg;		/* get the body section text */

			// JOK - Calling "imap_parse_string()" will send the data to the caller.
			s = NULL;	// So we won't try to free it.
			ParseString (txtptr, reply, md, &size);

			/* done if partial */
			if (md->first || md->last)
			{
				mail_free_stringlist (&stl);
				return;
			}

			// JOK - Ignore the rest.

			break;

		default:			/* bogon */
		    sprintf (tmp, "Bad body fetch: %.80s",seg);
			mm_log (tmp, WARN);
			return;
	 }  // switch
}





// ParseBodyStructure [PRIVATE]
//
/* IMAP parse body structure
 * Accepts:
 *	    body structure to write into
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer
 */

void CProtocol::ParseBodyStructure (BODY *body, char **txtptr, IMAPPARSEDREPLY *reply)
{
	int i;
	char *s;
	PART *part = NIL;
	char c;

	// Sanity: Must have these:
	if (! (body && txtptr && *txtptr) )
	{
		ASSERT (0);
		return;
	}

	// grab first character
	//
	c = *((*txtptr)++);

	/* ignore leading spaces */
	while (c == ' ') c = *((*txtptr)++);

	/* dispatch on first character */
	switch (c)
	{
		case '(':			/* body structure list */

		    if (**txtptr == '(')		/* multipart body */
			{
				body->type= TYPEMULTIPART;	/* yes, set its type */

				do
				{			/* instantiate new body part */
					if (part)
					{
						part = part->next = mail_newbody_part ();
					}
					else
					{
						body->nested.part = part = mail_newbody_part ();
					}

					/* parse it */
					ParseBodyStructure (&part->body, txtptr, reply);

					/* ignore possible spaces until the next '(' (JOK) */
					while (**txtptr == ' ') ++(*txtptr);

				} while (**txtptr == '(');/* for each body part */

				if (body->subtype = ParseString (txtptr, reply, NIL, NIL))
				{
					ucase (body->subtype);
				}
				else
				{
					// Set it to "Multipart/Mixed":
					body->subtype = cpystr ("Mixed");
					mm_log ("Missing multipart subtype",WARN);
				}

				if (**txtptr == ' ')	/* multipart parameters */
				{
					body->parameter = ParseBodyParameter (txtptr, reply);
				}

				if (**txtptr == ' ')	/* disposition */
				{
					ParseDisposition (body, txtptr, reply);
				}

				if (**txtptr == ' ')	/* language */
					body->language = ParseLanguage (txtptr, reply);

				while (**txtptr == ' ')
					ParseExtension (txtptr, reply);

				if (**txtptr != ')')
				{	/* validate ending */
					sprintf (m_tmpbuf, "Junk at end of multipart body: %.80s",*txtptr);
					mm_log (m_tmpbuf, WARN);
				}
				else
					++*txtptr;		/* skip past delimiter */
			} // ifmultipart body.
			else
			{			/* not multipart, parse type name */
				if (**txtptr == ')')
				{	/* empty body? */
					++*txtptr;		/* bump past it */
					break;			/* and punt */
				}

				body->type = TYPEOTHER;	/* assume unknown type */
				body->encoding = ENCOTHER;/* and unknown encoding */

				/* parse type */
				if (s = ucase (ParseString (txtptr, reply, NIL, NIL)))
				{
					for (i=0; (i<=TYPEMAX) && body_types[i] && strcmp(s,body_types[i]);i++);

					if (i <= TYPEMAX)
					{	/* only if found a slot */
						body->type = (unsigned short)i;	/* set body type */

						if (body_types[i])
							fs_give ((void **) &s);
						else
							body_types[i] = s;	/* assign empty slot */
					}
				}

				body->subtype =		/* parse subtype */
					ucase (ParseString (txtptr, reply, NIL, NIL));

				// If no body->subtype, create a dummy one. (JOK)
				if (!body->subtype)
				{
					body->subtype = cpystr ("PLAIN");
				}

				body->parameter = ParseBodyParameter (txtptr, reply);

				body->id = ParseString (txtptr, reply, NIL, NIL);

				body->description = ParseString (txtptr, reply, NIL, NIL);

				if (s = ucase (ParseString (txtptr, reply, NIL, NIL)))
				{
					/* search for body encoding */
					for (i = 0; (i <= ENCMAX) && body_encodings[i] && strcmp (s,body_encodings[i]); i++)
					{
						// Hack for multiple CTEs for uuencode
						if (i == ENCUUENCODE)
						{
							if (!strcmp(s, "X-UUE") || !strcmp(s, "UUENCODE") || !strcmp(s, "UUE"))
								break;
						}
					}

					if (i > ENCMAX)
					{
						body->encoding = ENCOTHER;
					}
					else
					{			/* only if found a slot */
						body->encoding = (unsigned short) i;	/* set body encoding */

						if (body_encodings[i])
							fs_give ((void **) &s);

						/* assign empty slot */
						else
							body_encodings[i] = s;
					}

					//
					// If the subtype is binhex and the encoding is read as just text,
					// this should be decoded with binhex. Since we only pass an encoding type to
					// "FetchAttachmentContentsToFile" below, set an appropriate encoding flag.
					// 
					CRString szBinhex (IDS_MIME_BINHEX);
					CString  szSubtype = body->subtype;

					if ( (body->subtype != NULL) &&
					     (!strnicmp ( (LPCSTR)szSubtype, (LPCSTR)szBinhex, szBinhex.GetLength() ) ) )
					{
						if ( body->encoding == ENC7BIT || body->encoding == ENCOTHER )
						{
							body->encoding = ENCBINHEX;
						}
					}
				}

				/* parse size of contents in bytes */
				body->size.bytes = strtoul (*txtptr,txtptr,10);

				switch (body->type)
				{	/* possible extra stuff */
					case TYPEMESSAGE:		/* message envelope and body */
						if (strcmp (body->subtype,"RFC822"))
							break;

						body->nested.msg = mail_newmsg ();

						ParseEnvelope (&body->nested.msg->env, txtptr, reply);
						body->nested.msg->body = mail_newbody ();
						ParseBodyStructure(body->nested.msg->body, txtptr, reply);

					/* drop into text case */
					case TYPETEXT:		/* size in lines */
						body->size.lines = strtoul (*txtptr,txtptr,10);
						break;

					default:			/* otherwise nothing special */
						break;
				}

				if (**txtptr == ' ')	/* if extension data */
					body->md5 = ParseString (txtptr, reply, NIL, NIL);

				if (**txtptr == ' ')	/* disposition */
					ParseDisposition (body, txtptr, reply);

				if (**txtptr == ' ')	/* language */
					body->language = ParseLanguage (txtptr, reply);

				while (**txtptr == ' ')
					 ParseExtension (txtptr, reply);

				if (**txtptr != ')')
				{	/* validate ending */
					sprintf (m_tmpbuf, "Junk at end of body part: %.80s",*txtptr);
					mm_log (m_tmpbuf,WARN);
				}
				else
				{
					++*txtptr;		/* skip past delimiter */

					/* ignore possible spaces until the next '(' (JOK) */
					while (**txtptr == ' ') ++(*txtptr);
				}
			}
			break;

		  case 'N':			/* if NIL */
		  case 'n':
			++*txtptr;			/* bump past "I" */
			++*txtptr;			/* bump past "L" */
			break;

		default:			/* otherwise quite bogus */
			sprintf (m_tmpbuf,"Bogus body structure: %.80s",*txtptr);
			mm_log (m_tmpbuf,WARN);
			break;

	}	// switch.
}





// ParseBodyParameter [PRIVATE]
//
/* IMAP parse body parameter
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: body parameter
 * Updates text pointer
 */

PARAMETER* CProtocol::ParseBodyParameter (char **txtptr, IMAPPARSEDREPLY *reply)
{
	PARAMETER *ret = NIL;
	PARAMETER *par = NIL;
	char c,*s;

	/* ignore leading spaces */
	while ((c = *(*txtptr)++) == ' ');

	/* parse parameter list */
	if (c == '(')
	{
		while (c != ')')
		{
			/* append new parameter to tail */
			if (ret)
				 par = par->next = mail_newbody_parameter ();
			else
				ret = par = mail_newbody_parameter ();

			if(!(par->attribute = ParseString (txtptr, reply, NIL, NIL)))
			{
				mm_log ("Missing parameter attribute",WARN);
				par->attribute = cpystr ("UNKNOWN");
			}

			if (!(par->value = ParseString (txtptr, reply, NIL, NIL)))
			{
				sprintf (m_tmpbuf, "Missing value for parameter %.80s",par->attribute);
				mm_log (m_tmpbuf, WARN);
				par->value = cpystr ("UNKNOWN");
			}

			switch (c = **txtptr)
			{
				/* see what comes after */
				case ' ':			/* flush whitespace */
					while ((c = *++*txtptr) == ' ');
					break;

				case ')':			/* end of attribute/value pairs */
					++*txtptr;		/* skip past closing paren */
					break;

				default:
					sprintf (m_tmpbuf,"Junk at end of parameter: %.80s",*txtptr);
					mm_log (m_tmpbuf,WARN);
					break;
			}
		}
	} // if
	/* empty parameter, must be NIL */
	else if (((c == 'N') || (c == 'n')) &&
	   ((*(s = *txtptr) == 'I') || (*s == 'i')) &&
	   ((s[1] == 'L') || (s[1] == 'l')))
	{
		 *txtptr += 2;
	}
	else
	{
		sprintf (m_tmpbuf,"Bogus body parameter: %c%.80s",c,(*txtptr) - 1);
		mm_log (m_tmpbuf, WARN);
	}

	return ret;
}








/* IMAP parse body disposition
 * Accepts: MAIL stream
 *	    body structure to write into
 *	    current text pointer
 *	    parsed reply
 */

void CProtocol::ParseDisposition (BODY *body, char **txtptr, IMAPPARSEDREPLY *reply)
{
	switch (*++*txtptr)
	{
		case '(':
			++*txtptr;			/* skip open paren */
			body->disposition.type = ParseString (txtptr, reply, NIL, NIL);
			body->disposition.parameter =
					ParseBodyParameter (txtptr, reply);

			if (**txtptr != ')')
			{
				/* validate ending */
				sprintf (m_tmpbuf,"Junk at end of disposition: %.80s",*txtptr);
				mm_log (m_tmpbuf,WARN);
			}
			else ++*txtptr;		/* skip past delimiter */

			break;

		case 'N':			/* if NIL */
		case 'n':
			++*txtptr;			/* bump past "N" */
			++*txtptr;			/* bump past "I" */
			++*txtptr;			/* bump past "L" */
			break;

		default:
			sprintf (m_tmpbuf,"Unknown body disposition: %.80s",*txtptr);
			mm_log (m_tmpbuf,WARN);

				/* try to skip to next space */
			while ((*++*txtptr != ' ') && (**txtptr != ')') && **txtptr);

			break;
	}
}







/* IMAP parse body language
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: string list or NIL if empty or error
 */

STRINGLIST* CProtocol::ParseLanguage (char **txtptr, IMAPPARSEDREPLY *reply)
{
	unsigned long i;
	char *s;
	STRINGLIST *ret = NIL;

	/* language is a list */
	if (*++*txtptr == '(')
	{
		ret = ParseStringlist (txtptr, reply);
	}
	else if (s = ParseString (txtptr, reply, NIL, &i))
	{
		(ret = mail_newstringlist ())->text.data = s;
		ret->text.size = i;
	}

	return ret;
}







/* IMAP parse string list
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 * Returns: string list or NIL if empty or error
 */

STRINGLIST* CProtocol::ParseStringlist (char **txtptr, IMAPPARSEDREPLY *reply)
{
	STRINGLIST *stl = NIL;
	STRINGLIST *stc;
	char c,*s;
	char *t = *txtptr;

	/* parse the list */
	if (*t++ == '(')
	{
		while (*t != ')')
		{
			if (stl)
			{
				stc = stc->next = mail_newstringlist ();
			}
			else
			{
				stc = stl = mail_newstringlist ();
			}
		
			/* atom */
			if ((*t != '{') && (*t != '"') && (s = strpbrk (t," )")))
			{
				c = *s;			/* note delimiter */
				*s = '\0';		/* tie off atom and copy it*/
				stc->text.size = strlen (stc->text.data = cpystr (t));

				if (c == ' ')
					t = ++s;	/* another atom follows */
				else
					*(t = s) = c;	/* restore delimiter */
			}
			/* string */
			else if (!(stc->text.data = ParseString (&t, reply, NIL, &stc->text.size)))
			{
				sprintf (m_tmpbuf,"Bogus string list member: %.80s",t);
				mm_log (m_tmpbuf,WARN);
				mail_free_stringlist (&stl);
				break;
			}
		}
	}

	if (stl)
		*txtptr = ++t;	/* update return string */

	return stl;
}







/* IMAP parse unknown body extension data
 * Accepts: MAIL stream
 *	    current text pointer
 *	    parsed reply
 *
 * Updates text pointer
 */

void CProtocol::ParseExtension (char **txtptr, IMAPPARSEDREPLY *reply)
{
	if (!(reply && txtptr && *txtptr))
		return;

	unsigned long i,j;

	switch (*++*txtptr)
	{
		/* action depends upon first character */
		case '(':
			while (**txtptr != ')') ParseExtension (txtptr, reply);
			++*txtptr;			/* bump past closing parenthesis */
			break;

		case '"':			/* if quoted string */
			while (*++*txtptr != '"')
			{
				if (**txtptr == '\\')
					 ++*txtptr;
			}

			++*txtptr;			/* bump past closing quote */
			break;

		case 'N':			/* if NIL */
		case 'n':
			++*txtptr;			/* bump past "N" */
			++*txtptr;			/* bump past "I" */
			++*txtptr;			/* bump past "L" */
			break;

		case '{':			/* get size of literal */

		    ++*txtptr;			/* bump past open squiggle */

			if (i = strtoul (*txtptr,txtptr,10)) do
			{
				Getbuffer (j = max (i,(long)IMAPTMPLEN), m_tmpbuf);
			}
			while ((i -= j) && IsConnected());

			/* get new reply text line */

			reply->line = Getline ();

			if (reply->line)
			{
				mm_dlog (reply->line);
			}

			*txtptr = reply->line;	/* set text pointer to point at it */
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			strtoul (*txtptr,txtptr,10);
			break;

		default:
			sprintf (m_tmpbuf, "Unknown extension token: %.80s",*txtptr);
			mm_log (m_tmpbuf,WARN);

			/* try to skip to next space */
			while ((*++*txtptr != ' ') && (**txtptr != ')') && **txtptr);

			break;
	}  // switch
}






// DoList [PRIVATE]
//
// IMAP find list of mailboxes
//	    reference
//	    pattern to search
//	    list flag
//	    string to scan
//
void CProtocol::DoList (char *ref, char *pat, long list, char *contents)
{
  char *s, prefix[MAILTMPLEN], mbx[MAILTMPLEN];
  IMAPARG *args[4],aref,apat,acont;

	// Must have an open stream (JOK).
	//
	if ( !IsConnected() )
		return;

	/* have a reference? */
	if (ref && *ref)
	{	
		if (strlen (ref) >= sizeof (prefix))
			return;

		strcpy (prefix, ref);	/* build prefix */
		m_Prefix = prefix;	/* note prefix */
	}
	else
	{
		// no prefix.
		//
		m_Prefix = NIL;
	}

	if (contents)
	{		/* want to do a scan? */
		if (m_bUseScan)
		{	
			// can we?
			//
			args[0] = &aref; args[1] = &apat; args[2] = &acont; args[3] = NIL;
			aref.type = ASTRING; aref.text = (void *) (ref ? ref : "");
			apat.type = LISTMAILBOX; apat.text = (void *) pat;
			acont.type = ASTRING; acont.text = (void *) contents;
			Send ("SCAN", args);
		}
		else mm_log ("Scan not valid on this IMAP server",WARN);
	}
	else if (IsImap4 ())
	{
		// easy if IMAP4/
		//
	    args[0] = &aref; args[1] = &apat; args[2] = NIL;
		aref.type = ASTRING; aref.text = (void *) (ref ? ref : "");
		apat.type = LISTMAILBOX; apat.text = (void *) pat;
		Send (list ? "LIST" : "LSUB",args);
	}
	else if (IsImap1176 ())
	{
		// Convert to IMAP2 format wildcard.
		// kludgy application of reference.
		//
		if (ref && *ref)
			sprintf (mbx, "%s%s", ref, pat);
		else
			strcpy (mbx, pat);

		for (s = mbx; *s; s++)
		{
			if (*s == '%') *s = '*';
		}

		args[0] = &apat; args[1] = NIL;
		apat.type = LISTMAILBOX; apat.text = (void *) mbx;

		IMAPPARSEDREPLY *reply;

		if (! ( list &&		/* if list, try IMAP2bis, then RFC-1176 form */
			    ( (reply = Send ("FIND ALL.MAILBOXES", args)) && strcmp (reply->key,"BAD") )	||
			    ( (reply = Send ("FIND MAILBOXES",args)) && strcmp (reply->key,"BAD") ) 
		      )
		   )
		{
			m_bRfc1176 = NIL;	/* must be RFC-1064 */
		}
	}

	m_Prefix = NIL;		/* no more prefix */
}




/* IMAP manage a mailbox
 * Accepts: mail stream
 *	    mailbox to manipulate
 *	    command to execute
 *	    optional second argument
 * Returns: T on success, NIL on failure
 */

RCODE CProtocol::DoManage (char *mailbox, char *command, char *arg2)
{
	IMAPPARSEDREPLY *reply;
	BOOL		bRet = FALSE;
	char		mbx[MAILTMPLEN], mbx2[MAILTMPLEN];
	IMAPARG*	args[3],ambx,amb2;


	ambx.type = amb2.type = ASTRING; ambx.text = (void *) mbx;
	amb2.text = (void *) mbx2;
	args[0] = &ambx; args[1] = args[2] = NIL;

	// Must have a valid open stream.
	if ( !IsConnected() )
		return R_FAIL;

	// Clear these:
	mbx[0] = mbx2[0] = 0;

	strcpy (mbx, mailbox);

    // Also, if second argument, use arg2 as passed in.
	if (arg2)
	{
		strcpy (mbx2, arg2);
		args[1] = &amb2;	/* second arg present? */
	}

	// JOK
	reply = Send (command,args);
	if (reply)
	{
		bRet = ImapOK (reply);

		mm_log (reply->text, bRet ? (long) NIL : IMAPERROR);
	}

	return bRet ? R_OK : R_FAIL;
}





// FetchUids [PRIVATE]
//
/* Accepts:
 *	    sequence
 *	    option flags
 */
// send "FETCH sequence FLAGS".

// HISTORY:
// END HISTORY.

RCODE CProtocol::FetchUids (char *sequence)
{
	char *cmd = NULL;
	IMAPPARSEDREPLY *reply = NIL;
	IMAPARG *args[3],aseq,aatt;
	RCODE rResult = R_FAIL;

	if ( !IsSelected() )
		return R_FAIL;

	// Only for IMAP4 and up.
	//
	if ( !IsImap4() )
		return R_FAIL;

	// Set this while we're in this process.
	//
	m_bRecreatingMessageMap = TRUE;

	// Format command.
	cmd = "FETCH";
	aseq.type = SEQUENCE; aseq.text = (void *) sequence;

	aatt.type = ATOM; aatt.text = (void *) "(UID)";

	args[0] = &aseq; args[1] = &aatt; args[2] = NIL;

	// IF there is a CurrentElt in the stream, delete it.
	if (m_pCurrentElt)
		mail_free_elt (&m_pCurrentElt);

	// Allocate a new one.
	MailElt ();

	// Now this:
	reply = Send (cmd, args);
	if (reply)
	{
		if ( ImapOK (reply) )
			rResult = R_OK;

		if ( !RCODE_SUCCEEDED (rResult) && reply->text)
			mm_log (reply->text,IMAPERROR);
	}

	// Reset!!
	//
	m_bRecreatingMessageMap = FALSE;

	return rResult;
}






// ======================= Static functions =============================//


/* Get challenge to authenticator in binary
 * Accepts: CProtocol object.
 *			pointer to returned size
 * Returns: challenge or NIL if not challenge
 */

// This is a stub to route the call to the appropriate CProtocol object.
// Should use a class callback here.
//
void *imap_challenge (void *s,  // Set this to a CProtocol object.
					  unsigned long *len
					 )
{
	CProtocol* pProtocol = (CProtocol *) s;

	if (pProtocol)
		return pProtocol->ImapChallenge (len);
	else
		return NULL;
}



/* Send authenticator response in BASE64
 * Accepts: CProtocol object.
 *			string to send
 *			length of string
 * Returns: T if successful, else NIL
 */

BOOL imap_response (void *s,	// Set this to a CProtocol object.
					char *response,
					unsigned long size)
{
	CProtocol* pProtocol = (CProtocol *) s;

	if (pProtocol)
		return pProtocol->ImapResponse (response, size);
	else
		return FALSE;
}



// Static callback funtion
//
BOOL net_getbuffer (void *st, unsigned long size, char *buffer)
{
	CNetStream *stream = (CNetStream *) st;

	if (stream)
	  return stream->Getbuffer (size, buffer);

	return FALSE;
}



//
// Callback to read data from a string.
// st is a pointer to a "ParenStrData" data structure.
//
BOOL str_getbuffer (void *st, unsigned long size, char *buffer)
{
	ParenStrData *pStr = ( ParenStrData *) st;

	// Must have a ParenStrData structure.
	if (!pStr)
		return FALSE;

	// Must have a buffer.
	if (!buffer)
		return FALSE;

	// Must have a tringf pointer.
	if (!pStr->s)
		return FALSE;

	// Initialize this:
	*buffer = 0;

	// Internal size must be positive.
	if (pStr->size == 0)
		return TRUE;

	unsigned long len = min (pStr->size, size);

	strncpy (buffer, pStr->s, len);

	// Tie off.
	buffer[len] = 0;

	// Update pStr:
	pStr->s += len;

	pStr->size -= len;

	return TRUE;
}




#endif // IMAP4