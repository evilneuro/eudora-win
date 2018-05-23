// TridentView.cpp : implementation of the CBidentView class
//

#include "stdafx.h"
#include "eudora.h"

#ifdef IDM_PROPERTIES
#undef IDM_PROPERTIES
#endif

#include <AFXPRIV.H>
#include <afxdisp.h>

#include "BidentView.h"
#include "CompMsgd.h"
#include "site.h"
#include "mshtmcid.h"
#include "mainfrm.h"
#include "QCRecipientDirector.h"
// delete 
#include "newmbox.h"

#include "fileutil.h"
#include "guiutils.h"
#include "rs.h"
#include "msgutils.h"
//#include "summary.h"
#include "convhtml.h"
#include "font.h"
#include "ReadMessageFrame.h"
#include "QCOleDropSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//#include "BidentModel.h"
//#include "BidentModel_i.c"

extern QCRecipientDirector	g_theRecipientDirector;

/////////////////////////////////////////////////////////////////////////////
// CBidentView

IMPLEMENT_DYNAMIC(CBidentView, CView)

BEGIN_MESSAGE_MAP(CBidentView, CView)
	//{{AFX_MSG_MAP(CBidentView)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	//}}AFX_MSG_MAP

	ON_COMMAND( ID_VIEW_SOURCE, OnViewSource )
	ON_COMMAND( ID_SEND_TO_BROWSER, OnSendToBrowser)

	ON_COMMAND( ID_EDIT_COPY, OnCopy )
	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY, OnUpdateCopy )

	ON_COMMAND( ID_SPECIAL_ADDASRECIPIENT, OnSpecialAddAsRecipient )
	ON_UPDATE_COMMAND_UI( ID_SPECIAL_ADDASRECIPIENT, OnUpdateAddAsRecipient )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBidentView construction/destruction

UINT CBidentView::s_uTmpFileUnique = 0;

CBidentView::CBidentView() 
{
	m_cRef = 0;
	m_hWndObj = NULL;
	    
    m_pSite = NULL;
    m_pIOleIPActiveObject = NULL;
	m_fCreated = FALSE;
	
	m_szTmpFile = "";
	SetExplorerOptions();
	m_lReadyState = READYSTATE_UNINITIALIZED;

#ifdef ADWARE_PROTOTYPE
	m_DocHostUIFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DIV_BLOCKDEFAULT;
#endif
}


CBidentView::~CBidentView()
{
	// Debug check that nobody is trying to hang onto us!
	unlink( m_szTmpFile );
	ASSERT( m_cRef == 0 );
}


/////////////////////////////////////////////////////////////////////////////
// CBidentView diagnostics

#ifdef _DEBUG
void CBidentView::AssertValid() const
{
	CView::AssertValid();
}

void CBidentView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG


/*
 * IUnknown implementation
 */
/*
 * CBidentView::QueryInterface
 * CBidentView::AddRef
 * CBidentView::Release
 */
STDMETHODIMP CBidentView::QueryInterface( REFIID riid, void **ppv )
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


STDMETHODIMP_(ULONG) CBidentView::AddRef( void )
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CBidentView::Release( void )
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
 * CBidentView::GetWindow
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
STDMETHODIMP CBidentView::GetWindow( HWND* phWnd )
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
 * CBidentView::ContextSensitiveHelp
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
STDMETHODIMP CBidentView::ContextSensitiveHelp( BOOL fEnterMode )
{
    return NOERROR;
}



/*
 * CBidentView::GetBorder
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
STDMETHODIMP CBidentView::GetBorder( LPRECT prcBorder )
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
 * CBidentView::RequestBorderSpace
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
STDMETHODIMP CBidentView::RequestBorderSpace( LPCBORDERWIDTHS /*pBW*/ )
{
    // We have no border space restrictions
    return NOERROR;
}


/*
 * CBidentView::SetBorderSpace
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
STDMETHODIMP CBidentView::SetBorderSpace( LPCBORDERWIDTHS /*pBW*/ )
{
	// We turn off the Trident UI so we ignore all of this.

    return NOERROR;
}




/*
 * CBidentView::SetActiveObject
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
STDMETHODIMP CBidentView::SetActiveObject( LPOLEINPLACEACTIVEOBJECT pIIPActiveObj,
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
 * CBidentView::InsertMenus
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
STDMETHODIMP CBidentView::InsertMenus( HMENU /*hMenu*/, LPOLEMENUGROUPWIDTHS /*pMGW*/ )
{
	// We've turned off the Trident Menus so we don't expect any merging to go on!
	return E_NOTIMPL;
}


