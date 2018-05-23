// BidentReadMessageView.cpp : implementation of the CBidentReadMessageView class
//

#include "stdafx.h"
#include "eudora.h"

#ifdef IDM_PROPERTIES
#undef IDM_PROPERTIES
#endif

#include <AFXPRIV.H>
#include <afxdisp.h>

#include "ReadMessageFrame.h"
#include "ReadMessageDoc.h"
#include "BidentReadMessageView.h"
#include "site.h"
#include "mshtmcid.h"
#include "mainfrm.h"
#include "Text2Html.h"
#include "Html2Text.h"
// delete 
#include "newmbox.h"

#include "fileutil.h"
#include "rs.h"
#include "msgutils.h"
#include "summary.h"
#include "guiutils.h"


#include "QCOleDropSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CBidentReadMessageView

IMPLEMENT_DYNCREATE(CBidentReadMessageView, CBidentView)

BEGIN_MESSAGE_MAP(CBidentReadMessageView, CBidentView)
	//{{AFX_MSG_MAP(CBidentReadMessageView)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	//}}AFX_MSG_MAP

	ON_COMMAND( ID_EDIT_SELECT_ALL, OnSelectAll )
	ON_UPDATE_COMMAND_UI( ID_EDIT_SELECT_ALL, OnUpdateSelectAll )
	ON_UPDATE_COMMAND_UI( ID_EDIT_MESSAGE, OnUpdateAlwaysDisabled )

	ON_BN_CLICKED( ID_BLAHBLAHBLAH, OnRefreshFile )
	ON_BN_CLICKED( ID_USE_FIXED_FONT, OnRefreshFile )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBidentReadMessageView construction/destruction

CBidentReadMessageView::CBidentReadMessageView() 
{
	m_pParentFrame = NULL;
}


CBidentReadMessageView::~CBidentReadMessageView()
{
	CMessageDoc *pDoc = (CMessageDoc*) GetDocument();
	if (pDoc)
	{
		ASSERT_KINDOF(CMessageDoc, pDoc);
		if (pDoc->m_Sum)
			pDoc->m_Sum->SetShowAllHeaders( FALSE );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CBidentReadMessageView diagnostics

#ifdef _DEBUG
void CBidentReadMessageView::AssertValid() const
{
	CView::AssertValid();
}

void CBidentReadMessageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CReadMessageDoc* CBidentReadMessageView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReadMessageDoc)));
	return (CReadMessageDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CBidentReadMessageView message handlers



void CBidentReadMessageView::OnSelectAll() 
{
	ExecCommand( IDM_SELECTALL );
}


void CBidentReadMessageView::OnUpdateSelectAll(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	dwStatus = GetCommandStatus( IDM_SELECTALL );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CBidentReadMessageView::OnInitialUpdate() 
{
	m_pParentFrame = ( CReadMessageFrame* ) GetParentFrame();

	ASSERT_KINDOF( CReadMessageFrame, m_pParentFrame );

	CBidentView::OnInitialUpdate();	
		
    //We created the thing, now activate it with "Show"
    if( GetStyle() & WS_VISIBLE )
	{
		m_pSite->Activate( OLEIVERB_SHOW );
	}
	else
	{
		m_pSite->Activate( OLEIVERB_HIDE );
	}
	
	// editing is not available 

	ExecCommand( IDM_BROWSEMODE );

	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) )
	{
		// bring the button back up
		extern UINT umsgButtonSetCheck;		// see ReadMessageFrame.cpp
		m_pParentFrame->SendMessage( umsgButtonSetCheck, ID_EDIT_MESSAGE, FALSE );
	}
}



