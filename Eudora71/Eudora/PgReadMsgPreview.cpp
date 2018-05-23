// PgReadMsgPreview.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PgReadMsgPreview.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "pghtmdef.h"
#include "ReadMessageDoc.h"

#include "summary.h"
#include "tocview.h"
#include "tocframe.h"
#include "msgdoc.h"
#include "msgutils.h"
#include "guiutils.h"
#include "text2html.h"
#include "rs.h"
#include "QCFindMgr.h"
#include "SearchEngine.h" // StringSearch
#include "compmsgd.h"
#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview

IMPLEMENT_DYNCREATE(PgReadMsgPreview, PgMsgView)

PgReadMsgPreview::PgReadMsgPreview()
	:	m_pAssociatedFiles(NULL)
{
}

PgReadMsgPreview::~PgReadMsgPreview()
{
	//	Delete any associated files - used to cleanup up any on display translation
	//	for security.
	DeleteAssociatedFiles();
}


void PgReadMsgPreview::DeleteAssociatedFiles()
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


extern UINT umsgLoadNewPreview;

BEGIN_MESSAGE_MAP(PgReadMsgPreview, PgMsgView)
	//{{AFX_MSG_MAP(PgReadMsgPreview)
    ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP

	ON_REGISTERED_MESSAGE(umsgLoadNewPreview, LoadNewPreview)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview diagnostics

#ifdef _DEBUG
void PgReadMsgPreview::AssertValid() const
{
	PgMsgView::AssertValid();
}

void PgReadMsgPreview::Dump(CDumpContext& dc) const
{
	PgMsgView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview message handlers
void PgReadMsgPreview::OnInitialUpdate() 
{
	SetReadOnly();

	PgMsgView::OnInitialUpdate();
	
	LoadNewPreview(0, 0);
}


void PgReadMsgPreview::SetMoreMessageStuff(
	PgStuffBucket *						pStuffBucket)
{
	pStuffBucket->subkind = PgStuffBucket::kPreviewMessage;

	//	We set pMessage to NULL because:
	//	* The QCMessage doesn't hang around for previews
	//	* We need to be able support display of multiple concentrated messages
	//	  and thus we can't rely on a *single* QCMessage
	pStuffBucket->pMessage = NULL;
}


CTocDoc * PgReadMsgPreview::GetTocDoc()
{
	CTocDoc *		pTocDoc = NULL;	
	CTocFrame *		pParentFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );

	ASSERT_KINDOF(CTocFrame, pParentFrame);

	if ( pParentFrame && pParentFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)) )
	{
		CTocView *		pTocView = pParentFrame->GetTocView();

		ASSERT(pTocView);
		if (pTocView)
			pTocDoc = pTocView->GetDocument();
	}

	return pTocDoc;
}


BOOL PgReadMsgPreview::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
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
	
	return PgMsgView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


//	Handle OnChar even though CPaigeEdtView does similar stuff because
//	we need to send the message to the correct location, which in our
//	case is our parent frame the CTocFrame.
void PgReadMsgPreview::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if ( (nChar == VK_SPACE) && ShiftDown() )
    {
        // A Shift+Space on a read-only view translates into a
        // "toggle read status" command.

		//	Send the message to the CTocFrame, which knows how to handle it properly
		//	(as opposed to other non-preview parent classes that would rely on their
		//	associated doc to handle the message).
		CFrameWnd *		pFrame = GetParentFrame();
		ASSERT(pFrame);
		if (pFrame)
			pFrame->SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE, LPARAM(GetSafeHwnd()));

		return;
    }

    PgMsgView::OnChar(nChar, nRepCnt, nFlags);
}


