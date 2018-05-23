//	SearchManager.cpp
//
//	Manages X1 indexed search.
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



#include "stdafx.h"
#include "SearchManager.h"
#include "SearchManagerInfo.h"
#include "SearchManagerPersistence.h"
#include "SearchManagerSearching.h"
#include "SearchManagerUtils.h"
#include "DebugNewHelpers.h"
#include <memory>

#include "fileutil.h"
#include "guiutils.h"
#include "mainfrm.h"
#include "progress.h"
#include "QCCommandStack.h"
#include "QCSharewareManager.h"
#include "SearchCriteria.h"
#include "SearchResult.h"
#include "SearchView.h"
#include "tocdoc.h"

//	X1 Include
#include "X1Wrapper.h"
#include "X1EmailScanner.h"
#include "X1EventListener.h"

//	Externs
extern QCCommandDirector		g_theSearchDirector;
extern QCCommandStack			g_theCommandStack;

//	Static pointer to the one and only instance of the Search Manager
SearchManager *		SearchManager::s_pSearchManager = NULL;

//	Constants

//	When indexing and searching, we prefix all relative paths with
//	"UniqueRootPrefixForEudoraMailboxPaths" to ensure that we can uniquely find
//	mailboxes. 
//	For example consider the following directory structure:
//	In.mbx
//	Foo\Bar\In.mbx
//	Test\Foo\Bar\In.mbx
//	
//	If we were searching for "In.mbx" or "Foo\Bar\" we would incorrectly find
//	the other mailboxes that contain those sub-paths. By instead indexing the
//	above mailboxes as:
//	UniqueRootPrefixForEudoraMailboxPaths In.mbx
//	UniqueRootPrefixForEudoraMailboxPaths Foo\Bar\In.mbx
//	UniqueRootPrefixForEudoraMailboxPaths Test\Foo\Bar\In.mbx
//
//	We can now uniquely find "UniqueRootPrefixForEudoraMailboxPaths In.mbx" and
//	"UniqueRootPrefixForEudoraMailboxPaths Foo\Bar\".
//
//	The string "UniqueRootPrefixForEudoraMailboxPaths" was chosen because:
//	* It is less than 39 characters (the default maximum word length)
//	* It is very unlikely to appear as an actual IMAP folder name (POP folder names
//	  were not an issue for uniqueness because they are suffixed with .fol)
//
//	Note that the prefix only affects the *indexed* value of the relative path.
//	X1 allows us to index one value and store another value. For indexing and
//	searching we include the prefix. When retrieving the mailbox relative path from
//	X1 we do not need to worry about removing the prefix because the storage value
//	does not include the prefix.
const char *		SearchManager::kMailboxRelativePathRootTag = "UniqueRootPrefixForEudoraMailboxPaths ";


// ---------------------------------------------------------------------------
//		* Initialize										[Static Public]
// ---------------------------------------------------------------------------
//	Use to create and initialize the SearchManager.

bool
SearchManager::Initialize()
{
	TRACE("SearchManager::Initialize()\n");
	ASSERT(!s_pSearchManager);

	if (s_pSearchManager)
		delete s_pSearchManager;

	s_pSearchManager = DEBUG_NEW_NOTHROW SearchManager;

	if ( s_pSearchManager && s_pSearchManager->IsIndexedSearchOn() )
		s_pSearchManager->m_pInfo->Initialize();

	return (s_pSearchManager != NULL);
}


// ---------------------------------------------------------------------------
//		* Destroy											[Static Public]
// ---------------------------------------------------------------------------
//	Use to destroy the SearchManager before quitting.

bool SearchManager::Destroy()
{
	TRACE("SearchManager::Destroy()\n");

	if (s_pSearchManager)
	{
		delete s_pSearchManager;
		s_pSearchManager = NULL;
	}

	return true;
}


// ---------------------------------------------------------------------------
//		* SearchManager											  [Protected]
// ---------------------------------------------------------------------------
//	SearchManager constructor, protected so the client programmer is
//	forced to use Initialize.

SearchManager::SearchManager()
	:	m_pInfo(DEBUG_NEW SearchManager::Info),
		m_bInitializedSavedSearchesList(false)
{
}


// ---------------------------------------------------------------------------
//		* ~SearchManager										  [Protected]
// ---------------------------------------------------------------------------
//	SearchManager destructor, protected so the client programmer is
//	forced to use Destroy.

SearchManager::~SearchManager()
{
	delete m_pInfo;

	//	Delete items in saved searches list (don't need to bother erasing
	//	them from the list since we're being deleted).
	for (SavedSearchList::iterator i = m_listSavedSearches.begin(); i != m_listSavedSearches.end(); i++)
	{
		SavedSearchCommand *	pSavedSearchCommand = *i;
		
		delete pSavedSearchCommand;
	}
}


// ---------------------------------------------------------------------------
//		* GetX1Database												 [Public]
// ---------------------------------------------------------------------------
//	Returns the X1 email database object.

X1::IX1Database *
SearchManager::GetX1EmailDB()
{
	return m_pInfo->m_pX1EmailDB;
}


// ---------------------------------------------------------------------------
//		* GetX1Search												 [Public]
// ---------------------------------------------------------------------------
//	Returns the X1 search object.

X1::IX1Search *
SearchManager::GetX1Search()
{
	return m_pInfo->m_pX1Search;
}


// ---------------------------------------------------------------------------
//		* IsInitialIndexingComplete									 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not initial indexing is complete.

bool
SearchManager::IsInitialIndexingComplete() const
{
	return m_pInfo && m_pInfo->IsX1Loaded() &&
		   (m_pInfo->m_nLastScanCompletionTime != 0);
}


// ---------------------------------------------------------------------------
//		* IsIndexedSearchOn											 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not indexed search is available and on.

bool
SearchManager::IsIndexedSearchOn() const
{
	m_pInfo->m_bIndexedSearchOnCached = UsingPaidForX1FeatureSet() &&
										(GetIniShort(IDS_INI_USE_INDEXED_SEARCH) == 1);
	
	return m_pInfo->m_bIndexedSearchOnCached;
}


// ---------------------------------------------------------------------------
//		* ShouldUseIndexedSearch									 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not indexed search is available and ready to be used.

bool
SearchManager::ShouldUseIndexedSearch() const
{
	return IsIndexedSearchOn() && IsInitialIndexingComplete();
}


// ---------------------------------------------------------------------------
//		* GetSavedSearchesList										 [Public]
// ---------------------------------------------------------------------------
//	Returns the saved searches list after initializing it if necessary.

