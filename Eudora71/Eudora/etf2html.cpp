/*
|| File       : etf2html.cpp
|| Date       : 11-Aug-97
|| Description: Text/enriched to HTML translator
||
|| Author     : Brian Kelley
|| Copyright  : Qualcomm, Incorporated, 1997
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


|| Misc.      : tabs=3
|| Revisions  :
|| 
|| 20-Oct-97 bhk : Fixed color param handling
||                 A) use strchr(pcParam, ',') instead of isdigit(*pcParam) to distinguish names
||                    (e.g. "blue")from numbers (e.g. "bb00,0000,0000")
||                 B) use most- instead of least-significant digits ("bb00,0080,f800" -> "#bb00f8", not "#008000").
||                 C) color attribute ("#rrggbb") is now properly quoted ("#" is not a legal SGML token character).
|| 21-Oct-97 bhk : Added auto-recognition and markup of URLs (TextChar(), TextFlush() and TextBreak()).
||                 I only markup as URLs strings that begin with "scheme:" where "scheme" is one of a few URL
||                 schemes;  this should minimize false-positives and still work for all the commonly-used
||                 schemes.  Further, the ":" must be followed by a non-sace character in order for it to
||                 be considered a URL.   An additional improvement is to drop a single "," and "." character
||                 from the end of URLs which were space-delimited (URLs in <>, "", or [] will be retained
||                 literally).
||
*/

#include "stdafx.h"

#include <string.h>

#include "etf2html.h"


#include "DebugNewHelpers.h"


//----------   Macros and inline functions

#define ISINRANGE(val, low, high)   ((unsigned) ((val) - (low)) <= (unsigned) ((high) - (low)))

#define ARRAYSIZE(a)    (sizeof(a) / sizeof( *(a) ) )


inline BOOL CharEQI(char c1, char c2)
{
   return (c1 == c2 || ((c1|32) == (c2|32) && ISINRANGE(c1|32, 'a', 'z')));
}


inline int imin(int i1, int i2) {
   return (i1 < i2 ? i1 : i2);
}



//----------   Types and Constants

enum CEtf2HtmlState {
   EHS_NORMAL,
   EHS_SP,
   EHS_NL,
   EHS_LT,
   EHS_INTAG,
   EHS_PARAM,
   EHS_PARAMLT
};


enum CEtf2HtmlTextState {
	EHTS_REGULAR,
	EHTS_NAME,
	EHTS_MAYBEURL,
	EHTS_URL,
};

static char gszParamEnd[] = "</param>";


struct TagData {
   char *szEtf;
   char *szHtml;
   char *szHtmlClose;
};


// This array lists ETF tags which are translated into HTML, and contains the associated HTML open and
//   close tags for each.
// EHTAG_... enum values correspond to gaTags[] entries.

TagData gaTags[] =  {
   {"bold",       "<b>",     "</b>"},
   {"italic",     "<i>",     "</i>"},
   {"underline",  "<u>",     "</u>"},
   {"fixed",      "<tt>",    "</tt>"},
   {"fontfamily", "<font>",  "</font>"},
   {"color",      "<font>",  "</font>"},
   {"smaller",    "<small>", "</small>"},
   {"bigger",     "<big>",   "</big>"},

   {"center",     "<center>",               "</center>"},
   {"flushleft",  "<div align=left>",       "</div>"},
   {"flushright", "<div align=right>",      "</div>"},
   {"paraindent", "<dl>",                   "</dl>"},
   {"nofill",     "<pre>",                  "</pre>"},
   {"excerpt",    "<blockquote type=cite>", "</blockquote>"},
};


enum {
   EHTAG_B,
   EHTAG_I,
   EHTAG_U,
   EHTAG_TT,
   EHTAG_FACE,
   EHTAG_COLOR,
   EHTAG_SMALL,
   EHTAG_BIG,
   EHTAG_CENTER,
   EHTAG_LEFT,
   EHTAG_RIGHT,
   EHTAG_INDENT,
   EHTAG_NOFILL,
   EHTAG_EXCERPT
};


// This list of schemes is used by the URL auto-markup code
char *gaszSchemes[] = {
	"http",
	"https",
	"news",
	"mailto",
	"gopher",
	"mk",
	0		// zero-terminated array of pointers
};


// Helper functions

// Put 'c' into pc[] at nPos, and increment nPos unless it would be >= nMax

static void BoundPutc(char *pc, int &nPos, int nMax, char c)
{
	if( c == '\"' )
	{
		if( nPos + 5 < nMax ) 
		{
			memmove( pc + nPos, "&#34;", 5 );
			nPos += 5;
		}

		return;
	}

	if (nPos > nMax)
      nPos = nMax - 1;

   pc[nPos++] = c;
   if (nPos == nMax)
      --nPos;      
}



