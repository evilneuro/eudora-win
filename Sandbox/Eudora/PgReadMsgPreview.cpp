// PgReadMsgPreview.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PgReadMsgPreview.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "pghtmdef.h"

#include "summary.h"
#include "tocview.h"
#include "tocframe.h"
#include "msgdoc.h"
#include "msgutils.h"
#include "text2html.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview

IMPLEMENT_DYNCREATE(PgReadMsgPreview, PgMsgView)

PgReadMsgPreview::PgReadMsgPreview() 
{
}

PgReadMsgPreview::~PgReadMsgPreview()
{
}


extern UINT umsgLoadNewPreview;

BEGIN_MESSAGE_MAP(PgReadMsgPreview, PgMsgView)
	//{{AFX_MSG_MAP(PgReadMsgPreview)
	ON_WM_SYSKEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(umsgLoadNewPreview, LoadNewPreview)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview diagnostics

#ifdef _DEBUG
void PgReadMsgPreview::AssertValid() const
{
	PgMsgView::AssertValid();
}

void PgReadMsgPreview::Dump(CDumpContext& dc) const
{
	PgMsgView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview message handlers
void PgReadMsgPreview::OnInitialUpdate() 
{
	SetReadOnly();

	PgMsgView::OnInitialUpdate();
	
	LoadNewPreview(0, 0);
}


void PgReadMsgPreview::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Turn message-switching hotkeys into ID_PREVIOUS_MESSAGE/ID_NEXT_MESSAGE
	if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN)
	{
		BOOL CtrlDown = (GetKeyState(VK_CONTROL) < 0);
		BOOL AltArrows = GetIniShort(IDS_INI_ALT_ARROWS) != 0;
		
		if (AltArrows && !CtrlDown)
		{
			SendMessage(WM_COMMAND, (nChar == VK_LEFT || nChar == VK_UP) ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
			return;
		}
	}
	
	PgMsgView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to "tweak" the standard command routing to forward commands from the
// view to the "fake" CMessageDoc document that was used to create this
// view in the first place.
////////////////////////////////////////////////////////////////////////
BOOL PgReadMsgPreview::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	switch (nID)
	{
	case ID_NEXT_MESSAGE:
	case ID_PREVIOUS_MESSAGE:
		//
		// Fall through and let the MessageDoc, if any, handle these.
		// Trust me.
		//
		break;
	case ID_EDIT_UNDO:
		{
			//Since PaigeEdtView has a handler for Undo, the tocdoc will
			//never get a crack at it. Since previewview does not need to handle
			//the undo msg, we'll let tocdoc handle it
			CTocDoc *pTocDoc = (CTocDoc*) GetDocument();
			if (pTocDoc->IsKindOf( RUNTIME_CLASS(CTocDoc) ))
			{
				pTocDoc->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
				return TRUE;
			}
			break;
		}
	default:
		//
		// Let this view and the TocDoc have first crack at the command.
		//
		if (PgMsgView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
		break;
	}

	//
	// If we get this far, neither this view nor the TocDoc handled the
	// command, so let the preview's MessageDoc have a crack at it.
	//
	CSummary*	pSummary = NULL;

	CTocFrame* pTocFrame = (CTocFrame *) GetParentFrame();		// the Ugly Typecast(tm)
	if (pTocFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)))
		pSummary = pTocFrame->GetPreviewSummary();
	else
	{
		ASSERT(0);
	}

	if( pSummary == NULL )
		return FALSE;

	//
	// This is ugly since GetMessageDoc() leaves the message document
	// loaded into memory, typically without a view.  We rely
	// on CTocFrame to clean up this document for us.
	//
	CMessageDoc* pDoc = pSummary->GetMessageDoc();
	return pDoc->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

////////////////////
// This should retrieve the full message body or the header depending on the Include Headers line.
// In the preview we're going to have to look up the whole message, and read it in and then


