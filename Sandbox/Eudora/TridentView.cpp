// TridentView.cpp : implementation of the CTridentView class
//

#include "stdafx.h"
#include "shlobj.h"
#include "eudora.h"

#ifdef IDM_PROPERTIES
#undef IDM_PROPERTIES
#endif

#include <AFXPRIV.H>
#include <afxdisp.h>

#include "TridentPreviewView.h"
#include "TridentView.h"
#include "site.h"
#include "mshtmcid.h"
#include "mainfrm.h"
#include "QCRecipientDirector.h"
#include "address.h"
#include "font.h" // remove this!
// delete 
#include "newmbox.h"

#include "resource.h"
#include "fileutil.h"
#include "rs.h"
#include "guiutils.h"
#include "msgutils.h"
#include "summary.h"
#include "convhtml.h"
#include "bstr.h"

#include "QCOleDropSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define ODS(x) OutputDebugString(x)

extern QCRecipientDirector	g_theRecipientDirector;
extern CString EudoraDir;

/////////////////////////////////////////////////////////////////////////////
// CTridentView

IMPLEMENT_DYNAMIC(CTridentView, CView)

BEGIN_MESSAGE_MAP(CTridentView, CView)
	//{{AFX_MSG_MAP(CTridentView)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_ATTACHMENT, OnUpdateFileOpenAttachment)
	ON_COMMAND(ID_FILE_OPEN_ATTACHMENT, OnFileOpenAttachment)
//	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP

	ON_COMMAND( ID_VIEW_SOURCE, OnViewSource )
	ON_COMMAND( ID_SEND_TO_BROWSER, OnSendToBrowser)

	ON_COMMAND( ID_EDIT_COPY, OnCopy )
	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY, OnUpdateCopy )

	ON_COMMAND( ID_EDIT_COPY_ATTACHMENT, OnCopyAttachment )
	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY_ATTACHMENT, OnUpdateCopyAttachment )

	ON_COMMAND( ID_SPECIAL_ADDASRECIPIENT, OnSpecialAddAsRecipient )
	ON_UPDATE_COMMAND_UI( ID_SPECIAL_ADDASRECIPIENT, OnUpdateAddAsRecipient )

	ON_COMMAND( ID_EDIT_SELECT_ALL, OnSelectAll )
	ON_UPDATE_COMMAND_UI( ID_EDIT_SELECT_ALL, OnUpdateSelectAll )

	ON_REGISTERED_MESSAGE( wmReadyStateComplete, FixupSource )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTridentView construction/destruction

UINT CTridentView::s_uTmpFileUnique = 0;

CTridentView::CTridentView() 
{
    m_cRef = 0;
    m_hWndObj = NULL;
            
    m_pSite = NULL;
    m_pIOleIPActiveObject = NULL;
    m_fCreated = FALSE;
        
    m_szTmpFile = "";
    m_bFixedupSource = FALSE;
    m_nIsVisible = 0;

    m_bEnableCopyAttachment = FALSE;

#ifdef ADWARE_PROTOTYPE
	m_DocHostUIFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DIV_BLOCKDEFAULT;
#endif

}


CTridentView::~CTridentView()
{
	// Debug check that nobody is trying to hang onto us!
	unlink( m_szTmpFile );
	ASSERT( m_cRef == 0 );
}

// extern UINT _AfxGetMouseScrollLines(BOOL bForceFresh = FALSE);
/*
BOOL CTridentView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	UINT uWheelScrollLines;
	CWnd *Child = GetDescendantWindow(0);
	if (!Child)
		return FALSE;

	// Maybe make a handler for Preview and view separately?
	if (IsKindOf(RUNTIME_CLASS(CTridentPreviewView)))
	{
		uWheelScrollLines = _AfxGetMouseScrollLines(TRUE) * GetIniLong(IDS_INI_TRIDENT_PREVIEW_MOUSEWHEEL_MULTIPLIER); //3
	}
	else
	{
		uWheelScrollLines = _AfxGetMouseScrollLines(TRUE) * GetIniLong(IDS_INI_TRIDENT_VIEW_MOUSEWHEEL_MULTIPLIER); //1
	}
	int nToScroll;

	nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	WORD nScrollCode;

	if ((nFlags & 0x0000000F) == 4)
	{
		nScrollCode = (WORD)(zDelta > 0? SB_PAGEUP : SB_PAGEDOWN);
		nToScroll = 1;
	}
	else
	{
		nScrollCode = (WORD)(zDelta > 0? SB_LINEUP : SB_LINEDOWN);
	}
	if (nToScroll < 0)
		nToScroll = -nToScroll;
	for (int i = 0; i < nToScroll; i++)
		Child->SendMessage(WM_VSCROLL, MAKEWPARAM(nScrollCode, 0));

	return TRUE;
}
*/

/////////////////////////////////////////////////////////////////////////////
// CTridentView diagnostics

#ifdef _DEBUG
void CTridentView::AssertValid() const
{
	CView::AssertValid();
}

void CTridentView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG


/*
 * IUnknown implementation
 */
/*
 * CTridentView::QueryInterface
 * CTridentView::AddRef
 * CTridentView::Release
 */
