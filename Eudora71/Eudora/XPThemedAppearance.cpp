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



#include "stdafx.h"
#include "XPThemedAppearance.h"


//	Statics
long															CXPThemedAppearance::s_nRefCount = 0;
HMODULE															CXPThemedAppearance::s_hThemeLib = NULL;
CXPThemedAppearance::IndirectIsThemeActive						CXPThemedAppearance::s_pIsThemeActive = NULL;
CXPThemedAppearance::IndirectOpenThemeData						CXPThemedAppearance::s_pOpenThemeData = NULL;
CXPThemedAppearance::IndirectCloseThemeData						CXPThemedAppearance::s_pCloseThemeData = NULL;
CXPThemedAppearance::IndirectDrawThemeIcon						CXPThemedAppearance::s_pDrawThemeIcon = NULL;
CXPThemedAppearance::IndirectDrawThemeText						CXPThemedAppearance::s_pDrawThemeText = NULL;
CXPThemedAppearance::IndirectDrawThemeEdge						CXPThemedAppearance::s_pDrawThemeEdge = NULL;
CXPThemedAppearance::IndirectDrawThemeBackground				CXPThemedAppearance::s_pDrawThemeBackground = NULL;
CXPThemedAppearance::IndirectGetThemeBackgroundContentRect		CXPThemedAppearance::s_pGetThemeBackgroundContentRect = NULL;


// ---------------------------------------------------------------------------
//		* CXPThemedAppearance										 [Public]
// ---------------------------------------------------------------------------
//	Constructor.

CXPThemedAppearance::CXPThemedAppearance()
	:	m_hTheme(NULL), m_hWnd(NULL)
{
	//	Use of library is ref counted and shared
	if (s_nRefCount == 0)
	{
		//	Attempt to load the theme DLL
		s_hThemeLib = LoadLibrary( _T("UXTHEME.DLL") );

		if (s_hThemeLib)
		{
			//	Attempt to get all the proc addresses we want
			s_pIsThemeActive = reinterpret_cast<IndirectIsThemeActive>(
													GetProcAddress( s_hThemeLib, "IsThemeActive" ) );
			s_pOpenThemeData = reinterpret_cast<IndirectOpenThemeData>(
													GetProcAddress( s_hThemeLib, "OpenThemeData" ) );
			s_pCloseThemeData = reinterpret_cast<IndirectCloseThemeData>(
													GetProcAddress( s_hThemeLib, "CloseThemeData" ) );
			s_pDrawThemeIcon = reinterpret_cast<IndirectDrawThemeIcon>(
													GetProcAddress( s_hThemeLib, "DrawThemeIcon" ) );
			s_pDrawThemeText = reinterpret_cast<IndirectDrawThemeText>(
													GetProcAddress( s_hThemeLib, "DrawThemeText" ) );
			s_pDrawThemeEdge = reinterpret_cast<IndirectDrawThemeEdge>(
													GetProcAddress( s_hThemeLib, "DrawThemeEdge" ) );
			s_pDrawThemeBackground = reinterpret_cast<IndirectDrawThemeBackground>(
													GetProcAddress( s_hThemeLib, "DrawThemeBackground" ) );
			s_pGetThemeBackgroundContentRect = reinterpret_cast<IndirectGetThemeBackgroundContentRect>(
													GetProcAddress( s_hThemeLib, "GetThemeBackgroundContentRect" ) );

			if ( !s_pOpenThemeData || !s_pCloseThemeData || !s_pDrawThemeIcon || !s_pDrawThemeText ||
				 !s_pDrawThemeEdge || !s_pDrawThemeBackground || !s_pGetThemeBackgroundContentRect )
			{
				//	One or more functions are apparently missed - bail
				ASSERT( !"UxTheme.dll present, but functions missing!" );
				::FreeLibrary(s_hThemeLib);
				s_hThemeLib = NULL;
			}
		}
	}

	//	Increment the ref count
	s_nRefCount++;
}


// ---------------------------------------------------------------------------
//		* ~CXPThemedAppearance										 [Public]
// ---------------------------------------------------------------------------
//	Destructor.

CXPThemedAppearance::~CXPThemedAppearance()
{
	//	Decrement ref count
	s_nRefCount--;

	//	Close theme data, if any
	if (m_hTheme)
		s_pCloseThemeData(m_hTheme);

	//	If this was the last object, then free the library
	if (s_nRefCount == 0)
	{
		::FreeLibrary(s_hThemeLib);
		s_hThemeLib = NULL;
	}
}


// ---------------------------------------------------------------------------
//		* OpenTheme													 [Public]
// ---------------------------------------------------------------------------
//	Opens theme data.

void
CXPThemedAppearance::OpenTheme(
	HWND						hWnd,
	LPCWSTR						pszClassList)
{
	//	Do we have a theme lib?
	if (s_hThemeLib)
	{
		//	Did we already have theme data?
		if (m_hTheme)
		{
			s_pCloseThemeData(m_hTheme);
			m_hTheme = NULL;
		}
		
		//	Verify that our caller gave us a window
		ASSERT(hWnd);

		if (hWnd)
		{
			//	Open theme data for our window with the indicated class list
			m_hWnd = hWnd;
			m_hTheme = s_pOpenThemeData(m_hWnd, pszClassList);
		}
	}
}


