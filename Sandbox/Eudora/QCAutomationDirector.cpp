// QCAutomationDirector.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "compmsgd.h"
#include "eudora.h"
#include "Automation.h"
#include "QCCommandStack.h"
#include "QCAutomationDirector.h"
#include "QCMailboxDirector.h"
#include "QCPluginDirector.h"
#include "QCRecipientDirector.h"
#include "QCSignatureDirector.h"
#include "QCStationeryDirector.h"
#include "QCMailboxCommand.h"
#include "QComApplication.h"
#include "QComFolder.h"
#include "QComFolders.h"
#include "QCPluginCommand.h"
#include "QCRecipientCommand.h"
#include "QCSignatureCommand.h"
#include "QCStationeryCommand.h"
#include "Password.h"
#include "Pop.h"
#include "resource.h"
#include "rs.h"
#include "tocdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;
extern QCPluginDirector		g_thePluginDirector;
extern QCRecipientDirector	g_theRecipientDirector;
extern QCSignatureDirector	g_theSignatureDirector;
extern QCStationeryDirector	g_theStationeryDirector;

IMPLEMENT_DYNAMIC(QCAutomationDirector, QCCommandObject)

QCAutomationDirector::QCAutomationDirector()
{
	m_bRunning = false;
	m_bCommandCheckMail = false;
	m_bCommandSendMail = false;
	m_bCommandEmptyTrash = false;
	m_bCommandCompactFolders = false;
	m_csPass = "";
}

QCAutomationDirector::~QCAutomationDirector()
{
	if (m_bRunning)
	{
		Stop();
	}
}

void QCAutomationDirector::Start(void)
{
	if (m_bRunning) return;

	// Register as a command client with other components
	g_theMailboxDirector.Register(this);
	g_thePluginDirector.Register(this);
	g_theRecipientDirector.Register(this);
	g_theStationeryDirector.Register(this);
	g_theSignatureDirector.Register(this);

	// Build list of all mail folders including IMAP
	QComFolder* pRootFolder = g_pApplication->GetRootFolder();
	if (!pRootFolder) return;
	CPtrList* pSearchList = g_theMailboxDirector.GetMailboxList();
	if (pSearchList)
	{
		BuildFolderList(pRootFolder, pSearchList, 0);
	}

	m_bRunning = true;
}

void QCAutomationDirector::Stop(void)
{
	if (!m_bRunning) return;

	// Unregister as a command client with other components
	g_theMailboxDirector.UnRegister(this);
	g_thePluginDirector.UnRegister(this);
	g_theRecipientDirector.UnRegister(this);
	g_theStationeryDirector.UnRegister(this);
	g_theSignatureDirector.UnRegister(this);
	
	m_bRunning = false;
}

void QCAutomationDirector::ExecuteCommand(void)
{
	if (!g_pApplication) return;

	if (m_bCommandCheckMail)
	{
		// Prevent checking mail while already in progress
		m_bCommandCheckMail = false;
		if (gPOP) return;
		CAutomationCall c;
		// Use password specified by automation
		// if it exists
		if (!m_csPass.IsEmpty())
		{
			if (m_csPass.GetLength())
			{
				::POPPassword = m_csPass;
			}
		}
		// This flag is only set while checking mail
		// Other automation calls can take place during this time
		// with the exception of checking mail
		gbAutomationCheckMail = true;
		g_pApp->OnCheckMail();
		gbAutomationCheckMail = false;
		g_pApplication->FireCheckMailComplete();
		return;
	}
	if (m_bCommandSendMail)
	{
		CAutomationCall c;
		m_bCommandSendMail = false;
		::SendQueuedMessages();
		g_pApplication->FireSendMailComplete();
		return;
	}
	if (m_bCommandEmptyTrash)
	{
		CAutomationCall c;
		m_bCommandEmptyTrash = false;
		EmptyTrash();
		g_pApplication->FireEmptyTrashComplete();
		return;
	}
	if (m_bCommandCompactFolders)
	{
		CAutomationCall c;
		m_bCommandCompactFolders = false;
		g_theMailboxDirector.CompactMailboxes();
		g_pApplication->FireCompactFoldersComplete();
		return;
	}
}

