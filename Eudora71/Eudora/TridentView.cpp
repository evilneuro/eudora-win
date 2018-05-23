// TridentView.cpp : implementation of the CTridentView class
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

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
#include "TridentReadMessageView.h"
#include "ReadMessageDoc.h"
#include "TridentView.h"
#include "msgdoc.h"
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
#include "TocDoc.h"
#include "summary.h"
#include "convhtml.h"
#include "Text2HTML.h"
#include "bstr.h"
#include "utils.h"
#include "URLUtils.h"

#include "trnslate.h"
#include "RegInfoReader.h"

#include "QCOleDropSource.h"

// Support for link warning tooltip style popup text.
#include "PopupText.h"
#include "UpdatedMsHTML.h"

#include "DebugNewHelpers.h"

#define ODS(x) OutputDebugString(x)

extern QCRecipientDirector	g_theRecipientDirector;
extern CString EudoraDir;


//	Constants
static const int kMaxCharsInContextSearchSelection = 120;


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
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrintOne)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_ATTACHMENT, OnUpdateFileOpenAttachment)
	ON_COMMAND(ID_FILE_OPEN_ATTACHMENT, OnFileOpenAttachment)
//	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP

	ON_COMMAND( ID_VIEW_SOURCE, OnViewSource )
	ON_COMMAND( ID_SEND_TO_BROWSER, OnSendToBrowser)

	ON_COMMAND( ID_EDIT_COPY, OnCopy )
	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY, OnUpdateCopy )

	ON_COMMAND(ID_COPY_ATTACHMENT, OnCopyAttachment)
	ON_COMMAND_RANGE(ID_SAVEAS_ATTACHMENT, ID_DELETE_ATTACHMENT, OnAttachmentAction)
	ON_UPDATE_COMMAND_UI_RANGE(ID_COPY_ATTACHMENT, ID_DELETE_ATTACHMENT, OnUpdateAttachmentAction)

	ON_COMMAND( ID_SPECIAL_ADDASRECIPIENT, OnSpecialAddAsRecipient )
	ON_UPDATE_COMMAND_UI( ID_SPECIAL_ADDASRECIPIENT, OnUpdateAddAsRecipient )

	ON_COMMAND( ID_EDIT_SELECT_ALL, OnSelectAll )
	ON_UPDATE_COMMAND_UI( ID_EDIT_SELECT_ALL, OnUpdateSelectAll )

	ON_COMMAND( ID_SEARCH_WEB_FOR_SEL, OnSearchWebForSelection )
	ON_UPDATE_COMMAND_UI( ID_SEARCH_WEB_FOR_SEL, OnUpdateSearchWebForSelection )

	ON_COMMAND( ID_SEARCH_EUDORA_FOR_SEL, OnSearchEudoraForSelection )
	ON_UPDATE_COMMAND_UI( ID_SEARCH_EUDORA_FOR_SEL, OnUpdateSearchEudoraForSelection )

	ON_COMMAND( ID_SEARCH_MAILBOX_FOR_SEL, OnSearchMailboxForSelection )
	ON_UPDATE_COMMAND_UI( ID_SEARCH_MAILBOX_FOR_SEL, OnUpdateSearchMailboxForSelection )

	ON_COMMAND( ID_SEARCH_MAILFOLDER_FOR_SEL, OnSearchMailfolderForSelection )
	ON_UPDATE_COMMAND_UI( ID_SEARCH_MAILFOLDER_FOR_SEL, OnUpdateSearchMailfolderForSelection )

	ON_REGISTERED_MESSAGE( wmReadyStateComplete, FixupSource )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTridentView construction/destruction

UINT CTridentView::s_uTmpFileUnique = 0;

CTridentView::CTridentView() 
{
	m_bstrBookmark = NULL;
    m_cRef = 0;
    m_hWndObj = NULL;
            
    m_pSite = NULL;
    m_pIOleIPActiveObject = NULL;
    m_fCreated = FALSE;
        
    m_szTmpFile = "";
    m_bFixedupSource = FALSE;
    m_nIsVisible = 0;

	m_bWasConcentrated = false;
	m_bCanConcentrate = false;

    m_bEnableAttachmentMenus = FALSE;
}


