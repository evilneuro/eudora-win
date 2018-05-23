// QCCommandStack.h: interface for the QCCommandStack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCOMMANDSTACK_H__3068C5AC_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCCOMMANDSTACK_H__3068C5AC_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define QC_FIRST_COMMAND_ID 0x8200
#define QC_LAST_COMMAND_ID 0xCFFF

class QCCommandObject;
extern enum COMMAND_ACTION_TYPE;


class QCCommandStack  
{
	class QCCommandMapEntry : public CObject
	{
	public:
		COMMAND_ACTION_TYPE	m_theAction;
		WORD				m_wCommandID;
		QCCommandObject*	m_pObject;
		UINT				m_uCount;		
	};


	class QCCommandMap
	{
		WORD			m_wLastCommand;
		CMapWordToPtr	m_theMap;
		CWordArray		m_theDeletedIDArray;
#ifdef OLDSTUFF		
		HMENU			m_hParentMenu;
		HMENU			m_hMenu;
		INT				m_iPopupPos;
#endif
	public:
		
		QCCommandMap(); 
#ifdef OLDSTUFF		
//		HMENU	hParentMenu,
//		INT		iPopupPos );
#endif
		virtual ~QCCommandMap();
		
		WORD Add( 
		QCCommandObject*	pObject,
		COMMAND_ACTION_TYPE	theAction);

		void Delete( 
		WORD				wID,
		COMMAND_ACTION_TYPE	theAction);

#ifdef OLDSTUFF		
		HMENU	GetParentHMenu() const { return m_hParentMenu; }
		
		HMENU	GetHMenu() const { return m_hMenu; }

		void	SetHMenu( HMENU hMenu ) { m_hMenu = hMenu; }

		INT		GetPopupPos() const { return m_iPopupPos; }
#endif
		WORD FindCommandID( 
		QCCommandObject*	pObject,
		COMMAND_ACTION_TYPE	theAction);

		BOOL	Lookup( 
		WORD					wCommandID,
		QCCommandObject**		ppObject,
		COMMAND_ACTION_TYPE*	pAction );

	};

	QCCommandMapEntry	m_theSelectedCommand;
	CPtrList			m_theStack;
		
public:

	QCCommandStack();
	virtual ~QCCommandStack();
	
	UINT	GetCount() const { return m_theStack.GetCount(); }	
	
#ifdef OLDSTUFF
	HMENU	GetTopHMenu();

	HMENU	GetTopParentHMenu();

	void	SetTopHMenu(
	HMENU	hMenu );

	HMENU	Find(
	HMENU	hParent,
	INT		iPopupPos);

	BOOL	FindChild(
	HMENU	hParent, 
	HMENU*	hChild );

	BOOL	IsParent(
	HMENU	hMenu,
	INT*	pPos );
#endif

	void	NewCommandMap(); 
#ifdef OLDSTUFF
//	HMENU	hParentMenu,
//	INT		uPos );
#endif

	void	Pop();
		
	WORD	AddCommand( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction );

	void	DeleteCommand( 
	WORD				wID,
	COMMAND_ACTION_TYPE	theAction );

	WORD FindCommandID( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction);

	BOOL	Lookup( 
	WORD					wCommandID,
	QCCommandObject**		ppObject,
	COMMAND_ACTION_TYPE*	pAction );

	void	SaveCommand(
	WORD	wCommandID );

	BOOL	GetCommand(	
	WORD					uCommandID,
	QCCommandObject**		ppObject,
	COMMAND_ACTION_TYPE*	pAction);

	void DeleteAllMenuCommands(
	CMenu*				pMenu,
	COMMAND_ACTION_TYPE	theAction);

	void GetSavedCommand(	
	WORD*					pCommandID,
	QCCommandObject**		ppObject,
	COMMAND_ACTION_TYPE*	pAction);

};


#endif // !defined(AFX_QCCOMMANDSTACK_H__3068C5AC_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
