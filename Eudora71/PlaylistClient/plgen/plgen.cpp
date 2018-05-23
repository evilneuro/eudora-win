// plgen.cpp -- generate a skelleton playlist from a collection of image files

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "io.h"
#include "direct.h"


////////////////////////////////////////////////////////////////////////////////
// Utils

inline void trim( char* s )
{
	char* c;
	for ( c = s; *c == ' '; c++ )
		;
	if ( c != s )
		strcpy( s, c );
	for ( c = &s[strlen(s)-1]; *c == ' '; c-- )
		;
	*(c+1) = 0;
}


////////////////////////////////////////////////////////////////////////////////
// xml generation

char* elements[] = {"clientUpdateResponse", "clientInfo", "reqInterval",
		"playlist", "playlistID", "entry", "adID", "title", "src", "url",
		"showFor", "showForMax", "dayMax", "blackBefore", "blackAfter",
		"startDT", "endDT", "nextAd"};

enum element_ids {clientUpdateResponse, clientInfo, reqInterval, playlist,
		playlistID, entry, entryID, title, src, href, showFor, showForMax,
		dayMax, blackBefore, blackAfter, startDT, endDT, nextEntry};

const int kNumElements = sizeof(elements)/4;
char* defdata[kNumElements];


// track levels of indentation
int level = 0;

// write out an element start tag
inline void startElement( int id, char** attrs, bool linebreak )
{
	for ( int j = level; j > 0; j-- )
		printf( "\t" );

	printf( "<%s>", elements[id] );

	if ( linebreak )
		printf( "\n" );

	level++;
}

// write out an element end tag
inline void endElement( int id, bool linebreak )
{
	level--;

	if ( linebreak ) {
		for ( int j = level; j > 0; j-- )
			printf( "\t" );
	}

	printf( "</%s>\n", elements[id] );
}

inline void writeElementData( int id )
{
	if ( defdata[id] ) {
		printf( defdata[id] );

		if ( id == src )
			*(defdata[src]) = 0;
	}
}

inline void outputDataField( int id )
{
	startElement( id, 0, false );
	writeElementData( id );
	endElement( id, false );
}

void initDefdata()
{
	char buf[512];
	char* key, * value;

	memset( defdata, 0, sizeof(defdata) );
	FILE* fp = fopen( "plgen.ini", "r" );

	if ( fp ) {
		while ( fgets( buf, sizeof(buf), fp ) ) {
			strtok( buf, "\n" );

			key = buf;
			value = strchr( buf, '=' );

			if ( !value || *key == ';' || *key == '[' )
				continue;

			*value++ = 0;
			trim( value );

			for ( int i = 0; i < kNumElements; i++ ) {
				if ( i != src && stricmp( key, elements[i] ) == 0 )
					defdata[i] = strdup( value );
			}
		}

		fclose( fp );
	}

	// the src url cannot be initialized from the ini file
	defdata[src] = (char*) calloc( 1024, sizeof(char) );
	assert( defdata[src] );
}

void disposeDefdata()
{
	for ( int i = 0; i < kNumElements; i++ ) {
		if ( defdata[i] )
			free( defdata[i] );
	}
}


////////////////////////////////////////////////////////////////////////////////
// path manglers

inline void ripslash( char* s )
{
	int i;
	if ( s[(i=strlen(s))-1] == '\\' )
		s[i] = 0;
}

inline void makeSearchSpec( char* dirpath, char* out )
{
	strcpy( out, dirpath );
	ripslash( out );
	strcat( out, "\\*" );
}

//
// this is pretty limited right now. it only handles relative paths that are
// at or below the current working directory, or fully qualified paths that
// begin with a drive specifier.
//
void getContentLocation( char* dirpath, char* out, int maxlen )
{
	strcpy( out, "file://" );

	// if dirpath is fully qualified, just use that
	if ( dirpath[1] == ':' ) {
		strcpy( out, dirpath );
	}
	else {
		int dpLen = strlen( dirpath );
		bool cwd = (*dirpath == '.') && (dpLen == 1);

		// magic number is "file://" and one backslash
		int maxwdlen = maxlen - (cwd ? 0 : dpLen) - 8;

		if ( _getcwd( out+7, maxwdlen ) ) {
			ripslash( out );

			if ( !cwd ) {
				strcat( out, "\\" );
				strcat( out, dirpath );
				ripslash( out );
			}
		}
		else
			*out = 0;
	}
}

#define errmsg(s) (fputs( s"\n", stderr ))

int main( int argc, char* argv[] )
{
	if ( argc < 2 ) {
		errmsg( "plgen -- generate playlist from a collection of image files\n" );
		errmsg( "   usage: plgen <image path>" );
		exit(-1);
	}
	else if ( argc > 2 ) {
		if ( !freopen( argv[2], "w+", stdout ) ) {
			fprintf( stderr, "Unable to create %s\n", argv[2] );
			exit( -1 );
		}
	}
	else if ( argc > 3 )
		errmsg( "plgen -- ignoring additional arguements" );

	int ret = -1;
	int entries = 0;
	long hFind;
	bool fileFound;
	char srchSpec[_MAX_PATH] = "";
	char contentLoc[_MAX_PATH] = "";
	struct _finddata_t fileInfo;

	initDefdata();
	makeSearchSpec( argv[1], srchSpec );
	getContentLocation( argv[1], contentLoc, sizeof(contentLoc) );

	if ( (hFind = _findfirst( srchSpec, &fileInfo )) != -1 ) {

		// first two files are always directories, and we need to find at least
		// one file to continue---currently don't do subdirs.
		while ( _findnext( hFind, &fileInfo ) == 0 ) {
			if ( fileFound = !(fileInfo.attrib & _A_SUBDIR) )
				break;
		}

		if ( fileFound ) {
			startElement( clientUpdateResponse, 0, true );

			// write the clientInfo block
			startElement( clientInfo, 0, true );
			outputDataField( reqInterval );
			endElement( clientInfo, true );

			// the actual playlist starts here
			startElement( playlist, 0, true );
			outputDataField( playlistID );

			do {
				// make a full URL to our found file
				sprintf( defdata[src], "%s\\%s", contentLoc, fileInfo.name );
				startElement( entry, 0, true );

				for ( int i = entryID; i < kNumElements; i++ )
					outputDataField( i );

				endElement( entry, true );
				entries++;
			} while ( _findnext( hFind, &fileInfo ) == 0 );

			endElement( playlist, true );
			endElement( clientUpdateResponse, true );
			ret = 0;
		}
		else
			errmsg( "plgen -- that directory ain't got a darn thing in it" );

		_findclose( hFind );
	}
	else
		errmsg( "plgen -- we don't like that image directory path" );

	disposeDefdata();
	fprintf( stderr, "%i playlist entries written\n", entries );
	return ret;
}

