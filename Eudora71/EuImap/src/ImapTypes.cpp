// ImapTypes.cpp
//
// Miscellaneous utility classes and functions.
//
// Copyright (c) 1997-2003 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "ImapTypes.h"

#include "fileutil.h"		// For EudoraDir, JJFileMT
#include "guiutils.h"
#include "rs.h"				// For CRString
#include "summary.h"		// For IMFLAGS_*

//	Defines and other Constants	=============================
#include "DebugNewHelpers.h"


// Debug logger.
void __ImapDebugLogger (char *string,long errflg)
{
	JJFileMT	File;
	CString		path;

	// Try to open the file for appending..
	path = EudoraDir + CRString (IDS_IMAP_TOP_DIRECTORY_NAME) + "\\" + CRString (IDS_IMAP_DEBUG_FILENAME);

	if (SUCCEEDED (File.Open (path, O_WRONLY | O_APPEND | O_CREAT ) ) )
	{
		File.PutLine (string);
	}
}

//
//	GetConnectionState()
//
//	Returns a code indicating the online/offline state of Eudora, possibly after asking the user
//	what to do.  This must also take into account whether the given action is allowed to be
//	performed offline (as indicated by the bAllowOffline parameter).
//
//	Parameters:
//		bAllowOffline [in] - If true then the calling code is in a location where the IMAP action can be
//			legimately performed offline.  If false then we must be online to perform the action.
//
int GetConnectionState(bool bAllowOffline)
{
	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		// Eudora is currently in offline mode.

		// Although there is no support for this, I can imagine a user setting for not allowing IMAP
		// actions to be performed offline.  At the very least for now we need this to disable this
		// feature for testing. -dwiggins
		bool		bOfflineActionsAllowed = (GetIniShort(IDS_INI_IMAP_ALLOW_OFFLINE_ACTIONS) !=0);

		if (bOfflineActionsAllowed && bAllowOffline)
		{
			// User specified that when offline allow actions to be performed and this action is supported
			// when offline: tell caller to proceed with offline action.
			return iStateStayOfflineAllow;
		}
		else
		{
			// Either the user does not want any actions performed offline, or this specific action
			// cannot be performed offline: ask the user what to do.
			int		 iRes = AlertDialog(IDD_ERR_OFFLINE);
			if (iRes == IDC_ONLINE_BUTTON)
			{
				// User said to switch back to online mode: set the flag and return the go online code.
				SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);
				return iStateGoOnline;
			}
			else if (iRes == IDOK)
			{
				// User said to go online for this one action: return the go online for this action code.
				return iStateGoOnlineForThis;
			}
			else
			{
				// User cancelled: return the stay offline and disallow the action code.
				return iStateStayOfflineDisallow;
			}
		}
	}

	// Eudora is online.
	return iStateOnline;
}

