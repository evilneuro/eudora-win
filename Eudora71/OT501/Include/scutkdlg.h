//
// Stingray Software Extension Classes
// Copyright (C) 1996 Jim Beveridge
// All Rights Reserved
//
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
//
//  Author:			Jim Beveridge
//  Description:	Hotkey control dialog - Headers
//  Created:		Sun Jan 28 15:49:50 1996
//
//


#ifndef __SCUTKDIALOG_H__
#define __SCUTKDIALOG_H__

#ifdef WIN32

#ifndef __SCUTCMDS_H__
#include "SCutCmds.h"
#endif // __SCUTCMDS_H__

#ifndef __SCUTACCL_H__
#include "SCutAccl.h"
#endif // __SCUTACCL_H__

//AAB #include "resource.h"

#ifdef _SECDLL
        #undef AFXAPP_DATA
        #define AFXAPP_DATA AFXAPI_DATA
        #undef AFX_DATA
        #define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


/////////////////////////////////////////////////////////////////////////////
// SECAssignKeyDlg dialog

class SECAssignKeyDlg : public CDialog
{
// Construction
public:
	SECAssignKeyDlg(
		SECCommandList& commands,
		SECShortcutTable& shortcuts,
		CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(SECAssignKeyDlg)
	enum { IDD = IDD_ASSIGN_SHORTCUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	DWORD	m_HotkeyValue;
	ACCEL m_Accel;
	SECCommandList&	m_Commands;
	SECShortcutTable&	m_Shortcuts;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECAssignKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void OnOK();

// Implementation
protected:

	void UpdateKeyDescription();

		// Generated message map functions
	//{{AFX_MSG(SECAssignKeyDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif //WIN32
#endif //__SCUTKDIALOG_H__