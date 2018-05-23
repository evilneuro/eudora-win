// QCPluginDirector.h: interface for the QCPluginDirector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCPLUGINDIRECTOR_H__7BB106C3_C6FC_11D0_97D1_00805FD2F268__INCLUDED_)
#define AFX_QCPLUGINDIRECTOR_H__7BB106C3_C6FC_11D0_97D1_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"
#include "QCImageList.h"

class QCPluginCommand;

class QCPluginDirector : public QCCommandDirector  
{
	CPtrList	m_thePluginList;

public:
	QCPluginDirector();
	virtual ~QCPluginDirector();

	BOOL	Build();

	void	NewMessageCommands( 
	COMMAND_ACTION_TYPE	theAction, 
	CMenu*				pPopupMenu,
	INT					iPosition = -1 );

	QCPluginCommand*	Find( 
	LPCSTR				ssName,
	COMMAND_ACTION_TYPE	theType );

	POSITION GetFirstPluginPosition();

	QCPluginCommand* GetNextPlugin(
	POSITION& pos);

	int	GetCount();

public:	
	QCImageList m_ImageList;
};

#endif // !defined(AFX_QCPLUGINDIRECTOR_H__7BB106C3_C6FC_11D0_97D1_00805FD2F268__INCLUDED_)
