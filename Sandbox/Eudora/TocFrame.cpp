// TocFrame.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "mainfrm.h"
#include "rs.h"
#include "utils.h"
#include "guiutils.h"
#include "helpcntx.h"
#include "msgutils.h"
#include "TocFrame.h"
#include "TocView.h"
#include "ReadMessageDoc.h"
#include "PgReadMsgPreview.h"
#include "BidentPreviewView.h"
#include "TridentPreviewView.h"
#include "SearchDoc.h"
#include "SearchView.h"
#include "eudora.h"
#include "cursor.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"


// FOR IMAP4 - yuck. Shouldn't need to include this.

#include "imapfol.h"

// END IMAP4

extern QCMailboxDirector g_theMailboxDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern UINT umsgPreviewMessage;

UINT umsgPurgeSummaryCache = ::RegisterWindowMessage("umsgPurgeSummaryCache");
UINT umsgLoadNewPreview = ::RegisterWindowMessage("umsgLoadNewPreview");

/////////////////////////////////////////////////////////////////////////////
// CTocFrame

IMPLEMENT_DYNCREATE(CTocFrame, CMDIChildTarget)

CTocFrame::CTocFrame()
{
	m_pPreviewSummary = NULL;
	m_dwStartPreviewTime = 0;
	m_iMajorVersion = -1;
	m_iMinorVersion = -1;
	m_bIsPreviewPaneEmpty = TRUE;
	m_bDoAutoMarkAsRead = FALSE;
	m_nTocListHeight = 0x7FFFFFFF;
	m_pPreviousPreviewableSummaryFromDoc = NULL;
	m_bSchmookieState = false;
}

CTocFrame::~CTocFrame()
{
}


BEGIN_MESSAGE_MAP(CTocFrame, CMDIChildTarget)
	//{{AFX_MSG_MAP(CTocFrame)
	ON_REGISTERED_MESSAGE( umsgPreviewMessage, OnPreviewMessage )
	ON_REGISTERED_MESSAGE( umsgPurgeSummaryCache, OnPurgeSummaryCache )
	ON_WM_TIMER()
	ON_COMMAND(ID_MESSAGE_DELETE, OnCmdDelete)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_DELETE, OnUpdateCmdDelete)
	ON_WM_CONTEXTMENU()
	ON_WM_CLOSE()
	ON_COMMAND(IDA_TOGGLEHEADER, OnToggleHeader)
	ON_COMMAND(ID_EDIT_FIND_FINDMSG, OnEditFindFindMsg)
	ON_WM_CHILDACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_UNDELETE, OnUpdateCmdUndelete)
	ON_COMMAND(ID_MESSAGE_UNDELETE, OnCmdUnDelete)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PURGE, OnUpdateCmdImapExpunge)
	ON_COMMAND(ID_MESSAGE_PURGE, OnCmdImapExpunge)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CTocFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
	BOOL fRet;

	GetMSHTMLVersion( &m_iMajorVersion, &m_iMinorVersion);

	fRet = m_wndSplitter.CreateStatic( this, 2, 1, WS_CHILD | WS_VISIBLE );
	ASSERT(fRet);

	if (fRet)
	{
		fRet = m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CTocView ),
					CSize( 315, 140 ), pContext );
		ASSERT(fRet);
	}

	if (fRet)
	{
		CRuntimeClass* pPreviewRuntimeClass = RUNTIME_CLASS(PgReadMsgPreview);

		if (GetIniShort(IDS_INI_USE_BIDENT_ALWAYS) && 
			(m_iMajorVersion > 4 || (m_iMajorVersion == 4 && m_iMinorVersion >= 71)))
		{
			pPreviewRuntimeClass = RUNTIME_CLASS(CTridentPreviewView);
		}

		fRet = m_wndSplitter.CreateView(1, 0, pPreviewRuntimeClass, CSize(10, 600), pContext);
		ASSERT(fRet);
	}

	return fRet;
}

