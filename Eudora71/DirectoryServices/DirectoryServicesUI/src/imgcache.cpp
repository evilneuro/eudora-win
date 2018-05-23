/******************************************************************************/
/*	Application	:	DSERV.EXE    																*/
/*	Name			:	IMGLIST.CPP  																*/
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

/*	Desc.			:	Protocol key'd image list cache thing...							*/
/******************************************************************************/
#include "stdafx.h"
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <ole2.h>
#include <commctrl.h>
#include "imgCache.h"

#include "DebugNewHelpers.h"


typedef struct tagIMGITEM
{
   ITEM        item;
   HBITMAP     hBit;
   int         iIndex;
} IMGITEM, *LPIMGITEM;

CImgCache::CImgCache()
{
   m_hil          = NULL;
   m_pProtoList   = NULL;
}

CImgCache::~CImgCache()
{
   if(m_hil)         ImageList_Destroy(m_hil);
   if(m_pProtoList)  m_pProtoList->Release();

   m_pProtoList   = NULL;
   m_hil          = NULL;
}

/* Static */
void CImgCache::ImgItemFreeCB(LPITEM pItem,LPVOID)
{
	delete pItem;
}

HRESULT __stdcall CImgCache::Initialize(COLORREF crMask)
{
   HRESULT  hErr;

   m_crMask = crMask;
	m_hil	= ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_MASK,1,1);
   if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pProtoList))) {
      m_pProtoList->Initialize(CImgCache::ImgItemFreeCB,NULL);
   }
   return hErr;
}

/* Protected */
LPIMGITEM __stdcall CImgCache::Find(HBITMAP hBit)
{
   DWORD       i;
   DWORD       dwCount  = m_pProtoList->Count();
   IMGITEM     *pItem   = (IMGITEM*) m_pProtoList->Head();

   for(i=0;i<dwCount;i++) {
      if(pItem->hBit == hBit)
         return pItem;
      pItem = (IMGITEM *) ILIST_ITEMNEXT(pItem);
   }
   return NULL;
}

/* Protected */
LPIMGITEM __stdcall CImgCache::AddBitmap(HBITMAP hBit)
{
	IMGITEM  *pItem = DEBUG_NEW_NOTHROW IMGITEM;

   if (pItem) {
      pItem->hBit    = hBit;
      pItem->iIndex  = ImageList_AddMasked(m_hil,hBit,m_crMask);
      m_pProtoList->Attach(pItem);
   }
   return pItem;
}

int __stdcall CImgCache::Add(HBITMAP hBit)
{
   IMGITEM  *pItem = NULL;

   if(NULL == (pItem = Find(hBit))) {
      pItem = AddBitmap(hBit);
   }
   return pItem->iIndex;
}

HIMAGELIST __stdcall CImgCache::GetImageList()
{
   return m_hil;
}
