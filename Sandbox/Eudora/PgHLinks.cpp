// PgHLinks.cpp -- support for URLs, attachments, etc...

#include "stdafx.h"

// eudora
#include "eudora.h"
#include "trnslate.h"
#include "guiutils.h"

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

// our public face
#include "PgHLinks.h"


///////////////////////////////////////////////////////////////////////////////
// stuff at module scope

static void scroll_to_target( pg_ref pg, char* url );
static void run_plugin_schmookie( paige_rec_ptr pgRec, long linkBegin,
                        long linkEnd, char* url );
static bool get_hyperlink_schmookie( pg_ref pg, long hlinkID,
                        pg_hyperlink_ptr pHyperlink, long* pBegin, long* pEnd, char* pUrl );

static void clean_filepath( char* in, char* out );


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
                char filepath[MAX_PATH] = "";
                clean_filepath( url, filepath );
                run_plugin_schmookie( pgRec, begin, end, filepath );
            }
            else if ( pgHLink.target_id && *url == '#' ) {
                scroll_to_target( pgRec->doc_pg, &url[1] );

                // here comes da butt-ugliest hack...
                ((CPaigeEdtView*)pSB->pWndOwner)->UpdateScrollBars();
            }
            else /*if ( pgHLink.type & HYPERLINK_EUDORA_AUTOURL )*/ {
                // we probably need to give some of the autourl stuff special
                // treatment; such as "mailto". for now, we ShellExecute
                // everything.
                ::SetCursor( ::LoadCursor( 0, IDC_APPSTARTING ) );
                //::ShellExecute( NULL, "open", (LPCSTR)url, NULL, NULL,
                //              SW_SHOWNORMAL );
                LaunchURL((LPCSTR)url);
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

        QCProtocol* QP = QCProtocol::QueryProtocol( QCP_TRANSLATE, pSB->pWndOwner );
        assert( QP );

        if ( QP ) {
            pgSetSelection( pgRec->doc_pg, linkEnd, linkEnd, 0, true );
            CEudoraApp* pApp = (CEudoraApp*) AfxGetApp();
            ::SetCursor( ::LoadCursor( 0, IDC_APPSTARTING ) );
            pApp->GetTranslators()->XLateDisplay( QP, url );
        }
    }
    else
        ::MessageBeep( MB_ICONEXCLAMATION );
}
#endif   // unsupported com protocol


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
    long begin = pgFindHyperlinkTarget( pg, 0, &end, (unsigned char*)url,
                                        false, false, true);
}


// BOG: this exist at module scope in both PgEmbeddedImage.cpp, and here.
// hopefully, someday, I'll come up with something better!

static void clean_filepath( char* in, char* out )
{
    static const char* file_spec = "file://";
    char* path = in;     // set this up so failure returns the "in"

    if ( strnicmp( in, file_spec, strlen(file_spec) ) == 0 ) {
        path = strstr( in, ":" );
        path += 3;
    }

    strcpy( out, path );
}