// FUNCTION
// Wade through CurrentUidMap and any entries that are also in NewUidMap and have their flags
// modified, move to ModifiedMap with the new flags.
// If there is any entry in NewUidMap that has Uid == 0, remove it.
// AT the end, we should have NewUidMap containing truly new messages and 
// "CurrentUidMap" containing only messages that are no longer on the server.
// "ModifiedMap" will contain only messages that should remain in the local cache,
// but with new flags (e.g, someone may have set the \Deleted flags.
// END FUNCTION
// 
// NOTE:
// END NOTE
void MergeUidMapsMT (CUidMap& CurrentUidMap, CUidMap& NewUidMap, CUidMap& ModifiedMap)
{
	CImapFlags *pOldF, *pNewF;
	unsigned long curUid;

	//
	// Clear NewOldUidList
	//
	ModifiedMap.DeleteAll ();

	// We can loop through CurrentUidMap and do a find() on the next map.
	// Not necessary to play the incrementing game.
	//

	UidIteratorType ci, Nextci;
	unsigned long oldFlags, newFlags;
	unsigned long InterestedFlags = IMFLAGS_SEEN | IMFLAGS_ANSWERED | IMFLAGS_FLAGGED | IMFLAGS_DELETED | IMFLAGS_DRAFT;
	
	for ( ci = CurrentUidMap.begin(); ci != CurrentUidMap.end(); ci = Nextci)
	{
		Nextci = ci; Nextci++;

		pOldF = ( CImapFlags * ) (*ci).second;

		if (pOldF)
		{
			curUid = pOldF->m_Uid;

			// Is it in NewUidMap?? If so, delete it from CurrentUidMap AND from
			// NewUidMap. If it's flags have changed, add it to ModifiedMap. Update flags.
			//
			UidIteratorType newCi = NewUidMap.find (curUid);

			if (newCi != NewUidMap.end())
			{
				pNewF = ( CImapFlags * ) (*newCi).second;

				// Remove from current map.
				CurrentUidMap.erase (ci);

				if (pNewF)
				{
					oldFlags = pOldF->m_Imflags  & InterestedFlags;
					newFlags = pNewF->m_Imflags  & InterestedFlags;

					if (oldFlags != newFlags)
					{
						//
						// Merge flags. Set the Seen flag from our value but copy the rest from remote.
						//
						// First, clear the bits we'er interested in.
						pOldF->m_Imflags &= ~InterestedFlags;
						//
						pOldF->m_Imflags |= newFlags;
	
						// Add it to ModifiedMap.
						// 
						ModifiedMap [curUid] = pOldF;
					}
					else
					{
						delete pOldF; pOldF = NULL;
					}
				}

				// Remove from new map.
				//
				NewUidMap.erase (newCi);

				if (pNewF)
				{
					delete pNewF;  pNewF = NULL;
				}
			}
		}
		else
		{
			// Invalid entry.
			//
			CurrentUidMap.erase (ci);
		}
	}
}											

// RemoveSubUidMapMT [EXPORTED]
//
// Remove all UID's that are in "SubUidMap" from "MainUidMap"
void RemoveSubUidMapMT (CUidMap& MainUidMap,CUidMap& SubUidMap)
{
	CImapFlags *pF;
	unsigned long curUid;

	// We can loop through CurrentUidMap and do a find() on the next map.
	// Not necessary to play the incrementing game.
	//

	UidIteratorType ci, Nextci;
	
	for ( ci = MainUidMap.begin(); ci != MainUidMap.end(); ci = Nextci)
	{
		Nextci = ci; Nextci++;

		// Is it in the sub map?
		//
		curUid = (*ci).first;

		UidIteratorType subci = SubUidMap.find (curUid);

		if ( subci != SubUidMap.end() )
		{
			pF = (*ci).second;

			// Remove from main map.
			//
			MainUidMap.erase (ci);

			if (pF)
				delete pF;
		}
	}
}											

// Extract the Uid's from a comma-separated list and add to a CUidMap, 
// We don't have any flags info so set that to 0.
void CommaListToUidMap (LPCSTR pStr, CUidMap& DstUidMap)
{
	const char *p, *q;
	char Comma = ',';
	char buf [12];
	unsigned long len;
	unsigned long Uid;

	p = pStr;

	while (p && *p)
	{
		*buf = 0;

		q = strchr (p, Comma);

		if (q)
		{
			len = (unsigned long)q - (unsigned long)p;

			if ( len < sizeof(buf) )
			{
				strncpy (buf, p, len);
				buf[len] = 0;
			}

			// Advance over comma.
			p = q + 1;
		}
		else
		{
			if (strlen (p) < sizeof (buf) )
			{
				strcpy (buf, p);
			}

			// Terminate.
			p = NULL;
		}

		Uid = atol (buf);

		if (Uid > 0)
		{
			DstUidMap.OrderedInsert (Uid, 0, FALSE);
		}
	}
}


//=================================================================================//
// ======================= CPtrUidList implementation =============================//

CPtrUidList::CPtrUidList()
{
}

CPtrUidList::~CPtrUidList()
{
	// Make sure none left.
	DeleteAll();
}

// DeleteAll [PUBLIC]
// FUNCTION
// Delete all data in the list and call RemoveAll.
// END FUNCITON
void CPtrUidList::DeleteAll ()
{
	POSITION pos, next;
	CImapFlags *pF;

	pos = GetHeadPosition();

	for( next = pos; pos; pos = next )
	{
		pF = ( CImapFlags * ) GetNext( next );
		if (pF)
		{
			SetAt (pos, NULL);
			delete pF;
		}
	}

	RemoveAll();
	
}

