// PaigeEdtView.cpp : implementation of the CPaigeEdtView class
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include <afxodlgs.h>		// For Paste Special

#include "resource.h"
#include "guiutils.h"
#include "utils.h"

#include "rs.h"
#include "MoodMailStatic.h"
//#include "PaigeEdtDoc.h"

// For all text functions
#include "pgtraps.h"
#include "pgscrap.h"

// For Import, Export of files in Text, RTF, Native Paige
// and HTML formats
#include "pgTxrCPP.h"

// paige utility routines
#include "pgutils.h"

// high-level style interface
#include "pghlevel.h"

// Paige html definitions
#include "pghtmdef.h"

// Paige hyperlinks
#include "pghtext.h"

// machine specific
#include "machine.h"
#include "pgosutl.h"

// Import/Export == included in PaigeEdtView.h
#include "Paige_io.h"

//#include "pgtables.h"

// eudora font info
#include "font.h"
#include "FaceNameDialog.h"

// line glitter functions
#include "defprocs.h"
#include "pgdefpar.h"
#include "pggrafx.h"

#include "spell.h"
#include "PaigeEdtView.h"

#include "PgStyleUtils.h"
#include "spellpaige.h"
#include "paige.h"
#include "PGEDIT.H"
#include "PGSELECT.H"

#include "QCRecipientDirector.h"

#include "address.h"
#include "PaigePreviewDC.h"

#include "PaigeStyle.h"
#include "pgStyleUtils.h"
#include "msgutils.h"
#include "COLORLIS.H"
#include "SafetyPal.h"
#include "pgStyleUtils.h"
#include "PgEmbeddedImage.h"
#include "InsertLinkDialog.h"
#include "PictureLinkDlg.h"
#include "Text2Html.h"
#include "mainfrm.h"
#include "QCCommandStack.h"
#include "QCSharewareManager.h"
#include "CompMessageFrame.h"
#include "Summary.h"
#include "TocDoc.h"
#include "TocFrame.h"
#include "msgdoc.h"

#include "wininet.h"
#include "PrintPreviewView.h"
#include "URLUtils.h"

#include "PopupText.h"
#include "EmoticonManager.h"

// BOGOLE:
#include "OleDoc.h"
#include "PgCntrItem.h"
#include "ObjectSpec.h"
#define embed_type(p) (p->type & EMBED_TYPE_MASK)

#include "DebugNewHelpers.h"

void FindWord(pg_ref pg, long offset, long PG_FAR *first_byte,
              long PG_FAR *last_byte, pg_boolean left_side, pg_boolean smart_select);
long FindNextWhitespace(pg_ref pg, long offset, long end);
long SkipWhiteSpaces(pg_ref pg, long offset, long end);
long FindValidURLStart(pg_ref pg, long offset, long end, char &startChar);
long FindValidURLEnd(pg_ref pg, long offset, long end, long begin, char startChar,BOOL& bFound );
//theStr is a IN-OUT parameter. If the scheme is www. or ftp. then 
//http:// or ftp:// are appended to the beginning of the string
BOOL ValidURLScheme(char * theStr);
bool InDifferentWord(pg_ref paigeRef, select_pair_ptr oldSel);
bool GetWordContainingSelection(pg_ref paigeRef,select_pair_ptr theWord);
bool SelPairIntersects(select_pair_ptr sel1, select_pair_ptr sel2);
int	 DoMoodMailCheck( pg_ref paigeRef);//will return the score of the Mood Mail

static  graf_device m_device;


// OG
#define BORDER_LEFT		8
#define BORDER_TOP		2
#define BORDER_RIGHT	8
#define BORDER_BOTTOM	2

#define MOUSE_DOWN 0x1
#define MOUSE_TRIPLE_CLICKED 0x2
#define MOUSE_DRAG_SELECT 0x4
#define MOUSE_DRAG_DROP 0x8
#define MOUSE_FORMAT_PAINTER 0x10

BOOL g_bInteractiveSpellCheck = FALSE;
BOOL g_bMoodMailCheck = FALSE;
DWORD g_nMoodMailInterval = 0;

static const int kMaxCharsInContextSearchSelection = 120;

extern QCCommandStack			g_theCommandStack;
extern QCRecipientDirector		g_theRecipientDirector;
extern EmoticonDirector			g_theEmoticonDirector;

extern bool g_qtInitialized;
extern bool g_bOverwriteMode;

//	Statics
long				CPaigeEdtView::s_nLastCheckedHyperlinkID = 0;
long				CPaigeEdtView::s_nCurrentWarningHyperlinkID = 0;


// EmoticonInText keeps track of where we found emoticons in the text
class EmoticonInText
{
  public:
					EmoticonInText(
						long				in_nEmoticonOffset,
						Emoticon *			in_pEmoticon)
						:	m_nEmoticonOffset(in_nEmoticonOffset), m_pEmoticon(in_pEmoticon) {}
					~EmoticonInText() {}

	//	Accessors
	long			GetEmoticonOffset() { return m_nEmoticonOffset; }
	Emoticon *		GetEmoticon() { return m_pEmoticon; }

  protected:
	long			m_nEmoticonOffset;
	Emoticon *		m_pEmoticon;
};


typedef EmoticonsInTextList::iterator		EmoticonsInTextIterator;

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView

IMPLEMENT_DYNCREATE(CPaigeEdtView, CView)

BEGIN_MESSAGE_MAP(CPaigeEdtView, CView)
    //{{AFX_MSG_MAP(CPaigeEdtView)
    ON_WM_SETCURSOR()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
//      ON_MESSAGE(WM_GETFONT,OnGetFont)
//      ON_MESSAGE(WM_SETFONT,OnSetFont)
    ON_WM_SYSCOLORCHANGE()
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
    ON_WM_MOUSEWHEEL()
    ON_COMMAND(ID_EDIT_TEXT_FONT, OnFormatFont)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_FONT, OnUpdateFormatFont)
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_PASTE_SPECIAL, OnEditPasteSpecial)
    ON_COMMAND(WM_UNDO, OnEditUndo)
    ON_COMMAND(WM_CUT, OnEditCut)
    ON_COMMAND(WM_COPY, OnEditCopy)
    ON_COMMAND(WM_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(WM_UNDO, OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(WM_CUT, OnUpdateEditCut)
    ON_UPDATE_COMMAND_UI(WM_COPY, OnUpdateEditCopy)
    ON_UPDATE_COMMAND_UI(WM_PASTE, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_PASTEASQUOTATION, OnEditPasteAsQuotation)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEASQUOTATION, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_TEXT_BOLD, OnTextBold)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_BOLD, OnUpdateTextBold)
    ON_COMMAND(ID_EDIT_TEXT_ITALIC, OnTextItalic)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_ITALIC, OnUpdateTextItalic)
    ON_COMMAND(ID_EDIT_TEXT_UNDERLINE, OnTextUnderline)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_UNDERLINE, OnUpdateTextUnderline)
    ON_COMMAND(ID_EDIT_TEXT_STRIKEOUT, OnTextStrikeout)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_STRIKEOUT, OnUpdateTextStrikeout)
    ON_COMMAND(ID_EDIT_TEXT_PLAIN, OnTextPlain)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_PLAIN, OnUpdateTextPlain)
    ON_COMMAND(ID_EDIT_TEXT_INDENT_IN, OnIndentIn)
    ON_COMMAND(ID_EDIT_TEXT_INDENT_OUT, OnIndentOut)
    ON_COMMAND(ID_EDIT_TEXT_MARGINS_NORMAL, OnMarginsNormal)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_INDENT_IN, OnUpdateIndentCommand)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_INDENT_OUT, OnUpdateIndentCommand)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_MARGINS_NORMAL, OnUpdateIndentCommand)
    ON_COMMAND(ID_EDIT_TEXT_CENTER, OnParaCenter)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_CENTER, OnUpdateParaCenter)
    ON_COMMAND(ID_EDIT_TEXT_LEFT, OnParaLeft)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_LEFT, OnUpdateParaLeft)
    ON_COMMAND(ID_EDIT_TEXT_LAST_TEXT_COLOR, OnLastTextColor)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_LAST_TEXT_COLOR, OnUpdateLastTextColor)
    ON_COMMAND(ID_EDIT_TEXT_RIGHT, OnParaRight)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_RIGHT, OnUpdateParaRight)
    ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
    ON_COMMAND( ID_SPECIAL_ADDASRECIPIENT, OnSpecialAddAsRecipient )
    ON_UPDATE_COMMAND_UI( ID_SPECIAL_ADDASRECIPIENT, OnUpdateAddAsRecipient )
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT, OnUpdateEditStyle)
    ON_COMMAND(ID_EDIT_INSERT_PICTURE, OnInsertPicture)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_PICTURE, OnUpdateInsertPicture)
    ON_COMMAND(ID_EDIT_INSERT_PICTURE_LINK, OnInsertPictureLink)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_PICTURE_LINK, OnUpdateInsertPicture)
    ON_COMMAND(ID_EDIT_INSERT_HR, OnEditInsertHR)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_HR, OnUpdateEditInsertHR)
    ON_COMMAND(ID_EDIT_WRAPSELECTION, OnEditWrapSelection)
    ON_UPDATE_COMMAND_UI(ID_EDIT_WRAPSELECTION, OnUpdateNeedSelEdit)
    ON_COMMAND(ID_EDIT_TEXT_TT, OnBlkfmtFixedwidth)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_TT, OnUpdateBlkfmtFixedwidth)
    ON_COMMAND(ID_FILE_OPEN_ATTACHMENT, OnOpenAttachment)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_ATTACHMENT, OnUpdateOpenAttachment)
    ON_COMMAND(ID_EDIT_TEXT_ADD_QUOTE, OnAddQuote)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_ADD_QUOTE, OnUpdateQuoteCommand)
    ON_COMMAND(ID_EDIT_TEXT_REMOVE_QUOTE, OnRemoveQuote)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_REMOVE_QUOTE, OnUpdateQuoteCommand)
    ON_COMMAND(ID_SEARCH_WEB_FOR_SEL, OnSearchWebForSelection)
    ON_UPDATE_COMMAND_UI(ID_SEARCH_WEB_FOR_SEL, OnUpdateSearchWebForSelection)
    ON_COMMAND(ID_SEARCH_EUDORA_FOR_SEL, OnSearchEudoraForSelection)
    ON_UPDATE_COMMAND_UI(ID_SEARCH_EUDORA_FOR_SEL, OnUpdateSearchEudoraForSelection)
    ON_COMMAND(ID_SEARCH_MAILBOX_FOR_SEL, OnSearchMailboxForSelection)
    ON_UPDATE_COMMAND_UI(ID_SEARCH_MAILBOX_FOR_SEL, OnUpdateSearchMailboxForSelection)
    ON_COMMAND(ID_SEARCH_MAILFOLDER_FOR_SEL, OnSearchMailfolderForSelection)
    ON_UPDATE_COMMAND_UI(ID_SEARCH_MAILFOLDER_FOR_SEL, OnUpdateSearchMailfolderForSelection)
    //}}AFX_MSG_MAP

    ON_CBN_SELENDOK( IDC_FONT_COMBO, OnFontChange )
    ON_UPDATE_COMMAND_UI( IDC_FONT_COMBO, OnUpdateFontCombo )
	ON_COMMAND( ID_EDIT_TEXT_FORMAT_PAINTER, OnFormatPainter )
	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_FORMAT_PAINTER, OnUpdateFormatPainter )
    ON_COMMAND( ID_EDIT_TEXT_CLEAR, OnClearFormatting )
    ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_CLEAR, OnUpdateClearFormatting )
    ON_COMMAND_RANGE(ID_EDIT_TEXT_SIZE_VERY_SMALL, ID_EDIT_TEXT_SIZE_HUMONGOUS, OnTextSizeChange)
    ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_TEXT_SIZE_VERY_SMALL, ID_EDIT_TEXT_SIZE_HUMONGOUS, OnUpdateEditStyle)
    ON_COMMAND_RANGE(ID_COLOR0, ID_COLOR16, OnColorChange)
    ON_UPDATE_COMMAND_UI_RANGE(ID_COLOR0, ID_COLOR16, OnUpdateEditStyle)

    ON_COMMAND_EX_RANGE(QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand)
    ON_UPDATE_COMMAND_UI_RANGE(QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand)

    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE, OnUpdateEditStyle)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_COLOR, OnUpdateEditStyle)
    ON_COMMAND(ID_EDIT_INSERT_LINK, OnEditInsertLink)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_LINK, OnUpdateInsertLink)
    ON_COMMAND(ID_BLKFMT_BULLETTED_LIST, OnBlkfmtBullettedList)
    ON_UPDATE_COMMAND_UI(ID_BLKFMT_BULLETTED_LIST, OnUpdateBlkfmtBullettedList)
	ON_UPDATE_COMMAND_UI(ID_EDIT_EMOTICON, OnUpdateEditStyle)
	ON_UPDATE_COMMAND_UI(ID_EDIT_LAST_EMOTICON, OnUpdateLastEmoticon)
	ON_COMMAND(ID_EDIT_LAST_EMOTICON, OnLastEmoticon)
	ON_REGISTERED_MESSAGE(umsgInvalidateImage, OnUpdateImage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

    PG_PASCAL (void) myPostPaginateProc(paige_rec_ptr pg, text_block_ptr block,
                                        smart_update_ptr update_info, long lineshift_begin, long lineshift_end,
                                        short action_taken_verb)
{
    pgPostPaginateProc(pg, block,update_info, lineshift_begin, lineshift_end,
                       action_taken_verb);

    if ( (action_taken_verb == paginated_line_shift) ||
         (action_taken_verb == paginated_full_block) )
    {
        PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;

        if((pSB == NULL) || !pSB->pWndOwner->IsKindOf(RUNTIME_CLASS(CPaigeEdtView)))
        {
            ASSERT(TRUE);
            return;
        }
        //((CPaigeEdtView*)pSB->pWndOwner)->UpdateScrollBars();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView construction/destruction

CPaigeEdtView::CPaigeEdtView()
	:	m_bHasBeenInitiallyDrawn(false)
{
	m_bDidAFind = FALSE;
    m_paigeRef = NULL;
    m_styleEx = NULL;
    m_mouseState = 0;
    m_pgModifiers = 0;
    m_mousePt = 0;
    m_bDontDeactivateHighlight=FALSE;
    //undoCount = redoCount = 0;
    m_fRO = false;
    m_bAllowStyled = true;
	m_bAllowTables = false;

    m_chngCount = m_ccLastSaved = 0;
    m_bIncludeHeaderFooters     = GetIniShort(IDS_INI_PRINT_HEADERS);
    //memset( &m_thePrintInfo, 0, sizeof( m_thePrintInfo ) );
    m_thePrintInfo.m_printPgRef = 0;
    m_bFixedWidth = FALSE;
    bHasBeenInitiallyUpdated = FALSE;
    m_prevPoint.x = m_prevPoint.y = 0;
    m_pDirtyPair = NULL;
    m_pMoodDirtyPair = NULL;
    last_typed = 0;
    m_checkdastuff = false;
	m_bMoodMailDirty = true;
	m_pABOrigView = NULL;
	m_bServicingSpellMoodBPTimer = 0;

	m_bCopiedStyleInfo = false;
	m_bCopiedParInfo = false;
}

CPaigeEdtView::~CPaigeEdtView()
{       

    // delete the header font
    if(m_thePrintInfo.m_pHeaderFont)
        delete m_thePrintInfo.m_pHeaderFont;
    m_thePrintInfo.m_pHeaderFont = NULL;

    // delete the pen
    if(m_thePrintInfo.m_pPen)
        delete m_thePrintInfo.m_pPen;
    m_thePrintInfo.m_pPen = NULL;
    delete m_styleEx;
}

BOOL CPaigeEdtView::PreCreateWindow(CREATESTRUCT& cs)
{
	static char* className = "EudoraEdit";
	static bool registered = false;

	if ( !registered ) {
		WNDCLASS wndclass;

		wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = AfxWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = AfxGetInstanceHandle();
		wndclass.hIcon = AfxGetApp()->LoadIcon(AFX_IDI_STD_FRAME);
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = className;

		if( AfxRegisterClass( &wndclass ) )
			registered = true;
		else {
			TRACE("Window class \"EudoraEdit\" could not be registered.\n");
			return FALSE;
		}
	}

	cs.lpszClass = className;
    return CView::PreCreateWindow(cs);
}

LRESULT CPaigeEdtView::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT result;
	if ( !EditWindowProc( message, wParam, lParam, &result ) )
		result = CView::WindowProc( message, wParam, lParam );

	return result;
}


BOOL CPaigeEdtView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( (nCode == BN_CLICKED) && (nID != ID_EDIT_TEXT_FORMAT_PAINTER) && (pExtra == NULL) )
	{
		//	Cancel Format Painter when any other command is performed
		m_mouseState &= ~MOUSE_FORMAT_PAINTER;
	}
	
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


///////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView drawing

void CPaigeEdtView::OnDraw(CDC* pDC)
{
	// BOG: if you don't give paige a vis region to draw in, it will simply use
	// the one returned by "pgGetVisArea," which is almost the entire view.
	// fyi, paige uses a different dc than the one passed in here.

	// BOG: this could be optimized further by keeping the shape refs around
	// between calls. as it is, this is probably doing a rather large number
	// of small allocations.

	RECT rc;
    rectangle clipBox;
	pDC->GetClipBox( &rc );
    RectToRectangle( &rc, &clipBox );

	shape_ref clipRgn, visRgn, drawRgn;
	clipRgn = pgRectToShape( PgMemGlobalsPtr(), &clipBox );
	drawRgn = pgRectToShape( PgMemGlobalsPtr(), NULL );
	visRgn  = pgGetVisArea( m_paigeRef );

	// the intersection of the view's clipBox and paige's "vis" area
	pgSectShape( clipRgn, visRgn, drawRgn );

	// BOG: paige doesn't handle selections properly when "full drag" is
	// enabled, because full drag is a kludge hack---so here's one
	// more: we erase the part of the "page" area that intersects the drawRgn
	// and the selection, causing paige to completely redraw it.

    if ( pgNumSelections( m_paigeRef ) ) {
		short fbState, permState;
		pgGetHiliteStates( m_paigeRef, &fbState, &permState );

		// only do this if selections are not hidden
		if ( fbState != deactivate_verb && permState != deactivate_verb ) {

			// this call allocates an empty shape; no point in checking the
			// return, paige will have already crashed.
			shape_ref hiliteRgn, kludgeRgn;
			hiliteRgn = pgRectToShape( PgMemGlobalsPtr(), NULL );
			kludgeRgn = pgRectToShape( PgMemGlobalsPtr(), NULL );

			if ( pgGetHiliteRgn( m_paigeRef, NULL, MEM_NULL, hiliteRgn ) ) {
				pgSectShape( drawRgn, hiliteRgn, kludgeRgn );
				pgErasePageArea( m_paigeRef, kludgeRgn );
			}

			pgDisposeShape( hiliteRgn );
			pgDisposeShape( kludgeRgn );
		}
    }

	// this file has 7866 lines in it, try commenting this one!
    pgDisplay( m_paigeRef, NULL, drawRgn, MEM_NULL, NULL, bits_emulate_or );

	// we do not dispose of paige's vis area
	pgDisposeShape( clipRgn );
	pgDisposeShape( drawRgn );

	m_bHasBeenInitiallyDrawn = true;
}


/*
LRESULT CPaigeEdtView::OnGetFont(WPARAM wParam, LPARAM lParam)
{
        long                                    style_index;
        style_info                              style;
        par_info                                par;
        font_info                               font;
        if ( !(style_index = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
                return NULL;

        pgGetNamedStyleInfo(m_paigeRef, style_index, &style, &font, &par);      

        return  (long)&MessageFont.m_hObject;
        

}

LRESULT CPaigeEdtView::OnSetFont(WPARAM wParam, LPARAM lParam)
{
        short styleID;
        if ( !(styleID = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
                return TRUE;

        // Set the body style to have the MessageFont
        LOGFONT lf;
        font_info fontInfo;
        style_info styleInfo;
        style_info styleInfoMask;
        par_info parInfo;
        par_info parMask;
        if( GetIniShort( IDS_INI_USE_PROPORTIONAL_AS_DEFAULT ) )
                MessageFont.GetLogFont( &lf );
        else
                MessageFixedFont.GetLogFont( &lf );
        
        PgConvertLogFont( m_paigeRef, PgGlobalsPtr(), &lf, &fontInfo, &styleInfo, &styleInfoMask );
        pgChangeStyle(m_paigeRef, styleID, &styleInfo, &fontInfo, best_way);

        return TRUE;
}

*/
void CPaigeEdtView::OnSysColorChange()
{
    style_info styleInfo;
    //
    // Reset the style sheet for the body
    //  
    long styleID;
    if ( !(styleID = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
        return;

    pgFillBlock( &styleInfo, sizeof(style_info), 0 );
    pgGetStyle(m_paigeRef, (short)styleID, &styleInfo);

    // set default paige text color
    DWORD fgColor = GetSysColor( COLOR_WINDOWTEXT );
    pgOSToPgColor( &fgColor, &styleInfo.fg_color );

    DWORD bkColor = GetSysColor( COLOR_WINDOW );
    pgOSToPgColor( &bkColor, &styleInfo.bk_color );
        
    pgChangeStyle(m_paigeRef, (short)styleID, &styleInfo, NULL /*&fontInfo*/, best_way);
    pgSetDefaultStyle(m_paigeRef, &styleInfo, best_way);
    //
    // Modify the pgRec
    //
    paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
    pgOSToPgColor( &bkColor, &(pgRecPtr->bk_color));
    UnuseMemory(m_paigeRef);
    // 
    // Modify the Globals.  This happens for every open paige view
    // which is bad. Maybe later, centratlize this call by intercepting
    // it in Mainfrm and doing it once for all open paige views.
    //
    styleInfo = PgGlobalsPtr()->def_style;
    pgOSToPgColor( &fgColor, &styleInfo.fg_color );
    pgOSToPgColor( &bkColor, &styleInfo.bk_color );
        
    pgOSToPgColor( &bkColor, &(PgGlobalsPtr()->def_bk_color));
    memcpy( &(PgGlobalsPtr())->def_style, &styleInfo, sizeof(style_info) );
}

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView printing

BOOL CPaigeEdtView::OnPreparePrinting(CPrintInfo* pInfo)
{
    //If a selection exists, enable the Selection radio button in the
    //Print dialog
	if ( pgNumSelections(m_paigeRef) && !(pInfo->m_bPreview) )
    {
        pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;
    }

    CDC *pDC =  GetDC( );
    UINT        uOldMappingMode;   
        
    // save the mapping mode
    uOldMappingMode = pDC->SetMapMode( MM_TEXT );

    // get the x & y resolutions
    m_thePrintInfo.m_ulXScreenRes = pDC->GetDeviceCaps( LOGPIXELSX );
    m_thePrintInfo.m_ulYScreenRes = pDC->GetDeviceCaps( LOGPIXELSY );

    pDC->SetMapMode(uOldMappingMode);
    ReleaseDC(pDC);

        
    return DoPreparePrinting(pInfo);
}

BOOL CPaigeEdtView::QuietPrint()
{
    // get default print info
    CPrintInfo printInfo;
    BOOL bError = FALSE;

    ASSERT(printInfo.m_pPD != NULL);    // must be set


    printInfo.m_bDirect = TRUE;

    if (OnPreparePrinting(&printInfo))
    {
        // hDC must be set (did you remember to call DoPreparePrinting?)
        ASSERT(printInfo.m_pPD->m_pd.hDC != NULL);

                // set up document info and start the document printing process
        CString strTitle;
        CDocument* pDoc = GetDocument();
        if (pDoc != NULL)
            strTitle = pDoc->GetTitle();
        else
            GetParentFrame()->GetWindowText(strTitle);
        
        if (strTitle.GetLength() > 31)
            strTitle.ReleaseBuffer(31);
                
        DOCINFO docInfo;
        memset(&docInfo, 0, sizeof(DOCINFO));
        docInfo.cbSize = sizeof(DOCINFO);
        docInfo.lpszDocName = strTitle;
                
        CString strPortName;
                
        docInfo.lpszOutput = NULL;
        strPortName = printInfo.m_pPD->GetPortName();
                
        // setup the printing DC
        CDC dcPrint;
        dcPrint.Attach(printInfo.m_pPD->m_pd.hDC);  // attach printer dc
        dcPrint.m_bPrinting = TRUE;
        OnBeginPrinting(&dcPrint, &printInfo);
                
                // start document printing process
        if (dcPrint.StartDoc(&docInfo) == SP_ERROR)
        {
            // enable main window before proceeding
            AfxGetMainWnd()->EnableWindow(TRUE);

                        // cleanup and show error message
            OnEndPrinting(&dcPrint, &printInfo);
            dcPrint.Detach();   // will be cleaned up by CPrintInfo destructor
            AfxMessageBox(AFX_IDP_FAILED_TO_START_PRINT);
            return FALSE;
        }

        // Guarantee values are in the valid range
        UINT nEndPage = printInfo.GetToPage();
        UINT nStartPage = printInfo.GetFromPage();

        if (nEndPage < printInfo.GetMinPage())
            nEndPage = printInfo.GetMinPage();
        if (nEndPage > printInfo.GetMaxPage())
            nEndPage = printInfo.GetMaxPage();

        if (nStartPage < printInfo.GetMinPage())
            nStartPage = printInfo.GetMinPage();
        if (nStartPage > printInfo.GetMaxPage())
            nStartPage = printInfo.GetMaxPage();

        int nStep = (nEndPage >= nStartPage) ? 1 : -1;
        nEndPage = (nEndPage == 0xffff) ? 0xffff : nEndPage + nStep;


        for (printInfo.m_nCurPage = nStartPage;
             printInfo.m_nCurPage != nEndPage; printInfo.m_nCurPage += nStep)
        {
            OnPrepareDC(&dcPrint, &printInfo);

            // check for end of print
            if (!printInfo.m_bContinuePrinting)
                break;

            // set up drawing rect to entire page (in logical coordinates)
            printInfo.m_rectDraw.SetRect(0, 0,
                                         dcPrint.GetDeviceCaps(HORZRES),
                                         dcPrint.GetDeviceCaps(VERTRES));
            dcPrint.DPtoLP(&printInfo.m_rectDraw);

            // attempt to start the current page
            if (dcPrint.StartPage() < 0)
            {
                bError = TRUE;
                break;
            }

            // must call OnPrepareDC on newer versions of Windows because
            // StartPage now resets the device attributes.
            OnPrepareDC(&dcPrint, &printInfo);

            ASSERT(printInfo.m_bContinuePrinting);

            // page successfully started, so now render the page
            OnPrint(&dcPrint, &printInfo);
            if (dcPrint.EndPage() < 0 )
            {
                bError = TRUE;
                break;
            }
        }

        // cleanup document printing process
        if (!bError)
            dcPrint.EndDoc();
        else
            dcPrint.AbortDoc();

        OnEndPrinting(&dcPrint, &printInfo);    // clean up after printing
                
        dcPrint.Detach();   // will be cleaned up by CPrintInfo destructor

    }
    return !bError;
}

//OnBeginPrinting calcualtes all of the rectangles for printing and max num of pages
void CPaigeEdtView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    TEXTMETRIC  theTM;
    CFont*              pOldFont = NULL;
    ULONG               ulXRes;
    ULONG               ulYRes;
    ULONG               ulPageWidthPixels;
    ULONG               ulPageHeightPixels;
    ULONG               ulOneHalfInchX;
    ULONG               ulOneHalfInchY;
    ULONG               ulThreeQuartersInchX;
    ULONG               ulThreeQuartersInchY;
    rectangle   page_rect;

    SetRedraw( FALSE );

    // call the base class
    CView::OnBeginPrinting(pDC,pInfo); 

    // NOTE:  Paige prints the document with the current font of the document (wsywig).  Unfortunately, Eudora
    // has seperate fonts for messages and for printing which means the font must be changed in the doc to the printer
    // font and then switched back....yuk :0

    HDC hDC = 0;
    hDC = pDC->GetSafeHdc();

    //Set the print device for the original pg_ref only if we are printing the whole doc
    //For printing a selection, we copy the highlighted text into a temp pg_ref and print that.
    if (! (pInfo->m_pPD->m_pd.Flags & PD_SELECTION) )
        pgSetPrintDevice(m_paigeRef, (generic_var)hDC);

    //Incase we print the selection, this is the temp pg_ref we will use
    m_thePrintInfo.m_printPgRef = 0;

    CString msgFont;
    if( GetIniShort( IDS_INI_USE_PROPORTIONAL_AS_DEFAULT ) )
        msgFont = GetIniString(IDS_INI_MESSAGE_FONT);
    else
        msgFont = GetIniString(IDS_INI_MESSAGE_FIXED_FONT);
    FontSizeMapType& FontSizeMap = GetFontSizeMap();
    int msgFontSize = FontSizeMap[(int)GetIniShort(IDS_INI_MESSAGE_FONT_SIZE)][2];

    //Set the font to the printer font and size
    CString strName = GetIniString(IDS_INI_PRINTER_FONT);
    int fontSize = FontSizeMap[(int)GetIniShort(IDS_INI_PF_SIZE)][2];

    BOOL fontsDiff = (stricmp(msgFont, strName) == 0) ? FALSE : TRUE;
    BOOL sizesDiff = msgFontSize == fontSize ? FALSE : TRUE;

    pgGetSelection(m_paigeRef, &m_thePrintInfo.m_currSelection.begin, &m_thePrintInfo.m_currSelection.end);

    if (fontsDiff || sizesDiff)
    {
        long styleID;
        styleID = pgGetNamedStyleIndex (m_paigeRef, body_style);
        ASSERT(styleID);

        style_info printerStyle;

        pgFillBlock( &printerStyle, sizeof(style_info), 0 );

        pgGetStyle(m_paigeRef, (short)styleID, &printerStyle);
        printerStyle.point = fontSize;

        font_info printerFont;
        if (fontsDiff)
        {
            //font_info printerFont;
            style_info styleInfo, styleInfoMask;
                        
            pgFillBlock( &printerFont, sizeof(font_info), 0 );
                        
            extern CFontInfo PrinterFont;
            LOGFONT lf;
            PrinterFont.GetLogFont(&lf);
            PgConvertLogFont(m_paigeRef, PgGlobalsPtr(), &lf, &printerFont, &styleInfo, &styleInfoMask );

            if ( lf.lfWeight == FW_BOLD )
                printerStyle.styles[bold_var] = -1;
            if ( lf.lfItalic )
                printerStyle.styles[italic_var] = -1;
            if ( lf.lfUnderline )
                printerStyle.styles[underline_var] = -1;
            if ( lf.lfStrikeOut )
                printerStyle.styles[strikeout_var] = -1;
        }

        if (sizesDiff)
        {
            //Change the relative point sizes first
            select_pair sel;
            int currSize, newSize;
            int msgBase   = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);
            int printBase = GetIniShort(IDS_INI_PF_SIZE);
			long start    = 0;
			long end      = 0;

            if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
            {
                start = m_thePrintInfo.m_currSelection.begin;
                end   = m_thePrintInfo.m_currSelection.end;
            }

            style_walk walker;
            paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
            pgPrepareStyleWalk(pgRecPtr, start, &walker, false);

			if ( !end )
				end = walker.t_length;

			for ( bool bDoneWalking = false;
					!bDoneWalking && (walker.current_offset < end); /*noop*/ )
			{
				sel.begin = walker.current_offset;
                currSize = walker.cur_style->point;

                for ( ;; )
				{
					if ( pgWalkNextStyle( &walker ) )
					{
						if ( walker.cur_style->point != currSize )
						{
							// we got a style change with a different point size. as long as the
							// current offset isn't past the end of a selection, it ends this
							// run of same-sized text---otherwise just do the selection.

							sel.end = (walker.current_offset < end) ? walker.current_offset : end;
							break;
						}
					}
					else
					{
						// there are no styles left to walk, "end" points at end of document
						// or end of selection---it will *never* exceed t_length.
						assert( end <= walker.t_length );

						sel.end = end;
						bDoneWalking = true;
						break;
					}
				}

                m_thePrintInfo.m_offsetsQueue.push(sel);

				// Find the font index and set the relative point size for the new index.
				// The order may seem a little strange, but it's a way to prefer
				// that matches occur closer toward the default font size.

				if (currSize == FontSizeMap[msgBase][2])
					newSize = FontSizeMap[printBase][2];
				else if (currSize == FontSizeMap[msgBase][1])
					newSize = FontSizeMap[printBase][1];
				else if (currSize == FontSizeMap[msgBase][0])
					newSize = FontSizeMap[printBase][0];
				else if (currSize == FontSizeMap[msgBase][3])
					newSize = FontSizeMap[printBase][3];
				else if (currSize == FontSizeMap[msgBase][4])
					newSize = FontSizeMap[printBase][4];
				else if (currSize == FontSizeMap[msgBase][5])
					newSize = FontSizeMap[printBase][5];
				else if (currSize == FontSizeMap[msgBase][6])
					newSize = FontSizeMap[printBase][6];
				else
				{
					// This happens with the strange case where specifying 11 point really
					// generates 12 point, so we don't wind up finding a match in the font
					// size map.  Just assume the default size for lack of a better choice.

					ASSERT(0);
					newSize = FontSizeMap[printBase][2];
				}

                m_thePrintInfo.m_fontSizeQueue.push(newSize);
            }

            pgPrepareStyleWalk(pgRecPtr, 0, NULL, false);
            UnuseMemory(m_paigeRef);

            //Set up the undo stack
            int count = m_thePrintInfo.m_fontSizeQueue.size();
            for (int i = 0; i < count; i++)
            {
                sel = m_thePrintInfo.m_offsetsQueue.front();
                m_thePrintInfo.m_offsetsQueue.pop();
                pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, FALSE);
                m_thePrintInfo.m_printUndoStack.push( pgPrepareUndo(m_paigeRef, undo_format, NULL) );
                pgSetPointSize( m_paigeRef, (short)(m_thePrintInfo.m_fontSizeQueue.front() >> 16), &sel, FALSE );
                m_thePrintInfo.m_fontSizeQueue.pop();
            }
        }

        pgSetSelection(m_paigeRef, m_thePrintInfo.m_currSelection.begin, m_thePrintInfo.m_currSelection.end, 0, FALSE);
        m_thePrintInfo.m_printUndoStack.push(pgPrepareUndo(m_paigeRef, undo_format, (void PG_FAR *)MEM_NULL));
        if (fontsDiff)
        {
            pgChangeStyle(m_paigeRef, (short)styleID, &printerStyle, &printerFont, draw_none);
        }
        else
        {
            //Only the point size is different
            pgChangeStyle(m_paigeRef, (short)styleID, &printerStyle, MEM_NULL, draw_none);
        }
    }

    if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
    {
        //Copy the text into a temporary pg_ref for printing
        m_thePrintInfo.m_printPgRef = pgCopy(m_paigeRef, &(m_thePrintInfo.m_currSelection));
        //Set the print device for the temporary pg_ref
        pgSetPrintDevice(m_thePrintInfo.m_printPgRef, (generic_var)hDC);
    }
        
    // save the mapping mode
    m_thePrintInfo.m_uOldMappingMode = pDC->SetMapMode( MM_TEXT );

    // get the x & y resolutions
    ulXRes = pDC->GetDeviceCaps( LOGPIXELSX );
    ulYRes = pDC->GetDeviceCaps( LOGPIXELSY );

    // get the size of the page in pixels
    ulPageWidthPixels = pDC->GetDeviceCaps( HORZRES );
    ulPageHeightPixels = pDC->GetDeviceCaps( VERTRES );
        
    // save some handy values
    ulOneHalfInchX = ( ulXRes / 2 );
    ulOneHalfInchY = ( ulYRes / 2 );
    ulThreeQuartersInchX = ( ulXRes * 3UL ) / 4;
    ulThreeQuartersInchY = ( ulYRes * 3UL ) / 4;
                                         
    // get the body rect
    m_thePrintInfo.m_theBodyRect.left = ( int ) ulThreeQuartersInchX;
    m_thePrintInfo.m_theBodyRect.right = ( int ) ( ulPageWidthPixels - ulThreeQuartersInchX );
        
    if ( !m_bIncludeHeaderFooters || (pInfo->m_pPD->m_pd.Flags & PD_SELECTION) )
    {
        m_thePrintInfo.m_theBodyRect.top = ( int ) ulThreeQuartersInchY;
        m_thePrintInfo.m_theBodyRect.bottom = ( int ) ( ulPageHeightPixels - ulThreeQuartersInchY );
    }
    else
    {
        // create the header font 
        m_thePrintInfo.m_pHeaderFont = DEBUG_NEW_MFCOBJ_NOTHROW CFontInfo(FW_BOLD);
                
        if ( m_thePrintInfo.m_pHeaderFont != NULL )
        {
            // Lets get the user demanded font info
            int hdrFontSize = fontSize >> 16;
            m_thePrintInfo.m_pHeaderFont->CreateFont(strName, hdrFontSize, true, pDC);
        }
        
        // create the pen
        m_thePrintInfo.m_lPenWidth = ulYRes / 64; // 1/64 inch in pixels 
		m_thePrintInfo.m_pPen = DEBUG_NEW_MFCOBJ_NOTHROW CPen( PS_SOLID, ( int ) ( m_thePrintInfo.m_lPenWidth ), RGB( 0, 0, 0 ) );

        if ( m_thePrintInfo.m_pHeaderFont != NULL )
        {
            // select the font
            pOldFont = pDC->SelectObject( m_thePrintInfo.m_pHeaderFont );
        }
        
        // get the metrics
        pDC->GetTextMetrics( &theTM );
                
        // get the position for the header text in pixels
        m_thePrintInfo.m_HeaderTextRect.top = ( int ) ulOneHalfInchY;
        m_thePrintInfo.m_HeaderTextRect.bottom = ( int )(       m_thePrintInfo.m_HeaderTextRect.top + 
                                                                theTM.tmHeight + 
                                                                theTM.tmExternalLeading );
        m_thePrintInfo.m_HeaderTextRect.left = ( int ) ulOneHalfInchX;
        m_thePrintInfo.m_HeaderTextRect.right = ( int ) ( ulPageWidthPixels - ulOneHalfInchX );

        // get the position for the footer text
        m_thePrintInfo.m_FooterTextRect.bottom = ( int )( ulPageHeightPixels - ulOneHalfInchY );
        m_thePrintInfo.m_FooterTextRect.top =   ( int )(        m_thePrintInfo.m_FooterTextRect.bottom - 
                                                                theTM.tmHeight );
        m_thePrintInfo.m_FooterTextRect.left = ( int ) m_thePrintInfo.m_HeaderTextRect.left;
        m_thePrintInfo.m_FooterTextRect.right = ( int ) m_thePrintInfo.m_HeaderTextRect.right;

                
        // reset the font
        if ( m_thePrintInfo.m_pHeaderFont != NULL && pOldFont)
        {
            // select the font
            pDC->SelectObject( pOldFont );
        }
                                        
        // set the top margin in pixels
        m_thePrintInfo.m_theBodyRect.top = ( int )(     m_thePrintInfo.m_HeaderTextRect.bottom +
                                                        ( 2 * m_thePrintInfo.m_lPenWidth ) + 
                                                        ( ulOneHalfInchY / 2 ) );

        m_thePrintInfo.m_theBodyRect.bottom = ( int ) ( ulPageHeightPixels - m_thePrintInfo.m_theBodyRect.top );
    }

    ////////////////////////////////////////////////////////////////
        // Need to calculate the body rect in the current resolution...



        double bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.left / (double)ulXRes));

        //Convert to the screen resolution...for Paige
        m_thePrintInfo.m_theBodyRect.left = (int) ((bDub)* (double)m_thePrintInfo.m_ulXScreenRes);
        
        bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.right / (double)ulXRes));
        m_thePrintInfo.m_theBodyRect.right = (int)(bDub  * (double)m_thePrintInfo.m_ulXScreenRes);
        
        bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.top / (double)ulYRes));
        m_thePrintInfo.m_theBodyRect.top = (int)(bDub * (double)m_thePrintInfo.m_ulYScreenRes);
        
        bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.bottom / (double)ulYRes));
        m_thePrintInfo.m_theBodyRect.bottom = (int)(bDub * (double)m_thePrintInfo.m_ulYScreenRes);

        //Now convert the rect to a page_rect and use this calculate the number of pages
        RectToRectangle(&m_thePrintInfo.m_theBodyRect, &page_rect);
        
        //Get and then set the number of pages
        if(!m_bFixedWidth)
        {
            shape_ref newArea;
            newArea = pgRectToShape(PgMemGlobalsPtr(), &page_rect);
        
            m_thePrintInfo.visArea = pgRectToShape(PgMemGlobalsPtr() , NULL);
            m_thePrintInfo.pageArea = pgRectToShape(PgMemGlobalsPtr() , NULL);

            pgGetAreas (m_paigeRef, m_thePrintInfo.visArea, m_thePrintInfo.pageArea, MEM_NULL);

            if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
                pgSetAreas(m_thePrintInfo.m_printPgRef, newArea, newArea, MEM_NULL);
            else
                pgSetAreas (m_paigeRef, newArea, newArea, MEM_NULL);

            pgDisposeShape(newArea);
        }

        SHORT sPages = 0;

        if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
            sPages = pgNumPages(m_thePrintInfo.m_printPgRef, &page_rect);
        else
            sPages = pgNumPages(m_paigeRef, &page_rect);
        
        pInfo->SetMaxPage(sPages);
}

