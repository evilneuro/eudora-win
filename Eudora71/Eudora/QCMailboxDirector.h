// QCMailboxDirector.h: interface for the QCMailboxDirector class.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#ifndef _QCMailboxDirector_h_
#define _QCMailboxDirector_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"
#include "QCCommandActions.h"

#include <list>

// Forward dec:
class QCImapMailboxCommand;

class QCMailboxTreeCtrl;
class QCMailboxCommand;
class CTocDoc;
class QCPOPNotifier;

extern enum MailboxType;

class QCMailboxDirector : public QCCommandDirector
{
	CPtrList	m_theMailboxList;
	CBitmap		m_theUnreadStatusBitmap;
	CBitmap		m_theTransferBitmap;
	
	LPTSTR LegalizeFilename(LPTSTR pFilename);
	BOOL ProcessDescMapFile(LPCTSTR szPath, CPtrList& theMailboxList);
	BOOL Salvage(LPCTSTR szPath, CPtrList& theMailboxList);
	QCMailboxCommand* FindByFilename(CPtrList& theList, LPCTSTR szPathame);
	POSITION Insert(CPtrList& theList, QCMailboxCommand* pCommand, bool bDeleteIfNotInserted = false);
	BOOL WriteDescMapFile(LPCTSTR szPath, const CPtrList& theList);
	void RenameMailboxOrFolder(QCMailboxCommand *pCommand,
							   CTocDoc *pTocDoc,
							   CPtrList *pMailboxList,
							   LPCTSTR szNewName,
							   BOOL bUpdateFilters = TRUE);
	void UpdateChildPaths(const CString& szOldPathname, const CString& szNewPathname,
							const CPtrList& theList);
	void NewMBXFilename(LPCTSTR szName, LPCTSTR szOldPathname, LPCTSTR szNewPath, LPCTSTR szExt,
						CString& szFilename, BOOL& bNoFileChanges);

	BOOL DoCompactMailboxes(const CPtrList& theList);

	// private
	BOOL WriteImapDescMapFile(LPCTSTR szPath, const CPtrList& theList, BOOL bRecurse = FALSE);
	BOOL ProcessMailboxesFile(unsigned long AccountId, LPCTSTR pPath, CPtrList& theMailboxList,
								BOOL Recurse = TRUE, BOOL TmpList = FALSE);
	BOOL SalvageImapMailboxesFile(unsigned long Id, LPCTSTR pPath, CPtrList& theMailboxList);
	BOOL HandleDeleteImapMailbox(QCMailboxCommand* pCommand, COMMAND_ACTION_TYPE theAction,
									BOOL bDeleteChildren);
	BOOL HandleDeleteImapAccount(QCMailboxCommand* pCommand, COMMAND_ACTION_TYPE theAction,
									BOOL bDeleteLocalCache);
	BOOL HandleRenameImapMailbox(QCMailboxCommand** ppCommand, COMMAND_ACTION_TYPE theAction,
									void* pData);
	BOOL HandleMoveImapMailbox(QCMailboxCommand** ppCommand, COMMAND_ACTION_TYPE  theAction,
									QCMailboxCommand* pNewParentCommand);
	BOOL DeleteRemoteMailboxes(QCImapMailboxCommand* pImapCommand, BOOL bDeleteChildren);
	BOOL DeleteLocalChildMailboxes(QCImapMailboxCommand* pImapCommand, BOOL bDeleteFromServer);
	POSITION FindContainingList(QCMailboxCommand* pChild, CPtrList** ppList,
								QCMailboxCommand** ppParent);
	BOOL UpdateAfterRename(QCImapMailboxCommand* pImapCommand, LPCTSTR pNewName,
							LPCTSTR pNewDirname);
	void UpdateChildrenAfterRename(QCImapMailboxCommand* pImapCommand);


public:
	QCMailboxDirector();
	virtual ~QCMailboxDirector();
	
	BOOL Build();

	QCMailboxCommand* AddCommand(LPCTSTR szName, MailboxType theType, QCMailboxCommand* pParent);
	QCMailboxCommand* FindByNamedPath(LPCTSTR szNamedPath);
	QCMailboxCommand* FindByName(CPtrList* pTheList, LPCTSTR szName);
	QCMailboxCommand* FindByPathname(LPCTSTR szPathname);
	QCMailboxCommand* FindByNameInTree(CPtrList* pList, LPCTSTR szName);

