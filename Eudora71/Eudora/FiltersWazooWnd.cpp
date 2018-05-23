// FiltersWazooWnd.cpp : implementation file
//
// CFiltersWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "doc.h"
#include "resource.h"
#include "utils.h"
#include "summary.h"

#include "tocdoc.h"			// for FILTERSD.H
#include "3dformv.h"		// for FILTERSV.H
#include "controls.h"
#include "filtersd.h"		// for FILTERSV.H
#include "filtersv.h"
#include "FiltersWazooWnd.h"
#include "QCProtocol.h"
#include "QCFindMgr.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CFiltersWazooWnd

IMPLEMENT_DYNCREATE(CFiltersWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CFiltersWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CFiltersWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

END_MESSAGE_MAP()


CFiltersWazooWnd::CFiltersWazooWnd() : CWazooWnd(IDR_FILTERS)
{
}

CFiltersWazooWnd::~CFiltersWazooWnd()
{
}


////////////////////////////////////////////////////////////////////////
// DestroyWindow [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CFiltersWazooWnd::DestroyWindow() 
{
	if (m_wndSplitter.GetSafeHwnd())
	{
		int cxCur, cxMin;
		m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
		if (cxCur >= 0)
		{
			//
			// Empirically, we find that 'cxCur' is -1 if the filters
			// window has never been shown during a given Eudora
			// session.  Therefore, we only save the position if 
			// the window has been made visible at least once.
			//
			SetIniLong(IDS_INI_FILTERS_WINDOW_SPLITER, cxCur);
		}
	}
		
	return CWnd::DestroyWindow();
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to change the standard command routing to forward commands from the
// Wazoo control bar all the way to the active splitter pane view.
////////////////////////////////////////////////////////////////////////
BOOL CFiltersWazooWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{

	if (m_wndSplitter.GetSafeHwnd())
	{
		CWnd* pActivePane = m_wndSplitter.GetActivePane();
		if (pActivePane && pActivePane->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}
	
	return CWazooWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


////////////////////////////////////////////////////////////////////////
// OnDeactivateWazoo [public, virtual]
//
// Override for virtual base class method.  This gets called when the
// filters window tab is deactivated or when the Wazoo container is
// closed.
////////////////////////////////////////////////////////////////////////
void CFiltersWazooWnd::OnDeactivateWazoo()
{
	
	//ASSERT(::IsWindow(m_wndSplitter.GetSafeHwnd()));

	//
	// Give the parent implementation a chance to save the keyboard focus.
	//
	CWazooWnd::OnDeactivateWazoo();

	//
	// Notify the document that it has been deactivated so that it can
	// prompt the user for saving any unsaved changes.  FORNOW, this is
	// totally too late since by the time we get here, the deactivation
	// has already occurred.
	//
	GetFiltersDoc()->CanCloseFrame(NULL);
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Override for virtual base class method.  This gets called when the
// filters window tab is activated
////////////////////////////////////////////////////////////////////////
void CFiltersWazooWnd::OnActivateWazoo()
{
	static BOOL bFuncEntered = FALSE;
	
	// Do this only once, when it is called (i.e only first time). We create the view when the user actually wants to see the Filters Wazoo
	// or if the filters wazoo was left open when Eudora exited last time successfully. - Apul 01/31/2000

	if (bFuncEntered == FALSE)
	{
		// Make sure the parent window is present before we proceed .. OnActivateWazoo is also called under this scenario.
		// Say if you open the filters wazoo, then "close" it & proceed to exit from Eudora. So ideally the Filter Wazoo should not
		// be displayed the next time. Altho' we don't display the Filters wazoo when Eudora is executed next time, OnActivateWazoo function
		// is still called. To make sure that we create the views only & only when we are actually displaying the filters tab, we handle it this way
		// Could be still fine tuned .. Apul 01/31/2000

		if (GetParent())
		{
			CCreateContext myBogusContext;	
			if ((! m_wndSplitter.CreateStatic(this, 1, 2)) ||
				(! m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CFiltersViewLeft), CSize(130,130), &myBogusContext)) ||
				(! m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CFiltersViewRight), CSize(0,0), &myBogusContext)))
			{
				ASSERT(0);
				return ; //-1;
			}

			CRect rectClient;
			// I know i could have just taken the ClientRect for myself(i.e 'this'), but then that would be (0,0,0,0) when 'this' is in the process of creation
			::GetClientRect(GetParent()->m_hWnd,rectClient);
			int nSplit = 0;
			int nStaticallyAssignedWidth = 140;
			int nDynamicallyAssignedWidth = rectClient.Width()/4; 
						
			nSplit = GetIniShort(IDS_INI_FILTERS_WINDOW_SPLITER);
			
			// If value is negative or more than the width of the screen, then just use default
			if (nSplit < 0 || nSplit >= GetSystemMetrics(SM_CXFULLSCREEN))
			{
				ASSERT(0);
				// Lets choose it depending on the mode we are in, anyways it will be atleast 140 units (if not more)
				nSplit = __max(nDynamicallyAssignedWidth,nStaticallyAssignedWidth);
			}
			else	// This means that the value is positive & within the boundaries of acceptance
			{
				// If user is not forcing us to use his splitter window position setting, then lets set the default value
				if (FALSE == GetIniShort(IDS_INI_USE_MY_FILTERS_WINDOW_POSITION))
				{
					// The max of these values
					nSplit = __max(nSplit,nStaticallyAssignedWidth);
					// It's better to also compare with width of the client's rectangular area to have better size allocation
					nSplit = __max(nSplit,nDynamicallyAssignedWidth);
				}				
			}

			m_wndSplitter.SetColumnInfo(0, nSplit, 0);
			m_wndSplitter.SetColumnInfo(1, 0, 0);

			// Need to notify the child wazoos to perform their OnInitialUpdate() sequence.			
			SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

			PostMessage( WM_SIZE, SIZE_RESTORED, MAKELPARAM( rectClient.Width(), rectClient.Height() ) );		

			bFuncEntered = TRUE;
		}
	}

	
	GetFiltersDoc();
}


/////////////////////////////////////////////////////////////////////////////
// CFiltersWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CFiltersWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	//
	// We just need a bogus context to keep CWndSplitter::CreateView()
	// happy.
	//
	//CCreateContext myBogusContext;
	
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CFiltersWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	if (m_wndSplitter.GetSafeHwnd())
	{	
		m_wndSplitter.SetWindowPos(NULL, rectClient.left, rectClient.top,
											rectClient.Width(), rectClient.Height(),
											SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

	}

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnGetMinMaxInfo [protected]
//
////////////////////////////////////////////////////////////////////////
void CFiltersWazooWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	lpMMI->ptMinTrackSize.x += 420; 
	lpMMI->ptMinTrackSize.y += 360;

	CWnd::OnGetMinMaxInfo(lpMMI);
}
// --------------------------------------------------------------------------

//
// FIND TEXT
//

void CFiltersWazooWnd::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CFiltersWazooWnd::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG CFiltersWazooWnd::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCProtocol *pProtocol = NULL;
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (!pFindMgr)
		return (EuFIND_ERROR);

	// This is our internal message to ask if we support find.
	// Return non-zero (TRUE).
	if (pFindMgr->IsAck(wParam, lParam))
		return (EuFIND_ACK_YES);

	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if (!lpFR)
		return (EuFIND_ERROR);

	if (lpFR->Flags & FR_DIALOGTERM)
	{
		ASSERT(0); // Should never fwd a terminating msg
		return (EuFIND_ERROR);
	}

	CWnd *pwndFocus = m_wndSplitter.GetPane(0,0);
	ASSERT(pwndFocus);

	if(!pwndFocus)
		return (EuFIND_ERROR);

	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, pwndFocus );

	ASSERT(pProtocol);


	
	if( ( pProtocol != NULL ) &&
		( pProtocol->DoFindNext( lpFR->lpstrFindWhat, (lpFR->Flags & FR_MATCHCASE), (lpFR->Flags & FR_WHOLEWORD), TRUE) ) == EuFIND_OK)
	{
		// activate the window
		pwndFocus->GetParentFrame()->ActivateFrame();
		return (EuFIND_OK);
	}
		

	return (EuFIND_NOTFOUND);
}