bool QCAutomationDirector::BuildFolderList(
	QComFolder* pCurrentFolder,
	CPtrList* pSearchList,
	short Level)
{
	POSITION pos;
	QCMailboxCommand* pCommand;

	QComFolders* pFolderList = NULL;
	pFolderList = pCurrentFolder->GetFolders();
	if (!pFolderList) return false;
	Level++;
	pos = pSearchList->GetHeadPosition();
	while (pos)
	{
		pCommand = (QCMailboxCommand*)pSearchList->GetNext(pos);
		if (pCommand)
		{
			// Create COM folder object
			QComFolder* pNewFolder = NULL;
			pNewFolder = QComFolder::Create();
			if (!pNewFolder) return false;

			// Add it to folder list
			pFolderList->InsertAtEnd(pNewFolder);

			// Initialize new folder object
			pNewFolder->Init(pCurrentFolder, pCommand, Level);

			// Look for sub-folders
			CPtrList* pChildren = &pCommand->GetChildList();
			if (!pChildren->IsEmpty())
			{
				BuildFolderList(pNewFolder, pChildren, Level);
			}
		}
	}
	Level--;
	return true;
}

void QCAutomationDirector::Notify(QCCommandObject* pc, COMMAND_ACTION_TYPE Action, void* pData)
{
	if (pc->IsKindOf(RUNTIME_CLASS(QCMailboxCommand)) == TRUE)
	{
		OnMailboxEvent((QCMailboxCommand*)pc, Action, pData);
		return;
	}
	if (pc->IsKindOf(RUNTIME_CLASS(QCPluginCommand)) == TRUE)
	{
		OnPluginEvent((QCPluginCommand*)pc, Action, pData);
		return;
	}
	if (pc->IsKindOf(RUNTIME_CLASS(QCRecipientCommand)) == TRUE)
	{
		OnRecipientEvent((QCRecipientCommand*)pc, Action, pData);
		return;
	}
	if (pc->IsKindOf(RUNTIME_CLASS(QCSignatureCommand)) == TRUE)
	{
		OnSignatureEvent((QCSignatureCommand*)pc, Action, pData);
		return;
	}
	if (pc->IsKindOf(RUNTIME_CLASS(QCStationeryCommand)) == TRUE)
	{
		OnStationeryEvent((QCStationeryCommand*)pc, Action, pData);
		return;
	}
}

void QCAutomationDirector::OnMessage(DWORD wParam, DWORD lParam)
{
	if (!gbAutomationEnabled) return;
	if (wParam != 0)
	{
		ASSERT(0);
	}

	// Complete initialization of automation objects
	if (!g_pApplication) return;
	AutomationStart();
	Start();
}

void QCAutomationDirector::OnMailboxEvent(QCMailboxCommand* pc, COMMAND_ACTION_TYPE Action, void* pData)
{
	switch (Action)
	{
	case CA_NEW:
		InternalInsertFolder(pc);
		break;
	case CA_GRAFT:
		InternalMoveFolder(pc, (QCMailboxCommand*)pData);
		break;
	case CA_SORT_AFTER_RENAME:
		InternalRenameFolder(pc);
		break;
	case CA_DELETE:
		InternalDeleteFolder(pc);
		break;
	case CA_NEW_MAILBOX:
		break;
	}
}

bool QCAutomationDirector::FindFolderByCommand(QCMailboxCommand* pCommand, QComFolder** ppResultFolder, bool bRecursive)
{
	*ppResultFolder = NULL;
	QComFolder* pRootFolder = g_pApplication->GetRootFolder();
	if (!pRootFolder) return false;
	return RecurseFindFolderByCommand(pCommand, pRootFolder, ppResultFolder, true);
}

bool QCAutomationDirector::FindFolderByID(long ID, QComFolder** ppResultFolder, bool bRecursive)
{
	*ppResultFolder = NULL;
	QComFolder* pRootFolder = g_pApplication->GetRootFolder();
	if (!pRootFolder) return false;
	return RecurseFindFolderByID(ID, pRootFolder, ppResultFolder, true);
}

bool QCAutomationDirector::FindFolderByName(LPCSTR Name, QComFolder* pParentFolder, QComFolder** ppResultFolder, bool bRecursive)
{
	*ppResultFolder = NULL;
	return RecurseFindFolder(0, Name, pParentFolder, ppResultFolder);
}