// ---------------------------------------------------------------------------
//		* DrawThemeIcon												 [Public]
// ---------------------------------------------------------------------------
//	Draws theme icon.

HRESULT
CXPThemedAppearance::DrawThemeIcon(
	HDC							hdc,
	int							iPartId,
	int							iStateId,
	const RECT *				pRect,
	HIMAGELIST					himl,
	int							iImageIndex)
{
	ASSERT(m_hTheme);
	
	if (m_hTheme)
		return s_pDrawThemeIcon(m_hTheme, hdc, iPartId, iStateId, pRect, himl, iImageIndex);

	return E_FAIL;
}


// ---------------------------------------------------------------------------
//		* DrawThemeText												 [Public]
// ---------------------------------------------------------------------------
//	Draws theme text - converting string format for convenience.

HRESULT
CXPThemedAppearance::DrawThemeText(
	HDC							hdc,
	int							iPartId,
	int							iStateId,
	LPCSTR						pszText,
	int							iCharCount,
	DWORD						dwTextFlags,
	DWORD						dwTextFlags2,
	const RECT *				pRect)
{
	ASSERT(m_hTheme);

	HRESULT		hr = E_FAIL;

	if (m_hTheme)
	{
		//	Allocate buffer
		int			nLength = (iCharCount != -1) ? iCharCount : _mbstrlen(pszText);
		wchar_t *	wBuffer = new wchar_t[nLength+1];
		
		if (wBuffer)
		{
			//	Convert string to unicode
			MultiByteToWideChar( CP_THREAD_ACP, 0, pszText, nLength, wBuffer, nLength+1 );
			
			//	Terminate buffer
			wBuffer[nLength] = '\0';
			
			//	Draw text
			hr = s_pDrawThemeText( m_hTheme, hdc, iPartId, iStateId, wBuffer, iCharCount,
							  dwTextFlags, dwTextFlags2, pRect );
			
			//	Free buffer
			delete [] wBuffer;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}


// ---------------------------------------------------------------------------
//		* DrawThemeText												 [Public]
// ---------------------------------------------------------------------------
//	Draws theme text.

HRESULT
CXPThemedAppearance::DrawThemeText(
	HDC							hdc,
	int							iPartId,
	int							iStateId,
	LPCWSTR						pszText,
	int							iCharCount,
	DWORD						dwTextFlags,
	DWORD						dwTextFlags2,
	const RECT *				pRect)
{
	ASSERT(m_hTheme);
	
	if (m_hTheme)
	{
		return s_pDrawThemeText( m_hTheme, hdc, iPartId, iStateId, pszText, iCharCount,
								 dwTextFlags, dwTextFlags2, pRect );
	}
	
	return E_FAIL;
}


// ---------------------------------------------------------------------------
//		* DrawThemeBackground										 [Public]
// ---------------------------------------------------------------------------
//	Draws theme background.

HRESULT
CXPThemedAppearance::DrawThemeBackground(
	HDC							hdc,
	int							iPartID,
	int							iStateID,
	const RECT *				pRect,
	RECT *						pClipRect)
{
	ASSERT(m_hTheme);
	
	if (m_hTheme)
	{
		CRect r = *pRect;

		r.InflateRect(1, 1);
		
		return s_pDrawThemeBackground(m_hTheme, hdc, iPartID, iStateID, &r, pClipRect);
	}

	return E_FAIL;
}


// ---------------------------------------------------------------------------
//		* DrawThemeEdge												 [Public]
// ---------------------------------------------------------------------------
//	Draws theme edge.

HRESULT
CXPThemedAppearance::DrawThemeEdge(
	HDC							hdc,
	RECT *						pRect,
	UINT						uEdge,
	UINT						uFlags,
	int							iPartID,
	int							iStateID,
	RECT *						pClipRect)
{
	ASSERT(m_hTheme);
	
	if (m_hTheme)
		return s_pDrawThemeEdge(m_hTheme, hdc, iPartID, iStateID, pRect, uEdge, uFlags, pClipRect);

	return E_FAIL;
}


// ---------------------------------------------------------------------------
//		* GetThemeBackgroundContentRect								 [Public]
// ---------------------------------------------------------------------------
//	Gets theme background content rect.

HRESULT
CXPThemedAppearance::GetThemeBackgroundContentRect(
	HDC							hdc,
	int							iPartId,
	int							iStateId,
	const RECT *				pBoundingRect,
	RECT *						pContentRect)
{
	ASSERT(m_hTheme);
	
	if (m_hTheme)
	{
		return s_pGetThemeBackgroundContentRect( m_hTheme, hdc, iPartId, iStateId,
												 pBoundingRect, pContentRect );
	}

	return E_FAIL;
}
