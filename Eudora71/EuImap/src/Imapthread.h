// imapthread.h - declaration of threading classes.
// hierarchies.
//

#ifndef __IMAPTHREAD_H
#define __IMAPTHREAD_H

#include "ImapTypes.h"
#include "ImapMailbox.h"

//======================================================================//
//====================== CMessageThread class interface ==================//

// Forward declarations


// CImapThread class - Base class inheritted from the MFC CWinThread. This is what
// a CImapAccount manages in it's thread object list.
class CImapThread : public CWinThread
{
public:
	CImapThread ();
	virtual ~CImapThread();

	// Interface
	void	  SetAccountID (ACCOUNT_ID AccountId) 
						{ m_AccountId = AccountId; }
	ACCOUNT_ID GetAccountID()
						{ return m_AccountId; }
			
protected:
	// Account Context..
	ACCOUNT_ID		m_AccountId;

	// The MFC thread.
	CWinThread		*m_pWinThread;
};


// CImapMessageThread class - Fetch messages in a background thread.
class CImapMessageThread : public CImapThread
{
public:
	CImapMessageThread ();
	virtual ~CImapMessageThread();

	// public Methods
	BOOL	FetchNewMessages(CImapMailbox *pImapMailbox);
	// Do the real work.
	void	DoFetchNewMessages ();

	// Copy the given list into the internal "m_OldUidList".
	void CopyUidList (CPtrList& UidList);

// Attributes.
private:
	// Full name of server.
	CString		m_ImapName;

	// List of messages already downloaded.
	CPtrUidList m_OldUidList;

	// Our own mailbox
	CImapMailbox	m_ImapMailbox;
};


#endif // __IMAPTHREAD_H
