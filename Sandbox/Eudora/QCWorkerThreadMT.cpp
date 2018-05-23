// QCWorkerThreadMT.cpp: implementation of the QCWorkerThreadMT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QCWorkerThreadMT.h"
#include "TaskStatusView.h"
#include "QCTaskManager.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// QCWorkerThreadMT

QCWorkerThreadMT::QCWorkerThreadMT()
{
	m_bIsCurrentTaskInfo = FALSE;
	m_bStopThread = false;
	
	m_pTaskInfo = new CTaskInfoMT;
	m_pTaskInfo->m_pThread = this;
	m_pTaskInfo->SetState(TSS_CREATED);
		
}


QCWorkerThreadMT::~QCWorkerThreadMT(){}
