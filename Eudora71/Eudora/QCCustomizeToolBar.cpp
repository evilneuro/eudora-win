// QCCustomizeToolBar.cpp: implementation of the QCCustomizeToolBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizeToolBar.h"
#include "QCCustomToolBar.h"
#include "QCToolBarCmdPage.h"
#include "QCToolBarManager.h"
#include "QCCommandStack.h"
#include "mainfrm.h"
#include "NameToolBarDialog.h"

#include "DebugNewHelpers.h"

extern SEC_AUX_DATA secData;

IMPLEMENT_DYNAMIC(QCCustomizeToolBar, SECCustomizeToolBar);

BEGIN_MESSAGE_MAP(QCCustomizeToolBar, SECCustomizeToolBar)
	ON_MESSAGE(WM_USER_QUERY_PALETTE, OnMyQueryNewPalette)
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCCustomizeToolBar::QCCustomizeToolBar()
{

}

QCCustomizeToolBar::~QCCustomizeToolBar()
{
}


BOOL QCCustomizeToolBar::DragButton(int nIndex, CPoint point)
{
	QCToolBarCmdPage*	pParent;
	UINT				uNewID;
	SECStdBtn			theButton;		
	
	SetConfigFocus(nIndex, TRUE);

	BOOL bAdd = GetDragMode();

	// translate the id
	pParent = ( QCToolBarCmdPage* ) GetParent();		
	uNewID = pParent->TranslateID( ( m_btns[ m_nDown ] )->m_nID );
	if( uNewID == 0 )
	{
		return FALSE;
	}

	theButton.m_nID = uNewID;
	theButton.m_ulData = ( m_btns[ m_nDown ] )->m_nID;
	theButton.m_nImage = ( m_btns[ m_nDown ] )->m_nImage;
	theButton.m_nStyle = ( m_btns[ m_nDown ] )->m_nStyle;
	theButton.m_x = ( m_btns[ m_nDown ] )->m_x;
	theButton.m_y = ( m_btns[ m_nDown ] )->m_y;
	theButton.m_cx = ( m_btns[ m_nDown ] )->m_cx;
	theButton.m_cy = ( m_btns[ m_nDown ] )->m_cy;

	// Now start the drag ....
	CRect rect;
	GetItemRect(nIndex, rect);
	SECToolBarRectTracker tracker(rect);
	
	if(bAdd)
		::SetCursor(AfxGetApp()->LoadCursor(IDC_TOOLBAR_DRAGADD));

	if(tracker.Track(this, point))
	{
		// Drag successful
		ClientToScreen(&tracker.m_rect);
		rect = tracker.m_rect;

#ifdef _DEBUG
		if (m_pManager)
			ASSERT_KINDOF(SECToolBarManager, m_pManager);
#endif

		QCCustomToolBar* pToolBar;
		if( (m_pManager && 
			(pToolBar = ( QCCustomToolBar* )((SECToolBarManager*)m_pManager)->ToolBarUnderRect(rect)) != NULL) ||
		   (pToolBar = ( QCCustomToolBar* )ToolBarFromPoint(rect.left, rect.top)) != NULL ||
		   (pToolBar = ( QCCustomToolBar* )ToolBarFromPoint(rect.right, rect.top)) != NULL ||
		   (pToolBar = ( QCCustomToolBar* )ToolBarFromPoint(rect.left, rect.bottom)) != NULL ||
		   (pToolBar = ( QCCustomToolBar* )ToolBarFromPoint(rect.right, rect.bottom)) != NULL)
		{
			ASSERT_KINDOF( QCCustomToolBar, pToolBar );
			// Button was dropped over a toolbar.
			if(pToolBar->AcceptDrop())
			{
				// The toolbar wants the button, so drop it here
				pToolBar->ScreenToClient(&rect);
				pToolBar->DropButton(rect, &theButton, bAdd);
			}
			else if(!bAdd)
			{
				// We are not in add mode, so remove the button from the toolbar.
				int nBtn = m_nDown;
				RemoveConfigFocus();
				if(!RemoveButton(nBtn)) 
				{
					// Careful, Toolbar does not exist now.
					return FALSE;
				}
			}
		}
		else
		{
			// Button not dropped over a toolbar
			if(m_pManager != NULL)
			{
				CRect tempRect;
				((SECToolBarManager*)m_pManager)->GetNoDropRect(rect);
				if(tempRect.IntersectRect(rect, tracker.m_rect))
				{
					// It has been dropped over our no drop rect. So ignore drag
					return TRUE;
				}
			}

			if(bAdd) 
				// We're in add mode, so we can create a new button
				NewToolBar( tracker.m_rect.TopLeft(), theButton, TRUE );
			else
			{
				// Need in add mode, so remove the button
				int nBtn = m_nDown;
				RemoveConfigFocus();
				if(!RemoveButton(nBtn))
					// Careful, Toolbar does not exist now.
					return FALSE;
			}
		}
	}

	return TRUE;
}

