//	LinkHistoryManager.cpp
//
//	Manages persistent list of URLs that the user has launched (or ads displayed).
//
//	Copyright (c) 1999 by QUALCOMM, Incorporated
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

#include "LinkHistoryManager.h"
#include "LinkHistoryList.h"
#include "LHOfflineLinkDialog.h"
#include "LHRemindDialog.h"

#include "fileutil.h"
#include "guiutils.h"
#include "mainfrm.h"
#include "resource.h"
#include "URLUtils.h"

#include "QCSharewareManager.h"

//	Playlist subsystem interface
#include "plist_mgr.h"

#include "DebugNewHelpers.h"

//	Static pointer to the one and only instance of the Link History Manager
LinkHistoryManager *	LinkHistoryManager::s_pLinkHistoryManager = NULL;
const long				LinkHistoryManager::kSecondsPerDay = 60 * 60 * 24;
const long				LinkHistoryManager::kPurgeFrequencySecs = kSecondsPerDay;
const char *			LinkHistoryManager::kEndOfEntryString = "---end";
const long				LinkHistoryManager::kEndOfEntryStrLen = 6;


// ---------------------------------------------------------------------------
//		* LinkHistoryManager::UrlItemData						[Public]
// ---------------------------------------------------------------------------
//	LinkHistoryManager::UrlItemData default constructor

LinkHistoryManager::UrlItemData::UrlItemData()
	:	m_szLinkName(), m_szURL(), m_szThumbnailFileName(), m_szAdID(),
		m_lLastVisitTime(0), m_lLastTouchTime(0), m_type(lt_Unknown),
		m_status(ls_Unknown), m_nIconIndex(0)
{
}


// ---------------------------------------------------------------------------
//		* LinkHistoryManager::UrlItemData						[Public]
// ---------------------------------------------------------------------------
//	LinkHistoryManager::UrlItemData temp search item constructor

LinkHistoryManager::UrlItemData::UrlItemData(
	LinkTypeEnum		type,
	LPCSTR				szFindString)
	:	m_szLinkName(), m_szURL(), m_szThumbnailFileName(), m_szAdID(),
		m_lLastVisitTime(0), m_lLastTouchTime(0), m_type(type),
		m_status(ls_Unknown), m_nIconIndex(0)
{
	if (type == lt_Ad)
		m_szAdID = szFindString;
	else
		m_szURL = szFindString;
}


// ---------------------------------------------------------------------------
//		* LinkHistoryManager::UrlItemData						[Public]
// ---------------------------------------------------------------------------
//	LinkHistoryManager::UrlItemData constructor

LinkHistoryManager::UrlItemData::UrlItemData(
	LPCSTR				szLinkName,
	LPCSTR				szURL,
	LPCSTR				szAdID,
	LPCSTR				szThumbnailFileName,
	long				lLastVisitTime,
	long				lLastTouchTime,
	LinkTypeEnum		type,
	LinkStatusEnum		status)
	:	m_szLinkName(szLinkName), m_szURL(szURL), m_szThumbnailFileName(szThumbnailFileName),
		m_szAdID(szAdID), m_lLastVisitTime(lLastVisitTime), m_lLastTouchTime(lLastTouchTime),
		m_type(type), m_status(status), m_nIconIndex(0)
{
}


// ---------------------------------------------------------------------------
//		* LinkHistoryManager::LessUrlItemDataByKey::operator()		[Public]
// ---------------------------------------------------------------------------
//	Comparison method which allows ads to be found by ad id, and non-ads
//	by URL.

bool
LinkHistoryManager::LessUrlItemDataByKey::operator()(
	const UrlItemData *			lhs,
	const UrlItemData *			rhs) const
{
	bool	isLessThan = false;

	if (lhs->m_type == lt_Ad)
	{
		if (rhs->m_type == lt_Ad)
		{
			//	Both are ads, compare by Ad ID
			isLessThan = (lhs->m_szAdID.CompareNoCase(rhs->m_szAdID) < 0);
		}
		else
		{
			//	Sort ads before non-ads (arbitrary choice that's internal only - no UI impact)
			isLessThan = true;
		}
	}
	else
	{
		if (rhs->m_type != lt_Ad)
		{
			//	Both are not ads, compare by URL
			isLessThan = (lhs->m_szURL.CompareNoCase(rhs->m_szURL) < 0);
		}
		else
		{
			//	Sort non-ads after ads (arbitrary choice that's internal only - no UI impact)
			isLessThan = false;
		}
	}

	return isLessThan;
}


// ---------------------------------------------------------------------------
//		* Initialize										[Static Public]
// ---------------------------------------------------------------------------
//	Use to create and initialize the LinkHistoryManager.

bool LinkHistoryManager::Initialize()
{
	TRACE("LinkHistoryManager::Initialize()\n");
	ASSERT(!s_pLinkHistoryManager);

	if (s_pLinkHistoryManager)
		delete s_pLinkHistoryManager;

	s_pLinkHistoryManager = DEBUG_NEW_NOTHROW LinkHistoryManager;

	if (s_pLinkHistoryManager)
		s_pLinkHistoryManager->Load();

	return (s_pLinkHistoryManager != NULL);
}


// ---------------------------------------------------------------------------
//		* Destroy											[Static Public]
// ---------------------------------------------------------------------------
//	Use to destroy the LinkHistoryManager before quitting.

bool LinkHistoryManager::Destroy()
{
	TRACE("LinkHistoryManager::Destroy()\n");

	if (s_pLinkHistoryManager)
	{
		if ( s_pLinkHistoryManager->IsDirty() )
		{
			//	Some information was added or changed. Save before destroying.
			s_pLinkHistoryManager->Save();
		}


		delete s_pLinkHistoryManager;
		s_pLinkHistoryManager = NULL;
	}

	return true;
}


// ---------------------------------------------------------------------------
//		* LinkHistoryManager								[Protected]
// ---------------------------------------------------------------------------
//	LinkHistoryManager constructor, protected so the client programmer is
//	forced to use Initialize.

LinkHistoryManager::LinkHistoryManager()
	:	m_UrlList(), m_szLHFilePathName(""), m_LastPurgeTime(0),
		m_bIsDirty(false), m_bShouldRemind(false), m_lRemindSnoozeTime(0),
		m_bIconImageListCreated(false)
{
	m_szLHFilePathName = EudoraDir + CRString(IDS_LINK_HISTORY_FILENAME);
	m_szLHDirPath = EudoraDir + CRString(IDS_LINK_HISTORY_DIRNAME) + "\\";
}


// ---------------------------------------------------------------------------
//		* ~LinkHistoryManager								[Protected]
// ---------------------------------------------------------------------------
//	LinkHistoryManager destructor, protected so the client programmer is
//	forced to use Destroy.
//	Removes UrlItemData items because the LinkHistoryManager owns them
//	(LinkHistoryList just uses the pointers).

