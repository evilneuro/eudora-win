// QCSignatureCommand.h: interface for the QCSignatureCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCSIGNATURECOMMAND_H__38565F35_B836_11D0_97BF_00805FD2F268__INCLUDED_)
#define AFX_QCSIGNATURECOMMAND_H__38565F35_B836_11D0_97BF_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"

class QCSignatureDirector;
extern enum COMMAND_ACTION_TYPE;

class QCSignatureCommand : public QCCommandObject  
{
	CString	m_szName;
	CString	m_szPathname;

public:
	
	QCSignatureCommand(
	QCSignatureDirector*	pDirector,
	LPCSTR					szName,
	LPCSTR					szPathname );

	virtual ~QCSignatureCommand();
	
	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	const CString&	GetName() const { return m_szName; }
	const CString&	GetPathname() const { return m_szPathname; }

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction );

	DECLARE_DYNAMIC(QCSignatureCommand)
};

#endif // !defined(AFX_QCSIGNATURECOMMAND_H__38565F35_B836_11D0_97BF_00805FD2F268__INCLUDED_)
