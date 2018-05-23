// QCPluginCommand.h: interface for the QCPluginCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCPLUGINCOMMAND_H__7BB106C2_C6FC_11D0_97D1_00805FD2F268__INCLUDED_)
#define AFX_QCPLUGINCOMMAND_H__7BB106C2_C6FC_11D0_97D1_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"

class QCPluginDirector;
class QCProtocol;
class CSpecial;

extern enum COMMAND_ACTION_TYPE;


struct TRANSLATE_DATA
{
	QCProtocol*	m_pProtocol;
	CView*		m_pView;
	BOOL		m_bBuildAddresses;

	TRANSLATE_DATA() { m_pProtocol = NULL; m_pView = NULL; m_bBuildAddresses = FALSE; }
};


struct TOGGLE_DATA
{
	CMenu*		m_pMenu;
	UINT		m_uID;
	UINT		m_uFlags;
	CString*	m_pszProperties;
	BOOL		m_ForceCheck;

	TOGGLE_DATA() { m_pMenu = NULL; m_uID = m_uFlags = 0; m_pszProperties = NULL; m_ForceCheck = FALSE; }
};

class QCPluginCommand : public QCCommandObject  
{
	void*				m_pPluginObject;
	CString				m_szName;
	COMMAND_ACTION_TYPE	m_theType;
public:
	int					m_ImageOffset;

private:

	bool DrawNeedsFullFeatureSetBitmap(bool bSelected, CPoint & ptDrawHere, CDC & memDC, CDC * pdcDraw);
	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct);
	void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );

public:
	
	QCPluginCommand(
	QCPluginDirector*	pDirector,
	void*				pPluginObject,
	LPCSTR				szName,
	COMMAND_ACTION_TYPE	theType	);
	
	virtual ~QCPluginCommand();

	virtual void	Execute(
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	const CString&	GetName() { return m_szName; }
	COMMAND_ACTION_TYPE	GetType() { return m_theType; }

	void GetTransmissionID( 
	LONG*	plModuleID, 
	LONG*	plTranslatorID);

	CSpecial *		GetSpecialObject() const;

	virtual CString	GetToolTip(
	COMMAND_ACTION_TYPE theAction );

	BOOL GetIcon(HICON &icn);
	BOOL InYourFace();
	BOOL IsDefaultOn();
	BOOL IsOnCompletion();


DECLARE_DYNAMIC( QCPluginCommand )
};

#endif // !defined(AFX_QCPLUGINCOMMAND_H__7BB106C2_C6FC_11D0_97D1_00805FD2F268__INCLUDED_)
