// QCMailboxDirector.cpp: implementation of the QCMailboxDirector class.
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

#include "eudora.h"
#include <stdlib.h>

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCMailboxTreeCtrl.h"
#include "Automation.h"
#include "QCGraphics.h"
#include "QCImapMailboxCommand.h"
#include "ImapMailbox.h"
#include "DynamicMailboxMenu.h"

#include "rs.h"
#include "utils.h"
#include "guiutils.h"
#include "fileutil.h"
#include "mainfrm.h"
#include "tocdoc.h"
#include "progress.h"
#include "newmbox.h"
#include "SearchManager.h"

#include "DebugNewHelpers.h"

std::list<LPCTSTR> QCMailboxDirector::s_RecentMailboxList;

extern CString			EudoraDir;
extern QCCommandStack	g_theCommandStack;
extern QCMailboxDirector g_theMailboxDirector;

extern BOOL FilterMBoxLink(const char *MailboxName, BOOL IsFolder);
extern void FiltersUpdateMailbox(const char *OldName, const char *NewName, const char *NewMenuName, BOOL IsFolder);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCMailboxDirector::QCMailboxDirector()
{
}


QCMailboxDirector::~QCMailboxDirector()
{
	while (m_theMailboxList.IsEmpty() == FALSE)
		delete (QCMailboxCommand*)m_theMailboxList.RemoveTail();
}


LPTSTR QCMailboxDirector::LegalizeFilename(LPTSTR pFilename)
{
	if (!pFilename)
	{
		ASSERT(0);
		return NULL;
	}

	// Remove spaces and other nasty characters from the filename
	::StripIllegalMT(pFilename, EudoraDir);
	
	// Truncate the filename for 8.3 file systems
	if (strlen(pFilename) > 8 && !::LongFileSupportMT(EudoraDir))
		pFilename[8] = 0;	
	
	return pFilename;
}


// Salvage
// Searches through the directory for mailboxes and folders,
// and adds them if they're not already there
BOOL QCMailboxDirector::Salvage(LPCTSTR szPath, CPtrList& theList)
{
	TCHAR				Filename[_MAX_PATH + 1];
	BOOL				ChangeMade = FALSE;
	QCMailboxCommand*	pCommand;
	CRString Extension(IDS_MAILBOX_EXTENSION);

	strcpy(Filename, szPath);
	int PathLen = strlen(Filename);

	// If this is the top level, then make sure In, Out, and Trash mailboxes and entries exist
	if (stricmp(szPath, EudoraDir) == 0)
	{
		CRString InFilename(IDS_IN_MBOX_FILENAME);
		CRString OutFilename(IDS_OUT_MBOX_FILENAME);
		CRString JunkFilename(IDS_JUNK_MBOX_FILENAME);
		CRString TrashFilename(IDS_TRASH_MBOX_FILENAME);
		CRString InName(IDS_IN_MBOX_NAME);
		CRString OutName(IDS_OUT_MBOX_NAME);
		CRString JunkName(IDS_JUNK_MBOX_NAME);
		CRString TrashName(IDS_TRASH_MBOX_NAME);
		JJFile  CreateFile;

		strcpy(Filename + PathLen, InFilename);
		strcat(Filename + PathLen, Extension);
		if (!::FileExistsMT(Filename))
		{
			if (SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)))
				CreateFile.Close();
		}
		
		if ((pCommand = FindByFilename(theList, Filename)) == NULL)
		{
			Insert(theList, DEBUG_NEW QCMailboxCommand(this, InName, Filename, MBT_IN), true);
			ChangeMade = TRUE;
		}
		else
		{
			if (pCommand->GetType() != MBT_IN)
			{
				pCommand->SetType(MBT_IN);
				ChangeMade = TRUE;
			}
			if (stricmp(pCommand->GetName(), InName))
			{
				pCommand->SetName(InName);
				ChangeMade = TRUE;
			}
		}

		strcpy(Filename + PathLen, OutFilename);
		strcat(Filename + PathLen, Extension);
		if (!::FileExistsMT(Filename))
		{
			if (SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)))
				CreateFile.Close();
		}

		if ((pCommand = FindByFilename(theList, Filename)) == NULL)
		{
			Insert(theList, DEBUG_NEW QCMailboxCommand(this, OutName, Filename, MBT_OUT), true);
			ChangeMade = TRUE;
		}
		else
		{
			if (pCommand->GetType() != MBT_OUT)
			{
				pCommand->SetType(MBT_OUT);
				ChangeMade = TRUE;
			}
			if (stricmp(pCommand->GetName(), OutName))
			{
				pCommand->SetName(OutName);
				ChangeMade = TRUE;
			}
		}
		
		if (GetIniShort(IDS_INI_ASKED_ABOUT_JUNK))
		{
			// Only create the Junk mailbox if this isn't the first time the user
			// has run a Junk feature-enabled version.  If this is the first time
			// we need to check with the user before doing anything.
			strcpy(Filename + PathLen, JunkFilename);
			strcat(Filename + PathLen, Extension);
			if (!::FileExistsMT(Filename))
			{
				if (SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)))
					CreateFile.Close();
			}

			if ((pCommand = FindByFilename(theList, Filename)) == NULL)
			{
				Insert(theList, DEBUG_NEW QCMailboxCommand(this, JunkName, Filename, MBT_JUNK));
				ChangeMade = TRUE;
			}
			else
			{
				if (pCommand->GetType() != MBT_JUNK)
				{
					pCommand->SetType(MBT_JUNK);
					ChangeMade = TRUE;
				}
				if (stricmp(pCommand->GetName(), JunkName))
				{
					pCommand->SetName(JunkName);
					ChangeMade = TRUE;
				}
			}
		}

		strcpy(Filename + PathLen, TrashFilename);
		strcat(Filename + PathLen, Extension);
		if (!::FileExistsMT(Filename))
		{
			if (SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)))
				CreateFile.Close();
		}

		if ((pCommand = FindByFilename(theList, Filename)) == NULL)
		{
			Insert(theList, DEBUG_NEW QCMailboxCommand(this, TrashName, Filename, MBT_TRASH), true);
			ChangeMade = TRUE;
		}
		else
		{
			if (pCommand->GetType() != MBT_TRASH)
			{
				pCommand->SetType(MBT_TRASH);
				ChangeMade = TRUE;
			}
			if (stricmp(pCommand->GetName(), TrashName))
			{
				pCommand->SetName(TrashName);
				ChangeMade = TRUE;
			}
		}
	}
    
	// Get mailboxes first
	wsprintf(Filename + PathLen, "*%s", (LPCTSTR)Extension);
	
	WIN32_FIND_DATA Find;
	Find.dwFileAttributes = _A_NORMAL;
	
	TCHAR	Name[MAX_PATH + 1];
	HANDLE FindHandle = FindFirstFile(Filename,&Find);
	int Result = 0;
	
	if (FindHandle != INVALID_HANDLE_VALUE) 
		Result = 1;
	for (; Result; Result = FindNextFile(FindHandle, &Find))
	{
		// Find{First,Next}File will return matches for files that don't exactly match
		// the search criteria.  For example, a criteria of "*.mbx" will match a file
		// that is named "In.mbx_EXTRA_STUFF_HERE".
		int len = strlen(Find.cFileName);
		if (len < Extension.GetLength() || stricmp(Find.cFileName + len - Extension.GetLength(), Extension))
			continue;

		wsprintf(Filename + PathLen, "%s", Find.cFileName);
		
		if (FindByFilename(theList, Filename) == NULL)
		{
			CString		szMailboxFilename = Filename;

			::SetFileExtensionMT(Filename, CRString(IDS_TOC_EXTENSION));
			Name[0] = 0;
			JJFile Toc(JJFileMT::BUF_SIZE, FALSE);

			if (FAILED(Toc.Open(Filename, O_RDONLY)) || 
				FAILED(Toc.Read(Name, 8)) || 				// dummy read to skip 8-byte version info
				FAILED(Toc.Read(Name, 32)) ||				// real read to get 32-byte name
				( FindByName(&theList, Name) != NULL ) )		// check for duplicate name at this folder level
			{
				//
				// Indicate to the code below that we need to generate
				// a mailbox name because 1) a TOC doesn't exist,
				// 2) we failed to extract a mailbox name from an
				// existing TOC, or 3) because the name extracted from the
				// TOC conflicts with an existing mailbox name.
				//
				// Bad things will (eventually) happen if we create a DESCMAP
				// entry with the same "user" mailbox name as an existing
				// mailbox name.
				//
				Name[0] = 0;
			}
			Toc.Close();

			if (0 == Name[0])
			{
				//
				// Auto-generate a mailbox name based on the base
				// portion of the MBX filename.
				//
				strncpy(Name, Find.cFileName, MAX_PATH);
				Name[MAX_PATH] = 0;
				LPTSTR dot = strrchr(Name, '.');
				if (dot)
					*dot = 0;

				// Update the name in the TOC if possible
				CTocDoc *	pTocDoc = GetToc(szMailboxFilename);
				if (pTocDoc)
				{
					pTocDoc->ReallySetTitle(Name);
					pTocDoc->WriteHeader();
				}
			}
			::SetFileExtensionMT(Filename, Extension);			
			Insert(theList, DEBUG_NEW QCMailboxCommand(this, Name, Filename, MBT_REGULAR), true);
			ChangeMade = TRUE;

			SearchManager::Instance()->NotifyMailboxAdded(Filename);
		}
	}
    FindClose(FindHandle);

	// Get the directories
	wsprintf(Filename + PathLen, "*%s", (LPCTSTR)CRString(IDS_FOLDER_EXTENSION));
	
	Find.dwFileAttributes = _A_SUBDIR;
	FindHandle = FindFirstFile(Filename,&Find);
	if (FindHandle != INVALID_HANDLE_VALUE) 
		Result = 1;
	else
		Result = 0;

	while (Result)
	{
		// Do we have a folder? And is it an actual child directory (i.e. not the
		// current or parent directory)?
		// Make sure to check bitwise so that we don't fail if the folder has
		// some other bits set, like archive (fixes bug reported by Cornell).
		if ( (Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			 strcmp(Find.cFileName, ".") && strcmp(Find.cFileName, "..") )
        {
			CString szPathname(szPath);
			szPathname += Find.cFileName;

			if (FindByFilename(theList, szPathname) == NULL)
			{
				strncpy(Name, Find.cFileName, MAX_PATH);
				Name[MAX_PATH] = 0;
				LPTSTR dot = strrchr(Name, '.');
				if (dot)
					*dot = 0;

				Insert(theList, DEBUG_NEW QCMailboxCommand(this, Name, szPathname , MBT_FOLDER), true);
				ChangeMade = TRUE;
			}
        }
		Result = FindNextFile(FindHandle,&Find);
	}
	if (FindHandle != INVALID_HANDLE_VALUE)
		FindClose(FindHandle);
	// Save the changes, if any
	return (ChangeMade? WriteDescMapFile( szPath, theList ) : TRUE);
}


