// CONVHTML.CPP
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

/*====================================================================*/
/* CONVHTML.CPP - HTML Conversion Routine                             */
/* --------------------------------------                             */
/* Version 1.0 Rev 7/17/97                                            */
/* Author V. Leamons                                                  */
/*                                                                    */
/* Description:                                                       */
/*    This file contains the ConvertURIs() routine to be included in   */
/* Eudora 4.0. This routine converts an HTML block containing CIDs to */
/* it's equivalent using file: references only.                       */
/*                                                                    */
/* History (all by V. Leamons unless otherwise marked):               */
/* 7/14/97  Created.                                                  */
/* 7/17/97  Completed, ready for testing.                             */
/*====================================================================*/

#include "stdafx.h"

// Include the files in the correct order to allow leak checking with malloc.
// CRTDBG_MAP_ALLOC already defined in stdafx.h
#include <stdlib.h>
#include <crtdbg.h>

#include <malloc.h>
#include <string.h>

#include <QCUtils.h>

#include "convhtml.h"
#include "resource.h"
#include "fileutil.h"
#include "rs.h"
#include "mime.h"
#include "msgutils.h"
#include "PgEmbeddedImage.h"
#include "utils.h"
#include "guiutils.h"


#include "DebugNewHelpers.h"


// *** Local Prototypes ***
BOOL WriteCharToBlock(char ** ppOut, char c, int & nMaxBlock, int & nPos);

