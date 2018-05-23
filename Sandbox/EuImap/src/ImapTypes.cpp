// ImapTypes.cpp - Implementations of the utility class and structures for IMAP/
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.
#include "summary.h"
#include "imapfol.h"
#include "ImapTypes.h"


// Internals!!
//


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
	CImapFlags* pNewFlags = new CImapFlags;
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
//
unsigned long CUidMap::GetHighestUid ()
{
	unsigned long Uid = 0;

	for (ReverseUidIteratorType ri = rbegin(); ri != rend(); ++ri)
	{
		// First non-zero one.
		//
		Uid = (*ri).first;
		break;
	}

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







//===============================================================================//



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
//
// 
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
//
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



//
// Extract the Uid's from a comma-separated list and add to a CUidMap, 
// We don't have any flags info so set that to 0.
//
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



#endif // ONLY FOR IMAP


