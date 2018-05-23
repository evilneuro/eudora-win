// 3DFORMV.CPP
//
// Base class for CFormViews that need 3D look

#include "stdafx.h"

#include "helpcntx.h"
#include "resource.h"
#include "guiutils.h"
#include "3dformv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DFormView

IMPLEMENT_DYNAMIC(C3DFormView, CFormView)

C3DFormView::C3DFormView(UINT nID)
	: CFormView(nID)
{
}

C3DFormView::~C3DFormView()
{
}

void C3DFormView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
}


BEGIN_MESSAGE_MAP(C3DFormView, CFormView)
	//{{AFX_MSG_MAP(C3DFormView)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)  
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DFormView message handlers

LRESULT C3DFormView::OnContextMenu(WPARAM, LPARAM lParam)
{
	// Invoke help
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	CPoint point(x,y);

	CContextHelp( m_hWnd, point, FALSE);

	return TRUE;
}

LRESULT C3DFormView::OnHelpHitTest(WPARAM, LPARAM lParam)
{
	// Called when the user enters help mode and clicks in this window.
	// Returns the Help ID of the selected control.

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	CPoint point(x,y);

	CContextHelp( m_hWnd, point, TRUE);
		
	return -1; 

}