SearchManager::SavedSearchList &
SearchManager::GetSavedSearchesList()
{
	if (!m_bInitializedSavedSearchesList)
		UpdateSavedSearchList();

	return m_listSavedSearches;
}


// ---------------------------------------------------------------------------
//		* UpdateSavedSearchList										 [Public]
// ---------------------------------------------------------------------------
//	Updates the list of saved searches to reflect the current contents on
//	disk. Currently called when SearchManager is initially created and
//	whenever a new search is saved.

void
SearchManager::UpdateSavedSearchList()
{
	if (m_bInitializedSavedSearchesList)
	{
		//	Delete all previous commands
		for (SavedSearchList::iterator i = m_listSavedSearches.begin(); i != m_listSavedSearches.end(); i++)
		{
			g_theCommandStack.DeleteCommand( (*i)->GetCommandID(), CA_SAVED_SEARCH );
			delete *i;
		}
		m_listSavedSearches.clear();
	}

	//	Only allow saved search feature with full feature set
	if ( !UsingFullFeatureSet() )
		return;

	//	Look for all the XML files in the "SavedSearches" directory
	CString		szSavedSearchDir = EudoraDir + CRString(IDS_IDX_SRCH_MGR_SAVED_SEARCHES_DIR);
	CFileFind	fileFinder;
	BOOL		bContinue = fileFinder.FindFile(szSavedSearchDir + "\\*.xml");

	//	Add all the XML files in the saved searches directory to the list
	while ( bContinue )
	{
		bContinue = fileFinder.FindNextFile();

		//	Create the command for the current file
		SavedSearchCommand *	pSavedSearchCommand = DEBUG_NEW SavedSearchCommand(
																	&g_theSearchDirector,
																	fileFinder.GetFilePath(),
																	fileFinder.GetFileTitle() );
		WORD				dwCommand = g_theCommandStack.AddCommand(pSavedSearchCommand, CA_SAVED_SEARCH);

		//	Stash the command ID inside the command itself, because it's more convenient
		//	than what appears to be the standard technique used elsewhere.
		pSavedSearchCommand->SetCommandID(dwCommand);

		//	Add the newly created command to our list
		m_listSavedSearches.push_back(pSavedSearchCommand);
	}

	//	Remember that we initialized the saved searches list
	m_bInitializedSavedSearchesList = true;
}


// ---------------------------------------------------------------------------
//		* SaveIfDirty												 [Public]
// ---------------------------------------------------------------------------
//	Saves if dirty and returns whether or not it needed to save.

BOOL
SearchManager::SaveIfDirty()
{
	BOOL	bDidSomething = FALSE;
	
	//	Check to see if we're dirty
	if ( (m_pInfo->m_eDirty != Info::d_NotDirty) &&
		 ((GetTickCount() - m_pInfo->m_nLastSavedTickCount) >= Info::kSaveFrequency) )
	{
		if (m_pInfo->m_eDirty == Info::d_BothDirty)
		{
			//	Flush the latest X1 indices before we save where we're at with the scan
			m_pInfo->m_pX1EmailDB->FlushNow();

			//	Save where we're at with the scan in the next idle
			m_pInfo->SetDirty(Info::d_InfoDirty, true);
		}
		else if (m_pInfo->m_eDirty == Info::d_X1Dirty)
		{
			//	Flush the latest X1 indices
			m_pInfo->m_pX1EmailDB->FlushNow();

			//	We're not dirty any more
			m_pInfo->SetDirty(Info::d_NotDirty);

			//	Remember when we last saved completely
			m_pInfo->m_nLastSavedTickCount = GetTickCount();
		}
		else //	(m_pInfo->m_eDirty == Info::d_InfoDirty)
		{
			//	Save where we're at in the scan process
			m_pInfo->SaveData();

			//	We're not dirty any more
			m_pInfo->SetDirty(Info::d_NotDirty);

			//	Remember when we last saved completely
			m_pInfo->m_nLastSavedTickCount = GetTickCount();
		}

		//	We did something
		bDidSomething = TRUE;
	}

	return bDidSomething;
}


// ---------------------------------------------------------------------------
//		* IdleStartScanningIfNeeded									 [Public]
// ---------------------------------------------------------------------------
//	Starts scanning if appropriate.

BOOL
SearchManager::IdleStartScanningIfNeeded()
{
	BOOL	bDidSomething = FALSE;
	
	if ( (m_pInfo->m_nLastScanCompletionTime == 0) &&
		 (m_pInfo->m_eScanType == X1EmailScanner::st_None) )
	{
		//	No completed scan, and no scan to pickup. Initialize for our first
		//	scan. Default to X1EmailScanner::st_AddOrUpdateItemsScan for
		//	m_eScanType, so that by default we'll start with a scan that adds
		//	all email messages.
		//
		//	m_eNextScanType defaults to nothing, because it doesn't make sense
		//	to bother with a culling scan the first time through.
		m_pInfo->m_eScanType = X1EmailScanner::st_AddOrUpdateItemsScan;
		m_pInfo->m_eNextScanType = X1EmailScanner::st_None;
	}

	if (m_pInfo->m_eScanType != X1EmailScanner::st_None)
	{
		//	Start first scan, or pickup scan where we last left off.
		m_pInfo->m_pX1EmailScanner->StartScan( m_pInfo->m_eScanType, m_pInfo->m_eNextScanType,
											   m_pInfo->m_szIndexingMailboxRelativePath );
		bDidSomething = TRUE;
	}
	else if ( HasIndexUpdateActions() )
	{
		//	Start processing index update actions
		m_pInfo->m_pX1EmailScanner->StartScan(X1EmailScanner::st_ProcessingIndexUpdateActions);
		bDidSomething = TRUE;
	}
	else
	{
		//	We're not currently scanning, should we be?
		if (m_pInfo->m_bNeedToInitiateScan)
		{
			//	Initiate full scan to update index
			m_pInfo->m_eScanType = X1EmailScanner::st_AddOrUpdateItemsScan;
			m_pInfo->m_eNextScanType = X1EmailScanner::st_CullingScan;
			m_pInfo->m_pX1EmailScanner->StartScan(m_pInfo->m_eScanType, m_pInfo->m_eNextScanType);

			//	Remember that we initiated the full scan
			m_pInfo->m_bNeedToInitiateScan = false;

			bDidSomething = TRUE;
		}
	}

	return bDidSomething;
}



// ---------------------------------------------------------------------------
//		* Idle														 [Public]
// ---------------------------------------------------------------------------
//	Idles the SearchManager.

