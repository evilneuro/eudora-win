//	SearchManagerInfo.h
//
//	Search Manager information and related methods.
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



#ifndef __SearchManagerInfo_H__
#define __SearchManagerInfo_H__

#pragma once

#include "SearchManager.h"
#include "X1EmailScanner.h"
#include "X1EventListener.h"

#include <map>
#include <memory>

//	X1 Include
#include "X1Wrapper.h"

#include "tocdoc.h"


class SearchManager::UpdateIdentifier
{
  public:
										UpdateIdentifier(
											const char *				in_szMailboxPath,
											long						in_nMessageID);

	bool								operator <(
											const UpdateIdentifier &	in_rhs) const;

  protected:
	CString								m_szMailboxRelativePath;
	long								m_nMessageID;
};


class SearchManager::IndexUpdateAction
{
  public:
	//	Typedefs

	//	Methods
										IndexUpdateAction(
											UpdateActionEnum			in_eUpdateAction,
											const char *				in_szMailboxPath,
											long						in_nMessageID);

	//	Accessors
	UpdateActionEnum					GetUpdateAction() const { return m_eUpdateAction; }
	void								SetUpdateAction(UpdateActionEnum in_eUpdateAction) { m_eUpdateAction = in_eUpdateAction; }

	const CString &						GetMailboxRelativePath() const { return m_szMailboxRelativePath; }
	void								SetMailboxRelativePath(const char * in_szMailboxPath) { m_szMailboxRelativePath = in_szMailboxPath; }

	long								GetMessageID() const { return m_nMessageID; }
	void								SetMessageID(long in_nMessageID) { m_nMessageID = in_nMessageID; }

	bool								IsUpdate() const { return m_bIsUpdate; }
	void								SetIsUpdate(bool in_bIsUpdate) { m_bIsUpdate = in_bIsUpdate; }

  protected:
	//	Disallow default constructor
										IndexUpdateAction();

	//	Persistent data members
	UpdateActionEnum					m_eUpdateAction;			//	Type of update - add, update, or remove
	CString								m_szMailboxRelativePath;	//	Path to mailbox
	long								m_nMessageID;				//	Message ID

	//	Ephemeral data members
	bool								m_bIsUpdate;				//	Temporary value that enables us to display better progress
};


class SearchManager::Info
{
  public:
	//	Constants
	static const long					kDBID = 1;	
	static const long					kDBSchemaVersion = 6;

	static const long					kIgnore = -1;

	static const BOOL					kIsSmall = FALSE;
	static const BOOL					kIsBig = TRUE;

	static const BOOL					kIsNotColumn = FALSE;
	static const BOOL					kIsColumn = TRUE;

	static const BOOL					kIsNotMainContent = FALSE;
	static const BOOL					kIsMainContent = TRUE;

	static const long					kUseDefaultMaxWordLength = -1;

	static const long					kJunkScoreWordLength = 100;
	static const long					kAttachmentCountWordLength = 100;

	static const DWORD					kSaveFrequency = 10 * 60 * 1000;		//	Save when dirty once every 10 minutes

	//	Internal class declarations

	//	Typedefs
	typedef enum
	{
		d_NotDirty,
		d_InfoDirty,
		d_X1Dirty,
		d_BothDirty
	} DirtyEnum;

	typedef SearchManager::UpdateIdentifier								UpdateIdentifier;
	typedef SearchManager::IndexUpdateAction							IndexUpdateAction;

	typedef std::map<UpdateIdentifier, IndexUpdateAction *>				IndexUpdateActionMapT;
	typedef IndexUpdateActionMapT::iterator								IndexUpdateActionMapIteratorT;

	typedef std::deque<SortType>										SortColumnsT;
	typedef SortColumnsT::iterator										SortColumnsIteratorT;

	//	Construction/destruction
										Info();
										~Info();

	void								Initialize();

	//	Accessors
	bool								IsX1Loaded() const { return (m_pX1 != NULL) && (m_pX1EmailDB != NULL); }
	bool								InitializationFailed() const { return m_bInitializationFailed; }

	void								SetDirty(
											DirtyEnum					in_eDirty,
											bool						in_bNoSmartSet = false);

	//	Data storage
	void								LoadData();
	void								SaveData();

	//	Initializing and releasing X1
	HRESULT								InitX1();
	HRESULT								LoadX1();
	void								InitX1DB();
	void								ReleaseX1();

	//	Static methods
	static BOOL							GetDirectory(
											CString &					out_szSearchDir,
											bool						in_bThrowExceptionOnFailure = false);

	//	Data members
	bool								m_bInitializationFailed;			//	Whether or not initialization failed
	bool								m_bIndexedSearchOnCached;			//	Whether or not indexed search was on the last time we checked
	bool								m_bNeedToInitiateScan;				//	Whether or not we need to initiate scanning
	HMODULE								m_hX1Lib;							//	The X1 DLL
	X1::IX1Ptr							m_pX1;								//	The X1 runtime
	X1::IX1ScannerManagerPtr			m_pX1ScannerManager;				//	The X1 Scanner Manager object
	X1::IX1DatabasePtr					m_pX1EmailDB;						//	The X1 Email Database
	X1::IX1SearchPtr					m_pX1Search;						//	The X1 Search Object
	CComPtr<X1EventListener>			m_pX1EventListener;					//	Receives all X1 events
	X1::IX1EventListenerPtr				m_pIX1EventListener;				//	Interface for general X1 events
	X1::IX1DbEventListenerPtr			m_pIX1DBEventListener;				//	Interface for X1 database events
	CComPtr<X1EmailScanner>				m_pX1EmailScanner;					//	Email scanner
	LPFNGETCLASSOBJECT					m_pX1DllGetClassObject;				//	Function pointer to X1's DllGetClassObject
	LPFNCANUNLOADNOW					m_pX1DllCanUnloadNow;				//	Function pointer to X1's DllCanUnloadNow
	X1EmailScanner::ScanTypeEnum		m_eScanType;						//	Current scan type
	X1EmailScanner::ScanTypeEnum		m_eNextScanType;					//	Next scan type
	bool								m_bPauseExtendedIdleProcessing;		//	Whether or not we should temporarily pause extended idling
	DirtyEnum							m_eDirty;							//	Whether or not index scanning is dirty (i.e. needs to be saved)
	DWORD								m_nLastSavedTickCount;				//	When we last saved
	CString								m_szIndexingMailboxRelativePath;	//	Current mailbox being indexed
	IndexUpdateActionMapT				m_mapIndexUpdateActions;			//	Map of updates to apply to index (to find items quickly by UpdateIdentifier)
	long								m_nNumMailboxUpdateActions;			//	Number of mailbox update actions in m_mapIndexUpdateActions
	time_t								m_nLastScanCompletionTime;			//	Last time we finished scanning email messages to update X1 indices
	SortColumnsT						m_sortColumns;						//	List of sort columns for search results
	bool								m_bX1DBNeedsCleared;				//	Whether or not X1 DB needs to be cleared and rebuilt
	bool								m_bX1ExceptionFlagged;				//	Whether or not an X1 exception has occurred recently
};


#endif		//	__SearchManagerInfo_H__
