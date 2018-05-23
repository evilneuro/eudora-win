// PgHLinks.cpp -- support for URLs, attachments, etc...
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

// eudora
#include "eudora.h"
#include "trnslate.h"
#include "guiutils.h"
#include "HTMLUtils.h"

// standard Paige includes
#include "Paige.h"
#include "pgTraps.h"
#include "pgOSUtl.h"
#include "pgUtils.h"
#include "pgTxrCPP.h"
#include "defprocs.h"
#include "pgErrors.h"
#include "pgDefStl.h"
#include "pgHTMDef.h"
#include "pgEmbed.h"
#include "pgdeftbl.h"
#include "machine.h"
#include "pgTables.h"
#include "pgHText.h"
#include "pgFrame.h"

// editor protocols
#include "qcprotocol.h"

// plain ol' butt-ugly spaghetti
#include "PgReadMsgView.h"
#include "PgReadMsgPreview.h"
#include "PgStuffBucket.h"
#include "TocFrame.h"

// needed for reg info handling
#include "RegInfoReader.h"

// needed for Ad URL clicking
#include "LinkHistoryManager.h"

// needed for improved handling of XLateDisplay
#include "msgdoc.h"

// our public face
#include "PgHLinks.h"


#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// stuff at module scope

static void scroll_to_target( pg_ref pg, char* url );
static void run_plugin_schmookie( paige_rec_ptr pgRec, long linkBegin,
                        long linkEnd, char* url );
static void process_eudorainfo_schmookie(paige_rec_ptr	pgRec, long linkEnd, char * szAttachmentPath);
static bool get_hyperlink_schmookie( pg_ref pg, long hlinkID,
                        pg_hyperlink_ptr pHyperlink, long* pBegin, long* pEnd, char* pUrl );

static void clean_filepath( char* url, bool decode );


///////////////////////////////////////////////////////////////////////////////
// Da Beef


// PgInitHyperlinks - Sets up the callback handlers for hyperlinks in a Paige
// reference. The Paige docs claim this should be called immediately after an
// import, which seems a little strange; whatever.

void PgInitHyperlinks( pg_ref pg )
{
    pgSetHyperlinkCallback( pg, hlCallback, NULL );
}


// PgProcessHyperlinks - hyperlink handling for any pg_ref. if the user has
// clicked a link, its id will be set in the Paige reference's "stuff bucket";
// this routine is called in CPaigeEdtView's "OnLButtonUp" handler.
//
// UGLY: this is gettin' there!

void PgProcessHyperlinks( pg_ref pg )
{
    // only if there is a link id a-waiting
    long hlinkID = 0;
    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( pg );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;

    if ( pSB->hlinkProcessing == true ) {
        // BOG: this should catch any instance of a paige editor trying to
        // process a hyperlink while one is currently being processed. note
        // this condition cannot be addressed here; it's the caller's problem.
        UnuseMemory( pg );
        assert( 0 );
        return;
    }
    else {
        // dupe the eventID  and blank it out right now; ya never know if we
        // might get called again too soon. Schmookie!!
        pSB->hlinkProcessing = true;
        hlinkID = pSB->hlinkEvent;
        pSB->hlinkEvent = 0;
    }

    if ( hlinkID ) {
        pg_hyperlink pgHLink;
        long begin, end;
        char url[1024];

        bool all_schmooked_up = get_hyperlink_schmookie( pg, hlinkID, &pgHLink,
                                                         &begin, &end, url );

        if ( all_schmooked_up ) {

            // BOG: don't really know how big a run_plugin path can get; i
            // assume it's MAX_PATH + the size of the trans_id. That would
            // mean, of course, that the following code has a never to be
            // found bug.
                        
            if ( pgHLink.type & HYPERLINK_EUDORA_PLUGIN ) {
				// BOG [2/11/00]: apparently no need to decode this url. never
				// have in the past; better not to start now.
                clean_filepath( url, false );
                run_plugin_schmookie( pgRec, begin, end, url );
            }
			else if (pgHLink.type & HYPERLINK_EUDORA_INFO) {
                clean_filepath( url, true );
				process_eudorainfo_schmookie(pgRec, end, url);
			}
            else if ( pgHLink.target_id && *url == '#' ) {
                scroll_to_target( pgRec->doc_pg, &url[1] );

                // here comes da butt-ugliest hack...
                ((CPaigeEdtView*)pSB->pWndOwner)->UpdateScrollBars();
            }
            else /*if ( pgHLink.type & HYPERLINK_EUDORA_AUTOURL )*/ {
                ::SetCursor( ::LoadCursor( 0, IDC_APPSTARTING ) );

				CPaigeEdtView* pPEV = DYNAMIC_DOWNCAST(CPaigeEdtView, pSB->pWndOwner);
				if (pPEV)
				{
					char			szLinkText[1024];
					select_pair		linkSel;

					linkSel.begin = begin;
					linkSel.end = end;

					pPEV->GetPgText(szLinkText, sizeof(szLinkText), linkSel, FALSE);
					
					pPEV->LaunchURL((LPCSTR)url, szLinkText);
				}
				else
				{
					// Why didn't we get a CPaigeEdtView object?
					// Oh well, just launch the URL anyway, as
					// we'd hate to make the user pay for our mistake.
					ASSERT(0);
					LaunchURL((LPCSTR)url);
				}
            }
        }
    }

    pSB->hlinkProcessing = false;
    UnuseMemory( pg );
}