BOOL CTocFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Pass sort-related commands to the TocView.  Kind of a hack, but allows a good
	// separation of layers in not having all the sort-related functions being in CTocFrame.
	if ((nID >= ID_EDIT_SORT_SORTBYSTATUS && nID <= ID_EDIT_SORT_SORTBYSUBJECT) ||
		nID == ID_EDIT_SORT_GROUPBYSUBJECT)
	{
		CTocView* pTocView = DYNAMIC_DOWNCAST(CTocView, m_wndSplitter.GetPane(0, 0));
		if (pTocView)
		{
			if (pTocView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
				return TRUE;
		}
	}

	return CMDIChildTarget::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


// every time the mdi child is moved, resized, clicked-to-front...
void CTocFrame::OnChildActivate()
{
    CMDIChildTarget::OnChildActivate();

    // message is handled by our OnPreviewMessage routine
    PostMessage( umsgPreviewMessage );
}


INT CTocFrame::GetSplitterPos()
{
	INT cyCur;
	INT cyMin;
	m_wndSplitter.GetRowInfo(1, cyCur, cyMin);

	if ((0 == cyCur) && (m_nTocListHeight != 0x7FFFFFFF))
	{
		//
		// Preview pane is currently closed due to F7 toggle, so return
		// the last known splitter position instead of the current 
		// splitter position.
		//
		return m_nTocListHeight;
	}

	m_wndSplitter.GetRowInfo(0, cyCur, cyMin);
	return cyCur;
}


void CTocFrame::ShowHideSplitter()
{
	CTocDoc* TocDoc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, TocDoc);

	if (TocDoc->UsingPreviewPane())
	{
		//
		// Do some analysis to see if the splitter will be positioned
		// in some reasonable spot.  Remember that this calculation
		// happens before the CMDIChild::ActivateFrame() stuff mangles
		// the TocFrame window position, so the calculation below
		// is, at best, only a guess.
		//
		int nFrameClientHeight = 0;
		{
			CRect rectSplitter;
			m_wndSplitter.GetWindowRect(rectSplitter);

			CRect rectMDIClient;
			{
				CMainFrame* pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
				ASSERT_KINDOF(CMainFrame, pMainFrame);
				pMainFrame->GetRealClientRect(rectMDIClient);
				pMainFrame->ClientToScreen(rectMDIClient);
			}

			nFrameClientHeight = rectMDIClient.bottom - rectSplitter.top;	// can be negative
		}

		int nSplitterPos = TocDoc->GetSplitterPos();
		if (nSplitterPos > 0 && nSplitterPos < nFrameClientHeight)
		{
			//
			// Only change the splitter position if it looks like it has
			// a chance of being positioned in the middle of the frame. 
			// 
			m_wndSplitter.SetRowInfo(0, nSplitterPos, 0);
			m_wndSplitter.SetRowInfo(1, 0x7FFFFFFF, 0);
			m_wndSplitter.RecalcLayout();
			m_wndSplitter.GetPane(1, 0)->InvalidateRect(NULL);
		}
	}
	else
	{
		//
		// Pin the splitter to the bottom of the window.
		//
		m_wndSplitter.SetRowInfo(1, 0, 0x7FFFFFFF);
		m_wndSplitter.RecalcLayout();
	}
}


////////////////////////////////////////////////////////////////////////
// GetPreviewableSummaryFromDoc [private]
//
// Just a little helper function for a commonly used code snippet.
////////////////////////////////////////////////////////////////////////
CSummary* CTocFrame::GetPreviewableSummaryFromDoc()
{
	CTocDoc* pTocDoc = (CTocDoc *) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, pTocDoc);
	return pTocDoc->GetPreviewableSummary();			// can be NULL
}


/////////////////////////////////////////////////////////////////////////////
// CTocFrame message handlers


