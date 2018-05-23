// QCImapMailboxCommand.h: interface for the QCImapMailboxCommand subclass.
//
//////////////////////////////////////////////////////////////////////

#ifndef __QCIMAPMAILBOXCOMMAND_H_
#define __QCIMAPMAILBOXCOMMAND_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCMailboxCommand.h"
#include "ImapExports.h"


class QCImapMailboxCommand : public QCMailboxCommand
{

	CString				m_szImapName;	// Full IMAP pathname.
	BOOL				m_bNoselect;
	BOOL				m_bMarked;
	BOOL				m_bUnmarked;
	BOOL				m_bNoInferiors;
	ACCOUNT_ID			m_AccountID;
	TCHAR				m_Delimiter;
	BOOL				m_bReadOnly;
	BOOL				m_bAutoSync;

	// Used to re-sync mailbox lists. 
	BOOL				m_Valid;

	// If we actually display this as a sub-menu or a menu item.
	BOOL				m_bMboxShownAsSubMenu;
	BOOL				m_bXferShownAsSubMenu;

// Interface
public:
	
	friend class QCMailboxDirector;
	
	void SetImapName( LPCSTR szImapName ) { m_szImapName = szImapName; }
	void SetNoSelect (BOOL bStatus)		  { m_bNoselect  = bStatus;	}
	void SetMarked	 (BOOL bStatus)		  { m_bMarked	 = bStatus;	}
	void SetUnmarked (BOOL bStatus)		  { m_bUnmarked  = bStatus;	}
	void SetNoInferiors (BOOL bStatus)	  { m_bNoInferiors  = bStatus;	}
	void SetAutoSync (BOOL bAutoSync)	  { m_bAutoSync = bAutoSync; }
	void SetAccountID (ACCOUNT_ID Id)	  { m_AccountID  = Id; }
	void SetDelimiter (TCHAR ch)		  { m_Delimiter  = ch; }
	void SetReadOnly  (BOOL value)		  { m_bReadOnly = value; }

	void MarkValid   ()					  { m_Valid = TRUE; }
	void MarkInvalid ()					  { m_Valid = FALSE; }
	BOOL IsValid     ()					  { return m_Valid; }

	// Is this the IMAP INBOX??
	BOOL IsImapInbox();

	// Decide if we're going to show this mailbox as a sub-menu:
	//
	BOOL ShouldShowAsSubMenu ();

	// If it's actually shown as a sub-menu:
	BOOL ShownAsSubMenu (BOOL bIsXferMenu)
		{ return bIsXferMenu ? m_bXferShownAsSubMenu : m_bMboxShownAsSubMenu; }


	// Tell us from outside.
	//
	void SetShownAsSubMenu(BOOL bVal, BOOL bIsXferMenu)
		{ 
			if (bIsXferMenu) m_bXferShownAsSubMenu = bVal;
			else m_bMboxShownAsSubMenu = bVal;
		}

public:

	QCImapMailboxCommand(
		QCMailboxDirector*	pDirector,
		LPCSTR				szName,
		LPCSTR				szImapName,	
		LPCSTR				szPathname,	// For IMAP, this is the pathname to the directory containing the mailbox files.
		MailboxType			theType,
		UnreadStatusType	theStatus,
		ACCOUNT_ID			AccountID,
		BOOL				bNoselect = FALSE,
		BOOL				bMarked = FALSE,
		BOOL				bUnmarked = FALSE,
		BOOL				bNoInferiors = FALSE,
		BOOL				bAutoSync = FALSE);
	
	virtual	 ~QCImapMailboxCommand();

	// Override from parent.
	virtual BOOL IsEmpty();

	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	void	InvalidateChildMailboxes (BOOL bRecurse);
	void	RemoveInvalidChildMailboxes (BOOL bRecurse);

	// Return the directory housing the command object.
	BOOL				GetObjectDirectory (CString& Dirpath);
	MailboxType			ImapTypeToMBTType (ImapMailboxType Type);
	ImapMailboxType		MBTTypeToImapType (MailboxType     Type);

	// Determine if this can have child mailboxes.
	virtual BOOL		CanHaveChildren ();

	// Determine if this mailbox is selectable.
	virtual BOOL CanContainMessages ()
		{ return !IsNoSelect(); }

	// Access to internals.
	LPCSTR	GetImapName ()				  { return (LPCSTR)m_szImapName; }
	ACCOUNT_ID GetAccountID ()			  { return m_AccountID; }
	BOOL	IsNoSelect ()				  { return m_bNoselect; }
	BOOL	IsMarked ()					  { return m_bMarked; }
	BOOL	IsUnmarked()				  { return m_bUnmarked; }
	BOOL	IsNoInferiors ()		      { return m_bNoInferiors; }
	BOOL	IsAutoSync()				  { return m_bAutoSync; }  
	TCHAR	GetDelimiter ()				  { return m_Delimiter; }
	BOOL	IsReadOnly   ()				  { return m_bReadOnly; }

	DECLARE_DYNAMIC(QCImapMailboxCommand)
};


#endif //  __QCIMAPMAILBOXCOMMAND_H_