CTridentView::~CTridentView()
{
	unlink( m_szTmpFile );

	// IE 5.x isn't calling Release() for each of its AddRef()s when we use it in an open message
	// window, which makes this assert really annoying so we're commenting it out for now
	//ASSERT( m_cRef == 0 );
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
	//	New item - cancel popup text
	CPopupText::Instance()->Cancel();
	
	CFrameWnd* pFrame = GetTopLevelFrame();
	if ( pFrame != NULL )
	{
		USES_CONVERSION;

		CString		szText = OLE2T(pszText);

		//	Keep an unmodified version of the URL
		CString		szURL = szText;

		//	Check for space runs of 5 or greater. If found, remove all spaces
		//	from URL as displayed to user.
		//
		//	Prevents obfuscation of URL reported in security forums where a
		//	user name followed by a bunch of spaces then @ the real URL is
		//	used to make only the user name be displayed. For example:
		//	"http://www.e-gold.com
		//	&#32&#32&#32&#32&#32&#32&#32&#32&#32&#32 (repeated a whole bunch of times)
		//	@example.com/"
		if (szText.Find("     ") >= 0)
			szText.Remove(' ');

		pFrame->SetMessageText(szText);

		CPoint				ptCursor;

		//	We'll check with ScamWatch if:
		//	* We're using the full feature set
		//	* The user has ScamWatch ToolTips turned on
		//	* There's status text
		//	* We can get the cursor position
		if ( UsingFullFeatureSet() && GetIniShort(IDS_INI_SHOW_SCAMWATCH_TOOLTIP) &&
			 !szText.IsEmpty() && GetCursorPos(&ptCursor) )
		{
			CPoint		ptAnchor = ptCursor;
			
			//	Figure out if the cursor is over an anchor element
			ScreenToClient(&ptAnchor);
			
			HRESULT					hr;
			IHTMLElement *			pElement = GetAnchorElement(ptAnchor);
			IHTMLAnchorElement *	pAnchor = NULL;

			if (pElement)
			{
				hr = pElement->QueryInterface(IID_IHTMLAnchorElement, reinterpret_cast<void **>(&pAnchor));
				if (hr != S_OK)
					pAnchor = NULL;
			}
			
			if (pAnchor)
			{
				//	The cursor is over an anchor element.
				CBstr		cbstr;

				CString		szLinkText;

				//	We could get the URL here via pAnchor->get_href, but that
				//	wouldn't interpret HTML encoding, so instead we just use
				//	the text passed to us that we stashed above.

				//	Get the link text
				hr = pElement->get_innerText( ( BSTRARG) cbstr );
				if ( (hr == S_OK) && !cbstr.IsNull() )
					szLinkText = cbstr;

				if (szURL)
				{
					//	Is the link text combined with the URL target naughty?
					UINT		nURLIsNaughty = CURLInfo::IsURLNaughty(szURL, szLinkText);

					if (nURLIsNaughty)
					{
						//	The URL is naughty - format the popup message to display to the user.
						CString		szMessage;

						szMessage.Format( nURLIsNaughty, static_cast<LPCTSTR>(szURL), static_cast<LPCTSTR>(szLinkText) );
						
						//	Set the popup text
						if ( CPopupText::WasCreated() )
							CPopupText::Instance()->SetText(szMessage);

						//	Attempt to calculate the link's top left corner.
						CPoint				ptLinkTopLeft(0, 0);						
						IHTMLElement *		pScanElement = NULL;

						//	Start with the element's offset parent
						hr = pElement->get_offsetParent(&pScanElement);

						if ( (hr == S_OK) && pScanElement )
						{
							//	Start out with the offset top, then adjust it for the
							//	current scroll positions of every parent container element.
							hr = pElement->get_offsetTop(&ptLinkTopLeft.y);
							
							IHTMLElement2 *			pScanElement2;
							IHTMLElement *			pParentElement;
							long					nTop;
							
							//	Work our way up the chain of elements checking
							//	out the scroll top of each.
							while ( (hr == S_OK) && (pScanElement != NULL) )
							{
								//	Get the IHTMLElement2 interface for the current scan element
								hr = pScanElement->QueryInterface( IID_IHTMLElement2, reinterpret_cast<void **>(&pScanElement2) );
								
								if ( (hr != S_OK) || !pScanElement2 )
									break;

								//	Get the offset top of the current scan element
								hr = pScanElement->get_offsetTop(&nTop);
								if (hr != S_OK)
									break;
								
								//	Add the offset top to the reported offset top
								ptLinkTopLeft.y += nTop;
								
								//	Get the scroll top of the current scan element
								hr = pScanElement2->get_scrollTop(&nTop);
								if (hr != S_OK)
									break;
								
								//	Subtract the scroll top from the reported offset top
								ptLinkTopLeft.y -= nTop;
								
								//	Get the parent for the current scan element
								pParentElement = NULL;
								hr = pScanElement->get_offsetParent(&pParentElement);
								
								//	Release the previous element
								pScanElement->Release();
								pScanElement2->Release();
								
								//	Move to the parent if we found it
								pScanElement = (hr == S_OK) ? pParentElement : NULL;
							}
							
							//	Release the last scan element, if any
							if (pScanElement)
								pScanElement->Release();
							
							if (hr == S_OK)
								ClientToScreen(&ptLinkTopLeft);	
						}
						else
						{
							hr = S_FALSE;
						}

						//	If an error occurred bail and just use the cursor location
						if (hr != S_OK)
							ptLinkTopLeft = ptCursor;

						LONG		nLinkHeight, nLinkBottom;

						//	Try to use the actual link height, but if we can't then just use 20.
						hr = pElement->get_offsetHeight(&nLinkHeight);
						if (hr == S_OK)
							nLinkBottom = ptLinkTopLeft.y + nLinkHeight;
						else
							nLinkBottom = ptLinkTopLeft.y + 20;

						//	Calculate a rough rect of where the link is. We attempted above
						//	to calculate the top and bottom accurately, since that's used
						//	to position the popup text. For the left and right, we'll just
						//	use then entire client rect because it's only used as a way to
						//	make sure that the popup text goes away when the cursor is
						//	moved outside of us (since SetStatusText is no longer called when
						//	the cursor moves outside of us).
						CRect		rectLinkLimits;

						GetClientRect(rectLinkLimits);
						ClientToScreen(rectLinkLimits);

						rectLinkLimits.top = ptLinkTopLeft.y;
						rectLinkLimits.bottom = nLinkBottom;

						if ( CPopupText::WasCreated() )
						{
							//	Position the popup text window based on the cursor location
							//	and the link text rectangle.
							CPopupText::Instance()->PositionWindow(rectLinkLimits, AfxGetMainWnd(), ptCursor);
							
							//	Show the popup text after a short delay
							CPopupText::Instance()->ShowDelayed(5);
						}
					}
				}

				pAnchor->Release();
			}

			if (pElement)
				pElement->Release();
		}
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
				OLECMD prgCmds[], OLECMDTEXT* pCmdText )
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
	m_pSite = DEBUG_NEW_NOTHROW CSite( m_hWnd, this );

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

		lpUnk->QueryInterface(IID_IOleCommandTarget, (void **)&pCommandTarget);

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

void CTridentView::OnUpdateAttachmentAction(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( m_bEnableAttachmentMenus );
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


void CTridentView::OnUpdateSearchForSelection(
	CCmdUI *				pCmdUI,
	UINT					nMenuItemWithSelectionFormat,
	UINT					nMenuItemDefaultText)
{
	BOOL		bLoadGood = TRUE;
	CString		szText;
	CString		szMenuItemText;
	
	if ( GetSelectedTextAndTrim(szText, true, true, kMaxCharsInContextSearchSelection, true) )
		szMenuItemText.Format(nMenuItemWithSelectionFormat, szText);
	else
		bLoadGood = szMenuItemText.LoadString(nMenuItemDefaultText);

	if (bLoadGood)
		pCmdUI->SetText(szMenuItemText);
	
	pCmdUI->Enable(TRUE);
}


void CTridentView::OnSearchInEudoraForSelection(
	bool								in_bSelectParent,
	bool								in_bSelectAll)
{
	// Get the selected text and trim spaces.
	CString			 szText;

	GetSelectedTextAndTrim(szText, true, true, kMaxCharsInContextSearchSelection);
	
	const char *	szMailboxName = NULL;

	CTocDoc* pTocDoc = GetTocDoc();
	if (pTocDoc)
		szMailboxName = pTocDoc->GetMBFileName();
	
	OpenFindMessagesWindow(szMailboxName, szText, true, in_bSelectParent, in_bSelectAll, true);
}


void CTridentView::OnUpdateSearchWebForSelection(CCmdUI* pCmdUI)
{
	OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_WEB_FOR_SEL_MENU_FORMAT, IDS_SEARCH_WEB_FOR_SEL_MENU);
}


void CTridentView::OnSearchWebForSelection()
{
	// Get the selected text and trim spaces.
	CString			 szText;
	
	GetSelectedTextAndTrim(szText, true, true, kMaxCharsInContextSearchSelection);
	LaunchURLWithQuery(NULL, ACTION_SEARCH_WEB, szText, NULL, NULL, NULL, NULL, ACTION_SEARCH_PARAM_LABEL);
}


void CTridentView::OnUpdateSearchEudoraForSelection(CCmdUI* pCmdUI)
{
	OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_EUDORA_FOR_SEL_MENU_FORMAT, IDS_SEARCH_EUDORA_FOR_SEL_MENU);
}


