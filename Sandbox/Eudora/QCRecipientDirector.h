// QCRecipientDirector.h: interface for the QCRecipientDirector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCRECIPIENTDIRECTOR_H__FB6756E2_B5AA_11D0_97BC_00805FD2F268__INCLUDED_)
#define AFX_QCRECIPIENTDIRECTOR_H__FB6756E2_B5AA_11D0_97BC_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"

class QCRecipientCommand;

class QCRecipientDirector : public QCCommandDirector  
{
	CPtrList	m_theRecipientList;

	POSITION Insert(
	LPCSTR szText);

	BOOL Write();

public:
	QCRecipientDirector();
	virtual ~QCRecipientDirector();

	QCRecipientCommand*	AddCommand( 
	LPCSTR				szName );
	
	void	DeleteCommand( 
	LPCSTR				szName );

	BOOL	Build();
	
	QCRecipientCommand*	Find(
	LPCSTR	szNickName );

	void	NewMessageCommands( 
	COMMAND_ACTION_TYPE	theAction, 
	CMenu*				pPopupMenu );

	virtual void	NotifyClients(
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	uAction,
	void*				pData = NULL );

	POSITION GetFirstRecipientPosition();

	QCRecipientCommand* GetNextRecipient(
	POSITION& pos);

};

#endif // !defined(AFX_QCRECIPIENTDIRECTOR_H__FB6756E2_B5AA_11D0_97BC_00805FD2F268__INCLUDED_)