///////////////////////////////////////////////////////////////////////////////
// Callback interface to Paige

// hlCallback - "Hyperlink Callback". This gets called whenever the user clicks
// on an anchor. We check the hyperlink type and give special processing to
// attachments and plugins; otherwise we pass it off to a browser.

PG_PASCAL (long) hlCallback( paige_rec_ptr pg, pg_hyperlink_ptr hypertext,
                   short command, short modifiers, long position,
                   pg_char_ptr URL )
{
	if (command == hyperlink_delete_verb) {
		// No need for Ben's assert, or indeed any action - just return
		return 0;
	}
	
	PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;

    if ( pSB->hlinkProcessing == true ) {
        // BOG: this should catch any instance of a paige editor trying to
        // process a hyperlink while one is currently being processed. note
        // this condition cannot be addressed here; it's the caller's problem.
        assert( 0 );
        return 0;
    }

    // call default handler for standard htext behaviors
    pgStandardSourceCallback( pg, hypertext, command, modifiers,
                              position, URL );

    long begin, end;
    pgGetSelection( pg->doc_pg, &begin, &end );

    if ( (command == hyperlink_mouseup_verb) && (begin == end) ) {

        // simply stash the hyperlink's id for later processing
        pSB->hlinkEvent = hypertext->unique_id;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// little helpers


// run_plugin_schmookie - another fabulous schmookie routine! this one knows
// how to run EMSAPI plugins. it currently is typed to take a ptr to a Paige
// record in order to keep house keeping calls to a minimum; can't think
// what might change my mind about that decision.

#if 0   // This code uses the new com-style protocol, smart ptrs, etc. It is
        // not currently supported outside of the Graceland branch.

static void run_plugin_schmookie( paige_rec_ptr pgRec, long linkBegin,
                                        long linkEnd, char* url )
{
    // BOG: this code does not allow plugins to run except in "read message"
    // views. currently, running them in preview panes causes errors in the
    // translator code as preview panes do not provide message headers through
    // their GetMessageAsText interface. it would seem that this limitation
    // could be overcome without too much trouble.

    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    CWnd* pwnd = pSB->pWndOwner;

    // UGLY: should really do this with type/subtype "stuff"; leave it for now.
    if ( pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgView)) ||
         pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgPreview)) ) {

        ProtocolPtr spProtocol(0);   // smartptr

        if ( SUCCEEDED(::QueryProtocol(pSB->pWndOwner, QCP_TRANSLATE, &spProtocol)) ) {
            pgSetSelection( pgRec->doc_pg, linkEnd, linkEnd, 0, true );

            // run the translator
            ::SetCursor( ::LoadCursor( 0, IDC_APPSTARTING ) );
            CEudoraApp* pApp = (CEudoraApp*) AfxGetApp();
            pApp->GetTranslators()->XLateDisplay( spProtocol, url );
        }
    }
    else
        ::MessageBeep( MB_ICONEXCLAMATION );
}
#else
static void run_plugin_schmookie( paige_rec_ptr pgRec, long linkBegin,
                                        long linkEnd, char* url )
{
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    CWnd* pwnd = pSB->pWndOwner;

    if ( pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgView)) ||
         pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgPreview)) ) {

		bool			bAllow = true;
		CMessageDoc *	pMessageDoc = NULL;
		CSummary *		pSummary = NULL;

		if ( pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgPreview)) ) {
			CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( pwnd->GetParentFrame() );

			ASSERT(pTocFrame);
			ASSERT_KINDOF(CTocFrame, pTocFrame);

			if ( pTocFrame && pTocFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)) ) {
				const CTocFrame::SummaryArrayT &	summaryArray = pTocFrame->GetPreviewSummaryArray();
				int									nNumSelectedMessages = summaryArray.GetSize();

				if (nNumSelectedMessages > 1) {
					//	We don't support this when multiple messages are selected - display error message
					ErrorDialog(IDS_TLR_ERROR_MULTIPLE_MESSAGES);

					bAllow = false;
				}
				else if (nNumSelectedMessages == 1) {
					pSummary = summaryArray[0];
				}
				else {
					::MessageBeep( MB_ICONEXCLAMATION );
					bAllow = false;
				}
			}
			else {
				::MessageBeep( MB_ICONEXCLAMATION );
				bAllow = false;
			}
		}
		else {
			pMessageDoc = reinterpret_cast<CMessageDoc *>( reinterpret_cast<PgReadMsgView *>(pwnd)->GetDocument() );
			pSummary = pMessageDoc->m_Sum;

			if (!pMessageDoc) {
				::MessageBeep( MB_ICONEXCLAMATION );
				bAllow = false;
			}
		}

		if (bAllow) {
			QCProtocol* QP = QCProtocol::QueryProtocol( QCP_TRANSLATE, pSB->pWndOwner );
			assert( QP );
			
			if ( QP ) {
				pgSetSelection( pgRec->doc_pg, linkEnd, linkEnd, 0, true );
				CEudoraApp* pApp = (CEudoraApp*) AfxGetApp();
				::SetCursor( ::LoadCursor( 0, IDC_APPSTARTING ) );
								
				pApp->GetTranslators()->XLateDisplay(QP, pSummary, pMessageDoc, url);
			}
		}
    }
    else
        ::MessageBeep( MB_ICONEXCLAMATION );
}
#endif   // unsupported com protocol

