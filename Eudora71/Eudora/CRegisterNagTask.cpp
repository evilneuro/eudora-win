// CRegisterNagTask.cpp: implementation for the register nagging classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "resource.h"
#include "CRegisterNagTask.h"
#include "PaymentAndRegistrationDlg.h"
#include "RegistrationCodeDlg.h"
#include "rs.h"
#include "guiutils.h"
#include "debug.h"


#include "DebugNewHelpers.h"


//////////////////////////////////////////////////////////////////////
// CRegisterNagDialog: Dialog for register nag
//////////////////////////////////////////////////////////////////////

CRegisterNagDialog::CRegisterNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	CNagDialog(pntNagTask, nIDTemplate, pParentWnd)
{
}

CRegisterNagDialog::~CRegisterNagDialog()
{
}

void CRegisterNagDialog::OnOK()
{
	m_bDoingTask = TRUE;

	// Have the nag do the action (bringing up the registration dialog)
	// It's very important to do this before calling CNagDialog::OnOK, because
	// it will invoke a chain of calls that will end up deleting this object
	// thereby invalidating m_pntNagTask.
	if (m_pntNagTask)
	{
		m_pntNagTask->DoAction();
	}
	
	// Dismiss the dialog
	CNagDialog::OnOK();
}

void CRegisterNagDialog::GetDialogText(CString & out_szDialogText)
{
	// Get the nag dialog wording
	CString		szNagDialogText;
	CWnd *		pNagText = GetDlgItem(IDC_NAGTEXT);

	if (pNagText)
		pNagText->GetWindowText(out_szDialogText);
}


//////////////////////////////////////////////////////////////////////
// CRegisterNagTask: Particular behavior for the register task
//////////////////////////////////////////////////////////////////////

CRegisterNagTask::CRegisterNagTask(CNagTaskEntryP initEntry)
:	CNagTask(initEntry)
{
}

CRegisterNagTask::~CRegisterNagTask()
{
}

bool CRegisterNagTask::DoNag(NagTimes context)
{
// No more nagging about registration
//	#ifdef _DEBUG
//	// See if the nag is being forced.
//	if (GetIniLong(IDS_INI_DEBUG_FORCE_NAG))
//	{
//		// Don't actually have to check the return value for the force nag
//		// INI entry.  If the entry exists and we got here, it is us.
//		return CNagTask::DoNag(context);
//	}
//	#endif
//
//	if ( QCSharewareManager::NeedsRegistrationNag() ) 
//	{
//		return CNagTask::DoNag(context);
//	}
	return false;
}

void CRegisterNagTask::MaybeCancelNag(void *pData)
{
	if (m_active_window && !((CNagDialog*)m_active_window)->GetDoingTask())
	{
		if ( !QCSharewareManager::NeedsRegistrationNag() )
		{
			CancelNag();
		}
	}
}

void CRegisterNagTask::DoAction(int iAction)
{
	// Go to the registration page
	UINT	nDialogMessageStringID = (GetSharewareMode() == SWM_MODE_PRO) ?
									 IDS_WEB_STEP_INFO_DIALOG_PAID_MODE_REG_MSG :
									 IDS_WEB_STEP_INFO_DIALOG_REG_MSG;
	
	//	Display dialog telling the user what the registration steps are, so
	//	the user isn't confused by suddenly being taken to a web page.
	CPaymentAndRegWebStepInfoDlg	dlg( IDD_PAYMENT_REG_INFO,
										 IDS_WEB_STEP_INFO_DIALOG_REG_DLG_TITLE,
										 IDS_WEB_STEP_INFO_DIALOG_REG_MSG_TITLE,
										 nDialogMessageStringID );
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
				// Should only get here when we need the user to register their pro reg code with us
				ASSERT( QCSharewareManager::GetNeedsPaidRegistrationNag() );

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
	}
}

CWnd* CRegisterNagTask::CreateNagWindow()
{
	CRegisterNagDialog		*	pRegisterNagDialog = NULL;

	pRegisterNagDialog = DEBUG_NEW CRegisterNagDialog(this, IDD_NAG_REGISTER, AfxGetMainWnd());
	pRegisterNagDialog->Create(IDD_NAG_REGISTER, AfxGetMainWnd());

	return pRegisterNagDialog;
}
