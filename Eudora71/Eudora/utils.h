// UTILS.H
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

#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>

#ifndef _IS_
#define _IS_
#define IS_ASCII	0
#define IS_FLOWED	1
#define IS_RICH		2
#define IS_HTML		3
#endif


inline HINSTANCE QCFindResourceHandle(LPCTSTR lpszName, LPCTSTR lpszType)
	{ return AfxFindResourceHandle(lpszName, lpszType); }

inline HCURSOR QCLoadCursor(LPCTSTR lpCursorName)
	{ return ::LoadCursor(QCFindResourceHandle(lpCursorName, RT_GROUP_CURSOR), lpCursorName); }
inline HCURSOR QCLoadCursor(UINT uCursor)
	{ return QCLoadCursor(MAKEINTRESOURCE(uCursor)); }
inline HCURSOR QCSetCursor(UINT uCursor)
	{ return ::SetCursor(QCLoadCursor(uCursor)); }

inline HICON QCLoadIcon(LPCTSTR lpIconName)
	{ return ::LoadIcon(QCFindResourceHandle(lpIconName, RT_GROUP_ICON), lpIconName); }
inline HICON QCLoadIcon(UINT uIcon)
	{ return QCLoadIcon(MAKEINTRESOURCE(uIcon)); }

inline HMENU QCLoadMenu(LPCTSTR lpMenuName)
	{ return ::LoadMenu(QCFindResourceHandle(lpMenuName, RT_MENU), lpMenuName); }
inline HMENU QCLoadMenu(UINT uMenu)
	{ return QCLoadMenu(MAKEINTRESOURCE(uMenu)); }

inline int QCLoadString(UINT uID, LPTSTR lpBuffer, int nBufferMax)
	{ return AfxLoadString(uID, lpBuffer, nBufferMax); }


HBITMAP QCLoadDIBitmap( LPCTSTR lpString, CPalette* pPalette );

BOOL QCLoadTextData(UINT uID, CString& TextData);


BOOL QCCopyFile( LPCTSTR lpExistingFileName, 	// pointer to name of an existing file 
				 LPCTSTR lpNewFileName,			// pointer to filename to copy to 
				 BOOL bFailIfExists );

BOOL IsMatchingResourceDLL(const char* exePathname,		//(i) Full pathname of this executable
						   const char* dllPathname);	//(i) Full pathname of resource DLL

UINT AddMenuItem(UINT uID, LPCSTR szMenuItem, HMENU hMenu, HMENU hSubMenu = NULL, INT iStartPos = 0, INT iEndPos = INT_MAX);
UINT RemoveMenuItem(LPCSTR szMenuItem, HMENU hMenu);
CMenu* FindNamedMenuItem(const char* szFullNamedPath, CMenu* pMenu, INT* pIndex, BOOL bFindPopup);
CMenu* ImapFindNamedMenuItem(const char* szFullNamedPath, CMenu* pMenu, INT* pIndex, BOOL bFindPopup);
void WrapMenu(HMENU hMenu);

void ReadPlatform(CString& sPlatform, CString& sVer, CString* sMachineType = NULL,
				  CString* sProcessor = NULL, CString* sTotalPhys = NULL,
				  CString* csMSHTLVersion= NULL, CString* csWININETVersion= NULL);

int IsFancy(const char* buf);
int RichExcerptLevel(CRichEditCtrl* ctrl, int nChar);

// Character set translation
LONG ISOTranslate(LPTSTR szBuf, LONG lSize, UINT iCharsetIdx);

// Generic Function for retrieving a file's version
BOOL GetFileVersion(LPCTSTR szDLLName, DWORD& dwMajorVersion, DWORD& dwMinorVersion);

// Functions telling versions of system DLLs, and specific ones for wininet.dll and mshtml.dll
BOOL GetSystemDLLVersion(LPCTSTR SystemDLLName, int& MajorVersion, int& MinorVersion);
BOOL GetMSHTMLVersion(int& MajorVersion, int& MinorVersion);
BOOL IsAtLeastMSHTML4_71();
BOOL GetWININETVersion(int& MajorVersion, int& MinorVersion);
BOOL IsAtLeastWININET4_71();
BOOL UsingTrident();
BOOL MSHTMLHasPrintPreview();

BOOL AttachmentsToClipboard(LPCTSTR AttachList, BOOL bIsSingle = TRUE, COleDataSource* pDS = NULL);

#endif //_UTILS_H
