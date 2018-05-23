//////////////////////////////////////////////////////////////////////////////
// DirectoryServicesWazooWndNewImpl.cpp
// 
// Created: 10/09/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DIRECTORYSERVICES_WAZOOWNDNEW_INTERFACE_IMPLEMENTATION_
#include "DirectoryServicesWazooWndNewImpl.h"

#ifndef BASED_CODE
#define BASED_CODE
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(DirectoryServicesWazooWndNew, CWazooWnd)

BEGIN_MESSAGE_MAP(DirectoryServicesWazooWndNew, CWazooWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_SETCURSOR()
END_MESSAGE_MAP()

DirectoryServicesWazooWndNew::DirectoryServicesWazooWndNew() : CWazooWnd(IDR_DIRECTORY_SERVICES),
	m_pDirectoryServicesViewNew(NULL)
{
    m_hArrow      = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
    m_hArrowHour  = AfxGetApp()->LoadStandardCursor(IDC_APPSTARTING);
}

DirectoryServicesWazooWndNew::~DirectoryServicesWazooWndNew()
{
    m_pDirectoryServicesViewNew = NULL; //Deleted in CView::OnPostNcDestroy()
}

BOOL
DirectoryServicesWazooWndNew::DestroyWindow()
{
    ASSERT(m_pDirectoryServicesViewNew);
    m_pDirectoryServicesViewNew->PrepareForDestruction();
    return(CWnd::DestroyWindow());
}
     
////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to change the standard command routing to forward commands from the
// Wazoo control bar all the way to this view.
////////////////////////////////////////////////////////////////////////
BOOL
DirectoryServicesWazooWndNew::OnCmdMsg(UINT nID, int nCode, void* pExtra,
				       AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (m_pDirectoryServicesViewNew) {
	return(m_pDirectoryServicesViewNew->OnCmdMsg(nID, nCode, pExtra,
						     pHandlerInfo));
    }

    ASSERT(0);
    return(FALSE);
}

BOOL
DirectoryServicesWazooWndNew::OnCreateClient(LPCREATESTRUCT lpcs,
					     CCreateContext *pContext)
{
    ASSERT(m_pDirectoryServicesViewNew);
    return(TRUE);
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to right hand Directory Services view.
////////////////////////////////////////////////////////////////////////
void 
DirectoryServicesWazooWndNew::OnActivateWazoo()
{
    CControlBar* pParentControlBar = GetParentControlBar();

    if (pParentControlBar && pParentControlBar->IsVisible())
    {
        if (m_pDirectoryServicesViewNew)
            m_pDirectoryServicesViewNew->SendMessage(umsgActivateWazoo);
    }
}


/////////////////////////////////////////////////////////////////////////////
// DirectoryServicesWazooWndNew message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int
DirectoryServicesWazooWndNew::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CWazooWnd::OnCreate(lpCreateStruct) == -1) {
	return(-1);
    }

    ASSERT(NULL == m_pDirectoryServicesViewNew);
    m_pDirectoryServicesViewNew = new DirectoryServicesViewNew;
    fBaptizeBlockMT(m_pDirectoryServicesViewNew, "DSUI-View");
    fNoteBlockOriginMT(m_pDirectoryServicesViewNew, THIS_FILE, __LINE__ - 2);

    if (m_pDirectoryServicesViewNew == NULL) {
	return(-1);
    }

    // For some reason, MFC is really insistent that this CView class
    // get created as a Child window.
    BOOL fRet
	= m_pDirectoryServicesViewNew->Create(NULL, NULL,
					      WS_CHILD | WS_VISIBLE,
					      CRect(0,0,0,0), this,
					      UINT(-1),  NULL);
    if (fRet == FALSE) {
	delete m_pDirectoryServicesViewNew;
	return(-1);
    }
    else {
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMainFrame, pMainWnd);
	ASSERT(NULL == pMainWnd->m_pDirectoryServicesWazooWndNew);
	pMainWnd->m_pDirectoryServicesWazooWndNew = this;
	return(0);
    }
}

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void
DirectoryServicesWazooWndNew::OnSize(UINT nType, int cx, int cy) 
{
    CRect rectClient;
    GetClientRect(rectClient);

    ASSERT(m_pDirectoryServicesViewNew);
    m_pDirectoryServicesViewNew->SetWindowPos(NULL,
					      rectClient.left,
					      rectClient.top,
					      rectClient.Width(),
					      rectClient.Height(),
					      SWP_NOZORDER | 
					      SWP_NOACTIVATE |
					      SWP_NOREDRAW);
    CWazooWnd::OnSize(nType, cx, cy);
}

bool
DirectoryServicesWazooWndNew::GetSelectedAddresses(CStringList& addressList)
{
    ASSERT(m_pDirectoryServicesViewNew);
    return(m_pDirectoryServicesViewNew->GetSelectedAddresses(addressList));
}

BOOL
DirectoryServicesWazooWndNew::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg)
{
    if (nHitTest == HTCLIENT) {
	ASSERT(m_pDirectoryServicesViewNew);
	::SetCursor((m_pDirectoryServicesViewNew->IsQueryInProgress()) ?
		    m_hArrowHour : m_hArrow);

	return(TRUE);
    }
    return(CWnd::OnSetCursor(pWnd, nHitTest, msg));
}

bool
DirectoryServicesWazooWndNew::IsPrintPreviewMode()
{
    bool retVal = false;

    ASSERT(m_pDirectoryServicesViewNew);
    if (m_pDirectoryServicesViewNew) {
	retVal = m_pDirectoryServicesViewNew->IsPrintPreviewMode();
    }

    return(retVal);
}

void
DirectoryServicesWazooWndNew::CheckClosePrintPreview()
{
    ASSERT(m_pDirectoryServicesViewNew);
    if (m_pDirectoryServicesViewNew) {
	m_pDirectoryServicesViewNew->CheckClosePrintPreview();
    }
}

void
DirectoryServicesWazooWndNew::ResetResize()
{
    ASSERT(m_pDirectoryServicesViewNew);
    if (m_pDirectoryServicesViewNew) {
	m_pDirectoryServicesViewNew->ResetResize();
    }
}