BOOL PgReadMsgPreview::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
	//Should collapse this func and GetMessageAsHtml, but maybe too expensive.
	//Lets leave it for now.
	pg_ref impRef;

	szText = "";

	if (IncludeHeaders)
	{
		CSummary *pSum = ((CTocDoc *)m_pDocument)->GetPreviewableSummary();
		CMessageDoc* pMsgDoc = pSum->GetMessageDoc();
		char *blah = pMsgDoc->GetFullMessage( TRUE );

		char *floater = strstr(blah, "\r\n\r\n");
		if (floater)
		{
			*floater = 0;
		}
		szText = blah;
		delete [] blah;
	}

	CString tempMsg;
	char *TempBuffer;

	select_pair sel;
	sel.begin = 0;
	sel.end = pgTextSize(m_paigeRef);

	impRef = pgCopy(m_paigeRef, &sel);

	// We're going to get the whole message out of the pane and then 
	// rip the headers off
	GetTextAs( impRef, tempMsg, PgDataTranslator::kFmtText );
	TempBuffer = new char [tempMsg.GetLength()+1];
	if (!TempBuffer)
		return false;

	strcpy(TempBuffer, tempMsg);

	char *floater = strstr(TempBuffer, "\r\n\r\n");
	if (floater)
	{
		szText += floater;
	}
	else
	{
		szText += TempBuffer;
	}

	delete [] TempBuffer;

	UnuseAndDispose(impRef);

	return (TRUE);
}

void PgReadMsgPreview::OnSetFocus( CWnd* pOldWnd ) 
{
	PgMsgView::OnSetFocus( pOldWnd );
	ShowCaret();
}

void PgReadMsgPreview::OnKillFocus( CWnd* pNewWnd ) 
{
	PgMsgView::OnKillFocus( pNewWnd );
	HideCaret();
}

LRESULT PgReadMsgPreview::LoadNewPreview(WPARAM, LPARAM)
{
	CSummary*	pSummary = NULL;

	CTocFrame* pTocFrame = (CTocFrame *) GetParentFrame();		// the Ugly Typecast(tm)
	if (pTocFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)))
		pSummary = pTocFrame->GetPreviewSummary();

	SetRedraw(FALSE);
	NewPaigeObject();

	if( pSummary == NULL )
	{
		SetRedraw(TRUE);
		Invalidate();
		return TRUE;
	}


	CMessageDoc* pDoc = pSummary->GetMessageDoc();
	
	SetMessageStuff( pDoc );
	ImportMessage( pDoc );

	//Show the To, Cc, Subject headers for the preview msg
	char* FullMsg = pDoc->GetFullMessage();
	CString PreviewHeaders(StripNonPreviewHeaders(FullMsg));
	PreviewHeaders += "\r\n";

	int bytes = PreviewHeaders.GetLength();
	// create a Paige memory ref containing the headers
	memory_ref impRef = Bytes2Memory( PgMemGlobalsPtr(),
				(pg_char_ptr)(LPCTSTR)PreviewHeaders, sizeof(pg_char), bytes );

	long sizeBefore = pgTextSize(m_paigeRef);

	//Remember the par info because pgApplyNamedStyle func of
	//paige calculated style runs incorrectly
	select_pair sel;
	sel.begin = sel.end = 0;
	par_info info, mask;
	pgGetParInfo(m_paigeRef, &sel, FALSE, &info, &mask);

	// Import the headers at the beginning.
	pgSetSelection( m_paigeRef, 0, 0, 0, TRUE );
	pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);
	PgDataTranslator theTxr( m_paigeRef );
	theTxr.ImportData( &impRef, PgDataTranslator::kFmtText);

	long sizeAfter = pgTextSize(m_paigeRef);

	//Now reset the par info for the first line of the paragraph
	//else the first paragraph will lose the formatting
	sel.begin = sel.end = sizeAfter - sizeBefore;
	pgFillBlock( &mask, sizeof(par_info), SET_MASK_BITS );
	pgSetParInfo(m_paigeRef, &sel, &info, &mask, best_way);

	//Clean up
	delete [] FullMsg;
	DisposeMemory( impRef );

	SetRedraw(TRUE);

	// set the cursor to beginning of message body, and set it to
	// read only.
	pgSetSelection( m_paigeRef, 0, 0, 0, TRUE );
	Invalidate();
	UpdateScrollBars(true);
	HideCaret();

	return TRUE;
}