// OrderedInsert
// FUNCTION
// Insert uid and Flags in ascending order of uid.
// END FUNCTION
// NOTES
// Insert Imflags as is.
// END NOTES.
POSITION CPtrUidList::OrderedInsert(unsigned long uid, unsigned long Imflags, BOOL IsNew)
{
	POSITION			pos, next;
	CImapFlags			*pNewFlags, *pCurFlags;
			
	if (uid == 0)
		return NULL;

	// Initialize
	pNewFlags = pCurFlags = NULL;
	
	// Insert in order of uid.
	pos = GetHeadPosition();
	next = pos;

	// If there is a previous entry with the same uid, modify that.
 	// We insert message in ascending order of uid. 
	for(; pos != NULL; pos = next )
	{
		pCurFlags = (CImapFlags *) GetNext( next );
		
		if( pCurFlags )
		{
			// Do we already have an entry with same uid?
			if (pCurFlags->m_Uid == uid)
			{
				// Ok. Just modify this and get out.
				pCurFlags->m_Imflags = Imflags;
				pCurFlags->m_IsNew = IsNew;

#ifdef _MYDEBUG
{
				char szUid[128];
				sprintf (szUid, "Found equal uid in OrderedInsert: %lu\n", uid);
				__ImapDebugLogger (szUid, 0);
}

#endif // _MYDEBUG

				return pos;
			}
			else if (pCurFlags->m_Uid < uid)
				continue;
			else
				break;
		}
	}


	// If we get here, we need to add a new entry.
	// Allocate mem CImapFlags object.
	pNewFlags = DEBUG_NEW_NOTHROW CImapFlags;
	if (!pNewFlags)
		return NULL;

	pNewFlags->m_Uid = uid;
	pNewFlags->m_Imflags = Imflags;
	pNewFlags->m_IsNew = IsNew;


#ifdef _MYDEBUG
{
				char szUid[128];
				sprintf (szUid, "Adding uid = %lu in OrderedInsert\n", uid);
				__ImapDebugLogger (szUid, 0);
}

#endif // _MYDEBUG


	// Insert in order..
	if( pos )
	{
		return InsertBefore( pos, pNewFlags );
	}
	else
	{
		return  AddTail( pNewFlags );
	}
}

// OrderedInsert
// FUNCTION
// Convert the given BOOL's into bitflags and call OrderedInsert (uid, flags).
// Remember that IMAP uid's are in ascending order!
// END FUNCTION
// NOTES
// "uid" MUST be non-zero, otherwise we can't do an insert!! 
// END NOTES.
POSITION CPtrUidList::OrderedInsert(unsigned long uid,
									BOOL seen,
									BOOL deleted,
									BOOL flagged,
									BOOL answered,
									BOOL draft,
									BOOL recent,
									BOOL IsNew)
{
	unsigned long		Flags;
			
	if (uid == 0)
		return NULL;

	// Set flags.
	Flags = 0;

	Flags |= (seen		? IMFLAGS_SEEN : 0);
	Flags |= (answered	? IMFLAGS_ANSWERED : 0);
	Flags |= (flagged	? IMFLAGS_FLAGGED : 0);
	Flags |= (deleted	? IMFLAGS_DELETED : 0);
	Flags |= (draft		? IMFLAGS_DRAFT : 0);
	Flags |= (recent	? IMFLAGS_RECENT : 0);

	// Now call other method.
	return OrderedInsert (uid, Flags, IsNew);
}


//========== CImapFlags ====================//

CImapFlags::CImapFlags (unsigned long msgno /* = 0 */, unsigned long uid /* = 0 */,
				unsigned long Imflags /* = 0 */, BOOL IsNew /* = FALSE */)
{
	m_Imflags	= 0;
	m_Uid		= uid;
	m_IsNew		= IsNew;
}


//=============== CPtrUidList will be replaced by CUidMap ==========================/

CUidMap::~CUidMap()
{
	DeleteAll ();
}

