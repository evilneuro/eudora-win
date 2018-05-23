// ImapChecker.h
//
// Does the actual work of seeing what messages in a given mailbox need to be downloaded
// and downloading those messages, assigning junk scores if appropriate and filtering them
// if the mailbox is the Inbox.
//
// Copyright (c) 1997-2004 by QUALCOMM, Incorporated
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

#ifndef __IMAPCHECKER_H
#define __IMAPCHECKER_H

#include "QCWorkerThreadMT.h"
#include "ImapTypes.h"

class CTocDoc;
class CTaskInfoMT;
class CImapSumList;
class QCPOPNotifier;

class CImapChecker : public QCWorkerThreadMT
{
public:
	CImapChecker(LPCSTR szPersona, LPCSTR szImapName, unsigned long ulCheckBitFlags);
	virtual ~CImapChecker();

// Thread initialization.
public:
	HRESULT		 DoMainThreadInitializations(CTocDoc *pTocDoc);

private:
	HRESULT		 AcquireNetworkConnection();

// Thread work.
public:
	// Functions for setting up the CImapChecker object for the appropriate background task.
	void		 SetCheckMailWorkFunction();
	void		 SetResyncWorkFunction(BOOL bDownloadedOnly, unsigned long ulMaxUid);

	HRESULT		 DoWork();

private:
	void		 InitTaskInfo();
	void		 ShowCheckMailError();
	BOOL		 StartResyncProgress();
	void		 StopResyncProgress(BOOL bWasInProgress);
	void		 StartDownloadProgress(int iProgressInc, CTaskInfoMT *pTaskInfo);
	void		 UpdateDownloadProgress(int iProgressInc, CTaskInfoMT *pTaskInfo);
	HRESULT		 DoCheckMailMT();
	HRESULT		 SetupConnectionMT();
	HRESULT		 GetNewMboxState();
	HRESULT		 FetchNewUIDsMT();
	HRESULT		 DoResyncMailboxMT();
	HRESULT		 DownloadNewMessagesToTmpTocMT();
	HRESULT		 DownloadRemainingMessagesMT(CUidMap &mapNewUIDs,
											 CImapSumList *pSumList,
											 BOOL bToTmpMbx,
											 CTaskInfoMT *pTaskInfo);
	HRESULT		 DoMinimalDownload(CUidMap &uidmap,
								   CImapSumList *pSumList,
								   unsigned long *puiHighestUID);
	HRESULT		 DownloadSingleMessage(CImapSum *pImapSum, BOOL bDownloadAttachments, BOOL bToTmpMbx);
	HRESULT      DownloadSingleMessageFully(CImapSum *pSum);


// Thread post processing.
public:
	HRESULT		 DoMergeNewTocs();

private:
	void		 ShowPostProcessProgress(CString &strTocName);
	BOOL		 PostProcessSummaries();
	HRESULT		 HandlePostFetch(CImapSumList &islSummaries);
	HRESULT		 DoJunkProcessing(CImapSumList& imapSumList);
	void		 ProcessMessageForJunk(CImapSum *pImapSum, CObArray *poaABHashes, bool bUsingWhiteList);
	bool		 MaybeJunkTheMessage(CImapSum *pImapSum);
	void		 DoPostFiltering(CSumList &slNewSummaries);
	void		 DoPostFilterExpunge(CString &strUIDList);
	void		 RedrawTOC(CTocView *pView);
	void		 SetNotificationFlags();

// Error callback
	void		 ErrorCallback(LPCSTR szStr, UINT iErrorType);

//	Unused - Should it be?
public:
	void		 RequestThreadStop();

	
// Data access
public:
	void		 SetNewMailNotifier(QCPOPNotifier *pNotifier);

	void		 SetTaskKey(LPCSTR pTaskKey) {m_strTaskKey = pTaskKey;}
	void		 SetReadOnly(BOOL bVal) {m_bIsReadOnly = bVal;}

	CUidMap		&GetInternalCurrentUidMap() {return m_mapCurrentUIDs;}


	void		 SetTocName(LPCSTR pTocName) {m_strTocName = pTocName;}
	LPCSTR		 GetTocName() {return m_strTocName;}

	BOOL		 IsInbox() {return m_bIsInbox;}

