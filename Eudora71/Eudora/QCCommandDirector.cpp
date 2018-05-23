// QCCommandDirector.cpp: implementation of the QCCommandDirector class.
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

#include "resource.h"

#include "QCCommandDirector.h"
#include "QCCommandObject.h"
#include "QCCommandStack.h"
#include "QICommandClient.h"

#include "DebugNewHelpers.h"


extern QCCommandStack	g_theCommandStack;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCCommandDirector::QCCommandDirector()
{

}

QCCommandDirector::~QCCommandDirector()
{

}


///////////////////////////////////////////////////////////////////////////////
//
// void Register( QICommandClient*	pClient );
//
// Registers a client of a QCCommandObject
//
// Input:
//		QICommandClient*	pClient	- the client
//
// Return Value:
//		None.  If there isn't enough memory, an exception is thrown by MFC.  A user
//		exception is thrown if there is an attempt to add NULL to the client list.
//

void QCCommandDirector::Register(QICommandClient* pClient)
{
	if (pClient)
		m_theClientList.AddTail(pClient);
}


///////////////////////////////////////////////////////////////////////////////
//
// void UnRegister( QICommandClient*	pClient );
//
// Unregisters a client of a QCCommandObject
//
// Input:
//		QICommandClient*	pClient	- the client
//
// Return Value:
//		None. 
//

void QCCommandDirector::UnRegister(
QICommandClient*	pClient )
{
	POSITION			pos;
	POSITION			posSave;
	QICommandClient*	pCurrent;

	pos = m_theClientList.GetHeadPosition();
	
	while( pos != NULL )
	{
		posSave = pos;

		pCurrent = ( QICommandClient* ) m_theClientList.GetNext( pos );

		if( pCurrent == pClient )
		{
			// Found it.  Now remove it and bail.
			m_theClientList.RemoveAt( posSave );
			return;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// void NotifyClients( 
// QCCommandObject*		pObject,
// COMMAND_ACTION_TYPE	theAction );
//
// Notifies all registered clients that an action has occurred on a 
// QCCommandObject
//
// Input:
//		QCCommandObject*		pObject		-	the QCCommandObject 
//		COMMAND_ACTION_TYPE		theAction	-	the action
//
// Return Value:
//		None. 
//

void	QCCommandDirector::NotifyClients(
QCCommandObject*	pCommand,
COMMAND_ACTION_TYPE	uAction,
void*				pData )
{
	POSITION			pos;
	QICommandClient*	pClient;

	pos = m_theClientList.GetHeadPosition();

	while( pos != NULL )
	{
		pClient = ( QICommandClient* ) m_theClientList.GetNext( pos );
		pClient->Notify( pCommand, uAction, pData );
	}
}



void QCCommandDirector::RemoveMessageCommands( 
COMMAND_ACTION_TYPE	theAction,
CMenu*				pPopupMenu )
{
	INT		nItems;
	INT		i;
	UINT	uID;

	ASSERT( pPopupMenu );

	if( pPopupMenu == NULL )
	{
		return;
	}

	nItems = pPopupMenu->GetMenuItemCount();	

	for( i = 0; i < nItems; i ++ )
	{
		uID = pPopupMenu->GetMenuItemID( i );

		if( (INT) uID == - 1 )
		{
			RemoveMessageCommands( theAction, pPopupMenu->GetSubMenu( i ) );
		}
		else if ( ( uID >= QC_FIRST_COMMAND_ID ) &&
				  ( uID <= QC_LAST_COMMAND_ID ) )
		{
			g_theCommandStack.DeleteCommand( ( WORD ) uID, theAction );			
		}
	}
}
