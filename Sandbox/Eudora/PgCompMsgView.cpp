// PgCompMsgView.cpp : implementation file
//

#include "stdafx.h"
#include "QCUtils.h"	// required early else ugly stuff happens...
#include "PgCompMsgView.h"
#include "PgStyleUtils.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "pghlevel.h"
#include "msgutils.h"
#include "utils.h"

#include "msgdoc.h"
#include "summary.h"
#include "compmsgd.h"
#include "rs.h"
#include "resource.h"
#include "compmessageframe.h"
#include "pghtmdef.h"


#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern BOOL g_bInteractiveSpellCheck;


/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView

IMPLEMENT_DYNCREATE(PgCompMsgView, PgMsgView)

PgCompMsgView::PgCompMsgView() :
	m_SysKeyDownTime(0)
{
}

PgCompMsgView::~PgCompMsgView()
{
}


BEGIN_MESSAGE_MAP(PgCompMsgView, PgMsgView)
    //{{AFX_MSG_MAP(PgCompMsgView)
    ON_WM_CONTEXTMENU()
    ON_WM_CHAR()

    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView drawing

/*
void PgCompMsgView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}
*/

/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView diagnostics

#ifdef _DEBUG
void PgCompMsgView::AssertValid() const
{
	PgMsgView::AssertValid();
}

void PgCompMsgView::Dump(CDumpContext& dc) const
{
	PgMsgView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView message handlers


void PgCompMsgView::OnInitialUpdate() 
{
    PgMsgView::OnInitialUpdate();

    // set the document subtype; PgMsgView::OnInitialUpdate will have set
    // the major type (kMessage)
    //
    // BOG: maybe this should be in OnCreate?
    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    pSB->subkind = PgStuffBucket::kCompMessage;
    UnuseMemory( m_paigeRef );

    ImportMessage();
    pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);

    if (! pgTextSize(m_paigeRef) )
    {
        pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);
    }

    CCompMessageDoc* pDoc = (CCompMessageDoc *) GetDocument();
    ASSERT_KINDOF(CCompMessageDoc, pDoc);
    if (pDoc->m_Sum->CantEdit())
        SetReadOnly();

    if (GetIniShort(IDS_INI_SEND_PLAIN_ONLY) && !GetIniShort(IDS_INI_WARN_QUEUE_STYLED_TEXT))
        SetAllowStyled(false);

    CCompMessageFrame* pFrame = (CCompMessageFrame*) GetParentFrame();
    m_pHeader = pFrame->GetHeaderView();
    UpdateScrollBars(true);

    if ( !g_bInteractiveSpellCheck && pgTextSize( m_paigeRef ) && !IsReadOnly() ) {
        select_pair sp;
        sp.begin = 0;
        sp.end = pgTextSize( m_paigeRef );
        m_spell.Check( &sp );
    }
}


BOOL PgCompMsgView::GetMessageAsHTML( CString& szText, BOOL IncludeHeaders )
{
	pg_ref impRef;

	szText.Empty();

	if (IncludeHeaders)
	{
		select_pair sel;
		sel.begin = 0;
		sel.end = pgTextSize(m_paigeRef);
		impRef = pgCopy(m_paigeRef, &sel);

		CString hdrs;
		( (CCompMessageDoc *)GetDocument() )->GetMessageHeaders(hdrs);

		int size = hdrs.GetLength();

		char *hdrText = new char [size + 1];

		strcpy(hdrText, hdrs);

		pgInsert(impRef, (pg_byte_ptr)hdrText, size, 0, key_insert_mode, 0, draw_none);
		GetTextAs( impRef, szText, PgDataTranslator::kFmtHtml );

		delete hdrText;
		
		UnuseAndDispose(impRef);
	}
	else
		GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtHtml );


	return (TRUE);
}