//	Handle OnKeyDown even though CPaigeEdtView does similar stuff because
//	we need to send the message to the correct location, which in our
//	case is our parent frame the CTocFrame.
void PgReadMsgPreview::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN)	
	{
		// Check if the CONTROL key is down.
		bool	bControlDown = (CtrlDown() != 0);

		// Get the appropriate settings
		bool	bPlainArrows = (GetIniShort(IDS_INI_PLAIN_ARROWS) != 0);
		bool	bControlArrows = (GetIniShort(IDS_INI_CONTROL_ARROWS) != 0);
		
		if ( (bPlainArrows && !bControlDown) || (bControlArrows && bControlDown) )
		{
			//	Send the message to the CTocFrame, which knows how to handle it properly
			//	(as opposed to other non-preview parent classes that would rely on their
			//	associated doc to handle the message).
			CFrameWnd *		pFrame = GetParentFrame();
			ASSERT(pFrame);
			if (pFrame)
				pFrame->SendMessage(WM_COMMAND, (nChar == VK_LEFT || nChar == VK_UP) ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
			return;
		}
	}

	PgMsgView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void PgReadMsgPreview::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Turn message-switching hotkeys into ID_PREVIOUS_MESSAGE/ID_NEXT_MESSAGE
	if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN)
	{
		bool	bCtrlDown = (CtrlDown() != 0);
		bool	bAltArrows = (GetIniShort(IDS_INI_ALT_ARROWS) != 0);
		
		if (bAltArrows && !bCtrlDown)
		{
			//	Send the message to the CTocFrame, which knows how to handle it properly
			//	(as opposed to other non-preview parent classes that would rely on their
			//	associated doc to handle the message).
			CFrameWnd *		pFrame = GetParentFrame();
			ASSERT(pFrame);
			if (pFrame)
				pFrame->SendMessage(WM_COMMAND, (nChar == VK_LEFT || nChar == VK_UP) ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
			return;
		}
	}
	
	PgMsgView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


////////////////////
// This should retrieve the full message body or the header depending on the Include Headers line.
// In the preview we're going to have to look up the whole message, and read it in and then


BOOL PgReadMsgPreview::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
	//Should collapse this func and GetMessageAsHtml, but maybe too expensive.
	//Lets leave it for now.
	pg_ref impRef;

	szText = "";

	if (IncludeHeaders)
	{
		CSummary *		pSummary = NULL;
		CTocFrame *		pTocFrame = (CTocFrame *) GetParentFrame();		// the Ugly Typecast(tm)
		if (pTocFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)))
			pSummary = pTocFrame->GetSingleSelectionPreviewSummary();

		if (pSummary)
		{
			bool			bCreatedDoc = false;
			CMessageDoc *	pMsgDoc = pSummary->GetMessageDoc(&bCreatedDoc);
			char *			blah = pMsgDoc->GetFullMessage(TRUE);
			char *			pEndHeaders = strstr(blah, "\r\n\r\n");

			if (pEndHeaders)
				*pEndHeaders = 0;

			szText = blah;
			delete [] blah;

			if (bCreatedDoc)
				pSummary->NukeMessageDocIfUnused(pMsgDoc);
		}
	}

	CString tempMsg;
	char *TempBuffer;

	select_pair sel;
	sel.begin = 0;
	sel.end = pgTextSize(m_paigeRef);

	impRef = pgCopy(m_paigeRef, &sel);

	// We're going to get the whole message out of the pane and then 
	// rip the headers off
	GetTextAs( impRef, tempMsg, PgDataTranslator::kFmtText );
	TempBuffer = DEBUG_NEW_NOTHROW char [tempMsg.GetLength()+1];
	if (!TempBuffer)
		return false;

	strcpy(TempBuffer, tempMsg);

	char *floater = strstr(TempBuffer, "\r\n\r\n");
	if (floater)
	{
		szText += floater;
	}
	else
	{
		szText += TempBuffer;
	}

	delete [] TempBuffer;

	UnuseAndDispose(impRef);

	return (TRUE);
}


BOOL PgReadMsgPreview::RefreshMessage()
{
	//	Turn off redrawing - LoadNewPreview will turn it back on when it
	//	calls FinalizeDisplay.
	SetRedraw(FALSE);

	//	Delete the entire previous version that we loaded
	select_pair		delete_range;
	delete_range.begin = 0;
	delete_range.end = pgTextSize(m_paigeRef);
	pgDelete(m_paigeRef, &delete_range, draw_none);

	return LoadNewPreview();
}


BOOL PgReadMsgPreview::SetAssociatedFiles(CStringList * in_pAssociatedFiles)
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


void PgReadMsgPreview::OnSetFocus( CWnd* pOldWnd ) 
{
	PgMsgView::OnSetFocus( pOldWnd );
	ShowCaret();
}