////////////////////////////////////////////////////////////////////////
// OnPreviewMessage [protected]
//
// Notification that the TocDoc's cached "preview summary" has
// changed.  The WPARAM is non-zero if TOC list box contains a
// single-selection and was changed as a result of direct user action
// (i.e., it got a LBN_SELCHANGE notification).  Otherwise, a zero
// WPARAM value means the list box contains an unknown selection, no
// selection, or a multiple selection.
////////////////////////////////////////////////////////////////////////
long CTocFrame::OnPreviewMessage(WPARAM wParam, LPARAM)
{
	if ( !ShouldPreview() )
		return 0;

	//
	// We can't grab a summary pointer posted via a message to the
	// message queue and expect it to be valid, so we need to go
	// directly to the source CTocDoc and grab the summary pointer
	// from there.
	//

	// BOG: I smell trouble!
	//
	// First off, this is a public interface, and it requires the caller to know
	// enough not to *send* a message, but to post the message instead. I don't
	// like *that* very much. Secondly, if it were not for the special "null
	// summary" case, there would be no reason for this to be a message handler
	// at all. All we do is get the summary, do a little housekeeping, set the
	// automarkasread stuff, fire off the timer, then split. Most importantly
	// though, posting the message is not a reliable way to set state info like
	// m_bDoAutoMarkAsRead; this behavior is flaky when posting the message,
	// and fine when the message is sent.
	//
	// I don't really understand why the null summary can't be handled by the
	// timer, the same way non-null summaries are; so that's what I've done.
	// The OnDrawItem code in TocView has been changed to use SendMessage;
	// hopefully even that is temporary, and later we'll just change this to
	// an ordinary accessor function for setting the summary to be previewed.

	CSummary* pPreviewableSummary = GetPreviewableSummaryFromDoc();

//	TRACE2("CTocFrame::OnPreviewMessage: pPreviewableSummary=0x%08X, m_pPreviewSummary=0x%08X\n", pPreviewableSummary, m_pPreviewSummary);
//	TRACE2("                             m_pPreviousPreviewableSummaryFromDoc=0x%08X, bAuto=%d\n", m_pPreviousPreviewableSummaryFromDoc, wParam);

	if (NULL == pPreviewableSummary)
	{
		//
		// A NULL "focus" Summary pointer occurs when there are no
		// more messages left in the mailbox or when we're redrawing
		// after deleting a message from the TOC.  This is our clue to
		// clear the contents of the preview pane...
		//

		// BOG: The schmookie stuff is a hack; one that will hopefully go away
		// some day when I get inspired, and figure a better solution to
		// all this preview mumbo-jumbo. The main thing to remember is that we
		// are handling a message that has been *sent* right in the middle of an
		// OnDrawItem handler; time and reentrancy are serious concerns.

//		OnPurgeSummaryCache(0, 0);
//		ASSERT(NULL == m_pPreviewSummary);
//		DoPreviewDisplay();
//		KillTimer(1);
//		m_bDoAutoMarkAsRead = FALSE;
//		m_pPreviousPreviewableSummaryFromDoc = NULL;

		m_bSchmookieState = true;	// signals some special-case code in OnTimer
		SetTimer(1, 100, NULL);
		return 0;
	}
	m_bSchmookieState = false ;	// un-signals some special-case code in OnTimer

	ASSERT_KINDOF(CSummary, pPreviewableSummary);

	//
	// Okay, the following if-else logic is not at all obvious.
	// The idea here is to capture and "latch" a non-zero wParam
	// value that could occur in any notification message for a
	// *particular* newly-recognized summary.  Empirically, we see
	// that the TOC list box generally sends us multiple umsgPreviewMessage
	// notifications for the *same* Summary object.  However, due to the
	// unpredictable order of the owner-draw processing vs. the LBN_SELCHANGE
	// notifications, a non-zero wParam value doesn't always pair up
	// with the first umsgPreviewMessage notification for a given summary.
	// Therefore, we use the following logic to detect *changes* in the
	// Summary being previewed and "latch" the non-zero wParam, if any,
	// for any notification for that Summary.  When we detect a change
	// in Summary objects, we clear the latch state.  WKS 97.10.31.
	// 

	if (m_pPreviousPreviewableSummaryFromDoc != pPreviewableSummary)
	{
		m_pPreviousPreviewableSummaryFromDoc = pPreviewableSummary;

		//
		// Detected a new previewable candidate, so clear the latch
		// state, unless of course the latch is set on the first
		// message for this summary.
		//
		if (wParam)
			m_bDoAutoMarkAsRead = TRUE;		// see auto-mark-as-read logic in OnTimer()
		else
			m_bDoAutoMarkAsRead = FALSE;	// clear latch state
	}
	else
	{
		if (wParam)
			m_bDoAutoMarkAsRead = TRUE;		// see auto-mark-as-read logic in OnTimer()
	}

	if (pPreviewableSummary != m_pPreviewSummary)
	{
		SetTimer(1, 250, NULL);
		m_dwStartPreviewTime = GetTickCount();
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnPurgeSummaryCache [protected]
//
// Handler for registered message "umsgPurgeSummaryCache".  Somebody
// is telling us that one or more memory-resident Summary objects are
// going away, so the frame responds by getting rid of the cached
// Summary pointer and associated view-less MessageDoc, if any.
////////////////////////////////////////////////////////////////////////
long CTocFrame::OnPurgeSummaryCache(WPARAM, LPARAM)
{
	if (m_pPreviewSummary)
	{
		m_pPreviewSummary->NukeMessageDocIfUnused();
		m_pPreviewSummary = NULL;
	}

	return TRUE;
}

	
void CTocFrame::OnTimer(UINT nIDEvent) 
{
	// BOG: more "I don't quite understand" schmookie stuff
	if ( m_bSchmookieState == true ) {
		KillTimer(1);
		m_bSchmookieState = false;

		//
		// A NULL "focus" Summary pointer occurs when there are no
		// more messages left in the mailbox or when we're redrawing
		// after deleting a message from the TOC.  This is our clue to
		// clear the contents of the preview pane...
		//

		OnPurgeSummaryCache(0, 0);
		ASSERT(NULL == m_pPreviewSummary);
		DoPreviewDisplay();
		m_bDoAutoMarkAsRead = FALSE;
		m_pPreviousPreviewableSummaryFromDoc = NULL;
		return;
	}

	CSummary* pPreviewableSummary = GetPreviewableSummaryFromDoc();

	if( pPreviewableSummary )
	{
		//
		// Got a candidate for a preview change, but ignore it if
		// the user is holding down navigation keys or either
		// mouse button.
		//
		if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) ||
			GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT) ||
			GetAsyncKeyState(VK_PRIOR) || GetAsyncKeyState(VK_NEXT) ||
			GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON))
		{
			return;
		}

		//
		// Got a candidate for a preview change, but ignore it until
		// at least 1/4 second has gone by since we first saw this
		// candidate.
		//
		if (GetTickCount() - m_dwStartPreviewTime < 250)
			return;
	}
	else
	{
		//
		// If we get here, then the current preview item no longer needs
		// to be marked as read so we can kill the timer.  Everything
		// is basically stable since there is no new candidate and
		// the current item doesn't need a state change.
		//
		KillTimer(1);
		return;
	}

	// The timer needs to stay around if the message is unread so
	// that it can be marked as read after a user-defined number of seconds
	if (pPreviewableSummary->m_State != MS_UNREAD)
	{
		KillTimer(1);
	}

	if (m_pPreviewSummary == pPreviewableSummary)
	{
		//
		// The "focus" Summary and the displayed Summary are the
		// same, so check to see if it is time for the displayed 
		// preview item to be marked as read.  Only perform the
		// auto-mark if this mailbox changed as a result of direct
		// user action -- automatic mail checks and just opening a
		// mailbox don't count.
		//
		if( GetIniShort( IDS_INI_SET_PREVIEW_READ ) )
		{
			if ((GetTickCount() - m_dwStartPreviewTime) < (DWORD)(GetIniLong(IDS_INI_SET_PREVIEW_READ_SECONDS) * 1000))
				return;

			ASSERT( m_pPreviewSummary != NULL );
			if( m_bDoAutoMarkAsRead && (m_pPreviewSummary->m_State == MS_UNREAD))
			{
				m_pPreviewSummary->SetState( MS_READ );

#ifdef IMAP4 // IMAP4
				// Must also set the remote message status.
				// We should do this before we call CSummary::SetState() but this
				// is less disruptive of the code.
				// 
				if (m_pPreviewSummary->m_TheToc && m_pPreviewSummary->m_TheToc->IsImapToc())
				{
					// Make sure to kill the timer before calling the IMAP command:
					//
					KillTimer (1);

					m_pPreviewSummary->m_TheToc->ImapSetMessageState (m_pPreviewSummary, MS_READ);

					return;
				}
#endif // IMAP4
			}
		}

		KillTimer(1);
		return;
	}

	//
	// At this point, we're definitely changing preview views, so
	// nuke any view-less docs associated with the old summary.
	//
	OnPurgeSummaryCache(0, 0);
	ASSERT(NULL == m_pPreviewSummary);

	m_pPreviewSummary = pPreviewableSummary;

	//
	// Call a common routine which keys off of 'm_pPreviewSummary' to
	// handle the details of displaying a new preview pane.
	//
	DoPreviewDisplay();
}


