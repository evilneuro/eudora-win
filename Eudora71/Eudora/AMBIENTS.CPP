/*
 * Ambients.CPP
 * IDispatch for Document Events
 *
 * Copyright (c)1995-1997 Microsoft Corporation, All Rights Reserved
 */

#include "stdafx.h"
#include "Ambients.h"
#include "site.h"

#include "mshtmdid.h"

#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"


/*
 * CImpIDispatch::CImpIDispatch
 * CImpIDispatch::~CImpIDispatch
 *
 * Parameters (Constructor):
 *  pSite           PCSite of the site we're in.
 *  pUnkOuter       LPUNKNOWN to which we delegate.
 */

CImpIDispatch::CImpIDispatch( PCSite pSite, LPUNKNOWN pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIDispatch::~CImpIDispatch( void )
{
	ASSERT( m_cRef == 0 );
}


/*
 * CImpIDispatch::QueryInterface
 * CImpIDispatch::AddRef
 * CImpIDispatch::Release
 *
 * Purpose:
 *  IUnknown members for CImpIDispatch object.
 */

STDMETHODIMP CImpIDispatch::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;


    if ( IID_IDispatch == riid || DIID_HTMLDocumentEvents == riid )
	{
        *ppv = this;
	}
	
	if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIDispatch::AddRef(void)
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIDispatch::Release(void)
{
    --m_cRef;
    return m_pUnkOuter->Release();
}


//IDispatch
STDMETHODIMP CImpIDispatch::GetTypeInfoCount(UINT* /*pctinfo*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImpIDispatch::GetTypeInfo(/* [in] */ UINT /*iTInfo*/,
            /* [in] */ LCID /*lcid*/,
            /* [out] */ ITypeInfo** /*ppTInfo*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImpIDispatch::GetIDsOfNames(
            /* [in] */ REFIID /*riid*/,
            /* [size_is][in] */ LPOLESTR* /*rgszNames*/,
            /* [in] */ UINT /*cNames*/,
            /* [in] */ LCID /*lcid*/,
            /* [size_is][out] */ DISPID* /*rgDispId*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImpIDispatch::Invoke(
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID /*lcid*/,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS* pDispParams,
            /* [out] */ VARIANT* pVarResult,
            /* [out] */ EXCEPINFO* /*pExcepInfo*/,
            /* [out] */ UINT* puArgErr)
{
	HRESULT hr;
	VARIANT	varResult;

    if (IID_NULL!=riid)
        return ResultFromScode(E_INVALIDARG);

    if(NULL==pVarResult)
      pVarResult=&varResult;

    VariantInit(pVarResult);

    //The most common case is boolean, use as an initial type
    V_VT(pVarResult)=VT_BOOL;

    /*
     * Process the requested ambient property.  Anything but a
     * request for a property is invalid, so we can check that
     * before looking at the specific ID.  We can only get away
     * with this because all properties are read-only.
     */

    if (!(DISPATCH_PROPERTYGET & wFlags))
        return ResultFromScode(DISP_E_MEMBERNOTFOUND);

    hr=S_OK;
	CComVariant vtResult;

    switch (dispIdMember)
	{
        case DISPID_AMBIENT_USERMODE:
            V_BOOL(pVarResult) = ( short ) ( m_pSite->IsEditing() );
            break;

		case DISPID_AMBIENT_DLCONTROL:            

			if (!GetIniShort(IDS_INI_RUN_HTML_CODE))
			{
				// OR the required flags which dictate how the document behaves ..
				vtResult = static_cast<long>(
										DLCTL_BGSOUNDS |			  // The browsing component will play background sounds associated with the document
										DLCTL_DLIMAGES |			  // The browsing component will download images from the server.
										DLCTL_VIDEOS |				  // The browsing component will play any video clips that are contained in the document.
										DLCTL_OFFLINEIFNOTCONNECTED | // The browsing component will operate in offline mode if not connected to the Internet. 
										DLCTL_NO_SCRIPTS | 			  // The browsing component will not execute any scripts.
										DLCTL_NO_JAVA |				  // The browsing component will not execute any Java applets.
										DLCTL_NO_RUNACTIVEXCTLS |     // The browsing component will not execute any ActiveX Controls in the document.
										DLCTL_NO_DLACTIVEXCTLS		  // The browsing component will not download any ActiveX Controls in the document.
										);
				*pVarResult = vtResult;
				break;
			}

        default:
            hr=ResultFromScode(DISP_E_MEMBERNOTFOUND);
            break;
	}

    return hr;
	
#ifdef OK1
		// Let's get hold of the event object to find out what the x and y coordinates of the event were.

		// First let's get the doc
		IUnknown* lpUnk = m_pView->m_pSite->GetObjectUnknown();

		if ( lpUnk != NULL )
		{
			HRESULT         hr;

			// Then, request the "document" object from the object
			IHTMLDocument2* pHTMLDocument2;
			hr = lpUnk->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);
			if ( SUCCEEDED( hr ) )
			{
				// Get the window object
				IHTMLWindow2* pWindow;
				hr = pHTMLDocument2->get_parentWindow( &pWindow );
				if ( SUCCEEDED( hr ) )
				{
					// Get the event object
					IHTMLEventObj* pEventObj;
					hr = pWindow->get_event( &pEventObj );
					if ( SUCCEEDED( hr ) )
					{
						long x, y;
						// get x and y
						pEventObj->get_x( &x );

						pEventObj->get_y( &y );

						CMainFrame* pFrame = (CMainFrame*)m_pView->GetTopLevelFrame();

						pFrame->m_nXPos = x;

						pFrame->m_nYPos = y;


						pEventObj->Release();
					}
					pWindow->Release();
				}
				pHTMLDocument2->Release();
			}
		}
	}

	return S_OK;
#endif
}
