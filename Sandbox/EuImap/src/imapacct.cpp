// imapacct.cpp: implementation
//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include <fcntl.h>

#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "cursor.h"
#include "progress.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "password.h"
#include "usermenu.h"
#include "font.h"
#include "station.h"
#include "utils.h"
#include "guiutils.h"
#include "ImapGuiUtils.h"

#ifdef WIN32
// #include "tooltip.h"	// for MAINFRM.H
#include "mainfrm.h"
#endif

#include "eudora.h"
#include "ems-wglu.h"
#include "trnslate.h"
#include "toolmenu.h"
#include "sigmgr.h"
#include "persona.h"

#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"

#include "imapopt.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imaputil.h"
#include "imapdlg.h"
#include "imapmlst.h"
#include "imapthread.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


static CBitmap g_UnreadStatusBitmap;


// ========= GLobal externals =============/
extern QCMailboxDirector		g_theMailboxDirector;

//==================== Internals ==========================/
static const DWORD	dwLockTimeout = 100;			// Milliseconds.
static const time_t tLoginCancelInterval = 2;		// seconds;

// =================== Internal functions ====================

// ============ STATIC FUNCTIONS ======================/

void UpdateDelimiterChar (CIMAP *pImap, CImapAccount *pAccount);

// Folder management.

// ============================================================================//

// Class CImapAccount

IMPLEMENT_DYNAMIC(CImapAccount, CObject)

// Constructor
CImapAccount::CImapAccount()
{
	Init();
}



CImapAccount::CImapAccount(ACCOUNT_ID AccountID, LPCSTR pDirectory)
{
	// Call this first!!
	//
	Init();

	// Override these:
	//
	m_AccountID = AccountID;

	// Set this
	m_Directory = pDirectory;

	//reset this
	m_strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	ASSERT(!m_strPersona.IsEmpty());

}



// Init [PRIVATE]
//
void CImapAccount::Init ()
{
	m_AccountID = 0;
	m_Password.Empty();
	m_pParentList = NULL;

	// state
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;

	m_bIgnoreInbox		= TRUE;

	m_TopDelimiter		= 0;

	m_bGotNewMail	= FALSE;

	// Create the synchronization object.
	//
	m_pLockable = new CCriticalSection;

	// Initially unlocked.
	m_sAdvisoryLock = 0;

	// Exclusive lock.
	m_bExclusiveLock = FALSE;

	// Valid password??
	m_PasswordValidity = PWVALIDITY_UNKNOWN;

	// Set this so the first attempt to login will succeed!!
	//
	m_tLoginCancelled = 0x7FFFFFFF;

	m_pLoginLockable = new 	CCriticalSection;

	m_bPWNeedsSavingToDisk = FALSE;
}





//
// ResetInternalState [PRIVATE]
//
// Reset state use in fetching mailboxes.
//
void CImapAccount::ResetInternalState ()
{
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;
	m_bIgnoreInbox		= TRUE;
}



CImapAccount::~CImapAccount()
{
	if (m_pLockable)
		delete m_pLockable;

	delete m_pLoginLockable;

}




// LOCK management.

// 
// AdviseLock [PUBLIC]
//
// Attempt to set an advisory lock on 
BOOL CImapAccount::SetAdvisoryLock (AccountLockType sAccountLock)
{
	BOOL bResult = FALSE;

	// First, attempt to acquire the synchronization object.
	//
	// Try to lock the global list.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return FALSE;
	}

	// We can now set the advisory lock.
	//
	switch (sAccountLock)
	{
		case ACCOUNT_LOCK_EXCLUSIVE:
			// Can't get the lock if soneone else has it.
			//
			if (IsExclusiveLocked())
			{
				bResult = FALSE;
			}
			else
			{
				LockExclusively();

				bResult = TRUE;
			}

			break;

		case ACCOUNT_LOCK_DONT_DELETE:
			IncrementAdvisoryLock ();

			bResult = TRUE;
			break;

		default:
			ASSERT(0);

			break;
	}

	return bResult;
}




// 
// RemoveAdviseLock [PUBLIC]
//
// Attempt to set an advisory lock on 
BOOL CImapAccount::RemoveAdvisoryLock (AccountLockType sAccountLock)
{
	BOOL bResult = FALSE;

	// First, attempt to acquire the synchronization object.
	//
	// Try to lock the global list.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return FALSE;
	}

	// We can now set the advisory lock.
	//
	switch (sAccountLock)
	{
		case ACCOUNT_LOCK_EXCLUSIVE:
			// We must have had it. Note: We can possibly be stealing someone
			// else's lock!!!
			//
			if (!IsExclusiveLocked())
			{
				bResult = FALSE;
			}
			else
			{
				UnLockExclusively();

				bResult = TRUE;
			}

			break;

		case ACCOUNT_LOCK_DONT_DELETE:
			DecrementAdvisoryLock ();

			bResult = TRUE;
			break;

		default:
			ASSERT(0);

			break;
	}

	return bResult;
}


// IsExclusiveLocked [PUBLIC]
//
BOOL CImapAccount::IsExclusiveLocked()
{
	return m_bExclusiveLock;
}



// LockExclusively [PRIVATE]
//
// Make some validity checks!
//
void CImapAccount::LockExclusively()
{
	if (m_bExclusiveLock)
	{
		ASSERT (0);
	}

	m_bExclusiveLock = TRUE;
}



// UnLockExclusively [PRIVATE]
//
// Make some validity checks!
//
void CImapAccount::UnLockExclusively()
{
	if (!m_bExclusiveLock)
	{
		ASSERT (0);
	}

	m_bExclusiveLock = FALSE;
}


// IncrementAdvisoryLock [PRIVATE]
//
void CImapAccount::IncrementAdvisoryLock()
{
	m_sAdvisoryLock++;

	if (m_sAdvisoryLock <= 0)
	{
		ASSERT (0);
		m_sAdvisoryLock = 1;
	}
}


// DecrementAdvisoryLock [PRIVATE]
//
void CImapAccount::DecrementAdvisoryLock()
{	
	m_sAdvisoryLock--;

	if (m_sAdvisoryLock < 0)
	{
		ASSERT (0);

		m_sAdvisoryLock = 0;
	}
}

	

//===================================================================//



/////////////////////////////////
// InitializeDir [PUBLIC]	
// Create the directory if it doesn't already exist.
// 
BOOL CImapAccount::InitializeDir ()
{
	CString strDir;

	GetDirectory(strDir);

	if (strDir.IsEmpty())
		return FALSE;

	// If the directory exists, OK.
	if (DirectoryExists (strDir))
		return TRUE;
	else
	{
		// Doesn't exist. Try to create.
		if (!CreateDirectory((LPCSTR) strDir, NULL))
			return FALSE;
		else
			return TRUE;
	}
}



// FetchChildFolders [PUBLiC]
// Fetch the list of child mailboxes of the given mailbox name. The "mailbox" may be an account,
// a "namespace" or a mailbox.
// Return a "ImapMailboxNode" tree structure.
HRESULT CImapAccount::FetchChildMailboxList (LPCSTR pParentImapname, TCHAR Delimiter, ImapMailboxType Type, ImapMailboxNode **ppTopmostNode, BOOL Recurse)
{
	BOOL				IncludeInbox = FALSE;
	ImapMailboxNode		*pTopNode = NULL;
	HRESULT				hResult = E_FAIL;

	// Sanity
	if ( ! (pParentImapname && ppTopmostNode) )
		return E_INVALIDARG;

	// Make sure:
	//
	*ppTopmostNode = NULL;

	// Must have a server address.
	CString strAddress; GetAddress (strAddress);

	CIMAP Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return HRESULT_MAKE_CANCEL;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// Clear current state:
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;
	m_bIgnoreInbox		= TRUE;

	// If we are starting at the account level, include inbox.
	if (IMAP_ACCOUNT == Type)
	{
		IncludeInbox = TRUE;
		m_bIgnoreInbox = FALSE;
	}

	// Put up progress immediately:
	//
	CImmediateProgress Imp;

	// Put up progres dialog if in main thread.
	if ( ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format ( CRString(IDS_IMAP_FETCHING_MAILBOXES), strAddress );
		MainProgress(buf);
	}


	hResult = Imap.OpenControlStream(this);

	if ( SUCCEEDED(hResult) )
	{
		// If parent is an Account and there's no delimiter, go get it.
		if ((IMAP_ACCOUNT == Type) && (Delimiter == 0))
			UpdateDelimiterChar (&Imap, this);

		__FetchChildMailboxList (pParentImapname, Delimiter, &pTopNode, &Imap, 0, IncludeInbox,  Recurse);

		// Return what we got:
		//
		*ppTopmostNode = pTopNode;
	}

	// Reset these:
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;
	m_bIgnoreInbox		= TRUE;

	if ( ::IsMainThreadMT() )
	{
		CloseProgress();

		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}

	return hResult;
}