void PgReadMsgPreview::OnKillFocus( CWnd* pNewWnd ) 
{
	PgMsgView::OnKillFocus( pNewWnd );
	HideCaret();
}


void PgReadMsgPreview::FinalizeDisplay(bool bNeedToDraw)
{
	//	We're done - turn redrawing back on
	SetRedraw(TRUE);

	//	Set the cursor to beginning of message body, and set it to read only
	pgSetSelection( m_paigeRef, 0, 0, 0, TRUE );
	if (bNeedToDraw)
	{
		Invalidate();
		UpdateWindow();
		UpdateScrollBars(true);
	}
	
	HideCaret();
}


LRESULT PgReadMsgPreview::LoadNewPreview()
{
	CTocFrame *		pTocFrame = (CTocFrame *) GetParentFrame();		// the Ugly Typecast(tm)

	//	Delete any associated files - used to cleanup up any on display translation
	//	for security.
	DeleteAssociatedFiles();

	ASSERT(pTocFrame);
	ASSERT_KINDOF(CTocFrame, pTocFrame);
	if ( !pTocFrame || !pTocFrame->IsKindOf(RUNTIME_CLASS(CTocFrame)) )
	{
		FinalizeDisplay(true);
		return TRUE;
	}
	
	const CTocFrame::SummaryArrayT &	summaryArray = pTocFrame->GetPreviewSummaryArray();
	int									nNumSelectedMessages = summaryArray.GetSize();
	if (nNumSelectedMessages == 0)
	{
		FinalizeDisplay(true);
		return TRUE;
	}

	bool								bMultipleMessages = (nNumSelectedMessages > 1);
	ContentConcentrator::ContextT		context = bMultipleMessages ?
												  ContentConcentrator::kCCMultipleContext :
												  ContentConcentrator::kCCPreviewContext;
	bool			bLoadedDoc = false;
	CSummary *		pSummary = NULL;
	CMessageDoc *	pDoc = NULL;
	CString			szPreviewHeaders;
	CString			szBody;
	bool			bWasConcentrated;
	int				nHeaderBytes;
	CString			szDivStart = "<div>\r\n";
	CString			szDivEnd = "\r\n</div>";
	CString			szMessageDivider = "<br><hr>\r\n";
	select_pair		sel;
	bool			bNoScrollingNeeded = true;
	bool			bNeedToDraw = true;

	//	We used to import the body first, then set the selection back to the beginning
	//	and import the headers there. An important characteristic of that approach
	//	was that importing of the body was done directly into our m_paigeRef (because
	//	no "shell" Paige reference is used during importing if the destination is
	//	completely empty - see PaigeImportFilter::pgImportFile).
	//
	//	Now that we're importing multiple messages at times, it no longer makes sense
	//	to import the body and then import the headers at the beginning. It would be
	//	less convenient than the more obvious order of importing the headers then the
	//	body. Moreover it wouldn't allow us to avoid the use of the "shell" importing
	//	Paige reference, because as soon as more than one message is involved we would
	//	no longer have a completely empty destination.
	//
	//	Other engineers who worked on Eudora in the past noted and attempted to fix
	//	some of the inconsistencies associated with the "shell" reference during
	//	importing (and yet the one attempt to remove the "shell" reference code was
	//	backed out - leading me to believe that it is still important for code
	//	robustness and stability).
	//
	//	In rewriting the code to use the new import order, I hit additional
	//	inconsistencies, which I fixed (described here in case future bugs are found
	//	of a similar nature):
	//	* In one case the font used didn't match up correctly when the user had changed
	//	  it recently enough, because the "shell" reference used the values from
	//	  Eudora's Paige global which wasn't kept up-to-date (to fix it I made Eudora's
	//	  Paige global grab the latest settings every time an object is created).
	//	* In another case hyperlinks were not being imported with the proper style
	//	  (i.e. blue and underlined) when the imported body was transferred from the
	//	  "shell" reference to our m_paigeRef. The problem there was twofold - the
	//	  hyperlink styles weren't being setup in our m_paigeRef and even if I made
	//	  that happen the IDs of the styles didn't necessarily match up between the
	//	  "shell" reference and m_paigeRef. The solution was to initialize the "shell"
	//	  reference with the style numbers from the destination Paige reference (in our
	//	  case m_paigeRef).
	//	In both cases the problem came down to ways in which the "shell" reference
	//	did not quite match the destination Paige reference.
	for (int i = 0; i < nNumSelectedMessages; i++)
	{
		pSummary = summaryArray[i];
		pDoc = pSummary->GetMessageDoc(&bLoadedDoc);
		
		ASSERT(pDoc);
		if (pDoc)
		{
			//	Pass in true for the last parameter so that CIDs *are* converted to local
			//	file URLs, because we have no need for them to remain CIDs and we won't
			//	have the message around later to perform the conversion later.
			GetMessageForDisplay( pDoc, context, bMultipleMessages, false,
								  szBody, &szPreviewHeaders, &bWasConcentrated, true );

			//	We only need to set up the stuff bucket message stuff (message kind
			//	and subkind) once. We've overridden SetMoreMessageStuff so that
			//	we don't keep track of the message (which wouldn't work in a multiple
			//	message scenario), so there's no need to be concerned about the fact
			//	that we're passing it the current pDoc.
			if (i == 0)
				SetMessageStuff(pDoc);
			
			//	We're all done with the current pDoc, so get rid of it
			//	if no one else is using it.
			if (bLoadedDoc)
				pSummary->NukeMessageDocIfUnused(pDoc);
			
			//	If we're not concentrating then use the old code
			//	which strips the preview headers.
			if (!bWasConcentrated)
			{
				szPreviewHeaders = StripNonPreviewHeaders(szPreviewHeaders);
				
				// Stripped preview headers don't have end CRLF
				szPreviewHeaders += "\r\n";
			}

			//	Apply the body style so that the headers are imported with the correct
			//	style (I'm not completely clear why this works, but it does).
			sel.begin = sel.end = pgTextSize(m_paigeRef);
			pgApplyNamedStyle(m_paigeRef, &sel, body_style, draw_none);
			
			//	Create a Paige memory ref containing the headers
			nHeaderBytes = szPreviewHeaders.GetLength();
			memory_ref			impRef = Bytes2Memory( PgMemGlobalsPtr(),
													   reinterpret_cast<pg_char_ptr>( const_cast<LPTSTR>(static_cast<LPCTSTR>(szPreviewHeaders)) ),
													   sizeof(pg_char), nHeaderBytes );
			//	Import the headers
			PgDataTranslator	theTxr( m_paigeRef );
			theTxr.ImportData( &impRef, PgDataTranslator::kFmtText, draw_none);

			//	Do the auto URL stuff for the headers that we just imported
			//	(not done as they're imported since we import them as plain text)
			MakeAutoURLSpaghetti( sel.begin, pgTextSize(m_paigeRef) );

			//	We're done with impRef
			DisposeMemory(impRef);

			if (nNumSelectedMessages > 1)
			{
				//	Wrap the body with div tags to make sure that there's a consistent
				//	amount of space before the message divider.
				szBody.Insert(0, szDivStart);
				szBody += szDivEnd;

				//	Insert the message divider between messages
				if ( i < (nNumSelectedMessages-1) )
					szBody += szMessageDivider;
			}

			//	Import the body of the message
			ImportMessage(szBody);

			//	Turn on redrawing
			SetRedraw(TRUE);

			//	Update our display if we didn't already have a full page of text last time
			//	(i.e. once scrolling is needed - we have more text than we can display
			//	and so further updating just slows things down and causes flickering).
			if (bNoScrollingNeeded)
			{
				//	We're done - go ahead and redraw
				Invalidate();
				UpdateWindow();
				bNeedToDraw = false;
			}

			//	Update our scrollbars no matter what - they give an indication of progress as we import
			UpdateScrollBars(true, false, &bNoScrollingNeeded);

			//	Turn off redrawing
			SetRedraw(FALSE);
		}
	}

	FinalizeDisplay(bNeedToDraw);

	return TRUE;
}


LRESULT PgReadMsgPreview::LoadNewPreview(WPARAM, LPARAM)
{
	//	Turn off redrawing - other version of LoadNewPreview will turn it back on
	//	when it calls FinalizeDisplay.
	SetRedraw(FALSE);
	NewPaigeObject();

	return LoadNewPreview();
}
