// PaymentAndRegistrationDlg.cpp : implementation file
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "rs.h"
#include "guiutils.h"
#include "mainfrm.h"
#include "ProfileNag.h"

#include "PaymentAndRegistrationDlg.h"
#include "RegistrationCodeDlg.h"

#include "DebugNewHelpers.h"


CPaymentAndRegWebStepInfoDlg::CPaymentAndRegWebStepInfoDlg(
	UINT			nDialogResID,
	UINT			nDialogTitleStringID,
	UINT			nMessageTitleStringID,
	UINT			nMessageStringID,
	CWnd *			pParent /*=NULL*/)
	:	CDialog(nDialogResID, pParent),
		m_DialogTitle(nDialogTitleStringID),
		m_MessageTitle(nMessageTitleStringID), m_Message(nMessageStringID)
{

}


BOOL CPaymentAndRegWebStepInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//	Change the dialog title.
	SetWindowText(m_DialogTitle);

	return TRUE;
}


void CPaymentAndRegWebStepInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	//{{AFX_DATA_MAP(CPaymentAndRegWebStepInfoDlg)
	DDX_Text(pDX, IDC_MESSAGE_TITLE, m_MessageTitle);
	DDX_Text(pDX, IDC_MESSAGE, m_Message);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CRepayDlg dialog

CRepayDlg::CRepayDlg(CWnd *	pParent /*=NULL*/)
	: CDialog(IDD_REPAY, pParent)
{
	m_Message.Format(IDS_REPAY_DLG_MSG, GetSharewareManager()->DemoDaysLeft());
}

void CRepayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Text(pDX, IDC_REPAY_STATIC, m_Message);
}

BEGIN_MESSAGE_MAP(CRepayDlg, CDialog)
	ON_BN_CLICKED(IDC_REPAY_SPONSORED_MODE_BTN, OnSponsoredModeButton)
	ON_BN_CLICKED(IDC_REPAY_SHOW_VERSIONS_BTN, OnShowVersionsButton)
	ON_BN_CLICKED(IDC_REPAY_PAY_NOW_BTN, OnPayNowButton)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CRepayDlg::OnClose()
{
	QCSharewareManager* pSWM = GetSharewareManager();

	if (pSWM)
	{
		pSWM->SetMode(SWM_MODE_ADWARE, false, NULL);
	}

	CDialog::OnClose();
}

void CRepayDlg::OnSponsoredModeButton()
{
	QCSharewareManager* pSWM = GetSharewareManager();

	if (pSWM)
	{
		pSWM->SetMode(SWM_MODE_ADWARE, false, NULL);
	}
	EndDialog(IDCANCEL);
}

void CRepayDlg::OnShowVersionsButton()
{
//	QCSharewareManager::SetAdwareSwitchTime();
	EndDialog(IDCANCEL);
	CDialog alert(IDD_REPAY_SHOW_VERSIONS_ALERT, NULL);
	alert.DoModal();
	LaunchURLWithQuery(NULL, ACTION_ARCHIVED);
}

void CRepayDlg::OnPayNowButton()
{
//	QCSharewareManager::SetAdwareSwitchTime();
	EndDialog(IDCANCEL);
	CDialog alert(IDD_REPAY_PAY_NOW_ALERT, NULL);
	alert.DoModal();
	LaunchURLWithQuery(NULL, ACTION_PAY);
}
/////////////////////////////////////////////////////////////////////////////
// CJunkDownDlg dialog

CJunkDownDlg::CJunkDownDlg(CWnd *	pParent /*=NULL*/)
	: CDialog(IDD_JUNK_DOWN_DIALOG, pParent)
{
	;
}

BEGIN_MESSAGE_MAP(CJunkDownDlg, CDialog)
	ON_BN_CLICKED(IDC_REPAY_SPONSORED_MODE_BTN, OnOK)
	ON_BN_CLICKED(IDC_ABOUT, OnTellMeMoreButton)
	ON_BN_CLICKED(IDC_REPAY_PAY_NOW_BTN, OnPayNowButton)
END_MESSAGE_MAP()

void CJunkDownDlg::OnOK()
{
	EndDialog(IDCANCEL);
}

void CJunkDownDlg::OnTellMeMoreButton()
{
	LaunchURLWithQuery(NULL, ACTION_SUPPORT,_T("junkDowngrade"));
}

