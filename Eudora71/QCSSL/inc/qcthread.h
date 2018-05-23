
#ifndef _THREADS_H_
#define _THREADS_H_
 
#include <windows.h>

 

class QCMutex
{
	HANDLE  m_hObject;

public:
	QCMutex(bool bInitiallyOwn = FALSE, LPCTSTR lpszName = NULL,
		LPSECURITY_ATTRIBUTES lpsaAttribute = NULL);
	~QCMutex();
	operator HANDLE() const { return m_hObject;}
	bool Lock(DWORD dwTimeout = INFINITE);
	bool Unlock();
};

#endif 