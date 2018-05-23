//
// SITE.CPP
// Document Object Site Object
//
// Copyright (c)1995-1997 Microsoft Corporation, All Rights Reserved
//

#include "stdafx.h"
#include "ambients.h"
#include "site.h"
#include "BidentView.h"
#include "TridentView.h"
#include "mshtmdid.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

UINT wmReadyStateComplete = RegisterWindowMessage( "wmReadyStateComplete" );

#define ODS(x) OutputDebugString(x)

/*
 * CSite::CSite
 * CSite::~CSite
 *
 * Constructor Parameters:
 *  hWnd            HWND of the window associated with the site
 *  pFR             PCFrame to the parent structure.
 */
CSite::CSite( HWND hWnd, CTridentView* pFR )
{
    m_cRef = 0;
    m_hWnd = hWnd;
    m_pTridentView = pFR;

    m_pObj = NULL;
  
    m_pIOleObject = NULL;
    m_pIOleIPObject = NULL;
    m_pIOleDocView = NULL;
	m_pIOleCommandTarget = NULL;
	m_pCP = NULL;
	m_dwCookie = 0;

    m_pImpIOleClientSite = NULL;
    m_pImpIAdviseSink = NULL;
    m_pImpIOleIPSite = NULL;
    m_pImpIOleDocumentSite = NULL;
	m_pImpIDocHostUIHandler = NULL;
	m_pImpIDocHostShowUI = NULL;
	m_pImpIDispatchAmbients= NULL;
	m_pBidentView = NULL;
	m_dwUnadvise = 0;
	m_pDoc2 = NULL;
	m_lReadyState = READYSTATE_UNINITIALIZED;
}


/*
 * CSite::CSite
 * CSite::~CSite
 *
 * Constructor Parameters:
 *  hWnd            HWND of the window associated with the site
 *  pFR             PCFrame to the parent structure.
 */
CSite::CSite( HWND hWnd, CBidentView* pFR )
{
    m_cRef = 0;
    m_hWnd = hWnd;
    m_pBidentView = pFR;
	m_pTridentView = NULL;

    m_pObj = NULL;
    
    m_pIOleObject = NULL;
    m_pIOleIPObject = NULL;
    m_pIOleDocView = NULL;
	m_pIOleCommandTarget = NULL;
	m_pCP = NULL;
	m_dwCookie = 0;

    m_pImpIOleClientSite = NULL;
    m_pImpIAdviseSink = NULL;
    m_pImpIOleIPSite = NULL;
    m_pImpIOleDocumentSite = NULL;
	m_pImpIDocHostUIHandler = NULL;
	m_pImpIDocHostShowUI = NULL;
	m_pImpIDispatchAmbients= NULL;

	m_dwUnadvise = 0;
	m_pDoc2 = NULL;
	m_lReadyState = READYSTATE_UNINITIALIZED;
}


CSite::~CSite(void)
{
    //Object pointers cleaned up in Close.

    //We delete our own interfaces since we control them
    DeleteInterfaceImp( m_pImpIOleDocumentSite );
    DeleteInterfaceImp( m_pImpIOleIPSite );
    DeleteInterfaceImp( m_pImpIAdviseSink );
    DeleteInterfaceImp( m_pImpIOleClientSite );
	DeleteInterfaceImp( m_pImpIDocHostUIHandler );
	DeleteInterfaceImp( m_pImpIDocHostShowUI );
	DeleteInterfaceImp( m_pImpIDispatchAmbients );
}


/*
 * CSite::QueryInterface
 * CSite::AddRef
 * CSite::Release
 *
 * Purpose:
 *  IUnknown members for CSite object.
 */
STDMETHODIMP CSite::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;

#ifdef _DEBUG
	OLECHAR wszGUID[39];
	StringFromGUID2(riid, wszGUID, 39);
	USES_CONVERSION;
	LPTSTR szGUID = OLE2T(wszGUID);
//	OutputDebugString("CSite::QI(");
//	OutputDebugString(szGUID);
//	OutputDebugString(")\n");
#endif

    if ( IID_IOleClientSite == riid )
	{
        *ppv = m_pImpIOleClientSite;
	}

    if ( IID_IAdviseSink == riid )
	{
        *ppv = m_pImpIAdviseSink;
	}

    if ( IID_IOleWindow == riid || IID_IOleInPlaceSite == riid )
	{
        *ppv = m_pImpIOleIPSite;
	}

    if ( IID_IOleDocumentSite == riid )
	{
        *ppv = m_pImpIOleDocumentSite;
	}

    if ( IID_IDocHostUIHandler == riid )
    {
        *ppv = m_pImpIDocHostUIHandler;
    }

    if ( IID_IDocHostShowUI == riid )
    {
        *ppv = m_pImpIDocHostShowUI;
    }

    if ( IID_IDispatch== riid )
	{
        *ppv = m_pImpIDispatchAmbients;
	}
    
	if( IID_IPropertyNotifySink == riid )
	{
		*ppv = (IPropertyNotifySink*)this;
	}

	if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

	// Try the frame instead
	if( m_pTridentView != NULL )
	{
		return m_pTridentView->QueryInterface( riid, ppv );
	}
	
	return m_pBidentView->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CSite::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CSite::Release(void)
{
    if ( 0 != --m_cRef )
	{
        return m_cRef;
	}

    delete this;

    return 0;
}

