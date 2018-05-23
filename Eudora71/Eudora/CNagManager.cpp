// CNagManager.cpp: implementation for the core nagging classes.
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
#include "CNagManager.h"

#include "PaymentAndRegistrationDlg.h"

#include "DebugNewHelpers.h"


//////////////////////////////////////////////////////////////////////
// CNagManager
//////////////////////////////////////////////////////////////////////

// Constants

// Frequency in seconds to actually do any nag checking
// 60 for debugging, probably 3600 (1 hour) for normal builds.

#ifdef ELVIS_LIVES//_DEBUG
const	int		k_NagIdleFrequency = 10;
#else
const	int		k_NagIdleFrequency = 3600;
#endif

const	long	kSecondsPerDay		= 60 * 60 * 24;

//////////////////////////////////////////////////////////////////////
// CNagManager Statics
//////////////////////////////////////////////////////////////////////
CNagManager *  CNagManager::m_pTheNagManager = NULL;


CNagTaskEntry	CNagManager::m_pInitTaskTable[CNM_NAG_LAST + 1] = 
{
	// ID				Flags,		m_base_setting_id	m_last_setting_id	m_loc_setting_id		m_task_schedule

	{CNM_NAG_REGISTER,		0,		IDS_INI_NAG_BASE_1,	IDS_INI_NAG_LAST_1,	IDS_INI_NAG_LOC_1,		{7,7,-1,0,0,0,0,0,0,0}},
#ifdef EXPIRING
	{CNM_NAG_UPDATE,		0,		IDS_INI_NAG_BASE_2,	IDS_INI_NAG_LAST_2,	IDS_INI_NAG_LOC_2,		{2,2,-1,0,0,0,0,0,0,0}},
#else
	{CNM_NAG_UPDATE,		0,		IDS_INI_NAG_BASE_2,	IDS_INI_NAG_LAST_2,	IDS_INI_NAG_LOC_2,		{7,7,7,7,30,-1,0,0,0,0}},
#endif
	{CNM_NAG_AUDIT,			0,		IDS_INI_NAG_BASE_3,	IDS_INI_NAG_LAST_3,	IDS_INI_NAG_LOC_3,		{7,7,-1,0,0,0,0,0,0,0}},
	{CNM_NAG_DEADBEAT,		0,		IDS_INI_NAG_BASE_4,	IDS_INI_NAG_LAST_4,	IDS_INI_NAG_LOC_4,		{1,-1,0,0,0,0,0,0,0,0}},
	{CNM_NAG_FULLFEATURED,	0,		IDS_INI_NAG_BASE_5,	IDS_INI_NAG_LAST_5,	IDS_INI_NAG_LOC_5,		{30,30,-1,0,0,0,0,0,0,0}},
	{CNM_NAG_DEMOEXPIRE,	0,		IDS_INI_NAG_BASE_6,	IDS_INI_NAG_LAST_6,	IDS_INI_NAG_LOC_6,		{1,-1,0,0,0,0,0,0,0,0}}
};

//////////////////////////////////////////////////////////////////////
// CNagManager Methods
//////////////////////////////////////////////////////////////////////

// [PUBLIC] Initialize
/* static */ bool CNagManager::Initialize()
{
	TRACE("CNagManager::Initialize()\n");
	ASSERT(!m_pTheNagManager);

	if (m_pTheNagManager)
		delete m_pTheNagManager;

	m_pTheNagManager = DEBUG_NEW_NOTHROW CNagManager;

	if (m_pTheNagManager)
	{
		return m_pTheNagManager->Load();
	}

	return (m_pTheNagManager != NULL);
}

// --------------------------------------------------------------------------

// [PUBLIC] Destroy
/* static */ bool CNagManager::Destroy()
{
	TRACE("CNagManager::Destroy()\n");

	if (m_pTheNagManager)
	{
		m_pTheNagManager->Save();

		delete m_pTheNagManager;
		m_pTheNagManager = NULL;
	}

	return (true);
}

// --------------------------------------------------------------------------

// [PUBLIC] Destroy
/* static */ CNagManager* CNagManager::GetNagManager(void)
{
	return m_pTheNagManager;
}


CNagManager::CNagManager()
: m_LastIdle(0)
{
	for (UINT which = CNM_NAG_FIRST; which <= CNM_NAG_LAST; which++) 
	{
		m_theTaskList[which] = NULL;
	}
}

CNagManager::~CNagManager()
{
	for (UINT which = CNM_NAG_FIRST; which <= CNM_NAG_LAST; which++) 
	{
		if (m_theTaskList[which] != NULL) {
			delete m_theTaskList[which];
		};
	}
}