BOOL QCMailboxDirector::ProcessDescMapFile(LPCTSTR szPath, CPtrList& theMailboxList)
{
	CString				szMapFilename;
	JJFile				theMapFile;
	BOOL				bNeedsWrite;
	TCHAR				buf[256];
	LPTSTR				szFilename;
	MailboxType			theType;
	UnreadStatusType	theStatus;
	LPTSTR				szStatusChar;
	CString				szMbxPathname;
	BOOL				bValidEntry;
	QCMailboxCommand*	pCommand;
	POSITION			pos;
	POSITION			next;
	bool				bCheckForJunk = (!GetIniShort(IDS_INI_ASKED_ABOUT_JUNK));

	szMapFilename = szPath;
	szMapFilename += CRString(IDS_DESCMAP);

	if ( ::FileExistsMT( szMapFilename ) )
	{
		if( FAILED(theMapFile.Open( szMapFilename, O_RDONLY )) )
			return FALSE;
		
		//
		// Set the following flag TRUE if we need to immediately write this 
		// file in order to flush out any stale entries.
		//
		bNeedsWrite = FALSE;

		//
		// Description mapping file (DESCMAP.PCE) lines looks like this:
		//		Name,Filename,TypeChar,UnreadStatus
		//
		// Process the entire DESCMAP file, one line at a time.
		//
		long lNumBytesRead = 0;
		while( SUCCEEDED(theMapFile.GetLine( buf, sizeof( buf ), &lNumBytesRead)) && (lNumBytesRead > 0) )
		{
			szFilename = strchr( buf, ',' );
			
			if (! szFilename )
			{
				ASSERT(FALSE);
				continue;
			}
			
			*szFilename++ = 0;
			
			LPTSTR szTypeChar = strchr( szFilename, ',' );
			
			if (! szTypeChar )
			{
				ASSERT(FALSE);
				continue;
			}
			
			*szTypeChar++ = 0;
			
			theType = MBT_REGULAR;
			
			switch( toupper( *szTypeChar ) )
			{
				case 'S':
					{
						//
						// Localized versions are supposed to use IN.MBX, OUT.MBX, JUNK.MBX,
						// and TRASH.MBX as filenames no matter what localized mailbox
						// names appear on the menus.  Therefore, we need to look
						// at the root part of the 8.3 filename.
						//
						CString filename( szFilename );
						int idx = filename.Find('.');
						
						if (idx != -1)
						{
							filename = filename.Left(idx);
						}

						if( filename.CompareNoCase( CRString( IDS_IN_MBOX_FILENAME ) ) == 0 )
						{
							theType = MBT_IN;
						}
						else if ( filename.CompareNoCase( CRString( IDS_OUT_MBOX_FILENAME ) ) == 0 )
						{
							theType = MBT_OUT;
						}
						else if ( filename.CompareNoCase( CRString( IDS_JUNK_MBOX_FILENAME ) ) == 0 )
						{
							theType = MBT_JUNK;
							bCheckForJunk = false;
							((CEudoraApp *)AfxGetApp())->SetJunkMboxStatus(JUNK_MBOX_SYSTEM);
						}
						else if ( filename.CompareNoCase( CRString( IDS_TRASH_MBOX_FILENAME ) ) == 0 )
						{
							theType = MBT_TRASH;
						}
						else
						{
							ASSERT( 0 );
							continue;
						}
					}
				break;
				case 'M':
					if (bCheckForJunk)
					{
						// If we haven't found a junk mailbox yet make sure we are not
						// dealing with a case where the user has a non-system mailbox
						// named Junk.
						CString filename( szFilename );
						int idx = filename.Find('.');
						if (idx != -1)
						{
							filename = filename.Left(idx);
						}
						if ( filename.CompareNoCase( CRString( IDS_JUNK_MBOX_FILENAME ) ) == 0 )
						{
							// Set a flag noting that the user has a regular mailbox named Junk
							// so we can deal with it later.
							((CEudoraApp *)AfxGetApp())->SetJunkMboxStatus(JUNK_MBOX_REGULAR);
							bCheckForJunk = false;
						}
					}
					theType = MBT_REGULAR;
				break;
				case 'F': 
					theType = MBT_FOLDER; 
				break;
				default:
					// Better not get here!
					ASSERT(FALSE);
					continue;
			}

		
			theStatus = US_UNKNOWN;
		
			szStatusChar = strchr( szTypeChar, ',' );
		
			if ( szStatusChar )
			{
				switch( toupper( szStatusChar[1] ) )
				{
					case 'U': 
						theStatus = US_UNKNOWN;	
					break;
					case 'Y': 
						theStatus = US_YES;		
					break;
					case 'N': 
						theStatus = US_NO;			
					break;
					
					default: 
						ASSERT(FALSE);
					break;
				}
			}

			bValidEntry = TRUE;
			
			szMbxPathname = szPath;
			szMbxPathname += szFilename;

			//
			// At this point, it looks and smells like a valid DESCMAP
			// entry, but let's make sure that the MBX file still exists.
			// If there's no matching mailbox file, then don't bother
			// keeping the DESCMAP entry.
			//
			
			if (! ::FileExistsMT( szMbxPathname ) )
			{
				//
				// Make some (debug-only) noise about a bogus DESCMAP
				// entry.  Did you do a naughty thing and delete or
				// rename a mailbox outside of Eudora when Eudora
				// wasn't looking?  If so, it's okay.  Calm down.
				// Relax.  We'll just pretend that the bogus
				// DESCMAP entry never existed and keep going...
				//
				ASSERT(0);
				bValidEntry = FALSE;
				bNeedsWrite = TRUE;

				SearchManager::Instance()->NotifyMailboxDeleted(szMbxPathname);
			}

			if( bValidEntry )
			{
				//
				// Look for duplicates.  If there this entry conflicts with
				// an earlier entry, then don't allow it.  This should be
				// fixed up during the Salvage() sequence, which makes sure
				// there is a DESCMAP entry for each MBX file.
				//
				pos = theMailboxList.GetHeadPosition();

				for( next = pos; pos; pos = next )
				{
					pCommand = ( QCMailboxCommand* ) theMailboxList.GetNext( next );

					if( ( stricmp( buf, pCommand->GetName() ) == 0 ) || 
						( pCommand->GetPathname().CompareNoCase( szMbxPathname ) == 0 ) )
					{
						ASSERT( 0 );
						bNeedsWrite = TRUE;
						break;
					}
				}

				if( pos == NULL )
				{
					Insert(theMailboxList, DEBUG_NEW QCMailboxCommand(this, buf, szMbxPathname, theType, theStatus), true);
				}
			}
		}
		
		theMapFile.Close();

		if( bNeedsWrite && ! WriteDescMapFile( szPath, theMailboxList ) )
		{
			return FALSE;
		}
	}			

	// build the file (if it's missing) or add in files we didn't know about
	if( !Salvage( szPath, theMailboxList ) )
	{
		return FALSE;
	}


	// find and process subdirectories

	pos = theMailboxList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCMailboxCommand* ) theMailboxList.GetNext( pos );
		
		if( pCommand->GetType() == MBT_FOLDER )
		{
			if( !ProcessDescMapFile( pCommand->GetPathname() + "\\", 
									 pCommand->GetChildList() ) )
			{
				return FALSE;
			}
		}
	}				
	
	return TRUE;
}


