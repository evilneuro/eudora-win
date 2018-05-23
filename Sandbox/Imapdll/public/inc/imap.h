// imap.h 
//

// Declarations for the CImap* classes.
//
// Notes:
// This file is the main C++ interface to the imap.dll and exports the following:
// 1. CIMAP class - Does low-level IMAP fetch, store, etc for a single mailbox or
//    control stream.


#ifndef _IMAP_H_
#define _IMAP_H_

#define DllExport	__declspec( dllexport )

// Typedefs used internally and also exported:
//
#include "exports.h"


// ================ New for Eudora  =====================//

// For passing IMAP flag info.
// Note: NEED to extend this to handle Eudora's flags via annotations. In that
// case, we'd probably need a way to indicate which flags IMAP can handle (via annotations).
typedef struct
{
	BOOL	DELETED;
	BOOL	SEEN;
	BOOL	FLAGGED;
	BOOL	ANSWERED;
	BOOL	DRAFT;
	BOOL	RECENT;
} IMAPFLAGS;


// IMAPFULL structure for returning info from FetchFast, FetchFlags and FetchFull.

typedef struct
{
	IMAPFLAGS		*Flags;
	char			*InternalDate;
	ENVELOPE		*Env;
	BODY			*Body;
} IMAPFULL;



// Forward class declarations.
	class CCImap;
	class CMImap;

// CWriter: abstract base class for saving data returned from the IMAP server.
class CWriter
{
public:
	CWriter (){}

// These functions must be overridden:
	virtual  BOOL   Write   (readfn_t readfn, void * read_data, unsigned long size) = 0;
};


// CReader: abstract base class for fetching data and sending it to the IMAP server.

const unsigned long  BufferSize = 4096;

class CReader
{
public:
	CReader (){}

// These functions must be overridden:
	virtual  unsigned long	GetTotalSize () = 0;
	virtual	 long 			Read	(char *buffer, ULONG BufferSize) = 0;
	virtual	 BOOL			Done	() = 0;
};


class CLister
{
public:
	CLister(){}

// These methods MUST be overridden:
	virtual BOOL	AddMailbox (char *name, int delimiter, long attributes) = 0;
};



class  CIMLogin
{
public:
	CIMLogin(){}

// These methods MUST be overridden:
	virtual BOOL	Login (char *user, char *pwd, long trial) = 0;
#if 0 	
	  {return FALSE;}
#endif
};


// This class is used to fetch flags, timestamps, etc., about messages. 
class CEltWriter
{
public:
	CEltWriter (){}

	// This must be overridden.
	virtual BOOL  SetFlag (	unsigned long uid,
							unsigned int seen,
							unsigned int deleted,
							unsigned int flagged,
							unsigned int answered,
							unsigned int draft,
							unsigned int recent) = 0;
};


// ==========================================================================
// Exported functions ==============
void DllExport SetDebugLog (void (*fn) (char *, long));
void DllExport SetTimeoutQuery (BOOL (*fn) () );
void DllExport SetPreviewMessage (UINT umsgPreviewMessage);


//	=========================================================================

// CImapMailbox class, inherited from CimapControl. 
// This class MUST have a mailbox name.
// 

class QCNetworkSettings;

class AFX_EXT_CLASS CImapStream
{
   
// Implementation
public:
	// Use this to create a "control" stream.
	CImapStream(const char *ServerName, UINT ServiceID, const char *PortNum = NULL);
	~CImapStream();

// Open a "control" stream, i.e., with no mailbox selected.
	HRESULT		OpenControlStream();

// Methods for initiating the connection to the IMAP server.
// NOTE: Do NOT provide a way to close the stream without destroying
// the object itselt, othersize the reference count scheme could be corrupted.
//
	HRESULT		OpenMailbox  (void);
	HRESULT		OpenMailbox  (const char *MailboxName);

	void		RequestStop();

// Free memory allocated by this DLL.	
	void		FreeMemory(void **pMem);

	void		DestroyEnvelope (ENVELOPE **ppEnvelope);

//  // mailbox list functions.
	virtual HRESULT		IMAPListUnSubscribed (LPCSTR pReference, CLister *Lister, BOOL INCLUDE_INBOX);

	// Create mailbox
	HRESULT		CreateMailbox (LPCSTR pImapName);
	HRESULT		DeleteMailbox (LPCSTR pImapName);
	HRESULT		RenameMailbox (LPCSTR pImapName, LPCSTR pNewImapName);

