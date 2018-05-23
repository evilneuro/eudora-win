// QCTaskManager.cpp: impementation file for QCTaskManager
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include "QCWorkerThreadMT.h"
#include "TaskStatusView.h"
#include "QCTaskManager.h"

#include "eudora.h"  //for CEudoraApp

#include "TaskInfo.h"
#include "TaskErrorMT.h"
#include "QCRasConnection.h"

#include "QCRas.h"  //For CloseConnection
#include "resource.h"
#include "rs.h" //GetIniLong
#include "persona.h"

#include "QCWorkerSocket.h"		// for Network
#include "QCSSL.h"				// for CertData struct
	
#ifdef EXPIRING
	#include "timestmp.h"
	#include "mainfrm.h"
	extern CTimeStamp	g_TimeStamp;
#endif


#include "DebugNewHelpers.h"


CTrustCertDlg::CTrustCertDlg(CWnd *pwndParent, CString &strCertText, CString &strReasonText)
	: CHelpxDlg(CTrustCertDlg::IDD, pwndParent)
{
	m_strCertText = strCertText;
	m_strReasonText = strReasonText;
}

void CTrustCertDlg::DoDataExchange(CDataExchange *pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMakeFilter)
	DDX_Text(pDX, IDC_EDIT_CERT, m_strCertText);
	DDX_Text(pDX, IDC_REASON_TEXT, m_strReasonText);
	//}}AFX_DATA_MAP
}


// --------------------------------------------------------------------------

QCTaskManager *g_TaskManager = NULL;

// --------------------------------------------------------------------------

int QCTaskManager::s_nGroupID = 0; //Group IDs start at 0


//Global helper function to get the one and only QCTaskManager object
QCTaskManager *QCGetTaskManager()
{ 
	ASSERT(g_TaskManager!=NULL);
	return g_TaskManager;
}

QCTaskManager::QCTaskManager() :  m_nActiveTasks(0),
			m_nActiveDialupTasks(0), m_bDelayQueuing(false),
			m_bIgnoreIdle(false), m_nStartIdle(0), m_bPostProcessInProgress(false)
{
	// Retrieve the maximum number of concurrent tasks, where 0 will mean no limit
	m_nMaxConcurrentTasks = GetIniLong(IDS_INI_MAX_CONCURRENT_TASKS);
	m_nMaxConcurrentDialupTasks = GetIniLong(IDS_INI_MAX_CONCURRENT_DIALUP_TASKS);
}


bool QCTaskManager::DelayTasks(GroupCallback gc /*NULL*/, bool bIgnoreIdle /*=false*/)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);
	if(!m_bDelayQueuing)
	{
		m_bDelayQueuing = true;
		m_bIgnoreIdle = bIgnoreIdle;
		
		//new Group ID
		if(gc != NULL)
		{
			s_nGroupID++;
			m_GroupMap[s_nGroupID] = gc;
		}

		return true;
	}
	return false;  //false means already delayed
}

void QCTaskManager::StartTasks()
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);
	ASSERT(m_bDelayQueuing == true);

	m_bDelayQueuing  = false; //reset
	m_bIgnoreIdle = false;	  //reset

	CTaskInfoMT *task = NULL;
	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		task = (*ti);
		if(task->GetState() == TSS_WAITING_TO_QUEUE) 
			task->SetState(TSS_QUEUED);
	}


	m_nStartIdle = ((CEudoraApp *)AfxGetApp())->GetStartIdle();

	ScheduleTasks();
}
	

QCTaskManager *QCTaskManager::Create()
{
	ASSERT(g_TaskManager==NULL);
	g_TaskManager = DEBUG_NEW_NOTHROW QCTaskManager;
	return g_TaskManager;
}

void QCTaskManager::Destroy()
{
	if(g_TaskManager)
	{
		delete g_TaskManager;
		g_TaskManager = NULL;
	}
}