//
// ====== Methods that query name, login, etc, are routed to the personality.
// ====== Note: An account's m_AccountID is a hash of the personality name.
//

//
// Get the prefix from the INI file.
// Copy the result into the given string.
//
// NOTE:
// Don't use GetIniString here because it reads from [Settings] if
// there is no persona prefix.
// END NOTE

BOOL CImapAccount::GetPrefix(CString& strBuf)
{
	// ALways do this first.
	strBuf.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_PERSONA_IMAP_PREFIX );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// Note: The prefix can be empty.
	return TRUE;
}


//
// NOTES
// Copy the result into the given string.
// END NOTES
//
BOOL CImapAccount::GetName(CString& strBuf)
{
	// Find the personality corresponding to the account ID and copy the name.
	strBuf = g_Personalities.ImapGetByHash( m_AccountID );

	// An empty name means it's the dominant personality.
	if ( strBuf.IsEmpty() )
	{
		strBuf = CRString (IDS_DOMINANT);
	}

	return !strBuf.IsEmpty();
}



//
// Copy the result into the given string.
//

BOOL CImapAccount::GetLogin(CString& strBuf)
{
	// Must be called in the made thread.
	ASSERT ( IsMainThreadMT() );

	// ALways do this first.
	strBuf.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_LOGIN_NAME );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// Note: It's an error if no login name.
	return !strBuf.IsEmpty();
}



// MustSavePasswordToDisk [PUBLIC]
//
// Determine if this account's password must be saved to disk.
//
BOOL CImapAccount::MustSavePasswordToDisk ()
{
	// 
	//CString key = g_Personalities.GetIniKeyName( IDS_INI_SAVE_PASSWORD );

	// Get our personality name.
	//CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	//if ( strPersona.IsEmpty() )
	//	return FALSE;

	//Set to the persona to current
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_strPersona );

	BOOL bSavePass = GetIniShort(IDS_INI_SAVE_PASSWORD) > 0?TRUE:FALSE;

	g_Personalities.SetCurrent( CachePersonality );

	return bSavePass;

	//
	// Read from disk.
	//
	//return (g_Personalities.GetProfileInt (strPersona, key, 0) > 0);
}



//
// Copy the result into the given string.
//

BOOL CImapAccount::GetPasswordMT(CString& strBuf)
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock(0))
	{
		return FALSE;
	}

	// Just copy internal data member.
	strBuf = m_Password;

	return !strBuf.IsEmpty();
}



// 
// Return TRUE if the user has anything other than Password" clicked.
//
BOOL CImapAccount::IsAllowAuthenticate ()
{
	CString key1 = g_Personalities.GetIniKeyName( IDS_INI_AUTH_APOP );
	CString key2 = g_Personalities.GetIniKeyName( IDS_INI_AUTH_KERB );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Read from disk.
	// Return TRUE if CRAM-MD5 or Kerberos is selected.
	//
	return (g_Personalities.GetProfileInt (strPersona, key1, 0) > 0) ||
		   (g_Personalities.GetProfileInt (strPersona, key2, 0) > 0);
}


// 
// Return TRUE if the user has anything other than Password" clicked.
//
BOOL CImapAccount::IsAuthKerberos ()
{
//	ASSERT ( IsMainThreadMT() );

	CString key2 = g_Personalities.GetIniKeyName( IDS_INI_AUTH_KERB );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Read from disk.
	// Return TRUE if CRAM-MD5 or Kerberos is selected.
	//
	return (g_Personalities.GetProfileInt (strPersona, key2, 0) > 0);
}



// UseFancyTrash [PUBLIC]
//
// 	Return TRUE if to move to TRASH on delete.
//
BOOL CImapAccount::UseFancyTrash()
{
	CString key2 = g_Personalities.GetIniKeyName( IDS_INI_IMAP_XFERTOTRASH );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Read from disk.
	//
	return (g_Personalities.GetProfileInt (strPersona, key2, 0) > 0);
}




// GetTrashMailboxName [PUBLIC]
//
// Go read the persona's specified trash mailbox name.
// Note: This is the full IMAP name of the mailbox.
//
BOOL CImapAccount::GetTrashMailboxName (CString& strBuf)
{
	ASSERT ( IsMainThreadMT() );

	// ALways do this first.
	strBuf.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_IMAP_TRASH_MBOXNAME );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// If empty, see what's in the default.
	//
	if ( strBuf.IsEmpty() )
		strBuf = GetIniString (IDS_INI_IMAP_TRASH_MBOXNAME);


#if 0 // IN PROGRESS

	// If it's still empty, ask user to specify one.
	//

	CString strTemp ;
	strTemp.Format( "Account Settings for %s", strPersona) ;
	CModifyAcctSheet dlg( strTemp, this, 0, strName) ;
	dlg.Create();

#endif // 


	// Note: The prefix can be empty.
	return TRUE;
}





//
// Cache the account's password.
//
void CImapAccount::SetPasswordMT (LPCSTR pPassword)
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock(0))
	{
		return;
	}

	// Cache the password locally.
	m_Password = pPassword;

	//
	// Also, if this is the dominant personality, save the password in
	// POPPassword.

	// Find the personality corresponding to the account ID.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return;

	if (g_Personalities.IsDominant (strPersona) )
	{
		// Copy to the CString.
		POPPassword = pPassword;
	}
}




//
// Go read the persona's saved password.
//
BOOL CImapAccount::GetSavedPassword (CString& Password)
{
	// Must be called in the main thread!
	//
	ASSERT( IsMainThreadMT() );

	// Clear parm.
	Password.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_SAVE_PASSWORD_TEXT );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	Password = ::DecodePassword(szValue);

	return TRUE;
}



// 
// Save the cached password to disk, even if it's blank.
// NOTE: Can use this to clear the password as well.
//
BOOL CImapAccount::SavePasswordToDisk ()
{
	// Must be called in the main thread!
	//
	//ASSERT( IsMainThreadMT() );

	//CString key = g_Personalities.GetIniKeyName( IDS_INI_SAVE_PASSWORD_TEXT );
	CRString key(IDS_INI_SAVE_PASSWORD_TEXT);

	// Get our personality name.
	//CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	CString szPw = m_Password;

	// Note: we may want to save a blank password!
	if (!szPw.IsEmpty())
		szPw = EncodePassword(szPw);

	// Encode and write.
	g_Personalities.WriteProfileString( m_strPersona, key, szPw );

	return TRUE;
}	





//
// Return the server name or IP address.
// Copy the result into the given string.
//

BOOL CImapAccount::GetAddress(CString& strBuf)
{
	// ALways do this first.
	strBuf.Empty();


#if 0 // JOK

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	// Find the personality corresponding to the account ID.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Temporarily switch to this personality so that we can get the value.
	//
	if (g_Personalities.SetCurrent(strPersona))
	{
		 strBuf = GetIniString ( IDS_INI_PERSONA_POP_SERVER );

		// Strip any text after a colon (possible "address:port")
		int nc = strBuf.Find (':');
		if (nc >= 0)
			strBuf = strBuf.Left (nc);

		// Trim!!
		strBuf.TrimRight();
		strBuf.TrimLeft();
	}

	//
	// Reset current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);

	// Note: It's an error if no address.
	return !strBuf.IsEmpty();

#endif 


	CString key = g_Personalities.GetIniKeyName( IDS_INI_PERSONA_POP_SERVER );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Strip any text after a colon (possible "address:port")
	int nc = strBuf.Find (':');
	if (nc >= 0)
		strBuf = strBuf.Left (nc);

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// Note: It's an error if no address.
	return !strBuf.IsEmpty();
}



// FUNCTION
// Get the server address from the persona to see if it's of the
// form "address:port_num". Otherwiase, return the IMAP default.
// END FUNCTION

//
// Return the server name or IP address.
// Copy the result into the given string.
//