void CTridentView::OnSearchEudoraForSelection()
{
	//	Search in Eudora for the selection, selecting all mailboxes
	OnSearchInEudoraForSelection(false, true);
}


void CTridentView::OnUpdateSearchMailboxForSelection(CCmdUI* pCmdUI)
{
	OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_MAILBOX_FOR_SEL_MENU_FORMAT, IDS_SEARCH_MAILBOX_FOR_SEL_MENU);
}


void CTridentView::OnSearchMailboxForSelection()
{
	//	Search in Eudora for the selection, selecting the current mailbox
	OnSearchInEudoraForSelection(false, false);
}


void CTridentView::OnUpdateSearchMailfolderForSelection(CCmdUI* pCmdUI)
{
	OnUpdateSearchForSelection(pCmdUI, IDS_SEARCH_MAILFOLDER_FOR_SEL_MENU_FORMAT, IDS_SEARCH_MAILFOLDER_FOR_SEL_MENU);
}


void CTridentView::OnSearchMailfolderForSelection()
{
	//	Search in Eudora for the selection, selecting the parent folder of the current mailbox
	OnSearchInEudoraForSelection(true, false);
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


void
CTridentView::SetTitleIfNecessary(CMessageDoc * in_pDoc)
{
	//	Don't do anything by default
}


BOOL
CTridentView::WriteTempFile( 
	CFile &								theFile,
	ContentConcentrator::ContextT		in_context,
	bool								in_bIsBlahBlahBlah)
{
	BOOL		bWriteGood = TRUE;
	
	//	Get the font for the style sheet
	CString szFontName;
	if ( UseFixedFont() )
		szFontName = GetIniString(IDS_INI_MESSAGE_FIXED_FONT);
	else
		szFontName = GetIniString(IDS_INI_MESSAGE_FONT);
	
	//	Get the format string for the style sheet
	CString		szStyleSheetFormat;
	try
	{
		CString		szStyleSheetFileName = EudoraDir + "read.css";
		CFile		theStyleSheetFormatFile;
		
		if( theStyleSheetFormatFile.Open( szStyleSheetFileName, CFile::modeRead | CFile::typeBinary ) )
		{
			LPTSTR	pBuf = szStyleSheetFormat.GetBufferSetLength( static_cast<int>(theStyleSheetFormatFile.GetLength()) + 1 );
			
			if( pBuf )
			{
				theStyleSheetFormatFile.Read( pBuf, static_cast<UINT>(theStyleSheetFormatFile.GetLength()) );
				pBuf[ theStyleSheetFormatFile.GetLength() ] = '\0';
			}
		}
	}
	catch( CException* pExp )
	{
		szStyleSheetFormat.Empty();
		szStyleSheetFormat.FreeExtra();
		pExp->Delete();
	}
	
	if (szStyleSheetFormat.IsEmpty())
	{
		// there's no read.css, so get it from the .ini 
		// file or the resource file.
		szStyleSheetFormat = GetIniString(IDS_INI_READMESSAGE_STYLE_SHEET);
	}
	
	//	Format the style sheet
	CString		szStyleSheet;
	szStyleSheet.Format( szStyleSheetFormat,
		(LPCSTR) szFontName,
		(LPCSTR) GetIniString(IDS_INI_MESSAGE_FIXED_FONT),
		(LPCSTR) GetIniString(IDS_INI_EXCERPT_BARS) );

	//	Write out the style sheet
	try
	{			
		theFile.Write( szStyleSheet, szStyleSheet.GetLength() );
	}
	catch( CException* pExp )
	{
		pExp->Delete();
		bWriteGood = FALSE;
	}

	bool			bLoadedDoc = false;
	CSummary *		pSummary = NULL;
	int				nNumMessageDocs = GetNumMessageDocs();
	CString			szDivStart = "<div>\r\n";
	CString			szDivEnd = "\r\n</div>";
	CString			szMessageDivider = "<br><hr>\r\n";

	for (int i = 0; i < nNumMessageDocs; i++)
	{
		CMessageDoc *	pDoc = GetMessageDoc(i, &bLoadedDoc, &pSummary);
		
		if (!pDoc)
		{
			if (!pSummary)
				bWriteGood = IsNoMessageToDisplayAllowed();
			
			ASSERT(bWriteGood);
			break;
		}
		
		ASSERT_KINDOF(CMessageDoc, pDoc);
		
		//	Get the whole message
		char *	szFullMessage = pDoc->GetFullMessage( RAW );
		
		// One reason this could fail is if the message couldn't be retrieved from an IMAP server
		if (!szFullMessage) 
		{
			if (bLoadedDoc)
			{
				pSummary->NukeMessageDocIfUnused();
				pDoc = NULL;
			}
			
			bWriteGood = IsNoMessageToDisplayAllowed();
			break;
		}
		
		//	Won't do anything by default (in particular no-op for preview view)
		SetTitleIfNecessary(pDoc);

		pDoc->m_QCMessage.Init(pDoc->m_MessageId, szFullMessage);
		
		try
		{
			CString							szHeaders;
			CString							szBody;
			
			//	Get the headers and body for display, concentrating if appropriate
			pDoc->m_QCMessage.GetMessageForDisplay( in_context, szBody, &szHeaders, &m_bWasConcentrated, false, (nNumMessageDocs > 1) );
			
			//	If we were supposed to concentrate, then whether or not we can concentrate
			//	in the future was determined by the simple fact of whether or not we did
			//	concentrate.
			//	If we were NOT supposed to concentrate, then assume for now that we can in the
			//	future.
			if ( ContentConcentrator::Instance()->ShouldConcentrate(in_context, pDoc->m_Sum) )
				m_bCanConcentrate = m_bWasConcentrated;
			else
				m_bCanConcentrate = true;
			
			//	Include taboo headers if:
			//	* We're doing blah blah blah
			//	* We were concentrated, which already decided what headers should be output
			//	* The setting IDS_INI_SHOW_ALL_HEADERS is on
			bool		bIncludeTabooHeaders = ( in_bIsBlahBlahBlah || m_bWasConcentrated || ShouldShowAllHeaders() );
						
			// Write out the headers that we're displaying
			WriteHeaders(theFile, pDoc, szHeaders, bIncludeTabooHeaders);

			theFile.Write( szDivStart, szDivStart.GetLength() );
			
			// Write out the body
			theFile.Write( szBody, szBody.GetLength() );

			theFile.Write( szDivEnd, szDivEnd.GetLength() );

			if ( i < (nNumMessageDocs-1) )
				theFile.Write( szMessageDivider, szMessageDivider.GetLength() );
		}
		catch( CException* pExp )
		{
			pExp->Delete();
			bWriteGood = FALSE;
		}
		
		if (bLoadedDoc)
			pSummary->NukeMessageDocIfUnused();
		
		delete [] szFullMessage;
	}

	//	Flush all that we've written
	try
	{
		theFile.Flush();
	}
	catch( CException* pExp )
	{
		pExp->Delete();
		bWriteGood = FALSE;
	}

	return bWriteGood;
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
	
	// create the file
	CFileException FE;
	if ( ! theFile.Open( m_szTmpFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary, &FE ))
	{
		::ErrorDialog( IDS_ERR_OPEN_TEMP_FILE, FE.m_cause, FE.m_lOsError, (LPCTSTR)FE.m_strFileName );
		
		// fool callers into not fixing up the source on errors
		m_bFixedupSource = TRUE;
		return FALSE;
	}
	if ( !WriteTempFile(theFile, GetContentConcentratorContext()) )
	{
		::ErrorDialog( IDS_ERR_WRITE_TEMP_FILE );
		
		// fool callers into not fixing up the source on errors
		m_bFixedupSource = TRUE;
		return FALSE;
	}

	try
	{
		DWORD dwLength = static_cast<DWORD>( theFile.GetLength() );
		if (dwLength)
			theFile.Write("</body></html>", 14);
		theFile.Close();	
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in CTridentView::LoadMessage" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		// Probably not a big deal if we can't write out the ending tags.
		// It may be a big deal if we can't close the file, but it still might work.
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in CTridentView::LoadMessage" );
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in CTridentView::LoadMessage" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		// Probably not a big deal if we can't write out the ending tags.
		// It may be a big deal if we can't close the file, but it still might work.
		ASSERT( !"Caught std::exception (not std::bad_alloc) in CTridentView::LoadMessage" );
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

BOOL CTridentView::DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
	HRESULT				hr;
	IHTMLBodyElement*	pBody;
	IHTMLTxtRange*		pRange;
	IHTMLTxtRange*		pNextRange;
	BOOL				bReturn;
	VARIANT_BOOL		vbFound;
	CBstr				cbstr;
	CBstr				cbstrSearch;
	CBstr				cbstrStartToEnd( IDS_START_TO_END );
	MSG					msg;
	CString				strTag;
	LONG				lFlags;

	USES_CONVERSION;

	if( bSelect )
	{
		while( ( !IsReady() || ( m_nIsVisible < 2 ) ) && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
			{
				// we have to wait until the trident control is
				// is ready to go before we can do anything with it.

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
	}
	else
	{
		while( !IsReady() && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			// we have to wait until the trident control is
			// is ready to go before we can do anything with it.

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	m_bstrBookmark = NULL;
	bReturn = FALSE;

	cbstrSearch = A2BSTR( szSearch );

	if( ( ( BSTR ) cbstrSearch ) == NULL )
	{
		return FALSE;
	}

	// Get document
	pBody = m_pSite->GetBody();
	
	if( !pBody ) 
	{
		return bReturn;
	}

	pNextRange = NULL;
	pRange = NULL;
	
	// Create range
	hr = pBody->createTextRange( &pNextRange );

	if( ( hr == S_OK ) && pNextRange )
	{
		hr = pNextRange->duplicate( &pRange );
	
		ASSERT( pRange );

		vbFound = VARIANT_FALSE;
		lFlags = ( ( bMatchCase ? SEARCHFLAG_MATCHCASE : 0 ) | ( bWholeWord ? SEARCHFLAG_MATCHWHOLEWORD : 0 ) );

		hr = pRange->findText( cbstrSearch, 0, lFlags, &vbFound );
		
#pragma warning(disable : 4310)			
		if( vbFound == VARIANT_TRUE ) 
#pragma warning(default : 4310)
		{
			// set the new range
			hr = pNextRange->setEndPoint( cbstrStartToEnd, pRange );

			bReturn = TRUE;
			
			if( bSelect ) 
			{
				pRange->select();
#pragma warning(disable : 4310)
				pRange->scrollIntoView( VARIANT_TRUE );
#pragma warning(default : 4310)
			}
								
			hr = pNextRange->getBookmark( &m_bstrBookmark );
		}
		
		if( pNextRange )
		{
			pNextRange->Release();
		}

		if( pRange )
		{
			pRange->Release();
		}
	}

	pBody->Release();
	return bReturn;
}



BOOL CTridentView::DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
	HRESULT				hr;
	IHTMLBodyElement*	pBody;
	IHTMLTxtRange*		pRange;
	IHTMLTxtRange*		pNextRange;
	BOOL				bReturn;
	VARIANT_BOOL		vbFound;
	CBstr				cbstr;
	CBstr				cbstrSearch;
	CBstr				cbstrStartToEnd( IDS_START_TO_END );
	CString				strTag;
	LONG				lFlags;

	USES_CONVERSION;

	if( m_bstrBookmark == NULL )
	{
		return DoFindFirst( szSearch, bMatchCase, bWholeWord, bSelect );
	}

	cbstrSearch = A2BSTR( szSearch );

	if( ( ( BSTR ) cbstrSearch ) == NULL )
	{
		return FALSE;
	}

	bReturn = FALSE;
		
	// Get body
	pBody = m_pSite->GetBody();
	
	if( !pBody ) 
	{
		return bReturn;
	}

	pRange = NULL;
	pNextRange = NULL;

	// Create range
	hr = pBody->createTextRange( &pNextRange );
	pBody->Release();

	if( ( S_OK == hr ) && pNextRange )
	{
		vbFound = VARIANT_FALSE;

		hr = pNextRange->moveToBookmark ( m_bstrBookmark , &vbFound ); 					
		m_bstrBookmark = NULL;
		
		if( !vbFound )
		{
			pNextRange->Release();
			return FALSE;
		}


		vbFound = VARIANT_FALSE;

		hr = pNextRange->duplicate( &pRange );
		
		if( ( hr == S_OK ) && pRange )
		{
			lFlags = ( ( bMatchCase ? SEARCHFLAG_MATCHCASE : 0 ) | ( bWholeWord ? SEARCHFLAG_MATCHWHOLEWORD : 0 ) );
		
			hr = pRange->findText( cbstrSearch, 0, lFlags, &vbFound );
		}

#pragma warning(disable : 4310)
		if( vbFound == VARIANT_TRUE ) 
#pragma warning(default : 4310)
		{
			// set the new range
			hr = pNextRange->setEndPoint( cbstrStartToEnd, pRange );

			bReturn = TRUE;

			if( bSelect )
			{
				pRange->select();
#pragma warning(disable : 4310)
				pRange->scrollIntoView( VARIANT_TRUE );
#pragma warning(default : 4310)
			}
								
			hr = pNextRange->getBookmark( &m_bstrBookmark );
		}

		if( pNextRange )
		{
			pNextRange->Release();
		}
		
		if( pRange )
		{
			pRange->Release();
		}
	}

	return bReturn;
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

							// We used to just check for the presence of a CITE attribute.
							// Unfortunately MSHTML seems to always tell us that there's a CITE
							// attribute even when there's not. This proves to be a problem
							// because Mac Eudora uses BLOCKQUOTE without CITE for indenting,
							// so if we misidentify the use of BLOCKQUOTE we make Mac Eudora
							// indenting look like excerpting - WOOPS!
							//
							// In looking at the use of BLOCKQUOTE most mail programs seem to
							// use all three ways of the following ways of tagging BLOCKQUOTES:
							// * type="cite"
							// * cite (sometimes with nothing after it, sometimes with ="",
							//   and sometimes with an actual non-empty value).
							// * class=cite (not present in every case - which is why we need
							//   this code in the first place - we're checking to see when
							//   we need to add this third one to match up with the style sheet).
							//
							// What we'll do now is only match cite when it has a string value.
							// Since all major mailers seem to use type="cite" also, hopefully this
							// won't be a problem.
							if( ( hr == S_OK ) && ( vType.vt == VT_BSTR ) && ( vType.bstrVal != NULL ) )
							{
								// there is a CITE attribute with a value so
								// add the correct class attribute
								bAddClass = TRUE;
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
							}

							VariantClear( &vType );

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


int CTridentView::GetTotalHeight()
{
    // Since we can't figure anything out, just return a very large
    // number.  Better that the window be too big than too small
    return 2000;
}


// QuietPrint -- prints without bringing up the print dialog.
BOOL CTridentView::QuietPrint()
{
	return Print(FALSE);
}

BOOL CTridentView::Print(BOOL bDialog, BOOL bDoPreview /*= FALSE*/)
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
	DWORD OldHeaderLen = 0;
	DWORD OldFooterLen = 0;
	DWORD HeaderValueType = 0;
	DWORD FooterValueType = 0;
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
	ULONG uCmd = bDoPreview? OLECMDID_PRINTPREVIEW : OLECMDID_PRINT;
	ExecCommandOnNullSet( uCmd, NULL, NULL, bDialog );

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

void CTridentView::SetFont(const char* szFontName, INT iSize)
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
	CRString						szBODY( IDS_HTML_BODY );

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
			
			// we're looking for the default style sheet rule for the BODY
			if( szText.CompareNoCase( szBODY ) ) 
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
			szText.Empty();
			hr = pRule->get_fontFamily( ( BSTRARG ) cbstr );
			szText = cbstr;

			if( szText.IsEmpty() == FALSE)
			{
				szText = szFontName;
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

BOOL CTridentView::HandleLeftMouseClick(CPoint pt)
{
	IHTMLElement* piFileAttachment = NULL;
	QueryElement_FileAttachment(pt, &piFileAttachment);

	if (piFileAttachment)
	{
		CString szHRef;
		const BOOL bGotAttach = GetAttachmentURL(piFileAttachment, szHRef);
		piFileAttachment->Release();

		if (bGotAttach)
		{
			LeftClickAttachment(this, pt, szHRef);
			return TRUE;
		}
	}

	if (LaunchURL(pt))
		return TRUE;

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
                m_bEnableAttachmentMenus = TRUE;
            }
            else
                m_bEnableAttachmentMenus = FALSE;

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

            if (HandleLeftMouseClick(pt))
                return TRUE;
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
	DoFileCommand(m_szTmpFile, 0, false);
}


IHTMLElement *
CTridentView::GetAnchorElement(
	CPoint		point)
{
    //	Get the width in pixels of a vertical scroll bar
    int		nScrollBarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	
	CRect	rectClient;
    GetClientRect(rectClient);

	//	Make sure that the mouse is not over the vertical scroll bar.
    if ( (rectClient.BottomRight().x - nScrollBarWidth) < point.x )
		return NULL;

	// Get document
	IHTMLDocument2 *		pDoc = m_pSite->GetDoc2();
	if (!pDoc)
		return NULL;

	//	Get the anchor element that the mouse is over (if any)
	IHTMLElement *			pElement = NULL;
	IHTMLElement *			pParentElement = NULL;
	HRESULT					hr = pDoc->elementFromPoint( point.x, point.y, &pElement );
	CBstr					cbstr;
	CString					szValue;
	CRString				szHTMLAnchor(IDS_HTML_ANCHOR);

	// Work our way up the chain of elements until we find something we like
	while ( (hr == S_OK) && (pElement != NULL) )
	{
		//	Get the tag
		hr = pElement->get_tagName(cbstr);

		if (hr == S_OK)
		{
			szValue = cbstr;
			
			//	If it's an anchor, then we're done
			if (szValue.CompareNoCase(szHTMLAnchor) == 0)
				break;
		}

		//	Get the parent
		pParentElement = NULL;
		hr = pElement->get_parentElement(&pParentElement);

		//	Release the previous element
		pElement->Release();
		
		//	Move to the parent if we found it
		pElement = ( (hr == S_OK) && (pParentElement != NULL) ) ?
				   pParentElement : NULL;
	}

	return pElement;
}


//	DoSpecialAttachment handles any special attachments (replaces DoOnDisplayPlugin).
//	DoSpecialAttachment currently handles both plugins and registration information.
BOOL CTridentView::DoSpecialAttachment(
IHTMLElement* pElement )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLElement*			pParent;
	IHTMLElement*			p;
	IHTMLAnchorElement*		pAnchor;
	CBstr					cbstr;
	CString					szValue;
	VARIANT					v;
	CBstr					cbstrEudora( IDS_EUDORA_TAG );
	IHTMLBodyElement*		pBody;
	IHTMLTxtRange*			pRange;
	CEudoraApp*				pApp;
	CString					szHRef( "" );
	bool					isOnDisplayPlugin = false;
	bool					isEudoraInfoAttachment = false;

	USES_CONVERSION;

	// Get document
	VERIFY( pDoc = m_pSite->GetDoc2() );
	
	if ( !pDoc ) 
	{
		return FALSE;
	}

	VERIFY( pBody = m_pSite->GetBody() );

	if( !pBody )
	{
		return FALSE;
	}

	hr = pElement->get_tagName( BSTRARG( cbstr ) );
	szValue = cbstr;

	pParent = NULL;
	p = pElement;

	if( szValue.CompareNoCase( CRString( IDS_HTML_IMG ) ) == 0 )
	{
		pParent = NULL;
		hr = pElement->get_parentElement( &pParent );

		if( ( hr == S_OK ) && ( pParent != NULL ) )
		{
			hr = pParent->get_tagName( BSTRARG( cbstr ) );
			p = pParent;
			szValue = cbstr;
		}
	}

	if( szValue.CompareNoCase( CRString( IDS_HTML_ANCHOR ) ) == 0 )
	{
		// See if it's a plugin or registration information
		hr = p->getAttribute( cbstrEudora, 0, &v );

		if( ( hr == S_OK ) && ( v.vt == VT_BSTR ) )
		{
			szValue = v.bstrVal;
			
			isOnDisplayPlugin = ( szValue.CompareNoCase( CRString( IDS_PLUGIN ) ) == 0 );
			if (!isOnDisplayPlugin)		//	It's not a plugin, is it special Eudora information?
				isEudoraInfoAttachment = ( szValue.CompareNoCase( CRString( IDS_EUDORA_INFO ) ) == 0 );
			if(isOnDisplayPlugin || isEudoraInfoAttachment)
			{
				// get the anchor
				
				pAnchor = NULL;
				hr = p->QueryInterface( IID_IHTMLAnchorElement, ( void** )( &pAnchor ) );
				
				if( ( hr == S_OK ) && ( pAnchor != NULL ) )
				{
					hr = pAnchor->get_href( BSTRARG( cbstr ) );
					
					if( ( hr == S_OK ) && ( ( ( BSTR ) cbstr ) != NULL ) )
					{
						szHRef = cbstr;
						hr = pBody->createTextRange( &pRange );
						hr = pRange->moveToElementText( p );
						hr = pRange->collapse( FALSE );
						hr = pRange->select();
					}

					pAnchor->Release();					
				}
			}
		}

		VariantClear( &v );
	}


	if( pParent )
	{
		pParent->Release();
	}

	pBody->Release();
	
	if( szHRef == "" )
	{
		return FALSE;
	}
	
	// don't release it if we're returning FALSE
	pElement->Release();

	// get rid of the "file:///"
	szHRef = szHRef.Right( szHRef.GetLength() - 8 ) ;

	// Convert %20 to ' ' and forward slashes to back slashes
	szHRef.Replace("%20", " ");
	szHRef.Replace('/', '\\');

	if (isOnDisplayPlugin)
	{
		if (GetNumMessageDocs() > 1)
		{
			//	We don't support this when multiple messages are selected - display error message
			ErrorDialog(0, IDS_TLR_ERROR_MULTIPLE_MESSAGES);
		}
		else
		{
			//	Handle plugin translation
			CSummary *		pSummary = NULL;
			CMessageDoc *	pMessageDoc = NULL;
			
			//	RTTI should be used sparingly, but it makes sense here. We only care about
			//	passing the CMessageDoc when the results of the translation might become
			//	permanent (which is only possible when it is a full message window).
			//	Furthermore we don't want to call GetMessageDoc because CTridentPreviewView's
			//	implementation of it might load the document.
			if ( IsKindOf(RUNTIME_CLASS(CTridentReadMessageView)) )
			{
				pMessageDoc = reinterpret_cast<CTridentReadMessageView *>(this)->GetDocument();
				pSummary = pMessageDoc->m_Sum;
			}
			else if ( IsKindOf(RUNTIME_CLASS(CTridentPreviewView)) )
			{
				pSummary = reinterpret_cast<CTridentPreviewView *>(this)->GetSummary(0);
			}
			
			pApp = (CEudoraApp*) AfxGetApp();
			pApp->GetTranslators()->XLateDisplay(this, pSummary, pMessageDoc, szHRef);
		}
	}
	else if (isEudoraInfoAttachment)
	{
		RegInfoReader::ProcessEudoraInfoAttachment(szHRef);
	}

	return TRUE;
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

      //get the width in pixels of a vertical scroll bar
    int nWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	CRect cRect;
    GetClientRect(cRect);
	//make sure that the user has not clicked on the vertical scroll bar.
    if ( cRect.BottomRight().x - nWidth < point.x )
	{
		return FALSE;
	}

	// Get document
	pDoc = m_pSite->GetDoc2();

  


	
	if ( !pDoc ) 
	{
		return FALSE;
	}

	// get the element we clicked on
	pElement = NULL;

	hr = pDoc->elementFromPoint( point.x, point.y, &pElement );
	
	// if it's a plugin or registration information, then handle that first
	if( DoSpecialAttachment( pElement ) )
	{
		return TRUE;
	}

	// Work our way up the chain of elements until we find something we like
	while ( ( hr == S_OK ) && ( pElement != NULL ) )
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
			break;
		}

		pParent = NULL;
		hr = pElement->get_parentElement( &pParent );

		if( ( hr == S_OK ) && ( pParent != NULL ) )
		{
			pElement->Release();
			pElement = pParent;
		}
		else
			break;
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

// OnFilePrintOne: doesn't brings up the print dialog before printing, prints to the default printer
void CTridentView::OnFilePrintOne() 
{
	Print(FALSE);
}

void CTridentView::OnFilePrintPreview()
{
	Print(FALSE, TRUE);
}

void CTridentView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(MSHTMLHasPrintPreview());
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

	// see if it's a plugin or registration information
	if( DoSpecialAttachment( pElement ) )
	{
		return TRUE;
	}

	// launch the url/attachment
	return LaunchURL( pElement );
}


BOOL CTridentView::LaunchURL(
IHTMLElement*	pElement, int command /*=0*/ )
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
		
		if ( (hr == S_OK) && !cbstr.IsNull() )
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
								IHTMLElement* pTargetLink;
								hr = pDispatch->QueryInterface( IID_IHTMLElement, ( void** )( &pTargetLink ) );
								
								if( ( hr == S_OK ) && ( pTargetLink != NULL ) )
								{
									VariantInit( &var );
									var.vt = VT_BOOL;
#pragma warning(disable : 4310)
									var.boolVal = VARIANT_TRUE;
#pragma warning(default : 4310)
									pTargetLink->scrollIntoView( var );
									bRet = TRUE;
									pTargetLink->Release();
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
	else if( szHRef != "" )   // it's an external link or attachment
	{
		// BOO: if this is a eudora attachment, then we'll try to get our own
		// sneaky local-file attribute---the real "href" can sometimes be messed
		// up when the file name contains special chars (e.g. '%'), due to in-
		// consistent url encoding/decoding on Trident's part.

		CBstr cbstrEudora( IDS_EUDORA_TAG );
		CBstr cbstrLocalPath( "file" );

		VariantInit( &var );
		hr = pElement->getAttribute( cbstrEudora, VARIANT_FALSE, &var );

		if( SUCCEEDED(hr) && var.vt == VT_BSTR ) {
			szValue = var.bstrVal;

			if( szValue.CompareNoCase( "attachment" ) == 0 ) {
				VariantInit( &var );
				hr = pElement->getAttribute( cbstrLocalPath, VARIANT_FALSE, &var );

				if( SUCCEEDED(hr) && var.vt == VT_BSTR )
					szHRef = var.bstrVal;
			}
		}

		// Get the link text
		CString			szLinkText;

		hr = pElement->get_innerText( ( BSTRARG) cbstr );
		
		if ( (hr == S_OK) && !cbstr.IsNull() )
			szLinkText = cbstr;

		bRet = ::LaunchURL(szHRef, szLinkText);
	}

	pElement->Release();
	return bRet;
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


// BOO:
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


BOOL CTridentView::GetAttachmentURL( IHTMLElement* piFileAttachment, CString &szHRef )
{
    USES_CONVERSION;

    IHTMLBodyElement* pBody = NULL;

    if ( (pBody = m_pSite->GetBody()) == NULL ) {
        assert( 0 );
        return FALSE;
    }
    
    szHRef.Empty();

    piFileAttachment->AddRef();

    // get the anchor, then the href (path to file attachment)
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

	// What Trident gives us has escaped characters. It also uses 3 slashes
	// for its file URL (file:///). Call UnEscapeURL to handle both issues. 
	szHRef = UnEscapeURL(szHRef);
   	
   	return !szHRef.IsEmpty();
}
    

BOOL CTridentView::DoCopyAttachment(IHTMLElement* piFileAttachment)
{
    USES_CONVERSION;

    BOOL bRet = FALSE;
    CString szHRef;

    if (GetAttachmentURL(piFileAttachment, szHRef))
        bRet = AttachmentsToClipboard(szHRef);

    return bRet;
}


void CTridentView::OnAttachmentAction(UINT nID)
{
    IHTMLElement* piFileAttachment = NULL;
    QueryElement_FileAttachment( m_ptFileAttachment, &piFileAttachment );

    if ( piFileAttachment ) {
    	CString szHRef;
    	if ( GetAttachmentURL( piFileAttachment, szHRef ))
        	::LaunchURL( szHRef, NULL, nID );
        piFileAttachment->Release();
    }
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
    IHTMLDocument2*          pDoc = NULL;
    IHTMLSelectionObject*    pSel = NULL;
    IHTMLTxtRange*           pRange = NULL;
    CBstr                    cbstrType;
    CBstr                    cbstrText;
    BOOL                     bReturn = FALSE;

    szText  = "";

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

BOOL CTridentView::GetSelectedTextAndTrim(
		CString &				out_szText,
		bool					in_bAllowMultipleLines,
		bool					in_bDoSuperTrim,
		int						in_nMaxLength,
		bool					in_bUseElipsisIfTruncating)
{
	BOOL	result = GetSelectedText(out_szText);

	if (result)
	{
		//	Trim leading and trailing space
		out_szText.TrimLeft();
		out_szText.TrimRight();
		
		if ( (!in_bAllowMultipleLines && (out_szText.Find('\r') != -1)) )
		{
			out_szText.Empty();
			result = FALSE;
		}
		else
		{
			//	Truncate the string if necessary before replacing characters
			//	so that we can be sure that super trimming can't take too long.
			if ( in_nMaxLength && (out_szText.GetLength() > in_nMaxLength) )
			{
				out_szText.GetBufferSetLength(in_nMaxLength);

				//	Trim trailing space again in case we truncated immediately after some
				out_szText.TrimRight();

				if (in_bUseElipsisIfTruncating)
					out_szText += CRString(IDS_ELIPSIS);
			}

			if (in_bDoSuperTrim)
			{
				//	Replace any tabs with spaces
				out_szText.Replace('\t', ' ');

				//	Replace any CRs with spaces
				out_szText.Replace('\r', ' ');

				//	Replace any LFs with spaces
				out_szText.Replace('\n', ' ');

				//	Drop down any space runs to a single space
				while ( out_szText.Replace("  ", " ") );
			}
		}
	}

	return result;
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

						// Convert emoticon image tags back to the original triggers
						ConvertEmoticonImageTagsToTriggers(szHTML);

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

			// Convert emoticon image tags back to the original triggers
			ConvertEmoticonImageTagsToTriggers(szHTML);

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


BOOL CTridentView::RefreshMessage()
{
	return LoadMessage();
}


void CTridentView::ConvertEmoticonImageTagsToTriggers(CString & io_szHTML)
{
	if ( !io_szHTML.IsEmpty() )
	{
		CString			szEmoticonTrigger;
		int				nStartTagIndex = 0;
		const char *	pEntireBodyBuffer = io_szHTML.GetBuffer( io_szHTML.GetLength() );
		const char *	pStartTag;
		
		//	Scan the HTML looking for emoticon image tags
		while (1)
		{
			bool		bInsideQuotes = false;

			//	Start scanning at the next character (we do the math everytime,
			//	because the buffer will be reallocated if we replace any image tags).
			pStartTag = pEntireBodyBuffer + nStartTagIndex;

			//	Find the start of a tag
			while ( *pStartTag && (bInsideQuotes || (*pStartTag != '<')) )
			{
				if (*pStartTag == '"')
					bInsideQuotes = !bInsideQuotes;

				pStartTag++;
			}

			//	Did we find the start of a tag? If not, then we're done looking.
			if (*pStartTag != '<')
				break;
			
			//	Remember the offset of the found tag
			nStartTagIndex = pStartTag - pEntireBodyBuffer;
			
			//	Is it an image tag?
			if (strnicmp(pStartTag, "<img", 4) == 0)
			{
				//	Find the end of the image tag
				char *		pEndTag = const_cast<char *>(pStartTag);
				
				bInsideQuotes = false;
				
				while ( *pEndTag && (bInsideQuotes || (*pEndTag != '>')) )
				{
					if (*pEndTag == '"')
						bInsideQuotes = !bInsideQuotes;
					
					pEndTag++;
				}

				ASSERT(*pEndTag == '>');
				
				//	Did we find the end of the image tag?
				if (*pEndTag == '>')
				{
					//	Empty the previous emoticon trigger
					szEmoticonTrigger.Empty();
					
					//	Temporarily modify the buffer so that we won't search
					//	any farther than necessary.
					*pEndTag = '\0';
					
					//	Look for the special eudora tag (note that this is
					//	currently case sensitive).
					const char *	pTagAttribute = strstr(pStartTag, "eudora=");
					
					if (pTagAttribute)
					{
						//	Move past the special eudora tag
						pTagAttribute += strlen("eudora=");
						
						//	Move past the quotation mark, if any
						if (*pTagAttribute == '"')
							pTagAttribute++;
						
						//	Is it an emoticon?
						if (strnicmp(pTagAttribute, "emoticon", strlen("emoticon")) == 0)
						{
							//	Look for the alt text. We really should handle mixed case,
							//	but for now let's just be lazy and check for lower and
							//	upper case.
							pTagAttribute = strstr(pStartTag, "alt=");

							if (!pTagAttribute)
								pTagAttribute = strstr(pStartTag, "ALT=");
							
							//	Did we find the alt text?
							if (pTagAttribute)
							{
								//	Move past the alt.
								pTagAttribute += 4;
								
								//	Move past the quotation mark, if any
								if (*pTagAttribute == '"')
									pTagAttribute++;
								
								//	Look for the end of the alt text.
								char *	pEndTagAttribute = const_cast<char *>(pTagAttribute);
								
								while ( *pEndTagAttribute && !isspace((unsigned char)*pEndTagAttribute) )
									pEndTagAttribute++;
								
								//	If the alt text ended with a quotation mark, back up
								//	one character.
								if (pEndTagAttribute[-1] == '"')
									pEndTagAttribute--;
								
								//	Temporarily modify the buffer so that we can work
								//	with the alt text without copying it.
								char	cSaveEndTagAttribute = *pEndTagAttribute;
								*pEndTagAttribute = '\0';
								
								//	Use Text2Html to handle character encoding of
								//	the trigger characters.
								szEmoticonTrigger = Text2Html(pTagAttribute, FALSE, FALSE);

								//	Text2Html ends with a CRLF - trim that off.
								szEmoticonTrigger.TrimRight();
								
								//	Restore the character to the buffer
								*pEndTagAttribute = cSaveEndTagAttribute;
							}
						}
					}
					
					//	Restore the end of the tag
					*pEndTag = '>';
					
					//	Did we find emoticon information?
					if ( !szEmoticonTrigger.IsEmpty() )
					{
						//	Delete the entire image tag, including the end '>',
						//	and replace it with the encoded trigger text.
						io_szHTML.Delete(nStartTagIndex, pEndTag - pStartTag + 1);
						io_szHTML.Insert(nStartTagIndex, szEmoticonTrigger);
						
						//	We'll start searching after the emoticon trigger (we use
						//	the -1 adjustment, because we increment nStartTagIndex below).
						nStartTagIndex += szEmoticonTrigger.GetLength() - 1;

						//	Get the entire body buffer again, because our changes
						//	above invalidated our previous pointer to the buffer.
						pEntireBodyBuffer = io_szHTML.GetBuffer( io_szHTML.GetLength() );
					}
				}
			}

			//	Move on to the next character
			nStartTagIndex++;
		}
	}
}
