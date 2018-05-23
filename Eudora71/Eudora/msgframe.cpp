// MSGFRAME.CPP
//

#include "stdafx.h"
#include "eudora.h"

#include "msgframe.h"
#include "msgdoc.h"
#include "summary.h"
#include "guiutils.h"

#include "mainfrm.h"
#include "rs.h"
#include "newmbox.h"
#include "tocdoc.h"
#include "tocview.h"

#include "compmsgd.h" //for SetQueueStatus
#include "QCCommandActions.h"
#include "QCMailboxDirector.h"

#include "QCOleDropSource.h"

extern QCMailboxDirector	g_theMailboxDirector;

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageFrame

IMPLEMENT_DYNAMIC(CMessageFrame, CMDIChild)

CMessageFrame::CMessageFrame()
{
#ifdef OLDSTUFF
	m_bWasIconic = FALSE;
#endif
}

CMessageFrame::~CMessageFrame()
{
}

void CMessageFrame::ActivateFrame(int nCmdShow /*= -1*/)
{
	CSummary* Sum = ((CMessageDoc*)GetActiveDocument())->m_Sum;
	
	if (Sum)
	{
		Sum->m_FrameWnd = this;
		if(Sum->m_TheToc && Sum->m_TheToc->m_Type == MBT_OUT && Sum->IsQueued())
			SetQueueStatus();
	}
		
	CMDIChild::ActivateFrame(nCmdShow);

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(&wp))
	{
		GetWindowRect(&wp.rcNormalPosition);
		AfxGetMainWnd()->ScreenToClient(&wp.rcNormalPosition);
	}
	m_InitialSize = wp.rcNormalPosition;
}


QCProtocol * CMessageFrame::GetProtocol(QCPROTOCOLID in_protocolID)
{
	QCProtocol *		pProtocol = NULL;
	CView *				pView = reinterpret_cast<CView *>( m_wndSplitter.GetPane(0, 0) );
	
	if (pView)
		pProtocol = QCProtocol::QueryProtocol(in_protocolID, pView);

	return pProtocol;
}


BEGIN_MESSAGE_MAP(CMessageFrame, CMDIChild)
	//{{AFX_MSG_MAP(CMessageFrame)
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMessageFrame message handlers

// Ctrl+double-clicking the title bar of a message opens up the mailbox that the message
// belongs to, and selects the summary of the message
//
void CMessageFrame::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if (nHitTest == HTCAPTION && GetKeyState(VK_CONTROL) < 0)
	{
		CSummary* Sum = ((CMessageDoc*)GetActiveDocument())->m_Sum;
		
		if (Sum && Sum->m_TheToc)
		{
			CTocDoc* TocDoc = Sum->m_TheToc;
			CTocView* TocView = TocDoc->GetView();
			
			if (TocView)
				TocView->SelectAll(FALSE, TRUE);

			// Pass in true for the second parameter because this is the result of a
			// direct user action and we want the auto-mark-as-read behavior to apply.
			if (TocDoc->Display())
				Sum->Select(TRUE, true);
		}
	}
	else
		CMDIChild::OnNcLButtonDblClk(nHitTest, point);
}

void CMessageFrame::OnDestroy()
{
	// See if window position/size needs to be saved
	WINDOWPLACEMENT wp;
	CMessageDoc*	pDoc;

	wp.length = sizeof(wp);
	if (!GetWindowPlacement(&wp))
	{
		GetWindowRect(&wp.rcNormalPosition);
		GetParent()->ScreenToClient(&wp.rcNormalPosition);
	}

	pDoc = (CMessageDoc*)GetActiveDocument();
	CSummary* Sum = NULL;
	if(pDoc)
		Sum = pDoc->m_Sum;

	if(Sum)
	{
		if( Sum->m_TheToc && Sum->m_TheToc->m_Type == MBT_OUT && Sum->IsQueued())
			SetQueueStatus();

		// RR Dialog Override - jdboyd 8/23/99
		// Forget about the fact that we may have skipped the RR dialog.
		if (Sum->ReadReceipt())
			Sum->m_rrDialogOverride = false;
	}
		
	if (m_InitialSize != wp.rcNormalPosition)
	{
		/*pDoc = (CMessageDoc*)GetActiveDocument();

		if( pDoc )
		{
			CSummary* Sum = pDoc->m_Sum;
		*/	
			if(Sum != NULL)
			{
				Sum->SetSavedPos( CRect( wp.rcNormalPosition ) );
				//
				// The following assignment used to be in 
				// CMessageDoc::PreCloseFrame().  That seems like an odd
				// place, especially since the 'm_FrameWnd' member is
				// set here in the ActivateFrame() method.  Since it was
				// causing shutdown crashes, it was moved here so that
				// we can avoid race conditions between deletion of the
				// CSummary object and the frame window object.
				//
				Sum->m_FrameWnd = NULL;
			}
		/*}*/
	}

	CMDIChild::OnDestroy();
}

