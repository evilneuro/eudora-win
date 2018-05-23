// PgCompMsgView.cpp : implementation file
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

#include "QCUtils.h"	// required early else ugly stuff happens...
#include "PgCompMsgView.h"
#include "PgStyleUtils.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "pghlevel.h"
#include "msgutils.h"
#include "utils.h"
#include "QCFindMgr.h"
#include "SearchEngine.h" // StringSearch

#include "msgdoc.h"
#include "summary.h"
#include "compmsgd.h"
#include "rs.h"
#include "resource.h"
#include "compmessageframe.h"
#include "pghtmdef.h"
#include "QCSharewareManager.h"
#include "MoodMailStatic.h"
#include "MoodWatch.h"
#include "Text2Html.h"
#include "TBarSendButton.h"
#include "DebugNewHelpers.h"


extern BOOL g_bInteractiveSpellCheck;
extern BOOL g_bMoodMailCheck;
extern DWORD g_nMoodMailInterval;


/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView

IMPLEMENT_DYNCREATE(PgCompMsgView, PgMsgView)

PgCompMsgView::PgCompMsgView()
	:	m_SysKeyDownTime(0), m_pHeader(NULL)
{
	m_LastCharTypedTime = 0;
	m_nScore = -1;
}

PgCompMsgView::~PgCompMsgView()
{
}


BEGIN_MESSAGE_MAP(PgCompMsgView, PgMsgView)
    //{{AFX_MSG_MAP(PgCompMsgView)
    ON_WM_CONTEXTMENU()
    ON_WM_CHAR()
	ON_WM_KEYDOWN()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(WM_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEASQUOTATION, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_PICTURE, OnUpdateInsertPicture)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView drawing

