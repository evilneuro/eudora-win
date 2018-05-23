//	SearchManagerTaskInfo.cpp
//
//	Search Manager task information progress display.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#include "stdafx.h"
#include "SearchManagerTaskInfo.h"
#include "SearchManager.h"


// ---------------------------------------------------------------------------
//		* SearchManagerTaskInfo										 [Public]
// ---------------------------------------------------------------------------
//	SearchManagerTaskInfo constructor.

SearchManagerTaskInfo::SearchManagerTaskInfo(
	bool				in_bAllowTaskStop)
	:	CTaskInfoMT()
{
	m_TaskType = TASK_TYPE_INDEXED_SEARCH_INDEXING;

	//	Don't allow queuing task to bring task status to front because
	//	we want to avoid disrupting the user.
	m_bAllowTaskStatusBringToFront = false;

	//	Don't count this task when counting tasks so that we idle
	//	correctly and so that we don't warn when quitting.
	m_bCountTask = false;

	//	Set whether or not we'll support stopping this task.
	m_bAllowTaskStop = in_bAllowTaskStop;
}


// ---------------------------------------------------------------------------
//		* RequestThreadStop											 [Public]
// ---------------------------------------------------------------------------
//	Requests that "thread" (or in our case idle processing) be stopped.

void
SearchManagerTaskInfo::RequestThreadStop()
{
	//	Don't call the base class, which would ASSERT because it
	//	expects that there will be a thread.

	//	Tell SearchManager to stop scanning
	SearchManager::Instance()->StopScanning();
}


