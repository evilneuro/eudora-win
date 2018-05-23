// PgMsgView.cpp : implementation file
//

#include "stdafx.h"
#include "shlobj.h"

#include "resource.h"
#include "rs.h"
#include "ReadMessageDoc.h"
#include "msgutils.h"
#include "utils.h"
#include "guiutils.h"

#include "PgMsgView.h"
#include "PgStyleUtils.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "pghtext.h"

#include "summary.h"
#include "TocFrame.h"
#include "etf2html.h"
#include "helpcntx.h"				// for CContextMenu helper

#include "PgStuffBucket.h"
// #include "PgEmbeddedImage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern CString EudoraDir;

/////////////////////////////////////////////////////////////////////////////
// PgMsgView

IMPLEMENT_DYNCREATE(PgMsgView, CPaigeEdtView)

PgMsgView::PgMsgView()
{
    fDiscardMessage = false;
    m_bEnableCopyAttachment = false;
}

PgMsgView::~PgMsgView()
{
}


/////////////////////////////////////////////////////////////////////////////
// PgMsgView diagnostics

#ifdef _DEBUG
void PgMsgView::AssertValid() const
{
	CPaigeEdtView::AssertValid();
}

void PgMsgView::Dump(CDumpContext& dc) const
{
	CPaigeEdtView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// PgMsgView methods

// OnInitialUpdate:
// This is the first thing called *after* the view has been attached to
// the document.

void PgMsgView::OnInitialUpdate() 
{
	CPaigeEdtView::OnInitialUpdate();

	// we cache a pointer to the Doc's QCMessage object in the Paige record.
	// this allows the various Paige callbacks to know what message they are
	// actually dealing with.

	CMessageDoc* pDoc = (CMessageDoc*) GetDocument();
	VERIFY( pDoc );

	if ( pDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)) )
		SetMessageStuff( pDoc );
}


// SetMessageStuff:
// Sets up the stuff bucket with some QCMessage related stuff.

void PgMsgView::SetMessageStuff( CMessageDoc* pMsgDoc )
{
	VERIFY( pMsgDoc );
	ASSERT( pMsgDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)) );

	paige_rec_ptr pPg = (paige_rec_ptr) UseMemory( m_paigeRef );
	PgStuffBucket* pSB = (PgStuffBucket*) pPg->user_refcon;
	pSB->kind = PgStuffBucket::kMessage;
	pSB->pMessage = &pMsgDoc->m_QCMessage;
	UnuseMemory( m_paigeRef );
}


// ImportMessage:
// Standard method of reading in a mail message
//
// In the past, there were differences in the way that Read and Comp messages
// were handled, and indeed the doc classes are different. This routine
// provides a generlized way of reading from both doc types, and helps to hide
// the fact that the docs are different, which I (bo) view as an unnecassary
// complication.
//
// However, we choose not to consider "headers" to be part of the message,
// and leave it to derived views to handle this detail as necessary.
// PgReadMsgView, for instance, imports the message headers into the Paige
// instance before calling the base implimentation.