	void		 ForceResync() {m_bDoResync = TRUE;}


private:
	// We deal only with a CImapConnection object here.
	// We MUST have one before we can do anything. We would
	// grab one from the Connections Manager. This MUST be one
	// that's shared with the CImapMailbox in the TOC.
	CImapConnection		*m_pImapConnection;

	typedef enum
	{
		WORKERFUNCTION_TYPE_BADTYPE		= 0,
		WORKERFUNCTION_TYPE_CHECKMAIL	= 1,
		WORKERFUNCTION_TYPE_RESYNC		= 2
	} WORKERFUNCTIONTYPE;

	WORKERFUNCTIONTYPE	 m_enumWorkerFunctionType;

	// We're associated with a single account.
	CString				 m_strPersona;
	unsigned long		 m_ulAccountID;

	// The TOC associated with the mailbox we are checking.
	CTocDoc				*m_pTocDoc;

	// We MUST have a task key.
	//
	CString				 m_strTaskKey;

	// Name of the remote mailbox.
	CString				 m_strImapName;

	// Set these based on m_strImapName;
	BOOL				 m_bIsInbox;
	BOOL				 m_bIsJunk;

	// Full path to the real MBX file.
	// Must be set before starting the background stuff.
	CString				 m_strRealMbxFilePath;

	// This is the name of the temporary MBX file to which we may be streaming stuff.
	// Must be set before starting the background stuff.
	CString				 m_strTmpMbxFilePath;

	// This is what is returned by CTocDoc->Name().
	CString				 m_strTocName;

	// Whether to show progress in main thread.
	BOOL				 m_bInMainTreadProgress;
	BOOL				 m_bSilent;

	// Type of download we're doing.
	BOOL				 m_bDoingMinimalDownload;

	// Maximum size of attachments to download.
	unsigned long		 m_ulMaxAttachmentSizeToDownload;

	// User wants to quit?
	BOOL				 m_bStopRequested;

	// Set to TRUE if we find out that the mailbox is read-only.
	BOOL				 m_bIsReadOnly;

	// Set from the CImapMailbox and must be used to reset CImapMailbox::->m_NumberOfMessages;
	unsigned long		 m_ulOldNumberOfMessages;
	unsigned long		 m_ulNewNumberOfMessages;

	// Highest UID on server this round.
	unsigned long		 m_ulServerUidHighest;

	// Highest UID downloaded.
	unsigned long		 m_ulOldUIDHighest;
	unsigned long		 m_ulNewUIDHighest;

	// UIDVALIDITY management.
	unsigned long		 m_ulOldUidValidity;
	unsigned long		 m_ulNewUidValidity;

	// If set, clear local cache before re-syncing.
	BOOL				 m_bRemoveAllLocalSumsBeforeMerging;

	// UIDs of messages currently in the cache.
	CUidMap				 m_mapCurrentUIDs;
	// UIDs of new messages on the server that need to be downloaded.
	CUidMap				 m_mapNewUIDs;
	// UIDs of old messages on the server that we need to download (without filtering even for INBOX).
	CUidMap				 m_mapNewOldUIDs;
	// UIDs of already downloaded messages with possibly modified flags.
	CUidMap				 m_mapModifiedUIDs;

	// Summaries of new messages on the server that need to be downloaded.
	CImapSumList		 m_islNewImapSummaries;
	// Summaries of old messages on the server that need to be downloaded (without filtering even for INBOX).
	CImapSumList		 m_islNewOldImapSummaries;

	// Summaries left after filtering
	CTempSumList		 m_slPostFilterSummaries;

	// This is what must be set to force a re-synchronization.
	BOOL				 m_bDoResync;

	// And an expunge before a resync.
	BOOL				 m_bDoExpunge;

	BOOL				 m_bGotNewMail;

	unsigned long		 m_ulBitFlags;

	// ============= Used during a mailbox resync ==========//
	unsigned long		 m_ulMaxUidToResync;
	BOOL				 m_bDownloadedOnlyDuringResync;

	// Number of new messages received:
	unsigned long		 m_ulNewMsgs;

	QCPOPNotifier		*m_pNewMailNotifier;

	// This is passed to the m_pImap methods.
	ImapErrorFunc		 m_pErrorCallback;
};

#endif // __IMAPCHECKER_H