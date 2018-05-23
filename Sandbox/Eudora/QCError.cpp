/*
 *   QCError.cpp - Implements Error related code
 *
 */

#include "stdafx.h"

#include "QCUtils.h"
#include "TaskErrorMT.h"
#include "summary.h"
#include "tocdoc.h"


void QCSmtpMsgError::ShowContext()
{
	ASSERT( ::IsMainThreadMT());
	ASSERT(m_nMsgID >= 0);

	CTocDoc *Toc = GetOutToc();
	if (!Toc)
		return;

	CSummary *Sum = NULL;
	bool bFound = false;
	
	//Start search from the end, because thats where new messages would be!!
	for (POSITION pos = Toc->m_Sums.GetTailPosition(); pos; )
	{
		Sum = Toc->m_Sums.GetPrev(pos);
		if(Sum->GetUniqueMessageId() == m_nMsgID)
		{
			bFound = true;
			break;
		}
	}

	if(bFound)
		Sum->Display();

}