void CPaigeEdtView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    CView::OnEndPrinting(pDC, pInfo);

    // sanity check the dc---if bad, we're just gonna bail & pray. otoh, in preview
    // mode the dc *is* supposed to be null, and does not cause bad stuff to happen
    // if we keep going. this printing code is NASTY!!

    if ( !pDC->m_hDC && !pInfo->m_bPreview )
        return;

    // delete the header font
    delete m_thePrintInfo.m_pHeaderFont;
    m_thePrintInfo.m_pHeaderFont = NULL;

    // delete the pen
    delete m_thePrintInfo.m_pPen;
    m_thePrintInfo.m_pPen = NULL;

    // reset the mapping mode
    pDC->SetMapMode( m_thePrintInfo.m_uOldMappingMode );

    int count = m_thePrintInfo.m_printUndoStack.size();
    if (count)
    {
        //Delete the font sizes undo stack
        for (int i = 0; i < count; i++)
        {
            pgUndo(m_paigeRef, (undo_ref)m_thePrintInfo.m_printUndoStack.top() , TRUE, best_way);
            m_thePrintInfo.m_printUndoStack.pop();
        }

        ASSERT(m_thePrintInfo.m_printUndoStack.size() == 0 );
        ASSERT(m_thePrintInfo.m_offsetsQueue.size() == 0 );
        ASSERT(m_thePrintInfo.m_fontSizeQueue.size() == 0 );
        //pgUndo(m_paigeRef, *m_thePrintInfo.m_pNewUndo , TRUE, best_way);
        //delete m_thePrintInfo.m_pNewUndo;
        //m_thePrintInfo.m_pNewUndo = NULL;
    }

    if(!m_bFixedWidth)
    {
        pgSetAreas (m_paigeRef, m_thePrintInfo.visArea, m_thePrintInfo.pageArea, MEM_NULL);
        pgDisposeShape(m_thePrintInfo.visArea);
        pgDisposeShape(m_thePrintInfo.pageArea);
    }

    if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
    {
        pgSetPrintDevice(m_thePrintInfo.m_printPgRef, (generic_var)NULL);
        pgDispose(m_thePrintInfo.m_printPgRef);
        m_thePrintInfo.m_printPgRef = 0;
    }
    else
        pgSetPrintDevice(m_paigeRef, (generic_var)NULL);

    pgSetSelection(m_paigeRef, m_thePrintInfo.m_currSelection.begin, m_thePrintInfo.m_currSelection.end, 0, FALSE);
    SetRedraw( TRUE );
    Invalidate();
}

void CPaigeEdtView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    CRect       theRect;
    rectangle page_rect;
        
    //Store the rectange Paige will use to print into
    RectToRectangle(&m_thePrintInfo.m_theBodyRect, &page_rect);
        

    pDC->SetMapMode( MM_TEXT );

    // print the page header and footer if necessary
    if (m_bIncludeHeaderFooters && !(pInfo->m_pPD->m_pd.Flags & PD_SELECTION))
    {
        DoPageHeader(pDC, pInfo);
        DoPageFooter(pDC, pInfo);
    }

        
    HDC hDC = 0;
    hDC = pDC->GetSafeHdc();

    //Init the graf_device
    pgInitDevice( PgGlobalsPtr(), MEM_NULL, (generic_var)hDC, &m_device);
        
    if(pInfo->m_bPreview)
    {
        //Paige scales if we are in scaled preview mode.  We must adjust the resolution
        //We need to change the resolution in the graf_device first!
        CPaigePreviewDC *pPreviewDC = (CPaigePreviewDC*)pDC;
        double nNum = pPreviewDC->GetScaleNum();
        double nDen = pPreviewDC->GetScaleDen();
        double dScaleFactor = 1.0;
        short sHiResolution = pgHiWord(m_device.resolution);
        short sLoResolution = pgLoWord(m_device.resolution);    
                
        dScaleFactor = nDen/nNum;
        sHiResolution = (short)((double)sHiResolution / dScaleFactor);
        sLoResolution = (short)((double)sLoResolution / dScaleFactor);

        long lNewResolution = sHiResolution << 16;
        lNewResolution |= sLoResolution;

        m_device.resolution = lNewResolution;
    }
        
    long lStartOffset;
    if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
    {
        lStartOffset = pgFindPage(m_thePrintInfo.m_printPgRef, (short)pInfo->m_nCurPage, &page_rect);
        pgPrintToPage(m_thePrintInfo.m_printPgRef, &m_device, lStartOffset, &page_rect, best_way);
    }
    else
    {
        //Find the offset of the page to print
        lStartOffset = pgFindPage(m_paigeRef, (short)pInfo->m_nCurPage, &page_rect);
        //Print the page out
        pgPrintToPage(m_paigeRef, &m_device, lStartOffset, &page_rect, best_way);
    }
        
    pgCloseDevice( PgGlobalsPtr(), &m_device);
}       

void CPaigeEdtView::DoPageHeader(CDC* pDC, CPrintInfo* pInfo)
{
    char        temp[133];
    CFont       *pOldFont;
    CPen        *pOldPen;
    CRect       rcWhoCares;
    BOOL        bLayoutOnly = (pDC->GetClipBox( &rcWhoCares ) == NULLREGION);

    (pInfo);
    // this is better than the above line of code. If all you want to do is
    // test for a null-region, don't also modify the drawing rect!

    if ( bLayoutOnly ) 
    {
        return;
    }
 
    // Get the old font...and install our emboldend one
    if ( m_thePrintInfo.m_pHeaderFont != NULL )
    {
        pOldFont = ( CFont* ) pDC->SelectObject( m_thePrintInfo.m_pHeaderFont ) ;
    }
        
    if ( m_thePrintInfo.m_pPen )
    {
        // set the pen
        pOldPen = pDC->SelectObject( m_thePrintInfo.m_pPen );
    }

    // Develope the text for the header
    GetHeaderText(temp, sizeof(temp));  
                
    // Draw the text and line, but only if we're not just checking the layout
    pDC->DrawText(      temp, 
                        -1, 
                        &( m_thePrintInfo.m_HeaderTextRect ), 
                        DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    pDC->MoveTo(        m_thePrintInfo.m_HeaderTextRect.left, 
                        ( int ) ( m_thePrintInfo.m_HeaderTextRect.bottom + m_thePrintInfo.m_lPenWidth ) );
    pDC->LineTo(        m_thePrintInfo.m_HeaderTextRect.right, 
                        ( int ) ( m_thePrintInfo.m_HeaderTextRect.bottom + m_thePrintInfo.m_lPenWidth ) );
}  

void   CPaigeEdtView::OnFilePrintPreview()
{

	if (this->m_pABOrigView == NULL) 
	{
		CView::OnFilePrintPreview(); //non -address book print previews
		return;
	}

	//the code below is a hack - so dont call it unless you are doing address 
	//book print preview -sagar

	// In derived classes, implement special window handling here
	// Be sure to Unhook Frame Window close if hooked.

	// must not create this on the frame.  Must outlive this function
 	CPrintPreviewState* pState = DEBUG_NEW CPrintPreviewState;

	// DoPrintPreview's return value does not necessarily indicate that
	// Print preview succeeded or failed, but rather what actions are necessary
	// at this point.  If DoPrintPreview returns TRUE, it means that
	// OnEndPrintPreview will be (or has already been) called and the
	// pState structure will be/has been deleted.
	// If DoPrintPreview returns FALSE, it means that OnEndPrintPreview
	// WILL NOT be called and that cleanup, including deleting pState
	// must be done here.

	if (!DoPrintPreview(AFX_IDD_PREVIEW_TOOLBAR, this,
							RUNTIME_CLASS(CPrintPreviewView), pState))
	{
		// In derived classes, reverse special window handling here for
		// Preview failure case

		TRACE0("Error: DoPrintPreview failed.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		delete pState;      // preview failed to initialize, delete State now
	}
}

//      ========================================================================================
// DoPageFooter
void CPaigeEdtView::DoPageFooter(CDC* pDC, CPrintInfo* pInfo)
{
    char	temp[256];
    long	tempSize;
    CFont*	pOldFont = NULL;
    CPen*	pOldPen = NULL;
    CSize	size;
    CRect	rectPage;

    // this is better than the above line of code. If all you want to do is
    // test for a null-region, don't also modify the drawing rect!
    CRect rcWhoCares;
    BOOL bLayoutOnly = (pDC->GetClipBox( &rcWhoCares ) == NULLREGION);
        
    if ( bLayoutOnly ) 
    {
        return;
    }

    if ( m_thePrintInfo.m_pHeaderFont )
    {
        // Set the font for our operation here...
        pOldFont = pDC->SelectObject( m_thePrintInfo.m_pHeaderFont );
    }

    if ( m_thePrintInfo.m_pPen )
    {
        // set the pen
        pOldPen = pDC->SelectObject( m_thePrintInfo.m_pPen );
    }

    // Lets get the footer text and put it in the footer
    GetFooterText(temp, sizeof(temp));
    tempSize = strlen(temp);
        
    if (tempSize > 60)
    {
        strcpy(temp + 60, "...");
    }

    // Draw the text
    pDC->DrawText(      temp, 
                        -1, 
                        &( m_thePrintInfo.m_FooterTextRect ), 
                        DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    // print the page #
    wsprintf(temp, "%d  ", pInfo->m_nCurPage);
        
    rectPage = m_thePrintInfo.m_FooterTextRect;
    size = pDC->GetTextExtent(temp, strlen(temp));
        
    rectPage.left = rectPage.right - size.cx;

    pDC->DrawText(temp, -1, &rectPage, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    // draw the line
    pDC->MoveTo( m_thePrintInfo.m_FooterTextRect.left, 
                 ( int ) ( m_thePrintInfo.m_FooterTextRect.top - ( m_thePrintInfo.m_lPenWidth * 2 ) ) );
    pDC->LineTo( m_thePrintInfo.m_FooterTextRect.right, 
                 ( int ) ( m_thePrintInfo.m_FooterTextRect.top - ( m_thePrintInfo.m_lPenWidth * 2 ) ) );

    // Restore the original font & pen
    if( m_thePrintInfo.m_pHeaderFont && pOldFont) 
        pDC->SelectObject( pOldFont );

    if( m_thePrintInfo.m_pPen && pOldPen) 
        pDC->SelectObject( pOldPen );
}


// GetHeaderText
void CPaigeEdtView::GetHeaderText(LPTSTR theText, int nSize)
{
    ASSERT(theText != NULL);

    CDocument* pDoc = GetDocument();
    if (pDoc)
    {
        strncpy(theText, pDoc->GetTitle(), nSize);
        theText[nSize - 1] = 0;
    }
    else
        *theText = 0;
}



// GetFooterText
void CPaigeEdtView::GetFooterText(LPTSTR theText, int nSize)
{
    _snprintf(theText, nSize, (LPCTSTR)CRString(IDS_PRINT_FOOTER), GetReturnAddress());
    theText[nSize - 1] = 0;
}



/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView diagnostics

#ifdef _DEBUG
void CPaigeEdtView::AssertValid() const
{
    CView::AssertValid();
}

void CPaigeEdtView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

// OG
/*
  CPaigeEdtDoc* CPaigeEdtView::GetDocument() // non-debug version is inline
  {
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPaigeEdtDoc)));
  return (CPaigeEdtDoc*)m_pDocument;
  }
*/
#endif //_DEBUG

int CPaigeEdtView::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	if (!pTI)
		return -1;

	CRect			rect;
	embed_ref		refEmbed = GetEmbedRectAtPoint(point, rect);

	if (!refEmbed)
		return -1;

	bool	bFoundImageAltText = false;

	pTI->hwnd = m_hWnd;
	pTI->rect = rect;
	pTI->uId = refEmbed;

	pg_embed_ptr		embed_ptr = reinterpret_cast<pg_embed_ptr>( UseMemory(refEmbed) );
	if (embed_ptr)
	{
		char *		szAltText = NULL;
		if (embed_type(embed_ptr) == embed_url_image)
		{
			pg_url_image_ptr	url_image = reinterpret_cast<pg_url_image_ptr>( UseMemory( reinterpret_cast<memory_ref>(embed_ptr->data) ) );
			if (url_image)
			{
				if (url_image->alt_string)
					szAltText = strdup( reinterpret_cast<char *>(url_image->alt_string) );

				UnuseMemory((memory_ref)embed_ptr->data);
			}
		}
		else
		{
			HtmlObjectSpec* pSpec;
			if ( pSpec = (HtmlObjectSpec*) embed_ptr->user_data )
				szAltText = strdup(pSpec->title);
		}

		if (szAltText)
		{
			pTI->lpszText = szAltText;
			bFoundImageAltText = true;
		}

		UnuseMemory(refEmbed);
	}

	return bFoundImageAltText ? 1 : -1;
}

LRESULT CPaigeEdtView::OnUpdateImage( WPARAM wParam, LPARAM lParam )
{
	bool found = false;
	unsigned uid = (unsigned) wParam;
	FetchContext* pctx = 0;
	embed_ref er = 0;
	select_pair sel;
	long pos;
	pg_embed_ptr embed_ptr;

	sel.begin = 0;
	sel.end = pgTextSize( m_paigeRef );

	for ( long li =  1; !found && (er = pgGetIndEmbed( m_paigeRef, &sel, li, &pos, 0 )); li++ ) {
		if ( er && (embed_ptr = (pg_embed_ptr)UseMemory( er )) ) {
			pctx = (FetchContext*) embed_ptr->user_refcon;

			if ( pctx && pctx->uid == uid ) {
				pgInvalEmbedRef( m_paigeRef, pos, embed_ptr, best_way );
				UnuseMemory( er );
				break;
			}

			UnuseMemory( er );
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView message handlers

int CPaigeEdtView::GetTotalHeight()
{
    return (pgTotalTextHeight(m_paigeRef, TRUE) + m_pgBorder.top + m_pgBorder.bottom);
}

int CPaigeEdtView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
/*      rectangle pgClientRect, emptyRect;
        shape_ref visArea, excludeArea;*/

    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (m_ToolTip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX) != -1)
		EnableToolTips();

        //Register the COleDropTarget object for OLE Drag n Drop
        //  Will be revoked when the view is destroyed
    m_dropTarget.Register(this);

    CRect clientRect;
    GetClientRect(&clientRect);

    // initialize our border space
    m_pgBorder.left =	BORDER_LEFT;
    m_pgBorder.top =	BORDER_TOP;
    m_pgBorder.right =	BORDER_RIGHT;
    m_pgBorder.bottom =	BORDER_BOTTOM;

    // initialize the spell checking method -- must do before "NewPaigeObject"
    g_bInteractiveSpellCheck = GetIniShort( IDS_INI_INTERACTIVE_SPELL_CHECK );
    //g_bMoodMailCheck = GetIniShort( IDS_INI_MOOD_MAIL_CHECK );

    // create an editor instance
    NewPaigeObject();

    // make sure that image/QT support is ready to rock
    PgMultimediaInit( PgGlobalsPtr() );

    SetPaigePalette();
/*
  pg_hooks hooks;
  pgGetHooks(m_paigeRef, &hooks);
  hooks.paginate_proc =         (post_paginate_proc)myPostPaginateProc;
  pgSetHooks(m_paigeRef, &hooks, FALSE);
*/
    return 0;
}

bool CPaigeEdtView::SetPaigePalette(void)
{
    CDC *pDC = GetDC();
    if (!pDC) return false;
    int rc = pDC->GetDeviceCaps(RASTERCAPS);
    if (rc & RC_PALETTE)
    {
        rc = pDC->GetDeviceCaps(SIZEPALETTE);
        if (256 == rc)
        {
            HPALETTE hPalette = m_Palette.GetPalette();
            pgSetDevicePalette(m_paigeRef, (generic_var)hPalette);
        }
    }
    ReleaseDC(pDC);
    return true;
}

bool CPaigeEdtView::NewPaigeObject(long AddFlags /*= 0*/, long AddFlags2 /*= 0*/)
{
    DeletePaigeObject();

    rectangle pgClientRect, emptyRect;
    shape_ref visArea, excludeArea;
        
    CRect clientRect;
    GetClientRect(&clientRect);
    clientRect.DeflateRect( m_pgBorder );

    // All Paige areas are represented as Shapes
    RectToRectangle(&clientRect,&pgClientRect);
    visArea = pgRectToShape(PgMemGlobalsPtr(), &pgClientRect);

    // set up an empty shape for "exclude area". with shared styles, if we
    // don't pass something now, we're screwed later.
    memset( &emptyRect, '\0', sizeof(rectangle) );
    excludeArea = pgRectToShape( PgMemGlobalsPtr(), &emptyRect );

    //Set both page area and visible area the same
    m_paigeRef = pgNew( PgGlobalsPtr(), (generic_var)GetSafeHwnd(),
                        visArea, visArea, excludeArea, /*0*/NO_HIDDEN_TEXT_BIT );

    paige_rec_ptr pPg = (paige_rec_ptr) UseMemory( m_paigeRef );
    pPg->flags |= AddFlags;
    pPg->flags2 |= AddFlags2;

    //Inform Paige to use def page background color to do the erasing.
    pg_doc_info docInfo;
    pgGetDocInfo(m_paigeRef, &docInfo);
    docInfo.attributes |= COLOR_VIS_BIT;
    pgSetDocInfo(m_paigeRef, &docInfo, FALSE, draw_none);

    // set up the Eudora/Paige glue bucket
    PgStuffBucket* pSB = DEBUG_NEW PgStuffBucket;
    pSB->kind = PgStuffBucket::kDocument;
    pSB->pWndOwner = this;
    pPg->user_refcon = (long) pSB;
    UnuseMemory( m_paigeRef );

    // Paige maintains a copy of the shape, so we can dispose what we created
    pgDisposeShape(visArea);
    pgDisposeShape( excludeArea );

    // Set the body style to have the MessageFont
    LOGFONT lf;
    font_info fontInfo;
    style_info styleInfo;
    style_info styleInfoMask;
    par_info parInfo;
    par_info parMask;
    GetMessageFont().GetLogFont( &lf );

    PgConvertLogFont( m_paigeRef, PgGlobalsPtr(), &lf, &fontInfo, &styleInfo, &styleInfoMask );
    pgGetParInfo( m_paigeRef, NULL, false, &parInfo, &parMask );

    //Set the tab space depending on the font and the tabstop value specified by the user.
    int avgWidth;
    avgWidth = GetMessageFont().CharWidth();
    parInfo.def_tab_space = avgWidth * GetIniShort(IDS_INI_TAB_STOP);

    long style_id = pgNewNamedStyle( m_paigeRef, body_style, &styleInfo, &fontInfo, &parInfo );

    //This is a hack to prevent the body_style from getting deleted. When paige fixes
    //up style runs, it deletes all styles which are unused, so we increment the 
    //used_ctr here to prevent it from being whacked.
    ASSERT(style_id);

    paige_rec_ptr pg;
    pg = (paige_rec_ptr) UseMemory(m_paigeRef);
        
    register style_info_ptr             styles;
    register pg_short_t                 style_ctr, style_qty;
        
    styles = (style_info_ptr)UseMemory(pg->t_formats);
    style_qty = (pg_short_t)GetMemorySize(pg->t_formats);
        
    for (style_ctr = 0; style_ctr < style_qty; ++styles, ++style_ctr)
    {
        if (styles->style_sheet_id == style_id) 
        {
            styles->used_ctr++;
            break;
        }
    }
        
    UnuseMemory(pg->t_formats);
    UnuseMemory(m_paigeRef);

    // smack it full of HTML styles
    CreateHTMLStyles( m_paigeRef, /*PgGlobalsPtr()->def_font.name*/ (unsigned char *)lf.lfFaceName, NULL );

    delete m_styleEx;
	m_styleEx = DEBUG_NEW CPaigeStyle(m_paigeRef);

    // Shareware: In reduced feature mode, you cannot spell check
	// or have emoticons converted as you type
    if (UsingFullFeatureSet())
    {
        // re-init the spelling object's paige reference
        m_spell.Init( this );

		// Set the initial scan timer. Probably not strictly necessary since we'll set
		// a scan timer anyway when we get focus, but it doesn't hurt.
        SetTimer( SCAN_TIMER, SCAN_INTERVAL, NULL );
    }

	// set our "global" line-adjust procedure
	pg_hooks hooks;
	pgGetHooks( m_paigeRef, &hooks );
	hooks.adjust_proc = PGS_MainLineAdjustProc;
	pgSetHooks( m_paigeRef, &hooks, FALSE );

    return true;
}

void CPaigeEdtView::DeletePaigeObject()
{
    undo_ref *theRef;
    for (int i = m_undoStack.GetCount(); i ; i--)
    {
        theRef = m_undoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_undoStack.RemoveTail();
    }
    for (i = m_redoStack.GetCount(); i; i--)
    {
        theRef = m_redoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_redoStack.RemoveTail();
    }
    for (i = m_deleteUndoStack.GetCount(); i ; i--)
    {
        theRef = m_deleteUndoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_deleteUndoStack.RemoveTail();
    }
    for (i = m_deleteRedoStack.GetCount(); i; i--)
    {
        theRef = m_deleteRedoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_deleteRedoStack.RemoveTail();
    }

    if (m_paigeRef)
    {
        // gotsta smoke ma stuff!
        paige_rec_ptr pPg = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pPg->user_refcon;
        delete pSB;
        UnuseMemory( m_paigeRef );

        pgDispose(m_paigeRef);
    }
}

void CPaigeEdtView::OnDestroy() 
{
    // unselect the palette
    generic_var hPalette;
    hPalette = pgGetDevicePalette(m_paigeRef);
    if (hPalette)
        pgSetDevicePalette(m_paigeRef, (generic_var)0);

    DeletePaigeObject();

    CView::OnDestroy();
}

void CPaigeEdtView::OnSize(UINT nType, int cx, int cy) 
{       
    CView::OnSize(nType, cx, cy);
    SetPgAreas();
    UpdateScrollBars();

#if 0
// BOG: this code was causing the "smearing" during repaint problem. trying to
// srcroll the caret into view in OnSize is a real bad idea; when live window
// dragging is enabled, all hell breaks loose. i'm leaving this here just in
// case somebody wants to try it somewhere else, like windowposchanged, but
// i don't think any other editors do this---why should we?

    //Get current caret position (client co_ords)
    CPoint curPoint = GetCaretPos();;
    //Fill Paige coord struct
    co_ordinate curCoOrd;
    CPointToPgPoint(&curPoint, &curCoOrd);

    //Scroll to caret only if it was visible before resizing
    short posBits = 0;
    posBits = pgPtInView(m_paigeRef, &curCoOrd, NULL);
    if (posBits & WITHIN_VIS_AREA) {
        //Reset page and visible areas after sizing
        if (pgTextSize(m_paigeRef)) ScrollToCursor();
    }
#endif
}       

void CPaigeEdtView::CPointToPgPoint(CPoint *cPoint, co_ordinate_ptr PgPoint) const
{
    PgPoint->h = cPoint->x;
    PgPoint->v = cPoint->y;

	// Fix to bug in Paige where a hyperlink around an image will return that it's
	// not in a hyperlink on the right half of the image.  We fix it by pointing to
	// the beginning of the embedded image.
	CRect rect;
	if ( GetEmbedRectAtPoint(*cPoint, rect) )
		PgPoint->h = rect.left;
}

embed_ref CPaigeEdtView::GetEmbedRectAtPoint(CPoint & ptMouse, CRect & rect) const
{
	co_ordinate		pgMousePt;
    pgMousePt.h = ptMouse.x;
    pgMousePt.v = ptMouse.y;

	embed_ref		embed = pgPtInEmbed( m_paigeRef, &pgMousePt, 0, 0, false );
	
	if (embed)
	{
		style_info		the_style;
		select_pair		the_range;
		rectangle		pgRect;
		co_ordinate		screen_extra;
		
		pgPtToStyleInfo(m_paigeRef, &pgMousePt, NO_BYTE_ALIGN | NO_HALFCHARS, &the_style, &the_range);

		pgGetEmbedBounds(m_paigeRef, 1, &the_range, &pgRect, NULL, &screen_extra);

		RectangleToRect(&pgRect, &screen_extra, &rect);
	}

	return embed;
}

void CPaigeEdtView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// Shareware: In reduced feature mode, you cannot right-click
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode

		//
		// Put cooridnates in a format that Paige recognizes
		//
		co_ordinate pg_mouse;
		CPointToPgPoint(&point,&pg_mouse);
		//
		// This point rests in a word. Select that word.
		//
		select_pair wordSel, curSel;
		long offset = pgPtToChar(m_paigeRef, &pg_mouse, NULL);
		pgFindWord(m_paigeRef, offset, &wordSel.begin, &wordSel.end,TRUE,FALSE);
		pgGetSelection(m_paigeRef, &curSel.begin, &curSel.end);

		// 
		// Avoid treating this as a spelling click if
		// the click took place in a selection that includes stuff
		// besides the word
		//
		if ( (curSel.begin < offset) && (offset <= curSel.end) )
		{
			bool	bSelectionIncludesOtherWords = false;
			
			if (curSel.begin < wordSel.begin)
			{
				bSelectionIncludesOtherWords = true;
			}
			else if (wordSel.end < curSel.end)
			{
				if ( (curSel.end - wordSel.end) > 7 )
				{
					bSelectionIncludesOtherWords = true;
				}
				else
				{
					// When the user double clicks on a word Paige selects all the spaces to right
					// of that word. Allow some extra spaces selected to the right within reason.
					char			szCheckChars[8];
					long			nLength = 0;
					select_pair		tempSel;

					tempSel.begin = wordSel.end + 1;
					tempSel.end = curSel.end;

					GetPgText(szCheckChars, sizeof(szCheckChars), tempSel, FALSE, &nLength);

					for (short i = 0; i < nLength; i++)
					{
						if (szCheckChars[i] != ' ')
						{
							bSelectionIncludesOtherWords = true;
							break;
						}
					}
				}

				if (bSelectionIncludesOtherWords)
					return;
			}
		}

		//
		// Misspelled?
		//
		if ( m_styleEx->IsMisspelled(&wordSel) )
		{
			pgSetSelection(m_paigeRef, wordSel.begin, wordSel.end, 0, TRUE);
			//
			// This dialog should really be off the lower right corner
			// of the selection.  Need to change.
			//
			POINT pt;
			pt.x = point.x;
			pt.y = point.y;
			ClientToScreen(&pt);
			m_spell.Popup(this, pt, wordSel);
		}           
	}

    return;
}

// handy-dandy "KindOf" boogie
#define is_kind_of(class_name, object) \
	((object)->IsKindOf(RUNTIME_CLASS(class_name)))

void CPaigeEdtView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    {   // BOG: this is a "re-entrant" hack. if we get called again in the middle
	// of hyperlink processing, bad things can/will happen.
        paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        UnuseMemory( m_paigeRef );

        if ( pSB->hlinkProcessing == true ) {
            assert( 0 );
            return;
        }
    }   // BOG: end re-entrant prophylactic
    
    {   // BOG: see if anything needs spell-checking
        CPaigeStyle objStyle( m_paigeRef );
        select_pair textRange;
        pgGetSelection( m_paigeRef, &textRange.begin, &textRange.end );

        select_pair curWord;
        pgFindWord( m_paigeRef, textRange.begin, &curWord.begin, &curWord.end, 1, 0 );

        co_ordinate newCoord;
        long newOffset;
        CPointToPgPoint( &point, &newCoord );
        newOffset = pgPtToChar( m_paigeRef, &newCoord, NULL );
        bool bInCurrentWord = !((newOffset < curWord.begin) || (newOffset > curWord.end));

        if ( !bInCurrentWord && objStyle.NeedsScanned( &textRange ) )
            m_checkdastuff = true;
    }   // BOG: end spellcheck hack


    BOOL ptInSelection = false;
    BOOL bSelectLine = FALSE;
    paige_rec_ptr pgPtr = MEM_NULL;

    co_ordinate pgMousePt;
    CPointToPgPoint(&point, &pgMousePt);

    SetCapture();
    m_pgModifiers = 0; 
    m_mousePt = CPoint(0,0);
    if (nFlags & MK_SHIFT) m_pgModifiers |= EXTEND_MOD_BIT;
    //if (nFlags & MK_CONTROL) m_pgModifiers |= DIS_MOD_BIT;

    //Drag-n-Drop and Triple-click selection will occur only if the click
    //was in selected text
    if ( pgNumSelections( m_paigeRef ) ) {
        //Create an empty shape and use it to get the current hilite region
        shape_ref hiliteRgn = 0;
        if ( (hiliteRgn = pgRectToShape(PgMemGlobalsPtr(), NULL)) != 0 ) {
            if ( pgGetHiliteRgn( m_paigeRef, NULL, MEM_NULL, hiliteRgn ) ) {
                if ( pgPtInShape( hiliteRgn, &pgMousePt, NULL, NULL, NULL ) )
                    ptInSelection = true;
            }

            pgDisposeShape( hiliteRgn );
        }
    }

    switch(m_mouseState)
    {
        //case MOUSE_DOUBLE_CLICKED : 
    case MOUSE_TRIPLE_CLICKED : 
        KillTimer(TRIPLE_CLICK_TIMER);
        m_mouseState &= ~MOUSE_TRIPLE_CLICKED;
        //Select only if the click was in selected text.
        if (ptInSelection)
            m_pgModifiers |= PAR_MOD_BIT;
        m_mouseState |= MOUSE_DRAG_SELECT;
        break;
    default :
        m_mousePt = point;
        if (ptInSelection) 
            m_mouseState |= (MOUSE_DRAG_DROP | MOUSE_DOWN);
        else
			m_mouseState |= MOUSE_DRAG_SELECT;
        break;
    }

    //Delay the func in case it turns out to be a drag-drop operation, else the 
    //text is deselected and the caret will be positioned at the current 
    //mouse-click pos. Incase it is an ordinary mouse click, then
    //we process DragSelect with both mouse_down and mouse up in LButtonUp.
    if (!(m_mouseState & MOUSE_DRAG_DROP))
    {
        //If the mouse click is in the *selection region* in the left border, 
        //select the whole line, extending/deselecting previous selections, if any.                     
        if ((point.x < m_pgBorder.left) && (point.y > m_pgBorder.top)&& (nFlags & MK_CONTROL))
        {
            SelectionMarginClicked(nFlags, point);
            m_prevPoint.x = point.x;
            m_prevPoint.y = point.y;
        }
        else if ((point.x < m_pgBorder.left) && (point.y > m_pgBorder.top) && (m_prevPoint.x == 0) && (m_prevPoint.y == 0))
        {       
            //Bug 2341. If this if the first time the user clicks in the margin then we need to pretend like a 
            //MOUSE_DRAG_SELECT action has occured.  Find the begining of the next line and set point.y to be that and
            //drag select to that point.  This will allow the user to drag and select lines at a time as well as
            //just select one line.
            bSelectLine = TRUE;
            CRect aRect;
            GetClientRect(aRect);
            CPointToPgPoint(&point, &pgMousePt);
            pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

            //Find the point of the next line...
            co_ordinate curCoOrd;
                        
            long offset, oldLineLeft, lineLeft, lineRight;
                        
            CPointToPgPoint(&point, &curCoOrd);


            offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                        
                        //Since Paige resets the anchor only on the next mouse_down
                        //to this offset [incase the user clicked with the Shift key
                        //down, Paige has to know from where to extend the selection]
                        //So we set the anchor if clicking in the margin or without the
                        //shift key down.
            pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
            pgPtr->hilite_anchor = offset;
            UnuseMemory(m_paigeRef);

            pgFindLine(m_paigeRef, offset, &lineLeft, &lineRight);
            oldLineLeft = lineLeft;
                        
            //Move to next line
            while((lineLeft == oldLineLeft) && (point.y < aRect.bottom))
            {
                point.y +=2;
                CPointToPgPoint(&point, &curCoOrd);
                offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                pgFindLine(m_paigeRef, offset, &lineLeft, &lineRight);
                
                if(point.y > aRect.bottom)
                {
                    point.y = m_prevPoint.y + 20;
                    break;
                }
            }
            //Use the next point.
            CPointToPgPoint(&point, &pgMousePt);
            SetTimer(DRAG_SELECT_TIMER, 100, NULL);
            pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
        }
        //Let Paige take care of any other ordinary, mundane stuff.
        /*else*/ if ((m_mouseState & MOUSE_DRAG_SELECT) && !bSelectLine)
        {
// BOGOLE: any in-place active control
// BOGOLE: not sure we need to assert the doc type here. perhaps it's a normal
// occurence for the doc to not be ole---just means nothing's in-place active.


            SetTimer(DRAG_SELECT_TIMER, 100, NULL);

			// Adjust the mouse position if necessary to correct Paige's selection
			// of embedded images. See AdjustMouseIfEmbedded for more details.
			AdjustMouseIfEmbedded(pgMousePt);

            pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

			COleDocument* poleDoc = (COleDocument*) GetDocument();
			if (poleDoc)
			{
				ASSERT_KINDOF( COleDocument, poleDoc );

				if ( is_kind_of(COleDocument, poleDoc) ) {
					COleClientItem* pItem = poleDoc->GetInPlaceActiveItem( this );

					if ( pItem )
						pItem->Close();
				}
			}

            if ( ! (nFlags & MK_SHIFT) )
            {
                pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
                co_ordinate curCoOrd;
                CPointToPgPoint(&point, &curCoOrd);
                long offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                pgPtr->hilite_anchor = offset;
                UnuseMemory(m_paigeRef);
            }
        }
    }

    // BOG: Mouse clicks cancel "always visible find selection" mode
    m_bDontDeactivateHighlight = FALSE;

    CView::OnLButtonDown(nFlags, point);
}

void CPaigeEdtView::SelectionMarginClicked(UINT nFlags, CPoint point)
{
    paige_rec_ptr pgPtr = MEM_NULL;
    pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
    ASSERT(pgPtr != MEM_NULL);
    long anchor = pgPtr->hilite_anchor;

    //Control Click selects the entire document
    if (nFlags & MK_CONTROL)
    {
        pgSetSelection(m_paigeRef, 0, pgTextSize(m_paigeRef), 0, TRUE);
        pgPtr->hilite_anchor = 0;
        m_mouseState &= ~MOUSE_DRAG_SELECT;
    }
    else 
    {
        co_ordinate curCoOrd;
        long offset, selectLeft, selectRight, lineLeft, lineRight;
                
        pgGetSelection(m_paigeRef, &selectLeft, &selectRight);

        CPointToPgPoint(&point, &curCoOrd);
        offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
        pgFindLine(m_paigeRef, offset, &lineLeft, &lineRight);
                
        if (nFlags & MK_SHIFT)
        {
            // The hilite-anchor can be 0 in the following 3 cases
                                //1. Anchor was at 0, text may or may not be selected
                                //2. The whole text has been selected (Deselect in this case)
                                //3. A line may have been previously selected with Ctrl+Click 
                                //   (Extend selection in any direction in this case)

            // Set the correct anchor if the previous selection was from a Ctrl+Click
            if (anchor == 0)
            {
                long tempLeft, tempRight;
                pgFindLine(m_paigeRef, selectLeft, &tempLeft, &tempRight);
                if ((selectLeft == tempLeft) && (selectRight == tempRight))
                    pgPtr->hilite_anchor = (lineLeft < selectLeft) ? selectRight : selectLeft;
            }
            // Extend selection OR deselect depending on the anchor position
            if (anchor == selectLeft)
            {
                if (lineLeft < selectRight)
                {
                    pgSetSelection(m_paigeRef, anchor, lineLeft, 0, true);
                    m_mouseState &= ~MOUSE_DRAG_SELECT;
                }
                                //else do the usual processing
            }
            else if (anchor == selectRight)
            {
                if (lineLeft > selectLeft)
                {
                    pgSetSelection(m_paigeRef, lineLeft, anchor, 0, true);
                    m_mouseState &= ~MOUSE_DRAG_SELECT;
                }
                                //else do the usual processing
            }

            /*OLD STUFF
              //If the caret/entire-current-selection is on the same line,
              //set the selection to the entire line
              //if ((selectLeft > lineLeft) && (selectRight < lineRight))
              //        pgSetSelection(m_paigeRef, lineLeft, lineRight, 0, TRUE);
              //if we are SHIFT clicking *below* any existing-selected-lines, we need
              //to extend the selection. Normal paige processing will take care of 
              //extending selection if we SHIFT click above the existing-selected-lines.
              //else 
              if (lineLeft < selectLeft )
              pgExtendSelection(m_paigeRef, lineLeft - selectLeft, EXTEND_MOD_BIT, TRUE);
              else if (lineRight > selectRight)
              pgExtendSelection(m_paigeRef, lineRight - selectRight, EXTEND_MOD_BIT, TRUE);*/
        }
        else 
        {
            //Ordinary click sets the selection to that line
            pgSetSelection(m_paigeRef, lineLeft, lineRight, 0, TRUE);
            pgPtr->hilite_anchor = 0;
            m_mouseState &= ~MOUSE_DRAG_SELECT;
        }
    }                           

    if (pgPtr) UnuseMemory(m_paigeRef);
}


void
CPaigeEdtView::FindSelectedParagraph(
	select_pair &			in_sel,
	long &					out_nParaBegin,
	long &					out_nParaEnd)
{
	//	Find the start and end of the paragraph in the middle of the selction
	//	or find the paragraph at the insertion point (math works for both).
	pgFindPar(m_paigeRef, (in_sel.begin+in_sel.end)/2, &out_nParaBegin, &out_nParaEnd);
		
	//	If there's a selection, check to see if an entire paragraph is selected
	if (in_sel.begin != in_sel.end)
	{
		if (out_nParaBegin < in_sel.begin)
		{
			//	The start of the paragraph is before the start of the selection.
			//	Look at the previous paragraph if it's possible that it will be
			//	part of the selection.
			if ( out_nParaEnd < (in_sel.end-2) )
				pgFindPar(m_paigeRef, out_nParaEnd+2, &out_nParaBegin, &out_nParaEnd);
		}
		else if ( (out_nParaEnd-1) > in_sel.end )
		{
			//	The end of the paragraph (after adjusting to not include the
			//	terminating CR) is after the end of the selection.
			//	Look at the next paragraph if it's possible that it will be
			//	part of the selection.
			if ( out_nParaBegin > (in_sel.begin+2) )
				pgFindPar(m_paigeRef, out_nParaBegin-2, &out_nParaBegin, &out_nParaEnd);
		}

		//	Adjust the paragraph end to not include the terminating CR.
		//
		//	We don't do this if there's just an insertion point, because it won't
		//	affect the decision in Format Painter of whether or not to copy or apply
		//	paragraph styles. Also when we're applying styles with a single insertion
		//	point, we want to affect the entire paragraph.
		out_nParaEnd--;
	}
}

void CPaigeEdtView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // BOG: this is a "re-entrant" hack. if we get called again in the middle
    // of hyperlink processing, bad things can/will happen.
    {
        paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        UnuseMemory( m_paigeRef );

        if ( pSB->hlinkProcessing == true ) {
            assert( 0 );
            return;
        }
    }   // BOG: end re-entrant prophylactic

    co_ordinate pgMousePt;
    //Reset the previous point for next time.
    m_prevPoint.x = m_prevPoint.y = 0;

        //If the click was on the selected text, we delayed incase it was a
        //Drag-n-drop operation. If it wasn't, then this is where we call 
        //pgDragSelect with mouse-down verb
    if (m_mouseState & MOUSE_DOWN)
    {
        CPointToPgPoint(&m_mousePt, &pgMousePt);
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);
        m_mouseState &= ~(MOUSE_DOWN | MOUSE_DRAG_DROP);
        // Call DragSelect again with mouse_up verb for Paige to do some internal cleanup
        CPointToPgPoint(&point, &pgMousePt);

		// Adjust the mouse position if necessary to correct Paige's selection
		// of embedded images. See AdjustMouseIfEmbedded for more details.
		AdjustMouseIfEmbedded(pgMousePt);
		
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_up, m_pgModifiers, 0, FALSE);
    }

    if (m_mouseState & MOUSE_DRAG_SELECT) 
    {
        KillTimer(DRAG_SELECT_TIMER);
        // Call DragSelect again with mouse_up verb for Paige to do some internal cleanup
        CPointToPgPoint(&point, &pgMousePt);

		// Adjust the mouse position if necessary to correct Paige's selection
		// of embedded images. See AdjustMouseIfEmbedded for more details.
		AdjustMouseIfEmbedded(pgMousePt);
		
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_up, m_pgModifiers, 0, FALSE);

		if (m_mouseState & MOUSE_FORMAT_PAINTER)
		{
			select_pair		sel;
			long			nParaBegin = 0;
			long			nParaEnd = 0;
			bool			bAffectPar = false;
			
			if (m_bCopiedStyleInfo || m_bCopiedParInfo)
			{
				//	Get the selection
				pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

				if (m_bCopiedParInfo)
				{
					//	Look for a selected paragraph
					FindSelectedParagraph(sel, nParaBegin, nParaEnd);

					if (sel.begin == sel.end)
					{
						//	The selection is an insertion point, so affect
						//	the entire paragraph found above
						sel.begin = nParaBegin;
						sel.end = nParaEnd;
						bAffectPar = true;
					}
					else
					{
						//	Affect paragraph styles if we found at least one
						//	paragraph that's entirely selected
						bAffectPar = ( (nParaBegin >= sel.begin) && (nParaEnd <= sel.end) );
					}
				}
				else if (sel.begin == sel.end)
				{
					//	The selection is an insertion point and we're didn't copy
					//	paragraph info when we started Format Painter, so affect
					//	the word that the insertion point is currently inside.
					pgFindWord(m_paigeRef, sel.begin, &sel.begin, &sel.end, true, false);
				}
				
				//	Prepare undo before we change anything
				PrepareUndo(undo_format);
			}
			
			if (m_bCopiedStyleInfo)
			{
				style_info		styleMask;
				
				//	Init the style mask to 0
				pgInitStyleMask(&styleMask, 0);

				//	Set fields we want to copy to -1
				styleMask.styles[bold_var] = -1;
				styleMask.styles[italic_var] = -1;
				styleMask.styles[underline_var] = -1;
				styleMask.styles[strikeout_var] = -1;
				styleMask.fg_color.red = 0xFFFF;
				styleMask.fg_color.green = 0xFFFF;
				styleMask.fg_color.blue = 0xFFFF;
				styleMask.bk_color.red = 0xFFFF;
				styleMask.bk_color.green = 0xFFFF;
				styleMask.bk_color.blue = 0xFFFF;
				styleMask.point = -1;

				font_info		fontMask;

				//	Init the font mask to -1 so that we copy all fields
				pgInitFontMask(&fontMask, -1);
				
				pgSetStyleAndFont( m_paigeRef, &sel, &m_styleInfo, &styleMask,
								   &m_fontInfo, &fontMask, best_way );
			}

			if (bAffectPar)
			{
				par_info		parMask;
				
				//	Init the par mask to 0
				pgInitParMask(&parMask, 0);

				//	Set fields we want to copy to -1
				parMask.justification = -1;
				parMask.class_info = -1;
				parMask.html_style = -1;
				parMask.indents.left_indent = -1;
				parMask.html_bullet = -1;
				parMask.html_numbers = -1;

				pgSetParInfo(m_paigeRef, &sel, &m_parInfo, &parMask, best_way);
			}

			m_mouseState &= ~MOUSE_FORMAT_PAINTER;
		}

		m_mouseState &= ~MOUSE_DRAG_SELECT;
    }

    if (m_pgModifiers & PAR_MOD_BIT)
    {
        //If it was a triple click, set the hilite anchor 
        //to the beginning of the selection
        long selectLeft;
        pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);

        paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
        pgPtr->hilite_anchor = selectLeft;
        UnuseMemory(m_paigeRef);
                
    }

    m_pgModifiers = 0;
    m_mousePt = CPoint(0,0);
    m_mouseState &= ~MOUSE_DRAG_SELECT;
    ReleaseCapture();

    CView::OnLButtonUp(nFlags, point);

        // BOG: we want to do this after anything else is out of the way, even
        // calling the base CView version first. This will prevent long
        // operations, resulting from the user clicking a hyperlink, from mucking
        // up anything else (selections, focus changes, etc...)
        //
        // As an aside, I wonder if we shouldn't be calling CView first. If the
        // user clicks a window in the background, do we really want to go
        // through all of our own Paige-related processing before the default
        // Windows stuff occurs? Just a thought ;-)     
        
    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    UnuseMemory( m_paigeRef );

    if (pSB->hlinkEvent)
    {
        select_pair sel;
        pg_hyperlink autoLink;
        
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end); 

        pgGetHyperlinkSourceInfo(m_paigeRef, sel.begin, pSB->hlinkEvent, TRUE, &autoLink);

        //If it's an AutoURL make sure it's not stale.
        if ((!m_fRO) && (autoLink.type & HYPERLINK_EUDORA_AUTOURL))
        {
            char buf[INTERNET_MAX_URL_LENGTH];
            GetPgText(buf, sizeof(buf), autoLink.applied_range, FALSE);
            if (ValidURLScheme(buf))
            {
                char temp[INTERNET_MAX_URL_LENGTH];
                strncpy(temp, EscapeURL(buf), sizeof(temp));
                temp[sizeof(temp) - 1] = 0;
                pgSetSourceURL( m_paigeRef, autoLink.applied_range.begin, 
                                (unsigned char *)temp );
            }
            else
            {
                pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);
                paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
                PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
                pSB->hlinkEvent = 0;
                UnuseMemory( m_paigeRef );
            }
        }               

        // Process the hyperlink now [whatever it is]
		// Alt + click on URL in edit mode will launch
        if (m_fRO || GetKeyState(VK_MENU) < 0)
            PgProcessHyperlinks( m_paigeRef );
    }

}

