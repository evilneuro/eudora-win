// Proto.h - definitions of the IMAP protocol class.

//
// Notes:
// This class is an encapsulation of the imap_* functions that used to be in imap4r1.cpp.
//

#ifndef __IMAPPROTO_H
#define __IMAPPROTO_H

#include "QCNetSettings.h"

#include "Stream.h"
#include "network.h"

#include "sslsettings.h"

// For now:
//
typedef unsigned long RCODE;
#define 	R_FAIL		0x1FFFFFFF
#define 	R_OK		0x0

#define		R_LOGIN_FAILED   0x10000001


#if 0 // OLD defs:
typedef enum
{
 	R_FAILED	= 0x1FFFFFFF,
	R_OK		= 0x0
} RCODE;
#endif


// Return code macros.
//
#define RCODE_FAILED(rc) \
	((rc & 0xF0000000) != 0)

#define RCODE_SUCCEEDED(rc) \
	((rc & 0xF0000000) == 0)


// Inherit this from CStream AND from CNetStream so we get all the generic MAILSTREAM and
// network goodies.
//
class CProtocol : public CStream, public CNetStream
{
public:

	CProtocol(LPCSTR pServer, LPCSTR pPortnum);
	virtual ~CProtocol();

// public methods:
	void Scan (char *ref, char *pat, char *contents);

	void List (char *ref, char *pat);

	void Lsub (char *ref, char *pat);

	RCODE Subscribe	(char *mailbox);
	RCODE Unsubscribe (char *mailbox);
	RCODE Create		(char *mailbox);

	RCODE Delete (char *mailbox);

	RCODE Rename (char *old, char *newname);

	RCODE Status (LPCSTR pMailbox, long flags);

	RCODE Open (LPCSTR pMailbox, unsigned long options);

	BOOL  Authenticate (char *tmp, char *usr);

	BOOL Login (char *tmp, char *usr);

	void Close (long options);

	RCODE FetchFast (char *sequence, long flags);

	RCODE FetchFlags (char *sequence, long flags);

	ENVELOPE *FetchEnvelope (unsigned long Uid, long flags);

	BODY *FetchStructure (unsigned long Uid, long flags);

	RCODE FetchHeader (unsigned long msgno, char *szSequence, char *section,
				STRINGLIST *lines, long flags, ENVELOPE **ppEnvelope);

	RCODE FetchMessage (unsigned long msgno, long flags);

	RCODE FetchText (unsigned long msgno, char *section, long flags);

	RCODE PartialBody (unsigned long msgno, char *section,
			unsigned long first, unsigned long last, long flags);

	RCODE FetchBody (unsigned long msgno, char *section, long flags);

	RCODE FetchMIME (unsigned long msgno, char *section, long flags);

	unsigned long GetUid (unsigned long msgno);

	unsigned long GetMsgno (unsigned long uid);

	unsigned long FetchRfc822size (unsigned long msgno, long flags);
	unsigned long FetchRfc822size (char *szSequence, unsigned long *pulSizes, int iNumMsgs, long flags);

	RCODE SetFlags (LPCSTR pUidList, char* sFlagList, long flags);

	void Search (char *charset, SEARCHPGM *pgm, long flags, CString& szResults);

	RCODE Ping ();

	void Check ();

	RCODE	UIDExpunge (LPCSTR pUidList, CString& szUidsActuallyRemoved);

	void Expunge (CString& szUidsActuallyRemoved);

	RCODE Copy (char *sequence, char *mailbox, long flags, CDWordArray *dwaNewUIDs);

	RCODE Append (char *mailbox, char *flags, char *date, STRING *msg, CDWordArray *dwaNewUIDs);

	void StoreUIDPLUSResponses(IMAPPARSEDREPLY *reply, CDWordArray *dwaNewUIDs);
	void VerifyUIDValidity(char *szUids, int iLen);

	MESSAGECACHE* MailElt ();

	void Notify (NOTIFY_TYPE notify_type, void *data)
		{ StreamNotify (notify_type, data); }

	// Overridden from CNetStream.
	//
	virtual void NetNotify (NOTIFY_TYPE notify_type, void *data);
	virtual void NetReportError (LPCSTR pStr, UINT Type);
	virtual void NetLog	(char *string,long errflg);

	// Must be public.
	//
	void*  ImapChallenge (unsigned long *len);
	BOOL   ImapResponse  (char *response, unsigned long size);

