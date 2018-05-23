/******************************************************************************/
/*																										*/
/*	Name		:	LINETBL.CPP  																   */
/* Date     :  10/17/1997                                                     */
/* Author   :  Jim Susoy                                                      */
/* Notice   :  (C) 1997 Qualcomm, Inc. - All Rights Reserved                  */
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

/*	Desc.		:	IDSPConfig Implementation  												*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "database.h"
#include "QCUtils.h"


#define LINEBUFSZ 0x10000              /* 64K line buffer size                */

/* Protected */
HRESULT __stdcall CDatabase::AddLine(LPSTR pszBuf)
{
   LPLTBL   pTbl        = DEBUG_NEW_NOTHROW LTBL;

   if(pTbl) {
      memset( pTbl, 0, sizeof(LTBL) );

      if(NULL == (pTbl->pszLine = SafeStrdupMT(pszBuf))) {
         delete pTbl;

         return E_OUTOFMEMORY;
      }
      m_pLineTable->Attach(pTbl);
      return S_OK;
   }
   return E_OUTOFMEMORY;
}

HRESULT __stdcall CDatabase::FoldInTheAddresBookKludge(LPSTR pszFoldedLine)
{
   int      nLen = lstrlen(pszFoldedLine);
   LPLTBL   pTbl = (LPLTBL) m_pLineTable->Head();

   if(pTbl) {
      LPSTR pszSave     = NULL;
      int   oLen        = 0;
      LPSTR pszNewLine  = NULL;

      /* Get the "tail" of the list; the last line that was added             */
      pTbl        = (LPLTBL) ILIST_ITEMPREV(pTbl);

      /* save off a pointer to it and get it's length                         */
      pszSave     = pTbl->pszLine;
      oLen        = lstrlen(pszSave);

      /* Then alloc a buffer for the new combined line                        */
	  pszNewLine = DEBUG_NEW_NOTHROW char[nLen+oLen+1];

      if(pszNewLine) {
         lstrcpy(pszNewLine,pszSave);
         lstrcat(pszNewLine,pszFoldedLine);

		 delete [] pTbl->pszLine;

         pTbl->pszLine = pszNewLine;
         return S_OK;
      }
      return E_OUTOFMEMORY;
   }
   return E_FAIL;
}

HRESULT __stdcall CDatabase::BuildLineTable()
{
   HRESULT  hErr        = E_FAIL;
   FILE     *hFile      = NULL;
   LPSTR    pszBuf      = DEBUG_NEW_NOTHROW char [LINEBUFSZ];

   if(!pszBuf)
      return E_OUTOFMEMORY;

   if(NULL != (hFile = fopen(m_pszPath,"r"))) {
      while(NULL != fgets(pszBuf,LINEBUFSZ,hFile)) {
         int   nLen = lstrlen(pszBuf);

         /* Remove CR and LF from data                                        */
         nLen--;                       /* adjust for zero relative            */
         while(nLen >= 0 && ('\n' == pszBuf[nLen] || '\r' == pszBuf[nLen])) {
            pszBuf[nLen] = 0;
            nLen--;
         }

         if(' ' == *pszBuf) {
            if(FAILED (hErr = FoldInTheAddresBookKludge(pszBuf))) {
               break;
            }
         }
         else if(*pszBuf) {
            if(FAILED(hErr = AddLine(pszBuf))) {
               break;
            }
         }
      }
      fclose(hFile);
   }

   delete [] pszBuf;

   return hErr;
}