BOOL CPaigeEdtView::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{	
	bool			bCheckedLink = false;
	bool			bShowingPopupText = false;
	
	HCURSOR hCur = NULL;

	if (nHitTest == HTCLIENT && pWnd == this)
	{
		CPoint		ptCursorScreen;

		if ( GetCursorPos(&ptCursorScreen) )
		{
			CPoint		ptCursorClient = ptCursorScreen;
			
			ScreenToClient(&ptCursorClient);

			co_ordinate mousePt;
			CPointToPgPoint(&ptCursorClient, &mousePt);
			short ptID = pgPtInView(m_paigeRef, &mousePt, NULL);

			if (ptID & WITHIN_VIS_AREA)
			{
				CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
				bool bSetStatusText = false;
				int offset = pgPtInHyperlinkSource(m_paigeRef, &mousePt);

				if (offset != -1)
				{
					bool		bShouldCheckLink = false;
					
					// Alt + click on URL in edit mode will launch
					if (m_fRO || GetKeyState(VK_MENU) < 0)
					{
						hCur = QCLoadCursor(IDC_APP_LAUNCH);

						// We should check the link if we're here (i.e. if the link
						// can be clicked on), we're using the full feature set,
						// and the user has ScamWatch ToolTips turned on.
						bShouldCheckLink = UsingFullFeatureSet() && GetIniShort(IDS_INI_SHOW_SCAMWATCH_TOOLTIP);
					}
					
					//
					// Do hyperlink check for status bar
					//
					char url[INTERNET_MAX_URL_LENGTH];
					if (pgGetSourceURL(m_paigeRef, offset, (pg_char_ptr)url, INTERNET_MAX_URL_LENGTH))
					{
						const char* DisplayURL = url;

						paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
						PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
						UnuseMemory( m_paigeRef );
						if (pSB->kind == PgStuffBucket::kAd)
						{
							// Ad hack for now: skip redirect part and show just final destination URL.
							// This should go away when we implement clickbase.
							char* s;
							if (s = strstr(DisplayURL, "url="))
								DisplayURL = s + 4;
							if (s = strstr(DisplayURL, "&distributorid"))
								*s = 0;
						}
						else if (bShouldCheckLink)
						{
							pg_hyperlink		link;
							char				szURL[1024];
							
							//	Get the hyperlink source info
							if ( pgGetHyperlinkSourceInfo(m_paigeRef, offset, 0, false, &link) )
							{
								//	Did we already check the current link?
								//	(We're checking because we'll called here repeatedly for a given link.)
								if (s_nLastCheckedHyperlinkID == link.unique_id)
								{
									//	Are we already showing a link warning for the current link?
									bShowingPopupText = (s_nCurrentWarningHyperlinkID == link.unique_id);
								}
								else if ( pgGetSourceURL(m_paigeRef, offset, reinterpret_cast<unsigned char *>(szURL), sizeof(szURL)) )
								{
									//	Get the link text
									char				szLinkText[1024];
									
									GetPgText(szLinkText, sizeof(szLinkText), link.applied_range, FALSE);
									
									//	Check to see if the link text combined with the link target is naughty
									UINT		nURLIsNaughty = CURLInfo::IsURLNaughty(szURL, szLinkText);
									
									if (nURLIsNaughty)
									{
										//	The link is naughty - so format the message that we'll popup and
										//	show the user.
										CString		szMessage;
										
										szMessage.Format( nURLIsNaughty, static_cast<LPCTSTR>(szURL), static_cast<LPCTSTR>(szLinkText) );
										
										//	Set the popup text
										if ( CPopupText::WasCreated() )
											CPopupText::Instance()->SetText(szMessage);
										
										//	Determine the rectangle of the link
										select_pair		sel = link.applied_range;
										rectangle		pgRect;
										
										pgTextRect (m_paigeRef, &sel, TRUE, FALSE, &pgRect);
										
										CRect			rectLink;
										
										RectangleToRect(&pgRect, NULL, &rectLink);
										
										ClientToScreen(&rectLink);
										
										if ( CPopupText::WasCreated() )
										{
											//	Position the popup text window based on the cursor location
											//	and the link text rectangle.
											CPopupText::Instance()->PositionWindow(rectLink, AfxGetMainWnd(), ptCursorScreen);
											
											//	Show the popup text after a short delay
											CPopupText::Instance()->ShowDelayed(5);
											
											//	Remember that we're showing the popup text
											bShowingPopupText = true;
										}
									}
								}

								bCheckedLink = true;
								s_nLastCheckedHyperlinkID = link.unique_id;
								s_nCurrentWarningHyperlinkID = bShowingPopupText ? link.unique_id : 0;
							}
						}

						pWnd->SetStatusBarText(UnEscapeURL(DisplayURL));
						bSetStatusText = true;
					}
				}
				
				if (!bSetStatusText)
					pWnd->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

				if (!hCur)
				{
					bool ptInSelection = false;
					if ( pgNumSelections( m_paigeRef ) )
					{
						shape_ref hiliteRgn = 0;
						if ( (hiliteRgn = pgRectToShape( PgMemGlobalsPtr(), NULL )) != 0 )
						{
							if ( pgGetHiliteRgn( m_paigeRef, NULL, MEM_NULL, hiliteRgn ) )
							{
								if ( pgPtInShape( hiliteRgn, &mousePt, NULL, NULL, NULL ) )
									ptInSelection = true;
							}

							pgDisposeShape( hiliteRgn );
						}
					}

					if (ptInSelection || pgPtInEmbed(m_paigeRef, &mousePt, NULL, NULL, false))
					{
						hCur = LoadCursor(NULL, IDC_ARROW);
					}
					else
					{
						if (m_mouseState & MOUSE_FORMAT_PAINTER)
							hCur = QCLoadCursor(IDC_FORMAT_PAINTER);
						else
							hCur = LoadCursor(NULL, IDC_IBEAM);
					}
				}

			}
			else if (ptCursorClient.x < m_pgBorder.left && ptCursorClient.y > m_pgBorder.top)
				hCur = QCLoadCursor(IDC_POINTER_SELECT);
		}
	}

	if (!bCheckedLink)
	{
		//	We didn't check any link - so clear out any previous link info.
		s_nLastCheckedHyperlinkID = 0;
		s_nCurrentWarningHyperlinkID = 0;
	}

	//	Cancel popup text if we're not showing anything right now
	if (!bShowingPopupText)
		CPopupText::Instance()->Cancel();

	if (hCur && SetCursor(hCur))
		return TRUE;

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CPaigeEdtView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_mouseState & MOUSE_DRAG_DROP)
    {
        //By default, Drag Drop will start after 200 millisecs or after it has
        //moved the number of pixels specified in the RECT parameter of DoDragDrop
        //But we want it to start only if the cursor has moved some pixels <ignore time factor>
        if (abs(point.x - m_mousePt.x) > 6 ||
            abs(point.y - m_mousePt.y) > 6)
        {
            COleDataSource *pDataSource = DEBUG_NEW COleDataSource();
            //Set up the Data Source for Drag and Drop
            OnCopy(pDataSource);
            DROPEFFECT dropEffect = pDataSource->DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE, NULL, NULL);
            //Let OnDrop take care of deleting selection on a intra-app move
                        
            //If dragged to a different target and it was a MOVE effect, then delete the current selection
            if ((dropEffect == DROPEFFECT_MOVE) && (!m_fRO))
            {
                if ( (m_mouseState & MOUSE_DRAG_DROP) && (pgNumSelections(m_paigeRef)) ) 
                {
                    /*if (offset > selectRight)
                      offset -= (selectRight - selectLeft);*/
                    long insertRef[2];
                    pgGetSelection(m_paigeRef, &insertRef[0], &insertRef[1]);
                    PrepareUndo(undo_drag, insertRef, FALSE);
                    // Delete the current selection since it's a MOVE operation
                    pgDelete(m_paigeRef, NULL, best_way);
                    /*verb = undo_drag_drop_move;*/
                }
            }

            //Cleanup, maybe this should be in OnDrop/OnDragLeave
            m_mouseState = 0;
            m_mousePt = CPoint(0,0);
        }
    }
    else if((m_mouseState & MOUSE_DRAG_SELECT) && (point.x < m_pgBorder.left) && (point.y > m_pgBorder.top) && (m_prevPoint.x == 0) && (m_prevPoint.y == 0))
    {
        m_prevPoint.x = point.x;
        m_prevPoint.y = point.y;                
    }   
    // Use the modifiers set in LButtonDown to make extended or discontinuous selections
    else if (m_mouseState & MOUSE_DRAG_SELECT)
    {
        m_mousePt = point;
        co_ordinate pgMousePt;
        CPointToPgPoint(&point, &pgMousePt);

		// Adjust the mouse position if necessary to correct Paige's selection
		// of embedded images. See AdjustMouseIfEmbedded for more details.
		AdjustMouseIfEmbedded(pgMousePt);

        pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
    }

    CView::OnMouseMove(nFlags, point);
}


inline bool is_word_delimiter( unsigned uChar ) {
    return (uChar == VK_SPACE || uChar == VK_RETURN || uChar == VK_TAB );   // ||
//            (ispunct((int)uChar) > 0));
}


inline bool chartype_in_range( pg_ref pg, long mask_bits, long offset, long nChars, bool toLeft ) {
    long li = offset;
    const long maxText = nChars;

    for ( int i = 0; i < maxText; i++ )
        if ( pgCharType( pg, toLeft ? li - 1 : li, mask_bits ) & mask_bits )
            return true;

    return false;
}

// adjust_text_range -- adjusts a select_pair to make sure that the "begin"
// and "end" fields fall on word boundaries.
inline void adjust_text_range( pg_ref pg, select_pair_ptr pTextRange ) {
    long offset, dummy;
    offset = pTextRange->begin;
    pgFindWord( pg, offset, &pTextRange->begin, &dummy, true, false );
    offset = pTextRange->end;
    pgFindWord( pg, offset, &dummy, &pTextRange->end, false, false );
}


void CPaigeEdtView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    TCHAR uChar = (TCHAR) nChar;
    bool bSplitExcerpt = FALSE;
    bool bBegOfExcerpt = FALSE;
    bool bEndOfMsg = FALSE;
    select_pair sel = {0,0};

	// How about we update this, huh?
	last_typed = GetTickCount();

    if( uChar == VK_ESCAPE )
	{
		//	Cancel Format Painter because escape was pressed
		m_mouseState &= ~MOUSE_FORMAT_PAINTER;
        return;
	}

    // Process the keystrokes only if the document is not read only.
    if ( m_fRO )
    {
        if (nChar == VK_SPACE)
        {
            if (ShiftDown())
            {
                // A Shift+Space on a read-only view translates into a
                // "toggle read status" command.
                SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE, LPARAM(GetSafeHwnd()));
            }
            else
            {
                // Translate a Space keypress on a read-only view 
                // into a scroll down keypress.  This is superior to
                // simulating a WM_VSCROLL scroll bar event since it
                // logically moves the cursor with the view.

                int nScrollPos = GetScrollPos(SB_VERT);
                SendMessage(WM_KEYDOWN, VK_NEXT, 1);
                SendMessage(WM_KEYUP, VK_NEXT, 1);

                if (GetScrollPos(SB_VERT) == nScrollPos)
                    SendMessage(WM_COMMAND, ID_NEXT_MESSAGE);
            }
        }

        if (uChar == PgGlobalsPtr()->line_wrap_char)
        {
            //If the caret or the beginning of the selection is
            //in the link, then process it.
            pgGetSelection(m_paigeRef, &sel.begin, MEM_NULL);
            long hLinkID = pgGetSourceID (m_paigeRef, sel.begin);

            if ( !hLinkID )
                return;

            paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

            // simply stash the hyperlink's id for later processing
            PgStuffBucket* pSB = (PgStuffBucket*) pgPtr->user_refcon;
            pSB->hlinkEvent = hLinkID;
            UnuseMemory(m_paigeRef);
            PgProcessHyperlinks(m_paigeRef);
        }

        return;
    }

	// If the character is printable, the current word is going
	// to be changed, possibly into two words.  If that happens
	// we want to be sure to both words get checked.  Thus, we'll
	// mark the whole thing right now as needing to be checked
	// SD 3/10/99
	if (isprint(uChar) && !isalnum(uChar)) m_styleEx->ClearMisspellingAndRecheck();

    
	// Keep track of where we were, for benefit of speller SD 2/24/99
	long oldChangeCount = pgGetChangeCtr(m_paigeRef);
	select_pair oldSel;
	pgGetSelection(m_paigeRef,&oldSel.begin,&oldSel.end);

    // The arrow keys are processed in OnKeyDown
    // Only traps Chars with an Ascii code. The rest are trapped in KeyDown.
    //
    // Pre-process Space, Enter keys for "Is the AutoURL going to go insane?" check

    long length = 0;
    pg_hyperlink autoLink;
    BOOL URLdirty = FALSE;

    autoLink.applied_range.begin = autoLink.applied_range.end = 0;

	if ( (uChar == PgGlobalsPtr()->line_wrap_char) || ((nChar == VK_SPACE)) )
    {
        long linkID;

        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
        
        if ( linkID = pgGetSourceID(m_paigeRef, sel.begin) )
        {
            pgGetHyperlinkSourceInfo(m_paigeRef, sel.begin, linkID, TRUE, &autoLink);

            if (autoLink.type & HYPERLINK_EUDORA_AUTOURL)
            {
                if ((sel.begin > autoLink.applied_range.begin) && (sel.begin < autoLink.applied_range.end))
                {
                    if (autoLink.applied_range.begin > 0)
                    {           
                        long charInfo = pgCharType(m_paigeRef, autoLink.applied_range.begin - 1,
                                                   BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

                        if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
                            URLdirty = TRUE;
                    }
                    else URLdirty = TRUE;
                }
            }
        }
    }


    // BOG: hack for overwrite mode. doesn't appear there's any built-in
    // way to do this. so here's my solution. i'm so proud.
    bool bDontOverwrite = ((uChar == PgGlobalsPtr()->line_wrap_char) ||
                           (uChar == PgGlobalsPtr()->bs_char));

    if ( g_bOverwriteMode && !bDontOverwrite ) {
        long begin, end;
        pgGetSelection( m_paigeRef, &begin, &end );

        if ( begin == end ) {
            long chType = pgCharType( m_paigeRef, begin, PAR_SEL_BIT );

            if ( !(chType & PAR_SEL_BIT) && end != pgTextSize(m_paigeRef)) {
                //PrepareUndo( undo_fwd_delete );
                PrepareUndo( undo_fwd_delete, MEM_NULL, TRUE );
                unsigned char chFwdDel = (TCHAR) PgGlobalsPtr()->fwd_delete_char;
                pgInsert( m_paigeRef, (pg_byte_ptr)&chFwdDel, 1, CURRENT_POSITION,
                          key_insert_mode, 0, best_way );
            }
            else
                bDontOverwrite = TRUE;
        }
        else
            bDontOverwrite = TRUE;
    }


    while (nRepCnt--) 
    {
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

        // Backspace
        if (uChar == PgGlobalsPtr()->bs_char) 
        {
            if (pgNumSelections(m_paigeRef))
                CleanURLonPosteriorDelete(sel.begin);

            PrepareUndo(undo_backspace);
        }
        else
        {
            // Ctrl + Backspace: already taken care of in OnKeyDOwn
            if ((GetKeyState(VK_CONTROL) < 0) && (uChar == 127)) 
                break;
            else
            {
                // Clear style (Plain Text)
                if ((nChar == VK_SPACE) && ( GetKeyState(VK_CONTROL)<0 ) )
                {
                    OnClearFormatting();

					// Fix for bug #5002
					// Get out of here right now because call to ClearMisspellingAndRecheck()
					// down below causes formatting not to be cleared.
                    return;
                }
                else
                {
                    if ( (nChar == VK_TAB) && (GetIniShort( IDS_INI_TAB_SPACES )) )
                    {
                        long numSpaces = (long)TabToSpaces();
                        if (numSpaces)
                            PrepareUndo(undo_insert, &numSpaces);

                        uChar = (TCHAR) VK_SPACE;

                        //Insert n-1 spaces, the last one is inserted below
                        for (int count = 1; count < numSpaces; count++)
                            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION,
                                      key_insert_mode, 0, draw_none );
                    }
                    else
                    {
                        // NewLine: In an excerpt, must split the excerpt in two
                        // and thus will require a series of carriage returns inserted.
                        if (uChar == PgGlobalsPtr()->line_wrap_char )
                        {
                            if ( bSplitExcerpt = m_styleEx->IsExcerpt(&sel) )
                            {
                                select_pair tempSel;
                                if (sel.begin > 0)
                                    tempSel.begin = tempSel.end = sel.begin - 1;
                                if (( sel.begin == 0 ) || ( ! m_styleEx->IsExcerpt(&tempSel) ))
                                {
                                    length = 2;
                                    bBegOfExcerpt = TRUE;
                                }
                                else
                                {
                                    length = 4;

                                    // If next character is a newline then insert one less new line.
                                    if (pgCharType(m_paigeRef, sel.end,PAR_SEL_BIT))
                                        length--;                   
                                    if ( bEndOfMsg= (sel.end == pgTextSize(m_paigeRef)) )
                                        length--;
                                }

                                PrepareUndo(undo_insert,&length);
                            }
                            else
                                PrepareUndo(undo_typing);

                            // Bullet list processing.  The bullet list style gets turned off if the following 
                            // conditions are met:
                            // 1) Nothing is selected; 2) The caret is in a non excerpted, bulleted list;
                            // 3) the caret is the first and only character on the line; and 4) the line
                            //    is the last line of a bulleted list.  Whew!

                            if ( (sel.begin==sel.end) && m_styleEx->IsBullet(&sel) && !bSplitExcerpt )
                            {
                                if (( sel.begin>0 ) && 
                                    ( pgCharType(m_paigeRef, sel.begin-1, PAR_SEL_BIT) & PAR_SEL_BIT) &&
                                    (( pgCharType(m_paigeRef, sel.begin,   PAR_SEL_BIT) & PAR_SEL_BIT) ||
                                     ( sel.begin >= pgTextSize(m_paigeRef) ) ) )
                                {
                                    select_pair selLine;
                                    pgFindLine(m_paigeRef, sel.begin, &selLine.begin, &selLine.end);
                                    selLine.begin = selLine.end;

                                    if (( selLine.end >= pgTextSize(m_paigeRef)) || !m_styleEx->IsBullet(&selLine) )
                                    {
                                        m_styleEx->ApplyBullet(FALSE);
                                        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
                                        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, FALSE);
                                    }
                                }
                            }
                        }
                        else    // All other characters
                        {
                            if ( g_bOverwriteMode && !bDontOverwrite )
                                PrepareUndo(undo_overwrite);
                            else
                                PrepareUndo(undo_typing);
                        }
                    }
                }
            }
        }


#ifdef NEVER	// this is now done at the bottom of the fn SD 2/24/99
        //
        // Check for Misspelled word.  Unmark word because it has now been modified.
        //

		sel2.begin = max(0,sel.begin-1);
        sel2.end = sel.end;

		m_styleEx->ClearMisspellingAndRecheck(&sel2);
#endif

        if ( is_word_delimiter(nChar) )
		{
            long begin, end;
            pgGetSelection( m_paigeRef, &begin, &end );

            if (begin == end)
			{
                long	beginUrl = PgScanBackwardsToJustAfterBlank(m_paigeRef, end);
                MakeAutoURLSpaghetti(beginUrl, end);

				QueueDirtyPair(beginUrl, end);
				m_checkdastuff = true;
            }
        }

                
        // Insert the character
        if ( !bSplitExcerpt )
        {
            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                      key_insert_mode, 0, best_way );
        }
        else
        {
            // Break the excerpt in two
            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                      key_insert_mode, 0, best_way );

            style_info  sinfo;
            font_info   fontInfo;
            sinfo = PgGlobalsPtr()->def_style;
            fontInfo = PgGlobalsPtr()->def_font;

            for (int i=0; i< length-1; i++)
            {
                pgSetInsertionStyles(m_paigeRef, &sinfo,&fontInfo);
                pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                          key_insert_mode, 0, best_way );
            }

            //
            // In the "broken" region, clear excerpting, paragraph and font styles
            //
 
            select_pair sel2;
            if (!bBegOfExcerpt)
            {
                sel2.begin = sel.begin+1;
                sel2.end = sel.begin+4;
            }
            else
            {
                sel2.begin = sel.begin;
                sel2.end = sel.begin + 2;
            }
            par_info parInfo, parInfoMask;
            long styleID;

            styleID = pgGetNamedStyleIndex(m_paigeRef, body_style);
            if (styleID)
                pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
            else
                parInfo = PgGlobalsPtr()->def_par;

            pgFillBlock( &parInfoMask, sizeof(par_info), SET_MASK_BITS );
            pgSetParInfo( m_paigeRef, &sel2, &parInfo, &parInfoMask, best_way );

            // Reset Caret in proper position
            if ( bEndOfMsg )
            {
                pgSetSelection(m_paigeRef, sel2.end, sel2.end, 0, FALSE);
                ClearStyle();
                ClearFormat();  

                // Clear the evil excerpt bar that didn't want to die (bug 5383)
                select_pair sel3;
                sel3.begin = sel2.end;
                sel3.end = sel2.end;
                pgSetParInfo( m_paigeRef, &sel3, &parInfo, &parInfoMask, best_way );
            }

            if (sel.begin == 0)
                pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);      
            else
                pgSetSelection(m_paigeRef, sel.begin+2, sel.begin+2, 0, TRUE);
            Invalidate();
            bSplitExcerpt=FALSE;

        }
    }

	//Flag Mood Mail Dirty
	m_bMoodMailDirty = true;
        
    //
    //  AutoURL Post-Processing
    //

    if (URLdirty)
    {
        select_pair tempSel;
        char buf[INTERNET_MAX_URL_LENGTH];
        BOOL firstURLStillGood = FALSE, secondURLStillGood = FALSE;
        long linkEnd = autoLink.applied_range.end + 1 + length;

        tempSel.begin = autoLink.applied_range.begin;
        tempSel.end = sel.begin;

        GetPgText(buf, sizeof(buf), tempSel, FALSE);
        firstURLStillGood = ValidURLScheme(buf);

        if (firstURLStillGood)
        {
            autoLink.applied_range.end = sel.begin;

            if ( URLIsValid(buf) )
            {
                char tempBuf[INTERNET_MAX_URL_LENGTH];
                strncpy(tempBuf, EscapeURL(buf), sizeof(tempBuf));
                tempBuf[sizeof(tempBuf) - 1] = 0;
                pgChangeHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, 
                                        &autoLink.applied_range, (unsigned char*)tempBuf, NULL, NULL, 
                                        NULL, NULL, NULL, best_way);

            }
        }
                
        tempSel.begin = sel.begin + 1 + length;
        tempSel.end = linkEnd;
        GetPgText(buf, sizeof(buf), tempSel, FALSE);
        secondURLStillGood = ValidURLScheme(buf);

        if (secondURLStillGood)
        {
            if ( URLIsValid(buf) )
            {
                if (firstURLStillGood)
                {
                    paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
                    autoLink.unique_id = pgAssignLinkID(pgPtr->hyperlinks);
                    UnuseMemory(m_paigeRef);
                }
                else 
                    pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);

                autoLink.applied_range = tempSel;
                char tempBuf[INTERNET_MAX_URL_LENGTH];
                strncpy(tempBuf, EscapeURL(buf), sizeof(tempBuf));
                tempBuf[sizeof(tempBuf) - 1] = 0;
                pgChangeHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, 
                                        &autoLink.applied_range, (unsigned char *)tempBuf, NULL, NULL, 
                                        NULL, NULL, NULL, best_way);

            }
                
        }

        if (!firstURLStillGood && !secondURLStillGood)
            pgDeleteHyperlinkSource(m_paigeRef, sel.begin, TRUE);
    }

    // If new-line or linefeed char, may have to scroll
//    if ((uChar == PgGlobalsPtr()->line_wrap_char) || (uChar == PgGlobalsPtr()->soft_line_char))
//    {
//        ScrollToCursor();
//    }
//    else {    // Temp fix, should really scroll only during word-wrap
    ScrollToCursor();
//    }

    CView::OnChar(nChar, nRepCnt, nFlags);

	// if we've changed the doc, recheck the current word
	if (oldChangeCount!=pgGetChangeCtr(m_paigeRef)) 
		m_styleEx->ClearMisspellingAndRecheck();
	// if we've changed which word is current, go check stuff
	else if (!m_checkdastuff)
		m_checkdastuff = InDifferentWord(m_paigeRef,&oldSel);
}


