// CNagTask.cpp: implementation for the core nagging classes.
//
// Copyright (c) 1999-2001 by QUALCOMM, Incorporated
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
#include "CNagTask.h"
#include "rs.h"
#include "PgCompMsgView.h"
#include "debug.h"


#include "DebugNewHelpers.h"

const	int	kMinNagSecondsPerDay = 5;				// minimum seconds in a day
const	int	kMaxNagSecondsPerDay = 86400;			// maximum seconds in a day

int CNagTask::m_iNagSecondsPerDay = -1;


#define	NAGDAYS(a) (0)
#define NAGSECONDS(a) (a * CNagTask::m_iNagSecondsPerDay)


// Assure rect is completely visible in the useable area of the desktop.
void AssureVisible(CRect &rect)
{
	CRect rectDesktop;
	// Get the actual work area of the desktop (minus the taskbar).
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rectDesktop, NULL);

	if (rect.left < 0)			rect.left = 0;
	if (rect.top < 0)			rect.top = 0;
 	if (rect.right > rectDesktop.Width())
	{
		rect.left = rectDesktop.Width() - rect.Width();
		rect.right = rect.left + rect.Width();
	}
	if (rect.bottom > rectDesktop.Height())
	{
		rect.top = rectDesktop.Height() - rect.Height();
		rect.bottom = rect.top + rect.Height();
	}
}


//////////////////////////////////////////////////////////////////////
// CNagDialog: Common parent of various nagging dialogs
//////////////////////////////////////////////////////////////////////

CNagDialog::CNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	m_nIDTemplate(nIDTemplate),
	m_pntNagTask(pntNagTask),
	m_bDoingTask(FALSE)
{
}

CNagDialog::~CNagDialog()
{
}