	// Subscribe/Unsubscribe:
	HRESULT		SubscribeMailbox (LPCSTR pImapName);
	HRESULT		UnSubscribeMailbox (LPCSTR pImapName);

	// DO a LIST on the mailbox to get it's attributes.
	HRESULT		FetchMailboxAttributes (LPCSTR pImapName, CLister *Lister);

// Fetches.

// Top level message only.
	HRESULT		UIDFetchEnvelope (unsigned long uid, ENVELOPE **ppEnvelope);

	HRESULT		UIDFetchEnvelopeAndExtraHeaders (
						IMAPUID		Uid, 
						ENVELOPE**	ppEnvelope,
						char*		pFields,
						CWriter*	pWriter);

	HRESULT		UIDFetchStructure	(unsigned long uid, BODY **ppBody);
	HRESULT		UIDFetchFlags (const char *pSequence, CEltWriter *pEltWriter);
	void		FreeBodyStructure	(BODY *pBody);

	HRESULT		UIDFetchFast (unsigned long uid, IMAPFULL **ppFlags);
	HRESULT		UIDFetchAll (unsigned long uid, IMAPFULL **ppFlags);
	HRESULT		UIDFetchFull	(unsigned long uid, IMAPFULL **ppFlags);

	HRESULT		UIDFetchInternalDate(unsigned long uid, char **ppDate);
	HRESULT		UIDFetchHeader	(unsigned long uid, CWriter *Writer);
	HRESULT		UIDFetchMessage  (unsigned long uid, CWriter *Writer, BOOL Peek);
	HRESULT		UIDFetchMessageBody (unsigned long uid, CWriter *Writer, BOOL Peek);

	HRESULT		UIDFetchPartialMessage  (unsigned long uid, unsigned long first, unsigned long nBytes,
						CWriter *Writer, BOOL Peek);
	HRESULT		UIDFetchPartialMessageBody (unsigned long uid, unsigned long first, unsigned long nBytes,
						CWriter *Writer, BOOL Peek);
	HRESULT		UIDFetchPartialBodyText (unsigned long uid, char *sequence, unsigned long first,
					unsigned long nBytes, CWriter *Writer, BOOL Peek);
	HRESULT		FetchUID (unsigned long msgNum, IMAPUID *pUid);


// These apply to a "message/rfc822"part. (Can also be used for top-level message.)
	HRESULT		UIDFetchRFC822Header (unsigned long uid, char *pSequence, CWriter *Writer);
	HRESULT		UIDFetchRFC822Text (unsigned long uid, char *sequence, CWriter *Writer);
	HRESULT		UIDFetchRFC822HeaderFields (unsigned long uid, char *sequence, char *Fields, CWriter *Writer);
	HRESULT		UIDFetchRFC822HeaderFieldsNot (unsigned long uid, char *sequence, char *fields, CWriter *Writer);

	// BODY part level fetches.
	HRESULT		UIDFetchMimeHeader (unsigned long uid, char *sequence, CWriter *Writer);
	HRESULT		UIDFetchBodyText (unsigned long uid, char *sequence, CWriter *Writer, BOOL Peek);


// Preamble and trailer text fetch (use complicated method).
// If "sequence" is NULL, apply to top level message. Must refer to a multipart message,
// an embedded message/rfc822 part that has a multipart body, or a multipart sub-part.
// or part. 
//
	HRESULT		UIDFetchPreamble(unsigned long uid, char *sequence, CWriter *Writer);
	HRESULT		UIDFetchTrailer(unsigned long uid, char *sequence, CWriter *Writer);

// Methods for STORE'ing flags. For IMAP commands that return new values of flags,
// the new values are returned in the IMAPFLAGS parameter.
// BUG: These methods should be extended to handle uid sets.
//
	HRESULT		UIDSaveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent);
	HRESULT		UIDAddFlags  (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent);
	HRESULT		UIDRemoveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent);

	HRESULT		UIDRemoveFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */);
	HRESULT		UIDAddFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */);


// COPY/MOVE methods. Destination mailbox MUST be on the same server.
// BUG: Should be extended to handle message sets.
	HRESULT		UIDCopy (const char *pUidlist, const char *pDestMailbox);

// Append:
	HRESULT		AppendMessage (const char* pFlags, unsigned long Seconds, STRING *pMsg);

// Utilities.
	HRESULT		UIDMessageIsMultipart (unsigned long uid, BOOL *pVal);
	HRESULT		UIDGetTime (IMAPUID uid, unsigned long *pSeconds);

	HRESULT		GetRfc822Size (IMAPUID uid,unsigned long *pSize);

	
