// QCStationeryDirector.h: interface for the QCStationeryDirector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCSTATIONERYDIRECTOR_H__3068C5AA_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCSTATIONERYDIRECTOR_H__3068C5AA_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandDirector.h"

class QCStationeryCommand;
extern enum COMMAND_ACTION_TYPE;

class QCStationeryDirector : public QCCommandDirector  
{
	CPtrList	m_theStationeryList;

	POSITION Insert(
	QCStationeryCommand*	pNewCommand );
protected:

	friend class QCStationeryCommand;

	void	NotifyClients(
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );


public:
	QCStationeryDirector();
	virtual ~QCStationeryDirector();

	QCStationeryCommand*	AddCommand(
	LPCSTR	szName );
	
	BOOL	Build();
	
	QCStationeryCommand*	Find(
	LPCSTR	szName );

	QCStationeryCommand*	FindByPathname(
	LPCSTR	szName );

	void FillListBox(
	CListBox*	pLB );

	void FillComboBox(
	CComboBox*	pCB );
	
	POSITION GetFirstStationeryPosition();

	QCStationeryCommand* GetNext(
	POSITION& pos );

	void	NewMessageCommands( 
	COMMAND_ACTION_TYPE	theAction,
	CMenu*				pPopupMenu );

	void GenerateNewName(CString strName, char* szNewName);
};

#endif // !defined(AFX_QCSTATIONERYDIRECTOR_H__3068C5AA_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