BOOL PgCompMsgView::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
	pg_ref impRef;

	szText.Empty();

	if (IncludeHeaders)
	{
		select_pair sel;
		sel.begin = 0;
		sel.end = pgTextSize(m_paigeRef);
		impRef = pgCopy(m_paigeRef, &sel);

		CString hdrs;
		( (CCompMessageDoc *)GetDocument() )->GetMessageHeaders(hdrs);
		int size = strlen(hdrs);
		char *hdrText = new char[size + 1];

		strcpy(hdrText, hdrs);

		pgInsert(impRef, (pg_byte_ptr)hdrText, size, 0, key_insert_mode, 0, draw_none);

		GetTextAs( impRef, szText, PgDataTranslator::kFmtText );

		UnuseAndDispose(impRef);
	}
	else
		GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtText );



	return ( TRUE );
}

////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void PgCompMsgView::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	CCompMessageDoc* pDoc = (CCompMessageDoc *) GetDocument();
	ASSERT_KINDOF(CCompMessageDoc, pDoc);
	pDoc->DoContextMenu(this, WPARAM(pWnd->GetSafeHwnd()), MAKELPARAM(ptScreen.x, ptScreen.y));
} 

void PgCompMsgView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	TCHAR uChar = (TCHAR) nChar;


	// Process the keystrokes only if the document is not read only.
	// The arrow keys are processed in OnKeyDown
	if (!m_fRO) 
	{
		CCompMessageDoc* pDoc = (CCompMessageDoc *) GetDocument();

		ASSERT_KINDOF(CCompMessageDoc, pDoc);
		CSummary *pSummary;

		pSummary = pDoc->m_Sum;

		if(nChar == VK_TAB)
		{
			if (!pSummary->TabsInBody() || GetKeyState(VK_SHIFT) < 0)
			{
				int nHeader = HEADER_TO;

				if (GetKeyState(VK_SHIFT) < 0)
					nHeader = (GetIniShort(IDS_INI_EDIT_ALL_HEADERS))? HEADER_ATTACHMENTS : HEADER_BCC;
				m_pHeader->SetFocusToHeader(nHeader);
				((CSplitterWnd*)GetParent())->SetActivePane(0,0);
			}
			else
			{
				CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags); 
			}
		}
		else
			CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags); 
	}
}


BOOL PgCompMsgView::PreTranslateMessage( MSG* pMsg )
{
    if ( pMsg->message == WM_SYSKEYDOWN ) {
        int nVirtKey = (int) pMsg->wParam;
        UINT nFlags = HIWORD(pMsg->lParam);

        if ( (nFlags & 0xE000) == 0x2000 ) {
            if ( nVirtKey != VK_MENU )
                return m_pHeader->SetFocusToHeaderWithShortcut( nVirtKey );
        }
    }

    return FALSE;    // we didn't handle it
}


void PgCompMsgView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	//Insert the message headers
	SetRedraw( FALSE );

	if ( !(pInfo->m_pPD->m_pd.Flags & PD_SELECTION) )
	{
		//Remember the currect selection and reset it after inserting the headers
		select_pair sel;
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
		
		InsertPrintHeader();
		
		sel.begin += chrgHeader.cpMax;
		sel.end += chrgHeader.cpMax;
		pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
	}

	// call the base class
	CPaigeEdtView::OnBeginPrinting( pDC, pInfo );
}


// OnEndPrinting:
// Called by the framework at the end of a printing job.
//
// This is where ya do cleanup after the job. We need to remove the message
// header text, and turn redraw back on.

void PgCompMsgView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CPaigeEdtView::OnEndPrinting( pDC, pInfo );

	// Remove the message headers. 
	// Done at the end to avoid redraw problems due to undo_format after printing
	if ( !(pInfo->m_pPD->m_pd.Flags & PD_SELECTION) )
	{
		RemovePrintHeader();
		//The selection offsets retained in the base class call have to be offset
		//by the size of the header
		m_thePrintInfo.m_currSelection.begin -= chrgHeader.cpMax;
		m_thePrintInfo.m_currSelection.end -= chrgHeader.cpMax;
		pgSetSelection(m_paigeRef, m_thePrintInfo.m_currSelection.begin, m_thePrintInfo.m_currSelection.end, 0, TRUE);
	}
}


