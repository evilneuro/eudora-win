// ContainerView.cpp : implementation file
//
// A CView-dervied class that acts as a simple container.
// Helpful for when you need a CView-wrapper around one
// or more controls but don't want the baggage of CFormView.
//

#include "stdafx.h"

#include "ContainerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CContainerView

IMPLEMENT_DYNCREATE(CContainerView, CView)

CContainerView::CContainerView() :
	m_hwndFocus(NULL)
{
}

CContainerView::~CContainerView()
{
}

BOOL CContainerView::Create(CWnd* pParentWnd)
{
	return CWnd::Create(NULL, NULL, WS_VISIBLE, CRect(0,0,0,0), pParentWnd, AFX_IDW_PANE_FIRST);
}


BEGIN_MESSAGE_MAP(CContainerView, CView)
	//{{AFX_MSG_MAP(CContainerView)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CContainerView drawing

void CContainerView::OnDraw(CDC* pDC)
{
	ASSERT(FALSE);
}


/////////////////////////////////////////////////////////////////////////////
// CContainerView diagnostics

#ifdef _DEBUG
void CContainerView::AssertValid() const
{
	CView::AssertValid();
}

void CContainerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CContainerView message handlers

void CContainerView::OnPaint()
{
	// this is done to avoid CView::OnPaint
	Default();
}

void CContainerView::OnSetFocus(CWnd* pOldWnd)
{
	if (!m_hwndFocus || !::IsWindow(m_hwndFocus) || !::IsChild(m_hWnd, m_hwndFocus))
		m_hwndFocus = ::GetWindow(m_hWnd, GW_CHILD);

	::SetFocus(m_hwndFocus);
}

void CContainerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// save focus window if focus is on this window's controls
	HWND hwndFocus = ::GetFocus();
	if (hwndFocus != NULL && ::IsChild(m_hWnd, hwndFocus))
	{
		m_hwndFocus = hwndFocus;
		return;
	}

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