bool QCAutomationDirector::FindFolderByName(LPCSTR Name, QComFolder** ppResultFolder, bool bRecursive )
{
	*ppResultFolder = NULL;
	QComFolder* pRootFolder = g_pApplication->GetRootFolder();
	if (!pRootFolder) return false;
	return RecurseFindFolder(0, Name, pRootFolder, ppResultFolder);
}

bool QCAutomationDirector::FindFolderByFullName(LPCSTR FullName, QComFolder* pParentFolder, QComFolder** ppResultFolder, bool bRecursive)
{
	*ppResultFolder = NULL;
	return RecurseFindFolder(1, FullName, pParentFolder, ppResultFolder);
}

bool QCAutomationDirector::FindFolderByFullName(LPCSTR FullName, QComFolder** ppResultFolder, bool bRecursive)
{
	*ppResultFolder = NULL;
	QComFolder* pRootFolder = g_pApplication->GetRootFolder();
	if (!pRootFolder) return false;
	return RecurseFindFolder(1, FullName, pRootFolder, ppResultFolder);
}

bool QCAutomationDirector::RecurseFindFolder(
	short SearchType, 
	LPCSTR SearchString, 
	QComFolder* pCurrentFolder, 
	QComFolder** ppResultFolder,
	bool bRecursive)
{
	// SearchType = 0, search by name
	// SearchType = 1, search by full name (path + name)
	// return true if folder found, otherwise return false
	bool bFound = false;
	QComFolder* pFolder = NULL;
	
	// This is an exception to allow the root folder to be
	// found even though it's one level higher
	if (pCurrentFolder->GetLevel() == 0)
	{
		CString str = "";
		switch (SearchType)
		{
		case 0:
			str = pCurrentFolder->GetName();
			break;
		case 1:
			str = pCurrentFolder->GetFullName();
			break;
		}
		if (!str.CompareNoCase(SearchString))
		{
			// They are finding the root folder
			bFound = true;
			pCurrentFolder->AutoAddRef();
			*ppResultFolder = pCurrentFolder;
			return true;
		}
	}
	
	// Search sub-folders
	POSITION pos;
	POSITION pos2;
	QComFolders* pFolderList = NULL;
	pFolderList = pCurrentFolder->GetFolders();
	if (pFolderList)
	{
		pos = pFolderList->GetHeadPosition();
		while (pos)
		{
			pos2 = pos;
			pFolder = (QComFolder*)pFolderList->GetNext(pos);
			if (!pFolder) break;
			// See if the folder matches
			CString str = "";
			switch (SearchType)
			{
			case 0:
				str = pFolder->GetName();
				break;
			case 1:
				str = pFolder->GetFullName();
				break;
			}
			if (!str.CompareNoCase(SearchString))
			{
				// This is the folder
				bFound = true;
				pFolder->AutoAddRef();
				*ppResultFolder = pFolder;
				break;
			}
			// Look for sub-folders
			if (bRecursive)
			{
				if (!pFolderList->IsEmpty())
				{
					if (true == RecurseFindFolder(SearchType, SearchString, pFolder, ppResultFolder))
					{
						bFound = true;
						break;
					}
				}
			}
		}
	}
	return bFound;
}

bool QCAutomationDirector::RecurseFindFolderByCommand(
	QCMailboxCommand* pCommand,
	QComFolder* pCurrentFolder, 
	QComFolder** ppResultFolder,
	bool bRecursive)
{
	bool bFound = false;
	QComFolder* pFolder = NULL;

	// Search sub-folders
	POSITION pos;
	POSITION pos2;
	QComFolders* pFolderList = NULL;
	pFolderList = pCurrentFolder->GetFolders();
	if (pFolderList)
	{
		pos = pFolderList->GetHeadPosition();
		while (pos)
		{
			pos2 = pos;
			pFolder = (QComFolder*)pFolderList->GetNext(pos);
			if (!pFolder) break;
			// See if the folder matches
			QCMailboxCommand *pCommand2 = pFolder->GetMailboxCommand();
			if (pCommand && pCommand2)
			{
				if (pCommand == pCommand2)
				{
					// This is the folder
					bFound = true;
					pFolder->AutoAddRef();
					*ppResultFolder = pFolder;
					break;
				}
			}
			// Look for sub-folders
			if (bRecursive)
			{
				if (!pFolderList->IsEmpty())
				{
					if (true == RecurseFindFolderByCommand(pCommand, pFolder, ppResultFolder, true))
					{
						bFound = true;
						break;
					}
				}
			}
		}
	}
	return bFound;
}

