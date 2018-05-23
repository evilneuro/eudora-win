// TaskSummary.h: interface for the CTaskSummary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASKSUMMARY_H__609B8CC8_0145_11D2_94D1_00805F9BF4D7__INCLUDED_)
#define AFX_TASKSUMMARY_H__609B8CC8_0145_11D2_94D1_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

#include "TaskInfo.h"

// --------------------------------------------------------------------------

class CTaskSummary  
{
public:
	virtual ~CTaskSummary();

	CTaskSummary(CTaskInfoMT *p = NULL);
	CTaskSummary(const CTaskSummary &copy);

	bool operator==(const CTaskSummary &x);
	bool operator!=(const CTaskSummary &x);

	bool SetInfo(CTaskInfoMT *p);

	unsigned long int m_UID;
	TaskStatusState	m_MainState;
	PostProcState m_PostState;
	unsigned int m_nPercentComplete;
	TaskType m_TaskType;
};

#endif // !defined(AFX_TASKSUMMARY_H__609B8CC8_0145_11D2_94D1_00805F9BF4D7__INCLUDED_)
