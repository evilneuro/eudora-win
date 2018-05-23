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
#include "tocview.h"
#include "msgutils.h"
#include "guiutils.h"
#include "rs.h"
#include "summary.h"
#include "mshtml.h"
#include "mshtmcid.h"
#include "text2html.h"
#include "site.h"
#include "bstr.h"
#include "trnslate.h"
#include "SearchEngine.h" // StringSearch

#include "DebugNewHelpers.h"

IMPLEMENT_DYNCREATE(CTridentPreviewView, CTridentView)


//	Constants
const UINT		CTridentPreviewView::kEnableWindowWithDelay	= 10;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTridentPreviewView::CTridentPreviewView()
	: m_nEnableWindowWithDelayTimerID(0), m_pAssociatedFiles(NULL)
{
}

CTridentPreviewView::~CTridentPreviewView()
{
	//	Delete any associated files - used to cleanup up any on display translation
	//	for security.
	DeleteAssociatedFiles();
}


void CTridentPreviewView::DeleteAssociatedFiles()
{
	//	Delete any associated files - used to cleanup up any on display translation
	//	for security.	
	if (m_pAssociatedFiles)
	{
		while ( !m_pAssociatedFiles->IsEmpty() )
		{
			CString		szPath = m_pAssociatedFiles->RemoveHead();

			if ( !szPath.IsEmpty() )
				DeleteFile(szPath);
		}

		delete m_pAssociatedFiles;
		m_pAssociatedFiles = NULL;
	}
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

	bool			bLoadedDoc = false;
	CSummary *		pSum = NULL;

	//	Just get the first CMessageDoc - assumes that we'll only
	//	be called for single selection previews.
	CMessageDoc *	pMsgDoc = GetMessageDoc(0, &bLoadedDoc, &pSum);

	if (!pMsgDoc)
		return FALSE;

	if (!pSum)
	{
		if (bLoadedDoc)
			CSummary::NukeMessageDocIfUnused(NULL, pMsgDoc);

		return FALSE;
	}

	char* FMBuffer = pMsgDoc->GetFullMessage(RAW);
	CString szFullMessage(FMBuffer);
	delete [] FMBuffer;

//	delete pSum;

	//HTML to Text translation is not really symmetric in Trident
	//So if the document has not been modified then return the text
	//from the message doc ONLY if it was originally a plain text message
	if ( !pMsgDoc->IsModified() && !pSum->IsXRich() && !pSum->IsHTML() )
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

		if (bLoadedDoc)
			pSum->NukeMessageDocIfUnused();

		return TRUE;
	}

	if (bLoadedDoc)
		pSum->NukeMessageDocIfUnused();

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


BOOL CTridentPreviewView::SetAssociatedFiles(CStringList * in_pAssociatedFiles)
{
	if (!in_pAssociatedFiles)
		return FALSE;
	
	if (m_pAssociatedFiles)
	{
		m_pAssociatedFiles->AddTail(in_pAssociatedFiles);
		delete in_pAssociatedFiles;
	}
	else
	{
		m_pAssociatedFiles = in_pAssociatedFiles;
	}

	return TRUE;
}


CTocDoc *
CTridentPreviewView::GetTocDoc()
{
	return reinterpret_cast<CTocDoc *>( GetDocument() );
}


int
CTridentPreviewView::GetNumMessageDocs() const
{
	int				nNumMessageDocs = 0;
	CTocFrame *		pParentFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );

	ASSERT_KINDOF(CTocFrame, pParentFrame);

	if ( pParentFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)) )
	{
		const CTocFrame::SummaryArrayT &	summaryArray = pParentFrame->GetPreviewSummaryArray();
		nNumMessageDocs = summaryArray.GetSize();
	}

	return nNumMessageDocs;
}


CSummary *
CTridentPreviewView::GetSummary(int in_nMessageIndex)
{
	CSummary *		pSummary = NULL;
	CTocFrame *		pParentFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );	// the Ugly Typecast(tm)
	ASSERT_KINDOF(CTocFrame, pParentFrame);

	if ( pParentFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)) )
	{
		// Can't use GetDocument() here since that gives you the CTocDoc.
		// We really need to grab the CMessageDoc from the summary that the
		// frame used to make this preview pane.

		const CTocFrame::SummaryArrayT &	summaryArray = pParentFrame->GetPreviewSummaryArray();

		ASSERT( in_nMessageIndex < summaryArray.GetSize() );
		if ( in_nMessageIndex < summaryArray.GetSize() )
			pSummary = summaryArray[in_nMessageIndex];
	}
	
	return pSummary;
}