void CJunkDownDlg::OnPayNowButton()
{
	CPaymentAndRegWebStepInfoDlg	dlg( IDD_PAYMENT_REG_INFO,
										 IDS_WEB_STEP_INFO_DIALOG_PAYMENT_DLG_TITLE,
										 IDS_WEB_STEP_INFO_DIALOG_PAYMENT_MSG_TITLE,
										 IDS_WEB_STEP_INFO_DIALOG_PAYMENT_MSG );
	if (dlg.DoModal() == IDOK)
	{
		LaunchURLWithQuery(NULL, ACTION_PAY);
		EndDialog(IDCANCEL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPaymentAndRegistrationDlg dialog


CPaymentAndRegistrationDlg::CPaymentAndRegistrationDlg(CWnd* pParent /*=NULL*/)
	:	CDialog(CPaymentAndRegistrationDlg::IDD, pParent),
		m_ApparentMode( GetSharewareMode() )
{
	if (m_ApparentMode == SWM_MODE_ADWARE)
	{
		const char *p = GetIniString(IDS_INI_PROFILE);
		m_AlreadyProfiled = p && *p;
	}
	else
		m_AlreadyProfiled = FALSE;

	// We pass TRUE to IsRegisteredForCurrentMode(bReally) because we only want to put
	// "Update Your Registration" in the dialog if the user has already gone to the
	// web site and registered. A reg number created by the installer isn't good enough.
	m_AlreadyRegistered = QCSharewareManager::IsRegisteredForCurrentMode(TRUE);

	m_Register = CRString(m_AlreadyRegistered ? IDS_PAR_UPDATE_REG : IDS_PAR_REGISTER);
	m_ProfileYourself = CRString(m_AlreadyProfiled ? IDS_PAR_UPDATE_PROFILE : IDS_PAR_PROFILE);

	//{{AFX_DATA_INIT(CPaymentAndRegistrationDlg)
	//}}AFX_DATA_INIT      
}

void CPaymentAndRegistrationDlg::InitBitmapRadioButton(
	int			nRadioButtonID,
	UINT		nBitmapIDResource,
	CBitmap &	bitmapMember)
{
	CButton *	pRadioButton;

	SECLoadSysColorBitmap(bitmapMember, MAKEINTRESOURCE(nBitmapIDResource), FALSE);
	pRadioButton = reinterpret_cast<CButton *>( GetDlgItem(nRadioButtonID) );
	ASSERT(pRadioButton != NULL);
	if (pRadioButton != NULL)
		pRadioButton->SetBitmap(bitmapMember);
}

BOOL CPaymentAndRegistrationDlg::OnInitDialog()
{
	InitBitmapRadioButton(IDC_PAR_ADWARE, IDB_ADWARE_BUTTON, m_AdwareBitmap);
	InitBitmapRadioButton(IDC_PAR_PAID, IDB_PAID_BUTTON, m_PaidBitmap);
	InitBitmapRadioButton(IDC_PAR_FREE, IDB_FREE_BUTTON, m_FreeBitmap);
	
	m_RegisterButton.AttachButton(IDC_PAR_REGISTER, SECBitmapButton::Al_Center, IDB_REGISTER_BUTTON, this);
	m_DemographicsButton.AttachButton(IDC_PAR_DEMOGRAPHICS, SECBitmapButton::Al_Center, IDB_DEMOGRAPHICS_BUTTON, this);
	m_FindVersionsButton.AttachButton(IDC_PAR_FIND_VERSIONS, SECBitmapButton::Al_Center, IDB_FIND_VERSIONS_BUTTON, this);
	m_ChangeRegistrationButton.AttachButton(IDC_PAR_CHANGE_REGISTRATION, SECBitmapButton::Al_Center, IDB_CHANGE_REGISTRATION_BUTTON, this);

	CDialog::OnInitDialog();

	return TRUE;
}


//	DoDataExchange
//
//	Correctly enables buttons and selects the appropriate mode button based
//	on the specified mode. This is both used to intialize the dialog, and to
//	change the buttons in response to a mode button click.
void CPaymentAndRegistrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CPaymentAndRegistrationDlg)
	//}}AFX_DATA_MAP
	
	DDX_Text(pDX, IDC_PAR_PROFILE, m_ProfileYourself);
	DDX_Text(pDX, IDC_PAR_REGISTER_TEXT, m_Register);

	if (!pDX->m_bSaveAndValidate)
	{
		CString RegistrationName(QCSharewareManager::GetFullNameForCurrentMode());
		CString RegistrationCode(QCSharewareManager::GetRegCodePlusMonthForCurrentMode());

		if (RegistrationName && *RegistrationName)
			DDX_Text(pDX, IDC_PAR_REGISTRATION_NAME, RegistrationName);
		if (RegistrationCode && *RegistrationCode)
			DDX_Text(pDX, IDC_PAR_REGISTRATION_CODE, RegistrationCode);

		CButton *	pAdwareRadioButton = reinterpret_cast<CButton *>( GetDlgItem(IDC_PAR_ADWARE) );
		CButton *	pPaidRadioButton = reinterpret_cast<CButton *>( GetDlgItem(IDC_PAR_PAID) );
		CButton *	pFreeRadioButton = reinterpret_cast<CButton *>( GetDlgItem(IDC_PAR_FREE) );

		ASSERT(pAdwareRadioButton != NULL);
		ASSERT(pPaidRadioButton != NULL);
		ASSERT(pFreeRadioButton != NULL);
		
		if ( (pAdwareRadioButton != NULL) && (pPaidRadioButton != NULL) && (pFreeRadioButton != NULL) )
		{
			pAdwareRadioButton->EnableWindow(TRUE);
			pPaidRadioButton->EnableWindow(TRUE);
			pFreeRadioButton->EnableWindow(TRUE);
		
			switch (m_ApparentMode)
			{
			case SWM_MODE_ADWARE:
				pAdwareRadioButton->SetCheck(BST_CHECKED);
				pFreeRadioButton->SetCheck(BST_UNCHECKED);
				pPaidRadioButton->SetCheck(BST_UNCHECKED);
				m_RegisterButton.EnableWindow(TRUE);
				m_DemographicsButton.EnableWindow(TRUE);
				break;
			case SWM_MODE_LIGHT:
				pAdwareRadioButton->SetCheck(BST_UNCHECKED);
				pFreeRadioButton->SetCheck(BST_CHECKED);
				pPaidRadioButton->SetCheck(BST_UNCHECKED);
				m_RegisterButton.EnableWindow(TRUE);
				m_DemographicsButton.EnableWindow(FALSE);
				break;
			case SWM_MODE_PRO:
				pAdwareRadioButton->SetCheck(BST_UNCHECKED);
				pFreeRadioButton->SetCheck(BST_UNCHECKED);
				pPaidRadioButton->SetCheck(BST_CHECKED);
				m_RegisterButton.EnableWindow(TRUE);
				m_DemographicsButton.EnableWindow(FALSE);
				break;
			}
		}

		// The .ini file may have a setting to disable the Register With Us button; this is
		// for site licenses.
		(GetDlgItem(IDC_PAR_REGISTER))->EnableWindow(!GetIniShort(IDS_INI_NEVER_REGISTER));

		CWnd* pDemoDaysLeftControl = GetDlgItem(IDC_PAR_DEMO_DAYS_LEFT);
		pDemoDaysLeftControl->ShowWindow(FALSE);
	}
}


BEGIN_MESSAGE_MAP(CPaymentAndRegistrationDlg, CDialog)
	//{{AFX_MSG_MAP(CPaymentAndRegistrationDlg)
	ON_BN_CLICKED(IDC_PAR_ADWARE, OnAdwareButton)
	ON_BN_CLICKED(IDC_PAR_PAID, OnPaidButton)
	ON_BN_CLICKED(IDC_PAR_FREE, OnFreeButton)
	ON_BN_CLICKED(IDC_PAR_REGISTER, OnRegisterButton)
	ON_BN_CLICKED(IDC_PAR_DEMOGRAPHICS, OnDemographicsButton)
	ON_BN_CLICKED(IDC_PAR_FIND_VERSIONS, OnFindVersionsButton)
	ON_BN_CLICKED(IDC_PAR_CHANGE_REGISTRATION, OnChangeRegistrationButton)
	ON_BN_CLICKED(IDC_PAR_TELL_ME_MORE, OnTellMeMoreButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaymentAndRegistrationDlg message handlers
void CPaymentAndRegistrationDlg::ChangeMode(SharewareModeType NewMode)
{
	bool	bChangedMode = false;
	
	QCSharewareManager* pSWM = GetSharewareManager();

	if (pSWM)
	{
		bChangedMode = pSWM->SetMode(NewMode, true, this, true);
		if (bChangedMode)
			EndDialog(IDCANCEL);
	}

	if (!bChangedMode)
	{
		//	Revert to the correct mode button and other information
		m_ApparentMode = GetSharewareMode();
		UpdateData(FALSE);
	}
}

void CPaymentAndRegistrationDlg::OnAdwareButton() 
{
	m_ApparentMode = SWM_MODE_ADWARE;
	UpdateData(FALSE);

	QCSharewareManager* pSWM = GetSharewareManager();
	ASSERT( pSWM );

	//	Do refund if they're registered for pro and the shift key is down
	if ( pSWM && pSWM->IsPaidModeOK() && ShiftDown() )
	{
		//	Do the refund smoke & mirrors
		if ( pSWM->DoRefund(this) )
			EndDialog(IDCANCEL);
	}
	else
	{
		ChangeMode(SWM_MODE_ADWARE);
	}
}

void CPaymentAndRegistrationDlg::OnPaidButton() 
{
	m_ApparentMode = SWM_MODE_PRO;
	UpdateData(FALSE);

	QCSharewareManager* pSWM = GetSharewareManager();

	// If they've already registered as PRO, then let them switch to PRO mode,
	// otherwise launch the Payment web page.
	if (pSWM && pSWM->IsPaidModeOK())
	{
		ChangeMode(SWM_MODE_PRO);
	}
	else
	{
		{
			//	Display dialog telling the user what the payment steps are, so
			//	the user isn't confused by suddenly being taken to a web page.
			CPaymentAndRegWebStepInfoDlg	dlg( IDD_PAYMENT_REG_INFO,
												IDS_WEB_STEP_INFO_DIALOG_PAYMENT_DLG_TITLE,
												IDS_WEB_STEP_INFO_DIALOG_PAYMENT_MSG_TITLE,
												IDS_WEB_STEP_INFO_DIALOG_PAYMENT_MSG );
			int		nResult = dlg.DoModal();

			if (nResult == IDOK)
			{
				LaunchURLWithQuery(NULL, ACTION_PAY);

				//	We already explained the steps to the user and they chose to continue
				//	Leaving the dialog open is likely to confuse them
				EndDialog(IDCANCEL);
			}
			else
			{
				//	Revert to the correct mode button and other information
				m_ApparentMode = GetSharewareMode();
				UpdateData(FALSE);
			}
		}
	}
}

void CPaymentAndRegistrationDlg::OnFreeButton() 
{
	m_ApparentMode = SWM_MODE_LIGHT;
	UpdateData(FALSE);

	ChangeMode(SWM_MODE_LIGHT);
}

void CPaymentAndRegistrationDlg::OnRegisterButton() 
{
	UINT			nDialogResID;
	UINT			nDialogTitleStringID, nMessageTitleStringID, nMessageStringID;

	if (m_AlreadyRegistered)
	{
		nDialogResID = IDD_UPDATE_REG_INFO;
		nDialogTitleStringID = IDS_WEB_STEP_INFO_DIALOG_UPDATE_REG_DLG_TITLE;
		nMessageTitleStringID = IDS_WEB_STEP_INFO_DIALOG_UPDATE_REG_MSG_TITLE;
		nMessageStringID = IDS_WEB_STEP_INFO_DIALOG_UPDATE_REG_MSG;
	}
	else
	{
		nDialogResID = IDD_PAYMENT_REG_INFO;
		nDialogTitleStringID = IDS_WEB_STEP_INFO_DIALOG_REG_DLG_TITLE;
		nMessageTitleStringID = IDS_WEB_STEP_INFO_DIALOG_REG_MSG_TITLE;
		nMessageStringID = (GetSharewareMode() == SWM_MODE_PRO) ?
									 IDS_WEB_STEP_INFO_DIALOG_PAID_MODE_REG_MSG :
									 IDS_WEB_STEP_INFO_DIALOG_REG_MSG;

	}
	
	//	Display dialog telling the user what the registration steps are, so
	//	the user isn't confused by suddenly being taken to a web page.
	CPaymentAndRegWebStepInfoDlg	dlg( nDialogResID, nDialogTitleStringID,
										 nMessageTitleStringID, nMessageStringID );

	int		nResult = dlg.DoModal();

	if (nResult == IDOK)
	{	
		switch (GetSharewareMode())
		{
			case SWM_MODE_ADWARE:
				LaunchURLWithQuery (NULL, ACTION_REGISTER_AD);
				break;

			case SWM_MODE_LIGHT:
				LaunchURLWithQuery (NULL, ACTION_REGISTER_FREE);
				break;

			case SWM_MODE_PRO:
			{
				CString firstName;
				CString lastName;
				CString regCode;
				CString regLevel;
				bool isFakeRegInfo = false;;

				QCSharewareManager::GetPaidModeRegInfo(regCode, firstName, lastName, regLevel, &isFakeRegInfo);
				LaunchURLWithQuery (NULL, isFakeRegInfo? ACTION_REGISTER_50BOX : ACTION_REGISTER_PAID, NULL,
									regCode, firstName, lastName, regLevel);
				break;
			}

			default:
				// Shouldn't get here
				ASSERT(0);
				return;
		}

		//	We already explained the steps to the user and they chose to continue
		//	Leaving the dialog open is likely to confuse them
		EndDialog(IDCANCEL);
	}
}

void CPaymentAndRegistrationDlg::OnDemographicsButton() 
{
	UINT			nDialogResID;
	UINT			nDialogTitleStringID, nMessageTitleStringID, nMessageStringID;

	if (m_AlreadyProfiled)
	{
		//	We already had a profile ID
		nDialogResID = IDD_PAYMENT_REG_INFO_SMALL;
		nDialogTitleStringID = IDS_WEB_STEP_INFO_DIALOG_UPDATE_PROFILE_DLG_TITLE;
		nMessageTitleStringID = IDS_WEB_STEP_INFO_DIALOG_UPDATE_PROFILE_MSG_TITLE;
		nMessageStringID = IDS_WEB_STEP_INFO_DIALOG_UPDATE_PROFILE_MSG;
	}
	else
	{
		//	This will be our first profile ID
		nDialogResID = IDD_PAYMENT_REG_INFO;
		nDialogTitleStringID = IDS_WEB_STEP_INFO_DIALOG_CUSTOMIZE_ADS_DLG_TITLE;
		nMessageTitleStringID = IDS_WEB_STEP_INFO_DIALOG_CUSTOMIZE_ADS_MSG_TITLE;
		nMessageStringID = IDS_WEB_STEP_INFO_DIALOG_CUSTOMIZE_ADS_MSG;
	}
	
	//	Display dialog telling the user what the ad customization steps are, so
	//	the user isn't confused by suddenly being taken to a web page.
	CPaymentAndRegWebStepInfoDlg	dlg( nDialogResID, nDialogTitleStringID,
										 nMessageTitleStringID, nMessageStringID );
	int		nResult = dlg.DoModal();

	if (nResult == IDOK)
	{
		LaunchURLWithQuery(NULL, ACTION_PROFILE);

		//	We already explained the steps to the user and they chose to continue
		//	Leaving the dialog open is likely to confuse the user
		EndDialog(IDCANCEL);
	}
}

void CPaymentAndRegistrationDlg::OnFindVersionsButton() 
{
	LaunchURLWithQuery(NULL, ACTION_ARCHIVED);
}

void CPaymentAndRegistrationDlg::OnChangeRegistrationButton() 
{
	CRegistrationCodeDlg dlg(this);

	if (dlg.DoModal() == IDOK)
	{
		m_ApparentMode = GetSharewareMode();
		UpdateData(FALSE);
	}
}

void CPaymentAndRegistrationDlg::OnTellMeMoreButton() 
{
	LaunchURLWithQuery(NULL, ACTION_INTRO);
}


BOOL CPaymentAndRegistrationDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( (pMsg->message == WM_KEYDOWN) &&
		 (pMsg->wParam == VK_RETURN) )
	{
		CWnd* pWndFocus = GetFocus();
		if (pWndFocus != NULL)
		{
			if ( (pWndFocus->GetDlgCtrlID() != IDOK) || (pWndFocus->GetDlgCtrlID() != IDCANCEL) )
			{
				::SendMessage( this->m_hWnd, WM_COMMAND, (WPARAM)MAKEWPARAM(pWndFocus->GetDlgCtrlID(),BN_CLICKED), (LPARAM)pWndFocus->m_hWnd);
				return TRUE;

			}
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
