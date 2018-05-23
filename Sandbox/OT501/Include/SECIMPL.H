/////////////////////////////////////////////////////////////////////////////
// SecImpl.h : Internal declarations for the SEC++ library
//
// Stingray Software Extension Classes
// Copyright (C) 1996 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
//	Author:		Don			05/15/96
//

#ifndef __SECIMPL_H__
#define __SECIMPL_H__

// Useful drawing functions ... these exist in the MFC from version 3 onwards.
#ifdef WIN32

#define FillSolidRect1(dc, rect, color) \
		(dc)->FillSolidRect(rect, color)
		
#define FillSolidRect2(dc, x, y, cx, cy, color) \
		(dc)->FillSolidRect(x, y, cx, cy, color)
		
#define Draw3dRect1(dc, rect, color1, color2) \
		(dc)->Draw3dRect(rect, color1, color2)
		
#define Draw3dRect2(dc, x, y, cx, cy, color1, color2) \
		(dc)->Draw3dRect(x, y, cx, cy, color1, color2)

#else // WIN32

extern void FillSolidRect1(CDC* pDC, LPCRECT lpRect, COLORREF color);
extern void FillSolidRect2(CDC* pDC, int x, int y, int cx, int cy, COLORREF color);
extern void Draw3dRect1(CDC* pDC, LPCRECT lpRect, COLORREF color1, COLORREF color2);
extern void Draw3dRect2(CDC* pDC, int x, int y, int cx, int cy, COLORREF color1,
						COLORREF color2);  
						
#endif // WIN32


#endif // __SECIMPL_H__
 