BOOL
SearchManager::Idle(
	unsigned long				in_nDeltaTime,
	bool						in_bIsExtendedIdleProcessing,
	bool						in_bMustDoScanning)
{
	BOOL			bDidSomething = FALSE;
	
	if ( IsIndexedSearchOn() )
	{
		//	Check to see if we have our info or if initialization failed
		if ( !m_pInfo || m_pInfo->InitializationFailed() )
		{
			ASSERT( !"Something went wrong with the creation or initialization of the SearchManagerInfo object" );

			//	Nothing more to do
			return FALSE;
		}

		//	Check to see if we need to initialize
		if ( !m_pInfo->IsX1Loaded() )
		{
			m_pInfo->Initialize();

			//	Notify Search View(s) that availablity of indexed search changed
			CSearchView::NotifyIndexedSearchAvailabilityChanged();

			//	We did something
			return TRUE;
		}
	}
	else
	{
		//	No reason to idle if we're not on.
		//	If we were previously loaded, then release X1.
		if ( m_pInfo && m_pInfo->IsX1Loaded() )
		{
			m_pInfo->ReleaseX1();
			bDidSomething = TRUE;

			//	Notify Search View(s) that availability of indexed search changed
			CSearchView::NotifyIndexedSearchAvailabilityChanged();
		}
		
		return bDidSomething;
	}
	
	try
	{
		if ( m_pInfo->m_pX1.GetInterfacePtr() && m_pInfo->m_pX1EmailScanner &&
			 m_pInfo->m_pX1EmailDB.GetInterfacePtr() )
		{
			if (m_pInfo->m_bX1ExceptionFlagged)
			{
				m_pInfo->m_pX1ScannerManager->start();
				m_pInfo->m_bX1ExceptionFlagged = false;
			}

			long	nLastScanCount = 0;
			bool	bIsIdleEnough = in_bMustDoScanning;
			
			if (!in_bMustDoScanning)
			{
				//	We weren't told that we must do scanning, so figure out if
				//	we should do scanning.
				if ( HasIndexUpdateActions() && (GetTopWindowID() == IDR_SEARCH_WND) )
				{
					//	We have index update actions and the search window is in front.
					//	Therefore we want to accelerate our idle scanning somewhat by
					//	always doing scanning now.
					bIsIdleEnough = true;
				}
				else
				{
					//	Normal situation - we're idle enough if we're doing extended
					//	idle processing and if we've been idle long enough.
					bIsIdleEnough = in_bIsExtendedIdleProcessing &&
									( in_nDeltaTime >= static_cast<unsigned long>(GetIniLong(IDS_INI_INDEXED_SEARCH_IDLE)) );
				}
			}

			if (bIsIdleEnough)
				bDidSomething = SaveIfDirty();

			if ( m_pInfo->m_pX1EmailScanner->IsScanning() )
			{
				//	Only scan when idle enough and we haven't done anything
				if (bIsIdleEnough && !bDidSomething)
				{
					m_pInfo->m_pX1EmailScanner->ResumeScan();
				}
				else
				{
					bool	bWaitingForIdle = in_bIsExtendedIdleProcessing && !bIsIdleEnough;
					m_pInfo->m_pX1EmailScanner->PauseScan(bWaitingForIdle);
				}

				nLastScanCount = m_pInfo->m_pX1EmailScanner->GetScanCount();
			}

			//	Always make sure the index is loaded (or loading).
			BOOL	bIsLoaded = m_pInfo->m_pX1EmailDB->EnsureLoaded();

			//	Must call this every idle.
			BYTE	bX1Done = 1;

			m_pInfo->m_pX1->Idle(&bX1Done);

			//	Consider starting scanning if:
			//	* We're idle enough
			//	* We didn't do anything worth noting thus far
			//	* X1 is loaded
			//	* We're not currently scanning
			if ( bIsIdleEnough && !bDidSomething && bIsLoaded && !m_pInfo->m_pX1EmailScanner->IsScanning() )
				bDidSomething = IdleStartScanningIfNeeded();

			if (m_pInfo->m_bPauseExtendedIdleProcessing)
			{
				//	Scanner indicated it wants us to return false to pause extended
				//	idle processing. Set bDidSomething and reset the flag.
				bDidSomething = false;
				m_pInfo->m_bPauseExtendedIdleProcessing = false;
			}
			else if (!bDidSomething)
			{
				//	We're actively scanning if:
				//	* We're scanning
				//	* We scanned something during the latest call to idle (above)
				bool	bActivelyScanning = m_pInfo->m_pX1EmailScanner->IsScanning() &&
											( nLastScanCount != m_pInfo->m_pX1EmailScanner->GetScanCount() );

				//	We did something if:
				//	* X1 says that it has more work to do
				//	* We were doing the add/update scan or culling scan
				bDidSomething = !bX1Done || bActivelyScanning;
			}


		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "Idle", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "Idle");

		//	Delete exception object
		pException->Delete();
	}

	return bDidSomething;
}


// ---------------------------------------------------------------------------
//		* HasIndexUpdateActions										 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not there are any index update actions to process.

bool
SearchManager::HasIndexUpdateActions() const
{
	return !m_pInfo->m_mapIndexUpdateActions.empty();
}


// ---------------------------------------------------------------------------
//		* HasMailboxUpdateActions									 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not there any mailbox update actions to process.

bool
SearchManager::HasMailboxUpdateActions() const
{
	return (m_pInfo->m_nNumMailboxUpdateActions > 0);
}


// ---------------------------------------------------------------------------
//		* GetNumberIndexUpdateActions								 [Public]
// ---------------------------------------------------------------------------
//	Returns how many index update actions there are to process.

long
SearchManager::GetNumberIndexUpdateActions() const
{
	return m_pInfo->m_mapIndexUpdateActions.size();
}


// ---------------------------------------------------------------------------
//		* GetFirstIndexUpdateAction									 [Public]
// ---------------------------------------------------------------------------
//	Returns first index update actions to process.

SearchManager::IndexUpdateAction *
SearchManager::GetFirstIndexUpdateAction()
{
	IndexUpdateAction *		pUpdateAction = NULL;

	ASSERT( !m_pInfo->m_mapIndexUpdateActions.empty() );

	if ( !m_pInfo->m_mapIndexUpdateActions.empty() )
		pUpdateAction = m_pInfo->m_mapIndexUpdateActions.begin()->second;
	
	return pUpdateAction;
}


// ---------------------------------------------------------------------------
//		* PopIndexUpdateAction										 [Public]
// ---------------------------------------------------------------------------
//	Pops first index update action.