void QCTaskManager::Register(CTaskInfoMT *pTaskInfo)
{
	ASSERT(m_DispWndMap[pTaskInfo->GetObjectType()]);

	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);
	
	bool bInserted = false;
	if(pTaskInfo->NeedsDialup())
	{
		//If task needs dialup, place it after the last dailup connection with the same dialup entry
		//if any exists+
		CString strEntry = pTaskInfo->GetDialupEntryName();
		for(RTI ri = m_TaskInfoList.rbegin(); ri != m_TaskInfoList.rend(); ++ri)
		{
			if( (*ri)->NeedsDialup())
			{
				if( (*ri)->GetDialupEntryName() == strEntry)
				{
					m_TaskInfoList.insert(ri.base(), 1, pTaskInfo);
					bInserted = true;
					break;
				}
			}
		}
	}
	
	if(!bInserted)
		m_TaskInfoList.push_back(pTaskInfo);
	
	CSingleLock objlock(&m_ObjGuard, TRUE); // Lock for access to object member variable
	pTaskInfo->SetDisplayWnd(m_DispWndMap[pTaskInfo->GetObjectType()], true); // Set display window for info and have it nitify the window immediately

	//Tell that the task is waiting to be run
	//pTaskInfo->SetMainText("Waiting in the task queue to be started ...");
	pTaskInfo->SetMainText(CRString(IDS_TASK_WAITING_TO_START));
}

void QCTaskManager::Register(CTaskErrorMT *pTaskError)
{
	ASSERT(m_DispWndMap[pTaskError->GetObjectType()]);

	CSingleLock lock(&m_Guard_TaskErrorList, TRUE);
	m_TaskErrorList.push_back(pTaskError);

	CSingleLock objlock(&m_ObjGuard, TRUE); // Lock for access to object member variable
	pTaskError->SetDisplayWnd(m_DispWndMap[pTaskError->GetObjectType()], true); // Set display window for info and have it nitify the window immediately

	PutDebugLog(DEBUG_MASK_DIALOG, pTaskError->GetErrText());
}


void QCTaskManager::RequestAllThreadsToStop()
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		// Request that the task info stop the "thread", because not all
		// task info's have an actual m_pThread.
		(*ti)->RequestThreadStop();
	}
}


CTaskInfoMT *QCTaskManager::GetTaskInfo(QCWorkerThreadMT *pThread)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
		if( (*ti)->m_pThread == pThread)
			return (*ti);
	return NULL;
}


bool QCTaskManager::QueueWorkerThread(QCWorkerThreadMT *pThread)
{
	CTaskInfoMT *pTaskInfo = pThread->GetTaskInfo();
	
	if( m_bDelayQueuing)
	{
		pTaskInfo->SetGroupID(s_nGroupID);
		pTaskInfo->SetState( TSS_WAITING_TO_QUEUE );
		if(m_bIgnoreIdle)
			pTaskInfo->IgnoreIdle();

	}
	else
		pTaskInfo->SetState( TSS_QUEUED );
	
	//StartWorkerThread(pTaskInfo);
	if(CanScheduleTask(pTaskInfo) == false)
	{
		return false;
	}
	

	//Clear previous Network errors for this task type
	ClearTaskErrors(pTaskInfo->GetPersona(), pTaskInfo->GetTaskType());

	// Add to the internal Task List
	Register(pTaskInfo);
	
	ScheduleTasks();  //which calls StartWorkerThread
	return true;
	
}


bool QCTaskManager::CanScheduleTask(CTaskInfoMT *pTaskInfo)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

#ifdef EXPIRING
	// this is the first line of defense
	if ( g_TimeStamp.IsExpired0() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
		return false;
	}
#endif

	int nScheduleTypes = pTaskInfo->GetScheduleTypes();

	//pTaskInfo isn't yet inserted into the TaskList
	if(  (pTaskInfo->GetTaskType() == TASK_TYPE_RECEIVING) && (nScheduleTypes & TT_USES_POP) )
	{
		CTaskInfoMT *task = NULL;
		for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
		{
			task = (*ti);
			if( (task->GetTaskType() == TASK_TYPE_RECEIVING) 
				&& (task->m_strPersona == pTaskInfo->m_strPersona) )
				return false;
		}
	}
	
	return true;
}

int QCTaskManager::GetTaskCount(const char* strPersona)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	int num = 0;
	CTaskInfoMT *pTaskInfo = NULL;
	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		pTaskInfo = *ti;
		if ( pTaskInfo->m_bCountTask &&
			 (!strPersona || pTaskInfo->GetPersona().CompareNoCase(strPersona) == 0) )
		{
			num++;
		}
	}

	return num;
}


