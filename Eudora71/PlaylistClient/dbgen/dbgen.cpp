// dbgen.cpp -- generates a playlist database from playlist xml
//
// Note: This program is used to create databases to be used for demonstration
//       and development purposes, by Eudora personnel only.

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"

#ifdef WIN32
#include "io.h"
#else
#include "unistd.h"
#endif

#include "plist_parser.h"
#include "plist_stg.h"


int main( int argc, char* argv[] )
{
	if ( argc > 1 ) {
		// redirect standard in
		if ( !freopen( argv[1], "r", stdin ) ) {
			printf( "dbgen -- failed to open %s\n", argv[1] );
			exit( -1 );
		}
	}
	else if ( argc > 2 )
		puts( "dbgen -- ignoring additional arguements" );

	int ret = -1;
	int hdb, hplst;

	if ( (hdb = DbInit( "eudora" )) != -1 ) {
		int len;
		bool done;
		char buf[5120];
		PlaylistParser pp;

		printf( "dbgen -- importing %s\n", argv[1] );

		do {
			len = fread( buf, sizeof(char), sizeof(buf), stdin );
			done = len < sizeof(buf);

			if ( (hplst = pp.ImportPlaylist( buf, len, done )) ) {
				if ( hplst == -1 ) {
					puts( "dbgen -- playlist import failed" );
					break;
				}
				else {
					DbAddPlaylist( hdb, hplst );
					ClosePlaylist( hplst );
					ret = 0;
				}
			}

		} while ( !done );

		DbShutdown( hdb );
	}
	else
		puts( "dbgen -- error initializing database" );

	return ret;
}