BOOL QCMailboxDirector::Build()
{
	int nCheckMenuHeight = ::GetSystemMetrics(SM_CYMENUCHECK);

	VERIFY( m_theUnreadStatusBitmap.LoadBitmap(IDB_UNREAD_STATUS) );
	VERIFY( m_theTransferBitmap.LoadBitmap(IDB_TRANSFER_MENU) );

	if( GetIniShort( IDS_INI_CENTER_UNREAD_STATUS ) )
	{
		// Center the bitmaps vertically in the space available
		::FitTransparentBitmapToHeight(m_theUnreadStatusBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(m_theTransferBitmap, nCheckMenuHeight);
	}

	// Build the IMAP account subtrees for the personality database.
	// Note: We don't want to process the IMAP stuff if the regular stuff failed.
	return ProcessDescMapFile(EudoraDir, m_theMailboxList) && BuildImapAccounts();
}


QCMailboxCommand* QCMailboxDirector::FindByFilename(CPtrList& theList, LPCTSTR szPathname)
{
	POSITION			pos;
	QCMailboxCommand*	pCommand;

	pos = theList.GetHeadPosition();
	while( pos )
	{
		pCommand = ( QCMailboxCommand* ) theList.GetNext( pos );

		if( pCommand->GetPathname().CompareNoCase( szPathname ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}


QCMailboxCommand*	QCMailboxDirector::FindByName(
CPtrList*	pTheList,
LPCTSTR		szName )
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

		if( stricmp( pCommand->GetName(), szName ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}



POSITION QCMailboxDirector::Insert(
	CPtrList&			theList,
	QCMailboxCommand*	pCommand,
	bool				bDeleteIfNotInserted)
{
	POSITION			next;
	POSITION			pos;
	QCMailboxCommand*	pCurrent;
	INT					i;
			
	pos = theList.GetHeadPosition();
	next = pos;

	if( pCommand->GetType() == MBT_IN ) 
	{	
		if( pos )
		{
			pCurrent = ( QCMailboxCommand* ) theList.GetNext( next );
		
			if( pCurrent->GetType() == MBT_IN ) 
			{
				// its already in the list
				if (bDeleteIfNotInserted)
					delete pCommand;
				return pos;
			}
		}

		return  theList.AddHead( pCommand );
	}

	if(	( pCommand->GetType() == MBT_OUT ) ||
		( pCommand->GetType() == MBT_JUNK ) ||
		( pCommand->GetType() == MBT_TRASH ) )
	{
		for( ; pos != NULL ; pos = next )
		{
			pCurrent = ( QCMailboxCommand* ) theList.GetNext( next );
		
			if( pCurrent->GetType() == pCommand->GetType() ) 
			{
				// its already in the list
				if (bDeleteIfNotInserted)
					delete pCommand;
				return pos;
			}

			// Insert the Out mailbox before the first item that is not
			// the In mailbox.
			if ((pCommand->GetType() == MBT_OUT) &&
				(pCurrent->GetType() != MBT_IN))
			{
				break;
			}

			// Insert the Junk mailbox before the first item that is not
			// the In or Out mailbox.
			if ((pCommand->GetType() == MBT_JUNK) &&
				(pCurrent->GetType() != MBT_IN) &&
				(pCurrent->GetType() != MBT_OUT))
			{
				break;
			}

			// Insert the Trash mailbox before the first item that is not
			// the In, Out or Junk mailbox.
			if ((pCommand->GetType() == MBT_TRASH) &&
				(pCurrent->GetType() != MBT_IN) &&
				(pCurrent->GetType() != MBT_OUT) &&
				(pCurrent->GetType() != MBT_JUNK))
			{
				break;
			}
		}

		if( pos )
		{
			return theList.InsertBefore( pos, pCommand );
		}

		return  theList.AddTail( pCommand );
	}

	// Need this below:
	//
	CString szImapInbox = CRString (IDS_IMAP_RAW_INBOX_NAME);

	for(; pos != NULL; pos = next )
	{
		pCurrent = ( QCMailboxCommand* ) theList.GetNext( next );
		
		if( ( pCurrent->GetType() != MBT_REGULAR ) && ( pCurrent->GetType() != MBT_FOLDER ) &&
			( !pCurrent->IsImapType() ) )
		{
			// skip in, out and trash and IMAP types.
			continue;			
		}

		// Put IMAP types last:
		//
		if( ( pCommand->IsImapType() ) && (! pCurrent->IsImapType() ) )
		{
			continue;
		}

		if( ( pCommand->GetType() == MBT_FOLDER ) && ( pCurrent->IsImapType() ) )
		{
			// put the folders before IMAP types.
			break;
		}

		if( ( pCommand->GetType() == MBT_FOLDER ) && ( pCurrent->GetType() != MBT_FOLDER ) )
		{
			// put the folders at the bottom (but before IMAP types).
			continue;
		}

		if( ( pCommand->GetType() == MBT_REGULAR ) &&
			 ( ( pCurrent->GetType() == MBT_FOLDER ) || pCurrent->IsImapType () ) )
		{	
			// put mailboxes before folders
			break;
		}

		// IMAP accounts go last:
		//
		if ( (pCommand->GetType() == MBT_IMAP_ACCOUNT) && (pCurrent->GetType() != MBT_IMAP_ACCOUNT) )
		{
			continue;
		}

		// Always put IMAP's "Inbox" before any other IMAP mboxes:
		if ( (pCommand->IsImapType() && pCurrent->IsImapType() ) &&
			 (! (pCommand->GetType() == MBT_IMAP_ACCOUNT || pCurrent->GetType() == MBT_IMAP_ACCOUNT) ) )
		{
			// If we're inserting Inbox:
			if ((stricmp( szImapInbox, pCommand->GetName() ) == 0 ) )
			{
				break;
			}
			// If curent is inbox, go past it:
			else if ((stricmp( szImapInbox, pCurrent->GetName() ) == 0 ) )
			{
				continue;
			}
		}

		i = stricmp( pCurrent->GetName(), pCommand->GetName() );
		
		if( ( i == 0 ) && ( pCommand->GetType() == pCurrent->GetType() ) )
		{
			// it's already in the list
			if (bDeleteIfNotInserted)
				delete pCommand;
			return (pos);
		}
		
		if( i > 0 )
		{
			// found the position
			break;
		}
	}

	if( pos )
	{
		return theList.InsertBefore( pos, pCommand );
	}
	
	return  theList.AddTail( pCommand );
}


BOOL QCMailboxDirector::WriteDescMapFile(
LPCTSTR			szPath,
const CPtrList& theList)
{	
	POSITION			pos;
	CString				TempFilename(GetTmpFileNameMT());
	CString				Filename;
	QCMailboxCommand*	pCommand;
	CString				line;
	TCHAR				szDir[_MAX_DIR];
	TCHAR				szDrive[_MAX_DRIVE];
	TCHAR				szFile[_MAX_FNAME];
	TCHAR				szExt[_MAX_EXT];
	JJFile				MapFile;
	TCHAR				cType;
	TCHAR				cStatus;

	Filename = szPath;

	if( Filename.Right( 1 ) != '\\' )
	{
		Filename += '\\';
	}

	Filename += CRString( IDS_DESCMAP );

	if (FAILED(MapFile.Open(TempFilename, O_CREAT | O_WRONLY | O_TRUNC)))
		return FALSE;

	pos = theList.GetHeadPosition();
	
	while( pos )
	{
		pCommand = ( QCMailboxCommand* ) theList.GetNext( pos );
	
#ifdef IMAP4
	// Eliminate IMAP mailboxes/accounts from this list.
		if ( pCommand->IsImapType() )
		{
			continue;
		}
#endif

		_splitpath( pCommand->GetPathname(), szDrive, szDir, szFile, szExt );
	
		switch( pCommand->GetType() )
		{
			case MBT_IN:
			case MBT_OUT:
			case MBT_JUNK:
			case MBT_TRASH:
				cType = 'S';
			break;
			case MBT_FOLDER:
				cType = 'F';
			break;
			case MBT_REGULAR:
				cType = 'M';
			break;
			default:
				ASSERT( 0 );
				return FALSE;
		}

		switch( pCommand->GetStatus() )
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
			default:
				ASSERT( 0 );
				return FALSE;
		}

		line.Format(	"%s,%s%s,%c,%c", 
						(LPCTSTR) pCommand->GetName(), 
						szFile,
						szExt,
						cType, 
						cStatus );
		
		if( FAILED(MapFile.PutLine(line)) )
		{
			MapFile.Delete();
			return FALSE;
		}
	}

	return ( FAILED( MapFile.Rename(Filename) ) ? FALSE : TRUE );
}


QCMailboxCommand*	QCMailboxDirector::FindByNamedPath(
LPCTSTR	szNamedPath )
{
	CPtrList*			pList;
	CString				szPath;
	CString				szThisPath;
	QCMailboxCommand*	pCommand;
	INT					i;

	pList = &m_theMailboxList;
	szPath = szNamedPath;

	while( 1 )
	{
		i = szPath.Find( '\\' );

		if( i == -1 )
		{
			// look in the current list for the mailbox name
			pCommand = FindByName( pList, szPath );
			return pCommand;
		}

		szThisPath = szPath.Left( i );
		szPath = szPath.Right( szPath.GetLength() - i - 1 );

		// look in the current list for this folder
		pCommand = FindByName( pList, szThisPath );
		
		if( pCommand == NULL )
		{
			return NULL;
		}

		pList = & ( pCommand->GetChildList() );
	}

	return NULL;
}



CString QCMailboxDirector::BuildNamedPath(
QCMailboxCommand*	pCommand)
{
	CPtrList*			pList;
	CString				szPath;
	CString				szThisPath;
	CString				szNamedPath;
	POSITION			pos;
	QCMailboxCommand*	pCurrent;

	szNamedPath = "";
	pList = &m_theMailboxList;
	szPath = pCommand->GetPathname();

	pos = pList->GetHeadPosition();

	while( pos )
	{
		pCurrent = ( QCMailboxCommand* ) pList->GetNext( pos );

		if( pCurrent->GetPathname().CompareNoCase( pCommand->GetPathname() ) == 0 )
		{
			// done!
			if( szNamedPath != "" )
			{
				szNamedPath += '\\';
			}
			szNamedPath += pCurrent->GetName();
			return szNamedPath;
		}

#ifdef IMAP4
		// IMAP mailboxes are housed in directories! 
		CString Objpath;
		if (pCurrent->IsImapType())
		{
			((QCImapMailboxCommand *)pCurrent)->GetObjectDirectory (Objpath);
		}
		else
			Objpath = pCurrent->GetPathname ();

		// Can have mailbox names like mbox and mbox0!
		if( ( ( pCommand->GetPathname() ).GetLength () > Objpath.GetLength() ) &&
			( strnicmp( Objpath, pCommand->GetPathname(), Objpath.GetLength() ) == 0 ) &&
			( ( pCommand->GetPathname() ).GetAt ( Objpath.GetLength() ) == '\\' ) )
#else
		if( strnicmp( pCurrent->GetPathname(), pCommand->GetPathname(), ( pCurrent->GetPathname() ).GetLength() ) == 0 )
#endif // IMAP4

		{
			// found a parent
			// done!
			if( szNamedPath != "" )
			{
				szNamedPath += '\\';
			}
		
			szNamedPath += pCurrent->GetName();
			pList = &( pCurrent->GetChildList() );
			pos = pList->GetHeadPosition();
		}
	}

	return szNamedPath;
}



QCMailboxCommand* QCMailboxDirector::AddCommand(LPCTSTR szName, MailboxType theType,
												QCMailboxCommand* pParent)
{
	CString	szPathname;

	// Redirect IMAP add commands to "ImapAddCommand()"
	if (pParent && pParent->IsImapType())
		return ImapAddCommand (szName, theType, pParent);

	CPtrList* pTheList = NULL;	
	if (pParent)
	{
		// If the parent type is not a folder, it must
		// mean we're at the top-level of the mailboxes
		if (pParent->GetType() == MBT_FOLDER)
		{
			pTheList = &( pParent->GetChildList() );
			szPathname = pParent->GetPathname() + '\\';
		}
	}
	if (!pTheList)
	{
		pTheList = &m_theMailboxList;
		szPathname = EudoraDir;
	}

	QCMailboxCommand* pCommand = FindByName(pTheList, szName);

	if (pCommand)
		return pCommand;

	CString	szDescMapPath(szPathname);
	CString	szFilename;
	BOOL	bNoChanges;
	NewMBXFilename(szName, "", szPathname,
					CRString(theType == MBT_FOLDER? IDS_FOLDER_EXTENSION : IDS_MAILBOX_EXTENSION),
					szFilename, bNoChanges);
	szPathname += szFilename;

	pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCMailboxCommand(this, szName, szPathname, theType, US_NO);

	if (!pCommand)
		return NULL;

	JJFile NewFile;
	if (theType == MBT_FOLDER)
	{
		int Status = mkdir(szPathname);

		if (Status != 0)
		{
			ErrorDialog(IDS_ERR_DIR_CREATE, szPathname);
			delete pCommand;
			return NULL;
		}
		
		CString	szDescMapPathname(szPathname + "\\" + CRString(IDS_DESCMAP));

		if (FAILED(NewFile.Open(szDescMapPathname, O_WRONLY | O_CREAT)))
		{
			delete pCommand;
			return NULL;
		}
		NewFile.Close();
	}
	else
	{
		// Create the mailbox
		if (FAILED(NewFile.Open( szPathname, O_WRONLY | O_CREAT)))
		{
			delete pCommand;
			return NULL;
		}
		NewFile.Close();
	}

	Insert(*pTheList, pCommand);

	if (theType != MBT_FOLDER)
	{
		// Build the TOC by calling GetToc.
		GetToc(szPathname, szName);
	}

	WriteDescMapFile(szDescMapPath, *pTheList);

	NotifyClients(pCommand, CA_NEW);

	return pCommand;
}



BOOL QCMailboxDirector::InitializeMailboxTreeControl( 
QCMailboxTreeCtrl*	pTreeCtrl,
UINT				uLevel,
CPtrList*			pList )
{
	QCMailboxCommand*	pCommand;
	BOOL				bHasUnreadMessages;
	POSITION			pos;
	BOOL				bUnreadChildren;

	if( pTreeCtrl == NULL )
	{
		return FALSE;
	}
	
	bHasUnreadMessages = FALSE;

	if ( uLevel == 0 )
	{
		// Add root node to QCMailboxTreeCtrl
		if( pTreeCtrl != NULL )
		{
			CString szLastDir;
			WIN32_FIND_DATA FindData;
			HANDLE FindHandle;

			// FindFirstFile() seems to be the only reliable way to get the long filename of a directory
			FindHandle = FindFirstFile(EudoraDir.Left(EudoraDir.GetLength() - 1), &FindData);
			if (INVALID_HANDLE_VALUE == FindHandle)
			{
				TCHAR dir[_MAX_DIR + 1];

				_splitpath( EudoraDir, NULL, dir, NULL, NULL );
				
				szLastDir = dir;
				
				if( szLastDir.GetLength() > 1)
				{
					// strip trailing slash, then keep only the last directory 
					// name in the path
					szLastDir = szLastDir.Left( szLastDir.GetLength() - 1 );
					szLastDir.MakeReverse();
					szLastDir = szLastDir.SpanExcluding( "/\\" );
					szLastDir.MakeReverse();
				}
			}
			else
			{
				szLastDir = FindData.cFileName;
				FindClose(FindHandle);
			}

			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_ROOT, szLastDir, NULL, FALSE );
		}

		pList = &m_theMailboxList;
	}
	else
	{
		ASSERT( pList );
	}

	pos = pList->GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCMailboxCommand* ) pList->GetNext( pos );
		switch (pCommand->GetType())
		{
		case MBT_IN:
			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_IN_MBOX, pCommand->GetName(), pCommand, pCommand->GetStatus() == US_YES );
			break;

		case MBT_OUT:
			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_OUT_MBOX, pCommand->GetName(), pCommand, pCommand->GetStatus() == US_YES );
			break;

		case MBT_JUNK:
		{
			// Don't show unread if the user doesn't want the Junk mailbox marked unread.
			BOOL	 bShowUnread = (GetIniShort(IDS_INI_JUNK_NEVER_UNREAD) ? FALSE : (pCommand->GetStatus() == US_YES));
			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_JUNK_MBOX, pCommand->GetName(), pCommand, bShowUnread );
			break;
		}

		case MBT_TRASH:
			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_TRASH_MBOX, pCommand->GetName(), pCommand, pCommand->GetStatus() == US_YES );
			break;

		case MBT_REGULAR:
			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_USER_MBOX, pCommand->GetName(), pCommand, pCommand->GetStatus() == US_YES );
			
			if( !bHasUnreadMessages )
			{
				bHasUnreadMessages = ( pCommand->GetStatus() == US_YES );
			}
			break;

		case MBT_FOLDER:
			pTreeCtrl->AddItem( QCMailboxTreeCtrl::ITEM_FOLDER, pCommand->GetName(), pCommand, FALSE );
			bUnreadChildren = InitializeMailboxTreeControl( pTreeCtrl, uLevel + 1, & ( pCommand->GetChildList() ) );

			if( bUnreadChildren )
			{
				bHasUnreadMessages = TRUE;
				pTreeCtrl->CheckItemByMenuId( pCommand->GetPathname(), TRUE );
			}
			break;

