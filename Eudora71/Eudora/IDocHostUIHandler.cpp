/*
 * IDOCHOSTUIHANDLER.CPP
 * IDocHostUIHandler for Document Objects CSite class
 *
 * Copyright (c)1995-1996 Microsoft Corporation, All Rights Reserved
 */


#include "stdafx.h"

#include <docobj.h>

#include "site.h"
#include "resource.h"
#include "rs.h"
#include "guiutils.h"

#include "DebugNewHelpers.h"

/*
 * CImpIDocHostUIHandler::CImpIDocHostUIHandler
 * CImpIDocHostUIHandler::~CImpIDocHostUIHandler
 *
 * Parameters (Constructor):
 *  pSite           PCSite of the site we're in.
 *  pUnkOuter       LPUNKNOWN to which we delegate.
 */
CImpIDocHostUIHandler::CImpIDocHostUIHandler( CSite* pSite, IUnknown* pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIDocHostUIHandler::~CImpIDocHostUIHandler( void )
{
}



/*
 * CImpIDocHostUIHandler::QueryInterface
 * CImpIDocHostUIHandler::AddRef
 * CImpIDocHostUIHandler::Release
 *
 * Purpose:
 *  IUnknown members for CImpIOleDocumentSite object.
 */
STDMETHODIMP CImpIDocHostUIHandler::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIDocHostUIHandler::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIDocHostUIHandler::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}



// * CImpIDocHostUIHandler::GetHostInfo
// *
// * Purpose: Called at initialisation
// *
STDMETHODIMP CImpIDocHostUIHandler::GetHostInfo( DOCHOSTUIINFO* pInfo )
{
    pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DIV_BLOCKDEFAULT;
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

    return S_OK;
}

// * CImpIDocHostUIHandler::ShowUI
// *
// * Purpose: Called when MSHTML.DLL shows its UI
// *
STDMETHODIMP CImpIDocHostUIHandler::ShowUI(
				DWORD dwID, 
				IOleInPlaceActiveObject * /*pActiveObject*/,
				IOleCommandTarget * pCommandTarget,
				IOleInPlaceFrame * /*pFrame*/,
				IOleInPlaceUIWindow * /*pDoc*/)
{
#ifdef OLD
	// Just to show this works we'll set the Browse/Edit mode flag using the ID
	if ( dwID == 1 /* hack cos editing removed from header*/ )
	{
		m_pSite->GetFrame()->m_bBrowse = FALSE;
	}
	else
	{
		m_pSite->GetFrame()->m_bBrowse = TRUE;
	}
#endif

	// If we havent already got the CommandTraget interface then store it away now.
	if ( m_pSite->GetCommandTarget() == NULL )
	{
		if ( pCommandTarget != NULL )
		{
			// Don't forget to AddRef or MSHTML.DLL will assume we aren't using it, and it'll go away.
			pCommandTarget->AddRef();
			m_pSite->SetCommandTarget( pCommandTarget );
		}
	}

	// We've already got our own UI in place so just return S_OK
    return S_OK;
}

// * CImpIDocHostUIHandler::HideUI
// *
// * Purpose: Called when MSHTML.DLL hides its UI
// *
STDMETHODIMP CImpIDocHostUIHandler::HideUI(void)
{
    return S_OK;
}

// * CImpIDocHostUIHandler::UpdateUI
// *
// * Purpose: Called when MSHTML.DLL updates its UI
// *
STDMETHODIMP CImpIDocHostUIHandler::UpdateUI(void)
{
	// MFC is pretty good about updating it's UI in it's Idle loop so I don't do anything here
	return S_OK;
}

// * CImpIDocHostUIHandler::EnableModeless
// *
// * Purpose: Called from MSHTML.DLL's IOleInPlaceActiveObject::EnableModeless
// *
STDMETHODIMP CImpIDocHostUIHandler::EnableModeless(BOOL /*fEnable*/)
{
    return E_NOTIMPL;
}