void CPaigeEdtView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    TCHAR uChar = 0;
    BOOL callPaige = FALSE;
    long wordLeft = 0, wordRight = 0, lineRight, lineLeft, paraBegin, paraEnd;
    BOOL ControlDown, ShiftDown, gottaScroll = FALSE;
    co_ordinate curCoOrd;
    CPoint curPoint;
    long anchor, scrollOffset = CURRENT_POSITION;
	long oldChangeCount = pgGetChangeCtr(m_paigeRef);

    // Map the virtual keycode to paige default values for non-ascii
    //     chars. Also check if the CONTROL and SHIFT keys are down.
    ControlDown = (GetKeyState(VK_CONTROL) < 0);
    ShiftDown = (GetKeyState(VK_SHIFT) < 0);

    //
    // Handle navigation keys for read-only views, *before* we do any 
    // Paige memory allocations and other nasty stuff.
    //
    if (m_fRO)
    {
        BOOL PlainArrows = GetIniShort(IDS_INI_PLAIN_ARROWS) != 0;
        BOOL ControlArrows = GetIniShort(IDS_INI_CONTROL_ARROWS) != 0;

        switch (nChar)
        {
        case VK_LEFT:
        case VK_UP:
            if ((PlainArrows && !ControlDown) ||
                (ControlArrows && ControlDown))
            {
                SendMessage(WM_COMMAND, ID_PREVIOUS_MESSAGE);
                return;
            }
            break;
        case VK_DOWN:
        case VK_RIGHT:
            // Check for special navigation keys
            if ((PlainArrows && !ControlDown) ||
                (ControlArrows && ControlDown))
            {
                SendMessage(WM_COMMAND, ID_NEXT_MESSAGE);
                return;
            }
            break;
        default:
            // fall through to normal Paige keystroke handling below...
            break;
        }
    }

	select_pair		oldSel;
	pgGetSelection(m_paigeRef,&oldSel.begin,&oldSel.end);	// keep track of old selection SD 2/22/99

   {   // BOG: see if anything needs spell-checking
        if ( !ShiftDown /* && !m_fRO */ ) {
            CPaigeStyle objStyle( m_paigeRef );

			// Selection hasn't changed yet - just use the selection we retrieved above
            select_pair textRange = oldSel;

			long offset = textRange.begin;

            if ( nChar == VK_LEFT || nChar == VK_RIGHT ) {
                bool bToLeft = (nChar == VK_LEFT);

                // for vk_right, we hack textrange to point to previous char
                if ( !bToLeft )
                    textRange.begin = textRange.end = (offset - nRepCnt < 0) ? 0 : offset - nRepCnt;

                if ( objStyle.NeedsScanned( &textRange ) )
                    if ( chartype_in_range( m_paigeRef, BLANK_BIT, offset, nRepCnt, bToLeft ) )
                        m_checkdastuff = true;
            }
            else if ( nChar == VK_UP || nChar == VK_DOWN ) {

                // for up/down we hack textrange to point to previous char if the next char
                // is a blank; otherwise, word won't get checked.
                if ( chartype_in_range( m_paigeRef, BLANK_BIT, offset, 1, false ) )
                    textRange.begin = textRange.end = (offset - nRepCnt < 0) ? 0 : offset - nRepCnt;

                if ( objStyle.NeedsScanned( &textRange ) )
                    m_checkdastuff = true;
            }
        }
    }   // BOG: end spellcheck hack

        
    paige_rec_ptr pgPtr  = NULL;
    pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

	// Selection hasn't changed yet, use selection we retrieved above
	long	selectLeft, selectRight;
	selectLeft = oldSel.begin;
	selectRight = oldSel.end;

    if (pgNumSelections(m_paigeRef)) {
        //Get the original anchor which started the selection
        //pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
        anchor = pgPtr->hilite_anchor;
    }
    else anchor = selectRight;  //just the plain ol' caret

    // Map the virtual keycode to paige default values for non-ascii chars
    switch(nChar)
    {
    case VK_LEFT : 
        // Custom FindWord, uses Paige's pgFindWord, left_side and smart_select MUST be TRUE
        if (ControlDown) {
            if (selectRight > anchor)
                FindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, TRUE, TRUE);
            else
                FindWord(m_paigeRef, selectLeft, &wordLeft, &wordRight, TRUE, TRUE);
            scrollOffset = wordLeft;
        }

        if (ShiftDown) {
            //Select to end of word (CTRL+SHIFT) or one char (SHIFT)
            if (ControlDown) {
                if (selectRight > anchor) //DeSelect
                {
                    pgSetSelection(m_paigeRef, selectLeft, wordLeft, 0, TRUE);
                    pgPtr->hilite_anchor = selectLeft;
                }
                else
                    pgExtendSelection(m_paigeRef, wordLeft - selectLeft, EXTEND_MOD_BIT, TRUE);
            }
            else {                                      
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->left_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                if (selectRight > anchor)
                    scrollOffset = selectRight - 1;
                else 
                    scrollOffset = selectLeft - 1;
            }
        }
        else if (ControlDown)
            pgSetSelection(m_paigeRef, wordLeft, wordLeft, 0, TRUE);
        else {uChar = (TCHAR) PgGlobalsPtr()->left_arrow_char/*0x1C*/;  callPaige = TRUE;}
        gottaScroll = TRUE; break;
        
    case VK_RIGHT :             
        //Testing
        /*pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, FALSE, TRUE);
          pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, FALSE, FALSE);
          pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, TRUE, TRUE);
          pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, TRUE, FALSE);*/

        // Edited FindWord (uses Paige's pgFindWord)
        // Get word boundaries, Select trailing blanks also to make it look
        //       like we are navigating to beginning of next word instead of end of current word
        // left_side MUST be FALSE, smart_select MUST be TRUE for the function
        if (ControlDown) {
            if (selectLeft < anchor)
                FindWord(m_paigeRef, selectLeft, &wordLeft, &wordRight, FALSE, TRUE);
            else
                FindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, FALSE, TRUE);
            scrollOffset = wordRight;
        }

        if (ShiftDown) {
            if (ControlDown) {                          
                if (selectLeft < anchor) //DeSelect
                {
                    pgSetSelection(m_paigeRef, selectRight, wordRight, 0, TRUE);                                        
                    pgPtr->hilite_anchor = selectRight;
                }
                else 
                    pgExtendSelection(m_paigeRef, wordRight - selectRight, EXTEND_MOD_BIT, TRUE);                                                                                               
            }
            else {                              
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->right_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                if (selectLeft < anchor)
                    scrollOffset = selectLeft + 1;
                else 
                    scrollOffset = selectRight + 1;
            }
        }
        else if (ControlDown) 
            pgSetSelection(m_paigeRef, wordRight, wordRight, 0, TRUE);
        else {  uChar = (TCHAR) PgGlobalsPtr()->right_arrow_char/*0x1D*/; callPaige = TRUE; }
        gottaScroll = TRUE; break;
        
    case VK_UP : 
		paraBegin = paraEnd = 0;
        if (ControlDown) {
            if (selectRight > anchor) {
                pgFindPar(m_paigeRef, selectRight, &paraBegin, &paraEnd);
                if (paraBegin == selectRight)
                    pgFindPar(m_paigeRef, selectRight-1, &paraBegin, &paraEnd);
                if (paraBegin != 0) 
                    scrollOffset = paraBegin - 1;
            }
            else {
                if (selectLeft == 0) break;
                pgFindPar(m_paigeRef, selectLeft, &paraBegin, &paraEnd);
                                //If we are already at the beginning of a paragraph, go to the previous one
                if (paraBegin == selectLeft)
                    pgFindPar(m_paigeRef, selectLeft-1, &paraBegin, &paraEnd);
                scrollOffset = paraBegin;
            }
        }

        if (ShiftDown) {
            // Select to beginning of the paragraph
            if (ControlDown) {
                if (selectRight > anchor) {//DeSelect
                    if (selectLeft > paraBegin)
                        pgSetSelection(m_paigeRef, paraBegin, selectLeft, 0, TRUE);
                    else
                        pgSetSelection(m_paigeRef, selectLeft, paraBegin, 0, TRUE);
                                
                    pgPtr->hilite_anchor = selectLeft;
                }
                else
                    pgExtendSelection(m_paigeRef, paraBegin - selectLeft, EXTEND_MOD_BIT, TRUE);
            }
            else
            {
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->up_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                long selLeft, selRight;
                pgGetSelection(m_paigeRef, &selLeft, &selRight);
                                //If it's stuck in the middle of the first line, select to the beginning
                if ((selRight == selectRight) && (selLeft == selectLeft))
                {
                    if ( selLeft != 0 )
                    {
                        if (selLeft <= pgPtr->hilite_anchor)
                            pgExtendSelection(m_paigeRef, -selLeft, EXTEND_MOD_BIT, TRUE);
                    }
                    else
                    {
                        if ( selRight > pgPtr->hilite_anchor )
                            pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
                    }
                }
                if (selRight > anchor)
                    scrollOffset = selRight;
                else 
                    scrollOffset = selLeft;
            }
        }
        else if (ControlDown) {
            // Navigate to beginning of the Current paragraph
            pgSetSelection(m_paigeRef, paraBegin, paraBegin, 0, TRUE);
        }
        else { uChar = (TCHAR) PgGlobalsPtr()->up_arrow_char/*0x1E*/; callPaige = TRUE;}
        gottaScroll = TRUE; break;
        
    case VK_DOWN : 
		paraBegin = paraEnd = 0;
        if (ControlDown) {
            pgFindPar(m_paigeRef, selectLeft < anchor? selectLeft : selectRight, &paraBegin, &paraEnd);
            scrollOffset = paraEnd;
        }

        if (ShiftDown) {
            // Select to the end of the current paragraph
            if (ControlDown) {
                if (selectLeft < anchor) { //DeSelect
                    if (paraEnd > selectRight)
                        pgSetSelection(m_paigeRef, selectRight, paraEnd, 0, TRUE);
                    else
                        pgSetSelection(m_paigeRef, paraEnd, selectRight, 0, TRUE);

                    pgPtr->hilite_anchor = selectRight;
                }
                else
                    pgExtendSelection(m_paigeRef, paraEnd - selectRight, EXTEND_MOD_BIT, TRUE);
            }
            else
            {
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->down_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                long selLeft, selRight;
                pgGetSelection(m_paigeRef, &selLeft, &selRight);
                if ((selRight == selectRight) && (selLeft == selectLeft))
                {
                    long textSize = pgTextSize(m_paigeRef);
                    if ( selRight != textSize )
                    {
                        if (selRight >= pgPtr->hilite_anchor)
                            pgExtendSelection(m_paigeRef, textSize, EXTEND_MOD_BIT, TRUE);
                    }
                    else
                    {
                        if ( selLeft < pgPtr->hilite_anchor )
                            pgSetSelection(m_paigeRef, textSize, textSize, 0, TRUE);
                    }                                   
                }
                if (selLeft < anchor)
                    scrollOffset = selLeft;
                else 
                    scrollOffset = selRight;
            }
        }
        else if (ControlDown) {
            // Navigate to the beginning of the next paragraph
            pgSetSelection(m_paigeRef, paraEnd, paraEnd, 0, TRUE);
        }
        else { uChar = (TCHAR) PgGlobalsPtr()->down_arrow_char/*0x1F*/; callPaige = TRUE; }
        gottaScroll = TRUE; break; 
        
     case VK_DELETE : 
        if (m_fRO) break;
        if (ControlDown) {
            select_pair selectPair;
            //Delete word to the Right
            FindWord(m_paigeRef, selectRight, &selectPair.begin, &selectPair.end, FALSE, TRUE);
            selectPair.begin = selectRight;
            if (pgNumSelections(m_paigeRef))
                CleanURLonPosteriorDelete(selectLeft);
            PrepareUndo(undo_delete, (long *)&selectPair);
            pgDelete(m_paigeRef, &selectPair, best_way);
        }
        else 
        {
            uChar = (TCHAR) PgGlobalsPtr()->fwd_delete_char;/*0x7F*/; 
            if (pgNumSelections(m_paigeRef))
            {
                CleanURLonPosteriorDelete(selectLeft);
                PrepareUndo(undo_delete);
            }
            else
                PrepareUndo(undo_fwd_delete);
            callPaige = TRUE; 
        }
        gottaScroll=TRUE;
        //UpdateScrollBars(/*GetScrollBarCtrl(SB_VERT)*/);
		//Flag Mood Mail Dirty
		m_bMoodMailDirty = true;
        break;

    case VK_BACK :
        if (m_fRO) break;
        //Trap Ctrl_BackSpace here, Backspace itself is trapped in OnChar
        if (ControlDown) {
            select_pair selectPair;
            //Delete word from the current position to the beginning of the current word
            FindWord(m_paigeRef, selectLeft, &selectPair.begin, &selectPair.end, TRUE, TRUE);
            selectPair.end = selectLeft;
            if (pgNumSelections(m_paigeRef))
                CleanURLonPosteriorDelete(selectLeft);
            PrepareUndo(undo_delete, (long *)&selectPair);
            pgDelete(m_paigeRef, &selectPair, best_way);
        }
		//Flag Mood Mail Dirty
		m_bMoodMailDirty = true;
        break;

    case VK_END : 
        if (ShiftDown)
        {
            if (ControlDown) {
                if (selectLeft < anchor)
                    pgSetSelection(m_paigeRef, selectRight, pgTextSize(m_paigeRef), 0, TRUE);
                else
                    pgSetSelection(m_paigeRef, selectLeft, pgTextSize(m_paigeRef), 0, TRUE);

                scrollOffset = pgTextSize(m_paigeRef);
            }
            else {
                pgSetSelection(m_paigeRef, selectRight, selectRight, 0, TRUE);
                pgSetCaretPosition(m_paigeRef, end_line_caret, TRUE);
                long currPos;
                pgGetSelection(m_paigeRef, &currPos, MEM_NULL);
                if (selectLeft < anchor)
                    pgSetSelection(m_paigeRef, selectRight, currPos, 0, TRUE);
                else
                    pgSetSelection(m_paigeRef, selectLeft, currPos, 0, TRUE);
                                //Reset the hilite anchor to the old one, since it's confused by now!!
                pgPtr->hilite_anchor = anchor;
            }
        }
        else if (ControlDown) {
            pgSetCaretPosition(m_paigeRef, doc_bottom_caret, TRUE);
            pgPtr->hilite_anchor = pgTextSize(m_paigeRef);
        }
        else {
            if ( ( pgNumSelections(m_paigeRef) ) /*|| ( selectRight != pgTextSize(m_paigeRef) ) */)
            {
                                //pgSetCaretPosition(m_paigeRef, end_line_caret, TRUE);
                pgFindLine(m_paigeRef, selectRight-1, &lineLeft, &lineRight);
                if (lineRight != pgTextSize(m_paigeRef))
                    --lineRight;
                pgSetSelection(m_paigeRef, lineRight, lineRight, 0, TRUE);
            }
            else if ( selectRight != pgTextSize(m_paigeRef) )
                pgSetCaretPosition(m_paigeRef, end_line_caret, TRUE);
            pgGetSelection(m_paigeRef, MEM_NULL, &selectRight);
            pgPtr->hilite_anchor = selectRight;
        }
        gottaScroll = TRUE;
        break;
        
    case VK_HOME :
        if (ShiftDown)
        {
            if (ControlDown) {
                if (selectRight > anchor)
                    pgSetSelection(m_paigeRef, 0, selectLeft, 0, TRUE);
                else 
                    pgSetSelection(m_paigeRef, 0, selectRight, 0, TRUE);
                scrollOffset = 0;
            }
            else {
                pgFindLine(m_paigeRef, selectLeft, &lineLeft, &lineRight);
                if (selectRight > anchor)
                    pgSetSelection(m_paigeRef, lineLeft, selectLeft, 0, TRUE);
                else
                {       //
                    // pgFindLine returns end-of-text values if caret is last character in msg.
                    //
                    if ( (lineLeft == selectLeft) && (selectLeft == pgTextSize(m_paigeRef)) )
                    {
                        select_pair sel;
                        pgSetCaretPosition(m_paigeRef, begin_line_caret, TRUE);
                        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
                        pgSetSelection(m_paigeRef, selectRight, sel.begin, 0, TRUE);
                    }
                    else
                        pgExtendSelection(m_paigeRef, lineLeft - selectLeft, EXTEND_MOD_BIT, TRUE);
                }
            }
        }
        else if (ControlDown) {
            pgSetCaretPosition(m_paigeRef, home_caret, TRUE);
            pgPtr->hilite_anchor = 0;
        }
        else 
        {
            if ( pgNumSelections(m_paigeRef) )
            {
                pgFindLine(m_paigeRef, selectLeft, &lineLeft, &lineRight);
                pgSetSelection(m_paigeRef, lineLeft, lineLeft, 0, TRUE);
            }
            else
            {
                pgSetCaretPosition(m_paigeRef, begin_line_caret, TRUE);
                pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
                pgPtr->hilite_anchor = selectLeft;
            }
        }
        gottaScroll = TRUE;
        break;

    case VK_PRIOR :
    {
        long offset;

        if ( pgNumSelections(m_paigeRef) )
        {
            if (selectLeft == anchor)
                pgSetSelection(m_paigeRef, selectRight, selectRight, 0, TRUE);
            else
                pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }

        curPoint = GetCaretPos();
        //Fill Paige coord struct
        CPointToPgPoint(&curPoint, &curCoOrd);
                        
        // Safe to pass 0 for nPos since it's not gonna be used unless 1st arg is SB_THUMBTRACK
        OnVScroll(SB_PAGEUP, 0, GetScrollBarCtrl(SB_VERT));
                        
        offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                        
        // No more pages above, set cursor to the beginning of the text
        if (offset == selectLeft)
            offset = 0;

        if (ShiftDown)
        {
            if ( !pgPtr ) 
                pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

            if (selectRight > anchor) //Deselecting
            {
                if (offset == selectRight)
                    offset = 0;
                pgSetSelection(m_paigeRef, selectLeft, offset, 0, TRUE);
                pgPtr->hilite_anchor = selectLeft;
            }
            else
            {
                pgSetSelection(m_paigeRef, selectRight, offset, 0, TRUE);
                pgPtr->hilite_anchor = selectRight;
            }
        }               
        else
            pgSetSelection(m_paigeRef, offset, offset, 0, TRUE);
                        
        //SetPgAreas();

        break;
    }
    case VK_NEXT :
    {
        long offset;

        if ( pgNumSelections(m_paigeRef) )
        {
                                //Might cause a flicker but this is the only way to get a caret.
            if (selectLeft == anchor)
                pgSetSelection(m_paigeRef, selectRight, selectRight, 0, TRUE);
            else
                pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }

        curPoint = GetCaretPos();
        //Fill Paige coord struct
        CPointToPgPoint(&curPoint, &curCoOrd);

        OnVScroll(SB_PAGEDOWN, 0, GetScrollBarCtrl(SB_VERT));
                        
        offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                        
        // No more pages below, set cursor to the end of the text
        if (offset == selectRight)
            offset = pgTextSize(m_paigeRef);

        if (ShiftDown)
        {
            if ( !pgPtr ) 
                pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

            if (selectLeft < anchor) //Deselect
            {
                if (offset == selectLeft)
                    offset = pgTextSize(m_paigeRef);
                pgSetSelection(m_paigeRef, selectRight, offset, 0, TRUE);
                pgPtr->hilite_anchor = selectRight;
            }
            else
            {
                pgSetSelection(m_paigeRef, selectLeft, offset, 0 , TRUE);
                pgPtr->hilite_anchor = selectLeft;
            }
        }
        else
            pgSetSelection(m_paigeRef, offset, offset, 0, TRUE);

        //Make sure you don't see half of the cursor, 
        // incase cursor is on the first line in the vis-area
        //gottaScroll = TRUE;
        //SetPgAreas();

        break;
    }
    }

    if (callPaige == TRUE)
    {
        while (nRepCnt--)
            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, sizeof(unsigned char), 
                      CURRENT_POSITION, key_insert_mode, 0, best_way );
    }
        
 	// if we've changed the doc, recheck the current word
	if (oldChangeCount!=pgGetChangeCtr(m_paigeRef)) 
		m_styleEx->ClearMisspellingAndRecheck();
	// if we've changed which word is current, go check stuff
	else if (!m_checkdastuff)
		m_checkdastuff = InDifferentWord(m_paigeRef,&oldSel);
	
	if (gottaScroll) ScrollToCursor(scrollOffset);
        
    if (pgPtr) UnuseMemory(m_paigeRef);
    //CView::OnKeyDown(nChar, nRepCnt, nFlags);

    // BOG: Text selections resulting from find operations remain visible even when the
    // editor window loses focus, but this causes user selections to also behave this
    // way unless we employ some criteria for canceling out of this mode. Here it is.
    // I think this is a reasonably complete set---add more if ya think of 'em!

    if ( (uChar == VK_LEFT || uChar == VK_RIGHT || uChar == VK_UP || uChar == VK_DOWN) ||
         (!m_fRO && (uChar == VK_RETURN || uChar == VK_TAB || isascii(uChar))) ) {

        m_bDontDeactivateHighlight = FALSE;
    }
		//Flag Mood Mail Dirty
/*	if (g_bMoodMailCheck )
	{
		m_bMoodMailDirty = true;
	}*/
}

// Has the insertion point moved to a new word?
// This is really only meant to work if the document
// hasn't changed since oldSel was taken; if you call
// it after changing the document, phhhhttttt!!!! to you.  SD 2/24/99
bool InDifferentWord(pg_ref paigeRef, select_pair_ptr oldSel)
{
	select_pair newSel;
	bool result = false;

	// get current selection
	pgGetSelection(paigeRef,&newSel.begin,&newSel.end);

	//SDSpellTRACE("raw %d.%d %d.%d ",oldSel->begin,oldSel->end,newSel.begin,newSel.end);

	// we only do this if the selection is an insertion point
	// we can only be in a different word if the selection has changed
	if (newSel.begin==newSel.end && (newSel.begin!=oldSel->begin || newSel.end!=oldSel->end))
	{
		// ok, we have an insertion point and it's not in the same place as it was

		// expand the offsets to word boundaries
		pgFindWord(paigeRef, newSel.begin, &newSel.begin,&newSel.end,TRUE,FALSE);
		pgFindWord(paigeRef, oldSel->begin, &oldSel->begin,&oldSel->end,TRUE,FALSE);
		// TRACE("SD word %d.%d %d.%d ",oldSel->begin,oldSel->end,newSel.begin,newSel.end);

		// Are we in a new spot? 
		if (newSel.begin!=oldSel->begin)
			result = true;
	}

	//SDSpellTRACE(" %s\n",result?"diff":"same");

	return result;
}

// Get the offsets of a single word that either contains the selection or
// has the selection on its ends.  Returns true if there is such a single word,
// and fills in theWord with its coordinates.  If there is no such word, returns
// false and puts -1,-1 into theWord
bool GetWordContainingSelection(pg_ref paigeRef,select_pair_ptr theWord)
{
	select_pair beginWord, endWord;

	// get the selection
	pgGetSelection(paigeRef,&theWord->begin,&theWord->end);
	//SDSpellTRACE("GWCS %d.%d",theWord->begin,theWord->end);

	// and figure out the word it starts in
	pgFindWord(paigeRef,theWord->begin,&beginWord.begin,&beginWord.end,true,false);

	// if it's not an insertion point, figure out the word it ends in
	if (theWord->begin != theWord->end)
	{
		pgFindWord(paigeRef,theWord->end,&endWord.begin,&endWord.end,true,false);

		// if the words aren't the same, bail!
		if (endWord.begin != beginWord.begin)
		{
			theWord->begin = theWord->end = -1;
			//SDSpellTRACE(" false\n",theWord->begin,theWord->end);
			return false;
		}
	}

	// Ok, we found a word!
	*theWord = beginWord;
	//SDSpellTRACE(" %d.%d true\n",theWord->begin,theWord->end);
	return true;
}

// Do two selection pairs intersect?
bool SelPairIntersects(select_pair_ptr sel1, select_pair_ptr sel2)
{
	if (sel1->end <= sel2->begin) return false;
	if (sel1->begin >= sel2->end) return false;
	return true;
}

void CPaigeEdtView::ScrollToCursor(long offset)
{
    if (offset == CURRENT_POSITION)
    {
        CPoint curPoint;
        //Get current caret position (client co_ords)
        curPoint = GetCaretPos();

        long left;
        rectangle rect;
        pgGetSelection(m_paigeRef, &left, MEM_NULL);
        pgCharacterRect(m_paigeRef, left, FALSE, FALSE, &rect);
        CRect clientRect;
        GetClientRect(&clientRect);
        clientRect.DeflateRect(m_pgBorder);
        if (rect.bot_right.v > clientRect.bottom )
            curPoint.y = rect.bot_right.v;

        //Fill Paige coord struct
        co_ordinate curCoOrd;
        CPointToPgPoint(&curPoint, &curCoOrd);

        //Do we need to scroll?
        short posBits = 0;
        posBits = pgPtInView(m_paigeRef, &curCoOrd, NULL);
        if (!(posBits & WITHIN_VIS_AREA))
        {
            if (!pgNumSelections(m_paigeRef)) {
                pgPaginateNow(m_paigeRef, CURRENT_POSITION, FALSE);
                if (pgScrollToView(m_paigeRef, CURRENT_POSITION, 0, 0, FALSE, best_way)) {
                    UpdateScrollBars();
                }
            }
            else UpdateScrollBars();
        }
        else
            UpdateScrollBars();
    }
    else 
    {
        pgPaginateNow(m_paigeRef, offset, FALSE);
        if (pgScrollToView(m_paigeRef, offset, 0, 0, TRUE, best_way)) {
            UpdateScrollBars();
        }

    }
}

void CPaigeEdtView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    // BOG: this is a "re-entrant" hack. if we get called again in the middle
    // of hyperlink processing, bad things can/will happen.
    {
        paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        UnuseMemory( m_paigeRef );

        if ( pSB->hlinkProcessing == true ) {
            assert( 0 );
            return;
        }
    }   // BOG: end re-entrant prophylactic

    co_ordinate pgMousePt;
    CPointToPgPoint(&point, &pgMousePt);

    SetCapture();
    m_pgModifiers |= WORD_MOD_BIT;
    pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

	// BOGOLE: if we just double-clicked an embedded ole object, then it's
	// time to activate that puppy!
	embed_ref er = pgPtInEmbed( m_paigeRef, &pgMousePt, 0, 0, false );

	if ( er ) {
		pg_embed_ptr per = (pg_embed_ptr) UseMemory( er );
		PgCntrItem* pItem = (PgCntrItem*) per->user_refcon;

		if ( pItem && is_kind_of(PgCntrItem, pItem) ) {
			if ( 0 /*pItem->GetType() != OT_STATIC*/ ) {
				pItem->m_canActivate = true;
				pItem->DoVerb( OLEIVERB_SHOW, this );
			}
		}

		UnuseMemory( er );
	}
	else {
//		m_pgModifiers |= WORD_MOD_BIT;
//		long drag_result = pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

		//Set Timer to catch the third click, if any
		m_mouseState |= ( MOUSE_TRIPLE_CLICKED | MOUSE_DRAG_SELECT );
		if (!SetTimer(TRIPLE_CLICK_TIMER, GetDoubleClickTime(), NULL)) 
			m_mouseState &= ~MOUSE_TRIPLE_CLICKED;
 
		long selectLeft;
		pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);

		paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory( m_paigeRef );
		pgPtr->hilite_anchor = selectLeft;
		UnuseMemory(m_paigeRef);
	}
        
    ReleaseCapture();
    CView::OnLButtonDblClk(nFlags, point);
}

void CPaigeEdtView::OnKillFocus(CWnd* pNewWnd) 
{
    //	Cancel Format Painter when window loses focus
	m_mouseState &= ~MOUSE_FORMAT_PAINTER;
	
	// Deactivate the cursor when window loses focus
    if (!m_bDontDeactivateHighlight )
        pgSetHiliteStates(m_paigeRef, deactivate_verb, no_change_verb, TRUE);

	// Free spell memory. When doing cool spell checking (i.e. checking
	// automatically as the user types) we keep the spell dictionaries
	// loaded for faster performance. However since we're losing focus
	// this we no longer need the spell dictionaries (until we next
	// gain focus) - we're better off saving the memory.
	//
	// As importantly if we never reloaded the spelling dictionaries
	// we wouldn't get any changes the user makes to the user dictionary.
	//
	// Cleanup is smart about not deleting the dialog if it's currently open.
	m_spell.Cleanup();

    CView::OnKillFocus(pNewWnd);
}

void CPaigeEdtView::OnSetFocus(CWnd* pOldWnd) 
{
    // Activate the cursor when window gets focus
    pgSetHiliteStates(m_paigeRef, activate_verb, no_change_verb, TRUE); 
    CView::OnSetFocus(pOldWnd);

	// If we're using the full feature set - set the scan timer since
	// we're focused now.
	if ( UsingFullFeatureSet() )
		SetTimer( SCAN_TIMER, SCAN_INTERVAL, NULL );
}

void CPaigeEdtView::OnEditCut() 
{
	bool	bPreventDestroyingSignatureStyle = PreventDestroyingSignatureStyle(false, false);

	// Even if we won't allow the cut, we should still allow the copy portion
	OnEditCopy();

	// Allow only if the selection was not entirely signature
	if (!bPreventDestroyingSignatureStyle)
	{
		PrepareUndo(undo_delete);
		// Delete the current selection since it's a CUT operation
		pgDelete(m_paigeRef, NULL, best_way);
		ScrollToCursor();
		m_bMoodMailDirty = true;
	}
}

void CPaigeEdtView::OnEditCopy() 
{
    COleDataSource *clipBoardStuff = DEBUG_NEW COleDataSource();
    OnCopy(clipBoardStuff);
}

void CPaigeEdtView::OnCopy(COleDataSource *clipBoardStuff)
{
    // setup the stuff bucket so that any embeds get saved in whatever
    // alternate formats are available. note that embeds only get rendered in
    // additional formats if they are explicitly selected.

    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    UnuseMemory( m_paigeRef );
    assert( pSB->bOleExport == false );
    assert( pSB->pDS == NULL );

    if ( clipBoardStuff ) {
        pSB->bOleExport = true;
        pSB->pDS = clipBoardStuff;
        PgDataTranslator theTxr( m_paigeRef );
        theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtText );

		// If Shift key is down, then only put plain text on clipboard
        if (GetKeyState(VK_SHIFT) >= 0)
        {
            theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtRtf );
            theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtNative );
            theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtHtml );
        }
        clipBoardStuff->SetClipboard();
    }

    // make sure that these are "dead"
    pSB->bOleExport = false;
    pSB->pDS = NULL;
}


void CPaigeEdtView::OnEditPaste() 
{
    COleDataObject clipBoardData;
    VERIFY(clipBoardData.AttachClipboard());
    OnPaste(&clipBoardData, undo_ole_paste);
}

void CPaigeEdtView::OnEditPasteSpecial()
{
	COlePasteSpecialDialog dlg;

	dlg.AddFormat(CF_TEXT, TYMED_HGLOBAL, AFX_IDS_TEXT_FORMAT, FALSE, FALSE);
	dlg.AddFormat(PgDataTranslator::kFmtRtf, TYMED_HGLOBAL, AFX_IDS_RTF_FORMAT, FALSE, FALSE);
	dlg.AddFormat(PgDataTranslator::kFmtNative, TYMED_HGLOBAL, IDS_PAIGE_FORMAT, FALSE, FALSE);
	dlg.AddFormat(CF_DIB, TYMED_HGLOBAL, AFX_IDS_DIB_FORMAT, FALSE, FALSE);
	dlg.AddFormat(CF_BITMAP, TYMED_GDI, AFX_IDS_BITMAP_FORMAT, FALSE, FALSE);
	// HTML last because it's the most likely to be screwed up
	dlg.AddFormat(PgDataTranslator::kFmtHtml, TYMED_HGLOBAL, IDS_HTML_FORMAT, FALSE, FALSE);

	if (dlg.DoModal() == IDOK)
	{
		COleDataObject clipBoardData;
		CLIPFORMAT cf = dlg.m_ps.arrPasteEntries[dlg.m_ps.nSelectedIndex].fmtetc.cfFormat;

		VERIFY(clipBoardData.AttachClipboard());

		OnPaste(&clipBoardData, undo_ole_paste, cf);
	}
}


void CPaigeEdtView::GuardBeforeImportingNative()
{
	//	When the clipFormat is native and the user is pasting over all existing text
	//	(or equivalently into an empty message), there is an obscure bug where
	//	paige_rec.insert_style ends up being 65535 (and possibly other oddities).
	//	I reproduced it by copying bold text from a web page, pasting it in Eudora
	//	(clipFormat is HTML), recopying it within Eudora (to get a native clipFormat),
	//	and then pasting over everything.
	//
	//	The bad data bug in turn causes a crash the first time this information is used
	//	such as in OnUpdateTextBold. The previous solution was an attempt to avoid the
	//	use of Native format when pasting over all text. Unfortunately this causes Eudora
	//	specific formatting such as excerpt bars to be lost. Moreover the user can still
	//	force Native format via Paste Special, which could still trigger the bug.
	//
	//	So why does it work for a paste that's not the entire message? Because
	//	PaigeImportFilter::pgImportFile (in PGIMPORT.CPP) has code that checks
	//	for existing text and works a little differently when there is existing text
	//	(it creates import_pg). Somehow import_pg and pgPaste recover from the
	//	bad data (the bad data still exists initially in import_pg - it just recovers
	//	so that it never makes it into the original_pg).
	//
	//	Since PaigeImportFilter::pgImportFile is not passed the clipFormat, it's easier
	//	to solve the issue here by convincing PaigeImportFilter::pgImportFile that
	//	in fact the user is not pasting over all existing text. Not pretty, but
	//	better than the previous solution.
	
	//	Insert a single space so that the text is not empty
	TCHAR	uChar = (TCHAR) VK_SPACE;

	pgInsert( m_paigeRef, (pg_byte_ptr) &uChar, 1, CURRENT_POSITION,
			  key_insert_mode, 0, draw_none );

	//	Place the selection before the space
	pgSetSelection(m_paigeRef, 0, 0, 0, FALSE);
}


void CPaigeEdtView::CleanupAfterImportingNative()
{
	//	Save the post paste selection
	long		selLeft, selRight;
	pgGetSelection(m_paigeRef, &selLeft, &selRight);

	//	Remove the space we previously inserted
	long		endText = pgTextSize(m_paigeRef);
	pgSetSelection(m_paigeRef, endText-1, endText, 0, FALSE);
	pgDelete(m_paigeRef, NULL, draw_none);

	//	Restore the selection
	pgSetSelection(m_paigeRef, selLeft, selRight, 0, FALSE);
}


// Paige doesn't handle shift-clicking of embedded images very well.
// By default, when shift-clicking an image to the right of the current selection
// Paige will select all the text up to the image, but not including the image.
// We want the image to be included in the selection. The easiest way to fix
// this is to detect this specific case and slightly adjust the mouse position
// that we'll pass Paige to be one pixel to the right of the embedded image.
void CPaigeEdtView::AdjustMouseIfEmbedded(co_ordinate & pgMousePt)
{
	// Check to see if we're extending the selection
	if (m_pgModifiers & EXTEND_MOD_BIT)
	{
		long	startSel, endSel;
		pgGetSelection(m_paigeRef, &startSel, &endSel);

		// Check to see if:
		// * We currently have a selection (can't be extending if we don't)
		// * The mouse is over an embedded image
		if ( (startSel != endSel) && pgPtInEmbed(m_paigeRef, &pgMousePt, NULL, NULL, false) )
		{
			style_info		the_style;
			select_pair		the_range;
			rectangle		pgRect;
			
			// Check to see if it looks like the embedded image is at the far
			// right of the selection.
			pgPtToStyleInfo(m_paigeRef, &pgMousePt, NO_BYTE_ALIGN | NO_HALFCHARS, &the_style, &the_range);

			if (the_range.end >= endSel)
			{
				co_ordinate		screen_extra;
				pgGetEmbedBounds(m_paigeRef, 1, &the_range, &pgRect, NULL, &screen_extra);
				
				// Adjust the mouse position to be one pixel to the right of the
				// embedded image
				pgMousePt.h = pgRect.bot_right.h + 1;
			}
		}
	}
}


bool CPaigeEdtView::OnPaste(COleDataObject *clipBoardData, short undoVerb /*= 0*/, PgDataTranslator::FormatType clipFormat /*= 0*/) 
{
    PgDataTranslator theTxr( m_paigeRef );

	if (clipFormat == CF_DIB)
		clipFormat = CF_BITMAP;
	else if (clipFormat == 0)
	{
		// Shift down indicates a plain paste, but not if it's Shift + Insert,
		// which is the ancient accelerator for doing paste.
		if ((GetKeyState(VK_SHIFT) < 0) && (GetKeyState(VK_INSERT) >= 0) &&
			clipBoardData->IsDataAvailable(CF_TEXT, NULL))
		{
			clipFormat = CF_TEXT;
		}
        
		else if (clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL))
			clipFormat = PgDataTranslator::kFmtNative;

		else if (GetIniShort(IDS_INI_ALLOW_CLIPBOARD_HTML) &&
			clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL))
		{
			clipFormat = PgDataTranslator::kFmtHtml;
		}

		else if (clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL))
			clipFormat = PgDataTranslator::kFmtRtf;
        
		else if (clipBoardData->IsDataAvailable(CF_TEXT, NULL))
			clipFormat = CF_TEXT;
		else if ( clipBoardData->IsDataAvailable( CF_BITMAP, NULL ) )
			clipFormat = CF_BITMAP;
	}

    if (!clipFormat)
		return false;

	// Paige implementation doesn't seem to handle selected text correctly, 
	//	so let's perform the deletion ourselves.
	if (pgNumSelections(m_paigeRef))
	{
		if (!(m_mouseState & MOUSE_DRAG_DROP))
		{
			PrepareUndo(undo_delete, MEM_NULL, TRUE);
			pgDelete(m_paigeRef, NULL, best_way);
		}
		switch(undoVerb)
		{
		case undo_ole_paste : 
			undoVerb = undo_ole_paste_delete; 
			break;
		case undo_drop : 
			//if (m_mouseState & MOUSE_DRAG_DROP) 
			undoVerb = undo_drag_drop_move ; 
			break;
		}
	}

	long	startSel, endSel;
	pgGetSelection(m_paigeRef, &startSel, &endSel);

	long	nTextSizeBeforePaste = pgTextSize(m_paigeRef);

	// BOG: here's a hack to handle bitmaps off the clipboard. don't know
	// if we should be doing CF_DIB as well; i suspect that we should :)
	if ( clipFormat == CF_BITMAP ) {
		STGMEDIUM stg;
		clipBoardData->GetData( CF_BITMAP, &stg );
		PgInsertImageFromBitmap( m_paigeRef, (HBITMAP)stg.hBitmap );
		Invalidate();
	}
	else {
		//	Is the clip native format and is the user pasting over all existing text?
		bool	bForceTempPG = (clipFormat == PgDataTranslator::kFmtNative) && (startSel == 0) && (endSel == nTextSizeBeforePaste);
		
		if (bForceTempPG)
			GuardBeforeImportingNative();
		
		theTxr.ImportData( clipBoardData, clipFormat );

		if (bForceTempPG)
			CleanupAfterImportingNative();
	}

	long	insert_ref[2];
	insert_ref[0] = startSel;
	insert_ref[1] = pgTextSize(m_paigeRef) - nTextSizeBeforePaste;
	PrepareUndo(undoVerb, insert_ref);

	// Any AutoURLs in the pasted text?
	if (clipFormat != PgDataTranslator::kFmtNative)
		MakeAutoURLSpaghetti(insert_ref[0], insert_ref[0] + insert_ref[1]);

	// Scan the new stuff
	select_pair tr;
	tr.begin = insert_ref[0];
	tr.end = tr.begin + insert_ref[1];
	adjust_text_range( m_paigeRef, &tr );		// make sure begin/end fall on word boundaries
	m_styleEx->SetNeedsScanned( TRUE, &tr );	// spelling happens later -- now is bad time
	m_checkdastuff = true;						// need to scan this stuff

	//Flag Mood Mail Dirty
	m_bMoodMailDirty = true;

	//Need to make sure the hyperlink callbacks is set to our default
	if (clipFormat == PgDataTranslator::kFmtNative)
	{
		pg_hyperlink hyperlink;
		//If there are any hyperlinks, then make sure the callback is set to our callback
		if (pgGetHyperlinkSourceInfo(m_paigeRef, insert_ref[0], 0, true, &hyperlink))
			pgSetHyperlinkCallback( m_paigeRef, hlCallback, NULL );
	}

	ScrollToCursor();

	return true;
}

void CPaigeEdtView::OnEditPasteAsQuotation()
{
    COleDataObject clipBoardData;
    VERIFY(clipBoardData.AttachClipboard());
    PgDataTranslator theTxr( m_paigeRef );

    PgDataTranslator::FormatType clipFormat = 0;

    bool bShift = (GetKeyState(VK_SHIFT) < 0);
    if ( bShift && clipBoardData.IsDataAvailable(CF_TEXT, NULL))
        clipFormat = CF_TEXT; 

    else if (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL))
        clipFormat = PgDataTranslator::kFmtNative;

	else if ( GetIniShort(IDS_INI_ALLOW_CLIPBOARD_HTML) &&
			  clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL) )
	{
		clipFormat = PgDataTranslator::kFmtHtml;
	}
                
    else if (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL))
        clipFormat = PgDataTranslator::kFmtRtf;
        
    else if (clipBoardData.IsDataAvailable(CF_TEXT, NULL))
        clipFormat = CF_TEXT; 

    //
    // Save the starting offset;
    //
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    long savedFirst = sel.begin;
    //
    //  If the caret is in column 1, then there is no need to
    //  prepend a lf.
    //
    long lineStart, lineEnd;
    pgFindLine(m_paigeRef, sel.begin, &lineStart, &lineEnd);
    bool bPrependBreak = (sel.begin != lineStart);

    // if Native translater must check to see if it is plain/rich       
    if ((clipFormat == PgDataTranslator::kFmtNative) ||
        (clipFormat == PgDataTranslator::kFmtRtf) )
    {
        if (pg_ref pg = pgNewShell(PgGlobalsPtr()))
        {
			PgDataTranslator theTempTxr( pg );
            theTempTxr.ImportData( &clipBoardData, clipFormat );
            if ( !PgHasStyledText(pg, PgGlobalsPtr(), NULL) )
                clipFormat = CF_TEXT;			
            pgDispose(pg);                                      
        }
    }
        
    if ( bShift || (clipFormat == CF_TEXT &&
					!GetIniShort(IDS_INI_ALWAYS_EXCERPT_RESPONSE) &&
					!GetIniShort(IDS_INI_EXCERPT_FIXED_TEXT) &&
					!PgHasStyledText(m_paigeRef, PgGlobalsPtr())) )
    {
        HANDLE hText;
        char* QuotedText = NULL;
        if (OpenClipboard())
        {
            hText = GetClipboardData(CF_TEXT);
            if (hText)
            {
                char* text = (char*)GlobalLock(hText);
                if (text)
                    QuotedText = QuoteText(text, FALSE);
                GlobalUnlock(hText);
                if (QuotedText)
                {
//					long length = strlen(QuotedText);
//					PrepareUndo(undo_insert,&length);
                    select_pair tempSel;
                    pgGetSelection(m_paigeRef, &tempSel.begin, &tempSel.end);
                    SetTextAs(QuotedText, &tempSel, PgDataTranslator::kFmtText, FALSE);
                    //pgInsert( m_paigeRef, (pg_byte_ptr)QuotedText, length, CURRENT_POSITION, 
                    //          data_insert_mode, 0, best_way );
                    delete [] QuotedText;
                }
            }
            CloseClipboard();
        }
    }
    else
    {
        if (clipFormat) {
            //
            //  Paige implementation doesn't seem to handle selected text correctly, 
            //  so let's perform the deletion ourselves.
            //
            short verb = undo_ole_paste;
            if (pgNumSelections(m_paigeRef))
            {
                PrepareUndo(undo_delete, MEM_NULL, TRUE);
                pgDelete(m_paigeRef, NULL, best_way);
                verb = undo_ole_paste_delete; 
            }
            //
            // Insert the text at the caret
            //
            long insert_ref[2];
            pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
            insert_ref[0] = sel.begin;
            insert_ref[1] = 0;
            char br = '\r';
            if ( bPrependBreak )
            {
                pgInsert( m_paigeRef, (pg_byte_ptr)&br, 1, CURRENT_POSITION, key_insert_mode, 0, best_way );
                insert_ref[1] += 1;
                savedFirst++;
            }

			long	nTextSizeBeforePaste = pgTextSize(m_paigeRef);
			//	Is the clip native format and is the user pasting over all existing text?
			bool	bForceTempPG = (clipFormat == PgDataTranslator::kFmtNative) && (sel.begin == 0) && (sel.end == nTextSizeBeforePaste);
			
			if (bForceTempPG)
				GuardBeforeImportingNative();

			sel.begin = nTextSizeBeforePaste;
            theTxr.ImportData( &clipBoardData, clipFormat );

			if (bForceTempPG)
				CleanupAfterImportingNative();
			
            sel.end = pgTextSize(m_paigeRef);
            insert_ref[1] += sel.end - sel.begin;
            //
            // Get the ending offset
            //
            pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
            long savedLast = sel.end;
            //
            // Was a carriage return the last character pasted?
            //
            text_ref    ref_for_text = NULL;
            long        length = 1;
            pg_byte_ptr pText;
            pText = pgExamineText(m_paigeRef, savedLast-1, &ref_for_text, &length);
            if (pText && *pText != PgGlobalsPtr()->line_wrap_char )                     
            {
                pgInsert( m_paigeRef, (pg_byte_ptr)&br, 1, CURRENT_POSITION, key_insert_mode, 0, best_way );
                insert_ref[1] +=1;
                savedLast++;
                sel.begin++;
                sel.end++;
            }
            PrepareUndo(verb, insert_ref);
            
			if (ref_for_text)
			{
				// Very important to call UnuseMemory because pgExamineText calls UseMemory - otherwise
				// later memory allocation calls can fail in bad ways (like pgResizeMemory setting
				// the ref to NULL when called from SetMemorySize)
				UnuseMemory(ref_for_text);
			}
            
            //
            // Apply Excerpt style to paste
            //
            pgSetSelection(m_paigeRef, savedFirst, savedLast, 0, FALSE);
			
			PaigeExcerptLevelChanger	excerptLevelChanger(true);
			ApplyStyleChange(excerptLevelChanger, false);

            pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
        }
    }
    ScrollToCursor();                                     
}