void CBidentReadMessageView::OnRefreshFile()
{
//	TCHAR*	pchPath;
	CFile	theFile;
	MSG		msg;

	m_lReadyState = READYSTATE_UNINITIALIZED;
	
	if ( ! theFile.Open( m_szTmpFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary )  )
	{
		::ErrorDialog( IDS_ERR_OPEN_TEMP_FILE );
		m_lReadyState = READYSTATE_COMPLETE;
		return;
	}

	CMessageDoc* pDoc = GetDocument();
	ASSERT_KINDOF(CMessageDoc, pDoc);
	BOOL isBlahBlahBlah = m_pParentFrame->GetCheck( ID_BLAHBLAHBLAH );
	pDoc->m_Sum->SetShowAllHeaders(isBlahBlahBlah);

	if( ! WriteTempFile( theFile, isBlahBlahBlah ) )
	{
		::ErrorDialog( IDS_ERR_WRITE_TEMP_FILE );
		m_lReadyState = READYSTATE_COMPLETE;
		return;
	}
		
	theFile.Close();	

	::SendMessage( m_hWndObj, WM_COMMAND, 28, 0 );// got the value from the dll

	while( !IsReady() && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
	{
		// we have to wait until the bident control is
		// is ready to go before we can do anything with it.

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


void CBidentReadMessageView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE );
}


void CBidentReadMessageView::SaveInfo() 
{
	CReadMessageDoc*		pMessageDoc;
	char*					pMessage;

	pMessageDoc = ( CReadMessageDoc* ) GetDocument();
	ASSERT_KINDOF( CReadMessageDoc, pMessageDoc );
	
	pMessage = pMessageDoc->GetFullMessage();
	pMessageDoc->SetText( pMessage );
	delete [] pMessage;
	pMessageDoc->m_Sum->SetFlag(MSF_XRICH);
	pMessageDoc->m_Sum->SetFlagEx( MSFEX_HTML );
}




BOOL CBidentReadMessageView::GetSelectedText(
CString& szText )
{
	return FALSE;
}


BOOL CBidentReadMessageView::GetAllText(
CString& szText )
{
	return FALSE;
}


BOOL CBidentReadMessageView::GetSelectedHTML(
CString& szHTML )
{
	return FALSE;
}


BOOL CBidentReadMessageView::GetAllHTML(
CString& szHTML )
{
	CFile	theFile;
	char*	sz;

	sz = NULL;

	try 
	{
		theFile.Open( m_szTmpFile, CFile::modeRead );
		sz = szHTML.GetBufferSetLength( theFile.GetLength() + 1 );
		theFile.Read( sz, theFile.GetLength() );
		sz[ theFile.GetLength() ] = '\0';
	}
	catch( CException*	pExp )
	{
		pExp->Delete();
		if( sz )
		{
			szHTML.ReleaseBuffer();
		}
		return FALSE;
	}
	
	return TRUE;
}



BOOL CBidentReadMessageView::SetAllText(
const char* szText )
{
	return FALSE;
}

BOOL CBidentReadMessageView::SetAllHTML(
const char* szHTML )
{
	return FALSE;
}


BOOL CBidentReadMessageView::WriteTempFile( 
CFile&	theFile,
BOOL	bShowTabooHeaders )
{
	CReadMessageDoc*	pDoc;
	char*				szHeader;
	char*				szColon;
	char*				szFullMessage;
	const char*			szBody;
	char*				szEnd;	
	BOOL				bIncludeHeader;
	INT					i;
	const CStringArray*	pTabooHeaderArray;
	CString				szText;
	CString				szStyleSheet;
	CString				szLine;
	char				cSave;
	CString				szFontName;
	BOOL				bReturn;

	if( m_pParentFrame->UseFixedWidthFont() )
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FIXED_FONT );
	}
	else
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FONT );
	}

	WriteDefaults( theFile, szFontName );

	pDoc = ( CReadMessageDoc* ) GetDocument();
	
	if( pDoc == NULL )
	{
		return FALSE;
	}
	
	// get the whole message
	szFullMessage = pDoc->GetFullMessage( RAW );

#ifdef IMAP4 // 
	//
	// It's possible for this to fail if the message couldn't be retrieved
	//  from the IMAP server
	//
	if (NULL == szFullMessage) 
	{
		return FALSE;
	}
