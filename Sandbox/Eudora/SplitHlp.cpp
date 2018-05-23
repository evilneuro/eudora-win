// SplitHlp.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>

#include "SplitHlp.h"

/////////////////////////////////////////////////////////////////////////////
// CSplitterHelpWnd

IMPLEMENT_DYNCREATE(CSplitterHelpWnd, CSplitterWnd)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CSplitterHelpWnd::CSplitterHelpWnd()
{
}

CSplitterHelpWnd::~CSplitterHelpWnd()
{
}

BEGIN_MESSAGE_MAP(CSplitterHelpWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CSplitterHelpWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
END_MESSAGE_MAP()



////////////////////////////////////////////////////////////////////////
// GetActivePane [public, virtual]
//
// The CSplitterWnd base class implementation assumes that the
// the parent frame's "active view" is the active pane.  We extend that
// here so that the view containing the keyboard focus is considered
// the active pane, even if the frame doesn't think it is the active
// view.
//
// Blame the Wazoo architecture for this one, too....
////////////////////////////////////////////////////////////////////////
CWnd* CSplitterHelpWnd::GetActivePane(int* pRow, int* pCol)
{
	ASSERT_VALID(this);

	CWnd* pView = CSplitterWnd::GetActivePane(pRow, pCol);

	if (pView != NULL)
		return pView;

	//
	// Here's where the fun part begins...
	//
	CWnd* pFocusWnd = GetFocus();

	for (int nRow = 0; nRow < GetRowCount(); nRow++)
	{
		for (int nCol = 0; nCol < GetColumnCount(); nCol++)
		{
			pView = GetPane(nRow, nCol);
			ASSERT_KINDOF(CView, pView);
			if (pView->IsChild(pFocusWnd))
			{
				if (pRow)
					*pRow = nRow;
				if (pCol)
					*pCol = nCol;
				return pView;
			}
		}
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CSplitterHelpWnd message handlers

LRESULT CSplitterHelpWnd::OnHelpHitTest(WPARAM, LPARAM lParam)
{
	CWnd*	pChildView;
	CWnd*	pChildCtrl;
	POINT	thePoint;

	thePoint.x = lParam & 0xFFFF;
	thePoint.y = ( lParam >> 16 );

	pChildView = ChildWindowFromPoint( thePoint );

	if( pChildView )
	{
		// convert the point into client coordinates of the child view
		ClientToScreen( &thePoint );
		pChildView->ScreenToClient( &thePoint );

		// get child control, if any
		pChildCtrl = pChildView->ChildWindowFromPoint( thePoint );

		if ( pChildCtrl )
		{
			return ( pChildCtrl->GetDlgCtrlID() +  HID_BASE_COMMAND );
		}
	}
	
	return 0;
}

