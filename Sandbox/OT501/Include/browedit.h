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
//  Author:                     John Williston
//  Description:        Declarations for SECDirSelectDlg, SECBrowseEditBase
//                     SECBrowseDirEdit and SECBrowseFileEdit
//  Created:            August 1996
//

#ifndef __BROWEDIT_H__
#define __BROWEDIT_H__

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

/////////////////////////////////////////////////////////////////////////////
// SECDirSelectDlg

// This helper class provides directory selection under Win16 and
// Win32 by using a customization of the common dialogs.

class SECDirSelectDlg : public CFileDialog
{
private:
	static DWORD c_dwFlags;
	static const TCHAR* c_pcszFilter;

protected:
    //{{AFX_MSG(SECDirSelectDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
    SECDirSelectDlg(LPCTSTR lpcszCaption, CWnd* pWndParent = NULL,
			  LPCTSTR lpcszInitialDir = NULL );
	void GetPath(CString& strPath) const;
};

// The following forward reference and abstract base class are
// required for the directory and file browse-edit classes.

class SECBrowseButton;

class SECBrowseEditBase : public CEdit
{
private:
	SECBrowseButton* m_pBrowseBtn;
	CString m_strCaption;

protected:
	//{{AFX_MSG(SECBrowseEditBase)
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual WNDPROC* GetSuperWndProcAddr();

public:
    SECBrowseEditBase();
    virtual ~SECBrowseEditBase();
	BOOL Initialize(UINT uiID, CWnd* pWndParent);
	virtual void OnBrowse() = 0;
	void SetBrowseCaption(LPCTSTR lpcszCaption)
		{ m_strCaption = lpcszCaption; }
	LPCTSTR GetBrowseCaption() const
		{ return m_strCaption; }
};

// The following provides directory browsing from an edit control.

class SECBrowseDirEdit : public SECBrowseEditBase
{
public:
	SECBrowseDirEdit();
	virtual void OnBrowse();

#ifdef WIN32
	enum{ win16Style = 0x1, win32Style = 0x2 };
	void SetDialogStyle( WORD wStyle );
	WORD GetDialogStyle() const;

	void _Win32Browse();

protected:
	WORD m_wDialogStyle;
#endif

};

// The following provides file browsing from an edit control.

class SECBrowseFileEdit : public SECBrowseEditBase
{
protected:
	CString m_strFilters;
	CString m_strInitialFile;
	CString m_strExt;
	DWORD   m_dwDialogStyle;

public:                           
	SECBrowseFileEdit();
	void SetFileFilters(const TCHAR* pcszFilters)
		{ m_strFilters = pcszFilters; }
	void SetDefaultExtension(const TCHAR* pcszExt)
		{ m_strExt = pcszExt; }
	void SetInitialFileName(const TCHAR* pcszName)
		{ m_strInitialFile = pcszName; }
	void ModifyDialogStyle( DWORD dwRemove, DWORD dwAdd );

	virtual void OnBrowse();
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __BROWEDIT_H__
