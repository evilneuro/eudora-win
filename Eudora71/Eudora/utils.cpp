// UTILS.CPP
//
// General utilties
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <QCUtils.h>

#include "eudora.h"
#include "utils.h"
#include "rs.h"
#include "font.h"
#include "guiutils.h"

#include "DebugNewHelpers.h"


// Maximum number of chars any charset will want to translate.
// The current value is 27 which is the number of UTF-8 chars
// that intersect with 1252.
#define XLATE_CHARS				27

// Maximum number of characters in string that will be translated.
// This number is actually determined by the number of bytes that
// can be in a UTF-8 character.  If we supported the full range of
// UTF-8 this value would be 6, but at the moment we only care about
// where UTF-8 intersects with 1252 and we can do this with only
// 3 bytes.  The array will need to be MAX_CHARS_TO_TRANS + 2 in
// size: 1 extra char for NULL termination (see ISOTranslate())
// and 1 char for the 1252 char to translate to.
#define MAX_CHARS_TO_TRANS			3
//#define MAX_CHARS_TO_TRANS		6

#define MAX_CHARACTER_SETS (IDS_MIME_UTF_8 - IDS_MIME_US_ASCII - 1)

// This table is wasteful in the IDS_MIME_ISO_LATIN9 case because
// there are fewer chars to be translated than in IDS_MIME_UTF_8.
// If this is deemed particularly egregious to anyone we can
// dynamically allocate this table and only allocate what is needed.
unsigned char pcXlateTable[MAX_CHARACTER_SETS][XLATE_CHARS][MAX_CHARS_TO_TRANS + 2] = {
	// IDS_MIME_ISO_LATIN9
	{{0xA4,		 0x00,		 0x00,		 0x00,		(UCHAR)'€'},
	 {0xA6,		 0x00,		 0x00,		 0x00,		(UCHAR)'Š'},
	 {0xA8,		 0x00,		 0x00,		 0x00,		(UCHAR)'š'},
	 {0xB4,		 0x00,		 0x00,		 0x00,		(UCHAR)'Ž'},
	 {0xB8,		 0x00,		 0x00,		 0x00,		(UCHAR)'ž'},
	 {0xBC,		 0x00,		 0x00,		 0x00,		(UCHAR)'Œ'},
	 {0xBD,		 0x00,		 0x00,		 0x00,		(UCHAR)'œ'},
	 {0xBE,		 0x00,		 0x00,		 0x00,		(UCHAR)'Ÿ'},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00},
	 {0x00,		 0x00,		 0x00,		 0x00,		0x00}},
	// IDS_MIME_UTF_8
	{{0xC5,		 0x92,		 0x00,		 0x00,		(UCHAR)'Œ'},
	 {0xC5,		 0x93,		 0x00,		 0x00,		(UCHAR)'œ'},
	 {0xC5,		 0xA0,		 0x00,		 0x00,		(UCHAR)'Š'},
	 {0xC5,		 0xA1,		 0x00,		 0x00,		(UCHAR)'š'},
	 {0xC5,		 0xB8,		 0x00,		 0x00,		(UCHAR)'Ÿ'},
	 {0xC5,		 0xBD,		 0x00,		 0x00,		(UCHAR)'Ž'},
	 {0xC5,		 0xBE,		 0x00,		 0x00,		(UCHAR)'ž'},
	 {0xC5,		 0xBF,		 0x00,		 0x00,		(UCHAR)'ƒ'},
	 {0xCB,		 0x82,		 0x00,		 0x00,		(UCHAR)'‹'},
	 {0xCB,		 0x83,		 0x00,		 0x00,		(UCHAR)'›'},
	 {0xCB,		 0x86,		 0x00,		 0x00,		(UCHAR)'ˆ'},
	 {0xCB,		 0x9C,		 0x00,		 0x00,		(UCHAR)'˜'},
	 {0xE2,		 0x80,		 0x93,		 0x00,		(UCHAR)'–'},
	 {0xE2,		 0x80,		 0x94,		 0x00,		(UCHAR)'—'},
	 {0xE2,		 0x80,		 0x9A,		 0x00,		(UCHAR)'‚'},
	 {0xE2,		 0x80,		 0x9E,		 0x00,		(UCHAR)'„'},
	 {0xE2,		 0x80,		 0xA0,		 0x00,		(UCHAR)'†'},
	 {0xE2,		 0x80,		 0xA1,		 0x00,		(UCHAR)'‡'},
	 {0xE2,		 0x80,		 0xA2,		 0x00,		(UCHAR)'•'},
	 {0xE2,		 0x80,		 0xA6,		 0x00,		(UCHAR)'…'},
	 {0xE2,		 0x80,		 0xB0,		 0x00,		(UCHAR)'‰'},
	 {0xE2,		 0x80,		 0xB2,		 0x00,		(UCHAR)'’'},
	 {0xE2,		 0x80,		 0xB3,		 0x00,		(UCHAR)'”'},
	 {0xE2,		 0x80,		 0xB5,		 0x00,		(UCHAR)'‘'},
	 {0xE2,		 0x80,		 0xB6,		 0x00,		(UCHAR)'“'},
	 {0xE2,		 0x82,		 0xAC,		 0x00,		(UCHAR)'€'},
	{0xE2,		 0x84,		 0xA2,		 0x00,		(UCHAR)'™'}}
};


