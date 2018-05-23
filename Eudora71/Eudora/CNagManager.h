// CNagManager.h: interface for the core nagging classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CNAGMANAGER__INCLUDED_)
#define _CNAGMANAGER__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"
#include "QCCommandActions.h"
#include "QICommandClient.h"
#include "QCSharewareManager.h"
#include "CAdFailureNagTask.h"
#include "CAuditNagTask.h"
#include "CFullFeatureNagTask.h"
#include "CRegisterNagTask.h"
#include "CUpdateNagTask.h"
#include "CDemoExpireNagTask.h"
#include "CNagTask.h"

//////////////////////////////////////////////////////////////////////
// CNagManager
//////////////////////////////////////////////////////////////////////

class CNagManager : public QCCommandDirector, public QICommandClient
{
public:
	
	// Do it like other classes in Eudora
	static	CNagManager*	m_pTheNagManager;
	static	CNagTaskEntry	m_pInitTaskTable[];

	static bool Initialize();
	static bool Destroy();
	static CNagManager* GetNagManager(void);


	CNagManager();
	virtual ~CNagManager();

	virtual	bool Load();
		// Setup that doesn't belong in the constructor.
		// Returns false if the nag schedules or other data has been tampered with
		// and the caller can decide if it should terminate the app or not.

	virtual	bool Save();

	virtual void Notify(QCCommandObject*	pObject,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData = NULL );
		// Some notifications are handled by the manager, unknown types are
		// just propogated to each task

	virtual	void DoCheckNags(NagTimes context);
		// Call at Startup and at the end of mail checks
		// At mail check, should be called before the net connection is torn down
		// so that we can check for the update page via HTTP

	virtual void DoChangeState(COMMAND_ACTION_TYPE	theAction,
								void*				pData);
		// Called from Notify when we are informed of a state change

protected:
	CNagTask*	m_theTaskList[CNM_NAG_LAST + 1];
	time_t		m_LastIdle;
};

#endif // !defined(_CNAGMANAGER__INCLUDED_)
