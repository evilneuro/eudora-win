// QCStationeryDirector.cpp: implementation of the QCStationeryDirector class.
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "eudora.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"

#include "fileutil.h"

#include "rs.h"

#include "DebugNewHelpers.h"

extern CString			EudoraDir;
extern QCCommandStack	g_theCommandStack;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//
//
//////////////////////////////////////////////////////////////////////

QCStationeryDirector::QCStationeryDirector()
{
}


QCStationeryDirector::~QCStationeryDirector()
{
	while( m_theStationeryList.IsEmpty() == FALSE )
	{
		delete ( QCStationeryCommand* ) m_theStationeryList.RemoveTail();
	}
}


POSITION QCStationeryDirector::Insert(
QCStationeryCommand*	pNewCommand )
{
	POSITION			next;
	POSITION			pos;
	INT					i;
	QCStationeryCommand*	pCommand;
			
	pos = m_theStationeryList.GetHeadPosition();
	
	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( next );
		
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
		return m_theStationeryList.InsertBefore( pos, pNewCommand );
	}
	
	return  m_theStationeryList.AddTail( pNewCommand );
}


///////////////////////////////////////////////////////////////////////////////
//
// BOOL	QCStationeryDirector::Build()
//
// Builds the stationery director.
//
// Return Value:
//		TRUE, on success.  Otherwise FALSE.
//
BOOL QCStationeryDirector::Build()
{
	WIN32_FIND_DATA			fd;
	HANDLE					findHandle;
	CString					szFind;
	LPCSTR					szExt;
	CString					szItemName;
	CRString				szStationeryExtension( IDS_STATIONERY_EXTENSION );
	QCStationeryCommand*	pStationeryCommand;
	CString					szPath;

	// build the find first/next string
	szPath = EudoraDir + CRString( IDS_STATIONERY_FOLDER ); 

	// make sure the directory exists
	_mkdir( szPath );

	szPath += "\\";
	szFind = szPath + "*." + CRString( IDS_STATIONERY_EXTENSION );

	
	findHandle = FindFirstFile( szFind, &fd ); 

	// find all the stationery and add it
	while( findHandle != HANDLE( INVALID_HANDLE_VALUE ) )
	{
		try 
		{
			// get just the file name

			szItemName = fd.cFileName;
			szExt = strrchr( fd.cFileName, '.' );
	
			if( szExt != NULL )
			{
				if( stricmp( szExt + 1, szStationeryExtension ) == 0 )
				{
					szItemName = szItemName.Left( szItemName.GetLength() - strlen( szExt ) );
				}
			}

			// create the new stationery command object
			pStationeryCommand = DEBUG_NEW QCStationeryCommand( this, szItemName, szPath + fd.cFileName );
			
			// add it to the list
			Insert( pStationeryCommand );
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

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//
// void	QCStationeryDirector::NewMessageCommands(
// COMMAND_ACTION_TYPE	theAction,
// CMenu*				pPopupMenu );
//
// Associates stationery and an action to a menu.
//
//		COMMAND_ACTION_TYPE	theAction	-	action to perform
//		CMenu*				pPopupMenu	-	the menu
//
// Return Value:
//		None, but the popup will contain all the appropriate menu entries,
//		and the commands will be added to the stack.
//
void QCStationeryDirector::NewMessageCommands(COMMAND_ACTION_TYPE theAction, CMenu* pPopupMenu)
{
	POSITION pos = m_theStationeryList.GetHeadPosition();

	while (pos)
	{
		// get the command
		QCStationeryCommand* pCommand = (QCStationeryCommand*)m_theStationeryList.GetNext(pos);
		
		// add it to the command map
		WORD wID = g_theCommandStack.AddCommand(pCommand, theAction);

		if (wID)
		{
			// add it to the menu
			CString strName = pCommand->GetName();
			char szNewName[255];

			//Bug 2760.  If '&' is used in a name it is underlined in the menu..this call fixes this
			//Will insert another '&' so it will appear correctly eg. if strOldName is "te&st" szNewName will be "te&&st"
			GenerateNewName(strName, szNewName);

			pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, szNewName); 
		}
	}			

	// If we didn't add any menu items, then put in an item that says there are no recipients
	if (pPopupMenu->GetMenuItemCount() == 0)
		pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, CRString(IDS_NO_STATIONERY));
}



QCStationeryCommand*	QCStationeryDirector::Find(
LPCSTR	szName )
{
	POSITION				pos;
	QCStationeryCommand*	pCommand;

	pos = m_theStationeryList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( pos );
		
		if( stricmp( pCommand->GetName(), szName ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}


QCStationeryCommand*	QCStationeryDirector::FindByPathname(
LPCSTR	szPathname )
{
	POSITION				pos;
	QCStationeryCommand*	pCommand;

	pos = m_theStationeryList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( pos );
		
		if( stricmp( pCommand->GetPathname(), szPathname ) == 0 )
		{
			return pCommand;
		}
	}

	return NULL;
}


void QCStationeryDirector::FillComboBox(
CComboBox*	pCB )
{
	POSITION				pos;
	QCStationeryCommand*	pCommand;

	pos = m_theStationeryList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( pos );
		pCB->AddString( pCommand->GetName() );
	}
}


void QCStationeryDirector::FillListBox(
CListBox*	pLB )
{
	POSITION				pos;
	QCStationeryCommand*	pCommand;

	pos = m_theStationeryList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( pos );
		pLB->AddString( pCommand->GetName() );
	}
}


