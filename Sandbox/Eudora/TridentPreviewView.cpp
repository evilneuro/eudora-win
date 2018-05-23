// TridentPreviewView.cpp: implementation of the CTridentPreviewView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"

#ifdef IDM_PROPERTIES
#undef IDM_PROPERTIES
#endif

#include "TridentReadMessageView.h"
#include "TridentPreviewView.h"
#include "ReadMessageDoc.h"
#include "TocFrame.h"
#include "tocdoc.h"
#include "msgutils.h"
#include "rs.h"
#include "summary.h"
#include "mshtml.h"
#include "mshtmcid.h"
#include "text2html.h"
#include "site.h"
#include "bstr.h"
#include "trnslate.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CTridentPreviewView, CTridentView)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTridentPreviewView::CTridentPreviewView()
{
}

CTridentPreviewView::~CTridentPreviewView()
{
}


void CTridentPreviewView::OnInitialUpdate() 
{
    CTridentView::OnInitialUpdate();
    EnableWindow( FALSE );
    ExecCommand( IDM_BROWSEMODE );
}

BOOL CTridentPreviewView::SetSelectedText(
const char* szText,
bool bSign )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLSelectionObject*	pSel;
	IHTMLTxtRange*			pRange;
	IHTMLTxtRange*			pBodyRange;
	CBstr					cbstrType;
	CBstr					cbstrText;
	CBstr					cbstr;
	BOOL					bReturn;
	BOOL					bDone;
	IHTMLElementCollection*	pCollection;
	IHTMLElement*			pElement;
	IDispatch*				pDispatch;
	VARIANT					varIndex;
	VARIANT					var;
	VARIANT_BOOL			vb;
	LONG					lCount;
	LONG					lIndex;
	CRString				szBody( IDS_HTML_BODY );
	CString					szTemp;

	USES_CONVERSION;

	pRange = NULL;
	pBodyRange = NULL;
	pSel = NULL;
	pDoc = NULL;
	bReturn = FALSE;
	bDone = FALSE;

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if (!pDoc) 
	{
		return FALSE;
	}

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( !SUCCEEDED( hr ) || !pSel ) 
	{
		return FALSE;
	}

	// Create range
	hr = pSel->createRange((IDispatch**)&pRange);

	if( !SUCCEEDED( hr ) || !pRange ) 
	{
		pSel->Release();
		return FALSE;
	}

	hr = pRange->duplicate( &pBodyRange );

	// see if it takes up the entire message body
	// note: this is a hack because CTRL-A run plugin caused
	// new text to put inside the taboo header span
	hr = pDoc->get_all( &pCollection );

	if( ( hr == S_OK ) && pCollection )
	{
		hr = pCollection->get_length( &lCount );

		if ( hr == S_OK )
		{
			for ( lIndex = 0; !bDone && ( lIndex < lCount ); lIndex++ )
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
						szTemp = cbstr;
						
						if( szTemp.CompareNoCase( szBody ) == 0 )
						{								
							// bail out -- even if bDone == FALSE
							lCount = 0;

							pBodyRange->moveToElementText( pElement );
							
							vb = VARIANT_FALSE;
							hr = pRange->isEqual( pBodyRange, &vb );
							
#pragma warning(disable : 4310)							
							if( SUCCEEDED( hr ) && ( vb == VARIANT_TRUE ) )
#pragma warning(default : 4310)
							{
								szTemp = Text2Html( szText, TRUE, FALSE );
								
								cbstr = A2BSTR( szTemp );
								
								if( ( ( BSTR ) cbstr ) != NULL )
								{
									hr = pElement->put_innerHTML( cbstr );
								
									if ( S_OK == hr )
									{							
										bDone = TRUE;
										GetDocument()->SetModifiedFlag();
									}
								}
							}
						}
					
						pElement->Release();	
					}
				}
			}
		}

		pCollection->Release();
	}
	
	pBodyRange->Release();

	if( bDone )
	{
		pRange->Release();
		pSel->Release();
		return TRUE;
	}

	// Get type
	hr = pSel->get_type( BSTRARG( cbstrType ) );

	if ( ( S_OK == hr )  && ( ( ( BSTR ) cbstrType ) != NULL ) )
	{
		CString strType = cbstrType;

		// If type isnt text, bail
		if( ( strType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 ) || ( strType.CompareNoCase( CRString( IDS_NONE ) ) == 0 ) )
		{
			cbstrText = A2BSTR( szText );
			
			if( ( ( BSTR ) cbstrText ) != NULL )
			{
				hr = pRange->put_text( cbstrText );
			
				if ( S_OK == hr )
				{							
					bReturn = TRUE;
					GetDocument()->SetModifiedFlag();
				}
			}
		}

		pRange->Release();
	}

	return bReturn;
}