////////////////////////////////////////////////////////////////////////
// DoPreviewDisplay [protected]
//
// Common routine for displaying a new Preview Pane of the appropriate
// type.  This all keys off of the current 'm_pPreviewSummary'
// pointer, which can be NULL in the case of an empty mailbox (which
// obviously contains no Summaries).
////////////////////////////////////////////////////////////////////////
void CTocFrame::DoPreviewDisplay() 
{
	//
	// The following static flag is designed to prevent this
	// routine from becoming re-entrant.  The re-entrancy can occur
	// because of the PeekMessage() loop below which waits for
	// the Bident/Trident view to initialize.  NEVER leave this routine
	// before setting this flag back to FALSE.
	//
	static BOOL				bIsDoingPreviewDisplay = FALSE;

	if (bIsDoingPreviewDisplay)
	{
		// Don't assert if IMAP - this is normal operation.
		if ( m_pPreviewSummary && m_pPreviewSummary->m_TheToc && m_pPreviewSummary->m_TheToc->IsImapToc() )
		{
			return;
		}

		ASSERT(0);
		return;
	}

	bIsDoingPreviewDisplay = TRUE;		// set re-entrancy semaphore
	
	if (m_bIsPreviewPaneEmpty && (NULL == m_pPreviewSummary))
	{
		//
		// Ugly Kludge Hack to get infinite redraw loop to stop in
		// the case of an empty mailbox.  This is all caused by
		// the order dependent stuff that happens when the mailbox
		// deletes messages and informs this Frame that its cached
		// summary pointer (m_pPreviewSummary) is invalid.  When this
		// happens, the Frame properly sets m_pPreviewSummary to NULL.
		// However, a NULL pointer doesn't give us information since
		// it can be there are no Summaries in the mailbox or there is
		// temporarily no preview item being displayed.  Hence, the
		// need to maintain the ugly extra flag, 'm_bIsPreviewPaneEmpty'.
		//
		bIsDoingPreviewDisplay = FALSE;
		return;
	}

// IMAP stuff.
	// Seems like we're going to be doing the preview. It it's an IMAP mailbox, go
	// download the message before we start all this crap.
	//
	if (m_pPreviewSummary)
	{
		if ( m_pPreviewSummary->m_TheToc && m_pPreviewSummary->m_TheToc->IsImapToc() )
		{
			BOOL bSuccess = FALSE;

			CImapMailbox* pImapMailbox = m_pPreviewSummary->m_TheToc->m_pImapMailbox;

			if ( pImapMailbox && pImapMailbox->PreviewAllowed() )
			{
				bSuccess = SUCCEEDED (DownloadImapMessage (m_pPreviewSummary));
			}

			if (!bSuccess)
			{
				bIsDoingPreviewDisplay = FALSE;
				return;
			}
		}
	}
// END IMAP STUFF

	CRuntimeClass* pOldPreviewRuntimeClass = m_wndSplitter.GetViewType();
	CRuntimeClass* pNewPreviewRuntimeClass = RUNTIME_CLASS(PgReadMsgPreview);
	BOOL bPaige = TRUE;
	BOOL bTrident = FALSE;

	if( GetIniShort( IDS_INI_USE_BIDENT_ALWAYS ) && 
		( ( m_iMajorVersion > 4 ) ||
		  ( ( m_iMajorVersion == 4 ) && ( m_iMinorVersion >= 70 ) ) ) )
	{
		if( ( m_iMajorVersion > 4 ) ||
		    ( ( m_iMajorVersion == 4 ) && ( m_iMinorVersion >= 71 ) ) )
		{
			bTrident = TRUE;
			bPaige = FALSE;
			pNewPreviewRuntimeClass = RUNTIME_CLASS(CTridentPreviewView);
		}
		else if( m_pPreviewSummary && m_pPreviewSummary->IsHTML() )
		{
			bPaige = FALSE;
			pNewPreviewRuntimeClass = RUNTIME_CLASS(CBidentPreviewView);
		}
	}

	if (pOldPreviewRuntimeClass == pNewPreviewRuntimeClass)
	{
		m_wndSplitter.GetPane(1, 0)->SendMessage(umsgLoadNewPreview);
	}
	else
	{
		INT cyCur;
		INT cxCur;
		INT cyMin;
		INT cxMin;

		m_wndSplitter.GetRowInfo( 1, cyCur, cyMin);
		m_wndSplitter.GetColumnInfo( 0, cxCur, cxMin);

		m_wndSplitter.SetViewType(pNewPreviewRuntimeClass);

		//
		// As an optimization for switching between previewed messages, we
		// don't normally repaint the TOC list part of the window.
		// However, in the case where this mailbox was previously obscured
		// behind some other window that just got nuked (before we have a
		// chance to repaint), then we *do* want to repaint the TOC list.
		//
		BOOL bTOCListNeedsFullRepaint = FALSE;
		{
			CRect rectUpdate;
			CRect rectClient;
			m_wndSplitter.GetPane(0, 0)->GetUpdateRect(rectUpdate);
			m_wndSplitter.GetPane(0, 0)->GetClientRect(rectClient);
			if (rectUpdate == rectClient)
				bTOCListNeedsFullRepaint = TRUE;
		}
		
		//
		// The following code sequence to switch out the old preview window
		// and splice in a new preview window was formulated after hours
		// and hours of experimentation and twiddling.  Please don't
		// mess with it unless you really know what you're doing.  :-)
		// WKS 97.10.22
		//
		m_wndSplitter.SetRedraw( FALSE );
		m_wndSplitter.DeleteView(1, 0);
		m_wndSplitter.CreateView( 1, 0, NULL, CSize( cxCur, cyCur ), NULL );
		m_wndSplitter.SetRowInfo( 1, cyCur, cyMin );
		m_wndSplitter.SetColumnInfo( 0, cxCur, cxMin );
		m_wndSplitter.SetRedraw( TRUE );
		m_wndSplitter.GetPane( 1, 0 )->Invalidate();
		m_wndSplitter.RecalcLayout();
		if (! bTOCListNeedsFullRepaint)
			m_wndSplitter.GetPane( 0, 0 )->ValidateRect(NULL);		// optimization to avoid WM_ERASEBKGND flicker
	}

	if( !bPaige )
	{
		//
		// Make sure something gets the focus back from those nasty
		// Microsoft controls.
		//
		CBidentPreviewView* pBidentView = NULL;
		CTridentPreviewView* pTridentView = NULL;
		if( bTrident )
		{
			pBidentView = NULL;
			pTridentView = (CTridentPreviewView *) m_wndSplitter.GetPane(1, 0);
		}
		else
		{
			pBidentView = (CBidentPreviewView *) m_wndSplitter.GetPane(1, 0);
			pTridentView = NULL;
		}

		MSG msg;
		while (((pTridentView && !pTridentView->IsReady()) || 
				(pBidentView && !pBidentView->IsReady())) &&
				PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			// Don't try to preview at this time, as it will
			// cause Weird Things(TM) to happen.
			if (msg.message != umsgPreviewMessage)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if ( pTridentView ) {
			pTridentView->EnableWindowWithDelay( 400 /* mSec */ );
		}
		else if ( pBidentView ) {
			pBidentView->EnableWindow( TRUE );
			m_wndSplitter.GetPane(0, 0)->SetFocus();
		}
	}

	// Set this here to determine when to mark message as read after certain elapsed time
	m_dwStartPreviewTime = GetTickCount();

	//
	// For next time, remember whether or not we actually put
	// something in the Preview Pane.
	//
	if (m_pPreviewSummary)
		m_bIsPreviewPaneEmpty = FALSE;
	else
		m_bIsPreviewPaneEmpty = TRUE;

	// IMAP STUFF
	// If this was a preview in an IMAP mailbox, download status would not have
	// been updated because the view was disabled.
	// We need to update the status now (in every case, unfortunately).
	//
	if ( m_pPreviewSummary  && (pOldPreviewRuntimeClass != pNewPreviewRuntimeClass) )
	{
		if (m_pPreviewSummary->m_TheToc && m_pPreviewSummary->m_TheToc->IsImapToc())
		{
			m_pPreviewSummary->RedisplayField (FW_SERVER);
		}
	}
	// END IMAP STUFF

	bIsDoingPreviewDisplay = FALSE;		// clear re-entrancy semaphore
}


////////////////////////////////////////////////////////////////////////
// OnClose [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnClose()
{
	//
	// At this point, we're definitely going away, so
	// nuke any view-less docs associated with the old summary.
	//
	OnPurgeSummaryCache(0, 0);

	KillTimer(1);
	
	CTocDoc* pTocDoc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, pTocDoc);
	pTocDoc->NotifyFrameClosing();

	CMDIChildTarget::OnClose();
}