void PgCompMsgView::OnDraw( CDC* pDC )
{
	// BOG: paige doesn't always call our signature glitter proc---not sure why.
	// here's a little hack to make sure the SigBar always gets drawn.

	CCompMessageDoc* pDoc;
	VERIFY(pDoc = (CCompMessageDoc *) GetDocument());
	ASSERT_KINDOF(CCompMessageDoc, pDoc);

	if ( pDoc && pDoc->m_Sum && (pDoc->m_Sum->GetFlagsEx() & MSFEX_INL_SIGNATURE) ) {
		rectangle visBounds;
		pgAreaBounds( m_paigeRef, NULL, &visBounds );

		select_pair sigRng, visRng;
		visRng.begin = pgPtToChar( m_paigeRef, &visBounds.top_left, NULL );
		visRng.end = pgPtToChar( m_paigeRef, &visBounds.bot_right, NULL );

		if ( find_signature( &sigRng, &visRng ) ) {
			select_pair firstLine;
			pgFindLine( m_paigeRef, sigRng.begin, &firstLine.begin, &firstLine.end );

			rectangle lineRect;
			pgTextRect( m_paigeRef, &firstLine, TRUE, FALSE, &lineRect );
			lineRect.bot_right.h = visBounds.bot_right.h - visBounds.top_left.h;

			shape_ref sigRgn, visRgn, hackRgn;
			hackRgn = pgRectToShape( PgMemGlobalsPtr(), NULL );
			sigRgn  = pgRectToShape( PgMemGlobalsPtr(), &lineRect );
			visRgn  = pgGetVisArea( m_paigeRef );

			pgSectShape( sigRgn, visRgn, hackRgn );
			pgErasePageArea( m_paigeRef, hackRgn );
			pgDisplay( m_paigeRef, NULL, hackRgn, MEM_NULL, NULL, bits_emulate_or );

			// don't dispose the VisArea!
			pgDisposeShape( sigRgn );
			pgDisposeShape( hackRgn );
		}
	}

	CPaigeEdtView::OnDraw( pDC );
}


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

    CCompMessageDoc* pDoc = (CCompMessageDoc *) GetDocument();
    ASSERT_KINDOF(CCompMessageDoc, pDoc);

	// make ImportMessage use our translator
	PgDataTranslator txr( m_paigeRef );

	// cause it knows what we want
	if ( pDoc->m_ResponseType != MS_REDIRECT )
		txr.m_importFlags |= IMPORT_SIG_FORMAT_FLAG;

	CString		szBody;

	// For now relax local file ref stripping when we were created
	// from stationery. We may or may not want to keep this. It doesn't
	// seem like much of a security risk, but OTOH stationery that
	// Eudora creates doesn't contain local file refs.
	GetMessageForDisplay( pDoc, ContentConcentrator::kCCNoConcentrationContext,
						  false, (pDoc->WasStationeryApplied() == TRUE), szBody );
    ImportMessage(szBody, &txr);

	if ( pDoc->m_Sum->CantEdit() )
	{
		// Can't edit - so just put the cursor at the start
		pgSetSelection(m_paigeRef, 0, 0, 0, FALSE);
	}
	else
	{
		// Select according to settings and whether or not the composition window
		// was just created because of a reply.
		long	nTextSize = pgTextSize(m_paigeRef);

		select_pair		selAllText = { 0, nTextSize };
		select_pair		selSig;
		
		// If there's a signature - the end of the text is before it
		if ( find_signature(&selSig, &selAllText) )
			nTextSize = selSig.begin - 1;

		if (pDoc->m_ResponseType == MS_REPLIED)
		{
			if ( GetIniShort(IDS_INI_WHEN_REPLYING_SELECT_ALL) )
			{
				pgSetSelection(m_paigeRef, 0, nTextSize, 0, FALSE);
			}
			else if ( GetIniShort(IDS_INI_WHEN_REPLYING_TYPE_AFTER) )
			{
				pgSetSelection(m_paigeRef, nTextSize, nTextSize, 0, FALSE);

				// If message is long enough bottom may not be in view - scroll to cursor
				ScrollToCursor();
			}
			else if ( GetIniShort(IDS_INI_WHEN_REPLYING_TYPE_BEFORE) )
			{
				pgSetSelection(m_paigeRef, 0, 0, 0, FALSE);
			}
		}
		else
		{
			if ( GetIniShort(IDS_INI_WHEN_OPENING_COMP_WIN_SELECT_ALL) )
			{
				pgSetSelection(m_paigeRef, 0, nTextSize, 0, FALSE);
			}
			else if ( GetIniShort(IDS_INI_WHEN_OPENING_COMP_WIN_SELECT_AFTER) )
			{
				pgSetSelection(m_paigeRef, nTextSize, nTextSize, 0, FALSE);

				// If message is long enough bottom may not be in view - scroll to cursor
				ScrollToCursor();
			}
			else if ( GetIniShort(IDS_INI_WHEN_OPENING_COMP_WIN_SELECT_BEFORE) )
			{
				pgSetSelection(m_paigeRef, 0, 0, 0, FALSE);
			}
		}
	}

	if (! pgTextSize(m_paigeRef) )
        pgApplyNamedStyle(m_paigeRef, NULL, body_style, draw_none);

	load_signature( NULL, false );

    if (pDoc->m_Sum->CantEdit())	
        SetReadOnly();

    if (GetIniShort(IDS_INI_SEND_PLAIN_ONLY) && !GetIniShort(IDS_INI_WARN_QUEUE_STYLED_TEXT))
        SetAllowStyled(false);

    CCompMessageFrame* pFrame = (CCompMessageFrame*) GetParentFrame();
    m_pHeader = pFrame->GetHeaderView();
    UpdateScrollBars(true);

    if (UsingFullFeatureSet())
	if ( !g_bInteractiveSpellCheck && pgTextSize( m_paigeRef ) && !IsReadOnly() ) {
        select_pair sp;
        sp.begin = 0;
        sp.end = pgTextSize( m_paigeRef );
        m_spell.Check( &sp );
    }

    g_nMoodMailInterval = GetIniLong( IDS_INI_MOOD_MAIL_INTERVAL);
	g_bMoodMailCheck = GetIniShort( IDS_INI_MOOD_MAIL_CHECK );
	if(UsingFullFeatureSet())
	{
		//Mood Mail changes
		if( !IsReadOnly())
		{
			SetMoodTimer();
		}
		else
		{
			UpdateMoodMailButton(pDoc->m_Sum->m_nMood);
		}
	}

	// See if the message has been addressed & has a subject
	if ( ( pDoc->m_Headers[ HEADER_TO ] == "" ) || ( pDoc->m_Headers[ HEADER_SUBJECT ] == "" ) ) 
	{
		// Disable the cursor in "this" view
		pgSetHiliteStates(m_paigeRef, deactivate_verb, no_change_verb, TRUE);
		// Set the focus in the header & appropriately in the field
		m_pHeader->SetFocus();
		if (pDoc->m_Headers[HEADER_TO] == "")
			m_pHeader->SetFocusToHeader(HEADER_TO);
		else if (pDoc->m_Headers[HEADER_SUBJECT] == "")
			m_pHeader->SetFocusToHeader(HEADER_SUBJECT);
		// Show the caret
		m_pHeader->ShowCaret();
	}
}


