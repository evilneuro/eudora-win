/*====================================================================*/
/* CONVHTML.CPP - HTML Conversion Routine                             */
/* --------------------------------------                             */
/* Version 1.0 Rev 7/17/97                                            */
/* Author V. Leamons for Qualcomm, Inc. All Rights Reserved.          */
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

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include <QCUtils.h>

#include "convhtml.h"
#include "resource.h"
#include "fileutil.h"
#include "rs.h"
#include "mime.h"
#include "utils.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// *** Local Prototypes ***
BOOL WriteCharToBlock(char ** ppOut, char c, int & nMaxBlock, int & nPos);

/*===============================================================================================================*/
/* ConvertURIs() - Scan an input HTML character block, replacing embedded URIs with the proper file: references.  */
/* 7/14/97  Created.                                                                                             */
/* 7/17/97  Completed.                                                                                           */
/*                                                                                                               */
/* Parameters:                                                                                                   */
/*    pszIn    Input HTML zero terminated string.                                                                */
/*    ppszOut  Output HTML zero terminated string. Allocated with malloc() by ConvertURIs(), should be freed      */
/*             with free(). This means it doesn't require C++, as new/delete would.                              */
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
int ConvertURIs(char * pszIn, char ** ppszOut) {
   const BYTE     kbyAttribBit = 1;                         // Bit set indicates that an attribute beginning is possible
   const BYTE     kbyWhiteSpaceBit = 2;                     // Bit set indicates a character is whitespace
   const char     kszAttribFirst[] = "ABCDHINUSabcdhinus";  // First letters of all important attributes
   const char     kszWhiteSpace[] = " \n\r\t";              // Legal whitespace characters

   // The important attribute strings and their lengths and indices
   const char *   kaszAttrib[13] = {"ACTION", "BACKGROUND", "CLASSID", "CODE", "CODEBASE", "DATA", "DYNSRC", "HREF",
                                    "ISMAP", "NAME", "URL", "USEMAP", "SRC"};
   const int      kanAttribLen[13] = {6, 10, 7, 4, 8, 4, 6, 4, 5, 4, 3, 6, 3}; // Counted by hand - maybe code would be better
   const int      knAttribAction = 0;
   const int      knAttribBackground = 1;
   const int      knAttribClassID = 2;
   const int      knAttribCode = 3;
   const int      knAttribCodeBase = 4;
   const int      knAttribData = 5;
   const int      knAttribDynSrc = 6;
   const int      knAttribHRef = 7;
   const int      knAttribIsMap = 8;
   const int      knAttribName = 9;
   const int      knAttribURL = 10;
   const int      knAttribUseMap = 11;
   const int      knAttribSrc = 12;

   // The possible tags that match the attributes and their lengths and indices
   const char *   kaszTag[19] = {"A", "APPLET", "AREA", "BASE", "BGSOUND", "BODY", "EMBED", "FORM", "FRAME", "IFRAME",
                                 "IMG", "INPUT", "ISINDEX", "LINK", "META", "OBJECT", "TABLE", "TD", "TH"};
   const int      kanTagLen[19] = {1, 6, 4, 4, 7, 4, 5, 4, 5, 6, 3, 5, 7, 4, 4, 6, 5, 2, 2}; // Counted by hand
   const int      knTagAnchor = 0;
   const int      knTagApplet = 1;
   const int      knTagArea = 2;
   const int      knTagBase = 3;
   const int      knTagBGSound = 4;
   const int      knTagBody = 5;
   const int      knTagEmbed = 6;
   const int      knTagForm = 7;
   const int      knTagFrame = 8;
   const int      knTagIFrame = 9;
   const int      knTagImg = 10;
   const int      knTagInput = 11;
   const int      knTagIsIndex = 12;
   const int      knTagLink = 13;
   const int      knTagMeta = 14;
   const int      knTagObject = 15;
   const int      knTagTable = 16;
   const int      knTagTD = 17;
   const int      knTagTH = 18;

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

   URIMap uriMap;
   int nURIs = uriMap.BuildURIMap( pszIn );
   if ( ! nURIs )
	   return 0;

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
                                 if (uriMap.ResolveURI(szURI, szURLPath, 512) && lstrcmpi(szURI, szURLPath)) {
                                    // We've actually transformed one URI into another - this is a real change!
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
   const int   cnBlockSize = 1024;  // The size of a block to reallocate. This is done to avoid thousands of realloc() calls.

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
int URIMap::BuildURIMap( const char * pMessage )
{
	// look for "Embedded content" lines and X-EmbeddedContent headers
	CRString ECHeader(IDS_HEADER_EMBEDDED_CONTENT);
	CRString ECTrailer(IDS_EMBEDDED_CONTENT);
	int		found = 0;
	char * pIt;

	// in case we've already built one...
	FreeURIMap();

	// first look for X-EmbeddedContent headers
	const char *	pTmp = pMessage;
	while ( pIt = strstr( pTmp, ECHeader ) )
	{
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
				EmbeddedElement* EE = new EmbeddedElement();
				if ( EE )
				{
					// init the EE
					char tmp = *pEndCID;
					*pEndCID = '\0';
					EE->SetCID( pCID );
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
			EmbeddedElement* EE = new EmbeddedElement();
			if ( EE )
			{
				// init the EE
				sscanf( pMRI, "%lx,%lx,%lx,%lx", &MRI, &CIDHash, &CLHash, &MRI_CB_CL_Hash );

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

		// look for another
		pTmp = pIt + 1;
	}

	return found;
}

int URIMap::ResolveURI( const char * pszInURI, char * pszOutFilename, int nBufSize )
{
	*pszOutFilename = '\0';

	// look throught the EmbeddedElements list for a match...
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if ( EE )
		{
			long lHash;

			if ( strnicmp( pszInURI, "cid:", 4) == 0 )
			{
				// the URIs a CID
				lHash = ::HashMT( &pszInURI[ 4 ] );
				if ( lHash == (long)EE->GetCIDHash() )
				{
					CString path = EE->GetPath();

					// BOG: here's my lame hack for telling if a path is actually
					// a full win32 path. (relative paths start without backslash.)
					if ( path[0] == '\\' || path[1] == ':' ) 
					{
						strcpy( pszOutFilename, path );
					}
					else 
					{
						CRString EmbeddedDir(IDS_EMBEDDED_FOLDER);
						if ( EudoraDir.GetLength() + 
							 EmbeddedDir.GetLength() + 1 +
							 path.GetLength() < nBufSize )
						{
							// must prepend <eudoradir>\Embedded\ ...
							// EudoraDir contains the \\; EmbeddedDir doesn't
							sprintf(pszOutFilename,"%s%s%s\\%s",
								(const char *)"file://",
								(const char *)EudoraDir, 
								(const char *)EmbeddedDir,
								(const char *)path);
						}
					}

					return strlen( pszOutFilename );
				}
			}
			else
			{
				// check the Content-Base / Content-Location stuff

				// KCM - wazoo magic goes in here...

				lHash = ::HashMT( pszInURI );
			}
		}
	}

	// must not have found a match...
	return 0;
}

void URIMap::FreeURIMap()
{
	// clean the list
	while (m_EmbeddedElements.IsEmpty() == FALSE)
		delete m_EmbeddedElements.RemoveHead();
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

BOOL URIMap::IsRealCID( CString TargetCID )
{
	// look for the TargetCID in string form
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if ( EE )
		{
			CString CID = EE->GetCID();

			if ( CID && ( CID.GetLength() > 4 ) )
			{
				if ( strcmpi( TargetCID, CID ) == 0 )
					return TRUE;
			}
		}
	}

	return FALSE;	// CID in string form not found...
}


// used when creating messages - generates a CID based on a messageid
int URIMap::AddCIDMap( const char * pInMessageId, const char * pInPath, 
			   char * pszOutCID, int nCIDSize )
{
	// gen up a unique CID
	CString CID;
	int seq = m_EmbeddedElements.GetCount();
	while ( 1 )
	{
		// BOG: hacked to 'prepend' "cid:"
		CID.Format( "cid:%s.%d", pInMessageId, seq++ );

		char filename[ 256 ];
		if ( ! URIMap::ResolveURI( CID, filename, sizeof( filename )) )
			break;
	}

	// build up the EmbeddedElement
	EmbeddedElement* EE = new EmbeddedElement();
	if ( EE )
	{
		// init the EE
		EE->SetPath( pInPath );
		EE->SetCID( CID );
			
		// add it to the list
		m_EmbeddedElements.AddTail(EE);
	}

	// return the CID
	strncpy( pszOutCID, CID, nCIDSize );

	return strlen( CID );
}

// used when creating messages
void URIMap::BuildEmbeddedObjectHeaders( CString & Header )
{
	BOOL	bFirst = TRUE;
	Header = "";
	CRString ECHeader(IDS_HEADER_EMBEDDED_CONTENT);

	// build up a header based on the URIMap (m_theMap)
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		CString CID = EE->GetCID();
		if ( CID && CID.GetLength() )
		{
			char buf[ 256 ];

			if ( bFirst )
			{
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
	Paths = "";

	char EmbeddedDir[_MAX_PATH + 1];
	wsprintf(EmbeddedDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_EMBEDDED_FOLDER));

	// build up a header based on the URIMap (m_theMap)
	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		CString path = EE->GetPath();
		if ( path && path.GetLength() )
		{
			char buf[ 256 ];

			if ( ! strchr( path, '\\' ) )
				sprintf( buf, "%s\\%s\n", EmbeddedDir, (const char *)path );
			else
				sprintf( buf, "%s\n", (const char *)path );

			Paths += buf;
		}
	}

	return;
}