BOOL CTridentPreviewView::GetMessageAsText(
CString&	szMsg,
BOOL		bIncludeHeaders)
{
	HRESULT				hr;
	IHTMLBodyElement*	pBody;
	IHTMLTxtRange*		pRange;
	BOOL				bReturn;
	CBstr				cbstrText;

	bReturn = FALSE;
	szMsg.Empty();

	CSummary *pSum = ((CTocDoc *)m_pDocument)->GetPreviewableSummary();
	if (!pSum)
		return FALSE;

	CMessageDoc* pMsgDoc = pSum->GetMessageDoc();
	if (!pMsgDoc)
		return FALSE;

	char* FMBuffer = pMsgDoc->GetFullMessage(RAW);
	CString szFullMessage(FMBuffer);
	delete [] FMBuffer;
	pSum->NukeMessageDocIfUnused();
//	delete pSum;

	//HTML to Text translation is not really symmetric in Trident
	//So if the document has not been modified then return the text
	//from the message doc ONLY if it was originally a plain text message
	if (!(pMsgDoc->IsModified()) && 
		!(pMsgDoc->m_Sum->IsXRich()) && !pMsgDoc->m_Sum->IsHTML())
	{
		if (bIncludeHeaders)
			szMsg = szFullMessage;
		else
		{
			const char* pHeadersBGone;
			pHeadersBGone = strstr(szFullMessage, "\r\n\r\n");
			if (pHeadersBGone)
				szMsg = pHeadersBGone + 4;
			else
				szMsg = szFullMessage;
		}

		return TRUE;
	}

	// Well what we'll do is read the headers off the disk and read the body from the pane.
	CString szMsgInPane;

	pBody = m_pSite->GetBody();
	if (!pBody) 
		return FALSE;

	pRange = NULL;

	// Create range
	hr = pBody->createTextRange( &pRange );

	if( ( S_OK == hr ) && pRange )
	{
		hr = pRange->get_text( BSTRARG( cbstrText ) );

		if ( S_OK == hr && ( ( ( BSTR ) cbstrText ) != NULL ) )
		{							
			szMsgInPane = cbstrText;
			bReturn = TRUE;
		}

		pRange->Release();
	}

	pBody->Release();

// BOG
//	CTridentReadMessageView::StripNBSP( szMsgInPane.GetBuffer( 0 ) );
	StripNBSP( szMsgInPane.GetBuffer( 0 ) );	
	szMsgInPane.ReleaseBuffer();

	char *EndOfHeader = strstr(szFullMessage, "\r\n\r\n");
	if (EndOfHeader)
		*EndOfHeader = 0;

	if (bIncludeHeaders)	// Here's the difference.
		szMsg = szFullMessage;

	if (EndOfHeader)
		*EndOfHeader = 65;	// I don't know what it was, but it better not be zero or CString's gonna have a fit.

	EndOfHeader = strstr(szMsgInPane, "\r\n\r\n");

	szMsg += EndOfHeader;

	return bReturn;
}


