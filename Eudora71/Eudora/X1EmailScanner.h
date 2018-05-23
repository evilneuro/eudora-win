//	X1EmailScanner.h
//
//	Scans email to build X1 indices.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#ifndef __X1EmailScanner_H__
#define __X1EmailScanner_H__

#include "SearchManager.h"

//	X1 Include
#include "X1Wrapper.h"

#include "fileutil.h"

//	Forward declarations and enums
class CSummary;
class CTocDoc;
class QCCommandObject;
class QCMailboxCommand;
class SearchManagerTaskInfo;
extern enum COMMAND_ACTION_TYPE;


//	Email scanner implementation. The scanner is called by the scanner
//	manager to iterate through the message store and add non-indexed 
//	items to the X1 database.
class X1EmailScanner : public X1::IX1Scanner
{
  public:
	//	Typedefs
	typedef enum
	{
		ss_Stopped = 0,
		ss_Running,
		ss_Paused
	} ScanStateEnum;

	typedef enum
	{
		st_None = 0,
		st_ReadyForNextScanType,
		st_AddOrUpdateItemsScan,
		st_CullingScan,
		st_ProcessingIndexUpdateActions
	} ScanTypeEnum;

	typedef enum
	{
		sd_MainScans,
		sd_IndexUpdateActions,
		sd_Total
	} ScanDataEnum;

	//	Typedef to hoist IndexUpdateAction into our namespace
	typedef SearchManager::IndexUpdateAction		IndexUpdateAction;
	typedef SearchManager::UpdateActionEnum			UpdateActionEnum;

	//	Methods
										X1EmailScanner(
											X1::IX1Database *			in_pX1DB);
	virtual								~X1EmailScanner();

	long								GetScanCount() const { return m_nScanCount; }

	void								SetConnectionHandle(unsigned long in_nConnectionHandle) { m_nConnectionHandle = in_nConnectionHandle; }
		//	Sets the scanner manager connection handle

	unsigned long						GetConnectionHandle() const { return m_nConnectionHandle; }
		//	Gets the scanner manager connection handle

	virtual HRESULT __stdcall			QueryInterface(
											const IID &					in_iid,
											void **						out_ppvObject);
		//	Standard IUnknown::QueryInterface() implementation

	virtual ULONG __stdcall				AddRef();
		//	Standard IUnknown::AddRef() implementation

	virtual ULONG __stdcall				Release();
		//	Standard IUnknown::Release() implementation

	virtual HRESULT __stdcall			raw_ScanItem(
											unsigned char *				out_pbDone);
		//	Called by the scanner manager to index the next item. This should generally 
		//	only scan a single item at a time. pbDone should be set to FALSE if there are
		//	more messages to scan, otherwise it should be left untouched.

	bool								IsScanning() const;
	bool								IsDoingFullScan() const;

	void								StartScan(
											ScanTypeEnum				in_eScanType,
											ScanTypeEnum				in_eNextScanType = st_None,
											const char *				in_szIndexingMailboxRelativePath = NULL);
		//	Starts scanning

	void								PauseScan(
											bool						in_bWaitingForIdle);
		//	Pauses scanning

	void								ResumeScan();
		//	Resumes scanning

	void								StopCurrentScan();
		//	Notes that we're finished with the current scan

	void								StopAllScanning();
		//	Stops all scanning

	void								NotifySumRemoval(
											const CTocDoc *				in_pTocDoc,
											POSITION					in_posSum);
	void								NotifyTemporarilyCloseMailbox(
											const char *				in_szMailboxRelativePath);
	void								NotifyRestartProcessingMailbox(
											const char *				in_szMailboxRelativePath);
	void								NotifyStopProcessingMailbox(
											const char *				in_szMailboxRelativePath);

	void								InitAddOrUpdateItemsScan(
											const char *				in_szContinueIndexingMailbox = NULL);
	void								InitCullingScan(
											const char *				in_szContinueIndexingMailbox = NULL);

	bool								AddOrUpdateMailboxMessages(
											ScanDataEnum				in_eScanData);
	void								DoAddOrUpdateItemsScan();
	void								DoCullingScan();
	void								ProcessNextIndexUpdateItem();
	bool								ProcessRemoveOrUpdateMessage(
											IndexUpdateAction *			in_pUpdateAction,
											X1::IX1OneUsePtr &			in_pOneUse);
	bool								ProcessAddMessage(
											IndexUpdateAction *			in_pUpdateAction,
											X1::IX1OneUsePtr &			in_pOneUse);
	bool								ProcessRemoveOrReindexMailbox(
											IndexUpdateAction *			in_pUpdateAction,
											X1::IX1OneUsePtr &			in_pOneUse);
	bool								ProcessAddMailbox(
											IndexUpdateAction *			in_pUpdateAction,
											X1::IX1OneUsePtr &			in_pOneUse);