void
SearchManager::PopIndexUpdateAction()
{
	ASSERT( !m_pInfo->m_mapIndexUpdateActions.empty() );

	if ( !m_pInfo->m_mapIndexUpdateActions.empty() )
	{
		//	Grab the update action pointer, then remove from the map
		Info::IndexUpdateActionMapIteratorT		iterator = m_pInfo->m_mapIndexUpdateActions.begin();
		IndexUpdateAction *						pUpdateAction = iterator->second;
		m_pInfo->m_mapIndexUpdateActions.erase(iterator);

		//	Decrement number of mailbox actions if we're removing a mailbox action
		UpdateActionEnum		eUpdateAction = pUpdateAction->GetUpdateAction();
		if (eUpdateAction >= ua_FirstMailboxAction)
		{
			m_pInfo->m_nNumMailboxUpdateActions--;

			//	Number of mailbox update actions must not be less than zero, or
			//	else I goofed somewhere.
			ASSERT(m_pInfo->m_nNumMailboxUpdateActions >= 0);
		}

		//	Delete the update action now that we're done with it
		delete pUpdateAction;

		//	Mark that the list of update objects is dirty, but don't worry
		//	about when it will be written because replaying update actions
		//	should be safe.
		m_pInfo->SetDirty(Info::d_InfoDirty);
	}
}


// ---------------------------------------------------------------------------
//		* AddIndexUpdateAction										 [Public]
// ---------------------------------------------------------------------------
//	Adds index update action to list and returns pointer to the newly added
//	action.

void
SearchManager::AddIndexUpdateAction(
	UpdateActionEnum			in_eUpdateAction,
	const CString &				in_szMailboxPath,
	long						in_nMessageID)
{
	bool	bShouldSave = false;
	
	ASSERT( (in_eUpdateAction != ua_Invalid) && !in_szMailboxPath.IsEmpty() );
	
	//	Only add the index update action if it looks valid
	if ( (in_eUpdateAction >= ua_FirstValidAction) &&
		 (in_eUpdateAction <= ua_LastValidAction) &&
		 !in_szMailboxPath.IsEmpty() )
	{
		if (in_eUpdateAction <= ua_LastMessageAction)
		{
			//	Only add a message action if it has a valid message ID
			ASSERT(in_nMessageID != 0);

			//	Look for any existing mailbox actions
			UpdateIdentifier						msg(in_szMailboxPath, 0);
			Info::IndexUpdateActionMapIteratorT		iterator = m_pInfo->m_mapIndexUpdateActions.find(msg);

			//	Only add a message action if it has a valid message ID and
			//	if there is no mailbox action for the associated mailbox.
			if ( (in_nMessageID != 0) && (iterator == m_pInfo->m_mapIndexUpdateActions.end()) )
			{
				IndexUpdateAction *						pIndexUpdateAction;
				
				//	Attempt to find an existing item for the specified message
				UpdateIdentifier						msg(in_szMailboxPath, in_nMessageID);
				Info::IndexUpdateActionMapIteratorT		iterator = m_pInfo->m_mapIndexUpdateActions.find(msg);

				if ( iterator == m_pInfo->m_mapIndexUpdateActions.end() )
				{
					//	We didn't find an existing item, so add a new item
					//	Create the new item
					pIndexUpdateAction = DEBUG_NEW IndexUpdateAction(in_eUpdateAction, in_szMailboxPath, in_nMessageID);

					//	Add the new item to the map
					m_pInfo->m_mapIndexUpdateActions[msg] = pIndexUpdateAction;

					//	Make sure we save addition
					bShouldSave = true;
				}
				else
				{
					//	We found an existing item - check to see if we need to modify it
					pIndexUpdateAction = iterator->second;

					ASSERT(pIndexUpdateAction);

					//	If the new action doesn't match the old action, then update it
					if (pIndexUpdateAction)
					{
						UpdateActionEnum	ePrevUpdateAction = pIndexUpdateAction->GetUpdateAction();

						if (in_eUpdateAction != ePrevUpdateAction)
						{
							//	New action not the same as old action - figure out what
							//	to do scissors, paper, rock style.
							if (ePrevUpdateAction == ua_RemoveMessage)
							{
								//	Only expect add or update
								ASSERT( (in_eUpdateAction == ua_AddMessage) || (in_eUpdateAction == ua_UpdateMessage) );
								
								//	New add or update overrides previous remove
								if ( (in_eUpdateAction == ua_AddMessage) || (in_eUpdateAction == ua_UpdateMessage) )
								{
									pIndexUpdateAction->SetUpdateAction(in_eUpdateAction);

									//	Make sure we save change
									bShouldSave = true;
								}
							}
							else if (ePrevUpdateAction == ua_AddMessage)
							{
								//	Only expect update or remove
								ASSERT( (in_eUpdateAction == ua_UpdateMessage) || (in_eUpdateAction == ua_RemoveMessage) );

								if (in_eUpdateAction == ua_RemoveMessage)
								{
									//	The add hadn't been processed yet - so by simply removing
									//	it we avoid the need for either an add or a remove action.
									m_pInfo->m_mapIndexUpdateActions.erase(iterator);
									delete pIndexUpdateAction;
									pIndexUpdateAction = NULL;

									//	Make sure we save change
									bShouldSave = true;
								}

								//	Do nothing if the new action is ua_UpdateMessage because we haven't
								//	processed the add yet - therefore there is no need for an update.
							}
							else if (ePrevUpdateAction == ua_UpdateMessage)
							{
								//	Only expect add or remove
								ASSERT( (in_eUpdateAction == ua_AddMessage) || (in_eUpdateAction == ua_RemoveMessage) );

								//	New remove overrides previous update
								if (in_eUpdateAction == ua_RemoveMessage)
								{
									pIndexUpdateAction->SetUpdateAction(in_eUpdateAction);

									//	Make sure we save change
									bShouldSave = true;
								}

								//	Do nothing if the new action is ua_AddMessage. We don't expect this, but
								//	it's not harmful. Just process the current update.
							}
						}
					}
				}
			}
		}
		else	//	It's a mailbox action
		{
			//	Look for any existing mailbox actions
			UpdateIdentifier						mailbox(in_szMailboxPath, 0);
			Info::IndexUpdateActionMapIteratorT		iterator = m_pInfo->m_mapIndexUpdateActions.find(mailbox);

			if ( iterator == m_pInfo->m_mapIndexUpdateActions.end() )
			{
				//	Didn't find any conflicting mailbox actions.
				//	Look for any message actions for messages in this mailbox.
				Info::IndexUpdateActionMapIteratorT		iterStartMessageActions = m_pInfo->m_mapIndexUpdateActions.lower_bound(mailbox);

				//	We found message actions for messages in the mailbox if:
				//	* We didn't hit the end
				//	* The mailbox path matches
				if ( ( iterStartMessageActions != m_pInfo->m_mapIndexUpdateActions.end() ) &&
					 (iterStartMessageActions->second->GetMailboxRelativePath().CompareNoCase(in_szMailboxPath) == 0) )
				{
					//	Find the action immediately after the last possible message action
					UpdateIdentifier						actionAfterMailbox(in_szMailboxPath, 0x7FFFFFFF);
					Info::IndexUpdateActionMapIteratorT		iterEndMessageActions = m_pInfo->m_mapIndexUpdateActions.upper_bound(actionAfterMailbox);

					//	Remove the message actions because the mailbox action overrides them.
					m_pInfo->m_mapIndexUpdateActions.erase(iterStartMessageActions, iterEndMessageActions);
				}

				//	We didn't find an existing item, so add a new item
				//	Create the new item
				IndexUpdateAction *		pIndexUpdateAction = DEBUG_NEW IndexUpdateAction(in_eUpdateAction, in_szMailboxPath, 0);

				//	Add the new item to the map
				m_pInfo->m_mapIndexUpdateActions[mailbox] = pIndexUpdateAction;

				//	Increment the number of mailbox update actions
				m_pInfo->m_nNumMailboxUpdateActions++;

				//	Make sure we save addition
				bShouldSave = true;
			}
			else
			{
				//	Found mailbox action that already exists for this mailbox
				IndexUpdateAction *				pIndexUpdateAction = iterator->second;

				ASSERT(pIndexUpdateAction);

				if (pIndexUpdateAction)
				{
					//	If the new action doesn't match the old action, then update it
					UpdateActionEnum	ePrevUpdateAction = pIndexUpdateAction->GetUpdateAction();
					if (in_eUpdateAction != ePrevUpdateAction)
					{
						switch (in_eUpdateAction)
						{
							case ua_RemoveMailbox:
								//	Remove overrides everything - set action to remove
								pIndexUpdateAction->SetUpdateAction(ua_RemoveMailbox);

								//	Make sure we save change
								bShouldSave = true;
								break;

							case ua_AddMailbox:
								//	Add overrides remove, but in case we were already
								//	processing this update, set it to reindex.
								if (ePrevUpdateAction == ua_RemoveMailbox)
								{
									pIndexUpdateAction->SetUpdateAction(ua_ReindexMailbox);

									//	Make sure we save change
									bShouldSave = true;
								}
								break;

							case ua_ReindexMailbox:
								//	Reindex overrides everything - set action to reindex.
								pIndexUpdateAction->SetUpdateAction(ua_ReindexMailbox);

								//	Make sure we save change
								bShouldSave = true;
								break;
						}
					}
				}
			}
		}
	}

	if (bShouldSave)
	{
		//	Mark dirty
		m_pInfo->SetDirty(Info::d_InfoDirty);

		//	Make sure we save in the next idle, because we don't want to risk
		//	forgetting to add the users latest changes.
		m_pInfo->m_nLastSavedTickCount = 0;
	}
}