/*
 * CSite::Create
 *
 * Purpose:
 *  Asks the site to instantiate the MSHTML.DLL object.
 *  
 *
 * Return Value:
 *  INT            Error value
*/
INT CSite::Create()
{
    HRESULT   hr;

	// Create the site's interface implementations which MSHTML.DLL will call
    m_pImpIOleClientSite = new CImpIOleClientSite( this, this );
    m_pImpIAdviseSink = new CImpIAdviseSink( this, this );
    m_pImpIOleIPSite = new CImpIOleInPlaceSite( this, this );
    m_pImpIOleDocumentSite = new CImpIOleDocumentSite( this, this );
	m_pImpIDocHostUIHandler = new CImpIDocHostUIHandler( this, this );
	m_pImpIDocHostShowUI = new CImpIDocHostShowUI( this, this );
	m_pImpIDispatchAmbients= new CImpIDispatch( this, this );

    if ( NULL == m_pImpIOleClientSite || NULL == m_pImpIAdviseSink
        || NULL == m_pImpIOleIPSite || NULL == m_pImpIOleDocumentSite
		|| NULL == m_pImpIDocHostUIHandler )
	{
        return SITE_ERROR_MEMORY;
	}


	// Create MSHTML.DLL
	hr = CoCreateInstance( CLSID_HTMLDocument, NULL, 
								CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&m_pObj );
    //If creation didn't work, then clean up.
    if ( FAILED( hr ) )
    {
        return SITE_ERROR_NO_MSHTML_DLL;
    }

	 // Initialise the object we just created. Set ClientSite, the AdviseSink, etc.
    return SITE_ERROR_NONE;
}


/*
 * CSite::ObjectInitialize
 * (Protected)
 *
 * Purpose:
 *  Performs operations necessary after creating an object or
 *  reloading one from storage.
 *
 * Parameters:
 *	pchPath			The path that we want to load from
 *
 * Return Value:
 *  INT            Error value
 */
INT CSite::ObjectInitialize( TCHAR* pchPath )
{
    HRESULT						hr;
	LPOLECONTROL				pOleControl = NULL;
	LPCONNECTIONPOINTCONTAINER	pCPC = NULL;

    if ( NULL == m_pObj )
	{
        return SITE_ERROR_INVALID_OBJECT;
	}


    //We need an IOleObject most of the time, so get one here.
    hr = m_pObj->QueryInterface( IID_IOleObject, (void **)&m_pIOleObject );
    if ( FAILED( hr ) )
	{
        return SITE_ERROR_INVALID_OLE_OBJECT;
	}
    
	//SetClientSite is critical for DocObjects
	m_pIOleObject->SetClientSite( m_pImpIOleClientSite );

	if( m_pBidentView == NULL )
	{
		if (FAILED(hr = m_pObj->QueryInterface(IID_IOleControl, (LPVOID*)&pOleControl)))
		{
			return SITE_ERROR_INVALID_OLE_OBJECT;
		}

	//	hr = pOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL);

		pOleControl->Release();

		// Hook up sink to catch ready state property change
		if (FAILED(hr = m_pObj->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pCPC)))
		{
			return SITE_ERROR_INVALID_OLE_OBJECT;
		}

		if (FAILED(hr = pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &m_pCP)))
		{
			return SITE_ERROR_INVALID_OLE_OBJECT;
		}

		hr = m_pCP->Advise((LPUNKNOWN)(IPropertyNotifySink*)this, &m_dwCookie);
		pCPC->Release();
	}

    hr = m_pIOleObject->Advise( m_pImpIAdviseSink, &m_dwUnadvise );

    if ( FAILED( Load( pchPath ) ) )
	{
		return 	SITE_ERROR_UNABLE_TO_LOAD_FILE;
	}


	// Put the object in the running state
	OleRun( m_pIOleObject );
    return 	SITE_ERROR_NONE;
}



/*
 * CSite::Close
 *
 * Purpose:
 *  Possibly commits the storage, then releases it, afterwards
 *  frees alls the object pointers.
 *
 * Parameters:
 *  fCommit         BOOL indicating if we're to commit.
 *
 * Return Value:
 *  None
 */