// DeleteAll [PUBLIC]
// FUNCTION
// Delete all entries in the map AND delete data items.
// END FUNCITON
void CUidMap::DeleteAll ()
{
	for (UidIteratorType ci = begin(); ci != end(); ++ci)
	{
		CImapFlags *pF = (*ci).second;
		delete pF;
	}

	// Remove the entries.
	//
	RemoveAll();
}

// RemoveAll [PUBLIC]
// FUNCTION
// Delete all entries in the map without deleting data items.
// END FUNCITON
void CUidMap::RemoveAll ()
{
	erase (begin(), end());
}

// OrderedInsert
// FUNCTION
// Insert uid and Flags in ascending order of uid.
// END FUNCTION
// NOTES
// Insert Imflags as is.
// END NOTES.
BOOL CUidMap::OrderedInsert(unsigned long uid, unsigned long Imflags, BOOL IsNew)
{
	// Must have a uid.

#if 0 // Allow a zero value!!
	if (uid == 0)
		return FALSE;
#endif //

	// If we already have one with this uid, just modify it's parms.
	//
	UidIteratorType ci = find (uid);

	// Must always check with "end()".
	//
	if ( ci != end() )
	{
		CImapFlags *pF = (*ci).second;

		if (pF)
		{
			// Ok. Just modify this and get out.
			pF->m_Imflags = Imflags;
			pF->m_IsNew = IsNew;
		}

		return TRUE;
	}
		
	// Add it.
	// A "map" enforces a unique key.
	//
	// Allocate mem CImapFlags object.
	//
	CImapFlags* pNewFlags = DEBUG_NEW_NOTHROW CImapFlags;
	if (!pNewFlags)
		return FALSE;

	pNewFlags->m_Uid = uid;
	pNewFlags->m_Imflags = Imflags;
	pNewFlags->m_IsNew = IsNew;

	(*this)[uid] = pNewFlags;

	return TRUE;
}

// OrderedInsert
// FUNCTION
// Convert the given BOOL's into bitflags and call OrderedInsert (uid, flags).
// Remember that IMAP uid's are in ascending order!
// END FUNCTION
// NOTES
// "uid" MUST be non-zero, otherwise we can't do an insert!! 
// END NOTES.
BOOL CUidMap::OrderedInsert(unsigned long uid,
									BOOL seen,
									BOOL deleted,
									BOOL flagged,
									BOOL answered,
									BOOL draft,
									BOOL recent,
									BOOL IsNew)
{
	unsigned long		Flags;
			
	if (uid == 0)
		return FALSE;

	// Set flags.
	Flags = 0;

	Flags |= (seen		? IMFLAGS_SEEN : 0);
	Flags |= (answered	? IMFLAGS_ANSWERED : 0);
	Flags |= (flagged	? IMFLAGS_FLAGGED : 0);
	Flags |= (deleted	? IMFLAGS_DELETED : 0);
	Flags |= (draft		? IMFLAGS_DRAFT : 0);
	Flags |= (recent	? IMFLAGS_RECENT : 0);

	// Now call other method.
	return OrderedInsert (uid, Flags, IsNew);
}

// The objects are ordered by UID, the smallest first.
// Use "rbegin()" and work from the bottom.
unsigned long CUidMap::GetHighestUid ()
{
	unsigned long Uid = 0;

	// Dale - Take a look at this.
	// Old way
	//for (ReverseUidIteratorType ri = rbegin(); ri != rend(); ++ri)
	//{
		// First non-zero one.
		//
		//Uid = (*ri).first;
	//	break;
	//}
	//
	// New way (avoids compiler warnings)
	ReverseUidIteratorType ri = rbegin();
	Uid = (*ri).first;

	// Could be 0.
	return Uid;
}


//=========================== CStringMap ======================================//

CStringMap::~CStringMap()
{
	DeleteAll ();
}

// DeleteAll [PUBLIC]
// FUNCTION
// Delete all entries in the map AND delete data items.
// END FUNCITON
void CStringMap::DeleteAll ()
{
	// We don't store any data objects, so just remove the entries.
	//

	RemoveAll();
}

// RemoveAll [PUBLIC]
// FUNCTION
// Delete all entries in the map without deleting data items.
// END FUNCITON
void CStringMap::RemoveAll ()
{
	erase (begin(), end());
}

#endif // IMAP4
