// QCImapMailboxDirector.cpp: implementation of the IMAP4 QCMailboxDirector methods.
// NOTE: This file is #included in CMailboxDirector.cpp.
//////////////////////////////////////////////////////////////////////

#ifdef IMAP4 // Only for IMAP.


#include "persona.h"
#include "imapactl.h"
#include "QCImapMailboxCommand.h"
#include "imapfol.h"
#include "imapmlst.h"
#include "imapmime.h"

#include "mainfrm.h"


#ifdef EXPIRING
	// The Evaluation Time Stamp object
	#include "timestmp.h"
	extern CTimeStamp	g_TimeStamp;
#endif

//==================== INTERNAL ======================================/
typedef struct
{
	char *pImapName;
	char *pDirname;
	MailboxType theType;
	char DelimiterChar;
	BOOL bNoselect;
	BOOL bNoInferiors;
	BOOL bMarked;
	BOOL bUnmarked;
	UnreadStatusType theStatus;
} MailboxRecord;

BOOL ProcessMailboxRecord (char *buf, MailboxRecord *pMboxRec);
BOOL ExtractNameAndDirname (LPCSTR pBuffer, CString& szNewName, CString& szNewDirname);
unsigned int DoWeUpdateFilters (QCMailboxCommand *pCommand, COMMAND_ACTION_TYPE theAction );
void UpdateChildrenAfterRename (QCImapMailboxCommand *pImapCommand);
void CloseMailboxes (QCImapMailboxCommand *pImapCommand, BOOL bRecurse);
static void ReInitializeImapMbox (QCImapMailboxCommand *pImapCommand,
						COMMAND_ACTION_TYPE	theAction);
static void ResortChildren (QCImapMailboxCommand *pParentImapCommand, BOOL bRecurse);
static BOOL TocHasUnreadSummaries (CTocDoc *pTocDoc);

// ====================== END INTERNAL ================================/



/////////////////////////////////////////////////////////////
// ReBuildImapAccounts [PUBLIC]
// NOTES
// Loop through all personalities and create accounts for new personalities,
// END NOTES
BOOL QCMailboxDirector::BuildImapAccounts ()
{

	// Enumerate personalities.
	for (LPSTR pszNameList = g_Personalities.List();
		 pszNameList && *pszNameList;
		 pszNameList += strlen(pszNameList) + 1)
	{
		//
		// Temporarily switch to this personality so that we can determine if
		// it's an IMAP personality.
		//
		CString strPersona(pszNameList);

		// Go create an account for this personality if it's an IMAP account.
		// Note: "AddPersona" took care of setting/resetting curent personality.
		// Don't add the command objects to the tree control here.

		AddPersona (strPersona, FALSE, FALSE);

		// Did it add an account?
		CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( g_Personalities.GetHash( strPersona ) );
		if (pAccount)
		{
			// Read mailboxes.
			CString szName; pAccount->GetName (szName);
			QCImapMailboxCommand* pImapCommand = (QCImapMailboxCommand *) ImapFindByName(&m_theMailboxList,
									  szName);
			if (pImapCommand)
			{
				CString szDir; pAccount->GetDirectory (szDir);
				ProcessMailboxesFile (pAccount->GetAccountID(), szDir, pImapCommand->GetChildList() );
			}
		}
	}

	return TRUE;
}




/////////////////////////////////////////////////////////////
// AddPersona [PUBLIC]
// NOTES
// A new personality has been created. Go create an account object for it.
// NOTE: If bRefreshMboxlist is TRUE, go get new list of mailboxes from server and
// update tree view.Note: bRefreshMboxList => bInitializeMboxSubtree.
// If bInitializeMboxSubtree alone is TRUE, just update the tree view.
// END NOTES
BOOL QCMailboxDirector::AddPersona (CString& Name, BOOL bRefreshMboxList /* = FALSE */, BOOL bInitializeMboxSubtree /* = FALSE */)
{
	CString					BasePath;
	CString					szFullPath;

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	//
	// Temporarily switch to the given personality so that we can determine if
	// it's an IMAP personality.
	//
	CString strPersona = Name;

	if (g_Personalities.SetCurrent(strPersona))
	{
		if (!GetIniShort (IDS_INI_USES_IMAP))
		{
			g_Personalities.SetCurrent(strCurrentPersona);
			return TRUE;
		}
	}
	else
		return FALSE;

	// If we get here, then this is an IMAP personality. Go create an account.

	// Make sure the Imap top directory exists.
	if ( !g_ImapAccountMgr.CreateTopImapDirectory() )
	{
		CString buf;

		g_ImapAccountMgr.GetTopImapDirectory (buf);

		ErrorDialog (IDS_ERR_IMAP_NO_CREATETOP, buf );

		// Bail.
		g_Personalities.SetCurrent(strCurrentPersona);

		return FALSE;
	}

	// Format the path to the parent directory of account directories.
	FormatBasePath (EudoraDir + CRString (IDS_IMAP_TOP_DIRECTORY_NAME), BasePath);	// BasePath will now have a trailing backslash.


	// The account keeps a relative dir. Format full path into szFullPath and set it into the account.
	szFullPath.Empty();

	//
	// Get the account's subdirectory. Note: Don't use GetIniString here becaues it gets the 
	// value in [Settings] if the personality's entry does not exist!!
	// 
	CString szAccountSubdir;
	TCHAR  szEntry [256];

	szEntry[0] = '\0';

	g_Personalities.GetProfileString(strPersona, CRString (IDS_INI_IMAP_ACCOUNT_DIR), 
								"", szEntry, sizeof( szEntry ) );

	szAccountSubdir = szEntry;

	// If no directory, go create one and save its name in the personality.
	// Create the directory also.
	if ( szAccountSubdir.IsEmpty () )
	{
		// Note: This returns a relative name in szAccountSubDir.
		if ( ! GetUniqueDir (BasePath, strPersona, szAccountSubdir, 24, TRUE) )
		{
			szFullPath.Empty();
		}
		else
		{
			// Success.
			szFullPath += szAccountSubdir;

			// We create a new directory, so update personality.
			CRString key( IDS_INI_IMAP_ACCOUNT_DIR );

			g_Personalities.WriteProfileString( strPersona, key, szAccountSubdir);
		}
	}
	else
	{
		szFullPath += szAccountSubdir;
	}

	// Did we get a directory?
	if (! szFullPath.IsEmpty() )
	{
		// Attempt to create an IMAP account object for this personality.

		CImapAccount *pAccount = g_ImapAccountMgr.AddAccountFromPersonality (strPersona,BasePath, szAccountSubdir);

		if (pAccount)
		{
			// Make account cache some internal stuff.
			BOOL	bNeedsModify;
			pAccount->ModifyFromPersonality (bNeedsModify, TRUE);

			// Add the account to the mailbox command list.

			CString szName;		pAccount->GetName (szName);
			CString szPrefix;	pAccount->GetPrefix (szPrefix);
			CString szDir;		pAccount->GetDirectory (szDir);

			QCImapMailboxCommand *pCommand = new QCImapMailboxCommand(this, szName, szPrefix,
										szDir, MBT_IMAP_ACCOUNT, US_UNKNOWN, pAccount->GetAccountID (), 
										TRUE,		// NoSelect
										FALSE,		// bMarked
										FALSE,		// bUnmarked
										FALSE );	// bNoInferiors
			if (pCommand)
			{
				pCommand->MarkValid();
				Insert( m_theMailboxList, (QCMailboxCommand *)pCommand );

				// If this personality was just created, go add the mailbox tree item.
				if (bRefreshMboxList)
				{
					ImapNotifyClients( pCommand, CA_NEW, NULL );

					// Must sort the new account within it's siblings also:
					//
					pCommand->Execute ( CA_SORT_AFTER_RENAME,
							(void *) pCommand->GetName() );

					// Fetch mailboxes.
					BOOL ViewNeedsRefresh;
					pCommand->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

					if (ViewNeedsRefresh)
						UpdateImapMailboxLists (pCommand, TRUE, TRUE);

				}
				else if (bInitializeMboxSubtree)
				{
					// Process any mailbox list file in the accoun's subdirectory.
					CString szDir; pAccount->GetDirectory (szDir);
					ProcessMailboxesFile (pAccount->GetAccountID(), szDir, pCommand->GetChildList() );

					// Initialize this mailbox ctrl subtree.
					InitializeImapSubtree( (QCMailboxTreeCtrl *) ((CMainFrame *) AfxGetMainWnd())->GetActiveMboxTreeCtrl(), 0, pCommand );
				}
			}
			else
			{
				// We have to delete the account.
				g_ImapAccountMgr.DeleteAccount ( pAccount->GetAccountID() );
				pAccount = NULL;

				// Should we delete local storage as well??
			}
		}
		else
		{
			// Put up error messages.
			ErrorDialog (IDS_ERR_IMAP_NO_CREATEACCT, strPersona);

			// Bail.
			g_Personalities.SetCurrent(strCurrentPersona);

			return FALSE;
		}
	} // SetCurrent

	//
	// Reset current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);

	return TRUE;
}