LinkHistoryManager::~LinkHistoryManager()
{
	//	Delete UrlItemData items, but don't bother erasing them from
	//	the m_UrlList because it is about to be destroyed anyway.
	LPUrlItemData	pLHItem;
	for ( UrlListIteratorT	urlIterator = m_UrlList.begin();
		  urlIterator != m_UrlList.end();
		  urlIterator++ )
	{
		pLHItem = *urlIterator;
		delete pLHItem;
	}
}


// ---------------------------------------------------------------------------
//		* Save													[Public]
// ---------------------------------------------------------------------------
//	Saves the current LinkHistoryManager information to "linkhistory.dat".

bool LinkHistoryManager::Save()
{
	CString		szTempName;
	JJFile		out;
	bool		bSaveFailed = false;
	char		szTemp[256];
		
	szTempName = TempDir + CRString(IDS_LINK_HISTORY_TEMP_FILENAME);
	if (FAILED(out.Open(szTempName, O_CREAT | O_TRUNC | O_WRONLY)))
		return false;

	//	Write out the current file version, the last purge time, and the
	//	oldest compatible version of the code that can read this version
	sprintf(szTemp, "%d %u %d", kLHCurrentFileVersion, m_LastPurgeTime, kLHFileVersionFirstBackwardFriendly); 
	if ( FAILED(out.PutLine(szTemp)) )
		return false;

	LPUrlItemData		pLHItem;

	for ( UrlListIteratorT urlIterator = m_UrlList.begin();
		 urlIterator != m_UrlList.end();
		 urlIterator++ )
	{
		pLHItem = *urlIterator;

		bSaveFailed = FAILED( out.PutLine(pLHItem->m_szLinkName) );
		if (bSaveFailed) break;
	
		bSaveFailed = FAILED( out.PutLine(pLHItem->m_szURL) );
		if (bSaveFailed) break;

		bSaveFailed = FAILED( out.PutLine(pLHItem->m_szThumbnailFileName) );
		if (bSaveFailed) break;

		bSaveFailed = FAILED( out.PutLine(pLHItem->m_szAdID) );
		if (bSaveFailed) break;
		
		wsprintf( szTemp, "%u %u %d %d",
				  pLHItem->m_lLastVisitTime, pLHItem->m_lLastTouchTime,
				  pLHItem->m_type, pLHItem->m_status );
		bSaveFailed = FAILED( out.PutLine(szTemp) );
		if (bSaveFailed) break;

		bSaveFailed = FAILED( out.PutLine(kEndOfEntryString) );
		if (bSaveFailed) break;
	}
	
	if (!bSaveFailed)
		bSaveFailed = FAILED( out.Close() );

	if (!bSaveFailed)
		bSaveFailed = FAILED( out.Rename(m_szLHFilePathName) );
	else
		out.Delete();

	if (!bSaveFailed)
	{
		//	Our save was good, so we're no longer dirty
		SetDirtyState(false);
	}
	
	return !bSaveFailed;
}


// ---------------------------------------------------------------------------
//		* ReadEntireLine										[Protected]
// ---------------------------------------------------------------------------
//	Reads an entire line using JJFile. Allows me to not have to guess the
//	largest possible URL, while still being robust. Should be called with
//	a large buffer for better performance (in which case it will probably
//	always degenerate into just a single read).

bool
LinkHistoryManager::ReadEntireLine(
	JJFile &		readFile,
	CString &		szRead,
	char *			szBuffer,
	long			lBufferSize) 
{
	long	lNumBytesRead = 0;
	
	lBufferSize--;
	
	//	Read as much of the line as we can fit into the buffer and assign it to the string
	bool	bReadGood = SUCCEEDED(readFile.GetLine(szBuffer, lBufferSize, &lNumBytesRead)) && (lNumBytesRead > 0);
	if (bReadGood)
	{
		szBuffer[lNumBytesRead] = '\0';
		szRead = szBuffer;
	}
	
	//	Continue reading the line and append it to the string as necessary
	while ( bReadGood && (lNumBytesRead == lBufferSize) && (szBuffer[lBufferSize-2] != '\0') )
	{
		bReadGood = SUCCEEDED(readFile.GetLine(szBuffer, lBufferSize, &lNumBytesRead)) && (lNumBytesRead > 0);
		if (bReadGood)
		{
			szBuffer[lNumBytesRead] = '\0';
			szRead += szBuffer;
		}
	}

	return bReadGood;
}


// ---------------------------------------------------------------------------
//		* Load													[Public]
// ---------------------------------------------------------------------------
//	Loads the previously saved information from "linkhistory.dat".
//	Note that the failure of this method will not prevent Initialize
//	from suceeding (Initialize currently ignores the return value).

