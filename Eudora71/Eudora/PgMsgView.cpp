// PgMsgView.cpp : implementation file
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

// #include "PgEmbeddedImage.h"

#include "DebugNewHelpers.h"

extern CString EudoraDir;


///////////////////////////////////////////////////////////////////////////////
// null-delimited list of strings

// traverse a list
inline char* next_ndlist_item( char* prev ) {
	return &prev[strlen(prev) + 1];
}

// size, in bytes, of a ndlist
inline size_t ndlist_size( char* list ) {
	size_t listSiz = 0;
	for ( char* pi = list; *pi; pi = next_ndlist_item(pi) )
		listSiz += strlen( pi ) + 1;

	return ++listSiz;   // double null-terminated
}

// add an item
int append_ndlist_item( char** list, const char* item )
{
	assert( list ); assert( item );
	if ( !list || !item ) return -1;

	int nItems = 0;
	char* curList = *list;
	int itemSiz = strlen( item );

	if ( curList ) {
		int listSiz = ndlist_size( curList );
		int newSiz = listSiz + itemSiz + 1;
		char* buf = 0;

		if ( buf = DEBUG_NEW_NOTHROW char[newSiz] ) {
			int oldBytes = listSiz - 1;
			memcpy( buf, curList, oldBytes );
			memcpy( &buf[oldBytes], item, itemSiz );
			(*(short*) &buf[oldBytes + itemSiz]) = 0x0000;
			*list = buf;
			nItems++;

			delete [] curList;
		}
	}
	else {
		if ( curList = DEBUG_NEW_NOTHROW char[itemSiz + 2] ) {   // double-null terminated
			memcpy( curList, item, itemSiz );
			(*(short*) &curList[itemSiz]) = 0x0000;
			*list = curList;
			nItems = 1;
		}
	}

	return nItems;
}


/////////////////////////////////////////////////////////////////////////////
// PgMsgView

IMPLEMENT_DYNCREATE(PgMsgView, CPaigeEdtView)

PgMsgView::PgMsgView()
{
    fDiscardMessage = false;
    m_bEnableAttachmentMenus = false;
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
	if (pPg)
	{
		PgStuffBucket *		pStuffBucket = reinterpret_cast<PgStuffBucket *>(pPg->user_refcon);

		if (pStuffBucket)
		{
			pStuffBucket->kind = PgStuffBucket::kMessage;
			pStuffBucket->pMessage = &pMsgDoc->m_QCMessage;

			//	SetMoreMessageStuff will set subkind appropriately and in the case of
			//	PgReadMsgPreview it will set pMessage back to NULL because we the message
			//	doesn't hang around for previews.
			SetMoreMessageStuff(pStuffBucket);
		}
	}
	
	UnuseMemory( m_paigeRef );
}

void PgMsgView::SetMoreMessageStuff(
	PgStuffBucket *						pStuffBucket)
{
	//	Empty implementation
	//	Override method in subclasses
}


// GetMessageForDisplay:
// Gets the headers and body for display

