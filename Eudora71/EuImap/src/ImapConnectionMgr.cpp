// ImapConnectionMgr.cpp: implementation
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include <fcntl.h>

#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapConnection.h"
#include "ImapConnectionMgr.h"

#include "DebugNewHelpers.h"


// Globals - one and only.
CImapConnectionMgr g_ImapConnectionMgr;


//==================== CImapConnectionEnvelope class - the objects in the list ================/

CImapConnectionEnvelope::CImapConnectionEnvelope()
{

	m_RefCount = 0;

	// Create the sync. object. Use a critical section.
	m_pLockable = DEBUG_NEW CCriticalSection;
}


// Make sure we free our allocated objects in the array elements.
CImapConnectionEnvelope::~CImapConnectionEnvelope()
{
	// Delete the CImapConnection connection.
	//
	delete m_pImap;
	m_pImap = NULL;

	delete m_pLockable;
}



// Lock the object and the contained CImapConnection.
//
HRESULT CImapConnectionEnvelope::QueryInterface (CImapConnection** ppImap)
{
	if (!ppImap)
		return E_INVALIDARG;

	// Make sure:
	//
	*ppImap = NULL;

	// Must acquire the critical section.
	//
	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return E_FAIL;
	}

	// Must increment it in QueryInterface.
	//
	m_RefCount++;

	*ppImap = m_pImap;

	return S_OK;
}


LONG CImapConnectionEnvelope::AddRef ()
{
	// Must acquire the critical section.
	//
	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return m_RefCount;
	}

	return ++m_RefCount;
}



// Release [PUBLIC]
//
// THis function just decrements the reference count. It doesn't
// attempt to free the object, even if it's ref count goes to zero!!
//
LONG CImapConnectionEnvelope::Release ()
{
	// Must acquire the critical section.
	//
	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		// Don't return zero!!
		return m_RefCount;
	}

	m_RefCount--;

	// Shouldn't!!
	//
	if (m_RefCount < 0)
	{
		ASSERT (0);
	}

	return m_RefCount;
}






// ================== class CImapConnectionMgr =================================

CImapConnectionMgr::CImapConnectionMgr()
{

	// Create the sync. object. Use a critical section.
	m_pLockable = DEBUG_NEW CCriticalSection;
}


// Make sure we free our allocated objects in the array elements.
CImapConnectionMgr::~CImapConnectionMgr()
{
	DeleteAllConnections ();

	delete m_pLockable;
}




// RemoveEntry [PUBLIC]
//
// Remove an entry from the list. Note: This does not attempt to 
// delete the object itself, just it;s place in the list.
//
HRESULT CImapConnectionMgr::RemoveEntry(CImapConnectionEnvelope* pConn)
{
	CImapConnectionEnvelope*	pConnection = NULL;

	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return E_FAIL;
	}

	// OK. We can zapp the whole list now.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;
	
	for (next = pos; pos; pos = next)
	{
		pConnection = (CImapConnectionEnvelope *) GetNext (next);

		RemoveAt (pos);
	}

	return S_OK;
}




