//
// TaskObject.h : header file
//

#ifndef _TASKOBJECT_H_
#define _TASKOBJECT_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxmt.h>
#include "resource.h"

// --------------------------------------------------------------------------

class CTaskObjectMT
{
public:
	CTaskObjectMT();

	CWnd *SetDisplayWnd(CWnd *, bool bNotify = false); // Change the window which receives display messages	
	unsigned long int GetUID(); // Get the UID of this object (uniqueness is only within this class & derived classes)
	CTime GetTimeStamp(); // Get the timestamp of this object
	void Kill();
	bool IsAlive();

	typedef enum { TOBJ_INFO, TOBJ_ERROR } TaskObjectType;

	virtual TaskObjectType GetObjectType() = 0;

protected:
	// Post a msg to the display wnd
	bool PostDisplayMsg(UINT message, WPARAM wParam = 0, LPARAM lParam = 0, bool bIgnoreDead = false);

	CCriticalSection m_DeadGuard; // Guard for these member variables
		bool m_bDead;

	CCriticalSection m_DispWndGuard; // Guard for these member variables
		CWnd *m_pDisplayWnd; // This is the window which is sent msgs when info is changed

	virtual void NotifyAlive() = 0;       // Notify the display that this info exists
	virtual void NotifyInfoChanged() = 0; // Notify the display that this info has changed
	virtual void NotifyDead() = 0;        // Notify the display that this info is about to die

private:
	void SetupUID(); // Called only once internally to assign a UID to this object

	unsigned long int m_UID; // This objects class-unique identifier
	CTime m_TimeStamp;

	static CCriticalSection m_ClassGuard; // Class-global guard for access to the UID variable
	static unsigned long int m_nNextUID; // Class-global counter to track UID for TaskObject objects
};

#endif // _TASKOBJECT_H_
