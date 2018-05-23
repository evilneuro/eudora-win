// BidentPreviewView.cpp: implementation of the CBidentPreviewView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"

#ifdef IDM_PROPERTIES
#undef IDM_PROPERTIES
#endif

#include "BidentPreviewView.h"
#include "ReadMessageDoc.h"
#include "TocFrame.h"
#include "tocdoc.h"
#include "msgutils.h"
#include "rs.h"
#include "summary.h"
#include "mshtmcid.h"
#include "text2html.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBidentPreviewView, CBidentView)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBidentPreviewView::CBidentPreviewView()
{

}

CBidentPreviewView::~CBidentPreviewView()
{

}


void CBidentPreviewView::OnInitialUpdate()
{
	CBidentView::OnInitialUpdate();
	EnableWindow( FALSE );
}


BOOL CBidentPreviewView::WriteTempFile( 
CFile& theFile,
BOOL )
{
	CString		szLine;
	BOOL		bReturn = FALSE;

	CTocFrame*	pParentFrame = (CTocFrame*)GetParentFrame();	// the Ugly Typecast(tm)
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
	
	// convert cid's to local file URLs
	MorphMHTML(&szFullMessage);
	pDoc->m_QCMessage.Init(pDoc->m_MessageId, szFullMessage);

	bReturn = TRUE;
	
	try
	{
		WriteDefaults(theFile, szFontName);

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

BEGIN_MESSAGE_MAP(CBidentPreviewView, CBidentView)
	//{{AFX_MSG_MAP(CBidentPreviewView)
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
BOOL CBidentPreviewView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
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
		if (CBidentView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
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


BOOL CBidentPreviewView::PreTranslateMessage(MSG* pMsg) 
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

	return CBidentView::PreTranslateMessage(pMsg);
}

LRESULT CBidentPreviewView::LoadNewPreview(WPARAM, LPARAM)
{
	EnableWindow( FALSE );
	return LoadMessage();
}