STDMETHODIMP CTridentView::QueryInterface( REFIID riid, void **ppv )
{
    /*
     * We provide IOleInPlaceFrame and IOleCommandTarget
	 *   interfaces here for the ActiveX Document hosting
	 */
    *ppv = NULL;

    if ( IID_IUnknown == riid || IID_IOleInPlaceUIWindow == riid
        || IID_IOleWindow == riid || IID_IOleInPlaceFrame == riid )
	{
        *ppv = (IOleInPlaceFrame *)this;
	}

	if ( IID_IOleCommandTarget == riid )
	{
        *ppv = (IOleCommandTarget *)this;
	}

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CTridentView::AddRef( void )
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CTridentView::Release( void )
{
    //Nothing special happening here-- life if user-controlled.
	// Debug check to see we don't fall below 0
	ASSERT( m_cRef != 0 );
    return --m_cRef;
}


/*
 * IOleInPlaceFrame implementation
 */
/*
 * CTridentView::GetWindow
 *
 * Purpose:
 *  Retrieves the handle of the window associated with the object
 *  on which this interface is implemented.
 *
 * Parameters:
 *  phWnd           HWND * in which to store the window handle.
 *
 * Return Value:
 *  HRESULT         NOERROR if successful, E_FAIL if there is no
 *                  window.
 */
STDMETHODIMP CTridentView::GetWindow( HWND* phWnd )
{
	CFrameWnd*	pParentFrame;

	pParentFrame = GetParentFrame();
	
	if( pParentFrame != NULL )
	{
		*phWnd = pParentFrame->m_hWnd;
	}
	
	return NOERROR;
}



/*
 * CTridentView::ContextSensitiveHelp
 *
 * Purpose:
 *  Instructs the object on which this interface is implemented to
 *  enter or leave a context-sensitive help mode.
 *
 * Parameters:
 *  fEnterMode      BOOL TRUE to enter the mode, FALSE otherwise.
 *
 * Return Value:
 *  HRESULT         NOERROR
 */
STDMETHODIMP CTridentView::ContextSensitiveHelp( BOOL fEnterMode )
{
    return NOERROR;
}



/*
 * CTridentView::GetBorder
 *
 * Purpose:
 *  Returns the rectangle in which the container is willing to
 *  negotiate about an object's adornments.
 *
 * Parameters:
 *  prcBorder       LPRECT in which to store the rectangle.
 *
 * Return Value:
 *  HRESULT         NOERROR if all is well, INPLACE_E_NOTOOLSPACE
 *                  if there is no negotiable space.
 */
STDMETHODIMP CTridentView::GetBorder( LPRECT prcBorder )
{
    if ( NULL == prcBorder )
	{
        return E_INVALIDARG;
	}

    //We return all the client area space
    GetClientRect( prcBorder );
    return NOERROR;
}


/*
 * CTridentView::RequestBorderSpace
 *
 * Purpose:
 *  Asks the container if it can surrender the amount of space
 *  in pBW that the object would like for it's adornments.  The
 *  container does nothing but validate the spaces on this call.
 *
 * Parameters:
 *  pBW             LPCBORDERWIDTHS containing the requested space.
 *                  The values are the amount of space requested
 *                  from each side of the relevant window.
 *
 * Return Value:
 *  HRESULT         NOERROR if we can give up space,
 *                  INPLACE_E_NOTOOLSPACE otherwise.
 */
STDMETHODIMP CTridentView::RequestBorderSpace( LPCBORDERWIDTHS /*pBW*/ )
{
    // We have no border space restrictions
    return NOERROR;
}


/*
 * CTridentView::SetBorderSpace
 *
 * Purpose:
 *  Called when the object now officially requests that the
 *  container surrender border space it previously allowed
 *  in RequestBorderSpace.  The container should resize windows
 *  appropriately to surrender this space.
 *
 * Parameters:
 *  pBW             LPCBORDERWIDTHS containing the amount of space
 *                  from each side of the relevant window that the
 *                  object is now reserving.
 *
 * Return Value:
 *  HRESULT         NOERROR
 */
STDMETHODIMP CTridentView::SetBorderSpace( LPCBORDERWIDTHS /*pBW*/ )
{
	// We turn off the Trident UI so we ignore all of this.

    return NOERROR;
}




/*
 * CTridentView::SetActiveObject
 *
 * Purpose:
 *  Provides the container with the object's IOleInPlaceActiveObject
 *  pointer
 *
 * Parameters:
 *  pIIPActiveObj   LPOLEINPLACEACTIVEOBJECT of interest.
 *  pszObj          LPCOLESTR naming the object.  Not used.
 *
 * Return Value:
 *  HRESULT         NOERROR
 */
STDMETHODIMP CTridentView::SetActiveObject( LPOLEINPLACEACTIVEOBJECT pIIPActiveObj,
											LPCOLESTR /*pszObj*/)
{
	// If we already have an active Object then release it.
    if ( NULL != m_pIOleIPActiveObject )
	{
        m_pIOleIPActiveObject->Release();
	}

    //NULLs m_pIOleIPActiveObject if pIIPActiveObj is NULL
    m_pIOleIPActiveObject = pIIPActiveObj;

    if ( NULL != m_pIOleIPActiveObject )
	{
        m_pIOleIPActiveObject->AddRef();
		m_pIOleIPActiveObject->GetWindow( &m_hWndObj );
	}
    return NOERROR;
}



/*
 * CTridentView::InsertMenus
 *
 * Purpose:
 *  Instructs the container to place its in-place menu items where
 *  necessary in the given menu and to fill in elements 0, 2, and 4
 *  of the OLEMENUGROUPWIDTHS array to indicate how many top-level
 *  items are in each group.
 *
 * Parameters:
 *  hMenu           HMENU in which to add popups.
 *  pMGW            LPOLEMENUGROUPWIDTHS in which to store the
 *                  width of each container menu group.
 *
 * Return Value:
 *  HRESULT         NOERROR
 */
STDMETHODIMP CTridentView::InsertMenus( HMENU /*hMenu*/, LPOLEMENUGROUPWIDTHS /*pMGW*/ )
{
	// We've turned off the Trident Menus so we don't expect any merging to go on!
	return E_NOTIMPL;
}


/*
 * CTridentView::SetMenu
 *
 * Purpose:
 *  Instructs the container to replace whatever menu it's currently
 *  using with the given menu and to call OleSetMenuDescritor so OLE
 *  knows to whom to dispatch messages.
 *
 * Parameters:
 *  hMenu           HMENU to show.
 *  hOLEMenu        HOLEMENU to the menu descriptor.
 *  hWndObj         HWND of the active object to which messages are
 *                  dispatched.
 *
 * Return Value:
 *  HRESULT         NOERROR
 */
STDMETHODIMP CTridentView::SetMenu( HMENU /*hMenu*/, HOLEMENU /*hOLEMenu*/, HWND /*hWndObj*/ )
{
	// We've turned off the Trident Menus so we don't expect any merging to go on!
	return E_NOTIMPL;
}



/*
 * CTridentView::RemoveMenus
 *
 * Purpose:
 *  Asks the container to remove any menus it put into hMenu in
 *  InsertMenus.
 *
 * Parameters:
 *  hMenu           HMENU from which to remove the container's
 *                  items.
 *
 * Return Value:
 *  HRESULT         NOERROR
 */
STDMETHODIMP CTridentView::RemoveMenus( HMENU /*hMenu*/ )
{
	// We've turned off the Trident Menus so we don't expect any merging to go on!
	return E_NOTIMPL;
}




/*
 * CTridentView::SetStatusText
 *
 * Purpose:
 *  Asks the container to place some text in a status line, if one
 *  exists.  If the container does not have a status line it
 *  should return E_FAIL here in which case the object could
 *  display its own.
 *
 * Parameters:
 *  pszText         LPCOLESTR to display.
 *
 * Return Value:
 *  HRESULT         NOERROR if successful, S_TRUNCATED if not all
 *                  of the text could be displayed, or E_FAIL if
 *                  the container has no status line.
 */
STDMETHODIMP CTridentView::SetStatusText( LPCOLESTR pszText )
{
	CFrameWnd* pFrame = GetTopLevelFrame();
	if ( pFrame != NULL )
	{
		USES_CONVERSION;
		pFrame->SetMessageText( OLE2T(pszText) );
	}
    return NOERROR;
}



/*
 * CTridentView::EnableModeless
 *
 * Purpose:
 *  Instructs the container to show or hide any modeless popup
 *  windows that it may be using.
 *
 * Parameters:
 *  fEnable         BOOL indicating to enable/show the windows
 *                  (TRUE) or to hide them (FALSE).
 *
 * Return Value:
 *  HRESULT         NOERROR
 */

STDMETHODIMP CTridentView::EnableModeless( BOOL /*fEnable*/ )
{
    return NOERROR;
}


/*
 * CTridentView::TranslateAccelerator
 *
 * Purpose:
 *  When dealing with an in-place object from an EXE server, this
 *  is called to give the container a chance to process accelerators
 *  after the server has looked at the message.
 *
 * Parameters:
 *  pMSG            LPMSG for the container to examine.
 *  wID             WORD the identifier in the container's
 *                  accelerator table (from IOleInPlaceSite
 *                  ::GetWindowContext) for this message (OLE does
 *                  some translation before calling).
 *
 * Return Value:
 *  HRESULT         NOERROR if the keystroke was used,
 *                  S_FALSE otherwise.
 */
STDMETHODIMP CTridentView::TranslateAccelerator( LPMSG /*pMSG*/, WORD /*wID*/ )
{
	//We should probably forward this to the top level frame!
    return S_FALSE;
}


/*
 * IOleCommandTarget methods.
 */
STDMETHODIMP CTridentView::QueryStatus( const GUID* pguidCmdGroup, ULONG cCmds,
				OLECMD* prgCmds, OLECMDTEXT* pCmdText )
{
    if ( pguidCmdGroup != NULL )
	{
		// It's a nonstandard group!!
        return OLECMDERR_E_UNKNOWNGROUP;
	}

    MSOCMD*     pCmd;
    INT         c;
    HRESULT     hr = S_OK;

    // By default command text is NOT SUPPORTED.
    if ( pCmdText && ( pCmdText->cmdtextf != OLECMDTEXTF_NONE ) )
	{
        pCmdText->cwActual = 0;
	}

    // Loop through each command in the ary, setting the status of each.
    for ( pCmd = prgCmds, c = cCmds; --c >= 0; pCmd++ )
    {
        // By default command status is NOT SUPPORTED.
        pCmd->cmdf = 0;

        switch ( pCmd->cmdID )
        {
			case OLECMDID_SETPROGRESSTEXT:
			case OLECMDID_SETTITLE:
				pCmd->cmdf = OLECMDF_SUPPORTED;
				break;
        }
    }

    return (hr);
}
        
STDMETHODIMP CTridentView::Exec( const GUID* pguidCmdGroup, DWORD nCmdID,
    DWORD /*nCmdexecopt*/, VARIANTARG* pvaIn, VARIANTARG* /*pvaOut*/ )
{
    HRESULT hr = S_OK;

    if ( pguidCmdGroup == NULL )
    {
		USES_CONVERSION;

        switch (nCmdID)
        {
			case OLECMDID_SAVE:
				// We don't support any save stuff!
				hr = OLECMDERR_E_NOTSUPPORTED;
				break;

			case OLECMDID_SETPROGRESSTEXT:
				if ( pvaIn && V_VT(pvaIn) == VT_BSTR )
				{
					CFrameWnd* pFrame = GetTopLevelFrame();
					if ( pFrame != NULL )
					{
						pFrame->SetMessageText( OLE2T(V_BSTR(pvaIn)) );
					}
				}
				else
				{
					hr = OLECMDERR_E_NOTSUPPORTED;
				}
				break;

			case OLECMDID_UPDATECOMMANDS:
				// MFC updates stuff in it's idle so we don't bother forcing the update here
				hr = OLECMDERR_E_NOTSUPPORTED;

				// we do stuffed based on when trident is done
				// displaying.  For some reason, the first time this is
				// called doesn't work.  The second time does, however.  
				// Go figure.

				if( m_nIsVisible < 2 )
				{
					m_nIsVisible ++;
				}
				//TRACE( "Trident is Visible\n" );
				break;

			case OLECMDID_SETTITLE:
				if (pvaIn && V_VT(pvaIn) == VT_BSTR)
				{
					CDocument* pDoc = GetDocument();
					if (pDoc)
					{
						ASSERT_VALID(pDoc);

						pDoc->SetTitle(OLE2T(V_BSTR(pvaIn)));
					}
				}
				else
				{
					hr = OLECMDERR_E_NOTSUPPORTED;
				}
				break;

			default:
				hr = OLECMDERR_E_NOTSUPPORTED;
				break;
        }
    }
    else
    {
        hr = OLECMDERR_E_UNKNOWNGROUP;
    }
    return (hr);
}


int CTridentView::CreateDocObject( TCHAR* pchPath )
{    
	INT	iRet;

	if( m_pSite != NULL )
	{
	    return SITE_ERROR_NONE;        
	}

	//Create and initialise the site for the ActiveX Document
	m_pSite = new CSite( m_hWnd, this );

    if ( NULL == m_pSite )
	{
        return SITE_ERROR_MEMORY;
	}

    m_pSite->AddRef();  //So we can free with Release

    /*
     * Now tell the site to create an object in it using the filename
     * and the storage we opened.  The site will create a sub-storage
     * for the doc object's use.
     */

	// Ask the Site to Create the Activex Document
    if ( ( ( iRet = m_pSite->Create() ) != SITE_ERROR_NONE ) ||
		 ( ( iRet = m_pSite->ObjectInitialize( pchPath ) ) != SITE_ERROR_NONE ) )
	{
        return iRet;
	}

	// If we havent already got the CommandTraget interface then store it away now.
	if ( m_pSite->GetCommandTarget() == NULL )
	{
		IUnknown* lpUnk = m_pSite->GetObjectUnknown();
		IOleCommandTarget* pCommandTarget = NULL;

		HRESULT hr = lpUnk->QueryInterface(IID_IOleCommandTarget, (void **)&pCommandTarget);

		if ( pCommandTarget != NULL )
		{
			m_pSite->SetCommandTarget( pCommandTarget );
		}
	}

    return SITE_ERROR_NONE;        
}


//////////////////////////////////////////////////////////////////////////
// Helper functions on IOleCommandTarget of the object

DWORD CTridentView::GetCommandStatus( ULONG ucmdID )
{
	DWORD dwReturn = 0;
	if ( m_pSite != NULL )
	{
		LPOLECOMMANDTARGET pCommandTarget = m_pSite->GetCommandTarget();
		if ( pCommandTarget != NULL )
		{
			HRESULT hr = S_OK;
			MSOCMD msocmd;
			msocmd.cmdID = ucmdID;
			msocmd.cmdf  = 0;
			hr = pCommandTarget->QueryStatus(&CMDSETID_Forms3, 1, &msocmd, NULL);
			dwReturn = msocmd.cmdf;
		}
	}
	return dwReturn;
}


void CTridentView::ExecCommand(
ULONG		ucmdID, 
VARIANT*	pVar1, 
VARIANT*	pVar2,
BOOL		bShowUI )
{
	HRESULT hr = S_OK;
		
	if ( m_pSite != NULL )
	{
		LPOLECOMMANDTARGET pCommandTarget = m_pSite->GetCommandTarget();

		if ( pCommandTarget != NULL )
		{
			hr = pCommandTarget->Exec(&CMDSETID_Forms3,
						ucmdID,
						( bShowUI ? MSOCMDEXECOPT_PROMPTUSER : MSOCMDEXECOPT_DONTPROMPTUSER ),
						pVar1,
						pVar2);
		}
	}
}


void CTridentView::ExecCommandOnNullSet( 
ULONG		ucmdID, 
VARIANT*	pVar1, 
VARIANT*	pVar2,
BOOL		bShowUI )
{
	if ( m_pSite != NULL )
	{
		LPOLECOMMANDTARGET pCommandTarget = m_pSite->GetCommandTarget();

		if ( pCommandTarget != NULL )
		{
			HRESULT hr = S_OK;
		
			hr = pCommandTarget->Exec(NULL,
						ucmdID,
						( bShowUI ? MSOCMDEXECOPT_PROMPTUSER : MSOCMDEXECOPT_DONTPROMPTUSER ),
						pVar1,
						pVar2);
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
// CTridentView message handlers

void CTridentView::OnDestroy() 
{
	if ( m_pSite != NULL )
	{
		CSite *pSite = m_pSite;			//Prevents reentry
		m_pSite = NULL;

        pSite->Close( FALSE );	// Cleans up the storage and closes object

		ReleaseInterface(pSite);
	}

	CView::OnDestroy();
}


void CTridentView::OnCopy() 
{
    ExecCommand( IDM_COPY );
}


void CTridentView::OnUpdateCopy(CCmdUI* pCmdUI) 
{
    DWORD dwStatus = 0;
    dwStatus = GetCommandStatus( IDM_COPY );
    pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentView::OnCopyAttachment() 
{
    IHTMLElement* piFileAttachment = NULL;
    QueryElement_FileAttachment( m_ptFileAttachment, &piFileAttachment );

    if ( piFileAttachment ) {
        DoCopyAttachment( piFileAttachment );
        piFileAttachment->Release();
    }
}

void CTridentView::OnUpdateCopyAttachment(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( m_bEnableCopyAttachment );
}


void CTridentView::OnSelectAll() 
{
	ExecCommand( IDM_SELECTALL );
}


void CTridentView::OnUpdateSelectAll(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	dwStatus = GetCommandStatus( IDM_SELECTALL );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	//Tell the site to tell the object.
	if ( NULL != m_pSite )
	{
    	m_pSite->UpdateObjectRects();
	}
}


void CTridentView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	// Give the focus to the ActiveX Document window
    if ( m_hWndObj != NULL )
	{
		::SetFocus( m_hWndObj );
	}
}


BOOL CTridentView::OnEraseBkgnd(CDC* pDC) 
{
	// Prevent MFC from annoyingly erasing the background when it thinks it should
	//  by blocking the call to CView::OnEraseBkgnd
	return TRUE;
}


void CTridentView::OnInitialUpdate() 
{
	if ( m_fCreated == TRUE )
		return;
	CView::OnInitialUpdate();
	m_fCreated = TRUE;
	
	// instantiate trident
	INT iRet;
	if ( ( iRet = CreateDocObject( NULL ) ) != SITE_ERROR_NONE )
	{
		if( iRet == SITE_ERROR_MEMORY )
		{				 
			// Destroy this window
			::ErrorDialog( IDS_ERROR_NOT_ENOUGH_MEMORY );
		}
		else if ( iRet == SITE_ERROR_NO_MSHTML_DLL )
		{
			// Destroy this window
			::ErrorDialog( IDS_ERR_IE_VERSION );
		}
		else if ( iRet == SITE_ERROR_INVALID_OBJECT )
		{
			// Destroy this window
			::ErrorDialog( IDS_ERR_INVALID_OBJECT );
		}
		else if ( iRet == SITE_ERROR_INVALID_OLE_OBJECT )
		{
			// Destroy this window
			::ErrorDialog( IDS_ERR_INVALID_OLE_OBJECT );
		}
		else if ( iRet == SITE_ERROR_UNABLE_TO_LOAD_FILE )
		{
			// Destroy this window
			::ErrorDialog( IDS_ERR_UNABLE_TO_LOAD_FILE );
		}
		
		// fool callers into not fixing up the source on errors
		m_bFixedupSource = TRUE;
		return;
	}

	if (!LoadMessage())
		return;

	// We created the thing, now activate it with "Show"
	if( GetStyle() & WS_VISIBLE )
		m_pSite->Activate( OLEIVERB_SHOW );
	else
		m_pSite->Activate( OLEIVERB_HIDE );
}

BOOL CTridentView::LoadMessage()
{
	CFile		theFile;
	TCHAR		szDrive[_MAX_DRIVE + 1];
	TCHAR		szPath[_MAX_PATH + 1];
	TCHAR		szTmpReal[_MAX_PATH + 1];
	TCHAR		szFile[_MAX_FNAME + 1];
	TCHAR		szExt[_MAX_EXT + 1];
	CString		szText;
	CString		szStyleSheetFormat;
	LPTSTR		pBuf;
	CFile		theStyleSheetFormatFile;
	CString		szFileName;

	m_bFixedupSource = FALSE;

	// Get rid of previous temp file
	unlink(m_szTmpFile);

	// get the temporary filename	
	//extern CString TempDir - declared in fileutil.h
	strcpy(szPath, TempDir);
	if ( !GetTempFileName( szPath, "eud", ++s_uTmpFileUnique, szTmpReal ) )
	{
		::ErrorDialog( IDS_ERR_TEMP_FILE );
		m_bFixedupSource = TRUE;
		return FALSE;
	}

	_splitpath( szTmpReal, szDrive, szPath, szFile, szExt );

	// change the extension to .htm
	m_szBaseFileName = szFile;
	m_szBaseFileName += ".";
	m_szBaseFileName += CRString( IDS_HTM_EXTENSION );
	m_szTmpFile = szDrive;
	m_szTmpFile += szPath + m_szBaseFileName;
	
	// get the format string for the style sheet
	szStyleSheetFormat = "";
	try
	{
		szFileName = EudoraDir + "read.css";
		
		if( theStyleSheetFormatFile.Open( szFileName, CFile::modeRead | CFile::typeBinary ) )
		{
			pBuf = szStyleSheetFormat.GetBufferSetLength( theStyleSheetFormatFile.GetLength() + 1 );
			
			if( pBuf )
			{
				theStyleSheetFormatFile.Read( pBuf, theStyleSheetFormatFile.GetLength() );
				pBuf[ theStyleSheetFormatFile.GetLength() ] = '\0';
			}
		}
	}
	catch( CException* pExp )
	{
		szStyleSheetFormat = "";
		szStyleSheetFormat.FreeExtra();
		pExp->Delete();
	}


	if (szStyleSheetFormat.IsEmpty())
	{
		// there's no read.css, so get it from the .ini 
		// file or the resource file.
		szStyleSheetFormat = GetIniString(IDS_INI_READMESSAGE_STYLE_SHEET);
	}

	
	// create the file
	CFileException FE;
	if ( ! theFile.Open( m_szTmpFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary, &FE ))
	{
		::ErrorDialog( IDS_ERR_OPEN_TEMP_FILE, FE.m_cause, FE.m_lOsError, (LPCTSTR)FE.m_strFileName );
		
		// fool callers into not fixing up the source on errors
		m_bFixedupSource = TRUE;
		return FALSE;
	}
	if (!WriteTempFile(theFile, szStyleSheetFormat))
	{
		::ErrorDialog( IDS_ERR_WRITE_TEMP_FILE );
		
		// fool callers into not fixing up the source on errors
		m_bFixedupSource = TRUE;
		return FALSE;
	}

	try
	{
		theFile.Write("</body></html>", 14);
		theFile.Close();	
	}
	catch (...)
	{
		// Probably not a big deal if we can't write out the ending tags.
		// It may be a big deal if we can't close the file, but it still might work.
	}

	if (!m_pSite)
	{
		// Uh oh, should have created site control by now.
		ASSERT(0);
		return FALSE;
	}

	if (FAILED(m_pSite->Load((LPTSTR)(LPCTSTR)m_szTmpFile)))
		return FALSE;

	return TRUE;
}


void CTridentView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


void CTridentView::OnSpecialAddAsRecipient()
{
	HRESULT hr;
	IHTMLDocument2* pDoc = NULL;
	IHTMLSelectionObject* pSel = NULL;
	IHTMLTxtRange* pRange = NULL;
	CBstr type;
	CBstr text;

	// Get document
	pDoc = m_pSite->GetDoc2();

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( SUCCEEDED( hr ) &&  pSel ) 
	{
		// Get type
		hr = pSel->get_type( BSTRARG( type ) );

		if ( ( S_OK == hr )  && ! type.IsNull() )
		{
			CString strType = type;

			// If type isnt text, bail
			if( strType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 )
			{
				// Create range
				hr = pSel->createRange((IDispatch**)&pRange);
				
				if( ( S_OK == hr ) && pRange )
				{
					// Get text
					hr = pRange->get_text( BSTRARG( text ) );
					if ( S_OK == hr && !text.IsNull() )
					{
						
						CString strText = text;
						
						if( ( strText != "" ) && ( strText.Find("\r\n" ) == -1 ) )
						{
							g_theRecipientDirector.AddCommand( strText );
						}
					}

					pRange->Release();
				}
			}
		}

		pSel->Release();
	}
}


void CTridentView::OnUpdateAddAsRecipient(CCmdUI* pCmdUI)
{	
	pCmdUI->Enable( HasSelectedText() );
}



//  FixupSource:
//
//  This function is called after trident has loaded an parsed the document.
//  It's purpose is to play nasty games with the source.  For instance, an
//  HTML message is sent to us from OE, it has a <BODY> in the body of the message.
//  However, when we add the headers to the message so trident creates another
//  <body> tag at the start of the message.  We end up with two body tags.  So,
//  to keep from losing attributes, we need to copy the attributes of the tag that
//  came in the original message to the tag trident put at the beginning of the
//  message when we put in the headers.  Whew.  It also adds the style sheet class
//  to elements that have the CITE attribute, etc.

long CTridentView::FixupSource(
WPARAM,
LPARAM )
{
	IHTMLDocument2*			pDoc;
	IHTMLElementCollection* pColl;
	IDispatch*				pDisp; 
	IHTMLElement*			pElement;
	HRESULT					hr;
	VARIANT					vType;
	INT						i;
	LONG					celem;
	CString					strTag;
	VARIANT					varIndex;
	VARIANT					var2;
	CBstr					cbstr;
	CBstr					cbstrText( IDS_TEXT );
	CBstr					cbstrLink( "link" );
	CBstr					cbstrAlink( "alink" );
	CBstr					cbstrVlink( "vlink" );
	CBstr					cbstrBackground( "background" );
	CBstr					cbstrBgColor( "bgColor" );
	CBstr					cbstrTopMargin( "topMargin" );
	CBstr					cbstrLeftMargin( "leftMargin" );
	CBstr					cbstrRightMargin( "rightMargin" );
	CBstr					cbstrBottomMargin( "bottomMargin" );
	CBstr					cbstrCite( IDS_CITE );
	CBstr					cbstrType( "TYPE" );
	CBstr					cbstrClass( "CLASS" );
	BOOL					bAddClass;
	INT						iSize;
	IHTMLBodyElement*		pFirstBody;
	CRString				szBodyTag( IDS_HTML_BODY );
	CRString				szBQTag( IDS_HTML_BLOCKQUOTE );
	CRString				szDivTag( IDS_HTML_DIV );
	CRString				szCite( IDS_CITE );
	
	USES_CONVERSION;

	iSize = GetIniShort( IDS_INI_MESSAGE_FONT_SIZE );

	if( iSize < 0 )
	{
		iSize = 0;
	}
	else if( iSize >= FONTSIZE_COUNT )
	{
		iSize = FONTSIZE_COUNT - 1;
	}

	ExecCommand( IDM_BASELINEFONT1 + iSize );

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if ( !pDoc ) 
	{
		return 0;
	}


	hr = pDoc->get_all( &pColl );

	pFirstBody = NULL;

	VariantInit( &var2 );
	VariantInit( &vType );

	if( ( hr == S_OK ) && pColl )
	{
		hr = pColl->get_length( &celem );

		if ( hr == S_OK )
		{
			for ( i=0; i< celem; i++ )
			{
				varIndex.vt = VT_UINT;
				varIndex.lVal = i;

				VariantClear( &var2 );

				hr = pColl->item( varIndex, var2, &pDisp );

				if( ( hr == S_OK ) && pDisp ) 
				{
					hr = pDisp->QueryInterface( IID_IHTMLElement, (void **)&pElement );

					if( ( hr == S_OK ) && pElement )
					{
						hr = pElement->get_tagName( ( BSTRARG ) cbstr );
						strTag = cbstr;
													
						if( ( hr == S_OK ) && 
							( strTag.CompareNoCase( szBodyTag ) == 0 ) )
						{
							if( pFirstBody == NULL )
							{
								hr = pElement->QueryInterface( IID_IHTMLBodyElement, (void **)&pFirstBody );
								ASSERT( hr == S_OK );
							}
							else
							{
								// copy the attributes
								
								VariantInit( &var2 );
								hr = pElement->getAttribute( cbstrBackground, VARIANT_FALSE, &var2  );
								
								if( ( hr == S_OK ) && ( var2.vt == VT_BSTR ) )
								{
									hr = pFirstBody->put_background( var2.bstrVal );
									VariantClear( &var2 );
								}
								
//								hr = pBody->get_bgProperties( &bstr );
								
//								if( ( hr == S_OK ) && bstr )
//								{
//									hr = pFirstBody->put_bgProperties( bstr );
//									::SysFreeString( bstr );
//									bstr = NULL;
//								}
								

								hr = pElement->getAttribute( cbstrTopMargin, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_topMargin( var2 );
									VariantClear( &var2 );
								}
								
								hr = pElement->getAttribute( cbstrLeftMargin, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_leftMargin( var2 );
									VariantClear( &var2 );
								}
								
								hr = pElement->getAttribute( cbstrRightMargin, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_rightMargin( var2 );
									VariantClear( &var2 );
								}

								hr = pElement->getAttribute( cbstrBottomMargin, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_bottomMargin( var2 );
									VariantClear( &var2 );
								}

//								pBody->get_noWrap( &vb2 );
//								
//								if( hr == S_OK )
//								{
//									hr = pFirstBody->get_noWrap( &vb1 );
//									if( ( hr != S_OK ) || ( vb1 != vb2 ) )
//									{ 
//										hr = pFirstBody->put_noWrap( vb2 );
//									}
//								}

								hr = pElement->getAttribute( cbstrBgColor, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_bgColor( var2 );
									VariantClear( &var2 );
								}

								hr = pElement->getAttribute( cbstrText, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_text( var2 );
									VariantClear( &var2 );
								}

								hr = pElement->getAttribute( cbstrLink, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_link( var2 );
									VariantClear( &var2 );
								}

								hr = pElement->getAttribute( cbstrAlink, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_aLink( var2 );
									VariantClear( &var2 );
								}
								
								hr = pElement->getAttribute( cbstrVlink, VARIANT_FALSE, &var2 );

								if( ( hr == S_OK ) && ( var2.vt != VT_NULL ) )
								{
									hr = pFirstBody->put_vLink( var2 );
									VariantClear( &var2 );
								}
							}

						}
	
						if( ( hr == S_OK ) && 
							(	( strTag.CompareNoCase( szBQTag ) == 0 ) || 
								( strTag.CompareNoCase( szDivTag ) == 0 ) ) )
						{
							
							hr = pElement->getAttribute ( cbstrCite , VARIANT_FALSE, &vType ) ;								
							bAddClass = FALSE;

							if( ( hr == S_OK ) && ( vType.vt == VT_BSTR ) )
							{
								// there is a CITE attribute so
								// add the correct class attribute
								bAddClass = TRUE;
								VariantClear( &vType );
							}
							else
							{
								VariantClear( &vType );
								hr = pElement->getAttribute ( cbstrType , VARIANT_FALSE, &vType ) ;								

								if( ( hr == S_OK ) && ( vType.vt == VT_BSTR ) )
								{
									strTag = vType.bstrVal;

									if( strTag.CompareNoCase( szCite ) == 0 ) 
									{
										// there is a TYPE=CITE attribute so
										// add the correct class attribute
										bAddClass = TRUE;
									}
								}
								
								VariantClear( &vType );
							}

							if( bAddClass )
							{
								hr = pElement->put_className( cbstrCite );
							}
						}
						
						pElement->Release();
					}
					pDisp->Release();
				}
			}
		}

		pColl->Release();
	}

	if( pFirstBody )
	{
		pFirstBody->Release();
	}

	m_bFixedupSource = TRUE;
	return 1;
}



// converts all the URIs to local file references based on the "Embedded Content
// on input *ppHTML should have the raw html
// on return *ppHTML will point to a buffer that contains morphed html
// *ppHTML may be changed as a side effect of this call
void CTridentView::MorphMHTML( char ** ppHTML )
{
	if ( ! *ppHTML ) return;	// did we get raw html?

	char * pCookedHTML = NULL;

	long lCookedHTMLSize = ConvertURIs( *ppHTML, &pCookedHTML );
	if ( lCookedHTMLSize > 0 && pCookedHTML )
	{
		delete [] *ppHTML;									// delete the old buffer
		*ppHTML = new char[ lCookedHTMLSize ];				// alloc a new buffer
		if ( *ppHTML )
			memcpy( *ppHTML, pCookedHTML, lCookedHTMLSize );// fill in the new buffer
		free( pCookedHTML );								// all done 
	}
}


// QuietPrint -- prints without bringing up the print dialog.
BOOL CTridentView::QuietPrint()
{
	return Print(FALSE);
}

BOOL CTridentView::Print(BOOL bDialog)
{
	CString	szFontName;
	INT		iSize;
	MSG		msg;

	SetRedraw( FALSE );

	while( !IsReady() && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
	{
		// we have to wait until the trident control is
		// is ready to go before we can do anything with it.

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// set the font and size
	szFontName = GetIniString( IDS_INI_PRINTER_FONT );
	iSize = GetIniShort( IDS_INI_PF_SIZE );

	SetFont( szFontName, iSize );

	// Temporarily set registry settings for header/footer.
	// Hope that we don't crash while printing.
	BYTE OldHeader[128];
	BYTE OldFooter[128];
	DWORD OldHeaderLen;
	DWORD OldFooterLen;
	DWORD HeaderValueType;
	DWORD FooterValueType;
	BOOL bSetHeader = FALSE;
	BOOL bSetFooter = FALSE;
	CRString IEPageSetupKeyName(IDS_REG_IE_PAGE_SETUP);
	CRString HeaderValueName(IDS_REG_IE_HEADER);
	CRString FooterValueName(IDS_REG_IE_FOOTER);
	HKEY hkIEPageSetup;
	const BOOL bDoHeaderFooter = GetIniShort(IDS_INI_PRINT_HEADERS);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, IEPageSetupKeyName, NULL, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkIEPageSetup) == ERROR_SUCCESS)
	{
		OldHeaderLen = sizeof(OldHeader);
		if (RegQueryValueEx(hkIEPageSetup, HeaderValueName, NULL, &HeaderValueType, OldHeader, &OldHeaderLen) == ERROR_SUCCESS)
		{
			CDocument* pDoc = GetDocument();
			CString Title;
			if (pDoc && bDoHeaderFooter)
			{
				Title = pDoc->GetTitle();
				Title += CRString(IDS_IE_PRINT_HEADER_EXTRA);
			}

			if (RegSetValueEx(hkIEPageSetup, HeaderValueName, NULL, REG_SZ, (LPBYTE)(LPCTSTR)Title, Title.GetLength()) == ERROR_SUCCESS)
			{
				bSetHeader = TRUE;
			}
		}

		OldFooterLen = sizeof(OldFooter);
		if (RegQueryValueEx(hkIEPageSetup, FooterValueName, NULL, &FooterValueType, OldFooter, &OldFooterLen) == ERROR_SUCCESS)
		{
			CString NewFooter;
			if (bDoHeaderFooter)
			{
				NewFooter.Format(CRString(IDS_PRINT_FOOTER), GetReturnAddress());
				NewFooter += CRString(IDS_IE_PRINT_FOOTER_EXTRA);
			}
			if (RegSetValueEx(hkIEPageSetup, FooterValueName, NULL, REG_SZ, (LPBYTE)(LPCTSTR)NewFooter, NewFooter.GetLength()) == ERROR_SUCCESS)
			{
				bSetFooter = TRUE;
			}
		}
	
		VERIFY(RegCloseKey(hkIEPageSetup) == ERROR_SUCCESS);
	}

	// print
	ExecCommandOnNullSet( OLECMDID_PRINT, NULL, NULL, bDialog );

	// Set registry entries for header/footer back to what they were before
	if (RegOpenKeyEx(HKEY_CURRENT_USER, IEPageSetupKeyName, NULL, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkIEPageSetup) != ERROR_SUCCESS)
	{
		// Uh oh.  We just could open it a second ago.  How come we can't now?
		// This is bad because it means we've just corrupted the user's header/footer settings in IE.
		ASSERT(0);
	}
	else
	{
		if (bSetHeader)
			VERIFY(RegSetValueEx(hkIEPageSetup, HeaderValueName, NULL, HeaderValueType, OldHeader, OldHeaderLen) == ERROR_SUCCESS);
		if (bSetFooter)
			VERIFY(RegSetValueEx(hkIEPageSetup, FooterValueName, NULL, FooterValueType, OldFooter, OldFooterLen) == ERROR_SUCCESS);

		VERIFY(RegCloseKey(hkIEPageSetup) == ERROR_SUCCESS);
	}

	// reset the font and size
	if( GetIniShort( IDS_INI_USE_PROPORTIONAL_AS_DEFAULT ) )
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FONT );
	}
	else
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FIXED_FONT );
	}
	
	iSize = GetIniShort( IDS_INI_MESSAGE_FONT_SIZE );

	SetFont( szFontName, iSize );
	
	SetRedraw( TRUE );	
	
	return TRUE;
}

