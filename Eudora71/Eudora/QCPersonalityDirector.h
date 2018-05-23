// QCPersonalityDirector.h: interface for the QCPersonalityDirector class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __QCRECIPIENTDIRECTOR_H__
#define __QCRECIPIENTDIRECTOR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"

class QCPersonalityCommand;

class QCPersonalityDirector : public QCCommandDirector  
{
	CPtrList	m_thePersonalityList;

	void	Cleanup();

public:
	QCPersonalityDirector();
	virtual ~QCPersonalityDirector();

	QCPersonalityCommand*	AddCommand( 
	LPCSTR				szName );
	
	BOOL	Build();

	QCPersonalityCommand*	Find(
	LPCSTR	szName );

	void	DeleteCommand( 
	LPCSTR				szName );

	void	NewMessageCommands( 
	COMMAND_ACTION_TYPE	theAction, 
	CMenu*				pPopupMenu );

	POSITION GetFirstPersonalityPosition();

	QCPersonalityCommand* GetNextPersonality(POSITION& pos);
};

#endif // __QCRECIPIENTDIRECTOR_H__