bool QCTaskManager::IsTaskRunning(const char* strPersona, int ScheduleBits)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	CTaskInfoMT *pTaskInfo = NULL;
	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		pTaskInfo = *ti;
		if(pTaskInfo->GetPersona().CompareNoCase(strPersona) != 0)
			continue;

		TaskStatusState ts = pTaskInfo->GetState();
		if( ts == TSS_RUNNING || ts == TSS_COMPLETE) 
		{	
			int nScheduleTypes = pTaskInfo->GetScheduleTypes();
			if( nScheduleTypes & ScheduleBits)
				return true;
		}
	}
	return false;
}


bool QCTaskManager::LookupTask(int ScheduleBits)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		int nScheduleTypes = (*ti)->GetScheduleTypes();
		if( nScheduleTypes & ScheduleBits)
			return true;
	}
	return false;
}





bool QCTaskManager::ScheduleTasks()
{

	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	CTaskInfoMT *pTaskInfo = NULL;
	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		pTaskInfo = *ti;
		int nScheduler = pTaskInfo->GetScheduleTypes();

		//Need a task thats queued
		if(pTaskInfo->GetState() != TSS_QUEUED)
			continue;

		//Rule 1 - Only allow specified number of Max concurrent tasks
		ASSERT( (m_nActiveTasks <= m_nMaxConcurrentTasks) || (m_nMaxConcurrentTasks == 0) );
		
		// Did we reach the max limit (where 0 means no limit)?
		if ( (m_nMaxConcurrentTasks != 0) && (m_nActiveTasks == m_nMaxConcurrentTasks) )
			break;		

		//Rule Don't allow more than one POP connection per persona concurrently
		if( nScheduler & TT_USES_POP )
		{
			if( IsTaskRunning(pTaskInfo->GetPersona(), TT_USES_POP) )
				continue;
		}
		
		//Rule 2 Allow only one dilaup task at any time
		if (nScheduler & TT_USES_DIALUP)
		{
			//if reached max allowed connections on a dialup, stop here
			if ( (m_nMaxConcurrentDialupTasks != 0) && (m_nActiveDialupTasks == m_nMaxConcurrentDialupTasks) )
				continue;
		}
		

		//TRACE("Starting the task by tid %d\n", GetCurrentThreadId());
		StartWorkerThread(pTaskInfo);
	}

	return true;	
}


bool QCTaskManager::StartWorkerThread(CTaskInfoMT *pTaskInfo)
{
	if (pTaskInfo)
	{
		QCWorkerThreadMT *pThread = pTaskInfo->m_pThread;

		if (pThread)
		{
			TRACE("Starting task: %d: %s for %s\n", pTaskInfo->GetUID(), pTaskInfo->GetTitle(), pTaskInfo->GetPersona());
			CWinThread *pWinThread = AfxBeginThread(ThreadStartupFunc, (LPVOID)pThread, 
									THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
			if (pWinThread)
			{
				//pWinThread->m_bAutoDelete = FALSE;
				pTaskInfo->m_pWinThread = pWinThread;

				pWinThread->ResumeThread();

				pTaskInfo->SetState(TSS_RUNNING);
				m_nActiveTasks++;
				if (pTaskInfo->NeedsDialup())
					m_nActiveDialupTasks++;

				//WaitForSingleObject(pThreadID->m_hThread, INFINITE);
				return true;
			}
		}

		// Make sure we clean up this task when something bad happened
		pTaskInfo->SetState(TSS_COMPLETE);
		pTaskInfo->SetPostProcessing(true);
		RequestPostProcessing(pTaskInfo);
	}

	ASSERT(0);
	
	return false;
}

void QCTaskManager::RemoveWorkerThread(QCWorkerThreadMT *pThread)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);
	static DWORD EudoraAppThreadID = AfxGetApp()->m_nThreadID;

	CTaskInfoMT *pTaskInfo = GetTaskInfo(pThread);
	if(!pTaskInfo)
	{
		ASSERT(pTaskInfo);
		return;
	}

	//Set the thread state to done 
	pTaskInfo->SetState(TSS_COMPLETE);

	unsigned long IdleTime = ((CEudoraApp *)AfxGetApp())->GetStartIdle();
	
	if( pTaskInfo->IsIgnoreIdleSet() || (m_nStartIdle == IdleTime) )
	{
		RequestPostProcessing(pTaskInfo);
	}

}