BOOL CPaigeEdtView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
    dwStyle |= WS_VSCROLL;
    return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CPaigeEdtView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( nSBCode == SB_ENDSCROLL ) {
		CView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}
		
	long h_pix = 0; long v_pix = 0;

	// see if there is an in-place active ole item that needs scrolling. there
	// is only one active at any given time---all other ole items are handled
	// by the embed callback's "EMBED_DRAW" handler. [PgEmbeddedObject.cpp]

	PgCntrItem* pItem = 0;
	COleDocument* poleDoc = (COleDocument*) GetDocument();

	if (poleDoc)
	{
		ASSERT_KINDOF( COleDocument, poleDoc );

		if ( is_kind_of(COleDocument, poleDoc) )
			pItem = (PgCntrItem*) poleDoc->GetInPlaceActiveItem( this );
	}

	// proportional thumbtrack requires special handling
	if ( nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION ) {
		short curH, curV, maxH, maxV;
		pgGetScrollValues( m_paigeRef, &curH, &curV, &maxH, &maxV );
		SCROLLINFO si;
		GetScrollInfo(SB_VERT,&si);
		int nPos1 = MapScrollPos(nPos,si.nMax-si.nPage-1,maxV);

		pgSetScrollValues( m_paigeRef, curH, (short)nPos1,
				(pg_boolean)(nSBCode == SB_THUMBPOSITION), best_way );

		// pixels to move an active ole item
		v_pix = curV - nPos1;
	}
	else {   // use standard paige scroll verbs
		short	v_verb = GetPaigeVerbForScrollCode(nSBCode);

		if ( pItem ) {
			// calc the pixel offset about to be caused by "pgScroll"
			pgScrollUnitsToPixels( m_paigeRef, scroll_none, v_verb, false,
					false, &h_pix, &v_pix );
		}
		
		pgScroll( m_paigeRef, scroll_none, v_verb, best_way );
	}

	// move the active ole item to its new position. not sure if this is the
	// ole-approved way of doing this, but i could not get frigging
	// SetItemRects() to do the right things for me.
	if ( pItem ) {
		CWnd* pwnd = pItem->GetInPlaceWindow();

		if ( pwnd ) {
			CRect rc;
			pwnd->GetWindowRect( &rc );
			ScreenToClient( &rc );
			rc.OffsetRect( h_pix, v_pix );
			pwnd->Invalidate();
			pwnd->MoveWindow( &rc );
		}
	}

    UpdateScrollBars( /* pScrollBar */ );

	// BOG: this seems rather pointless to me
    CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPaigeEdtView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    switch ( nSBCode ) {
    case SB_PAGEDOWN:
        pgScroll( m_paigeRef, -scroll_page, scroll_none, best_way );
        break;
    case SB_LINEDOWN:
        pgScroll( m_paigeRef, -scroll_unit, scroll_none, best_way );
        break;
    case SB_PAGEUP:
        pgScroll( m_paigeRef, scroll_page, scroll_none, best_way );
        break;
    case SB_LINEUP:
        pgScroll( m_paigeRef, scroll_unit, scroll_none, best_way );
        break;
    case SB_THUMBPOSITION: //Doc doesn't include SB_THUMBTRACK??  Not sure why.
    {
        short curH, curV, maxH, maxV;
        pgGetScrollValues( m_paigeRef, &curH, &curV, &maxH, &maxV );
        pgSetScrollValues( m_paigeRef, curH, (short)nPos,
                           (pg_boolean)(nSBCode == SB_THUMBPOSITION), best_way );
        break;
    }
    }

    UpdateScrollBars( /* pScrollBar */ );
    CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

UINT _AfxGetMouseScrollLines(BOOL bForceFresh = FALSE);

UINT _AfxGetMouseScrollLines(BOOL bForceFresh /* = FALSE */)
{
    // if we've already got it and we're not refreshing,
    // return what we've already got

    static BOOL bGotScrollLines = FALSE;
    static UINT uCachedScrollLines = 0;

    if (!bForceFresh && bGotScrollLines)
        return uCachedScrollLines;

        // see if we can find the mouse window

    bGotScrollLines = TRUE;

    static UINT msgGetScrollLines = 0;
    static WORD nRegisteredMessage = 0;

    if (nRegisteredMessage == 0)
    {
        msgGetScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
        if (msgGetScrollLines == 0)
            nRegisteredMessage = 1;     // couldn't register!  never try again
        else
            nRegisteredMessage = 2;     // it worked: use it
    }

    if (nRegisteredMessage == 2)
    {
        HWND hwMouseWheel = NULL;
        hwMouseWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
        if (hwMouseWheel && msgGetScrollLines)
        {
            uCachedScrollLines = (UINT)
                ::SendMessage(hwMouseWheel, msgGetScrollLines, 0, 0);
            return uCachedScrollLines;
        }
    }

    // couldn't use the window -- try system settings

    OSVERSIONINFO ver;
    memset(&ver, 0, sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(ver);

    uCachedScrollLines = 3; // reasonable default
    if (!::GetVersionEx(&ver))
        return uCachedScrollLines;

    if ((ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
         ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
        && ver.dwMajorVersion < 4)
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER,  _T("Control Panel\\Desktop"),
                         0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            TCHAR szData[128];
            DWORD dwKeyDataType;
            DWORD dwDataBufSize = sizeof(szData)/sizeof(szData[0]);

            if (RegQueryValueEx(hKey, _T("WheelScrollLines"), NULL, &dwKeyDataType,
                                (LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
            {
                uCachedScrollLines = _tcstoul(szData, NULL, 10);
            }
            RegCloseKey(hKey);
        }
    }
    else if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT && ver.dwMajorVersion >= 4)
    {
        ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);
    }

    return uCachedScrollLines;
}

BOOL CPaigeEdtView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // If we haven't been initially drawn, don't do scrolling yet
	if (!m_bHasBeenInitiallyDrawn)
		return TRUE;
	
	UINT uWheelScrollLines = _AfxGetMouseScrollLines(TRUE);

	// MSDN says that "If this number is 0, no scrolling should occur"
	if (uWheelScrollLines == 0)
		return TRUE;

    int nToScroll;
//      int nDisplacement;

    nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
    WORD nScrollCode;

	// Don't check for -1 as a sign of overflow - overflowing should be
	// rare, but more importantly -1 can occur when scrolling up if
	// scroll speed is set to slowest possible.
    if (uWheelScrollLines == WHEEL_PAGESCROLL)
    {
        nScrollCode = (WORD)(zDelta > 0? SB_PAGEUP : SB_PAGEDOWN);
        SendMessage(WM_VSCROLL, MAKEWPARAM(nScrollCode, 0));
    }
    else if (nToScroll != 0)
    {
//              nDisplacement = nToScroll * <line height>;
//              nDisplacement = min(nDisplacement, <page height>);
        nScrollCode = (WORD)(zDelta > 0? SB_LINEUP : SB_LINEDOWN);
        if (nToScroll < 0)
            nToScroll = -nToScroll;

		CScrollBar *	pScrollBar = GetScrollBarCtrl(SB_VERT);
		int				nScrollPos;

        for (int i = 0; i < nToScroll; i++)
		{
			nScrollPos = GetScrollPos(SB_VERT);
			OnVScroll(nScrollCode, nScrollPos, pScrollBar);
		}
    }

    return TRUE;
}

short CPaigeEdtView::GetPaigeVerbForScrollCode(UINT nSBCode)
{
	short		v_verb = scroll_none;
	
	switch ( nSBCode )
	{
		case SB_BOTTOM:
			v_verb = scroll_end;
			break;
		case SB_LINEDOWN:
			v_verb = -scroll_unit;
			break;
		case SB_LINEUP:
			v_verb = scroll_unit;
			break;
		case SB_PAGEDOWN:
			v_verb = -scroll_page;
			break;
		case SB_PAGEUP:
			v_verb = scroll_page;
			break;
		case SB_TOP:
			v_verb = scroll_home;
			break;

		default:
			ASSERT(0);
			break;
	}

	return v_verb;
}

void CPaigeEdtView::UpdateScrollBars( bool bSetPage /* = false */, bool bAllowScrollToView /* = true */, bool * pbNoScrollingNeeded /* = NULL */ )
{
    short maxH = 0, maxV = 0, hValue = 0, vValue = 0;

    if (bSetPage)
        bHasBeenInitiallyUpdated=true;

    if ( !bHasBeenInitiallyUpdated )
        return;

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
    si.nMin = 0;

    // Page size represents the number of data units that can
    // fit in the client area of the owner window given its current 
    // size.  Let's make a pixel THE data unit.
    RECT rect;
    GetClientRect(&rect);
    si.nPage = rect.bottom - rect.top; 

    // Maximum size of scroll bar will be the total height of
    // the document (plus top and bottom borders) in pixels.
    si.nMax = pgTotalTextHeight(m_paigeRef, true) + BORDER_TOP + BORDER_BOTTOM;

    // Translate current scroll position from Paige To Windows
    pgGetScrollValues(m_paigeRef, &hValue, &vValue, &maxH, &maxV );
    int yPos = MapScrollPos(vValue, maxV, si.nMax - si.nPage - 1);

    // Position cannot exceed maximum scroll position
    si.nPos = min(yPos,si.nMax);
        
	// if the viewable area is bigger than the doc height, make sure the
	// the whole doc is visible.
	bool	bNoScrollingNeeded = ( si.nPage > static_cast<UINT>(si.nMax) );
	if ( bAllowScrollToView && bNoScrollingNeeded )
		pgScrollToView( m_paigeRef, 0, 0, 0, FALSE, best_way );
	
    if ( si.nMax > 10000 ) {
        si.nMax  = si.nMax  / 100;
        si.nPage = si.nPage / 100;
        si.nPos  = si.nPos  / 100;
    }
        
    SetScrollInfo(SB_VERT, &si, TRUE);

	if (pbNoScrollingNeeded)
		*pbNoScrollingNeeded = bNoScrollingNeeded;
}


// OnFormatFont:
// Handles the "Font..." menu command

void CPaigeEdtView::OnFormatFont() 
{
    LOGFONT lf;
    memset( &lf, '\0', sizeof(LOGFONT) );
    pgGetFontByLog( m_paigeRef, &lf );

    /*CFontDialog*/CFaceNameDialog theFont( &lf, CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS );

    // get font attributes for current selection from user
    int iRet = theFont.DoModal();

    if ( iRet == IDOK ) {
        if ( pgNumSelections(m_paigeRef) )
            PrepareUndo(undo_format); 
        // set the face name
        pgSetFontByName( m_paigeRef,
                         (const pg_font_name_ptr)(LPCSTR)theFont.GetFaceName(), NULL, FALSE );

        // set the point size. note that windows gives this to us in tenths of a
        // point, while paige wants it in points.
        //pgSetPointSize( m_paigeRef, (short)pgAbsoluteValue(theFont.GetSize()/10),
        //              NULL, FALSE );

        // set the text color
        COLORREF theColor = theFont.GetColor();
        pgSetTextColor( m_paigeRef, &theColor, NULL, FALSE );

        // set the style attributes
        long styleBits, setBits;
        styleBits = setBits = 0;

        if ( theFont.IsBold() ) {
            styleBits |= X_BOLD_BIT;
        }
        if ( theFont.IsItalic() ) {
            styleBits |= X_ITALIC_BIT;
        }
        if ( theFont.IsUnderline() ) {
            styleBits |= X_UNDERLINE_BIT;
        }
        if ( theFont.IsStrikeOut() ) {
            styleBits |= X_STRIKEOUT_BIT;
        }

        // before we set styles, check for "plain text". not sure what this means
        // within the context of Paige, but as does the sample code, so do i.
        if ( styleBits == X_PLAIN_TEXT ) {
            setBits = X_ALL_STYLES;
        }
        else {
            long fartBits;
            pgGetStyleBits( m_paigeRef, &setBits, &fartBits );
            setBits ^= styleBits;
        }

        // set dem style bits!
        pgSetStyleBits( m_paigeRef, styleBits, setBits, NULL, TRUE );
    }
}

void CPaigeEdtView::OnUpdateFormatFont(CCmdUI* pCmdUI)
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
        pCmdUI->Enable( true );
}

// OnFormatStyles:
// Handles the "Format:Named Styles..." menu command
//
// This uses the "dialog from hell"; just give it a paige ref, and it'll do the
// rest. The dialog allows for creating a named style sheet and adding it to
// the document, as well as applying it to the current selection.

void CPaigeEdtView::OnFormatStyles() 
{
/*
  CNamedStyleDialog theNSDialog( m_paigeRef );
  theNSDialog.DoModal();
*/
}


void CPaigeEdtView::OnTextBold()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_BOLD_BIT, X_BOLD_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextBold( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );

        pCmdUI->SetCheck(GetStyleState( bold_var ));
    }
}


void CPaigeEdtView::OnTextItalic()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_ITALIC_BIT, X_ITALIC_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextItalic( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        pCmdUI->SetCheck(GetStyleState( italic_var ));
    }
}


void CPaigeEdtView::OnTextUnderline()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_UNDERLINE_BIT, X_UNDERLINE_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextUnderline( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        pCmdUI->SetCheck(GetStyleState( underline_var ));
    }   
        
}

void CPaigeEdtView::OnTextStrikeout()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_STRIKEOUT_BIT, X_STRIKEOUT_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextStrikeout( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        pCmdUI->SetCheck(GetStyleState( strikeout_var ));
    }   
        
}


void CPaigeEdtView::OnTextPlain()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_PLAIN_TEXT, X_ALL_STYLES, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextPlain( CCmdUI* pCmdUI )
{
    bool bStyled=FALSE;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        long style_bits, consist_bits;
        pgGetStyleBits( m_paigeRef, &style_bits, &consist_bits);
                
        bStyled = !(style_bits == X_PLAIN_TEXT );

        /*      style_info info, mask;
                pgInitStyleMask(&info,0);
                pgInitStyleMask(&mask,0);
                pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
                pg_globals_ptr  globals=pgGetGlobals(m_paigeRef);
                //
                // Character Styles: bold, italic, underline, strikeout
                // Text color, size     and embedded images
                //
                long style_bits = PaigeToQDStyle(&info);
                bStyled = ( ( style_bits & (X_BOLD_BIT | X_ITALIC_BIT | X_UNDERLINE_BIT | X_STRIKEOUT_BIT)) || 
                ( info.fg_color.red       != globals->def_style.fg_color.red    ) ||
                ( info.fg_color.green != globals->def_style.fg_color.green      ) ||
                ( info.fg_color.blue  != globals->def_style.fg_color.blue       ) ||
                ( info.fg_color.alpha != globals->def_style.fg_color.alpha      ) ||
                ( info.embed_id           != globals->def_style.embed_id                ) ||
                ( info.point              != globals->def_style.point                   ) );
                //
                // Font format: Name and size
                //
                if (!bStyled)
                {
                char szFontName[256]={NULL};
                pgGetFontByName (m_paigeRef, (unsigned char*) &szFontName);
                bStyled= stricmp((const char*)&szFontName,(const char*)globals->def_font.name);
                }
        */
    }
    if ( bStyled )
        pCmdUI->SetCheck( false );
    else
        pCmdUI->SetCheck( true );
        
}


void CPaigeEdtView::ChangeParaJustification(short in_nNewJustification)
{
	if ( UsingFullFeatureSet() )
	{
		PaigeJustificationChanger		justificationChanger(in_nNewJustification);

		ApplyStyleChange(justificationChanger);
	}
}


void CPaigeEdtView::OnParaCenter()
{
	ChangeParaJustification(justify_center);
}

void CPaigeEdtView::OnUpdateParaCenter( CCmdUI* pCmdUI )
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        pCmdUI->SetCheck(GetJustifyState(justify_center));
    }
}


void CPaigeEdtView::OnParaLeft()
{
	ChangeParaJustification(justify_left);
}

void CPaigeEdtView::OnUpdateParaLeft( CCmdUI* pCmdUI )
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        pCmdUI->SetCheck(GetJustifyState(justify_left));
    }
}

void CPaigeEdtView::OnUpdateLastTextColor( CCmdUI* pCmdUI )
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        //pCmdUI->SetCheck(GetJustifyState(justify_left));
    }
}

void CPaigeEdtView::OnUpdateLastEmoticon(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(UsingFullFeatureSet() && !m_fRO);
}

void CPaigeEdtView::OnLastTextColor()
{
	if (!UsingFullFeatureSet())
		return;
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);

	int textColor = GetIniShort(IDS_INI_LAST_TEXT_COLOR);
	COLORREF theColor =  CColorMenu::GetColor(textColor + ID_COLOR0);
	pgSetTextColor( m_paigeRef, &theColor, NULL, TRUE );

}


// Find the signature -- note that we only need to find the beginning, the end
// is always EOF, so to speak.
//
// I tried doing this backwards [starting at EOF] for performance, but no dice.
// Perhaps it's just pgWalkPreviousStyle(), but I couldn't do it reliably. It's
// not too bad though, since we only walk the paragraph style changes.
bool CPaigeEdtView::find_signature( select_pair_ptr sigRng,
		select_pair_ptr within /*= NULL*/ )
{
	bool found = false;
	long searchLimit = pgTextSize( m_paigeRef );

	if ( within ) {
		sigRng->begin = sigRng->end = within->begin;
		searchLimit = within->end;
	}
	else
		sigRng->begin = sigRng->end = 0;

	style_walk sw;
	paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
	pgPrepareStyleWalk( prp, sigRng->begin, &sw, true );

	for ( ; sw.current_offset < searchLimit; ) {
		if ( m_styleEx->IsSignature( sw.cur_par_style ) ) {
			sigRng->begin = sw.current_offset;
			found = true;
			break;
		}

		if ( sw.next_par_run->offset <= sw.t_length )
			pgWalkStyle( &sw, sw.next_par_run->offset - sw.current_offset );
		else
			break;
	}

	pgPrepareStyleWalk( prp, 0, NULL, true );
	UnuseMemory( m_paigeRef );

	if ( found )
		sigRng->end = pgTextSize( m_paigeRef );

	return found;
}


// Prevents destroying the signature style by:
// * Returning true (i.e. prevent action) when a forward delete is acting
//   at the end of the body (i.e. it will delete the signature start) or
//   when a delete is acting at the start of the signature (i.e. it will
//   delete the signature start).
// * Modifying selections that are mixed signature and body to only contain
//   one or the other
bool CPaigeEdtView::PreventDestroyingSignatureStyle(bool bIsDelete, bool bIsFowardDelete)
{
	// Get the current selection
	select_pair		curSel;
	pgGetSelection(m_paigeRef, &curSel.begin, &curSel.end);

	// Modify the current selection so that we can be sure to detect the
	// start of the signature
	curSel.end += 2;
	
	// Find the signature, if any
	bool			bPreventAction = false;
	select_pair		sigRng;
	if ( find_signature(&sigRng, &curSel) )
	{
		// Restore the current selection end to original value.
		curSel.end -= 2;
		
		// Does the selection involve the body at all?
		if (curSel.begin < sigRng.begin)
		{
			// Check for and prevent harmful forward delete from deleting
			// the division between the body and the signature
			if ( bIsFowardDelete && (curSel.begin+1 == sigRng.begin) )
			{
				bPreventAction = true;
			}
			else if (curSel.begin != curSel.end)
			{
				if ( (sigRng.begin - curSel.begin) > (curSel.end - sigRng.begin) )
				{
					// The selection is more non-signature text than signature text
					// Modify the selection to end immediately before the signature.
					curSel.end = sigRng.begin - 1;
				}
				else
				{
					// The selection is more signature text than non-signature text
					// Modify the selection to start immediately after the signature.
					curSel.begin = sigRng.begin;
				}

				pgSetSelection(m_paigeRef, curSel.begin, curSel.end, 0, TRUE);
			}
		}
		else
		{
			// Check for potentially harmful case where a delete is being processed
			// and the selection is empty and located at the start of the signature.
			if ( (curSel.begin == curSel.end) && (curSel.begin == sigRng.begin) &&
				 bIsDelete )
			{
				if (sigRng.begin == 0)
				{
					
					ASSERT( !"How did we get a signature range that begins at 0?" );
					bPreventAction = true;
				}
				else
				{
					// Check to see if there's more signature before us. If not,
					// then we're at the start of the signature, in which case we
					// want to prevent the operation of the delete key that was
					// pressed.
					sigRng.end = sigRng.begin;
					sigRng.begin--;

					if ( !m_styleEx->IsSignature(&sigRng) )
						bPreventAction = true;
				}
			}
			else
			{
				// Selection is entirely signature - allow the action.
				bPreventAction = false;
			}
		}
	}

	return bPreventAction;
}


void CPaigeEdtView::InsertEmoticonAtCurrentPosition(Emoticon * pEmoticon)
{
	paige_rec_ptr	pPaigeRec = reinterpret_cast<paige_rec_ptr>( UseMemory(m_paigeRef) );

	if (pPaigeRec)
	{
		//	By default we'll use the emoticon insertion undo verb
		short	undoVerb = undo_emoticon_insertion;

		//	When doing a current position insertion we should delete
		//	any previous selection
		if (pPaigeRec->num_selects)
		{
			PrepareUndo(undo_delete, MEM_NULL, TRUE);
			pgDelete(m_paigeRef, NULL, draw_none);

			//	Use a different undo verb so that we remember to undo both
			//	our emoticon insertion and the above deletion
			undoVerb = undo_emoticon_insertion_replace;
		}
		
		//	Determine our current position
		long	nPosition = pgCurrentInsertion(pPaigeRec);

		//	We're done with the Paige memory
		UnuseMemory(m_paigeRef);

		//	Remember our current size and position for undoing
		long	nTextSizeBeforeEmoticonInsertion = pgTextSize(m_paigeRef);
		long	insert_ref[2];
		insert_ref[0] = nPosition;

		//	Space character used for inserting
		TCHAR	uSpaceChar = (TCHAR) VK_SPACE;
		
		//	If there's no space before the emoticon, we'll need to insert one
		if ( (nPosition > 0) && !(pgCharType(m_paigeRef, nPosition-1, BLANK_BIT) & BLANK_BIT) )
		{
			//	Insert space
			pgInsert( m_paigeRef, (pg_byte_ptr) &uSpaceChar, 1, nPosition,
				key_insert_mode, 0, draw_none );

			//	Move past the space
			nPosition++;
		}

		if (GetIniShort(IDS_INI_DISPLAY_EMOTICON_AS_PICTURES) != 0)
		{
			//	Insert the emoticon embedded image
			PgInsertEmoticonImageFromFile(m_paigeRef, pEmoticon->GetImageFullPath(), pEmoticon->GetTrigger(), nPosition);

			//	Move past the inserted embedded image (embedded images use two placeholder characters)
			nPosition += 2;
		}
		else
		{
			const char *	szTrigger = pEmoticon->GetTrigger();
			int				nTriggerLength = pEmoticon->GetTrigger().GetLength();
			
			//	Insert the emoticon text trigger
			pgInsert( m_paigeRef,
					  reinterpret_cast<pg_byte_ptr>(const_cast<char *>(szTrigger)),
					  pEmoticon->GetTrigger().GetLength(), nPosition,
					  key_insert_mode, 0, draw_none );

			//	Move past the inserted trigger
			nPosition += nTriggerLength;
		}

		//	If there's no space after the emoticon, we'll need to insert one
		if ( (nPosition == pgTextSize(m_paigeRef)) ||
			 !(pgCharType(m_paigeRef, nPosition, BLANK_BIT) & BLANK_BIT) )
		{
			pgInsert( m_paigeRef, (pg_byte_ptr) &uSpaceChar, 1, nPosition,
					  key_insert_mode, 0, draw_none );

			//	Move past the space
			nPosition++;
		}

		//	Prepare undo with the actual number of characters that we inserted
		insert_ref[1] = pgTextSize(m_paigeRef) - nTextSizeBeforeEmoticonInsertion;
		
		PrepareUndo(undoVerb, insert_ref);

		//	Make sure the insertion point is at the correct location
		pgSetSelection(m_paigeRef, nPosition, nPosition, 0, TRUE);

		//	Refresh our display
		Invalidate();
		UpdateWindow();

		//	Make sure that we're scrolled to the cursor
		ScrollToCursor();
	}
}

void CPaigeEdtView::OnLastEmoticon()
{
	if ( !UsingFullFeatureSet() )
		return;
	
	CString		szEmoticonTrigger;

	GetIniString(IDS_INI_LAST_EMOTICON_TRIGGER, szEmoticonTrigger);

	ASSERT( !szEmoticonTrigger.IsEmpty() );
	if ( !szEmoticonTrigger.IsEmpty() )
	{
		Emoticon *		pEmoticon = NULL;
		g_theEmoticonDirector.LookUpEmoticonTriggerMap(szEmoticonTrigger, pEmoticon);

		ASSERT(pEmoticon);

		if (pEmoticon)
			InsertEmoticonAtCurrentPosition(pEmoticon);
	}
}

void CPaigeEdtView::OnParaRight()
{
	ChangeParaJustification(justify_right);
}

void CPaigeEdtView::OnUpdateParaRight( CCmdUI* pCmdUI )
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        pCmdUI->SetCheck(GetJustifyState(justify_right));
    }
}


//
// Applies the style change intelligently to the current selection
//
void CPaigeEdtView::ApplyStyleChange(PaigeStyleChanger & paigeStyleChanger, bool bNeedToPrepareUndo)
{
    int				prevInfoIndex = 0, curInfoIndex = 1;
	long			selectionBegin, selectionEnd;
	long			firstSameLevelOffset, lastSameLevelOffset;
	long			paraBegin, paraEnd;
	select_pair		paragraphSelection;

	// Get the entire affected selection
	pgGetSelection(m_paigeRef, &selectionBegin, &selectionEnd);

	// Prime the same style search, by getting the style info for the first paragraph
	pgFindPar(m_paigeRef, selectionBegin, &paraBegin, &paraEnd);
	
	paragraphSelection.begin = paraBegin;
	paragraphSelection.end = paraEnd;
	paigeStyleChanger.GetStyleInfo(m_paigeRef, m_styleEx, &paragraphSelection, prevInfoIndex);

	do
	{
		// Search for any consecutive paragraphs with compatible style information so
		// that we can call SetStyleInfo with as much consecutive text as possible.
		firstSameLevelOffset = paraBegin;
		lastSameLevelOffset = paraEnd;
		
		while (lastSameLevelOffset < selectionEnd)
		{
			pgFindPar(m_paigeRef, lastSameLevelOffset, &paraBegin, &paraEnd);
			
			paragraphSelection.begin = paraBegin;
			paragraphSelection.end = paraEnd;

			paigeStyleChanger.GetStyleInfo(m_paigeRef, m_styleEx, &paragraphSelection, curInfoIndex);
			
			// Is the style information that we need compatible? If not, then stop searching.
			if ( paigeStyleChanger.AreStylesIncompatible() )
				break;

			lastSameLevelOffset = paraEnd;
		}

		// Modify the offset range that had compatible styles.
		paragraphSelection.begin = firstSameLevelOffset;
		paragraphSelection.end = lastSameLevelOffset;

		// Does the style need to be changed for what we just scanned?
		if ( paigeStyleChanger.PrepareToSetStyleInfo(m_paigeRef, m_styleEx, &paragraphSelection, prevInfoIndex) )
		{
			if (bNeedToPrepareUndo)
			{
				// Prepare for undo before making first change
				PrepareUndo(undo_format);

				// Don't call PrepareUndo again
				bNeedToPrepareUndo = false;
			}

			paigeStyleChanger.SetStyleInfo(m_paigeRef, m_styleEx, &paragraphSelection, prevInfoIndex);
		}

		// Swap indices
		prevInfoIndex = curInfoIndex;
		curInfoIndex = (curInfoIndex + 1) % 2;
	}
	while (lastSameLevelOffset < selectionEnd);
}

void CPaigeEdtView::OnIndentIn()
{
	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		PaigeIndentLevelChanger		indentLevelChanger(DEFLIST_INDENT_VALUE);

		ApplyStyleChange(indentLevelChanger);
	}
}

void CPaigeEdtView::OnUpdateIndentCommand(CCmdUI* pCmdUI) 
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

	pCmdUI->Enable(!m_fRO && m_bAllowStyled);
}

void CPaigeEdtView::OnIndentOut()
{
 	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		PaigeIndentLevelChanger		indentLevelChanger(-DEFLIST_INDENT_VALUE);

		ApplyStyleChange(indentLevelChanger);
	}
}

void CPaigeEdtView::OnMarginsNormal()
{
 	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		PaigeMarginsNormalChanger		marginsNormalChanger;

		ApplyStyleChange(marginsNormalChanger);
	}
}


// Serialize:
// Loads/saves Paige contents to/from HTML files
//
// Once this code is in Eudora, it will need to be rewritten to use streams
// instead of just passing the file name to the SGML parser.

/*03-17-2000 Commented this function as this is moved to TextFileView.Cpp
void CPaigeEdtView::Serialize(CArchive& ar) 
{
    PgDataTranslator theTxr( m_paigeRef );
        
    CFile *theFile = ar.GetFile();
    CString fileName = theFile->GetFileName();

    //I guess we can just find ".ext" but this is being safe
    int dot = fileName.ReverseFind('.');
    CString fileExt = fileName.Right(fileName.GetLength() - dot - 1);

    PgDataTranslator::FormatType format;

    if ( ( fileExt.CompareNoCase(CRString(IDS_HTM_EXTENSION)) == 0 ) ||
         ( fileExt.CompareNoCase(CRString(IDS_HTML_EXTENSION)) == 0) )
        format = PgDataTranslator::kFmtHtml;
    else if ( fileExt.CompareNoCase(CRString(IDS_RTF_EXTENSION)) == 0)
        format = PgDataTranslator::kFmtRtf;
    else
        format = PgDataTranslator::kFmtText;

    if ( ar.IsStoring() ) {
        // storing code
        theTxr.ExportData( theFile, format);
        SaveChangeState();
    }
    else {
        // loading code

        pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);

        theTxr.ImportData( theFile, format);
        pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
        UpdateScrollBars(true); 
    }
        
    //theFile->Close();
}
*/

BOOL CPaigeEdtView::OnEraseBkgnd(CDC* pDC) 
{
    DWORD windowColor;
    color_value BackColor;

    pgGetPageColor(m_paigeRef, &BackColor);
    pgColorToOS(&BackColor, &windowColor);

    CBrush theBrush;
    theBrush.CreateSolidBrush(windowColor);
    CBrush* pOldBrush = (CBrush*) pDC->SelectObject( &theBrush );

    CRect rect;
    pDC->GetClipBox( &rect );
    pDC->PatBlt( rect.left, rect.top, rect.Width(), rect.Height(),PATCOPY );
    pDC->SelectObject( pOldBrush );
    return true;
}

