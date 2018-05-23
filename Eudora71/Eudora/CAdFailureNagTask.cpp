// CAdFailureNagTask.cpp: implementation for the ad failure nagging classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "resource.h"
#include "CAdFailureNagTask.h"
#include "rs.h"
#include "guiutils.h"
#include "debug.h"
#include "mainfrm.h"
#include "plist_mgr.h"


#include "DebugNewHelpers.h"


const int		nDaysUntilAdFailure = 4;
const int		nDaysUntilDeadbeat = kAdFailureGraceDays;


//////////////////////////////////////////////////////////////////////
// CAdFailureNagDialog: Dialog for ad failure nag
//////////////////////////////////////////////////////////////////////

CAdFailureNagDialog::CAdFailureNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	CNagDialog(pntNagTask, nIDTemplate, pParentWnd)
{
}

CAdFailureNagDialog::~CAdFailureNagDialog()
{
}

void CAdFailureNagDialog::OnOK()
{
	if (m_pntNagTask)
	{
		m_pntNagTask->DoAction();
	}

	// Dismiss the dialog
	CNagDialog::OnOK();
}

void CAdFailureNagDialog::OnCancel()
{
	if (m_pntNagTask)
	{
		m_pntNagTask->DoAction(IDCANCEL);
	}

	// Dismiss the dialog
	CNagDialog::OnCancel();
}

void CAdFailureNagDialog::GetDialogText(CString & out_szDialogText)
{
	// Get the nag dialog wording
	CString		szNagDialogText;
	CWnd *		pNagText = GetDlgItem(IDC_NAGTEXT);

	if (pNagText)
		pNagText->GetWindowText(out_szDialogText);
}


//////////////////////////////////////////////////////////////////////
// CDeadbeatNagDialog: Dialog for deadbeat nag
//////////////////////////////////////////////////////////////////////

CDeadbeatNagDialog::CDeadbeatNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	CNagDialog(pntNagTask, nIDTemplate, pParentWnd),
	m_bCancelHit(FALSE)
{
	m_lpszText.LoadString(IDS_NAG_DEADBEAT_TEXT);
}

CDeadbeatNagDialog::~CDeadbeatNagDialog()
{
}

void CDeadbeatNagDialog::OnOK()
{
	m_bDoingTask = TRUE;
	if (m_pntNagTask)
	{
		m_pntNagTask->DoAction();
	}

	// Dismiss the dialog
	CNagDialog::OnOK();
}

void CDeadbeatNagDialog::OnCancel()
{
	m_bDoingTask = TRUE;
	if (m_pntNagTask)
	{
		if (m_bCancelHit)
		{
			m_pntNagTask->DoAction(IDCANCEL);
		}
		else
		{
			m_pntNagTask->DoAction(IDOK);
		}
	}

	// Dismiss the dialog
	CNagDialog::OnCancel();
}

BOOL CDeadbeatNagDialog::OnCommand(WPARAM wParam, LPARAM lParam)
	// return TRUE if command invocation was attempted
{
	if (wParam == IDC_WEBSITE)
	{
		m_bCancelHit = TRUE;
		m_pntNagTask->DoAction(IDCANCEL);
		EndDialog(lParam);
	}

	return CNagDialog::OnCommand(wParam, lParam);
}

void CDeadbeatNagDialog::DoDataExchange(CDataExchange* pDX)
{
	CNagDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_DEADBEAT_STATIC, m_lpszText);
}

void CDeadbeatNagDialog::GetDialogText(CString & out_szDialogText)
{
	out_szDialogText = m_lpszText;
}


//////////////////////////////////////////////////////////////////////
// CAdFailureNagTask: Particular behavior for the ad failure task
//////////////////////////////////////////////////////////////////////

CAdFailureNagTask::CAdFailureNagTask(CNagTaskEntryP initEntry)
:	CNagTask(initEntry),
	m_bIsDeadbeat(FALSE),
	m_tLastAdRetrieval(0)
{
}

CAdFailureNagTask::~CAdFailureNagTask()
{
}

bool CAdFailureNagTask::DoNag(NagTimes context)
{
	// Only nag if we are in adware mode.
	if (GetSharewareMode() == SWM_MODE_ADWARE)
	{
		BOOL		bAdFailure = FALSE;
		BOOL		bGettingWorse;
		int				adFailureDays = CMainFrame::GetAdFailure();

		time_t theHardWay;
		bGettingWorse = MGR_GetClientUpdateStatus( CMainFrame::QCGetMainFrame()->GetLDB(), &theHardWay );

		if (adFailureDays >= nDaysUntilDeadbeat)
		{
			m_bIsDeadbeat = TRUE;
		}
		else if (adFailureDays >= nDaysUntilAdFailure)
		{
			bAdFailure = TRUE;
		}
		#ifdef _DEBUG
		if (GetIniLong(IDS_INI_DEBUG_FORCE_NAG))
		{
			// Don't actually have to check the return value for the force nag
			// INI entry.  If the entry exists and we got here, it is us.
			bAdFailure = TRUE;
			bGettingWorse = TRUE;
		}
		#endif
		if (m_bIsDeadbeat || bAdFailure && bGettingWorse)
		{
			return CNagTask::DoNag(context);
		}
	}

	return false;
}

void CAdFailureNagTask::MaybeCancelNag(void *pData)
{
	// TO BE DONE: Use real test for whether ads are being received.
	if (m_active_window && !((CNagDialog*)m_active_window)->GetDoingTask())
	{
		BOOL		bReceivingAds = FALSE;
		if (bReceivingAds)
		{
			CancelNag();
		}
	}
}

void CAdFailureNagTask::DoAction(int iAction)
{
	if (m_bIsDeadbeat)
	{
		if (iAction == IDOK)
		{
			QCSharewareManager* pSWM = GetSharewareManager();
			if (pSWM)
			{
				if (GetSharewareMode() == SWM_MODE_LIGHT) // Already in LIGHT
				{
					ASSERT(0);
				}
				else
				{
					pSWM->SetMode(SWM_MODE_LIGHT, FALSE);
				}
			}
		}
		else if (iAction == IDCANCEL)
		{
			// go to eudora web page
			LaunchURLWithQuery(NULL, ACTION_SUPPORT, _T("ad-fail"));
		}
	}
	else
	{
		if (iAction == IDOK)
		{
			// go to eudora web page
			LaunchURLWithQuery(NULL, ACTION_SUPPORT, _T("ad-fail"));
		}
	}
}

CWnd* CAdFailureNagTask::CreateNagWindow()
{
	CNagDialog	*	pNagDialog = NULL;

// No longer nagging about ad failure
//	
//	if (m_bIsDeadbeat)
//	{
//		CDeadbeatNagDialog		*	pDeadbeatNagDialog = NULL;
//		pDeadbeatNagDialog = DEBUG_NEW CDeadbeatNagDialog(this, IDD_NAG_DEADBEAT, AfxGetMainWnd());
//		pDeadbeatNagDialog->Create(IDD_NAG_DEADBEAT, AfxGetMainWnd());
//		pNagDialog = pDeadbeatNagDialog;
//	}
//	else
//	{
//		CAdFailureNagDialog		*	pAdFailureNagDialog = NULL;
//		pAdFailureNagDialog = DEBUG_NEW CAdFailureNagDialog(this, IDD_NAG_ADFAILURE, AfxGetMainWnd());
//		pAdFailureNagDialog->Create(IDD_NAG_ADFAILURE, AfxGetMainWnd());
//		pNagDialog = pAdFailureNagDialog;
//	}

	return pNagDialog;
}
