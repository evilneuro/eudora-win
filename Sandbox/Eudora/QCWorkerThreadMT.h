// QCWorkerThreadMT.h: interface for the QCWorkerThreadMT class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _QCWorkerThreadMT_h_
#define _QCWorkerThreadMT_h_

#include "TaskInfo.h"


/////////////////////////////////////////////////////////////////////////////
// QCWorkerThreadMT thread
//Abstract Base Class


class QCWorkerThreadMT //: public CWinThread
{
protected:
	QCWorkerThreadMT();
	
// Attributes
public:

// Operations
public:
	
	virtual HRESULT DoWork() = 0;
	CTaskInfoMT *GetTaskInfo(){ return m_pTaskInfo;}

	//virtual CString GetTaskName(const chr* szPersona);
	virtual void RequestThreadStop() = 0;

	bool IsStoppingThread(){ return m_bStopThread; }  // Call all the time.
	//bool IsNetworkThread() { return m_bUsesNetwork; }
	
	virtual ~QCWorkerThreadMT();
	void SetAsCurrentTask(bool bNewState = true)	{ m_bIsCurrentTaskInfo = bNewState; }

	
protected:

	// Info for Task Status window
	CTaskInfoMT *m_pTaskInfo;
	bool m_bIsCurrentTaskInfo;
	

protected:    //used by QCSMtpThread and possibly other derived threads
	bool m_bStopThread;
	//bool m_bUsesNetwork;  // All threads will initially use Network 
						  // but may not always be the case.
};

#endif

/////////////////////////////////////////////////////////////////////////////