// InsertPrintHeader:
// Inserts the header text prior to printing a message
//
// This is the easiest way to get the message header printed. It causes a
// little UI ugliness, because we turn off redraw, but otherwise it works
// pretty darn good. Currently we print like v2.2 does, but it might be
// nice to fancy it up with a little character/paragraph formatting.

void PgCompMsgView::InsertPrintHeader()
{
	// suck out the text for each field *and* its associated label, building
	// up one big block of text to be inserted at the top of the document.
	CString theFieldText;
	CString theLabelText;
	CString theHeaderText = "";

	for ( int i = 0; i <= 5; i++ ) {
		m_pHeader->GetText( i, theLabelText, theFieldText );
		theHeaderText += theLabelText + " " + theFieldText + "\n";
	}

	// double space between header and body text
	theHeaderText += "\r\n";

	long nLength = theHeaderText.GetLength();
	// save-off the selection range for later
	chrgHeader.cpMin = 0;
	chrgHeader.cpMax = nLength;
	
	char *pBuffer = theHeaderText.GetBuffer(nLength);

	pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);	
	
	// stuff it in at the beginning of the message
	pgInsert( m_paigeRef, (pg_byte_ptr)pBuffer, nLength, CURRENT_POSITION, 
							data_insert_mode, 0, best_way );

	theHeaderText.ReleaseBuffer();

	//Need to clear any formatting that the inserted text may have picked up along the way
	//Pg will clear the style/formatting from the current selection point to the end of a paragraph ,i.e until a \r\n is 
	//found.  We only insert one of these pairs (see above) at the end of the header information.
	pgSetSelection(m_paigeRef, 0, nLength - 3, 0, TRUE); //- 3 for /n/r/n found on the end	

	//Clear the style first
	ClearStyle();
	//Now Clear Formatting
	ClearFormat();

	select_pair sel;
	sel.begin = 0;
	sel.end = nLength - 3;

	style_info	mask, info;
	pgFillBlock( &mask, sizeof(style_info), SET_MASK_BITS );

	//info = PgGlobalsPtr()->def_style;
	long styleID = pgGetNamedStyleIndex (m_paigeRef, body_style);
	ASSERT(styleID);

	//Apply the message font and size because we change it to the printer font and size
	//in the base class for the whole document.
	if ( pgGetStyle(m_paigeRef, (short)styleID, &info) )
		pgSetStyleInfo(m_paigeRef, &sel, &info, &mask, draw_none);
	else
	{
		info = PgGlobalsPtr()->def_style;
		pgSetStyleInfo(m_paigeRef, &sel, &info, &mask, draw_none);
	}
}


// RemovePrintHeader:
// Removes the header text created with InsertPrintHeader

void PgCompMsgView::RemovePrintHeader()
{
	// this just set the selection using the range we saved-off in
	// InsertPrintHeader(), and replaces it with nothing.
	pgSetSelection(m_paigeRef, chrgHeader.cpMin, chrgHeader.cpMax, 0, TRUE);	
	pgDelete(m_paigeRef, NULL, best_way);
}


void PgCompMsgView::OnEditPaste() 
{
	COleDataObject clipboardData;
	VERIFY( clipboardData.AttachClipboard() );

	// if this is file(s) on the clipboard, we'll pass this on to
	// our parent frame and bail; otherwise call our base impl.
	if ( clipboardData.IsDataAvailable( CF_HDROP ) ) {

		// suck out the HDROP and delegate to OnDropFiles.
		STGMEDIUM stgMedium;
		clipboardData.GetData( CF_HDROP, &stgMedium );

		CWnd* pPF = GetParentFrame();
		pPF->SendMessage( WM_DROPFILES,(WPARAM)stgMedium.hGlobal );
		SetFocus();
	}
	else {
		CPaigeEdtView::OnPaste( &clipboardData, undo_ole_paste );
	}
}


void PgCompMsgView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if ( !m_fRO ) {
		COleDataObject clipBoardData;
		VERIFY( clipBoardData.AttachClipboard() );

		if ( clipBoardData.IsDataAvailable( CF_HDROP ) ) {
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	CPaigeEdtView::OnUpdateEditPaste( pCmdUI );
}