void PgCompMsgView::SetMoreMessageStuff(
	PgStuffBucket *						pStuffBucket)
{
	pStuffBucket->subkind = PgStuffBucket::kCompMessage;
}


BOOL PgCompMsgView::GetMessageAsHTML( CString& szText, BOOL IncludeHeaders )
{
	szText.Empty();

	if (IncludeHeaders)
	{
		select_pair sel;
		sel.begin = 0;
		sel.end = pgTextSize(m_paigeRef);
		pg_ref impRef = pgCopy(m_paigeRef, &sel);

		CString hdrs;
		( (CCompMessageDoc *)GetDocument() )->GetMessageHeaders(hdrs);

		szText = Text2Html(hdrs, TRUE, FALSE, FALSE);

		// pgCopy() will return a NULL memory ref if the selection is empty,
		// which in this case means that the body of the message is empty.
		if (impRef)
		{
			CString Body;

			GetTextAs(impRef, Body, PgDataTranslator::kFmtHtml);
			szText += Body;
			UnuseAndDispose(impRef);
		}
	}
	else
		GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtHtml );

	return (TRUE);
}


BOOL PgCompMsgView::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
	szText.Empty();

	if (IncludeHeaders)
	{
		select_pair sel;
		sel.begin = 0;
		sel.end = pgTextSize(m_paigeRef);
		pg_ref impRef = pgCopy(m_paigeRef, &sel);

		CString hdrs;
		( (CCompMessageDoc *)GetDocument() )->GetMessageHeaders(hdrs);

		szText = hdrs;

		// pgCopy() will return a NULL memory ref if the selection is empty,
		// which in this case means that the body of the message is empty.
		if (impRef)
		{
			CString Body;

			GetTextAs(impRef, Body, PgDataTranslator::kFmtText);
			szText += Body;
			UnuseAndDispose(impRef);
		}
	}
	else
		GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtText );



	return ( TRUE );
}

// import type of sig---text or html
inline PgDataTranslator::FormatType sig_format( const char* sig ) {
	return IsFancy( sig ) ? PgDataTranslator::kFmtHtml : PgDataTranslator::kFmtText;
}