// ---------------------------------------------------------------------------
//		* AddSortColumn												 [Public]
// ---------------------------------------------------------------------------
//	Adds sort column to list of sort columns.

void
SearchManager::AddSortColumn(
	int							in_nSortBy)
{
	bool	bAdd = true;

	if ( !m_pInfo->m_sortColumns.empty() )
	{
		bAdd = ( in_nSortBy != m_pInfo->m_sortColumns.back() );

		//	There is a known bug where search columns get duplicated.
		//	The above check prevents the bug from causing problems,
		//	but I still need to figure out why it happens.
		ASSERT(bAdd);
	}

	if (bAdd)
		m_pInfo->m_sortColumns.push_back( static_cast<SortType>(in_nSortBy) );
}


// ---------------------------------------------------------------------------
//		* IsDoingFullScan											 [Public]
// ---------------------------------------------------------------------------
//	Determines whether or not search manager is currently scanning.

bool
SearchManager::IsDoingFullScan() const
{
	bool	bIsScanning = false;

	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
		bIsScanning = m_pInfo->m_pX1EmailScanner->IsDoingFullScan();

	return bIsScanning;
}


// ---------------------------------------------------------------------------
//		* StartScanning												 [Public]
// ---------------------------------------------------------------------------
//	Starts scanning.

void
SearchManager::StartScanning()
{
	if ( m_pInfo && m_pInfo->m_pX1EmailScanner )
	{
		ASSERT( !m_pInfo->m_pX1EmailScanner->IsDoingFullScan() );

		//	Start scanning
		m_pInfo->m_eScanType = X1EmailScanner::st_AddOrUpdateItemsScan;
		m_pInfo->m_eNextScanType = X1EmailScanner::st_CullingScan;
		m_pInfo->m_pX1EmailScanner->StartScan(m_pInfo->m_eScanType, m_pInfo->m_eNextScanType);
	}
}


// ---------------------------------------------------------------------------
//		* StopScanning												 [Public]
// ---------------------------------------------------------------------------
//	Stops scanning.

void
SearchManager::StopScanning()
{
	if ( m_pInfo && m_pInfo->m_pX1EmailScanner )
	{
		ASSERT( m_pInfo->m_pX1EmailScanner->IsScanning() );

		//	Stop all scanning
		//	This isn't quite right yet, but should be good enough for alpha.
		m_pInfo->m_eScanType = X1EmailScanner::st_None;
		m_pInfo->m_pX1EmailScanner->StopAllScanning();
		NotifyIndexingStatus(X1EmailScanner::st_None, X1EmailScanner::st_None, "");
	}
}


// ---------------------------------------------------------------------------
//		* ReIndex													 [Public]
// ---------------------------------------------------------------------------
//	Allows user to force messages to be re-indexed.

void
SearchManager::ReIndex()
{
	//	Log what we're doing
	PutDebugLog(DEBUG_MASK_SEARCH, "User initiated re-indexing. Existing search index files will be removed.");

	//	Stop any current scanning
	if ( m_pInfo && m_pInfo->m_pX1EmailScanner && m_pInfo->m_pX1EmailScanner->IsScanning() )
		StopScanning();
	
	//	Save our data first just to be sure
	m_pInfo->SaveData();

	//	Release X1
	m_pInfo->ReleaseX1();

	//	Mark that we need the X1 DB to be cleared
	m_pInfo->m_bX1DBNeedsCleared = true;

	//	Re-init X1 which will clear the X1 DB
	m_pInfo->InitX1();
}


