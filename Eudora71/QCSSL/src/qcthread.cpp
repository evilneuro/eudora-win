#include "stdafx.h"

#include <process.h>
#include "qcthread.h"

#include "DebugNewHelpers.h"


#include <process.h>

QCMutex::QCMutex(bool bInitiallyOwn, LPCTSTR pstrName,
	LPSECURITY_ATTRIBUTES lpsaAttribute )
{
	m_hObject = ::CreateMutex(lpsaAttribute, bInitiallyOwn, pstrName);
//	if (m_hObject == NULL)
//		;//throw exception
}


QCMutex::~QCMutex()
{
		CloseHandle(m_hObject);
}

bool QCMutex::Lock(DWORD dwTimeout)
{
	DWORD	waitResult;
	waitResult = WaitForSingleObject( m_hObject, dwTimeout );
	switch ( waitResult ) 
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return TRUE;
			break;
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			return FALSE;
			break;
		}
}

bool QCMutex::Unlock( )
{
		return (ReleaseMutex(m_hObject) != FALSE);
}

