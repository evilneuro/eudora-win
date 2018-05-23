// ImapConnectionMgr.cpp: implementation
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include <fcntl.h>

#include "imapacct.h"
#include "imapactl.h"
#include "imaputil.h"
#include "ImapConnectionMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Globals - one and only.
CImapConnectionMgr g_ImapConnectionMgr;


// ============= INTERNALS ==========
static const DWORD dwLockTimeout = 5000;			// Milliseconds.




//==================== CImapConnection class - the objects in the list ================/

CImapConnection::CImapConnection()
{

	m_RefCount = 0;

	// Create the sync. object. Use a critical section.
	m_pLockable = new CCriticalSection;
}


// Make sure we free our allocated objects in the array elements.
CImapConnection::~CImapConnection()
{
	// Delete the CIMAP connection.
	//
	delete m_pImap;
	m_pImap = NULL;

	delete m_pLockable;
}



// Lock the object and the contained CIMAP.
//
HRESULT CImapConnection::QueryInterface (CIMAP** ppImap)
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

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// Must increment it in QueryInterface.
	//
	m_RefCount++;

	*ppImap = m_pImap;

	return S_OK;
}


LONG CImapConnection::AddRef ()
{
	// Must acquire the critical section.
	//
	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
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
LONG CImapConnection::Release ()
{
	// Must acquire the critical section.
	//
	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
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
	m_pLockable = new CCriticalSection;
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
HRESULT CImapConnectionMgr::RemoveEntry(CImapConnection* pConn)
{
	CImapConnection*	pConnection = NULL;
	int			count = GetCount();

	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// OK. We can zapp the whole list now.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;
	
	for (next = pos; pos; pos = next)
	{
		pConnection = (CImapConnection *) GetNext (next);

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
// NOTE: The returned CIMAP object is a member of a CImapConnection that has had
// it's reference counf incremented.
// When done, the caller MUST RELEASE the object by calling GetImapConnectionMgr()->Release (CIMAP object);
//
HRESULT	CImapConnectionMgr::CreateConnection (unsigned long AccountID, LPCSTR pImapName, CIMAP** ppImap, BOOL CaseSensitive /*= FALSE*/ )
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

	CIMAP* pImap = NULL;

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

			// For a brand new CIMAP instance, we need to set certain stuff into it.
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
// After a client has locked a CIMAP object and finished using it, he calls
// this method so other people can re-use the CIMAP connection.
//

HRESULT CImapConnectionMgr::Release (CIMAP *pImap)
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

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		// MUST return reason here!!
		//
		return E_FAIL;
	}

	// Find the CImapConnection object containing this CIMAP.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;
	CImapConnection *pConn;

	for (next = pos; pos; pos = next)
	{
		pConn = (CImapConnection *) GetNext (next);

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
HRESULT	CImapConnectionMgr::FindByImapName(unsigned long AccountID, LPCSTR pImapName, CIMAP** ppImap, BOOL CaseSensitive /* = FALSE */ )
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

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		// MUST return reason here!!
		//
		return E_FAIL;
	}

	CString	yrName = pImapName;
	CString myName;

	// Strip any trailing directory delimiter.
	StripTrailingDelimiter (yrName);

	CImapConnection*	pConn = NULL;
	CImapConnection*	pConnFound = NULL;

	// Loop through.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;

	// If we found one, this becomes non-null..
	//
	CIMAP* pImap = NULL;

	for (next = pos; pos; pos = next)
	{
		pConn = (CImapConnection *) GetNext (next);

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
// Lock the object before linking it in, and return the CIMAP pointer in ppImap.
//
// NOTE: pImapName can be NULL.
//
HRESULT	CImapConnectionMgr::CreateNewConnection (unsigned long AccountID, LPCSTR pImapName, CIMAP** ppImap)
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
	CImapConnection *pConn = new CImapConnection;

	if (!pConn)
	{
		pAccount->RemoveAdvisoryLock (lockType);

		return E_FAIL;
	}

	// Get the server name for the CIMAP connection.
	//
	CString szServer;
	pAccount->GetAddress(szServer);

	// Create the CIMAP embeded object.
	//
	CIMAP* pImap = new CIMAP (szServer);
	
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
	CIMAP *pNewImap = NULL;

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

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
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
	CImapConnection*	pConnection = NULL;
	int			count = GetCount();

	// We must acquire the lock on the list first.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// OK. We can zapp the whole list now.
	//
	POSITION pos = GetHeadPosition();
	POSITION next;
	
	for (next = pos; pos; pos = next)
	{
		pConnection = (CImapConnection *) GetNext (next);

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

