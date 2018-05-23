// QCRecipientDirector.cpp: implementation of the QCRecipientDirector class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"
#include "eudora.h"

#include "fileutil.h"
#include "rs.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack	g_theCommandStack;
extern CString			EudoraDir;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCRecipientDirector::QCRecipientDirector()
{

}

QCRecipientDirector::~QCRecipientDirector()
{
	while( m_theRecipientList.IsEmpty() == FALSE )
	{
		delete ( QCRecipientCommand* ) m_theRecipientList.RemoveTail();
	}
}


POSITION QCRecipientDirector::Insert(
LPCSTR szText)
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

	pCommand = new QCRecipientCommand( this, szText );
	
	if( pos )
	{
		return m_theRecipientList.InsertBefore( pos, pCommand );
	}
	
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

	try
	{
		pCommand = new QCRecipientCommand( this, szName );
	}
	catch( CMemoryException*	pExp )
	{
		// TODO:  ErrorDialog(...
		pExp->Delete();
		return NULL;
	}

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


void	QCRecipientDirector::DeleteCommand( 
LPCSTR				szName )
{
	POSITION			pos;
	POSITION			next;
	QCRecipientCommand*	pCommand;

			
	pos = m_theRecipientList.GetHeadPosition();

	for( next = pos; pos; pos = next )
	{
		pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( next );
		
		if ( pCommand->GetName().CompareNoCase( szName ) == 0 )
		{
			break;
		}
	}

	if( pos == NULL )
	{
		// the recipient wasn't found
		return;
	}
	
	QCCommandDirector::NotifyClients( pCommand, CA_DELETE );

	m_theRecipientList.RemoveAt( pos );
	delete pCommand;
	Write();
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
void	QCRecipientDirector::NewMessageCommands( 
COMMAND_ACTION_TYPE	theAction,
CMenu*				pPopupMenu )
{
	WORD				wID;
	POSITION			pos;
	QCRecipientCommand*	pCommand;
		
	try
	{
		pos = m_theRecipientList.GetHeadPosition();

		while( pos )
		{
			// get the command
			pCommand = ( QCRecipientCommand* ) m_theRecipientList.GetNext( pos );
			
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
