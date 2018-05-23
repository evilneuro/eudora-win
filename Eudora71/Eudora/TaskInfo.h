// TaskInfo.h : header file
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
		TASK_TYPE_INDEXED_SEARCH_INDEXING,
		TASK_TYPE_IMAP
	};

	enum TaskScheduling {
		TT_USES_POP		=	1,
		TT_USES_SMTP	=	2,
		TT_USES_DIALUP	=	4
	};


	enum SSLStatus
	{
		SSL_NONE,
		SSL_NORMAL,
		SSL_FAIL,
		SSL_LOWSECURITY,
		SSL_TOPSECURITY
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
	void IgnoreError(LPCSTR str, TaskErrorType terr = TERR_UNKNOWN);
	void CreateErrorContext(LPCSTR str, TaskErrorType terr, QCError *pErr);

	void SetTaskType(TaskType type);
	void SetPersona(const char* persona);
	void SetMainText(const char* NewText);
	void SetSecondaryText(const char* NewText);
	void SetTotal(int NewTotal);
	void SetSoFar(int NewSoFar);
	void SetState(TaskStatusState NewState);
	void SetPostState(PostProcState NewState);
	void SetTitle(const char* NewText);
	void SetGroupID(int gid){ m_nGroupID = gid; }
	int  GetGroupID(){ return m_nGroupID; }
	void SetNotifyInfoChanged();

	void IgnoreIdle(bool bIgnoreIdle = true){ InterlockedExchange((LONG*)&m_bIgnoreIdle, bIgnoreIdle); }
	bool IsIgnoreIdleSet(){ return m_bIgnoreIdle; }

	bool ShouldAllowTaskStatusBringToFront() const { return m_bAllowTaskStatusBringToFront; }

	bool ShouldAllowTaskStop() const { return m_bAllowTaskStop; }

	void Progress(const char* NewText);
	void ProgressAdd(int Increment);

	// Assigns the post-processing state and any function, if needed
	void SetPostProcessing(bool bNeedsPost);
	void SetPostProcFn(LPCSTR sDispStr, TaskInfoPostFnPtr pFn, void *pArg = NULL);

	bool NeedsPostProcessing(); // Does this object need post-processing?
	bool HasPostFunction(); // Does this object have a post-processing function?

	virtual void RequestThreadStop();

	bool NeedsDialup(){ return m_strDialupEntry.IsEmpty()?false:true; };
	void SetDialupEntryName(const char* name){ m_strDialupEntry = name; }
	CString GetDialupEntryName(){ return m_strDialupEntry; }
	void SetScheduleTypes(int nTypes){ m_nScheduler = m_nScheduler | nTypes; }
	int GetScheduleTypes(){ return m_nScheduler; }
	void SetSSL(SSLStatus ssl) { m_SSL = ssl;}
	SSLStatus GetSSL(){return m_SSL;}
	void SetSSLError(int iSSLError) {m_iSSLError = iSSLError;}
	int GetSSLError() {return m_iSSLError;}
	void SetSSLCertText(CString &strCertText) {m_strSSLCertText = strCertText;}
	CString GetSSLCertText() {return m_strSSLCertText;}
	void SetSSLCert(void *pCertData) {m_pCertData = pCertData;}
	void *GetSSLCert() {return m_pCertData;}
	void SetSSLCertRejection(CString &strSSLCertRejection) {m_strSSLCertRejection = strSSLCertRejection;}
	CString GetSSLCertRejection() {return m_strSSLCertRejection;}

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
		bool		m_bAllowTaskStatusBringToFront;
		bool		m_bAllowTaskStop;
		bool		m_bCountTask;
		SSLStatus	 m_SSL;
		int			 m_iSSLError;
		CString		 m_strSSLCertText;
		void		*m_pCertData;
		CString		 m_strSSLCertRejection;

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

		CCriticalSection		m_csShouldNotifyInfoChangedGuard;
		bool					m_bShouldNotifyInfoChanged;

	// Local (protected) member functions

	void CalcPercent(); // Update the current pct var
	
	virtual void NotifyAlive();   // Notify the display that this info exists
	virtual void NotifyInfoChanged(); // Notify the display that this info has changed
	virtual void NotifyDead();       // Notify the display that this info is about to die
};


#endif // !defined(AFX_TASKSTATUSINFO_H__7CA22ED2_A827_11D1_92DB_0060082B6AF8__INCLUDED_)
