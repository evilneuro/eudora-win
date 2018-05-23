// QCPluginDirector.cpp: implementation of the QCPluginDirector class.
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
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "trnslate.h"
#include "rs.h"


#include "DebugNewHelpers.h"

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
			
			pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCPluginCommand( this, pAtt, eM.desc, CA_ATTACH_PLUGIN );
			
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
			
			pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCPluginCommand( this, pSpec, eM.desc, CA_SPECIAL_PLUGIN );
			
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
	
			pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCPluginCommand( this, pTranslator, szDesc, CA_TRANSLATE_PLUGIN );
			
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
	
			pCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCPluginCommand( this, pTranslator, szDesc, CA_TRANSMISSION_PLUGIN );
			
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

	// If we didn't add any menu items, then put in an item that says there are no translators
	if (theAction != CA_SPECIAL_PLUGIN && pPopupMenu->GetMenuItemCount() == 0)
		pPopupMenu->AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, CRString(IDS_EMS_NO_TRANSLATORS));
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
