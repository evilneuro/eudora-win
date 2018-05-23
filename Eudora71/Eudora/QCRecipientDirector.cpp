// QCRecipientDirector.cpp: implementation of the QCRecipientDirector class.
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
#include "fileutil.h"
#include "rs.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "doc.h"
#include "nickdoc.h"

#include "DebugNewHelpers.h"

extern QCCommandStack	g_theCommandStack;
extern CString			EudoraDir;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCRecipientDirector::QCRecipientDirector()
{
	m_bRcpListChanged = false;
}

QCRecipientDirector::~QCRecipientDirector()
{
	while( m_theRecipientList.IsEmpty() == FALSE )
	{
		delete ( QCRecipientCommand* ) m_theRecipientList.RemoveTail();
	}
}


POSITION QCRecipientDirector::Insert(
LPCSTR szText, bool bRefresh /*=false*/)
{
	POSITION			next;
	POSITION			pos;
	QCRecipientCommand*	pCommand;
	INT					i;
			
	pos = m_theRecipientList.GetHeadPosition();
	
	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( next );
		
		i = pCommand->GetName().CompareNoCase( szText );
		
		if( i == 0 )
		{
			return (pos);
		}
		
		if( i > 0 )
		{
			break;
		}
	}

	pCommand = DEBUG_NEW QCRecipientCommand( this, szText );
	
	if( pos )
	{
		if(bRefresh) // refresh the list only when you rebuild, not required when you build for the first time
			NotifyClients( pCommand, CA_NEW );
		return m_theRecipientList.InsertBefore( pos, pCommand );
	}
	
	if(bRefresh) // refresh the list only when you rebuild, not required when you build for the first time
			NotifyClients( pCommand, CA_NEW );
	return  m_theRecipientList.AddTail( pCommand );
}



