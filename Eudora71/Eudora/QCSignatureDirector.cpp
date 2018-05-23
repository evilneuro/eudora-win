// QCSignatureDirector.cpp: implementation of the QCSignatureDirector class.
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

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"

#include "rs.h"
#include "utils.h"
#include "fileutil.h"

#include "QCSharewareManager.h"


#include "DebugNewHelpers.h"

extern CString			EudoraDir;
extern QCCommandStack	g_theCommandStack;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCSignatureDirector::QCSignatureDirector()
{

}

QCSignatureDirector::~QCSignatureDirector()
{
	while( m_theSignatureList.IsEmpty() == FALSE )
	{
		delete ( QCSignatureCommand* ) m_theSignatureList.RemoveTail();
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// BOOL	QCSignatureDirector::Build()
//
// Builds the Signature director.
//
// Return Value:
//		TRUE, on success.  Otherwise FALSE.
//
BOOL QCSignatureDirector::Build()
{
	CString				szFind;
	WIN32_FIND_DATA		fd;
	HANDLE				findHandle;
	CString				szPath;
	CString				szSignatureDir;
	CString				szItemName;
	LPCSTR				szExt;
	CRString			szTextExtension( IDS_SIGNATURE_EXTENSION );
	CRString			szHtmExtension( IDS_HTM_EXTENSION );
	CRString			szHtmlExtension( IDS_HTML_EXTENSION );
	CString				szSignatureExtension;
	QCSignatureCommand*	pSignatureCommand;
						
	// build the folder name
	szSignatureDir = EudoraDir + CRString( IDS_SIGNATURE_FOLDER ); 

	// see if it exists
	if ( _access( szSignatureDir, 0 ) )
	{
		// nope -- create it
		if ( mkdir( szSignatureDir ) )
		{
			return TRUE;
		}
	}

	// Move signatur.alt file
	MigrateSignatures();
   
	// Collect name of all files and add to list
	if (UsingFullFeatureSet())
	{
		szPath = szSignatureDir + "\\";
		
		for (int i = 0; i < 3; ++i)
		{
			switch( i )
			{
			case 0 :
				szSignatureExtension = szTextExtension;
				break;
			case 1 :
				szSignatureExtension = szHtmExtension;
				break;
			case 2 :
				szSignatureExtension = szHtmlExtension;
				break;
			}

			szFind = szPath + "*." + szSignatureExtension;
			findHandle = FindFirstFile( szFind, &fd ); 

			while( findHandle != HANDLE( INVALID_HANDLE_VALUE ) )
			{
				try 
				{
					// get just the file name

					szItemName = fd.cFileName;
					szExt = strrchr( fd.cFileName, '.' );
			
					if( szExt != NULL )
					{
						if( stricmp( szExt + 1, szSignatureExtension ) == 0 )
						{
							szItemName = szItemName.Left( szItemName.GetLength() - strlen( szExt ) );
						}
					}

					// create the new Signature command object
					pSignatureCommand = DEBUG_NEW QCSignatureCommand( this, szItemName, szPath + fd.cFileName );
					
					// add it to the list
					Insert( pSignatureCommand );
				}
				catch( CMemoryException*	pExp )
				{			
					// to do -- add error message
					pExp->Delete();
					return FALSE;
				}
				catch( CException*	pExp )
				{
					pExp->Delete();
					return FALSE;
				}

				if ( !FindNextFile( findHandle, &fd ) )  
				{
					break; 
				}
			}
			
			if( findHandle != HANDLE( INVALID_HANDLE_VALUE ) )
			{
				FindClose( findHandle );
			}			
		}
	}
	else
	{
		// In Light mode only two signatures are allowed (Standard and Alternate)
		AddCommand( CRString(IDS_STANDARD_SIGNATURE));
		AddCommand( CRString(IDS_ALTERNATE_SIGNATURE32));
	}

	return TRUE;
}


void QCSignatureDirector::MigrateSignatures()
{
	CString		szSrcFileName;
	CString		szDestFileName;
	BOOL		bCopied;
	CRString	szExt(IDS_SIGNATURE_EXTENSION);
	JJFile		fileNew;
	CString		szSignatureDir;

	bCopied = FALSE;

	if ( GetIniShort( IDS_INI_USER_SIGNATURES ) == 0 )
	{
		// check for signatur.pce
		szSrcFileName = EudoraDir + CRString( IDS_SIG_FILENAME_FIRST );

		// build the standard signature filename
		szSignatureDir = EudoraDir + CRString( IDS_SIGNATURE_FOLDER ); 
		szDestFileName = szSignatureDir + "\\" + CRString( IDS_STANDARD_SIGNATURE );
		szDestFileName += "." + szExt;

		if ( ::FileExistsMT( szSrcFileName ) )
		{
			// copy the default file
			QCCopyFile( szSrcFileName, szDestFileName, TRUE );
		}
		else if ( !::FileExistsMT( szDestFileName ) )
		{
			// create a blank standard signature
			fileNew.Open( szDestFileName, O_CREAT );
			fileNew.Close();
		}

		// check for signatur.alt
		szSrcFileName = EudoraDir + CRString( IDS_SIG_FILENAME_FIRST + 1 );
		if ( ::FileExistsMT( szSrcFileName ) )
		{
			// copy the default file
			szDestFileName = szSignatureDir + "\\" + CRString( IDS_ALTERNATE_SIGNATURE32 );			
			szDestFileName += "." + szExt;
			QCCopyFile( szSrcFileName, szDestFileName, TRUE );
		}

		SetIniShort( IDS_INI_USER_SIGNATURES, 1);
	}
}


POSITION QCSignatureDirector::Insert(QCSignatureCommand* pNewCommand)
{
	POSITION			next;
	POSITION			pos;
	INT					i;
	QCSignatureCommand*	pCommand;
			
	pos = m_theSignatureList.GetHeadPosition();
	
	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( next );
		
		i = stricmp( pCommand->GetName(),  pNewCommand->GetName() );
		
		if( i == 0 )
		{
			return (pos);
		}
		
		if( i > 0 )
		{
			break;
		}
	}

	if( pos )
	{
		return m_theSignatureList.InsertBefore( pos, pNewCommand );
	}
	
	return  m_theSignatureList.AddTail( pNewCommand );
}

///////////////////////////////////////////////////////////////////////////////
//
// void	QCSignatureDirector::NewMessageCommands(
// COMMAND_ACTION_TYPE	theAction,
// CMenu*				pPopupMenu );
//
// Associates signatures and an action to a menu.
//
//		COMMAND_ACTION_TYPE	theAction	-	action to perform
//		CMenu*				pPopupMenu	-	the menu
//
// Return Value:
//		None, but the popup will contain all the appropriate menu entries,
//		and the commands will be added to the stack.
//
void QCSignatureDirector::NewMessageCommands(COMMAND_ACTION_TYPE theAction, CMenu* pPopupMenu)
{
	WORD					wID;
	POSITION				pos;
	QCSignatureCommand*	pCommand;

	if(	theAction != CA_OPEN ) 
	{
		return;
	}
		
	try
	{
		pos = m_theSignatureList.GetHeadPosition();

		while( pos )
		{
			// get the command
			pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( pos );
			
			// add it to the command map
			wID = g_theCommandStack.AddCommand( pCommand, theAction );
			
			if( wID )
			{
				// add it to the menu
				pPopupMenu->AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, wID, pCommand->GetName() ); 
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


void QCSignatureDirector::FillComboBox(CComboBox* pCB)
{
	if (!UsingFullFeatureSet())
	{
		pCB->AddString(CRString(IDS_STANDARD_SIGNATURE));
		pCB->AddString(CRString(IDS_ALTERNATE_SIGNATURE32));
	}
	else
	{
		POSITION				pos;
		QCSignatureCommand*	pCommand;

		pos = m_theSignatureList.GetHeadPosition();

		while( pos )
		{
			pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( pos );
			pCB->AddString( pCommand->GetName() );
		}
	}
}


void QCSignatureDirector::FillListBox(CListBox* pLB)
{
	if (!UsingFullFeatureSet())
	{
		pLB->AddString(CRString(IDS_STANDARD_SIGNATURE));
		pLB->AddString(CRString(IDS_ALTERNATE_SIGNATURE32));
	}
	else
	{
		POSITION				pos;
		QCSignatureCommand*	pCommand;

		pos = m_theSignatureList.GetHeadPosition();

		while( pos )
		{
			pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( pos );
			pLB->AddString( pCommand->GetName() );
		}
	}
}



QCSignatureCommand*	QCSignatureDirector::Find(LPCSTR szName)
{
	POSITION				pos;
	QCSignatureCommand*	pCommand;

	pos = m_theSignatureList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( pos );
		
		if( stricmp( pCommand->GetName(), szName ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}


POSITION QCSignatureDirector::GetFirstSignaturePosition()
{
	return m_theSignatureList.GetHeadPosition();
}


QCSignatureCommand* QCSignatureDirector::GetNext(POSITION& pos)
{
	return ( QCSignatureCommand* ) m_theSignatureList.GetNext( pos );
}



QCSignatureCommand*	QCSignatureDirector::AddCommand(LPCSTR szName)
{
	POSITION			next;
	POSITION			pos;
	INT					i;
	QCSignatureCommand*	pCommand = NULL;
	CString				szPathname;
	JJFile				theFile;
					
	pos = m_theSignatureList.GetHeadPosition();
	
	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( next );

		if (pCommand)
		{
			i = stricmp(pCommand->GetName(), szName);
			
			if (i == 0)
				return pCommand;
		
			if( i > 0 )
				break;
		}
	}

	szPathname = EudoraDir + CRString( IDS_SIGNATURE_FOLDER ) + "\\";
	szPathname += szName;
	szPathname += "." + CRString( IDS_SIGNATURE_EXTENSION );
	
	// create an empty signature file
	if (FAILED(theFile.Open( szPathname, O_CREAT )))
		return NULL;

	theFile.Close();

	try
	{
		// create the new Signature command object
		pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCSignatureCommand(this, szName, szPathname);

		if (pCommand)
		{
			if (pos)
				m_theSignatureList.InsertBefore(pos, pCommand);
			else
				m_theSignatureList.AddTail(pCommand);
	
			NotifyClients(pCommand, CA_NEW);
		}
	}
	catch (CMemoryException* pExp)
	{
		// to do -- error message here
		pExp->Delete();
		
		delete pCommand;
		pCommand = NULL;
	}

	return pCommand;
}


void QCSignatureDirector::NotifyClients(QCCommandObject* pCommand, COMMAND_ACTION_TYPE theAction, void* pData)
{
	POSITION			pos;
	POSITION			next;
	QCSignatureCommand*	pCurrent;
			
	if( ( pCommand != NULL ) && 
		pCommand->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) )
	{
		if ( theAction == CA_DELETE ) 
		{
			pos = m_theSignatureList.GetHeadPosition();

			for( next = pos; pos; pos = next )
			{
				pCurrent = ( QCSignatureCommand* ) m_theSignatureList.GetNext( next );
				
				if ( stricmp( ( ( QCSignatureCommand* ) pCommand )->GetName(), pCurrent->GetName() ) == 0 )
				{
					break;
				}
			}

			if( pos != NULL )
			{			
				// remove it from the list
				m_theSignatureList.RemoveAt( pos );
			}
		}	
	}

	QCCommandDirector::NotifyClients( pCommand, theAction, pData );
}


QCSignatureCommand*	QCSignatureDirector::FindByPathname(LPCSTR szPathname)
{
	POSITION				pos;
	QCSignatureCommand*	pCommand;

	pos = m_theSignatureList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCSignatureCommand* ) m_theSignatureList.GetNext( pos );
		
		if( stricmp( pCommand->GetPathname(), szPathname ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}


