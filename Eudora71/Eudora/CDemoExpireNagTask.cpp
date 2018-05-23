// CDemoExpireNagTask.cpp: implementation for the demo expire nagging classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "resource.h"
#include "CDemoExpireNagTask.h"
#include "QCSharewareManager.h"
#include "debug.h"
#include "guiutils.h"

#include "DebugNewHelpers.h"


#define DEMO_DAYS_LEFT_WARN 10

//////////////////////////////////////////////////////////////////////
// CDemoExpireNagDialog: Dialog for demo expire nag
//////////////////////////////////////////////////////////////////////

CDemoExpireNagDialog::CDemoExpireNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	CNagDialog(pntNagTask, nIDTemplate, pParentWnd)
{
}

CDemoExpireNagDialog::~CDemoExpireNagDialog()
{
}

void CDemoExpireNagDialog::OnOK()
{
	m_bDoingTask = TRUE;

	if (m_pntNagTask)
	{
		m_pntNagTask->DoAction();
	}

	// Dismiss the dialog
	CNagDialog::OnOK();
}

BOOL CDemoExpireNagDialog::OnCommand(WPARAM wParam, LPARAM lParam)
	// return TRUE if command invocation was attempted
{
	if (wParam == IDC_DEMO_PAY_NOW_BTN)
	{
		m_pntNagTask->DoAction(wParam);
		EndDialog(lParam);
	}

	return CNagDialog::OnCommand(wParam, lParam);
}

void CDemoExpireNagDialog::DoDataExchange(CDataExchange* pDX)
{
	CNagDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate == FALSE)
	{
		if (GetDlgItem(IDC_NAGTEXT))
		{
			CString Existing;
			CString New;

			GetDlgItemText(IDC_NAGTEXT, Existing);
			New.Format(Existing, GetSharewareManager()->DemoDaysLeft());
			SetDlgItemText(IDC_NAGTEXT, New);
		}
	}
}

void CDemoExpireNagDialog::GetDialogText(CString & out_szDialogText)
{
	// Get the nag dialog wording
	GetDlgItemText(IDC_NAGTEXT, out_szDialogText);
}

//////////////////////////////////////////////////////////////////////
// CDemoExpireNagTask: Particular behavior for the demo expire task
//////////////////////////////////////////////////////////////////////

CDemoExpireNagTask::CDemoExpireNagTask(CNagTaskEntryP initEntry)
:	CNagTask(initEntry)
{
}

CDemoExpireNagTask::~CDemoExpireNagTask()
{
}

bool CDemoExpireNagTask::DoNag(NagTimes context)
{
// Shouldn't get here anymore, but just in case there's a loophole somewhere, just comment the Demo nag out
//
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
//	QCSharewareManager* pSWM = GetSharewareManager();
//	if (pSWM)
//	{
//		if (GetSharewareMode() == SWM_MODE_PRO &&
//			pSWM->IsPaidModeOK() == FALSE &&
//			pSWM->DemoDaysLeft() <= DEMO_DAYS_LEFT_WARN)
//		{
//			return CNagTask::DoNag(context);
//		}
//	}
	return false;
}

void CDemoExpireNagTask::MaybeCancelNag(void *pData)
{
	if (m_active_window && !((CNagDialog*)m_active_window)->GetDoingTask())
	{
		QCSharewareManager* pSWM = GetSharewareManager();
		if (pSWM)
		{
			if (GetSharewareMode() != SWM_MODE_PRO || pSWM->IsPaidModeOK())
			{
				CancelNag();
			}
		}
	}
}

void CDemoExpireNagTask::DoAction(int iAction)
{
	if (iAction == IDC_DEMO_PAY_NOW_BTN)
		LaunchURLWithQuery(NULL, ACTION_PAY);

	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM->DemoDaysLeft() == 0)
		pSWM->SetMode(SWM_MODE_ADWARE, false);
}

CWnd* CDemoExpireNagTask::CreateNagWindow()
{
	CDemoExpireNagDialog		*	pDemoExpireNagDialog = NULL;
	UINT nID = GetSharewareManager()->DemoDaysLeft() > 0? IDD_NAG_DEMOEXPIRE : IDD_NAG_DEMOEXPIRED;

	pDemoExpireNagDialog = DEBUG_NEW CDemoExpireNagDialog(this, nID, AfxGetMainWnd());
	pDemoExpireNagDialog->Create(nID, AfxGetMainWnd());

	return pDemoExpireNagDialog;
}