QCRecipientCommand*	QCRecipientDirector::AddCommand( 
LPCSTR				szName )
{
	POSITION			next;
	POSITION			pos;
	QCRecipientCommand*	pCommand;
	INT					i;
	CString				szFileName;
	JJFile				fOut;

			
	pos = m_theRecipientList.GetHeadPosition();
	
	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( next );
		
		i = pCommand->GetName().CompareNoCase( szName );
		
		if( i == 0 )
		{
			return pCommand;
		}
		
		if( i > 0 )
		{
			break;
		}
	}

	pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCRecipientCommand( this, szName );
	if (!pCommand)
		return NULL;
		// TODO:  ErrorDialog(...

	szFileName = EudoraDir;
	szFileName += CRString(IDS_NICK_RCPT_DBASE_NAME);

	if( FAILED(fOut.Open( szFileName, O_CREAT | O_WRONLY | O_APPEND )) )
	{
		delete pCommand;
		return NULL;
	}

	fOut.PutLine( pCommand->GetName() );

	if( pos )
	{
		m_theRecipientList.InsertBefore( pos, pCommand );
	}
	else
	{	
		m_theRecipientList.AddTail( pCommand );
	}

	NotifyClients( pCommand, CA_NEW );

	return pCommand;
}

	
///////////////////////////////////////////////////////////////////////////////
//
// BOOL	QCRecipientDirector::Build()
//
// Builds the Recipient director.
//
// Return Value:
//		TRUE, on success.  Otherwise FALSE.
//
BOOL QCRecipientDirector::Build()
{
	CString 	Filename;
	JJFile		in;
	char		buf[128];

	Filename = EudoraDir;
	Filename += CRString(IDS_NICK_RCPT_DBASE_NAME);

	if( ::FileExistsMT( Filename ) )
	{
		if (FAILED(in.Open( Filename, O_RDONLY )))
			return FALSE;

		long lNumBytesRead = 0;
		while( SUCCEEDED(in.GetLine( buf, sizeof( buf ), &lNumBytesRead ) ) && ( lNumBytesRead > 0 ) )
		{
				Insert( buf );
		}
	}
    
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// This function is used to rebuild the recipient list if the user discards changes 
// in the address book
// this could happen if the user creates a new nickname and puts it on the recipient
// list. now as soon as it is put on the recipient list, the recipient is written to
// file. however if the user later discards changes the nickname is not written to
// the nickname file as well as nickname is removed from the address book pane, 
// we need to remove the nickname from the recipient list. so we rebuild the recipient list 
// based on valid nicknames that still exist after the address book discards any changes 
// can also happen when the user deletes a nickname that was also on the recipient 
// list we used to delete the nickname from the recipient file also. if the user decides to discard the
// delete, then the nickname is recovered from the nickname file. however Eudora does not remember
// that it was on the recipient list since we have already deleted it from the file. hence when a user
// does a delete, we only delete from memory and keep the nickname in recipient list file. only when the
// user saves the changes, we delete nickname from recipient list file. if user discards the delete
// we use the recipient list file to rebuild the recipient list
//	
//
BOOL QCRecipientDirector::Rebuild()
{

	CString 	Filename;
	JJFile		in;
	char		buf[128];

	Filename = EudoraDir;
	Filename += CRString(IDS_NICK_RCPT_DBASE_NAME);

	bool bWriteFile = false;

	if( ::FileExistsMT( Filename ) )
	{
		if (FAILED(in.Open( Filename, O_RDONLY )))
			return FALSE;

		long lNumBytesRead = 0;
		while( SUCCEEDED(in.GetLine( buf, sizeof( buf ), &lNumBytesRead ) ) && ( lNumBytesRead > 0 ) )
		{
			//before you insert make sure that the name is a valid nickname
			CNickname* pNickName = g_Nicknames->Find( buf );
		
			if( !pNickName ) { 			
				//we don't write to file each time we delete a nickname from rcpt list
				//at the end if we see that there there has been a change we write the
				//changed rcpt list to the file
				DeleteCommand( buf, false );
				bWriteFile = true;
			} else {
				if( !Find(buf) ) {
					Insert(buf, true);
					pNickName->SetRecipient(true);
				}
			}

		}
	}

	in.Close();

	if(bWriteFile)
		Write();

	//at this point we have rebuilt the list from the file and both are consistent with each other
	m_bRcpListChanged = false;
    
	return TRUE;


}


void	QCRecipientDirector::DeleteCommand( 
LPCSTR				szName, bool bWriteToFile /*=true*/ )
{
	POSITION			pos;
	POSITION			next;
	QCRecipientCommand*	pCommand = NULL;
			
	pos = m_theRecipientList.GetHeadPosition();

	for (next = pos; pos; pos = next )
	{
		pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( next );
		
		if (pCommand && pCommand->GetName().CompareNoCase( szName ) == 0 )
			break;
	}

	if (pos == NULL || pCommand == NULL)
	{
		// the recipient wasn't found
		return;
	}

	m_theRecipientList.RemoveAt( pos );
	QCCommandDirector::NotifyClients( pCommand, CA_DELETE );

	delete pCommand;

	// this handles special case. earlier when the user deletes a nickname that was also on the recipient 
	// list we always delete the nickname from the recipient file also. if the user decides to discard the
	// delete, then the nickname is recovered from the nickname file. however Eudora does not remember
	// that it was on the recipient list since we have already deleted it from the file. hence when a user
	// does a delete, we only delete from memory and keep the nickname in recipient list file. only when the
	// user saves the changes, we delete nickname from recipient list file. if user discards the delete
	// we use the recipient list file to recover the recipient list
	if(bWriteToFile) 
		Write();
	else
		m_bRcpListChanged = true; // indicates only rcpt list has changed in memory.and later needs to be written to file on save
}


QCRecipientCommand*	QCRecipientDirector::Find(
LPCSTR	szNickName )
{
	POSITION			pos;
	QCRecipientCommand*	pCommand;
	INT					i;
		
	pos = m_theRecipientList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( pos );

		i = pCommand->GetName().CompareNoCase( szNickName );

		if( i == 0 )
		{
			return pCommand;
		}

		if( i > 0 )
		{
			break;
		}

	}	

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// void	QCRecipientDirector::NewMessageCommands(
// COMMAND_ACTION_TYPE	theAction,
// CMenu*				pPopupMenu );
//
// Associates recipients and an action to a menu.
//
//		COMMAND_ACTION_TYPE	theAction	-	action to perform
//		CMenu*	pPopupMenu				-	the menu
//
// Return Value:
//		None, but the popup will contain all the appropriate menu entries,
//		and the commands will be added to the stack.
//
void QCRecipientDirector::NewMessageCommands(COMMAND_ACTION_TYPE theAction, CMenu* pPopupMenu)
{
	POSITION pos = m_theRecipientList.GetHeadPosition();

	while (pos)
	{
		// get the command
		QCRecipientCommand* pCommand = (QCRecipientCommand*)m_theRecipientList.GetNext(pos);
		
		// add it to the command map
		WORD wID = g_theCommandStack.AddCommand(pCommand, theAction);
		
		// add it to the menu
		if (wID)
			pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, pCommand->GetName()); 
	}			

	// If we didn't add any menu items, then put in an item that says there are no recipients
	if (pPopupMenu->GetMenuItemCount() == 0)
		pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, CRString(IDS_NO_RECIPIENTS));
}


