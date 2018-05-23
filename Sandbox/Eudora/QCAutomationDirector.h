// QCAutomationDirector.h
//

#ifndef _QCAutomationDirector_h_
#define _QCAutomationDirector_h_

#include "QICommandClient.h"
#include "QCCommandObject.h"
#include "QCCommandDirector.h"

class CPtrList;
class QCAutomationCommand;
class QCMailboxCommand;
class QCRecipientCommand;
class QCPluginCommand;
class QCStationeryCommand;
class QCSignatureCommand;
class QComFolder;
class QComApplication;
class CSummary;

class QCAutomationDirector : 
	public QCCommandDirector, 
	public QICommandClient
{
public:
	friend QComApplication;
	QCAutomationDirector();
	virtual ~QCAutomationDirector();
	void Start(void);
	void Stop(void);
	void ExecuteCommand(void);
	bool BuildFolderList(QComFolder* pCurrentFolder, CPtrList* pSearchList, short Level);
	void InternalDeleteFolder(QCMailboxCommand* pc);
	void InternalInsertFolder(QCMailboxCommand* pc);
	void InternalMoveFolder(QCMailboxCommand* pTravel, QCMailboxCommand* pTo);
	void InternalRenameFolder(QCMailboxCommand* pc);
	bool FindFolderByID(long ID, QComFolder* pParentFolder, QComFolder** ppResultFolder, bool bRecursive = true);
	bool FindFolderByName(LPCSTR Name, QComFolder* pParentFolder, QComFolder** ppResultFolder, bool bRecursive = true);
	bool FindFolderByName(LPCSTR Name, QComFolder** ppResultFolder, bool bRecursive = true);
	bool FindFolderByFullName(LPCSTR FullName, QComFolder* pParentFolder, QComFolder** ppResultFolder, bool bRecursive = true);
	bool FindFolderByFullName(LPCSTR FullName, QComFolder** ppResultFolder, bool bRecursive = true);
	bool FindFolderByCommand(QCMailboxCommand* pCommand, QComFolder** ppResultFolder, bool bRecursive);
	bool FindFolderByID(long ID, QComFolder** ppResultFolder, bool bRecursive);
	bool RecurseFindFolder(short SearchType, LPCSTR SearchString, QComFolder* pCurrentFolder, QComFolder** ppResultFolder, bool bRecursive = true);
	bool RecurseFindFolderByCommand(QCMailboxCommand* pCommand,	QComFolder* pCurrentFolder, QComFolder** ppResultFolder, bool bRecursive);
	bool RecurseFindFolderByID(long ID, QComFolder* pCurrentFolder, QComFolder** ppResultFolder, bool bRecursive);
	void Notify(QCCommandObject* pc, COMMAND_ACTION_TYPE Action, void* pData);
	void NotifyClients(QCCommandObject*	pCommand, COMMAND_ACTION_TYPE uAction, void*pData = NULL);
	void OnMailboxEvent(QCMailboxCommand* pc, COMMAND_ACTION_TYPE Action, void* pData);
	void OnPluginEvent(QCPluginCommand* pc, COMMAND_ACTION_TYPE Action, void* pData);
	void OnRecipientEvent(QCRecipientCommand* pc, COMMAND_ACTION_TYPE Action, void* pData);
	void OnStationeryEvent(QCStationeryCommand* pc, COMMAND_ACTION_TYPE Action, void* pData);
	void OnSignatureEvent(QCSignatureCommand* pc, COMMAND_ACTION_TYPE Action, void* pData);
	void OnMessage(DWORD wParam, DWORD lParam);
	bool RemoveMessage(CSummary* pSum);
	bool MoveMessage(CSummary* pSum, QCMailboxCommand* pNewMailbox);
	void UsePassword(LPCSTR Password);
public:
	DECLARE_DYNAMIC(QCAutomationDirector)
protected:
	bool m_bRunning;
	bool m_bCommandCheckMail;
	bool m_bCommandSendMail;
	bool m_bCommandEmptyTrash;
	bool m_bCommandCompactFolders;
	CString m_csPass;
};

extern QCAutomationDirector g_theAutomationDirector;

#endif // _QCAutomationDirector_h_
