// QCStationeryDirector.cpp: implementation of the QCStationeryDirector class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"
#include "eudora.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"

#include "fileutil.h"

#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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
			pStationeryCommand = new QCStationeryCommand( this, szItemName, szPath + fd.cFileName );
			
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
void	QCStationeryDirector::NewMessageCommands( 
COMMAND_ACTION_TYPE	theAction,
CMenu*				pPopupMenu )
{
	WORD					wID;
	POSITION				pos;
	QCStationeryCommand*	pCommand;

	try
	{
		pos = m_theStationeryList.GetHeadPosition();

		while( pos )
		{
			// get the command
			pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( pos );
			
			// add it to the command map
			wID = g_theCommandStack.AddCommand( pCommand, theAction );
			
			if( wID )
			{
				// add it to the menu
				CString strName = pCommand->GetName();
				char szNewName[255];

				//Bug 2760.  If '&' is used in a name it is underlined in the menu..this call fixes this
				//Will insert another '&' so it will appear correctly eg. if strOldName is "te&st" szNewName will be "te&&st"
				GenerateNewName(strName, szNewName);

				pPopupMenu->AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, wID, szNewName ); 
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
	QCStationeryCommand* pCommand;
	CString				szPathname;
	JJFile				theFile;
					
	pos = m_theStationeryList.GetHeadPosition();
	
	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCStationeryCommand* ) m_theStationeryList.GetNext( next );
		
		i = stricmp( pCommand->GetName(),  szName );
		
		if( i == 0 )
		{
			return pCommand;
		}
	
		if( i > 0 )
		{
			break;
		}
	}

	szPathname = EudoraDir + CRString( IDS_STATIONERY_FOLDER ) + "\\";
	szPathname += szName;
	szPathname += "." + CRString( IDS_STATIONERY_EXTENSION );
	
	try
	{
		// create the new Stationery command object
		pCommand = new QCStationeryCommand( this, szName, szPathname );
		
		if( pos )
		{
			m_theStationeryList.InsertBefore( pos, pCommand );
		}
		else
		{
			m_theStationeryList.AddTail( pCommand );
		}
	
		NotifyClients( pCommand, CA_NEW, (void *)szName );
	}
	catch( CMemoryException*	pExp )
	{
		// to do -- error message here
		pExp->Delete();
		
		if( pCommand )
		{
			delete pCommand;
			pCommand = NULL;
		}
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