// Return the mailbox's current UIDVALIDITY number.
// The mailbox must have already been OpenMailbox()'d before this will return a valid value, 
// otherwise it returns 0. (Note: UIDVALIDITY cannot be zero (see Imap4rev1 formal spec.)).
	UIDVALIDITY			UIDValidity();

	BOOL				IsReadOnly ();
	
// Status and capability methods.
	HRESULT		UIDFetchStatus (const char *pMailbox, long flags, MAILSTATUS *pStatus);

	BOOL		HasStatusCommand ();
	BOOL		IsImap4Rev1 ();
	BOOL		IsImap4Only ();
	BOOL		IsLowerThanImap4 ();
	BOOL		HasOptimize1 ();
	BOOL		HasSort ();
	BOOL		HasScan ();
	BOOL		HasAcl ();
	BOOL		HasQuota ();
	BOOL		HasLiteralPlus ();
	BOOL		HasXNonHierarchicalRename ();


// Search a mailbox.

	void		Search (char *charset, SEARCHPGM *pgm, CString& szResults);
	BOOL		FormatSearchCriteria (SEARCHPGM *pPgm, LPCSTR pHeaderList, BOOL bBody, 
						BOOL bNot, LPCSTR pSearchString);


	HRESULT		UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr,  CString& szResults);

// Search message text.
	HRESULT		UIDFindText (LPCSTR pSearchString, unsigned long UidFirst, unsigned long UidLast, CString& szResults);

	//
	// These are class methods:  (STATIC)
	//
	static SEARCHPGM* NewSearchPgm();

	static void	FreeSearchpgm (SEARCHPGM** pPgm);

	static void	FreePgmUidList (SEARCHPGM *pPgm);

	static BOOL	SetORSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList, BOOL bBody,
						LPCSTR pSearchString);

	static BOOL	SetORHeaderSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList,
						LPCSTR pSearchString);

	static BOOL	SetNewPgmUidList (SEARCHPGM *pPgm, LPCSTR pStr);


// ========== non_UID functions ==================/

	HRESULT		FetchHeader (unsigned long msgNum, CWriter *Writer);

// Mailbox level methods:

// Mailbox-level commands that do not require a SELECTED mailbox.


// Check and NOOP
	HRESULT		Check();
	HRESULT		Noop ();

// delete list of messages.
	HRESULT		UIDDeleteMessages (LPCSTR pUidList, BOOL Expunge);
// Undelete 
	HRESULT		UIDUnDeleteMessages (LPCSTR pUidList);
// Expunge.
	HRESULT		UIDExpunge (LPCSTR pUidList, CString& szUidsActuallyRemoved);

// Simple Expunge
	HRESULT		Expunge ();

	HRESULT		RecreateMessageMap ();

// Logout.
	HRESULT		Logout ();

// Access to internal state
	BOOL		IsSelected ();
	BOOL		IsConnected();
	BOOL		IsAuthenticated();

// Utility functions
	unsigned long		GetMessageCount();

// Top level delimiter.
	HRESULT		GetTopMboxDelimiter (TCHAR *pDelimiter);

	void		SetNetSettingsObject (QCNetworkSettings* pNetSettings);

	// For fetching error strings.
	HRESULT		GetLastErrorStringAndType (LPSTR szErrorBuf, short nBufSize, int *pType);

	// This returns the last tagged server message, whether it was an error or not.
	HRESULT		GetLastServerMessage (LPSTR szErrorBuf, short nBufSize);

// Callabcks:
public:

	void		AllowAuthenticate (BOOL Value);

	// Passes back IMAP notification messages
	//
	void		RegisterUserCallback (ImapNotificationCallback_p pUserCallback,  unsigned long ulData);

	void		SetErrorCallback (ImapErrorFunc pImapErrorFn);
	void		ResetErrorCallback ();

	// Set login and password strings:
	//
	void		SetLogin	(LPCSTR pLogin);
	void		SetPassword	(LPCSTR pPassword);

	void		SetKrbDllName (LPCSTR pKrbLibName);
	void		SetDesDllName (LPCSTR pDesLibName);
	void		SetGssDllName (LPCSTR pGssLibName);

	// Static: 

	// Must call this before using the DLL!!!
	static HRESULT Initialize ();

	static HRESULT Cleanup ();

// Settings management. These will be placed in a separate "INTERFACE" when this DLL becomes
// a COM component.
//
public:
	


// Internal state.
private:
	// All this stuff actually gets implemented in the foll. embedded class.
	CMImap			*m_cmImap;
};


#endif // _IMAP_H_