BOOL CNagDialog::OnInitDialog()
{
	// Always call CDialog::OnInitDialog() first.
	CDialog::OnInitDialog();

	if (m_pntNagTask)
	{
		CRect		rect;
		CRect		rectSaved;
		CRect		rectOld;
		GetWindowRect(&rectOld);
		if (m_pntNagTask->GetSavedWindowRect(rectSaved) && !rectSaved.IsRectEmpty())
		{
			// If a non-empty rect was recorded, use that.
			rect = rectSaved;
			// If a window extends off the left or top GetWindowRect() ends up returning
			// a number that is not quite 65536 - left (or top).  If we find a large number
			// just set it to 0 because we're about to shift the dialog onto the screen
			// anyways.
			if (rect.left > 65000)	rect.left = 0;
			if (rect.top > 65000)	rect.top = 0;
			rect.right = rect.left + rectOld.Width();
			rect.bottom = rect.top + rectOld.Height();
			// Make sure the entire dialog is on the screen.
			::AssureVisible(rect);
			// Move the dialog but don't size it.
			SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		else
		{
			// If there is no rect, center the dialog.
			CenterWindow(AfxGetMainWnd());
		}
	}

	// Format the nag dialog log entry
	CString		szNagDialogLogEntry;
	CString		szDialogText;
	GetDialogText(szDialogText);
	szNagDialogLogEntry.Format(IDS_NAG_DIALOG_LOG_ENTRY, m_nIDTemplate, szDialogText);

	// Output the nag dialog log entry
	PutDebugLog( DEBUG_MASK_DIALOG, szNagDialogLogEntry, szNagDialogLogEntry.GetLength() );

	return TRUE;
}

void CNagDialog::OnOK()
{
	// Hitting OK or Cancel is the same thing for this base class.  It just needs to clean up.
	// It's the derived classes that want to do different behavior in OK vs. Cancel.
	CloseDialog(IDOK);
}

void CNagDialog::OnCancel()
{
	CloseDialog(IDCANCEL);
}

void CNagDialog::CloseDialog(int nButton)
{
	m_bDoingTask = TRUE;

	SaveWindowRect();
	DestroyWindow();

	if (m_pntNagTask)	m_pntNagTask->SetActiveWindow(NULL);

	// Output the result of what the user chose in response to the nag dialog
	CString		szNagDialogResultEntry;
	szNagDialogResultEntry.Format(IDS_DEBUG_DISMISSED_ALERT, nButton);
	PutDebugLog(DEBUG_MASK_DIALOG, szNagDialogResultEntry);

	m_bDoingTask = FALSE;
}

void CNagDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CNagDialog::SaveWindowRect()
{
	if (m_pntNagTask)
	{
		CRect		rect;
		GetWindowRect(&rect);
		m_pntNagTask->SaveWindowRect(rect);
	}
}

void CNagDialog::SetNagTask(CNagTask *pntNagTask)
{
	m_pntNagTask = pntNagTask;
}

CNagTask * CNagDialog::GetNagTask() const
{
	return m_pntNagTask;
}

void CNagDialog::SetDoingTask(BOOL bDoingTask)
{
	m_bDoingTask = bDoingTask;
}

BOOL CNagDialog::GetDoingTask() const
{
	return m_bDoingTask;
}

void CNagDialog::GetDialogText(CString & out_szDialogText)
{
	// Override to return actual message presented to user
}


//////////////////////////////////////////////////////////////////////
// CNagTask: Abstraction for a scheduled nagging event
//////////////////////////////////////////////////////////////////////

CNagTask::CNagTask(CNagTaskEntryP initEntry)
:	m_task_id(initEntry->m_task_id),
	m_task_flags(initEntry->m_task_flags),
	m_nag_schedule(initEntry->m_task_schedule),
	m_nag_last_scheduled(-1),
	m_nag_repeat(0),
	m_nag_base(0),
	m_nag_last(0),
	m_base_setting_id(initEntry->m_base_setting_id),
	m_last_setting_id(initEntry->m_last_setting_id),
	m_loc_setting_id(initEntry->m_loc_setting_id),
	m_active_window(NULL),
	m_window_rect(0,0,0,0)
#ifdef _DEBUG
	,m_debug_name(_T("NagTask"))
#endif
{
	// First time through, read seconds per day setting from INI file.
	if (m_iNagSecondsPerDay == -1)
	{
		m_iNagSecondsPerDay = GetIniInt(IDS_INI_NAG_SECS_PER_DAY);
		if (m_iNagSecondsPerDay == 0)
		{
			// If there is no entry, default to kMaxNagSecondsPerDay second days.
			m_iNagSecondsPerDay = kMaxNagSecondsPerDay;
		}
		if (m_iNagSecondsPerDay < kMinNagSecondsPerDay)		m_iNagSecondsPerDay = kMinNagSecondsPerDay;
		if (m_iNagSecondsPerDay > kMaxNagSecondsPerDay)		m_iNagSecondsPerDay = kMaxNagSecondsPerDay;
	}
}

CNagTask::~CNagTask()
{
	CancelNag();
}


bool CNagTask::Load()
	// Setup that doesn't belong in the constructor.
	// Returns false if the nag schedules or other data has been tampered with
	// and the caller can decide if it should terminate the app or not.
{

#ifdef _DEBUG
	switch (m_task_id) 
	{
		case CNM_NAG_REGISTER: m_debug_name = _T("Nag-Register"); break;
		case CNM_NAG_UPDATE: m_debug_name = _T("Nag-Update"); break;
		case CNM_NAG_AUDIT: m_debug_name = _T("Nag-Audit"); break;
		case CNM_NAG_DEADBEAT: m_debug_name = _T("Nag-Deadbeat"); break;
		case CNM_NAG_FULLFEATURED: m_debug_name = _T("Nag-Fullfeatured"); break;
		case CNM_NAG_DEMOEXPIRE: m_debug_name = _T("Nag-DemoExpire"); break;
		default: {
					m_debug_name.Format(_T("Unknown Nag Task %d"), m_task_id);
				 }
			break;
	}
#endif

	for (int i = 0; i < CNM_NAG_MAX_SCHEDULE; i++) {
		if (m_nag_schedule[i] == -1)
		{
			if (i >= 2) {
				m_nag_last_scheduled = i - 2;
			}
			if (i >= 1) 
			{
				m_nag_repeat = m_nag_schedule[i - 1];
			}
		}
	}

	// Restore: m_nag_base and m_nag_last
	m_nag_base = GetIniLong(m_base_setting_id);
	m_nag_last = GetIniLong(m_last_setting_id);

	return true;
}

bool CNagTask::Save()
{
	bool	bRetVal = SetIniLong(m_base_setting_id, m_nag_base) &&
					SetIniLong(m_last_setting_id, m_nag_last);
	if (!m_window_rect.IsRectEmpty())
	{
		bRetVal = bRetVal && SetIniWindowPos(m_loc_setting_id, m_window_rect);
	}
	return bRetVal;
}

void CNagTask::Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData)
{

}