// process_eudorainfo_schmookie - processes eudora info (reg, profile, etc.) indicated by a special file URL.
// When in schmookie land be schmookie! - GCW
static void process_eudorainfo_schmookie(paige_rec_ptr	pgRec, long linkEnd, char * szAttachmentPath)
{
    PgStuffBucket *		pSB = (PgStuffBucket*) pgRec->user_refcon;
    CWnd *				pwnd = pSB->pWndOwner;

	if ( pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgView)) ||
		 pwnd->IsKindOf(RUNTIME_CLASS(PgReadMsgPreview)) )
	{
		pgSetSelection( pgRec->doc_pg, linkEnd, linkEnd, 0, true );
		RegInfoReader::ProcessEudoraInfoAttachment(szAttachmentPath);
	}
	else
	{
		::MessageBeep( MB_ICONEXCLAMATION );
	}
}



// get_hyperlink_schmookie - a typical "schmookie" routine, this does what the
// Paige fellows should have done from the start: provide a method of getting
// everything about a particular link in one clean shot. Idiots!

static bool get_hyperlink_schmookie( pg_ref pg, long hlinkID,
                                     pg_hyperlink_ptr pHyperlink, long* pBegin,
                                     long* pEnd, char* pUrl )
{
    bool fRet = 0;     // assume failure! (my personal motto)

    // cleanliness is next to nothing!
    memset( pHyperlink, 0, sizeof(pg_hyperlink) );
    *pBegin = 0;
    *pEnd = 0;
    *pUrl = '\0';

    long end = 0;
    long begin = pgFindHyperlinkSourceByID( pg, 0, &end, hlinkID, false );

    if ( begin != -1 ) {
        // TODO: error handling is incomplete.
        pgGetHyperlinkSourceInfo( pg, begin, 0, false, pHyperlink );
        pgGetSourceURL( pg, begin, (unsigned char*)pUrl, 1024 );
        *pBegin = begin;
        *pEnd = end;
        fRet = true;
    }

    assert( begin != -1 ); 
    assert( *pUrl );
    assert( pHyperlink->unique_id );

    return fRet;
}

static void scroll_to_target( pg_ref pg, char* url )
{
    long end = 0;
    pgFindHyperlinkTarget( pg, 0, &end, (unsigned char*)url, false, false, true);
}


// strip the "file://" from a local file url and optionally decode it
void clean_filepath( char* url, bool decode )
{
	if ( strnicmp( url, "file://", 7 ) == 0 )
		strcpy( url, &url[7] );

	if ( decode )
		unescape_url( url );
}