bool LinkHistoryManager::Load()
{
	JJFile		readFile;
	bool		bReadGood = true;
	int			nFileVersion = kLHCurrentFileVersion;
	int			nOldestReaderVersion = kLHCurrentFileVersion;

	//	Make sure the LinkHistory directory gets created
	if ( !::FileExistsMT(m_szLHDirPath) && (mkdir(m_szLHDirPath) < 0) )
		return false;

	if ( !::FileExistsMT(m_szLHFilePathName) ) 
		return true;

	if ( FAILED(readFile.Open(m_szLHFilePathName, O_RDONLY)) )
		return false;
	
	char *			szBuffer = NULL;
	LPUrlItemData	pNextEntry = NULL;
	try
	{
		long					lNumBytesRead = 0;
		const long				lBufferSize = 1024;
		
		szBuffer = DEBUG_NEW char[lBufferSize];
		pNextEntry = DEBUG_NEW UrlItemData;

		if ( FAILED(readFile.GetLine(szBuffer, lBufferSize-1, &lNumBytesRead)) || (lNumBytesRead <= 0) )
			return false;

		//	Get the file version by itself to determine how to read the rest of the line
		sscanf(szBuffer, "%d", &nFileVersion);

		if (nFileVersion > kLHFileVersionLastPreBackwardFriendly)
		{
			//	Backward friendly file versions include the oldest compatible reader version, which
			//	is used to indicate whether or not skipping extra lines will work when
			//	reading a file format newer than the code (i.e. was there an incompatible change?)
			sscanf(szBuffer, "%d %u %d", &nFileVersion, &m_LastPurgeTime, &nOldestReaderVersion);
		}
		else
		{
			//	Versions kLHFileVersionLastPreBackwardFriendly and before incorrectly had an 'l'
			//	after the %u
			sscanf(szBuffer, "%d %ul", &nFileVersion, &m_LastPurgeTime);
		}

		//	Before reading make sure that the file format will be understandable to us
		if (nOldestReaderVersion <= kLHCurrentFileVersion)
		{
			while ( ReadEntireLine(readFile, pNextEntry->m_szLinkName, szBuffer, lBufferSize) )
			{
				bReadGood = ReadEntireLine(readFile, pNextEntry->m_szURL, szBuffer, lBufferSize);
				if (!bReadGood) break;

				if (nFileVersion > kLHFileVersionLastPreThumbnail)
				{
					//	File contains thumbnail path info
					bReadGood = ReadEntireLine(readFile, pNextEntry->m_szThumbnailFileName, szBuffer, lBufferSize);
					if (!bReadGood) break;
				}

				if (nFileVersion > kLHFileVersionLastPreAdID)
				{
					//	File contains separate ad id
					bReadGood = ReadEntireLine(readFile, pNextEntry->m_szAdID, szBuffer, lBufferSize);
					if (!bReadGood) break;
				}

				bReadGood = SUCCEEDED(readFile.GetLine(szBuffer, lBufferSize, &lNumBytesRead)) && (lNumBytesRead > 0);
				if (!bReadGood) break;
				szBuffer[lNumBytesRead] = '\0';

				if (nFileVersion > kLHFileVersionLastPreBackwardFriendly)
				{
					//	File includes m_lLastTouchTime
					sscanf( szBuffer, "%u %u %d %d",
							&pNextEntry->m_lLastVisitTime, &pNextEntry->m_lLastTouchTime,
							&pNextEntry->m_type, &pNextEntry->m_status );
					
					//	Scan for end of entry marker. For a current or old file version this will just be
					//	the next line, but with a file version newer than this code scanning will allow us
					//	to skip lines we don't understand.
					do
					{
						bReadGood = SUCCEEDED(readFile.GetLine(szBuffer, lBufferSize, &lNumBytesRead)) &&
									(lNumBytesRead > 0);
					} while ( bReadGood && (strncmp(szBuffer, kEndOfEntryString, kEndOfEntryStrLen) != 0) );
					if (!bReadGood) break;
				}
				else if (nFileVersion > kLHOriginalFileVersionNoTouchDate)
				{
					//	File includes m_lLastTouchTime, but has spurious l's after %u's
					sscanf( szBuffer, "%ul %ul %d %d",
							&pNextEntry->m_lLastVisitTime, &pNextEntry->m_lLastTouchTime,
							&pNextEntry->m_type, &pNextEntry->m_status );
				}
				else // (nFileVersion == kLHOriginalFileVersionNoTouchDate)
				{
					//	File doesn't include m_lLastTouchTime. Read in the rest, and assign
					//	a reasonable value to m_lLastTouchTime.
					sscanf( szBuffer, "%ul %d %d",
							&pNextEntry->m_lLastVisitTime, &pNextEntry->m_type, &pNextEntry->m_status );
					
					//	Set the touched time to be the visit time if it's non-zero (i.e. the link was
					//	visited). If it was never visited set it to right now to avoid prematurely
					//	expiring ads that we just read.
					if (pNextEntry->m_lLastVisitTime > 0)
						pNextEntry->m_lLastTouchTime = pNextEntry->m_lLastVisitTime;
					else
						pNextEntry->m_lLastTouchTime = time(NULL);
				}

				//	Do a quick sanity check of the read data
				if (pNextEntry->m_type > lt_Unknown)
					pNextEntry->m_type = ClassifyURL(pNextEntry->m_szURL);

				if ( (pNextEntry->m_type == lt_Ad) &&
					 ((nFileVersion <= kLHFileVersionLastPreCorrectedAdID) || pNextEntry->m_szAdID.IsEmpty())  )
				{
					//	For compatibility parse out the ad id from the URL
					pNextEntry->m_szAdID = pNextEntry->m_szURL;

					int		pos = pNextEntry->m_szAdID.Find("adid=");
					if (pos >= 0)
						pNextEntry->m_szAdID = pNextEntry->m_szAdID.Right(pNextEntry->m_szAdID.GetLength() - pos - 5);

					pos = pNextEntry->m_szAdID.Find("&");
					if (pos >= 0)
						pNextEntry->m_szAdID = pNextEntry->m_szAdID.Left(pos);
				}
				
				//	Note that we can reuse the already allocated pNextEntry if we're not adding it.
				if ( IsTrackedLinkType(pNextEntry->m_type) )
				{
					//	Make sure the values make sense
					switch (pNextEntry->m_status)
					{
						case ls_Remind:
						case ls_Bookmarked:
						case ls_NotVisited:
						case ls_Removed:
						case ls_Unknown:
							break;

						case ls_Attempted:
						case ls_None:
							//	m_lLastVisitTime should not be 0
							if (pNextEntry->m_lLastVisitTime == 0)
								pNextEntry->m_lLastVisitTime = pNextEntry->m_lLastTouchTime;
							break;

						default:
							//	Unknown status - reset to something reasonable
							if (pNextEntry->m_lLastVisitTime == 0)
								pNextEntry->m_status = ls_NotVisited;
							else
								pNextEntry->m_status = ls_None;
							break;
					}
					
					//	Check to see if the item is a duplicate before we add it to our list.
					//	This eliminates duplicates and prevents leaks that would occur otherwise
					//	(a set only allows one item per unique key - adding a duplicate would
					//	forget about the first item).
					UrlListIteratorT	urlIterator = m_UrlList.find(pNextEntry);
					if ( urlIterator == m_UrlList.end() )
					{
						InternalAddURL(pNextEntry);

						if (pNextEntry->m_status == ls_Remind)
							SetShouldRemind();

						pNextEntry = DEBUG_NEW UrlItemData;
					}
				}
			}
		}
	}
	catch (CMemoryException* pException)
	{
		// to do -- add an error message
		// this is a less serious error -- it won't screw up the stack
		pException->Delete();
	}

	delete [] szBuffer;
	delete pNextEntry;

	if (bReadGood)
	{
		if (nFileVersion >= kLHCurrentFileVersion)
		{
			//	Our reading was good, the info is fresh, and the file version is up to date
			//	or newer than us - so we're not dirty
			SetDirtyState(false);
		}
		else
		{
			//	The file version is not up to date. Make sure we resave our data in the latest format.
			SetDirtyState(true);
		}

		//	Quick sanity check of the last purge time
		long	currentTime = time(NULL);
		if (m_LastPurgeTime > currentTime)
		{
			//	The last purge time shouldn't be in the future. Make it be right now.
			m_LastPurgeTime = currentTime;
			
			if (nFileVersion == kLHCurrentFileVersion)
			{
				//	Make sure we save the corrected m_LastPurgeTime
				SetDirtyState(true);
			}
		}
	}

	return bReadGood;
}


// ---------------------------------------------------------------------------
//		* LoadThumbnailIfNecessary								[Protected]
// ---------------------------------------------------------------------------
//	Loads the thumbnail for the indicated entry if necessary.

void LinkHistoryManager::LoadThumbnailIfNecessary(LPUrlItemData pEntry)
{
	//	If someone is calling us, then the icon image list should already be created
	ASSERT(m_bIconImageListCreated);
	
	//	Does this entry have a thumbnail?
	if ( m_bIconImageListCreated && (pEntry != NULL) && !pEntry->m_szThumbnailFileName.IsEmpty() )
	{
		CString		szThumbnailFullPath = m_szLHDirPath + pEntry->m_szThumbnailFileName;
		
		if ( ::FileExistsMT(szThumbnailFullPath) )
		{
			SECImage * pThumbnailImage = LoadImage(szThumbnailFullPath, COLOR_WINDOW);

			if (pThumbnailImage)
			{
				//	Create a CBitmap based on the thumbnail
				CWindowDC	screenDC(NULL);
				CBitmap *	pThumbnailBitmap = pThumbnailImage->MakeBitmap(&screenDC);
				
				if (pThumbnailBitmap != NULL)
				{
					//	Add the CBitmap to the image list
					pEntry->m_nIconIndex = m_IconImageList.Add( pThumbnailBitmap, RGB(255,255,255) );
					delete pThumbnailBitmap;
				}

				delete pThumbnailImage;
			}
		}
	}
}


