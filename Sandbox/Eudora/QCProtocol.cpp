// QCProtocol.cpp: implementation of the QCProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "QCProtocol.h"
#include "ReadMessageFrame.h"
#include "BidentPreviewView.h"
#include "BidentReadMessageView.h"
#include "TridentPreviewView.h"
#include "TridentReadMessageView.h"
#include "PaigeEdtView.h"
#include "PgMsgView.h"
#include "PgCompMsgView.h"
#include "PgReadMsgView.h"
#include "PgReadMsgPreview.h"
#include "CompMessageFrame.h"
#include "pgDocumentFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCProtocol::QCProtocol()
{

}

QCProtocol::~QCProtocol()
{

}


QCProtocol* QCProtocol::QueryProtocol(
QCPROTOCOLID	theID, 
CObject*		pObject )
{
	if( pObject->IsKindOf( RUNTIME_CLASS( CTridentReadMessageView ) ) )
	{
		if( ( theID == QCP_FIND ) ||
			( theID == QCP_SAVE_INFO ) ||
			( theID == QCP_TRANSLATE ) ||
			( theID == QCP_GET_MESSAGE ) ||
			( theID == QCP_QUIET_PRINT ) )
		{
			
			CTridentReadMessageView*	pView;
			
			pView = ( CTridentReadMessageView* )pObject;
			return pView;
		}
	}
	else if( pObject->IsKindOf( RUNTIME_CLASS( CBidentReadMessageView ) ) )
	{
		if( ( theID == QCP_SAVE_INFO ) ||
			( theID == QCP_GET_MESSAGE ) ||
			( theID == QCP_QUIET_PRINT ) )
		{
			
			CBidentReadMessageView*	pView;
			
			pView = ( CBidentReadMessageView* )pObject;
			return pView;
		}
	}
	else if( pObject->IsKindOf( RUNTIME_CLASS( CTridentPreviewView ) ) )
	{
		if( ( theID == QCP_FIND ) ||
		    ( theID == QCP_GET_MESSAGE ) ||
		    ( theID == QCP_QUIET_PRINT ) )
		{
			
			CTridentPreviewView*	pView;
			
			pView = ( CTridentPreviewView* )pObject;
			return pView;
		}
	}
	else if( pObject->IsKindOf( RUNTIME_CLASS( CBidentPreviewView ) ) )
	{
		if( theID == QCP_QUIET_PRINT ) 
		{
			
			CBidentPreviewView*		pView;
			
			pView = ( CBidentPreviewView* )pObject;
			return pView;
		}
	}
	else if( pObject->IsKindOf( RUNTIME_CLASS( CCompMessageFrame ) ) )
	{
		if((theID == QCP_FORMAT_TOOLBAR ) || 
			(theID == QCP_QUIET_PRINT))
		{
			return ( CCompMessageFrame* ) pObject;
		}
	}else if( pObject->IsKindOf( RUNTIME_CLASS( CReadMessageFrame ) ) )
	{
		if(	(theID == QCP_QUIET_PRINT) ||
			(theID == QCP_FORMAT_TOOLBAR) ||
			(theID == QCP_READ_TOOLBAR) ) 
		{
			return ( CReadMessageFrame* ) pObject;
		}
	}else if( pObject->IsKindOf( RUNTIME_CLASS( PgDocumentFrame ) ) )
	{
		if(	theID == QCP_FORMAT_TOOLBAR ) 
		{
			return ( PgDocumentFrame* ) pObject;
		}
	} 
	else 
	{
		if (pObject->IsKindOf( RUNTIME_CLASS( PgCompMsgView ) ) )
		{
			if (( theID == QCP_TRANSLATE ) ||
				( theID == QCP_GET_MESSAGE))
				return ( PgCompMsgView* ) pObject;
		}

		if (pObject->IsKindOf( RUNTIME_CLASS( PgReadMsgView ) ) )
		{
			if (( theID == QCP_TRANSLATE ) ||
				( theID == QCP_GET_MESSAGE))
				return ( PgReadMsgView* ) pObject;
		}

		if (pObject->IsKindOf( RUNTIME_CLASS( PgReadMsgPreview ) ) )
		{
			if (( theID == QCP_TRANSLATE ) ||
				( theID == QCP_GET_MESSAGE))
				return ( PgReadMsgPreview* ) pObject;
		}

		if (pObject->IsKindOf( RUNTIME_CLASS( CPaigeEdtView ) ) )
		{
			if( ( theID == QCP_FIND		) || 
				( theID == QCP_SPELL	) ||
				( theID == QCP_METRICS	) ||
				( theID == QCP_QUIET_PRINT) ||
				( theID == QCP_TRANSLATE) )
			{
				return ( CPaigeEdtView*) pObject;
			}
		}
		
		if (pObject->IsKindOf( RUNTIME_CLASS( PgMsgView ) ) )
		{
			if( theID == QCP_SAVE_INFO  )
			{
				return ( PgMsgView*) pObject;
			}
		}	
	}

	
	return NULL;
}

QCChildToolBar* QCProtocol::GetFormatToolbar()
{
	ASSERT(0);
	return FALSE;
}

QCChildToolBar* QCProtocol::GetToolbar()
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::CheckSpelling( BOOL )
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::DoFindFirst( 
const CString&, 
BOOL, 
BOOL,
BOOL)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::DoFindNext( 
const CString&, 
BOOL, 
BOOL,
BOOL)
{
	ASSERT(0);
	return FALSE;
}


void QCProtocol::SaveInfo()
{
	ASSERT( 0 );
}


BOOL QCProtocol::QuietPrint()
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetSelectedText(
CString& szText )
{
	szText = "";
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetAllText(
CString& szText )
{
	szText = "";
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetSelectedHTML(
CString& szHTML )
{
	szHTML = "";
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetAllHTML(
CString& szHTML )
{
	szHTML = "";
	ASSERT(0);
	return FALSE;
}



BOOL QCProtocol::SetSelectedText(
const char* szText,bool bSign )
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetAllText(
const char* szText, bool bSign )
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetSelectedHTML(
const char* szHTML,bool bSign )
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::PasteOnDisplayHTML(
const char* szHTML,bool bSign )
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetAllHTML(
const char* szHTML,bool bSign )
{
	ASSERT(0);
	return FALSE;
}

BOOL QCProtocol::GetMessageAsHTML(
	CString& msg,
	BOOL IncludeHeaders)
{
	ASSERT(0);
	return FALSE;
}

BOOL QCProtocol::GetMessageAsText(
	CString& msg,
	BOOL IncludeHeaders)
{
	ASSERT(0);
	return FALSE;
}

int QCProtocol::GetTotalHeight()
{
	ASSERT(0);
	return 0;
}
