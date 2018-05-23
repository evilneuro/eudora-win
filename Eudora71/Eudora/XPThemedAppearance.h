//	XPThemedAppearance.h
//
//	Eases use of theme lib.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#ifndef __XPThemedAppearance_H__
#define __XPThemedAppearance_H__

#pragma once


#include <uxtheme.h>
#include <tmschema.h>


class CXPThemedAppearance
{
  public:
											CXPThemedAppearance();
											~CXPThemedAppearance();

	//	Accessors
	bool									ShouldUseXPThemedDrawing() const { return s_hThemeLib && s_pIsThemeActive && (s_pIsThemeActive() != FALSE); }

	void									OpenTheme(
												HWND						hWnd,
												LPCWSTR						pszClassList);

	HRESULT									DrawThemeIcon(
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												const RECT *				pRect,
												HIMAGELIST					himl,
												int							iImageIndex);
	HRESULT									DrawThemeText(
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												LPCSTR						pszText,
												int							iCharCount,
												DWORD						dwTextFlags,
												DWORD						dwTextFlags2,
												const RECT *				pRect);
	HRESULT									DrawThemeText(
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												LPCWSTR						pszText,
												int							iCharCount,
												DWORD						dwTextFlags,
												DWORD						dwTextFlags2,
												const RECT *				pRect);
	HRESULT									DrawThemeBackground(
												HDC							hdc,
												int							iPartID,
												int							iStateID,
												const RECT *				pRect,
												RECT *						pClipRect = NULL); 
	HRESULT									DrawThemeEdge(
												HDC							hdc,
												RECT *						pRect,
												UINT						uEdge,
												UINT						uFlags,
												int							iPartID,
												int							iStateID,
												RECT *						pClipRect = NULL);
	HRESULT									GetThemeBackgroundContentRect(
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												const RECT *				pBoundingRect,
												RECT *						pContentRect);
  protected:
	//	Theme function pointer typedefs
	typedef BOOL							(__stdcall *IndirectIsThemeActive)();
	typedef HTHEME							(__stdcall *IndirectOpenThemeData)(
												HWND						hwnd,
												LPCWSTR						pszClassList);
	typedef HRESULT							(__stdcall *IndirectCloseThemeData)(
												HTHEME						hTheme);
	typedef HRESULT							(__stdcall *IndirectDrawThemeIcon)(
												HTHEME						hTheme,
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												const RECT *				pRect,
												HIMAGELIST					himl,
												int							iImageIndex);
	typedef HRESULT							(__stdcall *IndirectDrawThemeText)(
												HTHEME						hTheme,
												HDC							hdc,
												int							iPartId, 
												int							iStateId,
												LPCWSTR						pszText,
												int							iCharCount,
												DWORD						dwTextFlags, 
												DWORD						dwTextFlags2,
												const RECT *				pRect);
	typedef HRESULT							(__stdcall *IndirectDrawThemeEdge)(
												HTHEME						hTheme,
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												const RECT *				pDestRect,
												UINT						uEdge,
												UINT						uFlags,
												RECT *						pContentRect);
	typedef HRESULT							(__stdcall *IndirectDrawThemeBackground)(
												HTHEME						hTheme,
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												const RECT *				pRect,
												OPTIONAL const RECT *		pClipRect);
	typedef HRESULT							(__stdcall *IndirectGetThemeBackgroundContentRect)(
												HTHEME						hTheme,
												HDC							hdc,
												int							iPartId,
												int							iStateId,
												const RECT *				pBoundingRect,
												RECT *						pContentRect);


  protected:
	//	Statics
	static long										s_nRefCount;
	static HMODULE									s_hThemeLib;
	static IndirectIsThemeActive					s_pIsThemeActive;
	static IndirectOpenThemeData					s_pOpenThemeData;
	static IndirectCloseThemeData					s_pCloseThemeData;
	static IndirectDrawThemeIcon					s_pDrawThemeIcon;
	static IndirectDrawThemeText					s_pDrawThemeText;
	static IndirectDrawThemeEdge					s_pDrawThemeEdge;
	static IndirectDrawThemeBackground				s_pDrawThemeBackground;
	static IndirectGetThemeBackgroundContentRect	s_pGetThemeBackgroundContentRect;

	//	Data members
	HTHEME									m_hTheme;
	HWND									m_hWnd;
};


#endif		//	__XPThemedAppearance_H__