// ---------------------------------------------------------------------------
//		* SetShouldRemind											[Public]
// ---------------------------------------------------------------------------
//	Turns remind on, and sets the remind snooze time to 0.
//	Call when changing a link to a remind status.

void LinkHistoryManager::SetShouldRemind()
{
	m_bShouldRemind = true;
	m_lRemindSnoozeTime = 0;
}


// ---------------------------------------------------------------------------
//		* CreateIconImageList										[Public]
// ---------------------------------------------------------------------------
//	Called from CLinkHistoryWazooWnd::OnActivateWazoo so that the icon image
//	list is created just before it's needed. This helps Eudora to startup
//	faster and not use up the memory until it's needed. Note that
//	OnActivateWazoo must call this before creating the CLinkHistoryList.

void LinkHistoryManager::CreateIconImageList()
{
	//	Load the default icons for the various link types
	BOOL	bLoadGood = m_IconImageList.Load( kLinkTypeIconSize, kLinkTypeIconPaddedHeight, 0,
										  MAKEINTRESOURCE(IDB_LINK_HISTORY_TYPES), RGB(255,255,255) );
	ASSERT(bLoadGood);

	if (bLoadGood)
	{
		m_bIconImageListCreated = true;

		//	Load any thumbnails that were created in this or a previous session in Eudora
		LPUrlItemData		pLHItem;
		for ( UrlListIteratorT urlIterator = m_UrlList.begin();
			 urlIterator != m_UrlList.end();
			 urlIterator++ )
		{
			pLHItem = *urlIterator;

			LoadThumbnailIfNecessary(pLHItem);
		}
	}
}


// ---------------------------------------------------------------------------
//		* GetIconImageList											[Public]
// ---------------------------------------------------------------------------
//	Returns the icon image list if it's been created, NULL otherwise.

CImageList * LinkHistoryManager::GetIconImageList()
{
	if (m_bIconImageListCreated)
		return &m_IconImageList;
	else
		return NULL;
}


// ---------------------------------------------------------------------------
//		* Idle														[Public]
// ---------------------------------------------------------------------------
//	Idles the LinkHistoryManager. Save if dirty and purge old links once a
//	day. Only do one action at idle to avoid taking too long.
//	For now amount of idleness (deltaTime) amounts are hardwired.

BOOL LinkHistoryManager::Idle(unsigned long currentTime, unsigned long deltaTime)
{
	if (deltaTime > 5)
	{
		if ( IsDirty() )
		{
			//	Some information was added or changed. Save ASAP.
			Save();

			//	We did something, return immediately
			return TRUE;
		}

		if ( UsingFullFeatureSet() && m_bShouldRemind && (currentTime > m_lRemindSnoozeTime) && !IsOffline() )
		{
			//	Determine whether or not m_bShouldRemind should really be true
			//	(because of status changing it's hard to keep exact track of this -
			//	so we just set it to true whenever appropriate and we don't worry
			//	about setting it to false until we get here).
			LPUrlItemData		pLHItem;
			m_bShouldRemind = false;

			for ( UrlListIteratorT urlIterator = m_UrlList.begin();
				 urlIterator != m_UrlList.end();
				 urlIterator++ )
			{
				pLHItem = *urlIterator;
				if (pLHItem->m_status == ls_Remind)
				{
					m_bShouldRemind = true;
					break;
				}
			}
			
			if (m_bShouldRemind)
			{
				RemindActionEnum	raUserChoice = ra_Show;
				CLHRemindDialog *	pRemind = DEBUG_NEW_MFCOBJ_NOTHROW CLHRemindDialog( AfxGetMainWnd() );
			
				if (pRemind)
				{
					pRemind->DoModal();
					raUserChoice = pRemind->GetUserChoice();
					delete pRemind;
				}

				switch (raUserChoice)
				{
					case ra_Show:
						{
							//	Activate the Link History window
							CMainFrame* pMainFrame = (CMainFrame *) AfxGetMainWnd();
							ASSERT_KINDOF(CMainFrame, pMainFrame);

							pMainFrame->PostMessage(WM_COMMAND, ID_WINDOW_LINK_HISTORY);
							
							//	We've reminded them - don't do so again until we get a new remind item
							m_bShouldRemind = false;
						}
						break;

					case ra_Snooze:
						//	Don't change the remind status, just delay our next possible remind
						m_lRemindSnoozeTime = time(NULL) + (GetIniLong(IDS_INI_LH_REMIND_SNOOZE_MINS) * 60);
						break;

					case ra_Forget:
						{
							//	Change all URLs with remind status to be bookmarked instead
							LPUrlItemData		pLHItem;

							for ( UrlListIteratorT urlIterator = m_UrlList.begin();
								 urlIterator != m_UrlList.end();
								 urlIterator++ )
							{
								pLHItem = *urlIterator;
								if (pLHItem->m_status == ls_Remind)
									pLHItem->m_status = ls_Bookmarked;
							}
							
							//	Dirty so that we save next idle
							SetDirtyState(true);

							//	Update list sort
							CLinkHistoryList *	pLinkHistoryList = CLinkHistoryList::Instance();
							if (pLinkHistoryList != NULL)
								pLinkHistoryList->ResortItemsIfSortedByDate();
						}
						break;
				}

				//	We did something, return immediately
				return TRUE;
			}
		}
	}

	bool	bDidSomething = FALSE;

	if ( (deltaTime > 60) && ((currentTime-m_LastPurgeTime) > kPurgeFrequencySecs) )
	{
		//	Time to purge
		LPUrlItemData		pLHItem;
		UrlListIteratorT	urlIterator = m_UrlList.begin();
		UrlListIteratorT	urlIteratorToRemove;
		unsigned long		oldestTouchSecs = currentTime - (GetIniLong(IDS_INI_LH_LINK_MAX_AGE) * kSecondsPerDay);
		long				lNumberItemsRemoved = 0;
		CLinkHistoryList *	pLinkHistoryList = CLinkHistoryList::Instance();
		FileNameListT		goodThumbnailFileList;

		//	Scan for Link History items that should be purged
		while ( urlIterator != m_UrlList.end() )
		{
			pLHItem = *urlIterator;
			
			//	If the link hasn't been touched recently enough and it's not a special status,
			//	then remove it.	A link is touched when it's added, visited, status changed, etc.
			if ( (pLHItem != NULL) && (pLHItem->m_status != ls_Remind) &&
				 (pLHItem->m_status != ls_Bookmarked) && (pLHItem->m_lLastTouchTime < oldestTouchSecs) )
			{
				//	Be careful about removing iterators in an STL container. In this case
				//	we're ok because we're using a set, provided we move to the next element
				//	before erasing the old element.
				//	From <http://www.sgi.com/Technology/STL/set.html>:
				//	"Set has the important property that inserting a new element into a set
				//	does not invalidate iterators that point to existing elements. Erasing an
				//	element from a set also does not invalidate any iterators, except, of course,
				//	for iterators that actually point to the element that is being erased."

				urlIteratorToRemove = urlIterator;

				//	Move on to the next item, before removing this item
				urlIterator++;
				m_UrlList.erase(urlIteratorToRemove);
				
				//	Remove the item from the Link History list view
				if (pLinkHistoryList != NULL)
					pLinkHistoryList->RemoveURL(pLHItem);

				//	Delete thumbnail for entry if any
				if ( !pLHItem->m_szThumbnailFileName.IsEmpty() )
				{
					CString		szThumbnailFullPath = m_szLHDirPath + pLHItem->m_szThumbnailFileName;
					
					if ( ::FileExistsMT(szThumbnailFullPath) )
						DeleteFile(szThumbnailFullPath);
				}

				delete pLHItem;
				
				lNumberItemsRemoved++;
			}
			else
			{
				if ( (pLHItem != NULL) && !pLHItem->m_szThumbnailFileName.IsEmpty() )
				{
					CString		szThumbnailFullPath = m_szLHDirPath + pLHItem->m_szThumbnailFileName;
					
					if ( ::FileExistsMT(szThumbnailFullPath) )
					{
						//	Add thumbnail file name to list of good thumbnail files
						goodThumbnailFileList.insert(pLHItem->m_szThumbnailFileName);
					}
					else
					{
						//	Forget about this thumbnail file - it doesn't exist
						pLHItem->m_szThumbnailFileName = "";
					}
				}
				
				//	Move on to the next item
				urlIterator++;
			}
		}

		//	Scan the LinkHistory directory for rogue bmp's that we don't know about
		//	and delete them. This makes sure we cleanup in any unexpected cases.
		bool		bShouldDeleteFile = false;
		CFileFind	fileFinder;
		CString		szFilePathToDelete;
		BOOL		bContinue = fileFinder.FindFile(m_szLHDirPath + "*.bmp");
		
		while ( bContinue )
		{
			bContinue = fileFinder.FindNextFile();

			if (bShouldDeleteFile)
			{
				//	Delete file from previous time through loop. We moved first
				//	just in case CFileFind iteration is delete sensitive.
				DeleteFile(szFilePathToDelete);
			}
			
			FileNameIteratorT	fileIterator = goodThumbnailFileList.find( fileFinder.GetFileName() );

			//	If we don't know about this file we should delete it
			bShouldDeleteFile = ( fileIterator == goodThumbnailFileList.end() );
			
			if (bShouldDeleteFile)	//	Grab the path of the file we need to delete
				szFilePathToDelete = fileFinder.GetFilePath();
		}

		//	Delete last file if necessary.
		if (bShouldDeleteFile)
			DeleteFile(szFilePathToDelete);

		m_LastPurgeTime = currentTime;
		
		//	We're dirty even if we didn't remove anything, because we need
		//	to save the updated last purge time stamp.
		SetDirtyState(true);

		if (lNumberItemsRemoved > 0)
		{
			//	Redraw the Link History list view
			if (pLinkHistoryList != NULL)
				pLinkHistoryList->Invalidate();
		}

		bDidSomething = TRUE;
	}

	return bDidSomething;
}