#ifdef IMAP4
		// Look for account trees at this point. We should have only account node types here!!
		case MBT_IMAP_ACCOUNT:
			{
				// If the command type is one of these, then the pCommand is a QCImapMailboxCommand object.
				ASSERT_KINDOF( QCImapMailboxCommand, ( QCImapMailboxCommand* ) pCommand );
				BOOL bImapHasUnreadMessages = InitializeImapSubtree (pTreeCtrl, uLevel, (QCImapMailboxCommand *)pCommand);
				bHasUnreadMessages = bHasUnreadMessages || bImapHasUnreadMessages;
			}
			break;
#endif
		default:
			// Shouldn't get here
			ASSERT(0);
			break;
		}
	}

	return bHasUnreadMessages;
}


void QCMailboxDirector::NewMBXFilename(
LPCTSTR		szName,
LPCTSTR		szOldPathname,
LPCTSTR		szNewDir,
LPCTSTR		szExt,
CString&	szFilename,
BOOL&		bNoFileChanges )
{
	UINT	uCount;
	CString	szNewPath;

	szNewPath = szNewDir;
	uCount = 0;
	bNoFileChanges = FALSE;

	do
	{
		szFilename = szName;
		LegalizeFilename( szFilename.GetBuffer( 0 ) );
		szFilename.ReleaseBuffer();

		if( uCount )
		{
			CString szCount;
			szCount.Format("%u", uCount);
			if (!::LongFileSupportMT(szNewDir))
				szFilename.ReleaseBuffer(8 - szCount.GetLength());
			szFilename += szCount;
		}

		szFilename += szExt;
		
		if( stricmp( szOldPathname , szNewPath + szFilename ) == 0 )
		{
			// If this is a rename and the new name is the same as the existing name,
			// then that's ok since its just going to be overwritten
        	bNoFileChanges = TRUE;
			break;
		}
		
		uCount++;

	} while ( ::FileExistsMT( szNewPath + szFilename ) );
}


void	QCMailboxDirector::NotifyClients(
QCCommandObject*	pQCCommand,
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	CPtrList*			pList = NULL;
	QCMailboxCommand*	pParent = NULL;
	QCMailboxCommand*	pCommand;
	QCMailboxCommand*	pCurrent;
	POSITION			pos = NULL;
	POSITION			next;
	POSITION			posChild;
	CTocDoc*			pTocDoc = NULL;
	UINT				uRet = IDC_UPDATE_FILTER;
	CString				szMsg;
	BOOL				bFound;
	CString				szOldPathname;
	CString				szNewPathname;
	CString				szFilename;
	BOOL				bNoFileChanges;

	if( pQCCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) )  )
	{
		pCommand = ( QCMailboxCommand* ) pQCCommand;
	}
	else
	{
		pCommand = NULL;
	}

#ifdef IMAP4
	// Handle IMAP objects separately.
	if( pQCCommand && ((QCMailboxCommand *)pQCCommand)->IsImapType () )
	{
		ImapNotifyClients( pQCCommand, theAction, pData );
		return;
	}