BOOL CImapAccount::GetPortNum(CString& strBuf)
{
	// This is the default.
	strBuf = CRString (IDS_IMAP_DEFAULT_PORT);


#if 0 // JOK

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	// Find the personality corresponding to the account ID.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Temporarily switch to this personality so that we can get the value.
	//
	if (g_Personalities.SetCurrent(strPersona))
	{
		 CString szServer = GetIniString ( IDS_INI_PERSONA_POP_SERVER );

		// Look for ":"
		int nc = szServer.Find (':');
		if (nc >= 0)
		{
			strBuf = szServer.Mid (nc + 1);

			// Trim!!
			strBuf.TrimRight();
			strBuf.TrimLeft();
		}
	}

	//
	// Reset current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);

	// Note: It's an error if no address.
	return !strBuf.IsEmpty();

#endif // JOK


	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	// Look for the IMAPPort INI setting first:
	//
	CString key = g_Personalities.GetIniKeyName( IDS_INI_IMAP_PORT );

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	CString szTmp = szValue;
	szTmp.TrimRight();
	szTmp.TrimLeft();

	// Did we get anything?
	if ( szTmp.IsEmpty() )
	{
		key = g_Personalities.GetIniKeyName( IDS_INI_PERSONA_POP_SERVER );

		*szValue = 0;

		g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

		// Copy the value to the output string.
		CString szServer = szValue;

		// Look for ":"
		int nc = szServer.Find (':');
		if (nc >= 0)
		{
			szTmp = szServer.Mid (nc + 1);

			szTmp.TrimRight();
			szTmp.TrimLeft();
		}
	}

	// Did we get?
	if ( !szTmp.IsEmpty() )
	{
		strBuf = szTmp;
	}

	// Note: It's an error if no portnum.
	return !strBuf.IsEmpty();
}




//
// Return the server name or IP address.
// Copy the result into the given string.
//

BOOL CImapAccount::GetDirectory(CString& strBuf)
{
	// Just copy internal data member.
	strBuf = m_Directory;

	return !strBuf.IsEmpty();
}




// ModifyFromPersonality [PUBLIC]
////////////////////////////////////////////////////////////////////////////////
// Called when a new personality is created and when an existing personality
// is modified.
// If login name or server address has changed, we may have to refresh the 
// mailbox list and the local cache. Ask user if to do that, then set output 
// parameter: "MboxListNeedsUpdating" to TRUE and return.

/////////////////////////////////////////////////////////////////////////////////

BOOL CImapAccount::ModifyFromPersonality (BOOL& MboxlistNeedsUpdating, BOOL IsNewAccount /* = FALSE */)
{
	// Initialize this for return.
	MboxlistNeedsUpdating = FALSE;
	
	// If this is a new account, just set internal values.
	if (IsNewAccount)
	{
		GetPrefix	(m_Prefix);
		GetLogin	(m_Login);
		GetAddress	(m_Address);
	}
	else
	{
		// we need to determine what has changed.
		CString szPrefix;	GetPrefix (szPrefix);
		CString szAddress;  GetAddress (szAddress);
		CString szLogin;	GetLogin (szLogin);

		// Has login changed?
		if (m_Login.Compare (szLogin) != 0)
		{
			MboxlistNeedsUpdating = TRUE;

			// Clear password.
			SetPasswordMT ("");
		}

		// Has prefix changed?
		if (m_Prefix.Compare (szPrefix) != 0)
		{
			MboxlistNeedsUpdating = TRUE;

			// We need to modify the ImapName in the account's command object,
			// because the prefix is the ImapName of an account object.
			//
			// Find the top level command object for this account.
			CString strDirectory; GetDirectory (strDirectory);

			QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( strDirectory );

			ASSERT (pCommand);

			if (pCommand)
			{
				ASSERT ( pCommand->IsKindOf ( RUNTIME_CLASS( QCImapMailboxCommand ) ) );

				ASSERT ( pCommand->GetType() == MBT_IMAP_ACCOUNT );

				(( QCImapMailboxCommand * ) pCommand)->SetImapName (szPrefix);
			}
		}

		// Has address changed?
		if (m_Address.Compare (szAddress) != 0)
		{
			MboxlistNeedsUpdating = TRUE;

			// Clear delimiter and password.
			SetPasswordMT ("");

			SetTopDelimiter ('\0');
		}

		// We must update in any case.
		m_Prefix	= szPrefix;
		m_Login		= szLogin;
		m_Address	= szAddress;

		// Ask user? Do this whether we have a connection to the current server
		// or not.
		if ( MboxlistNeedsUpdating )
		{
			// Clear the top level delimiter.

			SetTopDelimiter ('\0');
		}
	}

	return TRUE;
}




// __FetchChildFolders [PRIVATE]
// NOTES
// Do the hard work of the fetch.
// END NOTES

void CImapAccount::__FetchChildMailboxList (LPCSTR pParentImapname, TCHAR Delimiter, ImapMailboxNode **ppNode, CIMAP *pImap, short Level, BOOL IncludeInbox,  BOOL Recurse)
{
	ImapMailboxNode		*pTopNode = NULL;
	CString				Reference;

	// Sanity
	if (!(pParentImapname && ppNode && pImap))
		return;

	// Set ppNode into the current mailbox address so stuff gets aded to it.
	m_curMailboxList = ppNode;

	// Construct parent reference.
	Reference = pParentImapname;

	// If we have a delimiter, make sure the name ends in it.
	if (!Reference.IsEmpty() && Delimiter)
	{
		if (Reference.Right ( 1 ) != Delimiter)
		{
			Reference += Delimiter;
		}
	}

	// Set the m_Level to zero since this is the top of the new ImapMailboxNode tree.
	m_curLevel = Level;

	// Set current reference string.
	m_curReference = Reference;
		
	// Go fetch the names of the mailboxes. 
	pImap->MailFetchThisLevel (Reference, this, IncludeInbox);
	
	// Did we get any folders??
	if (*ppNode)
	{
		// Do we recurse?
		if (Recurse)
		{
			pTopNode = *ppNode;
			while (pTopNode)
			{
				// Recurse if not "NoInferiors". Also, MUST have a delimiter.
				if (pTopNode->Delimiter && (!pTopNode->NoInferiors))
				{
					// Set current parent mailbox.
					m_curParentMailbox = *ppNode;
					__FetchChildMailboxList (pTopNode->pImapName, pTopNode->Delimiter, &(pTopNode->ChildList), pImap, (short)(Level + 1), FALSE,  TRUE);

					// Reset our level
					m_curLevel = Level;
				}

				pTopNode = pTopNode->SiblingList;
			}
		}
	}
}
				