BOOL CTridentPreviewView::WriteTempFile( 
CFile& theFile,
CString& szStyleSheetFormat )
{
	CString		szLine;
	BOOL		bReturn = FALSE;
	
	CTocFrame* pParentFrame = (CTocFrame* )GetParentFrame();	// the Ugly Typecast(tm)
	ASSERT_KINDOF(CTocFrame, pParentFrame);
	
	// Can't use GetDocument() here since that gives you the CTocDoc.
	// We really need to grab the CMessageDoc from the summary that the
	// frame used to make this preview pane.
	//
	CSummary* pSummary = pParentFrame->GetPreviewSummary();
	if( pSummary == NULL )
	{
		// This isn't actually an error.  No summary means no message
		// to display.
		return TRUE;
	}

	BOOL bLoadedDoc = FALSE;
	CMessageDoc* pDoc = NULL;
	
	if (NULL == (pDoc = pSummary->FindMessageDoc()))
	{
		//
		// Document not loaded into memory, so force it to load.
		//
		pDoc = pSummary->GetMessageDoc();
		bLoadedDoc = TRUE;
	}

	if (NULL == pDoc)
	{
		ASSERT(0);
		return FALSE;
	}

	ASSERT_KINDOF(CMessageDoc, pDoc);
	
	// get the whole message
	char* szFullMessage = pDoc->GetFullMessage( RAW );


#ifdef IMAP4
	//
	// It's possible for this to fail if the message couldn't be retrieved
	//	from the IMAP server
	//
	if (NULL == szFullMessage) 
	{
		if (bLoadedDoc)
		{
			pSummary->NukeMessageDocIfUnused();
			pDoc = NULL;
		}

		return TRUE;
	}
#endif // IMAP4


	CString szFontName;
	if (GetIniShort(IDS_INI_USE_PROPORTIONAL_AS_DEFAULT))
	{
		szFontName = GetIniString(IDS_INI_MESSAGE_FONT);
	}
	else
	{
		szFontName = GetIniString(IDS_INI_MESSAGE_FIXED_FONT);
	}

	CString szStyleSheet;
	szStyleSheet.Format(szStyleSheetFormat, 
			 (LPCSTR)szFontName,
			 (LPCSTR)GetIniString(IDS_INI_MESSAGE_FIXED_FONT),
			 (LPCSTR)GetIniString(IDS_INI_EXCERPT_BARS),
			 "none");
	
	// convert cid's to local file URLs
	MorphMHTML(&szFullMessage);
	pDoc->m_QCMessage.Init(pDoc->m_MessageId, szFullMessage);

	bReturn = TRUE;
		
	try
	{
		theFile.Write(szStyleSheet, szStyleSheet.GetLength());

		COLORREF cr;
		char szBgColor[8];
		char szFgColor[8];
		cr = ::GetSysColor(COLOR_BTNFACE);
		sprintf(szBgColor, "#%02X%02X%02X", GetRValue(cr), GetGValue(cr), GetBValue(cr));
		cr = ::GetSysColor(COLOR_BTNTEXT);
		sprintf(szFgColor, "#%02X%02X%02X", GetRValue(cr), GetGValue(cr), GetBValue(cr));

		CString PreviewHeaders(StripNonPreviewHeaders(szFullMessage));
		if (PreviewHeaders.IsEmpty() == FALSE)
		{
			szLine.Format(GetIniString(IDS_INI_PVTABLE_START), (LPCSTR)szBgColor, (LPCSTR)szFgColor);
			theFile.Write(szLine, szLine.GetLength());
			szLine.Format(GetIniString(IDS_INI_PVTABLE_ROW_START), (LPCSTR)szBgColor, (LPCSTR)szFgColor);
			theFile.Write(szLine, szLine.GetLength());

			szLine = Text2Html(PreviewHeaders, TRUE, FALSE);
			theFile.Write(szLine, szLine.GetLength());

			const char* szTableRowEnd = GetIniString(IDS_INI_PVTABLE_ROW_END);
			theFile.Write(szTableRowEnd, strlen(szTableRowEnd));

			const char* szTableEnd = GetIniString(IDS_INI_PVTABLE_END);
			theFile.Write(szTableEnd, strlen(szTableEnd));

			theFile.Write("<BR>\r\n", 6);
		}

		// get the body as html and save it to theFile
		CString theBody;
		pDoc->m_QCMessage.GetBodyAsHTML(theBody);
		theFile.Write(theBody, theBody.GetLength());

		theFile.Flush();
	}
	catch( CException* pExp )
	{
		pExp->Delete();
		bReturn = FALSE;
	}

	if (bLoadedDoc)
	{
		pSummary->NukeMessageDocIfUnused();
		pDoc = NULL;
	}
	
	delete [] szFullMessage;

	return bReturn;
}

extern UINT umsgLoadNewPreview;