void FindWord(pg_ref pgRef, long offset, long PG_FAR *wordLeft, long PG_FAR *wordRight, 
              pg_boolean left_side, pg_boolean smart_select) 
{
    long charInfo=0;
    long textSize = pgTextSize(pgRef);

    //Find Word to the Right, breaks at tabs, carriage returns, most punctuations and new-words
    if (!left_side) {
        *wordLeft = *wordRight = offset;
        charInfo = pgCharType(pgRef, *wordRight, PUNCT_NORMAL_BIT);
		if (charInfo & PUNCT_NORMAL_BIT)
		{
			// If we're starting next to punctuation, then find the next non-punct character
			while ((*wordRight != textSize) && (charInfo & PUNCT_NORMAL_BIT))                        
				charInfo = pgCharType(pgRef, ++*wordRight, PUNCT_NORMAL_BIT);
			// then skip space immediately to the right of the punct chars
			if (*wordRight != textSize)
			{
				charInfo = pgCharType(pgRef, *wordRight, BLANK_BIT | PAR_SEL_BIT | TAB_BIT);
				while ((*wordRight != textSize) && (charInfo & BLANK_BIT) && !(charInfo & (PAR_SEL_BIT | TAB_BIT)))
					charInfo = pgCharType(pgRef, ++*wordRight, BLANK_BIT | PAR_SEL_BIT | TAB_BIT);
			}
		}
		else
		{
//			charInfo = pgCharType(pgRef, *wordRight, TAB_BIT);
//			while ((*wordRight != textSize) && (charInfo & TAB_BIT))                        
//				charInfo = pgCharType(pgRef, ++*wordRight, TAB_BIT);
			charInfo = pgCharType(pgRef, *wordRight, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
			while ((*wordRight != textSize) && (charInfo & BLANK_BIT) && !(charInfo & (TAB_BIT | PAR_SEL_BIT)))
				charInfo = pgCharType(pgRef, ++*wordRight, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

			if ((*wordRight == offset) && (*wordRight != textSize)) {                       
				pgFindWord(pgRef, offset, wordLeft, wordRight, left_side, smart_select);
				if ((*wordRight == offset) && (*wordRight != textSize))
					++*wordRight;
				charInfo = pgCharType(pgRef, *wordRight, BLANK_BIT | PAR_SEL_BIT | TAB_BIT);
				while ((*wordRight != textSize) && (charInfo & BLANK_BIT) && !(charInfo & (PAR_SEL_BIT | TAB_BIT)))
					charInfo = pgCharType(pgRef, ++*wordRight, BLANK_BIT | PAR_SEL_BIT | TAB_BIT);                  
			}
		}
    }
    else {
        //Find word to the Left, breaks at beginning of tabs, carriage-returns, most punctuations and new-words
        //Traverse blanks on immediate left, if any, so we don;t get stuck at the same word
        *wordRight = *wordLeft = offset;
		if (*wordLeft != 0)
		{
			//Find all blanks
			charInfo = pgCharType(pgRef, *wordLeft-1, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
			while ((charInfo & BLANK_BIT) && !(charInfo & (TAB_BIT | PAR_SEL_BIT)) && (--*wordLeft != 0))
				charInfo = pgCharType(pgRef, *wordLeft, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

			if (*wordLeft != 0)
			{
				charInfo = pgCharType(pgRef, *wordLeft-1, PUNCT_NORMAL_BIT);
				if (charInfo & PUNCT_NORMAL_BIT)
				{
					// If we're starting next to punctuation, then find the next non-punct character
					while ((charInfo & PUNCT_NORMAL_BIT) && (--*wordLeft != 0))
						charInfo = pgCharType(pgRef, *wordLeft-1, PUNCT_NORMAL_BIT);
				}
				else
				{
					offset = *wordLeft;
					pgFindWord(pgRef, offset, wordLeft, wordRight, left_side, smart_select);
					if ((*wordLeft == offset) && (*wordLeft != 0)) {
						//Find all tabs
						charInfo = pgCharType(pgRef, *wordLeft-1, TAB_BIT);
						while ((charInfo & TAB_BIT) && (--*wordLeft != 0))
							charInfo = pgCharType(pgRef, *wordLeft-1, TAB_BIT);
						if ((*wordLeft == offset) && (*wordLeft != 0)) --*wordLeft; 
					}
				}
			}
		}
    }
}

void CPaigeEdtView::SetPgAreas( /*CPaigeEdtView *view*/ )
{       
    rectangle pgClientRect;
//      shape_ref visArea;

    CRect clientRect;
    GetClientRect(&clientRect);

    // don't want to fill the *entire* client area
    clientRect.DeflateRect( m_pgBorder );

    // Get the client area dimensions
    RectToRectangle(&clientRect,&pgClientRect);
//      visArea = pgRectToShape(PgMemGlobalsPtr(), &pgClientRect);
        
    // Reset the page area and the visible area to the current size
//      pgSetAreas(m_paigeRef, visArea, visArea, MEM_NULL);
    pgSetAreaBounds(m_paigeRef, &pgClientRect, &pgClientRect);

    // Dispose of the shape bcos Paige maintains a copy
//      pgDisposeShape(visArea);

    pgAdjustScrollMax(m_paigeRef, best_way);
}


void CPaigeEdtView::OnTimer(UINT nIDEvent)
{
    switch(nIDEvent) {
        // TRIPLE_CLICK .... Time Up and no third click
    case TRIPLE_CLICK_TIMER : 
        m_mouseState &= ~MOUSE_TRIPLE_CLICKED;
        KillTimer(nIDEvent); break;
    case DRAG_SELECT_TIMER : //Fall thru
        // DRAG_DROP scrolling
    case DRAG_DROP_SCROLL_TIMER : 
    {
        CRect clientRect;
        GetClientRect(&clientRect);
        co_ordinate pgMousePt;
        CPointToPgPoint(&m_mousePt, &pgMousePt);

		// Adjust the mouse position if necessary to correct Paige's selection
		// of embedded images. See AdjustMouseIfEmbedded for more details.
		AdjustMouseIfEmbedded(pgMousePt);

        if (clientRect.bottom - m_mousePt.y  <= 10)
        {
            int LinesToScroll = (m_mousePt.y - clientRect.bottom) / 16 + 1;
            for (; LinesToScroll > 0; LinesToScroll--)
            {
                OnVScroll(SB_LINEDOWN, 0, GetScrollBarCtrl(SB_VERT));
                if (nIDEvent == DRAG_SELECT_TIMER)
                    pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
            }
        }
        else if (m_mousePt.y - clientRect.top <= 10)
        {
            int LinesToScroll = (clientRect.top - m_mousePt.y) / 16 + 1;
            for (; LinesToScroll > 0; LinesToScroll--)
            {
                OnVScroll(SB_LINEUP, 0, GetScrollBarCtrl(SB_VERT));
                if (nIDEvent == DRAG_SELECT_TIMER)
                    pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
            }
        }
        break;
    }
    case SCAN_TIMER:
    {
        if (IsReadOnly())
		{
            KillTimer( SCAN_TIMER );

			// Free spell memory. When doing cool spell checking (i.e. checking
			// automatically as the user types) we keep the spell dictionaries
			// loaded for faster performance. However since we're read only
			// we no longer need the spell dictionaries (at least for now) so
			// we're better off saving the memory.
			//
			// As importantly if we never reloaded the spelling dictionaries
			// we wouldn't get any changes the user makes to the user dictionary.
			//
			// Cleanup is smart about not deleting the dialog if it's currently open.
			m_spell.Cleanup();
		}
        else if ( ((GetTickCount() - last_typed) > 250) && m_checkdastuff  && (!m_bServicingSpellMoodBPTimer)) 
		{
			m_bServicingSpellMoodBPTimer = true;
            KillTimer( SCAN_TIMER );

			//SDSpellTRACE("Spell begin\n");

			g_bInteractiveSpellCheck = GetIniShort(IDS_INI_INTERACTIVE_SPELL_CHECK);

			HideCaret();

			// Reset whether or not we think redraw is needed
			m_spell.ResetIsRedrawNeeded();

			bool					bFoundEmoticons;
			EmoticonsInTextList		emoticonsInText;
			select_pair				textRange;
			bool					bConvertEmoticons = UsingFullFeatureSet() &&
														(GetIniShort(IDS_INI_DISPLAY_EMOTICON_AS_PICTURES) != 0);

            while ( DequeueDirtyPair( &textRange.begin, &textRange.end ) )
			{
				// Find and queue any emoticon triggers we find
				if (bConvertEmoticons)
					FindEmoticonTriggerLocations(textRange, emoticonsInText);
				
				if (!g_bInteractiveSpellCheck)
					m_spell.Check( &textRange );
			}

			// Remember whether or not we found any emoticons
			bFoundEmoticons = !emoticonsInText.empty();

			// Replace emoticon triggers that we found above
			if (bConvertEmoticons)
				ReplaceEmoticonTriggersWithImages(emoticonsInText);

			// find the current word
			select_pair curWord;
			GetWordContainingSelection(m_paigeRef,&curWord);

			// prepare to walk styles
            style_walk sw;
            paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
            pgPrepareStyleWalk( prp, 0, &sw, false );
            UnuseMemory( m_paigeRef );

            // walk my ass off!
			for (;;) {
                style_info_ptr psi = sw.cur_style;
				
				//SDSpellTRACE("Walking: %d.%d %d\n",sw.current_offset,sw.next_style_run->offset-sw.current_offset,CPaigeStyle::NeedsScanned( psi ));

                if ( CPaigeStyle::NeedsScanned( psi ) ) {
                    textRange.begin = textRange.end = sw.current_offset;
                    textRange.end += sw.next_style_run->offset - textRange.begin;
					if ( SelPairIntersects(&textRange,&curWord) )
					{
						if ( textRange.begin<curWord.begin )
						{
							QueueDirtyPair( textRange.begin, curWord.begin );
						}
						if ( curWord.end < textRange.end )
						{
							QueueDirtyPair( curWord.end, textRange.end );
						}
					}
					else
					{
						QueueDirtyPair( textRange.begin, textRange.end );
					}
                }
				if (!pgWalkNextStyle(&sw)) break;
            }

            pgPrepareStyleWalk( prp, 0, NULL, false );    // tells paige we're done

            CPaigeStyle boogieMama( m_paigeRef );
            while ( DequeueDirtyPair( &textRange.begin, &textRange.end ) )
			{
				// Replace emoticon triggers that we found above
				if (bConvertEmoticons)
					FindEmoticonTriggerLocations(textRange, emoticonsInText);
				
				boogieMama.SetNeedsScanned(FALSE, &textRange);

				if (!g_bInteractiveSpellCheck)
					m_spell.Check( &textRange );
            }

			// Remember whether or not we found any emoticons
			if (!bFoundEmoticons)
				bFoundEmoticons = !emoticonsInText.empty();

			// Replace emoticon triggers that we found above
			if (bConvertEmoticons)
				ReplaceEmoticonTriggersWithImages(emoticonsInText);

			// We used to update repeatedly when we changed the style to reflect
			// misspelled words inside of Check. That was *really* slow for messages
			// with a lot of misspelled words (we were even doing this when the text
			// wasn't scrolled into view). Now we only update when necessary after
			// changing the styles or converting emoticon triggers to images.
			if ( bFoundEmoticons || (!g_bInteractiveSpellCheck && m_spell.IsRedrawNeeded()) )
			{
				Invalidate();
				UpdateWindow();
			}

			m_checkdastuff = false;

			// If we're still focused, set the timer so that we scan again
			if (CWnd::GetFocus() == this)
				SetTimer( SCAN_TIMER, SCAN_INTERVAL, NULL );

			m_bServicingSpellMoodBPTimer = false;
			ShowCaret();
        }
		else
		{
			if (GetFocus() != this)
			{
				// Free spell memory. When doing cool spell checking (i.e. checking
				// automatically as the user types) we keep the spell dictionaries
				// loaded for faster performance. However since we're losing focus
				// this we no longer need the spell dictionaries (until we next
				// gain focus) - we're better off saving the memory.
				//
				// As importantly if we never reloaded the spelling dictionaries
				// we wouldn't get any changes the user makes to the user dictionary.
				//
				// Cleanup is smart about not deleting the dialog if it's currently open.
				m_spell.Cleanup();

				// Kill scan timer until we next get focus
				KillTimer( SCAN_TIMER );
			}
		}
        break;
    }
	}

}

void CPaigeEdtView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();
        
    pgSetSelection( m_paigeRef, 0, 0, 0, false );
	PGS_InitDrawGlobals();
}


short CPaigeEdtView::GetPaigeUndoVerb(short EudoraUndoVerb)
{
    //In case we happen to call with a valid Paige Undo verb
    short PaigeUndoVerb = EudoraUndoVerb;

    switch(EudoraUndoVerb)
    {
    case undo_drag : 
        PaigeUndoVerb = undo_delete; break;
    case -undo_drag :
        PaigeUndoVerb = -undo_delete; break;
                
    case undo_insert_hr : 
    case undo_insert_hr_withCR : 
        PaigeUndoVerb = undo_format; break;
    case -undo_insert_hr :
    case -undo_insert_hr_withCR : 
        PaigeUndoVerb = -undo_delete; break;

    case undo_drop :
    case undo_ole_paste :
    case undo_translation :
	case undo_emoticon_insertion :
    case undo_ole_paste_delete :
    case undo_drag_drop_move :
    case undo_translation_replace :
	case undo_emoticon_insertion_replace :
	case undo_spelling_replace :
        PaigeUndoVerb = undo_app_insert; break;
    case -undo_drop :
    case -undo_ole_paste :
    case -undo_translation :
	case -undo_emoticon_insertion :
    case -undo_ole_paste_delete :
    case -undo_drag_drop_move :
    case -undo_translation_replace :
	case -undo_emoticon_insertion_replace :
	case -undo_spelling_replace :
        PaigeUndoVerb = -undo_app_insert; break;

    case undo_overwrite :
        PaigeUndoVerb = undo_typing; break;
    case -undo_overwrite :
        PaigeUndoVerb = -undo_typing; break;
    }

    return PaigeUndoVerb;
}


void CPaigeEdtView::PrepareUndo(short verb, long *insert_ref, BOOL useDeleteStack)
{
    undo_ref prevUndo, *newUndo = DEBUG_NEW undo_ref ;
    short originalVerb = verb;

    CUndoStack &theStack = useDeleteStack ? m_deleteUndoStack : m_undoStack;
        
    //For all verbs except the following three
    prevUndo = MEM_NULL; 

    //Need to force new undo refs ourselves?
    if ((verb == undo_typing) || /*(verb = undo_overwrite) ||*/
        (verb == undo_fwd_delete) || (verb == undo_backspace))
    {
        if (theStack.GetCount() > 0)
            if (pgUndoType(prevUndo = *theStack.GetTail()) == verb)
            {
                long selectLeft;
                pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);

                pg_undo_ptr undoPtr = (pg_undo_ptr)UseMemory(prevUndo);

                if ( ( (verb == undo_typing && undoPtr->verb == undo_typing) /*|| 
                                                                               (verb == undo_overwrite && undoPtr->verb == undo_overwrite)*/ ) 
                     &&
                     (!HasChanged() || undoPtr->verb != undoPtr->real_verb) )
                {
                    //If the last saved undo ref was for typing and the doc is currently
                    //in a saved state, then typing some more will add it to the same 
                    //undo ref, but we need to force a new one to be created

                    //Or if we performed an undo/redo, the real verb gets changed to paste
                    //and they Paige clears out the old undo info
                    UnuseMemory(prevUndo);
                    prevUndo = MEM_NULL; 
                }
                else if ((undoPtr->alt_range.end != selectLeft) || (pgNumSelections(m_paigeRef)))
                {
                    UnuseMemory(prevUndo);
                    prevUndo = MEM_NULL; 
                }
                else UnuseMemory(prevUndo);
            }
            else prevUndo = MEM_NULL;
    }

    //Now change all our self-defined verbs to what Paige can understand b'4 calling pgPrepareUndo
    if (abs(originalVerb) >= undo_drag)
        verb = CPaigeEdtView::GetPaigeUndoVerb(originalVerb);

    if (insert_ref)
        *newUndo = pgPrepareUndo(m_paigeRef, verb, (void PG_FAR *)insert_ref);
    else
        *newUndo = pgPrepareUndo(m_paigeRef, verb, (void PG_FAR *)prevUndo);

    //Now set the undo verbs back to what we understand
    if (abs(originalVerb) >= undo_drag)
    {
        pg_undo_ptr undoPtr = (pg_undo_ptr)UseMemory(*newUndo);
        undoPtr->verb = originalVerb;
        UnuseMemory(*newUndo);
    }

    if (!prevUndo || (*newUndo != prevUndo)) 
    {
        AddRefToTail(theStack, newUndo);
        //Enough to keep track of the Main undo stack
        if ( ! useDeleteStack )
            UpdateChangeState( kAddChange );
    }
    else 
        delete newUndo;

}

void CPaigeEdtView::OnEditUndo() 
{
    if (m_undoStack.GetCount() == 0)
    {
        // Why did this get enabled when there's nothing to undo?
        ASSERT(0);
        return;
    }

    undo_ref *undoRef = m_undoStack.GetTail();

    if (!undoRef || !*undoRef)
    {
        // Uh oh, something weird happened!
        ASSERT(0);
        return;
    }

    //Change the verbs to what Paige can understand b'4 calling pgUndo
    pg_undo_ptr refPtr = (pg_undo_ptr)UseMemory(*undoRef);

    undo_ref *redoRef = DEBUG_NEW undo_ref;
    undo_ref *undoDeleteRef, *redoDeleteRef = NULL;

    short originalVerb = refPtr->verb;

    if (abs(originalVerb) >= undo_drag)
        refPtr->verb = CPaigeEdtView::GetPaigeUndoVerb(originalVerb);

    UnuseMemory(*undoRef);

    //pgUndo returns a ref to be used for Redo-ing the operation
    if (abs(originalVerb) >= undo_ole_paste_delete)
    {
        *redoRef = pgUndo(m_paigeRef, *undoRef, TRUE, best_way);

// BOO: overwrite/undo disaster prevention; make sure we have something to redo.
		if ( !m_deleteUndoStack.IsEmpty() ) {
			undoDeleteRef = m_deleteUndoStack.GetTail();
			redoDeleteRef = DEBUG_NEW_NOTHROW undo_ref;
			*redoDeleteRef = pgUndo(m_paigeRef, *undoDeleteRef, TRUE, best_way);
		}
		else {
			// BOO: we are now very confused. we have stuff to undo, the "verb"
			// says it's a delete, but there's nothing on the delete stack.
			// this multi-undo stuff needs a rewrite.
			assert(0);
			redoDeleteRef = 0;
		}
    }
    else 
        *redoRef = pgUndo(m_paigeRef, *undoRef, TRUE, best_way);
        
    //Change the verb back to what we understand
    if (abs(originalVerb) >= undo_drag)
    {
        refPtr = (pg_undo_ptr)UseMemory(*redoRef);
        refPtr->verb = (short)-(originalVerb);
        UnuseMemory(*redoRef);
    }

    //TODO: Update Current Style Run somehow here
    if ((abs(originalVerb) == undo_format) || (abs(originalVerb) >= undo_ole_paste))
    {
        if (!pgNumSelections(m_paigeRef))
        {
            long selectLeft;
            pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
            pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }
        //TODO: Update the toolbar buttons here for format changes
    }

    RemoveRef(m_undoStack);
    AddRefToTail(m_redoStack, redoRef);
    UpdateChangeState( kUndoChange );

// BOO: overwrite/undo disaster prevention; make sure we have something to redo.
    if (abs(originalVerb) >= undo_ole_paste_delete && redoDeleteRef)
    {
        RemoveRef(m_deleteUndoStack);
        AddRefToTail(m_deleteRedoStack, redoDeleteRef);
        //UpdateChangeState( kUndoChange );
    }
    ScrollToCursor();
	m_bMoodMailDirty = true;
}

void CPaigeEdtView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
    if (!m_fRO)
    {
        int undoCount = m_undoStack.GetCount();
        pCmdUI->Enable(undoCount);

        if (undoCount) 
        {
            switch(pgUndoType(*m_undoStack.GetTail())) 
            {
                //case undo_overwrite :
            case undo_typing : 
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_TYPING)); break;
            case undo_fwd_delete :
            case undo_backspace :
            case undo_delete : 
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_DELETE)); break;
            case undo_format :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_FORMATTING)); break;
            case undo_ole_paste :
            case undo_ole_paste_delete :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_PASTE)); break;
            case undo_drag :
            case undo_drop :
            case undo_drag_drop_move :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_DRAG_DROP)); break;
            case undo_insert_hr :
            case undo_insert_hr_withCR :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_HR)); break;
            case undo_none :
            default : 
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO)); break;
            }
        }
        else pCmdUI->SetText(CRString(IDS_EDIT_UNDO));
    }
    else
    {
        pCmdUI->SetText(CRString(IDS_EDIT_UNDO));
        pCmdUI->Enable(FALSE);
    }

}

void CPaigeEdtView::OnEditRedo() 
{
    ASSERT(m_redoStack.GetCount());

    undo_ref *redoRef = m_redoStack.GetTail();
    undo_ref *undoRef = DEBUG_NEW undo_ref;

    undo_ref *redoDeleteRef, *undoDeleteRef;

    //Change the verb to what Paige can understand
    pg_undo_ptr refPtr = (pg_undo_ptr)UseMemory(*redoRef);

    short originalVerb = refPtr->verb;

    if (abs(originalVerb) >= undo_drag)
        refPtr->verb = CPaigeEdtView::GetPaigeUndoVerb(originalVerb);

    UnuseMemory(*redoRef);

    //Redo the deletes, if any, first
    if (abs(originalVerb) >= undo_ole_paste_delete)
    {
        redoDeleteRef = m_deleteRedoStack.GetTail();
        undoDeleteRef = DEBUG_NEW undo_ref;
        *undoDeleteRef = pgUndo(m_paigeRef, *redoDeleteRef, TRUE, best_way);
        //Pop the delete ref from the secondary redo stack and 
        //add the new one to the secondary undo stack
        RemoveRef(m_deleteRedoStack);
        AddRefToTail(m_deleteUndoStack, undoDeleteRef);
        //UpdateChangeState( kRedoChange );
    }

    //Redo the main action
    *undoRef = pgUndo(m_paigeRef, *redoRef, TRUE, best_way);

    //Change the verb from [-]undo_app_insert back to [-]paste/drag_drop
    if (abs(originalVerb) >= undo_drag)
    {
        refPtr = (pg_undo_ptr)UseMemory(*undoRef);
        refPtr->verb = (short)-(originalVerb);
        UnuseMemory(*undoRef);
    } 

    //TODO: Update Current Style Run somehow here
    if ((abs(originalVerb) == undo_format) || (abs(originalVerb) >= undo_ole_paste))
    {
        if (!pgNumSelections(m_paigeRef))
        {
            long selectLeft;
            pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
            pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }
    }

    //Pop the redo ref from the Tail
    RemoveRef(m_redoStack);
    AddRefToTail(m_undoStack, undoRef);
    UpdateChangeState( kRedoChange );
	m_bMoodMailDirty = true;

}

void CPaigeEdtView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
    if (!m_fRO)
    {
        int redoCount = m_redoStack.GetCount();
        pCmdUI->Enable(redoCount);
        if (redoCount) {
            switch(pgUndoType(*m_redoStack.GetTail())) 
            {
            case -undo_typing : 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_TYPING)); break;
            case -undo_fwd_delete :
            case -undo_backspace :
            case -undo_delete: 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_DELETE)); break;
            case -undo_format : 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_FORMATTING)); break;
            case -undo_ole_paste :
            case -undo_ole_paste_delete :
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_PASTE)); break;
            case -undo_drag :
            case -undo_drop :
            case -undo_drag_drop_move :
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_DRAG_DROP)); break;
            case -undo_insert_hr :
            case -undo_insert_hr_withCR :
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_HR)); break;
            case undo_none :
            default : 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO)); break;
            }
        }
        else pCmdUI->SetText(CRString(IDS_EDIT_REDO));
    }
    else
    {
        pCmdUI->SetText(CRString(IDS_EDIT_REDO));
        pCmdUI->Enable(FALSE);
    }
}



void CPaigeEdtView::AddRefToTail(CUndoStack &theStack, undo_ref *theRef) 
{
    if (theStack.GetCount() == UNDO_STACK_MAX)
        RemoveRef(theStack, /*fromTail=*/FALSE);
    theStack.AddTail(theRef);
}

void CPaigeEdtView::RemoveRef(CUndoStack &theRefStack, BOOL fromTail)
{
    undo_ref *theRef;
    if (fromTail) {
        theRef = theRefStack.GetTail();
        theRefStack.RemoveTail();       
    }
    else {
        theRef = theRefStack.GetHead();
        theRefStack.RemoveHead();
    }
    pgDisposeUndo(*theRef);
    delete theRef;
}

void CPaigeEdtView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(pgNumSelections(m_paigeRef));        
}

void CPaigeEdtView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
    if (m_fRO)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(pgNumSelections(m_paigeRef));            
}

void CPaigeEdtView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
    BOOL dataAvailable = FALSE;

    if (!m_fRO)
    {
        COleDataObject clipBoardData;
        VERIFY(clipBoardData.AttachClipboard());

        if ((clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL)) ||
            (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL)) ||
            (clipBoardData.IsDataAvailable(CF_TEXT, NULL)) ||
            (clipBoardData.IsDataAvailable(CF_BITMAP, NULL)) ||
            (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL)))
            dataAvailable = TRUE;
    }

    pCmdUI->Enable(dataAvailable);
}

DROPEFFECT CPaigeEdtView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
    //m_mouseState |= MOUSE_DRAG_DROP;

    DROPEFFECT dropEffect;
    dropEffect = OnDragOver(pDataObject, dwKeyState, point);
    if ((dropEffect == DROPEFFECT_MOVE) || (dropEffect == DROPEFFECT_COPY)) {
        //m_mouseState |= MOUSE_DRAG_DROP;
        SetTimer(DRAG_DROP_SCROLL_TIMER, 200, NULL);
    }
    return dropEffect;
    //return CView::OnDragEnter(pDataObject, dwKeyState, point);
}

void CPaigeEdtView::OnDragLeave() 
{
    // TODO: Add your specialized code here and/or call the base class
    KillTimer(DRAG_DROP_SCROLL_TIMER);
    //m_mouseState &= ~MOUSE_DRAG_DROP;
    m_mousePt = 0;
    CView::OnDragLeave();
}

DROPEFFECT CPaigeEdtView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
    if (((pDataObject->IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL)) ||
         (pDataObject->IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL)) ||
         (pDataObject->IsDataAvailable(CF_TEXT, NULL)) ||
         (pDataObject->IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL)))
        &&
        (!m_fRO))
    {
        m_mousePt = point;
        if (dwKeyState & MK_CONTROL)
            return DROPEFFECT_COPY;
        else //if ((dwKeyState & MK_SHIFT) || (dwKeyState & MK_ALT)) 
            return DROPEFFECT_MOVE;
    }
    else return DROPEFFECT_NONE;
    //return CView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL CPaigeEdtView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
    if (m_fRO) 
    { 
        OnDragLeave();
        return false;
    }

    //Kill the *AutoScroll* timer for drag and drop
    KillTimer(DRAG_DROP_SCROLL_TIMER); 

    long selectRight, selectLeft;
    pgGetSelection(m_paigeRef, &selectLeft, &selectRight);

    //Get current *CURSOR* position
    CPoint curPoint;
    ::GetCursorPos(&curPoint);
    ScreenToClient(&curPoint);
    //Fill Paige coord struct
    co_ordinate curCoOrd;
    CPointToPgPoint(&curPoint, &curCoOrd);
    long offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
    short verb = undo_drop;

    //No op if drag and drop is within the same selection
    if ((m_mouseState & MOUSE_DRAG_DROP) && (offset >= selectLeft) && (offset <= selectRight))
        return FALSE;

    // source and target are the same, hence handle the deletion of text is previous location
    if ((dropEffect == DROPEFFECT_MOVE) && (!m_fRO)){
        if ( (m_mouseState & MOUSE_DRAG_DROP) && (pgNumSelections(m_paigeRef)) ) {
            if (offset > selectRight)
                offset -= (selectRight - selectLeft);
            PrepareUndo(undo_delete, MEM_NULL, TRUE);
            // Delete the current selection since it's a MOVE operation
            pgDelete(m_paigeRef, NULL, best_way);
            verb = undo_drag_drop_move;
        }
    }

    pgSetSelection(m_paigeRef, offset, offset, 0, TRUE);        
    OnPaste(pDataObject, verb);

    return TRUE;
    //return CView::OnDrop(pDataObject, dropEffect, point);
}

void CPaigeEdtView::UpdateChangeState( ucs_verb ucsVerb /* = kDocOnly */ )
{
    switch ( ucsVerb ) {

    case kAddChange:
    case kRedoChange:
        m_chngCount++;
        break;
    case kUndoChange:
        ASSERT( m_chngCount > 0 );
        m_chngCount--;
        break;
    case kDocOnly:
        break;

    default:
        // we have been passed a bad 'verb'
        ASSERT(0);
    }

    // Note that we could also do some type of check for the 4th "gigachange",
    // but the likelyhood of reaching it at all is only less remote than
    // the chance that it would happen in a development build.

    // this may look expensive, but it's all in-line
    CDocument *pDoc = GetDocument();

	if ( !IsReadOnly() )
		pDoc->SetModifiedFlag( HasChanged() || pDoc->IsModified());
}


void CPaigeEdtView::OnEditSelectAll() 
{
    long textSize = pgTextSize(m_paigeRef);

	if (textSize)
	{
		select_pair		selAllText = { 0, textSize };
		select_pair		selSig;
		
		// If there's a signature - eliminate it from the Select All selection
		if ( find_signature(&selSig, &selAllText) )
			textSize = selSig.begin - 1;
		
		pgSetSelection(m_paigeRef, 0, textSize, 0, TRUE);
	}
}

void CPaigeEdtView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{       
    long textSize = pgTextSize(m_paigeRef);

    if (textSize)
    {
        long selectLeft, selectRight;
        pgGetSelection(m_paigeRef, &selectLeft, &selectRight);
        pCmdUI->Enable(!((selectLeft == 0) && (selectRight == textSize)));
    }
    else pCmdUI->Enable(FALSE);
}

int CPaigeEdtView::CheckSpelling(BOOL autoCheck /*=FALSE*/)
{
    int ret = 0;

	// Shareware: In reduced feature mode, you cannot right-click
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode

		MakeAutoURLSpaghetti(0, pgTextSize(m_paigeRef));

		// Reset whether or not we think redraw is needed
		m_spell.ResetIsRedrawNeeded();

		ret = m_spell.Check(this,0,autoCheck);

		// We used to update repeatedly when we changed the style to reflect
		// misspelled words inside of Check. That was *really* slow for messages
		// with a lot of misspelled words (we were even doing this when the text
		// wasn't scrolled into view). Now we only update when necessary after
		// changing the styles.
		if ( m_spell.IsRedrawNeeded() )
		{
			Invalidate();
			UpdateWindow();
		}

	/*      int stCh, endCh;
			GetEditCtrl().GetSel(stCh, endCh);
			if (GetFocus() == &GetEditCtrl() && stCh != endCh)
			ret = Spell.Check(&GetEditCtrl());
			else
			{
			GetEditCtrl().SetSel(0, 0);
			CEdit *sub = (CEdit *)m_pHeader->GetDlgItem(IDC_HDRFLD_EDIT3);
			if (sub) 
			ret = Spell.Check(sub, 0, TRUE); 
			if (!ret)
			ret = Spell.Check(&GetEditCtrl(), 0, autoCheck);
			}
	*/
	}
    
	return ret;
}

void CPaigeEdtView::MarkWordNotMisspelled(const char * in_szWord)
{
	m_spell.MarkWordNotMisspelled(this, in_szWord);
}

BOOL CPaigeEdtView::HasSelection()
{
    long first = 0;
	long last= 0;
    pgGetSelection(m_paigeRef, &first, &last);
	if (first == last)
		return FALSE;
	else 
		return TRUE;
	
}

BOOL CPaigeEdtView::DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
	m_bDidAFind = TRUE;
    return Find(0, szSearch, bMatchCase, bWholeWord, bSelect);
}

BOOL CPaigeEdtView::DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
    long offset=0;
    select_pair sel;

	if(!m_bDidAFind)
		return DoFindFirst(szSearch, bMatchCase, bWholeWord, bSelect);

    //
    // Begin search after the current selection
    //
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    offset = sel.end + 1;
        
    return Find(offset, szSearch, bMatchCase, bWholeWord, bSelect);
}


BOOL CPaigeEdtView::Find(long offset, const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
    select_pair sel = {0,0};
    long lMax = pgTextSize(m_paigeRef);
    bool bFound=FALSE;
    int ret;

    //
    // Make a local copy of the search string and the target string,
    // so that we can call MakeLower() on them, if necessary.
    //
    CString tmpSearch(szSearch);
    if (!bMatchCase )
        tmpSearch.MakeLower();

    text_ref    ref_for_text;
    long                length;
    pg_byte_ptr pText;

    while ( offset <= lMax )
    {
        //
        // Get a block of text
        //
        pText = pgExamineText(m_paigeRef, offset, &ref_for_text, &length);
        if (NULL == pText)
        {
            //
            // Fell off the end of the message, so break out of this loop.
            //
            break;
        }
        ASSERT(length > 0);
        CString         tmpText( (char*) pText,length );
        if (!bMatchCase )
            tmpText.MakeLower();
        //
        // Search for string
        //
        if ( (ret = tmpText.Find(tmpSearch)) >= 0 )
        {
            //
            // We've found the string but it could be a Red Herring
            // if Whole Word is checked.   Verify this.
            //
            if ( !bWholeWord )
                bFound = TRUE;
            else
            {
                pgFindWord(m_paigeRef, offset+ret, &sel.begin,&sel.end,TRUE,FALSE);
                if ( StrPgCmp( tmpSearch,sel ) == 0 )
                    bFound =TRUE;
                else
                    offset = sel.end+1;
            }

            if (bFound)
            {
                        
                sel.begin = offset+ret;
                sel.end = sel.begin + tmpSearch.GetLength();
                                //
                                // Could be hidden text in Taboo headers
                                //
                style_info                      info, mask;
                pgInitStyleMask(&info,0);
                pgInitStyleMask(&mask,0);
                pgGetStyleInfo(m_paigeRef,&sel,FALSE,&info,&mask);
                if (info.styles[hidden_text_var] == 1)
                {
                    bFound = FALSE;
                    offset = sel.end+1;
                }
                else
                {
                    UnuseMemory(ref_for_text);
                    break;
                }
            }
        }
        else
        {
            // No match in this block
            offset += length+1;
        }
        UnuseMemory(ref_for_text);
    }

    if ( bFound && bSelect )
    {
// BOG: don't want to grab the focus, just turn on highlighting and make sure it
// stays on until the (l)user clicks in the client area or types something.
//        SetFocus();
        m_bDontDeactivateHighlight = TRUE;
        pgSetHiliteStates(m_paigeRef, activate_verb, no_change_verb, TRUE); 
        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
        pgScrollToView(m_paigeRef, CURRENT_POSITION, 32, 32, TRUE, best_way);
        Invalidate();
        UpdateScrollBars();
    }

    return bFound;      
}

char* CPaigeEdtView::GetPgText(char* pDstText, int nMaxLen, select_pair& sel, bool bSelection, long * out_pTextLen)
{
	long		nTextLength = 0;
	
	*pDstText = '\0';

    if ( bSelection )
        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);

    memory_ref  refText = pgCopyText(m_paigeRef, &sel, all_data);

	if ( refText ) {
		char* pText=(char*) UseMemory(refText);
		//
		// Append a terminating NULL
		//
		AppendMemory(refText,sizeof(pg_byte),TRUE);
		nTextLength = strlen(pText);
		if (nTextLength <= nMaxLen)
			strcpy(pDstText,pText);
		else
		{
			nTextLength = nMaxLen-1;
			strncpy(pDstText, pText, nTextLength);
			pDstText[nTextLength] = NULL;
		}

		UnuseAndDispose(refText);
	}

	if (out_pTextLen)
		*out_pTextLen = nTextLength;

    return pDstText;
}

int CPaigeEdtView::StrPgCmp(
    const char* pSrcText,
    select_pair& sel,
    BOOL                        bMatchCase , // = TRUE
    BOOL                        bWholeWord,  // = FALSE
    BOOL                        bSelect)         // = FALSE
{

    //
    // Compare length of strings first
    //
    long lSrcLen = strlen(pSrcText);
    if ( lSrcLen > (sel.end-sel.begin)  )
        return 1;
    else if ( lSrcLen < (sel.end-sel.begin) )
        return -1;
    //
    // Strings are same size, so examine text
    //
    text_ref    ref_for_text;
    long                length;
    pg_byte_ptr pText;
    pText = pgExamineText(m_paigeRef, sel.begin, &ref_for_text, &length);
    //
    // Just to be sure that selection doesn't cross text boundaries
    //
    length = min(lSrcLen,length);
    //
    // Comare the strings
    //
    int ret = strncmp(pSrcText, (char*) pText, lSrcLen);

    UnuseMemory(ref_for_text);
        
    return ret;

}

void CPaigeEdtView::OnSpecialAddAsRecipient()
{
    select_pair sel;
    char buf[1024];

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    if ( sel.end != sel.begin )
    {
        GetPgText(buf, sizeof(buf), sel, FALSE);
        if (  (*buf != '\0') && (strchr(buf,'\r') == NULL ) )
            g_theRecipientDirector.AddCommand( buf );
    }
                                
}

void CPaigeEdtView::OnUpdateAddAsRecipient(CCmdUI* pCmdUI)
{       
    select_pair sel;
    char buf[1024];

    pCmdUI->Enable( FALSE );
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    if ( sel.end != sel.begin )
    {
        GetPgText(buf, sizeof(buf), sel, FALSE);
        if (  (*buf != '\0') && (strchr(buf,'\r') == NULL ) )
            pCmdUI->Enable( TRUE );
    }
}


BOOL CPaigeEdtView::GetTextAs( pg_ref theRef, CString &text, PgDataTranslator::FormatType format)
{
    text.Empty();

    // do the deed
    memory_ref pgmRef;
    PgDataTranslator theTxr( theRef );
    theTxr.ExportData( &pgmRef, format);

    // convert the PGMM reference to a c-string, and jam it in the doc. note
    // that anything in the paige instance (headers, whatever) is gonna
    // get saved as part of the message.
    char *theStr = (char *) UseMemoryToCStr( pgmRef );
        
    if (format == PgDataTranslator::kFmtText)
    {
        ConvertNBSPsToSpaces(theStr);
    }

    text = theStr;

    // get out before someone gets hurt
    UnuseAndDispose( pgmRef );

    if ( text.IsEmpty() )
        return FALSE;

    return TRUE;
}

//	Need to provide this default version, so that not every subclass
//	has to implement GetTocDoc.
CTocDoc * CPaigeEdtView::GetTocDoc()
{
	return NULL;
}

BOOL CPaigeEdtView::GetSelectedText( CString& szText )
{
    BOOL	result = FALSE;
	
	if (! pgNumSelections(m_paigeRef) )
        return FALSE;

    select_pair selection;
    pgGetSelection(m_paigeRef, &selection.begin, &selection.end);
    pg_ref selRef = pgCopy(m_paigeRef, &selection);

	if (selRef)
	{
		// need to stash ptr to the "stuffbucket"
		paige_rec_ptr pRec = (paige_rec_ptr) UseMemory( m_paigeRef );

		if (pRec)
		{
			paige_rec_ptr pRecCopy = (paige_rec_ptr) UseMemory( selRef );
			if (pRecCopy)
			{
				pRecCopy->user_refcon = pRec->user_refcon;
				UnuseMemory(selRef);
				result = GetTextAs(selRef, szText, PgDataTranslator::kFmtText);;
			}
			
			UnuseMemory(m_paigeRef);
		}

		pgDispose( selRef );
	}

    return result;
}

BOOL CPaigeEdtView::GetSelectedTextAndTrim(
		CString &				out_szText,
		bool					in_bAllowMultipleLines,
		bool					in_bDoSuperTrim,
		int						in_nMaxLength,
		bool					in_bUseElipsisIfTruncating)
{
	BOOL	result = GetSelectedText(out_szText);

	if (result)
	{
		//	Trim leading and trailing space
		out_szText.TrimLeft();
		out_szText.TrimRight();
		
		if ( (!in_bAllowMultipleLines && (out_szText.Find('\r') != -1)) )
		{
			out_szText.Empty();
			result = FALSE;
		}
		else
		{
			//	Truncate the string if necessary before replacing characters
			//	so that we can be sure that super trimming can't take too long.
			if ( in_nMaxLength && (out_szText.GetLength() > in_nMaxLength) )
			{
				out_szText.GetBufferSetLength(in_nMaxLength);

				//	Trim trailing space again in case we truncated immediately after some
				out_szText.TrimRight();

				if (in_bUseElipsisIfTruncating)
					out_szText += CRString(IDS_ELIPSIS);
			}

			if (in_bDoSuperTrim)
			{
				//	Replace any tabs with spaces
				out_szText.Replace('\t', ' ');

				//	Replace any CRs with spaces
				out_szText.Replace('\r', ' ');

				//	Replace any LFs with spaces
				out_szText.Replace('\n', ' ');

				//	Drop down any space runs to a single space
				while ( out_szText.Replace("  ", " ") );
			}
		}
	}

	return result;
}

BOOL CPaigeEdtView::GetAllText( CString& szText )
{
    if (! pgTextSize(m_paigeRef) )
        return ( FALSE );

    BOOL result = GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtText );
        
    return result;
}


BOOL CPaigeEdtView::GetSelectedHTML( CString& szHTML )
{
    if (! pgNumSelections(m_paigeRef) )
        return ( FALSE );

    select_pair selection;
    pgGetSelection(m_paigeRef, &selection.begin, &selection.end);
    pg_ref selRef = pgCopy(m_paigeRef, &selection);

    // need to stash ptr to the "stuffbucket"
    paige_rec_ptr pRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    paige_rec_ptr pRecCopy = (paige_rec_ptr) UseMemory( selRef );
    pRecCopy->user_refcon = pRec->user_refcon;
    UnuseMemory( m_paigeRef );
    UnuseMemory( selRef );

    BOOL result = GetTextAs(selRef, szHTML, PgDataTranslator::kFmtHtml);

    pgDispose( selRef );
    return result;
}

BOOL CPaigeEdtView::GetAllHTML( CString& szText )
{
    if (! pgTextSize(m_paigeRef) )
        return ( FALSE );

    BOOL result = GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtHtml );

    return result;
}

BOOL CPaigeEdtView::GetMessageAsHTML( CString& szText, BOOL IncludeHeaders )
{
    //Implemented in derived classes
    ASSERT(0);
    return (FALSE);
}

BOOL CPaigeEdtView::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
    //Implemented in derived classes
    ASSERT(0);
    return (FALSE);
}

//
//EXPECTS a selection pointer if text is to be replaced ELSE 
//inserts text at the current position or at the left selection position.
//
BOOL CPaigeEdtView::SetTextAs
( 
    const char* text, 
    select_pair_ptr sel,
    PgDataTranslator::FormatType format,
    bool bSign           /*= false*/,
	bool bPrepareUndo    /*= true*/,
	bool bScrollToCursor /*= true*/
)
{
    short verb = undo_translation;
    //
    // Paste=Delete+Insert.  Delete the current selection
    //
    if ( sel && (sel->begin < sel->end) )
    {
		if ( bPrepareUndo )
			PrepareUndo(undo_delete, (long *)sel, TRUE);

        pgDelete(m_paigeRef, sel, best_way);
        verb = undo_translation_replace;
    }
    //
    // Init the translator
    //
    memory_ref strRef = CString2Memory (PgMemGlobalsPtr(), (LPCTSTR)text);
    PgDataTranslator theTxr(m_paigeRef);

	if ( m_bAllowTables )
		theTxr.m_importFlags |= IMPORT_TABLES_FLAG;

    //
    // Save beginning offset of selection
    //
    long first, last;
    pgGetSelection(m_paigeRef, &first, &last);
    //
    // Insert text and caculate how much text was inserted.
    //
    long oldSize, newSize;
    oldSize = pgTextSize(m_paigeRef);
    theTxr.ImportData(&strRef, format);
    newSize = pgTextSize(m_paigeRef);

    long insert_ref[2];
    insert_ref[0] = first;
    insert_ref[1] = newSize - oldSize;

    // Store undo information
	if ( bPrepareUndo )
		PrepareUndo(verb, insert_ref);

    DisposeMemory(strRef);

    //
    // Sign the inserted text
    //
    if (bSign)
    {
        pgGetSelection(m_paigeRef, &first,&last);
        pgSetSelection(m_paigeRef, insert_ref[0], last,0, FALSE);
        m_styleEx->ApplySigned(TRUE);
        pgSetSelection(m_paigeRef, first, last,0, TRUE);
    }

	//
	// Mark it for spelling
	//
	select_pair sel2 = {first,first + newSize - oldSize};
	m_styleEx->ClearMisspellingAndRecheck(&sel2);
	m_checkdastuff = true;
	m_bMoodMailDirty = true;
        
	if ( bScrollToCursor )
		ScrollToCursor();
        
    return ( TRUE );
}

BOOL CPaigeEdtView::SetSelectedText( const char* szText, bool bSign )
{
    select_pair sel, newSel;

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    BOOL result = SetTextAs(szText, &sel, PgDataTranslator::kFmtText, bSign);

	// Select the new text, if the old selection was not an insertion point
	if (result && sel.begin!=sel.end)
	{
		pgGetSelection(m_paigeRef, &newSel.begin, &newSel.end);
		pgSetSelection(m_paigeRef, sel.begin, newSel.end, 0, true);
	}

    return result;
}

BOOL CPaigeEdtView::SetAllText( const char* szText, bool bSign )
{
    select_pair sel;

    sel.begin = 0;
    sel.end = pgTextSize(m_paigeRef);

    BOOL result = SetTextAs(szText, &sel, PgDataTranslator::kFmtText, bSign);

    return result;
}

BOOL CPaigeEdtView::SetSelectedHTML( const char* szHTML, bool bSign )
{
    select_pair sel, newSel;

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    BOOL result = SetTextAs(szHTML, &sel, PgDataTranslator::kFmtHtml, bSign);

	// Select the new text, if the old selection was not an insertion point
	if (result && sel.begin!=sel.end)
	{
		pgGetSelection(m_paigeRef, &newSel.begin, &newSel.end);
		pgSetSelection(m_paigeRef, sel.begin, newSel.end, 0, true);
	}

    return result;
}

BOOL CPaigeEdtView::SetAllHTML( const char* szHTML, bool bSign )
{
    select_pair sel;

    sel.begin = 0;
    sel.end = pgTextSize(m_paigeRef);

    BOOL result = SetTextAs(szHTML, &sel, PgDataTranslator::kFmtHtml, bSign);

    return result;
}

BOOL CPaigeEdtView::PasteOnDisplayHTML( const char* szHTML,bool bSign )
{
    return SetSelectedHTML(szHTML, bSign);
}


BOOL CPaigeEdtView::ClearFormat()
{
    par_info parInfo, parInfoMask;
    long styleID;
    if ( styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) )
        pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
    else
        parInfo = PgGlobalsPtr()->def_par;
    pgFillBlock( &parInfoMask, sizeof(par_info), SET_MASK_BITS );

    //
    // Don't clear excerpting
    //
    parInfoMask.user_data=0;
    parInfoMask.user_id = 0;
    parInfoMask.procs.line_glitter = 0;
    parInfoMask.left_extra =0;

    pgSetParInfo( m_paigeRef, NULL, &parInfo, &parInfoMask, best_way );
        
    return TRUE;
}