void PgMsgView::ImportMessage( CMessageDoc* pMsgDoc /* = NULL */ )
{
    CMessageDoc* pDoc = NULL;

    if ( pMsgDoc )
	pDoc = pMsgDoc;
    else
	pDoc = (CMessageDoc*) GetDocument();

    ASSERT_KINDOF(CMessageDoc, pDoc);
	
    //
    // Make sure the QCMessage stuff embedded inside the CMessageDoc
    // loads the latest stuff into memory so that the GetBodyAsHTML() 
    // thing below has good stuff to party on.
    //
    {
	char* pszDummy = pDoc->GetFullMessage();
	if (pszDummy)
	    delete [] pszDummy;
    }

    paige_rec_ptr pPg = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pPg->user_refcon;
    QCMessage* pMess = pSB->pMessage;
    UnuseMemory( m_paigeRef );

    CString htmlBody;
    pMess->GetBodyAsHTML( htmlBody );

    // BOG: doing a paige import from memory seems to be quite slow on win95,
    // so we're gonna write this out to a file, then have paige do the import
    // from file instead. makes me wonder if we should have just written
    // GetBodyAsHTML to return a CFile object instead of a CString. for totally
    // sick robustness, we'll keep the old code just in case the new fails.

    PgDataTranslator theTxr( m_paigeRef );
    char* tmpName = _tempnam( EudoraDir, "bo" );

    try
    {
	CFile tmpFile( tmpName,
		       CFile::modeReadWrite |
		       CFile::modeCreate );

	tmpFile.Write( htmlBody, htmlBody.GetLength() );
	tmpFile.SeekToBegin();
	theTxr.ImportData( &tmpFile, PgDataTranslator::kFmtHtml );

	try
	{
	    tmpFile.Close();
	    CFile::Remove( tmpName );
	}
	catch ( CFileException* e )
	{
	    // we need to catch file operations performed after the import
	    // separately, otherwise we will do a double import!
	    e->Delete();
	    assert(0);
	}
    }
    catch ( CFileException* e )
    {
	e->Delete();

	try
	{
	    // note that this is likely to fail, since the file may not exist,
	    // or may not be closed. whatever. it's better than nothing.
	    CFile::Remove( tmpName );
	}
	catch ( CFileException* e )
	{
	    e->Delete();
	}

	const char* pMessageBody =
	    htmlBody.GetBuffer( htmlBody.GetLength() + 1 );

	// convert our c-string to a Paige Memory Manager reference
	memory_ref impRef = CString2Memory( PgMemGlobalsPtr(), pMessageBody );
	htmlBody.ReleaseBuffer();

	// call the appropriate Paige-based import method. note that at this
	// point all msf_xrich can be treated as html.
	theTxr.ImportData( &impRef, PgDataTranslator::kFmtHtml );

	// CString2Memory allocates memory (its got a sneaky name!)
	DisposeMemory( impRef );
	assert(0);
    }

    free( tmpName );
}


// ExportMessage:
// Standard, generalized way of saving to a "message doc"

void PgMsgView::ExportMessage( CMessageDoc* pMsgDoc /* = NULL */ )
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
	
	select_pair sel;
	sel.begin = 0;
	sel.end = pgTextSize(m_paigeRef);

	// figger out whatsup wi' dat data format!
	PgDataTranslator::FormatType txrType;

	bool bExcerptCausedStyle = false;
	if (pSum->IsComp())
	{
		pSum->UnsetFlagEx(MSFEX_SEND_PLAIN);
		pSum->UnsetFlagEx(MSFEX_SEND_STYLED);
	}
	if ((PgHasStyledText(m_paigeRef, PgGlobalsPtr(), NULL, &bExcerptCausedStyle)) || (SelectionHasHtmlSnippet(sel)))
	{
		pSum->SetFlag(MSF_XRICH);
		pSum->SetFlagEx(MSFEX_HTML);
		txrType = PgDataTranslator::kFmtHtml;

		// If the only reason this message was saved with styles was because it had an excerpt,
		// then set the flags so it only sends a plain text version.
		if (pSum->IsComp() && bExcerptCausedStyle)
		{
			pSum->SetFlagEx(MSFEX_SEND_PLAIN);
			pSum->UnsetFlagEx(MSFEX_SEND_STYLED);
		}
	}
	else
	{
		pSum->UnsetFlag(MSF_XRICH);
		pSum->UnsetFlagEx(MSFEX_HTML);
		txrType = PgDataTranslator::kFmtText;
	}
	

	MakeAutoURLSpaghetti(0, pgTextSize(m_paigeRef));

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

BOOL PgMsgView::SelectionHasHtmlSnippet(select_pair &sel)
{
	if (pgTextSize(m_paigeRef) == 0)
		return FALSE;

	BOOL hasSnippet = FALSE;

	PgDataTranslator::FormatType txrType = PgDataTranslator::kFmtText;

	pg_ref tempRef = pgCopy(m_paigeRef, &sel);
	memory_ref pgmRef;
	PgDataTranslator theTxr( tempRef );
	theTxr.ExportData( &pgmRef, txrType );

	// convert the PGMM reference to a c-string, and jam it in the doc. note
	// that anything in the paige instance (headers, whatever) is gonna
	// get saved as part of the message.
	char* pMem = (char*) UseMemoryToCStr( pgmRef );

	if (strstr(pMem, "<html>") || strstr(pMem, "</html>") )
		hasSnippet = TRUE;

	UnuseAndDispose(pgmRef);

	return hasSnippet;
}


