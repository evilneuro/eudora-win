// ImapChecker.h - Interface to the object that does the background or foreground 
// mail checking.
//
#ifndef __IMAPCHECKER_H
#define __IMAPCHECKER_H

#include "QCWorkerThreadMT.h"
#include "ImapTypes.h"

class CPreFilterActions;
class CTaskInfoMT;
class CImapSumList;
class QCPOPNotifier;

class CImapChecker : public QCWorkerThreadMT
{
public:
	CImapChecker(LPCSTR pszPersona, LPCSTR pImapName, unsigned long ulCheckBitFlags);
	virtual ~CImapChecker();

// Worker function type:
//
	void RequestThreadStop();

	BOOL CreatePreFilterActions (CTocDoc* pSrcToc);

	// Interface to setting up the object for it's background task.
	//
	void	SetCheckMailWorkFunction ();
	void	SetResyncWorkFunction (BOOL bDownloadedOnly, unsigned long ulMaxUid);

	// Get current state of the local cache from the CImapMailbox associated with the TOC.
	//
	HRESULT DoMainThreadInitializations (CTocDoc* pTocDoc);

	// Called by the thread manager to do the first stage of the fetching.
	//
	HRESULT DoWork();

	// Post processing.
	//
	HRESULT		DoMergeNewTocs();

	// We MUST be given a task key.
	//
	void SetTaskKey (LPCSTR pTaskKey)
		{ m_szTaskKey = pTaskKey; }

	void SetReadOnly (BOOL bVal)
		{ m_bIsReadOnly = bVal; }

	// Return a reference to the internal list.
	//
	CPtrUidList& GetInternalCurrentUidList ()
		{ return m_CurrentUidList; }

	CUidMap& GetInternalCurrentUidMap ()
		{ return m_CurrentUidMap; }


	void SetTocName (LPCSTR pTocName)
		{ m_szTocName = pTocName; }

	LPCSTR GetTocName ()
		{ return m_szTocName; }

	BOOL IsInbox ()
		{ return m_bIsInbox; }

	void ForceResync ()
		{ m_bDoResync = TRUE; }

	void ForceExpunge ()
		{ m_bDoExpunge = TRUE; }

// Public members - uh uh!!
public:
	// THis is contained by reference. Don't delete!!
	//
	void  SetNewMailNotifier (QCPOPNotifier *pNotifier)
			{ m_pNewMailNotifier = pNotifier; }

	QCPOPNotifier*	m_pNewMailNotifier;

	// Called when we're done.
	void SetNotificationFlags ();


// Internal methods.
private:
	HRESULT		SetupConnectionMT ();
	HRESULT		AcquireNetworkConnection ();

	HRESULT		DoPrefilterMT ();

	HRESULT		GetNewMboxState ();

	HRESULT		FetchNewUidsMT ();

	HRESULT		DownloadNewMessagesToTmpTocMT ();

	HRESULT		DownloadRemainingMessagesMT (CUidMap& NewUidList,
							 CImapSumList *pSumList, BOOL bToTmpMbx, CTaskInfoMT* pTaskInfo);

	// Worker function for checking mail.
	//
	HRESULT		DoCheckMailMT ();

	// Worker function for resyncing a mailbox.
	//
	HRESULT		DoResyncMailboxMT ();

	// Do the download.
	//
	HRESULT		DoMinimalDownload (unsigned long uid, CImapSum** pImapSum);

	HRESULT		DownloadSingleMessage (CImapSum* pImapSum, BOOL bDownloadAttachments, BOOL bToTmpMbx);

	void		UpdateUIDHighest ( CImapMailbox *pImapMailbox, unsigned long val);

	HRESULT		HandlePostFetch (CImapSumList& ImapSumList);

	void		DoPostFiltering (CTocDoc* pTocDoc, CSumList& NewSumList);

	void		ErrorCallback (LPCSTR str, UINT ErrorType);

private:
	// We deal only with a CIMAP object here.
	// We MUST have one before we can do anything. We would
	// grab one from the Connections Manager. THis MUST be one
	// that's shared with the CImapMailbox in the TOC.
	//
	CIMAP*	m_pImap;

