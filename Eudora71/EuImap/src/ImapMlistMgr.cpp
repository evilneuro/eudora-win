/////////////////////////////////////////////////////////////////////////////
// 	imapmlst.cpp - The CImapMlistMgr class implementation file.


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "cursor.h"
#include "resource.h"
#include "rs.h"
#include "fileutil.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"

#include "ImapMlistMgr.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"

#include "DebugNewHelpers.h"

// =========== Internal routines ==================//

//================================= CImapMlistMgr class =====================//

// Constructor
// NOTES
// Must have valid AccountID, pPathname and pParentImapname.
// END NOTES
CImapMlistMgr::CImapMlistMgr(ACCOUNT_ID AccountID, LPCSTR pPathname, LPCSTR pParentImapname,
		 TCHAR Delimiter, ImapMailboxType Type)
{
	m_AccountID			= AccountID;
	m_Pathname			= pPathname;
	m_ParentImapname	= pParentImapname;
	m_Delimiter			= Delimiter;
	m_Type				= Type;
	m_bNeedsResync		= FALSE;
}



CImapMlistMgr::~CImapMlistMgr()
{
}



// UpdateMailboxLists [PUBLIC]
// NOTES
// Fetch mailbox lists. If "ALL" is TRUE, recurse through child lists.
// Return FALSE if a grievous error acurred, otherwiase TRUE.
// END NOTES

BOOL  CImapMlistMgr::UpdateMailboxLists (BOOL Recurse)
{
	BOOL	bResult = TRUE;
	ImapMailboxNode		*pTopNode = NULL;
	CImapAccount		*pAccount = NULL;

	// Must have a valid account.
	pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
		return FALSE;

	// Go fetch the complete list.
	HRESULT hResult = pAccount->FetchChildMailboxList (m_ParentImapname, m_Delimiter, m_Type, &pTopNode, Recurse);

	// Did we get an inbox?
	//
	if ( m_Type == IMAP_ACCOUNT )
	{
		CRString szInbox (IDS_IMAP_RAW_INBOX_NAME);

		BOOL bFetched = TRUE;

		if ( ! SUCCEEDED (hResult) )
		{
			// If we at least got an inbox.
			//
			bFetched = FALSE;

			ImapMailboxNode* pCurNode = pTopNode;
			while (pCurNode)
			{
				if ( pCurNode->pImapName && (szInbox.CompareNoCase (pCurNode->pImapName) == 0) )
				{
					bFetched = TRUE;

					break;
				}

				pCurNode = pCurNode->SiblingList;
			}
		}

		g_ImapAccountMgr.SetTopMboxesFetched ( m_AccountID, bFetched );
	}


	if ( SUCCEEDED (hResult) )
	{
		// Go create temporary lists, even if we didn't get maiboxes. This will clear any
		// existing lists.

		__UpdateMailboxListFiles (pTopNode, m_Pathname);
	}
	else
	{
		bResult = FALSE;
	}

	// If this is a top-level fetch and user cancelled, flag that mailboxes may not
	// have been all downloaded
	//

	if (pTopNode)
	{
		// Free it.
		DeleteFolderSubtree (pTopNode);
		pTopNode = NULL;
	}

	return bResult;
}



