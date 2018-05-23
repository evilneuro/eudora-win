// plist_secret.cpp -- hide from "strings"
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

#include "memory.h"
#include "plist_secret.h"

typedef unsigned char uchar;
const char* MD5Secret::ourLittleSecret = "0fca0e1d2a8f001465cb0b1c6baf6271";

//	Disable warnings for:
//	4244: "unreferenced formal parameter"
//	4514: "assignment within conditional expression"
#pragma warning(disable : 4244 4514)


///////////////////////////////////////////////////////////////////////////////
// santa's little helpers

// uses exclusive OR to garble a buffer of text
inline void garble_not_garble( uchar* buf, int len )
{
	long* plong;
	int i, j;

	for ( i = j = 0; i < len; i++, j += 4 ) {
		plong = (long*) &buf[j];
		*plong ^= 0x7162af45;
	}
}

// converts hex pair into a char
inline uchar x2c( uchar *what )
{
   uchar digit;

   digit = (uchar)(what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
   digit *= 16;
   digit += (uchar)(what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));

   return(digit);
}

// decode a hex-encoded string of chars
inline int unhexit( uchar *url )
{
    int i,j;

    for( i=0, j=0; url[j]; ++i, j+=2 ) {
		url[i] = x2c( &url[j] ) ;
    }

    url[i] = '\0' ;
	return i/2;
}


///////////////////////////////////////////////////////////////////////////////
// MD5Secret -- decodes "Jello be dim."

MD5Secret::MD5Secret()
{
	memcpy( buf, ourLittleSecret, 32 );
	buf[32] = 0;

	// the reverse of how we got here
	int len = unhexit( buf );
	garble_not_garble( buf, len );
}

MD5Secret::~MD5Secret()
{
	memset( buf, '\0', sizeof(buf) );
}


#if 0
#include "stdio.h"
#include "string.h"


// a test decoding of "ourLittleSecret"
int main()
{
	MD5Secret theSecret;
	printf( "%i character secret: \"%s\"\n", strlen(theSecret),
				(const char*)theSecret );

	return 0;
}



// print out a buffer of potential garbage
inline void print_buf( const uchar* buf, int len )
{
	for ( int i = 0; i < len; i++ )
		putchar( buf[i] );

	puts("");
}

// hex-encode a string of chars
inline void hexit( uchar* dest, const uchar* src, int len )
{
	static uchar hex[] = "0123456789abcdef";

	for (int i = 0; i < len; i++) {
		dest[2*i] = hex[(src[i] >> 4) & 0xf];
		dest[2*i+1] = hex[src[i] & 0xf];
	}
}

// this is the code that generates "ourLittleSecret"
int main()
{
	static uchar* msg = (uchar*) "Jello be dim.";
	uchar buf[16] = "";
	uchar buf2[sizeof(buf)*2] = "";
	int msglen = strlen( (char*)msg );

	// show what the original looks like
	print_buf( msg, msglen );

	for ( int i = 0; i < msglen; i++ )
		buf[i] = msg[i];

	// now mess it up real good
	garble_not_garble( buf, sizeof(buf) );
	print_buf( buf, sizeof(buf) );
	hexit( buf2, buf, sizeof(buf) );
	print_buf( buf2, sizeof(buf2) );

	// boy, was that something?
	return 0;
}

#endif