#endif // IMAP4

	pDoc->ReallySetTitle( pDoc->m_Sum->MakeTitle() );
		
	theFile.Write( "<HTML>", 6 );

	// convert cid's to local file URLs
	MorphMHTML( &szFullMessage );

	pDoc->m_QCMessage.Init( pDoc->m_MessageId, szFullMessage );

	// find the headers.  Wrap <SPAN> tags around each.
	// set each class to EUDORAHEADER or TABOOHEADER depending on whether or not
	// it should be shown in normal mode.

	szHeader = szFullMessage;
	szBody = FindBody( szFullMessage );
	pTabooHeaderArray = & ( pDoc->GetTabooHeaderArray() );

	bReturn = TRUE;

	try
	{
		while( szHeader < szBody )
		{
			szColon = szHeader;
			
			while(	( *szColon != ':' ) && 
					( szColon < szBody ) && 
					( *szColon != '\0' ) &&
					( *szColon != '\n' ) )
			{
				szColon ++;
			}
			
			if( ( szColon == szBody ) || ( *szColon != ':' ) )
			{
				// no more headers
				break;
			}
		
			bIncludeHeader = TRUE;
						
			if( !bShowTabooHeaders )
			{
				for( i = 0; i < pTabooHeaderArray->GetSize(); i ++ )
				{
					if( !strnicmp( (*pTabooHeaderArray)[i], szHeader, ( ( *pTabooHeaderArray )[i] ).GetLength() ) )
					{
						bIncludeHeader = FALSE;
        				break;
					}
				}
			}
			

			// write the whole header (it may be on several lines) to file, 
			// replace the \r\n with <BR>'s
			
			do
			{
				szEnd = szHeader;

				while(	( *szEnd != '\0' ) && 
						( *szEnd != '\r' ) &&
						( *szEnd != '\n' ) )
				{
					szEnd ++;
				}
				
				cSave = *szEnd;

				*szEnd = '\0';

				szLine = Text2Html( szHeader, TRUE, FALSE );

				if( bIncludeHeader )
				{
					// write the line
					theFile.Write( szLine, szLine.GetLength() );
					theFile.Write( "<BR>\r\n", 6 );			
				}

				*szEnd = cSave;

				if( *szEnd == 0 )
				{
					break;
				}

				++szEnd;			
				
				if( ( *szEnd == '\r' ) || ( *szEnd == '\n' ) )
				{
					++szEnd;
				}
				
				szHeader = szEnd;
			}
			while( ( *szHeader == ' ' ) || ( *szHeader == '\t' ) );	// if the line starts with 
																	// a space or tab, it's 
																	// still part of this 
																	// header
		}

		// get the body as html and save it to theFile
		CString theBody;
		pDoc->m_QCMessage.GetBodyAsHTML( theBody );
		theFile.Write( "<BR>\r\n", 6 );
		theFile.Write( theBody, theBody.GetLength() );

		theFile.Write( "</HTML>", 7 );
		
		theFile.Flush();
	}
	catch( CException*	pExp )
	{
		pExp->Delete();
		bReturn = FALSE;
	}

	delete [] szFullMessage;

	return bReturn;
}


void CBidentReadMessageView::OnUpdateAlwaysDisabled(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( FALSE );
}


BOOL CBidentReadMessageView::GetMessageAsHTML(
CString&	szMsg,
BOOL		bIncludeHeaders)
{
	CReadMessageDoc*	pDoc;
	char*				szFullMessage;
	CString				szFontName;

	if( bIncludeHeaders )
	{
		return GetAllHTML( szMsg );
	}

	pDoc = ( CReadMessageDoc* ) GetDocument();
	
	if( pDoc == NULL )
	{
		return FALSE;
	}
	
	// get the whole message
	szFullMessage = pDoc->GetFullMessage( RAW );

	// convert cid's to local file URLs
	MorphMHTML( &szFullMessage );

	pDoc->m_QCMessage.Init( pDoc->m_MessageId, szFullMessage );

	// get the body as html 
	pDoc->m_QCMessage.GetBodyAsHTML( szMsg );
	szMsg += "<BR>\r\n";
	
	delete [] szFullMessage;

	return TRUE;
}


BOOL CBidentReadMessageView::GetMessageAsText(
CString&	szMsg,
BOOL		bIncludeHeaders)
{
	CString				szHTML;
	
	if( ! GetMessageAsHTML( szHTML, bIncludeHeaders ) )
	{
		return FALSE;
	}
	
	szMsg = Html2Text( ( char* ) ( const char* ) szHTML );

	return TRUE;
}