void CSite::Close(BOOL fCommit)
{
	if( m_pDoc2 != NULL )
	{
		m_pDoc2->Release();
	}

    //OnInPlaceDeactivate releases this pointer.
    if ( NULL != m_pIOleIPObject )
	{
        m_pIOleIPObject->InPlaceDeactivate();
	}

	if( m_pCP != NULL )
	{
		m_pCP->Unadvise( m_dwCookie );
		ReleaseInterface( m_pCP );
		m_pCP = NULL;
	}

	ReleaseInterface( m_pIOleDocView );

	ReleaseInterface( m_pIOleCommandTarget );
    ReleaseInterface( m_pObj );
    
	if ( NULL != m_pIOleObject )
    {
        m_pIOleObject->Close( fCommit ? OLECLOSE_SAVEIFDIRTY : OLECLOSE_NOSAVE );
	    
		m_pIOleObject->SetClientSite( NULL );
		m_pIOleObject->Unadvise( m_dwUnadvise );

        ReleaseInterface( m_pIOleObject );
    }
}


/*
 * CSite::Activate
 *
 * Purpose:
 *  Activates a verb on the object living in the site.
 *
 * Parameters:
 *  iVerb           LONG of the verb to execute.
 *
 * Return Value:
 *  None
 */

void CSite::Activate( LONG iVerb )
{
    RECT rc;
            
    GetClientRect( m_hWnd, &rc );
    m_pIOleObject->DoVerb( iVerb, NULL, m_pImpIOleClientSite, 0, m_hWnd, &rc );
}




/*
 * CSite::UpdateObjectRects
 *
 * Purpose:
 *  Informs the site that the client area window was resized and
 *  that the site needs to also tell the DocObject of the resize.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CSite::UpdateObjectRects( void )
{

    if ( NULL != m_pIOleDocView )
	{
		RECT    rc;
        
	    GetClientRect(m_hWnd, &rc);
		m_pIOleDocView->SetRect(&rc);
	}
}


/*
 * CSite::Load
 *
 * Purpose:
 *  Loads the path provided using IPersistMoniker or IPersistFile. If no path
 *	was provided it simply does an InitNew
 *
 * Parameters:
 *  pchPath - The path
 *
 * Return Value:
 */
HRESULT CSite::Load(TCHAR* pchPath)
{
    HRESULT   hr = S_OK;

    if ( pchPath != NULL && *pchPath != 0)
    {
		// Path has been provided so check should we use IPersisMoniker or IPersistFile?
        if (memcmp(pchPath, _T("file:"), 5 * sizeof(TCHAR)) == 0 ||
            memcmp(pchPath, _T("http:"), 5 * sizeof(TCHAR)) == 0)
        {
		    OLECHAR  szwName[256];
			MultiByteToWideChar(CP_ACP, 0, pchPath, -1, szwName, 256);    

			// Ask the system for a URL Moniker
			IMoniker* pIMoniker;
			hr = CreateURLMoniker(NULL, (LPWSTR)szwName, &pIMoniker);
		    if ( SUCCEEDED(hr) )
			{
				// Get the IPersistMoniker interface
			    IPersistMoniker* pPMk;
				hr = m_pObj->QueryInterface(
										IID_IPersistMoniker,
										(void **)&pPMk);
			    if ( SUCCEEDED(hr) )
				{

					// note: do not register our BSC when calling IPM::Load directly
					IBindCtx *pBCtx;
					hr = CreateBindCtx(0, &pBCtx);
				    if ( SUCCEEDED(hr) )
					{
						// Call Load on the IPersistMoniker
						hr = pPMk->Load(FALSE, pIMoniker, pBCtx, STGM_READ);

						pBCtx->Release();
					}
				    ReleaseInterface(pPMk);
				}
				ReleaseInterface( pIMoniker );
			}
        }
		else
        {
		    OLECHAR  szwName[256];
			MultiByteToWideChar(CP_ACP, 0, pchPath, -1, szwName, 256);    
		    IPersistFile*  pPFile;
            hr = m_pObj->QueryInterface(
									IID_IPersistFile,
									(void **) &pPFile);
		    if ( SUCCEEDED(hr) )
			{
				// Call Load on the IPersistFile
				hr = pPFile->Load((LPWSTR)szwName, 0);
			    ReleaseInterface(pPFile);
			}
        }
    }
	else
	{
// This makes the control unusable for Bident, so we don't do it
//
//		// No path provided so just do an InitNew on the Stream 
//	    IPersistStreamInit* pPStm;
//		hr = m_pObj->QueryInterface(
//					IID_IPersistStreamInit,
//					(void **)&pPStm);
//	    if ( SUCCEEDED(hr) )
//		{
//			hr = pPStm->InitNew();
//		    ReleaseInterface(pPStm);
//		}
	}

	return hr;
}


