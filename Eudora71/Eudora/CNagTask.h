// CNagTask.h: interface for the core nagging classes.
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

#if !defined(_CNAGTASK__INCLUDED_)
#define _CNAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"
#include "QCCommandActions.h"
#include "QICommandClient.h"
#include "QCSharewareManager.h"
#include "AuditLog.h"
#include "AuditEvents.h"



#define NAG_SECS_PER_DAY	CNagTask::m_iNagSecondsPerDay


//////////////////////////////////////////////////////////////////////
// CNagTaskEntry
//////////////////////////////////////////////////////////////////////

const int CNM_NAG_NEVER		= 0;
const int CNM_NAG_FOREVER	= -1;

// Nag Task ID's
// TBD: have string and dialog IDs based on these numbers

const UINT	CNM_NAG_REGISTER		= 1;
const UINT	CNM_NAG_UPDATE			= 2;
const UINT	CNM_NAG_AUDIT			= 3;
const UINT	CNM_NAG_DEADBEAT		= 4;
const UINT	CNM_NAG_FULLFEATURED	= 5;
const UINT	CNM_NAG_DEMOEXPIRE		= 6;

const UINT	CNM_NAG_FIRST		= CNM_NAG_REGISTER;
const UINT	CNM_NAG_LAST		= CNM_NAG_DEMOEXPIRE;

const int	CNM_NAG_MAX_SCHEDULE	= 10;

typedef struct
{
	UINT		m_task_id;
	UINT		m_task_flags;			// for dialog placement, other attributes TBD
	UINT		m_base_setting_id;
	UINT		m_last_setting_id;
	UINT		m_loc_setting_id;
	int			m_task_schedule[CNM_NAG_MAX_SCHEDULE];	// Array with the schedule
} CNagTaskEntry, *CNagTaskEntryP;


enum	NagTimes
{				
	NAG_AT_STARTUP,
	NAG_AFTER_MAILCHECK,
	NAG_AT_IDLE
};


//////////////////////////////////////////////////////////////////////
// CNagDialog: Common parent of various nagging dialogs
//////////////////////////////////////////////////////////////////////

class CNagTask;

class CNagDialog : public CDialog
{
public:
	CNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CNagDialog();

	virtual BOOL			OnInitDialog();
	virtual void			OnOK();
	virtual void			OnCancel();
	virtual void			CloseDialog(int nButton = IDOK);
	virtual void			DoDataExchange(CDataExchange* pDX);

	virtual void			SaveWindowRect();

	virtual void			SetNagTask(CNagTask *lpntNagTask);
	virtual CNagTask	*	GetNagTask() const;

	virtual void			SetDoingTask(BOOL bDoingTask);
	virtual BOOL			GetDoingTask() const;

	virtual void			GetDialogText(CString & out_szDialogText);

protected:
	UINT					m_nIDTemplate;
	CNagTask			*	m_pntNagTask;
	BOOL					m_bDoingTask;
};


//////////////////////////////////////////////////////////////////////
// CNagTask: Abstraction for a scheduled nagging event
//////////////////////////////////////////////////////////////////////

class CNagTask : public QICommandClient
{
public:

	CNagTask(CNagTaskEntryP initEntry);
	virtual ~CNagTask();

	virtual	bool			Load();
		// Setup that doesn't belong in the constructor.
		// Returns false if the nag schedules or other data has been tampered with
		// and the caller can decide if it should terminate the app or not.

	virtual bool			Save();

	virtual void			Notify(QCCommandObject*	pObject,
								COMMAND_ACTION_TYPE	theAction,
								void*				pData = NULL );

	virtual bool			MaybeDoNag(NagTimes context);
		// generally don't need to override this

	virtual bool			NagInProgress(NagTimes context);
		// override if you want to do something extra if the given
		// nag is already happening

	virtual bool			DoNag(NagTimes context);
		// override if the default behavior isn't good enough

	virtual void			MaybeCancelNag(void *pData);
		// cancel this nag if it is no longer needed
		// override if you want to check before cancelling nag

	virtual void			CancelNag(void);
		// cancel this nag

	virtual void			DoAction(int iAction = IDOK);
		// do the action associated with the given nag, 
		// by default this is called on "ok" exit from dialog
		// but it may be called by other buttons

	virtual CWnd		*	CreateNagWindow(void);
		// create the appropriate nag window for the given nag

	virtual void			SetActiveWindow(CWnd* pwndNagDialog);
	virtual CWnd		*	GetActiveWindow(void);

	virtual void			SaveWindowRect(CRect rect);
	virtual BOOL			GetSavedWindowRect(CRect &rect);

	static int				m_iNagSecondsPerDay;

protected:
	UINT					m_task_id;
	UINT					m_task_flags;		// for dialog placement, other attributes TBD
	int					*	m_nag_schedule;		// raw schedule
	int						m_nag_last_scheduled;	// index of the last NON-REPEATING schedule item
										// -1 if there are no non-repeating parts
	int						m_nag_repeat;		// if non-zero, the repeat count
	time_t					m_nag_base;			// nag base.  Persistent.
	time_t					m_nag_last;			// last successful nag . Persistent.

	UINT					m_base_setting_id;	// settings IDs to use for persistence
	UINT					m_last_setting_id;
	UINT					m_loc_setting_id;

	CWnd				*	m_active_window;	// the window if one is already up for this nag task
	CRect					m_window_rect;		// placement of the window when it was dismissed

#ifdef _DEBUG
	CString					m_debug_name;		// a name to use in tracing, etc.
#endif
};


#endif // !defined(_CNAGTASK__INCLUDED_)