CEtf2Html::CEtf2Html(CHtmlOut *pho)
{
   m_nState = EHS_NORMAL;
   m_pho = pho;

   m_pcParam[0] = 0;

	m_nTextState = EHTS_REGULAR;
	m_nTextLen = 0;

   m_nTagLen = 0;
   m_nParamLen = 0;

   m_nTagPending = 0;

   // Eudora wants this, I assume. - bhk
   OutPuts( "<x-html>" );
}



CEtf2Html::~CEtf2Html()
{
}


inline BOOL IsValidScheme(char *pcScheme, int cbScheme)
{
	char **ppszScheme = gaszSchemes;
	char *szScheme;

	while (szScheme = *(ppszScheme++)) {
	   if (cbScheme == (int) strlen(szScheme) && _strnicmp(szScheme, pcScheme, cbScheme) == 0)
			return TRUE;
	}
	return FALSE;
}



// TextChar(c) : Output 'c' as text.
//    Each character of text comes through here.  Escaping (quoting) characters for HTML,
//    and markup of URLs is handled in the Text...() functions.
//
//  For URL markup, we must buffer text and determine whether it's a URL before outputting it.
//  While not in a URL, we buffer each word in m_pcText[].  If the word is followed by a colon and
//  is a scheme, we enter the EHTS_URL state, otherwise we output the word.
//  At the end of the URL, we output <A HREF='url'>url</A>
void CEtf2Html::TextChar(char c)
{
	char *pcChar = "";
	int cbChar = 0;

	// Get quoted representation of character: bytes in pcChar, length in cbChar

   if (c == '&')
      pcChar = "&amp;";
   else if (c == '<')
      pcChar = "&lt;";
	else if (c == '"')
		pcChar = "&quot;";
   else if (c == '\t')
      pcChar = "<x-tab>&nbsp;&nbsp;&nbsp;&nbsp;</x-tab>";      // bhk 21-Oct-97
   else {
      pcChar = (char *) &c;    // m_pho->Write(&c, 1);
		cbChar = 1;
	}

	if (!cbChar)
		cbChar = strlen(pcChar);

	// Buffer or output character
	//    State machine handles recognition and buffering of URLs or potential URLs
	//    EHTS_REGULAR : Not in a word; output character and potentially enter EHTS_NAME
	//    EHTS_NAME : In a 'word' which may turn out to be a scheme name
	//    EHTS_MAYBEURL : One colon has been seen; is the next character whitespace?
	//    EHTS_URL :  After "scheme:...".  Buffer characters and look for end of URL.

tc_imm_trans:
	switch (m_nTextState) {
	case EHTS_REGULAR:
		m_pho->Write(pcChar, cbChar);
		if (isspace((int)(unsigned char)c) || c == '<' || c == '[' || c == '(' || c == '"') {
			m_nTextState = EHTS_NAME;
			m_nTextLen = 0;
			m_chUrlEnd = (c == ' ' ? ' ' :
							  c == '"' ? '"' :
							  c == '<' ? '>' :
							  c == '[' ? ']' :
							  c == '(' ? ')' :
							  ' ');
		}
		break;

	case EHTS_MAYBEURL:
		// We've just 'seen' a colon... is it followed by a non-space character?
		if (!isspace((int)(unsigned char)c)) 
			m_nTextState = EHTS_URL;
		// If space, stay in MAYBEURL and the TextFlush() below will flush without creating a link
		// FALLTHROUGH

	case EHTS_NAME:
	case EHTS_URL:
		if (m_nTextState == EHTS_NAME && c == ':' && IsValidScheme(m_pcText, m_nTextLen))
			m_nTextState = EHTS_MAYBEURL;

		if (m_nTextLen + cbChar >= MAXTEXTLEN
			 || isspace((int)(unsigned char)c)
			 || c == m_chUrlEnd
			 || (m_nTextState == EHTS_NAME && !isalnum((int)(unsigned char)c)) ) {
			// end of URL or end of name
			TextFlush(c != ' ' && c == m_chUrlEnd);
			// go process and output 'c'
			m_nTextState = EHTS_REGULAR;
			goto tc_imm_trans;
		}

		memmove(m_pcText + m_nTextLen, pcChar, cbChar);
		m_nTextLen += cbChar;
		break;
	}

}

