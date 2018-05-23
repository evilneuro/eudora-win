// CFullFeatureNagTask.cpp: implementation for the full feature nagging classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "resource.h"
#include "CFullFeatureNagTask.h"
#include "QCSharewareManager.h"
#include "debug.h"


#include "DebugNewHelpers.h"


//////////////////////////////////////////////////////////////////////
// CFullFeatureNagDialog: Dialog for full feature nag
//////////////////////////////////////////////////////////////////////

CFullFeatureNagDialog::CFullFeatureNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	CNagDialog(pntNagTask, nIDTemplate, pParentWnd)
{
	m_lpszText.LoadString(IDS_NAG_FULLFEATURED_TEXT);
}

CFullFeatureNagDialog::~CFullFeatureNagDialog()
{
}

void CFullFeatureNagDialog::OnOK()
{
	m_bDoingTask = TRUE;

	if (m_pntNagTask)
	{
		m_pntNagTask->DoAction();
	}

	// Dismiss the dialog
	CNagDialog::OnOK();
}

void CFullFeatureNagDialog::DoDataExchange(CDataExchange* pDX)
{
	CNagDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MISSINGFEATURESTEXT, m_lpszText);
}

void CFullFeatureNagDialog::GetDialogText(CString & out_szDialogText)
{
	// Get the nag dialog wording
	CString		szNagDialogText;
	CWnd *		pNagText = GetDlgItem(IDC_NAGTEXT);

	if (pNagText)
	{
		pNagText->GetWindowText(out_szDialogText);
		out_szDialogText += "\r\n";
	}

	out_szDialogText += m_lpszText;
}

//////////////////////////////////////////////////////////////////////
// CFullFeatureNagTask: Particular behavior for the full feature task
//////////////////////////////////////////////////////////////////////

CFullFeatureNagTask::CFullFeatureNagTask(CNagTaskEntryP initEntry)
:	CNagTask(initEntry)
{
}

CFullFeatureNagTask::~CFullFeatureNagTask()
{
}

bool CFullFeatureNagTask::DoNag(NagTimes context)
{
	#ifdef _DEBUG
	// See if the nag is being forced.
	if (GetIniLong(IDS_INI_DEBUG_FORCE_NAG))
	{
		// Don't actually have to check the return value for the force nag
		// INI entry.  If the entry exists and we got here, it is us.
		return CNagTask::DoNag(context);
	}
	#endif

	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		if (GetSharewareMode() == SWM_MODE_LIGHT)
		{
			return CNagTask::DoNag(context);
		}
	}
	return false;
}

void CFullFeatureNagTask::MaybeCancelNag(void *pData)
{
	if (m_active_window && !((CNagDialog*)m_active_window)->GetDoingTask())
	{
		if (*((SharewareModeType *)pData) == SWM_MODE_LIGHT)
		{
			CancelNag();
		}
	}
}

void CFullFeatureNagTask::DoAction(int iAction)
{
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		if (GetSharewareMode() == SWM_MODE_ADWARE) // Already in ADWARE
		{
			ASSERT(0);
		}
		else
		{
			pSWM->SetMode(SWM_MODE_ADWARE, true, m_active_window, true);
		}
	}
}

CWnd* CFullFeatureNagTask::CreateNagWindow()
{
	CFullFeatureNagDialog		*	pFullFeatureNagDialog = NULL;
	pFullFeatureNagDialog = DEBUG_NEW CFullFeatureNagDialog(this, IDD_NAG_FULLFEATURED, AfxGetMainWnd());
	pFullFeatureNagDialog->Create(IDD_NAG_FULLFEATURED, AfxGetMainWnd());

	return pFullFeatureNagDialog;
}
