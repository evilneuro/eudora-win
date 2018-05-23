// QCRecipientCommand.cpp: implementation of the QCRecipientCommand class.
//
//////////////////////////////////////////////////////////////////////

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


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


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
			{
				CString Name = *(CString *)pData;
				InsertRecipient( Name );
			}
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


void QCRecipientCommand::InsertRecipient( CString Name )
{
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