// * CImpIDocHostUIHandler::OnDocWindowActivate
// *
// * Purpose: Called from MSHTML.DLL's IOleInPlaceActiveObject::OnDocWindowActivate
// *
STDMETHODIMP CImpIDocHostUIHandler::OnDocWindowActivate(BOOL /*fActivate*/)
{
    return E_NOTIMPL;
}

// * CImpIDocHostUIHandler::OnFrameWindowActivate
// *
// * Purpose: Called from MSHTML.DLL's IOleInPlaceActiveObject::OnFrameWindowActivate
// *
STDMETHODIMP CImpIDocHostUIHandler::OnFrameWindowActivate(BOOL /*fActivate*/)
{
    return E_NOTIMPL;
}

// * CImpIDocHostUIHandler::ResizeBorder
// *
// * Purpose: Called from MSHTML.DLL's IOleInPlaceActiveObject::ResizeBorder
// *
STDMETHODIMP CImpIDocHostUIHandler::ResizeBorder(
				LPCRECT /*prcBorder*/, 
				IOleInPlaceUIWindow* /*pUIWindow*/,
				BOOL /*fRameWindow*/)
{
    return E_NOTIMPL;
}

// * CImpIDocHostUIHandler::ShowContextMenu
// *
// * Purpose: Called when MSHTML.DLL would normally display its context menu
// *
STDMETHODIMP CImpIDocHostUIHandler::ShowContextMenu(
				DWORD /*dwID*/, 
				POINT* /*pptPosition*/,
				IUnknown* /*pCommandTarget*/,
				IDispatch* /*pDispatchObjectHit*/)
{
    return E_NOTIMPL;
}

// * CImpIDocHostUIHandler::TranslateAccelerator
// *
// * Purpose: Called from MSHTML.DLL's TranslateAccelerator routines
// *
STDMETHODIMP CImpIDocHostUIHandler::TranslateAccelerator(LPMSG lpMsg,
            /* [in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ DWORD nCmdID)
{
    return S_FALSE;
}

// * CImpIDocHostUIHandler::GetOptionKeyPath
// *
// * Purpose: Called by MSHTML.DLL to find where the host wishes to store 
// *	its options in the registry
// *
STDMETHODIMP CImpIDocHostUIHandler::GetOptionKeyPath(BSTR* pbstrKey, DWORD)
{
	if (GetIniShort(IDS_INI_SEPARATE_MSHTML_SETTINGS))
	{
		const WCHAR RegPath[] = L"Software\\Qualcomm\\Eudora\\MSHTML";
		ULONG ulSize = sizeof(RegPath) + sizeof(WCHAR);
		
		*pbstrKey = (BSTR)CoTaskMemAlloc(ulSize);
		if (*pbstrKey)
			wcscpy((LPWSTR)*pbstrKey, RegPath);
	}

	return S_OK;
}

STDMETHODIMP CImpIDocHostUIHandler::GetDropTarget( 
            /* [in] */ IDropTarget __RPC_FAR *pDropTarget,
            /* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImpIDocHostUIHandler::GetExternal( 
    /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImpIDocHostUIHandler::TranslateUrl( 
    /* [in] */ DWORD dwTranslate,
    /* [in] */ OLECHAR __RPC_FAR *pchURLIn,
    /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	// Hackety-Hack-Hack!
	// Can't do anything to convince it not to launch a URL, so the best thing
	// is to let it bring up something that's benign: a blank page.
	const OLECHAR NewURL[] = L"about:blank";

	*ppchURLOut = (OLECHAR*)CoTaskMemAlloc(sizeof(NewURL) + sizeof(OLECHAR));
	if (*ppchURLOut)
		wcscpy(*ppchURLOut, NewURL);

	// Call our own launch routine which will warn on questionable content.
	CString csURL(pchURLIn);
	LaunchURL(csURL);

	return S_OK;
//    return E_NOTIMPL;
}

STDMETHODIMP CImpIDocHostUIHandler::FilterDataObject( 
    /* [in] */ IDataObject __RPC_FAR *pDO,
    /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
    return E_NOTIMPL;
}
