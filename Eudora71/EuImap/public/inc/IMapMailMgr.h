#ifndef __IMAPMAILMGR_H__
#define __IMAPMAILMGR_H__

// ImapMailMgr.h : header file
//


class QCPOPNotifier;


// ========== CImapAccountMgr class ============/
// 
// This is the global array of "account lists". Its elements are
// CImapAccountList's
// 
class CImapMailMgr
{
public:
	CImapMailMgr();
	~CImapMailMgr();


// Interface
public:
	// Public interface to checking mail.
	//
	HRESULT			ImapGetMail(LPCSTR pszPersonaName,
								unsigned long ulBitflags,
								BOOL bInBackground = FALSE);

	HRESULT			ImapGetMail(LPCSTR pszPersonaName,
								unsigned long ulBitflags,
								QCPOPNotifier *pNewMailNotifier,
								BOOL bInBackground /*=FALSE*/,
								BOOL bFullMailCheck = TRUE);

	// Check mail
	HRESULT			CheckMail(LPCSTR szPersona,
							  CTocDoc *pTocDoc,
							  BOOL bCheck,
							  unsigned long ulBitflags,
							  QCPOPNotifier *pNewMailNotifier = NULL,
							  BOOL bDownloadedOnly = TRUE);

	// Check mail
	HRESULT			CheckMailOnServer(LPCSTR szPersona,
									  CTocDoc *pTocDoc,
									  BOOL bCheck,
									  unsigned long ulBitflags,
									  QCPOPNotifier *pNewMailNotifier = NULL,
									  BOOL bDownloadedOnly = TRUE);

	// Background resync of any mailbox.
	//
	HRESULT			DoManualResync (CTocDoc *pTocDoc,
								       BOOL bCheckMail = FALSE,
									   BOOL bDownloadedOnly = TRUE,
									   BOOL bInBackground = FALSE,
									   BOOL bDoSubMailboxes = FALSE);

	// Resync all opened TOCS:
	void			ResyncOpenedTocs ();

	// Task list management.
	//
	void			RemoveTask (LPCSTR pTaskKey);


	// Initialize and cleanup IMAP.
	//
	static void	InitializeImapProvider();
	static void CloseImapConnections (int nAccountID = -1);
	static void	Shutdown ();

	// Notify IMAP subsystem that the application has been initialized on startup.
	static void NotifyAppInitialized();
	static BOOL AppIsInitialized()
		{ return m_bAppIsInitialized; }

// 
private:
	// Internal methods:
	HRESULT CheckMailInBackground(LPCSTR pszPersona,
								  unsigned long ulBitflags,
								  QCPOPNotifier *pNewMailNotifier,
								  BOOL bFullMailCheck = TRUE);

	// Access to task list.
	//
	BOOL	AddTask(unsigned long AccountID, LPCSTR pImapName, CString& szTaskKey);
	void	GenerateKey (unsigned long AccountID, LPCSTR pImapName, CString& szKey);

private:
	// private state:

	// These are set by the last ImapGetMail.
	BOOL m_bGotNewMail;

	// Global flag to tell IMAP subsystem that the application has been initialized after
	// startup:
	static BOOL m_bAppIsInitialized;

};



// Exported functions
CImapMailMgr* GetImapMailMgr ();


#endif // __IMAPMAILMGR_H__
