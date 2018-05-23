// TBarMenuButton.cpp: implementation of the CTBarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "TBarMenuButton.h"

#include "DebugNewHelpers.h"


IMPLEMENT_BUTTON(CTBarMenuButton);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTBarMenuButton::CTBarMenuButton()
{
	m_hMenu = NULL;
}

CTBarMenuButton::~CTBarMenuButton()
{

}

void CTBarMenuButton::SetHMenu( HMENU hMenu )
{
	m_hMenu = hMenu;
}


BOOL CTBarMenuButton::BtnPressDown(CPoint point)
{
	POINT			thePoint;

	if( !SECStdBtn::BtnPressDown( point ) )
	{
		return FALSE;
	}

	if( m_hMenu == NULL )
	{
		return TRUE;
	}

	thePoint.x = m_x;
	thePoint.y = m_y + m_cy;
	
	m_pToolBar->ClientToScreen( &thePoint );

	::TrackPopupMenu( m_hMenu, TPM_LEFTALIGN, thePoint.x, thePoint.y, 0, AfxGetMainWnd()->GetSafeHwnd(), NULL );//m_pToolBar->GetParentFrame()->GetSafeHwnd(), NULL );

	SECStdBtn::BtnPressUp( point );

	return TRUE;
}
