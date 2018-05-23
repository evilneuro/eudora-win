// plist_utils.cpp
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "plist_utils.h"
#include "plist_debug.h"
#include "plist_mgr.h"

#include "DebugNewHelpers.h"

////////////////////////////////////////////////////////////////////////////////
// private defs & decls

static char x2c( char *what );


////////////////////////////////////////////////////////////////////////////////
// public interface

// TODO: Can ports be zero? should i always return *some* port# from this?
// that first "for" loop is a bug, right?
void MunchURL( char* url, char** host, char** uri, int* port )
{
    // get the host server's address/name
    char* pDomBegin = url;
    for ( ; *pDomBegin && *pDomBegin != '/'; pDomBegin++ )
        ;
    for ( ; *pDomBegin && *pDomBegin == '/'; pDomBegin++ )
        ;
    char* pDomEnd = strchr( pDomBegin, '/' );
    int len = pDomEnd - pDomBegin;
	char* pHost = DEBUG_NEW char[len + 1];
    strncpy( pHost, pDomBegin, len );
    pHost[len] = 0;

    // the rest is the responder's URI
    char* pResponder = DEBUG_NEW char[strlen(pDomEnd) + 1];
    strcpy( pResponder, pDomEnd );

    *host = pHost;
    *uri  = pResponder;
    *port = 80;

    // sometimes host name contains a port number
    char* pOtherPort = strchr( pHost, ':' );

    if ( pOtherPort ) {
        *pOtherPort = 0;
        pOtherPort++;
        *port = atoi( pOtherPort );
    }
}


// duplicate a string using "new" operator
char* DupeString( const char* str )
{
	char* ps = DEBUG_NEW_NOTHROW char[strlen( str ) + 1];

	if ( ps )
		strcpy( ps, str );

	return ps;
}


// return a mime-type string for a url
//
// TODO: Make this thing real!!
char* MimeString( const char* url )
{
	char* mimeString = 0;
	char* pType = strrchr( url, '.' );

	if ( pType ) {
		if ( strstr( ++pType, "htm" ) )
			mimeString = DupeString( "text/html" );
		else
			mimeString = DupeString( "image/jpeg" );
	}

	return mimeString;
}


// returns a fully qualified name for a temp file. we do two allocations here
// because we don't want the use of "malloc" by "tempnam" to leak out.
char* TempFile()
{
	char* fileName = 0;
	char* tmp = tempnam( 0, "plst" );

	if ( tmp ) {
		fileName = DupeString( tmp );
		free( tmp );
	}
	
	SDTrace1("TempFile %s",fileName);

	return fileName;
}


// BOG: this dynamic duo needs to be rewritten. they handle hex-encoded
// html escape sequences, but what they should be handling is xml
// escape sequences. maybe should just let wininet do this.

// Reduce any %xx escape sequences to the characters they represent
void UnescapeUrl( char* /*url*/ )
{
#if 0
    int i,j;

    for( i=0, j=0; url[j]; ++i,++j ) {
        if( (url[i] = url[j]) == '%' ) {
            url[i] = x2c( &url[j + 1] ) ;
            j += 2 ;
        }
    }

    url[i] = '\0' ;
#endif
}

#if 0
// Convert a two-char hex string into the char it represents
char x2c( char *what )
{
   char digit;

   digit = (char)(what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
   digit *= 16;
   digit += (char)(what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));

   return(digit);
}
#endif

// Get an ini string from the client app
// Note that this does not return a static buffer, you
// must delete [] what you get back
char *GetIniStringNotStatic ( void *inProc, int num )
{
	plist_callback userProc = (plist_callback)inProc;
	char *returnStr=NULL;
	userProc( PLIST_GET_INI_STRING, (long)&returnStr, num );
	return returnStr;
}

#ifdef PLIST_LOGGING
// pretty-print a duration
CString Secs2Duration( int secs, int sigdigs/*=1*/ )
{
	CString buffer;
	char tmp[32];
		
	if ( secs < 0 )
	{
		secs *= -1;
		buffer += '-';
	}
	
	if ( secs<60 && sigdigs<=0 ) sigdigs = 1;
	
	do
	{
		if ( secs > A_YEAR )
		{
			sprintf(tmp,"%dy",secs/A_YEAR);
			buffer += tmp;
			secs %= A_YEAR;
		}
		else if ( secs > A_MONTH )
		{
			sprintf(tmp,"%dmo",secs/A_MONTH);
			buffer += tmp;
			secs %= A_MONTH;
		}
		else if ( secs > A_DAY )
		{
			sprintf(tmp,"%dd",secs/A_DAY);
			buffer += tmp;
			secs %= A_DAY;
		}
		else if ( secs > AN_HOUR )
		{
			sprintf(tmp,"%dh",secs/AN_HOUR);
			buffer += tmp;
			secs %= AN_HOUR;
		}
		else if ( secs > A_MINUTE )
		{
			sprintf(tmp,"%dm",secs/A_MINUTE);
			buffer += tmp;
			secs %= A_MINUTE;
		}
		else
		{
			if ( sigdigs > 0 )
			{
				if (secs) sprintf(tmp,"%ds",secs);
				else strcpy(tmp,"0");
				buffer += tmp;
			}
			secs = 0;
		}
	}
	while ( secs && --sigdigs!=0 );
	
	return buffer;
}	
#endif
