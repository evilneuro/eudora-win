// QCProtocol.cpp: implementation of the QCProtocol class.
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

#include "stdafx.h"

#include "QCProtocol.h"
#include "ReadMessageFrame.h"
#include "TridentPreviewView.h"
#include "TridentReadMessageView.h"
#include "PaigeEdtView.h"
#include "PgMsgView.h"
#include "PgCompMsgView.h"
#include "PgReadMsgView.h"
#include "PgReadMsgPreview.h"
#include "CompMessageFrame.h"
#include "pgDocumentFrame.h"
#include "tocview.h"
#include "filtersv.h"
#include "doc.h"
#include "nickdoc.h"
#include "nicktree.h"
#include "urledit.h"	// for URLEDIT.H
#include "nickpage.h"	// for NICKSHT.H
#include "nicksht.h"	// for NICKVIEW.H
#include "nickview.h"

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCProtocol::QCProtocol()
{

}

QCProtocol::~QCProtocol()
{

}


QCProtocol* QCProtocol::QueryProtocol(QCPROTOCOLID theID, CObject* pObject)
{
	if( pObject->IsKindOf( RUNTIME_CLASS( CTridentReadMessageView ) ) )
	{
		if( ( theID == QCP_FIND ) ||
			( theID == QCP_SAVE_INFO ) ||
			( theID == QCP_TRANSLATE ) ||
			( theID == QCP_GET_MESSAGE ) ||
			( theID == QCP_METRICS ) ||
			( theID == QCP_QUIET_PRINT ) )
		{
			
			CTridentReadMessageView*	pView;
			
			pView = ( CTridentReadMessageView* )pObject;
			return pView;
		}
	}
	else if( pObject->IsKindOf( RUNTIME_CLASS( CTridentPreviewView ) ) )
	{
		if( ( theID == QCP_FIND ) ||
		    ( theID == QCP_GET_MESSAGE ) ||
		    ( theID == QCP_QUIET_PRINT ) ||
			( theID == QCP_TRANSLATE ) )
		{
			
			CTridentPreviewView*	pView;
			
			pView = ( CTridentPreviewView* )pObject;
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
	}else if( pObject->IsKindOf( RUNTIME_CLASS( CTocView ) ) )
	{
		if(	theID == QCP_FIND ) 
		{
			return ( CTocView* ) pObject;
		}
	}else if( pObject->IsKindOf( RUNTIME_CLASS( CHeaderView ) ) )
	{
		if(	theID == QCP_FIND ) 
		{
			return ( CHeaderView* ) pObject;
		}
	}else if( pObject->IsKindOf( RUNTIME_CLASS( CFiltersViewLeft ) ) )
	{
		if(	theID == QCP_FIND ) 
		{
			return ( CFiltersViewLeft* ) pObject;
		}
	}else if( pObject->IsKindOf( RUNTIME_CLASS( CNicknamesViewLeft32 ) ) )
	{
		if(	theID == QCP_FIND ) 
		{
			return ( CNicknamesViewLeft32* ) pObject;
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
				( theID == QCP_GET_MESSAGE) ||
				(theID == QCP_FIND        ))
				return ( PgReadMsgPreview* ) pObject;
		}

		if (pObject->IsKindOf( RUNTIME_CLASS( CPaigeEdtView ) ) )
		{
			if( ( theID == QCP_FIND		) || 
				( theID == QCP_SPELL	) ||
				( theID == QCP_METRICS	) ||
				( theID == QCP_QUIET_PRINT) ||
				( theID == QCP_TRANSLATE) ||
				( theID == QCP_MOODMAIL))
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


BOOL QCProtocol::DoFindFirst(const char*, BOOL, BOOL, BOOL)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::DoFindNext(const char*, BOOL, BOOL, BOOL)
{
	ASSERT(0);
	return FALSE;
}


HRESULT QCProtocol::SaveInfo()
{
	ASSERT( 0 );

	return E_NOTIMPL;
}


BOOL QCProtocol::QuietPrint()
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetSelectedText(CString& szText)
{
	szText.Empty();
	ASSERT(0);
	return FALSE;
}


BOOL
QCProtocol::GetSelectedTextAndTrim(
	CString &				out_szText,
	bool					in_bAllowMultipleLines,
	bool					in_bDoSuperTrim,
	int						in_nMaxLength,
	bool					in_bUseElipsisIfTruncating)
{
	out_szText.Empty();
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetAllText(CString& szText)
{
	szText.Empty();
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetSelectedHTML(CString& szHTML)
{
	szHTML.Empty();
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::GetAllHTML(CString& szHTML)
{
	szHTML.Empty();
	ASSERT(0);
	return FALSE;
}



BOOL QCProtocol::SetSelectedText(const char* szText, bool bSign)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetAllText(const char* szText, bool bSign)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetSelectedHTML(const char* szHTML, bool bSign)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::PasteOnDisplayHTML(const char* szHTML, bool bSign)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::RefreshMessage()
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetAssociatedFiles(CStringList * in_pAssociatedFiles)
{
	ASSERT(0);
	return FALSE;
}


BOOL QCProtocol::SetAllHTML(const char* szHTML, bool bSign)
{
	ASSERT(0);
	return FALSE;
}

BOOL QCProtocol::GetMessageAsHTML(CString& msg, BOOL IncludeHeaders)
{
	msg.Empty();
	ASSERT(0);
	return FALSE;
}

BOOL QCProtocol::GetMessageAsText(CString& msg, BOOL IncludeHeaders)
{
	msg.Empty();
	ASSERT(0);
	return FALSE;
}

// we'll add this to QCP_GET_MESSAGE for now, but all this stuff needs to be
// replaced with something that at least makes a small amount of sense.
BOOL QCProtocol::OnSignatureChanged()
{
	ASSERT(0);
	return FALSE;
}

int QCProtocol::GetTotalHeight()
{
	ASSERT(0);
	return 0;
}
