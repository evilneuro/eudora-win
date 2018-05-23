// ModifyAcctDlgs.cpp
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include "eudora.h"
#include "helpcntx.h"
#include "utils.h"
#include "ModifyAcctSheet.h"
#include "persona.h"
#include "rs.h"			// CRString
#include "guiutils.h"	// ErrorDialog()
#include "QCSignatureDirector.h"
#include "QCStationeryDirector.h"
#include "ValidEdit.h" // CValidEdit
#include "password.h"

#include "ModifyAcctDlgs.h"
#include "QCTaskManager.h"
#include "SSLConnectionDlg.h"


#include "DebugNewHelpers.h"

// --------------------------------------------------------------------------

void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew); 

// --------------------------------------------------------------------------

extern QCSignatureDirector	g_theSignatureDirector;
extern QCStationeryDirector	g_theStationeryDirector;

// --------------------------------------------------------------------------

CPropHelpxDlg::CPropHelpxDlg(UINT nID)
	: CPropertyPage(nID)
{
	//{{AFX_DATA_INIT(CPropHelpxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void CPropHelpxDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropHelpxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropHelpxDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CPropHelpxDlg)
	//}}AFX_MSG_MAP

	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_MESSAGE(WM_HELP, OnHelp)	
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_RBUTTONUP, OnRightButton)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

long CPropHelpxDlg::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
	// Called when the user enters help mode and hits a toolbar button.
	// Returns the Help ID of the selected button.

	// Not implemented since a modal dialog doesn't allow for this help mode
	return 0;
}

long CPropHelpxDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
	// Called when the user presses F1 in a control.
	HELPINFO* lphi;
	lphi = (HELPINFO*)lParam;
	if ( lphi->iContextType == HELPINFO_WINDOW)
		CContextHelp((HWND)lphi->hItemHandle);

	return TRUE;	
} 

long CPropHelpxDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	// Called when the user right clicks on a control.

	// Invoke help
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	CPoint point(x,y);
	
	CContextHelp( m_hWnd, point, FALSE);

	return TRUE;
}

long CPropHelpxDlg::OnRightButton(WPARAM wParam, LPARAM lParam)
{
	if (IsWin32s() || (IsWinNT() &&	GetMajorVersion() == 3 && GetMinorVersion() < 51))
		return (OnContextMenu(wParam, lParam));
	return (CDialog::DefWindowProc(WM_RBUTTONUP, wParam, lParam));
}



LRESULT CPropHelpxDlg::OnCommandHelp(WPARAM, LPARAM lParam)
{  
	// 32bit version eats this message.  OnHelp covers for it.
 	return TRUE;
}            



 


/////////////////////////////////////////////////////////////////////////////
// CModifyAcct_Generic dialog



CModifyAcct_Generic::CModifyAcct_Generic( CModifyAcctSheet *parent)
	: CPropHelpxDlg(CModifyAcct_Generic::IDD)
{
	ASSERT( parent);
	m_pParent = parent ;
}

void CModifyAcct_Generic::DoDataExchange(CDataExchange* pDX)
{
	CPropHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyAcct_Generic)
	DDX_Control(pDX, IDS_INI_PERSONA_SIGNATURE, m_comboSignature);
	DDX_Control(pDX, IDS_INI_PERSONA_STATIONERY, m_comboStationery);
	DDX_Control(pDX, IDC_PERSONA_NAME_EDIT, m_PersNameEdit);
	DDX_Control(pDX, IDC_COMBO_SSLOPTIONS, m_SSLCombo);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CModifyAcct_Generic, CPropHelpxDlg)
	//{{AFX_MSG_MAP(CModifyAcct_Generic)
	ON_EN_CHANGE(IDC_PERSONA_NAME_EDIT, OnChangePersonaNameEdit)
	ON_BN_CLICKED(IDC_BUTTON_LAST_NEGOTIATED, OnLastNegotiated)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyAcct_Generic message handlers