// Insert
// Insert a new mailbox into the mailbox tree of the account, at the
// correct level and in the correct order.
// Return the position (in the sibling list at that level) at which it is inserted, or -1.
// 
int CImapAccount::Insert(const char *mailbox, TCHAR delimiter, long attributes, ImapMailboxType Type)
{
	short i;
	ImapMailboxNode *pFolder = NULL, *pPrevFolder = NULL;
	CString		str;
	QCImapMailboxCommand *pExistingCommand = NULL;

	// Sanity
	if (!mailbox)
	{
		ASSERT (0);
		return -1;
	}

	// m_curMailboxList should NOT be NULL!!
	if (!m_curMailboxList)
	{
		ASSERT (0);
		return -1;
	}

	//
	// If the mailbox begins with the delimiter, remove it.
	//
	CString szMbox = mailbox;

#if 0 // Keep blanks in the mailbox name!! (JOK - 8/12/98).

	szMbox.TrimLeft ();
	szMbox.TrimRight ();

#endif // 

#if 0 //

	int nc = szMbox.Find (delimiter);
	if (nc == 0)
	{
		szMbox = szMbox.Mid (1);
	}


	// Remove any delimiter left.
	nc = szMbox.Find (delimiter);
	if (nc >= 0)
	{
		szMbox = szMbox.Left (nc);
	}

#endif //

	// Must have something left.
	if ( szMbox.IsEmpty () )
	{
		return FALSE;
	}

	// If the name is the same as the current reference, ignore it.
	if ( m_curReference.Compare (szMbox) == 0)
		return (-1);

	CRString Inbox (IDS_IMAP_RAW_INBOX_NAME);

	// Trap duplicate instances of INBOX.
	if ( Inbox.CompareNoCase (szMbox) == 0 )
	{
		// Should we ignore this or not?
		if (m_bIgnoreInbox)
			return -1;
		else
		{
			// Ignore all other inboxes
			m_bIgnoreInbox = TRUE;
		}
	}

	// Create a new ImapFolder structure and insert it into the list at this level.
	pFolder = NewImapMailboxNode();

	if (pFolder)
	{
		// Level and type.
		pFolder->level			= m_curLevel;
		pFolder->Type			= Type;
		pFolder->Delimiter		= delimiter;
		pFolder->AccountID		= m_AccountID;
		pFolder->pParentMailbox = m_curParentMailbox;

		// pImapName is a straight copy of mailbox. .
		pFolder->pImapName = new char[szMbox.GetLength () + 1];
		if (pFolder->pImapName)
		{
			strcpy (pFolder->pImapName, szMbox);
		}

		pFolder->NoInferiors	= (attributes & LATT_NOINFERIORS) > 0;
		pFolder->NoSelect		= (attributes & LATT_NOSELECT) > 0;
		pFolder->Marked			= (attributes & LATT_MARKED) > 0;
		pFolder->UnMarked		= (attributes & LATT_UNMARKED) > 0;

		// Check to see if we already have an IMAP mailbox by this name and
		// mark it as FOUND so that the lister will not attempt to create a new local directory
		// for it. 
		// Also, if it's found, set the directory name.
		pExistingCommand = g_theMailboxDirector.ImapFindByImapName ( GetAccountID(), szMbox, delimiter);

		// If this already exists locally, set the local directory.
		if (pExistingCommand)
		{
			CString Dirname;
			pExistingCommand->GetObjectDirectory (Dirname);

			pFolder->pDirname = CopyString ( (LPCSTR) Dirname);
			pFolder->bExists = TRUE;
		}

		// Make sure.						
		pFolder->SiblingList	= pFolder->ChildList = NULL;

		// pFolder->HasUnread   = ?? Won't know this until the folder is selected.
	}

	// Validity check.
	if (!pFolder)
	{
		return (-1);
	} 
	// MUST have ImapName!!
	else if (!(pFolder->pImapName))
	{
		delete pFolder;
		return (-1);
	}
	else
	{
		// Insert this in correct order.
		if (!*m_curMailboxList)
		{
			*m_curMailboxList = pFolder;
			i = 0;		// First position, zero based.
		}
		else if (strcmp ((*m_curMailboxList)->pImapName, pFolder->pImapName) > 0)
		{
			pFolder->SiblingList = *m_curMailboxList;
			*m_curMailboxList = pFolder;
			i = 0;		// First position, zero based.
		}
		else
		{
			pPrevFolder = *m_curMailboxList;
			i = 1;
			while (pPrevFolder->SiblingList)
			{
				if (strcmp (pPrevFolder->SiblingList->pImapName, pFolder->pImapName) > 0)
				{
					pFolder->SiblingList = pPrevFolder->SiblingList;
					pPrevFolder->SiblingList = pFolder;
					break;
				}
				else
				{
					pPrevFolder = pPrevFolder->SiblingList;
					i++;
				}
			}

			// Append to end of list?
			if (!pPrevFolder->SiblingList)
				pPrevFolder->SiblingList = pFolder;
		}

		return i;
	}
}



// GetTrashLocalMailboxPath [PUBLIC]
// FUNCTION
// Wade through the list of top-level mailboxes and look for a mailbox with
// terminal IMAP name of "Trash".
// If we have namespaces at the top level, look for the trash
// in the "#Personal" namespace.
// If "MakeSureExists" is TRUE, we have to attempt to create the mailbox if it
// doesn't exist, and make a local cached copy of it.
// END FUNCTION

// NOTES
// Return the path to the MBX file within the directory representing the mailbox.
// END NOTES.
BOOL CImapAccount::GetTrashLocalMailboxPath (CString& TrashMbxPath, BOOL MakeSureExists, BOOL bSilent /* = FALSE */)
{
	QCMailboxCommand		*pC;
	BOOL					bResult = FALSE;

	// Find the top level command object for this account.
	CString strDirectory; GetDirectory (strDirectory);

	pC = g_theMailboxDirector.FindByPathname( strDirectory );
	if (!pC)
	{
		ASSERT (0);
		return FALSE;
	}

	// This command must be of type MBT_IMAP_ACCOUNT!
	if (pC->GetType() != MBT_IMAP_ACCOUNT)
	{
		ASSERT(0);
		return FALSE;
	}

	// Verify the runtime class type as well!
	if (! ( ( CObject* ) pC ) ->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Wade through through the child list.
	CPtrList& ChildList = pC->GetChildList ();

	bResult = __GetTrashLocalMailboxPath (ChildList, TrashMbxPath);

	// If we haven't found one, attempt to create one at the top level.
	if (!bResult)
	{
		QCImapMailboxCommand *pNewCommand = CreateMailbox (CRString (IDS_IMAP_TRASH_BASENAME), (QCImapMailboxCommand *)pC, bSilent);
		if (pNewCommand)
		{
			// Make sure the mbx file actually exists.
			bResult = FileExistsMT (pNewCommand->GetPathname());
			if (bResult)
				TrashMbxPath = pNewCommand->GetPathname();
		}
	}

	return bResult;
}


// __GetTrashLocalMailboxPath [PRIVATE]
// FUNCTION
// Search pList for the mailbox. Recurse down through.
// Return the path to thye MBX file in "TrashMbxPath".
// END FUNCTION
BOOL CImapAccount::__GetTrashLocalMailboxPath (CPtrList& List, CString& TrashMbxPath)
{
	POSITION				pos;
	CString					Basename;
	QCImapMailboxCommand*	pImapCommand;
	TCHAR					Delimiter;
	int						i;
	CString					TrashBaseName;
	BOOL					bResult = FALSE;
	
	pos = List.GetHeadPosition();
	TrashBaseName = CRString (IDS_IMAP_TRASH_BASENAME);
	while( pos )
	{
		pImapCommand = ( QCImapMailboxCommand* ) List.GetNext( pos );
		if (pImapCommand)
		{
			if( pImapCommand->GetType() == MBT_IMAP_NAMESPACE )
			{
				// Not yet handled.
				return FALSE;
			}
			else if( pImapCommand->GetType() == MBT_IMAP_MAILBOX )
			{
				const CString& Pathname = pImapCommand->GetImapName();
				Delimiter = pImapCommand->GetDelimiter ();
				i = Pathname.ReverseFind (Delimiter);
				if (i > 0)
				{
					Basename = Pathname.Mid (i + 1);
					if (Basename.Compare (TrashBaseName) == 0)
					{
						TrashMbxPath = pImapCommand->GetPathname();
						return TRUE;
					}
				}
			}
		}
	}

	return bResult;
}



// CreateMailbox [PUBLIC]
// FUNCTION
// Create a new mailbox within this account, with pParentCommand as its
// parent. Create local storage and add it to the command and mbox trees.
// END 

QCImapMailboxCommand* CImapAccount::CreateMailbox ( LPCSTR pName, QCImapMailboxCommand *pParentCommand, BOOL bSilent /* = FALSE */, BOOL bMakeItAFolder /* = FALSE */)
{
	CString					NewImapName;
	TCHAR					ImapDelimiter;
	QCImapMailboxCommand	*pNewImapCommand = NULL;
	BOOL					bCreateSuccess = FALSE;
	BOOL					bProgressIsUp = FALSE;
	BOOL					bWasInProgress = FALSE;

	// Sanity.
	if (!(pName && pParentCommand))
		return NULL;

	// Use this internally.
	CString szNewBasename = pName;

	//
	// Trim the new name - can't create a mailbox with a blank name.
	//
	szNewBasename.TrimRight();
	szNewBasename.TrimLeft();

	// Don't allow a blank name, or a name with just the delimiter.
	if ( szNewBasename.IsEmpty() )
	{
		ErrorDialog (IDS_IMAP_INVALID_MBOXNAME, pName);
		return NULL;
	}

	//
	// Put up progress bar.
	//
	if (!bSilent && ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		CString buf;
		buf.Format ( CRString(IDS_IMAP_CREATING_MBOX), szNewBasename);
		MainProgress(buf);

		bProgressIsUp = TRUE;
	}

	//
	// Get address for CIMAP and progress bar.
	//
	CString strAddress;
	GetAddress (strAddress);

	// Attempt to open a new control stream!!
	//
	CIMAP Imap (strAddress);

	// Initialize:
	// Open a new control stream to the server.
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		// close the progress dialog before returning -jdboyd
		if ( bProgressIsUp )
		{
			if (bWasInProgress)
				PopProgress ();
			else
				CloseProgress ();
                
			bProgressIsUp = FALSE;
		}

		return NULL;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	if ( !SUCCEEDED (Imap.OpenControlStream(this)) )
	{
		if ( bProgressIsUp )
		{
			if (bWasInProgress)
				PopProgress ();
			else
				CloseProgress ();

			bProgressIsUp = FALSE;
		}

		return NULL;
	}

	// Format the full IMAP name of the mailbox. The parent's name becomes a prefix.
	NewImapName		= pParentCommand->GetImapName();
	ImapDelimiter	= pParentCommand->GetDelimiter();

	// If ImapDelimiter is 0 and parent is an account, go update delimiter.
	if ( (ImapDelimiter == 0) && (pParentCommand->GetType () == MBT_IMAP_ACCOUNT) )
	{
		UpdateDelimiterChar (&Imap, this);

		// Did we get one?
		ImapDelimiter	= pParentCommand->GetDelimiter();
	}

	// 
	// Make a check here for a name with just the delimiter char.
	//
	if ( ImapDelimiter && (szNewBasename.GetLength() == 1) && 
			NewImapName.Right ( 1 ) == ImapDelimiter)
	{
		ErrorDialog (IDS_IMAP_INVALID_MBOXNAME, pName);

		// close the progress dialog before returning -JDB
		if ( bProgressIsUp )
		{
			if (bWasInProgress)
				PopProgress ();
			else
				CloseProgress ();
                
			bProgressIsUp = FALSE;
		}

		return NULL;
	}


	if (!NewImapName.IsEmpty() && ImapDelimiter)
	{
		if (NewImapName.Right ( 1 ) != ImapDelimiter)
			NewImapName += ImapDelimiter;
	}

	// Note: The desired IMAP name must have already been verified!!
	NewImapName += szNewBasename;

	// Must have a non-empty name now.
	if (NewImapName.IsEmpty())
	{
		ASSERT (0);

		// close the progress dialog before returning -jdboyd
		if ( bProgressIsUp )
		{
			if (bWasInProgress)
				PopProgress ();
			else
				CloseProgress ();
                
			bProgressIsUp = FALSE;
		}

		return NULL;
	}

	//
	// If "bMakeItAFolder is TRUE, append delimiter to this new name.
	//
	if (ImapDelimiter && bMakeItAFolder)
	{
		if (NewImapName.Right ( 1 ) != ImapDelimiter)
			NewImapName += ImapDelimiter;
	}	

	bCreateSuccess = SUCCEEDED (Imap.CreateMailbox (NewImapName));

	if (bCreateSuccess)
	{
		// Get attributes of the new mailbox so we know if it's really
		// been created.

		ResetInternalState ();

		// If "NewImapName" had a trailing delimiter (i.e. we did a "Make it a folder"),
		// that's not part of the mailbox name!! Take it back off.
		//
		if (NewImapName.Right (1) == ImapDelimiter)
			NewImapName = NewImapName.Left (NewImapName.GetLength() - 1);

		ImapMailboxNode *pImapNode = Imap.FetchMailboxAttributes (NewImapName);
		if (pImapNode)
		{
			// We succeeded. Subscribe to it:
			//
			Imap.SubscribeMailbox (NewImapName);

			CString	 Path, DirName;

			// We know it's on the server. Make a last ditch effort to make sure that
			// we really don't already have the mailbox in the mboxtree list.

			pNewImapCommand = g_theMailboxDirector.ImapFindByImapName ( GetAccountID(), 
											NewImapName, pImapNode->Delimiter);
			if (pNewImapCommand)
			{
				ASSERT (0);

				if ( bProgressIsUp )
				{
					if (bWasInProgress)
						PopProgress ();
					else
						CloseProgress ();

					bProgressIsUp = FALSE;
				}

				return NULL;
			}

			pParentCommand->GetObjectDirectory(Path);

			// Get a unique directory name and create it NOW!.We will create the
			// files within the directory later.
			if ( MakeSuitableMailboxName (Path, NULL, pImapNode, DirName, 64) )
			{
				// Format a path to DirName and copy it to pCurNode->pPathname.
				Path += DirectoryDelimiter + DirName;

				// Set it into the node.
				if (pImapNode->pDirname)
					delete[] (pImapNode->pDirname);
				pImapNode->pDirname	= CopyString (Path);

				// Make sure the directory exists or that we can create it.
				if (!DirectoryExists (pImapNode->pDirname))
					CreateDirectory (pImapNode->pDirname, NULL);

				// Get an MBX file path.
				GetMbxFilePath (pImapNode->pDirname, Path);

				// Create a new command object and return it.

				pNewImapCommand = new QCImapMailboxCommand ( &g_theMailboxDirector, szNewBasename, 
								pImapNode->pImapName, (LPCSTR)Path, 
								pParentCommand->ImapTypeToMBTType (pImapNode->Type),
								US_UNKNOWN,							
								GetAccountID(),
								pImapNode->NoSelect,
								pImapNode->Marked, 
								pImapNode->UnMarked,
								pImapNode->NoInferiors ); 

				if (pNewImapCommand)
				{
					pNewImapCommand->SetDelimiter (pImapNode->Delimiter);

				}
			}
			else
			{
				bCreateSuccess = FALSE;
			}

			// No longer need pImapNode.
			DeleteMailboxNode (pImapNode);
			pImapNode = NULL;
		}
		else
		{
			// We didn't really succeed.
			bCreateSuccess = FALSE;
		}
	}

	//
	// Close progress here.
	//
	if ( bProgressIsUp )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress ();

		bProgressIsUp = FALSE;
	}

	// Put up error if we failed.
	if (!bCreateSuccess)
	{
		if (!bSilent)
		{
			// Display error messages if we can.
			TCHAR  buf [512];
			int iType;
			buf[0] = '\0';

			Imap.GetLastImapErrorAndType (buf, 510, &iType);

			if ( buf[0] )
			{
				ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_SRVMSG, buf);
			}
			else
			{
				ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
			}
		}
	}

	return pNewImapCommand;
}