	CPtrList*	GetMailboxList() { return &m_theMailboxList; }
	CBitmap&	GetTransferBitmap() { return m_theTransferBitmap; }
	CBitmap&	GetUnreadStatusBitmap() { return m_theUnreadStatusBitmap; }

	BOOL InitializeMailboxTreeControl(QCMailboxTreeCtrl* pTreeCtrl, UINT uLevel, CPtrList* pList);

	void NotifyClients(QCCommandObject* pCommand, COMMAND_ACTION_TYPE uAction,void* pData = NULL);
	
	void NewMessageCommands(BOOL bIsTransfer, CMenu* pPopupMenu);
	// Moved to public 11/4/98 (JOK).
	void NewMessageCommandsLevel(QCMailboxCommand* pParentCommand, BOOL bIsTransfer, CMenu* pPopupMenu);

	CString BuildNamedPath(QCMailboxCommand* pCommand);

	BOOL CompactMailboxes();
	BOOL CompactAMailbox(CTocDoc* pTocDoc, bool bUserRequested = false);

	void CreateJunkMailbox();
	void BlessJunkMailbox(QCMailboxTreeCtrl *pTreeCtrl);
	void RenameJunkMailbox(QCMailboxTreeCtrl *pTreeCtrl);

	QCMailboxCommand* FindParent(QCMailboxCommand* pCommand);
	QCMailboxCommand* CreateTargetMailbox(QCMailboxCommand*	pParentCommand, BOOL bIsTransfer);

	// public
	BOOL BuildImapAccounts();
	BOOL AddPersona (LPCTSTR Name, BOOL bRefreshMboxList = FALSE, BOOL bInitializeMboxSubtree = FALSE);
	BOOL WillDeletePersona(LPCTSTR Name);
	BOOL ModifyPersona(LPCTSTR Name);

	BOOL InitializeImapSubtree(QCMailboxTreeCtrl* pTreeCtrl, UINT uLevel, QCImapMailboxCommand* pImapCommand);

	QCMailboxCommand* ImapAddCommand(LPCTSTR szName, MailboxType theType, QCMailboxCommand* pParent);

	BOOL UpdateImapMailboxLists(QCImapMailboxCommand* pParentImapCommand, BOOL Recurse, BOOL IsLevelZero);
	void ImapNotifyClients(QCCommandObject* pQCCommand, COMMAND_ACTION_TYPE theAction, void* pData);

	CTocDoc* ImapFindTOCByImapName(unsigned long ai, LPCTSTR szImapName, TCHAR cDelimiter);
	QCMailboxCommand* ImapFindByName(CPtrList* pTheList, LPCTSTR szName);
	QCImapMailboxCommand* ImapFindByImapName(unsigned long AccountId, LPCTSTR pImapName, TCHAR Delimiter);
	QCImapMailboxCommand* ImapFindByImapNameInAccount(unsigned long lAccountID, LPCTSTR szImapName);
	QCImapMailboxCommand* ImapFindByImapNameInTree(CPtrList *pList, LPCTSTR szImapName);
	void ImapSyncMailboxesInAccount(LPCSTR szPersona,
									unsigned long ulBitFlags,
									QCPOPNotifier *pNewMailNotifier = NULL);
	void ImapSyncMailboxesInTree(LPCSTR szPersona,
								 unsigned long ulBitFlags,
								 CPtrList *pList,
								 bool bOnlyIfPropSet,
								 QCPOPNotifier *pNewMailNotifier = NULL);
	void ImapSetHideDeletedMsgsInTree(QCMailboxCommand *pCommand, BOOL bHideDeleted);
	QCImapMailboxCommand* ImapFindParent(QCImapMailboxCommand* pImapCommand);

	void RemoveImapMailboxCommand(QCImapMailboxCommand* pImapCommand, BOOL bDeleteFromServer = FALSE,
									BOOL bUpdateMboxList = FALSE );

	// Recent mailbox functionality
	static void BuildRecentMailboxesList();
	static void UpdateRecentMailboxList(LPCTSTR MailboxPathname, BOOL bIsAdd);
	static std::list<LPCTSTR> s_RecentMailboxList;
};


#endif // _QCMailboxDirector_h_