// ---------------------------------------------------------------------------
//		* IsTrackedLinkType											[Public]
// ---------------------------------------------------------------------------
//	Returns whether or not the LinkHistoryManager tracks the given URL type.

bool LinkHistoryManager::IsTrackedLinkType(LinkHistoryManager::LinkTypeEnum type)
{
	//	Only allow ad or http links into the Link History for now.
	return (type == lt_Http) || (type == lt_Ad);
}


// ---------------------------------------------------------------------------
//		* ShouldLaunchURL											[Public]
// ---------------------------------------------------------------------------
//	Returns whether or not a given URL should be launched.
//	Checks link type, and if it's a type we track then checks offline status
//	and user preference.

bool LinkHistoryManager::ShouldLaunchURL(LPCSTR szURL, bool bIsAdURL, LPCSTR szLinkText, const char * szAdID)
{
	//	If it's an ad URL, the caller must give us the ad ID
	ASSERT( !bIsAdURL || (szAdID != NULL) );
	
	LinkTypeEnum	linkType = bIsAdURL ? lt_Ad : ClassifyURL(szURL);
	
	bool	bShouldLaunchURL = true;

	//	Only check launch feasibility for URLs we track if the user is in paid or sponsored mode
	if ( UsingFullFeatureSet() && IsTrackedLinkType(linkType) )
	{
		//	Check to see if it's an Ad. Ads are by definition good :-).
		//	Also check to see if the user has ScamWatch dialog warnings turned on.
		if ( !bIsAdURL && GetIniShort(IDS_INI_SHOW_SCAMWATCH_DIALOG) )
		{
			//	Check to see if the link text combined with the link target is naughty
			UINT		nURLIsNaughty = CURLInfo::IsURLNaughty(szURL, szLinkText);
			
			if (nURLIsNaughty)
			{
				//	The URL is naughty - ask the user if he or she wants to proceed
				//	with opening this potentially dangerous URL.
				CString		szMessage;
				
				szMessage.Format( nURLIsNaughty, static_cast<LPCTSTR>(szURL), static_cast<LPCTSTR>(szLinkText) );

				int			nButtonResult = WarnOneOptionCancelDialog( 0, IDS_URL_NAUGHTY_ALERT_MSG,
																	   IDC_URL_NAUGHTY_VISIT_BTN, szMessage);

				bShouldLaunchURL = (nButtonResult != IDCANCEL);
			}
		}
		
		if (bShouldLaunchURL)
		{
			bShouldLaunchURL = !IsOffline();
		
			if (!bShouldLaunchURL)
			{
				OfflineLinkActionEnum	olaUserPref =
					static_cast<OfflineLinkActionEnum>( GetIniLong(IDS_INI_LH_OFFLINE_LINK_ACTION) );

				if (olaUserPref == ola_Ask)
				{
					CLHOfflineLinkDialog *		pOfflineLink = DEBUG_NEW_MFCOBJ_NOTHROW CLHOfflineLinkDialog( AfxGetMainWnd() );
				
					if (pOfflineLink)
					{
						pOfflineLink->DoModal();
						olaUserPref = pOfflineLink->GetUserChoice();
						delete pOfflineLink;
					}
				}

				switch (olaUserPref)
				{
					case ola_Launch:
						bShouldLaunchURL = true;
						break;

					case ola_Bookmark:
						AddBookmarkedURL(szURL, ls_Bookmarked, bIsAdURL, szAdID);
						break;

					case ola_Remind:
						AddBookmarkedURL(szURL, ls_Remind, bIsAdURL, szAdID);
						break;
				}
			}
		}
	}

	return bShouldLaunchURL;
}


