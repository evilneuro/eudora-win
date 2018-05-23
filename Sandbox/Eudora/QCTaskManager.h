#ifndef _QCTaskManager_h_
#define _QCTaskManager_h_

#include "TaskInfo.h"
#include "TaskErrorMT.h"
#include "Callbacks.h"

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include <list>
#include <algorithm>
#pragma warning (default: 4663 4244)

#include <map>


//support C functions 
UINT ThreadStartupFunc(LPVOID);

//typedef void (*GroupCallback)();
typedef Callback0 GroupCallback;

class QCWorkerThreadMT;

class QCTaskManager 
{
public:
	static QCTaskManager *Create();
	static void Destroy();
	
	void Register(CTaskInfoMT *pTaskInfo);
	void Register(CTaskErrorMT *pTaskError);

	bool CanScheduleTask(CTaskInfoMT *pTaskInfo);
	bool ScheduleTasks();

	bool RemoveTaskInfo(unsigned long int nUID);
	bool RemoveTaskError(unsigned long int nUID);
	CTaskErrorMT *GetTaskError(unsigned long int nUID);
	int GetNumTaskErrors(CTime afterThisTime, TaskType tt);

	//Notify view to remove errors that match persona and Task types
	bool ClearTaskErrors(CString strPersona, TaskType tt);


	CTaskInfoMT *GetTaskInfo(unsigned long int nUID);
	CTaskInfoMT *GetTaskInfo(QCWorkerThreadMT *pThread);
	void RemoveWorkerThread(QCWorkerThreadMT *pThread);

	void RequestThreadStop(HANDLE);
	void RequestAllThreadsToStop();

	//Returns false if can't queue due to scheduling conflicts
	bool QueueWorkerThread(QCWorkerThreadMT *pThread);
	bool StartWorkerThread(CTaskInfoMT *pTaskInfo);

	bool DelayTasks(GroupCallback gc=NULL, bool bIgnoreIdle=false);
	void StartTasks();
	
	bool IsTaskRunning(CString strPersona, int ScheduleBits);
	bool LookupTask(int ScheduleBits);
	
	CWnd *SetDisplayWindow(CTaskObjectMT::TaskObjectType type, CWnd *pWnd);

	int GetTaskCount();
	int GetTaskCount(CString strPersona);

	bool IsPostProcessingRequested();
	bool NeedsPostProcessing();
	void RequestPostProcessing(CTaskInfoMT *pTaskInfo = NULL);
	
	//Only gets called from OnIdle
	void DoPostProcessing();
	
	
	//Method that controls when to send ID_POST_PROCESSING message to EudoraApp
	//bool SendPostProcessMsg();


private:
	
	QCTaskManager();
	CCriticalSection m_ObjGuard;
//	CWnd *m_pDisplayWnd; // This is the window which is sent msgs by the tasks

	CCriticalSection m_Guard_TaskErrorList; // This guards ONLY the list 'm_TaskErrorList'
	list<CTaskErrorMT *> m_TaskErrorList;  //STL list to keep track of CTaskInfoMT's
	typedef list<CTaskErrorMT *>::iterator TaskErrorIterator;

	CCriticalSection m_Guard_TaskInfoList; // This guards ONLY the list 'm_TaskInfoList'
	list<CTaskInfoMT *> m_TaskInfoList;  //STL list to keep track of CTaskInfoMT's
	typedef list<CTaskInfoMT *>::iterator TaskIterator;
	typedef list<CTaskInfoMT *>::reverse_iterator RTI;

	
	CCriticalSection m_Group_Guard; // This guards ONLY the list 'm_TaskInfoList'
	map<int, GroupCallback> m_GroupMap;  //STL list to keep track of CTaskInfoMT's
	typedef map<int, GroupCallback>::iterator GroupIter;
	
	map<CTaskObjectMT::TaskObjectType, CWnd *> m_DispWndMap;

	list<CTaskInfoMT *> m_PostProcessList;
	CCriticalSection m_PostProcessListGuard;
	bool m_bPostProcessInProgress;
	
	bool m_bDelayQueuing;
	bool m_bIgnoreIdle;
	static int s_nGroupID;

	unsigned long m_LastIdleTime;

	//rules
	int m_nMaxConcurrentTasks;
	int m_nActiveTasks;
	int m_nActiveDialupTasks;
	int m_nMaxConcurrentDialupTasks;
};

//Helper functions
QCTaskManager *QCGetTaskManager();
	

class QCTaskGroup {
	bool m_bTopLevel;
public:	
	QCTaskGroup(GroupCallback gc = NULL, bool bIgnoreIdle=false);
	IgnoreIdleTime();
	~QCTaskGroup();

};

#endif

