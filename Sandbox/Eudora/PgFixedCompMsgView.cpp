// PgFixedCompMsgView.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include "QCUtils.h"	// required early else ugly stuff happens...
#include "PgFixedCompMsgView.h"
#include "PgStyleUtils.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "pghlevel.h"
#include "msgutils.h"
#include "utils.h"

#include "msgdoc.h"
#include "summary.h"
#include "font.h"
#include "compmsgd.h"
#include "resource.h"
#include "compmessageframe.h"


#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// PgFixedCompMsgView

IMPLEMENT_DYNCREATE(PgFixedCompMsgView, PgMsgView)

PgFixedCompMsgView::PgFixedCompMsgView() :
	m_SysKeyDownTime(0)
{
	m_nWordWrapColumn = 0;
}

PgFixedCompMsgView::~PgFixedCompMsgView()
{
}


BEGIN_MESSAGE_MAP(PgFixedCompMsgView, PgMsgView)
	//{{AFX_MSG_MAP(PgFixedCompMsgView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_BOLD, OnUpdateTextBold)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_ITALIC, OnUpdateTextItalic)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_UNDERLINE, OnUpdateTextUnderline)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_INDENT_OUT, OnUpdateIndent)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_INDENT_IN, OnUpdateIndent)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_CENTER, OnUpdateParaAlign)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_LEFT, OnUpdateParaAlign)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_RIGHT, OnUpdateParaAlign)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TABLE_INSERTTABLE, OnUpdateEditInsertTable)
	ON_WM_CONTEXTMENU()
	ON_WM_CHAR()
	ON_WM_SYSKEYDOWN()
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PgFixedCompMsgView drawing

/*
void PgFixedCompMsgView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}
*/

/////////////////////////////////////////////////////////////////////////////
// PgFixedCompMsgView diagnostics

#ifdef _DEBUG
void PgFixedCompMsgView::AssertValid() const
{
	PgMsgView::AssertValid();
}

void PgFixedCompMsgView::Dump(CDumpContext& dc) const
{
	PgMsgView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// PgFixedCompMsgView message handlers
BOOL PgFixedCompMsgView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	dwStyle |= (WS_VSCROLL | WS_HSCROLL);
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void PgFixedCompMsgView::OnInitialUpdate() 
{
	PgMsgView::OnInitialUpdate();
	
	ImportMessage();
	UpdateScrollBars();
	SetIniShort(IDS_INI_WORD_WRAP_MAX, GetIniShort(IDS_INI_WORD_WRAP_COLUMN));
	CCompMessageFrame* pFrame = (CCompMessageFrame*) GetParentFrame();
	m_pHeader = pFrame->GetHeaderView();
	m_bFixedWidth = TRUE;
	
	CString strName = GetIniString(IDS_INI_MESSAGE_FIXED_FONT);
	select_pair sel;

	sel.begin = 0;
	sel.end = pgTextSize(m_paigeRef);
	short fontSize = (short)(pgGetDefaultPointSize() >> 16);
	pgSetFontByName(m_paigeRef, (const pg_font_name_ptr)(LPCSTR)strName, &sel, FALSE);
	pgSetPointSize(m_paigeRef, fontSize, &sel, FALSE);
}


void PgFixedCompMsgView::ExportMessage( CMessageDoc* pMsgDoc /* = NULL */ )
{
	CMessageDoc* pDoc = NULL;

	if ( pMsgDoc )
		pDoc = pMsgDoc;
	else
		pDoc = (CMessageDoc*) GetDocument();

	VERIFY( pDoc );
	ASSERT( pDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)) );
	
	CSummary* pSum = pDoc->m_Sum;
	VERIFY( pSum );
	
	// figger out whatsup wi' dat data format!
	PgDataTranslator::FormatType txrType;

	//Only use the plain text translator
	txrType = PgDataTranslator::kFmtText;
	
	
	// do the deed
	memory_ref pgmRef;
	PgDataTranslator theTxr( m_paigeRef );
	theTxr.ExportData( &pgmRef, txrType );

	// convert the PGMM reference to a c-string, and jam it in the doc. note
	// that anything in the paige instance (headers, whatever) is gonna
	// get saved as part of the message.
	char* pMem = (char*) UseMemoryToCStr( pgmRef );
	pDoc->SetText( pMem );
	
	// get out before someone gets hurt
	UnuseAndDispose( pgmRef );
	SaveChangeState();
}


// SaveInfo:
// Just a little legacy glue...

void PgFixedCompMsgView::SaveInfo()
{
	ExportMessage();
}


