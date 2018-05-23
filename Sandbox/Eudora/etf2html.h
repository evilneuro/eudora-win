/*
|| File       : etf2html.h
|| Date       : 12-Aug-97
|| Description: E2 Interface
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


|| Revisions  :
*/


class CHtmlOut {
public:
   virtual void Write(LPCSTR pcBuf, int cbBuf) = 0;
};


#define MAXTAGLEN     32
#define MAXPARAMLEN   64
#define MAXTEXTLEN   500	// the size of URLs which can be recognized is limted to MAXOUTLEN

class CEtf2Html {
   int       m_nState;
   CHtmlOut *m_pho;		        // our 'target'
   BOOL      m_nTagPending;		// id of most recently-seen tag (+ 1)
   int       m_nSpaces;			// count of spaces seen in EHS_SP state
   int       m_nParamEndPos;            // count of how much of "</param>" has been seen in EHS_PARAMLT state

   char      m_pcTag[MAXTAGLEN];	// holds tag names
   int       m_nTagLen;

   char      m_pcParam[MAXPARAMLEN];	// holds param values
   int       m_nParamLen;

   char      m_pcText[MAXTEXTLEN];	// buffers text (possibly auto-recognized URLs) to be output
   int       m_nTextLen;
   int       m_nTextState;
   char      m_chUrlEnd;		// when in tsAccumUrl, this is a terminator to look for


   void TextChar(char c);
   void TextFlush(BOOL bQuoted = FALSE);
   void TextBreak();

   inline void OutPuts(LPCSTR sz) {
      m_pho->Write(sz, strlen(sz));
   }

   int  TakeTag();
   void OutputTag();

public:

   /*
   || Create a converter object.
   || On entry:
   ||    pho = The 'target' object which will received the HTML.
   */
   CEtf2Html(CHtmlOut *pho);

   ~CEtf2Html();

   /*
   || Call Take() to give the converter object the next chunk of text/enriched to convert.
   ||
   || Note: The CHtmlOut target's Write() function may (probably will) be called from within this function.
   */
   void Take(LPCSTR pcBuf, int cbBuf);    // takes the next chunk of ETF

   /*
   || Call this when there is no more text/enriched to convert, so the converter object may flush
   ||   any data it has cached.  This may also call CHtmlOut's Write().
   */
   void Flush();			    // indicates end of stream hsa been reached
};


extern "C" {
// These utility functions should make working with zero-terminated strings easy
//

int Etf2Html_Measure(LPCSTR szEtf);
void Etf2Html_Convert(char *szResult, LPCSTR szEtf);

}		// extern "C"