	void Stop ();

	// Clear the msgno-to-uid map and recreate it.
	//
	RCODE RecreateMessageMapMT ();

//  Private methods:
private:

	RCODE SelectMailbox (LPCSTR pMailbox);

	void DoList (char *ref, char *pat, long list, char *contents);

	RCODE DoManage (char *mailbox, char *command, char *arg2);

	RCODE Msgdata (unsigned long msgno, char *szSequence, char *section,
				  unsigned long first,unsigned long last,STRINGLIST *lines,
				  long flags, ENVELOPE **ppEnvelope);

	RCODE __SetFlags (char *sequence, char *flag, long flags);

	void GC (long gcflags);

	void GC_body (BODY *body);


	IMAPPARSEDREPLY *Send (char *cmd,IMAPARG *args[]);

	IMAPPARSEDREPLY *SendAstring (char *tag, char **s, char *t, unsigned long size, long wildok);

	IMAPPARSEDREPLY *SendLiteral (char *tag, char **s, STRING *st);

	IMAPPARSEDREPLY *SendSpgm (char *tag, char **s, SEARCHPGM *pgm);

	void SendSset (char **s, SEARCHSET *set);

	IMAPPARSEDREPLY *SendSlist (char *tag, char **s, char *name, STRINGLIST *list);

	void SendSdate (char **s, char *name,unsigned short date);

	IMAPPARSEDREPLY *ImapSout (char *tag, char *base, char **s);

	BOOL ImapSoutr (char *string);

	IMAPPARSEDREPLY *GetReply (char *tag);

	IMAPPARSEDREPLY *ParseReply (char *text);

	IMAPPARSEDREPLY *Fake (char *tag, char *text);

	BOOL ImapOK (IMAPPARSEDREPLY *reply);

	void ParseUnsolicited (IMAPPARSEDREPLY *reply);

	void ParseData (unsigned long msgno, char *text, IMAPPARSEDREPLY *reply);

	void ParseProp (MESSAGECACHE *elt, char *prop, char **txtptr, IMAPPARSEDREPLY *reply);

	void ParseEnvelope (ENVELOPE **env, char **txtptr, IMAPPARSEDREPLY *reply);

	MAILADDRESS *ParseAdrlist (char **txtptr, IMAPPARSEDREPLY *reply);

	MAILADDRESS *ParseAddress (char **txtptr, IMAPPARSEDREPLY *reply);

	void ParseFlags (MESSAGECACHE *elt, char **txtptr);

	unsigned long ParseUserFlag (char *flag);

	char *ParseString (char **txtptr, IMAPPARSEDREPLY *reply,
			 GETS_DATA *md, unsigned long *len);

	void ParseBody (GETS_DATA *md,BODY **body,char *seg,char **txtptr, IMAPPARSEDREPLY *reply);

	void ParseBodyStructure (BODY *body, char **txtptr, IMAPPARSEDREPLY *reply);

	PARAMETER *ParseBodyParameter (char **txtptr, IMAPPARSEDREPLY *reply);

	void ParseDisposition (BODY *body, char **txtptr, IMAPPARSEDREPLY *reply);

	STRINGLIST *ParseLanguage (char **txtptr, IMAPPARSEDREPLY *reply);

	STRINGLIST *ParseStringlist (char **txtptr, IMAPPARSEDREPLY *reply);

	void ParseExtension (char **txtptr, IMAPPARSEDREPLY *reply);

		// Used by "RecreateMessageMap".
	//
	RCODE FetchUids (char *sequence);

	BOOL IsRecreatingMessageMap()
		{ return m_bRecreatingMessageMap; }


	// Public Access:
	//
public:
	// Mail status.
	//
	MAILSTATUS* GetMailStatus ()
		{ return &m_MailStatus; }

	void ClearMailStatus ();
    SSLSettings m_SSLSettings;
	CString m_Persona;

// Attributes:
//
private:

	MESSAGECACHE	*m_pCurrentElt;

	// Store mail status command results in this. See "GetMailStatus()" above
	// where we return a pointer to this member.
	//
	MAILSTATUS	m_MailStatus;

	BOOL m_bRecreatingMessageMap;

	long		 m_lUIDValidity;

	CString			 m_strData;		// For grabbing extra data along the way.  This is used for grabbing
									// info on ranges of messages.
	unsigned int	 m_iDataType;	// What type of data is gathered in m_strData.
};




#endif // 