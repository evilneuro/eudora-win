#ifndef __IMAPCONNECTIONMGR__H
#define __IMAPCONNECTIONMGR__H

// This class manages a list of CIMAP objects. Whenever any IMAP object needs
// to communicate with an IMAP server, it comes to this class and requests
// a CIMAP object for the given account (represented by an ACCOUNT ID).
// If a mailbox is specified, a SELECT is performed on the mailbox as well.
//
// NO ONE MUST CREATE a CIMAP object outside of this class.
//

// For synchronization objects.
#include <afxmt.h>

// Forward defs.
class CIMAP;

// ========== CImapAccountMgr class ============/
// 
// This is an object that contains a CIMAP pointer associated with an ACCOUNT ID and 
// possibly an IMAP mailbox name.
//
// 
class CImapConnection : public CObject
{
public:
	CImapConnection();
	~CImapConnection();

	// Interface:
	//

	// Create the internal CIMAP object. MUST have a valid account.
	//
	HRESULT		Create (unsigned long AccountID, LPCSTR pImapName = NULL);

private:
	// Only CImapConnectionMgr can access these:
	//
	friend class CImapConnectionMgr;

	// Access - only by CImapConnectionMgr.
	//
	void SetAccountID (unsigned long AccountID)
		{ m_AccountID = AccountID; }

	unsigned long GetAccountID ()
		{ return m_AccountID; }

	void GetImapName (CString& szImapName)
	{
		// Copy:
		//
		szImapName = m_szImapMailboxName;
	}

	void SetImapName (LPCSTR pImapName)
	{
		// Copy:
		//
		m_szImapMailboxName = pImapName;
	}

	void SetImapObject (CIMAP* pImap)
	{	m_pImap = pImap; }

	CIMAP* GetImapObject ()
	{	return m_pImap; }


	// Interface to reference counting on the object.
	//  Don't delete the object until it's reference count goes to zero.
	//
	HRESULT		QueryInterface (CIMAP** ppImap);
	LONG		AddRef();
	LONG		Release ();


// Private list of IMAP lists. User shouldn't know about this.
private:
	// Internal data:
	//

	// Main connection object. This is created internally and must be freed in the
	// destructor.
	//
	CIMAP*			m_pImap;

	// MUST have a valid account ID.
	unsigned long	m_AccountID;

	// If associated with a mailbox, this is the name on the server.
	CString		m_szImapMailboxName;

	// Locking attributes:
	//
	CSyncObject* m_pLockable;

	LONG		m_RefCount;

};



//========================== CImapConnectionMgr class =========================/


// This class maintains a list of CImapConnection objects, each of which contains 
// a CIMAP object. The CIMAP object manages the connection to the server.
// There is one global in-memory list of CImapConnection's and whenever anyone
// wants to do IMAP protocol things. he has to get a pointer to a CIMAP object 
// from this class. This class will LOCK the object until the caller 
// responds with an unlock request.
// A CIMAP object can do JUST ONE IMAP command at a time (we may relax this later).
// Users of the CIMAP object will call "CreateConnection()" (below), which will either
// create a new CImapConnection object OR find an unused one for this account ID 
// (and possibly an IMAP mailbox name) and return the CIMAP object.
// Once the user finishes with the CIMAP object, he calls Release (pImap) so another user
// acquire it.
//
class CImapConnectionMgr : public CObList
{

public:
	CImapConnectionMgr();
	~CImapConnectionMgr();
	
// Interface
public:

	// This is what the world sees. 
	HRESULT	CreateConnection (unsigned long AccountID, LPCSTR pImapName, CIMAP** ppImap, BOOL CaseSensitive = FALSE );

	HRESULT Release (CIMAP *pImap);

	// Removes an entry without deleting the object itself.
	//
	HRESULT RemoveEntry(CImapConnection* pConn);

// Attributes:
private:
	HRESULT	FindByImapName(unsigned long AccountID, LPCSTR pImapName, CIMAP** ppImap, BOOL CaseSensitive = FALSE);

	HRESULT	CreateNewConnection (unsigned long AccountID, LPCSTR pImapName, CIMAP** ppImap);

	// Close connections and destroy the object in the list.
	//
	HRESULT	DeleteAllConnections (void);

// Atributes:
private:
	// Syncgronization object for locking the list while we search it or create a new one.
	//
	CSyncObject* m_pLockable;

};



// Globals

CImapConnectionMgr* GetImapConnectionMgr();


#endif // __IMAPCONNECTIONMGR__H
