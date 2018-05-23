// QCChildToolBar.cpp: implementation of the QCChildToolBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "rs.h"
#include "resource.h"
#include "QCChildToolBar.h"
#include "QCToolBarManager.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack			g_theCommandStack;
extern QCPluginDirector			g_thePluginDirector;

class CDontFloatDockContext : public SECDockContext
{
public:
	CDontFloatDockContext(CControlBar* pBar) : SECDockContext(pBar) {}

	virtual void StartDrag(CPoint pt);
};

void CDontFloatDockContext::StartDrag( CPoint )
{
}

IMPLEMENT_DYNCREATE(QCChildToolBar, SECCustomToolBar )

BEGIN_MESSAGE_MAP(QCChildToolBar, SECCustomToolBar)
	//{{AFX_MSG_MAP(QCChildToolBar)
	ON_MESSAGE(WM_USER_QUERY_PALETTE, OnMyQueryNewPalette)
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCChildToolBar::QCChildToolBar()
{

}

QCChildToolBar::~QCChildToolBar()
{

}

BOOL QCChildToolBar::AcceptDrop() const
{
	return FALSE;
}


BOOL QCChildToolBar::InitButtons()
{
//	for( i = 0; i < m_btns.GetCount(); i++ )
//	{
//		if( m_btns[i].m_nID == 
//
//		if( pBtn->IsKindOf
//	}
	return TRUE;
}

void QCChildToolBar::EnableDocking(DWORD dwDockStyle)
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
    ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

    m_dwDockStyle = dwDockStyle;
    if (m_pDockContext == NULL)
	m_pDockContext = new CDontFloatDockContext(this);
	
    // permanently wire the bar's owner to its current parent
	if (m_hWndOwner == NULL)
	m_hWndOwner = ::GetParent(m_hWnd);
}


void* QCChildToolBar::GetButton(
INT iIndex )
{
	return ( void* )( m_btns[ iIndex ] );
}



BOOL QCChildToolBar::GetDragMode() const
{
	// We are in Drag 'Add' mode if the CONTROL key is held down at the start
	// of the drag
	return FALSE;
}

BOOL QCChildToolBar::DragButton(int nIndex, CPoint point)
{
	m_bAltDrag = FALSE;
	return FALSE;
}


void QCChildToolBar::OnLButtonDblClk(UINT nFlags, CPoint pt)
{
	// ignore double click if inside our "gripper buttons"
	if( m_dwExStyle & CBRS_EX_GRIPPER_CLOSE) {
	   if(m_rcGripperCloseButton.PtInRect(pt))
	      return;
	   }
	if( m_dwExStyle & CBRS_EX_GRIPPER_EXPAND) {
	   if(m_rcGripperExpandButton.PtInRect(pt))
	      return;
	   }	

	CWnd::OnLButtonDblClk(nFlags, pt);
}


void QCChildToolBar::AddTranslatorButtons(int &startPos)
{
	int					err				= 0;
	LONG				lType;
	QCPluginCommand*	pCommand		= NULL;
	BOOL				ShouldIAddEm	= TRUE;
	long				lastModID		= 0;


	POSITION pos = g_thePluginDirector.GetFirstPluginPosition();
	while( pos != NULL )
	{
		pCommand = g_thePluginDirector.GetNextPlugin( pos );

		switch(pCommand->GetType())
		{
			case CA_TRANSMISSION_PLUGIN:
				lType = ID_TRANSLATORS_QUEUE;
				break;
			default:
				continue;
		}
	
		// okay, add it in
		UINT uCommandID = g_theCommandStack.FindCommandID( pCommand, pCommand->GetType() );

		if( uCommandID == 0 )
		{
			continue;
		}
	
		int imageOffset = ((QCPluginCommand *)pCommand)->m_ImageOffset;
		BOOL isOnCompletion = ((QCPluginCommand *)pCommand)->IsOnCompletion();
		long modID = 0, trID = 0;
		((QCPluginCommand *)pCommand)->GetTransmissionID(&modID, &trID);
		
		CImageList *pImageList = &g_thePluginDirector.m_ImageList;
		( ( QCToolBarManagerWithBM* ) m_pManager )->CopyButtonImage( pImageList, imageOffset, uCommandID,  isOnCompletion);
	
		if (ShouldIAddEm)
		{
			// Separator before first one
			AddButton( startPos, 0, TRUE, TRUE );
			ShouldIAddEm = FALSE;
		}
		else
		{
			// Separator between groups
			if (modID != lastModID)
				AddButton( startPos++, 0, TRUE, TRUE );
		}

		AddButton( startPos, uCommandID, FALSE, TRUE );
		SetButtonStyle( startPos, TBBS_CHECKBOX );

		m_btns[ startPos++ ]->m_ulData = lType;
		
		lastModID = modID;
	}
}


BOOL QCChildToolBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction;
	INT					iCurrentButton;
	WORD				wCommandID;
	CString				szToolTip;

	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*) lParam;

	switch (pNMHDR->code)
	{
	case TTN_NEEDTEXTA:
	case TTN_NEEDTEXTW:
		{
			//
			// The toolbar button command id is stuffed into the 'idFrom' field.
			//
			wCommandID = ( WORD ) pNMHDR->idFrom;
			ASSERT( wCommandID != ID_SEPARATOR );
			for( iCurrentButton = GetBtnCount() - 1; iCurrentButton >= 0; iCurrentButton-- )
			{
				if( m_btns[ iCurrentButton ]->m_nID == wCommandID )
				{
					break;
				}
			}

			if( iCurrentButton < 0 )
			{
				break;
			}

			SECStdBtn*	pButton = m_btns[ iCurrentButton ];

			if( ( m_btns[ iCurrentButton ]->m_ulData != 0 ) &&
				( m_btns[ iCurrentButton ]->m_nID != m_btns[ iCurrentButton ]->m_ulData ) )
			{
				if( g_theCommandStack.Lookup( wCommandID, &pObject, &theAction ) )
				{
					szToolTip = pObject->GetToolTip( theAction );				
					szToolTip = szToolTip.Left(79);		// truncate at 79 chars, if necessary

					if (TTN_NEEDTEXTW == pNMHDR->code)
					{
						TOOLTIPTEXTW *pTTT = (TOOLTIPTEXTW *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));

						USES_CONVERSION;
						wcscpy(pTTT->szText, A2W( szToolTip ) );
					}
					else
					{
						TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));
						strcpy(pTTT->szText, szToolTip);
					}
					return TRUE;
				}
			}
		}
		break;
		default:
		break;
	}
	
	return SECCustomToolBar::OnNotify( wParam, lParam, pResult );
}


BOOL QCChildToolBar::CreateEx(DWORD dwExStyle, CWnd* pParentWnd, 
								DWORD dwStyle, UINT nID, LPCTSTR lpszTitle)
{
	
	m_cyTopBorder    = 1;
	m_cyBottomBorder = 1;
	return SECCustomToolBar::CreateEx(dwExStyle, pParentWnd, dwStyle, nID, lpszTitle );
}

LRESULT QCChildToolBar::OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam)
{            
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void QCChildToolBar::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged((CWnd*)this, pFocusWnd);
}

void QCChildToolBar::OnSysColorChange()
{
	m_Palette.DoSysColorChange((CWnd*)this);
}