/*
 * CBidentView::SetMenu
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
STDMETHODIMP CBidentView::SetMenu( HMENU /*hMenu*/, HOLEMENU /*hOLEMenu*/, HWND /*hWndObj*/ )
{
	// We've turned off the Trident Menus so we don't expect any merging to go on!
	return E_NOTIMPL;
}



/*
 * CBidentView::RemoveMenus
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
STDMETHODIMP CBidentView::RemoveMenus( HMENU /*hMenu*/ )
{
	// We've turned off the Trident Menus so we don't expect any merging to go on!
	return E_NOTIMPL;
}




/*
 * CBidentView::SetStatusText
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
STDMETHODIMP CBidentView::SetStatusText( LPCOLESTR pszText )
{
	CString		szText;
	CFrameWnd*	pFrame;

	pFrame = GetTopLevelFrame();

	m_szURL = "";

	if ( pFrame != NULL )
	{
		USES_CONVERSION;
		szText = OLE2T(pszText);
		pFrame->SetMessageText( szText );
		if( strnicmp( szText, "Shortcut to ", 12 ) == 0 )
		{
			m_szURL = szText.Right( szText.GetLength() - 12 );
		}
		else if( stricmp( szText, "Done" ) == 0 )
		{
			m_lReadyState = READYSTATE_COMPLETE;
		}
	}
    return NOERROR;
}



/*
 * CBidentView::EnableModeless
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

STDMETHODIMP CBidentView::EnableModeless( BOOL /*fEnable*/ )
{
    return NOERROR;
}


/*
 * CBidentView::TranslateAccelerator
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
STDMETHODIMP CBidentView::TranslateAccelerator( LPMSG /*pMSG*/, WORD /*wID*/ )
{
	//We should probably forward this to the top level frame!
    return S_FALSE;
}


/*
 * IOleCommandTarget methods.
 */
STDMETHODIMP CBidentView::QueryStatus( const GUID* pguidCmdGroup, ULONG cCmds,
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
        
STDMETHODIMP CBidentView::Exec( const GUID* pguidCmdGroup, DWORD nCmdID,
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
				break;

			case OLECMDID_SETTITLE:
				if (pvaIn && V_VT(pvaIn) == VT_BSTR)
				{
					CDocument* pDoc = GetDocument();
					ASSERT_VALID(pDoc);

					pDoc->SetTitle(OLE2T(V_BSTR(pvaIn)));
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


int CBidentView::CreateDocObject( TCHAR* pchPath )
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
			// Don't forget to AddRef or Trident will assume we aren't using it, and it'll go away.
			pCommandTarget->AddRef();
			m_pSite->SetCommandTarget( pCommandTarget );
		}
	}

    return SITE_ERROR_NONE;        
}


//////////////////////////////////////////////////////////////////////////
// Helper functions on IOleCommandTarget of the object

DWORD CBidentView::GetCommandStatus( ULONG ucmdID )
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


void CBidentView::ExecCommand(
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


void CBidentView::ExecCommandOnNullSet( 
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
// CBidentView message handlers

void CBidentView::OnDestroy() 
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


void CBidentView::OnCopy() 
{
	::SendMessage( m_hWndObj, WM_COMMAND, 13, 0 );// got the value from the dll	
}


void CBidentView::OnUpdateCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE );
}




void CBidentView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	//Tell the site to tell the object.
	if ( NULL != m_pSite )
	{
    	m_pSite->UpdateObjectRects();
	}
}


void CBidentView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	// Give the focus to the ActiveX Document window
    if ( m_hWndObj != NULL )
	{
		::SetFocus( m_hWndObj );
	}
}



BOOL CBidentView::OnEraseBkgnd(CDC* pDC) 
{
	// Prevent MFC from annoyingly erasing the background when it thinks it should
	//  by blocking the call to CView::OnEraseBkgnd
	return TRUE;
}


void CBidentView::OnInitialUpdate() 
{
	if ( m_fCreated == TRUE )
		return;
	CView::OnInitialUpdate();	
	m_fCreated = TRUE;
	
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
		
		m_lReadyState = READYSTATE_COMPLETE; // set the ready flag if we've failed
		return;
	}

	if (!LoadMessage())
		return;


    //We created the thing, now activate it with "Show"
    if( GetStyle() & WS_VISIBLE )
		m_pSite->Activate( OLEIVERB_SHOW );
	else
		m_pSite->Activate( OLEIVERB_HIDE );
}

