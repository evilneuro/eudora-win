// QCMailboxCommand.h: interface for the QCMailboxCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCMAILBOXCOMMAND_H__171DE302_BCD1_11D0_97C3_00805FD2F268__INCLUDED_)
#define AFX_QCMAILBOXCOMMAND_H__171DE302_BCD1_11D0_97C3_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"
#include "tocdoc.h"

class QCMailboxDirector;
class CTocDoc;

enum  UnreadStatusType
{
	US_UNKNOWN,
	US_YES,
	US_NO,
};

enum  COMPACT_RESULT_TYPE
{
    CRT_COMPACTED,
    CRT_MILD_ERROR,
    CRT_DRASTIC_ERROR
};

class QCMailboxCommand : public QCCommandObject  
{

	CString				m_szName;
	CString				m_szPathname;
	MailboxType			m_theType;		// Type of mailbox
	UnreadStatusType	m_theStatus;	// Does mailbox have unread messages?
	CPtrList			m_theChildList;

protected:
	
	friend class QCMailboxDirector;
	
	void SetName( LPCSTR szName ) { m_szName = szName; }
	void SetPathname( LPCSTR szPathname ) { m_szPathname = szPathname; }

	COMPACT_RESULT_TYPE CompactMailbox( 
	CTocDoc*			pTocDoc );

	virtual void NotifyDirector( 
	COMMAND_ACTION_TYPE theAction,
	void*				pData = NULL )
	{
		QCCommandObject::NotifyDirector( theAction, pData );
	}

	void	SetType(
	MailboxType theType );

public:

	QCMailboxCommand(
	QCMailboxDirector*	pDirector,
	LPCSTR				szName,
	LPCSTR				szPathname,
	MailboxType			theType = MBT_REGULAR,
	UnreadStatusType	theStatus = US_UNKNOWN );
	
	virtual ~QCMailboxCommand();

	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	LPCSTR				GetName() const { return m_szName; }
	const CString&		GetPathname() const { return m_szPathname; }
	MailboxType			GetType() const { return m_theType; }
	UnreadStatusType	GetStatus() const { return m_theStatus; }
	CPtrList&			GetChildList() { return m_theChildList; }

#ifdef IMAP4
	// Make it virtual
	virtual BOOL		IsEmpty();
#else
	BOOL				IsEmpty();
#endif
		
	virtual UINT	GetFlyByID(
	COMMAND_ACTION_TYPE	theAction );

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction);

	// Overrideable. Descendent classes can override,
	// e.g. IMAP mailbox command objects.
	//
	virtual BOOL  CanHaveChildren ()
				 { return m_theType == MBT_FOLDER; }

	// Added to handle IMAP cases.
	virtual BOOL CanContainMessages ()
				 { return m_theType != MBT_FOLDER; }


#ifdef IMAP4
	// Shortcut to determining if it's an IMAP command type.
	BOOL		IsImapType()
				{ return (	( m_theType == MBT_IMAP_ACCOUNT ) ||
							( m_theType == MBT_IMAP_MAILBOX ) ||
							( m_theType == MBT_IMAP_NAMESPACE) );}
#endif // IMAP4

	DECLARE_DYNAMIC(QCMailboxCommand)
};

#endif // !defined(AFX_QCMAILBOXCOMMAND_H__171DE302_BCD1_11D0_97C3_00805FD2F268__INCLUDED_)
