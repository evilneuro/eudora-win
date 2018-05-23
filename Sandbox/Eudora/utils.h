// UTILS.H
//
// General utilties
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


BOOL QCCopyFile( LPCTSTR lpExistingFileName, 	// pointer to name of an existing file 
				 LPCTSTR lpNewFileName,			// pointer to filename to copy to 
				 BOOL bFailIfExists );

BOOL IsMatchingResourceDLL(const CString& exePathname,	//(i) Full pathname of this executable
						   const CString& dllPathname);	//(i) Full pathname of resource DLL

UINT AddMenuItem(UINT uID, LPCSTR szMenuItem, CMenu* pMenu, CMenu* pSubMenu = NULL, INT iStartPos = 0, INT iEndPos = INT_MAX);
UINT RemoveMenuItem(LPCSTR szMenuItem, CMenu* pMenu);
CMenu* FindNamedMenuItem(CString szFullNamedPath, CMenu* pMenu, INT* pIndex, BOOL bFindPopup);
void WrapMenu(HMENU hMenu);
CMenu* ImapFindNamedMenuItem(CString szFullNamedPath, CMenu* pMenu, INT* pIndex, BOOL bFindPopup);

int IsFancy(const char* buf);
int RichExcerptLevel(CRichEditCtrl* ctrl, int nChar);

#endif //_UTILS_H