void PgMsgView::GetMessageForDisplay(
	CMessageDoc *						in_pMsgDoc,
	ContentConcentrator::ContextT		in_context,
	bool								in_bStripDocumentLevelTags,
	bool								in_bRelaxLocalFileRefStripping,
	CString &							out_szBody,
	CString *							out_szHeaders,
	bool *								out_bWasConcentrated,
	bool								in_bMorphHTML)
{
    CMessageDoc *		pDoc = !in_pMsgDoc ? (CMessageDoc*)GetDocument() : in_pMsgDoc;
    ASSERT_KINDOF(CMessageDoc, pDoc);
	if (!pDoc)
		return;

    //
    // Make sure the QCMessage stuff embedded inside the CMessageDoc
    // loads the latest stuff into memory so that the GetBodyAsHTML() 
    // thing below has good stuff to party on.
    //
	char* pszDummy = pDoc->GetFullMessage();
	if (pszDummy)
	    delete [] pszDummy;

	// Pass in true for fifth parameter so that executable content is always stripped
	// from HTML. This doesn't matter for security reasons (since Paige won't execute anything),
	// but it does matter for display reasons. Script, etc. tags aren't recognized by Paige's
	// importing and so the contents of the script (or whatever) would be displayed
	// (unless the coding of the HTML disguised them as a comment for backward compatibility
	// with non-scripting browsers).
	//
	// The last parameter, in_bMorphHTML, controls whether or not cid's are converted to
	// local file URLs. It is true for preview views (PgReadMsgPreview) and false for
	// every other view.
	//
	// For preview views CIDs *are* converted to local file URLs, because we have no
	// need for them to remain CIDs and we won't have the message around later to
	// perform the conversion later.
	//
	// For all other views CIDs are *not* converted to local file URLs. For those views CIDs will
	// be converted to local file URLs temporarily when loading the image in PgLoadUrlImage. We
	// don't want them to be stored as local file URLs all the time, because PgMsgView::SaveInfo
	// removes any parts that aren't actually in the message any more and that
	// determination/comparison is made via CIDs (conversion to local file URLs breaks that
	// determination and causes all embedded parts to be removed and recreated when saving).
	pDoc->m_QCMessage.GetMessageForDisplay( in_context, out_szBody, out_szHeaders,
											out_bWasConcentrated, true, in_bStripDocumentLevelTags,
											in_bRelaxLocalFileRefStripping, in_bMorphHTML );
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

void PgMsgView::ImportMessage(
	CString &					szBody,
	PgDataTranslator *			pTxr /*=NULL*/ )
{
	PgDataTranslator* theTxr, *ourTxr = 0;
	theTxr = !pTxr ? ourTxr = DEBUG_NEW_NOTHROW PgDataTranslator(m_paigeRef) : pTxr;
	if ( !theTxr ) return;

    // BOG: doing a paige import from memory seems to be quite slow on win95,
    // so we're gonna write this out to a file, then have paige do the import
    // from file instead. makes me wonder if we should have just written
    // GetBodyAsHTML to return a CFile object instead of a CString. for totally
    // sick robustness, we'll keep the old code just in case the new fails.

    char* tmpName = _tempnam( EudoraDir, "bo" );

    try
    {
		CFile tmpFile( tmpName,
				   CFile::modeReadWrite |
				   CFile::modeCreate );

		tmpFile.Write( szBody, szBody.GetLength() );
		tmpFile.SeekToBegin();
		theTxr->ImportData( &tmpFile, PgDataTranslator::kFmtHtml );

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
			szBody.GetBuffer( szBody.GetLength() + 1 );

		// convert our c-string to a Paige Memory Manager reference
		memory_ref impRef = CString2Memory( PgMemGlobalsPtr(), pMessageBody );

		// call the appropriate Paige-based import method. note that at this
		// point all msf_xrich can be treated as html.
		theTxr->ImportData( &impRef, PgDataTranslator::kFmtHtml, draw_none );

		// CString2Memory allocates memory (its got a sneaky name!)
		DisposeMemory( impRef );
		assert(0);
    }

    free( tmpName );
	delete ourTxr;
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

	bool bInternalStylesOnly = false;
	if (pSum->IsComp())
	{
		pSum->UnsetFlagEx(MSFEX_SEND_PLAIN);
		pSum->UnsetFlagEx(MSFEX_SEND_STYLED);
	}
	if ((PgHasStyledText(m_paigeRef, PgGlobalsPtr(), NULL, &bInternalStylesOnly)) || (SelectionHasHtmlSnippet(sel)))
	{
		pSum->SetFlag(MSF_XRICH);
		pSum->SetFlagEx(MSFEX_HTML);
		txrType = PgDataTranslator::kFmtHtml;

		// If the only reason this message was saved with styles was because it had an excerpt,
		// then set the flags so it only sends a plain text version.
		if (pSum->IsComp() && bInternalStylesOnly)
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

	if (tempRef)
	{
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
	}

	return hasSnippet;
}

// SaveInfo [QCProtocol] -- do a write
HRESULT PgMsgView::SaveInfo()
{
	// BOG: This gets called blindly in a place or two, and that is causing
	// some problems. It does not appear that anyone expects this routine to
	// do a "forced" save, so I'm changing it to only save when "dirty".

	//Added a check to see if the dirty flag was set due to Hyperlinks
	//This is because we don't support undo for hyperlink editing[as yet]
//	if ( HasChanged() || GetDocument()->IsModified() )

    if ( fDiscardMessage == false ) {
		// make sure that we, and the message object [QCMessage] have the same
		// view of the world prior to committing any changes.

		CMessageDoc* pDoc = (CMessageDoc*) GetDocument();
		QCMessage* pMsg = &pDoc->m_QCMessage;

		CString ech = "";
		pMsg->GetEmbeddedObjectHeaders( ech );

		if ( !ech.IsEmpty() ) {
			char* eoList = 0;
			EnumEmbeddedObjects( &eoList );

			char* cid = ech.GetBuffer(ech.GetLength());
			char* term = 0;
			bool found;

			for ( ;; ) {
				if ( cid = strchr( cid, '<' ) ) {
					*(term = strchr( ++cid, '=' )) = '\0';
					found = false;

					if ( eoList ) {
						for ( char* pItem = eoList; *pItem && !found;
										pItem = next_ndlist_item(pItem) ) {

							if ( stricmp( cid, pItem ) == 0 )
								found = true;
						}
					}

					if ( !found )
						pMsg->RemovePart( cid );
				}
				else
					break;

				(cid = term)++;
			}

			ech.ReleaseBuffer();
			delete [] eoList;
		}

		// now write out the message
		ExportMessage();
    }

	return S_OK;
}


// enumerate our embedded content
int PgMsgView::EnumEmbeddedObjects( char** eoList )
{
	int found = 0;
	int listSiz = 0;
	char* list = 0;

	// walk all the "embed" styles, and add their uri to "eoList", which is
	// a double-null terminated list of strings.

	style_walk sw;
	embed_ref  er;
	paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
	pgPrepareStyleWalk( prp, 0, &sw, false );

	// start walkin'
	for (;;) {
		style_info_ptr sip = sw.cur_style;

		if ( er = sip->embed_object ) {
			pg_embed_ptr ep = (pg_embed_ptr) UseMemory( er );

			if ( (ep->type & EMBED_TYPE_MASK) == embed_url_image ) {
				memory_ref imgRef = (memory_ref) ep->data;
				pg_url_image_ptr uip = (pg_url_image_ptr) UseMemory( imgRef );

				// add the image's uri to our list
				const char* uri = (const char*) uip->URL;
				int uriLen = strlen( uri );

				if ( list ) {
					char* buf = 0;
					int newSiz = listSiz + uriLen + 1;

					if ( buf = DEBUG_NEW_NOTHROW char[newSiz] ) {
						int oldBytes = listSiz - 1;
						memcpy( buf, list, oldBytes );
						memcpy( &buf[oldBytes], uri, uriLen );
						(*(short*) &buf[oldBytes + uriLen]) = 0x0000;

						delete [] list;

						list = buf;
						listSiz = newSiz;
					}
				}
				else {
					listSiz = uriLen + 2;   // double-null terminated

					if ( list = DEBUG_NEW_NOTHROW char[listSiz] ) {
						memcpy( list, uri, uriLen );
						(*(short*) &list[uriLen]) = 0x0000;
					}
				}

				UnuseMemory( imgRef );
				found++;
			}

			UnuseMemory( er );
		}

		if ( !pgWalkNextStyle( &sw ) )
			break;
	}

	// use "delete []" to get rid of this
	*eoList = list;

	//	Tell Paige that we're done and it can call UnUseMemory on
	//	a bunch of internal structures.
	pgPrepareStyleWalk( prp, 0, NULL, false );

	UnuseMemory( m_paigeRef );

	return found;
}


BEGIN_MESSAGE_MAP(PgMsgView, CPaigeEdtView)
    //{{AFX_MSG_MAP(PgMsgView)
    ON_WM_CHAR()
    ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCE, OnUpdateCmdViewSource)
    ON_COMMAND(ID_INVISIBLES, OnInvisibles)
    ON_COMMAND(ID_COPY_ATTACHMENT, OnCopyAttachment)
    ON_COMMAND_RANGE(ID_SAVEAS_ATTACHMENT, ID_DELETE_ATTACHMENT, OnAttachmentAction)
    ON_UPDATE_COMMAND_UI_RANGE(ID_COPY_ATTACHMENT, ID_DELETE_ATTACHMENT, OnUpdateAttachmentAction)

    ON_WM_CONTEXTMENU()
    ON_WM_LBUTTONDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PgMsgView message handlers

void PgMsgView::OpenFileAtOffset( long offset, int command /*=0*/ )
{
	char url[MAX_PATH + 7] = "";   // "7" for the "file://" on the url
	pgGetSourceURL( m_paigeRef, offset, (pg_char_ptr)url, sizeof(url) );
	CString filePath = UnEscapeURL( url );

	if (strnicmp(filePath, "file://", 7) == 0)
	{
		DoFileCommand( ((LPCSTR)filePath) + 7, command );
	}
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

	if ( pTocFrame && (pTocFrame->GetPreviewSummaryArray().GetSize() == 0) )
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

void PgMsgView::OnCopy( COleDataSource* pDS )
{
	// do base behavior first
	CPaigeEdtView::OnCopy( pDS );

	// if there is a selection, try to copy attachments
	select_pair sel;
	pgGetSelection( m_paigeRef, &sel.begin, &sel.end );

	if ( sel.begin != sel.end )
		copy_attachments( &sel, pDS );
}

void PgMsgView::OnCopyAttachment() 
{
	COleDataSource* pData = DEBUG_NEW COleDataSource;
	copy_attachments( &m_attachmentOffset, pData );
    pData->SetClipboard();
}

void PgMsgView::copy_attachments(select_pair_ptr pSel, COleDataSource* pDS)
{
	if (pSel->begin == pSel->end)
	{
		char url[MAX_PATH + 7] = "";   // "7" for the "file://" on the url
		pgGetSourceURL(m_paigeRef, pSel->begin, (pg_char_ptr)url, sizeof(url));
		AttachmentsToClipboard(url, TRUE, pDS);
	}
	else
	{
		char* attachList = NULL;
		enum_attachments(&attachList, pSel);

		AttachmentsToClipboard(attachList, FALSE, pDS);
		delete [] attachList;
	}
}

void PgMsgView::OnAttachmentAction(UINT nID)
{
	OpenFileAtOffset( m_attachmentOffset.begin, nID );
}

void PgMsgView::OnUpdateAttachmentAction(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( m_bEnableAttachmentMenus );
}


// alakazaam!
#define is_attachment(p) ((p)->type & HYPERLINK_EUDORA_ATTACHMENT)
bool PgMsgView::enum_attachments( char** ndlist, select_pair_ptr within /*= NULL*/ )
{
	long startOffset, searchLimit;
	long maxOffset = pgTextSize( m_paigeRef );

	if ( within ) {
		startOffset = within->begin;
		searchLimit = within->end;
	}
	else {
		startOffset = 0;
		searchLimit = maxOffset;
	}

	paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
	pg_hyperlink_ptr hyperlink = (pg_hyperlink_ptr) UseMemory( prp->hyperlinks );

	for ( ; hyperlink->applied_range.end <= maxOffset; hyperlink++ ) {
		if ( hyperlink->applied_range.begin <= searchLimit ) {
			if ( is_attachment(hyperlink) ) {
				memory_ref mr = hyperlink->alt_URL;
				char* url = mr ? (char*)UseMemory(mr) : (char*)hyperlink->URL;
				CString listItem = UnEscapeURL( url );

				// get rid of the "file://" thingie
				append_ndlist_item( ndlist, ((LPCSTR)listItem) + 7 );

				if ( mr )
					UnuseMemory( mr );
			}
		}
		else
			break;
	}

	UnuseMemory( prp->hyperlinks );
	UnuseMemory( m_paigeRef );
	return (*ndlist != 0);
}

bool PgMsgView::point_in_attach(CPoint ptClient, long* pOffset /*= NULL*/)
{
	pg_hyperlink hlink;
	co_ordinate ptPgClient;
	CPointToPgPoint(&ptClient, &ptPgClient);
	long offset = pgPtInHyperlinkSource(m_paigeRef, &ptPgClient);

	if (pOffset)
		*pOffset = offset;

	if (offset != -1)
	{
		pgGetHyperlinkSourceInfo(m_paigeRef, offset, 0, false, &hlink);
		if (is_attachment(&hlink))
			return true;
	}

	return false;
}

void PgMsgView::OnContextMenu( CWnd* pWnd, CPoint ptScreen )
{
// If the hyperlink is an attachment we save the offset and set m_bEnableAttachmentMenus,
// then forward this message to the parent MessageFrame.

    CPoint ptClient(ptScreen);
    ScreenToClient(&ptClient);
    long offset;

    m_bEnableAttachmentMenus = false;

    if (point_in_attach(ptClient, &offset))
    {
        m_bEnableAttachmentMenus = true;
        long begin, end;
        pgGetSelection( m_paigeRef, &begin, &end );
        if ( begin != end ) {
            m_attachmentOffset.begin = begin;
            m_attachmentOffset.end = end;
        }
        else
            m_attachmentOffset.begin = m_attachmentOffset.end = offset;
    }

    WPARAM wParam = (WPARAM) pWnd->GetSafeHwnd();
    LPARAM lParam = MAKELPARAM(ptScreen.x, ptScreen.y);
    GetParentFrame()->SendMessage( WM_CONTEXTMENU, wParam, lParam );
}

void PgMsgView::OnLButtonDown(UINT nFlags, CPoint point)
{
	long offset;

	if (!point_in_attach(point, &offset))
		CPaigeEdtView::OnLButtonDown(nFlags, point);
	else
	{
		char url[MAX_PATH + 7] = "";   // "7" for the "file://" on the url
		pgGetSourceURL(m_paigeRef, offset, (pg_char_ptr)url, sizeof(url));

		LeftClickAttachment(this, point, url);
	}
}
