// plist_html.cpp
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

// Include the files in the correct order to allow leak checking with malloc.
// CRTDBG_MAP_ALLOC already defined in stdafx.h
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>

// our own lame self
#include "plist_html.h"

#ifdef __GNUC__
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

//	Ignore "unreferenced inline function has been removed" for placement new method.
#pragma warning(disable : 4514)

#include "DebugNewHelpers.h"

////////////////////////////////////////////////////////////////////////////////

// *** Local Prototypes ***
static BOOL WriteCharToBlock(char ** ppOut, char c, int & nMaxBlock, int & nPos);
static int stub_callback( char*, long, char*, int );
static int munge_url_callback( char*, long, char*, int );


int ParseURLs(char * pszIn, ParseURLCallback pFn, long userRef, char ** ppszOut)
{
    // Bit set indicates that an attribute beginning is possible
    const BYTE kbyAttribBit = 1;

    // Bit set indicates a character is whitespace
    const BYTE kbyWhiteSpaceBit = 2;

    // First letters of all important attributes
    const char kszAttribFirst[] = "ABCDHINUSabcdhinus";

    // Legal whitespace characters
    const char kszWhiteSpace[] = " \n\r\t";

    // The important attribute strings and their lengths and indices
    const char *   kaszAttrib[13] = {"ACTION", "BACKGROUND", "CLASSID","CODE",
                                     "CODEBASE", "DATA", "DYNSRC", "HREF",
                                     "ISMAP", "NAME", "URL", "USEMAP", "SRC"};

    // Counted by hand - maybe code would be better
    const int  kanAttribLen[13] = {6, 10, 7, 4, 8, 4, 6, 4, 5, 4, 3, 6, 3};
    const int  knAttribAction = 0;
    const int  knAttribBackground = 1;
    const int  knAttribClassID = 2;
    const int  knAttribCode = 3;
    const int  knAttribCodeBase = 4;
    const int  knAttribData = 5;
    const int  knAttribDynSrc = 6;
    const int  knAttribHRef = 7;
    const int  knAttribIsMap = 8;
    const int  knAttribName = 9;
    const int  knAttribURL = 10;
    const int  knAttribUseMap = 11;
    const int  knAttribSrc = 12;

    // The possible tags that match the attributes and their
    // lengths and indices
    const char *   kaszTag[19] = {"A", "APPLET", "AREA", "BASE", "BGSOUND",
                                  "BODY", "EMBED", "FORM", "FRAME", "IFRAME",
                                  "IMG", "INPUT", "ISINDEX", "LINK", "META",
                                  "OBJECT", "TABLE", "TD", "TH"};

    const int  kanTagLen[19] = {1, 6, 4, 4, 7, 4, 5, 4, 5, 6, 3, 5, 7,
                                4, 4, 6, 5, 2, 2}; // Counted by hand
    const int  knTagAnchor = 0;
    const int  knTagApplet = 1;
    const int  knTagArea = 2;
    const int  knTagBase = 3;
    const int  knTagBGSound = 4;
    const int  knTagBody = 5;
    const int  knTagEmbed = 6;
    const int  knTagForm = 7;
    const int  knTagFrame = 8;
    const int  knTagIFrame = 9;
    const int  knTagImg = 10;
    const int  knTagInput = 11;
    const int  knTagIsIndex = 12;
    const int  knTagLink = 13;
    const int  knTagMeta = 14;
    const int  knTagObject = 15;
    const int  knTagTable = 16;
//  const int  knTagTD = 17;
//  const int  knTagTH = 18;

    BYTE  abyAttrib[256];      // Array indicates whether or not a character can start an
                               // important attribute
                               // If 1, the character is the first character of an important attribute
                               // If 2, the character is whitespace
                               // Otherwise, the character is unimportant

    BOOL  bChange = FALSE;     // If TRUE, indicates that we have made a change
    BOOL  bOK;                 // If TRUE, indicates that a legal tag/attribute combination
                               // has been found
    BOOL  bWhiteSpace = FALSE; // If TRUE, the last character we processed was whitespace
    char  c;                   // The input character currently under consideration
    char  cQuote;              // Indicates the current quote-character when parsing attrib values
    char  szURI[80];           // Input URI string. This should be adequate to contain any
                               // URI, (header line limit = 76)
    char  szURLPath[512];      // Used to resolve a URI reference into it's proper filename
    char* pc;                  // Utility pointer
    char* pOut = NULL;         // Pointer to the output buffer to be allocated
    int   i;                   // Utility counter
    int   nLength;             // Length of the input buffer
    int   nMatchAttrib;        // The currently-matched attribute's index (during searching)
    int   nMatchTag;           // The currently-matched tag (during searching)
    int   nMaxBlock = 0;       // Maximum number of characters the output buffer can hold before
                               // the next realloc()
    int   nOutPos = 0;         // Position in the output buffer where we will write the
                               // next character to
    int   nRemLength;          // Remaining number of characters in the input buffer after
                               // the current one


    // We want to clear the pointer in all cases of error
    if (ppszOut)
        *ppszOut = NULL;
    else
        return -1;
    // Make sure we have something to do
    if (!pszIn || !strlen(pszIn))
        return -1;

    // Initialize our "first character" array - this optimization will greatly speed up checking
    memset(abyAttrib, 0, 256);
    for (i = 0; kszAttribFirst[i]; i++)
        abyAttrib[kszAttribFirst[i]] = kbyAttribBit;
    for (i = 0; kszWhiteSpace[i]; i++)
        abyAttrib[kszWhiteSpace[i]] = kbyWhiteSpaceBit;

    // OK, walk over the buffer
    nLength = strlen(pszIn);
    for (int nPos = 0; nPos < nLength; nPos++) {
        c = pszIn[nPos];

        // Check to see if we *might* have an attribute by looking at the character to see if
        // it's the first character of one of our attribute strings. This is a simple lookup
        // instead of 22 tests because of the first character table.

        switch (abyAttrib[c]) {
        case kbyAttribBit: // Attribute character
            // We can test the simple stuff first. For example, all attributes must be preceded
            // by a whitespace character. Also, because the shortest preceding tag string possible
            // is "<TD ", the start index must be at least 4.

            if (bWhiteSpace && nPos >= 4) {
                nMatchAttrib = -1;
                nRemLength = nLength - nPos;

                // Now we have to see if we actually have a legal attribute string. Compare the
                // strings, making sure the remaining buffer can possibly hold the attribute
                // in the first place.

                for (i = 0; i < 13; i++)
                    if ((nRemLength > kanAttribLen[i]) &&
                        !strnicmp(pszIn + nPos, kaszAttrib[i], kanAttribLen[i])) {

                        nMatchAttrib = i;
                        break;
                    }

                // Have we found an attribute tag?
                if (-1 != nMatchAttrib) {

                    // We have a valid attribute string. Search backwards for the last left angle
                    // bracket, which starts the tag.If there is no such angle bracket, our work
                    // has been in vain (we aren't really in an attribute). Note that this is
                    // busted if there is a quoted left-bracket attribute before the correct
                    // left-angle bracket which starts the tag, or if there is a right angle-bracket
                    // intervening, which should nominally close the tag before the attribute. These
                    // problems would be caught by a correct SGML parser.

                    for (int nSearch = nPos - 2; (0 <= nSearch) && ('<' != pszIn[nSearch]); nSearch--)
                        ;

                    if (0 <= nSearch) {
                        // OK, we've found the angle bracket. Now we have to see if it's a tag at
                        // all, and if that tag matches the attribute we've found.

                        nMatchTag = -1;
                        nRemLength = nPos - nSearch - 2;
                        for (i = 0; i < 19; i++)

                            // can the proposed tag fit?
                            if ((nRemLength >= kanTagLen[i]) &&
                                // followed by whitespace?
                                (kbyWhiteSpaceBit == abyAttrib[pszIn[nSearch + kanTagLen[i] + 1]]) &&
                                // does the tag match?
                                !strnicmp(pszIn + nSearch + 1, kaszTag[i], kanTagLen[i] )) {

                                nMatchTag = i;
                                break;
                            }

                        if (-1 != nMatchTag) {
                            bOK = FALSE;

                            // Got the matching tag - let's see if it makes sense in this context
                            switch (nMatchAttrib) {
                            case knAttribAction:
                                if (knTagForm == nMatchTag || knTagIsIndex == nMatchTag) // FORM, ISINDEX
                                    bOK = TRUE;
                                break;

                            case knAttribBackground: // BACKGROUND
                                if (knTagBody == nMatchTag || knTagTable <= nMatchTag) // BODY, TABLE, TD, TH
                                    bOK = TRUE;
                                break;

                            case knAttribClassID: // CLASSID
                            case knAttribData: // DATA
                            case knAttribName: // NAME
                                if (knTagObject == nMatchTag) // OBJECT
                                    bOK = TRUE;
                                break;

                            case knAttribCode: // CODE
                                if (knTagApplet == nMatchTag) // APPLET
                                    bOK = TRUE;
                                break;

                            case knAttribCodeBase: // CODEBASE
                                if (knTagApplet == nMatchTag || knTagObject == nMatchTag) // APPLET, OBJECT
                                    bOK = TRUE;
                                break;

                            case knAttribDynSrc: // DYNSRC
                            case knAttribIsMap:  // ISMAP
                                if (knTagImg == nMatchTag) // IMG
                                    bOK = TRUE;
                                break;

                            case knAttribHRef: // HREF
                                switch (nMatchTag) {
                                case knTagAnchor: // A
                                case knTagArea:   // AREA
                                case knTagBase:   // BASE
                                case knTagLink:   // LINK
                                    bOK = TRUE;
                                    break;
                                }
                                break;

                            case knAttribURL: // URL
                                if (knTagMeta == nMatchTag) // META
                                    bOK = TRUE;
                                break;

                            case knAttribUseMap: // USEMAP
                                if (knTagImg == nMatchTag || knTagObject == nMatchTag) // IMG, OBJECT
                                    bOK = TRUE;
                                break;

                            case knAttribSrc: // SRC
                                switch (nMatchTag) {
                                case knTagBGSound:   // BGSOUND
                                case knTagEmbed:     // EMBED
                                case knTagFrame:     // FRAME
                                case knTagIFrame:    // IFRAME
                                case knTagImg:       // IMG
                                case knTagInput:     // INPUT
                                    bOK = TRUE;
                                    break;
                                }
                                break;
                            }

                            if (bOK) {
                                // We have an attribute-tag pair that makes sense in this context!
                                // Write out the attribute name, advancing the cursor (we know the
                                // input buffer is long enough)

                                for (i = 0; i < kanAttribLen[nMatchAttrib]; i++) {
                                    if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                        return -1;
                                    nPos++;
                                }

                                // Write and skip all leading whitespace
                                while ((nPos < nLength) &&
                                       (kbyWhiteSpaceBit == abyAttrib[pszIn[nPos]])) {

                                    if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                        return -1;
                                    nPos++;
                                }

                                // The current character MUST be an equals sign, or else the
                                // HTML is busted
                                if ((nPos < nLength) && ('=' == pszIn[nPos])) {

                                    // Don't forget to write out the equal sign!
                                    if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                        return -1;
                                    nPos++;

                                    // Write and skip all whitespace between the equals sign and
                                    // the attribute value
                                    while ((nPos < nLength) &&
                                           (kbyWhiteSpaceBit == abyAttrib[pszIn[nPos]])) {

                                        if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                            return -1;
                                        nPos++;
                                    }

                                    // At this point, we should be looking at the first character
                                    // of the attribute value

                                    // Now we check to see if it's quoted
                                    if ('"' == pszIn[nPos] || '\'' == pszIn[nPos] ||
                                        '[' == pszIn[nPos]) {

                                        cQuote = pszIn[nPos];
                                        if ('[' == cQuote)   // Netscape allows brackets as a
                                                             // quote character
                                            cQuote = ']';

                                        if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                            return -1;
                                        nPos++;
                                    }
                                    else
                                        cQuote = 0;

                                    // Now amass the actual URI
                                    memset(szURI, 0, 80);
                                    bOK = FALSE;

                                    if (cQuote) {
                                        // Quoted stuff is actually simpler - the only thing that
                                        // can terminate it is the matching quote
                                        for (i = nPos; (i < nLength) && (i - nPos < 79) &&
                                                 (pszIn[i] != cQuote); i++) {

                                            szURI[i - nPos] = pszIn[i];
                                        }

                                        // Hokay, now we should be looking at the matching quote
                                        if ((i < nLength) && (i - nPos < 79) && (pszIn[i] == cQuote))
                                            bOK = TRUE; // We are, so things can continue
                                    }
                                    else {
                                        // We can be terminated by any whitespace or a right
                                        // angle-bracket
                                        for (i = nPos; (i < nLength) && (i - nPos < 79) &&
                                                 (kbyWhiteSpaceBit != abyAttrib[pszIn[i]]) &&
                                                 ('>' != pszIn[i]); i++) {

                                            szURI[i - nPos] = pszIn[i];
                                        }

                                        // We must now be looking at a legal terminator
                                        if ((i < nLength) && (('>' == pszIn[i]) ||
                                                    (kbyWhiteSpaceBit == abyAttrib[pszIn[i]]))) {

                                            bOK = TRUE; // We are, so things can continue
                                        }
                                    }

                                    if (bOK) {
										ParseURLCallback pPUC = (pFn == 0) ? stub_callback : pFn;

                                        // Still chugging along... now attempt to resolve the URI
                                        if (pPUC(szURI, userRef, szURLPath, 512) &&
                                            stricmp(szURI, szURLPath)) {

                                            // We've actually transformed one URI into another - this
                                            // is a real change!
                                            if ( cQuote != '"' )
                                                if (!WriteCharToBlock(&pOut, '"', nMaxBlock, nOutPos))
                                                    return -1;
                                            for (pc = szURLPath; *pc; pc++)
                                                if (!WriteCharToBlock(&pOut, *pc, nMaxBlock, nOutPos))
                                                    return -1;
                                            if ( cQuote != '"' )
                                                if (!WriteCharToBlock(&pOut, '"', nMaxBlock, nOutPos))
                                                    return -1;
                                            bChange = TRUE;

                                            // We've changed a URI - now sync up the input buffer
                                            nPos = i;   // This sets the input cursor to the
                                                        // terminating character of the attribute

                                            // Now this will fall through and write the terminator
                                            // (which might be a quote character)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            bWhiteSpace = FALSE;
            break;

        case kbyWhiteSpaceBit: // Whitespace character
            bWhiteSpace = TRUE;
            break;

        default: // Nothing special
            bWhiteSpace = FALSE;
            break;
        }

        // OK, write out the character before advancing the counter. Note that a lot of
        // fall-throughs end up here, where they are written like normal characters
        if ((nPos < nLength) && !WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
            return -1;
    }

    // And return our result
    *ppszOut = pOut;
    return (bChange ? (nOutPos + 1) : 0);
}


BOOL WriteCharToBlock(char ** ppOut, char c, int & nMaxBlock, int & nPos) {
    const int   cnBlockSize = 1024;  // The size of a block to reallocate. This is done to
                                     // avoid thousands of realloc() calls.

    if (nPos >= nMaxBlock) {
        nMaxBlock = ((nPos / cnBlockSize) + 1) * cnBlockSize;
        *ppOut = (char *)realloc(*ppOut, nMaxBlock);

        if (!(*ppOut))
            return FALSE;

        memset((*ppOut) + nPos, 0, nMaxBlock - nPos);
    }

    *((*ppOut) + nPos) = c;
    nPos++;

    return TRUE;
}


int MungeURLs( char* pszIn, char* base, char** ppszOut )
{
	return ParseURLs( pszIn, munge_url_callback, (long)base, ppszOut );
}

int munge_url_callback( char* in, long ref, char* out, int maxOut )
{
	return ResolveURL( in, (char*)ref, out, maxOut );
}


int ResolveURL( char* uri, char* base, char* url, int /*maxUrl*/ )
{
//    puts( uri );

    bool resolve = true;
    char* p = uri;

    for ( int i = 0; p != 0 && i < 8; i++, p++ ) {
        if ( *p == ':' ) {
            resolve = false;
            break;
        }
    }

    if ( resolve ) {
        strcpy( url, base );

        if ( !(*uri == '/') )
            strcat( url, "/" );

        strcat( url, uri );
        return TRUE;
    }

    return FALSE;
}


// just so ParseURLs always has something to call
int stub_callback( char* inURL, long, char*, int )
{
	puts( inURL );
	return 0;
}

#ifdef _TESTIES_

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"


// provide html file on command line. all relative URIs will be converted
// to full URLs using base href, which is currently just hard-coded.
int main( int argc, char* argv[] )
{
    // get a buffer big enough to hold the entire file
    int fd = open( argv[1], O_RDONLY );
    long nBytes = lseek( fd, 0, SEEK_END );
    lseek( fd, 0, SEEK_SET );
    char* pbuf = (char*) malloc( nBytes );

    // suck it up
    read( fd, pbuf, nBytes );

    // convert relative URIs into URLs
    char* out = 0;
    MungeURLs( pbuf, "http://adserver.eudora.com", &out );

    // dump the result
    puts( out );

    // clean up and go home
    free( out );
    free( pbuf );
    close( fd );

    return 0;
}

#endif
