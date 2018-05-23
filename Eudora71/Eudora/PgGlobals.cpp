// PgGlobals.cpp
// Wrapper classes for the Paige API global data structures. Provides namespace
// protection while automatically handling startup/shutdown sequence.

#include "stdafx.h"
#include "PgGlobals.h"
#include "PgStyleUtils.h"
#include "PaigeStyle.h"
//#include "pgcntl.h"
#include "font.h"
#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"

///////////////////////////////////////////////////////////////////////////////
// static storage class data

// storage for shared global Paige objects
PgGlobals PgSharedAccess::pgGlobals;
PgMemGlobals PgSharedAccess::pgMemGlobals;

// storage for ref counting and locking
unsigned PgSharedAccess::ucRef = 0;
bool PgSharedAccess::bInitialized = false;
bool PgSharedAccess::bLocked = true;

///////////////////////////////////////////////////////////////////////////////
// the primordial shared access object

// TOOPS: the paige globals are initialized on the *second* instantiation of a
// PgSharedAccess object; they are destroyed when the last PgSharedAccess
// object is destroyed (ucRef == 0). The first and last instance of this class
// is "theSA", which is *never* used directly.

static PgSharedAccess theSA;

///////////////////////////////////////////////////////////////////////////////
// PgGlobals - Wrapper class for "pg_globals"

// init:
// Set default values common to all "pg_ref"s

bool PgGlobals::init()
{
    // Init fonts
	InitFonts();

    // set the default par line glitter proc to our own
    def_par.procs.line_glitter = PGS_MainGlitterProc;

    // failure is not in my lexicon
    return true;
}


void PgGlobals::InitFonts()
{
    // set default paige background color to the system default
    DWORD windowColor = GetSysColor( COLOR_WINDOW );
    pgOSToPgColor( &windowColor, &def_bk_color );

    // set default font and style to Eudora's "MessageFont"
    LOGFONT lf;
    font_info fontInfo;
    style_info styleInfo;
    style_info styleInfoMask;

    GetMessageFont().GetLogFont( &lf );
    PgConvertLogFont( 0, this, &lf, &fontInfo, &styleInfo, &styleInfoMask );

    // set default paige text color
    windowColor = GetSysColor( COLOR_WINDOWTEXT );
    pgOSToPgColor( &windowColor, &styleInfo.fg_color );

    memcpy( &def_font, &fontInfo, sizeof(font_info) );
    memcpy( &def_style, &styleInfo, sizeof(style_info) );

    //Set the default tab space to the user specified value * avg width of char in 
    //the default font. We also handle this in each pg_ref, incase the user changes
    //the msg font or size in between the session
    int avgWidth;
    avgWidth = GetMessageFont().CharWidth();
    def_par.def_tab_space = avgWidth * GetIniShort(IDS_INI_TAB_STOP);
}


///////////////////////////////////////////////////////////////////////////////
// PgSharedAccess - Decoupled instantiation/destruction of wrappers

// PgSharedAccess:
// Default constructor for class

PgSharedAccess::PgSharedAccess()
{
    if ( !bInitialized ) {
	if ( bLocked ) {

	    // first time this constructor is called, we will be in a "locked"
	    // state; Startup() will unlock the object + any other pre-init.
	    Startup();
	}
	else {
	    // init the Paige control (for headervw edit fields)
//	    ::InitPaigeLibs( AfxGetInstanceHandle(), (UCHAR*)"Times New Roman", NULL );

	    // fire up 'dat memory manager!
	    pgMemStartup( &pgMemGlobals, 0 );

	    // get that 'ol show on the road!
	    pgInit( &pgGlobals, &pgMemGlobals );

	    // now that the globals are copasetic, do that fabulous
	    // two step instantiation thing-a-majig.
	    pgMemGlobals.init();
	    pgGlobals.init();
	    bInitialized = true;
	}
    }
	else
	{
		// Re-initialize the global fonts everytime an object based on us is created.
		//
		// Various areas used this information and we have to make sure it reflects
		// the latest reality of settings, etc. One spot in particular that uses this
		// information is importing, which creates a temporary "shell" pgRef which in
		// turns bases it's styles on this information. If the information isn't
		// up-to-date, the new Content Concentrator code for PgReadMsgPreview doesn't
		// use the correct font if the setting has been changed since Eudora loaded.
		//
		// This may be overkill for handling the specific case in question, but it
		// seems likely that there are other similar areas that could otherwise
		// use out of date information.
		pgGlobals.InitFonts();
	}

    ucRef++;
}


PgSharedAccess::~PgSharedAccess()
{
    assert( ucRef > 0 );
    ucRef--;

    if ( ucRef == 0 ) 
	{
		Shutdown();
    }
}


// Shutdown:
// Calls Paige API to destroy Paige global objects

void PgSharedAccess::Shutdown()
{
    // this should never be called while windows objects are still holding
    // references to the globals. don't know what we'll do if this happens!
// BOG: supports silly mode.
//    assert( ucRef == 0 );

    // shutdown occurs in the opposite order to startup
	if (bInitialized)
	{
		pgShutdown( &pgGlobals );
		pgMemShutdown( &pgMemGlobals );
		bInitialized = false;
		bLocked = true;
	}
}


// SillyShutdown - this routine's purpose is preserve the old "shutdown
// in ExitInstance" way of doing business. I just couldn't get comfortable with
// the new auto-shutdown thing (even though it appears to work fine), as it
// happens outside of WinMain in CRT code.

void PgSharedAccess::SillyShutdown()
{
    // this is a bit of hack, and not really recommended. the assert makes
    // sure that only the primordial "theSA" still exists.
    assert( ucRef == 1 );
    theSA.Shutdown();
}