// ---------------------------------------------------------------------------
//		* SetPauseExtendedIdleProcessing							 [Public]
// ---------------------------------------------------------------------------
//	Allows scanner to request that we temporarily pause a given extended
//	idling session by return false for bDidSomething.

void
SearchManager::SetPauseExtendedIdleProcessing(
	bool						in_bPauseExtendedIdleProcessing)
{
	m_pInfo->m_bPauseExtendedIdleProcessing = in_bPauseExtendedIdleProcessing;
}


// ---------------------------------------------------------------------------
//		* OnActivateApp												 [Public]
// ---------------------------------------------------------------------------
//	Throttles the amount of CPU X1 will use based on whether or not
//	Eudora is the frontmost app.

void
SearchManager::OnActivateApp(BOOL in_bActive)
{
	try
	{
		if (m_pInfo->m_pX1) 
		{
			//	Control X1's CPU usage based on whether or not Eudora is the
			//	frontmost app.
			if (in_bActive) 
				m_pInfo->m_pX1->SetCPUUsage(X1::culHigh);
			else
				m_pInfo->m_pX1->SetCPUUsage(X1::culLow);
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "OnActivateApp", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "OnActivateApp");

		//	Delete exception object
		pException->Delete();
	}
}


// ---------------------------------------------------------------------------
//		* NotifyIniChange											 [Public]
// ---------------------------------------------------------------------------
//	Called when a setting we care about has changed.

void
SearchManager::NotifyIniChange(
	UINT						in_nStringNum)
{
	bool	bIndexedSearchOnCached = m_pInfo->m_bIndexedSearchOnCached;
	bool	bIndexedSearchOn = IsIndexedSearchOn();

	if (bIndexedSearchOn != bIndexedSearchOnCached)
	{
		if (bIndexedSearchOn)
		{
			//	Indexed search was just turned on, we need to update the index by
			//	performing a full scan.
			m_pInfo->m_bNeedToInitiateScan = true;
		}
		else if ( m_pInfo && m_pInfo->m_pX1EmailScanner &&
				  m_pInfo->m_pX1EmailScanner->IsScanning() )
		{
			//	Indexed search was just turned off and we're scanning, stop scanning now.
			StopScanning();
		}
	}
}


// ---------------------------------------------------------------------------
//		* NotifySumAddition											 [Public]
// ---------------------------------------------------------------------------
//	Called by CTocDoc when a summary is being added.

void
SearchManager::NotifySumAddition(
	const CTocDoc *				in_pTocDoc,
	const CSummary *			in_pSum)
{
	//	No need to do anything with sum additions if indexed search is not on
	if ( !IsIndexedSearchOn() )
		return;

	//	Verify calling parameters
	ASSERT(in_pTocDoc);
	ASSERT(in_pSum);

	if (!in_pTocDoc || !in_pSum)
		return;
	
	//	Determine the relative path of the mailbox
	const char *	szMailboxRelativePath = static_cast<const char *>(in_pTocDoc->GetMBFileName()) + EudoraDirLen;

	//	Add add/update action
	AddIndexUpdateAction( ua_AddMessage, szMailboxRelativePath, in_pSum->GetUniqueMessageId() );
}


// ---------------------------------------------------------------------------
//		* NotifySumRemoval											 [Public]
// ---------------------------------------------------------------------------
//	Called by CTocDoc when a summary is being deleted. Used to allow
//	email scanning to move to next summary.

void
SearchManager::NotifySumRemoval(
	const CTocDoc *				in_pTocDoc,
	POSITION					in_posSum,
	const CSummary *			in_pSum)
{
	//	No need to do anything with sum removals if indexed search is not on
	if ( !IsIndexedSearchOn() )
		return;

	//	Verify calling parameters
	ASSERT(in_pTocDoc);
	ASSERT(in_pSum);

	if (!in_pTocDoc || !in_pSum)
		return;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
		m_pInfo->m_pX1EmailScanner->NotifySumRemoval(in_pTocDoc, in_posSum);

	//	Determine the relative path of the mailbox
	const char *	szMailboxRelativePath = static_cast<const char *>(in_pTocDoc->GetMBFileName()) + EudoraDirLen;

	//	Add remove action
	AddIndexUpdateAction( ua_RemoveMessage, szMailboxRelativePath, in_pSum->GetUniqueMessageId() );
}


// ---------------------------------------------------------------------------
//		* NotifySumModification										 [Public]
// ---------------------------------------------------------------------------
//	Called by CTocDoc when the information for a summary is being changed.

void
SearchManager::NotifySumModification(
	const CTocDoc *				in_pTocDoc,
	const CSummary *			in_pSum)
{
	//	No need to do anything with sum modifications if indexed search is not on
	if ( !IsIndexedSearchOn() )
		return;

	//	Verify calling parameters
	ASSERT(in_pTocDoc);
	ASSERT(in_pSum);

	if (!in_pTocDoc || !in_pSum)
		return;
	
	//	Determine the relative path of the mailbox
	const char *	szMailboxRelativePath = static_cast<const char *>(in_pTocDoc->GetMBFileName()) + EudoraDirLen;

	//	Add add/update action
	AddIndexUpdateAction( ua_UpdateMessage, szMailboxRelativePath, in_pSum->GetUniqueMessageId() );
}


// ---------------------------------------------------------------------------
//		* NotifyTemporarilyCloseMailbox								 [Public]
// ---------------------------------------------------------------------------
//	Called by QCMailboxCommand::CompactMailbox when compacting a mailbox
//	to make sure that we don't have the mailbox open so that we don't
//	prevent renaming the temporary mailbox to the final name.

void
SearchManager::NotifyTemporarilyCloseMailbox(
	const char *				in_szMailboxFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
	{
		const char *	szMailboxRelativePath = in_szMailboxFullPath + EudoraDirLen;
		
		m_pInfo->m_pX1EmailScanner->NotifyTemporarilyCloseMailbox(szMailboxRelativePath);
	}
}


// ---------------------------------------------------------------------------
//		* NotifyRestartProcessingMailbox							 [Public]
// ---------------------------------------------------------------------------
//	Called when we need to restart processing a specific mailbox because it
//	has been sorted (so the summaries no longer have the same positions).