////////////////////////////////////////////////////////////////////////
// OnCmdDelete [protected]
//
// Since CTocView has a handler for ID_MESSAGE_DELETE, the only way we
// should end up here is if the preview pane is the active view.  This
// handler is here in the frame so that it can be shared by both the
// Paige and Trident implementations of the preview pane.
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnCmdDelete()
{
	// validate our command routing theory
	CWnd* pActivePane = m_wndSplitter.GetActivePane();
	if (pActivePane->IsKindOf(RUNTIME_CLASS(PgReadMsgPreview)) ||
		pActivePane->IsKindOf(RUNTIME_CLASS(CBidentPreviewView)) ||
		pActivePane->IsKindOf(RUNTIME_CLASS(CTridentPreviewView)))
	{
		if (m_pPreviewSummary)
		{
			CTocDoc* pTocDoc = m_pPreviewSummary->m_TheToc;
			CMessageDoc* pMessageDoc = m_pPreviewSummary->FindMessageDoc();
			if (pMessageDoc)
			{
				POSITION pos = pMessageDoc->GetFirstViewPosition();
				if (pos)
				{
					//
					// Found an open document with a view, so save
					// it before we nuke it.
					//
					if (! pMessageDoc->OnSaveDocument(NULL))
						pTocDoc = NULL;		// save was not successful
				}
			}

			if (pTocDoc)
			{

#ifdef IMAP4  // IMAP4

				// If this is an IMAP mailbox, just flag messages and get out.
				if (pTocDoc->IsImapToc())
				{
					// Pass this to the IMAP command object.
					//
					QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) pTocDoc->MBFilename() );
					if( pImapFolder != NULL )
					{
						pImapFolder->Execute( CA_DELETE_MESSAGE, NULL);
					}
					else
					{
						ASSERT(0);
					}
	
					return;
				}
#endif // IMAP4

				//
				// We no longer call Xfer() here since we want to go
				// through the command director logic for
				// CA_TRANSFER_TO.  The command director notifies the
				// TOC window containing this message that it is okay
				// to invoke the "auto-mark-as-read" logic since the
				// mailbox was changed due to an explicit user action.
				// The Xfer() function does not notify the TOC window.
				//
				//pTocDoc->Xfer(::GetTrashToc(), m_pPreviewSummary);
				CTocDoc* pTrashToc = ::GetTrashToc();
				if (pTrashToc)
				{
					QCMailboxCommand* pFolder = g_theMailboxDirector.FindByPathname( (const char *) pTrashToc->MBFilename() );
					if (pFolder != NULL)
						pFolder->Execute(CA_TRANSFER_TO, m_pPreviewSummary);
					else
					{
						ASSERT(0);
					}
				}
				ASSERT(NULL == m_pPreviewSummary);
			}
		}
		else
		{
			ASSERT(0);		// hmm.  how did we get here?
		}
	}
	else
	{
		ASSERT(0);			// hmm.  how did we get here?
	}
}