bool CNagTask::MaybeDoNag(NagTimes context)
{
	bool		shouldNag					= false;
	bool		after_scheduled_interval	= false; // true when the non-periodic schedule is exhausted, or doesn't exist
	CTime		now			= CTime::GetCurrentTime();
	CTime		baseNag;
	CTime		lastNag;
	CTimeSpan	last_base_interval;

#ifdef DEATH_BUILD
	// The Death Build(TM) won't nag users about anything
	return false;
#endif

	// If the main window is minimized, return out without nagging.
	CWnd	*pwndMain = AfxGetMainWnd();
	if (pwndMain)
	{
		if (pwndMain->IsIconic())
		{
			return false;
		}
	}

	time_t		nNowSeconds = static_cast<time_t>( now.GetTime() );

	if (m_nag_base == 0) 
	{
		// first time 
		m_nag_last = m_nag_base = nNowSeconds;
	}

	// Sanity check the numbers to see if someone's been playing with the "last nag" time
	if ((m_nag_last > nNowSeconds) || (m_nag_base > nNowSeconds))
	{
		// naughty, you get reset as if you hadn't been nagged.
		m_nag_last = m_nag_base = static_cast<time_t>( now.GetTime() );
	}

	baseNag		= m_nag_base;
	lastNag		= (m_nag_last == 0? now : m_nag_last);

	last_base_interval = lastNag - baseNag;

	if (m_nag_last_scheduled == -1)
	{
		// None scheduled, so use periodic
		after_scheduled_interval = true;
	}
	else
	{
		// See if we are past the schedule
		CTimeSpan	last_sched_interval(NAGDAYS(m_nag_schedule[m_nag_last_scheduled]),0,0,NAGSECONDS(m_nag_schedule[m_nag_last_scheduled]));

		if (last_sched_interval < last_base_interval)
		{
			// off the end, so use periodic
			after_scheduled_interval = true;
		} 
		else
		{
			CTimeSpan	now_base_interval(now - baseNag);

			for (int i = 0; i <= m_nag_last_scheduled; i++)
			{
				CTimeSpan	sched_interval(NAGDAYS(m_nag_schedule[i]),0,0,NAGSECONDS(m_nag_schedule[i]));

				if ((sched_interval >= last_base_interval) &&
					(sched_interval <= now_base_interval))
				{
					shouldNag = true;
					break;
				}
			}
		}
	}

	if (after_scheduled_interval)
	{
		// only periodic part
		if (m_nag_repeat > 0)
		{
			CTimeSpan	repeat_interval(NAGDAYS(m_nag_repeat),0,0,NAGSECONDS(m_nag_repeat));
			CTimeSpan	last_to_now_interval(now - lastNag);
			if (last_to_now_interval > repeat_interval)
			{
				shouldNag = true;
			}
		}
	}

#ifdef _DEBUG
	long			lNagToForce = GetIniLong(IDS_INI_DEBUG_FORCE_NAG);
	if (lNagToForce == 0)
	{
		lNagToForce = GetIniLong(IDS_INI_DEBUG_ACCELERATE_NAG);
	}
	if (m_task_id == (unsigned long)lNagToForce)
	{
		shouldNag = true;
	}
#endif

	if (shouldNag) {

		TRACE2("CNagTask::MaybeDoNag: will nag %s (%d)\n", m_debug_name, m_task_id);
		bool	bNagHappened = false;
		if (m_active_window)
		{
			bNagHappened = NagInProgress(context);
		}
		else
		{
			bNagHappened = DoNag(context);
		}
		if (bNagHappened)
		{
			// bump the clock to avoid fenceposts
			now += 1;
			m_nag_last = static_cast<time_t>( now.GetTime() );
			Save();
			return true;
		}
	}
	return false; // nothing happened, or failure
}

bool CNagTask::NagInProgress(NagTimes context)
{
	// Sanity check only -- we shouldn't reach here if m_active_window is NULL.
	if (m_active_window)
	{
		// If we already have a window for this nag bring it to the front.
		m_active_window->SetFocus();
		return true;
	}
	return false;
}

bool CNagTask::DoNag(NagTimes context)
{
	if ((m_task_id >= CNM_NAG_FIRST) && (m_task_id <= CNM_NAG_LAST))
	{
		m_active_window = CreateNagWindow();
		if (m_active_window)
		{
			return true;
		}
	}

	// failure , clean up.  Maybe should be an try/catch here.

	if (m_active_window != NULL) 
	{
		// ?? destroy ?? Delete ?? Something safe.
		m_active_window->DestroyWindow();

		m_active_window = NULL;
	}
	return false; // failure
}

void CNagTask::MaybeCancelNag(void *pData)
{
}

void CNagTask::CancelNag(void)
{
	if (m_active_window)
	{
		if (IsWindow(m_active_window->GetSafeHwnd()))
		{
			CNagDialog* pnd = dynamic_cast<CNagDialog*>(m_active_window);
			if (pnd)
				pnd->SaveWindowRect();
			m_active_window->DestroyWindow();
		}
		delete m_active_window;
		m_active_window = NULL;
	}
}

void CNagTask::DoAction(int iAction)
{
	// Failure!  Do we reset the times?
}

// Changed this from CNagDialog* to CWnd* because the update nag won't be a dialog.
CWnd* CNagTask::CreateNagWindow(void)
{
	return NULL;
}

void CNagTask::SetActiveWindow(CWnd* pwndNagDialog)
{
	m_active_window = pwndNagDialog;
}

CWnd* CNagTask::GetActiveWindow(void)
{
	return m_active_window;
}

void CNagTask::SaveWindowRect(CRect rect)
{
	m_window_rect = rect;
}

BOOL CNagTask::GetSavedWindowRect(CRect &rect)
{
	if (!m_window_rect.IsRectEmpty())
	{
		rect = m_window_rect;
		return TRUE;
	}
	else
	{
		return GetIniWindowPos(m_loc_setting_id, rect);
	}
}