// __UpdateMailboxLists [PRIVATE]
// NOTES
// This is a private method that does the real work!!
// Starting at the top node of the given top node and the directory specified in pPathname,
// build temporary tmp_mailboxes.lst files.
// END NOTES
BOOL  CImapMlistMgr::__UpdateMailboxListFiles(ImapMailboxNode *pTopNode, LPCSTR pPathname)
{
	ImapMailboxNode		*pCurNode = NULL;
	CString				Filename;
	CString				DirName;
	CString				Line;
	CString				buf;
	JJFileMT			tmpfile;

	// IMPORTANT NOTE:
	// At this point, allow pTopNode to be NULL so we go and, at least, create an
	// empty tmp_mboxlist.lst file. This will clear out any stale entries that don't exist anymore.

	if (!pPathname)
		return FALSE;

	// Create pPathname if it doesn't exist.
	if (!DirectoryExists (pPathname))
	{
		if (!CreateDirectory (pPathname, NULL))
			return FALSE;
	}
		
	// Create the temporary mailboxes file.
	Filename = pPathname;
	if (Filename.Right ( 1 ) != DirectoryDelimiter)
	{
		Filename += DirectoryDelimiter;
	}
	Filename += CRString (IDS_IMAP_TMP_MBOXLIST_FILENAME);

	// If the file exists, remove it.
	DeleteLocalFile ((LPCSTR) Filename);

	if ( !SUCCEEDED (tmpfile.Open (Filename, _O_RDWR | _O_CREAT)) )
		return FALSE;

	// OK. If pTopNode is NULL, close the file and exist.
	if (!pTopNode)
	{
		tmpfile.Close();
		return TRUE;
	}


	// Loop through the mailbox list now.
	// NOTE: These will all be mailboxes that don't exist locally so can go ahead and
	// create unique direcrories to house them.
	pCurNode = pTopNode;
	while (pCurNode)
	{
		// Make sure we have a valid imap name.
		if (pCurNode->pImapName)
		{
			// Format is: Imapname, Directory, TypeChar, Delimiter, NoSelect, NoInferiors, Marked, Unmarked, HasUnread.
			// NOte: No spaces between fields.

			// If pCurNode->bExists is TRUE, it means that this mailbox already exists
			// locally and the the local directory exists and already set into pCurNode.
			if (pCurNode->bExists)
			{
				// Extract the basename from pCurnode->pDirname
				if (pCurNode->pDirname)
				{
					DirName = pCurNode->pDirname;
					int i = DirName.ReverseFind (DirectoryDelimiter);
					if (i >= 0)
						DirName = DirName.Mid ( i + 1 );
				}
			}
			else
			{
				// Get a directory name for housing this mailbox.
				if ( MakeSuitableMailboxName (pPathname, pTopNode, pCurNode, DirName, 64) )
				{
					// Format a path to DirName and copy it to pCurNode->pDirname.
					MakePath (pPathname, DirName, buf);
					if (pCurNode->pDirname)
						delete pCurNode->pDirname;
					pCurNode->pDirname	= CopyString (buf);
				}
			}

			// Must have a directory name now!!
			if (!DirName.IsEmpty())
			{
				// Make sure the directory exists or that we can create it.
				if (!DirectoryExists (pCurNode->pDirname))
					CreateDirectory (pCurNode->pDirname, NULL);

				// Write this line only if the directory exists.
				if (DirectoryExists (pCurNode->pDirname))
				{
					// Format the line now.
					// BUG: Should find some way of determining HasUnread.
					CString csImapName = pCurNode->pImapName;
					csImapName.Replace(",", ",,");

					Line.Format ("%s,%s,%c,%c,%c,%c,%c,%c,%c", 
						csImapName, DirName,
						ImapMailboxTypeToTypeChar (pCurNode->Type), 
						pCurNode->Delimiter ? pCurNode->Delimiter : ' ',
						pCurNode->NoSelect ? '1' : '0',
						pCurNode->NoInferiors ? '1' : '0',
						pCurNode->Marked ? '1' : '0',
						pCurNode->UnMarked ? '1' : '0',
						'U' );
					tmpfile.PutLine ( (LPCSTR)Line);
				}
			}
		}

		// Do the next at this level.
		pCurNode = pCurNode->SiblingList;
	}

	tmpfile.Close();

	// Go back and do child lists, if any.
	pCurNode = pTopNode;
	while (pCurNode)
	{
		// We CAN pass a NULL child list to __UpdateMailboxListFiles().
		// But, go do this only if the mailbox can have inferiors.
		if (!pCurNode->NoInferiors)
		{
			__UpdateMailboxListFiles(pCurNode->ChildList, pCurNode->pDirname);
		}

		pCurNode = pCurNode->SiblingList;
	}

	return TRUE;
}



	





//================== Exported functions ==========================/


// ImapMailboxTypeToTypeChar
// NOTES
// Get the character representing the given ImapMailboxMode that gets saved to the mboxlist.lst file.
// END NOTES

TCHAR ImapMailboxTypeToTypeChar (ImapMailboxType Type)
{
	switch (Type)
	{
		case IMAP_ACCOUNT:
			return 'A';

		case IMAP_NAMESPACE:
			return 'N';

		case IMAP_MAILBOX:
		default:
			return 'M';
	}
}		



// MakeSuitableMailboxName()
// NOTES:
// Get a name for a directory housing a mailbox. The name must not match a name already 
// used.
// Base the name on the last component of the node's ImapName. 
// Set the new name into the referenced CString. 
// The resulting name must be at most MaxNameLength long.
// The name cannot contain directory delimiter chars, nor the "Celimiter" chars passed in.
// END NOTES
//

