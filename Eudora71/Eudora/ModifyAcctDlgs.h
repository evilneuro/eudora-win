// ModifyAcctDlgs.h
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#ifndef _MODIFYACCTDLGSH__
#define _MODIFYACCTDLGSH__

#include "resource.h"
#include "helpxdlg.h"
#include "ValidEdit.h" // CValidEdit
#include "PersParams.h"

class CModifyAcctSheet ; //forward reference


/////////////////////////////////////////////////////////////////////////////
// PropCHelpxDlg dialog

// Use this class if you want Context Sensitive Help in a PropertySheet

class CPropHelpxDlg : public CPropertyPage
{
// Construction
public:
	CPropHelpxDlg(UINT nID);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHelpxDlg)
//	enum { IDD = nID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	afx_msg long OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg long OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM lParam);
	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg long OnRightButton(WPARAM wParam, LPARAM lParam);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropHelpxDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
 
};

/////////////////////////////////////////////////////////////////////////
/*** Generic Traits Property Page ************/

class CModifyAcct_Generic : public CPropHelpxDlg
{
// Construction
public:
	CModifyAcct_Generic( CModifyAcctSheet *);   
	CModifyAcctSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CModifyAcct_Generic)
	enum { IDD = IDD_PROPPAGE_PERSONALITY1 };
	CValidEdit	m_PersNameEdit;
	CComboBox	m_comboSignature;
	CComboBox	m_comboStationery;
	CComboBox   m_SSLCombo;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyAcct_Generic)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive() ;
	virtual void OnOK() ;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL SaveDialogSettings(); // Only 'saves' to the parent's CPersParam block, not to file
	BOOL IsNameAcceptable(bool bShowErr);

protected:
	BOOL LoadDialogSettings(); // Loads from parent's CPersParam block

	void PopulateSignatureCombo();
	void PopulateStationeryCombo();
	void PopulateSSLCombo();
	
	// Generated message map functions
	//{{AFX_MSG(CModifyAcct_Generic)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePersonaNameEdit();
	afx_msg void OnLastNegotiated();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////
/*** Servers Property Page ************/

class CModifyAcct_Servers : public CPropHelpxDlg
{
// Construction
public:
	CModifyAcct_Servers( CModifyAcctSheet *);   
	CModifyAcctSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CModifyAcct_Servers)
	enum { IDD = IDD_PROPPAGE_PERSONALITY2 };
	CComboBox m_SSLCombo;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyAcct_Servers)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive() ;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL SaveDialogSettings(); // Only 'saves' to the parent's CPersParam block, not to file
	bool HasActiveTasks();
    void PopulateSSLCombo();
protected:
//	void LoadControlSetting(CWnd* pWndControl, UINT uIniID, bool bUseDefault = false) ;
	typedef enum { SERV_POP, SERV_IMAP } ServType;

	CString m_OriginalInServer, m_OriginalOutServer;
	int m_nOriginalServTypeID;
	bool m_bInit;

	void ShowServerSpecificControls(ServType nServ);
	void ShowControl(int nID, bool bShow);
	void EnableControl(int nID, bool bShow);
	void VerifyAuthCheck();

	BOOL LoadDialogSettings(); // Loads from parent's CPersParam block

	// Generated message map functions
	//{{AFX_MSG(CModifyAcct_Servers)
	virtual void OnOK() ;
	virtual BOOL OnInitDialog();
	afx_msg void OnPop();
	afx_msg void OnImap() ;
	afx_msg void OnImapTrashSelect();
	afx_msg void OnLastNegotiated();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif _MODIFYACCTDLGSH__
