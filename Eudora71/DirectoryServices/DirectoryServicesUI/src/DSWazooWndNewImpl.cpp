//////////////////////////////////////////////////////////////////////////////
// DSWazooWndNewImpl.cpp
// 
// Created: 10/09/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#define __DIRECTORYSERVICES_WAZOOWNDNEW_INTERFACE_IMPLEMENTATION_
#include "DSWazooWndNewImpl.h"

#ifndef BASED_CODE
#define BASED_CODE
#endif

#include "DebugNewHelpers.h"

IMPLEMENT_DYNCREATE(DirectoryServicesWazooWndNew, CWazooWnd)

BEGIN_MESSAGE_MAP(DirectoryServicesWazooWndNew, CWazooWnd)
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
	if (m_pDirectoryServicesViewNew)
		m_pDirectoryServicesViewNew->PrepareForDestruction();

	return CWnd::DestroyWindow();
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

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to right hand Directory Services view.
////////////////////////////////////////////////////////////////////////
void 
DirectoryServicesWazooWndNew::OnActivateWazoo()
{
	CreateView();

	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pDirectoryServicesViewNew)
			m_pDirectoryServicesViewNew->SendMessage(umsgActivateWazoo);
	}
}


BOOL
DirectoryServicesWazooWndNew::CreateView() 
{
	if (!m_pDirectoryServicesViewNew)
	{
		m_pDirectoryServicesViewNew = DEBUG_NEW_MFCOBJ_NOTHROW DirectoryServicesViewNew;

		if (!m_pDirectoryServicesViewNew)
			return FALSE;

		// For some reason, MFC is really insistent that this CView class
		// get created as a Child window.
		BOOL fRet = m_pDirectoryServicesViewNew->Create(NULL, NULL, WS_CHILD | WS_VISIBLE,
							  CRect(0,0,0,0), this, UINT(-1), NULL);
		if (!fRet)
		{
			delete m_pDirectoryServicesViewNew;
			return FALSE;
		}

		CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();
		ASSERT_KINDOF(CMainFrame, pMainWnd);
		ASSERT(NULL == pMainWnd->m_pDirectoryServicesWazooWndNew);
		pMainWnd->m_pDirectoryServicesWazooWndNew = this;

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// DirectoryServicesWazooWndNew message handlers

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void
DirectoryServicesWazooWndNew::OnSize(UINT nType, int cx, int cy) 
{
	if (m_pDirectoryServicesViewNew)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		m_pDirectoryServicesViewNew->SetWindowPos(NULL,
							  rectClient.left,
							  rectClient.top,
							  rectClient.Width(),
							  rectClient.Height(),
							  SWP_NOZORDER | 
							  SWP_NOACTIVATE |
							  SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}

bool
DirectoryServicesWazooWndNew::GetSelectedAddresses(CStringList& addressList)
{
	if (m_pDirectoryServicesViewNew)
		return m_pDirectoryServicesViewNew->GetSelectedAddresses(addressList);

	return false;
}

BOOL
DirectoryServicesWazooWndNew::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg)
{
	if (m_pDirectoryServicesViewNew && nHitTest == HTCLIENT)
	{
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

	if (m_pDirectoryServicesViewNew)
		retVal = m_pDirectoryServicesViewNew->IsPrintPreviewMode();

	return retVal;
}

void
DirectoryServicesWazooWndNew::CheckClosePrintPreview()
{
	if (m_pDirectoryServicesViewNew)
		m_pDirectoryServicesViewNew->CheckClosePrintPreview();
}

void
DirectoryServicesWazooWndNew::ResetResize()
{
	if (m_pDirectoryServicesViewNew)
		m_pDirectoryServicesViewNew->ResetResize();
}

void
DirectoryServicesWazooWndNew::DoQuery(LPCTSTR QueryString)
{
	if (m_pDirectoryServicesViewNew)
		m_pDirectoryServicesViewNew->DoQuery(QueryString);
}

void
DirectoryServicesWazooWndNew::GiveQueryInputFocus()
{
	if (m_pDirectoryServicesViewNew)
		m_pDirectoryServicesViewNew->GiveQueryInputFocus();
}