CMessageDoc *
CTridentPreviewView::GetMessageDoc(int in_nMessageIndex, bool * out_bLoadedDoc, CSummary ** out_ppSummary)
{
	CMessageDoc *	pDoc = NULL;
	CSummary *		pSummary = GetSummary(in_nMessageIndex);
	
	if (pSummary)
	{
		// Find or load the message doc
		pDoc = pSummary->GetMessageDoc(out_bLoadedDoc);
	}

	if (out_ppSummary)
		*out_ppSummary = pSummary;

	return pDoc;
}


bool
CTridentPreviewView::IsNoMessageToDisplayAllowed() const
{
	// No message to display is ok
	return true;
}


BOOL
CTridentPreviewView::UseFixedFont() const
{
	return (GetIniShort(IDS_INI_USE_PROPORTIONAL_AS_DEFAULT) == 0);
}


bool
CTridentPreviewView::ShouldShowAllHeaders() const
{
	return false;
}


ContentConcentrator::ContextT
CTridentPreviewView::GetContentConcentratorContext(bool in_bIsBlahBlahBlah) const
{
	return (GetNumMessageDocs() > 1) ? ContentConcentrator::kCCMultipleContext : ContentConcentrator::kCCPreviewContext;
}


void
CTridentPreviewView::WriteHeaders(
	CFile &				theFile,
	CMessageDoc *		pDoc,
	const CString &		szHeaders,
	BOOL				bShowTabooHeaders)
{
	COLORREF		cr;
	char			szBgColor[8];
	char			szFgColor[8];

	cr = ::GetSysColor(COLOR_BTNFACE);
	sprintf( szBgColor, "#%02X%02X%02X", GetRValue(cr), GetGValue(cr), GetBValue(cr) );

	cr = ::GetSysColor(COLOR_BTNTEXT);
	sprintf( szFgColor, "#%02X%02X%02X", GetRValue(cr), GetGValue(cr), GetBValue(cr) );

	CString		szPreviewHeaders;
	
	if (bShowTabooHeaders)
	{
		szPreviewHeaders = szHeaders;

		//	szHeaders come with trailing CRLF, which we don't want
		szPreviewHeaders.TrimRight();
	}
	else
	{
		//	StripNonPreviewHeaders takes care of trailing CRLF
		szPreviewHeaders = StripNonPreviewHeaders(szHeaders);
	}

	if (!szPreviewHeaders.IsEmpty())
	{
		CString		szLine;
		
		szLine.Format(GetIniString(IDS_INI_PVTABLE_START), (LPCSTR)szBgColor, (LPCSTR)szFgColor);
		theFile.Write(szLine, szLine.GetLength());
		szLine.Format(GetIniString(IDS_INI_PVTABLE_ROW_START), (LPCSTR)szBgColor, (LPCSTR)szFgColor);
		theFile.Write(szLine, szLine.GetLength());

		szLine = Text2Html(szPreviewHeaders, TRUE, FALSE);
		theFile.Write(szLine, szLine.GetLength());

		const char* szTableRowEnd = GetIniString(IDS_INI_PVTABLE_ROW_END);
		theFile.Write(szTableRowEnd, strlen(szTableRowEnd));

		const char* szTableEnd = GetIniString(IDS_INI_PVTABLE_END);
		theFile.Write(szTableEnd, strlen(szTableEnd));

		theFile.Write("<BR>\r\n", 6);
	}
}


extern UINT umsgLoadNewPreview;

BEGIN_MESSAGE_MAP(CTridentPreviewView, CTridentView)
	//{{AFX_MSG_MAP(CTridentPreviewView)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(umsgLoadNewPreview, LoadNewPreview)
	ON_WM_TIMER()