bool QCAutomationDirector::RecurseFindFolderByID(
	long ID,
	QComFolder* pCurrentFolder, 
	QComFolder** ppResultFolder,
	bool bRecursive)
{
	bool bFound = false;
	QComFolder* pFolder = NULL;

	// Search sub-folders
	POSITION pos;
	POSITION pos2;
	QComFolders* pFolderList = NULL;
	pFolderList = pCurrentFolder->GetFolders();
	if (pFolderList)
	{
		pos = pFolderList->GetHeadPosition();
		while (pos)
		{
			pos2 = pos;
			pFolder = (QComFolder*)pFolderList->GetNext(pos);
			if (!pFolder) break;
			// See if the folder matches
			if (ID == pFolder->m_ID)
			{
				// This is the folder
				bFound = true;
				pFolder->AutoAddRef();
				*ppResultFolder = pFolder;
				break;
			}
			// Look for sub-folders
			if (bRecursive)
			{
				if (!pFolderList->IsEmpty())
				{
					if (true == RecurseFindFolderByID(ID, pFolder, ppResultFolder, true))
					{
						bFound = true;
						break;
					}
				}
			}
		}
	}
	return bFound;
}

void QCAutomationDirector::InternalDeleteFolder(QCMailboxCommand* pc)
{
	QComFolder* pResultFolder = NULL;
	CString strFullName = pc->GetPathname();
	FindFolderByFullName(strFullName, &pResultFolder);
	if (!pResultFolder)
	{
		ASSERT(0);
		return;
	}

	// Notify automation clients
	LPDISPATCH lpDisp = NULL;
	pResultFolder->AutoQueryInterface(IID_IDispatch, (void**)&lpDisp);
	// g_pApplication->FireOnFolderChange(lpDisp, fcThisFolderDeleted);
	g_pApplication->FireOnFolderChange();

	// Deleting the folder is done after the notification
	pResultFolder->Unlink();
	pResultFolder->AutoFree();
}

void QCAutomationDirector::InternalMoveFolder(QCMailboxCommand* pTravel, QCMailboxCommand* pTo)
{
	// Folder has already been moved in the Mailbox Director
	// We need to reflect this in the COM Folder list
	// Note that the FullName has changed

	QComFolder* pFromFolder = NULL;
	QComFolder* pToFolder = NULL;
	QComFolder* pTravelFolder = NULL;

	try
	{
		// Find folders
		FindFolderByCommand(pTravel, &pTravelFolder, true);
		if (!pTravelFolder)
		{
			ASSERT(0);
			return;
		}
		pFromFolder = pTravelFolder->GetParent();
		if (!pTo)
		{
			// moved to root folder
			pToFolder = g_pApplication->GetRootFolder();
		}
		else
		{
			FindFolderByCommand(pTo, &pToFolder, true);
		}
		if ((!pFromFolder) || 
			(!pToFolder) || 
			(!pTravelFolder))
		{
			ASSERT(0);
			return;
		}

		// Unlink the folder from its previous parent
		pTravelFolder->Unlink();

		// Link the folder under the new parent
		QComFolders* pFolderList = NULL;
		pFolderList = pToFolder->GetFolders();
		if (!pFolderList)
		{
			ASSERT(0);
			return;
		}
		pFolderList->InsertAtEnd(pTravelFolder);

		// Update folder FullName, Path, Parent, and Level
		LPCSTR Name = NULL;
		LPCSTR FullName = NULL;
		if (pTo)
		{
			Name = pTo->GetName();
			FullName = pTo->GetPathname();
			if (!Name) return;
			if (!FullName) return;
		}
		else
		{
			Name = ROOTFOLDER;
			FullName = ROOTFOLDER;
		}
		char szDrive[_MAX_DRIVE];
		char szDir[_MAX_DIR];
		_splitpath(FullName, szDrive, szDir, NULL, NULL);
		CString strDrive = szDrive;
		CString strDir = szDir;
		CString strPath = strDrive + strDir;
		CString strb = FullName;
		strb = strb + "\\";
		if (!pTo)
		{
			// moved to root folder
			pTravelFolder->SetFullName(ROOTFOLDER);
			pTravelFolder->SetPath(ROOTFOLDER);
		}
		else
		{
			pTravelFolder->SetFullName(strb);
			pTravelFolder->SetPath(strPath);
		}
		pTravelFolder->SetParent(pToFolder);
		pTravelFolder->SetLevel((short)(pToFolder->GetLevel() + 1));

		// Notify automation clients
		LPDISPATCH lpDisp = NULL;
		pTravelFolder->AutoQueryInterface(IID_IDispatch, (void**)&lpDisp);
		// g_pApplication->FireOnFolderChange(lpDisp, fcThisFolderMoved);
		g_pApplication->FireOnFolderChange();
	}
	catch (...)
	{
		ASSERT(0);
	}

}