bool QCTaskManager::NeedsPostProcessing()
{
	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		if((*ti)->NeedsPostProcessing())
			return true;
	}

	return false;
}


//Should only get called from OnIdle loop
//Pop a command from the PostProcess Request queue and do post processing on the requested tasks
void QCTaskManager::DoPostProcessing()
{
	ASSERT(::IsMainThreadMT());

	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	if(m_bPostProcessInProgress == true || m_PostProcessList.empty())
	{
		TRACE("Refused to do PostProcessing due to Reentrancy\n");
		ASSERT(0);
		return;
	}

	CTaskInfoMT *pTaskInfo = NULL;
	{
		CSingleLock lock(&m_PostProcessListGuard, TRUE);
		pTaskInfo = *(m_PostProcessList.begin());
		m_PostProcessList.pop_front();
	}//m_PostProcessListGuard lock removed


	try  {

		m_bPostProcessInProgress = true;

		if(pTaskInfo)
		{
			//Task might have been processed already. Check for validity of the task info
			TaskIterator ti= find(m_TaskInfoList.begin(), m_TaskInfoList.end(), pTaskInfo);
			
			if(ti != m_TaskInfoList.end() && pTaskInfo->NeedsPostProcessing())
			{
				//TRACE("Doing postprocessing for %d\n", pTaskInfo->GetUID());
				pTaskInfo->DoPostProcessing();
			}
		}
		else
		{

			for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
			{
				pTaskInfo = (*ti);
				if(pTaskInfo->NeedsPostProcessing())
				{
					//TRACE("Doing postprocessing for %d\n", pTaskInfo->GetUID());
					pTaskInfo->DoPostProcessing();
				}
			}
		}
		
		m_bPostProcessInProgress = false;
		
	}
	catch (CException * /* pException */)
	{
		// Catch and rethrow MFC exceptions
		ASSERT( !"Rethrowing CException in QCTaskManager::DoPostProcessing" );
		m_bPostProcessInProgress = false;
		throw;
	}
	catch (std::exception & /* exception */)
	{
		// Catch and rethrow standard C++ derived exception
		ASSERT( !"Rethrowing std::exception in QCTaskManager::DoPostProcessing" );
		m_bPostProcessInProgress = false;
		throw;
	}
}


bool QCTaskManager::IsPostProcessingRequested()
{
	CSingleLock lock(&m_PostProcessListGuard, TRUE);
	return !m_PostProcessList.empty();
}

void QCTaskManager::RequestPostProcessing(CTaskInfoMT *pTaskInfo /*= NULL*/)
{
	{
		CSingleLock lock(&m_PostProcessListGuard, TRUE);
		//TRACE("Add PostProcess cmd\n");
		m_PostProcessList.push_back( pTaskInfo);
	} //Lock destroyed here
}



	

// --------------------------------------------------------------------------

// [PUBLIC] GetTaskInfo
//
// Find and return a pointer to the TaskInfo identified by the UID. NULL
// indicates not found.
//
CTaskInfoMT *QCTaskManager::GetTaskInfo(unsigned long int nUID)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
		if( (*ti)->GetUID() == nUID)
			return (*ti);
		
	return NULL;
}

// --------------------------------------------------------------------------

// [PUBLIC] GetTaskError
//
// Find and return a pointer to the ErrorInfo identified by the UID. NULL
// indicates not found.
//
CTaskErrorMT *QCTaskManager::GetTaskError(unsigned long int nUID)
{
	CSingleLock lock(&m_Guard_TaskErrorList, TRUE);

	for(TaskErrorIterator ti=m_TaskErrorList.begin(); ti != m_TaskErrorList.end(); ++ti)
		if( (*ti)->GetUID() == nUID)
			return (*ti);
		
	return NULL;
}

int QCTaskManager::GetNumTaskErrors(CTime afterThisTime, TaskType tt)
{
	CSingleLock lock(&m_Guard_TaskErrorList, TRUE);

	int nErrors = 0;
	for(TaskErrorIterator ti=m_TaskErrorList.begin(); ti != m_TaskErrorList.end(); ++ti)
	{
		if( ((*ti)->GetTaskType() == tt) && ((*ti)->GetInfoTimeStamp() >= afterThisTime) )
			nErrors++;
	}

	return nErrors;
}


	