END_MESSAGE_MAP()


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

	//	Handle arrow keys and shift-space even though CTridentView does similar
	//	stuff because we need to send the message to the correct location, which
	//	in our case is our parent frame the CTocFrame.
    if (WM_KEYDOWN == pMsg->message)
    {
        //	Look for plain Arrow keystrokes and Ctrl+Arrow keystrokes. If setting is enabled,
		//	translate those keystrokes into "next/previous message" commands.  Also, look
        //	for Shift+Space keystrokes and translate those into "change status to unread" commands.
        switch (pMsg->wParam)
        {
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
			{
				bool	bCtrlDown = (CtrlDown() != 0);
				bool	bUsePlainArrows = (GetIniShort(IDS_INI_PLAIN_ARROWS) != 0);
				bool	bUseCtrlArrows = (GetIniShort(IDS_INI_CONTROL_ARROWS) != 0);

				if ( (bUsePlainArrows && !bCtrlDown) || (bUseCtrlArrows && bCtrlDown) )
				{
					//	Send the message to the CTocFrame, which knows how to handle it properly
					//	(as opposed to other non-preview parent classes that would rely on their
					//	associated doc to handle the message).
					CFrameWnd *		pFrame = GetParentFrame();
					ASSERT(pFrame);
					if (pFrame)
					{
						pFrame->SendMessage( WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP) ?
														 ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE );
					}

					return TRUE;    // don't let Trident control see this message
				}
			}
			break;

			case VK_SPACE:
				if ( ShiftDown() )
				{
					//	A Shift+Space on a read-only view translates into a "toggle read status" command.
					//
					//	Send the message to the CTocFrame, which knows how to handle it properly
					//	(as opposed to other non-preview parent classes that would rely on their
					//	associated doc to handle the message).
					CFrameWnd *		pFrame = GetParentFrame();
					ASSERT(pFrame);
					if (pFrame)
						pFrame->SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE);
					return TRUE;            // don't let the Trident control see this message
				}
				break;
        }
    }

    if (WM_SYSKEYDOWN == pMsg->message)
    {
        //	Look for Alt+Arrow keystrokes and if setting is enabled, translate those
		//	keystrokes into "next/previous message" commands.
        switch (pMsg->wParam)
        {
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
			{
				bool	bCtrlDown = (CtrlDown() != 0);
				bool	bUseAltArrows = (GetIniShort(IDS_INI_ALT_ARROWS) != 0);

				if (bUseAltArrows && !bCtrlDown)
				{
					CFrameWnd *		pFrame = GetParentFrame();
					ASSERT(pFrame);
					if (pFrame)
					{
						pFrame->SendMessage( WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP) ?
														 ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE );
					}

					return TRUE;    // don't let Trident control see this message
				}
			}
			break;
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


BOOL CTridentPreviewView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	//	If the focus is in us, there's no reason that File->Save As... shouldn't
	//	be supported. Find the CTocView, which has code to handle this and route
	//	the command handling to it.
	if (nID == ID_FILE_SAVE_AS)
	{
		CTocFrame *		pParentFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );

		ASSERT_KINDOF(CTocFrame, pParentFrame);

		if ( pParentFrame && pParentFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)) )
		{
			CTocView *		pTocView = pParentFrame->GetTocView();

			ASSERT(pTocView);
			if (pTocView)
				return pTocView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}
	}
	
	return CTridentView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


///////////////////////////////////////////////////////////////////////////////
// Timer stuff for delayed enabling


// EnableWindowWithDelay - to prevent Trident from rudely grabbing focus, we
// disable ourselves in OnInitialUpdate; this routine re-enables us with a
// delay to compensate for Trident's sloppy completion reporting.

void CTridentPreviewView::EnableWindowWithDelay()
{
	m_nEnableWindowWithDelayTimerID = SetTimer(kEnableWindowWithDelay, GetIniInt(IDS_INI_MSHTML_FOCUS_TIMER), NULL);
	if (m_nEnableWindowWithDelayTimerID == 0)
	{
		//	Ouch - we were unable to set the timer. Go ahead and reenable right now.
		EnableWindow();
	}
}


void CTridentPreviewView::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == kEnableWindowWithDelay)
	{
		//	m_nEnableWindowWithDelayTimerID could be 0 if we killed the timer while
		//	the WM_TIMER message was already on the message queue.  If so, there's
		//  no need to kill the timer again.
		if (m_nEnableWindowWithDelayTimerID)
		{
			//	Kill the timer because it's done its work
			KillTimer(m_nEnableWindowWithDelayTimerID);
			m_nEnableWindowWithDelayTimerID = 0;
		}
		
		EnableWindow();
	}
}


LRESULT CTridentPreviewView::LoadNewPreview(WPARAM, LPARAM)
{
	//	Kill any previous timer so that it doesn't fire before we're ready to be enabled
	if (m_nEnableWindowWithDelayTimerID)
	{
		KillTimer(m_nEnableWindowWithDelayTimerID);
		m_nEnableWindowWithDelayTimerID = 0;
	}

	//	Disable ourselves until someone calls EnableWindowWithDelay and our timer
	//	later fires.
	EnableWindow( FALSE );

	//	Delete any associated files - used to cleanup up any on display translation
	//	for security.
	DeleteAssociatedFiles();

	return LoadMessage();
}
