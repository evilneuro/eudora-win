// QCPersonalityCommand.h: interface for the QCPersonalityCommand class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __QCPERSONALITYCOMMAND_H__
#define __QCPERSONALITYCOMMAND_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"

class QCPersonalityDirector;
extern enum COMMAND_ACTION_TYPE;


class QCPersonalityCommand : public QCCommandObject  
{
	CString m_szPersonality;

public:
	QCPersonalityCommand(
	QCPersonalityDirector*	pDirector,
	LPCSTR					szText );

	virtual ~QCPersonalityCommand();

	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	const CString&	GetName() { return m_szPersonality; }
	
	virtual UINT	GetFlyByID(
	COMMAND_ACTION_TYPE	theAction );

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction );

	DECLARE_DYNAMIC(QCPersonalityCommand)
};

#endif // __QCPERSONALITYCOMMAND_H__
