// QCPluginDirector.cpp: implementation of the QCPluginDirector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "trnslate.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack	g_theCommandStack;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCPluginDirector::QCPluginDirector()
{

}

QCPluginDirector::~QCPluginDirector()
{
	while( m_thePluginList.IsEmpty() == FALSE )
	{
		delete ( QCPluginCommand* ) m_thePluginList.RemoveTail();
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// BOOL	QCPluginDirector::Build()
//
// Builds the Plugin director.
//
// Return Value:
//		TRUE, on success.  Otherwise FALSE.
//
BOOL QCPluginDirector::Build()
{

	QCPluginCommand*	pCommand;
	CAttachArray*		pAttArray;
	CAttacher*			pAtt;	
	int					size;
	short				i;
	emsMenu				eM;	
	CSpecialArray*		pSpecialArray;
	CSpecial*			pSpec;
	CTranslatorManager* pAPI;
	CTranslator*		pTranslator;
	HICON				hIcon;
	CString				szDesc;
	HICON				icn;

	if (! m_ImageList.LoadEmpty(16, 16, 99))
	{
		ASSERT(0);
		return FALSE;
	}
	m_ImageList.SetBkColor( CLR_NONE );

	pAttArray = ((CEudoraApp *)AfxGetApp())->GetTranslators()->GetAttachers();
	size = pAttArray->GetSize();

	for( i = 0; i < size; i++ ) 
	{
		pAtt = pAttArray->GetAt(i);
		
		if (pAtt)	
		{
			eM =(emsMenu)*pAtt;
			
			pCommand = new QCPluginCommand( this, pAtt, eM.desc, CA_ATTACH_PLUGIN );
			
			if( pCommand == NULL )
			{
				return FALSE;
			}
			else
			{
				if (pCommand->GetIcon(icn))
					pCommand->m_ImageOffset = m_ImageList.Add(icn);
			}

			m_thePluginList.AddTail( pCommand );
		}
	}
	
	pSpecialArray = ((CEudoraApp *)AfxGetApp())->GetTranslators()->GetSpecials();
	
	size = pSpecialArray->GetSize();
	
	for( i = 0; i < size; i++ ) 
	{
		pSpec = pSpecialArray->GetAt(i);
		
		if (pSpec)	
		{
			eM =(emsMenu)*pSpec;
			
			pCommand = new QCPluginCommand( this, pSpec, eM.desc, CA_SPECIAL_PLUGIN );
			
			if( pCommand == NULL )
			{
				return FALSE;
			}
			else
			{
				if (pCommand->GetIcon(icn));
					pCommand->m_ImageOffset = m_ImageList.Add(icn);
			}

			m_thePluginList.AddTail( pCommand );
		}
	}

	
	pAPI = ((CEudoraApp *)AfxGetApp())->GetTranslators();
	
	i = pAPI->FindFirstTranslator( EMST_LANGUAGE, EMSF_ON_REQUEST );
	
	while( i >= 0 ) 
	{
		pTranslator = pAPI->GetNextTranslator( &i, EMST_LANGUAGE, EMSF_ON_REQUEST );

		if( pTranslator == NULL )	
		{
			break;
		}
	
		if ( ( pTranslator->GetType() != EMST_COALESCED ) || 
			 pTranslator->GetIcon( hIcon ) )
		{
			pTranslator->getDescription( szDesc );
	
			pCommand = new QCPluginCommand( this, pTranslator, szDesc, CA_TRANSLATE_PLUGIN );
			
			if( pCommand == NULL )
			{
				return FALSE;
			}
			else
			{
				if (pCommand->GetIcon(icn));
					pCommand->m_ImageOffset = m_ImageList.Add(icn);
			}

			m_thePluginList.AddTail( pCommand );
		}
	}
	
	i = pAPI->FindFirstTranslator( EMST_NO_TYPE, EMSF_Q4_COMPLETION | EMSF_Q4_TRANSMISSION );
	
	while( i >= 0 ) 
	{
		pTranslator = pAPI->GetNextTranslator( &i, EMST_NO_TYPE, EMSF_Q4_COMPLETION | EMSF_Q4_TRANSMISSION );

		if( pTranslator == NULL )	
		{
			break;
		}
	
		if ( ( pTranslator->GetType() != EMST_COALESCED ) || 
			 pTranslator->GetIcon( hIcon ) )
		{
			pTranslator->getDescription( szDesc );
	
			pCommand = new QCPluginCommand( this, pTranslator, szDesc, CA_TRANSMISSION_PLUGIN );
			
			if( pCommand == NULL )
			{
				return FALSE;
			}
			else
			{
				if (pCommand->GetIcon(icn));
					pCommand->m_ImageOffset = m_ImageList.Add(icn);
			}

			m_thePluginList.AddTail( pCommand );
		}
	}

	return TRUE;
}


void	QCPluginDirector::NewMessageCommands( 
COMMAND_ACTION_TYPE	theAction, 
CMenu*				pPopupMenu,
INT					iPosition )
{
	WORD				wID;
	POSITION			pos;
	QCPluginCommand*	pCommand;
	UINT				uFlags;
	
	uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;

	if( ( theAction == CA_TRANSLATE_PLUGIN ) || 
		( theAction == CA_TRANSMISSION_PLUGIN ) ||
		( theAction == CA_ATTACH_PLUGIN ) || 
		( theAction == CA_SPECIAL_PLUGIN ) )
	{
		// need a cleaner way of specifying owner draw stuff
		uFlags |= MF_OWNERDRAW;
	}

	try
	{
		long lastModID = -1;
		pos = m_thePluginList.GetHeadPosition();

		while( pos )
		{
			// get the command
			pCommand = ( QCPluginCommand* ) m_thePluginList.GetNext( pos );
			
			// add it to the command map
			if( pCommand->GetType() == theAction )
			{
				wID = g_theCommandStack.AddCommand( pCommand, theAction );
				
				if( wID )
				{
					long modID = 0, trID = 0;
					((QCPluginCommand *)pCommand)->GetTransmissionID(&modID, &trID);
					
					if( iPosition >= 0 )
					{
						// append it to the end of the menu
						pPopupMenu->InsertMenu( iPosition++,
												uFlags | MF_BYPOSITION, 
												wID, 
												pCommand->GetName() ); 
					}
					else
					{
	
						// Add separators between plugin groups and do this only in the sub-menu case!
						if (lastModID >= 0 && modID != lastModID)
							pPopupMenu->AppendMenu( MF_SEPARATOR ); 

						// append it to the end of the menu
						pPopupMenu->AppendMenu( uFlags, 
												wID, 
												pCommand->GetName() ); 
					}
					lastModID = modID;
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


QCPluginCommand*	QCPluginDirector::Find( 
LPCSTR				szName,
COMMAND_ACTION_TYPE	theType )
{
	QCPluginCommand*	pCommand;
	POSITION			pos;

	pos = m_thePluginList.GetHeadPosition();

	while( pos )
	{
		// get the command
		pCommand = ( QCPluginCommand* ) m_thePluginList.GetNext( pos );
		
		if ( pCommand->GetType() == theType )
		{
			if( pCommand->GetName().CompareNoCase( szName ) == 0 )
			{
				return pCommand;
			}
		}
	}

	return NULL;
}


POSITION QCPluginDirector::GetFirstPluginPosition()
{
	return m_thePluginList.GetHeadPosition();
}

int QCPluginDirector::GetCount()
{
	return m_thePluginList.GetCount( );
}


QCPluginCommand* QCPluginDirector::GetNextPlugin(
POSITION& pos)
{
	return ( QCPluginCommand* ) m_thePluginList.GetNext( pos );
}