void CTridentView::SetFont(
const CString&	szFontName,
INT				iSize )
{
	HRESULT							hr;
	IHTMLDocument2*					pDoc;
	IHTMLStyleSheetsCollection*		pStyleSheetCollection;
	IHTMLRuleStyle*					pRule;
	IHTMLStyleSheetRule*			pStyleSheetRule;
	IHTMLStyleSheet*				pStyleSheet;
	IHTMLStyleSheetRulesCollection*	pStyleSheetRulesCollection;
	LONG							lCount;
	LONG							lIndex;
	LONG							lRuleCount;
	LONG							lRuleIndex;
	VARIANT							varIndex;
	VARIANT							varResult;
	CBstr							cbstr;
	CString							szText;
	CRString						szHTML( IDS_MIME_HTML );

	USES_CONVERSION;

	// set the font size
	if( iSize < 0 )
	{
		iSize = 0;
	}
	else if( iSize >= FONTSIZE_COUNT )
	{
		iSize = FONTSIZE_COUNT;
	}

	ExecCommand( IDM_BASELINEFONT1 + iSize );

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if ( !pDoc ) 
		return;

	// first, we need to find the style sheet for the default font 
	// so get the style sheet collection to search
	
	pStyleSheetCollection = NULL;
	hr = pDoc->get_styleSheets( &pStyleSheetCollection );

	if ( ( hr != S_OK ) || ( pStyleSheetCollection == NULL ) ) 
	{
		// no style sheet collection was found
		return;
	}

	lCount = 0;

	hr = pStyleSheetCollection->get_length( &lCount );
	
	if( ( hr != S_OK ) || ( lCount == 0 ) )
	{
		pStyleSheetCollection->Release();
		return;
	}


	// loop through the style sheet collection
	for( lIndex = 0; lIndex < lCount; lIndex ++ )
	{
		if ( hr != S_OK )
		{
			break;
		}

		// get the next style sheet
		varIndex.vt = VT_UINT;
		varIndex.lVal = lIndex;
		VariantInit( &varResult );

		hr = pStyleSheetCollection->item( &varIndex, &varResult );

		if ( ( hr != S_OK ) || ( varResult.vt != VT_DISPATCH ) )
		{
			if( hr == S_OK )
			{
				VariantClear( &varResult );
			}

			break;
		}
				
		pStyleSheet = NULL;

		hr = varResult.pdispVal->QueryInterface( IID_IHTMLStyleSheet, (void **)&pStyleSheet );		
		VariantClear( &varResult );

		if ( ( hr != S_OK ) || ( pStyleSheet == NULL ) )
		{
			break;
		}

		// get the rules collection for this style sheet
		pStyleSheetRulesCollection = NULL;
		hr = pStyleSheet->get_rules( &pStyleSheetRulesCollection );

		if ( ( hr != S_OK ) || ( pStyleSheetRulesCollection == NULL ) )
		{
			pStyleSheet->Release();
			break;
		}
		
		lRuleCount = 0;
		hr = pStyleSheetRulesCollection->get_length( &lRuleCount );					
		
		if( hr != S_OK )
		{
			pStyleSheetRulesCollection->Release();
			pStyleSheet->Release();
			break;
		}

		// look through the rules for the default font
		for( lRuleIndex = 0; lRuleIndex < lRuleCount; lRuleIndex ++ )
		{
			pStyleSheetRule = NULL;

			hr = pStyleSheetRulesCollection->item( lRuleIndex, &pStyleSheetRule );

			if( ( hr != S_OK ) || ( pStyleSheetRule == NULL ) )
			{
				break;
			}

			hr =  pStyleSheetRule->get_selectorText( ( BSTRARG ) cbstr );

			if( ( hr != S_OK ) || cbstr.IsNull() )
			{
				pStyleSheetRule->Release();
				continue;
			}

			szText = cbstr;
			
			// we're looking for the default style sheet rule -- HTML
			if( szText.CompareNoCase( szHTML ) ) 
			{
				pStyleSheetRule->Release();
				continue;
			}

			pRule = NULL;

			// get the associated style
			hr = pStyleSheetRule->get_style( &pRule );
			
			if( ( hr != S_OK ) || ( pRule == NULL ) )
			{
				pStyleSheetRule->Release();
				continue;
			}

			// get the font name
			szText = "";
			hr = pRule->get_fontFamily( ( BSTRARG ) cbstr );
			szText = cbstr;

			if( szText != "" )
			{
				szText.Format( "%s", ( LPCSTR )szFontName );
				cbstr = A2BSTR( szText );
				hr = pRule->put_fontFamily( cbstr );
				lCount = 0;  // we're done
			}

			pRule->Release();
			pStyleSheetRule->Release();
		}

		pStyleSheetRulesCollection->Release();
		pStyleSheet->Release();
	}

	pStyleSheetCollection->Release();
}


