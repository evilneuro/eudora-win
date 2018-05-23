#include "stdafx.h"
#include "eudora.h"
#include "MsgSplitter.h"
#include "rs.h"



#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// CMsgSplitterWnd:

BEGIN_MESSAGE_MAP(CMsgSplitterWnd, CSplitterHelpWnd)
	//{{AFX_MSG_MAP(CMsgSplitterWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CMsgSplitterWnd:
// Default constructor for CMsgSplitterWnd objects
CMsgSplitterWnd::CMsgSplitterWnd()
{
	m_pClass = NULL;
}


// ~CMsgSplitterWnd:
// Destructor for CMsgSplitterWnd objects
CMsgSplitterWnd::~CMsgSplitterWnd()
{
}


// OnLButtonDblClk:
// Handles user double clicking on the splitter bar
//
// We override this method to provide double-click behavior similar to that
// provided by dynamic splitters (i.e. opening/shutting a pane).

void CMsgSplitterWnd::OnLButtonDblClk( UINT nFlags, CPoint pt )
{
	// call the base implimentation
	CSplitterHelpWnd::OnLButtonDblClk( nFlags, pt );

	// make sure the event is meant for us
	int ptHit = HitTest( pt );

	// turns out that the hit-test value for our splitter bar is 101. This is
	// something that is not programatically available. You simply have to
	// dig into the CSplitterWnd code and find it. This, of course, means that
	// Microsoft can hose us any ol' time they see fit. Poop.
	if ( ptHit == 101 ) {
		// tell our parent frame to toggle the header
		GetParentFrame()->SendMessage(WM_COMMAND, IDA_TOGGLEHEADER);
	}
}

// OnMouseWheel
// Handles the case where the splitter window gets the mouse wheel message.
// This hapeens with IE 5.x when the message cannot scroll anymore (i.e. at
// top or bottom).  If you call the base class of this it will wind up
// ASSERTing because the panes that the splitter window sizes are not
// derived from CScrollView.  So we just eat the message.
BOOL CMsgSplitterWnd::OnMouseWheel(UINT, short, CPoint)
{
	return FALSE;
}

BOOL CMsgSplitterWnd::CreateView( int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext )
{
	if( m_pClass == NULL )
	{
		return CSplitterWnd::CreateView( row, col, pViewClass, sizeInit, pContext );
	}

	return CSplitterWnd::CreateView( row, col, m_pClass, sizeInit, pContext );
}


void CMsgSplitterWnd::SetViewType( 
CRuntimeClass* pClass )
{
	m_pClass = pClass;
}



//
// this was added so that the read only mode of the Blah Blah Blah 
// view would not be gray
//
HBRUSH CMsgSplitterWnd::OnCtlColor(
CDC*	pDC, 
CWnd*	pWnd, 
UINT	nCtlColor) 
{
	if( !GetIniShort(IDS_INI_CRAIG) && IsVersion4() )
	{
		// Do this so read only text is not gray but background color
		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
		pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		return (GetSysColorBrush(COLOR_WINDOW));
	}

	return CSplitterHelpWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