// --------------------------------------------------------------------------

// [PUBLIC] RemoveTaskInfo
//
// Removes and deletes the TaskInfo object from the task manager list.
// Returns true if successful.
//
bool QCTaskManager::RemoveTaskInfo(unsigned long int nUID)
{
	CSingleLock lock(&m_Guard_TaskInfoList, TRUE);
	bool bRemoved = false;

	CTaskInfoMT *pTaskInfo = NULL;
	for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
	{
		pTaskInfo = *ti;
		if( pTaskInfo->GetUID() == nUID)
		{
			m_TaskInfoList.erase(ti);
			
			//do some bookkeeping
			m_nActiveTasks--;
			
			if (pTaskInfo->NeedsDialup())
			{
				m_nActiveDialupTasks--;
				if(!m_nActiveDialupTasks && !LookupTask(TT_USES_DIALUP))
				{
					ASSERT(::IsMainThreadMT());
					CString	Cache =	g_Personalities.GetCurrent();
					g_Personalities.SetCurrent(pTaskInfo->GetPersona());
					if( GetIniShort(IDS_INI_RAS_CLOSE_AFTER_DONE))
						QCRasLibrary::CloseConnection();
					g_Personalities.SetCurrent(Cache);

				}

			}
			
			if(pTaskInfo->GetGroupID())
			{
				//we are a group.  If the last one in the group call the Group Callback
				
				bool bLastInGroup = true;
				int gid = pTaskInfo->GetGroupID();

				//Are we the last task in the group
				for(TaskIterator IT=m_TaskInfoList.begin(); IT != m_TaskInfoList.end(); ++IT)
				{
					if((*IT)->GetGroupID() == gid)
					{
						bLastInGroup = false;
						break;
					}
				}
				

				if( bLastInGroup)
				{
					// we are the last task.  Find our Callback function
					GroupIter gi = m_GroupMap.find(gid);
					if( gi != m_GroupMap.end())
					{
						GroupCallback gc = (*gi).second;
						//Run the callback
						if(gc)
							gc();
					}
				}
			}

			// If the error is a certificate error, ask user if they want to trust the cert anyway.
			if ((pTaskInfo->GetSSLError() > 0) && pTaskInfo->GetSSLCert())
			{
				CString				strRejection = pTaskInfo->GetSSLCertRejection();
				CString				strCertText = pTaskInfo->GetSSLCertText();
				CTrustCertDlg		trustCertDlg(NULL, strCertText, strRejection);
				if (trustCertDlg.DoModal() == IDOK)
				{
					FPNQCSSLAddTrustedUserCert	 fnAddTrustedUserCert = NULL;
					if (Network::GetQCSSLDll() != NULL)
					{
						fnAddTrustedUserCert = Network::GetQCSSLAddTrustedUserCert();
						if(fnAddTrustedUserCert)
						{
							fnAddTrustedUserCert((CertData*)(pTaskInfo->GetSSLCert()), NULL/*ignored*/, NULL/*ignored*/);
						}
					}
				}
			}

			//cleanup resources
			delete pTaskInfo->m_pThread;
			delete pTaskInfo; // Do we have to delete? I think so.
			
			bRemoved = true;

			//without this break, the next iterator will be invalid and crashes
			break;
		}
	}

	
	//Schedule tasks that might wating for this task to be completed..
	ScheduleTasks();

	return bRemoved;
}

// --------------------------------------------------------------------------



bool QCTaskManager::ClearTaskErrors(const char* strPersona, TaskType tt)
{
	CSingleLock lock(&m_Guard_TaskErrorList, TRUE);

	CTaskErrorMT *pTaskError = NULL;
	for(TaskErrorIterator ti=m_TaskErrorList.begin(); ti != m_TaskErrorList.end(); ++ti)
	{

		pTaskError = *ti;
		if( pTaskError->GetPersona().CompareNoCase(strPersona) == 0 && pTaskError->GetTaskType() == tt)
		{
			TaskErrorType terr = pTaskError->GetErrorType();
			if(terr & TERR_WINSOCK || terr & TERR_RAS)
				pTaskError->Kill();
		}
	}
	
	return true;
}




