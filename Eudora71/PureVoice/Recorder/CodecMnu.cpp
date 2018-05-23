/*////////////////////////////////////////////////////////////////////////////

NAME:
	CCodecMenu - CCodecMenu Class implementation for the purpose of 
				 displaying codec names in the menu

FILE:		CODECMNU.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:
	CCodecMenu simplifies adding menu items under our "Codec" submenu

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

DEPENDENCIES:

FILES:
	STDAFX.H
	QVOICE.H
	CODECMNU.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "LsDefines.h"
#include "QVoice.h"

// LOCAL INCLUDES
#include "CodecMnu.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CCodecMenu::CCodecMenu(
	UINT menuPosStart,		// Start position on the main menu
	UINT subMenuPosStart,	// Start position on the submenu
	UINT menuStartId,		// Start menu ID #
	CWnd *pCWnd				// Put the menu on this window 
)
{
	if ( pCWnd ) mp_CWnd = pCWnd;
	else mp_CWnd = AfxGetMainWnd();
	m_menu.Attach((mp_CWnd->GetMenu())->GetSafeHmenu());

	mi_itemCount		= 0;
	mi_menuPosStart		= menuPosStart;
	mi_subMenuPosStart	= subMenuPosStart;
	mi_menuStartId		= menuStartId;

	// use the menu index 'mi_menuPosStart'
	m_submenu.Attach((m_menu.GetSubMenu(mi_menuPosStart))->GetSafeHmenu());
	
	showDefault();
}

CCodecMenu::~CCodecMenu()
{
	// make sure there is a menu to remove
	if ( ::IsMenu(m_menu.m_hMenu) )
	 {
		removeAll();
		showDefault( FALSE ); // get rid of default menu item too
		m_submenu.Detach();
		m_menu.Detach();
	 }
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CCodecMenu::checkItem( UINT nPos, BOOL bEnable )
{
	UINT	ret;
	UINT	nEnable;

	if ( bEnable )
		nEnable = MF_CHECKED;
	else
		nEnable = MF_UNCHECKED;

	ret = m_submenu.CheckMenuItem( nPos + mi_subMenuPosStart,
									nEnable | MF_BYPOSITION );
	mp_CWnd->SetMenu( &m_menu );

	return (ret != -1);
}

BOOL CCodecMenu::enableItem( UINT nPos, BOOL bEnable )
{
	UINT	ret;
	UINT	nEnable;

	if ( bEnable )
		nEnable = MF_ENABLED;
	else
		nEnable = MF_DISABLED | MF_GRAYED;

	ret = m_submenu.EnableMenuItem( nPos + mi_subMenuPosStart,
									nEnable | MF_BYPOSITION );
	mp_CWnd->SetMenu( &m_menu );

	return (ret != -1 );
}

BOOL CCodecMenu::appendItem( LPCTSTR lpszNewItem )
{
	BOOL ret;

	if ( mi_itemCount == 0 ) showDefault( FALSE );
	ret = m_submenu.InsertMenu( mi_subMenuPosStart + mi_itemCount,
		MF_BYPOSITION, mi_menuStartId + mi_itemCount, lpszNewItem );
	mp_CWnd->SetMenu( &m_menu );
	if (ret) mi_itemCount++;

	return ret;
}

BOOL CCodecMenu::removeItem( LPCTSTR lpszItem )
{
	char	lpName[80];
	BOOL	ret;
	int		nPos = mi_subMenuPosStart - 1;

	if ( mi_itemCount < 1 ) return FALSE;
	do {
		m_submenu.GetMenuString( ++nPos, lpName, 80, MF_BYPOSITION );
	} while (strcmp(lpName, lpszItem));
	ret = m_submenu.RemoveMenu( nPos, MF_BYPOSITION );
	mp_CWnd->SetMenu( &m_menu );
	if ( ret ) mi_itemCount--;
	if ( !mi_itemCount ) showDefault();

	return ret;
}

BOOL CCodecMenu::removeItem( UINT nPos )
{
	BOOL	ret;

	if ( mi_itemCount < 1 ) return FALSE;
	ret = m_submenu.RemoveMenu( nPos + mi_subMenuPosStart, MF_BYPOSITION );
	mp_CWnd->SetMenu( &m_menu );
	if ( ret ) mi_itemCount--;
	if ( !mi_itemCount ) showDefault();

	return ret;
}

BOOL CCodecMenu::removeAll()
{
	BOOL ret = TRUE;

	if ( mi_itemCount < 1 ) return TRUE;
	while ( mi_itemCount )
	 {
		mi_itemCount--;
		ret = m_submenu.RemoveMenu( mi_subMenuPosStart + mi_itemCount,
										MF_BYPOSITION );
	 }
	mp_CWnd->SetMenu( &m_menu );
	if ( mi_itemCount ) ret = FALSE;
	if ( !mi_itemCount ) showDefault();

	return ret;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOL CCodecMenu::showDefault( BOOL bShow /* = TRUE */ )
{
/*
	int err = m_menu.RemoveMenu( ID_CODEC_NONE, MF_BYCOMMAND );
	if ( bShow )
		err = m_submenu.InsertMenu( ID_CODEC_NONE, MF_BYCOMMAND,
			ID_CODEC_NONE, "Compression Plug-Ins Not Found" );
*/
	int err;
	if ( bShow )
	 {
		CString loadStr; loadStr.LoadString( LS_STR_NO_CODEC );
		err = m_submenu.InsertMenu( mi_subMenuPosStart, MF_BYPOSITION,
		0, loadStr );
	 }
	else
		err = m_submenu.RemoveMenu( mi_subMenuPosStart, MF_BYPOSITION );

	return err;
}