void CTocFrame::OnUpdateCmdDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pPreviewSummary != NULL);
}


BOOL CTocFrame::PreTranslateMessage(MSG* pMsg) 
{
	UINT	nChar;

	if( pMsg->message == WM_CHAR )
	{
		nChar = pMsg->wParam;
	
		if( ( nChar == VK_TAB ) && GetIniShort( IDS_INI_SWITCH_PREVIEW_WITH_TAB))
		{
			if ( ShouldPreview() )
			{
				if (ShiftDown())
					SendMessage(WM_COMMAND, ID_PREV_PANE);
				else
					SendMessage(WM_COMMAND, ID_NEXT_PANE);

				return TRUE;
			}
		}
	}

	return CMDIChildTarget::PreTranslateMessage(pMsg);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Common implementation for the Trident and Paige preview panes.  The
// CTocListBox has its own context menu handler, so this should only
// get called if the user clicks in the preview pane.
//
// FORNOW, this is a duplicate of code in CReadMessageFrame.  By
// putting it in both places, we allow the future possibility of
// having different context menus in previews vs real views (Christie
// made me do it).
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = ::QCLoadMenu(IDR_CONTEXT_POPUPS);
	if ( ! hMenu || ! menu.Attach( hMenu ) )
	{
		ASSERT(0);		// resources hosed?
		return;
	}

	// MP_POPUP_RECEIVED_MSG is the offset for the read message submenu.
	CMenu* pMenuPopup = menu.GetSubMenu(MP_POPUP_RECEIVED_MSG);
	if (pMenuPopup != NULL)
	{
		//
		// Since the popup menu we get from GetSubMenu() is a pointer
		// to a temporary object, let's make a local copy of the
		// object so that we have explicit control over its lifetime.
		//
		// Note that we edit the context menu on-the-fly in order to
		// stick in the latest/greatest Transfer menu, display the
		// edited context menu, then remove the Transfer menu.
		//
		CMenu tempPopupMenu;
		tempPopupMenu.Attach(pMenuPopup->GetSafeHmenu());

		CMenu theTransferMenu;
		theTransferMenu.CreatePopupMenu();

		//
		// Populate the new Transfer popup menu.
		//
		g_theMailboxDirector.NewMessageCommands( CA_TRANSFER_TO, &theTransferMenu, CA_TRANSFER_NEW );
		::WrapMenu( theTransferMenu.GetSafeHmenu() );
		theTransferMenu.InsertMenu( 3, MF_BYPOSITION, ID_TRANSFER_NEW_MBOX_IN_ROOT, CRString( IDS_MAILBOX_NEW ) );
		theTransferMenu.InsertMenu( 3, MF_BYPOSITION | MF_SEPARATOR );

		tempPopupMenu.InsertMenu(	MP_TRANSFER_READMSG,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theTransferMenu.GetSafeHmenu(), 
									CRString( IDS_TRANSFER_NAME ) );

		::WrapMenu( tempPopupMenu.GetSafeHmenu() );
		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
		CContextMenu(&tempPopupMenu, ptScreen.x, ptScreen.y);
		
		tempPopupMenu.RemoveMenu(MP_TRANSFER_READMSG, MF_BYPOSITION);

		g_theMailboxDirector.RemoveMessageCommands(CA_TRANSFER_TO, &theTransferMenu);
		g_theMailboxDirector.RemoveMessageCommands(CA_TRANSFER_NEW, &theTransferMenu);
		
		VERIFY(tempPopupMenu.Detach());
	}
	else
	{
		ASSERT(0);		// resources hosed?
	}

	menu.DestroyMenu();
} 


