// Stingray Software Extension Classes
// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Author:			Dean Hallman
//  Description:	Declarations for SECDialogBar
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __SBARDLG_H__
#define __SBARDLG_H__

// CDockBar is an private, undocumented MFC class that we subclass.
// Hence, the necessity to include afxpriv.h
#ifndef __AFXPRIV_H__
#include "afxpriv.h"
#endif

#ifndef __SBARCORE_H__
#include "sbarcore.h"
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

//////////////////////////////////////////////////////////////////////
// SECCDialogBar

class SECDialogBar : public SECControlBar
{
	DECLARE_DYNAMIC(SECDialogBar)

// Construction
public:
	SECDialogBar();

	/*BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, UINT nID,
		DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd = NULL,
		CCreateContext* pContext = NULL);*/
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
			DWORD dwExStyle = CBRS_EX_STDCONTEXTMENU, UINT nID = 1);
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
			DWORD dwExStyle = CBRS_EX_STDCONTEXTMENU, UINT nID = 1)
		{ return Create(pParentWnd, MAKEINTRESOURCE(nIDTemplate), nStyle, dwExStyle, nID); };

// Implementation
public:
	virtual ~SECDialogBar();
	CSize m_sizeDefault;
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
#ifndef _AFX_NO_OCC_SUPPORT
	// data and functions necessary for OLE control containment
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	LPCTSTR m_lpszTemplateName;
	virtual BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
#endif

// Generated message map functions
protected:
    //{{AFX_MSG(SECDialogBar)
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SBARDLG_H__

#endif // WIN32