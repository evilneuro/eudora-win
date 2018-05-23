// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	Declaration for old-style Thumbnail dialog
//  Created:		Fri Jun 30 16:19:42 1995

#ifndef __SECTNFILEDLG_H__
#define __SECTNFILEDLG_H__

#if !defined(WIN32) && !defined(_INC_TOOLHELP)
#include /* */ "toolhelp.h"
#endif

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#ifndef WIN32
typedef UINT (CALLBACK *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
#endif //WIN32

class SECTNBitmap; //predeclaration
class SECTNFileDialog : public CFileDialog
{
    DECLARE_DYNAMIC(SECTNFileDialog);
   
// Construction
public:
    SECTNFileDialog(BOOL bOpenFileDialog,
		    LPCTSTR lpszDefExt = NULL,
		    LPCTSTR lpszFileName= NULL,
		    DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		    LPCTSTR lpszFilter = NULL,
		    CWnd * pParentWnd = NULL);
    
// Attributes
// Operations
// Implementation    
public:
    TCHAR m_szCurrentFile[MAX_PATH+1]; //For paints
    CStatic * m_pStatic;  //Pointer to our static where TN displayed.
    BOOL m_bExplorerStyle;
#ifdef _UNICODE
    void SetCurrentFile(const unsigned short* lpszNewFile)
#else
    void SetCurrentFile(LPCTSTR lpszNewFile)
#endif
		{_tcscpy(m_szCurrentFile,lpszNewFile);};

    // Load the thumbnail from a filename
#ifdef WIN32    
    SECTNBitmap * LoadThumbnail(LPCTSTR);
#else //WIN16
    SECTNBitmap * LoadThumbnail(const TCHAR *);
#endif //WIN32
    
    // Thumbnail drawing routines
    void ClearThumbnail(CDC *,CRect *);
    void DisplayNoThumbnail(CDC *,CRect *);
    void DisplayThumbnail(CDC *,CRect *, SECTNBitmap *);
    void UpdateThumbnail();
    void OnNewFile(LPTSTR lpszFileName);

    // Hooks - only needed/used for Win32 commdlgs.
#ifdef WIN32    
    static BOOL CALLBACK TNFileDlgHookProc(HWND,UINT,WPARAM,LPARAM);
    static SECTNFileDialog * StaticGetPointer(HWND);
#endif //WIN32

//Overrides
public:
    virtual ~SECTNFileDialog(void);
    virtual void OnLBSelChangedNotify(UINT,UINT,UINT);
    virtual BOOL OnInitDialog();
    virtual int DoModal();
    void ResizeDialog();
    
#if _MFC_VER >= 0x400
    void OnFileNameChange();
    void OnFolderChange();
#endif //_MFC_VER >= 0x400
    
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECTNFILEDLG_H__
