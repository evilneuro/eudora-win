// QCCommandStack.cpp: implementation of the QCCommandStack class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"
#include "QCCommandActions.h"
#include "QCCommandStack.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCCommandStack::QCCommandStack()
{
	m_theSelectedCommand.m_wCommandID = 0;
	m_theSelectedCommand.m_pObject = NULL;		
	m_theSelectedCommand.m_theAction = CA_NONE;		
}


QCCommandStack::~QCCommandStack()
{
	QCCommandMap*	pMap;

	while( GetCount() )
	{
		// Somebody didn't clean up
		ASSERT(0);

		pMap = ( QCCommandMap* ) m_theStack.RemoveTail();	
		delete pMap;
	}
}


void	QCCommandStack::NewCommandMap()
#ifdef OLDSTUFF
HMENU	hParent,
INT		iPos )
#endif
{
	QCCommandMap*	pMap;

	pMap = new QCCommandMap();// hParent, iPos );

	if( pMap != NULL )
	{
		m_theStack.AddTail( pMap );
	}
}


void	QCCommandStack::Pop()
{
	QCCommandMap*	pMap;

	if (GetCount() == 0)
		ASSERT(0);
	else
	{
		pMap = ( QCCommandMap* ) m_theStack.RemoveTail();	
		delete pMap;
	}
}

WORD	QCCommandStack::AddCommand( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction )
{
	QCCommandMap*	pMap;

	if( GetCount() == 0 )
	{
		::AfxThrowUserException();
		return 0;
	}
	
	pMap = ( QCCommandMap* ) m_theStack.GetTail();
	return pMap->Add( pObject, theAction );	
}


void	QCCommandStack::DeleteCommand( 
WORD				wID,
COMMAND_ACTION_TYPE	theAction )
{
	QCCommandMap*	pMap;

	if( GetCount() == 0 )
	{
		return;
	}
	
	pMap = ( QCCommandMap* ) m_theStack.GetTail();
	pMap->Delete( wID, theAction );	
}


WORD	QCCommandStack::FindCommandID(
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction )
{
	QCCommandMap*	pMap;

	pMap = ( QCCommandMap* ) m_theStack.GetTail();
	return pMap->FindCommandID( pObject, theAction );	
}


BOOL	QCCommandStack::Lookup( 
WORD					wCommandID,
QCCommandObject**		ppObject,
COMMAND_ACTION_TYPE*	pAction )
{
	QCCommandMap*	pMap;

	if( GetCount() == 0 )
	{
		return FALSE;
	}
	
	pMap = ( QCCommandMap* ) m_theStack.GetTail();
	
	return pMap->Lookup( wCommandID, ppObject, pAction );	
}



void	QCCommandStack::SaveCommand(
WORD	wCommandID )
{
	QCCommandMap*		pMap;
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction; 
	
	m_theSelectedCommand.m_wCommandID = 0;
	
	if( GetCount() == 0 )
	{
		return;
	}
	
	pMap = ( QCCommandMap* ) m_theStack.GetTail();
	
	if( pMap->Lookup( wCommandID, &pObject, &theAction ) )
	{
		m_theSelectedCommand.m_wCommandID = wCommandID;
		m_theSelectedCommand.m_pObject = pObject;		
		m_theSelectedCommand.m_theAction = theAction;		
	}	
	else
	{
		m_theSelectedCommand.m_wCommandID = 0;
		m_theSelectedCommand.m_pObject = NULL;		
		m_theSelectedCommand.m_theAction = CA_NONE;		
	}
}



BOOL	QCCommandStack::GetCommand(	
WORD					uCommandID,
QCCommandObject**		ppObject,
COMMAND_ACTION_TYPE*	pAction)
{
	if( uCommandID == m_theSelectedCommand.m_wCommandID )
	{		
		*ppObject = m_theSelectedCommand.m_pObject;		
		*pAction = m_theSelectedCommand.m_theAction;		
		return TRUE;
	}
	
	return Lookup( uCommandID, ppObject, pAction );
}


QCCommandStack::QCCommandMap::QCCommandMap()
#ifdef OLDSTUFF
HMENU	hParentMenu,
INT		iPopupPos )
#endif
{
//	m_hParentMenu = hParentMenu;
	m_wLastCommand = QC_FIRST_COMMAND_ID - 1;
//	m_hMenu = NULL;
//	m_iPopupPos = iPopupPos;
}

