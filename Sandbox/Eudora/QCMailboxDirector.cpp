// QCMailboxDirector.cpp: implementation of the QCMailboxDirector class.
//
//////////////////////////////////////////////////////////////////////

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

#ifdef IMAP4
#include "QCImapMailboxCommand.h"
#include "imapfol.h"
#endif

#include "rs.h"
#include "guiutils.h"
#include "fileutil.h"
#include "tocdoc.h"
#include "progress.h"
#include "newmbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CString			EudoraDir;
extern QCCommandStack	g_theCommandStack;

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
	while( m_theMailboxList.IsEmpty() == FALSE )
	{
		delete ( QCMailboxCommand* ) m_theMailboxList.RemoveTail();
	}
}


LPTSTR QCMailboxDirector::LegalizeFilename(LPTSTR pFilename)
{
	if (NULL == pFilename)
	{
		ASSERT(0);
		return NULL;
	}

	// Remove spaces and other nasty characters from the filename
	::StripIllegalMT(pFilename, EudoraDir);
	
	// Truncate the filename for 8.3 file systems
	if (strlen(pFilename) > 8 && !::LongFileSupportMT(EudoraDir))
	{
		pFilename[8] = 0;	
	}
	
	return pFilename;
}


// Salvage
// Searches through the directory for mailboxes and folders,
// and adds them if they're not already there
BOOL QCMailboxDirector::Salvage( 
LPCTSTR		szPath,
CPtrList&	theList )
{
	TCHAR				Filename[_MAX_PATH + 1];
	BOOL				ChangeMade = FALSE;
	QCMailboxCommand*	pCommand;

	strcpy( Filename, szPath );
	int PathLen = strlen(Filename);

	// If this is the top level, then make sure In, Out, and Trash mailboxes and entries exist
	if ( stricmp( szPath, EudoraDir ) == 0)
	{
		CRString InFilename(IDS_IN_MBOX_FILENAME);
		CRString OutFilename(IDS_OUT_MBOX_FILENAME);
		CRString TrashFilename(IDS_TRASH_MBOX_FILENAME);
		CRString InName(IDS_IN_MBOX_NAME);
		CRString OutName(IDS_OUT_MBOX_NAME);
		CRString TrashName(IDS_TRASH_MBOX_NAME);
		CRString Extension(IDS_MAILBOX_EXTENSION);
		JJFile  CreateFile;

		strcpy(Filename + PathLen, InFilename);
		strcat(Filename + PathLen, Extension);
		if (! ::FileExistsMT(Filename))
		{
			if ( SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)) )
				CreateFile.Close();
		}
		
		if ( ( pCommand = FindByFilename( theList, Filename ) ) == NULL )
		{
			Insert( theList, new QCMailboxCommand( this, InName, Filename, MBT_IN ) );
			ChangeMade = TRUE;
		}
		else
		{
			if( pCommand->GetType() != MBT_IN )
			{
				pCommand->SetType( MBT_IN );
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
		if (! ::FileExistsMT(Filename))
		{
			if ( SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)) )
				CreateFile.Close();
		}

		if ( ( pCommand = FindByFilename( theList, Filename ) )  == NULL )
		{
			Insert( theList, new QCMailboxCommand( this, OutName, Filename, MBT_OUT));
			ChangeMade = TRUE;
		}
		else
		{
			if( pCommand->GetType() != MBT_OUT )
			{
				pCommand->SetType( MBT_OUT );
				ChangeMade = TRUE;
			}
			if (stricmp(pCommand->GetName(), OutName))
			{
				pCommand->SetName(OutName);
				ChangeMade = TRUE;
			}
		}
		
		strcpy(Filename + PathLen, TrashFilename);
		strcat(Filename + PathLen, Extension);
		if (! ::FileExistsMT(Filename))
		{
			if ( SUCCEEDED(CreateFile.Open(Filename, O_CREAT | O_RDWR)) )
				CreateFile.Close();
		}

		if ( ( pCommand = FindByFilename(theList, Filename) ) == NULL )
		{
			Insert( theList, new QCMailboxCommand( this, TrashName, Filename, MBT_TRASH));
			ChangeMade = TRUE;
		}
		else
		{
			if( pCommand->GetType() != MBT_TRASH )
			{
				pCommand->SetType( MBT_TRASH );
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
	wsprintf(Filename + PathLen, "*%s", (LPCTSTR)CRString(IDS_MAILBOX_EXTENSION));
	
	WIN32_FIND_DATA Find;
	Find.dwFileAttributes = _A_NORMAL;
	
	HANDLE FindHandle = FindFirstFile(Filename,&Find);
	int Result = 0;
	
	if (FindHandle != INVALID_HANDLE_VALUE) 
		Result = 1;
	while (Result)
	{
		wsprintf(Filename + PathLen, "%s", Find.cFileName);
		
		if ( FindByFilename( theList, Filename ) == NULL )
		{
			::SetFileExtensionMT(Filename, "toc");
			TCHAR Name[32];
			Name[0] = 0;
			JJFile Toc(JJFileMT::BUF_SIZE, FALSE);

			if (FAILED(Toc.Open(Filename, O_RDONLY)) || 
				FAILED(Toc.GetLine(Name, 8)) || 				// dummy read to skip 8-byte version info
				FAILED(Toc.GetLine(Name, 32)) ||				// real read to get 32-byte name
				( FindByName(&theList, Name) != NULL ) )	// check for duplicate name at this folder level
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

			if (0 == Name[0])
			{
				//
				// Auto-generate a mailbox name based on the base
				// portion of the MBX filename.
				//
				strcpy(Name, Find.cFileName);
				ASSERT(strlen(Name) < sizeof(Name));
				LPTSTR dot = strrchr(Name, '.');
				if (dot)
					*dot = 0;
			}
			Toc.Close();
			::SetFileExtensionMT(Filename, "mbx");			
			Insert( theList, new QCMailboxCommand(this, Name, Filename, MBT_REGULAR));
			ChangeMade = TRUE;
		}
		Result = FindNextFile(FindHandle,&Find);
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
		if (Find.dwFileAttributes == _A_SUBDIR && strcmp(Find.cFileName, ".") && strcmp(Find.cFileName, ".."))
        {
			CString szPathname( szPath );
			szPathname += Find.cFileName;

			if ( FindByFilename( theList, szPathname ) == NULL )
			{
				TCHAR Name[32];
				strcpy(Name, Find.cFileName);
				ASSERT(strlen(Name) < sizeof(Name));
				LPTSTR dot = strrchr(Name, '.');
				if (dot)
					*dot = 0;

				Insert( theList, new QCMailboxCommand( this, Name, szPathname , MBT_FOLDER ) );
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


BOOL QCMailboxDirector::ProcessDescMapFile(
LPCTSTR		szPath,
CPtrList&	theMailboxList )
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
		//		Name,Filename,TypeChar
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
						// Localized versions are supposed to use IN.MBX, OUT.MBX,
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

	//		if (autoDelete)
	//		{
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
				}
	//		}

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
					Insert( theMailboxList, new QCMailboxCommand( this, buf, szMbxPathname, theType, theStatus ) );
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


BOOL	QCMailboxDirector::Build()
{
	int nCheckMenuHeight = ::GetSystemMetrics(SM_CYMENUCHECK);

	QCLoadBitmap( IDB_UNREAD_STATUS, m_theUnreadStatusBitmap );
	QCLoadBitmap( IDB_TRANSFER_MENU, m_theTransferBitmap );

	if( GetIniShort( IDS_INI_CENTER_UNREAD_STATUS ) )
	{
		// Center the bitmaps vertically in the space available
		::FitTransparentBitmapToHeight(m_theUnreadStatusBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(m_theTransferBitmap, nCheckMenuHeight);
	}

#ifndef IMAP4
	return ProcessDescMapFile( EudoraDir, m_theMailboxList );
#else
	// Build the IMAP account subtrees for the personality database.
	// Note: We don't want to process the IMAP stuff if the regular stuff failed.

	return ProcessDescMapFile( EudoraDir, m_theMailboxList ) && BuildImapAccounts ();

#endif // IMAP4
}


QCMailboxCommand*	QCMailboxDirector::FindByFilename(
CPtrList&	theList,
LPCTSTR		szPathname )
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
QCMailboxCommand*	pCommand )
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
				return pos;
			}
		}

		return  theList.AddHead( pCommand );
	}


	if(	( pCommand->GetType() == MBT_OUT ) ||
		( pCommand->GetType() == MBT_TRASH ) )
	{
		for( ; pos != NULL ; pos = next )
		{
			pCurrent = ( QCMailboxCommand* ) theList.GetNext( next );
		
			if( pCurrent->GetType() == pCommand->GetType() ) 
			{
				// its already in the list
				return pos;
			}

			if( ( pCurrent->GetType() != MBT_IN ) &&
				( pCurrent->GetType() != MBT_OUT ) ) 
			{
				// we found the position
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
	CString				TempFilename(TempDir + CRString(IDS_DESCMAP_TMP));
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



QCMailboxCommand*	QCMailboxDirector::AddCommand(
LPCTSTR				szName,
MailboxType			theType,
QCMailboxCommand*	pParent )
{
	CPtrList*			pTheList;	
	QCMailboxCommand*	pCommand;
	CString				szPathname;
	CString				szFilename;
	CString				szDescMapPathname;
	JJFile				NewFile;
	CString				szDescMapPath;
	int					Status;
	BOOL				bNoChanges;

#ifdef IMAP4
	// Redirect IMAP add commands to "ImapAddCommand()"
	if ( pParent && pParent->IsImapType())
	{
		return ImapAddCommand ( szName, theType, pParent );
	}
#endif

	if( pParent != NULL )
	{
		ASSERT( pParent->GetType() == MBT_FOLDER );
		pTheList = &( pParent->GetChildList() );
		szPathname = pParent->GetPathname() + '\\';
	}
	else
	{
		pTheList = &m_theMailboxList;
		szPathname = EudoraDir;
	}

	pCommand = FindByName( pTheList, szName );

	if( pCommand != NULL )
	{
		return pCommand;
	}

	szDescMapPath = szPathname;

	NewMBXFilename( szName,
					"",
					szPathname,
					( theType == MBT_FOLDER ) ? CRString( IDS_FOLDER_EXTENSION ) : CRString( IDS_MAILBOX_EXTENSION ),
					szFilename,
					bNoChanges );

	szPathname += szFilename;

	pCommand = new QCMailboxCommand( this, szName, szPathname, theType, US_NO );

	if( pCommand == NULL )
	{
		return pCommand;
	}

	if( theType == MBT_FOLDER )
	{
		Status = mkdir( szPathname );

		if (Status != 0)
		{
			ErrorDialog( IDS_ERR_DIR_CREATE, szPathname );
			delete pCommand;
			return NULL;
		}
		
		szDescMapPathname = szPathname + "\\" + CRString( IDS_DESCMAP );

		if ( FAILED(NewFile.Open( szDescMapPathname, O_WRONLY | O_CREAT )) )
		{
			delete pCommand;
			return NULL;
		}
		
		NewFile.Close();
	}
	else
	{
		// Create the mailbox
		if ( FAILED(NewFile.Open( szPathname, O_WRONLY | O_CREAT )) )
		{
			delete pCommand;
			return NULL;
		}
		
		NewFile.Close();

	}

	Insert( *pTheList, pCommand );
	
	if( theType != MBT_FOLDER )
	{
		// Build the TOC by calling GetToc.
		GetToc( szPathname, szName );
	}

	WriteDescMapFile( szDescMapPath, *pTheList );

	NotifyClients( pCommand, CA_NEW );

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
	CPtrList*			pList;
	QCMailboxCommand*	pParent;
	QCMailboxCommand*	pCommand;
	QCMailboxCommand*	pCurrent;
	POSITION			pos;
	POSITION			next;
	POSITION			posChild;
	CTocDoc*			pTocDoc;
	UINT				uRet;
	CString				szMsg;
	BOOL				bFound;
	CString				szOldPathname;
	CString				szNewPathname;
	CString				szFilename;
	TCHAR				szDrive[ _MAX_DRIVE ];
	TCHAR				szDir[ _MAX_DIR ];
	TCHAR				szFname[ _MAX_FNAME ];
	TCHAR				szExt[ _MAX_EXT ];
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
		// Is this a mailbox?
		if( pCommand->GetType() == MBT_REGULAR )
		{
			// Force all message windows to be closed as well
			int OldMSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
			SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, TRUE);
			pTocDoc->OnCloseDocument();
			SetIniShort(IDS_INI_MAILBOX_SUPERCLOSE, ( short ) OldMSC);
		
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
				FiltersUpdateMailbox( pCommand->GetPathname(), InTOC->MBFilename(), CRString(IDS_IN_MBOX_NAME), FALSE );
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

		szOldPathname = pCommand->GetPathname();

		_splitpath( szOldPathname, szDrive, szDir, szFname, szExt );
		
		szNewPathname = szDrive;
		szNewPathname += szDir;		
		
		NewMBXFilename(	(LPCTSTR) pData,
						szOldPathname, 
						szNewPathname,
						szExt,
						szFilename,
						bNoFileChanges );


		if( bNoFileChanges == FALSE )
		{
			// rename the .mbx or .fol
			::FileRenameMT( szOldPathname, szNewPathname + szFilename );	
		}


		if( pCommand->GetType() == MBT_REGULAR )
		{
			// update the toc
			pTocDoc->ChangeName( (LPCTSTR) pData, szNewPathname + szFilename );
		}
		
		// Check to see if any filters get affected
		if( uRet == IDC_UPDATE_FILTER )
		{
			FiltersUpdateMailbox( szOldPathname, szNewPathname + szFilename, (LPCTSTR) pData, pCommand->GetType() == MBT_FOLDER );
		}


		// finally, change the object names, write the desc map file 

		pCommand->SetPathname( szNewPathname + szFilename );
		pCommand->SetName( (LPCTSTR) pData );
		WriteDescMapFile( szNewPathname, *pList );

		if( pCommand->GetType() == MBT_REGULAR )
		{
			QCCommandDirector::NotifyClients( pCommand, CA_SORT_AFTER_RENAME, pData );
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

		QCCommandDirector::NotifyClients( pCommand, CA_SORT_AFTER_RENAME, pData );
		return;
	}

	
	if( theAction == CA_GRAFT )
	{
		// remove it from the old list
		pList->RemoveAt( pos );
		
		if( pParent == NULL )
		{
			WriteDescMapFile( EudoraDir, *pList );
		}
		else
		{
			WriteDescMapFile( pParent->GetPathname(), *pList );
		}

		pParent = ( QCMailboxCommand* ) pData;	
		
		if( pParent == NULL )
		{
			szNewPathname = EudoraDir;
		}
		else
		{
			szNewPathname = pParent->GetPathname() + '\\';
		}

		szOldPathname = pCommand->GetPathname();
		
		NewMBXFilename(	pCommand->GetName(),
						szOldPathname, 
						szNewPathname,
						( LPCTSTR ) CRString( IDS_MAILBOX_EXTENSION ), 
						szFilename,
						bNoFileChanges );

		pCommand->SetPathname( szNewPathname + szFilename );

		::FileRenameMT( szOldPathname, pCommand->GetPathname() );
		
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

		// update the toc
		pTocDoc->ChangeName( pCommand->GetName(), pCommand->GetPathname() );

		// Check to see if any filters get affected
		if( uRet == IDC_UPDATE_FILTER )
		{
			FiltersUpdateMailbox( szOldPathname, pCommand->GetPathname(), pCommand->GetName(), pCommand->GetType() == MBT_FOLDER );
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
// void	QCMailboxDirector::NewMessageCommands(
// COMMAND_ACTION_TYPE	theAction,
// CMenu*				pPopupMenu );
//
// Associates mailboxes and an action to a menu.
//
//		COMMAND_ACTION_TYPE	theAction	-	action to perform
//		CMenu*				pPopupMenu	-	the menu
//
// Return Value:
//		None, but the popup will contain all the appropriate menu entries,
//		and the commands will be added to the stack.
//
void	QCMailboxDirector::NewMessageCommands( 
COMMAND_ACTION_TYPE	theAction,
CMenu*				pPopupMenu,
COMMAND_ACTION_TYPE theNewAction )
{		
	ASSERT( pPopupMenu );
	NewMessageCommandsLevel( m_theMailboxList, theAction, pPopupMenu, theNewAction, 1 );
}


void QCMailboxDirector::NewMessageCommandsLevel( 
CPtrList&			theList,
COMMAND_ACTION_TYPE	theAction,
CMenu*				pPopupMenu,
COMMAND_ACTION_TYPE theNewAction,
LONG				lMenuCount )

{
	WORD				wID = 0;
	POSITION			pos = NULL;
	QCMailboxCommand	*pCommand = NULL;
	CMenu				*pTheSubMenu = NULL;
	INT					iPos = 0;
	LONG				lMaxMenus;

	try
	{
		// I'd like to just do this once, in the constructor, but the mailbox director is
		// global and GetIniLong can't be called before some initialization occurs.

		lMaxMenus = GetIniLong( IDS_INI_MAX_MENUS );

		if( lMaxMenus < 10 )
		{
			lMaxMenus = 10;
		}

		pos = theList.GetHeadPosition();

		while( pos )
		{
			// get the command
			pCommand = ( QCMailboxCommand* ) theList.GetNext( pos );
			
			// IMAP:
			//   We only want to show leaf menus (non-folders menus which have no children) when the
			//   server supports it and the INI says to show it.
			//
			// smanjo, 8/4/98

			if ( (pCommand->GetType() == MBT_FOLDER)
				|| ( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShouldShowAsSubMenu() ) )
#if 0 
				|| ( ((GetIniShort(IDS_INI_IMAP_LEAFMENU)) || ((pCommand->GetChildList().GetCount()) > 0)) && (pCommand->CanHaveChildren()) ) )
#endif 
			{

				if( lMenuCount >= lMaxMenus )
				{
					continue;
				}
				else
				{
					lMenuCount ++;
				}

				// Put a separator before IMAP ACCOUNTS:
				//
				if (pCommand->GetType() == MBT_IMAP_ACCOUNT)
				{
					pPopupMenu->AppendMenu(MF_BYPOSITION | MF_SEPARATOR);
				}


				pTheSubMenu = new CMenu;

				// build the submenu

				pTheSubMenu->CreatePopupMenu();

				NewMessageCommandsLevel( pCommand->GetChildList(), theAction, pTheSubMenu, theNewAction, lMenuCount );

				pPopupMenu->AppendMenu( MF_POPUP, ( UINT ) ( pTheSubMenu->GetSafeHmenu() ), pCommand->GetName() );

				// Tell IMAP command objects that they're shown as sub-menus:
				//
				if ( pCommand->IsImapType() )
				{
					BOOL bIsXferMenu = FALSE;

					if (theNewAction == CA_TRANSFER_TO | theNewAction == CA_TRANSFER_NEW)
						bIsXferMenu = TRUE;

					((QCImapMailboxCommand *)pCommand)->SetShownAsSubMenu (TRUE, bIsXferMenu);
				}
		
				if( theNewAction != CA_NONE )
				{
					wID = g_theCommandStack.AddCommand( pCommand, theNewAction );

					if( wID )
					{
						pTheSubMenu->InsertMenu( 0, MF_BYPOSITION | MF_STRING | MF_DISABLED | MF_GRAYED, wID, CRString( IDS_MAILBOX_NEW ) ); 
					}
				}

// IMAP4 stuff:
				// If this is a selectable IMAP mailbox that can have children, put in the "This Mailbox" item.
				//if (pCommand->IsImapType() && (theAction == CA_OPEN || theAction == CA_TRANSFER_TO || theAction == CA_FILTER_TRANSFER) )
				if ( pCommand->IsImapType() )
				{
					QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *) pCommand;
					if ( !pImapCommand->IsNoSelect() )
					{
						wID = g_theCommandStack.AddCommand( pCommand, theAction );

						if( wID )
						{
							// Insert separator after New....
							pTheSubMenu->InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);

							// The item.
							pTheSubMenu->InsertMenu( 2, MF_BYPOSITION | MF_STRING | MF_DISABLED | MF_GRAYED, wID, CRString ( IDS_IMAP_THIS_MAILBOX) ); 

							if (theAction == CA_TRANSFER_TO)
							{
								pTheSubMenu->SetMenuItemBitmaps( 2, 
													MF_BYPOSITION, 
													&m_theTransferBitmap, 
													NULL );

							}

							// Insert separator between This Mailbox and children; if children exist
							if ((pCommand->GetChildList().GetCount()) > 0)
								pTheSubMenu->InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);
						}
					}
				}
// END IMAP4 stuff

				pTheSubMenu->Detach();
				delete pTheSubMenu;
				pTheSubMenu = NULL;

				if( theAction == CA_OPEN )
				{
					pPopupMenu->SetMenuItemBitmaps( iPos = pPopupMenu->GetMenuItemCount() - 1, 
													MF_BYPOSITION, 
													NULL, 
													&m_theUnreadStatusBitmap);
					
					if( pCommand->GetStatus() == US_YES )
					{
						pPopupMenu->CheckMenuItem( iPos, MF_BYPOSITION | MF_CHECKED );
					}
				}
				else if( ( theAction == CA_TRANSFER_TO ) || ( theAction == CA_FILTER_TRANSFER ) )
				{
					pPopupMenu->SetMenuItemBitmaps( pPopupMenu->GetMenuItemCount() - 1, 
													MF_BYPOSITION, 
													&m_theTransferBitmap, 
													NULL );
				}
			}
			else
			{
				// add it to the command map
				wID = g_theCommandStack.AddCommand( pCommand, theAction );

				ASSERT(wID);
				if (wID)
				{
					if ( ( theAction == CA_OPEN ) &&
						(	( pCommand->GetType() == MBT_IN ) ||
							( pCommand->GetType() == MBT_OUT ) ||
							( pCommand->GetType() == MBT_TRASH ) ) )
					{
						CString szName;
						switch (pCommand->GetType())
						{
						case MBT_IN:	szName = CRString(IDS_IN_MBOX_MENU);	break;
						case MBT_OUT:	szName = CRString(IDS_OUT_MBOX_MENU);	break;
						case MBT_TRASH:	szName = CRString(IDS_TRASH_MBOX_MENU);	break;
						default: ASSERT(0); break;
						}

						// add it to the menu
						pPopupMenu->AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED,
												wID,
												szName );
					}
					else
					{
						// add it to the menu
						pPopupMenu->AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, wID, pCommand->GetName() );
					}

					if (theAction == CA_OPEN)
					{
						pPopupMenu->SetMenuItemBitmaps( iPos = pPopupMenu->GetMenuItemCount() - 1,
														MF_BYPOSITION,
														NULL,
														&m_theUnreadStatusBitmap);

						if( pCommand->GetStatus() == US_YES )
						{
							pPopupMenu->CheckMenuItem( iPos, MF_BYPOSITION | MF_CHECKED );
						}
					}
					else if ((theAction == CA_TRANSFER_TO) || (theAction == CA_FILTER_TRANSFER))
					{
						pPopupMenu->SetMenuItemBitmaps( pPopupMenu->GetMenuItemCount() - 1,
														MF_BYPOSITION,
														&m_theTransferBitmap,
														NULL );
					}
				}
			}
			
		}			
	}
	catch( CMemoryException*	pExp )
	{
		// to do -- add an error message
		// this is a less serious error -- it won't screw up the stack
		pExp->Delete();
		return;
	}
}


QCMailboxCommand*	QCMailboxDirector::FindByPathname(
LPCTSTR szPathname )
{
	CPtrList*			pList;
	CString				szPath;
	CString				szThisPath;
	CString				szNamedPath;
	POSITION			pos;
	QCMailboxCommand*	pCurrent;

	pList = &m_theMailboxList;

	pos = pList->GetHeadPosition();

	while( pos )
	{
		pCurrent = ( QCMailboxCommand* ) pList->GetNext( pos );

		if( pCurrent->GetPathname().CompareNoCase( szPathname ) == 0 )
		{
			return pCurrent;
		}

#ifdef IMAP4
		// Need to modify this to account for the fact that IMAP mailboxes are housed in 
		// directories.
		CString Objpath;
		if (pCurrent->IsImapType())
		{
			((QCImapMailboxCommand *)pCurrent)->GetObjectDirectory (Objpath);
		}
		else
			Objpath = pCurrent->GetPathname ();

		// Can have mailbox names like mbox and mbox0!
		if( (strlen (szPathname) > (size_t) Objpath.GetLength() ) &&
			( strnicmp( Objpath, szPathname, Objpath.GetLength() ) == 0 ) &&
			( szPathname [Objpath.GetLength()] == '\\' ) )
#else
		if( strnicmp( pCurrent->GetPathname(), szPathname, strlen( pCurrent->GetPathname() ) ) == 0 )
#endif
		{
			// found a parent
			pList = &( pCurrent->GetChildList() );
			pos = pList->GetHeadPosition();
		}
	}

	return NULL;
}



BOOL	QCMailboxDirector::DoCompactMailboxes(
const CPtrList&	theList )
{
	POSITION			pos;
	QCMailboxCommand*	pCommand;
	BOOL				bRet;

	bRet = TRUE;
	pos = theList.GetHeadPosition();
	
	while( pos && bRet )
	{
		pCommand = ( QCMailboxCommand* ) theList.GetNext(pos);
		
		// Changed (JOK - 4/13/98) to handle IMAP mailboxes.
		// Handles all cases where a mailbox/folder
		// can have child mailboxes (including IMAP).
		//
		if ( pCommand->CanContainMessages() )
		{
			// Compact only mailboxes (no New... or separators)
			bRet = ( pCommand->CompactMailbox( NULL ) != CRT_DRASTIC_ERROR );
			if( bRet )
			{
				pCommand->NotifyDirector( CA_COMPACT, NULL );
			}
		}

		// Compact children?
		if( pCommand->CanHaveChildren() )
		{
			bRet = DoCompactMailboxes( pCommand->GetChildList() );
		}
	}

    return bRet;
}


BOOL	QCMailboxDirector::CompactMailboxes()
{	
	BOOL	bRet;

	MainProgress(CRString(IDS_COMPACTING));
	Progress(-1, NULL, -1);

	bRet = DoCompactMailboxes( m_theMailboxList );
	
	CloseProgress();

	return bRet;
}


BOOL	QCMailboxDirector::CompactAMailbox(
CTocDoc* pTocDoc )
{
	BOOL				bRet;
	QCMailboxCommand*	pCommand;

	pCommand = FindByPathname( pTocDoc->MBFilename() );

	if( pCommand == NULL )
	{
		return FALSE;
	}

	MainProgress(CRString(IDS_COMPACTING));
	Progress(-1, NULL, -1);

	bRet = ( pCommand->CompactMailbox( pTocDoc ) != CRT_DRASTIC_ERROR );
	
	if( bRet )
	{
		pCommand->NotifyDirector( CA_COMPACT, pTocDoc );
	}

	CloseProgress();

	return bRet;
}


QCMailboxCommand*	QCMailboxDirector::FindParent(
QCMailboxCommand*	pCommand )
{
	TCHAR	szDrive[ _MAX_DRIVE ];
	TCHAR	szDir[ _MAX_DIR ];
	TCHAR	szFname[ _MAX_FNAME ];
	TCHAR	szExt[ _MAX_EXT ];
	CString	szParentPathname;

#ifdef IMAP4
	// If pCommand is and IMAP mailbox command object, defer to
	// "ImapFindParent".
	if (pCommand && pCommand->IsImapType())
	{
		return (QCMailboxCommand *) ImapFindParent ((QCImapMailboxCommand *)pCommand);
	}
#endif

	_splitpath( pCommand->GetPathname(), szDrive, szDir, szFname, szExt );
	
	szParentPathname = szDrive;
	szParentPathname += szDir;
	
	if( !szParentPathname.CompareNoCase( EudoraDir ) )
	{
		return NULL;
	}

	if( szParentPathname.Right(1) == '\\' )
	{
		szParentPathname = szParentPathname.Left( szParentPathname.GetLength() - 1 );
	}

	return FindByPathname( szParentPathname );
				
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
	if( pCommand != NULL )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
	}

	while( 1 )
	{
		CNewMailboxDlg theDialog( pCommand ? pCommand->GetName() : NULL,
								  pCommand ? &( pCommand->GetChildList() ) : NULL,
								  bIsTransfer );
	
		if( theDialog.DoModal() == IDOK )
		{
			//
			// User wants to create a new Mailbox/Folder, so do it.
			//
			pCommand = AddCommand( theDialog.GetName(), theDialog.GetType(), ( QCMailboxCommand* ) pCommand );
			if( NULL == pCommand )
				return NULL;			// Ouch.  Mailbox/Folder creation failure.
		
			switch( pCommand->GetType() )
			{
			case MBT_REGULAR:
				//
				// The dialog UI has the option to "don't transfer,
				// just create mailbox".  So, if we're doing a
				// transfer and the user chooses to just create a
				// mailbox, then return NULL to the caller to indicate
				// that the mailbox is not a Transfer target.
				//
				// Otherwise, return the newly-created mailbox command object
				// to the caller.
				//
				if( bIsTransfer && !theDialog.DoTransfer() )
					return NULL;
				return pCommand;
			case MBT_FOLDER:
				//
				// User created a Folder, but the real point of all this
				// is to create a Mailbox.  Run through the loop again 
				// with the updated 'pCommand' to see if the user wants
				// to create a Mailbox in the newly-created Folder.
				//
				break;

#ifdef IMAP4  // IMAP4
			case MBT_IMAP_MAILBOX:
				//
				// Same as for "MBT_REGULAR" above..
				//
				if( bIsTransfer && !theDialog.DoTransfer() )
					return NULL;
				return pCommand;
			case MBT_IMAP_NAMESPACE:
			case MBT_IMAP_ACCOUNT:
				//
				// Shouldn't get here for these:
				//
				break;
#endif // IMAP4				 

			default:
				ASSERT(0);
				return NULL;
			}
		}
		else
		{
			//
			// User canceled so we're outta here.
			//
			return NULL;
		}
	}

	ASSERT( 0 );		// should never get here
	return NULL;
}


#ifdef IMAP4
// QCImapMailboxDirector.cpp contains IMAP-specific methods that are part of 
// the QCMailboxDirector class.
#include "QCImapMailboxDirector.cpp"
#endif