	void								NextMailbox();
	void								OpenCurrentMailbox(
											ScanDataEnum				in_eScanData);
	void								CloseCurrentMailbox(
											ScanDataEnum				in_eScanData,
											bool						in_bOkToCloseTOC = true);

	bool								HasIndexedMessages(
											const char *				in_szMailboxRelativePath,
											X1::IX1ItemListPtr &		out_pMailboxItemList);
	bool								IsIndexed(
											const char *				in_szMailboxRelativePath,
											long						in_nUniqueMessageID,
											long &						out_nX1ItemNum);
	bool								DoesIndexedItemMatchSummary(
											CSummary *					in_pSummary,
											long						in_nX1ItemNum);
	bool								AddToIndex(
											ScanDataEnum				in_eScanData,
											X1::IX1OneUse *				in_pOneUse,
											CSummary *					in_pSummary,
											bool &						out_bWaitingForX1);

  protected:

										X1EmailScanner();	//	Disallow default constructor

	//	Common data members
	long								m_nRefCount;							//	Ref count needed for X1 COM purposes
	X1::IX1Database *					m_pEmailDB;								//	Pointer to X1 database.
	unsigned long						m_nConnectionHandle;					//	Connection handle for this object's registration with X1.

	//	Scan task management data members
	long								m_nScanCount;							//	Scan items process. Used to keep track if work was done.
	ScanStateEnum						m_eScanState;							//	Current scan state.
	ScanTypeEnum						m_eLastScanType;						//	Last scan type.
	ScanTypeEnum						m_eScanType;							//	Current scan type.
	ScanTypeEnum						m_eNextScanType;						//	Next scan type.
	bool								m_bProcessingIndexUpdateActions;		//	Whether or not we're processing index update actions

	//	All scan types shared data members
	CTocDoc *							m_pCurrentTocDoc[sd_Total];				//	TOC doc for current mailbox
	JJFile								m_CurrentMailboxFile[sd_Total];			//	File object for current mailbox
	CString								m_szMailboxRelativePath[sd_Total];		//	Current mailbox being scanned
	BOOL								m_bMailboxFileExists[sd_Total];			//	Whether or not current mailbox file exists
	POSITION							m_posCurrentSummary[sd_Total];			//	Position of current summary in current mailbox
	POSITION							m_posLastSummaryAddAttempt[sd_Total];	//	Position of the last summary we attempted to add
	long								m_nCurrentMessage[sd_Total];			//	Which number message we're processing - used for progress
	int									m_nSoFarBase[sd_Total];					//	Current progress base
	bool								m_bCheckForIndexedMessages[sd_Total];	//	Whether or not mailbox has been indexed at all yet - allows optimization
	bool								m_bCheckingMessages[sd_Total];			//	When adding messages, whether or not we're currently checking for indexed message
	bool								m_bNeedToAddMessages[sd_Total];			//	When adding messages, whether or not we found any that we need to add
	CString								m_szMessageBuffer;						//	Buffer used to read in message contents

	//	Index update scan data members
	long								m_nMessageIDLastAddAttempt;				//	Message ID of last summary we attempted to add
	SearchManagerTaskInfo *				m_pUpdateActionsTaskInfo;				//	Task info for progress display
	long								m_nTotalUpdateActions;					//	Total number of update actions for progress display
	long								m_nCurrentUpdateAction;					//	Current update action for progress display
	CString								m_szMailboxName;						//	Name of current mailbox (for progress)
	long								m_nTotalMessagesToRemove;				//	Total number of messages to remove (for progress)

	//	Add/update and culling scan shared data members
	SearchManagerTaskInfo *				m_pMainScansTaskInfo;					//	Task info for progress display

	//	Add/update scan data members
	CPtrList *							m_pMailboxList;							//	List of mailboxes inside of a mailfolder
	POSITION							m_posCurrentMailbox;					//	Position of current mailbox inside of mailbox list
	QCMailboxCommand *					m_pCurrentMailbox;						//	Current mailbox command object
	long								m_nTotalMailboxes;						//	Total number of mailboxes we're processing - used for progress
	long								m_nCurrentMailbox;						//	Which number mailbox we're processing - used for progress

	//	Culling scan data members
	X1::IX1ItemListPtr					m_pX1IndexedItems;						//	List of all items indexed by X1
	long								m_nNumX1IndexedItems;					//	Number of X1 indexed items
	long								m_nX1IndexedItem;						//	Current X1 indexed item number
};

#endif	//	__X1EmailScanner_H__
