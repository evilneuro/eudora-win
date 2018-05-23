// xml_parser_base.cpp -- base class implimentation for xml parsers.
// Just drink and derive!
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


#include "stdafx.h"

#include "stdlib.h"
#include "assert.h"
#include "ctype.h"

// our own ugly butt
#include "xml_parser_base.h"

#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// Define this to compile test stub (console app)

// #define XML_PARSER_TEST


///////////////////////////////////////////////////////////////////////////////
// Our one and only public interface

int XmlParser::Parse( const char* xml, unsigned len, bool done )
{
    int ret = 0;

	if ( !m_pElementMap )
		initElementMap( &m_pElementMap );

    if (!XML_Parse(m_parser, xml, len, done)) {
//#ifdef XML_PARSER_TEST
        fprintf(stderr,
                "%s at line %d\n",
                XML_ErrorString(XML_GetErrorCode(m_parser)),
                XML_GetCurrentLineNumber(m_parser));
//#endif
        ret = 1;
    }

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// The guts

void XmlParser::s_startElement(void *userData, const char *name, const char **atts)
{
    XmlParser* pMyAss = (XmlParser*) userData;
    pMyAss->m_depth += 1;

    int id = pMyAss->id_from_name( name );
    pMyAss->m_openTags.push( id );
    pMyAss->startElement( id, name, atts );
}

void XmlParser::s_endElement(void *userData, const char *name)
{
    XmlParser* pMyAss = (XmlParser*) userData;
    int id = pMyAss->m_openTags.top();

    assert( pMyAss->id_from_name( name ) == id );

    pMyAss->endElement( id, name );
    pMyAss->m_depth -= 1;
    pMyAss->m_openTags.pop();
}

void XmlParser::s_characterDataHandler( void *userData, const XML_Char *s, int len )
{
    XmlParser* pMyAss = (XmlParser*) userData;

    for ( ; (!isalnum((int)(unsigned char)*s) && !ispunct((int)(unsigned char)*s)) && len; s++, len-- )
        ;

    if ( len )
        pMyAss->handleData( pMyAss->m_openTags.top(), s, len );
}


///////////////////////////////////////////////////////////////////////////////
// Test stub

#ifdef XML_PARSER_TEST
#include "io.h"
#include "fcntl.h"

// empty map; we handle anything.
ElementMap MyElementMap[] = {
    id_none,    "youSuck"
};


// little parser that likes to print out tags
class XmlParserTest : public XmlParser
{
    char m_outBuffer[1152];    // Max URL + 128 for tags

public:
    XmlParserTest(){*m_outBuffer = 0;}
    ~XmlParserTest(){};

private:
    void initElementMap( ElementMap** map ) {
        *map = MyElementMap;
    }

    void handleData( int id, const char* data, int len ) {
        strcat( m_outBuffer, "=" );
        strncat( m_outBuffer, data, len );
    }

    int startElement( int id, const char* name, const char** atts ) {
        if ( *m_outBuffer )
            puts( m_outBuffer );

        for (int i = 0; i < m_depth; i++)
            putchar('\t');

        strcpy( m_outBuffer, name );
        return 0;    // continue default processing
    }

    int endElement( int id, const char* name ) {
        if ( *m_outBuffer ) {
            puts( m_outBuffer );
            *m_outBuffer = '\0';
        }

        return 0;    // continue default processing
    }
};


int main( int argc, char* argv[] )
{
    int h = open( argv[1], O_BINARY | O_RDONLY );
    long lSize = lseek( h, 0, SEEK_END );
    char* buf = (char*) malloc( lSize + 1 );

    lseek( h, 0, SEEK_SET );
    read( h, buf, lSize );
    buf[lSize] = 0;

    XmlParserTest xpt;
    return xpt.Parse( buf );
}

#endif    // XML_PARSER_TEST
