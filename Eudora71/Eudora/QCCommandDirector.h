// QCCommandDirector.h: interface for the QCCommandDirector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCOMMANDDIRECTOR_H__3068C5A3_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCCOMMANDDIRECTOR_H__3068C5A3_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class QCCommandObject;
class QICommandClient;

extern enum COMMAND_ACTION_TYPE;

class QCCommandDirector  
{
	CPtrList	m_theClientList;
	
protected:
	
	friend	QCCommandObject;

public:
	QCCommandDirector();
	virtual ~QCCommandDirector();

	virtual void	NotifyClients(
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	uAction,
	void*				pData = NULL );

	virtual void RemoveMessageCommands( 
	COMMAND_ACTION_TYPE	theAction,
	CMenu*				pPopupMenu );

	virtual	void Register(
	QICommandClient*	pClient );

	virtual void UnRegister(
	QICommandClient*	pClient );

};

#endif // !defined(AFX_QCCOMMANDDIRECTOR_H__3068C5A3_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
