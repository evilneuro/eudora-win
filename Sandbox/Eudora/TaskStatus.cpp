// TaskStatus.cpp: implementation of the CTaskStatus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "TaskSummary.h"

#include "TaskStatus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// --------------------------------------------------------------------------

CTaskStatus::CTaskStatus()
	: m_bRunning(false), m_bWaiting(false), m_nPercent(0), m_nPctCnt(0),
	  m_nRunCnt(0), m_nSendCount(0), m_nRecvCount(0)
{ }

// --------------------------------------------------------------------------

CTaskStatus::CTaskStatus(const CTaskStatus &copy)
{
	m_bRunning = copy.m_bRunning;
	m_bWaiting = copy.m_bWaiting;
	m_nPercent = copy.m_nPercent;
	m_nPctCnt = copy.m_nPctCnt;
	m_nRunCnt = copy.m_nRunCnt;
	m_nSendCount = copy.m_nSendCount;
	m_nRecvCount = copy.m_nRecvCount;
}

// --------------------------------------------------------------------------

CTaskStatus::~CTaskStatus()
{ }

// --------------------------------------------------------------------------

bool CTaskStatus::operator==(const CTaskStatus& x)
{
	return ((m_bRunning == x.m_bRunning)
		&& (m_bWaiting == x.m_bWaiting)
		&& (m_nPercent == x.m_nPercent)
		&& (m_nPctCnt == x.m_nPctCnt)
		&& (m_nRunCnt == x.m_nRunCnt)
		&& (m_nSendCount == x.m_nSendCount)
		&& (m_nRecvCount == x.m_nRecvCount)
		);
}

// --------------------------------------------------------------------------

bool CTaskStatus::operator!=(const CTaskStatus& x)
{
	return ((m_bRunning != x.m_bRunning)
		|| (m_bWaiting != x.m_bWaiting)
		|| (m_nPercent != x.m_nPercent)
		|| (m_nPctCnt != x.m_nPctCnt)
		|| (m_nRunCnt != x.m_nRunCnt)
		|| (m_nSendCount != x.m_nSendCount)
		|| (m_nRecvCount != x.m_nRecvCount)
		);
}

// --------------------------------------------------------------------------

bool CTaskStatus::AnyRunning() const
{
	return (m_bRunning);
}

// --------------------------------------------------------------------------

bool CTaskStatus::AnyWaiting() const
{
	return (m_bWaiting);
}

// --------------------------------------------------------------------------

bool CTaskStatus::HasPercent() const
{
	return (m_nPctCnt > 0);
}

// --------------------------------------------------------------------------

unsigned int CTaskStatus::GetPercent() const
{
	return (m_nPercent);
}

// --------------------------------------------------------------------------

unsigned int CTaskStatus::GetRunCount() const
{
	return (m_nRunCnt);
}

// --------------------------------------------------------------------------

unsigned int CTaskStatus::GetPctCount() const
{
	return (m_nPctCnt);
}

// --------------------------------------------------------------------------

unsigned int CTaskStatus::GetSendCount() const
{
	return (m_nSendCount);
}

// --------------------------------------------------------------------------

unsigned int CTaskStatus::GetRecvCount() const
{
	return (m_nRecvCount);
}

// --------------------------------------------------------------------------

bool CTaskStatus::ProcessSummary(const CTaskSummary *pSum)
{
	if ((!m_bWaiting) && (TSS_POST_WAITING == pSum->m_PostState))
		m_bWaiting = true;

	if ((TSS_CREATED == pSum->m_MainState) || (TSS_QUEUED == pSum->m_MainState) || (TSS_RUNNING == pSum->m_MainState))
	{
		m_nRunCnt++;

		if (!m_bRunning)
			m_bRunning = true;

		if ((pSum->m_nPercentComplete >=0) && (pSum->m_nPercentComplete <= 100))
		{
			const unsigned int oldSumCount = (m_nPctCnt++);
			const unsigned int nTop = (oldSumCount * m_nPercent) + (pSum->m_nPercentComplete);
			const double tmpPct = ( ((double) nTop) / ((double) m_nPctCnt) );

			m_nPercent = ((unsigned int) floor(tmpPct + 0.5));

			if (m_nPercent < 0)
				m_nPercent = 0;

			if (m_nPercent > 100)
				m_nPercent = 100;
		}

		switch (pSum->m_TaskType)
		{
			case (TASK_TYPE_SENDING):
			{
				m_nSendCount++;
			}
			break;

			case (TASK_TYPE_RECEIVING):
			{
				m_nRecvCount++;
			}
			break;
		}
	}

	return (true);
}

// --------------------------------------------------------------------------

bool CTaskStatus::IsDifferent(const CTaskStatus& x, unsigned int nMinPctDelta /* = 1 */)
{
	bool a = (m_bRunning != x.m_bRunning);
	bool b = (m_bWaiting != x.m_bWaiting);
	unsigned int n = ((unsigned int)labs((long)m_nPercent - (long)x.m_nPercent));
	bool c = (n >= nMinPctDelta);

	return (a || b || c);

	//	return ((m_bRunning != x.m_bRunning) || (m_bWaiting != x.m_bWaiting)
///		&& (((unsigned int)labs((long)m_nPercent - (long)x.m_nPercent)) >= nMinPctDelta) );
}