BOOL CTridentView::IsEditing()
{
	return FALSE;
}


BOOL CTridentView::PreTranslateMessage(MSG* pMsg) 
{
    CWnd*               pWnd;
    CFrameWnd*  pFrameWnd;
    CPoint              pt;

	if( ::IsChild( GetSafeHwnd(), pMsg->hwnd ) )
    {
        // see if the user hit return while highlighting a url/attachment in 
        // browse mode
        if( !IsEditing() && ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_RETURN ) )
        {
            // yep. launch it if necessary.
            if( LaunchURL() )
            {
                return TRUE;
            }
        }

        // BOG: This prevents default handling of rbuttondown by Trident, which changes the
        // insertion point in response to this message, which was causing text selections
        // to be lost. Now only lbuttondown changes the insertion point, but I don't see a
        // problem w/that---the Paige impl is the same.

        if ( pMsg->message == WM_RBUTTONDOWN )
            return TRUE;

        // do the context menu
        if ( pMsg->message == WM_RBUTTONUP ) {

            VERIFY( pWnd = CWnd::FromHandle( pMsg->hwnd ) );
            VERIFY( pFrameWnd = GetParentFrame() );
            pt.x = LOWORD( pMsg->lParam );
            pt.y = HIWORD( pMsg->lParam );

            // BOG: see if user is right-clicking an attachment
            if ( QueryElement_FileAttachment( pt ) ) {
                m_ptFileAttachment = pt;
                m_bEnableCopyAttachment = TRUE;
            }
            else
                m_bEnableCopyAttachment = FALSE;

            // wm_contextmenu handlers expect screen coordinates
            pWnd->ClientToScreen( &pt );
            pFrameWnd->PostMessage( WM_CONTEXTMENU, WPARAM(GetSafeHwnd()),
                                    MAKELPARAM(pt.x, pt.y) );

            return TRUE;
        }

        // see if the user pressed the left mouse button while in browse mode.  
        // if so, try to launch the url/attachment.  NOTE:  Processing the 
        // button up message will not work -- trident will process the button 
        // down message if we dont.
        if( !IsEditing() && ( pMsg->message == WM_LBUTTONDOWN ) )
        {
            pt.x = LOWORD( pMsg->lParam );
            pt.y = HIWORD( pMsg->lParam );

            if( LaunchURL( pt ) )
            {
                return TRUE;
            }
        }
    }

    if ((! IsEditing()) && (WM_KEYDOWN == pMsg->message))
    {
        //
        // Look for plain Arrow keystrokes and Ctrl+Arrow
        // keystrokes.  If setting is enabled, translate those
        // keystrokes into "next/previous message" commands.  Also,
        // look for Shift+Space keystrokes and translate those
        // into "change status to unread" commands.
        //
        switch (pMsg->wParam)
        {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        {
            BOOL bCtrlDown = (GetKeyState(VK_CONTROL) < 0);
            BOOL bUsePlainArrows = GetIniShort(IDS_INI_PLAIN_ARROWS) != 0;
            BOOL bUseCtrlArrows = GetIniShort(IDS_INI_CONTROL_ARROWS) != 0;

            if ((bUsePlainArrows && !bCtrlDown) ||
                (bUseCtrlArrows && bCtrlDown))
            {
                SendMessage(WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP)
                            ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);

                return TRUE;    // don't let Trident control see this message
            }
        }
        break;
        case VK_SPACE:
            if (ShiftDown())
            {
                                //
                                // A Shift+Space on a read-only view translates into a
                                // "toggle read status" command.
                                //
                SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE);
                return TRUE;            // don't let the Trident control see this message
            }
            break;
        default:
            break;
        }
    }

    if (WM_SYSKEYDOWN == pMsg->message)
    {
        //
        // Look for Alt+Arrow keystrokes and if setting is enabled, 
        // translate those keystrokes into "next/previous message"
        // commands.
        //
        switch (pMsg->wParam)
        {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        {
            BOOL bCtrlDown = (GetKeyState(VK_CONTROL) < 0);
            BOOL bUseAltArrows = GetIniShort(IDS_INI_ALT_ARROWS) != 0;

            if (bUseAltArrows && !bCtrlDown)
            {
                SendMessage(WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP)
                            ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);

                return TRUE;    // don't let Trident control see this message
            }
        }
        break;
        default:
            break;
        }
    }
        
    return CView::PreTranslateMessage(pMsg);
}