HRESULT CSite::Save(TCHAR* pchPath)
{
    HRESULT   hr = S_OK;

    if ( pchPath != NULL && *pchPath != 0)
    {
	    OLECHAR  szwName[256];
		MultiByteToWideChar(CP_ACP, 0, pchPath, -1, szwName, 256);    
	    IPersistFile*  pPFile;
        hr = m_pObj->QueryInterface(
									IID_IPersistFile,
									(void **) &pPFile);
	    if ( SUCCEEDED(hr) )
		{
			// Call Load on the IPersistFile
			hr = pPFile->Save((LPWSTR)szwName, TRUE);
		    ReleaseInterface(pPFile);
	    }
    }

	return hr;
}


BOOL CSite::IsEditing()
{
	if( m_pTridentView != NULL )
	{
		return m_pTridentView->IsEditing();
	}

	return FALSE;
}

CWnd*	CSite::GetFrame( void )
{ 
	if( m_pTridentView != NULL )
	{
		return m_pTridentView;
	}

	return m_pBidentView;
}


IHTMLDocument2* CSite::GetDoc2()
{
	HRESULT hr;

	if( m_pDoc2 == NULL )
	{
		hr = m_pObj->QueryInterface(IID_IHTMLDocument2, (void **)( &m_pDoc2 ) );
	}
	
	return m_pDoc2;
}


IHTMLBodyElement* CSite::GetBody()
{
	HRESULT				hr;
	IHTMLElement*		pElement;
	IHTMLBodyElement*	pBody;

	pElement = NULL;
	pBody = NULL;
	
	if( m_pDoc2 == NULL )
	{
		hr = m_pObj->QueryInterface(IID_IHTMLDocument2, (void **)( &m_pDoc2 ) );
	}
	
	if( m_pDoc2 )
	{
		hr = m_pDoc2->get_body( &pElement );

		if( ( hr == S_OK ) && pElement )
		{
			hr = pElement->QueryInterface( IID_IHTMLBodyElement, (void**) &pBody );
			pElement->Release();
			pElement = NULL;
		}
	}

	return pBody;
}


// Fired on change of the value of a 'bindable' property
STDMETHODIMP CSite::OnChanged(DISPID dispID)
{
	HRESULT			hr;

	if (DISPID_READYSTATE == dispID)
	{
		// check the value of the readystate property
		ASSERT( m_pObj );

		VARIANT vResult = {0};
		EXCEPINFO excepInfo;
		UINT uArgErr;

		DISPPARAMS dp = {NULL, NULL, 0, 0};
		VERIFY ( GetDoc2() );
		if (SUCCEEDED(hr = m_pDoc2->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
			DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr)))
		{
			ASSERT(VT_I4 == V_VT(&vResult));
			m_lReadyState = (READYSTATE)V_I4(&vResult);
			switch (m_lReadyState)
			{	
			case READYSTATE_UNINITIALIZED:	//= 0,
				//TRACE( "ReadyState = READYSTATE_UNINITIALIZED\n" );
				break;
			case READYSTATE_LOADING: //	= 1,
				//TRACE( "ReadyState = READYSTATE_LOADING\n" );
				break;	
			case READYSTATE_LOADED:	// = 2,
				//TRACE( "ReadyState = READYSTATE_LOADED\n" );
				break;
			case READYSTATE_INTERACTIVE: //	= 3,
				//TRACE( "ReadyState = READYSTATE_INTERACTIVE\n" );
				break;
			case READYSTATE_COMPLETE: // = 4
				//TRACE( "ReadyState = READYSTATE_COMPLETE\n" );
				if( m_pTridentView )
				{
					m_pTridentView->PostMessage( wmReadyStateComplete, 0, 0 );
				}
				
				if( m_pBidentView )
				{
					m_pBidentView->PostMessage( wmReadyStateComplete, 0, 0 );
				}
				break;
			}
			VariantClear(&vResult);
		}
	}

	return NOERROR;
}


STDMETHODIMP CSite::OnRequestEdit(DISPID dispID)
{
	// Property changes are OK any time as far as this app is concerned
	return NOERROR;
}

#ifdef ADWARE_PROTOTYPE
DWORD CSite::GetDocHostUIFlags() {return m_pTridentView ? m_pTridentView->GetDocHostUIFlags():m_pBidentView->GetDocHostUIFlags();}
void CSite::SetDocHostUIFlags(DWORD dwFlags)	{m_pTridentView ? m_pTridentView->SetDocHostUIFlags(dwFlags):m_pBidentView->SetDocHostUIFlags(dwFlags);}
#endif