BOOL MakeSuitableMailboxName (LPCSTR pParentDir, ImapMailboxNode *pTopNode, ImapMailboxNode *pCurNode, CString &NewName, short MaxNameLength)
{
	CString			Name, BaseName, Pname;
	int				i, trial;
	const int		MaxTrial = 1000;
	ImapMailboxNode *pNode;
	BOOL			TryAgain;
	BOOL			bResult = FALSE;
	CString			DirectoryPrefix;

	// pTopNode can be NULL.
	if (!(pParentDir && pCurNode && pCurNode->pImapName))
		return FALSE;

	// Use pCurNode->pImapName as the suggested name.
	// If name begins with a dot, keep it a part of the name,
	// otherwise use only the chars before the dot.
	// Make sure it's at most MaxNameLength characters long.

	Name = pCurNode->pImapName;;

	// Delete any ".extension" in filename if "." is not the imap delimiter.
	if (pCurNode->Delimiter != '.')
	{
		i = Name.Find('.');
		while (i >= 0)
		{
			if (i == 0)
			{
				// If name is something like ".xxx", use "xxx".
				Name = Name.Mid ( 1 );

				// Make sure we don't have any more.
				i = Name.Find ('.');
			}
			else if (i > 0)
			{
				// Use basename;
				Name = Name.Left (i);
				i = -1;  // Stop here.
			}
			else
			{
				// else use as is.
				i = -1;
			}
		}
	}

	// Do we have anything left?
	if (Name.IsEmpty())
		Name = '0';

	// Omit Delimiter char.
	if (pCurNode->Delimiter)
	{
		i = Name.ReverseFind (pCurNode->Delimiter);
		if (i >= 0)
		{
			Name = Name.Mid (i + 1);
		}
	}

	// Remove any weird characters from the mailbox's name.
	//
	char szDirName [_MAX_FNAME + 4];

	*szDirName = 0;

	i = 0;
	for (int j = 0; (j < _MAX_FNAME) && (j < Name.GetLength()); j++)
	{
		int c = Name [j];

		if ( c == 0 )
		{
			break;
		}
		else if ( isalnum (c) )
		{
			szDirName[i++] = (char)c;
		}
	}

	szDirName[i] = '\0';

	// This is our new name:
	//
	Name = szDirName;


	// Extract basename if contains directory delimiters.

	i = Name.ReverseFind (DirectoryDelimiter);
	if (i >= 0)
	{
		Name = Name.Mid (i + 1);
	}

	// Now, make sure it's at least of maximum length
	Name = Name.Left (MaxNameLength);

	// Do we have anything left?
	if (Name.IsEmpty())
		Name = '0';

	// See if we already have that name (Case insensitive compare, since the name become
	// files in a possibly FAT file system). Add a uniquifier digit.
	BaseName = Name;

	// Format the containing directory name with a trailing directory delimiter.
	DirectoryPrefix = pParentDir;
	if (DirectoryPrefix.Right ( 1 ) != DirectoryDelimiter)
		DirectoryPrefix += DirectoryDelimiter;

	// Make sure we terminate.
	for (trial = 0; trial <= MaxTrial; trial++)
	{
		TryAgain = FALSE;
		pNode = pTopNode;
		while (pNode && (pNode != pCurNode))
		{
			if (pNode->pDirname)
			{
				// Extract basename.
				Pname = pNode->pDirname;
				i = Pname.ReverseFind (DirectoryDelimiter);
				if (i >= 0)
				{
					Pname = Pname.Mid (i + 1);
				}

				// Remove "."
				i = Pname.Find ('.');
				if (i >= 0)
				{
					Pname = Pname.Left ( i );
				}

				if (Name.CompareNoCase (Pname) == 0)
				{
					// Found a previous matching name. Add a uniquifier.
					Name.Format ("%s%d", (LPCSTR)BaseName, trial);
					TryAgain = TRUE;
					// Go and try again.
					break;
				}
			}
			pNode = pNode->SiblingList;
		} // while;

		// Break out of for.
		if (!TryAgain)
			break; 
	}

	// Did we get a unique name??
	if (trial > MaxTrial || Name.IsEmpty())
		return FALSE;

	// Now go make sure there's no duplicate file or directory name.
	BaseName = Name;
	bResult = FALSE;
	for (trial = 0; trial <= MaxTrial; trial++)
	{
		if (FileExistsMT (DirectoryPrefix + Name))
		{
			// Found an existing file or directory. Add a uniquifier.
			Name.Format ("%s%d", (LPCSTR)BaseName, trial);
		} // if
		else
		{
			// Found a unique name.
			bResult = TRUE;
			break;		
		}
	}

	// Did we get a unique name??
	if (trial > MaxTrial || Name.IsEmpty())
		return FALSE;

	// Ok. Found one at last.
	bResult = TRUE;		// Found a unique name.

	// Make sure copy Name to NewName.
	if (bResult)
	{
		NewName = Name;
	}

	return bResult;
}		


#endif // IMAP4