void CTridentView::OnViewSource()
{
	ExecCommand( IDM_VIEWSOURCE );
}

void CTridentView::OnSendToBrowser()
{
	ShellExecute(AfxGetMainWnd()->m_hWnd, NULL, m_szTmpFile, NULL, NULL, SW_SHOWNORMAL);
}


BOOL CTridentView::LaunchURL(
CPoint point) 
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLElement*			pElement;
	IHTMLElement*			pParent;
	CBstr					cbstr;
	CString					szValue;
	BOOL					bRet;
	CString					szName;
	CString					szHRef;

	szHRef = "";
	szName = "";
	bRet = FALSE;

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if ( !pDoc ) 
	{
		return FALSE;
	}

	// get the element we clicked on
	pElement = NULL;

	hr = pDoc->elementFromPoint( point.x, point.y, &pElement );
	
	// if it's a plugin, then handle that first
	if( DoOnDisplayPlugin( pElement ) )
	{
		return TRUE;
	}

	if( ( hr == S_OK ) && ( pElement != NULL ) )
	{
		// get the tag
		hr = pElement->get_tagName( ( BSTRARG )cbstr );
		szValue = cbstr;

		if( szValue.CompareNoCase( CRString( IDS_HTML_IMG ) ) == 0 ) 
		{
			// it's an image
			if( IsEditing() )
			{
				// edit the image
				pElement->Release();
				ExecCommand( IDM_IMAGE, NULL, NULL, TRUE );
				GetDocument()->SetModifiedFlag();
				return TRUE;
			}
			else
			{
				// In browse mode, we need to treat an image embedded in an anchor as if
				// the user had clicked on the url itself.  So swap the current element 
				// for the parent.

				pParent = NULL;
				hr = pElement->get_parentElement( &pParent );

				if( ( hr == S_OK ) && ( pParent != NULL ) )
				{
					pElement->Release();
					pElement = pParent;
					hr = pElement->get_tagName( ( BSTRARG ) cbstr );
					szValue = cbstr;
				}
			}
		}
		
		if( szValue.CompareNoCase( CRString( IDS_HTML_ANCHOR ) ) == 0 )
		{
			if( IsEditing() )
			{
				// edit the anchor
				pElement->Release();
				ExecCommand( IDM_HYPERLINK, NULL, NULL, TRUE );
				GetDocument()->SetModifiedFlag();
				return TRUE;
			}

			// If we're in browse mode, launch it.
			if( LaunchURL( pElement ) )
			{
				return TRUE;
			}

			pElement = NULL;
		}
	}

	if( pElement )
	{
		pElement->Release();
	}

	return FALSE;
}