void	QCRecipientDirector::NotifyClients(
QCCommandObject*	pCommand,
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	POSITION			pos;
	POSITION			next;
	QCRecipientCommand*	pCurrent;
	INT					i;
		
	if( ( pCommand != NULL ) && 
		pCommand->IsKindOf( RUNTIME_CLASS( QCRecipientCommand ) ) )
	{
		if ( theAction == CA_DELETE ) 
		{
			DeleteCommand( ( ( QCRecipientCommand* ) pCommand )->GetName() );
			return;
		}
	
		if( theAction == CA_RENAME )
		{
			QCCommandDirector::NotifyClients( pCommand, theAction, pData );
				
			pos = m_theRecipientList.GetHeadPosition();

			for( next = pos; pos; pos = next )
			{
				pCurrent = ( QCRecipientCommand* ) m_theRecipientList.GetNext( next );

				i = pCurrent->GetName().CompareNoCase( ( ( QCRecipientCommand* ) pCommand ) ->GetName() );

				if( i == 0 )
				{
					break;
				}

				if( i > 0 )
				{
					// this should never happen 
					pos = NULL;
					break;
				}
			}	

			if( pos )
			{
				// remove the old pointer
				m_theRecipientList.RemoveAt( pos );			
			}

			pos = m_theRecipientList.GetHeadPosition();

			for( next = pos; pos; pos = next )
			{
				pCurrent = ( QCRecipientCommand* ) m_theRecipientList.GetNext( next );

				i = pCurrent->GetName().CompareNoCase( ( LPCSTR ) pData );

				if( i == 0 )
				{
					// this should never happen
					return;
				}

				if( i > 0 )
				{
					break;
				}
			}	

			if( pos )
			{
				m_theRecipientList.InsertBefore( pos, pCommand );
			}
			else
			{
				m_theRecipientList.AddTail( pCommand );
			}

			return;
		}
	}

	QCCommandDirector::NotifyClients( pCommand, theAction, pData );
}


BOOL QCRecipientDirector::Write()
{
	CString				szFileName;
	JJFile				fOut;
	QCRecipientCommand*	pCommand;
	POSITION			pos;

	szFileName = EudoraDir;
	szFileName += CRString(IDS_NICK_RCPT_DBASE_NAME);

	if (FAILED(fOut.Open( szFileName, O_CREAT | O_WRONLY | O_TRUNC )))
		return FALSE;

	pos = m_theRecipientList.GetHeadPosition();
	
	while ( pos )
	{
		pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( pos );
		fOut.PutLine( pCommand->GetName() );
	}

	m_bRcpListChanged = false;
	return TRUE;
}


POSITION QCRecipientDirector::GetFirstRecipientPosition()
{
	return m_theRecipientList.GetHeadPosition();
}


QCRecipientCommand* QCRecipientDirector::GetNextRecipient(
POSITION& pos)
{
		return ( QCRecipientCommand* ) m_theRecipientList.GetNext( pos );
}