#include "sendmail.h"
bool PgCompMsgView::load_signature( CCompMessageDoc* pDoc /*= 0*/, bool bReplace /*=false*/ )
{
	if ( !UsingFullFeatureSet() || !GetIniShort( IDS_INI_INLINE_SIGNATURE ) )
		return false;

	if ( !pDoc ) {
		VERIFY(pDoc = (CCompMessageDoc *) GetDocument());
		ASSERT_KINDOF(CCompMessageDoc, pDoc);
	}

	// is there already one in the body?
	select_pair sigRng;
	bool foundSig = find_signature( &sigRng );

	// if we didn't find an inline sig, but the toc says there is one, or if
	// we've been told not to replace, then just leave it alone.
	if ( (!foundSig && pDoc->m_Sum->GetFlagsEx() & MSFEX_INL_SIGNATURE)
			|| (foundSig && !bReplace) ) {

		return false;
	}
	else if ( foundSig ) {
		// smoke existing sig
		pgDelete( m_paigeRef, &sigRng, best_way );

		// sigs that were imported [i.e. from the outbox] are hard to remove.
		// if we can find it again, smoke the preceeding CR as well.
		if ( foundSig = find_signature(&sigRng) ) {
			sigRng.begin -= 1;
			pgDelete( m_paigeRef, &sigRng, best_way );
		}

		// update the toc flags---we no longer have a sig
		pDoc->m_Sum->UnsetFlagEx( MSFEX_INL_SIGNATURE );
	}

	// get the current user-specified sig
	char* curSig = GetSignature( pDoc, TRUE );

	if ( curSig && *curSig ) {
		// sig's always at end of document
		select_pair curSel;
		long sigOffset = pgTextSize( m_paigeRef );
		pgGetSelection( m_paigeRef, &curSel.begin, &curSel.end );
		pgSetSelection( m_paigeRef, sigOffset, sigOffset, 0, FALSE );

		TCHAR cr = (TCHAR) PgGlobalsPtr()->line_wrap_char;

		// always start on our own line, never on offset zero
		if ( !sigOffset || !pgCharType(m_paigeRef, sigOffset-1, PAR_SEL_BIT) ) {
			pgInsert( m_paigeRef, (pg_char_ptr)&cr, 1, CURRENT_POSITION,
					data_insert_mode, 0, best_way );
			sigOffset++;
		}

		SetTextAs( curSig, 0, sig_format(curSig), false, false, false );

		sigRng.begin = sigOffset;
		sigRng.end = pgTextSize( m_paigeRef );
		m_styleEx->ApplySignature( true, &sigRng );

		pDoc->m_Sum->SetFlagEx( MSFEX_INL_SIGNATURE );
        pgSetSelection( m_paigeRef, curSel.begin, curSel.end, 0, TRUE );
		delete [] curSig;
	}

	return true;
}

// qcprotocol: qcp_get_message
BOOL PgCompMsgView::OnSignatureChanged()
{
	return load_signature() == true;
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
	// Process the keystrokes only if the document is not read only.
	// The arrow keys are processed in OnKeyDown
	m_LastCharTypedTime = GetTickCount();

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
				if ( !PreventDestroyingSignatureStyle(false, false) )
					CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags); 
			}
		}
		else
		{
			if ( !PreventDestroyingSignatureStyle( (nChar == VK_BACK), false ) )
				CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags); 
		}
	}
}

void PgCompMsgView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    bool	bPreventDestroyingSignatureStyle = false;

	if (nChar == VK_DELETE)
		bPreventDestroyingSignatureStyle = PreventDestroyingSignatureStyle(false, true);

	if (!bPreventDestroyingSignatureStyle)
		CPaigeEdtView::OnKeyDown(nChar, nRepCnt, nFlags);
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


void PgCompMsgView::OnSetFocus(CWnd* pOldWnd) 
{
    PgMsgView::OnSetFocus(pOldWnd);

	if( UsingFullFeatureSet() && !IsReadOnly() )
		SetMoodTimer();
}


void PgCompMsgView::OnEditPaste() 
{
	COleDataObject clipboardData;

	BOOL	bAttachGood = clipboardData.AttachClipboard();
	VERIFY(bAttachGood);

	if (bAttachGood)
	{
		// Let base class get a whack at it.  If there's nothing it can
		// handle, then see if there's some file(s) we can attach.
		if (!CPaigeEdtView::OnPaste( &clipboardData, undo_ole_paste ))
		{
			if ( clipboardData.IsDataAvailable( CF_HDROP ) )
			{
				// suck out the HDROP and delegate to OnDropFiles.
				STGMEDIUM stgMedium;
				clipboardData.GetData( CF_HDROP, &stgMedium );

				CWnd* pPF = GetParentFrame();
				pPF->SendMessage( WM_DROPFILES,(WPARAM)stgMedium.hGlobal );
				SetFocus();
			}
		}
	}
}