void
SearchManager::NotifyRestartProcessingMailbox(
	const char *				in_szMailboxFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
	{
		const char *	szMailboxRelativePath = in_szMailboxFullPath + EudoraDirLen;
		
		m_pInfo->m_pX1EmailScanner->NotifyRestartProcessingMailbox(szMailboxRelativePath);
	}
}


// ---------------------------------------------------------------------------
//		* NotifyStopProcessingMailbox								 [Public]
// ---------------------------------------------------------------------------
//	Called when we need to stop processing a specific mailbox because it
//	has been deleted, moved, or renamed.

void
SearchManager::NotifyStopProcessingMailbox(
	const char *				in_szMailboxFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
	{
		const char *	szMailboxRelativePath = in_szMailboxFullPath + EudoraDirLen;
		
		m_pInfo->m_pX1EmailScanner->NotifyStopProcessingMailbox(szMailboxRelativePath);
	}
}


// ---------------------------------------------------------------------------
//		* NotifyMailboxPathChange									 [Public]
// ---------------------------------------------------------------------------
//	Called by QCMailboxCommand::CompactMailbox when compacting a mailbox
//	to make sure that we don't have the mailbox open so that we don't
//	prevent renaming the temporary mailbox to the final name.

void
SearchManager::NotifyMailboxPathNameChange(
	const char *				in_szMailboxOldFullPath,
	const char *				in_szMailboxNewFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;

	const char *	szMailboxOldRelativePath = in_szMailboxOldFullPath + EudoraDirLen;
	const char *	szMailboxNewRelativePath = in_szMailboxNewFullPath + EudoraDirLen;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
		m_pInfo->m_pX1EmailScanner->NotifyStopProcessingMailbox(szMailboxOldRelativePath);

	//	Add update actions to remove the old mailbox path and
	//	add the new mailbox path.
	AddIndexUpdateAction(ua_RemoveMailbox, szMailboxOldRelativePath, 0);
	AddIndexUpdateAction(ua_AddMailbox, szMailboxNewRelativePath, 0);
}


// ---------------------------------------------------------------------------
//		* NotifyMailboxAdded										 [Public]
// ---------------------------------------------------------------------------
//	Called by QCMailboxDirector to notify us that a mailbox was added.
//	Currently this only matters when a mailbox that we previously didn't
//	know about (as recorded by the descmap.pce files) is discovered while
//	we're starting up.

void
SearchManager::NotifyMailboxAdded(
	const char *				in_szMailboxFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;

	const char *	szMailboxRelativePath = in_szMailboxFullPath + EudoraDirLen;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
		m_pInfo->m_pX1EmailScanner->NotifyStopProcessingMailbox(szMailboxRelativePath);

	//	Add update actions to remove the old mailbox path and
	//	add the new mailbox path.
	AddIndexUpdateAction(ua_AddMailbox, szMailboxRelativePath, 0);
}


// ---------------------------------------------------------------------------
//		* NotifyMailboxDeleted										 [Public]
// ---------------------------------------------------------------------------
//	Called by QCMailboxDirector to notify us that a mailbox is about to
//	be deleted.

void
SearchManager::NotifyMailboxDeleted(
	const char *				in_szMailboxFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;

	const char *	szMailboxRelativePath = in_szMailboxFullPath + EudoraDirLen;
	
	//	Pass on notification to X1 email scanner
	if (m_pInfo && m_pInfo->m_pX1EmailScanner)
		m_pInfo->m_pX1EmailScanner->NotifyStopProcessingMailbox(szMailboxRelativePath);

	//	Add update actions to remove the old mailbox path and
	//	add the new mailbox path.
	AddIndexUpdateAction(ua_RemoveMailbox, szMailboxRelativePath, 0);
}


// ---------------------------------------------------------------------------
//		* NotifyReindexMailbox										 [Public]
// ---------------------------------------------------------------------------
//	Called by CTocDoc when a toc doc needed to be salvaged or rebuilt.
//	Indicates that we need to re-index the indicated mailbox.

void
SearchManager::NotifyReindexMailbox(
	const char *				in_szMailboxFullPath)
{
	//	No need to close mailbox if indexed search is not on.
	if ( !IsIndexedSearchOn() )
		return;

	const char *	szMailboxRelativePath = in_szMailboxFullPath + EudoraDirLen;

	//	Add update action to reindex the mailbox indicated.
	AddIndexUpdateAction(ua_ReindexMailbox, szMailboxRelativePath, 0);
}


// ---------------------------------------------------------------------------
//		* NotifyX1Exception											 [Public]
// ---------------------------------------------------------------------------
//	Called by X1EventListener when an X1 exception occurs.

void
SearchManager::NotifyX1Exception()
{
	//	Pass on notification to X1 email scanner
	if (m_pInfo)
		m_pInfo->m_bX1ExceptionFlagged = true;
}


// ---------------------------------------------------------------------------
//		* NotifyX1SchemaVersion									  [Protected]
// ---------------------------------------------------------------------------
//	Called by SearchManager::XMLParser while loading XML.

void
SearchManager::NotifyX1SchemaVersion(
	long						in_nSchemaVersion)
{	
	m_pInfo->m_bX1DBNeedsCleared = (in_nSchemaVersion != SearchManager::Info::kDBSchemaVersion);

	if (m_pInfo->m_bX1DBNeedsCleared)
	{
		CString		szLogEntry;

		szLogEntry.Format( "X1 schema version does not match expected schema version. Version on disk: %d Expected version: %d",
						   in_nSchemaVersion, SearchManager::Info::kDBSchemaVersion );
		
		PutDebugLog(DEBUG_MASK_SEARCH, szLogEntry);
	}
}


// ---------------------------------------------------------------------------
//		* NotifyIndexingStatus										 [Public]
// ---------------------------------------------------------------------------
//	Called by X1EmailScanner when scanning is complete.

void
SearchManager::NotifyIndexingStatus(
	int							in_nCurrentScanType,
	int							in_nNextScanType,
	const char *				in_szIndexingMailboxRelativePath)
{
	if (m_pInfo)
	{
		//	Update the current mailbox being scanned
		m_pInfo->m_szIndexingMailboxRelativePath = in_szIndexingMailboxRelativePath;
		
		if ( (in_nCurrentScanType == X1EmailScanner::st_None) &&
			 (m_pInfo->m_eScanType != X1EmailScanner::st_None) )
		{
			//	Remember that we completed a full scan
			m_pInfo->m_nLastScanCompletionTime = time(NULL);

			//	Make sure we save in the next idle
			m_pInfo->m_nLastSavedTickCount = 0;
		}

		//	Update the current and next scan type
		m_pInfo->m_eScanType = static_cast<X1EmailScanner::ScanTypeEnum>(in_nCurrentScanType);
		m_pInfo->m_eNextScanType = static_cast<X1EmailScanner::ScanTypeEnum>(in_nNextScanType);

		//	Mark that we need to both flush X1 and save our data
		m_pInfo->SetDirty(Info::d_BothDirty);
	}
}