BOOL CPaigeEdtView::ClearStyle()
{
    //	Get the default style and font info
	style_info		defStyle;
    font_info		defFont;

    long styleID;
    if ( (styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) ) )
    {
        pgGetNamedStyleInfo(m_paigeRef, styleID, &defStyle, &defFont, NULL);
    }
    else
    {
        defStyle = PgGlobalsPtr()->def_style;
        defFont = PgGlobalsPtr()->def_font;
    }

	//	Get the current selection
    select_pair		selCurrent;
    pgGetSelection(m_paigeRef, &selCurrent.begin, &selCurrent.end);

	//	Prepare a style walk
	style_walk		sw;
	paige_rec_ptr	pPaige = reinterpret_cast<paige_rec_ptr>( UseMemory(m_paigeRef) );
		
	pgPrepareStyleWalk(pPaige, selCurrent.begin, &sw, TRUE);	// Include paragraph styles

	//	Keep track of the location of any styles that are *not* an embedded image
	vector<select_pair>		clearStyleList;
	select_pair				styleRange;
	bool					bCurrentStyleIsNotEmbeddedImage;

	styleRange.begin = selCurrent.begin;
	styleRange.end = selCurrent.begin;
	
	while (styleRange.end < selCurrent.end)
	{
		//	Check to see if the current style is an embedded image.
		//	We want to leave all embedded images alone, because removing
		//	styles from them destroys them.
		bCurrentStyleIsNotEmbeddedImage = (sw.cur_style->embed_id == defStyle.embed_id);
		
		if ( sw.next_style_run && (sw.next_style_run->offset <= selCurrent.end) )
		{
			//	Walk to the next style
			if ( pgWalkNextStyle(&sw) )
			{
				//	Current style ends where the next style begins
				styleRange.end = sw.current_offset;
			}
			else
			{
				//	Current style ends where the range ends
				styleRange.end = selCurrent.end;
			}
		}
		else
		{
			//	Current style ends where the range ends
			styleRange.end = selCurrent.end;
		}
		
		//	If it's not an embedded image add it to our list so that we can clear
		//	it's style
		if (bCurrentStyleIsNotEmbeddedImage)
			clearStyleList.push_back(styleRange);

		styleRange.begin = styleRange.end;
	}
	
	//	We're done walking, free the memory
	pgPrepareStyleWalk(pPaige, 0, NULL, TRUE);

	//	Done with pPaige - release Paige ref
	UnuseMemory(m_paigeRef);

	//	Does the selection contain something other than embedded images?
	if ( !clearStyleList.empty() )
	{
		style_info		styleMask;
		font_info		fontMask;

		//	Init the masks so that we blast everything
		pgFillBlock(&styleMask, sizeof(style_info), SET_MASK_BITS);
		pgFillBlock(&fontMask, sizeof(font_info), SET_MASK_BITS);
		
		vector<select_pair>::iterator	i;

		//	Run through the list and clear the styles
		for (i = clearStyleList.begin(); i != clearStyleList.end(); i++)
		{
			//	For another bug - we may want to somehow save and restore or
			//	avoid clearing mispelled and mood styles here
			pgSetStyleInfo(m_paigeRef, &(*i), &defStyle, &styleMask, draw_none);
			pgSetFontInfo(m_paigeRef, &(*i), &defFont, &fontMask, draw_none);
		}

		//	Make sure we update eventually
		Invalidate();
	}

    return TRUE;
}

BOOL CPaigeEdtView::ClearHyperlink()
{
    //
    // Get Selection range
    //
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    pg_hyperlink hyperlink;
    long offset = sel.begin;
    while (( pgGetHyperlinkSourceInfo (m_paigeRef, offset, 0, true, &hyperlink) ) &&
           (hyperlink.applied_range.begin < sel.end))
    {
        if ( !((hyperlink.type & HYPERLINK_EUDORA_ATTACHMENT)   ||
               (hyperlink.type & HYPERLINK_EUDORA_PLUGIN          )     ||
               (hyperlink.type & HYPERLINK_EUDORA_AUTOURL   ) ))
            pgDeleteHyperlinkSource (m_paigeRef, hyperlink.applied_range.begin, true);
        offset = hyperlink.applied_range.end+1;
    }
        
    return TRUE;

}

void CPaigeEdtView::OnFontChange()
{
	CString szString;
	CComboBox* pCombo;
	QCChildToolBar* pFormattingToolBar;

	// go through hell to get the stinkin' combobox
	CFrameWnd* Frame = (CFrameWnd*) GetParentFrame();
	QCProtocol* pProtocol =
			QCProtocol::QueryProtocol( QCP_FORMAT_TOOLBAR, Frame );

	if( pProtocol == NULL ) 
		return;
	
	SetFocus();   // this stops the toolbar from stealing focus

	pFormattingToolBar = pProtocol->GetFormatToolbar();
	VERIFY(pCombo = (CComboBox*)pFormattingToolBar->GetDlgItem(IDC_FONT_COMBO));

	if( pCombo->GetCurSel() < 0 )
		return;

	// gets us the font name
	pCombo->GetLBText( pCombo->GetCurSel(), szString );

	// are we changing existing text?
	if ( pgNumSelections( m_paigeRef ) )
		PrepareUndo( undo_format );

	// BOG: ok, turns out that "pgSetFontByName" is no good---doesn't work for
	// non-ansi charsets, etc.; we use "pgSetFontByLog" instead. we'll use
	// CFontInfo to fill out a logfont struct for us. the point size doesn't
	// matter---paige, unlike windows, deals with fonts & styles separately.

	CFontInfo theFont;
	theFont.CreateFont( szString, 12, true );

	LOGFONT lf;
	theFont.GetLogFont( &lf );
	pgSetFontByLog( m_paigeRef, &lf, NULL, TRUE );
}

void CPaigeEdtView::OnUpdateFontCombo(CCmdUI* pCmdUI) 
{
    CComboBox*                  pCombo;
    QCChildToolBar*             pFormattingToolBar;

    CFrameWnd* Frame = (CFrameWnd*) GetParentFrame();
    QCProtocol* pProtocol = QCProtocol::QueryProtocol( QCP_FORMAT_TOOLBAR, Frame );

    if( pProtocol == NULL ) 
        return ;
        
    pFormattingToolBar = pProtocol->GetFormatToolbar();
    VERIFY( pCombo = ( CComboBox* )  pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ));
        
    if ( !pCombo->GetDroppedState() )
    {
        char szFontName[256]={NULL};
        pgGetFontByName (m_paigeRef, (unsigned char*) &szFontName)      ;
        if( *szFontName != NULL )
        {
            int i = pCombo->FindStringExact( -1, szFontName );
            if ( pCombo->GetCurSel() != i )
                pCombo->SetCurSel( i );
        }
    }
        
    pCmdUI->Enable( TRUE );
        
}



// Happy-crappy picture stuff...
void CPaigeEdtView::OnInsertPicture()
{
	// Shareware: In reduced feature mode, you cannot insert pictures
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		char* filter = NULL;
		PgGetOfnFilterString( &filter );
		CFileDialog dlg( true, "jpg", NULL, 0, filter, this );

		if ( dlg.DoModal() == IDOK ) {
			unsigned er;
			if ( (er = PgValidateImageFile(dlg.GetPathName())) == 0 ) {
				//Need to call Undo here, maybe with app_insert verb
				PrepareUndo(undo_embed_insert);
				// Schmookie!
				PgInsertImageFromFile( m_paigeRef, dlg.GetPathName() );
				Invalidate();
			}
			else
				ErrorDialog( er );
		}
	}
}

void CPaigeEdtView::OnInsertPictureLink()
{
	// Shareware: In reduced feature mode, you cannot insert pictures
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		CPictureLinkDlg dlg;

		if (dlg.DoModal() == IDOK)
			SetSelectedHTML(dlg.GetHtmlCode(), false);
	}
}

void CPaigeEdtView::OnUpdateInsertPicture(CCmdUI* pCmdUI) 
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    pCmdUI->Enable( !m_fRO && m_bAllowStyled /*&& g_qtInitialized*/ );
}

void CPaigeEdtView::OnEditInsertHR()
{
	sec_toolbar_menu_hack();

	// FULL FEATURE MODE
	if ( UsingFullFeatureSet() ) {
		select_pair sel;
		pgGetSelection( m_paigeRef, &sel.begin, &sel.end );

		// HRs must have their own par. rather that trying to split line-wrapped
		// pars in two, we just always put the rule above the current par. this
		// simplifies the code a lot, and sidesteps some pretty flakey behavior.
		// the user must split pars themselves, if that's what they want.

		// BOG: the one remaining bug here is that if the previous line is an
		// HR, then instead of getting a second one, the existing one just kinda
		// gets pushed down a line---i give up.

		long parLeft, parRight;
		pgFindPar( m_paigeRef, sel.begin, &parLeft, &parRight );

		long insert_Ref[2];
		insert_Ref[0] = parLeft;
		insert_Ref[1] = 1;
		PrepareUndo( undo_app_insert, insert_Ref, TRUE );

		TCHAR cr = (TCHAR) PgGlobalsPtr()->line_wrap_char;
		pgInsert( m_paigeRef, (pg_char_ptr)&cr, sizeof(cr), parLeft,
				data_insert_mode, 0, best_way );

		par_info info, mask;
		pgInitParMask( &info, 0 );
		pgInitParMask( &mask, 0 );

		mask.table.border_info = -1;
		info.table.border_info |= PG_BORDER_LINERULE;

		sel.begin = sel.end = parLeft;
		PrepareUndo( undo_insert_hr_withCR );
		pgSetParInfoEx( m_paigeRef, &sel, &info, &mask, FALSE, best_way );
	}
}

void CPaigeEdtView::OnUpdateEditInsertHR( CCmdUI* pCmdUI )
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    OnUpdateEditStyle( pCmdUI );
}

void CPaigeEdtView::OnUpdateEditStyle( CCmdUI* pCmdUI )
{

    // Not to be confused with OnUpdateEdible.
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( FALSE );
    else
        pCmdUI->Enable( TRUE );
}

void CPaigeEdtView::OnUpdateDynamicCommand(CCmdUI * pCmdUI)
{
	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	

	//	Is the command an emoticon command?
	if( g_theCommandStack.Lookup( static_cast<WORD>(pCmdUI->m_nID), &pCommand, &theAction) &&
		(theAction == CA_EMOTICON) )
	{
		//	Disabled if in light mode or if message is read only
		if (!UsingFullFeatureSet() || m_fRO)
			pCmdUI->Enable(FALSE);
		else
			pCmdUI->Enable(TRUE);
	}
	else
	{
		//	Command is not an emoticon command - continue routing
		pCmdUI->ContinueRouting();
	}
}

void CPaigeEdtView::OnTextSizeChange( UINT uSizeID ) 
{
    if ( pgNumSelections( m_paigeRef ) ) {
        PrepareUndo( undo_format );
    }

    unsigned i = (uSizeID - ID_EDIT_TEXT_SIZE_VERY_SMALL);
    int baseFontSize = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);
    short size = (short)(GetFontSizeMap()[(int)baseFontSize][i] >> 16);
    pgSetPointSize( m_paigeRef, size, NULL, TRUE );
    sec_toolbar_menu_hack();
}


void CPaigeEdtView::OnColorChange( UINT uColorID ) 
{
	if (UsingFullFeatureSet())
	{
		if ( pgNumSelections( m_paigeRef ) ) {
			PrepareUndo( undo_format );
		}

		COLORREF theColor =  CColorMenu::GetColor( uColorID );

		pgSetTextColor( m_paigeRef, &theColor, NULL, TRUE );
	}
}


BOOL CPaigeEdtView::OnDynamicCommand(UINT uID) 
{
	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	
	
	if( !g_theCommandStack.GetCommand(static_cast<WORD>(uID), &pCommand, &theAction) )
		return FALSE;

	if ( UsingFullFeatureSet() && (theAction == CA_EMOTICON) )
	{
		//	Find indicated emoticon
		Emoticon *		pEmoticon = NULL;
		g_theEmoticonDirector.LookUpEmoticonIDMap(static_cast<unsigned short>(uID), pEmoticon);

		ASSERT(pEmoticon);

		if (pEmoticon)
		{
			InsertEmoticonAtCurrentPosition(pEmoticon);

			return TRUE;
		}
	}
	
	return FALSE;
}


void CPaigeEdtView::OnFormatPainter() 
{
	if (m_mouseState & MOUSE_FORMAT_PAINTER)
	{
		//	Format Painter is already on - cancel it now
		m_mouseState &= ~MOUSE_FORMAT_PAINTER;
	}
	else
	{
		//	Copy the appropriate styles and initiate Format Painter
		style_info		styleMask;
		font_info		fontMask;
		par_info		parMask;
		long			nParaBegin = 0;
		long			nParaEnd = 0;
		select_pair		sel;
		
		//	Get the selection
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
		
		//	Look for a selected paragraph
		FindSelectedParagraph(sel, nParaBegin, nParaEnd);
		
		//	We always copy style info for Format Painter
		m_bCopiedStyleInfo = true;
		
		//	We copy par info for Format Painter if:
		//	* There's at least one paragraph selected OR
		//	* The selection is a single insertion point
		m_bCopiedParInfo = ( (nParaBegin >= sel.begin) && (nParaEnd <= sel.end) ) || (sel.begin == sel.end);
		
		//	Modify the selection to make sure that we only get the style for the
		//	beginning of the selection
		sel.end = sel.begin + 1;
		
		//	Get the style info
		pgGetStyleInfo(m_paigeRef, &sel, FALSE, &m_styleInfo, &styleMask);

		//	Get the font info
		pgGetFontInfo(m_paigeRef, &sel, FALSE, &m_fontInfo, &fontMask);
		
		//	Copy the par info if appropriate
		if (m_bCopiedParInfo)
			pgGetParInfo(m_paigeRef, &sel, FALSE, &m_parInfo, &parMask);
		
		//	Set our mouse state to format painter
		m_mouseState |= MOUSE_FORMAT_PAINTER;
	}
}


void CPaigeEdtView::OnUpdateFormatPainter(CCmdUI* pCmdUI) 
{
	if ( UsingFullFeatureSet() )
	{
		//	If we'll be enabling Format Painter, do that before we call SetCheck
		if (m_bAllowStyled)
			pCmdUI->Enable(true);
		
		pCmdUI->SetCheck( (m_mouseState & MOUSE_FORMAT_PAINTER) ? 1 : 0 );
		
		//	If we'll be disabling Format Painter, do that after we call SetCheck
		if (!m_bAllowStyled)
			pCmdUI->Enable(false);
	}
	else
	{
		//	We're in Light mode - disable Format Painter
		pCmdUI->SetCheck(false);
		pCmdUI->Enable(false);
	}
}


void CPaigeEdtView::OnClearFormatting() 
{
	// Always PrepareUndo, because even if the selection is a single insertion point the 
	// below changes will have an affect that needs to be undoable.
    PrepareUndo(undo_format);
    ClearStyle();
    ClearFormat();      
    ClearHyperlink();
}


void CPaigeEdtView::OnUpdateClearFormatting(CCmdUI* pCmdUI) 
{
    select_pair sel;

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
	bool bExcerptCausedStyle;
    if ( PgHasStyledText(m_paigeRef, PgGlobalsPtr(), &sel, &bExcerptCausedStyle) && !bExcerptCausedStyle)
        pCmdUI->Enable( TRUE );
    else
        pCmdUI->Enable( FALSE );  
}

void CPaigeEdtView::UpdateBlockFmt(CCmdUI* pCmdUI, unsigned char* szStylename) 
{
    if (m_fRO)
        pCmdUI->Enable( FALSE );
    else
        pCmdUI->Enable( TRUE );
        
    short styleid;
    pgGetStyleSheet(m_paigeRef, NULL, &styleid);
    long id = pgGetNamedStyleIndex(m_paigeRef,szStylename);
    if ( styleid == id )
        pCmdUI->SetRadio( TRUE );
    else
        pCmdUI->SetRadio( FALSE );
}



void CPaigeEdtView::OnBlkfmtBullettedList() 
{
 	// Shareware: In reduced feature mode, you cannot insert bullet lists
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		if ( pgNumSelections(m_paigeRef) )
			PrepareUndo(undo_format);
		if ( !m_styleEx->IsBullet(NULL) )
			m_styleEx->ApplyBullet(TRUE);
		else
			m_styleEx->ApplyBullet(FALSE);
	}
}

void CPaigeEdtView::OnUpdateBlkfmtBullettedList(CCmdUI* pCmdUI) 
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

	if ( m_styleEx->IsBullet(NULL) )
		pCmdUI->SetCheck( TRUE );
	else
		pCmdUI->SetCheck( FALSE );

	pCmdUI->Enable(m_bAllowStyled);
}

/*

void CPaigeEdtView::OnBlkfmtBq() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, blockquote_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtBQ(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, blockquote_style );
}

void CPaigeEdtView::OnBlkfmtH1() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading1_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH1(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading1_style );
}

void CPaigeEdtView::OnBlkfmtH2() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading2_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH2(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading2_style );
}

void CPaigeEdtView::OnBlkfmtH3() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading3_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH3(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading3_style );
}

void CPaigeEdtView::OnBlkfmtH4() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading4_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH4(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading4_style );
}

void CPaigeEdtView::OnBlkfmtH5() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading5_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH5(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI,heading5_style );
}

void CPaigeEdtView::OnBlkfmtH6() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading6_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH6(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI,heading6_style );
}


void CPaigeEdtView::OnBlkfmtNormal() 
{
//      OnClearFormatting();
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, body_style,  best_way);

}

void CPaigeEdtView::OnUpdateBlkfmtNormal(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI,body_style );

}
*/
//
//  This is a helper function for the tri-state toolbar buttons.  
//  1 = style is consistently on.
//  0 = style is consistently off.
//  2 = style is both on and off in this selection.
//
int CPaigeEdtView::GetStyleState (int index)
{
    style_info                  info, mask;
    pgInitStyleMask(&info,0);
    pgInitStyleMask(&mask,0);

    //
    // Style is consistent throughout selection
    //
    pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
    if ( (info.styles[index] != 0) && ( mask.styles[index] != 0 ) )
        return 1;
    //
    // Style is indeterminate
    // 
    info.styles[index] = -1; 
    pgGetStyleInfo(m_paigeRef, NULL, TRUE, &info, &mask);
    if ( info.styles[index] != 0 )
    {
        if ( mask.styles[index] != 0 )
            return 2;
    }
    //
    // Style is not present
    //
    return 0;

        
}


int CPaigeEdtView::GetJustifyState (int index)
{
    par_info                    info, mask;
    pgInitParMask(&info,0);
    pgInitParMask(&mask,0);

    //
    // Style is consistent throughout selection
    //
    pgGetParInfo(m_paigeRef, NULL, FALSE, &info, &mask);
    if ( (info.justification == index) && ( mask.justification != 0 ) )
        return 1;
    //
    // Style is indeterminate
    // 
    info.justification = (short) index; 
    pgGetParInfo(m_paigeRef, NULL, TRUE, &info, &mask);
    if ( info.justification == index )
    {
        if ( mask.justification != 0 )
            return 2;
    }
    //
    // Style is not present
    //
    return 0;
        
}

int CPaigeEdtView::MapScrollPos(int nPos, int sourceMax, int dstMax)
{
    int ret = 0;
        
    if (sourceMax)
    {
        float ratio = (float)(nPos)/(float)sourceMax;
        ret = (int)((ratio * (float)dstMax)+.5);
                
        ret = min(dstMax,ret);
    }   
    return ret;
}

void CPaigeEdtView::OnUpdateNeedSelEdit(CCmdUI* pCmdUI)
{
    if (m_fRO)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(pgNumSelections(m_paigeRef));            
}

void CPaigeEdtView::OnEditWrapSelection()
{
    BOOL DoUnwrap = ShiftDown();
    if( !m_fRO)
    {
        OnEditCopy();
        if (OpenClipboard())
        {
            HANDLE hText = GetClipboardData(CF_TEXT);
            if (hText)
            {
                char* text = (char*)GlobalLock(hText);
                if (DoUnwrap)
                    UnwrapText(text);
                else
                    text = WrapText(text);
                if (text)
                {
                    SetSelectedText(text,FALSE);
                    if (!DoUnwrap)
                        delete [] text;
                }
                GlobalUnlock(hText);
            }
        }
        CloseClipboard();
    }
}

//extern "C" PG_PASCAL (long) hlCallback( paige_rec_ptr pg, pg_hyperlink_ptr hypertext,
//                      short command, short modifiers, long position, pg_char_ptr URL );


void CPaigeEdtView::OnUpdateInsertLink(CCmdUI* pCmdUI) 
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

	bool bEnable = false;

	if (m_bAllowStyled)
	{
		select_pair sel;
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
		//
		// If caret/selection contains a hyperlink, then the functionality is enabled
		// unless that hyperlink is a special, Eudora generated URL.
		//
		pg_hyperlink hyperlink;
		if ( pgGetHyperlinkSourceInfo (m_paigeRef, sel.begin, 0, false, &hyperlink) ||
			 pgGetHyperlinkSourceInfo (m_paigeRef, sel.end, 0, false, &hyperlink) )
		{
			bEnable = !((hyperlink.type & HYPERLINK_EUDORA_ATTACHMENT)  ||
						(hyperlink.type & HYPERLINK_EUDORA_PLUGIN         )     ||
						(hyperlink.type & HYPERLINK_EUDORA_AUTOURL   ) );                       
		}
		else
			//
			// Not in a hyperlink so must have a selection range to be valid
			// 
			bEnable = (sel.begin != sel.end);
	}

	pCmdUI->Enable( bEnable );
}

void CPaigeEdtView::OnEditInsertLink()
{
	// Shareware: In reduced feature mode, you cannot insert hyperlinks
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		char pURL[INTERNET_MAX_URL_LENGTH];

		pURL[0] = 0;
		//
		// Editing or Creating a Hyperlink?
		//
		select_pair sel;
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

		pg_hyperlink hyperlink;
		bool bEdit=FALSE;
		if ( (pgGetHyperlinkSourceInfo (m_paigeRef, sel.begin, 0, false, &hyperlink)) ||
			 (pgGetHyperlinkSourceInfo (m_paigeRef, sel.end  , 0, false, &hyperlink)))
		{
			sel.begin = min(sel.begin, hyperlink.applied_range.begin);
			sel.end   = max(sel.end, hyperlink.applied_range.end); 
			pgSetSelection(m_paigeRef, sel.begin, sel.end,0,true);
			//The URL source may have been stored in URL (char *) or 
			//cloaked in alt_URL (memory_ref) So use the function to get 
			//the URL.
			//pURL = hyperlink.URL;
			pgGetSourceURL(m_paigeRef, hyperlink.applied_range.begin, (unsigned char*)pURL, INTERNET_MAX_URL_LENGTH);
			bEdit = TRUE;
		}
		CInsertLinkDialog dlg((const char*)pURL);
		pgSetHiliteStates(m_paigeRef, activate_verb,activate_verb , FALSE);
		m_bDontDeactivateHighlight=TRUE;
		if ( dlg.DoModal() != IDOK)
			return;
		m_bDontDeactivateHighlight=FALSE;

		//Manually set the modified flag since there is no undo support for this.
		GetDocument()->SetModifiedFlag();

		CString szURL = EscapeURL( dlg.GetURL() );
		strncpy(pURL, szURL, sizeof(pURL));
		pURL[sizeof(pURL) - 1] = 0;

		if (m_styleEx)
		{
			//m_styleEx->ApplyMisspelled(FALSE,&sel);
			m_styleEx->SetNeedsScanned(TRUE,&sel);
			//m_styleEx->ApplyBadMoodWord(FALSE, &sel,0);
		}

		if (bEdit)
			pgChangeHyperlinkSource (m_paigeRef, hyperlink.applied_range.begin,     &sel,
									 (unsigned char*)pURL,NULL, hlCallback, 0, 0,0, best_way);
		else
		{
			paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
			pgSetHyperlinkSource (m_paigeRef, NULL,(unsigned char*) pURL,NULL, hlCallback,
								  HYPERLINK_EUDORA_URL,  pgAssignLinkID(pgPtr->hyperlinks),0,0,0, best_way);
			UnuseMemory(m_paigeRef);
		}
								
        
		//delete [] pURL;
	}
}


//theStr is an IN-OUT parameter. If the scheme is www. or ftp. then
// http:// and ftp:// are appended to the beginning of the string and
// this resultant string should be used to set the href
BOOL ValidURLScheme(char *theStr)
{
    char buffer[INTERNET_MAX_URL_LENGTH];

    if (strnicmp(theStr, "www.", 4) == 0)
    {
        strcpy(buffer, "http://");
        strncat(buffer, theStr, min(strlen(theStr), INTERNET_MAX_URL_LENGTH-8));
        strcpy(theStr, buffer);
        return TRUE;
    }
    else if (strnicmp(theStr, "ftp.", 4) == 0)
    {
        strcpy(buffer, "ftp://");
        strncat(buffer, theStr, min(strlen(theStr), INTERNET_MAX_URL_LENGTH-7));
        strcpy(theStr, buffer);
        return TRUE;
    }
        
    if (IsURLScheme(theStr))
        return TRUE;
    else 
        return FALSE;
}


BOOL CPaigeEdtView::MakeAutoURLSpaghetti(long start, long end)
{
    long linkID = 0;
    select_pair sel;
    char buf[INTERNET_MAX_URL_LENGTH], tempBuf[INTERNET_MAX_URL_LENGTH], startChar = ' ';
    BOOL bValidate;
	pg_hyperlink autoLink;
	
    InitHtmlStrings();

    paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

    //Validate existing AutoURLs first since text may have been added 
    //making them invalid or stale.

    long offset = start;
	long nextOffset;

    while (offset < end)
    {
        if ( pgGetHyperlinkSourceInfo(m_paigeRef, offset, 0, true, &autoLink) )
        {
            if (autoLink.applied_range.begin >= end) 
                break;

			// Grab our next offset before we make any adjustment to the end of it.
			// Fixes bug where if we updated the end offset below, but didn't call
			// pgChangeHyperlinkSource (because URLIsValid returned false) we would
			// be stuck in an infinite loop where we would always find the same URL
			// over and over again.
			nextOffset = autoLink.applied_range.end + 1;

            if (autoLink.type & HYPERLINK_EUDORA_AUTOURL)
            {
                //Check if the beginning is valid, if it is not, delete the link
                //Any valid link will be found later
                long charInfo = 0;
                if (offset > start)
                    charInfo = pgCharType(m_paigeRef, offset - 1, UPPER_CASE_BIT | LOWER_CASE_BIT);

                if ((offset > start) && ( charInfo & (UPPER_CASE_BIT | LOWER_CASE_BIT) ))
                    pgDeleteHyperlinkSource (m_paigeRef, autoLink.applied_range.begin, true);
                else
                {
                    startChar = ' ';
                                        
                    //if (offset > start)
                    {
                        unsigned char buf[8];
                        //pgCharByte(m_paigeRef, offset - 1, buf);
                        pgCharByte(m_paigeRef, offset, buf);
                        startChar = buf[0];
                        if (startChar != '"' && startChar != '<' && startChar != '\'' &&
							startChar != '(' && startChar != '[' && startChar != '{')
						{
                            startChar = ' ';
						}
                    }
					
					bValidate = FALSE;
                    //The URL beginning is still valid, now check if the end is valid
                    offset = FindValidURLEnd(m_paigeRef, autoLink.applied_range.begin, end, autoLink.applied_range.begin, startChar, bValidate);
                    if (offset != autoLink.applied_range.end)
                    {
                        //Update the end offset for the URL
                        autoLink.applied_range.end = offset;
                    }
                    //Sanity check for URL
                    if ((autoLink.applied_range.begin != autoLink.applied_range.end)&& bValidate)
                    {
                        GetPgText(buf, sizeof(buf), autoLink.applied_range, FALSE);
                        if (ValidURLScheme(buf))
                        {
                            if (URLIsValid(buf))
                            {
                                strncpy(tempBuf, EscapeURL(buf), sizeof(tempBuf));
                                tempBuf[sizeof(tempBuf) - 1] = 0;
                                pgChangeHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, 
                                                        &autoLink.applied_range, (unsigned char *)tempBuf, NULL, NULL, NULL, NULL, NULL, best_way);
								m_styleEx->ApplyMisspelled(false, &autoLink.applied_range);

                            }
                        }
                        else
                            pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);
                    }
                    else
                        pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);
                }
            }

			offset = nextOffset;
        }
        else 
		{
            break;
		}
    }

    //Parse the text for any new AutoURLs (new message / since last saved)
    sel.begin = start;
    startChar = ' ';

    while (sel.begin < end)
    {
        //Finds the beginning of the next word, skipping all spaces and tabs and newlines.
		sel.begin = FindValidURLStart(m_paigeRef, sel.begin, end, startChar);
        if (sel.begin == end)
            break;

        //If it is already a hyper link, don't mess with it, not even AutoURLs since they have been
        //processed in the loop above
        if ( linkID = pgGetSourceID(m_paigeRef, sel.begin) )
        {
            pgGetHyperlinkSourceInfo(m_paigeRef, sel.begin, linkID, TRUE, &autoLink);

            //End of the hyperlink is where our checking begins !!
            pgFindHyperlinkSourceByID(m_paigeRef, sel.begin, &sel.begin, linkID, FALSE);
                        
            //Finds the beginning of the next word, skipping all spaces and tabs and newlines.
            sel.begin = FindValidURLStart(m_paigeRef, sel.begin, end, startChar);
                        
            //Don't walk off the end
            if (sel.begin == end)
                break;
        }

        //Find the ending of a possible auto URL
		bValidate = FALSE;
		sel.end = FindValidURLEnd(m_paigeRef, sel.begin, end, sel.begin, startChar,bValidate);
        
		if (sel.begin != sel.end)
        {
            if ( sel.end - sel.begin > sizeof(buf) )
                sel.end = sel.begin + sizeof(buf) - 1;

            GetPgText(buf, sizeof(buf), sel, FALSE);
			if (bValidate)
			{
				if ( *buf && ValidURLScheme(buf) )
				{
					strncpy(tempBuf, EscapeURL(buf), sizeof(tempBuf));
					tempBuf[sizeof(tempBuf) - 1] = 0;
									//Make the AutoURL
					m_styleEx->ApplyMisspelled(false, &sel);
					pgSetHyperlinkSource (m_paigeRef, &sel,(unsigned char*)tempBuf, NULL, hlCallback,
										  HYPERLINK_EUDORA_AUTOURL, pgAssignLinkID(pgPtr->hyperlinks),0,0,0, best_way);

				}
			}
        }

        if (sel.end == end)
            break;
                
        sel.begin = sel.end;
    }
        
    UnuseMemory(m_paigeRef);
    return TRUE;
}

long FindNextWhitespace(pg_ref pg, long offset, long end)
{
    long dontCareAboutThis;
    long charInfo;
    char theChar;

    charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

    while ((offset < end) && !(charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT)))
    {
        unsigned char buf[8];
        int byteCount = pgCharByte(pg, offset, buf);
        if (byteCount > 1 || (buf[0] & 0x80))
        {
            offset += byteCount;
            continue;
        }
        theChar = buf[0];
        if (theChar == '<' || theChar == '"')
            break;
                
        offset++;
        if (offset == end)
            break;
                
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
        if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
            break;
                
        pgFindWord(pg, offset, &dontCareAboutThis, &offset, TRUE, FALSE);
                
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
    }

    return offset;
}

long SkipWhiteSpaces(pg_ref pg, long offset, long end)
{
    long charInfo;

    charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

    while ((offset < end) && (charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT)))
    {
        offset++;
        if (offset == end)
            break;
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
    }

    return offset;
        
}

/*********************************************************
Skips all whitespaces and punctuations to find the 
beginning of a URL. The startChar is defaulted to a
space and is replaced with a valid URL start punctuation 
like < or ". This startChar should be used to determine 
what the valid end of the URL is, the closing punctuation 
> or " OR a whitespace
**********************************************************/
long FindValidURLStart(pg_ref pg, long offset, long end, char& startChar)
{
    //FOR REFERENCE PURPOSES
    //Paige considers the following as punctuation chars
    //       ! # $ % & * ( ) + - = : " ; ' < > ? , . /
    //Paige considers the following as lower case
    //       ~ @ ^ _ \ { } | \ [ ] 

    long charInfo;
    char theChar;
    BOOL notfound = TRUE;

    startChar = ' ';

    while ( notfound && (offset < end) )
    {
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT | 
                              PUNCT_NORMAL_BIT | UPPER_CASE_BIT | LOWER_CASE_BIT);

        unsigned char buf[8];
        int count = pgCharByte(pg, offset, buf);
        if (count > 1)
        {
            // Multibyte character definitely isn't something we're looking for
            offset += count;
            continue;
        }
        theChar = buf[0];
        if (theChar & 0x80)
        {
            // 8-bit characters aren't matches either
            offset++;
            continue;
        }

        // Skip if whitespace
        if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
        {
            startChar = ' ';
            offset++;
            // Whitespace chars are also lower case, so don't process it again
            continue;
        }

        //Skip if punctuation
        if (charInfo & PUNCT_NORMAL_BIT)
        {
            // Remember the char if it is a valid URL enclosing char
            if ( (theChar == '"') || (theChar == '<') || (theChar =='(') || 
				(theChar =='[') || (theChar =='\'')||(theChar =='{'))
                startChar = theChar;
            offset++;
            continue;
        }

        if (charInfo & LOWER_CASE_BIT)
        {
            if (theChar != '~' && theChar != '@' && theChar != '^'
                && theChar != '_' && theChar != '`' && theChar != '{' 
                && theChar != '}' && theChar != '|' && theChar != '[' 
                && theChar != ']' && theChar != '\\')
                notfound = FALSE;
            else 
			{
	            if ( (theChar == '"') || (theChar == '<') || (theChar =='(') || 
					(theChar =='[') || (theChar =='\'')||(theChar =='{'))
			        startChar = theChar;
                offset++;
			}
            continue;
        }

        if (charInfo & UPPER_CASE_BIT)
        {
            notfound = FALSE;
            continue;
        }

        // Maybe a digit or some other character that hasn't been covered by 
        // the cases above
        offset++;
    }

    return offset;
}

long FindValidURLEnd(pg_ref pg, long offset, long end, long begin, char delimiter,BOOL& bFound)
{
    BOOL found = FALSE;
    unsigned char theChar[8];
    long temp = offset;
	//05-05-2000 valid delimiters are ",'.(,[,<
    if ( (delimiter == '"') || (delimiter == '<') ||(delimiter =='(') ||
				(delimiter =='[')  ||(delimiter =='\'')||(delimiter =='{'))
    {
        //Make sure we check for the correct enclosing char
        //if ( delimiter == '<' )
        //   delimiter = '>';
		switch(delimiter)
		{
			case '<' :
				delimiter = '>';
				break;

			case '[' :
				delimiter = ']';
				break;
			
			case '(':
				delimiter = ')';
				break;

			case '{':
				delimiter = '}';
				break;
		}

        int startCount = 1;

        while ( !found && (temp < end) )
        {
            pgCharByte(pg, temp, theChar);
                        
            //if we have a URL in <>, then it can have embedded <>
            /*if ((*theChar == '<')||(*theChar == '[')||(*theChar == '(')||(*theChar == '{'))
                startCount++;
             */           
            if (*theChar == delimiter)
            {
                startCount--;
                                
                if (startCount == 0)
                    found = TRUE;
                else 
                    temp++;
            }
            else
			{
				if (*theChar =='\r')
				{
				     bFound = FALSE;
					 return temp;
				}
				else
                  temp++;
			}
        }
		//if delimters are brackets then just return do not backtrace.
        if (found )
		{
			bFound = TRUE;
			if ((delimiter == '>') || (delimiter == ']') || (delimiter == ')') 
				|| (delimiter == '}'))
			{
				return temp;
			}
		}
		else 
		{
			bFound = FALSE;
			return temp;
		}
    }
	
	if (delimiter == ' ')
	{
		bFound = TRUE;
	}

    // Either we haven't found the matching " or > or other delimiters  
    // the URL delimiter is a space
    long whitespace = FindNextWhitespace(pg, offset, end);

    long charInfo;

    //Now traverse backwards to see if we have a punctuation 
    //at the end of the URL
    if (whitespace > offset)
        --whitespace;

    while (whitespace > offset)
    {
        pgCharByte(pg, whitespace, theChar);

        unsigned char c = *theChar;
        if ( c == '/' || c == '\\' )
            break;

        // If it is one of those lower case punctuation thingies, it's not valid
        if (c == '~' || c == '@' && c == '^'
            || c == '_' || c == '`' || c == '{' 
            || c == '}' || c == '|' || c == '[' 
            || c == ']' /*|| c == '\'*/)
        {
            --whitespace;
            continue;
        }


        charInfo = pgCharType(pg, whitespace, PUNCT_NORMAL_BIT);
                
        if ( ! (charInfo & PUNCT_NORMAL_BIT) )
            break;
        else 
            --whitespace;
    }
    return ++whitespace;
}

BOOL CPaigeEdtView::CleanURLonPosteriorDelete(long selectBegin)
{
    long linkID;
    if (linkID = pgGetSourceID(m_paigeRef, selectBegin))
    {
        pg_hyperlink pgLink;

        pgGetHyperlinkSourceInfo(m_paigeRef, selectBegin, linkID, TRUE, &pgLink);

        if ( (selectBegin > pgLink.applied_range.begin) && 
             (selectBegin < pgLink.applied_range.end) )
        {
            pgLink.applied_range.end = selectBegin;
//                      pgDeleteHyperlinkSource(m_paigeRef, pgLink.applied_range.begin, TRUE);
//                      pgSetHyperlinkSource (m_paigeRef, &pgLink.applied_range, pgLink.URL, NULL,
//                              hlCallback, HYPERLINK_EUDORA_AUTOURL, pgLink.unique_id,0,0,0, best_way);
            pgSetHyperlinkSourceInfo(m_paigeRef, pgLink.applied_range.begin, 
                                     pgLink.unique_id, &pgLink);
            return TRUE;
        }
    }
    return FALSE;
}

/*virtual*/ BOOL CPaigeEdtView::LaunchURL(LPCSTR szURL, LPCSTR szLinkText /* = NULL */)
{
	return ::LaunchURL(szURL, szLinkText);
}

void CPaigeEdtView::OnBlkfmtFixedwidth() 
{
	if (!UsingFullFeatureSet())
		return;

	bool bOn = true;

	if ( pgNumSelections(m_paigeRef) )
		PrepareUndo(undo_format);

	style_info                  info, mask;
	pgInitStyleMask(&info,0);
	pgInitStyleMask(&mask,0);
	long id = pgGetNamedStyleIndex(m_paigeRef,teletype_style);
	//
	// Style is consistent throughout selection
	//
	pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
	if ( (abs(info.style_sheet_id) == id) && ( mask.style_sheet_id != 0 ) )
		bOn = false;
	else
		//
		// Style is indeterminate
		// 
		if ( mask.style_sheet_id == 0 )
			bOn=true;
		else
								//
								// Style is not present
								//
			bOn=true;
	if (bOn)
		pgApplyNamedStyle (m_paigeRef, NULL, teletype_style,  best_way);
	else
	{
		pgApplyNamedStyle (m_paigeRef, NULL, body_style,  best_way);    
	}
}

void CPaigeEdtView::OnUpdateBlkfmtFixedwidth(CCmdUI* pCmdUI) 
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );

        style_info                      info, mask;
        pgInitStyleMask(&info,0);
        pgInitStyleMask(&mask,0);
        long id = pgGetNamedStyleIndex(m_paigeRef,teletype_style);

        //
        // Style is consistent throughout selection
        //
        pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
        if ( (abs(info.style_sheet_id) == id) && ( mask.style_sheet_id != 0 ) )
            pCmdUI->SetCheck(1);
        else
            //
            // Style is indeterminate
            // 
            if ( mask.style_sheet_id == 0 )
            {
                pCmdUI->SetCheck(2);
            }
            else
                                //
                                // Style is not present
                                //
                pCmdUI->SetCheck(0);

    }
}