// ---------------------------------------------------------------------------
//		* UpdateLHListDisplay									[Protected]
// ---------------------------------------------------------------------------
//	Updates the Link History list to add or update the URL for its display.

void LinkHistoryManager::UpdateLHListDisplay(LPUrlItemData pURLToAddOrUpdate, bool bIsNewToList)
{
	CLinkHistoryList *	pLinkHistoryList = CLinkHistoryList::Instance();
	if (pLinkHistoryList != NULL)
	{
		//	Make sure to add previously removed items (currently only ads) to the LH list view
		if (bIsNewToList)
			pLinkHistoryList->AddURL(pURLToAddOrUpdate);
		else
			pLinkHistoryList->UpdateURL(pURLToAddOrUpdate);
	}
}


// ---------------------------------------------------------------------------
//		* UpdateVisitedURLInfo										[Public]
// ---------------------------------------------------------------------------
//	Updates the URL's info based upon whether or not the launch was
//	successful. Called both internally and by CLinkHistoryList.

void LinkHistoryManager::UpdateVisitedURLInfo(LPUrlItemData pURL, BOOL bLaunchGood)
{
	//	Update visited and touched date/time to right now
	pURL->m_lLastTouchTime = time(NULL);

	bool	bWasPreviouslyRemoved = (pURL->m_status == ls_Removed);

	if (bLaunchGood)
	{
		//	The user successfully launched the URL, so we no longer need to
		//	track remind or bookmarked for this URL.
		pURL->m_status = ls_None;
		pURL->m_lLastVisitTime = pURL->m_lLastTouchTime;
	}
	else if ( (pURL->m_status != ls_Remind) && (pURL->m_status != ls_Bookmarked) )
	{
		//	It was a failed launch for an item that is not marked bookmarked/remind -
		//	mark it attempted with the current time.
		pURL->m_status = ls_Attempted;
		pURL->m_lLastVisitTime = pURL->m_lLastTouchTime;
	}

	UpdateLHListDisplay(pURL, bWasPreviouslyRemoved);
}


// ---------------------------------------------------------------------------
//		* AddVisitedURL												[Public]
// ---------------------------------------------------------------------------
//	Adds a visited URL to the Link History Manager. Called from
//	LaunchURL.

void LinkHistoryManager::AddVisitedURL(
	const char *		szURL,
	BOOL				bLaunchGood,
	bool				bIsAdURL,
	const char *		szAdID)
{
	//	Only keep track of URLs if the user is in ad or paid mode
	if ( UsingFullFeatureSet() )
	{
		//	If it's an ad URL, the caller must give us the ad ID
		ASSERT( !bIsAdURL || (szAdID != NULL) );
		
		//	Prepare the correct values to search for
		LinkTypeEnum		ltToFind;
		const char *		szStringToFind;
		if (bIsAdURL)
		{
			ltToFind = lt_Ad;
			szStringToFind = szAdID;
		}
		else
		{
			//	lt_Http in this context just indicates it's not an ad
			ltToFind = lt_Http;
			szStringToFind = szURL;
		}

		//	Search for the item
		UrlItemData			tempItemForFind(ltToFind, szStringToFind);
		UrlListIteratorT	urlIterator = m_UrlList.find(&tempItemForFind);

		//	Found if iterator is not end
		if ( urlIterator != m_UrlList.end() )
		{
			LPUrlItemData	pFoundURL = *urlIterator;
			
			//	Update visited and touched date/time to right now
			UpdateVisitedURLInfo(pFoundURL, bLaunchGood);
			ASSERT( !bIsAdURL || (pFoundURL->m_type == lt_Ad) );
		}
		else
		{
			//	If it's an ad, there should already be an entry no matter what!
			ASSERT(!bIsAdURL);
			
			//	Add new entry
			LinkHistoryManager::LinkTypeEnum	linkType = bIsAdURL ? lt_Ad : ClassifyURL(szURL);
			long								lCurrentTime = time(NULL);
			InternalAddURL( "", szURL, szAdID, lCurrentTime, lCurrentTime, linkType,
							bLaunchGood ? ls_None : ls_Attempted );
		}

		//	We added or updated information, set ourselves to dirty so that we save at idle
		SetDirtyState(true);
	}
}


// ---------------------------------------------------------------------------
//		* AddBookmarkedURL											[Public]
// ---------------------------------------------------------------------------
//	Adds a URL with a bookmarked or remind status to the LinkHistoryManager.

void LinkHistoryManager::AddBookmarkedURL(
	const char *		szURL,
	LinkStatusEnum		ls_BookmarkOrRemind,
	bool				bIsAdURL,
	const char *		szAdID)
{
	//	Only keep track of URLs if the user is in ad or paid mode
	if ( UsingFullFeatureSet() )
	{
		//	If it's an ad URL, the caller must give us the ad ID
		ASSERT( !bIsAdURL || (szAdID != NULL) );
		
		//	Prepare the correct values to search for
		LinkTypeEnum		ltToFind;
		const char *		szStringToFind;
		if (bIsAdURL)
		{
			ltToFind = lt_Ad;
			szStringToFind = szAdID;
		}
		else
		{
			//	lt_Http in this context just indicates it's not an ad
			ltToFind = lt_Http;
			szStringToFind = szURL;
		}

		//	Search for the item
		UrlItemData			tempItemForFind(ltToFind, szStringToFind);
		UrlListIteratorT	urlIterator = m_UrlList.find(&tempItemForFind);

		//	Found if iterator is not end
		if ( urlIterator != m_UrlList.end() )
		{
			//	Update touched date/time to right now
			LPUrlItemData	pFoundURL = *urlIterator;
			pFoundURL->m_lLastTouchTime = pFoundURL->m_lLastVisitTime;

			bool	bWasPreviouslyRemoved = (pFoundURL->m_status == ls_Removed);
			pFoundURL->m_status = ls_BookmarkOrRemind;
			ASSERT( !bIsAdURL || (pFoundURL->m_type == lt_Ad) );

			UpdateLHListDisplay(pFoundURL, bWasPreviouslyRemoved);
		}
		else
		{
			//	If it's an ad, there should already be an entry no matter what!
			ASSERT(!bIsAdURL);
			
			//	Add new entry
			LinkHistoryManager::LinkTypeEnum	linkType = bIsAdURL ? lt_Ad : ClassifyURL(szURL);
			InternalAddURL( "", szURL, szAdID, 0, time(NULL), linkType, ls_BookmarkOrRemind );
		}

		//	Remember to remind the user if we are online later
		SetShouldRemind();

		//	We added or updated information, set ourselves to dirty so that we save at idle
		SetDirtyState(true);
	}
}


