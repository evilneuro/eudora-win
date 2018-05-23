// QCStationeryCommand.h: interface for the QCStationeryCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCSTATIONERYCOMMAND_H__3068C5AB_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCSTATIONERYCOMMAND_H__3068C5AB_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"

class QCStationeryDirector;
extern enum COMMAND_ACTION_TYPE;

class QCStationeryCommand : public QCCommandObject  
{
	CString	m_szName;
	CString	m_szPathname;

public:
	
	QCStationeryCommand(
	QCStationeryDirector*	pDirector,
	LPCSTR					szName,
	LPCSTR					szPathname );

	virtual ~QCStationeryCommand();
	
	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	const CString&	GetName() const { return m_szName; }
	const CString&	GetPathname() const { return m_szPathname; }

	virtual UINT	GetFlyByID(
	COMMAND_ACTION_TYPE	theAction );

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction );

	DECLARE_DYNAMIC(QCStationeryCommand)
};

#endif // !defined(AFX_QCSTATIONERYCOMMAND_H__3068C5AB_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