// 
// This method is called just before a personality is created.
// Delete the appropriate IMAP account.
//
// JOK, 9/16/97
//
BOOL QCMailboxDirector::WillDeletePersona (CString& Name)
{
	// Find the hash of the personality.
	DWORD dwHash = g_Personalities.GetHash( Name );

	// May not have an account for this personality if it's NOT an IMAP account.
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( dwHash );
	if (pAccount)
	{
		// Find the command object for this account..
		CString szName; pAccount->GetName(szName);
		QCImapMailboxCommand* pImapCommand = (QCImapMailboxCommand *) ImapFindByName(&m_theMailboxList,
									  szName);
		if (pImapCommand)
		{
			// Because this is an account, a CA_DELETE will only delete the locally
			// cached data, not the remote mailboxes.
			BOOL bDeleteLocalCache = TRUE;
			pImapCommand->Execute( CA_DELETE, &bDeleteLocalCache );
		}

		// Delete the IMAP account object.
		g_ImapAccountMgr.DeleteAccount ( pAccount->GetAccountID() );
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////
// PersonaModified [PUBLIC]
// This method is called a personality has been modified.
// Find the appropriate IMAP account and notify it.
//
// JOK, 9/16/97
/////////////////////////////////////////////////////////////////
BOOL QCMailboxDirector::ModifyPersona (CString& Name)
{
	BOOL bResult = FALSE;
	CImapAccount *pAccount = NULL;
	DWORD dwHash = 0;

	// Find the hash of the personality.

	dwHash = g_Personalities.GetHash( Name );

	// May not have an account for this personality if it's not an IMAP personality.
	pAccount = g_ImapAccountMgr.FindAccount ( dwHash );

	if (pAccount)
	{
		// Find the command object for this account..
		CString szName; pAccount->GetName(szName);
		QCImapMailboxCommand* pImapCommand = (QCImapMailboxCommand *) ImapFindByName(&m_theMailboxList, szName);
		if (pImapCommand)
		{
			if (pImapCommand->GetType() != MBT_IMAP_ACCOUNT)
			{
				ASSERT (0);
				return FALSE;
			}

			// If we have an account and it's no longer an IMAP account, remove the 
			// command object but leave the local message cache alone.

			// Save the current personality.
			CString strCurrentPersona = g_Personalities.GetCurrent();

			//
			// Temporarily switch to the given personality so that we can determine if
			// it's an IMAP personality.
			//
			if (g_Personalities.SetCurrent(Name))
			{
				if (!GetIniShort (IDS_INI_USES_IMAP))
				{
					// No longer an IMAP personality. Remove command object and delete local
					// mailbox cache.

					// Note - We must change this so we don't delete the local cache (JOK).
	
					BOOL bDeleteLocalCache = TRUE;
					pImapCommand->Execute( CA_DELETE, &bDeleteLocalCache );

					pImapCommand = NULL;

					// Delete the IMAP account object.
					g_ImapAccountMgr.DeleteAccount ( pAccount->GetAccountID() );

					pAccount = NULL;
				}

				// Reset personality.
				g_Personalities.SetCurrent(strCurrentPersona);
			}
		}
	}

	// If the account is still there, means that we need to change some internal 
	// account properties.

	pAccount = g_ImapAccountMgr.FindAccount ( dwHash );

	if (pAccount)
	{
		// Go see if we still have the command object. If we still do, means that
		// the account is still IMAP but other properties may have changed.

		CString szName; pAccount->GetName(szName);

		QCImapMailboxCommand* pImapCommand = (QCImapMailboxCommand *) ImapFindByName(&m_theMailboxList, szName);

		if (pImapCommand)
		{
			// Modify the IMAP account. If the changes need a refresh of the mailbox list, and the
			// user wants to refresh, go get new list of mailboxes.
			BOOL ViewNeedsRefresh;

			pAccount->ModifyFromPersonality (ViewNeedsRefresh, FALSE);

			if (ViewNeedsRefresh)
			{
				//
				// Go remove all locally cached mailboxes to avoid confusion.
				//
				pImapCommand->InvalidateChildMailboxes (TRUE);
				pImapCommand->RemoveInvalidChildMailboxes (TRUE);

				// Refresh mailbox list.
				pImapCommand->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

				if (ViewNeedsRefresh)
					UpdateImapMailboxLists (pImapCommand, TRUE, TRUE);
			}
		}
	}
	else
	{
		// Possibly a change from POP to IMAP??
		bResult = AddPersona (Name, TRUE, FALSE);
	}

	return bResult;
}




//////////////////////////////////////////////////////////
// ProcessMailboxesFile [Private]
// Process a mailboxes file in the directory "pPath" for the account
// given by AccountId.
// NOTES
// Return TRUE, except if a serious error ocurred.
// If "TmpList" is TRUE, then we are reading the temporary mailbox list. Do not 
// perform any salvage operations.
// If Recurse, then recurse through child mailboxes.
// 
// END NOTES
/////////////////////////////////////////////////////////
BOOL  QCMailboxDirector::ProcessMailboxesFile (ACCOUNT_ID AccountId, LPCSTR pPath, CPtrList &theMailboxList, BOOL Recurse /* = TRUE */, BOOL TmpList /* = FALSE */)
{
	CString				MbxFile;
	CString				MailboxDir;
	CString				MailboxesFile;
	JJFileMT			theMapFile;
	BOOL				bNeedsWrite;
	char				buf[1024];
	BOOL				bValidEntry;
	QCMailboxCommand*	pCommand;
	POSITION			pos;
	POSITION			next;
	CString				BasePath;

	// Make sure we have valid parameters.
	if (AccountId == 0 || pPath == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	// The account directory must exist.
	if (!DirectoryExists(pPath))
		return FALSE;

	// Form the mailbox directory name string with trailing delimiter.
	FormatBasePath (pPath, BasePath);	// BasePath will now have a trailing backslash.

	// Get the full pathname of the mailbox list file.
	if (TmpList)
		MailboxesFile = BasePath + CRString(IDS_IMAP_TMP_MBOXLIST_FILENAME);
	else
		MailboxesFile = BasePath + CRString(IDS_IMAP_MBOXLIST_FILENAME);

	if ( !FileExistsMT( (LPCSTR) MailboxesFile ) )
		return TRUE;		// OK if the file doesn't exist - means no mailboxes.

	// If we can't open the file, that's an error.
	if( FAILED (theMapFile.Open( MailboxesFile, O_RDONLY ) ) )
	{
		return FALSE;
	}
		
	//
	// Set the following flag TRUE if we need to immediately write this 
	// file in order to flush out any stale entries.
	//
	bNeedsWrite = FALSE;

	//
	// Process the entire mailboxlist file, one line at a time.
	//
	MailboxRecord		MboxRec;
	long lNumBytesRead = 0;
	while( SUCCEEDED(theMapFile.GetLine( buf, sizeof( buf ), &lNumBytesRead)) && (lNumBytesRead > 0) )
	{
		memset ((void *)&MboxRec, sizeof (MailboxRecord), 0);
		// Note: ProcessMailboxRecord returns pointers into "buf".
		bValidEntry = ProcessMailboxRecord (buf, &MboxRec);

		if (!bValidEntry)
		{
			ASSERT (0);
			bNeedsWrite = TRUE;
			continue;
		}

		// If the directoru name is blank, ignore this one.
		if ( ! (MboxRec.pDirname && *MboxRec.pDirname) )
			continue;

		// Extract base name if MBoxRec.pDirname is a full path. This shouldn't be, though.
		char *p = strrchr (MboxRec.pDirname, DirectoryDelimiter);
		if (p)
			p++;
		else
			p = MboxRec.pDirname;

		// Well, we don't care at this point if the directory exists or not.
		MailboxDir = BasePath + p;
		
		// Set the MBX file path into the command object.
		// Note: We don't care if the local mailbox exists or not.

		MbxFile.Empty();
		GetMbxFilePath ( (LPCSTR) MailboxDir, MbxFile);

		// OK.
		if( bValidEntry )
		{
			//
			// Look for duplicates.  If this entry conflicts with
			// an earlier entry, then don't allow it.  This should be
			// fixed up during the Salvage() sequence, which makes sure
			// there is a DESCMAP entry for each mailbox directory.
			//
			pos = theMailboxList.GetHeadPosition();

			for( next = pos; pos; pos = next )
			{
				pCommand = ( QCMailboxCommand* ) theMailboxList.GetNext( next );

				if( stricmp( (LPCSTR)MailboxDir, pCommand->GetPathname() ) == 0 )
				{
					ASSERT( 0 );
					bNeedsWrite = TRUE;
					break;
				}
			}

			if( pos == NULL )
			{
				// Formulate a friendly name.
				CString FriendlyName;
				FormulateFriendlyImapName (MboxRec.pImapName, MboxRec.DelimiterChar, FriendlyName);

				QCImapMailboxCommand *pImapCommand = new QCImapMailboxCommand(this, FriendlyName, 
								MboxRec.pImapName, (LPCSTR)MbxFile, MboxRec.theType,
								MboxRec.theStatus, AccountId, MboxRec.bNoselect, 
								MboxRec.bMarked, MboxRec.bUnmarked, MboxRec.bNoInferiors );
				if (pImapCommand)
				{
					pImapCommand->MarkValid();

					pImapCommand->SetDelimiter (MboxRec.DelimiterChar);
					Insert( theMailboxList, (QCMailboxCommand *)pImapCommand );
					
					// Before we leave, go make sure all the components of the mailbox directory
					// have been created, possibly as stubs.
					// Note: Only if we are processing the real mailbox list.
					if (!TmpList)
						ImapNotifyClients( pImapCommand, CA_IMAP_REFRESH, NULL );

					// Go process subdirectories.
					if (Recurse && !MboxRec.bNoInferiors)
					{
						ProcessMailboxesFile (AccountId, MailboxDir, pImapCommand->GetChildList(), Recurse, TmpList);
					}
				}
				else
				{
					ASSERT( 0 );
					bNeedsWrite = TRUE;
					continue;
				}
			}
		}
	}	// While.
		
	theMapFile.Close();

	if(!TmpList)
	{
		if (bNeedsWrite)
		{
			WriteImapDescMapFile( pPath, theMailboxList );
		}

		// build the file (if it's missing) or add in files we didn't know about
		if( !SalvageImapMailboxesFile( AccountId, pPath, theMailboxList ) )
		{
			return FALSE;
		}
	}
	
	return TRUE;
}



// WriteImapDescMapFile [PUBLIC]
// FUNCTION
// Writes the mboxlist file.
// END FUNCTION

// NOTES
// szPath is the directory housing the mailbox, account or namespace.
// END NOTES
BOOL QCMailboxDirector::WriteImapDescMapFile(
	LPCSTR			pPath,
	const CPtrList& theList,
	BOOL bRecurse /* = FALSE */)
{
	CString					Line, BasePath;
	CString					MailboxesFile;
	QCImapMailboxCommand	*pImapCommand;
	POSITION				pos;
	JJFileMT				theMapFile;

	// Form the mailbox directory name string with trailing delimiter.
	FormatBasePath (pPath, BasePath);	// BasePath will now have a trailing backslash.

	// Get the full pathname of the mailbox list file.
	MailboxesFile = BasePath + CRString(IDS_IMAP_MBOXLIST_FILENAME);

	// Open the file for overwriting.
	if( theMapFile.Open( MailboxesFile, O_CREAT | O_TRUNC | O_WRONLY ) < 0)
	{
		return FALSE;
	}

	// Loop through the list.
	pos = theList.GetHeadPosition ();
	while (pos)
	{
		pImapCommand = (QCImapMailboxCommand *) theList.GetNext (pos);
		if (pImapCommand)
		{
			CString Dir;
			pImapCommand->GetObjectDirectory(Dir);
			// Extract base name.
			int i = Dir.ReverseFind (DirectoryDelimiter);
			if (i >= 0)
				Dir = Dir.Mid (i + 1);

			// Handle Unread Status
			//
			char cStatus = US_UNKNOWN;

			switch( pImapCommand->GetStatus() )
			{
				case US_UNKNOWN:
					cStatus = 'U';
				break;
				case US_YES:
					cStatus = 'Y';
				break;
				case US_NO:
					cStatus = 'N';
				break;
			}

			Line.Format ("%s,%s,%c,%c,%c,%c,%c,%c,%c", 
						pImapCommand->GetImapName(), Dir,
						ImapMailboxTypeToTypeChar (pImapCommand->MBTTypeToImapType(pImapCommand->GetType())),
						pImapCommand->GetDelimiter() ? pImapCommand->GetDelimiter() : ' ',
						pImapCommand->IsNoSelect()  ? '1' : '0',
						pImapCommand->IsNoInferiors() ? '1' : '0',
						pImapCommand->IsMarked() ? '1' : '0',
						pImapCommand->IsUnmarked() ? '1' : '0',
						cStatus );

			theMapFile.PutLine ( (LPCSTR)Line);
		}
	}

	// Close the file.
	theMapFile.Close();

	// If bRecurse, go do child mailboxes if they can have children.
	// Loop through the list.
	if (bRecurse)
	{
		pos = theList.GetHeadPosition ();
		while (pos)
		{
			pImapCommand = (QCImapMailboxCommand *) theList.GetNext (pos);
			if (pImapCommand)
			{
				if ( !pImapCommand->IsNoInferiors () )
				{
					CString Dir;
					pImapCommand->GetObjectDirectory (Dir);
					WriteImapDescMapFile( Dir, pImapCommand->GetChildList(), bRecurse);
				}
			}
		}
	}

	return TRUE;
}


// InitializeImapSubtrees [PUBLIC]
// NOTES
// Handles the addition to the mailbox tree of all IMAP item types.
// Return TRUE if any item added had unread messages.
// END NOTES
BOOL QCMailboxDirector::InitializeImapSubtree( 
		QCMailboxTreeCtrl*	pTreeCtrl,
		UINT				uLevel,
		QCImapMailboxCommand*	pImapCommand)
{
	BOOL				bHasUnread = FALSE;
	QCMailboxTreeCtrl::ItemType itemType;
	POSITION			pos = NULL;
	CPtrList*			pChildList = NULL;
	QCMailboxCommand*   pChildCommand = NULL;

	// Sanity:
	if ( !(pTreeCtrl && pImapCommand) )
		return FALSE;

	// Make sure we have the right type of object.
	ASSERT_KINDOF ( QCImapMailboxCommand, pImapCommand );

	switch (pImapCommand->GetType ())
	{
		case MBT_IMAP_ACCOUNT:
			itemType = QCMailboxTreeCtrl::ITEM_IMAP_ACCOUNT;
			break;
		case MBT_IMAP_NAMESPACE:
			itemType = QCMailboxTreeCtrl::ITEM_IMAP_NAMESPACE;
			break;
		case MBT_IMAP_MAILBOX:
			itemType = QCMailboxTreeCtrl::ITEM_IMAP_MAILBOX;
			break;
		default:
			// Get here and it's an error.
			ASSERT ( 0 ) ;
			return FALSE;
	}

	// Add the account item to the tree control
	bHasUnread = pImapCommand->GetStatus() == US_YES;
	if ( !pTreeCtrl->AddImapItem(itemType, pImapCommand->GetName(), pImapCommand, bHasUnread ) )
		return FALSE;

	// Loop through the child list (if any).
	pChildList = &pImapCommand->GetChildList();
	pos = pChildList->GetHeadPosition();

	while( pos )
	{
		pChildCommand = ( QCMailboxCommand* ) pChildList->GetNext( pos );
		if (pChildCommand)
		{
			if ( pChildCommand->GetType() == MBT_IMAP_ACCOUNT ||
				  pChildCommand->GetType() == MBT_IMAP_NAMESPACE ||
				  pChildCommand->GetType() == MBT_IMAP_MAILBOX )
			{
				// If the command type is one of these, then the pCommand is a QCImapMailboxCommand object.
				ASSERT_KINDOF( QCImapMailboxCommand, ( QCImapMailboxCommand* ) pChildCommand );
				BOOL bImapHasUnreadMessages = InitializeImapSubtree ( pTreeCtrl, uLevel + 1, (QCImapMailboxCommand *)pChildCommand);
				bHasUnread = bHasUnread || bImapHasUnreadMessages;
			}
		}
	}

	// No need to resort as the list is already in order
//	ResortChildren ( pImapCommand, FALSE );

	return bHasUnread;
}


BOOL  QCMailboxDirector::SalvageImapMailboxesFile(
		ACCOUNT_ID			Id, 
		LPCSTR				pPath,
		CPtrList&			theMailboxList)
{
	return TRUE;
}




// UpdateImapMailboxLists [PUBLIC]
// FUNCTION
// 1. Read the tmp_mboxlist.lst file in the directory corresponding to the given
//    command object (account. namespace or mailbox) and build a CPtrlist.
// 2. May recurse.
// 3. Compare the list with the currently active list and add any new mailboxes
//	  to the active command list AND the tree control.
// 4, At each level, if a mailbox no longer exists, delete it and it's children
//	  the current command object list.
// END FUNCTION

// NOTES
// NOTE: IsLevelZero is TRUE if we are at the top level of the recursion.
// END NOTES.

BOOL QCMailboxDirector::UpdateImapMailboxLists (
	QCImapMailboxCommand *pParentImapCommand,
	BOOL Recurse, 
	BOOL IsLevelZero )
{
	CString		Dirpath;
	CPtrList	newMailboxList;
	CPtrList	*pNewList, *pOldList;
	QCImapMailboxCommand *pNewCurrent, *pOldCurrent;
	BOOL		bResult = TRUE;

	// Sanity.
	if (!pParentImapCommand)
		return FALSE;

	if( !pParentImapCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
		return FALSE;

	// Initialize
	pNewList = pOldList = NULL;
	Dirpath.Empty();

	// Get the directory housing the parent object.
	if (!pParentImapCommand->GetObjectDirectory (Dirpath))
	{
		// Can't be!!
		ASSERT (0);
		return FALSE;
	}

	// Go build the list from the temporary mailbox list. Don't recurse.
	if ( ! ProcessMailboxesFile (pParentImapCommand->GetAccountID(), Dirpath, newMailboxList, FALSE, TRUE) )
	{
		return FALSE;
	}

	// If we are at the top level of the recursion, Invalidate all child mailboxes
	// that we currently know about so we can spot the ones that either no longer exist
	// on the server or the user are no longer interested in.

	if (IsLevelZero)
	{
		pParentImapCommand->InvalidateChildMailboxes (Recurse);
	}

	// Prepare to loop.
	// BUG: We should be able to do a quick block replace here instead of what we do below
	
	pNewList = &newMailboxList;
	pOldList = &( pParentImapCommand->GetChildList() );

	// Ok. Loop through the new list and delete mailboxes we already know about.

	POSITION pos, next, oldPos;
	CString  ImapName, Mboxdir;

	pos = pNewList->GetHeadPosition();

	while( pos  )
	{
		next = pos;
		pNewCurrent = ( QCImapMailboxCommand * ) pNewList->GetNext(next);

		if (pNewCurrent)
		{
			ImapName = pNewCurrent->GetImapName();

			// How do we know if they are the same mailbox?? Compare the full imap name,
			// case sensitively.
			// 
			oldPos = pOldList->GetHeadPosition();
			while (oldPos)
			{
				pOldCurrent = ( QCImapMailboxCommand * ) pOldList->GetNext(oldPos);
				if ( pOldCurrent )
				{
					if (ImapName.Compare (pOldCurrent->GetImapName()) == 0)
					{
						// If it's found in the old list, delete it from the new list and mark the
						// old object as valid.
						pOldCurrent->MarkValid();

						pNewList->RemoveAt (pos);
						delete pNewCurrent;
						pNewCurrent = NULL;

						break;
					}
				}
			}

		} // if pNewCurrent.

		// Next entry.
		pos = next;

	} // while

	//
	// If we have invalid mailboxes at this level, then they no longer exist on the server.
	// Go delete local storage for those mailboxes and remove menu items.
	//
	pOldList = &( pParentImapCommand->GetChildList() );

	pos = pOldList->GetHeadPosition ();

	for (next = pos; pos; pos = next)
	{
		QCImapMailboxCommand *pCurrent = (QCImapMailboxCommand *) pOldList->GetNext (next);

		//
		// Be careful here!! "RemoveMailboxCommand deletes pCurrent AND removes it from 
		// the list we are traversing!!
		//
		if ( !pCurrent->IsValid() )
		{
			RemoveImapMailboxCommand ( pCurrent );
		}
	}

	// If we have any objects left in the list, go add them to the old list.
	pos = pNewList->GetHeadPosition();
	CPtrList& OldList = pParentImapCommand->GetChildList();

	for( next = pos; pos; pos = next )
	{
		pNewCurrent = ( QCImapMailboxCommand * ) pNewList->GetNext(next);
		if (pNewCurrent)
		{
			pNewCurrent->MarkValid();

			// Insert it into the old list.
			Insert(  OldList, (QCMailboxCommand *)pNewCurrent );

			// Remove it from the new list (but don't delete the object!!)
			newMailboxList.RemoveAt (pos);

			ImapNotifyClients( pNewCurrent, CA_NEW, NULL );
		}
	}

	// Now recurse through the "new" old list and handle child mailboxes.

	if (Recurse)
	{
		pOldList = &( pParentImapCommand->GetChildList() );
		oldPos = pOldList->GetHeadPosition();
		bResult = TRUE;
		while( oldPos && bResult )
		{
			pOldCurrent = ( QCImapMailboxCommand * ) pOldList->GetNext(oldPos);
			if (pOldCurrent)
			{
				if (!pOldCurrent->IsNoInferiors())
					bResult = UpdateImapMailboxLists ( pOldCurrent, Recurse, FALSE);
			}
		}
	}

	// If we are at level zero, go delete any invalid mailboxes from our
	// local lists.
	if (IsLevelZero)
	{
		// There may be mailboxes that no longer exists on the server. Loop through the list 
		// (possibly recursively) and send notifications that they have been deleted.

		pParentImapCommand->RemoveInvalidChildMailboxes (Recurse);

		// Update all the mailbox lists.
		WriteImapDescMapFile( Dirpath, pParentImapCommand->GetChildList(), TRUE );

		// Expand the top item.

		ImapNotifyClients( pParentImapCommand, CA_IMAP_REFRESH, NULL);
	}


	// If any left, cleanup.
	pos = newMailboxList.GetHeadPosition();

	for( next = pos; pos; pos = next )
	{
		pNewCurrent = ( QCImapMailboxCommand * ) newMailboxList.GetNext(next);
		if ( pNewCurrent )
		{
			newMailboxList.RemoveAt (pos);
			delete pNewCurrent;

			pNewCurrent = NULL;
		}
	}

	// Resort mailbox tree items at this level:
	//
	if (IsLevelZero)
	{
		ResortChildren (pParentImapCommand, Recurse);
	}

	// Ok. We're done.
	return bResult;
}




// ImapAddCommand [PUBLIC]
// FUNCTION
// Add an account, namespace or mailbox to the list of mailboxes.
// The local directory for the account, namespace or mailbox MUST have already been created.
// In the case of a namespace or mailbox, the imap info file MUST have already been written
// because that's where we get the IMAP information. This is just like what's done in 
// InitializeImapSubtree().
// END FUNCTION

// NOTES
// The parent can be NULL, in which case the new item MUST be ac account.
// Namespaces can only be added to parent accounts.
// Mailboxes can be added to account, namespaces or other mailboxes.
// Note: For adding an ACCOUNT, "pPath" must the the name of the directory housing the account, mailbox, namespace, etc.
//       For creating a new mailbox, pPath is the intended IMAP name of the mailbox.
// END NOTES

QCMailboxCommand*	QCMailboxDirector::ImapAddCommand(
LPCSTR				pPath,
MailboxType			theType,
QCMailboxCommand*	pParent )
{
	// Must have a valid path.
	if (!pPath)
		return NULL;

	// Insertion of an account?
	if (pParent == NULL)
	{
		QCImapMailboxCommand*	pImapCommand;

		// This must be addition of a new account to a command list.
		if (theType != MBT_IMAP_ACCOUNT)
			return NULL;

		// The account must have already been created and added to the internal account list.
		CImapAccount *pAccount = NULL;
		ACCOUNT_ID    AccountId;

		AccountId = g_ImapAccountMgr.FindOwningAccount (pPath);
		if (AccountId)
			pAccount = g_ImapAccountMgr.FindAccount (AccountId);
		if (!pAccount)
		{
			return NULL;
		}

		// Create the new command object.
		CString szName;		pAccount->GetName (szName);
		CString szPrefix;	pAccount->GetPrefix (szPrefix);
		CString szDir;		pAccount->GetDirectory (szDir);

		pImapCommand = new QCImapMailboxCommand(this, szName, szPrefix,
								szDir, MBT_IMAP_ACCOUNT, US_UNKNOWN,
								AccountId);
		if (pImapCommand)
		{
			Insert( m_theMailboxList, (QCMailboxCommand *)pImapCommand );

			ImapNotifyClients( pImapCommand, CA_NEW, NULL );

			return pImapCommand;
		}
		else
		{
			return NULL;
		}
	}
	else 
	{
		// Means create a new IMAP mailbox. 
		QCImapMailboxCommand *pImapParent;

		// The parent must be of IMAP type.
		if( !pParent->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
		{
			ASSERT (0);
			return NULL;
		}

		// Use this from now on.
		pImapParent = (QCImapMailboxCommand *)pParent;

		// Parent cannot have the "NoInferiors" flag set.
		if (pImapParent->IsNoInferiors())
		{
			ASSERT (0);
			return NULL;
		}
		
		// Create the remote mailbox on the parent IMAP server.
		CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( ((QCImapMailboxCommand *)pParent)->GetAccountID() );
		if (pAccount)
		{
			//
			// If theType is "MBT_FOLDER", then the user said "Make it a folder".
			//

			BOOL bMakeItAFolder = (theType == MBT_FOLDER);

			QCImapMailboxCommand *pNewImapCommand = 
					pAccount->CreateMailbox ( pPath, (QCImapMailboxCommand *)pParent, FALSE, bMakeItAFolder);

			if (pNewImapCommand)
			{
				//
				// If the mailbox name contained a delimiter character, refresh the mailbox list..
				//
				TCHAR Delimiter = pNewImapCommand->GetDelimiter();

				if ( bMakeItAFolder || 
					 ( Delimiter && strchr (pPath, Delimiter) ) )
				{
					// Don't need pNewImapCommand.
					delete pNewImapCommand;

					pNewImapCommand = NULL;

					// Fetch mailboxes.
					BOOL ViewNeedsRefresh;
					pImapParent->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

					UpdateImapMailboxLists (pImapParent, TRUE, TRUE);
				}
				else
				{
					CString Dir;

					Insert (pParent->GetChildList(), (QCMailboxCommand *)pNewImapCommand);

					ImapNotifyClients( pNewImapCommand, CA_NEW, NULL );

					pImapParent->GetObjectDirectory(Dir);

					// Keep the mailbox tree sorted:
					//
					QCCommandDirector::NotifyClients( pNewImapCommand, CA_SORT_AFTER_RENAME,
										 (void *) pNewImapCommand->GetName() );

					WriteImapDescMapFile( Dir, pImapParent->GetChildList() );

					return pNewImapCommand;
				}
			}
		}
	}

	return NULL;;
}





// ImapNotifyClients [PUBLIC]
// FUNCTION
// Fix up state after a major action.
// END FUNCTION
void	QCMailboxDirector::ImapNotifyClients(
	QCCommandObject*	pQCCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData )
{
	QCImapMailboxCommand*	pImapCommand;

	if (!pQCCommand)
	{
		ASSERT (0);
		return;
	}

	if( pQCCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
	{
		pImapCommand = ( QCImapMailboxCommand* ) pQCCommand;
	}
	else
	{
		ASSERT (0);
		return;
	}

	// Note: The CA_DELETE handlers call NotifyClients themselves.
	if (theAction == CA_DELETE)
	{
		if (pImapCommand->GetType () == MBT_IMAP_ACCOUNT)
		{
			// "pData" tells us if to delete the local account cache.
			// Default to TRUE.
			BOOL bDeleteLocalCache = TRUE;
			if (pData)
			{
				bDeleteLocalCache = *(BOOL *)pData;
			}

			HandleDeleteImapAccount (pImapCommand, theAction, bDeleteLocalCache);
		}
		else if ( pImapCommand->GetType () == MBT_IMAP_MAILBOX )
		{
			// "pData" tells us if to delete child mailboxes.
			BOOL bDeleteChildren = FALSE;
			if (pData)
			{
				bDeleteChildren = *(BOOL *)pData;
			}

			HandleDeleteImapMailbox (pImapCommand, theAction, bDeleteChildren);
		}
		return;
	}
	else if( theAction == CA_RENAME )
	{		
		// Note: It's the ADDRESS of pImapCommand that's passed to "HandleRenameImapMailbox".
		// On return from that function, "pImapCommand" may be NULL!!
		//
		
		HandleRenameImapMailbox ((QCMailboxCommand **) &pImapCommand, theAction, pData);
	}
	else if( theAction == CA_IMAP_GRAFT )
	{		
		// Note: DON'T assume that pImapCommand would still be valid after this!!!
		//
		// NOte: pData MUST be the new parent command object.
		//
		
		HandleMoveImapMailbox ((QCMailboxCommand **) &pImapCommand, theAction,
						(QCMailboxCommand *) pData);
	}

	// 
	else if( theAction == CA_UPDATE_STATUS )
	{
		QCImapMailboxCommand *pParent = NULL;
		CPtrList			 *pList   = NULL;
		POSITION			  pos	  = NULL;

		// Need to notify clients.
		QCCommandDirector::NotifyClients( pImapCommand, theAction, pData );

		// May need to set parent to BOLD as well.
		//
		FindContainingList (pImapCommand, &pList, (QCMailboxCommand **) &pParent);

		if( !(pParent && pList) )
		{
			return;
		}

		// write the descmap file. Don't recurse here.
		CString szParentDir; pParent->GetObjectDirectory (szParentDir);

		WriteImapDescMapFile( szParentDir, *pList, FALSE);

		// Notify parent that it should bold/unbold itself as well:
		//

		// see if we need to update the parent status
		if( pParent->GetStatus() == pImapCommand->GetStatus() )
		{
			// nope -- just bail
			return;
		}

#if 0 // JOK - 7/22/98.

		// sigh -- we need to walk the list
		
		pos = pList->GetHeadPosition();
		BOOL bFoundChildWithUnread = FALSE;

		while( pos )
		{
			QCMailboxCommand *pCurrent = ( QCMailboxCommand* ) pList->GetNext( pos );

			ASSERT( pCurrent );
			
			if( pCurrent && (pCurrent->GetStatus() == US_YES) )
			{
				bFoundChildWithUnread = TRUE;
				break;
			}
		}

		if (bFoundChildWithUnread)
		{
			if( pParent->GetStatus() != US_YES )
			{
				// turn on the parent status
				pParent->Execute( CA_UPDATE_STATUS, ( void* )  US_YES );
			}


		if( pParent->GetStatus() == US_YES )
		{
			// turn it off
			pParent->Execute( CA_UPDATE_STATUS, ( void* ) US_NO );
		}
#endif // JOK

		// If the parent mailbox is open, we can know for sure if it has unread summaries:
		//
		UnreadStatusType theNewParentStatus = (UnreadStatusType) (ULONG) pData;

		if ( (theNewParentStatus == US_NO) && pParent->CanContainMessages() )
		{
			CTocDoc* pTocDoc = GetToc (pParent->GetPathname(), NULL, FALSE, TRUE);

			if ( pTocDoc && TocHasUnreadSummaries (pTocDoc) )
			{
				theNewParentStatus = US_YES;
			}
			else if (!pTocDoc)
			{
				// If the mailbox is not open, then don't change it's status.
				theNewParentStatus = pParent->GetStatus();
			}
		}

		if ( pParent->GetStatus() != theNewParentStatus )
			pParent->Execute( CA_UPDATE_STATUS, (void *)theNewParentStatus );
	}
	else
	{
		// Just go notify clients for now.
		QCCommandDirector::NotifyClients( pImapCommand, theAction, pData );
	}

	// We need to (re)initialize IMAP storage in certain cases..
	// Note: pImapCOmmand may now be NULL!!
	//
	if ( pImapCommand &&
	     (theAction == CA_NEW			|| 
		  theAction == CA_IMAP_REFRESH	||
		  theAction == CA_RENAME) )
	{

		ReInitializeImapMbox (pImapCommand, theAction);

	}
}




// HandleDeleteImapMailbox [PRIVATE]
// FUNCTION
// Avoid congestion of the notify routine.
// END FUNCITON

BOOL QCMailboxDirector::HandleDeleteImapMailbox (QCMailboxCommand *pCommand,
									 COMMAND_ACTION_TYPE theAction, BOOL bDeleteChildren)
{
	BOOL		bResult = TRUE;
	CString		szMsg;

	// Must have a director and a command.
	if (! pCommand )
	{
		ASSERT (0);
		return FALSE;
	}

	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

	// We handle mailboxes only.
	if (pImapCommand->GetType () != MBT_IMAP_MAILBOX)
		return FALSE;

	// We only handle CA_DELETE of a mailbox..
	if( theAction != CA_DELETE )
		return FALSE;

	// If there are filters which involve this mailbox or it's children, 
	// ask user if to continue.
	//
	unsigned int uRet = IDC_LEAVE_FILTER;

#ifdef EXPIRING
	if ( !g_TimeStamp.IsExpired0() )
#endif // EXPIRING
	{
		uRet = DoWeUpdateFilters (pCommand, theAction );
	}

	// User may have said "Abort the delete"
	if (uRet == IDCANCEL)
		return FALSE;
	
	// Oh well, continue with the delete.	
	
	// Go delete mailboxes from the server, including child mailboxes.
	// NOTE: IT may have already been deleted from the server, in which case
	// it would have been marked as INVALID and "DeleteRemoteMailboxes()" will return TRUE.

	bResult = DeleteRemoteMailboxes (pImapCommand, bDeleteChildren);

	// If we failed somewhere, don't continue.
	if (!bResult)
		return FALSE;


	// OK, continue with the notifications.

	// This will remove menu and mailbox tree items.
	QCCommandDirector::NotifyClients( pImapCommand, theAction, NULL );

	// If we get here, it means we have already deleted stuff from the server and from our menus
	// and the mailbox tree. Go cleanup local storage.
	
	// Is this a mailbox without children?
	if( pImapCommand->GetChildList().IsEmpty() )
	{
		// Delete local storage whether the IMAP delete succeeded or not. BUG: This 
		// may not be the right thing to do!!

		// Get the toc if it's open so we can close it's viewers.
		CTocDoc *pTocDoc = GetToc (pImapCommand->GetPathname(), NULL, TRUE, TRUE);
		if (pTocDoc)
		{
#if 0 
			// Force all message windows to be closed as well
			int OldMSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
			SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, TRUE);
#endif

			// Really delete the TOC now.
			pTocDoc->m_bAutoDelete = TRUE;
			pTocDoc->OnCloseDocument();
#if 0 
			SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, ( short ) OldMSC);
#endif 

			// Make sure we don't use this below.
			pTocDoc = NULL;
		}
		
		// Remove the mailbox's local storage structures.
		// NOTE: Still continue if we didn't delete the directory.
		CString Dirname;
		pImapCommand->GetObjectDirectory ( Dirname );
		if (!RemoveLocalDirectory (Dirname, TRUE))
		{
			ErrorDialog( IDS_ERR_DIR_DELETE, Dirname );
			// Fall through.
		}

		// Check to see if any filters get affected
		if( uRet == IDC_UPDATE_FILTER )
		{
			FiltersUpdateMailbox( pCommand->GetPathname(), NULL, NULL, FALSE );
		}

		// remove it from the list
		QCImapMailboxCommand *pParent = NULL;
		CPtrList			 *pList   = NULL;
		POSITION			  pos	  = NULL;
		pos = FindContainingList (pImapCommand, &pList, (QCMailboxCommand **) &pParent);
		if (!(pos && pList))
		{
			ASSERT (0);
			return FALSE;
		}

		pList->RemoveAt( pos );

		// write the map
		CString ParentDir;
		if (pParent == NULL)
		{
			g_ImapAccountMgr.GetTopImapDirectory (ParentDir);
		}
		else
		{
			pParent->GetObjectDirectory(ParentDir);
		}

		WriteImapDescMapFile( ParentDir, *pList );

		// Must do this last!
		delete pImapCommand;
		return TRUE;
	}
			
	return bResult;
}




// HandleDeleteImapAccount [PRIVATE]
// FUNCTION
// Avoid congestion of the notify routine.
// END FUNCITON

// NOTES
// If bDeleteLocalCache is FALSE, don't delete local mailbox cache.

BOOL QCMailboxDirector::HandleDeleteImapAccount (QCMailboxCommand *pCommand,
									 COMMAND_ACTION_TYPE theAction, BOOL bDeleteLocalCache)
{
	BOOL		bResult = TRUE;
	CString		szMsg;
	CString Dirname;

	// Must have a director and a command.
	if (! pCommand )
	{
		ASSERT (0);
		return FALSE;
	}
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

	// We handle accounts only.
	if (pImapCommand->GetType () != MBT_IMAP_ACCOUNT)
		return FALSE;

	// We only handle delete here.
	if( theAction != CA_DELETE )
		return FALSE;

	// We may eventually remove this directory.
	pImapCommand->GetObjectDirectory (Dirname);

	if (bDeleteLocalCache)
	{
		// Go delete local storage for all contained mailboxes. Don't delete from server.
		bResult = DeleteLocalChildMailboxes (pImapCommand, FALSE);

		// If we failed somewhere, don't continue.
		if (!bResult)
			return FALSE;
	}

	// OK, continue with the notifications.

	// This will remove menu and mailbox tree items.
	QCCommandDirector::NotifyClients( pImapCommand, theAction, NULL );

	// If we get here, it means we have already delete stuff for child mailboxes.
	// Go clean it up.

	// remove it from the list
	QCImapMailboxCommand *pParent = NULL;
	CPtrList			 *pList   = NULL;
	POSITION			  pos	  = NULL;
	pos = FindContainingList (pImapCommand, &pList, (QCMailboxCommand **) &pParent);
	if (!(pos && pList))
	{
		ASSERT (0);
		return FALSE;
	}

	pList->RemoveAt( pos );

	// Remove our internal knowledge of this account.
	// NOTE: This will serialize the list containing this account.
	g_ImapAccountMgr.DeleteAccount (pImapCommand->GetAccountID());

	// If bDeleteLocalCache, go delete the account's directory. Doesn't matter if we fail.

	if (bDeleteLocalCache)
	{
		if (!RemoveLocalDirectory (Dirname, TRUE))
		{
			ErrorDialog( IDS_ERR_DIR_DELETE, Dirname );
		}
	}

	// Finally, delete the object.
	delete pImapCommand;

	return TRUE;
}




// DeleteRemoteMailboxes [private]
// FUNCTION
// Delete mailbox and, possibly, it's child maiboxes, from the IMAP server.
// After every delete, a NOTIFY message is sent to clean local storage and remove menus, etc.
// END FUNCTION

BOOL QCMailboxDirector::DeleteRemoteMailboxes (QCImapMailboxCommand *pImapCommand, BOOL bDeleteChildren)
{
	CString		szMsg;

	if (!pImapCommand)
		return FALSE;

	// This MUST be a mailbox, not an ACCOUNT or NAMESPACE.
	if (pImapCommand->GetType() != MBT_IMAP_MAILBOX)
		return FALSE;

	// If this is a selectable mailbox, get it's toc if it's open so we can close it's viewers.
	// This will also close it's connection to the remote host so we can delete it.
	// Many IMAP servers don't allow deleting a selected mailbox. 

	// Note: We get the toc only if it's already open.
	CTocDoc *pTocDoc = GetToc (pImapCommand->GetPathname(), NULL, TRUE, TRUE);

	if (pTocDoc)
	{
		// Force all message windows to be closed as well
		int OldMSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
		SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, TRUE);

		// close all associated message windows -jdboyd
		pTocDoc->NotifyFrameClosing();

		// Don't delete the document itself at this point.

		// JDB - close all associated message windows.
		pTocDoc->NotifyFrameClosing();

		pTocDoc->m_bAutoDelete = FALSE;
		pTocDoc->OnCloseDocument();

		SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, ( short ) OldMSC);
	}

	// Remember that it's an error (rfc2060) to attempt to delete a mailbox that has
	// the NOSELECT attribute but has inferiors.
	
	// Go remove the child mailboxes (that we know about) first.
	if (bDeleteChildren)
	{
		CPtrList& theChildList = pImapCommand->GetChildList();

		POSITION posChild = theChildList.GetHeadPosition ();
		QCImapMailboxCommand *pImapCurrent;

		while (posChild)
		{
			// Remove it's children.
			pImapCurrent = ( QCImapMailboxCommand* ) theChildList.GetNext( posChild );
			if (pImapCurrent)
			{
				szMsg = pImapCurrent->GetName();
					
				pImapCurrent->Execute( CA_DELETE , &bDeleteChildren);

				// Did we succeed in deleting it?
				posChild = pImapCommand->GetChildList().GetHeadPosition();

				if( ImapFindByName( &( pImapCommand->GetChildList() ), szMsg ) )
				{
					return FALSE;
				}
			}
		}
	
		// The child list should be empty now. If it isn't, don't continue.
		if (!theChildList.IsEmpty())
		{
			return FALSE;
		}
	}

	// Ok. We have deleted the babies, go commit suicide now.
	// NOTE: Make sure we can delete the server mailbox before we continue.

	// NOTE: If the command object is marked as INVALID, it means that we know that it
	// doesn;t exist on the server. Dont attempt to delete from server.
	if (!pImapCommand->IsValid())
	{
		// Assume it's already been removed from the server.
		return TRUE;
	}

	// Attempt to delete from server.
	if ( !g_ImapAccountMgr.DeleteMailbox (pImapCommand->GetAccountID(), pImapCommand->GetImapName(), pImapCommand->GetDelimiter()) )
	{
		return FALSE;
	}

	// If we get here, success.
	return TRUE;
}



// DeleteLocalChildMailboxes [PRIVATE]
// FUNCTION
// Delete all contained mailboxes and their storage, then delete the account's storage.
// If "DeleteFromServer" is TRUE, delete the mailboxes from the server as well.
// END FUNCTION
BOOL QCMailboxDirector::DeleteLocalChildMailboxes (QCImapMailboxCommand *pImapCommand, BOOL bDeleteFromServer)
{
	CString		szMsg;
	BOOL		bDeleteChildren;

	if (!pImapCommand)
		return FALSE;

	// This MUST be an ACCOUNT.
	if (pImapCommand->GetType() != MBT_IMAP_ACCOUNT)
		return FALSE;

	// If we are not deleting mailboxes from the servre, loop through the child list and invalidate all children.
	if (!bDeleteFromServer)
		pImapCommand->InvalidateChildMailboxes (TRUE);

	// Now go remove them.

	CPtrList& theChildList = pImapCommand->GetChildList();

	POSITION posChild = theChildList.GetHeadPosition ();
	QCImapMailboxCommand *pImapCurrent;

	bDeleteChildren = TRUE;

	while (posChild)
	{
		// Remove it's children.
		pImapCurrent = ( QCImapMailboxCommand* ) theChildList.GetNext( posChild );
		if (pImapCurrent)
		{
			szMsg = pImapCurrent->GetImapName();
					
			pImapCurrent->Execute( CA_DELETE , &bDeleteChildren);

			// Did we succeed in deleting it?

			posChild = pImapCommand->GetChildList().GetHeadPosition();

			if( ImapFindByName( &( pImapCommand->GetChildList() ), szMsg ) )
			{
				// the child couldn't be deleted -- bail out of the whole thing
				ASSERT( 0 );
				return FALSE;
			}
		}
	}
	
	// The child list should be empty now. If it isn't, we failed.
	if (!theChildList.IsEmpty())
	{
		return FALSE;
	}

	return TRUE;
}




//
// NOTES
// Locates mailbox in the given account. 
// Also finds the account if pImapName matched the account's imap name, which
// may be empty!
// Note: Delimiter applies to the given ImapName and may be nul.
// NOTE: If "MarkValid", set the IMAP command object's valid flag if found.
// NOTE: BUG: Must do a case insensitive compare for INBOX.
// END NOTES
QCImapMailboxCommand*	QCMailboxDirector::ImapFindByImapName(
ACCOUNT_ID  AccountId,
LPCSTR		pImapName,
TCHAR		Delimiter)
{
	CPtrList*			pList;
	CString				szThisName;
	POSITION			pos;
	QCMailboxCommand*	pCurrent;
	QCImapMailboxCommand *pImapCurrent;
	QCImapMailboxCommand *pImapAccount;
	BOOL				bDoCaseInsensitive = FALSE;

	if (!(pImapName && AccountId))
		return NULL;

	// If we are looking for INBOX, do case insensitive compare.
	if (stricmp (pImapName, "INBOX") == 0)
		bDoCaseInsensitive = TRUE;

	// search for matching account ID first.
	pList = &m_theMailboxList;

	pos = pList->GetHeadPosition();
	pImapAccount = NULL;

	while( pos )
	{
		pCurrent = ( QCMailboxCommand* ) pList->GetNext( pos );
		
		if (pCurrent->IsImapType())
		{
			pImapAccount = (QCImapMailboxCommand *)pCurrent;
			if (pImapAccount->GetAccountID() == AccountId)
				break;
		}

		pImapAccount = NULL;
	}

	if (!pImapAccount)
		return NULL;

	// search through its childen.
	pList = &(pImapAccount->GetChildList());

	pos = pList->GetHeadPosition();

	while( pos )
	{
		pImapCurrent = ( QCImapMailboxCommand* ) pList->GetNext( pos );
		
		if (bDoCaseInsensitive)
		{
			if( stricmp( pImapCurrent->GetImapName(), pImapName ) == 0 )
			{
				return pImapCurrent;
			}
		}
		else
		{
			if( strcmp( pImapCurrent->GetImapName(), pImapName ) == 0 )
			{
				return pImapCurrent;
			}
		}

		// Can have imap names like mbox and mbox0 on the same level!
		// If pImapName contains no delimiter, we must have a perfect match at the current level.

		szThisName = pImapCurrent->GetImapName();
		if (Delimiter)
		{
			// Make sure we do case sensitive compares!
			if( (strlen (pImapName) > (size_t) szThisName.GetLength() ) &&
				( strncmp( szThisName, pImapName, szThisName.GetLength() ) == 0 ) &&
				( pImapName [szThisName.GetLength()] == Delimiter ) )
			{
				// found a parent
				pList = &( pImapCurrent->GetChildList() );
				pos = pList->GetHeadPosition();
			}
		}
	}

	return NULL;
}




// ImapFindParent [PRIVATE]
// FUNCTION
// Return a pointer to the command object list that contains the given mailbox command object.
// Also, return the parent command, or NULL if this is at the top level.
// END FUNCTION

QCImapMailboxCommand* QCMailboxDirector::ImapFindParent (QCImapMailboxCommand *pChild)
{
	CPtrList*			pList;
	QCMailboxCommand*	pParent = NULL;
	QCMailboxCommand*	pCurrent = NULL;
	POSITION			pos, next;
	CString				ChildPath, ParentPath;

	// find the parent and the appropriate list

	if (!pChild)
	{
		ASSERT( 0 );
		return NULL;
	}

	// We compare IMAP mailbox directories, not MBX file paths.
	((QCImapMailboxCommand *)pChild)->GetObjectDirectory(ChildPath);

	pList = &m_theMailboxList;

	pParent = NULL;

	pos = pList->GetHeadPosition();
	next = NULL;

	while( pos )
	{
		for( next = pos; pos ; pos = next )
		{
			pCurrent = ( QCMailboxCommand* ) pList->GetNext( next );
			if (!pCurrent)
				continue;

			// Is this an IMAP object?
			if (pCurrent->IsImapType())
			{
				((QCImapMailboxCommand *)pCurrent)->GetObjectDirectory(ParentPath);
			}
			else
				ParentPath = pCurrent->GetPathname();
				
			// Check for a match.
			if( ( pCurrent->GetType() == pChild->GetType() ) && 
				( ParentPath.CompareNoCase( ChildPath ) == 0 ) )

			{
				// Found it. Return.
				if (pParent && (pParent->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) != TRUE) )
				{
					ASSERT (0);
					return NULL;
				}

				// else:
				return ( QCImapMailboxCommand *) pParent;
			}

			if( (pCurrent->GetType() == MBT_FOLDER) || pCurrent->CanHaveChildren () )
			{
				// Might be in this directory?
				// Note: Can have an IMAP ACCOUNT under a non_IMAP folder!!
				if( ( ParentPath.GetLength() < ChildPath.GetLength() ) &&
					( strnicmp( ParentPath, ChildPath, strlen( ParentPath ) ) == 0 ) &&
					( ChildPath.GetAt ( ParentPath.GetLength() ) == '\\' ) )
				{
					pParent = pCurrent;	
					pList = & ( pCurrent->GetChildList() );
					pos = pList->GetHeadPosition();
					break;	// Out of for.
				}
			}
		}
	}

	// If we get here, we failed.
	return NULL;
}





// HandleRenameImapMailbox [PRIVATE]
//
// FUNCTION
// Renamed the mailbox on the remote server and do the local housekeeping to
// reflect that change.
// 
// NOTE: We do NOT allow a change of name that may contain the mailbox
// hierarchy delimiter! In otherwords, the new mailbox lies within the same hierarchy.
// 
// NOTE: "The object pointed to by ppCommand MAY BE DELETED internally, so be 
// CAREFULL!!
//
// END FUNCITON
//
BOOL QCMailboxDirector::HandleRenameImapMailbox (QCMailboxCommand **ppCommand,
									 COMMAND_ACTION_TYPE theAction, void* pData)
{
	BOOL bResult = FALSE;

	// Sanity:
	if (! (ppCommand && *ppCommand) )
	{
		ASSERT (0);
		return FALSE;
	}

	// This must be a QCImapMailboxCommand!!
	if( !(*ppCommand)->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Use pImapCommand from here on.
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *) (*ppCommand);

	// We handle renaming of mailboxes only.
	if (pImapCommand->GetType () != MBT_IMAP_MAILBOX)
	{
		ASSERT (0);
		return FALSE;
	}

	//
	// Make sure we have a new name string.
	//
	if (!pData)
	{
		ASSERT (0);
		return FALSE;
	}

	// This is the new name.
	CString szNewName = (LPCSTR)pData;

	// 
	// Get the IMAP account object containing this mailbox.
	// 
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (pImapCommand->GetAccountID());
	if (!pAccount)
	{
		ASSERT (0);
		return FALSE;
	}


	// If the mailbox or any of its child maiboxes are open, go close them.
	CloseMailboxes (pImapCommand, TRUE);

	//
	// Put up progress bar here so we don't have several popping up and
	// shutting down as we proceed.
	//
	if ( ::IsMainThreadMT() )
	{
		MainProgress(" ");
	}


	// Ask the account owning this mailbox to rename the mailbox.
	// If successful, it will move the local mailbox cache to a new direcrtory.
	// This will also move child mailboxes.
	// The new directory name will be returned in "szNewDirname". Note that this is
	// the relative name only, not the full pathname.
	//
	// There is a catch - some servers (cyrus) may retain the old mailbox and create
	// a new sibling!! In this case, RenameMailbox sets "bOldMboxStillExists" to TRUE
	// and creates a new command object that's returned in "pNewSibking".
	//
	CString szNewDirname;
	BOOL bOldMailboxStillExists = FALSE;
	QCImapMailboxCommand *pNewSibling = NULL;

	bResult = pAccount->RenameMailbox ( pImapCommand, szNewName, szNewDirname, bOldMailboxStillExists, &pNewSibling );

	// Did we succeed?
	if (!bResult)
	{
		CloseProgress();
		return FALSE;
	}

	// Now, if the mailbox still exists, go see if it is no longer 
	// selectable and notify the mailbox tree.
	if (bOldMailboxStillExists)
	{
		if ( pImapCommand->IsNoSelect () )
		{
			// BUG: We should notify the mailbox tree to change it's display icon.
		}

		// If a new mailbox was created, link that in and initialize it.
		if (pNewSibling)
		{
			//
			// Go get the parent mailbox list of "pImapCommand" so we can update the serialized mailbox lists.
			// Note: pNewSibling and pImapCommand MUST have the same parent command!
			// 
			QCImapMailboxCommand *pParent = NULL;
			POSITION			  pos	  = NULL;

			pParent = ImapFindParent (pImapCommand);
	
			if (!pParent)
			{
				ASSERT (0);
				bResult = FALSE;
			}
			else
			{
				// Link in.
				//
				Insert (pParent->GetChildList(), (QCMailboxCommand *)pNewSibling);

				ImapNotifyClients( pNewSibling, CA_NEW, NULL );

				// Re-serialize. NOTE: no need to re-write child lists.

				CString szParentPath;
				pParent->GetObjectDirectory (szParentPath);

				WriteImapDescMapFile( szParentPath, pParent->GetChildList (), FALSE);

				bResult = TRUE;
			}
		}
	}
	else
	{
		bResult = UpdateAfterRename (pImapCommand, szNewName, szNewDirname);
	}

	// Make sure:
	CloseProgress();

	return bResult;
}





////////////////////////////////////////////////////////////////////////
// UpdateAfterRename [PRIVATE]
// FUNCTION
// If a mailbox was really renamed and the old name no longer exists,
// call this to perform the cleanup actions.
// END FUNCTION
// //////////////////////////////////////////////////////////////////////
BOOL QCMailboxDirector::UpdateAfterRename (QCImapMailboxCommand *pImapCommand, LPCSTR pNewName, LPCSTR pNewDirname)
{ 
	char	szDrive[ _MAX_DRIVE ];
	char	szDir[ 1024 ];
	char	szFname[ 256 ];
	char	szExt[ _MAX_EXT ];

	// Sanity:
	if ( ! (pImapCommand && pNewName && pNewDirname) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Send the CA_RENAME notification before we change the info in the command object!!!
	// The Notification handlers need to locate the command object by the old name so they
	// can rename menu items, etc.
	QCCommandDirector::NotifyClients( pImapCommand, CA_RENAME, (void *) pNewName );


	// Extract the parent directiory containing the object directory of pImapCommand.
	//
	CString szOldPathname;
	pImapCommand->GetObjectDirectory (szOldPathname);

	_splitpath( szOldPathname, szDrive, szDir, szFname, szExt );
		
	CString szNewPathname = szDrive;
	szNewPathname += szDir;		

	// Set the new friendly name.
	pImapCommand->SetName( pNewName );

	// Set the new IMAP name.
	CString szNewFullImapName = pImapCommand->GetImapName();
	TCHAR	Delimiter = pImapCommand->GetDelimiter();

	int nc = szNewFullImapName.ReverseFind (Delimiter);
	if (nc < 0)
	{
		// This is a top level mailbox. Replace complete name.
		szNewFullImapName = pNewName;
	}
	else
	{
		// Replace basename.
		szNewFullImapName = szNewFullImapName.Left (nc + 1);
		szNewFullImapName += pNewName;
	}

	// Set the new full IMAP name.
	pImapCommand->SetImapName (szNewFullImapName);

	// Set the new pathname to the full path of the MBX file within the object
	// directory.
	pImapCommand->SetPathname( szNewPathname + pNewDirname + 
					DirectoryDelimiter + pNewDirname +  CRString (IDS_MAILBOX_EXTENSION) );

	// Recurse through child mailboxes.
	UpdateChildrenAfterRename (pImapCommand);


#if 0 // 

	//
	// We nay need to update filters.
	//
	if( uRet == IDC_UPDATE_FILTER )
	{
		FiltersUpdateMailbox( szOldPathname, szNewPathname + szNewDirname, pNewName, pImapCommand->CanHaveChildren() );
	}

#endif //


	//
	// Go get the parent mailbox so we can update the serialized mailbox lists.
	// 
	QCImapMailboxCommand *pParent = NULL;

	pParent = ImapFindParent (pImapCommand);

	if (!pParent)
	{
		ASSERT (0);
		return FALSE;
	}

	// NOTE: Re-write child lists as well.
	WriteImapDescMapFile( szNewPathname, pParent->GetChildList(), TRUE);

	// Resort among siblings:
	if( pImapCommand->GetType() == MBT_IMAP_MAILBOX )
	{
		QCCommandDirector::NotifyClients( pImapCommand, CA_SORT_AFTER_RENAME, (void *) pNewName );
	}

	return TRUE;
}






// HandleMoveImapMailbox [PRIVATE]
//
// FUNCTION
// Rename the mailbox on the remote server and do the local housekeeping to
// reflect that change.
// 
// NOTE: "pNewParentCommand" is the command object representing the new parent.
//
// NOTE: "The object pointed to by pCommand MAY BE DELETED internally, so be 
// CAREFULL!!
//
// END FUNCITON
//
BOOL QCMailboxDirector::HandleMoveImapMailbox (QCMailboxCommand **ppCommand,
						 COMMAND_ACTION_TYPE theAction,
						 QCMailboxCommand *pNewParentCommand)
{
	BOOL bResult = FALSE;

	// Sanity:
	if (! ((ppCommand && *ppCommand) && pNewParentCommand) )
	{
		ASSERT (0);
		return FALSE;
	}

	// This must be a QCImapMailboxCommand!!
	if( !(*ppCommand)->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
	{
		ASSERT (0);
		return FALSE;
	}

	// This must be a QCImapMailboxCommand!!
	if( !pNewParentCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Use pImapCommand from here on.
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *) (*ppCommand);

	// We handle renaming of mailboxes only.
	if (pImapCommand->GetType () != MBT_IMAP_MAILBOX)
	{
		ASSERT (0);
		return FALSE;
	}

	// Use pNewParentImapCommand from here on.
	QCImapMailboxCommand *pNewParentImapCommand = (QCImapMailboxCommand *) pNewParentCommand;

	// 
	// Get the IMAP account object containing this mailbox.
	// 
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (pImapCommand->GetAccountID());
	if (!pAccount)
	{
		ASSERT (0);
		return FALSE;
	}

	// If the mailbox or any of its child maiboxes are open, go close them.
	//
	CloseMailboxes (pImapCommand, TRUE);

	// Save these flags here:
	BOOL bHasChildMailboxes = !pImapCommand->GetChildList().IsEmpty();

	//
	// Put up progress bar here so we don't have several popping up and
	// shutting down as we proceed.
	//
	if ( ::IsMainThreadMT() )
	{
		MainProgress(" ");
	}

	// Ask the account owning this mailbox to move the mailbox.
	// There are some optimizations here, based on the various possibilities:
	// 1. If the mailbox being moved does not have child mailboxes and 
	//    the rename succeeded, the account creates a new command object that must be
	//    linked to the target parent.
	//
	// 2. If the mailbox being moved has child mailboxes, no command object is 
	//    created and so we have to refresh both subtrees. In this case,
	//    CImapAccount::MoveMailbox() return the command object whose subtree
	//    must be refreshed. This is done because when a mailbox is renamed,
	//    ghosted parent mailboxes may be deleted by the server.
	//


	CString szNewDirname;
	BOOL bOldMailboxStillExists = FALSE;
	QCImapMailboxCommand *pNewImapCommand = NULL;
	QCImapMailboxCommand *pNewLastParent  = NULL;

	bResult = pAccount->MoveMailbox ( pImapCommand, pNewParentImapCommand,
									  bOldMailboxStillExists, &pNewLastParent,
									  &pNewImapCommand );

	// Did we succeed?
	if (!bResult)
	{
		CloseProgress();
		return FALSE;
	}

	// If a new mailbox was created, link that in and initialize it.
	// Then, if the old mailbox had child mailboxes, refresh it's child list.
	//
	if (pNewImapCommand)
	{
		// Link it into the command object's child list.
		//
		Insert (pNewParentImapCommand->GetChildList(), (QCMailboxCommand *)pNewImapCommand);

		ImapNotifyClients( pNewImapCommand, CA_NEW, NULL );

		if (bHasChildMailboxes)
		{
			// Refresh child list.
			BOOL ViewNeedsRefresh = TRUE;

			pNewParentCommand->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

			UpdateImapMailboxLists (pNewImapCommand, TRUE, TRUE);
		}
	}

	// Re-serialize. NOTE: re-write child lists.

	CString szParentPath;
	pNewParentImapCommand->GetObjectDirectory (szParentPath);

	WriteImapDescMapFile( szParentPath, pNewParentImapCommand->GetChildList (), TRUE);

	// If the old mailbox no longer exists, delete pImapCommand.
	// If it still exists, go see if it is no longer 
	// selectable and notify the mailbox tree.
	//
	if (bOldMailboxStillExists)
	{
		if ( pImapCommand->IsNoSelect () )
		{
			// BUG: We should notify the mailbox tree to change it's display icon.
		}
	}
	else
	{
		// Go delete local storage for all contained mailboxes. Don't delete from server.
		DeleteLocalChildMailboxes (pImapCommand, FALSE);

		// This will remove menu and mailbox tree items.
		QCCommandDirector::NotifyClients( pImapCommand, CA_DELETE, NULL );

		// Remove the mailbox's local storage structures.
		// NOTE: Still continue if we didn't delete the directory.
		CString Dirname;
		pImapCommand->GetObjectDirectory ( Dirname );

		if (!RemoveLocalDirectory (Dirname, TRUE))
		{
			ErrorDialog( IDS_ERR_DIR_DELETE, Dirname );
			// Fall through.
		}

		// Check to see if any filters get affected.
		//
		// If there are filters which involve this mailbox or it's children, 
		// ask user if to continue.
		unsigned int uRet = DoWeUpdateFilters (pImapCommand, CA_DELETE );

		if( uRet == IDC_UPDATE_FILTER )
		{
			FiltersUpdateMailbox( pImapCommand->GetPathname(), NULL, NULL, FALSE );
		}

		// remove it from the list
		QCImapMailboxCommand *pParent = NULL;
		CPtrList			 *pList   = NULL;
		POSITION			  pos	  = NULL;
		pos = FindContainingList (pImapCommand, &pList, (QCMailboxCommand **) &pParent);
		if (!(pos && pList))
		{
			ASSERT (0);
			CloseProgress();

			return FALSE;
		}

		pList->RemoveAt( pos );

		// Must do this last!
		delete pImapCommand;

		// We can now do this.
		pImapCommand = NULL;

		// Make sure we do this if we deleted pImapCommand.
		//
		*ppCommand = NULL;
	}

	// Refresh source tree.
	// Note: pImapComamnd may now be NULL.
	//
	// If pImapComamnd is still there, just refresh its list.
	//
	if (pImapCommand && bHasChildMailboxes)
	{
		BOOL ViewNeedsRefresh = TRUE;

		pImapCommand->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

		UpdateImapMailboxLists (pImapCommand, TRUE, TRUE);
	}
	else if (pNewLastParent)
	{
		BOOL ViewNeedsRefresh = TRUE;

		pNewLastParent->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

		UpdateImapMailboxLists (pNewLastParent, TRUE, TRUE);
	}

	// Resort:
	//
	if (bResult && pNewParentCommand)
		ResortChildren ((QCImapMailboxCommand *)pNewParentCommand, TRUE);

	// Make sure:
	if ( ::IsMainThreadMT() )
	{
		CloseProgress();
	}

	return bResult;
}





// FindContainingList [PRIVATE]
// FUNCTION
// Return a pointer to the command object list that contains the given mailbox command object.
// Also, return the parent command, or NULL if this is at the top level.
// END FUNCTION

POSITION QCMailboxDirector::FindContainingList (QCMailboxCommand *pChild, CPtrList **ppList, QCMailboxCommand **ppParent)
{
	CPtrList*			pList;
	QCMailboxCommand*	pParent;
	QCMailboxCommand*	pCurrent;
	POSITION			pos, next;
	CString				ChildPath, ParentPath;

	// find the parent and the appropriate list

	if (!pChild)
	{
		ASSERT( 0 );
		return FALSE;
	}

	// We compare IMAP mailbox directories, not MBX file paths.
	((QCImapMailboxCommand *)pChild)->GetObjectDirectory(ChildPath);

	pList = &m_theMailboxList;

	pParent = NULL;

	pos = pList->GetHeadPosition();
	next = NULL;

	while( pos )
	{
		for( next = pos; pos ; pos = next )
		{
			pCurrent = ( QCMailboxCommand* ) pList->GetNext( next );
			if (!pCurrent)
				continue;

			// Is this an IMAP object?
			if (pCurrent->IsImapType())
			{
				((QCImapMailboxCommand *)pCurrent)->GetObjectDirectory(ParentPath);
			}
			else
				ParentPath = pCurrent->GetPathname();
				
			// Check for a match.
			if( ( pCurrent->GetType() == pChild->GetType() ) && 
				( ParentPath.CompareNoCase( ChildPath ) == 0 ) )

			{
				// Found it. Return.
				if (ppParent)
					*ppParent = pParent;
				if (ppList)
					*ppList = pList;
				return pos;
			}

			if( (pCurrent->GetType() == MBT_FOLDER) || pCurrent->CanHaveChildren () )
			{
				// Might be in this directory?
				// Note: Can have an IMAP ACCOUNT under a non_IMAP folder!!
				if( ( ParentPath.GetLength() < ChildPath.GetLength() ) &&
					( strnicmp( ParentPath, ChildPath, strlen( ParentPath ) ) == 0 ) &&
					( ChildPath.GetAt ( ParentPath.GetLength() ) == '\\' ) )
				{
					pParent = pCurrent;	
					pList = & ( pCurrent->GetChildList() );
					pos = pList->GetHeadPosition();
					break;	// Out of for.
				}
			}
		}
	}

	// If we get here, we failed.
	if (ppParent)
		*ppParent = NULL;
	if (ppList)
		*ppList = NULL;

	return NULL;
}





// ImapFindByName [PUBLIC]
// FUNCTION
// Find the command object within the given list that has the given name.
// Does case sensitive compare!!
// END FUNCTION

QCMailboxCommand*	QCMailboxDirector::ImapFindByName(
CPtrList*	pTheList,
LPCSTR		szName )
{
	POSITION			pos;
	QCMailboxCommand*	pCommand;

	if( pTheList == NULL )
	{
		pTheList = & m_theMailboxList;
	}

	pos = pTheList->GetHeadPosition();
	while( pos )
	{
		pCommand = ( QCMailboxCommand* ) pTheList->GetNext( pos );

		// For IMAP, must do case sensitive compare,
		if( strcmp( pCommand->GetName(), szName ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}




/////////////////////////////////////////////////////////////////////
// UpdateChildrenAfterRename
//
// Update the imap names and pathnames in any child mailboxes of the given object.
//
// NOTES
// The command object itself has already been renamed. Use its values in renaming
// child objects.
// END NOTES
//
////////////////////////////////////////////////////////////////////
void QCMailboxDirector::UpdateChildrenAfterRename (QCImapMailboxCommand *pImapCommand)
{
	// Sanity
	if ( !pImapCommand)
	{
		ASSERT (0);
		return;
	}

	//
	// Extract the directory containing the given object. All child objects should
	// have this as the containing directory.
	//
	CString szParentPathname;
	pImapCommand->GetObjectDirectory (szParentPathname);

	// Terminate it with a directory delimiter.
	if (szParentPathname.Right (1) != DirectoryDelimiter)
		szParentPathname += DirectoryDelimiter;

	// Now format the parent IMAP hierarchy name,
	CString szParentImapName = pImapCommand->GetImapName();

	//
	// Terminate it with a delimiter.
	//
	TCHAR Delimiter = pImapCommand->GetDelimiter();

	// MUST have a delimiter!!
	if (!Delimiter)
	{
		ASSERT (0);
		return;
	}

	if (szParentImapName.Right (1) != Delimiter)
		szParentImapName += Delimiter;

	// Ok Loop through children and modify.
	CPtrList& childList = pImapCommand->GetChildList();

	POSITION pos = childList.GetHeadPosition();

	while (pos)
	{
		QCImapMailboxCommand *pCurrent = (QCImapMailboxCommand *)childList.GetNext (pos);

		if (pCurrent)
		{
			// Extract base IMAP name.
			CString szBaseImapName = pCurrent->GetImapName();

			int nc = szBaseImapName.ReverseFind (Delimiter);
			if (nc >= 0)
				szBaseImapName = szBaseImapName.Mid (nc + 1);
			
			// Set new full IMAP name. No need to change the friendly name.
			pCurrent->SetImapName (szParentImapName + szBaseImapName);

			// Extract base MBX filename. (This copies the contents).
			CString szBaseFilename = pCurrent->GetPathname ();

			nc = szBaseFilename.ReverseFind (DirectoryDelimiter);

			if (nc >= 0)
				szBaseFilename = szBaseFilename.Mid (nc + 1);

			// For IMAP, the MBX file is inside a directory with the same basename
			// as the MBX file. Get the name of that directory:
			//
			CString szDirname = szBaseFilename;
			nc = szDirname.Find ('.');
			if (nc > 0)
				szDirname = szDirname.Left (nc);

			// Set the new full path.
			pCurrent->SetPathname (szParentPathname + szDirname + 
									DirectoryDelimiter + szBaseFilename);

			// Need to re-initialize the IMAP mailbox's local cache.
			ReInitializeImapMbox (pCurrent, CA_RENAME);

			// Update children.
			UpdateChildrenAfterRename (pCurrent);
		}
	}
}




///////////////////////////////////////////////////////////////////////////////////
// RemoveImapMailboxCommand [PUBLIC]
//
// Go remove our knowledge of "pImapComamnd" from it's parent command object.
// This involves removing local mailbox storage, removing the tree control items, 
// and finally, removing it from it's parent.
// NOTE: This ALWAYS RECURSES.
// The object itself is also deleted.
// 
// NOTE: If "bDeleteFromServer" is TRUE, go delete the mailbox from the server before
// deleting local storage (NOT YET IMPLEMENTED!).
///////////////////////////////////////////////////////////////////////////////////
void QCMailboxDirector::RemoveImapMailboxCommand (QCImapMailboxCommand *pImapCommand, BOOL bDeleteFromServer /* = FALSE */, BOOL bUpdateMboxList /* = FALSE */)
{
	QCMailboxCommand	*pParent = NULL;
	POSITION			pos, next;

	// Sanity.
	if (!pImapCommand)
		return;

	// Make sure we can find it's parent command.
	//
	CPtrList *pList = NULL;		// We don't use pList this here.

	FindContainingList (pImapCommand, &pList, (QCMailboxCommand **) &pParent);

	if (!pParent)
		return;

	// Get the child list and do a pre-order recursion.
	CPtrList& ChildList = pImapCommand->GetChildList();

	// Loop through it.
	pos = ChildList.GetHeadPosition();

	for (next = pos; pos; pos = next)
	{
		QCImapMailboxCommand *pCurrent = (QCImapMailboxCommand *) ChildList.GetNext (next);

		if (pCurrent)
			RemoveImapMailboxCommand (pCurrent);
	}

	// Ok. We have removed it's children. remove itself.

	// Get the toc if it's open so we can close it's viewers.
	CTocDoc *pTocDoc = GetToc (pImapCommand->GetPathname(), NULL, TRUE, TRUE);
	if (pTocDoc)
	{
		// Force all message windows to be closed as well
		int OldMSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
		SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, TRUE);
		pTocDoc->OnCloseDocument();
		SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, ( short ) OldMSC);

		// Make sure we don't use this below.
		pTocDoc = NULL;
	}
		
	// Remove the mailbox's local storage structures.
	// NOTE: Still continue if we didn't delete the directory.
	CString Dirname;
	pImapCommand->GetObjectDirectory ( Dirname );
	if (!RemoveLocalDirectory (Dirname, TRUE))
	{
		ErrorDialog( IDS_ERR_DIR_DELETE, Dirname );
		// Fall through.
	}

	// Check to see if any filters get affected and forcibly update the filters.
	FiltersUpdateMailbox( pImapCommand->GetPathname(), NULL, NULL, FALSE );

	// This will remove menu and mailbox tree items.
	QCCommandDirector::NotifyClients( pImapCommand, CA_DELETE, NULL );

	// Locate the command in it's parent list and remove it.
	CPtrList& ParentList = pParent->GetChildList();

	pos = ParentList.GetHeadPosition ();

	// Do this so we can verify removal.
	BOOL bFound = FALSE;

	for (next = pos; pos; pos = next)
	{
		QCImapMailboxCommand *pCurrent = (QCImapMailboxCommand *)ParentList.GetNext (next);

		if (pCurrent == pImapCommand)
		{
			// Found it.
			bFound = TRUE;
			ParentList.RemoveAt( pos );
			break;
		}
	}

	// This should never fire.
	ASSERT (bFound);

	// Can free the command object's memory now.!
	delete pImapCommand; pImapCommand = NULL;

	// Re-serialize  parent's mailbox list>
	if (bUpdateMboxList && pParent && 
			pParent->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
	{
		CString Dir;

		((QCImapMailboxCommand *)pParent)->GetObjectDirectory (Dir);

		WriteImapDescMapFile( Dir, pParent->GetChildList() );
	}
}




// ======================== Internal routines ===========================/



// ProcessMailboxRecord [Internal]
// Parse a single line from the mailboxes list file.
	//
	// Each line of the Mailbox list file is of the following form:
	// ImapName, Directory name (name only - no path), TypeChar, DelimiterChar, NoSelect, \
	// NoInferiors, Marked, Unmarked, HasUnread
	// NOTES:
	// TypeChar is : N = NameSpace, M = Mailbox. A Mailbox may contain additional mailboxes.
	// TypeChar, NoSelect, NoInferiors, Marked, Unmarked have values of 1 or 0.
	// HasUnread is as in local mailboxes.
	//				

	//
	// Process the entire mailboxlist file, one line at a time.
	//
//

BOOL ProcessMailboxRecord (char *buf, MailboxRecord *pMboxRec)
{
	char	*p;

	// Make sure we have valid parameters.
	if (!(buf && pMboxRec))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	// Parse buf. 		

	// ImapName is the first string in the line.
	pMboxRec->pImapName = buf;

	// Local directory name for this mailbox.
	p = strchr( buf, ',' );
		
	if (!p )
	{
		ASSERT(FALSE);
		return FALSE;
	}

	*p++ = 0;
	pMboxRec->pDirname = p;

	// Type char.
	p  = strchr( p, ',' );
	if (!p )
	{
		ASSERT(FALSE);
		return FALSE;
	}
	*p++ = 0;
	// TypeChar is: M = mailbox, N = namespace.
	pMboxRec->theType = MBT_IMAP_MAILBOX;
			
	switch( toupper( *p ) )
	{
		case 'M':
			pMboxRec->theType = MBT_IMAP_MAILBOX; 
		break;
		case 'N': 
			pMboxRec->theType = MBT_IMAP_NAMESPACE; 
		break;
		case 'A': 
			pMboxRec->theType = MBT_IMAP_ACCOUNT; 
		break;

		default:
			// Better not get here!
			ASSERT(FALSE);
			return FALSE;
	}

	// Delimiter char.
	p  = strchr( p, ',' );
	if (!p )
	{
		ASSERT(FALSE);
		return FALSE;
	}
	*p++ = 0;
	pMboxRec->DelimiterChar = *p;


	// Determine NoSelect status.
	p  = strchr (p, ',');

	if (! p )
	{
		ASSERT(FALSE);
		return FALSE;
	}
	*p++ = 0;		
			
	pMboxRec->bNoselect = FALSE;

	switch( *p )
	{
		case '1':
			pMboxRec->bNoselect = TRUE; 
		break;
		case '0': 
			pMboxRec->bNoselect = FALSE;
		break;
		default:
			// Better not get here!
			ASSERT(FALSE);
			return FALSE;
	}


	// Determine NoInferiors status.
	p  = strchr (p, ',');

	if (! p )
	{
		ASSERT(FALSE);
		return FALSE;
	}
	*p++ = 0;		
			
	pMboxRec->bNoInferiors = FALSE;

	switch( *p )
	{
		case '1':
			pMboxRec->bNoInferiors = TRUE; 
		break;
		case '0': 
			pMboxRec->bNoInferiors = FALSE;
		break;
		default:
			// Better not get here!
			ASSERT(FALSE);
			return FALSE;
	}
	
	// Determine Marked status.
	p = strchr (p, ',');

	if (!p )
	{
		ASSERT(FALSE);
		return FALSE;
	}
			
	*p++ = 0;
			
	pMboxRec->bMarked = FALSE;

	switch( *p )
	{
		case '1':
			pMboxRec->bMarked = TRUE;
		break;
		case '0': 
			pMboxRec->bMarked = FALSE;
		break;
		default:
			// Better not get here!
			ASSERT(FALSE);
			return FALSE;
	}

	// Determine UnMarked status.
	p = strchr (p, ',');

	if (!p )
	{
		ASSERT(FALSE);
		return FALSE;
	}
			
	*p++ = 0;
			
	pMboxRec->bUnmarked = FALSE;

	switch( *p )
	{
		case '1':
			pMboxRec->bUnmarked = TRUE;
		break;
		case '0': 
			pMboxRec->bUnmarked = FALSE;
		break;
		default:
			// Better not get here!
			ASSERT(FALSE);
			return FALSE;
	}

	// Unread Status.
	pMboxRec->theStatus = US_UNKNOWN;
	p = strchr( p, ',' );
	if (!p )
	{
		ASSERT(FALSE);
		return FALSE;
	}

	*p++ = 0;

	switch( toupper( *p ))
	{
		case 'U': 
			pMboxRec->theStatus = US_UNKNOWN;	
		break;
		case 'Y': 
			pMboxRec->theStatus = US_YES;		
		break;
		case 'N': 
			pMboxRec->theStatus = US_NO;			
		break;
		default: 
			ASSERT(FALSE);
			return FALSE;
		break;
	}

	return TRUE;
}




///////////////////////////////////////////////////////
// ExtractNameAndDirname
//
// Extract the new command object's friendly name.
// "pData" is a pointer to a string containing the new object's friendly
// name followed by the new directory name. The format is as follows:
//
//		"friendly name""new relative directory name"
//
// including the double quotes. NOTE: the names are delimited by the
// double quotes!!!
///////////////////////////////////////////////////////
BOOL ExtractNameAndDirname (LPCSTR pBuffer, CString& szNewName, CString& szNewDirname)
{
	BOOL bResult = FALSE;
	CString str;

	// Sanity
	if (!pBuffer)
	{
		ASSERT (0);
		return FALSE;
	}


	LPCSTR p = pBuffer;

	for (int i = 0; p && *p && (i < 2); i++)
	{
		p = strchr (p, '"');

		if (!p)
		{
			// Didn't succeed.
			break;
		}

		// Skip ".
		str = ++p;

		LPSTR q = strchr (p, '"');
		if (!q)
		{
			// We didn't succeed.
			break;
		}

		// Copy to str.
		str = str.Left ( q - p);

		// Prepare for next word.
		p = ++q;

		// Which word is this??
		if ( i == 0)
			szNewName = str;
		else
		{
			szNewDirname = str;

			// If we get here, we succeeded.
			bResult = TRUE;
		}
	}

	// Did we succeed?			
	if (!bResult)
	{
		// Empty these:
		szNewName.Empty();
		szNewDirname.Empty();
	}

	return bResult;
}




unsigned int DoWeUpdateFilters (QCMailboxCommand *pCommand, COMMAND_ACTION_TYPE theAction )
{
	CString szMsg;
	unsigned int uRet = IDC_LEAVE_FILTER;

	if( FilterMBoxLink( pCommand->GetPathname(), pCommand->CanHaveChildren() ) )
	{		
		if( theAction == CA_DELETE )
		{
			szMsg.Format( ( LPCSTR )CRString(IDS_DELETE_FILTER), ( LPCSTR ) pCommand->GetName() );
		}
		else
		{
			szMsg.Format( ( LPCSTR )CRString(IDS_UPDATE_FILTER), ( LPCSTR ) pCommand->GetName() );
		}

		uRet = AlertDialog( IDD_UPDATE_FILTERS, ( const char* ) szMsg );
	}

	return uRet;
}



/////////////////////////////////////////////////////////////////////////////
// CloseMailboxesBeforeRaname ()
//
// If the TOC for the given mailbox command is open. close it now.
// If "bRecurse" is TRUE, close child mailbox TOC's as well.
/////////////////////////////////////////////////////////////////////////////
void CloseMailboxes (QCImapMailboxCommand *pImapCommand, BOOL bRecurse)
{
	// Sanity.
	if (!pImapCommand)
		return;

	// Get the toc if it's open.
	CTocDoc *pTocDoc = GetToc (	pImapCommand->GetPathname(), NULL, FALSE, TRUE );

	if (pTocDoc)
	{
		// Force all message windows to be closed as well
		int OldMSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
		SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, TRUE);

		// close all associated message windows -jdboyd
		pTocDoc->NotifyFrameClosing();

		// Force it to actually be deleted.
		pTocDoc->m_bAutoDelete = TRUE;
		pTocDoc->OnCloseDocument();

		SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, ( short ) OldMSC);

		// Make sure we don't use this below.
		pTocDoc = NULL;
	}

	// Close child mailboxes?
	if (bRecurse)
	{
		CPtrList& theChildList = pImapCommand->GetChildList();

		POSITION posChild = theChildList.GetHeadPosition ();
		QCImapMailboxCommand *pImapCurrent;

		while (posChild)
		{
			// Close its connection..
			pImapCurrent = ( QCImapMailboxCommand* ) theChildList.GetNext( posChild );
			if (pImapCurrent)
			{
				CloseMailboxes (pImapCurrent, TRUE);
			}
		}
	}
}


// Instantiate a temporary CImapMailbox object to Initialize the mailbox's local storage.
// This is required after a RENAME.
//

static void ReInitializeImapMbox (QCImapMailboxCommand *pImapCommand,
						COMMAND_ACTION_TYPE	theAction)
{
	// Sanity:
	if (!pImapCommand)
		return;

	CString Mboxdir;


	// Make sure the directory exists. Not for accounts.
	if ( (pImapCommand->GetType() != MBT_IMAP_ACCOUNT) && pImapCommand->GetObjectDirectory (Mboxdir))
	{
		// 
		CImapMailbox ImapMbox (pImapCommand->GetAccountID());

		ImapMbox.SetImapName ( pImapCommand->GetImapName() );
		ImapMbox.SetDirname (Mboxdir);
		ImapMbox.SetDelimiter ( pImapCommand->GetDelimiter() );
		ImapMbox.SetNoInferiors ( pImapCommand->IsNoInferiors() );
		ImapMbox.SetMarked ( pImapCommand->IsMarked() );
		ImapMbox.SetUnMarked ( pImapCommand->IsUnmarked() );
		ImapMbox.SetImapType ( pImapCommand->MBTTypeToImapType (pImapCommand->GetType()) );

		ImapMbox.InitializeLocalStorage( (theAction == CA_NEW) || (theAction == CA_RENAME));
	}
}



// Resort child items in the mailbox tree.
// if "bRecurse", do children of childern as well.
//
static void ResortChildren (QCImapMailboxCommand *pParentImapCommand, BOOL bRecurse)
{
	if (pParentImapCommand)
	{
		CPtrList* pList = &(pParentImapCommand->GetChildList());
		POSITION pos   = pList->GetHeadPosition();
		
		while (pos)
		{
			QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pList->GetNext (pos);

			if (pImapCommand)
			{
				pImapCommand->Execute ( CA_SORT_AFTER_RENAME,
							(void *) pImapCommand->GetName() );

				if ( bRecurse )
					ResortChildren (pImapCommand, bRecurse);
			}
		}
	}
}


// static
BOOL TocHasUnreadSummaries (CTocDoc *pTocDoc)
{
	BOOL bResult = FALSE;

	if (!pTocDoc)
		return FALSE;

	long FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;

	POSITION pos = pTocDoc->m_Sums.GetHeadPosition();

	while (pos)
	{
		CSummary *pSum = (CSummary *) pTocDoc->m_Sums.GetNext(pos);

		if (pSum)
		{
			UnreadStatusType UnreadStatus = US_NO;

			if (UnreadStatus == US_NO && pSum->m_State == MS_UNREAD && pSum->m_Seconds > FreshnessDate)
				UnreadStatus = US_YES;

			if (UnreadStatus == US_YES)
			{
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}



#endif // IMAP4