BOOL CBidentView::LoadMessage()
{
	CFile		theFile;
	TCHAR		szDrive[_MAX_DRIVE + 1];
	TCHAR		szPath[_MAX_PATH + 1];
	TCHAR		szTmpReal[_MAX_PATH + 1];
	TCHAR		szFile[_MAX_FNAME + 1];
	TCHAR		szExt[_MAX_EXT + 1];
	CString		szText;

	m_lReadyState = READYSTATE_UNINITIALIZED;

	// Get rid of previous temp file
	unlink(m_szTmpFile);

	// get the filename	
	//extern TempDir - declared in fileutil.h
	strcpy(szPath, TempDir);
	if ( !GetTempFileName( szPath, "eud", ++s_uTmpFileUnique, szTmpReal ) )
	{
		::ErrorDialog( IDS_ERR_TEMP_FILE );
		m_lReadyState = READYSTATE_COMPLETE; // set the ready flag if we've failed
		return FALSE;
	}

	_splitpath( szTmpReal, szDrive, szPath, szFile, szExt );

	// change the extension to .htm
	m_szTmpFile = szDrive;
	m_szTmpFile += szPath;
	m_szTmpFile += szFile;
	m_szTmpFile += ".";
	m_szTmpFile += CRString(IDS_HTM_EXTENSION);
	
	CFileException FE;
	if ( ! theFile.Open( m_szTmpFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary, &FE ))
	{
		::ErrorDialog( IDS_ERR_OPEN_TEMP_FILE, FE.m_cause, FE.m_lOsError, (LPCTSTR)FE.m_strFileName );
		m_lReadyState = READYSTATE_COMPLETE; // set the ready flag if we've failed
		return FALSE;
	}
	if (! WriteTempFile( theFile, GetIniShort(IDS_INI_SHOW_ALL_HEADERS) ))
	{
		::ErrorDialog( IDS_ERR_WRITE_TEMP_FILE );
		m_lReadyState = READYSTATE_COMPLETE; // set the ready flag if we've failed
		return FALSE;
	}
		
	theFile.Close();	

	if (!m_pSite)
	{
		// Uh oh, should have created site control by now.
		ASSERT(0);
		return FALSE;
	}

	if (FAILED(m_pSite->Load((LPTSTR)(LPCTSTR)m_szTmpFile)))
		return FALSE;

	SetFontSize( GetIniShort( IDS_INI_MESSAGE_FONT_SIZE ) );

	MSG msg;
	while( !IsReady() && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
	{
		// we have to wait until the bident control is
		// is ready to go before we can do anything with it.

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}


void CBidentView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


void CBidentView::OnSpecialAddAsRecipient()
{
	BOOL status = FALSE;

//	if (::OpenClipboard(GetSafeHwnd()))
	{
//		hText = ::GetClipboardData();
		
//		::EmptyClipboard();
//		status = ::SetClipboardData(CF_TEXT, h_mem) ? TRUE : FALSE;
//		::CloseClipboard();
	}


//	::SendMessage( m_hWndObj, WM_COMMAND, 13, 0 );// got the value from the dll
}


void CBidentView::OnUpdateAddAsRecipient(CCmdUI* pCmdUI)
{	
	pCmdUI->Enable( FALSE );
}




// converts all the URIs to local file references based on the "Embedded Content
// on input *ppHTML should have the raw html
// on return *ppHTML will point to a buffer that contains morphed html
// *ppHTML may be changed as a side effect of this call
void CBidentView::MorphMHTML( char ** ppHTML )
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


BOOL CBidentView::QuietPrint()
{
/*	CFile		theFile;
	BOOL		bBlahBlahBlah;
	CFrameWnd*	pFrameWnd;

	pFrameWnd = GetParentFrame();

	if( pFrameWnd->IsKindOf( RUNTIME_CLASS( CReadMessageFrame ) ) )
	{
		bBlahBlahBlah = ( ( CReadMessageFrame* ) pFrameWnd ) ->GetCheck( ID_BLAHBLAHBLAH );
	}
	else
	{
		bBlahBlahBlah = FALSE;
	}
*/
	MSG	msg;

	while( ( m_lReadyState != READYSTATE_COMPLETE ) &&
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
	{
		// we have to wait until the trident control is
		// is ready to go before we can do anything with it.

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	SetRedraw( FALSE );

	SetFontSize( GetIniShort( IDS_INI_PF_SIZE ) );
/*	
	if ( ! theFile.Open( m_szTmpFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary )  )
	{
		MessageBox("Error: Unable to open temp file" );
		return FALSE;
	}

	if( ! WriteTempFile( theFile, bBlahBlahBlah ) )
	{
		MessageBox("Error: Unable to write temp file" );
		return FALSE;
	}
		
	theFile.Close();	
	
	::SendMessage( m_hWndObj, WM_COMMAND, 28, 0 );// got the value from the dll
*/
	ExecCommandOnNullSet( OLECMDID_PRINT );
	
/*	if ( ! theFile.Open( m_szTmpFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary )  )
	{
		MessageBox("Error: Unable to open temp file" );
		return FALSE;
	}

	if( ! WriteTempFile( theFile,  bBlahBlahBlah  ) )
	{
		MessageBox("Error: Unable to write temp file" );
		return TRUE;
	}
		
	theFile.Close();	
	
	::SendMessage( m_hWndObj, WM_COMMAND, 28, 0 );// got the value from the dll
*/
	SetFontSize( GetIniShort( IDS_INI_MESSAGE_FONT_SIZE ) );
	
	SetRedraw( TRUE );	
	
	return TRUE;
}


BOOL CBidentView::IsEditing()
{
	return FALSE;
}


BOOL CBidentView::PreTranslateMessage(MSG* pMsg) 
{
	CWnd*		pWnd;
	CFrameWnd*	pFrameWnd;
	CPoint		pt;

	if( ::IsChild( GetSafeHwnd(), pMsg->hwnd ) )
	{
		if( pMsg->message == WM_RBUTTONUP ) 
		{
			VERIFY( pWnd = CWnd::FromHandle( pMsg->hwnd ) );
			VERIFY( pFrameWnd = GetParentFrame() );
			pt.x = LOWORD( pMsg->lParam );
			pt.y = HIWORD( pMsg->lParam );

			pWnd->ClientToScreen( &pt );

			pFrameWnd->PostMessage(	WM_CONTEXTMENU, 
									WPARAM( GetSafeHwnd() ),
									MAKELPARAM( pt.x, pt.y ) );
			return TRUE;
		}
		
		if( pMsg->message == WM_LBUTTONDOWN )
		{
			SetExplorerOptions();
		}

		if( pMsg->message == WM_LBUTTONUP )
		{
			if( ( m_szURL != "" ) && LaunchURL( m_szURL ) )
			{
				return TRUE;
			}
		}

//		if( ( pMsg->message == WM_KEYUP ) && ( pMsg->wParam == VK_RETURN ) )
//		{
//			pt.x = 0;
//			pt.y = 0;
//
//			if( HandleLButtonUp( 0, pt ) )
//			{
//				return TRUE;
//			}
//		}
	}

	if (WM_KEYDOWN == pMsg->message)
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
					SendMessage(WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP) ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
					return TRUE;	// don't let Bident control see this message
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
				return TRUE;		// don't let the Bident control see this message
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
					SendMessage(WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP) ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
					return TRUE;	// don't let Bident control see this message
				}
			}
			break;
		default:
			break;
		}
	}
	
	if ( NULL != m_pIOleIPActiveObject )
    {
        HRESULT     hr;

        hr = m_pIOleIPActiveObject->TranslateAccelerator( pMsg );

        //If the object translated the accelerator, we're done
        if ( NOERROR == hr )
		{
            return TRUE;
		}
    }

	return CView::PreTranslateMessage(pMsg);
}


