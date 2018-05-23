// plstclnt_test.cpp -- test stub for playlist subsystem workouts

#include "stdlib.h"
#include "stdio.h"
#include "process.h"
#include "conio.h"
#include "assert.h"

#include "plist_mgr.h"


// user key press
static int g_cmd = 0;
static int handle_cmd( long ldb, int cmd );

// callback command handlers
static void handle_showthis( ENTRY_INFO* pei );


// ui thread for getting keystrokes from the user
static void user_thread( void* p )
{
    for ( ;; ) {
        if ( (g_cmd = getch()) == 'q' )
            break;

        _sleep( 100 );
    }

    puts( "End UserThread" );
}


// handles callbacks from the playlist mgr
void PlaylistCallback( int cmd, long userRef, long lParam )
{
    switch ( cmd ) {
    case PLIST_BLANK:
        puts( "Blank." );
        break;
    case PLIST_SHOWTHIS:
	{
		ENTRY_INFO* pei = (ENTRY_INFO*) lParam;
		handle_showthis( pei );
		MGR_DisposeEntryInfo( pei );
	}
        break;
    case PLIST_FADEIN:
        puts( "Fadein!" );
        break;
    case PLIST_FADEOUT:
        puts( "Fadeout!" );
        break;

    default:
        assert(0);
    }
}


int main( int argc, char* argv[] )
{
    int ret = 1;

    // init the mgr and update the database
    long ldb = MGR_Init( "some_unique_id", 0, PlaylistCallback );
    MGR_UpdatePlaylists( ldb, false );

    // spawn a ui thread for getting keys
    _beginthread( user_thread, 0, NULL );

    // get the ball rolling
    MGR_BeginScheduling( ldb );

    // call MGR_Idle until user quits
    for ( ;; ) {
        if ( handle_cmd( ldb, g_cmd ) == -1 )
            break;
        else
            g_cmd = 0;

        MGR_Idle( ldb );
        _sleep( 100 );
    }

    MGR_Shutdown( ldb );
    return ret;
}


int handle_cmd( long ldb, int cmd )
{
    switch ( cmd ) {
    case 0:
        break;

    case 'q':
        return -1;

    case 'c':
	MGR_Cancel( ldb );
	puts( "Canceled." );
	break;

    case 's':
	MGR_Suspend( ldb );
	puts( "Suspended." );
	break;

    case 'r':
	MGR_Resume( ldb );
	puts( "Resumed." );
	break;

    default:
        printf( "You pressed \"%c\"\n", g_cmd );
    }

    return 0;
}

static void handle_showthis( ENTRY_INFO* pei )
{
	puts( pei->src );
}