BEGIN_MESSAGE_MAP(CTridentPreviewView, CTridentView)
	//{{AFX_MSG_MAP(CTridentPreviewView)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(umsgLoadNewPreview, LoadNewPreview)
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to "tweak" the standard command routing to forward commands from the
// view to the "fake" CMessageDoc document that was used to create this
// view in the first place.
////////////////////////////////////////////////////////////////////////
BOOL CTridentPreviewView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    switch (nID)
    {
    case ID_NEXT_MESSAGE:
    case ID_PREVIOUS_MESSAGE:
	//
	// Fall through and let the MessageDoc, if any, handle these.
	// Trust me.
	//
	break;
    default:
	//
	// Let this view and the TocDoc have first crack at most commands.
	//
	if (CTridentView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	    return TRUE;
	break;
    }

    //
    // If we get this far, neither this view nor the TocDoc handled the
    // command, so let the preview's MessageDoc have a crack at it.
    //
    CSummary*	pSummary = NULL;

    CTocFrame* pTocFrame = (CTocFrame *) GetParentFrame();		// the Ugly Typecast(tm)
    if (pTocFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)))
	pSummary = pTocFrame->GetPreviewSummary();
    else
    {
	ASSERT(0);
    }

    if( pSummary == NULL )
	return FALSE;

    //
    // This is ugly since GetMessageDoc() leaves the message document
    // loaded into memory, typically without a view.  We rely
    // on CTocFrame to clean up this document for us.
    //
    CMessageDoc* pDoc = pSummary->GetMessageDoc();
    return pDoc->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



BOOL CTridentPreviewView::DoOnDisplayPlugin(
IHTMLElement* pElement )
{	HRESULT					hr;
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
	CString					szFixed( "" );
	INT						i;
	CString					szHRef( "" );

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
		// see if it's a plugin
		hr = p->getAttribute( cbstrEudora, 0, &v );

		if( ( hr == S_OK ) && ( v.vt == VT_BSTR ) )
		{
			szValue = v.bstrVal;
			
			if( szValue.CompareNoCase( CRString( IDS_PLUGIN ) ) == 0 )
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

	szFixed.GetBuffer( szHRef.GetLength() + 1 );

	while( ( i = szHRef.Find( "%20" ) ) >= 0 )
	{
		szFixed += szHRef.Left( i );
		szFixed += ' ';
		szHRef = szHRef.Right( szHRef.GetLength() - i - 3 );
	}

	szFixed += szHRef;

	pApp = (CEudoraApp*) AfxGetApp();
	pApp->GetTranslators()->XLateDisplay( this, szFixed );

	return TRUE;
}


BOOL CTridentPreviewView::PreTranslateMessage(MSG* pMsg) 
{
    //
    // Treat Tab keys as a special case for optionally navigating
    // through the anchors in the HTML document.
    //
    if ((WM_KEYDOWN == pMsg->message) && (VK_TAB == pMsg->wParam))
    {
	if (! GetIniShort(IDS_INI_SWITCH_PREVIEW_WITH_TAB))
	{
	    if (m_pIOleIPActiveObject)
	    {
		HRESULT hr = m_pIOleIPActiveObject->TranslateAccelerator(pMsg);
		if ( NOERROR == hr )
		{
		    // The object translated the accelerator, so we're done
		    return TRUE;
		}
	    }
	}
    }

    return CTridentView::PreTranslateMessage(pMsg);
}


void CTridentPreviewView::OnActivateView
(
    BOOL bActivate,         // being activated or deactivated
    CView* pActivateView,   // view being activated
    CView* pDeactiveView    // view being deactivated (can be the same)
)
{
    if ( bActivate && !IsWindowEnabled() ) {
	EnableWindow( TRUE );
    }

    CTridentView::OnActivateView( bActivate, pActivateView, pDeactiveView );
}

///////////////////////////////////////////////////////////////////////////////
// Timer stuff for delayed enabling


// EnableWindowWithDelay - to prevent Trident from rudely grabbing focus, we
// disable ourselves in OnInitialUpdate; this routine re-enables us with a
// specified delay to compensate for Trident's sloppy completion reporting.

void CTridentPreviewView::EnableWindowWithDelay( unsigned ucDelay )
{
    HWND theWnd = GetSafeHwnd();
    ::SetTimer( theWnd,
		(UINT)theWnd,
		ucDelay,
		CTridentPreviewView::WMTimerProc );
}


// WMTimerProc - callback for ordinary WM_TIMER-based notifications
void CALLBACK CTridentPreviewView::WMTimerProc
(
    HWND hwnd,     // handle of window for timer messages
    UINT uMsg,     // WM_TIMER message
    UINT idEvent,  // timer identifier
    DWORD dwTime   // current system time
)
{
    ::KillTimer( hwnd, idEvent );

    if ( ::IsWindow( hwnd ) ) {
	::EnableWindow( hwnd, TRUE );
    }
}


LRESULT CTridentPreviewView::LoadNewPreview(WPARAM, LPARAM)
{
	EnableWindow( FALSE );
	return LoadMessage();
}