void CBidentView::OnViewSource()
{
	::SendMessage( m_hWndObj, WM_COMMAND, 65, 0 );// got the value from the dll
}

void CBidentView::OnSendToBrowser()
{
	ShellExecute(AfxGetMainWnd()->m_hWnd, NULL, m_szTmpFile, NULL, NULL, SW_SHOWNORMAL);
}


void CBidentView::SetExplorerOptions()
{
	HKEY		theKey;
	DWORD		dwDisposition;
	DWORD		dwType;
	CHAR		szDataBuf[1024];
	DWORD		dwSize;
	LONG		lError;

	// make sure we can grab the url from the status bar

	dwSize = 1023;

	lError = RegCreateKeyEx(	HKEY_CURRENT_USER, 
								"Software\\Microsoft\\Internet Explorer\\Main",
								0,
								NULL,
								REG_OPTION_NON_VOLATILE, 
								KEY_ALL_ACCESS,
								NULL,
								&theKey,
								&dwDisposition);

	lError = RegQueryValueEx( theKey, "Show_FullURL", 0, &dwType, ( UCHAR* )szDataBuf, &dwSize );
	
	if( stricmp( "yes", szDataBuf ) )
	{
		strcpy( szDataBuf, "yes" );
		lError = RegSetValueEx( theKey, "Show_FullURL", 0, dwType, ( UCHAR* ) szDataBuf, 4 );
	}

	dwSize = 1023;
	lError = RegQueryValueEx( theKey, "Show_URLinStatusBar", 0, &dwType, ( UCHAR* )szDataBuf, &dwSize );
	
	if( stricmp( "yes", szDataBuf ) )
	{
		strcpy( szDataBuf, "yes" );
		lError = RegSetValueEx( theKey, "Show_URLinStatusBar", 0, dwType, ( UCHAR* ) szDataBuf, 4 );
	}

	RegCloseKey( theKey );
}