// SaveInfo:
// QCProtocol method for doing a write operation

void PgMsgView::SaveInfo()
{
	// BOG: This gets called blindly in a place or two, and that is causing
	// some problems. It does not appear that anyone expects this routine to
	// do a "forced" save, so I'm changing it to only save when "dirty".

	//Added a check to see if the dirty flag was set due to Hyperlinks
	//This is because we don't support undo for hyperlink editing[as yet]
//	if ( HasChanged() || GetDocument()->IsModified() )

    if ( fDiscardMessage == false ) {
	ExportMessage();
    }
}

BEGIN_MESSAGE_MAP(PgMsgView, CPaigeEdtView)
    //{{AFX_MSG_MAP(PgMsgView)
    ON_WM_LBUTTONDBLCLK()
    ON_WM_CHAR()
    ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCE, OnUpdateCmdViewSource)
    ON_COMMAND(ID_INVISIBLES, OnInvisibles)
    ON_COMMAND( ID_EDIT_COPY_ATTACHMENT, OnCopyAttachment )
    ON_UPDATE_COMMAND_UI( ID_EDIT_COPY_ATTACHMENT, OnUpdateCopyAttachment )
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PgMsgView message handlers

void PgMsgView::OpenFileAtOffset(long offset)
{
	long first, last;
	pgFindLine(m_paigeRef, offset, &first, &last);

	text_block block;
	pgGetTextblock(m_paigeRef, offset, &block, TRUE);

	char* text = (char*)UseMemory(block.text);
	char* line = text + (first - block.begin);
	CRString AttachConvert(IDS_ATTACH_CONVERTED);
	int ACLen = AttachConvert.GetLength();
	if (strncmp(AttachConvert, line, ACLen) == 0)
	{
		const char* start = line + ACLen;
		const char* end = strchr(start, '\r');
		if (end)
		{
			end--;
			if (*start == '"')
				start++;
			if (*end == '"')
				end--;
			CString Filename(start, end - start + 1);
			OpenFile(Filename);
		}
	}

	UnuseMemory(block.text);
}

void PgMsgView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CPaigeEdtView::OnLButtonDblClk(nFlags, point);

	co_ordinate pgMousePt;
	CPointToPgPoint(&point, &pgMousePt);
	long offset = pgPtToChar(m_paigeRef, &pgMousePt, NULL);
	OpenFileAtOffset(offset);
}

void PgMsgView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags);

	if (nChar == VK_RETURN)
	{
		rectangle rect;
		pgCaretPosition(m_paigeRef, CURRENT_POSITION, &rect);
		long offset = pgPtToChar(m_paigeRef, &rect.top_left, NULL);
		OpenFileAtOffset(offset);
	}
}