// This is called before an element is opened or closed, so the Text...() functions can output
//    any buffered data.
// This is also called from within TextChar() to output pcText[] when exiting EHTS_NAME or EHTS_URL
//
//  bQuoted == TRUE if a URL is begin flushed as a result of a matching terminating quote.
//             FALSE if the buffer if being flushed due to whitespace, tags, etc.
void CEtf2Html::TextFlush(BOOL bQuoted)
{
	if (m_nTextLen) {
		if (m_nTextState == EHTS_URL) {
			// Write out pcText[] as URL

			// Drop trailing "," or "." if URL is terminated by space
			int cbChar = 0;
			if (!bQuoted && (m_pcText[m_nTextLen-1] == ',' || m_pcText[m_nTextLen-1] == '.'))
				cbChar = 1;

			OutPuts("<A HREF=\"");
			m_pho->Write(m_pcText, m_nTextLen - cbChar);
			OutPuts("\">");
			m_pho->Write(m_pcText, m_nTextLen - cbChar);
			OutPuts("</A>");

			if (cbChar)
				m_pho->Write(m_pcText + m_nTextLen - cbChar, cbChar);

		} else {
			// Write out pcText[] as regular text
			m_pho->Write(m_pcText, m_nTextLen);
		}
		// last character was probably a non-space
		m_nTextState = EHTS_REGULAR;
	} else {
		// leave state unchanged
	}
	m_nTextLen = 0;
}


// This is called when a paragraph, line or word break is output, so the Text output state can be adjusted
// accordingly.
void CEtf2Html::TextBreak()
{
	TextFlush();
	m_nTextState = EHTS_NAME;		// the "EHTS_NAME" state is appropriate after whitespace
	m_chUrlEnd = ' ' ;
}



// Take tag stored in m_pcArg[] and look it up, storing ID in m_nTagPending
// Return state parser should enter,
int CEtf2Html::TakeTag()
{
   if (strcmpi(m_pcTag, "param") == 0) {
      m_nParamLen = 0;
      return EHS_PARAM;
   }

   if (m_nTagPending)
      OutputTag();

   BOOL bClose = (m_pcTag[0] == '/');
   char *szTag = (bClose ? m_pcTag + 1 : m_pcTag);
      

   for (int nn = 0; nn < ARRAYSIZE(gaTags); ++nn) {
      if (!strcmpi(szTag, gaTags[nn].szEtf)) {
			if (bClose) {
				TextFlush();
				OutPuts(gaTags[nn].szHtmlClose);
			} else {
				m_nTagPending = nn + 1;
			}
			break;
      }
   }

   return EHS_NORMAL;
}


//  Output tag id
//  If m_nParamLen is non-zero, m_pcParam[] will be considered the contents of a <param>...</param> block
void CEtf2Html::OutputTag()
{
	if (m_nTagPending)
	{
		int nTag = m_nTagPending - 1;
		char *szTag = gaTags[nTag].szHtml;
		char szAttr[256] = {0};

		if (m_nParamLen)
		{
			if (nTag == EHTAG_COLOR)
			{
				// output color
				// text/enriched color param is "rrrr,gggg,bbbb"
				//  _or_ one of:  "red", "green", "blue", "yellow", "cyan", "magenta", "black", "white"
				// HTML tag syntax is "<font color=#rrggbb>"

			   // "#" is not a valid token character; color attribute should be quoted -- bhk 21-Oct-97
				//Paige does not handle ' , so changed it to output "    == Kusuma, 11/5/97
				strcpy(szAttr, "color=\"#");
				if (strchr(m_pcParam, ','))
				{
					int r,g,b;
					sscanf(m_pcParam, "%x,%x,%x", &r, &g, &b);
					r = (r>>8) & 0xFF;
					g = (g>>8) & 0xFF;
					b = (b>>8) & 0xFF;
					sprintf(szAttr + strlen(szAttr), "%02X%02X%02X", r, g, b);
				}
				else
				{
					strcat(szAttr, m_pcParam);
				}
				strcat(szAttr, "\"");
			}
			else if (nTag == EHTAG_FACE)
			{
				strcpy(szAttr, "face=\"");
				strcat(szAttr, m_pcParam);
				strcat( szAttr, "\"" );
			}
		}

		TextFlush();
		if (!*szAttr)
			OutPuts(szTag);
		else
		{
			char szTemp[100];

			strcpy(szTemp, szTag);
			szTemp[strlen(szTag) - 1] = ' ';
			OutPuts(szTemp);
			OutPuts(szAttr);
			OutPuts(">");
		}
	}

	m_nTagPending = 0;
	m_nParamLen = 0;
}


