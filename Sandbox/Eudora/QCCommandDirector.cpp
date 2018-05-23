// QCCommandDirector.cpp: implementation of the QCCommandDirector class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"
#include "QCCommandDirector.h"
#include "QCCommandObject.h"
#include "QCCommandStack.h"
#include "QICommandClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


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

void QCCommandDirector::Register(
QICommandClient*	pClient )
{
	if( pClient == NULL )
	{
		::AfxThrowUserException();
	}	
	else
	{
		m_theClientList.AddTail( pClient );
	}

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