#ifdef OLDSTUFF
LRESULT CMessageFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message ) {
	case WM_MOVE:
		CReadMessageView* pView = (CReadMessageView*) GetActiveView();
		if ( pView ) {
			if ( pView->IsKindOf(RUNTIME_CLASS(CReadMessageView)) ) {
				if ( pView->m_fCreated && !(pView->m_fAttachmentsConverted) ) {
					pView->OnPostInitialUpdate(
							CReadMessageView::kDelayedUpdate, 0 );
				}
			}
		}
		break;
	}
	
	return (CMDIChild::WindowProc(message, wParam, lParam));
}
#endif



void CMessageFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChild::OnSize(nType, cx, cy);
#ifdef OLDSTUFF
	// make sure the RichEditView re-flows the text
	if ( nType == SIZE_MINIMIZED )
		m_bWasIconic = TRUE;
	else if ( ( nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED ) && m_bWasIconic )
	{
		m_bWasIconic = FALSE;

		// is it a ReadMessageView?
		CReadMessageView* pRMView = (CReadMessageView*)GetActiveView();
		if ( pRMView && pRMView->IsKindOf(RUNTIME_CLASS(CReadMessageView)) ) 
		{
			CMainFrame* p_mainframe = (CMainFrame *) AfxGetMainWnd();
			ASSERT(p_mainframe && p_mainframe->IsKindOf(RUNTIME_CLASS(CMainFrame)));
			if ( p_mainframe->m_bStartedIconic && pRMView->m_bFirstRestore )
			{
				pRMView->m_bFirstRestore = FALSE;

				pRMView->SetText();
				pRMView->IconizeAttachments( FALSE );
				pRMView->RedrawWindow();
			}
			else
			{
				CRect theRect;
				pRMView->GetClientRect( theRect );
				pRMView->PostMessage( WM_SIZE, SIZE_RESTORED, MAKELPARAM( theRect.Width(), theRect.Height() ) );
			}

			return;
		}

		// is it a CompMessageView?
		// CompMessageViews can return a HeaderView as the ActiveView(), so do the
		// round about thing...
		CCompMessageDoc* pCMDoc = (CCompMessageDoc*)GetActiveDocument();
		if ( pCMDoc && pCMDoc->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)) ) 
		{
			POSITION viewpos = pCMDoc->GetFirstViewPosition();
			while (viewpos)
			{
				CCompMessageView* View = (CCompMessageView*)pCMDoc->GetNextView(viewpos);
				if ( View && View->IsKindOf(RUNTIME_CLASS(CCompMessageView)) ) 
				{
					// force the CompMessageView to re-flow the text
					CRect theRect;
					View->GetClientRect( theRect );
					View->PostMessage( WM_SIZE, SIZE_RESTORED, MAKELPARAM( theRect.Width(), theRect.Height() ) );
				}
			}
		}
	}
#endif
}


BOOL CMessageFrame::PreTranslateMessage(
MSG*	pMsg )
{
	UINT			uVirtKey;
	BOOL			bCtrlDown;
	BOOL			bAltArrows;

	if( pMsg->message == WM_SYSKEYDOWN )
	{
		uVirtKey = pMsg->wParam;

		if( ( uVirtKey == VK_LEFT ) || ( uVirtKey == VK_UP ) ||
			( uVirtKey == VK_RIGHT ) || ( uVirtKey == VK_DOWN ) )
		{
			bCtrlDown = (GetKeyState(VK_CONTROL) < 0);
			bAltArrows = GetIniShort(IDS_INI_ALT_ARROWS) != 0;
			
			if( bAltArrows && !bCtrlDown )
			{
				SendMessage(	WM_COMMAND, 
								( uVirtKey == VK_LEFT || uVirtKey == VK_UP)? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE );
				return TRUE;
			}
		}
	}

	return CMDIChild::PreTranslateMessage( pMsg );		
}



BOOL CMessageFrame::OnCmdMsg( 
UINT				nID, 
int					nCode, 
void*				pExtra, 
AFX_CMDHANDLERINFO* pHandlerInfo )
{
	return CMDIChild::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}