void CEtf2Html::Take(LPCSTR pcBuf, int cbBuf)
{
	char c; //, i, paramct=0, nofill=0;
	// char token[62], *p;

	for (int nIdx = 0; nIdx < cbBuf; ) {
		c = pcBuf[nIdx++];

	take_reprocess:	                     // (for immediate state transititions)

		switch (m_nState) {
		case EHS_NORMAL:
			if (c == '<')
				m_nState = EHS_LT;
			else if (c == '\n') {
				m_nState = EHS_NL;
			} else if (c == '\r' || c == ' ') {
				m_nSpaces = 1;
				m_nState = EHS_SP;
			} else {
				if (m_nTagPending) {
					// Output results of recently-encountered tag & perhaps param
					OutputTag();
				}
				// Optimize here all non-"<" and non-'&' characters will make one fast block Write()
				TextChar(c);
			}
			break;

		case EHS_SP:     // immediately following a space character
			if (c == ' ') {
				++m_nSpaces;
			} else {
				TextBreak();
				while (m_nSpaces-- > 1)
					OutPuts("&nbsp;");
				OutPuts(" ");
				m_nState = EHS_NORMAL;
				goto take_reprocess;
			}
			break;

		case EHS_NL:	// immediately following a "\n" character
			if (c != '\r') {
				if (c == '\n') {
					TextBreak();
					// woo-hoo : more than one!
					OutPuts("<BR>\r\n");
				} else {
					m_nState = EHS_NORMAL;
					goto take_reprocess;
				}
			}
			break;

		case EHS_LT:	// immediately following a "<" character
			if (c ==  '<') {
				TextChar(c);
				m_nState = EHS_NORMAL;
				break;
			} else {
				m_nTagLen = 0;
				m_nState = EHS_INTAG;
			}
			/* FALLTHROUGH */

		case EHS_INTAG:
			if (c == '>') {
				m_pcTag[m_nTagLen] = 0;
				m_nState = TakeTag();	// go to NORMAL or PARAM state
			} else {
				BoundPutc(m_pcTag, m_nTagLen, MAXTAGLEN, c);
			}
			break;


		case EHS_PARAM:	// After a <param> tag;  get contents...
			if (c == '<') {
				m_nState = EHS_PARAMLT;
				m_nParamEndPos = 1;
			} else {
				BoundPutc(m_pcParam, m_nParamLen, MAXPARAMLEN, c);
			}
			break;

		case EHS_PARAMLT: {	// after "<" inside <PARAM>..</PARAM>
			BoundPutc(m_pcParam, m_nParamLen, MAXPARAMLEN, c);
			char chEnd = gszParamEnd[m_nParamEndPos++];
			if (!chEnd) {
				// we're done with param!
				m_nParamLen -= strlen(gszParamEnd);
				m_pcParam[m_nParamLen] = 0;
				m_nState = EHS_NORMAL;
				goto take_reprocess;
			} else if (!CharEQI(c, chEnd)) {
				m_nState = EHS_PARAM;	// didn't match "/PARAM>"
			}
			break;
		}

		} // switch

	} // for
}

void CEtf2Html::Flush()
{
	TextFlush();
	OutPuts( "</x-html>" );
}


// This CHtmlOut implementation counts the bytes in the result
class CHtmlOutCount : public CHtmlOut {
public:
   int m_nCount;

   CHtmlOutCount() { m_nCount = 0; }

   // Override the virtual ABC member which receives all the HTML...
   void Write(LPCSTR pc, int cbBuf) { m_nCount += cbBuf; }
};


// This CHtmlOut implementation simply stuffs the result into a buffer
class CHtmlOutBuf : public CHtmlOut {
   char *m_pc;
   int   m_nPos;

public:
   CHtmlOutBuf(char *pc) {
      m_pc = pc;
   }

   // Override the virtual ABC member which receives all the HTML...
   void Write(LPCSTR pc, int cbBuf) {
      memmove(m_pc, pc, cbBuf);
      m_pc += cbBuf;
   }

   // Call this at any time to ensure a properly-terminated result
   void ZeroTerminate() {
      m_pc[0] = 0;
   }
};


// This CHtmlOut implementation writes to a file
class CHtmlOutFile : public CHtmlOut {
   HFILE m_hf;

public:
   CHtmlOutFile(HFILE hf) { m_hf = hf; }

   // Override the virtual ABC member which receives all the HTML...
   void Write(LPCSTR pc, int cbBuf) {
      _lwrite(m_hf, pc, cbBuf);
   }
};


extern "C" {
// These utility functions should make working with zero-terminated strings easy
//

// Determine the number of bytes of HTML which will be generated in the conversion of szEtf[]
//    from text/enriched to HTML.
//
// NOTE: This does not inlude room for any terminating zero....
//
int Etf2Html_Measure(LPCSTR szEtf)
{
   CHtmlOutCount hoc;
   CEtf2Html cehCount(&hoc);
   cehCount.Take(szEtf, strlen(szEtf));
   cehCount.Flush();

   return hoc.m_nCount;
}


// Convert szEtf to HTML, placing the result in szResult[]
// On exit:  szResult[] contains the zero-terminated HTML.
//
void Etf2Html_Convert(char *szResult, LPCSTR szEtf)
{
   CHtmlOutBuf hob(szResult);
   CEtf2Html ceh(&hob);

   ceh.Take(szEtf, strlen(szEtf));
   ceh.Flush();

   hob.ZeroTerminate();
}

}		// extern "C"
