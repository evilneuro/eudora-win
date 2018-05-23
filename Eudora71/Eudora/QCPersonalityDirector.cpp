// QCPersonalityDirector.cpp: implementation of the QCPersonalityDirector class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"
#include "eudora.h"

#include "rs.h"
#include "persona.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPersonalityCommand.h"
#include "QCPersonalityDirector.h"


#include "DebugNewHelpers.h"

extern QCCommandStack	g_theCommandStack;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCPersonalityDirector::QCPersonalityDirector()
{

}

QCPersonalityDirector::~QCPersonalityDirector()
{
	Cleanup();
}


void QCPersonalityDirector::Cleanup()
{
	while( m_thePersonalityList.IsEmpty() == FALSE )
	{
		delete ( QCPersonalityCommand* ) m_thePersonalityList.RemoveTail();
	}
}


QCPersonalityCommand*	QCPersonalityDirector::AddCommand( 
LPCSTR				szName )
{
	QCPersonalityCommand*	pCommand = NULL;
	POSITION pos = m_thePersonalityList.GetHeadPosition();
	
	for( POSITION next = pos; pos; pos = next )
	{
		pCommand = ( QCPersonalityCommand* ) m_thePersonalityList.GetNext( next );
		
		//
		// Theoretically, we should never get a duplicate name.
		//
		ASSERT(pCommand->GetName().CompareNoCase( szName ) != 0 );

		//
		// Note that we do a case-sensitive sort here since That's
		// The Way (uh, huh, uh, huh) the CPersona manager works.
		//
		INT i = pCommand->GetName().Compare( szName );
		
		if( i == 0 )
		{
			ASSERT(0);			// should never get an (exact) duplicate
			return pCommand;
		}
		
		if( i > 0 )
		{
			break;
		}
	}

	pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCPersonalityCommand( this, szName );
	if (!pCommand)
		return NULL;
		// TODO:  ErrorDialog(...

	if( pos )
	{
		m_thePersonalityList.InsertBefore( pos, pCommand );
	}
	else
	{	
		m_thePersonalityList.AddTail( pCommand );
	}

	NotifyClients( pCommand, CA_NEW );

	return pCommand;
}


///////////////////////////////////////////////////////////////////////////////
//
// BOOL	QCPersonalityDirector::Build()
//
// Builds the Personality director.
//
// Return Value:
//		TRUE, on success.  Otherwise FALSE.
//
BOOL QCPersonalityDirector::Build()
{
	ASSERT(m_thePersonalityList.IsEmpty());
	Cleanup();		// for good hygiene

	//
	// Enumerate the list of personalities into a null-terminated list
	// of null-terminated strings, then add each item to the internal
	// command list.  The CPersonality manager determines the order of
	// the command items.
	//
	for (LPSTR pszNameList = g_Personalities.List();
		 pszNameList && *pszNameList;
		 pszNameList += strlen(pszNameList) + 1)
	{
		QCPersonalityCommand* pCommand = DEBUG_NEW QCPersonalityCommand( this, pszNameList );
		m_thePersonalityList.AddTail( pCommand );
	}

	return TRUE;
}


void	QCPersonalityDirector::DeleteCommand( 
LPCSTR				szName )
{
	QCPersonalityCommand* pCommand = NULL;
	POSITION pos = m_thePersonalityList.GetHeadPosition();

	for( POSITION next = pos; pos; pos = next )
	{
		pCommand = ( QCPersonalityCommand* ) m_thePersonalityList.GetNext( next );
		
		if ( pCommand->GetName().CompareNoCase( szName ) == 0 )
		{
			break;
		}
	}

	if( pos == NULL )
	{
		// the persona wasn't found
		return;
	}
	
	NotifyClients( pCommand, CA_DELETE );

	m_thePersonalityList.RemoveAt( pos );
	delete pCommand;
}


QCPersonalityCommand*	QCPersonalityDirector::Find(
LPCSTR	szName )
{
	POSITION				pos;
	QCPersonalityCommand*	pCommand;
	INT						i;
		
	pos = m_thePersonalityList.GetHeadPosition();

	while( pos )
	{
		pCommand = ( QCPersonalityCommand* ) m_thePersonalityList.GetNext( pos );

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

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// void	QCPersonalityDirector::NewMessageCommands(
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
void	QCPersonalityDirector::NewMessageCommands( 
COMMAND_ACTION_TYPE	theAction,
CMenu*				pPopupMenu )
{
	try
	{
		POSITION pos = m_thePersonalityList.GetHeadPosition();

		while( pos )
		{
			// get the command
			QCPersonalityCommand* pCommand = ( QCPersonalityCommand* ) m_thePersonalityList.GetNext( pos );
			
			// add it to the command map
			WORD wID = g_theCommandStack.AddCommand( pCommand, theAction );
			
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


POSITION QCPersonalityDirector::GetFirstPersonalityPosition()
{
	return m_thePersonalityList.GetHeadPosition();
}


QCPersonalityCommand* QCPersonalityDirector::GetNextPersonality(
POSITION& pos)
{
		return ( QCPersonalityCommand* ) m_thePersonalityList.GetNext( pos );
}