	// This is passed to the m_pImap methods.
	//
	ImapErrorFunc  m_pErrorCallback;


private:
	typedef enum
	{
		WORKERFUNCTION_TYPE_BADTYPE		= 0,
		WORKERFUNCTION_TYPE_CHECKMAIL	= 1,
		WORKERFUNCTION_TYPE_RESYNC		= 2

	} WORKERFUNCTIONTYPE;

	WORKERFUNCTIONTYPE	m_WorkerFunctionType;

	// We're associated with a single account.
	CString			m_szPersona;
	unsigned long	m_AccountID;

	// We MUST have a task key.
	//
	CString			m_szTaskKey;

	// And with a single remote mailbox.
	CString		m_szImapName;

	// Set this based on m_szImapName;
	BOOL		m_bIsInbox;

	// Full path to the real MBX file.
	// Must be set before starting the background stuff.
	//
	CString		m_szRealMbxFilePath;

	// This is the name of the temporary MBX file to which we may be streaming stuff.
	// Must be set before starting the background stuff.
	//
	CString		m_szTmpMbxFilePath;

	// This is what woudl be returned by tocdoc->Name().
	//
	CString		m_szTocName;

	// Whether to show progress in main thread.
	//
	BOOL		m_bInMainTreadProgress;
	BOOL		m_bSilent;

	// Type of download we're doing.
	//
	BOOL		m_bDoingMinimalDownload;

	unsigned long m_ulMaxAttachmentSizeToDownload;

	// Uses one of these to do the communication with the IMAP sever.
	// Note: This is containment by reference.
	// 
	// User wants to quit?
	BOOL	m_bStopRequested;

	// Set to TRUE if we find out that the mailbox is read-only.
	//
	BOOL	m_bIsReadOnly;

	// Set from the CImapMailbox and must be used to reset
	// CImapMailbox::->m_NUmberOfMessages;
	//
	unsigned long	m_OldNumberOfMessages;
	unsigned long	m_NewNumberOfMessages;

	// Highest UID on server this round.
	unsigned long	m_ServerUidHighest;

	// Manage Highest UID downloaded.
	//
	unsigned long	m_OldUIDHighest;
	unsigned long	m_NewUIDHighest;

	// UIDVALIDITY management.
	//
	unsigned long	m_OldUidValidity;
	unsigned long	m_NewUidValidity;

	// Determine how to re-sync.
	//
	// If set, clear local cache before re-syncing.
	//
	BOOL		m_bRemoveAllLocalSumsBeforeMerging;

	//===== UID lists that we manipulate. ====

	// Will be filled with the list of already-downloaded uid's.
	//
	CPtrUidList		m_CurrentUidList;

	// Contains UID's of message we've never seen. We may have to filter them.
//	CPtrUidList		m_NewUidList;

	// Contains possibly modified flags of already downloaded messages.
	CPtrUidList		m_ModifiedUidList;

	// Contains UID's of messages we need to download without filtering (even for INBOX).
//	CPtrUidList		m_NewOldUidList;

// USE MAP now!!
//
	CUidMap			m_CurrentUidMap;

	// Contains UID's of message we've never seen. We may have to filter them.
	CUidMap			m_NewUidMap;

	// Contains possibly modified flags of already downloaded messages.
	CUidMap			m_ModifiedUidMap;

	// Contains UID's of messages we need to download without filtering (even for INBOX).
	CUidMap			m_NewOldUidMap;


	// For m_NewUidMap and m_NewOldUidMap, we download corresponding CImapSumLilst's
	// in the background and then merge them in the post-processing phase.
	//
	CImapSumList	m_NewImapSumList;
	CImapSumList	m_NewOldImapSumList;	

	// These are real CSummary's now!!
	//
	CSumList		m_PostFilterSumList;


	// For prefiltering.
	//

	CPreFilterActions*	m_pPreFilterActions;

	// This is what must be set to force a re-synchronization.
	//
	BOOL				m_bDoResync;

	// And an expunge before a resync.
	//
	BOOL				m_bDoExpunge;

	// 
	BOOL				m_bGotNewMail;

	unsigned long		m_ulBitFlags;

	// ============= Used during a mailbox resync ==========//
	unsigned long		m_ulMaxUidToResync;
	BOOL				m_bDownloadedOnlyDuringResync;

	// Number of new messages received:
	unsigned long		m_ulNewMsgs;

};


#endif // __IMAPCHECKER_H