//This function was overridden so that a fixed column could be maintained.  At the x'th column we want to wrap the words
//Used for telix's etc..
void PgFixedCompMsgView::SetPgAreas( /*CPaigeEdtView *view*/ )
{	
	TEXTMETRIC tm;
	CDC dc;
	HFONT hFont = (HFONT)GetMessageFixedFont().m_hObject;
	INT nCharsWidth;
	rectangle pgVisClientRect;
	rectangle pgPageClientRect;
	shape_ref visArea;
	shape_ref pageArea;
	INT nWindowWidth;
	CRect clientRect;


	//This function is also needed because of the calls to ClearStyle and ClearFormat in InsertPrintHeader.  
	//These two functions cause the paige areas to be recalculated.  We don't want this to
	//happen if we are in the process of printing because the pgAreas have already
	//been set in CPaigeEdtView::OnBeginPrinting.  Only calculate the areas if we
	//are not printing -JN

	//Not necessary, pgareas calculated in BeginPrinting now instead of OnPrint - KV
	//if(!fInsertedHeader)
	{	
		GetClientRect(&clientRect);
		nWindowWidth = clientRect.Width();

		//Create an information dc and verify it's validity
		VERIFY(dc.CreateIC("DISPLAY", NULL, NULL, NULL));
		VERIFY(::SelectObject(dc.m_hDC, hFont));

		m_nWordWrapColumn = GetIniShort(IDS_INI_WORD_WRAP_COLUMN);

		//in logical units
		dc.GetTextMetrics(&tm); 
		

		//Number of characters per line
		nCharsWidth = (tm.tmAveCharWidth * (m_nWordWrapColumn + 1)) + m_pgBorder.left;


		if(nWindowWidth > nCharsWidth)
			m_pgBorder.right = nWindowWidth - nCharsWidth;
		else
			m_pgBorder.right = 10;

		//Account for the horizontal scroll bar
		m_pgBorder.bottom = 5; 

		// don't want to fill the *entire* client area
		clientRect.DeflateRect( m_pgBorder );

		// Get the client area dimensions
		RectToRectangle(&clientRect,&pgPageClientRect);
		pageArea = pgRectToShape(PgMemGlobalsPtr(), &pgPageClientRect);

		//Visible area should always be what we can see..
		GetClientRect(&clientRect);
		RectToRectangle(&clientRect, &pgVisClientRect);
		visArea = pgRectToShape(PgMemGlobalsPtr(), &pgVisClientRect);

		if(nWindowWidth > nCharsWidth)
		{
			// If we can show all characters on the screen, set the vis and page areas to be the same
			// Reset the page area and the visible area to the current size
			pgSetAreas(m_paigeRef, pageArea,pageArea, MEM_NULL);
		}
		else 
		{
			//If we can't show all the characters, then inflate the client rect to be big enough to show all of
			//the characters.  Set the page area to be this value, and the vis area to be the client rect
			clientRect.InflateRect(0,0,((nCharsWidth - nWindowWidth) + 12), 0);
			clientRect.DeflateRect(m_pgBorder);
			
			RectToRectangle(&clientRect, &pgPageClientRect);
			
			pageArea = pgRectToShape(PgMemGlobalsPtr(), &pgPageClientRect);

			pgSetAreas(m_paigeRef, visArea, pageArea, MEM_NULL);
		}


		// Dispose of the shape bcos Paige maintains a copy
		pgDisposeShape(visArea);
		pgDisposeShape(pageArea);

		pgAdjustScrollMax(m_paigeRef, best_way);

		paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
		shape_ptr wptr = (shape_ptr) UseMemory(pgRecPtr->wrap_area);
		shape_ptr vptr = (shape_ptr) UseMemory(pgRecPtr->vis_area);
		UnuseMemory(pgRecPtr->wrap_area);
		UnuseMemory(pgRecPtr->vis_area);
		UnuseMemory(m_paigeRef);
	}
}

void PgFixedCompMsgView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	SetRedraw( FALSE );
	InsertPrintHeader();

	// call the base class
	CPaigeEdtView::OnBeginPrinting( pDC, pInfo );
}


// OnEndPrinting:
// Called by the framework at the end of a printing job.
//
// This is where ya do cleanup after the job. We need to remove the message
// header text, and turn redraw back on.

void PgFixedCompMsgView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CPaigeEdtView::OnEndPrinting( pDC, pInfo );

	// Remove the message headers
	// Done at the end to avoid redraw problems due to undo_format after printing
	RemovePrintHeader();
}


// InsertPrintHeader:
// Inserts the header text prior to printing a message
//
// This is the easiest way to get the message header printed. It causes a
// little UI ugliness, because we turn off redraw, but otherwise it works
// pretty darn good. Currently we print like v2.2 does, but it might be
// nice to fancy it up with a little character/paragraph formatting.

void PgFixedCompMsgView::InsertPrintHeader()
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
	pgSetSelection(m_paigeRef, 0, nLength - 3, 0, TRUE);	//- 3 for /n/r/n found on the end

	//Clear the style first
	ClearStyle();
	//Now Clear Formatting
	ClearFormat();
	
}


// RemovePrintHeader:
// Removes the header text created with InsertPrintHeader

void PgFixedCompMsgView::RemovePrintHeader()
{
	// this just set the selection using the range we saved-off in
	// InsertPrintHeader(), and replaces it with nothing.
	pgSetSelection(m_paigeRef, chrgHeader.cpMin, chrgHeader.cpMax, 0, TRUE);	
	pgDelete(m_paigeRef, NULL, best_way);
}