long CPaigeEdtView::GetAttachmentID(long offset)
{
    long linkID = 0;

    pg_hyperlink pgLink;

    if ( pgGetHyperlinkSourceInfo(m_paigeRef, offset, 0, TRUE, &pgLink) )
    {
        if (pgLink.type & HYPERLINK_EUDORA_ATTACHMENT)
        {
            if ( (offset >= pgLink.applied_range.begin) && (offset <= pgLink.applied_range.end) )
                linkID = pgLink.unique_id;
        }
    }

    return linkID;
}

void CPaigeEdtView::OnUpdateOpenAttachment(CCmdUI* pCmdUI)
{
    BOOL enable = FALSE;

    if (m_fRO)
    {
        select_pair sel;
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

        if ( GetAttachmentID(sel.begin) )
            enable = TRUE;
    }

    pCmdUI->Enable(enable);
}

void CPaigeEdtView::OnOpenAttachment()
{
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    long attachID;

    if (attachID = GetAttachmentID(sel.begin))
    {
        paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

        // simply stash the hyperlink's id for later processing
        PgStuffBucket* pSB = (PgStuffBucket*) pgPtr->user_refcon;
        pSB->hlinkEvent = attachID;
        UnuseMemory(m_paigeRef);
        PgProcessHyperlinks(m_paigeRef);
    }
}
void CPaigeEdtView::OnAddQuote()
{
	if (!m_fRO)
	{
		PaigeExcerptLevelChanger	excerptLevelChanger(true);

		ApplyStyleChange(excerptLevelChanger);
	}
}

void CPaigeEdtView::OnUpdateQuoteCommand(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_fRO);
}

void CPaigeEdtView::OnRemoveQuote()
{
	if (!m_fRO)
	{
		PaigeExcerptLevelChanger	excerptLevelChanger(false);

		ApplyStyleChange(excerptLevelChanger);
	}
}


void
CPaigeEdtView::OnUpdateSearchForSelection(
	CCmdUI *				pCmdUI,
	UINT					nMenuItemWithSelectionFormat,
	UINT					nMenuItemDefaultText)
{
	BOOL		bLoadGood = TRUE;
	CString		szText;
	CString		szMenuItemText;
	
	if ( GetSelectedTextAndTrim(szText, true, true, kMaxCharsInContextSearchSelection, true) )
		szMenuItemText.Format(nMenuItemWithSelectionFormat, szText);
	else
		bLoadGood = szMenuItemText.LoadString(nMenuItemDefaultText);

	if (bLoadGood)
		pCmdUI->SetText(szMenuItemText);
	
	pCmdUI->Enable( !szText.IsEmpty() );
}


void CPaigeEdtView::OnSearchInEudoraForSelection(
	bool								in_bSelectParent,
	bool								in_bSelectAll)
{
	// Get the selected text and trim spaces.
	CString			 szText;

	GetSelectedTextAndTrim(szText, true, true, kMaxCharsInContextSearchSelection);
	
	const char *	szMailboxName = NULL;

	CTocDoc* pTocDoc = GetTocDoc();
	if (pTocDoc)
		szMailboxName = pTocDoc->GetMBFileName();
	
	OpenFindMessagesWindow(szMailboxName, szText, true, in_bSelectParent, in_bSelectAll, true);
}


void CPaigeEdtView::OnUpdateSearchWebForSelection(CCmdUI* pCmdUI)
{
	OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_WEB_FOR_SEL_MENU_FORMAT, IDS_SEARCH_WEB_FOR_SEL_MENU);
}


void CPaigeEdtView::OnSearchWebForSelection()
{
	// Get the selected text and trim spaces.
	CString			 szText;
	
	GetSelectedTextAndTrim(szText, true, true, kMaxCharsInContextSearchSelection);
	LaunchURLWithQuery(NULL, ACTION_SEARCH_WEB, szText, NULL, NULL, NULL, NULL, ACTION_SEARCH_PARAM_LABEL);
}


void CPaigeEdtView::OnUpdateSearchEudoraForSelection(CCmdUI* pCmdUI)
{
	OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_EUDORA_FOR_SEL_MENU_FORMAT, IDS_SEARCH_EUDORA_FOR_SEL_MENU);
}


void CPaigeEdtView::OnSearchEudoraForSelection()
{
	//	Search in Eudora for the selection, selecting all mailboxes
	OnSearchInEudoraForSelection(false, true);
}


void CPaigeEdtView::OnUpdateSearchMailboxForSelection(CCmdUI* pCmdUI)
{
	CTocDoc *		pTocDoc = GetTocDoc();
	
	if (pTocDoc)
	{
		OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_MAILBOX_FOR_SEL_MENU_FORMAT, IDS_SEARCH_MAILBOX_FOR_SEL_MENU);
	}
	else
	{
		CRString		szMenuItemText(IDS_SEARCH_MAILBOX_FOR_SEL_MENU);
	
		pCmdUI->SetText(szMenuItemText);
		pCmdUI->Enable(FALSE);
	}
}


void CPaigeEdtView::OnSearchMailboxForSelection()
{
	//	Search in Eudora for the selection, selecting the current mailbox
	OnSearchInEudoraForSelection(false, false);
}


void CPaigeEdtView::OnUpdateSearchMailfolderForSelection(CCmdUI* pCmdUI)
{
	CTocDoc *		pTocDoc = GetTocDoc();
	
	if (pTocDoc)
	{
		OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_MAILFOLDER_FOR_SEL_MENU_FORMAT, IDS_SEARCH_MAILFOLDER_FOR_SEL_MENU);
	}
	else
	{
		CRString		szMenuItemText(IDS_SEARCH_MAILFOLDER_FOR_SEL_MENU);
	
		pCmdUI->SetText(szMenuItemText);
		pCmdUI->Enable(FALSE);
	}
}


void CPaigeEdtView::OnSearchMailfolderForSelection()
{
	//	Search in Eudora for the selection, selecting the parent folder of the current mailbox
	OnSearchInEudoraForSelection(true, false);
}


int CPaigeEdtView::TabToSpaces()
{
    select_pair         sel, line;
    int                         numSpaces;
    unsigned char       buf[8];

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
        
    short tabstop = GetIniShort(IDS_INI_TAB_STOP);

    if ((sel.begin == 0) || (pgCharByte(m_paigeRef, sel.begin - 1, buf) == 1) )
    {
        if (sel.begin == 0 || buf[0] == PgGlobalsPtr()->line_wrap_char)
        {
            numSpaces = tabstop;
            return numSpaces;
        }
    }

    //If Selection is at end of doc, returns bogus values
    if (pgTextSize(m_paigeRef) == sel.begin)
        pgFindLine(m_paigeRef, sel.begin - 1, &line.begin, &line.end);
    else
        pgFindLine(m_paigeRef, sel.begin, &line.begin, &line.end);

    numSpaces = tabstop - ((sel.begin - line.begin) % tabstop);

    return numSpaces;
}


void CPaigeEdtView::FindEmoticonTriggerLocations(const select_pair & in_selectPair, EmoticonsInTextList & in_emoticonsInText) 
{
	CPaigeStyle		styleEx(m_paigeRef);
	select_pair		styleRange;
	style_walk		sw;
	bool			bSkipStyle = false;
	text_block_ptr	pTextBlock = NULL;
	const char *	pStartText = NULL;
	const char *	pEndText = NULL;
	paige_rec_ptr	pPaige = (paige_rec_ptr) UseMemory( m_paigeRef );

	//	Make sure that we get access to Paige
	ASSERT(pPaige);
	if (!pPaige)
	{
		ASSERT(0);
		return;
	}

	//	I'm not sure why, but we're being passed a selection that is longer than
	//	the total length of text. Check for this case and compensate.
	long			nEndScanRange = (in_selectPair.end < pPaige->t_length) ? in_selectPair.end : pPaige->t_length;

	//	Walk through all the styles in the selection that we've been given
	pgPrepareStyleWalk(pPaige, in_selectPair.begin, &sw, true);
	styleRange.end = in_selectPair.begin;

	while (styleRange.end < nEndScanRange)
	{
		//	Current style begins with the current offset
		styleRange.begin = sw.current_offset;
		
		//	Figure out where the current style ends
		if ( sw.next_style_run && (sw.next_style_run->offset <= nEndScanRange) )
		{
			//	Walk to the next style
			if ( pgWalkStyle(&sw, nEndScanRange - sw.current_offset) )
			{
				//	Current style ends where the next style begins
				styleRange.end = sw.current_offset;
			}
			else
			{
				//	Current style ends where the range ends
				styleRange.end = nEndScanRange;
			}
		}
		else
		{
			//	Current style ends where the range ends
			styleRange.end = nEndScanRange;
		}

		//	We may want to skip certain styles - like URLs. Check to see
		//	if we should scan this style.
		if ( !bSkipStyle && (styleRange.begin != styleRange.end) )
		{
			//	Keep track of where we are within the style range
			int		nNextOffset = styleRange.begin;

			while (nNextOffset < styleRange.end)
			{
				//	Only bother to find the next text block if:
				//	* We don't currently have a text block
				//	* The next offset now falls outside of the text block we last retrieved.
				if ( !pTextBlock || (nNextOffset < pTextBlock->begin) || (nNextOffset >= pTextBlock->end) )
				{
					if (pStartText)
					{
						//	Release previous text
						UnuseMemory(pTextBlock->text);
						
						//	Forget previous text
						pStartText = NULL;
					}
					
					//	Find the text block that contains the current style
					pTextBlock = pgFindTextBlock(pPaige, nNextOffset, NULL, FALSE, TRUE);
					ASSERT(pTextBlock);
					if (pTextBlock)
					{
						//	Get the start and end of the text for the current text block
						pStartText = reinterpret_cast<char *>( UseMemory(pTextBlock->text) );
						pEndText = pStartText + pTextBlock->end - pTextBlock->begin;
					}

					//	Must release t_blocks because pgFindTextBlock called UseMemory with it.
					UnuseMemory(pPaige->t_blocks);
				}
				
				if (pTextBlock && pStartText)
				{
					//	Calculate the start and end of the scan for the current offset
					const char *	pScanText = pStartText + nNextOffset - pTextBlock->begin;
					const char *	pEndScan = pScanText + styleRange.end - nNextOffset;
					long			nEmoticonOffset;
					
					if (pEndScan > pEndText)
					{
						//	The style range crosses a text block, scan the current
						//	text block up until the end of it, and resume scanning
						//	at the next text block.
						pEndScan = pEndText;
						nNextOffset += pEndScan - pScanText;
					}
					else
					{
						//	The style range was completed by this text block.
						nNextOffset = styleRange.end;
					}
					
					//	Scan the text in the current style for emoticons
					while (pScanText < pEndScan)
					{
						int				nIgnoreTextReplaceLength = 0;
						Emoticon *		pEmoticon = g_theEmoticonDirector.GetMatchingEmoticon(pScanText, pStartText, pEndText, nIgnoreTextReplaceLength, false);
						
						if (pEmoticon)
						{
							//	Calculate emoticon location
							nEmoticonOffset = pScanText - pStartText + pTextBlock->begin;
							
							//	Push emoticon location and information onto the front so
							//	that we replace them in reverse order (so as to avoid
							//	invalidating the offsets).
							in_emoticonsInText.push_front( EmoticonInText(nEmoticonOffset, pEmoticon) );
							
							pScanText += pEmoticon->GetTrigger().GetLength();
							
						}
						else
						{
							//	Move on to the next character and check again
							pScanText++;
						}
					}
				}
			}
		}
	}

	//	pStartText is only ever non-NULL when we have previous text that we
	//	haven't released yet.
	if (pStartText)
	{
		//	Release previous text
		UnuseMemory(pTextBlock->text);
	}
	
	//	Done with style walk - free sw memory by calling pgPrepareStyleWalk
	//	again with NULL.
	//	A subtle fact about pgPrepareStyleWalk - if you pass true in for the
	//	last when initializing - you MUST pass in true for the last parameter
	//	when freeing. Otherwise UnuseMemory won't be called for pPaige->par_formats
	//	or pPaige->par_style_run, which will cause later operations to fail or
	//	crash when Paige cannot resize these crucial structures.
	pgPrepareStyleWalk(pPaige, 0, NULL, true);
	
	//	Done with pPaige - release Paige ref
	UnuseMemory(m_paigeRef);
}


void CPaigeEdtView::ReplaceEmoticonTriggersWithImages(EmoticonsInTextList & in_emoticonsInText)
{
	//	pgDelete destroys the selection - remember it so that we can restore it
	select_pair		currentSelection;
	pgGetSelection( m_paigeRef, &currentSelection.begin, &currentSelection.end );

	//	*** Do we need to do something with undo here? ***
	
	//	Note that in_emoticonsInText has been ordered in reverse order, so that we
	//	can just run through the entire list replacing the emoticon triggers with
	//	images without ever having to worry about offsets being messed up.
	long				nEmoticonLocation;
	select_pair			emoticonSel;
	Emoticon *			pEmoticon;
	for ( EmoticonsInTextIterator emoticonInText = in_emoticonsInText.begin();
		  emoticonInText != in_emoticonsInText.end();
		  emoticonInText++ )
	{
		pEmoticon = (*emoticonInText).GetEmoticon();
		nEmoticonLocation = (*emoticonInText).GetEmoticonOffset();

		//	Insert the emoticon image
		if ( PgInsertEmoticonImageFromFile(m_paigeRef, pEmoticon->GetImageFullPath(), pEmoticon->GetTrigger(), nEmoticonLocation) )
		{
			int		nEmoticonTriggerLength = pEmoticon->GetTrigger().GetLength();
			
			//	Emoticon insertion succeeded - delete the trigger text.
			//	It's 2 characters after the emoticon location because it's now
			//	immediately after the embedded image's 2 2 placeholder characters.
			emoticonSel.begin = nEmoticonLocation + 2;
			emoticonSel.end = emoticonSel.begin + nEmoticonTriggerLength;
			pgDelete(m_paigeRef, &emoticonSel, draw_none);

			//	If the emoticon is before the current selection that we're saving,
			//	then adjust the selection to account for the change in the text.
			if (nEmoticonLocation < currentSelection.begin)
			{
				//	Calculate how much to adjust the selection. We deleted the trigger,
				//	but added an embedded image, which uses 2 placeholder characters.
				int		nEmoticonReplacementLengthDif = nEmoticonTriggerLength - 2;
				
				//	Adjust the selection
				currentSelection.begin -= nEmoticonReplacementLengthDif;
				currentSelection.end -= nEmoticonReplacementLengthDif;
			}
		}
	}

	//	Restore the previous selection
	pgSetSelection(m_paigeRef, currentSelection.begin, currentSelection.end, 0, FALSE);

	//	We did all the replacing - clear the emoticons in text list
	in_emoticonsInText.clear();
}



///////////////////////////////////////////////////////////////////////////////
// Spell-checking Dirty List

#define nil NULL

// QueueDirtyPair -- adds a selection pair to the list of text ranges that
// need spell-checked. allocates memory.
void CPaigeEdtView::QueueDirtyPair( long begin, long end )
{
	DirtyPairPtr pRng = DEBUG_NEW_NOTHROW DirtyPair;

    if ( pRng ) {
        pRng->begin = begin;
        pRng->end = end;
        
		pRng->next = nil;

        if ( m_pDirtyPair ) {
            DirtyPairPtr pTrv = m_pDirtyPair;
            for ( ; pTrv->next; pTrv = pTrv->next )
                ;

            pTrv->next = pRng;
        }
        else
            m_pDirtyPair = pRng;
    }
}


// DequeueDirtyPair -- gets the next text range (FIFO) off the queue, and
// frees the memory associated with the DirtyPair.
bool CPaigeEdtView::DequeueDirtyPair( long* begin, long* end)
{
    bool bRet = false;

    if ( m_pDirtyPair ) {
        DirtyPairPtr pRng = m_pDirtyPair;
        m_pDirtyPair = pRng->next;
        *begin = pRng->begin;
        *end = pRng->end;
        delete pRng;
        bRet = true;
    }
    else {
        *begin = 0;
        *end = 0;
    }

    return bRet;
}


// PaigeExcerptLevelChanger - changes excerpt level of the text
PaigeExcerptLevelChanger::PaigeExcerptLevelChanger(bool bOn)
	:	m_bOn(bOn)
{
    pgInitParMask(&m_info[0], 0);
	pgInitParMask(&m_info[1], 0);
}

// ---------------------------------------------------------------------------
//		* PaigeExcerptLevelChanger::GetStyleInfo					[Public]
// ---------------------------------------------------------------------------
//	Fills the internal information at the infoIndex, with the information
//	for the selection.

void PaigeExcerptLevelChanger::GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	pgGetParInfo(pg, pSel, false, &m_info[infoIndex], &m_mask);
}

// ---------------------------------------------------------------------------
//		* PaigeExcerptLevelChanger::PrepareToSetStyleInfo			[Public]
// ---------------------------------------------------------------------------
//	Called immediately before SetStyleInfo is called. Prepares data
//	for SetStyleInfo call, and returns whether or not SetStyleInfo
//	needs to be called.

bool PaigeExcerptLevelChanger::PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	return CPaigeStyle::ApplyExcerpt(&m_info[infoIndex], m_bOn);
}

// ---------------------------------------------------------------------------
//		* PaigeExcerptLevelChanger::SetStyleInfo					[Public]
// ---------------------------------------------------------------------------
//	Sets the style information for the selection to the information at
//	infoIndex. 

void PaigeExcerptLevelChanger::SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	// Init the mask to only set the values that we want to change
	pgInitParMask(&m_mask,0);
	m_mask.user_id = -1;
	m_mask.user_data = -1;
	m_mask.left_extra = -1;

	pgSetParInfo(pg, pSel, &m_info[infoIndex], &m_mask, best_way);
}

// ---------------------------------------------------------------------------
//		* PaigeExcerptLevelChanger::AreStylesIncompatible			[Public]
// ---------------------------------------------------------------------------
//	Returns whether or not the pair of cached style information are
//	compatible - i.e. whether or not the two can be safely combined for the
//	purposes of calling SetStyleInfo. 

bool PaigeExcerptLevelChanger::AreStylesIncompatible()
{
	return ( (m_info[0].user_id != m_info[1].user_id) ||
			 (m_info[0].user_data != m_info[1].user_data) ||
			 (m_info[0].left_extra != m_info[1].left_extra) );
}


// PaigeIndentLevelChanger - changes indent level of the text
PaigeIndentLevelChanger::PaigeIndentLevelChanger(pg_fixed indentDelta)
	:	m_indentDelta(indentDelta)
{
	m_nHTMLBulletLevel[0] = m_nHTMLBulletLevel[1] = 0;
	// Initialize the mask, we only care about the left_indent.	
	m_mask.left_indent = -1;
	m_mask.right_indent = 0;
	m_mask.first_indent = 0;
}

// ---------------------------------------------------------------------------
//		* PaigeIndentLevelChanger::GetStyleInfo						[Public]
// ---------------------------------------------------------------------------
//	Fills the internal information at the infoIndex, with the information
//	for the selection.

void PaigeIndentLevelChanger::GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	pgGetIndents(pg, pSel, &m_indent[infoIndex], &m_mask, NULL, NULL);
	m_nHTMLBulletLevel[infoIndex] = pgStyle->GetBulletLevel(pSel);
}

// ---------------------------------------------------------------------------
//		* PaigeIndentLevelChanger::PrepareToSetStyleInfo			[Public]
// ---------------------------------------------------------------------------
//	Called immediately before SetStyleInfo is called. Prepares data
//	for SetStyleInfo call, and returns whether or not SetStyleInfo
//	needs to be called.

bool PaigeIndentLevelChanger::PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	pg_indents_ptr	indent = &m_indent[infoIndex];
	pg_fixed		baseIndent = 0;
	pg_fixed		previousLeftIndent = indent->left_indent;

	// If it's a negative delta and the selection has a bullet, then we have a
	// different left margin.
	if ( (m_indentDelta < 0) && (m_nHTMLBulletLevel[infoIndex] > 0) )
		baseIndent = DEFLIST_INDENT_VALUE;

	if ( (indent->left_indent > baseIndent) || (m_indentDelta > 0) )
	{
		indent->left_indent += m_indentDelta;

		if (indent->left_indent < 0)
			indent->left_indent = 0;
	}

	if (m_nHTMLBulletLevel[infoIndex] > 0)
	{
 		//	Change the bullet level
		if (m_indentDelta > 0)
			m_nHTMLBulletLevel[infoIndex]++;
		else
			m_nHTMLBulletLevel[infoIndex]--;

		//	Changing indent level shouldn't remove bullet
		if (m_nHTMLBulletLevel[infoIndex] == 0)
			m_nHTMLBulletLevel[infoIndex] = 1;
	}

	return (indent->left_indent != previousLeftIndent);
}

// ---------------------------------------------------------------------------
//		* PaigeIndentLevelChanger::SetStyleInfo						[Public]
// ---------------------------------------------------------------------------
//	Sets the style information for the selection to the information at
//	infoIndex. 

void PaigeIndentLevelChanger::SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	pgSetIndents(pg, pSel, &m_indent[infoIndex], &m_mask, best_way);

	par_info	info, mask;
	pgInitParMask(&info,0);
	pgInitParMask(&mask,0);

	//	Set the html_bullet level and html_style information
	mask.html_bullet = -1;
	mask.html_style = -1;
	info.html_bullet = m_nHTMLBulletLevel[infoIndex];

	if (m_nHTMLBulletLevel[infoIndex] > 0)
		info.html_style = html_unordered_list;
	else if (m_indent[infoIndex].left_indent >= DEFLIST_INDENT_VALUE)
		info.html_style = html_definition_list;
	else
		info.html_style = 0;
	
	pgSetParInfo(pg, pSel, &info, &mask, best_way);
}

// ---------------------------------------------------------------------------
//		* PaigeIndentLevelChanger::AreStylesIncompatible			[Public]
// ---------------------------------------------------------------------------
//	Returns whether or not the pair of cached style information are
//	compatible - i.e. whether or not the two can be safely combined for the
//	purposes of calling SetStyleInfo. 

bool PaigeIndentLevelChanger::AreStylesIncompatible()
{
	return (m_indent[0].left_indent != m_indent[1].left_indent) ||
		   (m_nHTMLBulletLevel[0] != m_nHTMLBulletLevel[1]);
}


// PaigeMarginsNormalChanger - reverts indent level of text to none
PaigeMarginsNormalChanger::PaigeMarginsNormalChanger()
	:	m_numChangesRequired(0), m_bLastGetStyleInfoChangeRequired(false)
{
	m_bIsBullet[0] = m_bIsBullet[1] = false;
	// Initialize the mask, we only care about the left_indent.	
	m_mask.left_indent = -1;
	m_mask.right_indent = 0;
	m_mask.first_indent = 0;
}

// ---------------------------------------------------------------------------
//		* PaigeMarginsNormalChanger::GetStyleInfo					[Public]
// ---------------------------------------------------------------------------
//	Fills the internal information at the infoIndex, with the information
//	for the selection.

void PaigeMarginsNormalChanger::GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	pgGetIndents(pg, pSel, &m_indent, &m_mask, NULL, NULL);
	m_bIsBullet[infoIndex] = pgStyle->IsBullet(pSel);

	pg_fixed	newLeftIndent = 0;
	if ( m_bIsBullet[infoIndex] )
		newLeftIndent = DEFLIST_INDENT_VALUE;

	if (m_bLastGetStyleInfoChangeRequired)
	{
		//	Accumulate change count from last time we were called. Set
		//	m_bLastGetStyleInfoChangeRequired to false so that we don't
		//	double count in the same code in AreStylesIncompatible.
		//	(GetStyleInfo handles the beginning point. PrepareToSetStyleInfo
		//	handles all other cases including the end point).
		m_numChangesRequired++;
		m_bLastGetStyleInfoChangeRequired = false;
	}

	m_bLastGetStyleInfoChangeRequired = (m_indent.left_indent != newLeftIndent);
}

// ---------------------------------------------------------------------------
//		* PaigeMarginsNormalChanger::PrepareToSetStyleInfo			[Public]
// ---------------------------------------------------------------------------
//	Called immediately before SetStyleInfo is called.
//	Returns whether or not SetStyleInfo needs to be called. To do so
//	it uses the information that was previously calculated in GetStyleInfo
//	and AreStylesIncompatible (because calculating it here would require
//	redundant style scanning due to the fact that PaigeMarginsNormalChanger's
//	AreStylesIncompatible is looser/more lenient than any other
//	implementations).

bool PaigeMarginsNormalChanger::PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	bool	changeRequired = (m_numChangesRequired > 0);

	m_numChangesRequired = 0;

	return changeRequired;
}

// ---------------------------------------------------------------------------
//		* PaigeMarginsNormalChanger::SetStyleInfo					[Public]
// ---------------------------------------------------------------------------
//	Sets the style information for the selection to the information at
//	infoIndex. 

void PaigeMarginsNormalChanger::SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	// If the selection has a bullet, then we have a different left margin.
	if ( m_bIsBullet[infoIndex] )
		m_indent.left_indent = DEFLIST_INDENT_VALUE;
	else
		m_indent.left_indent = 0;

	pgSetIndents(pg, pSel, &m_indent, &m_mask, best_way);
}

// ---------------------------------------------------------------------------
//		* PaigeMarginsNormalChanger::AreStylesIncompatible			[Public]
// ---------------------------------------------------------------------------
//	Returns whether or not the pair of cached style information are
//	compatible - i.e. whether or not the two can be safely combined for the
//	purposes of calling SetStyleInfo. 

bool PaigeMarginsNormalChanger::AreStylesIncompatible()
{
	bool	areStylesIncompatible = (m_bIsBullet[0] != m_bIsBullet[1]);
	
	if (!areStylesIncompatible && m_bLastGetStyleInfoChangeRequired)
	{
		//	Accumulate change count from last time GetStyleInfo was called. Set
		//	m_bLastGetStyleInfoChangeRequired to false so that we don't
		//	double count in the same code in AreStylesIncompatible.
		//	(GetStyleInfo handles the beginning point. PrepareToSetStyleInfo
		//	handles all other cases including the end point).
		m_numChangesRequired++;
		m_bLastGetStyleInfoChangeRequired = false;
	}

	return (m_bIsBullet[0] != m_bIsBullet[1]);
}


// PaigeJustificationChanger - sets justification of text to in_nNewJustification
PaigeJustificationChanger::PaigeJustificationChanger(short in_nNewJustification)
	:	m_nNewJustification(in_nNewJustification)
{
    pgInitParMask(&m_info[0], 0);
	pgInitParMask(&m_info[1], 0);
}

// ---------------------------------------------------------------------------
//		* PaigeJustificationChanger::GetStyleInfo					[Public]
// ---------------------------------------------------------------------------
//	Fills the internal information at the infoIndex, with the information
//	for the selection.

void PaigeJustificationChanger::GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	pgGetParInfo(pg, pSel, false, &m_info[infoIndex], &m_mask);
}

// ---------------------------------------------------------------------------
//		* PaigeJustificationChanger::PrepareToSetStyleInfo			[Public]
// ---------------------------------------------------------------------------
//	Called immediately before SetStyleInfo is called. Prepares data
//	for SetStyleInfo call, and returns whether or not SetStyleInfo
//	needs to be called.

bool PaigeJustificationChanger::PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	bool	bChangeJustification = (m_info[infoIndex].justification != m_nNewJustification);

	if (bChangeJustification)
		m_info[infoIndex].justification = m_nNewJustification;

	return bChangeJustification;
}

// ---------------------------------------------------------------------------
//		* PaigeJustificationChanger::SetStyleInfo					[Public]
// ---------------------------------------------------------------------------
//	Sets the style information for the selection to the information at
//	infoIndex. 

void PaigeJustificationChanger::SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex)
{
	// Init the mask to only set the values that we want to change
	pgInitParMask(&m_mask,0);
	m_mask.justification = -1;

	pgSetParInfo(pg, pSel, &m_info[infoIndex], &m_mask, best_way);
}

// ---------------------------------------------------------------------------
//		* PaigeJustificationChanger::AreStylesIncompatible			[Public]
// ---------------------------------------------------------------------------
//	Returns whether or not the pair of cached style information are
//	compatible - i.e. whether or not the two can be safely combined for the
//	purposes of calling SetStyleInfo. 

bool PaigeJustificationChanger::AreStylesIncompatible()
{
	return (m_info[0].justification != m_info[1].justification);
}


/*06-08-2000------------------------------------------------------------------------------
//DoMoodMailCheck 
//Description : This function connects to the Text Analysis Engine(TAE) and gives data in the 
		mail to the Engine Block by Block. Then it asks for the score of mood mail and disconnects.
Input: Session state
Output : Score of Mood Mail
-------------------------------------------------------------------------------------*/

int CPaigeEdtView::DoMoodMailCheck(CMoodWatch *pmoodwatch)
{
	bool found = false;
	int retVal = -1;
	memory_ref refText;
	style_walk sw;
	select_pair MoodRng;
	char* pText;
	MoodRng.begin = MoodRng.end = 0;
	paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
	pgPrepareStyleWalk( prp, MoodRng.begin, &sw, true );
    CPaigeStyle styleEx(m_paigeRef);
//	AfxMessageBox("You are in DoMoodMailCheck func");
	CMoodWatch moodwatch_Local;
	const BOOL bCheckOriginalMood = GetIniShort(IDS_INI_MOOD_MAIL_CHECK_ORIGINAL);
	TAEAllMatches sTaeAllMatches;	
	bool bDrawBadWords = false;

	if(GetIniShort(IDS_INI_MOOD_MAIL_CHECK)
		&& GetIniShort(IDS_INI_MOOD_SHOW_COMP_BADWORDS))
		bDrawBadWords = true;
	if(pmoodwatch==NULL)
	{
		if (!moodwatch_Local.Init())
		{
			UnuseMemory( m_paigeRef );
			return -1;
		}
	}	
	/*if (m_bMoodMailDirty)
		ClearBadWords();*/
	for ( ;; ) 
	{
		if ( bCheckOriginalMood || !styleEx.IsExcerpt( sw.cur_par_style ) ) 
		{
			if (found != true)
			{
				MoodRng.begin = sw.current_offset;
				found = true;
				//break;
			}
		}
		else if (found == true)
		{
			MoodRng.end = sw.current_offset;
			refText = pgCopyText(m_paigeRef, &MoodRng, all_data);
			if(refText)
			{
				AppendMemory(refText,sizeof(pg_byte),TRUE);
				pText=(char*) UseMemory(refText);
				//AfxMessageBox(pText);
				if (pmoodwatch)
				{
					pmoodwatch->AddText(pText,strlen(pText),&sTaeAllMatches);
				}
				else 
				{
					moodwatch_Local.AddText(pText,strlen(pText),&sTaeAllMatches);
				
				}
				if(bDrawBadWords)
					QueueBadWords(&sTaeAllMatches,&MoodRng);	
        		UnuseAndDispose(refText);
			}
			found = false;
		}

		if ( sw.next_par_run->offset < sw.t_length )
			pgWalkStyle( &sw, sw.next_par_run->offset - sw.current_offset );
		else
		{
			if ( found )
			{
				MoodRng.end = pgTextSize( m_paigeRef );
				refText = pgCopyText(m_paigeRef, &MoodRng, all_data);
				if(refText)
				{
					AppendMemory(refText,sizeof(pg_byte),TRUE);
					pText=(char*) UseMemory(refText);
					//AfxMessageBox(pText);
					if (pmoodwatch)
					{
						pmoodwatch->AddText(pText,strlen(pText),&sTaeAllMatches);
					}
					else 
					{
						moodwatch_Local.AddText(pText,strlen(pText),&sTaeAllMatches);
					}
					if(bDrawBadWords)
						QueueBadWords(&sTaeAllMatches,&MoodRng);	
					UnuseAndDispose(refText);
				}
				found = false;
			}
			break;
		}
	}
	pgPrepareStyleWalk( prp, 0, NULL, true );
	UnuseMemory( m_paigeRef );
//	if(bDrawBadWords & m_bMoodMailDirty)
//		DrawBadWords();
	if (pmoodwatch)
	{
		return -1;
	} 
	else
	{
		retVal = moodwatch_Local.GetScore()+1;
	}
	return retVal;
}

int CPaigeEdtView::QueueBadWords(TAEAllMatches *pTaeAllMatches,select_pair *pMoodTextRng)
{
	if(pTaeAllMatches)
	{
		int i;
		int nWordType;
		TAEMatch *ptaematch ;
		select_pair pSel;
		for (i = 0; i < pTaeAllMatches->iNumMatches; ++i)
		{
			ptaematch = (struct TAEMatch *)(pTaeAllMatches->ptaematches + i);
			if (ptaematch)
			{
				if (ptaematch->nCollection % 2 == 0)
					nWordType = MOOD_H_WORD;
				else
					nWordType = MOOD_NON_H_WORD;
				pSel.begin = pMoodTextRng->begin + (ptaematch->lStart);
				pSel.end = pMoodTextRng->begin + (ptaematch->lStart)+(ptaematch->lLength);
				/*QueueDirtyPair( pSel.begin, p );*/
 				/*if (m_styleEx)
					m_styleEx->ApplyBadMoodWord(true, &pSel,nWordType);*/
				QueueMoodDirtyPair( pSel.begin, pSel.end, nWordType);
            }
		}
		TAEFreeAllMatches(pTaeAllMatches);
		return TRUE;
	}
	return FALSE;
}

int CPaigeEdtView::DrawBadWords()
{
	/*if(!GetIniShort(IDS_INI_MOOD_MAIL_CHECK)
		|| !GetIniShort(IDS_INI_MOOD_SHOW_COMP_BADWORDS) )
		return 0;
	select_pair textRange;
	int nWordType;
	if (m_styleEx)
	{
		while ( DequeueMoodDirtyPair( &textRange.begin, &textRange.end,&nWordType ) )
		{
			m_styleEx->ApplyMisspelled(false,&textRange);
			if (!m_styleEx->IsWithinURLRange(&textRange))
				m_styleEx->ApplyBadMoodWord(true, &textRange,nWordType);

		}
	}*/
	if(!GetIniShort(IDS_INI_MOOD_MAIL_CHECK)
		|| !GetIniShort(IDS_INI_MOOD_SHOW_COMP_BADWORDS) )
		return 0;
		
	select_pair text_range,badWordRange;
	if (m_pMoodDirtyPair)
	{
		DirtyPairPtr pDirtyPair = m_pMoodDirtyPair;
		if(pDirtyPair)
		{
			text_range.begin = 0;
			text_range.end = pDirtyPair->begin;
			m_styleEx->ApplyBadMoodWord(false, &text_range,0);
		}
		while(pDirtyPair)
		{
			badWordRange.begin = pDirtyPair->begin;
			badWordRange.end = pDirtyPair->end;
			if (!m_styleEx->IsWithinURLRange(&badWordRange) && !m_styleEx->IsBadMoodWord(&badWordRange))
			{
				if(m_styleEx->IsMisspelled(&badWordRange))
					m_styleEx->ApplyMisspelled(false,&badWordRange);
				m_styleEx->ApplyBadMoodWord(true, &badWordRange,pDirtyPair->nMoodWordType);
			}
			text_range.begin = badWordRange.end;
			pDirtyPair= pDirtyPair->next;
			if(pDirtyPair)
				text_range.end = pDirtyPair->begin;
			else
				text_range.end = pgTextSize(m_paigeRef);
			//m_styleEx->ApplyBadMoodWord(false, &text_range,0);
			ClearBadWords(&text_range);
		}
		select_pair textRange;
		int nWordType;
		//Delete all the bad moodword pairs
		while ( DequeueMoodDirtyPair( &textRange.begin, &textRange.end,&nWordType ) )
		{}
	}
	else
	{
		text_range.begin = 0;
		text_range.end = pgTextSize(m_paigeRef);
		//m_styleEx->ApplyBadMoodWord(false, &text_range,0);
		ClearBadWords(&text_range);
	}
	//Invalidate();
	return TRUE;
}

int CPaigeEdtView::ClearBadWords(select_pair_ptr pSel)
{

	// prepare to walk styles
    style_walk sw;
    paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
    pgPrepareStyleWalk( prp, pSel->begin, &sw, false );
    UnuseMemory( m_paigeRef );
	CObList DirtyList;
	select_pair textRange;
	for (;;) {
        style_info_ptr psi = sw.cur_style;
        if ( m_styleEx->IsBadMoodWord( psi ) ) 
		{
            textRange.begin = textRange.end = sw.current_offset;
            textRange.end += sw.next_style_run->offset - textRange.begin;
			DirtyList.AddTail(DEBUG_NEW CObDirtyPair(textRange.begin, textRange.end));
        }
		if (!pgWalkNextStyle(&sw)||sw.next_style_run->offset > pSel->end) break;
    }
     pgPrepareStyleWalk( prp, 0, NULL, false );    // tells paige we're done
   POSITION pos = DirtyList.GetHeadPosition();
   for( pos = DirtyList.GetHeadPosition(); pos != NULL; )
   {
	   CObDirtyPair *pPair = (CObDirtyPair *)DirtyList.GetNext(pos);
	   textRange.begin = pPair->m_nBegin;
	   textRange.end = pPair->m_nEnd;
	   m_styleEx->ApplyBadMoodWord(false, &textRange,0);
	   delete pPair;
   }
   DirtyList.RemoveAll();
   return TRUE;
}

int CPaigeEdtView::ClearBadWords()
{

	// prepare to walk styles
    style_walk sw;
    paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
    pgPrepareStyleWalk( prp, 0, &sw, false );
    UnuseMemory( m_paigeRef );
	
	select_pair textRange;
	for (;;) {
        style_info_ptr psi = sw.cur_style;
		
		//SDSpellTRACE("Walking: %d.%d %d\n",sw.current_offset,sw.next_style_run->offset-sw.current_offset,CPaigeStyle::NeedsScanned( psi ));

        if ( m_styleEx->IsBadMoodWord( psi ) ) 
		{
            textRange.begin = textRange.end = sw.current_offset;
            textRange.end += sw.next_style_run->offset - textRange.begin;
			QueueMoodDirtyPair( textRange.begin, textRange.end );
        }
		if (!pgWalkNextStyle(&sw)) break;
    }
     pgPrepareStyleWalk( prp, 0, NULL, false );    // tells paige we're done
 	if (m_styleEx)
	{
		while ( DequeueMoodDirtyPair( &textRange.begin, &textRange.end ) )
		{
			m_styleEx->ApplyBadMoodWord(false, &textRange,0);
		}
	}

	return TRUE;
}

void CPaigeEdtView::QueueMoodDirtyPair( long begin, long end, int nWordType /*=0*/  )
{
	DirtyPairPtr pRng = DEBUG_NEW_NOTHROW DirtyPair;

    if ( pRng ) {
        pRng->begin = begin;
        pRng->end = end;
		pRng->nMoodWordType = nWordType;
        
		pRng->next = nil;

        if ( m_pMoodDirtyPair ) {
            DirtyPairPtr pTrv = m_pMoodDirtyPair;
            for ( ; pTrv->next; pTrv = pTrv->next )
                ;

            pTrv->next = pRng;
        }
        else
            m_pMoodDirtyPair = pRng;
    }
}


bool CPaigeEdtView::DequeueMoodDirtyPair( long* begin, long* end, int* nWordType )
{
    bool bRet = false;

    if ( m_pMoodDirtyPair ) {
        DirtyPairPtr pRng = m_pMoodDirtyPair;
        m_pMoodDirtyPair = pRng->next;
        *begin = pRng->begin;
        *end = pRng->end;
		if (nWordType)
			*nWordType = pRng->nMoodWordType; 
        delete pRng;
        bRet = true;
    }
    else {
        *begin = 0;
        *end = 0;
		if (nWordType)
			*nWordType=0;
    }	
    return bRet;
}
