// QCMailboxDirector.h: interface for the QCMailboxDirector class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _QCMailboxDirector_h_
#define _QCMailboxDirector_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"
#include "QCCommandActions.h"

#ifdef IMAP4
// Forward dec:
class QCImapMailboxCommand;
#endif  // IMAP4

class QCMailboxTreeCtrl;
class QCMailboxCommand;
class CTocDoc;

extern enum MailboxType;

class QCMailboxDirector : public QCCommandDirector
{
	CPtrList	m_theMailboxList;
	CBitmap		m_theUnreadStatusBitmap;
	CBitmap		m_theTransferBitmap;
	
	LPTSTR LegalizeFilename(LPTSTR pFilename);

	BOOL ProcessDescMapFile(
	LPCTSTR		szPath,
	CPtrList&	theMailboxList );

	BOOL Salvage( 
	LPCTSTR		szPath,
	CPtrList&	theMailboxList );

	QCMailboxCommand*	FindByFilename(
	CPtrList&	theList,
	LPCTSTR		szPathame );

	POSITION Insert(
	CPtrList&			theList,
	QCMailboxCommand*	pCommand );

	BOOL WriteDescMapFile(
	LPCTSTR			szPath,
	const CPtrList& theList);

	void UpdateChildPaths(
	const CString&	szOldPathname,
	const CString&	szNewPathname,
	const CPtrList&	theList );

	void NewMBXFilename(
	LPCTSTR		szName,
	LPCTSTR		szOldPathname,
	LPCTSTR		szNewPath,
	LPCTSTR		szExt,
	CString&	szFilename,
	BOOL&		bNoFileChanges );

	BOOL	DoCompactMailboxes(
	const CPtrList&	theList );

#ifdef IMAP4
	// private
	BOOL WriteImapDescMapFile(
		LPCTSTR			szPath,
		const CPtrList& theList,
		BOOL bRecurse = FALSE );

	BOOL  ProcessMailboxesFile (
		unsigned long		AccountId,
		LPCTSTR				pPath,
		CPtrList&			theMailboxList,
		BOOL				Recurse = TRUE,
		BOOL				TmpList = FALSE);

	BOOL  SalvageImapMailboxesFile(
		unsigned long		Id, 
		LPCTSTR				pPath,
		CPtrList&			theMailboxList);

	BOOL HandleDeleteImapMailbox (
		QCMailboxCommand *pCommand,
		COMMAND_ACTION_TYPE theAction,
		BOOL bDeleteChildren);

	BOOL HandleDeleteImapAccount (
		QCMailboxCommand *pCommand,
		COMMAND_ACTION_TYPE theAction,
		BOOL bDeleteLocalCache);

	BOOL HandleRenameImapMailbox (
		QCMailboxCommand **ppCommand,
		COMMAND_ACTION_TYPE theAction,
		void* pData);

	BOOL HandleMoveImapMailbox (
		QCMailboxCommand **ppCommand,
		COMMAND_ACTION_TYPE theAction,
		QCMailboxCommand *pNewParentCommand);

	BOOL DeleteRemoteMailboxes (
		QCImapMailboxCommand *pImapCommand,
		BOOL bDeleteChildren);

	BOOL DeleteLocalChildMailboxes (
		QCImapMailboxCommand *pImapCommand,
		BOOL bDeleteFromServer);

	POSITION FindContainingList (
		QCMailboxCommand *pChild,
		CPtrList **ppList,
		QCMailboxCommand **ppParent);

	BOOL UpdateAfterRename (
		QCImapMailboxCommand *pImapCommand,
		LPCTSTR pNewName, LPCTSTR pNewDirname);

	void UpdateChildrenAfterRename (
		QCImapMailboxCommand *pImapCommand);

#endif  // IMAP4

public:
	QCMailboxDirector();
	virtual ~QCMailboxDirector();
	
	QCMailboxCommand*	AddCommand(
	LPCTSTR				szName,
	MailboxType			theType,
	QCMailboxCommand*	pParent );

	BOOL	Build();

	QCMailboxCommand*	FindByNamedPath(
	LPCTSTR	szNamedPath );

	QCMailboxCommand*	FindByName(
	CPtrList*	pTheList,
	LPCTSTR		szName );

	QCMailboxCommand*	FindByPathname(
	LPCTSTR		szPathname );

	CPtrList*	GetMailboxList() { return &m_theMailboxList; }
	CBitmap&	GetTransferBitmap() { return m_theTransferBitmap; }
	CBitmap&	GetUnreadStatusBitmap() { return m_theUnreadStatusBitmap; }

	BOOL InitializeMailboxTreeControl( 
	QCMailboxTreeCtrl* 	pTreeCtrl,
	UINT				uLevel,
	CPtrList*			pList );

	void	NotifyClients(
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	uAction,
	void*				pData = NULL );


	void NewMessageCommands( 
	COMMAND_ACTION_TYPE	theAction,
	CMenu*				pPopupMenu,
	COMMAND_ACTION_TYPE theNewAction = CA_NONE);

	// Moved to public 11/4/98 (JOK).
	void NewMessageCommandsLevel( 
	CPtrList&			theList,
	COMMAND_ACTION_TYPE	theAction,
	CMenu*				pPopupMenu,
	COMMAND_ACTION_TYPE theNewAction ,
	LONG				lMenuCount );

	CString BuildNamedPath(
	QCMailboxCommand*	pCommand);

	BOOL	CompactMailboxes();

	BOOL	CompactAMailbox( 
	CTocDoc* pTocDoc);

	QCMailboxCommand*	FindParent(
	QCMailboxCommand*	pCommand );

	QCMailboxCommand*	CreateTargetMailbox(
	QCMailboxCommand*	pParentCommand,		//(i) NULL indicates root mailbox
	BOOL				bIsTransfer );

#ifdef IMAP4
	// public
	BOOL BuildImapAccounts	();
	BOOL AddPersona (CString& Name, BOOL bRefreshMboxList = FALSE, BOOL bInitializeMboxSubtree = FALSE);
	BOOL WillDeletePersona	(CString& Name);
	BOOL ModifyPersona	(CString& Name);

	BOOL InitializeImapSubtree( 
		QCMailboxTreeCtrl* 	pTreeCtrl,
		UINT				uLevel,
		QCImapMailboxCommand*	pImapCommand);

	QCMailboxCommand*	QCMailboxDirector::ImapAddCommand(
		LPCTSTR				szName,
		MailboxType			theType,
		QCMailboxCommand*	pParent );

	BOOL UpdateImapMailboxLists (
		QCImapMailboxCommand *pParentImapCommand,
		BOOL Recurse, 
		BOOL IsLevelZero );

	void QCMailboxDirector::ImapNotifyClients(
		QCCommandObject*	pQCCommand,
		COMMAND_ACTION_TYPE	theAction,
		void*				pData );

	QCMailboxCommand*	ImapFindByName(
		CPtrList*	pTheList,
		LPCTSTR		szName );

	QCImapMailboxCommand*	ImapFindByImapName(
		unsigned long  AccountId,
		LPCTSTR		pImapName,
		TCHAR		Delimiter );

	QCImapMailboxCommand*	ImapFindParent(
		QCImapMailboxCommand*	pImapCommand );

	void RemoveImapMailboxCommand (
		QCImapMailboxCommand *pImapCommand,
		BOOL		bDeleteFromServer = FALSE,
		BOOL		bUpdateMboxList = FALSE );

#endif // IMAP4

};

#endif // _QCMailboxDirector_h_