BOOL CTridentView::IsReady()
{
	// we can only return true if trident has completely parsed the 
	// source and we have fixed it up.
	return m_bFixedupSource;
}


bool CTridentView::IsReadOnly() 
{ 
	return !IsEditing(); 
}


// OnFilePrint: brings up the print dialog before printing
void CTridentView::OnFilePrint() 
{
	Print(TRUE);
}


IHTMLElement* CTridentView::GetSelectedElement(
LPCSTR	szTag )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLElementCollection* pCollection;
	IHTMLSelectionObject*	pSel;
	IDispatch*				pRangeID;
	IHTMLTxtRange*			pRange;
	IHTMLTxtRange*			pElementRange;
	IHTMLElement*			pElement;
	IDispatch*				pDispatch;
	VARIANT					varIndex;
	VARIANT					var;
	CString					szText;
	CBstr					cbstr;
	BOOL					bDone;
	LONG					lCount;
	LONG					lIndex;
	LONG					l;
	CBstr					cbstrEndToStart( IDS_END_TO_START );
	CBstr					cbstrStartToEnd( IDS_START_TO_END );

	USES_CONVERSION;

	if( ( szTag != NULL ) && ( *szTag == '\0' ) )
	{
		szTag = NULL;
	}

	pSel = NULL;
	pRange = NULL;
	pElementRange = NULL;
	pElement = NULL;

	// Get document
	VERIFY( pDoc = m_pSite->GetDoc2() );

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( ( hr != S_OK ) || !pSel )
	{
		return NULL;
	}

	// Get the selected range IDispatch ptr
	hr = pSel->createRange(&pRangeID);
	pSel->Release();

	if( ( S_OK != hr ) || !pRangeID )
	{
		return NULL;
	}

	//What we got was just the IDispatch interface ptr, now query it for the actual 
	//IHtmlTextRange ptr.
	hr = pRangeID->QueryInterface(IID_IHTMLTxtRange, (LPVOID *)&pRange);
	pRangeID->Release();

	if( ( S_OK != hr) && !pRange)
	{
		return NULL;
	}

	// get another copy of it
	hr = pRange->duplicate( &pElementRange );

	if( ( S_OK != hr ) || !pElementRange )
	{
		return NULL;
	}

	// get the element collection
	hr = pDoc->get_all( &pCollection );

	VariantInit( &var );

	if( ( hr == S_OK ) && pCollection )
	{
		hr = pCollection->get_length( &lCount );

		if ( hr == S_OK )
		{
			// loop through all the elements
			for ( lIndex = 0, bDone = FALSE; ( lIndex < lCount ) && !bDone; lIndex++ )
			{
				varIndex.vt = VT_UINT;
				varIndex.lVal = lIndex;

				hr = pCollection->item( varIndex, var, &pDispatch );

				if( ( hr == S_OK ) && pDispatch ) 
				{
					// get next the element
					hr = pDispatch->QueryInterface( IID_IHTMLElement, (void **)&pElement );

					if( ( hr == S_OK ) && pElement )
					{
						hr = pElement->get_tagName( ( BSTRARG ) cbstr );
						szText = cbstr;

						// If we are looking for a specific tag, see if it 
						// matches
						if( ( hr == S_OK ) && 
							( ( szTag == NULL ) ||
							  ( szText.CompareNoCase( szTag ) == 0 ) ) )
						{
							// get the range for the element
							hr = pElementRange->moveToElementText( pElement );

							if( hr == S_OK )
							{
								// see it overlaps the selection
								l = 0;

								hr = pRange->compareEndPoints( cbstrStartToEnd, pElementRange, &l );

								if( ( hr == S_OK ) && ( l <= 0 ) )
								{
									// pRange.Start <= pElementRange.End
	
									hr = pRange->compareEndPoints( cbstrEndToStart, pElementRange, &l );
									
									if( ( hr == S_OK ) && ( l >= 0 ) )
									{
										// pRange.End >= pElementRange.Start
										// found it
										bDone = TRUE;
									}
								}
							}
						}

						if( !bDone )
						{
							pElement->Release();
							pElement = NULL;
						}
					}

					pDispatch->Release();
				}
			}
		}
	}

	pCollection->Release();
	pRange->Release();
	pElementRange->Release();
	return pElement;
}