#endif // IMAP4

	if( ( theAction == CA_RENAME ) || 
		( theAction == CA_GRAFT ) || 
		( theAction == CA_DELETE ) )
	{
		ASSERT( pCommand );

		uRet = IDC_LEAVE_FILTER;

		if( FilterMBoxLink( pCommand->GetPathname(), pCommand->GetType() == MBT_FOLDER ) )
		{		
			if( theAction == CA_DELETE )
			{
				szMsg.Format( ( LPCTSTR )CRString(IDS_DELETE_FILTER), ( LPCTSTR ) pCommand->GetName() );
			}
			else
			{
				szMsg.Format( ( LPCTSTR )CRString(IDS_UPDATE_FILTER), ( LPCTSTR ) pCommand->GetName() );
			}

			if (!gbAutomationCall)
			{
				uRet = AlertDialog( IDD_UPDATE_FILTERS, (LPCTSTR) szMsg );
			}
			else
			{
				uRet = IDC_UPDATE_FILTER;
			}
		}
		
		if( uRet == IDCANCEL)
		{
			return;
		}

		if( ( theAction == CA_GRAFT ) )
		{
			//
			// 'pData' contains the target folder for the move, or NULL
			// if the root folder.
			//
			CPtrList* pChildList = NULL;
			if ( pData != NULL )
			{
				QCMailboxCommand* pTargetCommand = (QCMailboxCommand *) pData;	
				ASSERT_KINDOF( QCMailboxCommand, pTargetCommand );
				ASSERT( pTargetCommand->GetType() == MBT_FOLDER );
				pChildList = &(pTargetCommand->GetChildList());
				ASSERT( pChildList != NULL );
			}

			//
			// Moving item to a user folder or the root folder, so
			// check to see if there is an existing item in the target
			// folder with the same name.
			//
			if( FindByName( pChildList, pCommand->GetName() ) != NULL ) 
			{
				ErrorDialog(IDS_ERR_MAILBOX_EXISTS, pCommand->GetName() );
				return;
			}
		}

		if( pCommand->GetType() == MBT_REGULAR )
		{
			pTocDoc = GetToc( pCommand->GetPathname() );
			
			if( !pTocDoc )
			{
				ASSERT( 0 );
				return;
			}
		}
		else
		{
			pTocDoc = NULL;
		}
		
	}


	if( ( theAction == CA_RENAME ) || 
		( theAction == CA_GRAFT ) || 
		( theAction == CA_DELETE ) ||
		( theAction == CA_UPDATE_STATUS ) )
	{
		// find the parent and the appropriate list

		ASSERT( pCommand );

		pList = &m_theMailboxList;
		bFound = FALSE;
		pParent = NULL;

		while( ( bFound == FALSE ) && ( pList != NULL ) )
		{
			pos = pList->GetHeadPosition();
			
			for( next = pos; pos ; pos = next )
			{
				pCurrent = ( QCMailboxCommand* ) pList->GetNext( next );
				
				if( ( pCurrent->GetType() == pCommand->GetType() ) && 
					( pCurrent->GetPathname().CompareNoCase( pCommand->GetPathname() ) == 0 ) )
				{
					bFound = TRUE;
					break;
				}

				if( pCurrent->GetType() == MBT_FOLDER )
				{
					// see if it's in this folder
					
					if( ( pCurrent->GetPathname().GetLength() < pCommand->GetPathname().GetLength() ) &&
						( strnicmp( pCurrent->GetPathname(), pCommand->GetPathname(), strlen( pCurrent->GetPathname() ) ) == 0 ) )
					{
						pParent = pCurrent;				
						pList = & ( pCurrent->GetChildList() );
						break;		
					}
				}
			}

			if( pos == NULL )
			{
				// this should never happen...
				ASSERT( 0 );
				return;
			}
		}

	}

	if( ( theAction == CA_DELETE ) &&
		( pCommand->GetType() == MBT_FOLDER ) )
	{
		// We need to recursively delete subfolders
		// and mailboxes.
		//
	
		posChild = pCommand->GetChildList().GetHeadPosition();

		while( posChild )
		{
			pCurrent = ( QCMailboxCommand* ) pCommand->GetChildList().GetNext( posChild );
			
			if( ( pCurrent->GetType() == MBT_FOLDER ) || 
				( pCurrent->GetType() == MBT_REGULAR ) )
			{
				szMsg = pCurrent->GetName();
				
				pCurrent->Execute( CA_DELETE );
				
				posChild = pCommand->GetChildList().GetHeadPosition();
				
				if( FindByName( &( pCommand->GetChildList() ), szMsg ) )
				{
					// the child couldn't be deleted -- bail out of the whole thing
					ASSERT( 0 );
					return;
				}
			}
		}
	

		if( pCommand->IsEmpty() == FALSE )
		{
			// don't delete the directory if in, out, trash are there
			return;
		}
	}

	QCCommandDirector::NotifyClients( pCommand, theAction, pData );
	
	if( theAction == CA_DELETE )
	{
		if ( (pCommand->GetType() == MBT_REGULAR) || (pCommand->GetType() == MBT_IMAP_MAILBOX) )
		{
			// We're deleting a POP or IMAP mailbox. Notify Search Manager of the deletion
			// so that it can update the index. Don't currently check for any special
			// mailboxes, because they can't be deleted. Note that we may want to switch
			// to a more general notification mechanism at some point in the future.
			SearchManager::Instance()->NotifyMailboxDeleted( pCommand->GetPathname() );

			// Also update the Recent mailbox list, if necessary
			UpdateRecentMailboxList(pCommand->GetPathname(), FALSE);
		}
		
		// Is this a mailbox?
		if( pCommand->GetType() == MBT_REGULAR )
		{
			// Force all message windows to be closed as well
			if (pTocDoc)
			{
				int OldMSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
				SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, TRUE);
				pTocDoc->OnCloseDocument();
				SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, ( short ) OldMSC);
			}
		
			// Remove the mailbox
			if( ::FileExistsMT( pCommand->GetPathname() ) && FAILED(::FileRemoveMT( pCommand->GetPathname() )))
			{
				ErrorDialog(IDS_ERR_FILE_DELETE, pCommand->GetPathname() );
				return;
			}

			// Check to see if any filters get affected
			if( uRet == IDC_UPDATE_FILTER )
			{
				// Change the offending mailbox to the In mailbox
				CTocDoc	*InTOC = ::GetInToc();
				FiltersUpdateMailbox( pCommand->GetPathname(), InTOC->GetMBFileName(), CRString(IDS_IN_MBOX_NAME), FALSE );
			}

			// Remove the associated TOC
			szFilename = pCommand->GetPathname();
			::SetFileExtensionMT( szFilename.GetBuffer( 0 ) , CRString( IDS_TOC_EXTENSION ) );
			szFilename.ReleaseBuffer();

			if( ::FileExistsMT( szFilename ) && FAILED(::FileRemoveMT( szFilename )))
			{
				ErrorDialog( IDS_ERR_FILE_DELETE, szFilename );
				return;
			}

			// remove it from the list
			pList->RemoveAt( pos );

			// write the map
			if( pParent == NULL )
			{
				WriteDescMapFile( EudoraDir, *pList );
			}
			else
			{
				WriteDescMapFile( pParent->GetPathname(), *pList );
			}
			
			delete pCommand;
			return;
		}
				
		//
		// If we get this far, then we're dealing with a folder rather than
		// a mailbox.  
		//
		if( pCommand->IsEmpty() == FALSE )
		{
			// don't delete the directory if in, out, trash are there
			return;
		}

		
		// Now get rid of description mapping file
		szFilename = pCommand->GetPathname() + '\\' + CRString( IDS_DESCMAP );

		if( ::FileExistsMT( szFilename ) && FAILED(::FileRemoveMT( szFilename )))
		{
			ErrorDialog( IDS_ERR_FILE_DELETE, szFilename );
			return;
		}

		if( rmdir( pCommand->GetPathname() ) == -1 )
		{
			ErrorDialog( IDS_ERR_DIR_DELETE, szFilename );
			return ;		// directory not empty, or invalid path
		}

		// remove it from the list
		pList->RemoveAt( pos );
		delete pCommand;

		// write to the parent desmap
		if( pParent == NULL )
		{
			WriteDescMapFile( EudoraDir, *pList );
		}
		else
		{
			WriteDescMapFile( pParent->GetPathname(), *pList );
		}
		
		return;
	}

	if( theAction == CA_RENAME )
	{		
		// ok -- we've got the object and the list. Now get a valid filename
		//
		// We figured out the correct list/branch for the parent folder of the
		// item being renamed. Pass it to RenameMailboxOrFolder so that it can
		// write out the correct desc map.
		RenameMailboxOrFolder(pCommand, pTocDoc, pList, (LPCTSTR)pData, (uRet == IDC_UPDATE_FILTER));
		return;
	}

	
	if( theAction == CA_GRAFT )
	{
		// remove it from the old list
		pList->RemoveAt( pos );
		
		// Update the desc map file to reflect the item's removal
		if (pParent == NULL)
			WriteDescMapFile( EudoraDir, *pList );
		else
			WriteDescMapFile( pParent->GetPathname(), *pList );

		// Figure out the parent path
		pParent = (QCMailboxCommand *) pData;	
		
		if (pParent == NULL)
			szNewPathname = EudoraDir;
		else
			szNewPathname = pParent->GetPathname() + '\\';

		// Get the old path
		szOldPathname = pCommand->GetPathname();
		
		NewMBXFilename(	pCommand->GetName(),
						szOldPathname, 
						szNewPathname,
						( LPCTSTR ) CRString(pCommand->GetType() == MBT_FOLDER ? IDS_FOLDER_EXTENSION : IDS_MAILBOX_EXTENSION), 
						szFilename,
						bNoFileChanges );

		// Notify Search Manager that it needs to stop processing the mailbox and
		// TOC before we attempt to rename them.
		SearchManager::Instance()->NotifyStopProcessingMailbox(szOldPathname);

		// Update the pathname
		pCommand->SetPathname( szNewPathname + szFilename );
		::FileRenameMT( szOldPathname, pCommand->GetPathname() );

		// Insert the command into the new location and write the desc map file
		if( pParent == NULL )
		{
			Insert( m_theMailboxList, pCommand );
			WriteDescMapFile( EudoraDir, m_theMailboxList );
		}
		else
		{
			Insert( pParent->GetChildList(), pCommand );
			WriteDescMapFile( pParent->GetPathname(), pParent->GetChildList() );
		}

		if ( (pCommand->GetType() == MBT_REGULAR) && pTocDoc )
		{
			// Update the toc
			pTocDoc->ChangeName( pCommand->GetName(), pCommand->GetPathname() );
			
		}
		
		// Check to see if any filters get affected
		if (uRet == IDC_UPDATE_FILTER)
			FiltersUpdateMailbox( szOldPathname, pCommand->GetPathname(), pCommand->GetName(), pCommand->GetType() == MBT_FOLDER );

		if (pCommand->GetType() == MBT_FOLDER)
		{
			// Folder path change, so update pathnames embedded in any open TOCs.
			::UpdatePathnamesInOpenTocs( szOldPathname, pCommand->GetPathname() );
			
			if (bNoFileChanges == FALSE)
			{
				// Now update children
				UpdateChildPaths( szOldPathname, pCommand->GetPathname(), pCommand->GetChildList() );			
			}
		}
		return;
	}


	if( theAction == CA_UPDATE_STATUS )
	{
		if( pParent == NULL )
		{
			WriteDescMapFile( EudoraDir, *pList );
			return;
		}

		// write the descmap file
		WriteDescMapFile( pParent->GetPathname(), *pList );

		// see if we need to update the parent status
		if( pParent->GetStatus() == pCommand->GetStatus() )
		{
			// nope -- just bail
			return;
		}

		// sigh -- we need to walk the list
		
		pos = pList->GetHeadPosition();

		while( pos )
		{
			pCommand = ( QCMailboxCommand* ) pList->GetNext( pos );

			ASSERT( pCommand );
			
			if( pCommand->GetStatus() == US_YES )
			{
				if( pParent->GetStatus() != US_YES )
				{
					// turn on the parent status
					pParent->Execute( CA_UPDATE_STATUS, ( void* )  US_YES );
				}
				// bail
				return;
			}
		}

		if( pParent->GetStatus() == US_YES )
		{
			// turn it off
			pParent->Execute( CA_UPDATE_STATUS, ( void* ) US_NO );
		}

		return;		
	}

}

