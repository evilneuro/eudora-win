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

#include "ModifyAcctDlgs.h"
#include "QCTaskManager.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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

#ifdef WIN32
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_MESSAGE(WM_HELP, OnHelp)	
#endif
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
#ifdef WIN32
	ON_MESSAGE(WM_RBUTTONUP, OnRightButton)
#endif
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
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

#endif

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

#ifdef WIN32
long CPropHelpxDlg::OnRightButton(WPARAM wParam, LPARAM lParam)
{
	if (IsWin32s() || (IsWinNT() &&	GetMajorVersion() == 3 && GetMinorVersion() < 51))
		return (OnContextMenu(wParam, lParam));
	return (CDialog::DefWindowProc(WM_RBUTTONUP, wParam, lParam));
}
#endif



LRESULT CPropHelpxDlg::OnCommandHelp(WPARAM, LPARAM lParam)
{  
#ifndef WIN32
	CWnd* pWnd = GetFocus();
	int nID = pWnd->GetDlgCtrlID();
	WinHelp(nID+HID_OFFSET,HELP_CONTEXTPOPUP);
#endif
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
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CModifyAcct_Generic, CPropHelpxDlg)
	//{{AFX_MSG_MAP(CModifyAcct_Generic)
	ON_EN_CHANGE(IDC_PERSONA_NAME_EDIT, OnChangePersonaNameEdit)
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
		GetDlgItem(IDC_PERSONA_NAME_EDIT)->GetWindowText(pParams->PersName);

	GetDlgItem(IDS_INI_PERSONA_REAL_NAME)->GetWindowText(pParams->RealName);
	GetDlgItem(IDS_INI_PERSONA_DEFAULT_DOMAIN)->GetWindowText(pParams->DefaultDomain);
	GetDlgItem(IDS_INI_PERSONA_RETURN_ADDR)->GetWindowText(pParams->ReturnAddress);
	GetDlgItem(IDS_INI_PERSONA_LOGIN_NAME)->GetWindowText(pParams->LoginName);
	GetDlgItem(IDS_INI_PERSONA_SMTP_SERVER)->GetWindowText(pParams->OutServer);

	// CheckMail
	const CButton* pButton = (CButton *) GetDlgItem(IDS_INI_PERSONA_CHECK_MAIL);
	pParams->bCheckMail = (pButton->GetCheck() != 0);

	// Signature
	{
		m_comboSignature.GetWindowText(pParams->Signature);

		if (pParams->Signature == CRString(IDS_NO_DEFAULT))
			pParams->Signature.Empty();
	}

	// Stationary
	{
		m_comboStationery.GetWindowText(pParams->Stationery);

		if (pParams->Stationery == CRString(IDS_NO_DEFAULT))
			pParams->Stationery.Empty();
	}

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

		GetDlgItem(IDC_PERSONA_NAME_EDIT)->SetWindowText(pParams->PersName);
		OnChangePersonaNameEdit();
	}
	else
	{
		GetDlgItem(IDC_PERSONA_NAME_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PERSONA_NAME_EDIT)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_PERSONA_NAME_STATIC)->SetWindowText(pParams->PersName);
	}

	GetDlgItem(IDS_INI_PERSONA_REAL_NAME)->SetWindowText(pParams->RealName);
	GetDlgItem(IDS_INI_PERSONA_DEFAULT_DOMAIN)->SetWindowText(pParams->DefaultDomain);
	GetDlgItem(IDS_INI_PERSONA_RETURN_ADDR)->SetWindowText(pParams->ReturnAddress);
	GetDlgItem(IDS_INI_PERSONA_LOGIN_NAME)->SetWindowText(pParams->LoginName);
	GetDlgItem(IDS_INI_PERSONA_SMTP_SERVER)->SetWindowText(pParams->OutServer);
	GetDlgItem(IDS_INI_PERSONA_SMTP_SERVER)->EnableWindow(TRUE);

	// CheckMail
	((CButton *) GetDlgItem(IDS_INI_PERSONA_CHECK_MAIL))->SetCheck(pParams->bCheckMail);

	// Signature
	{
		int nIndex = m_comboSignature.FindStringExact(-1, pParams->Signature);
		if ( nIndex >= 0 )
			m_comboSignature.SetCurSel(nIndex);
		else
			m_comboSignature.SetCurSel(0);
	}

	// Stationary
	{
		int nIndex = m_comboStationery.FindStringExact(-1, pParams->Stationery);
		if ( nIndex >= 0 )
			m_comboStationery.SetCurSel(nIndex);
		else
			m_comboStationery.SetCurSel(0);
	}

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
	//}}AFX_DATA_MAP

	// my own code
	if( pDX->m_bSaveAndValidate) {
		DWORD Val ;
		DDX_Text( pDX, IDS_INI_BIG_MESSAGE_THRESHOLD, Val) ;
		DDX_Text( pDX, IDS_INI_IMAP_MAXSIZE, Val) ;
		DDX_Text( pDX, IDS_INI_LEAVE_ON_SERVER_DAYS, Val) ;
		DDX_Text( pDX, IDS_INI_IMAP_TRASH_MBOXNAME, Val) ;

		// Special check for LMOS
		CWnd *pTemp	=	GetDlgItem( IDS_INI_DELETE_MAIL_FROM_SERVER) ;
		if( pTemp) {
			if( ((CButton*) pTemp)->GetCheck() == 1) {
				// See if positive value in the LMOS field
				if( Val == 0) {
					ErrorDialog(IDS_ERR_SERVER_DELETE_X_DAYS);
					pDX->PrepareEditCtrl(IDS_INI_LEAVE_ON_SERVER_DAYS);
					pDX->Fail();
				}
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CModifyAcct_Servers, CPropHelpxDlg)
	//{{AFX_MSG_MAP(CModifyAcct_Servers)
	ON_BN_CLICKED( IDS_INI_USES_POP, OnPop)
	ON_BN_CLICKED( IDS_INI_USES_IMAP, OnImap)
	ON_BN_CLICKED(IDS_INI_IMAP_TRASH_MBOXNAME, OnImapTrashSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

BOOL CModifyAcct_Servers::OnInitDialog() 
{
	ASSERT(m_pParent);
	const CPersParams *pParams = &(m_pParent->m_params);
	CWnd* Ctrl;

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

	GetDlgItem(IDS_INI_POP_SERVER)->GetWindowText(pParams->InServer);
	GetDlgItem(IDS_INI_IMAP_PREFIX)->GetWindowText(pParams->IMAPPrefix);
	GetDlgItem(IDS_INI_LEAVE_ON_SERVER_DAYS)->GetWindowText(pParams->LeaveOnServDays);
	GetDlgItem(IDS_INI_IMAP_MAXSIZE)->GetWindowText(pParams->IMAPMaxSize);
	GetDlgItem(IDS_INI_IMAP_TRASH_MBOXNAME)->GetWindowText(pParams->IMAPTrashMailbox);

	{
		CString str;
		GetDlgItem(IDS_INI_BIG_MESSAGE_THRESHOLD)->GetWindowText(str);
		const long Num = atol(str) * 1024L;

		pParams->BigMsgThreshold.Format("%ld", Num);
	}

	pParams->bPOP = (((CButton *) GetDlgItem(IDS_INI_USES_POP))->GetCheck() != 0);
	pParams->bIMAP = (((CButton *) GetDlgItem(IDS_INI_USES_IMAP))->GetCheck() != 0);
	pParams->bLMOS = (((CButton *) GetDlgItem(IDS_INI_PERSONA_LMOS))->GetCheck() != 0);
	pParams->bPassword = (((CButton *) GetDlgItem(IDS_INI_AUTH_PASS))->GetCheck() != 0);
	pParams->bAPop = (((CButton *) GetDlgItem(IDS_INI_AUTH_APOP))->GetCheck() != 0);
	pParams->bKerberos = (((CButton *) GetDlgItem(IDS_INI_AUTH_KERB))->GetCheck() != 0);
	pParams->bRPA = (((CButton *) GetDlgItem(IDS_INI_AUTH_RPA))->GetCheck() != 0);
	pParams->bDelServerAfter = (((CButton *) GetDlgItem(IDS_INI_DELETE_MAIL_FROM_SERVER))->GetCheck() != 0);
	pParams->bDelWhenTrashed = (((CButton *) GetDlgItem(IDS_INI_SERVER_DELETE))->GetCheck() != 0);
	pParams->bSkipBigMsgs = (((CButton *) GetDlgItem(IDS_INI_SKIP_BIG_MESSAGES))->GetCheck() != 0);
	pParams->bIMAPMinDwnld = (((CButton *) GetDlgItem(IDS_INI_IMAP_MINDNLOAD))->GetCheck() != 0);
	pParams->bIMAPFullDwnld = (((CButton *) GetDlgItem(IDS_INI_IMAP_OMITATTACH))->GetCheck() != 0);
	pParams->bIMAPXferToTrash = (((CButton *) GetDlgItem(IDS_INI_IMAP_XFERTOTRASH))->GetCheck() != 0);
	pParams->bIMAPMarkDeleted = (((CButton *) GetDlgItem(IDS_INI_IMAP_MARK_DELETED))->GetCheck() != 0);

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

	GetDlgItem(IDS_INI_POP_SERVER)->SetWindowText(pParams->InServer);
	GetDlgItem(IDS_INI_IMAP_PREFIX)->SetWindowText(pParams->IMAPPrefix);
	GetDlgItem(IDS_INI_LEAVE_ON_SERVER_DAYS)->SetWindowText(pParams->LeaveOnServDays);
	GetDlgItem(IDS_INI_IMAP_MAXSIZE)->SetWindowText(pParams->IMAPMaxSize);
	GetDlgItem(IDS_INI_IMAP_TRASH_MBOXNAME)->SetWindowText(pParams->IMAPTrashMailbox);

	{
		// Big Message Threshold is in K in dlg, but stored in bytes
		const long Num = atol(pParams->BigMsgThreshold) / 1024L;
		CString str;
		str.Format("%ld", Num);
		GetDlgItem(IDS_INI_BIG_MESSAGE_THRESHOLD)->SetWindowText(str);
	}

	((CButton *) GetDlgItem(IDS_INI_USES_POP))->SetCheck(pParams->bPOP);
	((CButton *) GetDlgItem(IDS_INI_USES_IMAP))->SetCheck(pParams->bIMAP);

	((CButton *) GetDlgItem(IDS_INI_PERSONA_LMOS))->SetCheck(pParams->bLMOS);
	((CButton *) GetDlgItem(IDS_INI_AUTH_PASS))->SetCheck(pParams->bPassword);
	((CButton *) GetDlgItem(IDS_INI_AUTH_APOP))->SetCheck(pParams->bAPop);
	((CButton *) GetDlgItem(IDS_INI_AUTH_KERB))->SetCheck(pParams->bKerberos);
	((CButton *) GetDlgItem(IDS_INI_AUTH_RPA))->SetCheck(pParams->bRPA);
	((CButton *) GetDlgItem(IDS_INI_DELETE_MAIL_FROM_SERVER))->SetCheck(pParams->bDelServerAfter);
	((CButton *) GetDlgItem(IDS_INI_SERVER_DELETE))->SetCheck(pParams->bDelWhenTrashed);
	((CButton *) GetDlgItem(IDS_INI_SKIP_BIG_MESSAGES))->SetCheck(pParams->bSkipBigMsgs);
	((CButton *) GetDlgItem(IDS_INI_IMAP_MINDNLOAD))->SetCheck(pParams->bIMAPMinDwnld);
	((CButton *) GetDlgItem(IDS_INI_IMAP_OMITATTACH))->SetCheck(pParams->bIMAPFullDwnld);

	((CButton *) GetDlgItem(IDS_INI_IMAP_XFERTOTRASH))->SetCheck(pParams->bIMAPXferToTrash);
	((CButton *) GetDlgItem(IDS_INI_IMAP_MARK_DELETED))->SetCheck(pParams->bIMAPMarkDeleted);

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
			((CButton*)GetDlgItem(nCheckedID))->SetCheck(0); // Uncheck hidden check
	}

	if (bSetCheck)
		((CButton*)GetDlgItem(IDS_INI_AUTH_PASS))->SetCheck(1);
}


void CModifyAcct_Servers::OnPop()
{
	ASSERT(m_pParent);
	CPersParams *pParams = &(m_pParent->m_params);

	if( HasActiveTasks())
		return;

	ShowServerSpecificControls(SERV_POP);
}

void CModifyAcct_Servers::OnImap()
{
	ASSERT(m_pParent);
	CPersParams *pParams = &(m_pParent->m_params);

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
	if( ((CButton *) GetDlgItem(m_nOriginalServTypeID))->GetCheck() == 0)
	{
		if( QCGetTaskManager()->GetTaskCount(pParams->PersName) != 0)
		{
			AfxMessageBox(CRString(IDS_CANT_CHANGE_PERSONA));
			((CButton *) GetDlgItem(IDS_INI_USES_POP))->SetCheck(pParams->bPOP);
			((CButton *) GetDlgItem(IDS_INI_USES_IMAP))->SetCheck(pParams->bIMAP);

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
	ShowControl(IDS_INI_IMAP_TRASH_MBOXNAME,	bIMAP);
	
	// Change the text of the "APOP" to "CRAM-MD5" for IMAP
	GetDlgItem(IDS_INI_AUTH_APOP)->SetWindowText(bIMAP ? "&Cram-MD5" : "&APOP");

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