// ---------------------------------------------------------------------------
//		* MakeThumbnail											[Protected]
// ---------------------------------------------------------------------------
//	Creates a thumbnail bitmap for a given ad image, and saves it in
//	the LinkHistoryFolder as a BMP with the specified name.
//	Note that the dimensions of the thumbnail is 32 pixels wide by 32 pixels
//	tall, although the entire bitmap is in fact 34 pixels tall. The height is
//	padded with an extra blank line above and below the actual thumbnail to
//	make the list view draw with the correct line spacing.

CBitmap *
LinkHistoryManager::MakeThumbnail(
	ImageFormatType		imageType,
	const char *		szImageFullPath,
	const char *		szThumbnailName)
{
	SECImage *		pImage = NULL;
	CBitmap *		pThumbnailBitmap = NULL;

	pImage = LoadImage(szImageFullPath, COLOR_WINDOW);
	if (pImage)
	{
		//	Create the bitmap that we're going to blit into
		CPaletteDC		cdc;
		CWindowDC		screenDC(NULL);
		
		pThumbnailBitmap = DEBUG_NEW_MFCOBJ_NOTHROW CBitmap();

		if ( (pThumbnailBitmap != NULL) && cdc.CreateCDC() &&
			 pThumbnailBitmap->CreateCompatibleBitmap(&screenDC, kLinkTypeIconSize, kLinkTypeIconPaddedHeight) )
		{
			CBitmap *	pSavePrevBitmap = cdc.SelectObject(pThumbnailBitmap);

			//	Erase the background to white
			CRect		rect(0, 0, kLinkTypeIconSize, kLinkTypeIconPaddedHeight);
			CBrush		textBrush( RGB(255,255,255) );
			cdc.FillRect(&rect, &textBrush);

			//	We might prefer HALFTONE, but some msdn docs say that it only works on NT.
			//	COLORONCOLOR seems to work well enough.
			cdc.SetStretchBltMode(COLORONCOLOR);

			::StretchDIBits(
				cdc.GetSafeHdc(),
				0, 1,
				kLinkTypeIconSize, kLinkTypeIconSize,
				0, 0,
				pImage->dwGetWidth(), pImage->dwGetHeight(),
				pImage->m_lpSrcBits,
				pImage->m_lpBMI, DIB_RGB_COLORS,
				SRCCOPY );

			cdc.SelectObject(pSavePrevBitmap);

			SECDib *	pThumbnailBMP = DEBUG_NEW_MFCOBJ_NOTHROW SECDib();
			if (pThumbnailBMP)
			{
				CString		szThumbnailFullPath = m_szLHDirPath + szThumbnailName;
				
				pThumbnailBMP->CreateFromBitmap(&cdc, pThumbnailBitmap);
				pThumbnailBMP->SaveImage(szThumbnailFullPath);

				delete pThumbnailBMP;
			}
		}

		delete pImage;
	}

	return pThumbnailBitmap;
}


// ---------------------------------------------------------------------------
//		* FindUniqueThumbnailName								[Protected]
// ---------------------------------------------------------------------------
//	Finds a unique name for a given thumbnail to be saved in the LinkHistory
//	folder. Takes advantage of the uniqueness of ad image file names (to
//	avoid replicating PrivCacheURL2Path, etc. from plist_privcache.cpp),
//	although it will briefly attempt to find a unique name if necessary. 

bool LinkHistoryManager::FindUniqueThumbnailName(const char * szImageFullPath, CString & szThumbnailName)
{
	szThumbnailName = szImageFullPath;

	//	Detemine just the name of the ad image file, this name is likely to
	//	be unique because the play list private cache code needs it to be.
	int		pos = szThumbnailName.ReverseFind('\\');
	szThumbnailName = szThumbnailName.Right(szThumbnailName.GetLength() - pos - 1);

	//	Remove the .3 extension, if any
	pos = szThumbnailName.ReverseFind('.');
	if ( (pos >= 0) && (szThumbnailName.GetLength() - pos) < 5 )
		szThumbnailName = szThumbnailName.Left(pos);

	//	Check for the existence of the thumbnail file in the LinkHistoryFolder
	CString		szThumbnailFullPath = m_szLHDirPath + szThumbnailName + ".bmp";
	bool		bUniqueNameFound = !::FileExistsMT(szThumbnailFullPath);

	if (!bUniqueNameFound)
	{
		//	This *really* shouldn't happen. plist_privcache.cpp does a good
		//	job of ensuring that a given URL hashes to a collision free name.
		ASSERT(0);

		//	Quick & dirty search for a unique name
		szThumbnailName += "-";

		for (char uniqueChar = 'a'; uniqueChar <= 'z'; uniqueChar++)
		{
			//	Does this variation of the name exist?
			szThumbnailFullPath = m_szLHDirPath + szThumbnailName + uniqueChar + ".bmp";
			if ( !::FileExistsMT(szThumbnailFullPath) )
			{
				bUniqueNameFound = true;
				szThumbnailName += uniqueChar;
				break;
			}
		}
	}

	//	We found a unique name, complete the name (previously we kept the .bmp separate
	//	to make unique search easier).
	if (bUniqueNameFound)
		szThumbnailName += ".bmp";

	return bUniqueNameFound;
}


// ---------------------------------------------------------------------------
//		* AddAdToLinkHistory										[Public]
// ---------------------------------------------------------------------------
//	Adds an ad URL to the Link History Manager. Called from
//	AdCallback, which is in turn called by the Playlist subsystem when
//	ads are downloaded.
//
//	If necessary creates thumbnail corresponding to the ad image pointed
//	to by szImageFullPath. 