//
//	QCMailboxDirector::RenameMailboxOrFolder()
//
//	Rename the mailbox and do all necessary bookkeeping.
//
//	Parameters:
//		pCommand[in] - Command object for mailbox or folder being renamed.
//		pTocDoc[in] - Toc doc for mailbox being renamed (or NULL if folder)
//		pMailboxList[in] - Mailbox list for parent folder of mailbox or folder being
//						   renamed. Important for correct writing of desc map.
//		szNewName[in] - New mailbox name.
//		bUpdateFilters[in] - Flag to indicate whether or not we should update filters.
//
void QCMailboxDirector::RenameMailboxOrFolder(QCMailboxCommand *pCommand,
											  CTocDoc *pTocDoc,
											  CPtrList *pMailboxList,
											  LPCTSTR szNewName,
											  BOOL bUpdateFilters)
{
	if (!pCommand || !szNewName || !*szNewName)
	{
		return;
	}

	CString				szOldPathname;
	CString				szNewPathname;
	CString				szFilename;
	TCHAR				szDrive[ _MAX_DRIVE ];
	TCHAR				szDir[ _MAX_DIR ];
	TCHAR				szFname[ _MAX_FNAME ];
	TCHAR				szExt[ _MAX_EXT ];
	BOOL				bNoFileChanges = TRUE;

	szOldPathname = pCommand->GetPathname();

	_splitpath( szOldPathname, szDrive, szDir, szFname, szExt );
	
	szNewPathname = szDrive;
	szNewPathname += szDir;		

	NewMBXFilename( szNewName,
					szOldPathname, 
					szNewPathname,
					szExt,
					szFilename,
					bNoFileChanges );

	// Notify Search Manager that it needs to stop processing the mailbox and
	// TOC before we attempt to rename them.
	SearchManager::Instance()->NotifyStopProcessingMailbox(szOldPathname);

	if( bNoFileChanges == FALSE )
	{
		// rename the .mbx or .fol
		::FileRenameMT( szOldPathname, szNewPathname + szFilename );	
	}


	if( pCommand->GetType() == MBT_REGULAR )
	{
		// update the toc
		if (pTocDoc)
			pTocDoc->ChangeName( (LPCTSTR) szNewName, szNewPathname + szFilename );
	}
	
	// Check to see if any filters get affected
	if (bUpdateFilters)
	{
		FiltersUpdateMailbox( szOldPathname, szNewPathname + szFilename, (LPCTSTR) szNewName, pCommand->GetType() == MBT_FOLDER );
	}


	// finally, change the object names, write the desc map file 

	pCommand->SetPathname( szNewPathname + szFilename );
	pCommand->SetName( (LPCTSTR) szNewName );
	WriteDescMapFile( szNewPathname, *pMailboxList );

	if( pCommand->GetType() == MBT_REGULAR )
	{
		QCCommandDirector::NotifyClients( pCommand, CA_SORT_AFTER_RENAME, (void*)szNewName );
		return;
	}

	szNewPathname += szFilename;

	//
	// Folder name change, so update pathnames embedded in any
	// open TOCs.
	//
	
	::UpdatePathnamesInOpenTocs( szOldPathname, szNewPathname );
	
	if( bNoFileChanges == FALSE )
	{
		// now update children
		UpdateChildPaths( szOldPathname, szNewPathname, pCommand->GetChildList() );			
	}

	QCCommandDirector::NotifyClients( pCommand, CA_SORT_AFTER_RENAME, (void*)szNewName );
}


void QCMailboxDirector::UpdateChildPaths(
const CString&	szOldPathname,
const CString&	szNewPathname,
const CPtrList&	theList )
{
	QCMailboxCommand*	pCommand;
	POSITION			pos;
	CString				szPathname;

	pos = theList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCMailboxCommand* ) theList.GetNext( pos );
		szPathname = pCommand->GetPathname();
		szPathname = szPathname.Right( szPathname.GetLength() - szOldPathname.GetLength() );
		szPathname = szNewPathname + szPathname;

		if( pCommand->GetType() == MBT_FOLDER )
		{
			UpdateChildPaths( pCommand->GetPathname(), szPathname, pCommand->GetChildList() );
		}

		pCommand->SetPathname( szPathname );
	}
}



///////////////////////////////////////////////////////////////////////////////
//
// NewMessageCommands
//
// This routine has two functionalities:
//		1. Assign command IDs to mailboxes
//		2. Populate a menu with the appropriate menu items
//
// Return Value:
//		None, but the popup will contain all the appropriate menu entries,
//		or the commands will be added to the stack.
//
///////////////////////////////////////////////////////////////////////////////
void QCMailboxDirector::NewMessageCommands(BOOL bIsTransfer, CMenu* pPopupMenu)
{
	NewMessageCommandsLevel(NULL, bIsTransfer, pPopupMenu);
}

void QCMailboxDirector::NewMessageCommandsLevel(QCMailboxCommand* pParentCommand,
												BOOL bIsTransfer, CMenu* pPopupMenu)
{
	ASSERT(pPopupMenu);

	const COMMAND_ACTION_TYPE theAction =	bIsTransfer? CA_TRANSFER_TO  : CA_OPEN;
	const COMMAND_ACTION_TYPE theNewAction=	bIsTransfer? CA_TRANSFER_NEW : CA_NEW_MAILBOX;
	BOOL bFoundNonSysMailbox = FALSE;
	QCMailboxCommand* pCommand;
	CPtrList& theList = pParentCommand? pParentCommand->GetChildList() : m_theMailboxList;
	POSITION pos = theList.GetHeadPosition();

	while (pos)
	{
		pCommand = (QCMailboxCommand*)theList.GetNext(pos);
		int ThisItemPos = -1;
		
		// Need to add some items at the beginning of the menu,
		// but after all the system maliboxes
		if (!bFoundNonSysMailbox && pCommand->GetType() > MBT_TRASH)
		{
			// Add separator if there's already some menu items in here.
			// This should happen only at the very top-level
			if (pPopupMenu->GetMenuItemCount() > 0)
			{
				ASSERT(&theList == &m_theMailboxList);
				pPopupMenu->AppendMenu(MF_SEPARATOR);

				// Add the Recent mailboxes menu
				if (GetIniShort(IDS_INI_MAX_RECENT_MAILBOX))
				{
					CRecentMailboxMenu* pNewRecentMenu = DEBUG_NEW CRecentMailboxMenu(bIsTransfer);
					pNewRecentMenu->CreatePopupMenu();
					pPopupMenu->AppendMenu(MF_POPUP, (UINT)pNewRecentMenu->GetSafeHmenu(), CRString(IDS_RECENT_MBOX_MENU));
					pPopupMenu->AppendMenu(MF_SEPARATOR);
				}
			}

			// Add "New..." menu item
			QCMailboxCommand* pCommandForNewAction = pParentCommand? pParentCommand : pCommand;
			WORD wID = g_theCommandStack.AddCommand(pCommandForNewAction, theNewAction);
			if (wID)
				pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, CRString(IDS_MAILBOX_NEW));
	
			// If this is a selectable IMAP mailbox that can have children, put in the "This Mailbox" item.
			if (pParentCommand && pParentCommand->IsImapType())
			{
				QCImapMailboxCommand* pImapCommand = (QCImapMailboxCommand*)pParentCommand;
				if (!pImapCommand->IsNoSelect())
				{
					wID = g_theCommandStack.AddCommand(pParentCommand, theAction);

					if (wID)
					{
						
						// Insert separator after New....
						pPopupMenu->AppendMenu(MF_SEPARATOR);

						// The item.
						pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, CRString(IDS_IMAP_THIS_MAILBOX));
						
						

						ThisItemPos = pPopupMenu->GetMenuItemCount() - 1;

						// Insert separator between This Mailbox and children; if children exist
						if ((pParentCommand->GetChildList().GetCount()) > 0)
							pPopupMenu->AppendMenu(MF_SEPARATOR);
					}
				}
			}

			bFoundNonSysMailbox = TRUE;
		}

		// IMAP:
		//   We only want to show leaf menus (non-folders menus which have no children) when the
		//   server supports it and the INI says to show it.
		//
		// smanjo, 8/4/98
		if (pCommand->GetType() == MBT_FOLDER ||
				(pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShouldShowAsSubMenu()))
#if 0 
			|| ( ((GetIniShort(IDS_INI_IMAP_LEAFMENU)) || ((pCommand->GetChildList().GetCount()) > 0)) && (pCommand->CanHaveChildren()) ) )