BOOL CTridentView::LaunchURL()
{
	IHTMLElement*			pElement;

	// see if there is an anchor selected
	pElement = GetSelectedElement( CRString( IDS_HTML_ANCHOR ) );

	if( pElement == NULL )
	{
		return FALSE;
	}

	// see if it's a plugin
	if( DoOnDisplayPlugin( pElement ) )
	{
		return TRUE;
	}

	// launch the url/attachment
	return LaunchURL( pElement );
}


BOOL CTridentView::LaunchURL(
IHTMLElement*	pElement )
{
	IHTMLAnchorElement*		pAnchor;
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLElementCollection*	pCollection;
	IDispatch*				pDispatch;
	VARIANT					varIndex;
	VARIANT					var;
	BOOL					bRet;
	LONG					lCount;
	LONG					l;
	CString					szValue;
	CString					szHRef;
	CString					szName;
	CBstr					cbstr;

	ASSERT( pElement );
	VERIFY( pDoc = m_pSite->GetDoc2() );

	szHRef = "";
	szName = "";
	bRet = FALSE;

	// get the anchor
		
	pAnchor = NULL;
	hr = pElement->QueryInterface( IID_IHTMLAnchorElement, ( void** )( &pAnchor ) );
	
	if( ( hr == S_OK ) && ( pAnchor != NULL ) )
	{
		// get the HREF
		hr = pAnchor->get_href( ( BSTRARG) cbstr );
		
		szHRef = cbstr;

		hr = pAnchor->get_nameProp( ( BSTRARG ) cbstr );
		
		if( ( hr == S_OK ) && ! cbstr.IsNull() )
		{
			szValue = cbstr;

			// see if this is an internal link to another portion of this document
			if( strnicmp( m_szBaseFileName, szValue, m_szBaseFileName.GetLength() ) == 0 )
			{
				// includes the '#'
				szName = szValue.Right( szValue.GetLength() - m_szBaseFileName.GetLength() );
			}
		}

		pAnchor->Release();					
	}

	pElement->Release();

	if( szName != "" )
	{
		// it's an internal link -- search for the named element
		hr = pDoc->get_all( &pCollection );
	
		if( ( hr == S_OK ) && pCollection )
		{
			hr = pCollection->get_length( &lCount );

			if ( hr == S_OK )
			{
				for ( l=0 ;( l< lCount ) && ( bRet == FALSE ); l++ )
				{
					varIndex.vt = VT_UINT;
					varIndex.lVal = l;

					VariantInit( &var );

					hr = pCollection->item( varIndex, var, &pDispatch );

					if( ( hr == S_OK ) && pDispatch ) 
					{
						pAnchor = NULL;
						hr = pDispatch->QueryInterface( IID_IHTMLAnchorElement, (void **)&pAnchor );

						if( ( hr == S_OK ) && pAnchor )
						{
							pAnchor->get_name( ( BSTRARG) cbstr );
							szValue = cbstr;

							if( ( szName.CompareNoCase( szValue ) == 0 ) ||
								(	( szValue.GetLength() > 0 ) && 
									( szValue[ 0 ] != '#' ) &&
									( szValue.CompareNoCase( szName.Right( szName.GetLength() - 1 ) ) == 0 ) ) )
							{
								hr = pDispatch->QueryInterface( IID_IHTMLElement, ( void** )( &pElement ) );
								
								if( ( hr == S_OK ) && ( pElement != NULL ) )
								{
									VariantInit( &var );
									var.vt = VT_BOOL;
#pragma warning(disable : 4310)
									var.boolVal = VARIANT_TRUE;
#pragma warning(default : 4310)
									pElement->scrollIntoView( var );
									bRet = TRUE;
									pElement->Release();
								}
							}

							pAnchor->Release();
						}
						
						pDispatch->Release();
					}
				}
			}

			pCollection->Release();
		}
	}
	else if( szHRef != "" )
	{
		// it's an external link or attachment
		return ::LaunchURL( szHRef );
	}

	return FALSE;
}


BOOL CTridentView::HasSelectedText()
{
	HRESULT hr;
	CComVariant v;
	IHTMLDocument2* pDoc = NULL;
	IHTMLSelectionObject* pSel = NULL;
	IHTMLTxtRange* pRange = NULL;
	CBstr type;
	CBstr text;
	BOOL	bReturn;

	bReturn = FALSE;

	// Get document
	pDoc = m_pSite->GetDoc2();

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( SUCCEEDED( hr ) &&  pSel ) 
	{
		// Get type
		hr = pSel->get_type( ( BSTRARG ) type );

		if ( ( S_OK == hr )  && !type.IsNull() )
		{
			CString strType = type;

			// If type isnt text, bail
			if( strType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 )
			{

				// Create range
				hr = pSel->createRange((IDispatch**)&pRange);
				
				if( ( S_OK == hr ) && pRange )
				{
					hr = pRange->get_text( ( BSTRARG ) text );
					
					if ( S_OK == hr && !text.IsNull() )
					{
						
						CString strText = text;
					
						// only return true if there is selected text and 
						// NO cr/lf pairs
						if( ( strText != "" ) && ( strText.Find("\r\n" ) == -1 ) )
						{
							bReturn = TRUE;
						}
					}

					pRange->Release();
				}
			}
		}
		pSel->Release();
	}

	return bReturn;
}


void CTridentView::OnUpdateFileOpenAttachment(CCmdUI* pCmdUI) 
{
	IHTMLElement*	pElement;
	CBstr			cbstrEudora( IDS_EUDORA_TAG );
	HRESULT			hr;
	CString			szVal;
	VARIANT			v;

	USES_CONVERSION;

	pElement = GetSelectedElement( CRString( IDS_HTML_ANCHOR ) );
	
	pCmdUI->Enable( FALSE );

	if( pElement == NULL )
	{
		return;
	}

	VariantInit( &v );

	// look for the EUDORA=attachment attribute

	hr = pElement->getAttribute( cbstrEudora, VARIANT_FALSE, &v );
	pElement->Release();

	if( ( hr != S_OK ) || ( v.vt != VT_BSTR ) )
	{
		VariantClear( &v );
		return;
	}

	szVal = v.bstrVal;

	if( szVal.CompareNoCase( "attachment" ) == 0 ) 
	{
		pCmdUI->Enable( TRUE );
	}

	VariantClear( &v );
}

void CTridentView::OnFileOpenAttachment() 
{
	IHTMLElement*			pElement;

	pElement = GetSelectedElement( CRString( IDS_HTML_ANCHOR ) );
	if( pElement )
	{
		LaunchURL( pElement );
		pElement->Release();
	}
}


#if 0    // TODO: copying multiple attachments
BOOL CTridentView::DoCopyAttachments()
{
    return FALSE;
}
#endif


BOOL CTridentView::QueryElement_FileAttachment( CPoint pt, IHTMLElement** ppElement /* = NULL */ )
{
    HRESULT             hr;
    IHTMLDocument2*     pDoc;
    IHTMLElement*       pElement;
    IHTMLElement*       pParent;
    CBstr               cbstr;
    CString             szValue;
    BOOL                bRet = FALSE;
    CString             szName = "";
    CString             szHRef = "";

    // get document
    pDoc = m_pSite->GetDoc2();
        
    if ( !pDoc ) 
        return FALSE;

    // get the element we clicked on
    pElement = NULL;

    hr = pDoc->elementFromPoint( pt.x, pt.y, &pElement );

    if( ( hr == S_OK ) && ( pElement != NULL ) ) {
        hr = pElement->get_tagName( ( BSTRARG )cbstr );
        szValue = cbstr;

        if( szValue.CompareNoCase( CRString( IDS_HTML_IMG ) ) == 0 ) {

            // In browse mode, we need to treat an image embedded in an anchor as if
            // the user had clicked on the url itself.  So swap the current element
            // for the parent.

            pParent = NULL;
            hr = pElement->get_parentElement( &pParent );

            if( ( hr == S_OK ) && ( pParent != NULL ) ) {

                pElement->Release();
                pElement = pParent;
                hr = pElement->get_tagName( ( BSTRARG ) cbstr );
                szValue = cbstr;
            }
        }
                
        // look for the EUDORA=attachment attribute
        if( szValue.CompareNoCase( CRString( IDS_HTML_ANCHOR ) ) == 0 ) {
            USES_CONVERSION;

            VARIANT v;
            VariantInit( &v );
            CBstr cbstrEudora(IDS_EUDORA_TAG);

            hr = pElement->getAttribute( cbstrEudora, VARIANT_FALSE, &v );

            if( (hr == S_OK) && (v.vt == VT_BSTR) ) {
                CString szVal = v.bstrVal;

                if( szVal.CompareNoCase( "attachment" ) == 0 ) {
                    if ( ppElement ) {
                        *ppElement = pElement;   // in through the out door
                        pElement   = NULL;       // don't release this
                    }

                    bRet = TRUE;
                }
            }

            VariantClear( &v );
        }
    }

    if( pElement )
        pElement->Release();

    return bRet;
}


