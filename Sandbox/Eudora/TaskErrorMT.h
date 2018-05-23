// TaskErrorMT.h : header file
//

#if !defined(AFX_TASKERRORMT_H__09855E3F_D61C_11D1_94BC_00805F9BF4D7__INCLUDED_)
#define AFX_TASKERRORMT_H__09855E3F_D61C_11D1_94BC_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxmt.h>
#include "resource.h"

#include "TaskObject.h"
#include "TaskInfo.h"


// --------------------------------------------------------------------------

class CTaskErrorMT : public CTaskObjectMT
{
public:
	CTaskErrorMT(CTaskInfoMT *pInfo = NULL, LPCSTR pStr = NULL, TaskErrorType err = TERR_UNKNOWN);
	~CTaskErrorMT();

	virtual TaskObjectType GetObjectType();

	CString GetPersona()  { return m_strPersona; }
	CString	GetMainText() { return m_MainText; }
	CString	GetSecondaryText() { return m_SecondaryText; }
	int		GetTotal() { return m_Total; }
	int		GetSoFar() { return m_SoFar; }
	int     GetPercentDone() { return m_CurPct; }
	TaskStatusState	GetState() { return m_MainState; }
	PostProcState	GetPostState() { return m_PostState; }
	TaskType GetTaskType() { return m_TaskType; }
	CString	GetTitle() { return m_Title; }
	bool NeedsPostProcessing() { return m_bNeedsPost; }
	CString GetErrText() { return m_ErrStr; }
	CTime GetInfoTimeStamp() { return m_InfoTimeStamp; }

	TaskErrorType GetErrorType(){ return m_ErrorType; }
	void SetErrorObject(QCError *pErr) { m_pError = pErr; }
	QCError *GetErrorObject(){ return m_pError; }
	void ShowContext(){ if(m_pError) m_pError->ShowContext(); }

protected:
	virtual void NotifyAlive();       // Notify the display that this info exists
	virtual void NotifyInfoChanged(); // Notify the display that this info has changed
	virtual void NotifyDead();        // Notify the display that this info is about to die
	
	// All data is read-only !!!

	CString m_strPersona;
	CString	m_MainText;
	CString	m_SecondaryText;
	int		m_Total;
	int		m_SoFar;
	int     m_CurPct; // Current percent
	TaskStatusState	m_MainState;
	PostProcState m_PostState;

	TaskErrorType m_ErrorType;
	QCError *m_pError;

	TaskType		m_TaskType;
	CString			m_Title;
	
	bool m_bNeedsPost;

	CString m_ErrStr;
	CTime m_InfoTimeStamp;
};

#endif // !defined(AFX_TASKERRORMT_H__09855E3F_D61C_11D1_94BC_00805F9BF4D7__INCLUDED_)