#endif 
		{
			// Put a separator before IMAP ACCOUNTS:
			if (pCommand->GetType() == MBT_IMAP_ACCOUNT)
				pPopupMenu->AppendMenu(MF_SEPARATOR);

			// build the submenu
			CDynamicMailboxMenu* pSubMenu = DEBUG_NEW CDynamicMailboxMenu(pCommand, bIsTransfer);
			pSubMenu->CreatePopupMenu();

			// We don't do this anymore because we now build the submenus as the user
			// opens them up so as to conserve resources
			// NewMessageCommandsLevel(pCommand, bIsTransfer, pPopupMenu);

			pPopupMenu->AppendMenu(MF_POPUP, (UINT)pSubMenu->GetSafeHmenu(), pCommand->GetName());

			// Tell IMAP command objects that they're shown as sub-menus:
			if (pCommand->IsImapType())
				((QCImapMailboxCommand *)pCommand)->SetShownAsSubMenu(TRUE, bIsTransfer);
		}
		else
		{
			// Just a regular mailbox
			WORD wID = g_theCommandStack.AddCommand(pCommand, theAction);

			ASSERT(wID);
			if (wID)
			{
				MailboxType type = pCommand->GetType();
				if (theAction == CA_OPEN && type <= MBT_TRASH)
				{
					UINT StringID = 0;
					switch (type)
					{
					case MBT_IN:	StringID = IDS_IN_MBOX_MENU;	break;
					case MBT_OUT:	StringID = IDS_OUT_MBOX_MENU;	break;
					case MBT_JUNK:	StringID = IDS_JUNK_MBOX_MENU;	break;
					case MBT_TRASH:	StringID = IDS_TRASH_MBOX_MENU;	break;
					default: ASSERT(0); break;
					}

					CRString szName(StringID);
					pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, szName);
				}
				else
					pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, pCommand->GetName());
			}
		}

		if (ThisItemPos == -1)
			ThisItemPos = pPopupMenu->GetMenuItemCount() - 1;
		if (bIsTransfer)
			pPopupMenu->SetMenuItemBitmaps(ThisItemPos, MF_BYPOSITION, &m_theTransferBitmap, NULL);
		else
		{
			pPopupMenu->SetMenuItemBitmaps(ThisItemPos, MF_BYPOSITION, NULL, &m_theUnreadStatusBitmap);
			
			// Check the menu item if there are unread messages and either it is not the Junk mailbox
			// or the user doesn't mind the Junk mailbox showing as unread.
			if ((pCommand->GetStatus() == US_YES) &&
				((!pCommand->IsJunk() || !GetIniShort(IDS_INI_JUNK_NEVER_UNREAD))))
			{
				pPopupMenu->CheckMenuItem(ThisItemPos, MF_BYPOSITION | MF_CHECKED);
			}
		}
	}

	// Need to add some items at the beginning of the menu,
	// but after all the system maliboxes
	if (!bFoundNonSysMailbox)
	{
		if (pPopupMenu->GetMenuItemCount() == 0)
			VERIFY(pCommand = pParentCommand);
		else
		{
			ASSERT(&theList == &m_theMailboxList);
			pPopupMenu->AppendMenu(MF_SEPARATOR);
			pCommand = (QCMailboxCommand*)theList.GetTail();
		}

		// Add "New..." menu item
		WORD wID = g_theCommandStack.AddCommand(pCommand, theNewAction);
		if (wID)
			pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, CRString(IDS_MAILBOX_NEW));
	}
}



QCMailboxCommand* QCMailboxDirector::FindByPathname(LPCTSTR szPathname)
{
	CPtrList* pList = &m_theMailboxList;

	POSITION pos = pList->GetHeadPosition();

	while (pos)
	{
		QCMailboxCommand* pCurrent = (QCMailboxCommand*)pList->GetNext(pos);


		if (pCurrent->GetPathname().CompareNoCase(szPathname) == 0)
			return pCurrent;

		// Need to modify this to account for the fact that IMAP mailboxes are housed in 
		// directories.
		CString Objpath;
		if (pCurrent->IsImapType())
			((QCImapMailboxCommand*)pCurrent)->GetObjectDirectory (Objpath);
		else
			Objpath = pCurrent->GetPathname ();

		// Can have mailbox names like mbox and mbox0!
		if (strlen (szPathname) > (size_t) Objpath.GetLength() &&
			strnicmp(Objpath, szPathname, Objpath.GetLength()) == 0 &&
			szPathname[Objpath.GetLength()] == '\\')
		{
			// found a parent
			pList = &(pCurrent->GetChildList());
			pos = pList->GetHeadPosition();
		}
	}

	return NULL;
}

//
//	QCMailboxDirector::FindByNameInTree()
//
//	FindByName() with the addition that it works on the entire mailbox tree.
//
QCMailboxCommand *QCMailboxDirector::FindByNameInTree(CPtrList* pList, LPCTSTR szName)
{
	if (pList == NULL)
	{
		pList = &m_theMailboxList;
	}

	POSITION			 pos = NULL;
	QCMailboxCommand	*pCommand = NULL;

	pos = pList->GetHeadPosition();

	while(pos)
	{
		pCommand = (QCMailboxCommand*)pList->GetNext(pos);
		if (pCommand->GetType() == MBT_FOLDER)
		{
			pCommand = FindByNameInTree(&(pCommand->GetChildList()), szName);
			if (pCommand)
			{
				return pCommand;
			}
		}
		else
		{
			if (stricmp(pCommand->GetName(), szName) == 0)
			{
				return pCommand;
			}
		}
	}
	return NULL;
}

BOOL QCMailboxDirector::DoCompactMailboxes(const CPtrList& theList)
{
	POSITION pos = theList.GetHeadPosition();
	BOOL bRet = TRUE;
	
	while (pos && bRet)
	{
		QCMailboxCommand* pCommand = (QCMailboxCommand*)theList.GetNext(pos);
		
		// Changed (JOK - 4/13/98) to handle IMAP mailboxes.
		// Handles all cases where a mailbox/folder
		// can have child mailboxes (including IMAP).
		//
		if (pCommand->CanContainMessages())
		{
			bRet = pCommand->CompactMailbox(NULL) != CRT_DRASTIC_ERROR;
			if (bRet)
				pCommand->NotifyDirector(CA_COMPACT, NULL);
		}

		// Compact children?
		if (pCommand->CanHaveChildren())
			bRet = DoCompactMailboxes(pCommand->GetChildList());
	}

	return bRet;
}

BOOL QCMailboxDirector::CompactMailboxes()
{	
	MainProgress(CRString(IDS_COMPACTING));
	Progress(-1, NULL, -1);

	BOOL bRet = DoCompactMailboxes(m_theMailboxList);

	CloseProgress();

	return bRet;
}


BOOL QCMailboxDirector::CompactAMailbox(CTocDoc* pTocDoc, bool bUserRequested)
{
	QCMailboxCommand* pCommand = FindByPathname(pTocDoc->GetMBFileName());

	if (!pCommand)
	{
		ASSERT(0);
		return FALSE;
	}

	MainProgress(CRString(IDS_COMPACTING));
	Progress(-1, NULL, -1);

	BOOL bRet = pCommand->CompactMailbox(pTocDoc, bUserRequested) != CRT_DRASTIC_ERROR;
	
	if (bRet)
		pCommand->NotifyDirector(CA_COMPACT, pTocDoc);

	CloseProgress();

	return bRet;
}


//
//	QCMailboxDirector::CreateJunkMailbox()
//
//	Creates a system Junk mailbox.
//
void QCMailboxDirector::CreateJunkMailbox()
{
	QCMailboxCommand	*pCommand = NULL;
	TCHAR				 szDrive[_MAX_DRIVE];
	TCHAR				 szDir[_MAX_DIR];
	CString				 strDir;
	CRString			 strInName(IDS_IN_MBOX_NAME);
	CRString			 strJunkName(IDS_JUNK_MBOX_NAME);
	CRString			 strMBXExt(IDS_MAILBOX_EXTENSION);
	CString				 strJunkFullName;
	JJFile				 jjfNewJunkFile;

	// Find where the Junk mailbox should go by finding where the In mailbox is.
	pCommand = FindByName(&m_theMailboxList, strInName);
	_splitpath(pCommand->GetPathname(), szDrive, szDir, NULL, NULL);
	strDir = szDrive;
	strDir += szDir;

	// Create the Junk mailbox file.
	strJunkFullName = strDir + strJunkName + strMBXExt;
	if (!::FileExistsMT(strJunkFullName))
	{
		if (SUCCEEDED(jjfNewJunkFile.Open(strJunkFullName, O_CREAT | O_RDWR)))
		{
			jjfNewJunkFile.Close();
		}
	}

	// Create the Junk mailbox object.
	pCommand = DEBUG_NEW QCMailboxCommand(this, strJunkName, strJunkFullName, MBT_JUNK);
	Insert(m_theMailboxList, pCommand);
	QCCommandDirector::NotifyClients(pCommand, CA_NEW);
	QCCommandDirector::NotifyClients(pCommand, CA_SORT_AFTER_RENAME);

	// Save the changes.
	WriteDescMapFile(strDir, m_theMailboxList);
}


//
//	QCMailboxDirector::BlessJunkMailbox()
//
//	Converts an existing regular Junk mailbox into a system mailbox.
//
void QCMailboxDirector::BlessJunkMailbox(QCMailboxTreeCtrl *pTreeCtrl)
{
	if (!pTreeCtrl)
	{
		return;
	}

	POSITION			 pos = NULL;
	QCMailboxCommand*	 pCommand = NULL;
	CRString			 strJunkName(IDS_JUNK_MBOX_NAME);

	// Find the existing Junk mailbox.
	pos = m_theMailboxList.GetHeadPosition();
	while (pos)
	{
		pCommand = (QCMailboxCommand*)m_theMailboxList.GetNext(pos);

		if (strJunkName.CompareNoCase(pCommand->GetName()) == 0)
		{
			// Change the mailbox to a system mailbox.
			pCommand->SetType(MBT_JUNK);

			// Update the mailbox tree.
			pTreeCtrl->RetypeItemByMenuId(pCommand->GetPathname(), QCMailboxTreeCtrl::ITEM_JUNK_MBOX);
			QCCommandDirector::NotifyClients(pCommand, CA_SORT_AFTER_RENAME);

			// Save the changes.
			TCHAR				 szDrive[_MAX_DRIVE];
			TCHAR				 szDir[_MAX_DIR];
			CString				 strDir;
			_splitpath(pCommand->GetPathname(), szDrive, szDir, NULL, NULL);
			strDir = szDrive;
			strDir += szDir;
			WriteDescMapFile(strDir, m_theMailboxList);

			break;
		}
	}
}