////////////////////////////////////////////////////////////////////////
// CreateDIBPalette [static]
//
// Copy the DIB color table into the given CPalette object.
////////////////////////////////////////////////////////////////////////
static BOOL CreateDIBPalette(CPalette* pPalette, LPBITMAPINFO lpbmi, LPINT lpiNumColors)
{
	ASSERT(pPalette != NULL);
	ASSERT(NULL == pPalette->GetSafeHandle());
	ASSERT(lpbmi != NULL);
	ASSERT(lpiNumColors != NULL);

	LPBITMAPINFOHEADER lpbi = LPBITMAPINFOHEADER(lpbmi);
	if (lpbi->biBitCount <= 8)
		*lpiNumColors = (1 << lpbi->biBitCount);
	else
	{
		ASSERT(0);
		*lpiNumColors = 0;		// DIBs with more than 256 colors don't need a palette
	}

	BOOL status = FALSE;
	if (*lpiNumColors)
	{
		HANDLE hLogPal = ::GlobalAlloc (GHND, sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * (*lpiNumColors));
		LPLOGPALETTE lpPal = (LPLOGPALETTE) ::GlobalLock(hLogPal);
		lpPal->palVersion    = 0x300;
		lpPal->palNumEntries = ( WORD ) ( *lpiNumColors );

		for (int i = 0; i < *lpiNumColors; i++)
		{
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		if (pPalette)
		{
			status = pPalette->CreatePalette(lpPal);
			ASSERT(status);
			ASSERT(pPalette->GetSafeHandle());
		}
		::GlobalUnlock(hLogPal);
		::GlobalFree(hLogPal);
	}
	return status;
}


////////////////////////////////////////////////////////////////////////
// QCLoadDIBitmap [extern]
//
// Loads a resource bitmap as a device independent bitmap, complete
// with palette info (saved to the caller-provided CPalette object).
// This is used to manipulate the palette for 8-bit video drivers so
// that the colors in the bitmap are rendered properly instead of
// mapped to the nearest existing palette entry.
////////////////////////////////////////////////////////////////////////
HBITMAP QCLoadDIBitmap(LPCTSTR lpString, CPalette* pPalette)
{
	if (NULL == pPalette)
	{
		ASSERT(0);
		return NULL;
	}

	HBITMAP hBitmapFinal = NULL;

	HINSTANCE hInstance = QCFindResourceHandle(lpString, RT_BITMAP);
 
	HRSRC hRsrc = FindResource(hInstance, lpString, RT_BITMAP);
	if (hRsrc)
	{
		HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
		LPBITMAPINFOHEADER lpbi = LPBITMAPINFOHEADER(::LockResource(hGlobal));
 
		HDC hDesktopDC = ::GetDC(NULL);
		ASSERT(hDesktopDC);		// something is velly, velly wrongo
		if (hDesktopDC)
		{
			int iNumColors = 0;
			if (CreateDIBPalette(pPalette, (LPBITMAPINFO)lpbi, &iNumColors))
			{
				::SelectPalette(hDesktopDC, HPALETTE(pPalette->GetSafeHandle()), FALSE);
				::RealizePalette(hDesktopDC);
			}
			
			hBitmapFinal = ::CreateDIBitmap(hDesktopDC,
											(LPBITMAPINFOHEADER)lpbi,
											(LONG)CBM_INIT,
											(LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
											(LPBITMAPINFO)lpbi,
											DIB_RGB_COLORS );
	
			::ReleaseDC(NULL, hDesktopDC);
			hDesktopDC = NULL;
		}
 
		::UnlockResource(hGlobal);
		::FreeResource(hGlobal);
	}
	return hBitmapFinal;
}

BOOL QCLoadTextData(UINT uID, CString& TextData) 
{
	TextData.Empty();

	HINSTANCE	hInst = QCFindResourceHandle((LPCSTR)uID, "TEXTDATA");
	HRSRC		hResInfo;
	HGLOBAL		hResText;
	BOOL		bRet = FALSE;

	if (hInst && (hResInfo = ::FindResource(hInst, (LPCSTR)uID, "TEXTDATA")) &&
		(hResText = ::LoadResource(hInst, hResInfo)))
	{
		const char*	szTextData = (const char*)::LockResource(hResText);
		if (szTextData)
		{
			// Get the size of the resource so loaded
			DWORD dwTextDataLength = ::SizeofResource(hInst,hResInfo);		

			// Make sure that the string so formed is of the length of the resource expected to be loaded.			
			// Reason : When a resource gets loaded, it's loaded in chunks .. perhaps this was causing
			// a TEXTDATA (a binary resource) to be loaded with another TEXTDATA appended to it 
			// (IDT_MOREHELP_HTML was getting appended to IDT_USAGE_STATS_HTML when trying to load IDT_USAGE_STATS). 
			// This problem surfaced when Clockworks was localizing ver 5.0.2.1
			try 
			{
				CString csSizedTextData(szTextData, (int)dwTextDataLength);			
				TextData = csSizedTextData;				
				::UnlockResource(hResText);
				bRet = TRUE;

			}
			catch (CMemoryException * /* pMemoryException */)
			{
				// Catastrophic memory exception - rethrow
				ASSERT( !"Rethrowing CMemoryException in QCLoadTextData" );
				throw;
			}
			catch (CException * pException)
			{
				// Other MFC exception
				pException->Delete();
				ASSERT( !"Caught CException (not CMemoryException) in QCLoadTextData" );
				TextData = "";
				bRet = FALSE;
			}
			catch (std::bad_alloc & /* exception */)
			{
				// Catastrophic memory exception - rethrow
				ASSERT( !"Rethrowing std::bad_alloc in QCLoadTextData" );
				throw;
			}
			catch (std::exception & /* exception */)
			{
				ASSERT( !"Caught std::exception (not std::bad_alloc) in QCLoadTextData" );
				TextData = "";
				bRet = FALSE;
			}
		}
		::FreeResource(hResText);
	}

	return bRet;
}

BOOL QCCopyFile( LPCTSTR lpExistingFileName, 	// pointer to name of an existing file 
				 LPCTSTR lpNewFileName,			// pointer to filename to copy to 
				 BOOL bFailIfExists )
{
	return CopyFile( lpExistingFileName, lpNewFileName, bFailIfExists );
}

////////////////////////////////////////////////////////////////////////
// IsMatchingResourceDLL [extern]
//
// Peek at the VERSIONINFO resource in the EXE and DLL files and compare them.
// If the FILEVERSION values match, then return TRUE.
////////////////////////////////////////////////////////////////////////
BOOL IsMatchingResourceDLL(const char* exePathname,	//(i) Full pathname of this executable
						   const char* dllPathname)	//(i) Full pathname of resource DLL
{
	//
	// Do setup to prepare for the incoming version data.
	//
	DWORD handle1 = 0;
	DWORD size1 = ::GetFileVersionInfoSize((char *) exePathname, &handle1);
	if (NULL == size1)
		return FALSE;		// EXE doesn't have VERSIONINFO data?

	DWORD handle2 = 0;
	DWORD size2 = ::GetFileVersionInfoSize((char *) dllPathname, &handle2);
	if (NULL == size2)
		return FALSE;		// DLL doesn't have VERSIONINFO data?

	//
	// Allocate data buffers of the proper sizes.
	//
	LPSTR p_data1 = DEBUG_NEW_NOTHROW char[size1];
	if (NULL == p_data1)
		return FALSE;
	LPSTR p_data2 = DEBUG_NEW_NOTHROW char[size2];
	if (NULL == p_data2)
	{
		delete p_data1;
		return FALSE;
	}

	//
	// Now, fetch the actual VERSIONINFO data.
	//
	if ((! ::GetFileVersionInfo((char *) exePathname, handle1, size1, p_data1)) ||
		(! ::GetFileVersionInfo((char *) dllPathname, handle2, size2, p_data2)))
	{
		ASSERT(0);		// missing VERSIONINFO data?
		delete p_data1;
		delete p_data2;
		return FALSE;
	}

	//
	// Fetch and compare FileVersion values from each file.
	//
	LPSTR p_buf = NULL;
	UINT bufsize = 0;
	DWORD fileversion_ms1 = 0;		// most significant word
	DWORD fileversion_ls1 = 0;		// least significant word
	DWORD fileversion_ms2 = 0;
	DWORD fileversion_ls2 = 0;

	BOOL status = TRUE;				// innocent until proven guilty

	//
	// Okay, you would think that passing a raw string literal to an
	// argument that takes a string pointer would be okay.  Well, not
	// if you're running the 16-bit VerQueryValue() function under
	// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
	// the workaround is to copy the string to a local buffer first,
	// then pass the string in the buffer.  This, for some inexplicable
	// reason, works under all Windows OSes.
	//
	char kludgestr[60];
	strcpy(kludgestr, "\\");
	if (::VerQueryValue(p_data1,
						kludgestr, 
						(void **) &p_buf, 
						&bufsize))
	{
		if (p_buf && bufsize)
		{
			VS_FIXEDFILEINFO* p_info = (VS_FIXEDFILEINFO *) p_buf;	// type cast
			fileversion_ms1 = p_info->dwFileVersionMS;
			fileversion_ls1 = p_info->dwFileVersionLS;
		}
	}
	else
	{
		ASSERT(0);			// required base info data missing
		status = FALSE;
	}

	if (status)
	{
		p_buf = NULL;
		bufsize = 0;
		strcpy(kludgestr, "\\");
		if (::VerQueryValue(p_data2,
							kludgestr, 
							(void **) &p_buf, 
							&bufsize))
		{
			if (p_buf && bufsize)
			{
				VS_FIXEDFILEINFO* p_info = (VS_FIXEDFILEINFO *) p_buf;	// type cast
				fileversion_ms2 = p_info->dwFileVersionMS;
				fileversion_ls2 = p_info->dwFileVersionLS;
			}
		}
		else
		{
			ASSERT(0);			// required base info data missing
			status = FALSE;
		}
	}

	if (status)
	{
		TRACE2("IsMatchingResourceDLL: app_ms=0x%08X, dll_ms=0x%08X\n", fileversion_ms1, fileversion_ms2);
		TRACE2("IsMatchingResourceDLL: app_ls=0x%08X, dll_ls=0x%08X\n", fileversion_ls1, fileversion_ls2);
		if ((fileversion_ms1 != fileversion_ms2) ||
			(fileversion_ls1 != fileversion_ls2))
		{
			status = FALSE;
		}
	}

	if (status)
	{
		//
		// Okay, we now know that the version numbers match.  But,
		// what about the product names?
		//
		strcpy(kludgestr, "\\StringFileInfo\\040904E4\\ProductName");
		ASSERT(strlen(kludgestr) < sizeof(kludgestr));

		p_buf = NULL;
		bufsize = 0;
		CString strProductName1;
		CString strProductName2;
		if (::VerQueryValue(p_data1,
							kludgestr, 
							(void **) &p_buf, 
							&bufsize))
		{
			if (p_buf && bufsize)
				strProductName1 = (char *) p_buf;	// type cast
		}
		else
		{
			ASSERT(0);			// required base info data missing
			status = FALSE;
		}

		if (status)
		{
			p_buf = NULL;
			bufsize = 0;
			if (::VerQueryValue(p_data2,
								kludgestr, 
								(void **) &p_buf, 
								&bufsize))
			{
				if (p_buf && bufsize)
					strProductName2 = (char *) p_buf;	// type cast
			}
			else
			{
				ASSERT(0);			// required base info data missing
				status = FALSE;
			}
		}

		if (status)
		{
			TRACE2("IsMatchingResourceDLL: app=%s, dll=%s\n", (const char *) strProductName1, (const char *) strProductName2);
			if (strProductName1.IsEmpty() ||
				strProductName2.IsEmpty() ||
				strProductName1 != strProductName2)
			{
				status = FALSE;
			}
		}
	}


	//
	// All roads should lead here...
	//
	delete [] p_data1;
	delete [] p_data2;

	return status;
}


int IsFancy(const char* buf)
{
	int TextType = IS_ASCII;
	
	if (buf)
	{
		while (*buf)
		{
			if ( *buf++ == '<' )
			{
				if ( TextType != IS_FLOWED && strnicmp("x-flowed>", buf, 9) == 0 )
					TextType = IS_FLOWED;
				else if ( strnicmp("x-rich>", buf, 7) == 0 )
					return IS_RICH;
				else if ( strnicmp("x-html>", buf, 7) == 0 || strnicmp(buf, "html>", 5) == 0 )
					return IS_HTML;
			}
			buf = strchr(buf, '\n');
			if (!buf)
				break;
			buf++;
		}
	}

	return TextType;
}

int RichExcerptLevel(CRichEditCtrl* ctrl, int nChar)
{
	CHARFORMAT cf;
	PARAFORMAT pf;
	ctrl->SetSel(nChar,nChar);
	ctrl->GetSelectionCharFormat(cf);
	ctrl->GetParaFormat(pf);
	if ( cf.dwEffects & CFE_PROTECTED ) 
		return ( max(0,pf.cTabCount - 1) );
	else
		return 0;
}


/////////////////////////////////////////////////////////////////
//
//	AddMenuItem
//
//	Adds an item to a menu in alphabetical order.
//
//	Input:
//		UINT	uID			- the command id of the new item
//		LPCSTR	szMenuItem	- the string
//		HMENU	hMenu		- the menu
//		HMENU	hSubMenu	- if non-null, this popup will be inserted
//
//	Return Value:
//		None
//
UINT AddMenuItem(UINT uID, LPCSTR szMenuItem, HMENU hMenu, HMENU hSubMenu,
					INT iStartPos, INT iEndPos)
{
	INT		iVal;
	UINT	uCurrentID;
	INT		iPos;

	if (iEndPos > GetMenuItemCount(hMenu))
		iEndPos = GetMenuItemCount(hMenu);

	for (iPos = iStartPos; iPos < iEndPos; iPos++)
	{
		uCurrentID = GetMenuItemID(hMenu, iPos);

		if (uCurrentID != 0)
		{
			TCHAR szTemp[256];

			GetMenuString(hMenu, iPos, szTemp, sizeof(szTemp), MF_BYPOSITION);
			iVal = stricmp(szTemp, szMenuItem);
			if (iVal == 0)
				return iPos;
			if (iVal > 0)
				break;
		}
	}

	if (hSubMenu)
		InsertMenu(hMenu, iPos, MF_BYPOSITION | MF_POPUP, (UINT)hSubMenu, szMenuItem);
	else
		InsertMenu(hMenu, iPos, MF_BYPOSITION, uID, szMenuItem);

	WrapMenu(hMenu);

	return iPos;
}


/////////////////////////////////////////////////////////////////
//
//	RemoveMenuItem
//
//	Removes an item from a menu by string.
//
//	Input:
//		LPCSTR	szMenuItem	- the string
//		HMENU	hMenu		- the menu
//
//	Return Value:
//		the command ID of the removed menu item.
//
UINT RemoveMenuItem(LPCSTR szMenuItem, HMENU hMenu)
{
	INT		iPos;
	UINT	uReturnID = 0;

	for (iPos = 0; iPos < GetMenuItemCount(hMenu); iPos++)
	{
		TCHAR szTemp[256];

		GetMenuString(hMenu, iPos, szTemp, sizeof(szTemp), MF_BYPOSITION);
		if (stricmp(szTemp, szMenuItem) == 0)
			break;
	}

	if (iPos < GetMenuItemCount(hMenu))
	{
		uReturnID = GetMenuItemID(hMenu, iPos);
		RemoveMenu(hMenu, iPos, MF_BYPOSITION);
		WrapMenu(hMenu);
	}

	return uReturnID;
}


CMenu*	FindNamedMenuItem(const char* szFullNamedPath, CMenu* pMenu, INT* pIndex, BOOL bFindPopup)
{
	INT		i;
	CString szCurrentPath;
	CString	szPath;
	CMenu*	pNewMenu;
	UINT	uID;
		
	i = pMenu->GetMenuItemCount() - 1;
	szPath = "";

	while( ( i >= 0 ) && ( pMenu != NULL ) )
	{
		uID = pMenu->GetMenuItemID( i );

		pMenu->GetMenuString( i, szPath, MF_BYPOSITION );
		
		if( szCurrentPath != "" )
		{
			szPath = szCurrentPath + '\\' + szPath;
		}

		if( szPath.CompareNoCase( szFullNamedPath ) == 0 )
		{
			// we found it a match -- see if its a popup
			if( ( bFindPopup && ( ( INT ) ( pMenu->GetMenuItemID( i ) ) == -1 ) ) ||
				( !bFindPopup && ( ( INT ) ( pMenu->GetMenuItemID( i ) ) != -1 ) ) )
			{
				*pIndex = i;
				return pMenu;
			}
		}
		else
		if( ( uID == (UINT) -1 ) && 
			( strnicmp( szPath, szFullNamedPath, szPath.GetLength() ) == 0 ) )
		{
			// we found a parent

			szCurrentPath = szPath;

			pNewMenu = pMenu->GetSubMenu( i );

			if( pNewMenu )
			{
				pMenu = pNewMenu;
				i = pMenu->GetMenuItemCount() - 1;
				continue;
			}
		}

		i--;		
	}			

	return NULL;
}


#ifdef IMAP4

// 
// We need this because IMAP menu items may be case sensitive!!
// JOK - 7/2/97.
CMenu*	ImapFindNamedMenuItem(const char* szFullNamedPath, CMenu* pMenu, INT* pIndex, BOOL bFindPopup)
{
	INT		i;
	CString szCurrentPath;
	CString	szPath;
	CMenu*	pNewMenu;
	UINT	uID;
		
	i = pMenu->GetMenuItemCount() - 1;
	szPath = "";

	while( ( i >= 0 ) && ( pMenu != NULL ) )
	{
		uID = pMenu->GetMenuItemID( i );

		pMenu->GetMenuString( i, szPath, MF_BYPOSITION );
		
		if( szCurrentPath != "" )
		{
			szPath = szCurrentPath + '\\' + szPath;
		}

		if( szPath.Compare( szFullNamedPath ) == 0 )
		{
			// we found it a match -- see if its a popup
			INT mID = (INT) pMenu->GetMenuItemID( i );

			if( ( bFindPopup && (mID == -1) ) || ( !bFindPopup && (mID != -1) ) )
			{
				*pIndex = i;
				return pMenu;
			}
		}
		else
		if( ( uID == (UINT) -1 ) && 
			( strncmp( szPath, szFullNamedPath, szPath.GetLength() ) == 0 ) )
		{
			// we found a parent

			szCurrentPath = szPath;

			pNewMenu = pMenu->GetSubMenu( i );

			if( pNewMenu )
			{
				pMenu = pNewMenu;
				i = pMenu->GetMenuItemCount() - 1;
				continue;
			}
		}

		i--;		
	}			

	return NULL;
}

#endif // IMAP4


////////////////////////////////////////////////////////////////////
//
//	WrapMenu
//
//  Inserts MENUBARBREAKs in large menus so that they wrap.
//  Can be disabled with <x-Eudora-option:WrapMenus=0> if you're
//  using a newer OS (Win 98/2000), and like the scrolling menus
//  that are provided.
//
void  WrapMenu(HMENU hMenu)
{
	if (!GetIniShort(IDS_INI_WRAP_MENUS))
		return;

	INT				iLen;
	char			szBuf[200];
	UINT			uPosition;
	UINT			uCount;
	MENUITEMINFO	theInfo;
	HMENU			hSubMenu;
	UINT			uScreenHeight;
	UINT			uMenuHeight;
	UINT			uItemHeight;
	UINT			uBaseMenuHeight;
	TEXTMETRIC		tm;

	VERIFY( GetTextMetrics((HFONT)::GetStockObject( SYSTEM_FONT ), tm ) );
	
	uItemHeight = tm.tmHeight + tm.tmExternalLeading + 2;
	uScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	uBaseMenuHeight =	::GetSystemMetrics( SM_CYCAPTION ) + 
						::GetSystemMetrics( SM_CYFRAME ) + 
						uItemHeight;
	
	uMenuHeight = uBaseMenuHeight;

	uCount = ::GetMenuItemCount( hMenu );

	for( uPosition = 0; uPosition < uCount; uPosition++ )
	{
		memset( &theInfo, 0, sizeof( theInfo ) );

		theInfo.cbSize = sizeof( MENUITEMINFO ); 
		theInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;

		VERIFY( ::GetMenuItemInfo( hMenu, uPosition, TRUE, &theInfo ) );

		uMenuHeight += uItemHeight;
		hSubMenu = theInfo.hSubMenu;

		if( uMenuHeight > uScreenHeight )
		{
			if( ( theInfo.fType & MFT_MENUBARBREAK ) == 0 )
			{
				// wrap the menu
				theInfo.fMask = MIIM_TYPE;
				theInfo.fType |= MFT_MENUBARBREAK;
				iLen = ::GetMenuString( hMenu, uPosition, szBuf, sizeof( szBuf ), MF_BYPOSITION );
				if( iLen )
				{
					theInfo.dwTypeData = szBuf;
					theInfo.cch = iLen;
				}
				VERIFY( ::SetMenuItemInfo( hMenu, uPosition, TRUE, &theInfo ) );
			}

			// reset the height
			uMenuHeight = uBaseMenuHeight + uItemHeight;
		}
		else if( ( theInfo.fType & MFT_MENUBARBREAK ) != 0 )
		{
			// unwrap the menu
			theInfo.fMask = MIIM_TYPE;
			theInfo.fType &= ~MFT_MENUBARBREAK;
			iLen = ::GetMenuString( hMenu, uPosition, szBuf, sizeof( szBuf ), MF_BYPOSITION );
			if( iLen )
			{
				theInfo.dwTypeData = szBuf;
				theInfo.cch = iLen;
			}
			VERIFY( ::SetMenuItemInfo( hMenu, uPosition, TRUE, &theInfo ) );
		}

		if( hSubMenu )
		{
			WrapMenu( hSubMenu );
		}
	}
}


void ReadPlatform(CString& sPlatform, CString& sVer, CString* sMachineType /*= NULL*/,
				  CString* sProcessor /*= NULL*/, CString* sTotalPhys /*= NULL*/,
				  CString* csMSHTMLVersion/*= NULL*/, CString* csWININETVersion/*= NULL*/)
{
	sVer.Format("%lu.%02lu.%04u", GetMajorVersion(), GetMinorVersion(), LOWORD(GetBuildNumber()));
	
	// Have to be called in this kind of order.
	// I didn't want to grep thru the code and make a cechk 98 || 98SE || ME
	// So IsWin98 returns true 98 and up
	// IsWinNT returns true NT and up thru 2k
	// Calling the newer more specific functions first will ensure that
	// we don't get to the generic TRUE's
	UINT uOSID = 0;
	if (IsWinVista())
		uOSID = IDS_SYSINFO_WINVISTA;
	else if (IsWinServer2003())
		uOSID = IDS_SYSINFO_WINSERVER2003;
	else if (IsWinXP())
		uOSID = IDS_SYSINFO_WINXP;
	else if (IsWinNT2000())
		uOSID = IDS_SYSINFO_WIN2K;
	else if (IsWinNT())
		uOSID = IDS_SYSINFO_WINNT;
	else if (IsWinME())
		uOSID = IDS_SYSINFO_WINME;
	else if (IsWin98SE())
		uOSID = IDS_SYSINFO_WIN98SE;
	else if (IsWin98())
		uOSID = IDS_SYSINFO_WIN98;
	else if (IsWin95())
		uOSID = IDS_SYSINFO_WIN95;
	else if (IsWin32s())
		uOSID = IDS_SYSINFO_WIN32S;
	if (uOSID)
	{
		sPlatform = CRString(uOSID);

		// Don't add service pack for Win 98 SE (which is "A") because
		// we've already incorporated that info in the name ("SE").
		if (uOSID != IDS_SYSINFO_WIN98SE)
		{
			LPCTSTR pServicePack = GetServicePack();
			if (pServicePack && *pServicePack)
			{
				sPlatform += ' ';
				sPlatform += pServicePack;
			}
		}
	}
	
	if (sMachineType && sProcessor)
	{
		char stemp[20]={0};
		SYSTEM_INFO sinf;
		GetSystemInfo(&sinf);
		switch(sinf.wProcessorArchitecture)
		{
			case PROCESSOR_ARCHITECTURE_INTEL:
				*sMachineType = CRString(IDS_SYSINFO_PROCESS_INTEL);
				switch(sinf.wProcessorLevel)
				{
					case 3:
						*sProcessor = "80386";
						break;
					case 4:
						*sProcessor = "80486";
						break;
					case 5:
						*sProcessor = "Pentium";
						break;
					case 6:
						*sProcessor = "Pentium Pro, II, III, or 4";
						if (IsWinNT()) // We can get Step info.
						{
							DWORD dwModel, dwStep;
							dwModel = sinf.wProcessorRevision & 0x0000ff00;
							dwStep = sinf.wProcessorRevision & 0x00ff;
							switch(dwModel)
							{
							case 0x0400:
							  *sProcessor += " Model 4, ";
							  break;
							case 0x0500:
							  *sProcessor += " Model 5, ";
							  break;
							case 0x0600:
							  *sProcessor += " Model 6, ";
							  break;
							case 0x0700:
							  *sProcessor += " Model 7, ";
							  break;
							case 0x0800:
							  *sProcessor += " Model 8, ";
							  break;
							case 0x0900:
							  *sProcessor += " Model 9, ";
							  break;

							default:
								*sProcessor += " Model ";
								*sProcessor += CString(ultoa(dwModel, stemp, 10));
								*sProcessor += ", ";
							  break;
							}
							switch(dwStep)
							{
							case 2:
							  *sProcessor += "Stepping 2";
							  break;
							case 3:
							  *sProcessor += "Stepping 3";
							  break;
							case 4:
							  *sProcessor += "Stepping 4";
							  break;
							case 5:
							  *sProcessor += "Stepping 5";
							  break;
							case 6:
							  *sProcessor += "Stepping 6";
							  break;

							default:
								*sProcessor += " Stepping ";
								*sProcessor += CString(ultoa(dwStep, stemp, 10));
							  break;
							}


						}

						break;
	/*				case 7:
						*sProcessor = "Pentium III"; //P3 is also family 6
						break;
	*/				
					case 15:
						*sProcessor = "Pentium 4";
						break;
					default:
						*sProcessor = CString(ultoa(sinf.dwProcessorType, stemp, 10));
						break;
				}
				break;

			case PROCESSOR_ARCHITECTURE_MIPS:
				*sMachineType = CRString(IDS_SYSINFO_PROCESS_MIPS);
				*sProcessor = CString(ultoa(sinf.dwProcessorType, stemp, 10)) + " " +
							CString(itoa(sinf.wProcessorLevel, stemp, 10));
				break;

			case PROCESSOR_ARCHITECTURE_ALPHA:
				*sMachineType = CRString(IDS_SYSINFO_PROCESS_ALPHA);
				*sProcessor = CString(ultoa(sinf.dwProcessorType, stemp, 10)) + " " +
							CString(itoa(sinf.wProcessorLevel, stemp, 10));
				break;

			case PROCESSOR_ARCHITECTURE_PPC:
				*sMachineType = CRString(IDS_SYSINFO_PROCESS_PPC);
				switch(sinf.wProcessorLevel)
				{
					case 1:
						*sProcessor = "601";
						break;
					case 3:
						*sProcessor = "603";
						break;
					case 4:
						*sProcessor = "604";
						break;
					case 6:
						*sProcessor = "603+";
						break;
					case 9:
						*sProcessor = "604+";
						break;
					case 20:
						*sProcessor = "620";
						break;
					default:
						*sProcessor = *sMachineType;
						break;
				}
				break;
			default:
				*sMachineType = CRString(IDS_SYSINFO_PROCESS_UNKNOWN);
				*sProcessor = *sMachineType;
		}
	}

	if (sTotalPhys)
	{
		MEMORYSTATUS mst;
		mst.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&mst);
		sTotalPhys->Format("%lu MB", (mst.dwTotalPhys / (1024L * 1024L)));
	}

	int MajorVersion;
	int MinorVersion;
	if(csMSHTMLVersion)
	{
		if(GetMSHTMLVersion( MajorVersion, MinorVersion))
			csMSHTMLVersion->Format("%lu.%02lu",MajorVersion,MinorVersion);
	}
	if(csWININETVersion)
	{
		if(GetWININETVersion( MajorVersion, MinorVersion))
			csWININETVersion->Format("%lu.%02lu",MajorVersion,MinorVersion);
	}
}


//
//	ISOTranslate
//
//	Translates the specified buffer using the specified table.
//
//	Parameters:
//		szBuf[in/out] - Buffer to translate.
//		lSize[in] - Size of szBuf.
//		iCharsetIdx[in] - Index of charset to translate from.
//			This value should be whatever is returned by FindRStringIndexI()
//			when it is passed the appropriate charset string.
//
//	Return:
//		The size of the translated buffer.  The translated buffer
//		might be smaller than the original but will never be larger.
//
LONG ISOTranslate(LPTSTR szBuf, LONG lSize, UINT iCharsetIdx)
{
	// Bail if there is no buffer or no table.
	if (!szBuf)
	{
		return 0;
	}

	// An index of 0 is Windows, 1 is US ASCII and 2 is Latin1, none of which need
	// any translation.  Exit now if no translation needed.
	if (iCharsetIdx <= 2)
	{
		return lSize;
	}

	// We are still here so translation is needed.  Bump the index down by
	// 3 since Windows, US_ASCII and ISO_LATIN1 do not have entries in the table.
	iCharsetIdx -= 3;

	// Make sure there is a table entry for this character set
	if (iCharsetIdx >= MAX_CHARACTER_SETS)
	{
		ASSERT(0);	// Who gave me this bad data?
		return lSize;
	}

	// Make sure the input text is NULL terminated.
	//
	// Bug note - this NULL termination business is a bit nasty. TextReader::ReadIt
	// was filling up its entire buffer with text and then calling us. NULL terminating
	// here was writing one past the end of the buffer. I'd like to do away with this
	// NULL termination (because TextReader doesn't seem to need it - it relies on
	// the buffer size), but I can't easily be sure of the implications of that.
	// Instead TextReader::ReadIt is now making sure that it leaves space at the
	// end of its buffer for us to NULL terminate it.
	szBuf[lSize] = 0;

	// Copy the buffer into a CString for easier string substitution.
	CString			 csTranslated(szBuf);

	unsigned char	 szTransTo[2] = {'\0', '\0'};

	// Iterate through the translation table replacing occurrences
	// of target strings with their replacements.
	for (int i = 0; i < XLATE_CHARS; ++i)
	{
		if (pcXlateTable[iCharsetIdx][i][0] == '\0')
		{
			// We have reached an empty entry: no more strings to translate.
			break;
		}

		// Place the last item in the table's row in the string
		// to replace the above string.
		szTransTo[0] = pcXlateTable[iCharsetIdx][i][MAX_CHARS_TO_TRANS + 1];

		// Note that this might decrease the size of the text but will never
		// increase the size because the replacement string is always
		// exactly one character long.
		csTranslated.Replace((char*)pcXlateTable[iCharsetIdx][i], (char*)szTransTo);
	}

	// Recalculate the text length in case we got shorter.
	lSize = csTranslated.GetLength();

	// Copy the translated text back into the buffer.
	strncpy(szBuf, csTranslated, lSize);
	
	// Make sure the text is NULL terminated.
	szBuf[lSize] = 0;

	return strlen(szBuf);
}


BOOL GetFileVersion(LPCTSTR szDLLName, DWORD& dwMajorVersion, DWORD& dwMinorVersion)
{
	dwMajorVersion = 0;
	dwMinorVersion = 0;

	TCHAR szString[1024] = {0};
	BOOL bRet = FALSE;

	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize((LPTSTR)szDLLName, &dwHandle);
	
	if (dwSize)
	{
		LPSTR pData = DEBUG_NEW_NOTHROW char[dwSize];
		VS_FIXEDFILEINFO* pFileInfo = NULL;
		UINT uBufSize = 0;

		if (pData && GetFileVersionInfo((LPTSTR)szDLLName, dwHandle, dwSize, pData)) 
		{
			//
			// Okay, you would think that passing a raw string literal to an
			// argument that takes a string pointer would be okay.  Well, not
			// if you're running the 16-bit VerQueryValue() function under
			// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
			// the workaround is to copy the string to a local buffer first,
			// then pass the string in the buffer.  This, for some inexplicable
				// reason, works under all Windows OSes.
			//
			strcpy(szString, "\\");
			
			if (VerQueryValue(pData, szString, (void **)&pFileInfo, &uBufSize) && uBufSize && pFileInfo)
			{
				// now check the version
				dwMajorVersion = pFileInfo->dwFileVersionMS >> 16;
				dwMinorVersion = pFileInfo->dwFileVersionMS & 0xFFFF;
				bRet = TRUE;
			}
		}

		delete [] pData;
	}	

	return bRet;
}

BOOL GetSystemDLLVersion(LPCTSTR SystemDLLName, int& MajorVersion, int& MinorVersion)
{
	MajorVersion = 0;
	MinorVersion = 0;

	TCHAR szString[1024] = {0};
	BOOL bRet = FALSE;

	if (GetSystemDirectory(szString, sizeof(szString)))
	{
		CString szDllPathname(szString);

		szDllPathname += '\\';
		szDllPathname += SystemDLLName;

		bRet = GetFileVersion(szDllPathname, (DWORD &)MajorVersion, (DWORD &)MinorVersion);
	}

	return bRet;
}

BOOL GetMSHTMLVersion(int& MajorVersion, int& MinorVersion)
{
	static int sMajorVersion = -1;
	static int sMinorVersion = -1;

	if (sMajorVersion == -1 && sMinorVersion == -1)
		GetSystemDLLVersion("mshtml.dll", sMajorVersion, sMinorVersion);

	MajorVersion = sMajorVersion;
	MinorVersion = sMinorVersion;

	return (MajorVersion > 0) || (MinorVersion > 0);
}

BOOL IsAtLeastMSHTML4_71()
{
	int MajorVersion;
	int MinorVersion;

	GetMSHTMLVersion(MajorVersion, MinorVersion);

	if (MajorVersion > 4 || (MajorVersion == 4 && MinorVersion >= 71))
		return TRUE;

	return FALSE;
}

BOOL GetWININETVersion(int& MajorVersion, int& MinorVersion)
{
	static int sMajorVersion = -1;
	static int sMinorVersion = -1;

	if (sMajorVersion == -1 && sMinorVersion == -1)
		GetSystemDLLVersion("wininet.dll", sMajorVersion, sMinorVersion);

	MajorVersion = sMajorVersion;
	MinorVersion = sMinorVersion;

	return (MajorVersion > 0) || (MinorVersion > 0);
}

BOOL IsAtLeastWININET4_71()
{
	int MajorVersion;
	int MinorVersion;

	GetWININETVersion(MajorVersion, MinorVersion);

	if (MajorVersion > 4 || (MajorVersion == 4 && MinorVersion >= 71))
		return TRUE;

	return FALSE;
}

BOOL UsingTrident()
{
	return ( GetIniShort(IDS_INI_USE_BIDENT_ALWAYS) && IsAtLeastMSHTML4_71() );
}

BOOL MSHTMLHasPrintPreview()
{
	int MajorVersion;
	int MinorVersion;

	GetMSHTMLVersion(MajorVersion, MinorVersion);

	// Print Preview was introduced in the 5.5 version
	if (MajorVersion > 5 || (MajorVersion == 5 && MinorVersion >= 50))
		return TRUE;

	return FALSE;
}


BOOL AttachmentsToClipboard(LPCTSTR AttachList, BOOL bIsSingle /*= TRUE*/, COleDataSource* pDS /*= NULL*/)
{
	if (!AttachList || !*AttachList)
		return FALSE;

	BOOL bCreatedDataSource = FALSE;
	if (!pDS)
	{
		pDS = DEBUG_NEW COleDataSource;
		bCreatedDataSource = TRUE;
	}

	CString FilePath;
	LPCTSTR AttachStart = AttachList;
	DWORD dwAttachListBytes = 0;

	if (bIsSingle)
	{
		if (strnicmp(AttachList, "file://", 7) == 0)
		{
			FilePath = UnEscapeURL(AttachList);
			AttachStart = FilePath;
			AttachStart += 7;
		}
		dwAttachListBytes = strlen(AttachStart) + 1;
	}
	else
	{
		LPCTSTR AttachEnd = AttachStart;
		while (*AttachEnd)
			AttachEnd += strlen(AttachEnd) + 1;
		dwAttachListBytes = AttachEnd - AttachStart;
	}

	// Put Preferred Drop Effect on the clipboard (DROPEFFECT_COPY)
	HGLOBAL hglb = GlobalAlloc(GMEM_ZEROINIT | GMEM_DDESHARE, sizeof(DWORD));
	DWORD* pdw1 = (DWORD*)GlobalLock(hglb);
	(*pdw1) = DROPEFFECT_COPY;
	GlobalUnlock(hglb);
	pDS->CacheGlobalData((CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hglb);

	// A Little HDROP Documentation
	//
	// An HDROP is a global handle to a DROPFILES structure with a double-null terminated
	// list of strings appended at the end. "pFiles" in DROPFILES is simply set to
	// sizeof(DROPFILES), and then whack your filepath(s) on at the end of the structure.
	// All other fields of DROPFILES can be set to zero. Now put that thing on
	// the Clipboard!
	//
	// Note:
	//	 * If doing drag/drop "pt" is set to the drop point (mouse ptr coords).
	//	 * If paths are wide chars "fWide" is set to TRUE.
	//	 * On NT, HDROPS from the system have wide char paths. You don't *have* to do this
	//	   unless you need/want to do unicode. On the recieving end (drop/paste)
	//	   DragQueryFile does the W2A translation for you.
	//
	// List of strings example:
	//	 This is string one\0Here's string two\0String three\0\0  <--- double null.
	//
	DWORD dwBytes = (sizeof(DROPFILES) + dwAttachListBytes + 1);
	hglb = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, dwBytes);
	BYTE* pBytes = (BYTE*)GlobalLock(hglb);
	((DROPFILES*)pBytes)->pFiles = sizeof(DROPFILES);
	memcpy(pBytes + sizeof(DROPFILES), AttachStart, dwAttachListBytes);
	GlobalUnlock(hglb);
	pDS->CacheGlobalData(CF_HDROP, hglb);

	// Now put the full path of the first attachment on the clipboard in CF_TEXT format
	dwBytes = strlen(AttachStart) + 1;
	hglb = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, dwBytes);
	pBytes = (BYTE*)GlobalLock(hglb);
	memcpy(pBytes, AttachStart, dwBytes);
	GlobalUnlock(hglb);
	pDS->CacheGlobalData(CF_TEXT, hglb);

	// We're all done with the clipboard if we created the data source,
	// otherwise let the caller do some more to it before it gets set in stone
	if (bCreatedDataSource)
		pDS->SetClipboard();

	return TRUE;
}
