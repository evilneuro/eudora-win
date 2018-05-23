// MyTypes.cpp - Implementations of the utility class and structures for IMAP/
//

#include "stdafx.h"

#include "MyTypes.h"

#include "DebugNewHelpers.h"


// Internals!!
//


//=============== CPtrUidList will be replaced by CUidMap ==========================/

CMsgMap::~CMsgMap()
{
	RemoveAll ();
}





// RemoveAll [PUBLIC]
// FUNCTION
// Delete all entries in the map without deleting data items.
// END FUNCITON
void CMsgMap::RemoveAll ()
{
	erase (begin(), end());
}



//=================================================================================//
// ======================= CPtrUidList implementation =============================//

CPtrMsgList::CPtrMsgList()
{
	InitializeCriticalSection(&m_hMsgLockable);
}

CPtrMsgList::~CPtrMsgList()
{
	// Make sure none left.
	DeleteAll();

	// Put this last!!/
	//
	DeleteCriticalSection(&m_hMsgLockable);
}

// DeleteAll [PUBLIC]
// FUNCTION
// Delete all data in the list and call RemoveAll.
// END FUNCITON
void CPtrMsgList::DeleteAll ()
{
	// Protect this!!!
	//
	EnterCriticalSection(&m_hMsgLockable);

	POSITION pos, next;
	unsigned long *pF;

	pos = GetHeadPosition();

	for( next = pos; pos; pos = next )
	{
		pF = ( unsigned long * ) GetNext( next );
		if (pF)
		{
			SetAt (pos, NULL);
			delete pF;
		}
	}

	RemoveAll();

	// Make sure:
	LeaveCriticalSection (&m_hMsgLockable);
}



// Append
// FUNCTION
// Append a new object. Note: We assume that when adding msgno's to the list, we only
// higher msgnos, and consecutively!!!
//
// END FUNCTION
// NOTES
// END NOTES.

POSITION CPtrMsgList::Append(unsigned long msgno, unsigned long Uid)
{
	POSITION		pos = NULL;
			
	if (msgno == 0)
		return NULL;

	// Protect this!!!
	//
	EnterCriticalSection(&m_hMsgLockable);

	// We MUST be doing  this in sequence!!
	// This allows us to arbitrarily call this function anytime we get a UID response from the
	// server.
	//
	if ( msgno == (unsigned)GetCount() + 1 )
	{
		// Allocate mem unsigned long.
		//
		unsigned long* pNewUid = DEBUG_NEW_NOTHROW unsigned long;

		if (pNewUid)
		{
			*pNewUid = Uid;

			pos = AddTail( pNewUid );
		}
	}

	// Make sure.
	LeaveCriticalSection(&m_hMsgLockable);

	return pos;
}




// Remove [PUBLIC]
//
// Return the removed UID, or 0 if error.
//
unsigned long CPtrMsgList::Remove (unsigned long msgno)
{
	POSITION		pos, next;
	unsigned long	ulResult = 0;

	// Protect this!!!
	//
	EnterCriticalSection(&m_hMsgLockable);

	pos = GetHeadPosition ();

	unsigned long i = 1;

	for(next = pos; pos && i < msgno; pos = next, i++ )
	{
		GetNext (next);
	}

	if (pos == NULL || i != msgno)
	{
		ASSERT (0);
	}
	else
	{
		unsigned long* pUid = (unsigned long * )GetAt (pos);

		if (pUid)
		{
			ulResult = *pUid;
			delete pUid;
		}

		RemoveAt (pos);
	}

	// Make sure.
	LeaveCriticalSection(&m_hMsgLockable);
	
	return ulResult;
}



// FindUid [PUBLIC]
//
// Loop through the list and return the uid in the position corresponding to msgno.
// Return 0 if error.
//
unsigned long CPtrMsgList::FindUid (unsigned long msgno)
{
	// Protect this!!!
	//
	EnterCriticalSection(&m_hMsgLockable);

	// Return value:
	unsigned long ulResult = 0;

	POSITION pos = GetHeadPosition();
	POSITION next;
	unsigned long i = 1;

	for(next = pos; pos != NULL && i < msgno; pos = next, i++ )
	{
		GetNext (next);
	}

	if (pos == NULL || i != msgno)
	{
		ASSERT (0);
	}
	else
	{
		unsigned long* pCurUid = (unsigned long *)GetAt (pos);

		if (pCurUid)
		{
			ulResult = *pCurUid;
		}
	}

	// Make sure.
	LeaveCriticalSection(&m_hMsgLockable);

	return ulResult;
}




