// TaskErrorMT.cpp : implementation file
//

#include "stdafx.h"
#include "EudoraMsgs.h"

// Debug Memory Manager block
// This should be after all system includes and before any user includes
#ifdef _DEBUG 
	#undef THIS_FILE 
	static char THIS_FILE[] = __FILE__; 
	#define new DEBUG_NEW 
#endif // _DEBUG

#include "TaskObject.h"
#include "TaskInfo.h"

#include "TaskErrorMT.h"

// --------------------------------------------------------------------------

CTaskErrorMT::CTaskErrorMT(CTaskInfoMT *pInfo /* = NULL */, LPCSTR pStr /* = NULL */, TaskErrorType terr /*= TERR_UNKNOWN*/)
	: CTaskObjectMT(),
		m_strPersona(""), m_MainText(""), m_SecondaryText(""), m_Total(-1), m_SoFar(-1), m_CurPct(-1),
		m_MainState(TSS_UNKNOWN), m_PostState(TSS_POST_UNKNOWN), m_TaskType(TASK_TYPE_UNKNOWN),
		m_Title(""), m_bNeedsPost(false), m_ErrStr(""), m_ErrorType(terr), m_pError(NULL)
{
	ASSERT(pInfo);
	ASSERT(pStr);

	if (pInfo)
	{
		m_strPersona = pInfo->GetPersona();
		m_MainText = pInfo->GetMainText();
		m_SecondaryText = pInfo->GetSecondaryText();
		m_Total = pInfo->GetTotal();
		m_SoFar = pInfo->GetSoFar();
		m_CurPct = pInfo->GetPercentDone();
		m_MainState = pInfo->GetState();
		m_PostState = pInfo->GetPostState();
		m_TaskType = pInfo->GetTaskType();
		m_Title = pInfo->GetTitle();
		m_bNeedsPost = pInfo->NeedsPostProcessing();
		m_InfoTimeStamp = pInfo->GetTimeStamp();
	}

	if (pStr)
		m_ErrStr = CString(pStr);
}

// --------------------------------------------------------------------------

CTaskErrorMT::~CTaskErrorMT()
{
	if(m_pError)
		delete m_pError;
}

// --------------------------------------------------------------------------

// [PUBLIC] GetObjectType
//
// Pure-virtual base class function. Returns the object type.
//
CTaskObjectMT::TaskObjectType CTaskErrorMT::GetObjectType()
{
	return (CTaskObjectMT::TOBJ_ERROR); // This is an error object
}

// --------------------------------------------------------------------------


// [PROTECTED] NotifyAlive
//
// Sends a message to the display window indicating this obj exists.
//
void CTaskErrorMT::NotifyAlive()
{
	PostDisplayMsg(msgErrorViewNewError, (WPARAM) this, (LPARAM) GetUID());
}

// --------------------------------------------------------------------------

// [PROTECTED] NotifyInfoChanged
//
// Sends a message to the display window indicating this obj has changed.
//
void CTaskErrorMT::NotifyInfoChanged()
{
	ASSERT(0); // Errors are static info, never to change
}

// --------------------------------------------------------------------------

// [PROTECTED] NotifyDying
//
// Sends a message to the display window indicating this obj is about to die.
//
void CTaskErrorMT::NotifyDead()
{
	PostDisplayMsg(msgErrorViewDelError, (WPARAM) this, (LPARAM) GetUID(), true);
}

