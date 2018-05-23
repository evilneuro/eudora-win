// TaskStatus.h: interface for the CTaskStatus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASKSTATUS_H__12938FBD_FEF2_11D1_94D0_00805F9BF4D7__INCLUDED_)
#define AFX_TASKSTATUS_H__12938FBD_FEF2_11D1_94D0_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

#include "stdafx.h"
#include "TaskSummary.h"

// --------------------------------------------------------------------------

class CTaskStatus  
{
public:
	CTaskStatus();
	CTaskStatus(const CTaskStatus &copy);
	virtual ~CTaskStatus();

	bool ProcessSummary(const CTaskSummary *pSum);

	bool operator==(const CTaskStatus& x);
	bool operator!=(const CTaskStatus& x);

	bool IsDifferent(const CTaskStatus& x, unsigned int nMinPctDelta = 1);

	bool AnyRunning() const;
	bool AnyWaiting() const;
	bool HasPercent() const;

	unsigned int GetPercent() const;
	unsigned int GetPctCount() const;
	unsigned int GetRunCount() const;

	unsigned int GetSendCount() const;
	unsigned int GetRecvCount() const;

protected:
	bool m_bRunning, m_bWaiting;
	unsigned int m_nPercent, m_nPctCnt, m_nRunCnt;

	unsigned int m_nSendCount, m_nRecvCount;
};

#endif // !defined(AFX_TASKSTATUS_H__12938FBD_FEF2_11D1_94D0_00805F9BF4D7__INCLUDED_)