// DeleteMailbox [PUBLIC]
// FUNCTION
// Delete a mailbox on the server. "pImapName" should be the full IMAP name of the mailbox
// in this account.
// END 

BOOL CImapAccount::DeleteMailbox ( LPCSTR pImapName, TCHAR Delimiter )
{
	BOOL bResult = FALSE;
	BOOL bProgressIsUp = FALSE;

	// Sanity.
	if (!pImapName)
		return FALSE;

	//
	// Put up progress bar.
	//
	if ( ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format ( CRString(IDS_IMAP_DELETING_MBOX), pImapName);
		MainProgress(buf);

		bProgressIsUp = TRUE;
	}

	// Open a new control stream to the server.
	// BUG: Each account should have an open control stream that can be re-used!!
	CString strAddress;  GetAddress (strAddress);
	CIMAP Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		// close the progress dialog before returning -jdboyd
		if ( bProgressIsUp )
		{
			CloseProgress ();
			bProgressIsUp = FALSE;
		}

		return FALSE;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	if ( SUCCEEDED(Imap.OpenControlStream(this)) )
	{
		bResult = SUCCEEDED(Imap.DeleteMailbox (pImapName));

		if (!bResult)
		{
			QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											pImapName, Delimiter);
			if (pImapCommand)
			{
				TCHAR  buf [512];
				int iType;
				buf[0] = '\0';

				Imap.GetLastImapErrorAndType (buf, 510, &iType);

				if ( buf[0] )
				{
					ErrorDialog (IDS_ERR_IMAP_SERVER_MBOX_DELETE, pImapCommand->GetName(), buf);
				}
				else
				{
					ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
				}
			}
			else
				ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
		}
		else 
		{
			// We succeeded. Send UnSubscribe:
			//
			Imap.UnSubscribeMailbox (pImapName);
		}

	}

	if ( bProgressIsUp )
	{
		CloseProgress ();
		bProgressIsUp = FALSE;

		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}

	return bResult;
}




// RenameMailbox [PUBLIC]
// FUNCTION
// Rename an IMAP mailbox.
// END 