//	CreateConnection [PUBLIC]
//
// This is what the world sees. 
// First attempt to find a connection that matches. If that fails,
// we have to create a new one.
//
// NOTE: The returned CImapConnection object is a member of a CImapConnectionEnvelope that has had
// it's reference counf incremented.
// When done, the caller MUST RELEASE the object by calling GetImapConnectionMgr()->Release (CImapConnection object);
//
HRESULT	CImapConnectionMgr::CreateConnection (unsigned long AccountID, LPCSTR pImapName, CImapConnection** ppImap, BOOL CaseSensitive /*= FALSE*/ )
{
	HRESULT hResult = E_FAIL;

	// Sanity:
	//
	if (!ppImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Init:
	*ppImap = NULL;

	// Attempt to get an existing one.
	//

	CImapConnection* pImap = NULL;

	// Note: FindByImapName would have called AddRef on the connection object!!
	//
	hResult = FindByImapName(AccountID, pImapName, &pImap);

	if ( pImap && SUCCEEDED (hResult) )
	{
		// Note: Already properly AddRef'd.
		//
		*ppImap = pImap;

		hResult =  S_OK;
	}
	else
	{
		// If we get here, we have to create a new connection.
		// Note: Create new connection LOCKS the connection object
		// before returning.
		//
		pImap = NULL;

		// Create new connection would also have AdRef'd the connection object.
		//
		hResult = CreateNewConnection (AccountID, pImapName, &pImap);

		if ( pImap && SUCCEEDED (hResult) )
		{
			*ppImap = pImap;

			// For a brand new CImapConnection instance, we need to set certain stuff into it.
			//
			pImap->SetAccountID ( AccountID );

			CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (AccountID);
			if (pAccount)
			{
				pImap->SetAccount (pAccount );
			}

			// Tell it to grab a copy of current network settings.
			//
			pImap->UpdateNetworkSettings();

			hResult = S_OK;
		}
		else
		{
			hResult = E_FAIL;
		}
	}



	return hResult;

}



// Release [PUBLIC]
//
// After a client has locked a CImapConnection object and finished using it, he calls
// this method so other people can re-use the CImapConnection connection.
//

HRESULT CImapConnectionMgr::Release (CImapConnection *pImap)
{
	HRESULT hResult = E_FAIL;

	if (!pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Lock the list.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		// MUST return reason here!!
		//
		return E_FAIL;
	}

	// Find the CImapConnectionEnvelope object containing this CImapConnection.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;
	CImapConnectionEnvelope *pConn;

	for (next = pos; pos; pos = next)
	{
		pConn = (CImapConnectionEnvelope *) GetNext (next);

		if (pConn)
		{
			if ( pImap == pConn->GetImapObject() )
			{
				LONG lRef = pConn->Release();

				if (lRef <= 0)
				{
					// Means no one else has a pointer to this object.
					// And, since we have the list locked, no one else can
					// acquire the object while we're here.
					// So, we can remove the object from the list and delete it here.
					//
					RemoveAt (pos);

					delete pConn;

					pConn = NULL;
				}

				hResult = S_OK;

				break;
			}
		}
	}

	// If we didn't find it, SERIOUS ERROR.
	//
	if (!SUCCEEDED (hResult) )
	{
		ASSERT (0);
	}

	return hResult;
}



	
//////////////////////////////////////////////////////////////////////
// FindUnlockedByImapName [PRIVATE] [THREADSAFE]
//
// Given an account ID and an IMAP name, loop through all connections looking for one
// that matches and attempt to lock it.
// Return the CImapCOnnection ONLY IF WE WERE ABLE TO LOCK IT.
//
//////////////////////////////////////////////////////////////////////
HRESULT	CImapConnectionMgr::FindByImapName(unsigned long AccountID, LPCSTR pImapName, CImapConnection** ppImap, BOOL CaseSensitive /* = FALSE */ )
{
	HRESULT hResult = E_FAIL;

	// Note: pImapName can be NULL, in which case we're looking for a "Control Stream".
	//
	if (!ppImap)
	{
		return E_FAIL;
	}

	// Init:
	*ppImap = NULL;


	// Acquire a lock on the list:
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		// MUST return reason here!!
		//
		return E_FAIL;
	}

	CString	yrName = pImapName;
	CString myName;

	// Strip any trailing directory delimiter.
	StripTrailingDelimiter (yrName);

	CImapConnectionEnvelope*	pConn = NULL;
	CImapConnectionEnvelope*	pConnFound = NULL;

	// Loop through.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;

	// If we found one, this becomes non-null..
	//
	CImapConnection* pImap = NULL;

	for (next = pos; pos; pos = next)
	{
		pConn = (CImapConnectionEnvelope *) GetNext (next);

		pConnFound	= NULL;
		pImap		= NULL;

		if (pConn)
		{
			// Must have same account ID at least.
			//
			if (AccountID == pConn->GetAccountID() )
			{
				// If we're given a name, return it.
				//
				if ( yrName.IsEmpty() )
				{
					// Attempt to lock this below:
					//
					pConnFound = pConn;
				}
				else
				{
					// Well, we've got to compare names.

					pConn->GetImapName (myName);

					StripTrailingDelimiter (myName);

					BOOL iResult = FALSE;

					if (CaseSensitive)
					{
						iResult = yrName.Compare (myName);
					}
					else
					{
						iResult = yrName.CompareNoCase (myName);
					}
					if (iResult == 0)
					{
						pConnFound = pConn;
					}
				}
			} // if same accountID.

			if (pConnFound)
			{
				// This is it.
				//
				pConnFound->AddRef();

				*ppImap = pConnFound->GetImapObject();

				hResult = S_OK;

				break;
			}
		} // if pConn.

		// Didn't find it. Try next one.
		//
		pConnFound	= NULL;
		pImap		= NULL;
	}

	return hResult;
}