////////////////////////////////////////////////////////////////////////
// OnToggleHeader [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnToggleHeader() 
{
	CTocDoc* TocDoc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, TocDoc);

	if (TocDoc->UsingPreviewPane() == FALSE)
	{
		::MessageBeep(MB_OK);
		return;
	}

	//
	// Grab current TOC list height.
	//
	int nTocListCurrent, nTocListMin;
	m_wndSplitter.GetRowInfo( 0, nTocListCurrent, nTocListMin );

	//
	// Grab current Preview Pane height.
	//
	int nPreviewCurrent, nPreviewMin;
	m_wndSplitter.GetRowInfo( 1, nPreviewCurrent, nPreviewMin );

	//
	// Save current TOC list height so that we can restore it next time.
	//
	m_nTocListHeight = nPreviewCurrent ? nTocListCurrent : m_nTocListHeight;

	//
	// New header height is either saved value or very large.
	//
	nTocListCurrent = nPreviewCurrent ? 0x7FFFFFFF : m_nTocListHeight;
	m_wndSplitter.SetRowInfo( 0, nTocListCurrent, nTocListMin );
	m_wndSplitter.RecalcLayout();

	// BOG - Hack to make sure the selected item is visible when the user
	// opens the preview pane. This should probably check to see if the item
	// is allready visible. Maybe later.
	CTocView* pVW = (CTocView*) m_wndSplitter.GetPane( 0, 0 );
	pVW->m_SumListBox.SetTopIndex( pVW->m_SumListBox.GetCurSel() );
}

