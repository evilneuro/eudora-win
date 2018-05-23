// UTILS.CPP
//
// General utilties
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

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



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
BOOL IsMatchingResourceDLL(const CString& exePathname,	//(i) Full pathname of this executable
						   const CString& dllPathname)	//(i) Full pathname of resource DLL
{
	//
	// Do setup to prepare for the incoming version data.
	//
	DWORD handle1 = 0;
	DWORD size1 = ::GetFileVersionInfoSize((char *)(const char *) exePathname, &handle1);
	if (NULL == size1)
		return FALSE;		// EXE doesn't have VERSIONINFO data?

	DWORD handle2 = 0;
	DWORD size2 = ::GetFileVersionInfoSize((char *)(const char *) dllPathname, &handle2);
	if (NULL == size2)
		return FALSE;		// DLL doesn't have VERSIONINFO data?

	//
	// Allocate data buffers of the proper sizes.
	//
	LPSTR p_data1 = new char[size1];
	if (NULL == p_data1)
		return FALSE;
	LPSTR p_data2 = new char[size2];
	if (NULL == p_data2)
	{
		delete p_data1;
		return FALSE;
	}

	//
	// Now, fetch the actual VERSIONINFO data.
	//
	if ((! ::GetFileVersionInfo((char *)(const char *) exePathname, handle1, size1, p_data1)) ||
		(! ::GetFileVersionInfo((char *)(const char *) dllPathname, handle2, size2, p_data2)))
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
//	void AddMenuItem(
//	UINT	uID,
//	LPCSTR	szMenuItem,
//	CMenu*	pMenu,
//	CMenu*	pSubMenu = NULL )
//
//	Adds an item to a menu in alphabetical order.
//
//	Input:
//		UINT	uID			- the command id of the new item
//		LPCSTR	szMenuItem	- the string
//		CMenu*	pMenu		- the menu
//		CMenu*	pSubMenu	- if non-null, this popup will be inserted
//
//	Return Value:
//		None
//
UINT AddMenuItem(
UINT	uID,
LPCSTR	szMenuItem,
CMenu*	pMenu,
CMenu*	pSubMenu,
INT		iStartPos,
INT		iEndPos )
{
	CString	szTemp;
	INT		iVal;
	UINT	uCurrentID;
	INT		iPos;

	if( ( UINT ) iEndPos > pMenu->GetMenuItemCount() )
	{
		iEndPos = pMenu->GetMenuItemCount();
	}

	for( iPos = iStartPos; iPos < iEndPos; iPos++ )
	{
		uCurrentID = pMenu->GetMenuItemID( iPos );

		if( uCurrentID != 0 )
		{
			pMenu->GetMenuString( iPos, szTemp, MF_BYPOSITION );
			
			iVal = szTemp.CompareNoCase( szMenuItem );
			
			if( iVal == 0 )
			{
				return iPos;
			}
			
			if( iVal > 0 )
			{
				break;
			}
		}
	}
	
	if( pSubMenu != NULL )
	{
		pMenu->InsertMenu( iPos, MF_BYPOSITION | MF_POPUP, (UINT) pSubMenu->GetSafeHmenu(), szMenuItem );		
	}
	else
	{
		pMenu->InsertMenu( iPos, MF_BYPOSITION, uID, szMenuItem );
	}

	WrapMenu( pMenu->GetSafeHmenu() );

	return iPos;
}


/////////////////////////////////////////////////////////////////
//
//	void RemoveMenuItem(
//	LPCSTR	szMenuItem,
//	CMenu*	pMenu )
//
//	Removes an item from a menu by string.
//
//	Input:
//		LPCSTR	szMenuItem	- the string
//		CMenu*	pMenu		- the menu
//
//	Return Value:
//		the command ID of the removed menu item.
//
UINT RemoveMenuItem(
LPCSTR	szMenuItem,
CMenu*	pMenu )
{
	INT		iPos;
	CString	szTemp;
	UINT	uReturn;

	for( iPos = 0; (UINT) iPos < pMenu->GetMenuItemCount(); iPos++ )
	{
		pMenu->GetMenuString( iPos, szTemp, MF_BYPOSITION );
		
		if( szTemp == szMenuItem )
		{
			break;
		}
	}
	
	if( ( UINT ) iPos < pMenu->GetMenuItemCount() )
	{
		uReturn = pMenu->GetMenuItemID( iPos );
		pMenu->RemoveMenu( iPos, MF_BYPOSITION );
		WrapMenu( pMenu->GetSafeHmenu() );
		return uReturn;
	}

	return 0;
}


CMenu*	FindNamedMenuItem(
CString szFullNamedPath,
CMenu*	pMenu,
INT*	pIndex,
BOOL	bFindPopup )
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

		if( szFullNamedPath.CompareNoCase( szPath ) == 0 )
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
CMenu*	ImapFindNamedMenuItem(
CString szFullNamedPath,
CMenu*	pMenu,
INT*	pIndex,
BOOL	bFindPopup )
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

		if( szFullNamedPath.Compare( szPath ) == 0 )
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
//	void  WrapMenu(
//	CMenu* pMenu )
//
//  Inserts M

void  WrapMenu(
HMENU	hMenu)
{
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