//
//	QCMailboxDirector::RenameJunkMailbox()
//
//	Renames the user's regular junk mailbox and lets them know what
//	name it was changed to.
//
//	Parameters
//		pTreeCtrl[in] - Mailbox tree control.
//
void QCMailboxDirector::RenameJunkMailbox(QCMailboxTreeCtrl *pTreeCtrl)
{
	if (!pTreeCtrl)
	{
		return;
	}

	QCMailboxCommand	*pCommand = NULL;
	TCHAR				 szDrive[_MAX_DRIVE];
	TCHAR				 szDir[_MAX_DIR];
	CString				 strDir;
	CRString			 strJunkName(IDS_JUNK_MBOX_NAME);
	CRString			 strMBXExt(IDS_MAILBOX_EXTENSION);
	CRString			 strUserJunkBaseName(IDS_USER_JUNK_MBOX_NAME);
	CString				 strJunkFullName;
	CString				 strUserJunkName;
	CString				 strUserJunkFullName;
	CString				 strOldUserJunkFullName;
	int					 i = 0;
	char				 strUniqueNum[5];

	// Find the existing Junk mailbox.
	pCommand = FindByName(&m_theMailboxList, strJunkName);

	if (!pCommand)
	{
		// Shouldn't happen, but...
		return;
	}

	// Find a unique name to change the Junk mailbox (start with
	// the basename and append numbers as needed until we find
	// a name that isn't taken).
	strUserJunkName = strUserJunkBaseName;
	_splitpath(pCommand->GetPathname(), szDrive, szDir, NULL, NULL);
	strDir = szDrive;
	strDir += szDir;
	strUserJunkFullName = strDir + strUserJunkName + strMBXExt;
	for (i = 1; ::FileExistsMT(strUserJunkFullName) && (i < 10000); ++i)
	{
		sprintf(strUniqueNum, "%d", i);
		strUserJunkName = strUserJunkBaseName + strUniqueNum;
		strUserJunkFullName = strDir + strUserJunkName + strMBXExt;
	}

	// This is ridiculous, but..
	if (i == 10000)
	{
		return;
	}

	CTocDoc *pTocDoc = GetToc( pCommand->GetPathname() );

	if( !pTocDoc )
	{
		ASSERT( 0 );
		return;
	}

	// Rename the mailbox.
	RenameMailboxOrFolder(pCommand, pTocDoc, &m_theMailboxList, strUserJunkName);

	// Update the mailbox tree.
	pTreeCtrl->RenameItemByMenuId(strUserJunkFullName, strUserJunkName);
	QCCommandDirector::NotifyClients(pCommand, CA_SORT_AFTER_RENAME);

	// Let the user know what just happened.
	AlertDialog(IDD_RENAMED_JUNK_DIALOG, strUserJunkName);	
}


QCMailboxCommand* QCMailboxDirector::FindParent(QCMailboxCommand* pCommand)
{
	// If pCommand is and IMAP mailbox command object, defer to "ImapFindParent".
	if (pCommand && pCommand->IsImapType())
		return (QCMailboxCommand *) ImapFindParent ((QCImapMailboxCommand *)pCommand);

	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	CString	szParentPathname;

	_splitpath(pCommand->GetPathname(), szDrive, szDir, szFname, szExt);

	szParentPathname = szDrive;
	szParentPathname += szDir;

	if (!szParentPathname.CompareNoCase(EudoraDir))
		return NULL;

	if (szParentPathname.Right(1) == '\\')
		szParentPathname = szParentPathname.Left(szParentPathname.GetLength() - 1);

	return FindByPathname(szParentPathname);
}


////////////////////////////////////////////////////////////////////////
// CreateTargetMailbox [public]
//
// Generic routine for prompting the user for a new Mailbox or Folder
// name.  If the user chooses a Folder, then automatically ask the
// user again for another Mailbox or Folder name.  If the user ultimately
// creates a mailbox, return it to the caller so that the caller can
// execute a command on the mailbox object as needed.
////////////////////////////////////////////////////////////////////////
QCMailboxCommand*	QCMailboxDirector::CreateTargetMailbox(
QCMailboxCommand*	pCommand,		//(i) NULL indicates root mailbox
BOOL				bIsTransfer )	//(i) TRUE means Transfer New cmd, FALSE means Mailbox New cmd
{
	if (pCommand)
		ASSERT_KINDOF( QCMailboxCommand, pCommand );

	// If its a local mailbox (not folder), then assume we're at the top level
	if (pCommand && !pCommand->IsImapType() && pCommand->GetType() != MBT_FOLDER)
		pCommand = NULL;

	while (1)
	{
		CNewMailboxDlg theDialog(pCommand ? pCommand->GetName() : NULL,
									pCommand ? &( pCommand->GetChildList() ) : NULL,
									bIsTransfer);

		if (theDialog.DoModal() == IDOK)
		{
			//
			// User wants to create a new Mailbox/Folder, so do it.
			//
			pCommand = AddCommand(theDialog.GetName(), theDialog.GetType(), (QCMailboxCommand*)pCommand);
			if (!pCommand)
				return NULL;			// Ouch.  Mailbox/Folder creation failure.
		
			switch (pCommand->GetType())
			{
			case MBT_REGULAR:
			case MBT_IMAP_MAILBOX:
				// The dialog UI has the option to "don't transfer,
				// just create mailbox".  So, if we're doing a
				// transfer and the user chooses to just create a
				// mailbox, then return NULL to the caller to indicate
				// that the mailbox is not a Transfer target.
				//
				// Otherwise, return the newly-created mailbox command object
				// to the caller.
				if (bIsTransfer && !theDialog.DoTransfer())
					return NULL;
				return pCommand;

			case MBT_FOLDER:
				// User created a Folder, but the real point of all this
				// is to create a Mailbox.  Run through the loop again 
				// with the updated 'pCommand' to see if the user wants
				// to create a Mailbox in the newly-created Folder.
				break;

			default:
				ASSERT(0);
				return NULL;
			}
		}
		else
		{
			// User canceled so we're outta here.
			return NULL;
		}
	}

	ASSERT(0);		// should never get here
	return NULL;
}

void QCMailboxDirector::BuildRecentMailboxesList()
{
	if (s_RecentMailboxList.empty() == false || GetIniShort(IDS_INI_MAX_RECENT_MAILBOX) == 0)
		return;

	CRString SectionName(IDS_RECENT_MAILBOX_SECTION);
	CRString EntryFormat(IDS_RECENT_MAILBOX_ENTRY);
	const int NumEntries = GetIniShort(IDS_INI_MAX_RECENT_MAILBOX);

	for (int i = 1; i <= NumEntries; i++)
	{
		CString MailboxPath;
		char EntryName[64];

		sprintf(EntryName, EntryFormat, i);
		GetEudoraProfileString(SectionName, EntryName, &MailboxPath);
		if (MailboxPath.IsEmpty() == FALSE)
		{
			// Trick to put the pointer to the relative path in the filename on the list.
			// It will remain constant as the QCMailboxCommand object won't be deleted
			// until the app shuts down.
			QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname(EudoraDir + MailboxPath);
			if (pCommand) s_RecentMailboxList.push_back(((LPCTSTR)pCommand->GetPathname()) + EudoraDirLen);
		}
	}

	// Put the In mailbox in the list if there's nothing in it, just so that it's not empty
	if (s_RecentMailboxList.empty())
		s_RecentMailboxList.push_front(CRString(IDS_IN_MBOX_FILENAME) + CRString(IDS_MAILBOX_EXTENSION));
}

void QCMailboxDirector::UpdateRecentMailboxList(LPCTSTR MailboxPathname, BOOL bIsAdd)
{
	BOOL bMadeChange = FALSE;

	// Empty mailbox pathname signals a change in the number of max items
	if (!MailboxPathname)
		bMadeChange = TRUE;
	else
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname(MailboxPathname);

		if (pCommand)
		{
			// Trick to get the pointer to the relative path in the filename on the list.
			// It will remain constant as the QCMailboxCommand object won't be deleted
			// until the app shuts down.
			LPCTSTR RelativePathname = ((LPCTSTR)pCommand->GetPathname()) + EudoraDirLen;

			if (bIsAdd)
			{
				// Don't need to do anything if the mailbox is already at the front of the list
				if (s_RecentMailboxList.empty() || *(s_RecentMailboxList.begin()) != RelativePathname)
				{
					s_RecentMailboxList.remove(RelativePathname);
					s_RecentMailboxList.push_front(RelativePathname);
					bMadeChange = TRUE;
				}
			}
			else
			{
				const size_t ListSize = s_RecentMailboxList.size();
				s_RecentMailboxList.remove(RelativePathname);
				if (ListSize != s_RecentMailboxList.size())
					bMadeChange = TRUE;
			}
		}
	}

	if (bMadeChange)
	{
		// Truncate the list size based on the setting
		const size_t MaxItems = GetIniShort(IDS_INI_MAX_RECENT_MAILBOX);
		if (s_RecentMailboxList.size() > MaxItems)
			s_RecentMailboxList.resize(MaxItems);

		// Save list back out to the INI file
		CRString SectionName(IDS_RECENT_MAILBOX_SECTION);
		CRString EntryFormat(IDS_RECENT_MAILBOX_ENTRY);
		int i = 1;
		for (std::list<LPCTSTR>::iterator it = s_RecentMailboxList.begin(); it != s_RecentMailboxList.end(); ++it, i++)
		{
			char EntryName[64];
			sprintf(EntryName, EntryFormat, i);
			WriteEudoraProfileString(SectionName, EntryName, *it);
		}

		// Clear out the menus so that they get rebuilt the next time they are opened
		CDynamicMenu::DeleteMenuObjects(g_RecentMailboxesMenu, TRUE);
		CDynamicMenu::DeleteMenuObjects(g_RecentTransferMenu, TRUE);

		((QCMailboxTreeCtrl*)CMainFrame::QCGetMainFrame()->GetActiveMboxTreeCtrl())->UpdateRecentFolder();
	}
}