void PgCompMsgView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if ( !m_fRO ) {
		COleDataObject clipBoardData;
		VERIFY( clipBoardData.AttachClipboard() );

		CCompMessageDoc* pDoc = (CCompMessageDoc*) GetDocument();
		ASSERT_KINDOF( CCompMessageDoc, pDoc );

        if ( pDoc->IsStationery() &&
				(clipBoardData.IsDataAvailable( CF_BITMAP, NULL)) ) {

			pCmdUI->Enable( FALSE );
			return;
		}

		if ( clipBoardData.IsDataAvailable( CF_HDROP ) ) {
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	CPaigeEdtView::OnUpdateEditPaste( pCmdUI );
}

void PgCompMsgView::OnUpdateInsertPicture(CCmdUI* pCmdUI) 
{
	CCompMessageDoc* pDoc = (CCompMessageDoc*) GetDocument();
	ASSERT_KINDOF( CCompMessageDoc, pDoc );

	if ( pDoc->IsStationery() )
		pCmdUI->Enable( FALSE );
}


/*------------------------------------------------------------------------------
//DoMoodMailCheck 
//Description : This function connects to the Text Analysis Engine(TAE) and gives data in the 
		mail to the Engine Block by Block. Then it asks for the score of mood mail and disconnects.
Input: Sesstion State
Output : Score of Mood Mail
-------------------------------------------------------------------------------------*/

int PgCompMsgView::DoMoodMailCheck(CMoodWatch *pmoodwatch)
{
	int nRetVal = -1;
	CMoodWatch moodwatch;
	if(moodwatch.Init())
	{
		CCompMessageDoc* pMsgDoc= DYNAMIC_DOWNCAST(CCompMessageDoc, GetDocument());
		if(pMsgDoc)
		{
			for (int i = 0; i < m_pHeader->MAXHEADERS; i++)
			{
				m_pHeader->AddMoodText(&moodwatch, i);				
			}
		}
		CPaigeEdtView::DoMoodMailCheck(&moodwatch);
		nRetVal =moodwatch.GetScore();
		if (nRetVal != -1)
			return nRetVal+1;
	}
	return nRetVal;
}


void PgCompMsgView::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent != AUTO_MOOD_MAIL_TIMER || !UsingFullFeatureSet())
		CPaigeEdtView::OnTimer(nIDEvent);
	else
	{
		if (IsReadOnly())
			KillMoodTimer();
		else if ( ((GetTickCount() - m_LastCharTypedTime) > g_nMoodMailInterval) 
			&& (m_bMoodMailDirty || m_pHeader->IsHeaderMoodDirty())
			&&(GetIniShort( IDS_INI_MOOD_MAIL_CHECK ))
			&&(!m_bServicingSpellMoodBPTimer)) 
		{
			m_bServicingSpellMoodBPTimer = true;
			KillMoodTimer();
			HideCaret();
			
			int nPrevScore = m_nScore;
			int nScore = DoMoodMailCheck();
			if (nScore != -1)
			{
				m_nScore = nScore;
				UpdateMoodMailButton(m_nScore);
				DrawBadWords();

				// Invalidate if we're going from no mood to a mood, or from a mood to no mood
				if ((nPrevScore == 1 && m_nScore > 1) || (nPrevScore > 1 && m_nScore == 1))
					Invalidate();

				CCompMessageDoc* pMsgDoc= DYNAMIC_DOWNCAST(CCompMessageDoc, GetDocument());
				if (pMsgDoc)
				{
					m_pHeader->SetMoodScore(m_nScore);
					m_pHeader->ValidateFields(true, false);
				}
			}

			//Flag Mood Mail Dirty
			m_bMoodMailDirty = false;
			m_pHeader->SetHeaderMoodDirty(FALSE);

			SetMoodTimer();
			ShowCaret();
			m_bServicingSpellMoodBPTimer = false;
		}
	}
}

/*06-26-2000 
//UpdateMoodMailButton 
//Description : This function update the bmp on the Mood Mail button for composing message
	depending on the score
Input: Calculated Mood Mail Score
Output : void
-------------------------------------------------------------------------------------*/

void PgCompMsgView::UpdateMoodMailButton(int nScore)
{
	CCompMessageFrame* pFrame = DYNAMIC_DOWNCAST(CCompMessageFrame, GetParentFrame());

	if (pFrame)
	{
		QCChildToolBar* pToolBar = pFrame->GetToolbar();
		//(( CCompMessageFrame* )pProtocol)->m_pToolBar->InvalidateButton(1);
		if (pToolBar)
		{
			int nIndex = pToolBar->CommandToIndex(ID_MOOD_MAIL);
			if (nIndex != -1)
			{
				//to avoid flickering of toolbar do not paint if it is the same score
				if(((CMoodMailStatic*)pToolBar->GetButton(nIndex))->GetScore() != nScore)
				{			
					((CMoodMailStatic*)pToolBar->GetButton(nIndex))->SetScore(nScore);
					pToolBar->Invalidate(nIndex);
				}
			}
		}
	}
}