////////////////////////////////////////////////////////////////////////
// OnUpdateCmdViewSource [protected]
//
// Command UI handler for ID_VIEW_SOURCE command.  Only enable the View
// Source command if the message contains styled text.
////////////////////////////////////////////////////////////////////////
void PgMsgView::OnUpdateCmdViewSource(CCmdUI* pCmdUI)
{
	CTocFrame* pTocFrame = DYNAMIC_DOWNCAST(CTocFrame, GetParentFrame());
	if (pTocFrame && (NULL == pTocFrame->GetPreviewSummary()))
	{
		//
		// Happens in weird case of an empty mailbox and the focus
		// is set to the (empty) preview pane.
		//
		pCmdUI->Enable(FALSE);
		return;
	}

	if (PgHasStyledText(m_paigeRef, PgGlobalsPtr()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


///////////////////////////////////////////////////////////////////////////////
// Big Hack!!

// FlagAsDiscardable - this is a mondo hack we use to work around the fact
// that MIME Part handling requires us to do exports even when a document has
// not been modified (reply, forword, etc...). By calling this routine, the
// view will not be exported when SaveInfo is called.

void PgMsgView::FlagAsDiscardable( PgMsgView* pMsgView,
				   bool bDiscard /* = true */ )
{
    VERIFY( pMsgView );
    ASSERT( pMsgView->IsKindOf(RUNTIME_CLASS(PgMsgView)) );

    // BOG: commented out the IsKindOf test on the grounds that the only code
    // that calls this performs the same test before calling. if this gets used
    // elsewhere it should be put back (shouldn't be used elsewhere however).
//    if ( pMsgView->IsKindOf(RUNTIME_CLASS(PgMsgView)) ) {
	pMsgView->fDiscardMessage = bDiscard;
//    }
}

void PgMsgView::OnInvisibles()
{
	long attribs = pgGetAttributes(m_paigeRef);	
	attribs ^= SHOW_INVIS_CHAR_BIT;

	pgSetAttributes(m_paigeRef, attribs);
	Invalidate();
}


void PgMsgView::OnCopyAttachment() 
{
    //
    // A Little HDROP Documentation
    //
    // An HDROP is a global handle to a DROPFILES structure with a double-null terminated
    // list of strings appended at the end. "pFiles" in DROPFILES is simply set to
    // sizeof(DROPFILES), and then whack your filepath(s) on at the end of the structure.
    // All other fields of DROPFILES can be set to zero. Now put that thing on
    // the Clipboard!
    //
    // Note:
    //   * If doing drag/drop "pt" is set to the drop point (mouse ptr coords).
    //   * If paths are wide chars "fWide" is set to TRUE.
    //   * On NT, HDROPS from the system have wide char paths. You don't *have* to do this
    //     unless you need/want to do unicode. On the recieving end (drop/paste)
    //     DragQueryFile does the W2A translation for you.
    //
    // List of strings example:
    //   This is string one\0Here's string two\0String three\0\0  <--- double null.
    //

    char url[MAX_PATH + 7] = "";   // "7" for the "file://" on the url
    pgGetSourceURL( m_paigeRef, m_lAttachmentOffset, (pg_char_ptr)url, sizeof(url) );
    CString filePath = UnEscapeURL( url );

    // get rid of the "file://" thingie
    char* pcstr = filePath.GetBuffer( filePath.GetLength() );
    strcpy( pcstr, &pcstr[7] );
    filePath.ReleaseBuffer();

    unsigned pathLen = filePath.GetLength();
    DWORD dwBytes = (sizeof(DROPFILES) + pathLen + 2 /* double null terminated */);
    HGLOBAL hglb = GlobalAlloc( GMEM_SHARE | GMEM_ZEROINIT, dwBytes );
    BYTE* pBytes = (BYTE*) GlobalLock( hglb );
    ((DROPFILES*)pBytes)->pFiles = sizeof(DROPFILES);
    memcpy( pBytes + sizeof(DROPFILES), filePath, pathLen );
    GlobalUnlock( hglb );

    COleDataSource* theData = new COleDataSource;;
    theData->CacheGlobalData( CF_HDROP, hglb );
    theData->SetClipboard();
}

void PgMsgView::OnUpdateCopyAttachment(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( m_bEnableCopyAttachment );
}


void PgMsgView::OnContextMenu( CWnd* pWnd, CPoint ptScreen )
{
// Hit-test the mouse pt and see if we're in a hyperlink. If the hyperlink
// is an attachment we save the offset and set m_bEnableCopyAttachment, then
// forward this message to the parent MessageFrame.

    pg_hyperlink hlink;
    co_ordinate ptPgClient;
    CPoint ptClient = ptScreen;
    ScreenToClient( &ptClient );
    CPointToPgPoint( &ptClient, &ptPgClient );
    long offset = pgPtInHyperlinkSource( m_paigeRef, &ptPgClient );

    if ( offset != -1 ) {
        pgGetHyperlinkSourceInfo( m_paigeRef, offset, 0, false, &hlink );

        if ( hlink.type & HYPERLINK_EUDORA_ATTACHMENT ) {
            m_bEnableCopyAttachment = true;
            m_lAttachmentOffset = offset;
        }
    }

    WPARAM wParam = (WPARAM) pWnd->GetSafeHwnd();
    LPARAM lParam = MAKELPARAM(ptScreen.x, ptScreen.y);
    GetParentFrame()->SendMessage( WM_CONTEXTMENU, wParam, lParam );

    m_bEnableCopyAttachment = false;
}

