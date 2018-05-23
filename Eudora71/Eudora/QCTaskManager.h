// QCTaskManager.h
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

#ifndef _QCTaskManager_h_
#define _QCTaskManager_h_

#include "TaskInfo.h"
#include "TaskErrorMT.h"
#include "Callbacks.h"
#include "helpxdlg.h"			// for CHelpxDlg

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

class CTrustCertDlg : public CHelpxDlg
{
	CString		 m_strCertText;
	CString		 m_strReasonText;

	// Construction
public:
	CTrustCertDlg(CWnd *pwndParent, CString &strCertText, CString &strReasonText);

// Dialog Data
	//{{AFX_DATA(CTrustCert)
	enum { IDD = IDD_TRUST_CERT };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrustCert)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
};

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
	bool ClearTaskErrors(const char* strPersona, TaskType tt);


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
	
	bool IsTaskRunning(const char* strPersona, int ScheduleBits);
	bool LookupTask(int ScheduleBits);
	
	CWnd *SetDisplayWindow(CTaskObjectMT::TaskObjectType type, CWnd *pWnd);

	int GetTaskCount(const char* strPersona = NULL);

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
	std::list<CTaskErrorMT *> m_TaskErrorList;  //STL list to keep track of CTaskInfoMT's
	typedef std::list<CTaskErrorMT *>::iterator TaskErrorIterator;

	CCriticalSection m_Guard_TaskInfoList; // This guards ONLY the list 'm_TaskInfoList'
	std::list<CTaskInfoMT *> m_TaskInfoList;  //STL list to keep track of CTaskInfoMT's
	typedef std::list<CTaskInfoMT *>::iterator TaskIterator;
	typedef std::list<CTaskInfoMT *>::reverse_iterator RTI;

	
	CCriticalSection m_Group_Guard; // This guards ONLY the list 'm_TaskInfoList'
	std::map<int, GroupCallback> m_GroupMap;  //STL list to keep track of CTaskInfoMT's
	typedef std::map<int, GroupCallback>::iterator GroupIter;
	
	std::map<CTaskObjectMT::TaskObjectType, CWnd *> m_DispWndMap;

	std::list<CTaskInfoMT *> m_PostProcessList;
	CCriticalSection m_PostProcessListGuard;
	bool m_bPostProcessInProgress;
	
	bool m_bDelayQueuing;
	bool m_bIgnoreIdle;
	static int s_nGroupID;

	unsigned long m_nStartIdle;

	//rules
	int m_nMaxConcurrentTasks;
	int m_nActiveTasks;
	int m_nActiveDialupTasks;
	int m_nMaxConcurrentDialupTasks;
};

//Helper functions
QCTaskManager *QCGetTaskManager();
	

class QCTaskGroup
{
	bool m_bTopLevel;
public:	
	QCTaskGroup(GroupCallback gc = NULL, bool bIgnoreIdle=false);
	~QCTaskGroup();
};


#endif