void QCAutomationDirector::InternalRenameFolder(QCMailboxCommand* pRename)
{
	// Folder has already been renamed in the Mailbox Director
	// We need to reflect this in the COM Folder list
	try
	{
		QComFolder* pRenameFolder = NULL;
		FindFolderByCommand(pRename, &pRenameFolder, true);
		if (!pRenameFolder)
		{
			ASSERT(0);
			return;
		}
		// Re-initialize
		QComFolder* pf = pRenameFolder->m_pParent;
		QCMailboxCommand* pc = pRenameFolder->m_pCommand;
		pRenameFolder->Init(pf, pc, pRenameFolder->GetLevel());

		// Notify automation clients
		LPDISPATCH lpDisp = NULL;
		pRenameFolder->AutoQueryInterface(IID_IDispatch, (void**)&lpDisp);
		// g_pApplication->FireOnFolderChange(lpDisp, fcThisFolderMoved);
		g_pApplication->FireOnFolderChange();
	}
	catch (...)
	{
		ASSERT(0);
	}

}

void QCAutomationDirector::InternalInsertFolder(QCMailboxCommand* pc)
{
	// Folder has already been inserted into the Mailbox Director
	// We need to insert it into the COM Folder list

	QCMailboxCommand* pParent = NULL;
	QComFolder* pParentFolder = NULL;
	pParent = g_theMailboxDirector.FindParent(pc);
	if (pParent)
	{
		CString strParent = pParent->GetPathname();
		FindFolderByFullName(strParent, &pParentFolder);
		if (!pParentFolder) return;
	}
	else
	{
		pParentFolder = g_pApplication->GetRootFolder();
	}

	// Create the COM folder and insert it
	QComFolder* pNewFolder = pParentFolder->InsertNewChild(pc);

	// Notify automation clients
	LPDISPATCH lpDisp = NULL;
	pNewFolder->AutoQueryInterface(IID_IDispatch, (void**)&lpDisp);
	//g_pApplication->FireOnFolderChange(lpDisp, fcThisFolderAdded);
	g_pApplication->FireOnFolderChange();
}

bool QCAutomationDirector::RemoveMessage(CSummary* pSum)
{
	if (!pSum) return false;
	CRString csTrash(IDS_TRASH_MBOX_NAME);
	QCMailboxCommand* pCommand = g_theMailboxDirector.FindByName(
		g_theMailboxDirector.GetMailboxList(), csTrash);
	if (!pCommand) return false;
	pCommand->Execute(CA_TRANSFER_TO, pSum);
	return true;
}

void QCAutomationDirector::UsePassword(LPCSTR Password)
{
	m_csPass = Password;
}

bool QCAutomationDirector::MoveMessage(CSummary* pSum, QCMailboxCommand* pNewMailbox)
{
	if (!pSum) return false;
	if (!pNewMailbox) return false;
	pNewMailbox->Execute(CA_TRANSFER_TO, pSum);
	return true;
}

void QCAutomationDirector::OnPluginEvent(QCPluginCommand* pc, COMMAND_ACTION_TYPE Action, void* pData)
{

}

void QCAutomationDirector::OnRecipientEvent(QCRecipientCommand* pc, COMMAND_ACTION_TYPE Action, void* pData)
{

}

void QCAutomationDirector::OnSignatureEvent(QCSignatureCommand* pc, COMMAND_ACTION_TYPE Action, void* pData)
{

}

void QCAutomationDirector::OnStationeryEvent(QCStationeryCommand* pc, COMMAND_ACTION_TYPE Action, void* pData)
{

}

void QCAutomationDirector::NotifyClients(QCCommandObject*	pCommand, COMMAND_ACTION_TYPE uAction, void* pData)
{

}

