// QCRecipientCommand.cpp: implementation of the QCRecipientCommand class.
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

#include "STDAFX.H"
#include "QCCommandActions.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "QCProtocol.h"

// these will all go away
#include <afxrich.h>
#include "resource.h"  // for the fake ids
#include "compmsgd.h"
#include "guiutils.h"
#include "nickdoc.h"
#include "mainfrm.h"
#include "summary.h"


#include "DebugNewHelpers.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(QCRecipientCommand, QCCommandObject)

QCRecipientCommand::QCRecipientCommand(
QCRecipientDirector*	pDirector,
LPCSTR					szNickName ) : QCCommandObject( pDirector )
{
	m_szNickName = szNickName;
}

QCRecipientCommand::~QCRecipientCommand()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
void	QCRecipientCommand::Execute(
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	CCompMessageDoc*	pCompMessageDoc;
	CSummary*			pSummary;

	NotifyDirector( theAction, pData );	

	switch( theAction ) 
	{
		case CA_NEW_MESSAGE_TO:
			
			pCompMessageDoc = NewCompDocument( m_szNickName );

			if( pCompMessageDoc )
			{
				pCompMessageDoc->m_Sum->Display();
			}
		
		break;

		case CA_FORWARD_TO:
			
			ASSERT( pData );

			pSummary = ( CSummary* ) pData;			
			pSummary->ComposeMessage(ID_MESSAGE_FORWARD, m_szNickName );
		
		break;

		case CA_REDIRECT_TO:
		
			ASSERT( pData );

			pSummary = ( CSummary* ) pData;			
			pSummary->ComposeMessage(ID_MESSAGE_REDIRECT, m_szNickName );
		
		break;

		case CA_RENAME:
			m_szNickName = ( LPCSTR ) pData;
		break;

		case CA_INSERT_RECIPIENT:
			if ( pData )
				InsertRecipient( (const char*)pData );
		break;
	}
}


UINT QCRecipientCommand::GetFlyByID(
COMMAND_ACTION_TYPE theAction )
{
	switch( theAction )
	{
		case CA_NEW_MESSAGE_TO:
			return ID_RECIPIENT_NEW_MESSAGE;
		break;
		case CA_FORWARD_TO:
			return ID_RECIPIENT_FORWARD;
		break;
		case CA_REDIRECT_TO:
			return ID_RECIPIENT_REDIRECT;
		break;
		case CA_INSERT_RECIPIENT:
			return ID_RECIPIENT_INSERT;
		break;
		case CA_DELETE:
			return ID_RECIPIENT_REMOVE;
		break;

	}

	return 0;
}


CString	QCRecipientCommand::GetToolTip(
COMMAND_ACTION_TYPE theAction)
{
	CString szToolTip( "" );

	switch( theAction )
	{
		case CA_NEW_MESSAGE_TO:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_NEW_MESSAGE );
			szToolTip += " ";
			szToolTip += m_szNickName;
		break;
		case CA_FORWARD_TO:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_FORWARD );
			szToolTip += " ";
			szToolTip += m_szNickName;
		break;
		case CA_REDIRECT_TO:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_REDIRECT );
			szToolTip += " ";
			szToolTip += m_szNickName;
		break;
		case CA_INSERT_RECIPIENT:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_INSERT );
			szToolTip += " ";
			szToolTip += m_szNickName;
		break;
	}

	return szToolTip;
}


void QCRecipientCommand::InsertRecipient( const char* Name )
{
	if (!Name || !*Name)
		return;

	char Recip[256];
	strcpy(	Recip, Name );

	if (ShiftDown())
	{
		char *expandAl = ExpandAliases(Recip);
		if (expandAl)
		{
			ASSERT( strlen( expandAl ) < sizeof( Recip ) );
			if ( strlen( expandAl ) < sizeof( Recip ) )
				strcpy( Recip, expandAl );
			delete [] expandAl;
		}
	}

	CMainFrame* p_mainframe = (CMainFrame *) AfxGetMainWnd();
	ASSERT( p_mainframe );
	if (!p_mainframe) return;

	CEdit* Edit = p_mainframe->GetCurrentEdit();
	if (Edit)
	{
		Edit->SendMessage( EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)Recip );
		return;
	}
	else
	{
		CRichEditCtrl* RichEdit = p_mainframe->GetCurrentRichEdit();
		if (RichEdit)
		{
			RichEdit->SendMessage( EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)Recip );
			return;
		}
		else
		{
			CObject * pObj = p_mainframe->GetFocus();
			if ( pObj ) {
				QCProtocol* QP = QCProtocol::QueryProtocol( QCP_TRANSLATE, pObj );
				if ( QP )
				{
					QP->SetSelectedText( Recip, FALSE );
					return;
				}
			}
		}
	}
}
