// QCSignatureDirector.h: interface for the QCSignatureDirector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCSIGNATUREDIRECTOR_H__38565F34_B836_11D0_97BF_00805FD2F268__INCLUDED_)
#define AFX_QCSIGNATUREDIRECTOR_H__38565F34_B836_11D0_97BF_00805FD2F268__INCLUDED_

#include "QCCommandDirector.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class QCSignatureCommand;
extern enum COMMAND_ACTION_TYPE;

class QCSignatureDirector : public QCCommandDirector  
{
	CPtrList	m_theSignatureList;

	void MigrateSignatures( void );
	
	POSITION Insert(
	QCSignatureCommand*	pNewCommand );
protected:

	friend class QCSignatureCommand;

	void	NotifyClients(
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );


public:
	QCSignatureDirector();
	virtual ~QCSignatureDirector();

	QCSignatureCommand*	AddCommand(
	LPCSTR	szName );

	BOOL Build();

	void FillComboBox(
	CComboBox*	pCB );

	void FillListBox(
	CListBox*	pLB );

	QCSignatureCommand*	Find(
	LPCSTR szName );

	QCSignatureCommand*	FindByPathname(
	LPCSTR	szName );

	POSITION GetFirstSignaturePosition();

	QCSignatureCommand* GetNext(
	POSITION& pos );

	void	NewMessageCommands( 
	COMMAND_ACTION_TYPE	theAction,
	CMenu*				pPopupMenu );

};

#endif // !defined(AFX_QCSIGNATUREDIRECTOR_H__38565F34_B836_11D0_97BF_00805FD2F268__INCLUDED_)
