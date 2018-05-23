// TaskSummary.cpp: implementation of the CTaskSummary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskInfo.h"

#include "TaskSummary.h"

// --------------------------------------------------------------------------

CTaskSummary::CTaskSummary(CTaskInfoMT *p /* = NULL */ )
	: m_UID(0), m_MainState(TSS_UNKNOWN), m_PostState(TSS_POST_UNKNOWN)
{
	ASSERT(p);

	if (p)
	{
		m_UID = p->GetUID();
		m_MainState = p->GetState();				
		m_PostState = p->NeedsPostProcessing() ? TSS_POST_WAITING : TSS_POST_NONE; // p->GetPostState();
		m_nPercentComplete = ((unsigned int) p->GetPercentDone());

		m_TaskType = p->GetTaskType();
	}
}

// --------------------------------------------------------------------------

CTaskSummary::CTaskSummary(const CTaskSummary &copy)
{
	m_UID = copy.m_UID;
	m_MainState = copy.m_MainState;
	m_PostState = copy.m_PostState;
}

// --------------------------------------------------------------------------

CTaskSummary::~CTaskSummary()
{ }

// --------------------------------------------------------------------------

bool CTaskSummary::SetInfo(CTaskInfoMT *p)
{
	if (p)
	{
		CTaskSummary tmp(p);

		if (operator!=(tmp))
		{
			m_UID = p->GetUID();
			m_MainState = p->GetState();				
			m_PostState = p->NeedsPostProcessing() ? TSS_POST_WAITING : TSS_POST_NONE; // p->GetPostState();
			m_nPercentComplete = ((unsigned int) p->GetPercentDone());

			return (true);
		}
	}

	return (false);
}

// --------------------------------------------------------------------------

bool CTaskSummary::operator==(const CTaskSummary &x)
{
	return ((m_UID == x.m_UID) && (m_MainState == x.m_MainState)
				&& (m_PostState == x.m_PostState) && (m_nPercentComplete == x.m_nPercentComplete));
}

// --------------------------------------------------------------------------

bool CTaskSummary::operator!=(const CTaskSummary &x)
{
	return ((m_UID != x.m_UID) || (m_MainState != x.m_MainState)
				|| (m_PostState != x.m_PostState) || (m_nPercentComplete != x.m_nPercentComplete));
}

