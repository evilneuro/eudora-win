//
// TaskInfo.h : header file
//

#if !defined(AFX_TASKSTATUSINFO_H__7CA22ED2_A827_11D1_92DB_0060082B6AF8__INCLUDED_)
#define AFX_TASKSTATUSINFO_H__7CA22ED2_A827_11D1_92DB_0060082B6AF8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxmt.h>
#include "resource.h"

#include "TaskObject.h"
#include "TaskErrorTypes.h"

// --------------------------------------------------------------------------

// This stuff could be put inside the class definition, but oh well.


	enum TaskStatusState
	{ 
		TSS_UNKNOWN,
		TSS_CREATED,
		TSS_WAITING_TO_QUEUE,
		TSS_QUEUED,
		TSS_RUNNING,
		TSS_COMPLETE
	};

	enum PostProcState
	{
		TSS_POST_UNKNOWN,
		TSS_POST_NONE,     // Not waiting, not running -- nothing
		TSS_POST_WAITING,  // Waiting for post processing
		TSS_POST_RUNNING,  // Doing post processing
		TSS_POST_COMPLETE
	};

	enum TaskType {
		TASK_TYPE_UNKNOWN,
		TASK_TYPE_SENDING,
		TASK_TYPE_RECEIVING,
		TASK_TYPE_FIND,
		TASK_TYPE_IMAP
	};

	enum TaskScheduling {
		TT_USES_POP		=	1,
		TT_USES_SMTP	=	2,
		TT_USES_DIALUP	=	4
	};

	
	
	// The post-processing callback function. Returns void, takes one argument, a void pointer.
	typedef void (*TaskInfoPostFnPtr)(void *, bool);

// --------------------------------------------------------------------------

class QCWorkerThreadMT;
class QCTaskManager;

// --------------------------------------------------------------------------

class CTaskInfoMT : public CTaskObjectMT
{
public:
	friend class QCTaskManager; // We should not need this, but we do

	CTaskInfoMT();
	~CTaskInfoMT();

	virtual void Kill();
	virtual TaskObjectType GetObjectType();

	CString GetPersona(){ return m_strPersona; }   //read-Only
	TaskType GetTaskType(){ return m_TaskType; }   //read-Only
	CString	GetMainText();
	CString	GetSecondaryText();
	int		GetTotal();
	int		GetSoFar();
	int     GetPercentDone(); // Returns 0...100, -1 if unknown
	TaskStatusState	GetState();
	PostProcState	GetPostState();
	CString	GetTitle();
	void GetPostString(CString &str);

	int HasErrors();

	void CreateError(LPCSTR str, TaskErrorType terr = TERR_UNKNOWN);
	void CreateErrorContext(LPCSTR str, TaskErrorType terr, QCError *pErr);

	void SetTaskType(TaskType type);
	void SetPersona(const CString& persona);
	void SetMainText(const char* NewText);
	void SetSecondaryText(const char* NewText);
	void SetTotal(int NewTotal);
	void SetSoFar(int NewSoFar);
	void SetState(TaskStatusState NewState);
	void SetPostState(PostProcState NewState);
	void SetTitle(const char* NewText);
	void SetGroupID(int gid){ m_nGroupID = gid; }
	int  GetGroupID(){ return m_nGroupID; }

	void IgnoreIdle(bool bIgnoreIdle = true){ InterlockedExchange((LONG*)&m_bIgnoreIdle, bIgnoreIdle); }
	bool IsIgnoreIdleSet(){ return m_bIgnoreIdle; }

	void Progress(const char* NewText);
	void ProgressAdd(int Increment);

	// Assigns the post-processing state and any function, if needed
	void SetPostProcessing(bool bNeedsPost);
	void SetPostProcFn(LPCSTR sDispStr, TaskInfoPostFnPtr pFn, void *pArg = NULL);

	bool NeedsPostProcessing(); // Does this object need post-processing?
	bool HasPostFunction(); // Does this object have a post-processing function?

	void RequestThreadStop();

	bool NeedsDialup(){ return m_strDialupEntry.IsEmpty()?false:true; };
	void SetDialupEntryName(const CString name){ m_strDialupEntry = name; }
	CString GetDialupEntryName(){ return m_strDialupEntry; }
	void SetScheduleTypes(int nTypes){ m_nScheduler = m_nScheduler | nTypes; }
	int GetScheduleTypes(){ return m_nScheduler; }

	CWinThread *m_pWinThread; // UGLY! UGLY! UGLY! This is used directly by the POP code, yuck!
	QCWorkerThreadMT *m_pThread; // UGLY! UGLY! UGLY! This is used directly by the WorkerThread code, yuck!

private:
	void DoPostProcessing(); // Calls the post-processing function

protected:
	CCriticalSection m_ObjGuard; // Guard for these member variables
		CString m_strPersona;  //Some threads are associated with a Persona
		CString	m_MainText;
		CString	m_SecondaryText;
		int		m_Total;
		int		m_SoFar;
		TaskStatusState	m_MainState;
		PostProcState m_PostState;
		TaskType		m_TaskType;
		CString			m_Title;
		int m_nGroupID;
		bool m_bIgnoreIdle;

		CString m_PostDisplayString;
		TaskInfoPostFnPtr m_pPostFn; // The post-processing function
		void *m_pArgument; // The argument (obviously the caller is completely responsible for this data)
		bool m_bNeedsPost; // Does it need post-processing?

		CString m_strDialupEntry;

		//used for scheduling
		int m_nScheduler;

		int m_nErrorCount;

	CCriticalSection m_PercentGuard; // Guard for these member variables
		int m_CurPct, m_LastPct;

	// Local (protected) member functions

	void CalcPercent(); // Update the current pct var
	
	virtual void NotifyAlive();   // Notify the display that this info exists
	virtual void NotifyInfoChanged(); // Notify the display that this info has changed
	virtual void NotifyDead();       // Notify the display that this info is about to die
};


#endif // !defined(AFX_TASKSTATUSINFO_H__7CA22ED2_A827_11D1_92DB_0060082B6AF8__INCLUDED_)