// NOTES
// NOTE: "pNewImapName" is just the base name. Get the superior
// parts of the name from pImapCommand.
//
// NOTE: Only a "simple" rename is permitted here. That is, the name cannot contain
// the mailbox hierachy character.
//
// Important note!: It is possible for the rename to result in the creation of a new
// mailbox and the retention of the old one, albeit with modification of it's status.
// In that case, the output parameter "bOldMboxStillExists" is TRUE and "ppNewImapMailbox"
// will, on output, contain a pointer to the new (allocated) command object that must
// be linked into the UI structures.
//
// NOTE: (2/9/98, JOK) We now handle progress mesages only if the progress bar was already up.
//
// END NOTES
//
BOOL CImapAccount::RenameMailbox ( QCImapMailboxCommand *pImapCommand, LPCSTR pNewImapName, CString& szNewDirName, BOOL& bOldMboxStillExists, QCImapMailboxCommand **ppNewImapMailbox)
{
	BOOL	bResult = FALSE;
	TCHAR	ImapDelimiter = 0;

	// Do these. They are output parameters.
	szNewDirName.Empty();
	bOldMboxStillExists = TRUE;

	// Sanity.
	if (! (pImapCommand && pNewImapName && ppNewImapMailbox) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Open a new control stream to the server.
	// BUG: Each account should have an open control stream that can be re-used!!
	CString strAddress;  GetAddress (strAddress);
	CIMAP Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return FALSE;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	//
	// Open it..
	//
	if ( !SUCCEEDED(Imap.OpenControlStream(this)) )
	{

		return FALSE;
	}

	// Get hierarchical delimiter of the account itself if we haven't yet.
	//
	TCHAR TopDelimiter = GetTopDelimiter ();

	if (!TopDelimiter)
		UpdateDelimiterChar (&Imap, this);

	// Default this to TopDelimiter.
	//
	ImapDelimiter = TopDelimiter;

	// Get old IMAP name.
	CString szOldImapName = pImapCommand->GetImapName();
	CString szNewFullImapName;

	//
	// If we are trying to rename INBOX, prepend the account's location
	// prefix.
	//
	if (szOldImapName.CompareNoCase ( CRString (IDS_IMAP_RAW_INBOX_NAME) ) == 0)
	{
		// The default.
		szNewFullImapName = pNewImapName;

		// For INBOX, the delimiter is the account's delimiter.
		//
		ImapDelimiter = TopDelimiter;

		// Prepend account's location prefix.
		CString prefix; GetPrefix (prefix);

		if (!prefix.IsEmpty())
		{
			// Make sure ends in delimiter.
			if (ImapDelimiter && prefix.Right ( 1 ) != ImapDelimiter)
				prefix += ImapDelimiter;

			szNewFullImapName = prefix + pNewImapName;
		}
	}
	else
	{
		//
		// Format new full IMAP hierarchical name.
		//
		ImapDelimiter = pImapCommand->GetDelimiter();

		if (ImapDelimiter == '\0')
		{
			// No hierarchy. Use basename as is.
			szNewFullImapName = pNewImapName;
		}
		else
		{
			szNewFullImapName = szOldImapName;

			int nc = szNewFullImapName.ReverseFind (ImapDelimiter);

			if (nc < 0)
			{
				// Top level. Use as is.
				szNewFullImapName = pNewImapName;
			}
			else
			{
				// Replace basename.
				szNewFullImapName = szNewFullImapName.Left (nc + 1);
				szNewFullImapName += pNewImapName;
			}
		}
	}


	//
	// Chenge up progress message if bar is up.
	//
	if ( InProgress && ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format ( CRString(IDS_IMAP_RENAMING_MBOX), szOldImapName, szNewFullImapName);
		MainProgress(buf);
	}

	//
	// Note: we're NOT looping - just a way to easily get out.
	//
	while (1)
	{
		// Ask the server to rename.

		bResult = SUCCEEDED(Imap.RenameMailbox (szOldImapName, szNewFullImapName));

		// If we didn't succeed, see what the server had to say.
		if (!bResult)
			break;

		//
		// Seems like we succeeded. Make sure we did!!
		// Don't trust the result from. Go make sure that the old mailbox no longer exists
		// AND that the new one exists.
		//
		if (bResult)
		{
			bResult = FALSE;

			// Set this to FALSE unless we see that the old mailbox is still there.
			bOldMboxStillExists = FALSE;

			ResetInternalState ();

			ImapMailboxNode *pImapNode = Imap.FetchMailboxAttributes (szOldImapName);
			if (pImapNode)
			{
				// Still there! No fear. Some servers (Cyrus) may actually create another
				// mailbox and retain the old one, but now with the (\noselect} flag.
				// That will be handled below, but here, we retain the old mailbox
				// and just update it's attributes.

				pImapCommand->SetNoInferiors (pImapNode->NoInferiors);
				pImapCommand->SetNoSelect	 (pImapNode->NoSelect);
				pImapCommand->SetMarked		 (pImapNode->Marked);
				pImapCommand->SetUnmarked	 (pImapNode->UnMarked);

				DeleteMailboxNode (pImapNode);
				pImapNode = NULL;

				// Don't quit. Flag that the old mailbox still exists and fall through.
				bOldMboxStillExists = TRUE;
			}

			//
			// Ok Old mailbox may or may not still exist. In any case, look for new one.
			//

			ResetInternalState ();

			// If "szNewFullImapName" had a trailing delimiter.
			// that's not part of the mailbox name!! Take it back off.
			//
			if (szNewFullImapName.Right (1) == ImapDelimiter)
				szNewFullImapName = szNewFullImapName.Left (szNewFullImapName.GetLength() - 1);

			// 
			pImapNode = Imap.FetchMailboxAttributes (szNewFullImapName);

			if (!pImapNode)
			{
				// We failed. Grievous error.
				bResult = FALSE;
				break;
			}
			else
			{
				// Ok. We actually succeeded. If the old mailbox still exists, this is 
				// a new mailbox. Go create our local representation of it.

				if (bOldMboxStillExists)
				{
					// This is a new mailbox.  
					// We return the pathname housing the newly created mailbox in szNewDirName.
					QCImapMailboxCommand *pNewCommand = CreateChildCommandFromNode (
										(QCImapMailboxCommand *) g_theMailboxDirector.ImapFindParent (pImapCommand),
										pImapNode,
										pNewImapName,
										szNewDirName);

					if (pNewCommand)
					{
						// Copy to output parameter.
						*ppNewImapMailbox = pNewCommand;

						bResult = TRUE;
					}
					else
					{
						// Couldn't create new mailbox.
						bResult = FALSE;
					}
				}
				else
				{
					// We actually changed the name of the mailbox on the server.
					// 	Go rename the local directory and it's contents.
					// Return the new mailbox name in szNewDirName.

					CString szOldMboxDir;
					pImapCommand->GetObjectDirectory (szOldMboxDir);

					bResult = RenameMboxDirAndContents (szOldMboxDir, pImapNode, szNewDirName);
				}

				// Be sure to free it.
				DeleteMailboxNode (pImapNode);
				pImapNode = NULL;
			}
		}

		// Don't loop!!
		break;
	}


	//
	// Don't close progress bar.
	//
	if ( ::IsMainThreadMT() )
	{
		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}


	// If we didn't succeed, see what the server had to say.
	if (!bResult)
	{
		TCHAR  buf [512];
		int iType;
		buf[0] = '\0';

		Imap.GetLastImapErrorAndType (buf, 510, &iType);

		if ( buf[0] )
		{
			ErrorDialog (IDS_ERR_IMAP_SERVER_MBOX_RENAME, szOldImapName, szNewFullImapName, buf);
		}
		else
		{
			ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
		}
	}

	return bResult;
}





// MoveMailbox [PUBLIC]
// FUNCTION
// Rename an IMAP mailbox, but now, we know the full path of the destination
// mailbox.
// END 

// NOTES
// NOTE: (2/9/98, JOK) We now handle progress mesages only if the progress bar was already up.

// END NOTES
//
BOOL CImapAccount::MoveMailbox ( QCImapMailboxCommand *pImapCommand, QCImapMailboxCommand *pNewParent,
								  BOOL& bOldMboxStillExists,
								  QCImapMailboxCommand **ppNewLastParent,
								  QCImapMailboxCommand **ppNewImapMailbox)
{
	BOOL	bResult = FALSE;
	TCHAR	ImapDelimiter = 0;
	BOOL	bHasChildMboxes = FALSE;

	// Sanity.
	if (! (pImapCommand && pNewParent && ppNewLastParent && ppNewImapMailbox) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Initialize these.
	//
	*ppNewLastParent = NULL;
	*ppNewImapMailbox = NULL;

	// Flag this for use below:
	bHasChildMboxes = !pImapCommand->GetChildList().IsEmpty();


	// Open a new control stream to the server.
	// BUG: Each account should have an open control stream that can be re-used!!
	CString strAddress;  GetAddress (strAddress);
	CIMAP Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return FALSE;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	//
	// Open it..
	//
	if ( !SUCCEEDED(Imap.OpenControlStream(this)) )
	{

		return FALSE;
	}

	// Get hierarchical delimiter of the account itself if we haven't yet.
	//
	TCHAR TopDelimiter = GetTopDelimiter ();

	if (!TopDelimiter)
		UpdateDelimiterChar (&Imap, this);

	//
	// Get the hierarchy delimiter.
	//
	ImapDelimiter = pImapCommand->GetDelimiter();

	if (ImapDelimiter == '\0')
	{
		ImapDelimiter = TopDelimiter;
	}

	// Get basename of the mailbox we're moving.
	//
	CString szOldImapName = pImapCommand->GetImapName();
	CString szBaseName = szOldImapName;

	if (ImapDelimiter != '\0')
	{
		int nc = szBaseName.ReverseFind (ImapDelimiter);

		if (nc >= 0)
		{
			szBaseName = szBaseName.Mid (nc + 1);
		}
	}

	// Format new full IMAP name.
	//
	CString szNewFullImapName = pNewParent->GetImapName();

	szNewFullImapName.TrimRight();

	if ( ImapDelimiter && !szNewFullImapName.IsEmpty() &&
			(szNewFullImapName.Right(1) != ImapDelimiter) )
	{
		szNewFullImapName += ImapDelimiter;
	}

	// Add Basename:
	//
	szNewFullImapName += szBaseName;

	//
	// Put up progress bar.
	//
	if ( InProgress &&  ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format ( CRString(IDS_IMAP_RENAMING_MBOX), szOldImapName, szNewFullImapName);
		MainProgress(buf);
	}

	//
	// Note: we're NOT looping - just a way to easily get out.
	//
	while (1)
	{
		ImapMailboxNode *pImapNode = NULL;

		// Ask the server to rename.

		bResult = SUCCEEDED(Imap.RenameMailbox (szOldImapName, szNewFullImapName));

		// If we didn't succeed, see what the server had to say.
		if (!bResult)
		{
			break;
		}

		//
		// Seems like we succeeded. Make sure we did!!
		// Don't trust the result from IMAP. Go make sure that the old mailbox no longer exists
		// AND that the new one exists.
		//
		bResult = FALSE;

		// Set this to FALSE unless we see that the old mailbox is still there.
		bOldMboxStillExists = FALSE;

		ResetInternalState ();

		pImapNode = Imap.FetchMailboxAttributes (szOldImapName);

		if (pImapNode)
		{
			// Still there! No fear. Some servers (Cyrus) may actually create another
			// mailbox and retain the old one, but now with the (\noselect} flag.
			// That will be handled below, but here, we retain the old mailbox
			// and just update it's attributes.

			pImapCommand->SetNoInferiors (pImapNode->NoInferiors);
			pImapCommand->SetNoSelect	 (pImapNode->NoSelect);
			pImapCommand->SetMarked		 (pImapNode->Marked);
			pImapCommand->SetUnmarked	 (pImapNode->UnMarked);

			DeleteMailboxNode (pImapNode);
			pImapNode = NULL;

			// Don't quit. Flag that the old mailbox still exists and fall through.
			bOldMboxStillExists = TRUE;
		}
		else
		{
			// If old mailbox no longer exists, caller needs to refresh subtree.
			// Find the highest parent mailbox that still exists and return it as an output 
			// parameter.
			//

			ImapMailboxNode *pParentImapNode = NULL;
	
			CString szStr = szOldImapName;
			int nc = szStr.ReverseFind (ImapDelimiter);

			while (nc > 0)
			{
				szStr = szStr.Left (nc);

				ResetInternalState ();

				pParentImapNode = Imap.FetchMailboxAttributes (szStr);

				if (pParentImapNode)
					break;
				else
					nc = szStr.ReverseFind (ImapDelimiter);
			}

			// If we got a node, find it's local command object and return that in 
			// ppNewLastParent.

			if (pParentImapNode)
			{
				if (pParentImapNode->pImapName)
				{
					*ppNewLastParent = g_theMailboxDirector.ImapFindByImapName(
										GetAccountID(), 
										pParentImapNode->pImapName,
										ImapDelimiter);
				}
				
				DeleteMailboxNode (pParentImapNode);
				pParentImapNode = NULL;
			}
		} // Old mbox no longer exists.


		//
		// Ok Old mailbox may or may not still exist. In any case, look for new one.
		//

		ResetInternalState ();

		// If "szNewFullImapName" had a trailing delimiter.
		// that's not part of the mailbox name!! Take it back off.
		//
		if (szNewFullImapName.Right (1) == ImapDelimiter)
			szNewFullImapName = szNewFullImapName.Left (szNewFullImapName.GetLength() - 1);

		// 
		pImapNode = Imap.FetchMailboxAttributes (szNewFullImapName);

		if (!pImapNode)
		{
			// We failed. Grievous error. However, return TRUE so caller will clean things
			// up.

			bResult = TRUE;
			break;
		}
		else
		{
			// Well, we created the new one. Create a command object and associated local
			// storage for the new mailbox and return it. The caller should refresh
			// it's subtree if it had child mailboxes.
			//
			bResult = TRUE;

			CString szNewDirName;

			QCImapMailboxCommand *pNewCommand = CreateChildCommandFromNode (
										pNewParent, 
										pImapNode,
										szBaseName,
										szNewDirName);


			if (pNewCommand)
			{
				// Copy to output parameter.
				*ppNewImapMailbox = pNewCommand;
			}

			// Be sure to free it.
			DeleteMailboxNode (pImapNode);
			pImapNode = NULL;
		}

		// Don't loop!!
		break;
	}

	//
	// Don't close progress bar.
	//
	if ( ::IsMainThreadMT() )
	{

		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}


	// If we didn't succeed, see what the server had to say.
	if (!bResult)
	{
		TCHAR  buf [512];
		int iType;
		buf[0] = '\0';

		Imap.GetLastImapErrorAndType (buf, 510, &iType);

		if ( buf[0] )
		{
			ErrorDialog (IDS_ERR_IMAP_SERVER_MBOX_RENAME, szOldImapName, szNewFullImapName, buf);
		}
		else
		{
			ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
		}
	}

	return bResult;
}





//
// Checking mail for "inbox"
// NOTE: Do this only if inbox is OPEN.
//
void CImapAccount::CheckMail ()
{
	// Always reset the first.
	// NOTE: The mailbox itself (must be INBOX) will alert us if
	// it got new mail!!.
	//
	SetGotNewMail (FALSE);

	// Make sure we have a command object for inbox.
	QCImapMailboxCommand*	pImapCommand = g_theMailboxDirector.ImapFindByImapName(
												GetAccountID (),
												"Inbox",
												'\0');
	if (pImapCommand)
	{
		// Get the toc even if it's not already loaded.
		CTocDoc* pTocDoc = GetToc( pImapCommand->GetPathname(), pImapCommand->GetName());

		if (pTocDoc && pTocDoc->m_pImapMailbox)
		{
			// Get the number of summaries before update.
			unsigned long nOldCount = pTocDoc->NumSums();

			BOOL bUpdateSucceeded = SUCCEEDED (pTocDoc->m_pImapMailbox->CheckNewMail (pTocDoc) );
		}
	}
}



// ===  Methods to handle multithreaded IMAP operations =====//

BOOL CImapAccount::FetchNewMessages (CImapMailbox *pImapMailbox, CPtrList& UidList)
{
	if (!pImapMailbox)
		return FALSE;

	// Create a new message thread object.
	CImapMessageThread  *pMessageThread = new CImapMessageThread ();

	if (!pMessageThread)
		return FALSE;

	// Tell it our account ID so it can talk back to us.
	pMessageThread->SetAccountID (GetAccountID());

	// Get the thread to copy our uid list.
	pMessageThread->CopyUidList (UidList);


	// Start the thread and return.
	// NOTE: The thread object copies stuff from pImapMailbox.
	return pMessageThread->FetchNewMessages(pImapMailbox);
}





/////////////////////////////////////////////////////////////////////////////////////
// CreateChildCommandFromNode [private]
//
// FUNCTION
// Given an Imap command object representing a mailbox, and an ImapNode structure 
// representing a mailbox that resides on the server,
// END FUNCTION
/////////////////////////////////////////////////////////////////////////////////////
QCImapMailboxCommand* CImapAccount::CreateChildCommandFromNode (QCImapMailboxCommand *pParentCommand, ImapMailboxNode *pImapNode, LPCSTR pNewName, CString szNewDirname)
{
	QCImapMailboxCommand *pNewImapCommand = NULL;

	// Sanity.
	if (! (pParentCommand && pNewName && pImapNode) )
		return NULL;

	//
	// Get the parent directory so we can attempt to create local storage for the mailbox.
	//
	CString szPath;

	pParentCommand->GetObjectDirectory (szPath);

	//
	// Create local storage.
	//
	BOOL bResult = CreateDirectoryForMailbox (szPath, pImapNode, szNewDirname);

	// If we succeeded, create the command object.
	if (bResult)
	{
		// Get the MBX file path.
		szPath +=  DirectoryDelimiter + szNewDirname;

		CString szMbxPath;
		GetMbxFilePath (szPath, szMbxPath);

		// Create a new command object and return it.

		pNewImapCommand = new QCImapMailboxCommand ( &g_theMailboxDirector,
							    pNewName, 
								pImapNode->pImapName, (LPCSTR)szMbxPath, 
								MBT_IMAP_MAILBOX,
								US_UNKNOWN,							
								GetAccountID(),
								pImapNode->NoSelect,
								pImapNode->Marked, 
								pImapNode->UnMarked,
								pImapNode->NoInferiors ); 

		if (pNewImapCommand)
 			pNewImapCommand->SetDelimiter (pImapNode->Delimiter);


	}

	return pNewImapCommand;
}




/////////////////////////////////////////////////////////////////////////
// FetchMailboxAttributes
//
// FUNCTION
// This is a wrapper around CIMAP::FetchMailboxAttributes ().
// Useful for seeing if a remote mailbox exists.
// END FUNCTION

// NOTES
// If returns an ImapMailboxNode that must be freed by the caller!!
// END NOTES
/////////////////////////////////////////////////////////////////////////
ImapMailboxNode* CImapAccount::FetchMailboxAttributes (LPCSTR pImapName, BOOL bSilent /* = TRUE */)
{
	ImapMailboxNode *pImapNode = NULL;

	// Sanity:
	if (!pImapName)
	{
		ASSERT (0);
		return NULL;
	}

	// Open a new control stream to the server.
	CString strAddress;
	GetAddress (strAddress);

	CIMAP Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return NULL;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	BOOL bWePutUpProgress = FALSE;

	// Put up progres dialog if in main thread.
	if (!bSilent && ::IsMainThreadMT() )
	{
		bWePutUpProgress = TRUE;

		CString buf;
		buf.Format ( CRString(IDS_IMAP_CONNECTING_TO), strAddress );
		MainProgress(buf);
	}

	// Open a new control stream!!
	if (SUCCEEDED(Imap.OpenControlStream(this)))
	{
		// We can't stop the operation after this.
		if (bWePutUpProgress)
		{
			CloseProgress();

			// But, put up the wait cursor.
			CWaitCursor waitCursor;
		}

		// Reset CImapAccount internal state before a fetch.
		ResetInternalState ();


		// Get attributes now.
		pImapNode = Imap.FetchMailboxAttributes (pImapName);
	}
	else
	{
		if (bWePutUpProgress)
			CloseProgress();
	}

	return pImapNode;
}



// Login [PUBLIC]
//
// This doesn't really log into the IMAP server. It just gets a password,
// either a store one or from the user.
// Return FALSE if the user cancelled, TRUE if we got any password whatsoever.
// 
BOOL CImapAccount::Login(CString& szLogin, CString& szPassword)
{
	CString Login, Password;

	// MUST be called in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// If the user just cancelled a previous attempt, ignore this:
	//
	if ( UserJustCancelledLogin() )
	{
		return FALSE;
	}

	szPassword.Empty();

	// Initialize
	Login.Empty();
	Password.Empty();

	// Get the account's login name.
	//
	GetLogin(Login);

	// If "Kerberos" is selected for this account, don't prompt for a password.
	//
	if ( IsAuthKerberos () )
	{
		szLogin = Login;

		return TRUE;
	}

	// If we've got a non-empty (cached) password and it's flagged as valid, return that.
	//
	GetPasswordMT (Password);

	if ( !Password.IsEmpty() )
	{
//		if (m_PasswordValidity == PWVALIDITY_IS_VALID || m_PasswordValidity == PWVALIDITY_MAYBE_VALID)
		{
			// Copy to out parms. These may be empty.
			szLogin		= Login;
			szPassword	= Password;

			return TRUE;
		}
	}
	else //if ( MustSavePasswordToDisk() )
	{
		// Get saved password.
		GetSavedPassword (Password);
	}


	BOOL bGotNewLogin = FALSE;

	// Do we have to put up dialog box??
	if (Password.IsEmpty())
	{
		CString szAddress; GetAddress(szAddress);
		CString szPrompt;
		szPrompt.Format ("%s@%s", Login, szAddress);

		BOOL GOTLOGIN = ::GetPassword(Password, 0, FALSE, szPrompt);

		// User cancelled?.
		if (!GOTLOGIN)
		{
			// User cancelled. Make sure these are empty.
			Login.Empty();
			Password.Empty();

			// Clear out parms.
			szLogin.Empty();
			szPassword.Empty();

			// Flag this!
			SetUserJustCancelledLogin();
		}
		else
		{
			bGotNewLogin = TRUE;
		}
	}

	// Copy to out parms. These may be empty.
	szLogin		= Login;
	szPassword	= Password;

	// Cache the password, but we won't know if it's valid until user attempt to log in.
	//
	SetPasswordMT ((LPCSTR)szPassword);
	
	if( bGotNewLogin && MustSavePasswordToDisk())
		SavePasswordToDisk();
	
	m_PasswordValidity == PWVALIDITY_MAYBE_VALID;

	return !szPassword.IsEmpty();
}





// UserJustCancelled [PRIVATE]
// Note:
// Always use UserJustCancelled() in other methods. Don't try
// to reference m_tConnectionCancelled any place else.
//
BOOL CImapAccount::UserJustCancelledLogin ()
{
	time_t now = time (0);

	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock(0))
	{
		return FALSE;
	}


	// Allow login on startup:
	//
	if (m_tLoginCancelled > now)
		return FALSE;
	else
		return (now - m_tLoginCancelled) <= tLoginCancelInterval;
}



// SetUserJustCancelled [PRIVATE]
// Note:
// Always use SetUserJustCancelled() in other methods. Don't try
// to reference m_tConnectionCancelled any place else.
//
void CImapAccount::SetUserJustCancelledLogin ()
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock(0))
	{
		return;
	}

	// Just set the curent timestamp.
	//
	m_tLoginCancelled = time(0);
}