// ---------------------------------------------------------------------------
//		* DoSearch													 [Public]
// ---------------------------------------------------------------------------
//	Called by CSearchView to perform search.

void
SearchManager::DoSearch(
	MultSearchCriteria &		in_searchCriteria,
	std::deque<CString> &		in_listCheckedMailboxes,
	std::deque<CString> &		in_listUncheckedMailboxes,
	CSearchView *				in_pSearchView,
	unsigned int				in_nResultsFrequency,
	bool						in_bIncludeDeleteIMAPMessages)
{
	try
	{
		DWORD							nLastCallbackTick = GetTickCount();
		const SearchCriteriaList *		pCriteriaList = in_searchCriteria.GetCriteriaList();
		bool							bOR = in_searchCriteria.IsOpOR();
		SearchCriteriaIter				iter = pCriteriaList->begin();
		X1::IX1ItemResultPtr			pCombinedResult;
		X1::IX1ItemResultPtr			pPrevCombinedResult;
		X1::IX1ItemResultPtr			pCurrentResult;

		ASSERT( !pCriteriaList->empty() );
		if ( pCriteriaList->empty() )
			return;

		//	Start progress display
		PushProgress();

		//	Check to see if we have index update actions
		if ( HasIndexUpdateActions() )
		{
			//	Remember the last time we checked for escape
			DWORD		nLastEscapeCheck = GetTickCount();
			
			//	Set the progress message in case this takes a while
			Progress( CRString(IDS_SEARCH_PROGRESS_UPDATING_INDEX) );
			
			//	Loop while we still have index update action
			while ( HasIndexUpdateActions() )
			{
				//	Idle ourselves. Index update actions are automatically prioritized.
				//	Passing in true for the 3rd parameter ensures that idle processing
				//	will occur.
				Idle(0, true, true);

				//	Only check for escape once every 200 milliseconds. Allows us to
				//	operate faster while still being responsive enough.
				if ( (GetTickCount() - nLastEscapeCheck) > 200 )
				{
					//	Bail early if escape is pressed
					if ( ::EscapePressed() )
					{
						//	Get rid of the progress dialog
						PopProgress();

						//	Completely bail out of the entire method
						return;
					}

					//	Remember the last time we checked for escape
					nLastEscapeCheck = GetTickCount();
				}
			}
		}

		//	Set the progress message to indicate that we're searching now
		Progress( CRString(IDS_SEARCH_PROGRESS_SEARCHING) );

		if ( iter != pCriteriaList->end() )
		{
			//	Do the first criteria
			Searching::DoSearchTerm(*iter, &pCombinedResult);

			//	Move on to the next criteria
			++iter;
		}

		while ( iter != pCriteriaList->end() )
		{
			//	Do the next criteria
			Searching::DoSearchTerm(*iter, &pCurrentResult);

			//	Transfer the previous combined result into a new smart pointer
			pPrevCombinedResult = pCombinedResult;

			//	Combine the new result
			if (bOR)
				pCurrentResult->OpOr(pPrevCombinedResult, &pCombinedResult);
			else
				pCurrentResult->OpAnd(pPrevCombinedResult, &pCombinedResult);

			//	Move on to the next criteria
			++iter;
		}

		//	Search for the mailboxes in the easiest fashion - either by finding the
		//	checked mailboxes or the unchecked mailboxes.
		Searching::FindMessagesInMailboxes(in_listCheckedMailboxes, in_listUncheckedMailboxes, &pCurrentResult);

		//	Transfer the previous combined result into a new smart pointer
		pPrevCombinedResult = pCombinedResult;

		//	Combine all search results from above with mailbox search
		pCurrentResult->OpAnd(pPrevCombinedResult, &pCombinedResult);

		//	Check to see if we need to eliminate deleted IMAP messages
		//	from the search results.
		if (!in_bIncludeDeleteIMAPMessages)
		{
			//	Find all messages that are not IMAP deleted
			Searching::FindMessagesNotIMAPDeleted(&pCurrentResult);

			//	Transfer the previous combined result into a new smart pointer
			pPrevCombinedResult = pCombinedResult;

			//	Combine all search results from above with search for
			//	messages that are not IMAP deleted.
			pCurrentResult->OpAnd(pPrevCombinedResult, &pCombinedResult);
		}

		if (pCombinedResult)
		{
			//	Set up what we want the search results to be sorted by
			COleSafeArray	safeArraySortBy;
			COleSafeArray	safeArraySortOrder;

			Searching::PrepareSortColumns(safeArraySortBy, safeArraySortOrder);

			//	Actually get the search results
			X1::IX1ItemListPtr		pItemList = NULL;
			pCombinedResult->GetItemList(safeArraySortBy, safeArraySortOrder, &pItemList);

			ASSERT( pItemList.GetInterfacePtr() );

			if (pItemList)
			{				
				//	Determine how many search results we found
				int		nCount = pItemList->Getcount();

				if (nCount > 0)
				{
					X1::IX1DatabasePtr			pEmailDB = m_pInfo->m_pX1EmailDB;
					std::deque<SearchResult>	listResults;

					Progress(0, CRString(IDS_SEARCH_PROGRESS_DISPLAYING_RESULTS), nCount);

					// Iterate over the results
					_bstr_t results = "";
					for (long i = 0; i < nCount; i++)
					{
						long		nItemNum = pItemList->Getitem(i);
						DWORD		nTickCount = GetTickCount();

						listResults.push_back( SearchResult(pEmailDB, nItemNum) );

						if ( (nTickCount - nLastCallbackTick) >= in_nResultsFrequency )
						{
							//	Display the next batch of results
							in_pSearchView->ProcessResults(listResults);

							//	Update progress
							ProgressAdd( listResults.size() );

							listResults.clear();

							nLastCallbackTick = nTickCount;
						}

						//	Bail early if escape is pressed
						if ( ::EscapePressed() )
							break;
					}

					//	We're finished
					Progress(nCount, NULL, nCount);
					
					//	Display the last batch of results
					in_pSearchView->ProcessResults(listResults);
				}
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "DoSearch", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "DoSearch");

		//	Delete exception object
		pException->Delete();
	}

	//	Pop the progress
	PopProgress();
}