// [PUBLIC] RemoveTaskError
//
// Removes and deletes the TaskError object from the list.
// Returns true if successful.
//
bool QCTaskManager::RemoveTaskError(unsigned long int nUID)
{
	CSingleLock lock(&m_Guard_TaskErrorList, TRUE);

	for(TaskErrorIterator ti=m_TaskErrorList.begin(); ti != m_TaskErrorList.end(); ++ti)
		if( (*ti)->GetUID() == nUID)
		{
			delete (*ti); // Do we have to delete? I think so.
			m_TaskErrorList.erase(ti);
			return (true);
		}

	return (false);
}

// --------------------------------------------------------------------------

// [PUBLIC] SetDisplayWindow
//
// Assigns this TaskManager a display window. All tasks managed by this
// manager should post messages to this window.
//
//	CWnd *SetDisplayWindow(CTaskObjectMT::TaskObjectType type, CWnd *pWnd);

CWnd *QCTaskManager::SetDisplayWindow(CTaskObjectMT::TaskObjectType type, CWnd *pWnd)
{
	CSingleLock lock(&m_ObjGuard, TRUE); // Lock for access to object member variable
	
	CWnd *pOldWnd = m_DispWndMap[type];
	m_DispWndMap[type] = pWnd;

	// If we're replacing an old window, we need to tell the task objects
	// that their window is being replaced with a different one.
	if (pOldWnd)
	{
		if (type == CTaskObjectMT::TOBJ_INFO)
		{
			CSingleLock lock(&m_Guard_TaskInfoList, TRUE);

			for(TaskIterator ti=m_TaskInfoList.begin(); ti != m_TaskInfoList.end(); ++ti)
			{
				CTaskInfoMT* pTaskInfo = *ti;
				if (pTaskInfo->GetDisplayWnd() == pOldWnd)
					pTaskInfo->SetDisplayWnd(pWnd, true);
			}
		}
		else if (type == CTaskObjectMT::TOBJ_ERROR)
		{
			CSingleLock lock(&m_Guard_TaskErrorList, TRUE);

			for(TaskErrorIterator ti=m_TaskErrorList.begin(); ti != m_TaskErrorList.end(); ++ti)
			{
				CTaskErrorMT* pTaskError = *ti;
				if (pTaskError->GetDisplayWnd() == pOldWnd)
					pTaskError->SetDisplayWnd(pWnd, true);
			}
		}
		else
			ASSERT(0);	// Uh oh, unkown task type
	}

	return (pOldWnd);
}

// --------------------------------------------------------------------------

// Global THREAD function
UINT ThreadStartupFunc(LPVOID lpv)
{
	// Catch most exceptions that could occur in any thread other
	// than the main thread so that they don't cause Eudora to quit.
	bool	bExceptionOccurred = false;
	int		nResult = 0;

	try
	{
		QCWorkerThreadMT *	pWorkThread = (QCWorkerThreadMT *)lpv;

		if (pWorkThread)
		{
			pWorkThread->DoWork();

			QCGetTaskManager()->RemoveWorkerThread(pWorkThread);
		}
	}
	catch (CMemoryException * pMemoryException)
	{
		// MFC currently makes operator new throw a CMemoryException
		pMemoryException->Delete();

		bExceptionOccurred = true;

		// We can use the return code here to pass back information to the
		// main thread. The information can be retrieved via GetExitCodeThread.
		nResult = -1;
	}
	catch (std::bad_alloc & /* exception */)
	{
		// I don't think we can currently get this given MFC's overriding of
		// operator new, but perhaps they'll be standards compliant in the future.
		bExceptionOccurred = true;

		// We can use the return code here to pass back information to the
		// main thread. The information can be retrieved via GetExitCodeThread.
		nResult = -1;
	}

	ASSERT(!bExceptionOccurred);

	return nResult;
}




QCTaskGroup::QCTaskGroup(GroupCallback gc, bool bIgnoreIdle)
{ 
	m_bTopLevel = QCGetTaskManager()->DelayTasks(gc, bIgnoreIdle); 
}

QCTaskGroup::~QCTaskGroup()
{ 
	if(m_bTopLevel) 
		QCGetTaskManager()->StartTasks(); 
}