void PgFixedCompMsgView::OnUpdateEditInsertTable(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(false);	
}

void PgFixedCompMsgView::OnUpdateTextBold( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( false );
}

void PgFixedCompMsgView::OnUpdateTextItalic( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( false );
}

void PgFixedCompMsgView::OnUpdateTextUnderline( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( false );

}

void PgFixedCompMsgView::OnUpdateParaAlign( CCmdUI* pCmdUI )
{
	pCmdUI->Enable(false);
}

void PgFixedCompMsgView::OnUpdateIndent( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( false );
}

void PgFixedCompMsgView::UpdateScrollBars( bool bSetPage /* = false */ )
{
	short maxH, maxV;
	short hValue, vValue;

	if ( pgGetScrollValues(m_paigeRef, &hValue, &vValue, &maxH, &maxV ) ) {
		// OG -- I've decided that we don't want the vertical scrollbar to
		// go away; I think ;-) It actually sucks that when it does,
		// it causes the document to reformat.
		if ( maxV < 1 )	maxV = 1;
		if ( maxH < 1 )	maxH = 1;

		SetScrollRange( SB_VERT, 0, maxV, FALSE );
		SetScrollRange(SB_HORZ, 0, maxH, FALSE );
		SetScrollPos( SB_VERT, vValue, TRUE );
		SetScrollPos(SB_HORZ, hValue, TRUE );
	}

	// TODO: some kind of solution for proportional scrollbar thumbtracks. right
	// now we do nothing. Haven't been able to find any samples of how this is
	// reliably done.
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void PgFixedCompMsgView::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	CCompMessageDoc* pDoc = (CCompMessageDoc *) GetDocument();
	ASSERT_KINDOF(CCompMessageDoc, pDoc);
	pDoc->DoContextMenu(this, WPARAM(pWnd->GetSafeHwnd()), MAKELPARAM(ptScreen.x, ptScreen.y));
} 

void PgFixedCompMsgView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!m_fRO) 
	{
		CCompMessageDoc* pDoc = (CCompMessageDoc *) GetDocument();

		ASSERT_KINDOF(CCompMessageDoc, pDoc);
		CSummary *pSummary;

		pSummary = pDoc->m_Sum;

		if(nChar == VK_TAB)
		{
			if (pSummary->TabsInBody())
			{
				if(GetKeyState(VK_SHIFT) < 0)
				{
					m_pHeader->SetFocusToHeader(HEADER_ATTACHMENTS);
					((CSplitterWnd*)GetParent())->SetActivePane(0,0);
				}
				else
					CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags); 
			}
			else
			{
				int nHeader = (GetKeyState(VK_SHIFT) < 0) ? HEADER_ATTACHMENTS : HEADER_TO;
				m_pHeader->SetFocusToHeader(nHeader);
				((CSplitterWnd*)GetParent())->SetActivePane(0,0);
			}
		}
		else
			CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags); 
	}
}


LRESULT PgFixedCompMsgView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSCOMMAND:
		{
			//
			// Warning!  Kludge Alert!  This is other back end of the
			// hack set up by OnSysKeyDown().  In the case where we
			// get an accelerator key like Alt+O to set the keyboard
			// focus to a header field, we have seen that these also
			// generate WM_SYSCOMMAND/SC_KEYMENU messages that
			// eventually end up in CMainFrame.  The problem is that
			// CMainFrame's handler beeps since it doesn't recognize
			// the Alt key shortcut as fitting any of the main menu
			// items.  Therefore, this hack intercepts the
			// WM_SYSCOMMAND and "handles" it so that CMainFrame never
			// sees it.  Got all that?
			//
			// But just to be super paranoid, we want to make sure that
			// we're not improperly intercepting *all* WM_SYSCOMMAND/SC_KEYMENU
			// messages, so we check to see if this message occurred within
			// 2 seconds of the a recognized WM_SYSKEYDOWN message which
			// caused a header focus change.
			//
			//TRACE0("PgFixedCompMsgView::WindowProc(): Got WM_SYSCOMMAND\n");
			if ((wParam == SC_KEYMENU) && ((::GetCurrentTime() - m_SysKeyDownTime) < 2000))
			{
				m_SysKeyDownTime = 0;
				return 0;		// zero indicates message was processed
			}
			m_SysKeyDownTime = 0;
		}
		break;
	}

	return PgMsgView::WindowProc(message, wParam, lParam);
}


void PgFixedCompMsgView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nFlags & 0xE000) == 0x2000)
	{
		if (m_pHeader->SetFocusToHeaderWithShortcut(nChar))
		{
			TRACE0("Got Alt+key keypress in PgFixedCompMsgView::OnSysKeyDown()\n");
			((CSplitterWnd*)GetParent())->SetActivePane(0,0);

			//
			// Warning!  Kludge alert!  Grab the current wall clock
			// time so that we can later detect whether or not we
			// want to trap a subsequent, expected WM_SYSCOMMAND message.
			// See WindowProc() for the other half of the kludge.
			//
			m_SysKeyDownTime = ::GetCurrentTime();
			return;
		}
	}
	
	PgMsgView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}