void CBidentView::SetFontSize(
UINT uSize )
{
	HKEY		theKey;
	DWORD		dwDisposition;
	DWORD		dwType;
	DWORD		dwSize;
	DWORD		dwFontSize;
	LONG		lError;
	WPARAM		wParam;

	// make sure we can grab the url from the status bar

	dwSize = sizeof( DWORD );

	lError = RegCreateKeyEx(	HKEY_CURRENT_USER, 
								"Software\\Microsoft\\Internet Explorer\\Main",
								0,
								NULL,
								REG_OPTION_NON_VOLATILE, 
								KEY_ALL_ACCESS,
								NULL,
								&theKey,
								&dwDisposition);

	lError = RegQueryValueEx( theKey, "Default Font Size", 0, &dwType, ( UCHAR* )&dwFontSize, &dwSize );
	

	if( dwFontSize != uSize )
	{
		wParam = uSize + 51; // got the 51 from the dll

		::SendMessage( m_hWndObj, WM_COMMAND, wParam, 0 );

		// now set it back in the registry
		lError = RegSetValueEx( theKey, "Default Font Size", 0, dwType, ( UCHAR* ) &dwFontSize, dwSize );
	}

	RegCloseKey( theKey );
}



void CBidentView::OnFilePrint() 
{
	SetRedraw( FALSE );

	SetFontSize( GetIniShort( IDS_INI_PF_SIZE ) );
	ExecCommandOnNullSet( OLECMDID_PRINT, NULL, NULL, TRUE );
	SetFontSize( GetIniShort( IDS_INI_MESSAGE_FONT_SIZE ) );
	
	SetRedraw( TRUE );	
}


void CBidentView::WriteDefaults(
CFile&			theFile,
const CString&	szFontName )
{
	HKEY		theKey;
	DWORD		dwDisposition;
	DWORD		dwType;
	CHAR		szDataBuf[1024];
	DWORD		dwSize;
	LONG		lError;
	COLORREF	cr;
	CString		szText;
	CString		szTemp;

	dwSize = 1023;

	lError = RegCreateKeyEx(	HKEY_CURRENT_USER, 
								"Software\\Microsoft\\Internet Explorer\\Main",
								0,
								NULL,
								REG_OPTION_NON_VOLATILE, 
								KEY_ALL_ACCESS,
								NULL,
								&theKey,
								&dwDisposition);

	lError = RegQueryValueEx( theKey, "Use_DlgBox_Colors", 0, &dwType, ( UCHAR* )szDataBuf, &dwSize );

	RegCloseKey( theKey );	
	
	szText = "<HEAD></HEAD><BODY";

 	if( stricmp( "yes", szDataBuf ) == 0 )
	{
		// don't mess with user defined colors
		cr = ::GetSysColor( COLOR_WINDOW );	
		szTemp.Format( " bgColor=#%06x", cr );
		szText += szTemp;
	}

	szTemp.Format( "><FONT face=\"%s\">\r\n",	( const char* ) szFontName );

	szText += szTemp;
	
	theFile.Write( szText, szText.GetLength() );
}


BOOL CBidentView::IsReady()
{
//	HRESULT			hr;
//	TCHAR			szBuff[255];
//	IHTMLDocument*	pDoc;
	VARIANT			vResult = {0};
//	EXCEPINFO		excepInfo;
//	UINT			uArgErr;
	DISPPARAMS		dp = {NULL, NULL, 0, 0};

	if( m_lReadyState == READYSTATE_COMPLETE )
	{
		return TRUE;
	}

	// Get document
	IUnknown* pUnk = m_pSite->GetObjectUnknown();

	if ( !pUnk ) 
	{
		return FALSE;
	}

	m_lReadyState = READYSTATE_LOADED;
	return ( ( m_lReadyState == READYSTATE_COMPLETE ) ||
			 ( m_lReadyState == READYSTATE_INTERACTIVE ) ||
			 ( m_lReadyState == READYSTATE_LOADED ) );
}
