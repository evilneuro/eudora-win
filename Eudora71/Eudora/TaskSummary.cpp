// TaskSummary.cpp: implementation of the CTaskSummary class.
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskInfo.h"

#include "TaskSummary.h"


#include "DebugNewHelpers.h"


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

