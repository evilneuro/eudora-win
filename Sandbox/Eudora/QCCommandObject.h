// QCCommandObject.h: interface for the QCCommandObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCOMMANDOBJECT_H__3068C5A1_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCCOMMANDOBJECT_H__3068C5A1_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class QCCommandDirector;

extern	enum COMMAND_ACTION_TYPE;

class QCCommandObject  : public CObject
{
	
protected:
	
	QCCommandDirector*	m_pDirector;
	
	QCCommandObject(
	QCCommandDirector*	pDirector );
	
	virtual ~QCCommandObject();

	virtual void NotifyDirector( 
	COMMAND_ACTION_TYPE theAction,
	void*				pData = NULL );

public:
		
	DECLARE_DYNAMIC(QCCommandObject)

	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL ) = 0;

	virtual UINT	GetFlyByID(
	COMMAND_ACTION_TYPE	theAction);

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction );

};

#endif // !defined(AFX_QCCOMMANDOBJECT_H__3068C5A1_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
