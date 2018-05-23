/*////////////////////////////////////////////////////////////////////////////

NAME:
	CCodecMenu - CCodecMenu Class for the purpose of displaying codec names
				 in the menu

FILE:		CPLUGIN.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	CCodecMenu simplifies adding menu items under our "Codec" submenu

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef __CODECMNU_H
#define __CODECMNU_H

class CCodecMenu
{
public:
// LIFECYCLE
	CCodecMenu(
		UINT menuPosStart = 0,		// Start position on the main menu
		UINT subMenuPosStart = 0,	// Start position on the submenu
		UINT menuStartId = 0,		// Starting menu ID #
		CWnd *pCWnd = NULL);		// Put the menu on this window 
	~CCodecMenu();
	// Copy ctor or assignment operator not allowed

//	OPERATIONS
	BOOL checkItem( UINT nPos, BOOL bEnable = TRUE );
	BOOL enableItem( UINT nPos, BOOL bEnable = TRUE );
	BOOL appendItem( LPCTSTR lpszNewItem );
	BOOL removeItem( LPCTSTR lpszItem );
	BOOL removeItem( UINT nPos );
	BOOL removeAll();

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	showDefault( BOOL bShow = TRUE );

//	ATTRIBUTES
	CMenu	m_menu;
	CMenu	m_submenu;
	CWnd	*mp_CWnd;
	int		mi_itemCount;			// # menu items we currently have inserted
	UINT	mi_menuPosStart;		// start position on the main menu
	UINT	mi_subMenuPosStart;		// start position on the submenu
	UINT	mi_menuStartId;			// Starting Menu ID #

// Do not allow copy ctor or assignment
	CCodecMenu( const CCodecMenu& );
	CCodecMenu& operator=( const CCodecMenu& );

};

#endif