/*06-26-2000 
//UpdateBPButton 
//Description : This function updates the Send/Queue button in the composing message window
	if the header fields contain any BP addresses
-------------------------------------------------------------------------------------*/

void PgCompMsgView::UpdateBPButton(bool bBPWarning)
{
	CCompMessageFrame* pFrame = DYNAMIC_DOWNCAST(CCompMessageFrame, GetParentFrame());

	if (pFrame)
	{
		QCChildToolBar* pToolBar = pFrame->GetToolbar();
		//(( CCompMessageFrame* )pProtocol)->m_pToolBar->InvalidateButton(1);
		if (pToolBar)
		{
			int nIndex = pToolBar->CommandToIndex(ID_MESSAGE_SENDIMMEDIATELY);
			if (nIndex != -1)
			{
				//to avoid flickering of toolbar do not paint if it is in the same state
				if(((TBarSendButton*)pToolBar->GetButton(nIndex))->IsBPWarning() != bBPWarning)
				{			
					((TBarSendButton*)pToolBar->GetButton(nIndex))->SetBPWarning(bBPWarning);
					pToolBar->Invalidate(nIndex);
				}
			}
		}
	}
}


/*07-17-2000 
//ShowMoodMailButton 
//Description : This function will show or hide the update button depending on the BOOL passed
Input: Status TRUE to make it visible or FALSE to make it invisible
Output : void
-------------------------------------------------------------------------------------*/


void PgCompMsgView::ShowMoodMailButton(BOOL bStatus)
{
	CCompMessageFrame* pFrame = DYNAMIC_DOWNCAST(CCompMessageFrame, GetParentFrame());

	if (pFrame)
	{
		QCChildToolBar* pToolBar = pFrame->GetToolbar();
		//(( CCompMessageFrame* )pProtocol)->m_pToolBar->InvalidateButton(1);
		if (pToolBar)
		{
			int nIndex = pToolBar->CommandToIndex(ID_MOOD_MAIL);
			if (nIndex != -1)
			{
				//to avoid flickering of toolbar do not paint if it is the same score
				UINT uStyle = pToolBar->GetButtonStyle(nIndex);
				if(bStatus)
				{
					if (uStyle & ~WS_VISIBLE)
					{
						pToolBar->SetButtonStyle(nIndex, (unsigned int)WS_VISIBLE);
						//pToolBar->Invalidate(nIndex);
				
					}
				}
				else
				{
					if (uStyle & WS_VISIBLE)
					{
						pToolBar->SetButtonStyle(nIndex, (unsigned int)~WS_VISIBLE);
						//pToolBar->Invalidate(nIndex);
					}
				}
			}
		}
	}
}


void PgCompMsgView::SetMoodTimer()
{
	// Set the mood timer if:
	// * We're using the full feature set
	// * We're not read only
	// * We're focused or the subject header is focused
	if ( UsingFullFeatureSet() && !IsReadOnly() )
	{
		CWnd *		pFocusWnd = CWnd::GetFocus();
		CWnd *		pParentOfFocusWnd = pFocusWnd ? pFocusWnd->GetParent() : NULL;
		int			nCurrentHeader = m_pHeader ? m_pHeader->GetCurrentHeader() : -1;
		bool		bSubjectHeaderHasFocus = ( (nCurrentHeader == HEADER_SUBJECT) &&
											   ((pFocusWnd == m_pHeader) || (pParentOfFocusWnd == m_pHeader)) );

		if ( (pFocusWnd == this) || bSubjectHeaderHasFocus )
			SetTimer( AUTO_MOOD_MAIL_TIMER, MOOD_INTERVAL, NULL );
	}
}


void PgCompMsgView::KillMoodTimer()
{
	KillTimer(AUTO_MOOD_MAIL_TIMER);
}
