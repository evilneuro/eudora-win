/******************************************************************************/
/*	Application	:	DSERV.EXE    																*/
/*	Name			:	IMGLIST.H     																*/
/* Date     	:  9/18/1997                                                   */
/* Author   	:  Jim Susoy                                                   */
/* Notice   	:  (C) 1997 Qualcomm, Inc. - All Rights Reserved               */
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

/*	Desc.			:	Definitions etc for IMGLIST.CPP              					*/
/******************************************************************************/
#ifndef __IMGLIST_H__
#define __IMGLIST_H__

#include "IList.h"

typedef struct tagIMGITEM *LPIMGITEM;

class CImgCache
{
public:
   virtual HRESULT      __stdcall Initialize(COLORREF crMask);
   virtual int          __stdcall Add(HBITMAP hBit);
   virtual HIMAGELIST   __stdcall GetImageList();
   static void ImgItemFreeCB(LPITEM pItem,LPVOID pUser);
   CImgCache();
   ~CImgCache();

protected:
   virtual LPIMGITEM    __stdcall Find(HBITMAP hBit);
   virtual LPIMGITEM    __stdcall AddBitmap(HBITMAP hBit);

   HIMAGELIST  m_hil;
   IListMan    *m_pProtoList;
   COLORREF    m_crMask;
} ;

#endif