BOOL CModifyAcct_Generic::OnInitDialog() 
{
	ASSERT(m_pParent);

	CPropHelpxDlg::OnInitDialog();

	PopulateSignatureCombo() ;
	PopulateStationeryCombo() ;
	PopulateSSLCombo();

	if (m_pParent->m_bCreateNew)
	{
		GetDlgItem(IDC_PERSONA_NAME_EDIT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_PERSONA_NAME_EDIT))->SetSel(0,-1);
		
		m_PersNameEdit.SetInvalid(CRString(IDS_PERSONA_EXCLUDE_CHARS));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CModifyAcct_Generic::OnSetActive()
{
	ASSERT(m_pParent);
	LoadDialogSettings() ;

	return (TRUE);
}

BOOL CModifyAcct_Generic::OnKillActive()
{
	ASSERT_VALID(this);
	ASSERT(m_pParent);

	SaveDialogSettings();

	return (TRUE);
}

////////////////////////////////////////////////////////////////////////
// PopulateSignatureCombo [protected]
//
////////////////////////////////////////////////////////////////////////
void CModifyAcct_Generic::PopulateSignatureCombo()
{
	ASSERT(::IsWindow(m_comboSignature.GetSafeHwnd()));

	m_comboSignature.ResetContent();
	m_comboSignature.AddString(CRString(IDS_NO_DEFAULT));	// always have a blank option
	g_theSignatureDirector.FillComboBox(&m_comboSignature);
	m_comboSignature.SetCurSel(0);
}


////////////////////////////////////////////////////////////////////////
// PopulateStationeryCombo [protected]
//
////////////////////////////////////////////////////////////////////////
void CModifyAcct_Generic::PopulateStationeryCombo()
{
	ASSERT(::IsWindow(m_comboStationery.GetSafeHwnd()));

	m_comboStationery.ResetContent();
	m_comboStationery.AddString(CRString(IDS_NO_DEFAULT));	// always have a blank option
	g_theStationeryDirector.FillComboBox(&m_comboStationery);
	m_comboStationery.SetCurSel(0);
}


void CModifyAcct_Generic::OnOK()
{
}

void CModifyAcct_Generic::OnChangePersonaNameEdit() 
{
	ASSERT(m_pParent);
	ASSERT(m_pParent->m_bCreateNew); // Should only be able to edit if we're creating a new persona

	// This checks for a valid name on every change of the pers name field.
	// May be too slow with MANY personalities.

//	CPersParams *pParams = &(m_pParent->m_params);
//	GetDlgItem(IDC_PERSONA_NAME_EDIT)->GetWindowText(pParams->PersName);
//	m_pParent->GetDlgItem(IDOK)->EnableWindow(IsNameAcceptable(false));
}

BOOL CModifyAcct_Generic::IsNameAcceptable(bool bShowErr) 
{
	ASSERT(m_pParent);
	ASSERT(m_pParent->m_bCreateNew);
	
	LPCSTR sPersName = m_pParent->m_params.PersName;
	const size_t nLen = strlen(sPersName);

	if (nLen == 0) // Empty name not allows
	{
		if (bShowErr)
		{
			::ErrorDialog(IDS_ERR_WIZARD_NOPERSNAME);
			GetDlgItem(IDC_PERSONA_NAME_EDIT)->SetFocus();
			((CEdit*)GetDlgItem(IDC_PERSONA_NAME_EDIT))->SetSel(0,-1);
		}

		return (FALSE) ;
	}

	if (nLen >= MAX_PERSONA_LEN) // Too long
	{
		if (bShowErr)
		{
			::ErrorDialog(IDS_ERR_PERSONA_NAME_TOO_LONG, MAX_PERSONA_LEN);
			GetDlgItem(IDC_PERSONA_NAME_EDIT)->SetFocus();
			((CEdit*)GetDlgItem(IDC_PERSONA_NAME_EDIT))->SetSel(0,-1);
		}

		return (FALSE) ;
	}

	if (g_Personalities.IsA(sPersName)) // Already exists
	{
		if (bShowErr)
		{
			::ErrorDialog(IDS_ERR_DUPLICATE_PERSONA, (const char *) sPersName);
			GetDlgItem(IDC_PERSONA_NAME_EDIT)->SetFocus();
			((CEdit*)GetDlgItem(IDC_PERSONA_NAME_EDIT))->SetSel(0,-1);
		}
		
		return (FALSE) ;
	}

	return (TRUE);
}

// --------------------------------------------------------------------------

BOOL CModifyAcct_Generic::SaveDialogSettings()
{
	// If this page does not exist, do nothing. Pages are created dynamically,
	// so any page you don't visit will not exist.
	if (GetSafeHwnd() == NULL)
		return (FALSE);

	ASSERT(m_pParent);
	CPersParams *pParams = &(m_pParent->m_params);

	if (m_pParent->m_bCreateNew)
		GetDlgItemText(IDC_PERSONA_NAME_EDIT, pParams->PersName);


	// Fix bug #85; clear password if username change
	CString szLoginName;
	GetDlgItemText(IDS_INI_PERSONA_LOGIN_NAME, szLoginName);
	if (pParams->LoginName!=szLoginName) 
		ClearPasswordFor(pParams->PersName);

	GetDlgItemText(IDS_INI_PERSONA_REAL_NAME, pParams->RealName);
	GetDlgItemText(IDS_INI_PERSONA_DEFAULT_DOMAIN, pParams->DefaultDomain);
	GetDlgItemText(IDS_INI_PERSONA_RETURN_ADDR, pParams->ReturnAddress);
	GetDlgItemText(IDS_INI_PERSONA_LOGIN_NAME, pParams->LoginName);
	GetDlgItemText(IDS_INI_PERSONA_SMTP_SERVER, pParams->OutServer);

	pParams->bSMTPAuthAllowed	= (IsDlgButtonChecked(IDS_INI_SMTP_AUTH_ALLOWED) != 0);
	pParams->bUseSMTPRelay	= (IsDlgButtonChecked(IDS_INI_PERSONA_USE_RELAY) != 0);
	pParams->bUseSubmissionPort	= (IsDlgButtonChecked(IDS_INI_USE_SUBMISSION_PORT) != 0);
	pParams->bCheckMail			= (IsDlgButtonChecked(IDS_INI_PERSONA_CHECK_MAIL) != 0);

	m_comboSignature.GetWindowText(pParams->Signature);
	if (pParams->Signature == CRString(IDS_NO_DEFAULT))
		pParams->Signature.Empty();

	m_comboStationery.GetWindowText(pParams->Stationery);
	if (pParams->Stationery == CRString(IDS_NO_DEFAULT))
		pParams->Stationery.Empty();

	int nSel = m_SSLCombo.GetCurSel();
	if(nSel == CB_ERR)
		nSel =0;
	pParams->m_SSLSendUsage = nSel;

	return (TRUE);
}

// --------------------------------------------------------------------------

// [PUBLIC] LoadDialogSettings
//
//
BOOL CModifyAcct_Generic::LoadDialogSettings()
{
	ASSERT(m_pParent) ; //sanity check
	const CPersParams *pParams = &(m_pParent->m_params);

	if (m_pParent->m_bCreateNew)
	{
		GetDlgItem(IDC_PERSONA_NAME_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PERSONA_NAME_EDIT)->ShowWindow(SW_SHOW);

		SetDlgItemText(IDC_PERSONA_NAME_EDIT, pParams->PersName);
		OnChangePersonaNameEdit();
	}
	else
	{
		GetDlgItem(IDC_PERSONA_NAME_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PERSONA_NAME_EDIT)->ShowWindow(SW_HIDE);

		SetDlgItemText(IDC_PERSONA_NAME_STATIC, pParams->PersName);
	}

	SetDlgItemText(IDS_INI_PERSONA_REAL_NAME, pParams->RealName);
	SetDlgItemText(IDS_INI_PERSONA_DEFAULT_DOMAIN, pParams->DefaultDomain);
	SetDlgItemText(IDS_INI_PERSONA_RETURN_ADDR, pParams->ReturnAddress);
	SetDlgItemText(IDS_INI_PERSONA_LOGIN_NAME, pParams->LoginName);
	SetDlgItemText(IDS_INI_PERSONA_SMTP_SERVER, pParams->OutServer);
	GetDlgItem(IDS_INI_PERSONA_SMTP_SERVER)->EnableWindow(TRUE);

	CheckDlgButton(IDS_INI_SMTP_AUTH_ALLOWED,  pParams->bSMTPAuthAllowed);
	CheckDlgButton(IDS_INI_PERSONA_USE_RELAY,  pParams->bUseSMTPRelay);
	CheckDlgButton(IDS_INI_USE_SUBMISSION_PORT,pParams->bUseSubmissionPort);
	CheckDlgButton(IDS_INI_PERSONA_CHECK_MAIL, pParams->bCheckMail);

	int nIndex;
	
	nIndex = m_comboSignature.FindStringExact(-1, pParams->Signature);
	m_comboSignature.SetCurSel(nIndex >= 0? nIndex : 0);

	nIndex = m_comboStationery.FindStringExact(-1, pParams->Stationery);
	m_comboStationery.SetCurSel(nIndex >= 0? nIndex : 0);

	return TRUE;
}



// **************************************************************************
// **************************************************************************
//
// CModifyAcct_Servers dialog
//
// **************************************************************************
// **************************************************************************



CModifyAcct_Servers::CModifyAcct_Servers( CModifyAcctSheet *parent)
	: CPropHelpxDlg(CModifyAcct_Servers::IDD)
{
	ASSERT( parent);
	m_pParent = parent ;
}

// --------------------------------------------------------------------------

void CModifyAcct_Servers::DoDataExchange(CDataExchange* pDX)
{
	CPropHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyAcct_Servers)
	//DDX_Text(pDX, IDC_EDITPERSNAME, m_strpersname);
	DDX_Control(pDX, IDC_COMBO_SSLOPTIONS, m_SSLCombo);
	//}}AFX_DATA_MAP

	// my own code
	if( pDX->m_bSaveAndValidate)
	{
		DWORD Val;

		DDX_Text( pDX, IDS_INI_BIG_MESSAGE_THRESHOLD, Val);
		DDX_Text( pDX, IDS_INI_IMAP_MAXSIZE, Val) ;
		DDX_Text( pDX, IDS_INI_LEAVE_ON_SERVER_DAYS, Val);
		DDX_Text( pDX, IDS_INI_IMAP_TRASH_MBOXNAME, Val);

		// Special check for LMOS
		if (IsDlgButtonChecked(IDS_INI_DELETE_MAIL_FROM_SERVER))
		{
			// See if positive value in the LMOS field
			if (Val == 0)
			{
				ErrorDialog(IDS_ERR_SERVER_DELETE_X_DAYS);
				pDX->PrepareEditCtrl(IDS_INI_LEAVE_ON_SERVER_DAYS);
				pDX->Fail();
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CModifyAcct_Servers, CPropHelpxDlg)
	//{{AFX_MSG_MAP(CModifyAcct_Servers)
	ON_BN_CLICKED( IDS_INI_USES_POP, OnPop)
	ON_BN_CLICKED( IDS_INI_USES_IMAP, OnImap)
	ON_BN_CLICKED(IDS_INI_IMAP_TRASH_MBOXNAME, OnImapTrashSelect)
	ON_BN_CLICKED(IDC_BUTTON_LAST_NEGOTIATED, OnLastNegotiated)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

BOOL CModifyAcct_Servers::OnInitDialog() 
{
	ASSERT(m_pParent);
	const CPersParams *pParams = &(m_pParent->m_params);

	CPropHelpxDlg::OnInitDialog();

	LoadDialogSettings() ;

	m_bInit = false;
	if (pParams->bIMAP)
	{
		OnImap();
		m_nOriginalServTypeID = IDS_INI_USES_IMAP;
	}
	else // pParams->bPOP
	{
		OnPop();
		m_nOriginalServTypeID = IDS_INI_USES_POP;
	}

	m_bInit = true;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// --------------------------------------------------------------------------

// [PUBLIC] SaveDialogSettings
//
// Copy dialog settings into parent's CPersParam block
//
BOOL CModifyAcct_Servers::SaveDialogSettings()
{
	// If this page does not exist, do nothing. Pages are created dynamically,
	// so any page you don't visit will not exist.
	if (GetSafeHwnd() == NULL)
		return (FALSE);

	ASSERT(m_pParent) ; //sanity check
	CPersParams *pParams = &(m_pParent->m_params);

	// Fix bug #85; clear password if server changes
	CString szServer;
	GetDlgItemText(IDS_INI_POP_SERVER, szServer);
	if (pParams->InServer != szServer) ClearPasswordFor(pParams->PersName);

	GetDlgItemText(IDS_INI_POP_SERVER, pParams->InServer);
	GetDlgItemText(IDS_INI_IMAP_PREFIX, pParams->IMAPPrefix);
	GetDlgItemText(IDS_INI_LEAVE_ON_SERVER_DAYS, pParams->LeaveOnServDays);
	GetDlgItemText(IDS_INI_IMAP_MAXSIZE, pParams->IMAPMaxSize);
	GetDlgItemText(IDS_INI_IMAP_TRASH_MBOXNAME, pParams->IMAPTrashMailbox);
	GetDlgItemText(IDS_INI_IMAP_AUTO_EXPUNGE_PCT, pParams->strIMAPAutoExpPct);

	CString str;
	GetDlgItemText(IDS_INI_BIG_MESSAGE_THRESHOLD, str);
	const long Num = atol(str) * 1024L;
	pParams->BigMsgThreshold.Format("%ld", Num);

	pParams->bPOP				= (IsDlgButtonChecked(IDS_INI_USES_POP) != 0);
	pParams->bIMAP				= (IsDlgButtonChecked(IDS_INI_USES_IMAP) != 0);
	pParams->bLMOS				= (IsDlgButtonChecked(IDS_INI_PERSONA_LMOS) != 0);
	pParams->bPassword			= (IsDlgButtonChecked(IDS_INI_AUTH_PASS) != 0);
	pParams->bAPop				= (IsDlgButtonChecked(IDS_INI_AUTH_APOP) != 0);
	pParams->bKerberos			= (IsDlgButtonChecked(IDS_INI_AUTH_KERB) != 0);
	pParams->bRPA				= (IsDlgButtonChecked(IDS_INI_AUTH_RPA) != 0);
	pParams->bDelServerAfter	= (IsDlgButtonChecked(IDS_INI_DELETE_MAIL_FROM_SERVER) != 0);
	pParams->bDelWhenTrashed	= (IsDlgButtonChecked(IDS_INI_SERVER_DELETE) != 0);
	pParams->bSkipBigMsgs		= (IsDlgButtonChecked(IDS_INI_SKIP_BIG_MESSAGES) != 0);
	pParams->bIMAPMinDwnld		= (IsDlgButtonChecked(IDS_INI_IMAP_MINDNLOAD) != 0);
	pParams->bIMAPFullDwnld		= (IsDlgButtonChecked(IDS_INI_IMAP_OMITATTACH) != 0);
	pParams->bIMAPXferToTrash	= (IsDlgButtonChecked(IDS_INI_IMAP_XFERTOTRASH) != 0);
	pParams->bIMAPMarkDeleted	= (IsDlgButtonChecked(IDS_INI_IMAP_MARK_DELETED) != 0);
	pParams->bIMAPAutoExpNever	= (IsDlgButtonChecked(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER) != 0);
	pParams->bIMAPAutoExpAlways	= (IsDlgButtonChecked(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS) != 0);
	pParams->bIMAPAutoExpOnPct	= (IsDlgButtonChecked(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT) != 0);

	int nSel = m_SSLCombo.GetCurSel();
	if(nSel == CB_ERR)
		nSel =0;
	pParams->m_SSLReceiveUsage = nSel;

	return (TRUE);
}

// --------------------------------------------------------------------------

// [PUBLIC] LoadDialogSettings
//
// Update controls to match parent's CPersParam block
//
BOOL CModifyAcct_Servers::LoadDialogSettings()
{
	ASSERT(m_pParent) ; //sanity check
	const CPersParams *pParams = &(m_pParent->m_params);

	SetDlgItemText(IDS_INI_POP_SERVER, pParams->InServer);
	SetDlgItemText(IDS_INI_IMAP_PREFIX, pParams->IMAPPrefix);
	SetDlgItemText(IDS_INI_LEAVE_ON_SERVER_DAYS, pParams->LeaveOnServDays);
	SetDlgItemText(IDS_INI_IMAP_MAXSIZE, pParams->IMAPMaxSize);
	SetDlgItemText(IDS_INI_IMAP_TRASH_MBOXNAME, pParams->IMAPTrashMailbox);
	SetDlgItemText(IDS_INI_IMAP_AUTO_EXPUNGE_PCT, pParams->strIMAPAutoExpPct);

	// Big Message Threshold is in K in dlg, but stored in bytes
	const long Num = atol(pParams->BigMsgThreshold) / 1024L;
	CString str;
	str.Format("%ld", Num);
	SetDlgItemText(IDS_INI_BIG_MESSAGE_THRESHOLD, str);

	CheckDlgButton(IDS_INI_USES_POP, pParams->bPOP);
	CheckDlgButton(IDS_INI_USES_IMAP, pParams->bIMAP);

	CheckDlgButton(IDS_INI_PERSONA_LMOS, pParams->bLMOS);
	CheckDlgButton(IDS_INI_AUTH_PASS, pParams->bPassword);
	CheckDlgButton(IDS_INI_AUTH_APOP, pParams->bAPop);
	CheckDlgButton(IDS_INI_AUTH_KERB, pParams->bKerberos);
	CheckDlgButton(IDS_INI_AUTH_RPA, pParams->bRPA);
	CheckDlgButton(IDS_INI_DELETE_MAIL_FROM_SERVER, pParams->bDelServerAfter);
	CheckDlgButton(IDS_INI_SERVER_DELETE, pParams->bDelWhenTrashed);
	CheckDlgButton(IDS_INI_SKIP_BIG_MESSAGES, pParams->bSkipBigMsgs);
	CheckDlgButton(IDS_INI_IMAP_MINDNLOAD, pParams->bIMAPMinDwnld);
	CheckDlgButton(IDS_INI_IMAP_OMITATTACH, pParams->bIMAPFullDwnld);

	CheckDlgButton(IDS_INI_IMAP_XFERTOTRASH, pParams->bIMAPXferToTrash);
	CheckDlgButton(IDS_INI_IMAP_MARK_DELETED, pParams->bIMAPMarkDeleted);
	CheckDlgButton(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER, pParams->bIMAPAutoExpNever);
	CheckDlgButton(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS, pParams->bIMAPAutoExpAlways);
	CheckDlgButton(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT, pParams->bIMAPAutoExpOnPct);
	PopulateSSLCombo();

	return TRUE;
}

void CModifyAcct_Servers::VerifyAuthCheck()
{
	bool bSetCheck = true;
	int nCheckedID = GetCheckedRadioButton(IDS_INI_AUTH_PASS, IDS_INI_AUTH_RPA);
	
	if (nCheckedID != 0)
	{
		if (GetDlgItem(nCheckedID)->IsWindowVisible() == TRUE)
			bSetCheck = false;
		else
			CheckDlgButton(nCheckedID, 0); // Uncheck hidden check
	}

	if (bSetCheck)
		CheckDlgButton(IDS_INI_AUTH_PASS, 1);
}


void CModifyAcct_Servers::OnPop()
{
	ASSERT(m_pParent);

	if( HasActiveTasks())
		return;

	ShowServerSpecificControls(SERV_POP);
}

void CModifyAcct_Servers::OnImap()
{
	ASSERT(m_pParent);

	if( HasActiveTasks())
		return;

	ShowServerSpecificControls(SERV_IMAP);
}

bool CModifyAcct_Servers::HasActiveTasks()
{
	if(!m_bInit)
		return false;

	ASSERT(m_pParent);
	CPersParams *pParams = &(m_pParent->m_params);

	//if we only switch to a different server type
	if (IsDlgButtonChecked(m_nOriginalServTypeID) == 0)
	{
		if( QCGetTaskManager()->GetTaskCount(pParams->PersName) != 0)
		{
			AfxMessageBox(CRString(IDS_CANT_CHANGE_PERSONA));
			CheckDlgButton(IDS_INI_USES_POP, pParams->bPOP);
			CheckDlgButton(IDS_INI_USES_IMAP, pParams->bIMAP);

			return true;
		}
	}

	return false;
}

void CModifyAcct_Servers::ShowControl(int nID, bool bShow)
{
	GetDlgItem(nID)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

void CModifyAcct_Servers::EnableControl(int nID, bool bEnable)
{
	GetDlgItem(nID)->EnableWindow(bEnable ? TRUE : FALSE);
}

void CModifyAcct_Servers::ShowServerSpecificControls(ServType nServ)
{
	const bool bPOP = (nServ == SERV_POP);
	const bool bIMAP = (nServ == SERV_IMAP);

	ShowControl(IDS_INI_AUTH_KERB, true);
	ShowControl(IDS_INI_AUTH_APOP, true);

	// POP controls
	ShowControl(IDS_INI_SKIP_BIG_MESSAGES, bPOP);
	ShowControl(IDS_INI_BIG_MESSAGE_THRESHOLD, bPOP);
	ShowControl(IDS_INI_PERSONA_LMOS, bPOP);
	ShowControl(IDS_INI_DELETE_MAIL_FROM_SERVER, bPOP);
	ShowControl(IDS_INI_LEAVE_ON_SERVER_DAYS, bPOP);
	ShowControl(IDS_INI_SERVER_DELETE, bPOP);
	ShowControl(IDS_INI_AUTH_RPA, bPOP);
	ShowControl(IDC_STAT_INCOMING_DAYS, bPOP);
	ShowControl(IDC_STAT_INCOMING_KSIZE, bPOP);

	// IMAP controls
	ShowControl(IDC_STAT_INCOMING_MAILBPREFIX, bIMAP);
	ShowControl(IDC_STAT_INCOMING_DOWNLOAD, bIMAP);
	ShowControl(IDS_INI_IMAP_PREFIX, bIMAP);
	ShowControl(IDS_INI_IMAP_OMITATTACH, bIMAP);
	ShowControl(IDS_INI_IMAP_MINDNLOAD, bIMAP);
	ShowControl(IDS_INI_IMAP_MAXSIZE, bIMAP);
	ShowControl(IDC_STAT_INCOMING_K, bIMAP);
	ShowControl(IDC_STAT_IDELETE_MSG, bIMAP);
	ShowControl(IDS_INI_IMAP_MARK_DELETED, bIMAP);
	ShowControl(IDS_INI_IMAP_XFERTOTRASH, bIMAP);
	ShowControl(IDS_INI_IMAP_TRASH_MBOXNAME, bIMAP);
	ShowControl(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER, bIMAP);
	ShowControl(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS, bIMAP);
	ShowControl(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT, bIMAP);
	ShowControl(IDS_INI_IMAP_AUTO_EXPUNGE_PCT, bIMAP);
	ShowControl(IDC_STAT_AUTO_PURGE, bIMAP);
	ShowControl(IDC_LBL_SETTINGS_PCT_DELETED, bIMAP);

	// Change the text of the "APOP" to "CRAM-MD5" for IMAP
	SetDlgItemText(IDS_INI_AUTH_APOP, bIMAP ? "&Cram-MD5" : "&APOP");

	VerifyAuthCheck();
}

BOOL CModifyAcct_Servers::OnSetActive()
{
	LoadDialogSettings();
	return (true);
}

BOOL CModifyAcct_Servers::OnKillActive()
{
	ASSERT_VALID(this);
	ASSERT(m_pParent);

	SaveDialogSettings();

	return (true); // Let MFC continue on its course
}

void CModifyAcct_Servers::OnOK()
{
}

#include "Controls.h"
void CModifyAcct_Servers::OnImapTrashSelect() 
{
	BOOL bModified = false;
	CRect	rect;

	// This applies only to an IMAP personality.
	// 
	// only mailboxes belonging to this IMAP server may be used as the Trash -jdboyd
	CString szPersName = m_pParent->m_params.PersName; 

	if ( !g_Personalities.IsImapPersona (szPersName) )
	{
		ASSERT (0);
		return;
	}

	// Create an invisible CMailboxButton over the current one.
	//
	CWnd* pWnd = GetDlgItem(IDS_INI_IMAP_TRASH_MBOXNAME);
	if (!pWnd)
		return;
	
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);

	CMailboxButton mboxButton;

	mboxButton.Create ( "", BS_PUSHBUTTON | WS_TABSTOP, 
							rect, this, IDS_INI_IMAP_TRASH_MBOXNAME + 1);

	if (IsWindow(mboxButton.m_hWnd))
	{
		bModified = mboxButton.SelectMailbox(szPersName);

		if (bModified)
		{
			// Get the new selection.
			//
			if ( mboxButton.m_bIsImap )
			{
				AfxSetWindowText(pWnd->m_hWnd, mboxButton.m_SelectedImapName);
			}
			else
			{
				bModified = FALSE;
			}
		}
	}
}


void CModifyAcct_Generic::PopulateSSLCombo()
{
	const CPersParams *pParams = &(m_pParent->m_params);
	m_SSLCombo.ResetContent();
	m_SSLCombo.AddString(CRString(IDS_SSL_NEVER));
	m_SSLCombo.AddString(CRString(IDS_SSL_IFAVAILABLE));
	m_SSLCombo.AddString(CRString(IDS_SSL_ALTERNATEPORT));
	m_SSLCombo.AddString(CRString(IDS_SSL_ALWAYS));
	m_SSLCombo.SetCurSel(pParams->m_SSLSendUsage);

}



void CModifyAcct_Servers::PopulateSSLCombo()
{
	const CPersParams *pParams = &(m_pParent->m_params);
	m_SSLCombo.ResetContent();
	m_SSLCombo.AddString(CRString(IDS_SSL_NEVER));
	m_SSLCombo.AddString(CRString(IDS_SSL_IFAVAILABLE));
	m_SSLCombo.AddString(CRString(IDS_SSL_ALTERNATEPORT));
	m_SSLCombo.AddString(CRString(IDS_SSL_ALWAYS));
	m_SSLCombo.SetCurSel(pParams->m_SSLReceiveUsage);
}

void CModifyAcct_Generic::OnLastNegotiated()
{
	CSSLConnectionDlg dlg;
	CString szPersName = m_pParent->m_params.PersName;
	CRString Dominant( IDS_DOMINANT );
	if(szPersName == Dominant)
		szPersName = "";

	dlg.Run(szPersName, "SMTP");

}

void CModifyAcct_Servers::OnLastNegotiated()
{
	CSSLConnectionDlg dlg;
	CString szPersName = m_pParent->m_params.PersName;
	CRString Dominant( IDS_DOMINANT );

	CString csProtocol;
	bool bIsPOP	 = (IsDlgButtonChecked(IDS_INI_USES_POP) != 0);
	bool bIsIMAP = (IsDlgButtonChecked(IDS_INI_USES_IMAP) != 0);
	if((szPersName == Dominant) && bIsPOP)
		szPersName = "";

	if (bIsIMAP)
		csProtocol = "IMAP";
	else if(bIsPOP)
		csProtocol = "POP";

	dlg.Run(szPersName, csProtocol);
}





