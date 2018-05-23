/////////////////////////////////////////////////////////////////////////////
// File: MDIDRAG.CPP 
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "mdichild.h"
#include "mdidrag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// CMDIChild

IMPLEMENT_DYNCREATE(CMDIChildTarget, CMDIChild)

CMDIChildTarget::CMDIChildTarget()
{
}

CMDIChildTarget::~CMDIChildTarget()
{
}


BEGIN_MESSAGE_MAP(CMDIChildTarget, CMDIChild)
	//{{AFX_MSG_MAP(CMDIChildTarget)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

 

/////////////////////////////////////////////////////////////////////////////
// CMDIChildTarget message handlers

int CMDIChildTarget::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChild::OnCreate(lpCreateStruct) == -1)           
		return -1;                                      

	// Register the CMDIChildOleDropTarget object with the OLE dll's
	if (! m_dropTarget.Register(this))
	{
		ASSERT(0);
		return -1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
// Override COleDropTarget::OnDragEnter to work with CMDIChildTarget
// rather than CView.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMDIChildOleDropTarget::OnDragEnter(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildTarget)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}
	else if (pWnd->IsIconic())
	{
		// delegates to view contained in child window
		CMDIChildTarget* pChildWnd = (CMDIChildTarget *) pWnd;
		ASSERT_VALID(pChildWnd);
		return pChildWnd->GetActiveView()->OnDragEnter(pDataObject, dwKeyState, point);
	}
	else
		return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
// Override COleDropTarget::OnDragOver to work with CMDIChildTarget
// rather than CView.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMDIChildOleDropTarget::OnDragOver(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildTarget)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}
	else if (pWnd->IsIconic())
	{
		// delegates to view contained in child window
		CMDIChildTarget* pChildWnd = (CMDIChildTarget *)pWnd;
		ASSERT_VALID(pChildWnd);
		return pChildWnd->GetActiveView()->OnDragOver(pDataObject, dwKeyState, point);
	}
	else
		return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
// Override COleDropTarget::OnDrop to work with CMDIChildTarget
// rather than CView.
////////////////////////////////////////////////////////////////////////
BOOL CMDIChildOleDropTarget::OnDrop(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildTarget)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}
	else if (pWnd->IsIconic())
	{
		// delegates to the view enclosed in the child window
		CMDIChildTarget* pChildWnd = (CMDIChildTarget*) pWnd;
		ASSERT_VALID(pChildWnd);
		return pChildWnd->GetActiveView()->OnDrop(pDataObject, dropEffect, point);
	}
	else
		return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDropEx [public, virtual]
//
// Override COleDropTarget::OnDropEx to work with CMDIChildTarget
// rather than CView.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMDIChildOleDropTarget::OnDropEx(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropDefault, 
	DROPEFFECT dropList, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildTarget)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	//
	// Normally, this would delegate to the CMDIChildTarget::OnDropEx()
	// method.  However, since we're currently not doing anything like
	// right button drags, let's just return a "not implemented" status
	// code to force MFC to call the older OnDrop() method.
	//
	return DROPEFFECT(-1);
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
// Override COleDropTarget::OnDragLeave to work with CMDIChildTarget
// rather than CView.
////////////////////////////////////////////////////////////////////////
void CMDIChildOleDropTarget::OnDragLeave(CWnd* pWnd)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildTarget)))
	{
		ASSERT(0);
		return;
	}
	else if (pWnd->IsIconic())
	{
		// delegates to view contained in child window
		CMDIChildTarget* pChildWnd = (CMDIChildTarget *) pWnd;
		ASSERT_VALID(pChildWnd);
		pChildWnd->GetActiveView()->OnDragLeave();
	}
}