/*===============================================================================================================*/
/* ConvertURIs() - Scan an input HTML character block, replacing embedded URIs with the proper file: references. */
/* 7/14/97  Created.                                                                                             */
/* 7/17/97  Completed.                                                                                           */
/*                                                                                                               */
/* Parameters:                                                                                                   */
/*    pszIn    Input HTML zero terminated string.                                                                */
/*    ppszOut  Output HTML zero terminated string. Allocated with malloc() by ConvertURIs(), should be freed     */
/*             with free(). This means it doesn't require C++, as new/delete would.                              */
/*    uriMap   Input URIMap that is already built.                                                               */
/*                                                                                                               */
/* Returns:                                                                                                      */
/*    Length of the returned HTML block (NOT including the terminating zero, i.e. the result of lstrlen()).      */
/* Returns zero if no changes were made to the input block (in which case lstrlen() will give the block's length */
/* and -1 in case of error.                                                                                      */
/*                                                                                                               */
/* Remarks:                                                                                                      */
/*    This routine is just a heuristic - no interest was expressed in an algorithmic treatment of the problem,   */
/* so there will be cases where it will fail (PLAINTEXT and XMP tags, for example).                              */
/*===============================================================================================================*/
int ConvertURIs(const char * pszIn, char ** ppszOut, const URIMap & uriMap)
{
   static const BYTE     kbyAttribBit = 1;                         // Bit set indicates that an attribute beginning is possible
   static const BYTE     kbyWhiteSpaceBit = 2;                     // Bit set indicates a character is whitespace
   static const char     kszAttribFirst[] = "ABCDHINUSabcdhinus";  // First letters of all important attributes
   static const char     kszWhiteSpace[] = " \n\r\t";              // Legal whitespace characters

   // The important attribute strings and their lengths and indices
   static const char *   kaszAttrib[13] = {"ACTION", "BACKGROUND", "CLASSID", "CODE", "CODEBASE", "DATA", "DYNSRC", "HREF",
                                           "ISMAP", "NAME", "URL", "USEMAP", "SRC"};
   static const int      kanAttribLen[13] = {6, 10, 7, 4, 8, 4, 6, 4, 5, 4, 3, 6, 3}; // Counted by hand - maybe code would be better
   static const int      knAttribAction = 0;
   static const int      knAttribBackground = 1;
   static const int      knAttribClassID = 2;
   static const int      knAttribCode = 3;
   static const int      knAttribCodeBase = 4;
   static const int      knAttribData = 5;
   static const int      knAttribDynSrc = 6;
   static const int      knAttribHRef = 7;
   static const int      knAttribIsMap = 8;
   static const int      knAttribName = 9;
   static const int      knAttribURL = 10;
   static const int      knAttribUseMap = 11;
   static const int      knAttribSrc = 12;

   // The possible tags that match the attributes and their lengths and indices
   static const char *   kaszTag[19] = {"A", "APPLET", "AREA", "BASE", "BGSOUND", "BODY", "EMBED", "FORM", "FRAME", "IFRAME",
                                        "IMG", "INPUT", "ISINDEX", "LINK", "META", "OBJECT", "TABLE", "TD", "TH"};
   static const int      kanTagLen[19] = {1, 6, 4, 4, 7, 4, 5, 4, 5, 6, 3, 5, 7, 4, 4, 6, 5, 2, 2}; // Counted by hand
   static const int      knTagAnchor = 0;
   static const int      knTagApplet = 1;
   static const int      knTagArea = 2;
   static const int      knTagBase = 3;
   static const int      knTagBGSound = 4;
   static const int      knTagBody = 5;
   static const int      knTagEmbed = 6;
   static const int      knTagForm = 7;
   static const int      knTagFrame = 8;
   static const int      knTagIFrame = 9;
   static const int      knTagImg = 10;
   static const int      knTagInput = 11;
   static const int      knTagIsIndex = 12;
   static const int      knTagLink = 13;
   static const int      knTagMeta = 14;
   static const int      knTagObject = 15;
   static const int      knTagTable = 16;
// const int      knTagTD = 17;
// const int      knTagTH = 18;

   BYTE           abyAttrib[256];      // Array indicates whether or not a character can start an important attribute
                                       // If 1, the character is the first character of an important attribute
                                       // If 2, the character is whitespace
                                       // Otherwise, the character is unimportant
   BOOL           bChange = FALSE;     // If TRUE, indicates that we have made a change
   BOOL           bOK;                 // If TRUE, indicates that a legal tag/attribute combination has been found
   BOOL           bWhiteSpace = FALSE; // If TRUE, the last character we processed was whitespace
   char           c;                   // The input character currently under consideration
   char           cQuote;              // Indicates the current quote-character when parsing attrib values
   char           szURI[80];           // Input URI string. This should be adequate to contain any URI, (header line limit = 76)
   char           szURLPath[512];      // Used to resolve a URI reference into it's proper filename
   char *         pc;                  // Utility pointer
   char *         pOut = NULL;         // Pointer to the output buffer to be allocated
   int            i;                   // Utility counter
   int            nLength;             // Length of the input buffer
   int            nMatchAttrib;        // The currently-matched attribute's index (during searching)
   int            nMatchTag;           // The currently-matched tag (during searching)
   int            nMaxBlock = 0;       // Maximum number of characters the output buffer can hold before the next realloc()
   int            nOutPos = 0;         // Position in the output buffer where we will write the next character to
   int            nRemLength;          // Remaining number of characters in the input buffer after the current one


   // We want to clear the pointer in all cases of error
   if (ppszOut)
      *ppszOut = NULL;
   else
      return -1;
   // Make sure we have something to do
   if (!pszIn || !lstrlen(pszIn))
      return -1;

   // Initialize our "first character" array - this optimization will greatly speed up checking
   memset(abyAttrib, 0, 256);
   for (i = 0; kszAttribFirst[i]; i++)
      abyAttrib[kszAttribFirst[i]] = kbyAttribBit;
   for (i = 0; kszWhiteSpace[i]; i++)
      abyAttrib[kszWhiteSpace[i]] = kbyWhiteSpaceBit;

   // OK, walk over the buffer
   nLength = lstrlen(pszIn);
   for (int nPos = 0; nPos < nLength; nPos++) {
      c = pszIn[nPos];

      // Check to see if we *might* have an attribute by looking at the character to see if it's the first character of
      // one of our attribute strings. This is a simple lookup instead of 22 tests because of the first character table.
      switch (abyAttrib[c]) {
         case kbyAttribBit: // Attribute character
            // We can test the simple stuff first. For example, all attributes must be preceded by a whitespace character.
            // Also, because the shortest preceding tag string possible is "<TD ", the start index must be at least 4.
            if (bWhiteSpace && nPos >= 4) {
				nMatchAttrib = -1;
               nRemLength = nLength - nPos;

               // Now we have to see if we actually have a legal attribute string. Compare the strings, making sure the
               // remaining buffer can possibly hold the attribute in the first place.
               for (i = 0; i < 13; i++)
                  if ((nRemLength > kanAttribLen[i]) && !strnicmp(pszIn + nPos, kaszAttrib[i], kanAttribLen[i])) {
                     nMatchAttrib = i;
                     break;
                  }

               // Have we found an attribute tag?
               if (-1 != nMatchAttrib) {
                  // We have a valid attribute string. Search backwards for the last left angle bracket, which starts the tag.
                  // If there is no such angle bracket, our work has been in vain (we aren't really in an attribute). Note that
                  // this is busted if there is a quoted left-bracket attribute before the correct left-angle bracket which
                  // starts the tag, or if there is a right angle-bracket intervening, which should nominally close the tag
                  // before the attribute. These problems would be caught by a correct SGML parser.
                  for (int nSearch = nPos - 2; (0 <= nSearch) && ('<' != pszIn[nSearch]); nSearch--)
                     ;

               if (0 <= nSearch) {
					  // OK, we've found the angle bracket. Now we have to see if it's a tag at all, and if that tag matches
                     // the attribute we've found.
                     nMatchTag = -1;
                     nRemLength = nPos - nSearch - 2;
                     for (i = 0; i < 19; i++)
                        if ((nRemLength >= kanTagLen[i]) &&                                        // can the proposed tag fit?
                            (kbyWhiteSpaceBit == abyAttrib[pszIn[nSearch + kanTagLen[i] + 1]]) &&  // followed by whitespace?
                            !strnicmp(pszIn + nSearch + 1, kaszTag[i], kanTagLen[i] )) {            // does the tag match?
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

                           // Write out the attribute name, advancing the cursor (we know the input buffer is long enough)
                           for (i = 0; i < kanAttribLen[nMatchAttrib]; i++) {
                              if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                 return -1;
                              nPos++;
                           }

                           // Write and skip all leading whitespace
                           while ((nPos < nLength) && (kbyWhiteSpaceBit == abyAttrib[pszIn[nPos]])) {
                              if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                 return -1;
                              nPos++;
                           }

                           // The current character MUST be an equals sign, or else the HTML is busted
                           if ((nPos < nLength) && ('=' == pszIn[nPos])) {

                              // Don't forget to write out the equal sign!
                              if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                 return -1;
                              nPos++;

                              // Write and skip all whitespace between the equals sign and the attribute value
                              while ((nPos < nLength) && (kbyWhiteSpaceBit == abyAttrib[pszIn[nPos]])) {
                                 if (!WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
                                    return -1;
                                 nPos++;
                              }

                              // At this point, we should be looking at the first character of the attribute value
                              // Now we check to see if it's quoted
                              if ('"' == pszIn[nPos] || '\'' == pszIn[nPos] || '[' == pszIn[nPos]) {
                                 cQuote = pszIn[nPos];
                                 if ('[' == cQuote)   // Netscape allows brackets as a quote character
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
                                 // Quoted stuff is actually simpler - the only thing that can terminate it is the matching quote
                                 for (i = nPos; (i < nLength) && (i - nPos < 79) && (pszIn[i] != cQuote); i++)
                                    szURI[i - nPos] = pszIn[i];

                                 // Hokay, now we should be looking at the matching quote
                                 if ((i < nLength) && (i - nPos < 79) && (pszIn[i] == cQuote))
                                    bOK = TRUE; // We are, so things can continue
                              }
                              else {
                                 // We can be terminated by any whitespace or a right angle-bracket
                                 for (i = nPos; (i < nLength) && (i - nPos < 79) &&
                                      (kbyWhiteSpaceBit != abyAttrib[pszIn[i]]) && ('>' != pszIn[i]); i++)
                                    szURI[i - nPos] = pszIn[i];

                                 // We must now be looking at a legal terminator
                                 if ((i < nLength) && (('>' == pszIn[i]) || (kbyWhiteSpaceBit == abyAttrib[pszIn[i]])))
                                    bOK = TRUE; // We are, so things can continue
                              }

                              if (bOK) {
                                 // Still chugging along... now attempt to resolve the URI
                                 if (uriMap.ResolveURIToFileURL(szURI, szURLPath, 512) && lstrcmpi(szURI, szURLPath)) {
                                    // We've actually transformed one URI into another - this is a real change!
                                    // Allow the transformation if the szURLPath is not a dangerous file type,
                                    // simply skip/remove the URI if it is a dangerous file type.
                                    if ( !GetIniShort(IDS_INI_PREVENT_EXECUTABLE_EMBEDED_CONTENT) ||
                                         !IsProgram( szURLPath,
                                                     IDS_INI_PREVENT_EXECUTABLE_EMBEDED_CONTENT_ADD,
                                                     IDS_INI_PREVENT_EXECUTABLE_EMBEDED_CONTENT_SUBTRACT ) )
                                    {
										if ( cQuote != '"' )
											if (!WriteCharToBlock(&pOut, '"', nMaxBlock, nOutPos))
												return -1;
										for (pc = szURLPath; *pc; pc++)
										   if (!WriteCharToBlock(&pOut, *pc, nMaxBlock, nOutPos))
											  return -1;
										if ( cQuote != '"' )
											if (!WriteCharToBlock(&pOut, '"', nMaxBlock, nOutPos))
												return -1;
                                    }
                                    bChange = TRUE;

                                    // We've changed a URI - now sync up the input buffer
                                    nPos = i;   // This sets the input cursor to the terminating character of the attribute

                                    // Now this will fall through and write the terminator (which might be a quote character)
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

      // OK, write out the character before advancing the counter
      // Note that a lot of fall-throughs end up here, where they are written like normal characters
      if ((nPos < nLength) && !WriteCharToBlock(&pOut, pszIn[nPos], nMaxBlock, nOutPos))
         return -1;
   }

   // And return our result
   *ppszOut = pOut;
   return (bChange ? (nOutPos + 1) : 0);
} // End ConvertURIs()


/*=====================================================================================================================*/
/* WriteCharToBlock() - Helper routine for ConvertURIs() - Write a character and possibly reallocate the output buffer  */
/* 7/14/97  Created.                                                                                                   */
/*                                                                                                                     */
/* Parameters:                                                                                                         */
/*    ppOut       Address of the buffer pointer that may be reallocated.                                               */
/*    c           Character to write to output.                                                                        */
/*    nMaxBlock   Current maximum buffer size - may be modified by the routine.                                        */
/*    nPos        Current cursor location in the buffer.                                                               */
/*                                                                                                                     */
/* Returns:                                                                                                            */
/*    TRUE if the write succeeded, FALSE otherwise. Failure is possible only if reallocation is required and fails.    */
/*                                                                                                                     */
/* Remarks:                                                                                                            */
/*    Possibly could be improved to speed up ConvertURIs() by enabling the writing of blocks larger than one character. */
/* However, since probably 99.99% of all ConvertURIs() calls to this routine are single character writes, the speedup   */
/* might be insignificant.                                                                                             */
/*=====================================================================================================================*/
BOOL WriteCharToBlock(char ** ppOut, char c, int & nMaxBlock, int & nPos) {
   static const int   cnBlockSize = 1024;  // The size of a block to reallocate. This is done to avoid thousands of realloc() calls.

   if (nPos >= nMaxBlock) {
      nMaxBlock = ((nPos / cnBlockSize) + 1) * cnBlockSize;
      char *    pOut = (char *)realloc(*ppOut, nMaxBlock);
      if (!pOut) {
          free(*ppOut);
          *ppOut = NULL;
          return FALSE;
      }
      
      memset(pOut + nPos, 0, nMaxBlock - nPos);
      *ppOut = pOut;
   }
   *((*ppOut) + nPos) = c;
   nPos++;

   return TRUE;
} // End WriteCharToBlock()

URIMap::URIMap()
{
	m_EmbeddedElements.RemoveAll();
}

URIMap::~URIMap()
{
	FreeURIMap();
}

// pMessage must include the "Embedded content" lines
int URIMap::BuildURIMap( const char * pMessage, bool bFreePrevious )
{
	// look for "Embedded content" lines and X-EmbeddedContent headers
	CRString ECHeader(IDS_HEADER_EMBEDDED_CONTENT);
	CString ECTrailer("\r\n" + CRString(IDS_EMBEDDED_CONTENT));
	int		found = 0;
	char * pIt;

	if (bFreePrevious)
	{
		// in case we've already built one...
		FreeURIMap();
	}

	// first look for X-EmbeddedContent headers
	const char* pTmp = pMessage;
	const char* pBody = FindBody(pTmp);
	while ( pIt = strstr( pTmp, ECHeader ) )
	{
		// Skip by X-EmbeddedContent headers found in the body
		if (pIt >= pBody)
			break;

		BOOL	bMore;
		do
		{
			bMore = FALSE;	// assume there's no more

			// verify and parse the parameters
			// expects "X-EmbeddedContent: <cid=path>"
			char * pCID = NULL;
			char * pEndCID = NULL;
			char * pPath = NULL;
			char * pEndPath = NULL;

			pCID = strchr( pIt, '<' );
			if ( pCID )
			{
				pCID++;						// skip the '<'
				pEndCID = strchr( pCID, '=' );
				if ( pEndCID )
				{
					pPath = pEndCID + 1;	// skip '=' delimiter
					pEndPath = strchr( pPath, '>' );
				}
			}

			if ( pPath && pCID )
			{
				// add the hashes to the URI Map
				EmbeddedElement* EE = DEBUG_NEW_MFCOBJ_NOTHROW EmbeddedElement();
				if ( EE )
				{
					// init the EE
					char tmp = *pEndCID;
					*pEndCID = '\0';
					EE->SetCID( pCID );

					// Skip past the initial "cid:" so that we can check to see if
					// the CID indicates that the embedded image is an emoticon.
					if (strnicmp(pCID, "cid:", strlen("cid:")) == 0)
						pCID += strlen("cid:");
					EE->SetIsEmoticon(strnicmp(pCID, "emoticon", strlen("emoticon")) == 0);

					// Restore the character we NULL'ed
					*pEndCID = tmp;

					tmp = *pEndPath;
					*pEndPath = '\0';
					EE->SetPath( pPath );
					*pEndPath = tmp;

					EE->SetMRI( 0 );
					EE->SetCLHash( 0 );
					EE->SetMRI_CB_CL_Hash( 0 );
			
					if ( m_EmbeddedElements.AddTail(EE) )
						found++;
				}

				// if the next line starts with "  <" then we have another one
				char * pEnd = strchr( pCID, '\n' );
				if ( pEnd && ( strncmp( pEnd, "\n  <", 4 ) == 0 ) )
				{
					// we've got more...
					pIt = pEnd;
					bMore = TRUE;
				}
			}
		}
		while ( bMore );

		// look for another
		pTmp = pIt + 1;
	}

	// now look for "Embedded content" trailers
	pTmp = pMessage;
	while ( pIt = strstr( pTmp, ECTrailer ) )
	{
		// verify and parse the parameters
		// expects "Embedded content \path %x,%x,%x,%x\"
		char * pPath = NULL;
		char * pEndPath = NULL;
		char * pMRI = NULL;
		char * pCIDHash = NULL;
		char * pCLHash = NULL;
		char * pMRI_CB_CL_Hash = NULL;
		DWORD  MRI, CIDHash, CLHash, MRI_CB_CL_Hash;

		pPath = pIt + strlen(ECTrailer);
		pEndPath = pMRI = strchr( pPath, ':' );
		if ( pMRI )
		{
			pMRI++;			// skip the delimiter
			pCIDHash = strchr( pMRI, ',' );
			if ( pCIDHash )
			{
				pCIDHash++;	// skip delimiter
				pCLHash = strchr( pCIDHash, ',' );
				if ( pCLHash )
				{
					pCLHash++;
					pMRI_CB_CL_Hash = strchr( pCLHash, ',' );
					if ( pMRI_CB_CL_Hash )
						pMRI_CB_CL_Hash++;	// skip delimiter
				}
			}
		}

		if ( pPath && pMRI && pCIDHash && pCLHash && pMRI_CB_CL_Hash )
		{
			// add the hashes to the URI Map
			if (sscanf(pMRI, "%lx,%lx,%lx,%lx", &MRI, &CIDHash, &CLHash, &MRI_CB_CL_Hash) == 4)
			{
				EmbeddedElement* EE = DEBUG_NEW_MFCOBJ_NOTHROW EmbeddedElement();
				if ( EE )
				{
					// init the EE
					char tmp = * pEndPath;
					*pEndPath = '\0';
					EE->SetPath( pPath );
					*pEndPath = tmp;

					EE->SetMRI( MRI );
					EE->SetCIDHash( CIDHash );
					EE->SetCLHash( CLHash );
					EE->SetMRI_CB_CL_Hash( MRI_CB_CL_Hash );
				
					if ( m_EmbeddedElements.AddTail(EE) )
						found++;
				}
			}
		}

		// look for another
		pTmp = pIt + 1;
	}

	return found;
}

void URIMap::ResolvePathToFullPath(
	CString &		szPath) const
{
	// BOG: here's my lame hack for telling if a path is actually
	// a full win32 path. (relative paths start without backslash.)
	if ( (szPath.GetLength() > 1) && (szPath[0] != '\\') && (szPath[1] != ':') ) 
	{
		CRString	szEmbeddedDir(IDS_EMBEDDED_FOLDER);

		// must prepend <eudoradir>\Embedded\ ...
		// EudoraDir contains the \\; EmbeddedDir doesn't
		CString		szFullPathStart = EudoraDir + szEmbeddedDir;

		szFullPathStart += "\\";

		szPath.Insert(0, szFullPathStart);
	}
}

int URIMap::ResolveURIToPath( const char * pszInURI, char * pszOutFilename, int nBufSize, bool bValidateFile ) const
{
	*pszOutFilename = '\0';
	long lHash = 0;
	BOOL bIsCID = FALSE;

	if (strnicmp(pszInURI, "cid:", 4) == 0)
	{
		bIsCID = TRUE;
		lHash = ::HashMT(pszInURI + 4);
	}

	// look throught the EmbeddedElements list for a match...
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if ( EE )
		{
			BOOL bIsMatch = FALSE;
			if (bIsCID)
			{
				if (lHash == (long)EE->GetCIDHash())
					bIsMatch = TRUE;
			}
			else
			{
				CString MRIPlusURI;

				MRIPlusURI.Format("%d", EE->GetMRI());
				MRIPlusURI += pszInURI;
				lHash = ::HashMT(MRIPlusURI);
				if (lHash == (long)EE->GetMRI_CB_CL_Hash())
					bIsMatch = TRUE;
			}

			if (bIsMatch)
			{
				CString		szPath = EE->GetPath();
				
				ResolvePathToFullPath(szPath);

				//	Make sure that we have space for the path and that it looks like
				//	we can handle the image file. If either fails don't copy the
				//	path and don't mark it referenced. By not marking it referenced
				//	we ensure that we'll display it instead as an attachment.
				//	One example of where this is needed is PDF files sent inline
				//	from Mac Eudora.
				//
				//	Note that since we have no way to validate whether or not MSHTML
				//	will recognize the image, we make the assumption that MSHTML
				//	will handle everything that we can handle (either internally
				//	or via QuickTime).
				if ( (static_cast<int>(szPath.GetLength() + 1) < nBufSize) &&
					 (!bValidateFile || (PgValidateImageFile(szPath, FALSE) == 0)) )
				{
					strcpy(pszOutFilename, szPath);
					EE->SetWasReferenced(true);
				}

				return strlen( pszOutFilename );
			}
		}
	}

	// must not have found a match...
	return 0;
}

int URIMap::ResolveURIToFileURL( const char * pszInURI, char * pszOutFilename, int nBufSize ) const
{
	static const int	nFileURLPrefixLength = strlen("file://");
	int					nBufSizeNoFileURLPrefix = nBufSize-nFileURLPrefixLength;
	CString				sFileURL;

	int		nStringLength = ResolveURIToPath(pszInURI, sFileURL.GetBuffer(nBufSizeNoFileURLPrefix), nBufSizeNoFileURLPrefix);

	if ( (nStringLength > 0) && ((nStringLength + nFileURLPrefixLength) < nBufSize) )
	{
		strcpy(pszOutFilename, "file://");
		strcat(pszOutFilename, sFileURL);
		nStringLength += nFileURLPrefixLength;
	}

	return nStringLength;
}


void URIMap::AddOrphanedURIsToBodyAsAttachments(
	CString &		szBody)
{
	//	"Embedded content" prefix
	CRString		szECPrefix(IDS_EMBEDDED_CONTENT);
	
	//	Look throught the EmbeddedElements list for any unreferenced URIs
	POSITION		pos = m_EmbeddedElements.GetHeadPosition();

	while (pos)
	{
		EmbeddedElement *	pEE = reinterpret_cast<EmbeddedElement *>( m_EmbeddedElements.GetNext(pos) );

		if ( pEE && !pEE->WasReferenced() )
		{
			CString		szPath = pEE->GetPath();

			ResolvePathToFullPath(szPath);

			AddAttachmentToBody(szBody, szPath, szECPrefix);
		}
	}
}

void URIMap::FreeURIMap()
{
	// clean the list
	while (m_EmbeddedElements.IsEmpty() == FALSE)
		delete m_EmbeddedElements.RemoveHead();
}

//	Remove members from ourselves and add them to the dest URIMap.
void URIMap::TransferMembers(URIMap & dest)
{
	while ( !m_EmbeddedElements.IsEmpty() )
		dest.m_EmbeddedElements.AddTail( m_EmbeddedElements.RemoveHead() );
}

void URIMap::NukeCIDs()
{
	// Nuke all the CIDs so Paige will think it needs to create new parts
	// for all embedded content
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if ( EE )
		{
			DWORD CIDHash;
			CString CID = EE->GetCID();

			if ( CID && ( CID.GetLength() > 4 ) )
			{
				const char * pCID = CID;
				CIDHash = ::HashMT( &pCID[4] );
				EE->SetCID( "" );
				EE->SetCIDHash( CIDHash );
			}
		}
	}
}

BOOL URIMap::IsRealCID( const char * TargetCID )
{
	// look for the TargetCID in string form
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if ( EE )
		{
			CString& CID = EE->GetCID();

			if ( CID.GetLength() > 4 )
			{
				if ( strcmpi( TargetCID, CID ) == 0 )
					return TRUE;
			}
		}
	}

	return FALSE;	// CID in string form not found...
}


// used when creating messages - generates a CID based on a messageid
int
URIMap::AddCIDMap(
	const char *			pInMessageId,
	const char *			pInPath, 
	char *					pszOutCID,
	int						nCIDSize,
	bool					bIsEmoticon)
{
	// gen up a unique CID
	CString CID;
	int seq = m_EmbeddedElements.GetCount();
	while ( 1 )
	{
		// BOG: hacked to 'prepend' "cid:"
		if (bIsEmoticon)
		{
			// GCW: further hacked to prepend emoticon- so that we can identify
			// the CIDs of emoticons.
			CID.Format( "cid:emoticon-%s.%d", pInMessageId, seq++ );
		}
		else
		{
			CID.Format( "cid:%s.%d", pInMessageId, seq++ );
		}

		char filename[ _MAX_PATH + 1 ];
		if ( ! URIMap::ResolveURIToPath( CID, filename, sizeof( filename )) )
			break;
	}

	// build up the EmbeddedElement
	EmbeddedElement* EE = DEBUG_NEW_MFCOBJ_NOTHROW EmbeddedElement();
	if ( EE )
	{
		// init the EE
		EE->SetIsEmoticon(bIsEmoticon);
		EE->SetPath( pInPath );
		EE->SetCID( CID );
			
		// add it to the list
		m_EmbeddedElements.AddTail(EE);
	}

	// return the CID
	strncpy( pszOutCID, CID, nCIDSize );

	return strlen( CID );
}

void URIMap::RemoveCIDMap( const char* uri )
{
	// look throught the EmbeddedElements list for a match...
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	POSITION prev;

	while ( pos ) {
		prev = pos;
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if ( EE ) {
			long lHash;

			// is the URI a CID?
			if ( strnicmp( uri, "cid:", 4 ) == 0 ) {
				lHash = ::HashMT( &uri[ 4 ] );

				if ( lHash == (long)EE->GetCIDHash() ) {
					m_EmbeddedElements.RemoveAt( prev );
					delete EE;
				}
			}
			else {
				// check the Content-Base / Content-Location stuff
				// KCM - wazoo magic goes in here...
				lHash = ::HashMT( uri );
			}
		}
	}
}

// used when creating messages
void URIMap::BuildEmbeddedObjectHeaders(CString & Header, bool bIncludeEmoticons)
{
	BOOL bFirst = TRUE;

	Header.Empty();

	// build up a header based on the URIMap (m_theMap)
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		CString& CID = EE->GetCID();
		if ( CID.GetLength() && (bIncludeEmoticons || !EE->IsEmoticon()) )
		{
			char buf[ 1024 ];

			if ( bFirst )
			{
				CRString ECHeader(IDS_HEADER_EMBEDDED_CONTENT);

				sprintf( buf, "%s <%s=%s>", (const char*)ECHeader, (const char *)EE->GetCID(), (const char *)EE->GetPath() );
				bFirst = FALSE;
			}
			else
			{
				sprintf( buf, "\n  <%s=%s>", (const char *)EE->GetCID(), (const char *)EE->GetPath() );
			}

			Header += buf;
		}
	}

	return;

}

void URIMap::GetEmbeddedObjectPaths( CString & Paths )
{
	char EmbeddedDir[_MAX_PATH + 1];
	wsprintf(EmbeddedDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_EMBEDDED_FOLDER));

	Paths.Empty();

	// build up a header based on the URIMap (m_theMap)
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		CString& path = EE->GetPath();
		if ( path && path.GetLength() )
		{
			char buf[_MAX_PATH + 1];

			if ( ! strchr( path, '\\' ) )
				sprintf( buf, "%s\\%s\n", EmbeddedDir, (const char *)path );
			else
				sprintf( buf, "%s\n", (const char *)path );

			Paths += buf;
		}
	}

	return;
}