QCCommandStack::QCCommandMap::~QCCommandMap()
{
	POSITION			pos;
	QCCommandMapEntry*	pEntry;
	WORD				wCommand;

	pos = m_theMap.GetStartPosition();
	
	while( pos != NULL )
	{
		m_theMap.GetNextAssoc( pos, wCommand, ( void*& )pEntry );
		delete pEntry;			
	}
}
		

WORD QCCommandStack::QCCommandMap::Add( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction)
{
	QCCommandMapEntry*	pEntry;
	POSITION			pos;
	WORD				wID;

	pos = m_theMap.GetStartPosition();
	
	while( pos != NULL )
	{
		m_theMap.GetNextAssoc( pos, wID, ( void*& ) pEntry );
		
		if( ( pEntry->m_theAction == theAction ) &&
			( pEntry->m_pObject == pObject ) )
		{
			pEntry->m_uCount += 1;
			return wID;
		}
	}

	pEntry = new QCCommandMapEntry;

	if( pEntry == NULL )
	{
		::AfxThrowMemoryException();
		return 0;		
	}

	if( m_theDeletedIDArray.GetUpperBound() == -1 )
	{
		if( m_wLastCommand + 1 == QC_LAST_COMMAND_ID )
		{
			delete pEntry;
			::AfxThrowUserException();
			return 0;
		}
	
		pEntry->m_wCommandID = ++m_wLastCommand;
	}
	else
	{
		pEntry->m_wCommandID = m_theDeletedIDArray[ m_theDeletedIDArray.GetUpperBound() ]; 	
		m_theDeletedIDArray.RemoveAt( m_theDeletedIDArray.GetUpperBound() ); 	
	}
		
	pEntry->m_theAction = theAction;
	pEntry->m_pObject = pObject;
	pEntry->m_uCount = 1;

	m_theMap.SetAt( pEntry->m_wCommandID, pEntry );
	return pEntry->m_wCommandID;
}


void QCCommandStack::QCCommandMap::Delete( 
WORD				wID,
COMMAND_ACTION_TYPE	theAction)
{
	QCCommandMapEntry*	pEntry;

	if( m_theMap.Lookup( wID, ( void*& ) pEntry ) == FALSE )
	{
		return;
	}

	if( pEntry -> m_theAction == theAction )
	{		
		pEntry->m_uCount -= 1;

		if( pEntry->m_uCount == 0 )
		{
			m_theMap.RemoveKey( wID );
			delete pEntry;
			m_theDeletedIDArray.Add( wID );
		}
	}
}


BOOL	QCCommandStack::QCCommandMap::Lookup( 
WORD					wCommandID,
QCCommandObject**		ppObject,
COMMAND_ACTION_TYPE*	pAction )
{
	QCCommandMapEntry*	pEntry;

	if( m_theMap.Lookup( wCommandID, ( void*& ) pEntry ) )
	{
		*ppObject = pEntry->m_pObject;
		*pAction = pEntry->m_theAction;
		return TRUE;
	}
	
	return FALSE;		
}



WORD QCCommandStack::QCCommandMap::FindCommandID( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction)
{
	QCCommandMapEntry*	pEntry;
	POSITION			pos;
	WORD				wID;

	pos = m_theMap.GetStartPosition();
	
	while( pos != NULL )
	{
		m_theMap.GetNextAssoc( pos, wID, ( void*& ) pEntry );
		
		if( ( pEntry->m_theAction == theAction ) &&
			( pEntry->m_pObject == pObject ) )
		{
			return wID;
		}
	}

	return 0;
}


void QCCommandStack::DeleteAllMenuCommands(
CMenu*				pMenu,
COMMAND_ACTION_TYPE	theAction)
{
	UINT uID;
	INT  i;

	if( pMenu == NULL )
	{
		return;
	}

	for( i = pMenu->GetMenuItemCount() - 1;  i >= 0; i -- )
	{
		uID = pMenu->GetMenuItemID( i );

		if( ( INT ) uID == -1 ) 
		{
			DeleteAllMenuCommands( pMenu->GetSubMenu( i ), theAction );
		}
		else
		{
			DeleteCommand( WORD(uID), theAction );		
		}
	}
}


void	QCCommandStack::GetSavedCommand(	
WORD*					pCommandID,
QCCommandObject**		ppObject,
COMMAND_ACTION_TYPE*	pAction
)
{
	*pCommandID = m_theSelectedCommand.m_wCommandID;// = wCommandID;
	*ppObject = m_theSelectedCommand.m_pObject;// = pObject;		
	*pAction = m_theSelectedCommand.m_theAction;// = theAction;		
}