////////////////////////////////////////////////////////////////////////
// OnEditFindFindMsg [protected]
//
// If the Find Messages window isn't open, then open it up and select
// this mailbox in the Mailboxes tree control of the Find Messages
// window.  If the Find Messages window is alreay open, then just bring
// it to the front.
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnEditFindFindMsg()
{
	BOOL bSelectMailbox = TRUE;

	if (gSearchDoc && gSearchDoc->GetFirstViewPosition())
		bSelectMailbox = FALSE;

	// Opening the Find Messages window can take a while
	CCursor WaitCursor;
	NewChildDocFrame(SearchTemplate);

	if (!bSelectMailbox)
		return;

	// Now the global doc should be avail
	ASSERT(gSearchDoc);
	if (!gSearchDoc)
		return;

	// Get the SearchView
	POSITION pos = gSearchDoc->GetFirstViewPosition();
	CSearchView *pSearchView = pos? (CSearchView *) gSearchDoc->GetNextView(pos) : NULL;
	ASSERT(pSearchView);
	if (NULL == pSearchView)
		return;

	// Select the appropriate mbx
	// Needs to be posted for the item to be scrolled in to view correctly
	CTocDoc* pTocDoc = DYNAMIC_DOWNCAST(CTocDoc, GetActiveDocument());
	if (pTocDoc)
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname(pTocDoc->MBFilename());
		ASSERT(pCommand);
		if (pCommand)
			pSearchView->PostMessage(msgFindMsgMaiboxSel, (WPARAM)pCommand);
	}
}


#ifdef IMAP4 // IMAP4
//
//
// Enable undelete if this is an IMAP mailbox and there are messages 
// marked with the IMAP \delete flag.
//
void CTocFrame::OnUpdateCmdUndelete(CCmdUI* pCmdUI)
{

	CTocDoc* Doc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, Doc);

	if (m_pPreviewSummary && Doc && Doc->IsImapToc ())
	{
		pCmdUI->Enable( ImapSumIsDeleted (m_pPreviewSummary) );
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}



//
// Enable purge if this is an IMAP mailbox and there are messages 
// marked with the IMAP \delete flag.
//
void CTocFrame::OnUpdateCmdImapExpunge (CCmdUI* pCmdUI)
{
	CTocDoc* Doc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, Doc);

	if (Doc && Doc->IsImapToc())
	{
		pCmdUI->Enable( ImapTocHasDeletedMsgs (Doc) );
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}


void CTocFrame::OnCmdUnDelete()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, Doc);

	//
	// Note: Don't pass a NULL "SingleSum" to "ImapUndeleteMessages" because
	// it will undelete all selected messages!
	//
	if (Doc && Doc->IsImapToc() && (m_pPreviewSummary != NULL) )
	{
		Doc->ImapUnDeleteMessages(m_pPreviewSummary, FALSE);

		// Redisplay 
		InvalidateRect (NULL);
	}
}


void CTocFrame::OnCmdImapExpunge()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, Doc);

	if (Doc && Doc->IsImapToc())
	{
		Doc->ImapExpunge (FALSE);
	}
}


//
#endif // IMAP4


// ShouldPreview - this routine is an extension of "UsingPreviewPane" which
// takes into account the current disposition of the splitter. It is used prior
// to displaying messages, etc., in case the user has minimized the pane.
//
// Note: this routine cannot reliably tell you if the preview option is enabled
// or disabled; use "UsingPreviewPane()" in TocDoc for that.

bool CTocFrame::ShouldPreview()
{
	CTocDoc* TocDoc = ( CTocDoc* ) GetActiveDocument();
	ASSERT_KINDOF(CTocDoc, TocDoc);

	if ( TocDoc->UsingPreviewPane() == FALSE ) {

		// user has disabled preview pane globally
		return false;
	}

	int nPreviewCurrent, nPreviewMin;
	m_wndSplitter.GetRowInfo( 1, nPreviewCurrent, nPreviewMin );

	if ( nPreviewCurrent >= 5 /*a little slop*/ ) {

		// preview pane is open more that just a crack
		return true;
	}

	// preview pane must be closed
	return false;
}

