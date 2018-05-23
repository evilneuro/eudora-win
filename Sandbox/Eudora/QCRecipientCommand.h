// QCRecipientCommand.h: interface for the QCRecipientCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCRECIPIENTCOMMAND_H__38C33471_B67A_11D0_97BD_00805FD2F268__INCLUDED_)
#define AFX_QCRECIPIENTCOMMAND_H__38C33471_B67A_11D0_97BD_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"

class QCRecipientDirector;
extern enum COMMAND_ACTION_TYPE;


class QCRecipientCommand : public QCCommandObject  
{
	CString	m_szNickName;

public:
	QCRecipientCommand(
	QCRecipientDirector*	pDirector,
	LPCSTR					szText );

	virtual ~QCRecipientCommand();

	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	const CString&	GetName() { return m_szNickName; }
	
	virtual UINT	GetFlyByID(
	COMMAND_ACTION_TYPE	theAction );

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction );

	virtual void InsertRecipient( CString Name );

	DECLARE_DYNAMIC(QCRecipientCommand)
};

#endif // !defined(AFX_QCRECIPIENTCOMMAND_H__38C33471_B67A_11D0_97BD_00805FD2F268__INCLUDED_)