POSITION QCStationeryDirector::GetFirstStationeryPosition()
{
	return m_theStationeryList.GetHeadPosition();
}


QCStationeryCommand* QCStationeryDirector::GetNext(
POSITION& pos )
{
	return ( QCStationeryCommand* ) m_theStationeryList.GetNext( pos );
}


QCStationeryCommand* QCStationeryDirector::AddCommand(
LPCSTR	szName )
{
	POSITION			next;
	POSITION			pos;
	INT					i;
	QCStationeryCommand* pCommand = NULL;
	CString				szPathname;
	JJFile				theFile;
					
	pos = m_theStationeryList.GetHeadPosition();
	
	for (next = pos; pos; pos = next)
	{
		pCommand = (QCStationeryCommand*)m_theStationeryList.GetNext(next);

		if (pCommand)
		{
			i = stricmp( pCommand->GetName(),  szName );
			
			if (i == 0)
				return pCommand;
		
			if (i > 0)
				break;
		}
	}

	szPathname = EudoraDir + CRString( IDS_STATIONERY_FOLDER ) + "\\";
	szPathname += szName;
	szPathname += "." + CRString( IDS_STATIONERY_EXTENSION );
	
	try
	{
		// create the new Stationery command object
		pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCStationeryCommand(this, szName, szPathname);
		
		if (pCommand)
		{
			if (pos)
				m_theStationeryList.InsertBefore(pos, pCommand);
			else
				m_theStationeryList.AddTail(pCommand);
		
			NotifyClients(pCommand, CA_NEW, (void*)szName);
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


void	QCStationeryDirector::NotifyClients(
QCCommandObject*	pCommand,
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	POSITION			pos;
	POSITION			next;
	QCStationeryCommand* pCurrent;
			
	if( ( pCommand != NULL ) && 
		pCommand->IsKindOf( RUNTIME_CLASS( QCStationeryCommand ) ) )
	{
		if ( theAction == CA_DELETE ) 
		{
			pos = m_theStationeryList.GetHeadPosition();

			for( next = pos; pos; pos = next )
			{
				pCurrent = ( QCStationeryCommand* ) m_theStationeryList.GetNext( next );
				
				if ( stricmp( ( ( QCStationeryCommand* ) pCommand )->GetName(), pCurrent->GetName() ) == 0 )
				{
					break;
				}
			}

			if( pos != NULL )
			{			
				// remove it from the list
				m_theStationeryList.RemoveAt( pos );
			}
		}	
	}

	QCCommandDirector::NotifyClients( pCommand, theAction, pData );
}

//Bug 2760.  If '&' is used in a name it is underlined in the menu..this call fixes this
//Will insert another '&' so it will appear correctly eg. if strOldName is "te&st" szNewName will be "te&&st"
void QCStationeryDirector::GenerateNewName(CString strOldName, char *szNewName)
{
	INT nLen = strOldName.GetLength();
	LPTSTR pszName = strOldName.GetBuffer(nLen);
	int nFirstPos;

	if(nFirstPos = strOldName.Find("&") >= 0)
	{
		int i, j;
		
		_mbsnbcpy((unsigned char*)szNewName, (unsigned char*) pszName, nFirstPos + 1);
		
		for(i=j= (nFirstPos + 1); i < nLen; i++,j++)
		{
			if(pszName[i] == '&')
			{
				szNewName[j] = '&';
				j++;
				szNewName[j] = '&';
			}
			else
				szNewName[j] = pszName[i];
		}
		szNewName[j] = '\0';
		
	}
	else
		strcpy(szNewName, pszName);

	strOldName.ReleaseBuffer();
}



