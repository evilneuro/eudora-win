// xml_parser_base.h -- base implimentation of an xml parser. cannot be
// instantiated---you must derive!
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

#ifndef _XML_PARSER_BASE_H_
#define _XML_PARSER_BASE_H_

// unreferenced in inline function removed 
#pragma warning ( disable : 4514 ) 

#ifndef _DEBUG 
// function 'xyz' not inlined. 
#pragma warning ( disable : 4710 ) 
#endif 

// Force everything to warning level 3.
#pragma warning ( push , 3 ) 

// This is really a dumb warning .... 'xyz':identifier was truncated to '255' characters in the debug information 
#pragma warning ( disable : 4786 ) 

// unreachable code .. this will be generated when the code is compiled with /W4 and /Osle (optimize for size, loop, and
// global-register allocation). 
#pragma warning ( disable : 4702 ) 

#include "expat.h"

#include "stack"
using namespace std;


// keep track of open elements
typedef stack<int> TagStack;

// map tag names to internal ids
typedef struct _ElementMap {
    int   id;
    char* name;
} ElementMap;

enum {id_none = -1};    // *** always last item in map ***


// here's a sample element map. it maps tag names to ids. you must provide
// one of these things if you intend to use "handleData."

#if 0
enum {
    name,
    address
};
ElementMap _empty_map[] = {
    name,       "name",
    address,    "address",

    id_none,    "no_habla_xml"
};
#endif


///////////////////////////////////////////////////////////////////////////////
// XmlParser class - derive and override "handleData" and "initElementMap".

class XmlParser {
    ElementMap*       m_pElementMap;
	XML_Parser        m_parser;

protected:
    TagStack          m_openTags;
    int               m_depth;

public:
    XmlParser(){
		m_depth = 0;
		m_pElementMap = 0;
		m_parser = XML_ParserCreate(NULL);
		XML_SetUserData(m_parser, this);
		XML_SetElementHandler(m_parser, s_startElement, s_endElement);
		XML_SetCharacterDataHandler(m_parser, s_characterDataHandler);
 	}
    ~XmlParser(){XML_ParserFree(m_parser);}

    int Parse( const char* xml, unsigned len, bool done );

private:
    static void s_startElement(void *userData, const char *name, const char **atts);
    static void s_endElement(void *userData, const char *name);
    static void s_characterDataHandler( void *userData, const XML_Char *s, int len );

    // override any of these for low-level access. return zero to continue
    // normal processing, non-zero to do your own thing.

    virtual int startElement( int /*id*/, const char* /*name*/, const char** /*atts*/ ){return 0;}
    virtual int endElement( int /*id*/, const char* /*name*/ ){return 0;}

    // these callbacks are all you need for a simple parser. provide a ptr
    // to your ElementMap, and impliment "handleData"---which gets called
    // with a tag id, char data ptr, and length of data.

    virtual bool initElementMap( ElementMap** elementMap ) = 0;
    virtual void handleData( int id, const char* data, int len ) = 0;

    // move along. nothing to see here.
    int id_from_name( const char* name ) {
        for ( int i = 0; m_pElementMap[i].id != id_none; i++ ) {
            if ( stricmp( name, m_pElementMap[i].name ) == 0 )
                return m_pElementMap[i].id;
        }

        return id_none;
    }
};

// Get the warning level back to the original one.
#pragma warning ( pop ) 

#endif    // _XML_PARSER_BASE_H_