// InvalidatePasswordMT [PUBLIC]
//
// Signal that the cached password is invalid.
// If "bEraseSavedPassword" is TRUE, also remove any saved password for
// this account.
//
void CImapAccount::InvalidatePasswordMT (BOOL bEraseSavedPassword)
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock(0))
	{
		return;
	}

	// Login failed? Update in-memory flag.
	//
	m_PasswordValidity = PWVALIDITY_UNKNOWN;

	// Should we erase saved password??
	// Note: This just set the flag. Caller must arrange for the paswd to be cleared from disk 
	// in the main thread.
	//
	if ( bEraseSavedPassword )
	{	
		SetPasswordMT ("");
		//m_bPWNeedsSavingToDisk = TRUE;
		SavePasswordToDisk();
	}
}




// ValidatePasswordMT [PUBLIC]
//
//void CImapAccount::ValidatePasswordMT ()
//{
//	// Make his thread safe..
//	//
//	CSingleLock localLock (m_pLoginLockable, FALSE);
//
//	// Attempt to lock.
//	//
//	if (!localLock.Lock(0))
//	{
//		return;
//	}
//
//	// password must be non-empty to validate it.
//	if ( !m_Password.IsEmpty() )
//	{
//		// This may be called even though the password is already valid.
//		//
//		if ( m_PasswordValidity != PWVALIDITY_IS_VALID )
//		{
//			m_PasswordValidity = PWVALIDITY_IS_VALID;
//
//			// Force a save to disk in the main thread.
//			// Note: This method may be called in a worker thread, in which case
//			// we can't do the saving to disk here. Whoever calls this method
//			// will need to save the password in the main thread. Of course that
//			// depends on the INI setting for this account (which must be queried
//			// only in the main thread.
//			//
//			//m_bPWNeedsSavingToDisk = TRUE;
//			SavePasswordToDisk();
//		}
//	}
//}







//============================ non-class function ======================/

//
// Go get the account's delimiter char and update the command object's value.
//
void UpdateDelimiterChar (CIMAP *pImap, CImapAccount *pAccount)
{
	TCHAR Delimiter = 0;

	// Sanity:
	if (! (pImap && pAccount) ) 
		return;

	pImap->GetTopMboxDelimiter (&Delimiter);

	if (Delimiter)
	{
		// Set it into the account.
		pAccount->SetTopDelimiter (Delimiter);

		// Find the top level command object for this account.
		CString strDirectory; pAccount->GetDirectory (strDirectory);

		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( strDirectory );

		QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

		if (pImapCommand)
			pImapCommand->SetDelimiter (Delimiter);
	}
}

	
#endif // IMAP4