// CreateNewConnection [PRIVATE]
//
// Create a new connection object and link it into the list.
// Lock the object before linking it in, and return the CImapConnection pointer in ppImap.
//
// NOTE: pImapName can be NULL.
//
HRESULT	CImapConnectionMgr::CreateNewConnection (unsigned long AccountID, LPCSTR pImapName, CImapConnection** ppImap)
{
	HRESULT hResult = S_OK;

	// Sanity:
	if (!ppImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Init:
	*ppImap = NULL;

	// Must have a valid account ID.
	//
	CImapAccount* pAccount = NULL;
	AccountLockType lockType = ACCOUNT_LOCK_DONT_DELETE;

	// Note: This sets an advisory lock on account object. We must unlock it before we leave here.
	//
	hResult = GetImapAccountMgr()->FindAccount(AccountID, &pAccount, lockType);

	if (! SUCCEEDED (hResult) )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// pAccount should be valid here!!
	//
	ASSERT (pAccount);

	// Create the new object.
	//
	CImapConnectionEnvelope *pConn = DEBUG_NEW_MFCOBJ_NOTHROW CImapConnectionEnvelope;

	if (!pConn)
	{
		pAccount->RemoveAdvisoryLock (lockType);

		return E_FAIL;
	}

	// Get the server name for the CImapConnection connection.
	//
	CString szServer;
	pAccount->GetAddress(szServer);

	// Create the CImapConnection embeded object.
	//
	CImapConnection* pImap = DEBUG_NEW_MFCOBJ_NOTHROW CImapConnection (szServer);
	
	if (!pImap)
	{ 
		delete pConn;

		pAccount->RemoveAdvisoryLock (lockType);

		return E_FAIL;
	}

	// Lock it up and add to list.
	//
	pConn->SetImapObject ( pImap );

	pConn->SetAccountID (AccountID);

	// Note: pImapName can be NULL.
	pConn->SetImapName (pImapName);

	// Increment ref count.
	//
	CImapConnection *pNewImap = NULL;

	hResult = pConn->QueryInterface (&pNewImap);

	if (pNewImap != pImap)
	{
		// This will free pImap.
		// Note: Hasn't been added to the list as yet.
		//
		delete pConn;

		pAccount->RemoveAdvisoryLock (lockType);

		return E_FAIL;
	}

	// Seems like we succeeded. Add to list.
	//

	// Acquire a lock on the list:
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		delete pConn;

		pAccount->RemoveAdvisoryLock (lockType);

		return E_FAIL;
	}

	// Add to end of list.
	//
	AddTail (pConn);

	*ppImap = pNewImap;

	pAccount->RemoveAdvisoryLock (lockType);

	return S_OK;
}





// DeleteAllConnections [PRIVATE]
//
// Delete all account from this account list. That means deleting the folder tree
// from each account as well.
HRESULT CImapConnectionMgr::DeleteAllConnections(void)
{
	CImapConnectionEnvelope*	pConnection = NULL;

	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return E_FAIL;
	}

	// OK. We can zapp the whole list now.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;
	
	for (next = pos; pos; pos = next)
	{
		pConnection = (CImapConnectionEnvelope *) GetNext (next);

		SetAt (pos, NULL);

		if (pConnection)
			delete pConnection;
	}

	// Free list memory. Note that we had to free the objects ourselves.
	RemoveAll();

	return S_OK;
}




//=========================================================================//



// Exported function to return the global account manager.
//

CImapConnectionMgr* GetImapConnectionMgr()
{
	return &g_ImapConnectionMgr;
}




#endif // IMAP4