bool CNagManager::Load()
	// Setup that doesn't belong in the constructor.
	// Returns false if the nag schedules or other data has been tampered with
	// and the caller can decide if it should terminate the app or not.
{

	TRY
	{
		QCSharewareManager *pSWM = GetSharewareManager();

		if (pSWM)
		{
			pSWM->QCCommandDirector::Register((QICommandClient*)this);

			for (UINT which = CNM_NAG_FIRST; which <= CNM_NAG_LAST; which++) 
			{
				// Init the task from the table, possibly making a special task if need be
				CNagTask*	newTask = NULL;

				// if you need a custom task to be created, make the object here. 
				switch (m_pInitTaskTable[which-1].m_task_id) 
				{
					case CNM_NAG_AUDIT:
						newTask = DEBUG_NEW CAuditNagTask(&m_pInitTaskTable[which - 1]);
						break;
					case CNM_NAG_REGISTER:
						newTask = DEBUG_NEW CRegisterNagTask(&m_pInitTaskTable[which - 1]);
						break;
					case CNM_NAG_UPDATE:
						newTask = DEBUG_NEW CUpdateNagTask(&m_pInitTaskTable[which - 1]);
						break;
					case CNM_NAG_DEADBEAT:
						newTask = DEBUG_NEW CAdFailureNagTask(&m_pInitTaskTable[which - 1]);
						break;
					case CNM_NAG_FULLFEATURED:
						newTask = DEBUG_NEW CFullFeatureNagTask(&m_pInitTaskTable[which - 1]);
						break;
					case CNM_NAG_DEMOEXPIRE:
						newTask = DEBUG_NEW CDemoExpireNagTask(&m_pInitTaskTable[which - 1]);
						break;
					default: 
						newTask = DEBUG_NEW CNagTask(&m_pInitTaskTable[which - 1]);
						break;
				};

				m_theTaskList[m_pInitTaskTable[which-1].m_task_id] = newTask;
				newTask->Load();
			}
		}
	}
	CATCH_ALL(e)
	{
		ASSERT(0);
		return false;	// this should cause Eudora to quit, since it means the nagging has been tampered with
	}
	END_CATCH_ALL

	return true;
}

bool CNagManager::Save()
{
	for (UINT which = CNM_NAG_FIRST; which <= CNM_NAG_LAST; which++) 
	{
		if (m_theTaskList[which] != NULL) {
			m_theTaskList[which]->Save();
		};
	}
	return true;
}


void CNagManager::Notify(
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData)
{
	switch (theAction)
	{
		case CA_SWM_CHANGE_FEATURE:
		case CA_SWM_CHANGE_REGCODE:
		case CA_SWM_CHANGE_USERSTATE:
		{
			DoChangeState(theAction, pData);
			break;
		}
		default:
		{
			// Tell all the tasks who care about the event
			NotifyClients(pObject, theAction, pData);
			break;
		}
	}
}

void CNagManager::DoCheckNags(NagTimes context)
		// Call at Startup and at the end of mail checks
		// At mail check, should be called before the net connection is torn down
		// so that we can check for the update page via HTTP
{
	unsigned long	currentTime	= time(NULL);
	unsigned long	deltaTime	= currentTime - m_LastIdle;

#ifdef _DEBUG
	long			lNagToForce = GetIniLong(IDS_INI_DEBUG_FORCE_NAG);
	if (lNagToForce == 0)
	{
		lNagToForce = GetIniLong(IDS_INI_DEBUG_ACCELERATE_NAG);
	}
	if ((lNagToForce >= CNM_NAG_FIRST) && (lNagToForce <= CNM_NAG_LAST))
	{
		if (m_theTaskList[lNagToForce] != NULL)
		{
			m_theTaskList[lNagToForce]->MaybeDoNag(context);
			m_LastIdle = time(NULL); // remember the time we actually FINISHED as the start of the waiting period.
			return;
		}
	}
#endif

	if ((context == NAG_AT_STARTUP) ||
		(((context == NAG_AT_IDLE) || (context == NAG_AFTER_MAILCHECK)) && (deltaTime > k_NagIdleFrequency))) 
	{
		//TRACE("CNagManager::DoCheckNags\n");

		for (UINT which = CNM_NAG_FIRST; which <= CNM_NAG_LAST; which++) 
		{
			if (m_theTaskList[which] != NULL) {
				if (m_theTaskList[which]->MaybeDoNag(context))
				{
					// successful return means the Nag fired successfully
					break; // only Nag one task each idle
				}
			};
		}
		m_LastIdle = time(NULL); // remember the time we actually FINISHED as the start of the waiting period.
	}
	else if (context == NAG_AT_IDLE)
	{
		// Not enough idle time to normally put up a nag, but we need to speed up
		// the process if a background update check has completed
		if (CUpdateNagTask::m_BackgroundStatus >= CUpdateNagTask::BG_ERROR_CONNECTING)
		{
			if (m_theTaskList[CNM_NAG_UPDATE] != NULL)
				m_theTaskList[CNM_NAG_UPDATE]->MaybeDoNag(context);
		}

	}
}

void CNagManager::DoChangeState(COMMAND_ACTION_TYPE	theAction,
								void*				pData)
		// Called from Notify when we are informed of a mode change
{
	TRACE("CNagManager::DoChangeState\n");

	// Phase 1: check for new nags to run

	// Phase 2: Dismiss any nags that are no longer relevant give the state change
	switch (theAction)
	{
		case CA_SWM_CHANGE_REGCODE:
		{
			// On registration, the register nag is now out of date.
			m_theTaskList[CNM_NAG_REGISTER]->MaybeCancelNag(pData);
			break;
		}
		case CA_SWM_CHANGE_FEATURE:
		{
			// On feature change the register, audit, full-featured
			// and demo expire nags may no longer be valid.
			m_theTaskList[CNM_NAG_REGISTER]->MaybeCancelNag(pData);
			m_theTaskList[CNM_NAG_AUDIT]->MaybeCancelNag(pData);
			m_theTaskList[CNM_NAG_FULLFEATURED]->MaybeCancelNag(pData);
			m_theTaskList[CNM_NAG_DEMOEXPIRE]->MaybeCancelNag(pData);
			break;
		}
		case CA_SWM_CHANGE_USERSTATE:
		{
			m_theTaskList[CNM_NAG_DEADBEAT]->MaybeCancelNag(pData);
			break;
		}
	}
}