LRESULT QCCustomizeToolBar::OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam)
{            
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void QCCustomizeToolBar::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged((CWnd*)this, pFocusWnd);
}

void QCCustomizeToolBar::OnSysColorChange()
{
	m_Palette.DoSysColorChange((CWnd*)this);
}


void QCCustomizeToolBar::NewToolBar(CPoint pt, SECStdBtn& btn, BOOL bPrompt )
{
	UINT				nID;
	UINT				u;
	CNameToolBarDialog	theDlg;

	// Create a new toolbar containing the given button.
	if(!m_pManager)
		return;

	ASSERT_KINDOF(QCToolBarManager, m_pManager);

	while( 1 )
	{
		if( theDlg.DoModal() != IDOK )
		{
			return;
		}

		theDlg.m_szName.TrimLeft();
		theDlg.m_szName.TrimRight();

		if ( theDlg.m_szName != "" ) 
		{
			break;
		}
	}

	nID = btn.m_nID;
	
	if( ( btn.m_ulData != 0 ) && ( btn.m_ulData != nID ) )
	{
		// see if this button already exists	
		for ( u = 0; u < (UINT)m_nBmpItems; u++ )
		{
			if( nID == m_pBmpItems[ u ] )
			{
				btn.m_nImage = u;
				break;
			}
		}

		if( u == (UINT) m_nBmpItems )
		{
			// the button doesn't exist

			// find the index of the button in the toolbar bitmap using the orignal id
			for ( u = 0; u < (UINT)m_nBmpItems; u++ )
			{
				if( btn.m_ulData == m_pBmpItems[ u ] )
				{
					break;
				}
			}

			// if found, add the button 
			if( u < (UINT) m_nBmpItems )
			{	
				( ( QCToolBarManager* ) m_pManager )->CopyButtonImage( u, nID );
//				( ( QCToolBarManager* ) m_pManager )->SetToolBarInfo( this );
			}
		}
	}
	
	SECCustomToolBar* pToolBar = ((QCToolBarManager*)m_pManager)->CreateUserToolBar( theDlg.m_szName );
	if(!pToolBar)
		return;

	// Add the button and give it the config focus.
	//	SECStdBtn* pBtn = CreateButton(btn.m_nID, pToolBar);
	//	pToolBar->m_btns.Add(pBtn);
	
	pToolBar->AddButton( 0, btn.m_nID, FALSE, TRUE );

	// This is critical for ToolBar to have the correct functionality for the button. After the call to AddButton, a new toolbar button is created, 
	// but button's m_ulData is set to zero. Because of that, Eudora thinks of it as a normal (as opposed to a custom) button
	// Hence causing VI#6129. 
	
	// Altho' pToolBar is a SECCustonToolBar, the below typecase should not hurt what we want to achieve. We set the Button's (whose
	// index would be zero, 'coz it's a new toolbar) m_ulData to the appropriate value rather than zero.
	((QCCustomToolBar *)pToolBar)->SetButtonData(btn.m_ulData);
	
	if(!m_bAltDrag)
		// Note that when perform an "ALT" drag operation, when the operation
		// is complete no button has the config focus since we are not in
		// customize mode.
		pToolBar->SetConfigFocus(0, TRUE);

	// We want to position the floating window such that the toolbar button
	// is in the exact place where it was dropped. We therefore need to
	// calc. the height of the frame so we can offset the frames position.
	CRect r(pt.x, pt.y, pt.x, pt.y);
	::AdjustWindowRectEx(r, WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME, FALSE,
						 WS_EX_TOOLWINDOW|WS_EX_WINDOWEDGE);
	pt.x = r.left - m_cxLeftBorder - secData.cxBorder2;
	pt.y = r.top  - m_cyTopBorder - secData.cyBorder2;

	CFrameWnd* pFrameWnd = (CFrameWnd*) GetOwner();
	ASSERT_KINDOF(CFrameWnd, pFrameWnd);
	if(!pFrameWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
		return;

	if (pFrameWnd->IsKindOf(RUNTIME_CLASS(SECMDIFrameWnd)))
		((SECMDIFrameWnd*)pFrameWnd)->FloatControlBar(pToolBar, pt);
	else if (pFrameWnd->IsKindOf(RUNTIME_CLASS(SECFrameWnd)))
		((SECFrameWnd*)pFrameWnd)->FloatControlBar(pToolBar, pt);
	else
		pFrameWnd->FloatControlBar(pToolBar, pt);
}