BOOL CTridentView::DoCopyAttachment( IHTMLElement* piFileAttachment )
{
    USES_CONVERSION;

    BOOL bRet = FALSE;
    IHTMLBodyElement* pBody = NULL;

    if ( (pBody = m_pSite->GetBody()) == NULL ) {
        assert( 0 );
        return FALSE;
    }

    piFileAttachment->AddRef();

    // get the anchor, then the href (path to file attachment)
    CString szHRef( "" );
    IHTMLAnchorElement* pAnchor = NULL;
    HRESULT hr = piFileAttachment->QueryInterface( IID_IHTMLAnchorElement, (void**)(&pAnchor) );
                                
    if( (hr == S_OK) && (pAnchor != NULL) ) {
        CBstr cbstr;
        hr = pAnchor->get_href( BSTRARG(cbstr) );
                                        
        if( (hr == S_OK) && (((BSTR) cbstr) != NULL) ) {
            szHRef = cbstr;
            IHTMLTxtRange* pRange;
            hr = pBody->createTextRange( &pRange );
            hr = pRange->moveToElementText( piFileAttachment );
            hr = pRange->collapse( FALSE );
            hr = pRange->select();
        }

        pAnchor->Release();                                 
    }

    pBody->Release();
    piFileAttachment->Release();

    if ( szHRef.GetLength() ) {
        CString filePath = UnEscapeURL( szHRef );

        // get rid of the "file://" thingie
        char* pcstr = filePath.GetBuffer( filePath.GetLength() );
        strcpy( pcstr, &pcstr[7] );
        filePath.ReleaseBuffer();

        unsigned pathLen = filePath.GetLength();
        DWORD dwBytes = (sizeof(DROPFILES) + pathLen + 2 /* double null terminated */);
        HGLOBAL hglb = GlobalAlloc( GMEM_SHARE | GMEM_ZEROINIT, dwBytes );
        BYTE* pBytes = (BYTE*) GlobalLock( hglb );
        ((DROPFILES*)pBytes)->pFiles = sizeof(DROPFILES);
        memcpy( pBytes + sizeof(DROPFILES), filePath, pathLen );
        GlobalUnlock( hglb );

        COleDataSource* theData = new COleDataSource;;
        theData->CacheGlobalData( CF_HDROP, hglb );
        theData->SetClipboard();
        bRet = TRUE;
    }

    return bRet;
}


///////////////////////////////////////////////////////////////////////////////
// QCP_GET_MESSAGE Protocol

// BOG: this stuff was previously located in the derived CTridentReadMessageView.
// I moved it here, cause that's where it belongs :-p  There may be more QCP
// stuff that needs to be moved, but for now I'm only interested in the methods
// for getting text.

BOOL CTridentView::GetSelectedText( CString& szText )
{
    HRESULT                  hr;
    IHTMLDocument2*          pDoc;
    IHTMLSelectionObject*    pSel;
    IHTMLTxtRange*           pRange;
    CBstr                    cbstrType;
    CBstr                    cbstrText;
    BOOL                     bReturn;

    szText  = "";
    pRange  = NULL;
    pSel    = NULL;
    pDoc    = NULL;
    bReturn = FALSE;

    pDoc = m_pSite->GetDoc2();
        
    if ( !pDoc )
        return FALSE;

    hr = pDoc->get_selection(&pSel);

    if( SUCCEEDED( hr ) &&  pSel ) {
        hr = pSel->get_type( BSTRARG( cbstrType ) );

        if ( ( S_OK == hr )  && ( ( ( BSTR) cbstrType ) != NULL ) ) {
            CString strType = cbstrType;

            // if type isnt text, bail
//            if( ( strType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 ) ||
//                ( strType.CompareNoCase( CRString( IDS_NONE ) ) == 0 ) ) {
            if ( strType.CompareNoCase( CRString(IDS_TEXT) ) == 0 ) {
                hr = pSel->createRange((IDispatch**)&pRange);
                                
                if( ( S_OK == hr ) && pRange ) {
                    hr = pRange->get_text( BSTRARG( cbstrText ) );

                    if ( ( S_OK == hr ) && ( ( ( BSTR ) cbstrText ) != NULL ) ) {
                        szText = cbstrText;
                        bReturn = TRUE;
                    }

                    pRange->Release();
                }
            }
        }

        pSel->Release();
    }

    if ( szText.GetLength() ) {
	StripNBSP( szText.GetBuffer( 0 ) );
	szText.ReleaseBuffer();
	StripBogusCRLF( szText.GetBuffer( 0 ) );
	szText.ReleaseBuffer();
    }

    return bReturn;
}

BOOL CTridentView::GetSelectedHTML( CString& szHTML )
{
    HRESULT                  hr;
    IHTMLDocument2*          pDoc;
    IHTMLSelectionObject*    pSel;
    IHTMLTxtRange*           pRange;
    CBstr                    cbstrType;
    CBstr                    cbstrText;
    BOOL                     bReturn;

    szHTML  = "";
    pRange  = NULL;
    pSel    = NULL;
    pDoc    = NULL;
    bReturn = FALSE;

    pDoc = m_pSite->GetDoc2();
        
    if (!pDoc) 
        return FALSE;

    hr = pDoc->get_selection(&pSel);

    if( SUCCEEDED( hr ) &&  pSel ) {
        hr = pSel->get_type( BSTRARG( cbstrType ) );

        if ( ( S_OK == hr )  && ( ( ( BSTR ) cbstrType ) != NULL ) ) {
            CString strType = cbstrType;

            // If type isnt text, bail
//            if( ( strType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 ) ||
//                ( strType.CompareNoCase( CRString( IDS_NONE ) ) == 0 ) ) {
            if( strType.CompareNoCase( CRString(IDS_TEXT) ) == 0 ) {
                hr = pSel->createRange((IDispatch**)&pRange);
                                
                if( ( S_OK == hr ) && pRange ) {
                    hr = pRange->get_htmlText( BSTRARG( cbstrText ) );

                    if ( ( S_OK == hr ) && ( ( ( BSTR ) cbstrText ) != NULL ) ) {
                        szHTML  = "<html>";
                        szHTML += CString( cbstrText );
                        szHTML += "</html>";
                        bReturn = TRUE;
                    }

                    pRange->Release();
                }
            }
        }

        pSel->Release();
    }

    return bReturn;
}

BOOL CTridentView::GetAllText( CString& szText )
{
    HRESULT              hr;
    IHTMLBodyElement*    pBody;
    IHTMLTxtRange*       pRange;
    BOOL                 bReturn;
    CBstr                cbstrText;

    bReturn = FALSE;
    szText  = "";
    pBody   = m_pSite->GetBody();
        
    if( !pBody ) 
        return bReturn;

    pRange = NULL;
    hr = pBody->createTextRange( &pRange );

    if( ( S_OK == hr ) && pRange ) {
        hr = pRange->get_text( BSTRARG( cbstrText ) );

        if ( S_OK == hr && ( ( ( BSTR ) cbstrText ) != NULL ) ) {
            szText = cbstrText;
            bReturn = TRUE;
        }

        pRange->Release();
    }

    pBody->Release();
    StripNBSP( szText.GetBuffer( 0 ) );
    szText.ReleaseBuffer();
    return bReturn;
}

BOOL CTridentView::GetAllHTML( CString& szHTML )
{
    HRESULT              hr;
    IHTMLBodyElement*    pBody;
    IHTMLTxtRange*       pRange;
    BOOL                 bReturn;
    CBstr                cbstrText;

    bReturn = FALSE;
    szHTML  = "";
    pBody   = m_pSite->GetBody();
        
    if( !pBody ) 
        return bReturn;

    pRange = NULL;
    hr = pBody->createTextRange( &pRange );

    if( ( S_OK == hr ) && pRange ) {
        hr = pRange->get_htmlText( BSTRARG( cbstrText ) );

        if ( S_OK == hr && ( ( ( BSTR ) cbstrText ) != NULL ) ) {
            szHTML = cbstrText;
            bReturn = TRUE;
        }

        pRange->Release();
    }

    pBody->Release();
    return bReturn;
}

BOOL CTridentView::PasteOnDisplayHTML(const char* szHTML, bool bSign)
{
	HRESULT					hr;
	IDispatch*				pDispatch; 	
	IHTMLDocument2*			pDoc;
	IHTMLElement*			pElement;
	CBstr					cbstr;
	CString					szText;
	IHTMLElementCollection* pCollection;
	LONG					lIndex;
	VARIANT					varIndex;
	VARIANT					var;
	LONG					lCount;
	CRString				szAnchorTag( IDS_HTML_ANCHOR );
	BOOL					bRet;

	USES_CONVERSION;

	pDoc = m_pSite->GetDoc2();
	
	if( !pDoc ) 
	{
		return FALSE;
	}

	bRet = FALSE;

	pCollection = NULL;

	hr = pDoc->get_all( &pCollection );

	if( ( hr != S_OK ) || !pCollection )
	{
		return FALSE;
	}

	hr = pCollection->get_length( &lCount );

	if ( hr == S_OK )
	{
		for ( lIndex = 0; lIndex < lCount; lIndex++ )
		{
			varIndex.vt = VT_UINT;
			varIndex.lVal = lIndex;

			VariantInit( &var );

			hr = pCollection->item( varIndex, var, &pDispatch );

			if( ( hr == S_OK ) && pDispatch ) 
			{
				hr = pDispatch->QueryInterface( IID_IHTMLElement, (void **)&pElement );

				if( ( hr == S_OK ) && pElement )
				{
					hr = pElement->get_tagName( BSTRARG( cbstr ) );
					szText = cbstr;
					
					if( szText.CompareNoCase( szAnchorTag ) == 0 )
					{
						cbstr = szHTML;
						pElement->put_outerHTML( cbstr );
						bRet = TRUE;
						GetDocument()->SetModifiedFlag();
						// end the loop
						lCount = 0;
					}

					pElement->Release();
				}
				
				pDispatch->Release();
			}
		}
	}

	pCollection->Release();
	return bRet;
}