void LinkHistoryManager::AddAdToLinkHistory(
	const char *		szName,
	const char *		szImageFileURL,
	const char *		szURL,
	const char *		szAdID)
{
	//	Search for the ad
	UrlItemData			tempItemForFind(lt_Ad, szAdID);
	UrlListIteratorT	urlIterator = m_UrlList.find(&tempItemForFind);
	LPUrlItemData		pAdURL = NULL;
	bool				bThumbnailNeedsCreated = true;

	//	Found if iterator is not end
	if ( urlIterator != m_UrlList.end() )
	{
		//	Update touched date/time to right now
		pAdURL = *urlIterator;
		pAdURL->m_lLastTouchTime = time(NULL);

		//	The ad URL can change, but the ad ID is constant. Update the URL if necessary.
		if (pAdURL->m_szURL != szURL)
		{
			pAdURL->m_szURL = szURL;
			pAdURL->m_szURL.FreeExtra();
		}

		ASSERT(pAdURL->m_type == lt_Ad);

		if (pAdURL->m_status == ls_Removed)
		{
			//	The ad was previously removed. Reset its status and make sure it's displayed.
			if (pAdURL->m_lLastVisitTime == 0)
				pAdURL->m_status = ls_NotVisited;
			else
				pAdURL->m_status = ls_None;
			UpdateLHListDisplay(pAdURL, true);
		}

		if ( !pAdURL->m_szThumbnailFileName.IsEmpty() )
		{
			CString		szThumbnailFullPath = m_szLHDirPath + pAdURL->m_szThumbnailFileName;
			
			bThumbnailNeedsCreated = !::FileExistsMT(szThumbnailFullPath);
		}
	}
	else
	{
		//	Add new entry
		pAdURL = InternalAddURL(szName, szURL, szAdID, 0, time(NULL), lt_Ad, ls_NotVisited);
	}

	if ( bThumbnailNeedsCreated && ::FileExistsMT(m_szLHDirPath) )
	{
		//	Transform the file URL to a file path, by removing the file://
		CString				szImageFullPath = szImageFileURL;
		szImageFullPath.TrimLeft();
		szImageFullPath.TrimRight();
		if ( strnicmp(szImageFullPath, "file://", 7) == 0 )
			szImageFullPath = szImageFullPath.Right(szImageFullPath.GetLength() - 7);

		//	Determine if we can handle the ad image type (we should be able to, but just in case)
		ImageFormatType		imageType = CanHandleImageInternally(szImageFullPath);
		ASSERT(imageType != IF_CANT_HANDLE);

		if (imageType != IF_CANT_HANDLE)
		{
			//	First determine a name to use
			CString		szThumbnailName;
			bool		bUniqueNameFound = FindUniqueThumbnailName(szImageFullPath, szThumbnailName);

			if (bUniqueNameFound)
			{
				CBitmap *	pThumbnailBitmap = MakeThumbnail(imageType, szImageFullPath, szThumbnailName);
				
				if (pThumbnailBitmap != NULL)
				{
					//	Add the thumbnail to the icon image list if it has been created
					if (m_bIconImageListCreated)
						pAdURL->m_nIconIndex = m_IconImageList.Add( pThumbnailBitmap, RGB(255,255,255) );
					pAdURL->m_szThumbnailFileName = szThumbnailName;

					delete pThumbnailBitmap;
				}
			}
		}
	}

	//	We added or updated information, set ourselves to dirty so that we save at idle
	SetDirtyState(true);
}


// ---------------------------------------------------------------------------
//		* InternalAddURL										[Protected]
// ---------------------------------------------------------------------------
//	Adds a URL to the Link History Manager. Mostly used as a helper routine
//	by the main URL adding methods.

LinkHistoryManager::LPUrlItemData
LinkHistoryManager::InternalAddURL(
	LPCSTR				szLinkName,
	LPCSTR				szURL,
	LPCSTR				szAdID,
	long				lLastVisitTime,
	long				lLastTouchTime,
	LinkTypeEnum		type,
	LinkStatusEnum		status)
{
	LPUrlItemData	pNewURL = NULL;
	
	if ( IsTrackedLinkType(type) )
	{
		try
		{
			//	szAdID can be NULL when the URL is not an ad. Guard against this case to be
			//	safe (although it looks like the CString constructor could handle the NULL).
			pNewURL = DEBUG_NEW UrlItemData( szLinkName, szURL, (szAdID == NULL) ? "" : szAdID,
											 "", lLastVisitTime, lLastTouchTime, type, status );
			InternalAddURL(pNewURL);
		}
		catch (CMemoryException* pException)
		{
			// to do -- add an error message
			// this is a less serious error -- it won't screw up the stack
			pException->Delete();
		}
	}

	return pNewURL;
}


// ---------------------------------------------------------------------------
//		* InternalAddURL										[Protected]
// ---------------------------------------------------------------------------
//	Adds a URL to the Link History Manager. The final bottleneck
//	for any URLs that are added.

void LinkHistoryManager::InternalAddURL(LPUrlItemData pURLToAdd)
{
	//	The following strings won't change, so free any extra memory
	pURLToAdd->m_szLinkName.FreeExtra();
	pURLToAdd->m_szURL.FreeExtra();
	pURLToAdd->m_szThumbnailFileName.FreeExtra();
	pURLToAdd->m_szAdID.FreeExtra();

	m_UrlList.insert(pURLToAdd);

	UpdateLHListDisplay(pURLToAdd, true);
}


// ---------------------------------------------------------------------------
//		* RemoveURL												[Public]
// ---------------------------------------------------------------------------
//	Removes an ad URL to the Link History Manager and sets the IsDirty flag.

void LinkHistoryManager::RemoveURL(
	LPUrlItemData pURLToDelete)
{
	if (pURLToDelete->m_type == lt_Ad)
	{
		//	Mark it deleted rather than actually deleting it. It will actually
		//	be removed when we weed out old links. Not deleting it allows us
		//	to correctly remember the title and thumbnail info if the user
		//	later clicks on the ad.
		pURLToDelete->m_status = ls_Removed;
	}
	else
	{
		UrlListIteratorT	urlIterator = m_UrlList.find(pURLToDelete);

		//	Found if iterator is not end
		if ( urlIterator != m_UrlList.end() )
		{
			m_UrlList.erase(urlIterator);
		}
		else
		{
			//	We should have found it, alert the developer that something's awry
			ASSERT(0);
		}

		delete pURLToDelete;
	}

	//	We either removed the URL or marked it to be removed,
	//	set ourselves to dirty so that we save at idle
	SetDirtyState(true);
}


// ---------------------------------------------------------------------------
//		* ClassifyURL											[Protected]
// ---------------------------------------------------------------------------
//	Returns the type of URL based on the prefix.

LinkHistoryManager::LinkTypeEnum LinkHistoryManager::ClassifyURL(LPCSTR szURL)
{
	if ( strnicmp(szURL, "http:", 5) == 0 )
		return lt_Http;
	else if ( strnicmp(szURL, "https:", 6) == 0 )
		return lt_Http;
	else if ( strnicmp(szURL, "mailto:", 7) == 0 )
		return lt_Mail;
	else if ( strnicmp(szURL, "ftp:", 4) == 0 )
		return lt_Ftp;
	else if ( strnicmp(szURL, "x-eudora-option:", 16) == 0 )
		return lt_Setting;
	else if ( (strnicmp(szURL, "ph:", 3) == 0) || (strnicmp(szURL, "finger:", 7) == 0) )
		return lt_DirectoryServices;
	else if ( strnicmp(szURL, "file:", 5) == 0 )
		return lt_File;

	return lt_Unknown;
}


// ---------------------------------------------------------------------------
//		* NotifyLHViewReady											[Public]
// ---------------------------------------------------------------------------
//	Notify routine called by the LinkHistoryList when it's created or
//	switched back to a full featured mode (i.e. not light) so that the
//	LinkHistoryManager can fill it with information.

void LinkHistoryManager::NotifyLHViewReady()
{
	CLinkHistoryList *	pLinkHistoryList = CLinkHistoryList::Instance();
	
	if (pLinkHistoryList != NULL)
	{
		LPUrlItemData		pLHItem;

		for ( UrlListIteratorT urlIterator = m_UrlList.begin();
			 urlIterator != m_UrlList.end();
			 urlIterator++ )
		{
			pLHItem = *urlIterator;

			//	Add all non-deleted links to the LH list view
			if (pLHItem->m_status != ls_Removed)
				pLinkHistoryList->AddURL(pLHItem);
		}
	}
}
