// TaskObject.cpp : implementation file for CTaskObjectMT
//

#include <math.h>
#include "stdafx.h"
#include "resource.h"

#include "TaskObject.h"

// Debug Memory Manager block
// Place in all .h and .cpp files, after all includes
#ifdef _DEBUG
	#undef THIS_FILE 
	static char THIS_FILE[] = __FILE__; 
	#undef new
	#define new DEBUG_NEW
#endif // _DEBUG

// --------------------------------------------------------------------------

unsigned long int CTaskObjectMT::m_nNextUID = 1; // Start the UIDs from 1 (class-global variable)
CCriticalSection CTaskObjectMT::m_ClassGuard; // Class-global guard for access to the UID variable

// --------------------------------------------------------------------------

CTaskObjectMT::CTaskObjectMT()
	: m_bDead(false), m_pDisplayWnd(NULL)
{
	SetupUID(); // Get a unique identifier for this object
	m_TimeStamp = CTime::GetCurrentTime(); // Time-stamp the error
}

// --------------------------------------------------------------------------

// [PRIVATE] SetupUID
//
// Assigns this object a unique indentifer.
//
void CTaskObjectMT::SetupUID()
{
	CSingleLock lock(&m_ClassGuard, TRUE); // Lock for access to class-static variable
	m_UID = (m_nNextUID++);
}

// --------------------------------------------------------------------------

// [PUBLIC] GetUID
//
// Returns the unique identifier for this object. The identifier is unique
// across all objects of this class.
//
unsigned long int CTaskObjectMT::GetUID()
{
	return (m_UID); // read-only variable needs no lock
}

// --------------------------------------------------------------------------

// [PUBLIC] GetTimeStamp
//
// Returns timestamp of this object -- this time when the object was created.
//
CTime CTaskObjectMT::GetTimeStamp()
{
	return (m_TimeStamp); // read-only variable needs no lock
}

// --------------------------------------------------------------------------

// [PUBLIC] SetDisplayWnd
//
// Changes the associated display window. This is the window which receives
// the display messages for this obj. If bNofity is true, a message is sent
// to notify the window this obj exists.
//
// Returns old display window.
//
CWnd *CTaskObjectMT::SetDisplayWnd(CWnd *pWnd, bool bNotify /* = false */)
{
	m_DispWndGuard.Lock();
	CWnd *pOldVal = m_pDisplayWnd;
	m_pDisplayWnd = pWnd;
	m_DispWndGuard.Unlock();

	if (bNotify)
		NotifyAlive();

	return (pOldVal);
}

// --------------------------------------------------------------------------

// [PUBLIC] PostDisplayMsg

bool CTaskObjectMT::PostDisplayMsg(UINT message, WPARAM wParam /* = 0 */, LPARAM lParam /* = 0*/, bool bIgnoreDead /* = false */)
{
	if ((bIgnoreDead) || (IsAlive()))
	{
		CSingleLock lock(&m_DispWndGuard, TRUE); // Lock for access to object member variable

		if (m_pDisplayWnd)
			return (m_pDisplayWnd->PostMessage(message, (WPARAM) wParam, (LPARAM) lParam) == TRUE);
	}

	return (false);
}

// --------------------------------------------------------------------------

// [PUBLIC] Kill
//
// A quick death. But you only die once!
//
void CTaskObjectMT::Kill()
{
	m_DeadGuard.Lock();
	
	// We can't do any notification after we're dead, so get the lock
	// try to notify, then die.
	//
	// Having the lock is important during the notification, because it
	// means the dead state (m_bDead) cannot change, but can be true.
	//
	m_bDead = true;
	NotifyDead();
	
	m_DeadGuard.Unlock();
}

// --------------------------------------------------------------------------

// [PUBLIC] IsAlive
//
// Is this info dead or alive?
//
bool CTaskObjectMT::IsAlive()
{
	CSingleLock lock(&m_DeadGuard, TRUE); // Lock for access to object member variable
	return (!m_bDead);
}

// --------------------------------------------------------------------------
