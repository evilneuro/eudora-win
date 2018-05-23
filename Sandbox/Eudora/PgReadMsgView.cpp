// PgReadMsgView.cpp : implementation file
//

#include "stdafx.h"
#include "QCUtils.h"	// must be early else bad stuff happens
#include "resource.h"
#include "rs.h"
#include "PgReadMsgView.h"
#include "ReadMessageDoc.h"
#include "ReadMessageFrame.h"
#include "msgutils.h"
#include "utils.h"
#include "Paige_io.h"
#include "pgosutl.h"
#include "SaveAs.h"
#include "summary.h"
#include "pghtmdef.h"
#include "PgStyleUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// PgReadMsgView

IMPLEMENT_DYNCREATE(PgReadMsgView, PgMsgView)

PgReadMsgView::PgReadMsgView()
	: m_HeaderBytes(0)
{
}

PgReadMsgView::~PgReadMsgView()
{
//	pgRemoveStyle(m_paigeRef, m_TabooHeadersStyleID);
	CMessageDoc *pDoc = (CMessageDoc*) GetDocument();
	if (pDoc)
	{
		ASSERT_KINDOF(CMessageDoc, pDoc);
		if (pDoc->m_Sum)
			pDoc->m_Sum->SetShowAllHeaders( FALSE );
	}
}


BEGIN_MESSAGE_MAP(PgReadMsgView, PgMsgView)
	//{{AFX_MSG_MAP(PgReadMsgView)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_BN_CLICKED( ID_EDIT_MESSAGE, OnEditMode )
	ON_BN_CLICKED( ID_BLAHBLAHBLAH, OnBlahBlahBlah )
	//ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_BN_CLICKED( ID_USE_FIXED_FONT, OnFixedFont )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PgReadMsgView diagnostics

#ifdef _DEBUG
void PgReadMsgView::AssertValid() const
{
	PgMsgView::AssertValid();
}

void PgReadMsgView::Dump(CDumpContext& dc) const
{
	PgMsgView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// PgReadMsgView message handlers

void PgReadMsgView::OnInitialUpdate() 
{
    PgMsgView::OnInitialUpdate();

    // set the document subtype; PgMsgView::OnInitialUpdate will have set
    // the major type (kMessage)
    //
    // BOG: maybe this should be in OnCreate?
    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    pSB->subkind = PgStuffBucket::kReadMessage;
    UnuseMemory( m_paigeRef );

    //Initialize the new style sheet for displaying/hiding Taboo Headers
	long bodystyleID;
	font_info fontInfo;
	style_info TabooHeaderStyle;

	bodystyleID = pgGetNamedStyleIndex(m_paigeRef, body_style);
	if (bodystyleID)
	{
		pgGetNamedStyleInfo(m_paigeRef, bodystyleID, &TabooHeaderStyle, &fontInfo, NULL);	
	}
	else
	{
		TabooHeaderStyle = PgGlobalsPtr()->def_style;
		fontInfo = PgGlobalsPtr()->def_font;
	}
    
	TabooHeaderStyle.styles[hidden_text_var] = 1;
    m_TabooHeadersStyleID = pgNewStyle(m_paigeRef, &TabooHeaderStyle, &fontInfo);

    ASSERT(m_TabooHeadersStyleID != 0);

    ImportMessage();
    UpdateScrollBars(true);

    if (GetIniShort(IDS_INI_ZOOM_WINDOWS))
    {
	CReadMessageFrame* Frame = (CReadMessageFrame*)GetParentFrame();
	ASSERT_KINDOF(CReadMessageFrame, Frame);
	Frame->ZoomWindow();
    }

    SetReadOnly();
}


// ImportMessage:
// Overidden to supply the "read message" headers.

void PgReadMsgView::ImportMessage( CMessageDoc* pMsgDoc /* = NULL */ )
{
	CMessageDoc* pDoc = NULL;

	if ( pMsgDoc )
		pDoc = pMsgDoc;
	else
		pDoc = (CMessageDoc*) GetDocument();

	ASSERT_KINDOF(CMessageDoc, pDoc);
	
	//
	// Import the Message Body
	//	
	PgMsgView::ImportMessage( pMsgDoc );

	//Remember the par info cos ApplyNamedStyle messes up the
	//par info for the first paragraph after adding the headers
	select_pair sel;
	sel.begin = sel.end = 0;
	par_info info, mask;
	pgGetParInfo(m_paigeRef, &sel, FALSE, &info, &mask);

	int nPreHeaderSize = pgTextSize(m_paigeRef);
	//
	// Get the length of the headers.
	//
	char* headers = pDoc->GetText();
	const char* bodyOnly = FindBody( headers );
	int nHeaderBytes = bodyOnly - headers;
	//
	// create a Paige memory ref containing the headers
	//
	memory_ref impRef = Bytes2Memory( PgMemGlobalsPtr(),
				(pg_char_ptr)headers, sizeof(pg_char), nHeaderBytes );
	//
	// Import the headers at the beginning.
	//
	pgSetSelection( m_paigeRef, 0, 0, 0, TRUE );
	pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);
	PgDataTranslator theTxr( m_paigeRef );
	theTxr.ImportData( &impRef, PgDataTranslator::kFmtText );
	//
	// Recalculate the length of the headers after import
	//
	int nPostHeaderSize = pgTextSize(m_paigeRef);
	m_HeaderBytes = nPostHeaderSize - nPreHeaderSize;

	//Reset the par info for the first paragraph
	sel.begin = sel.end = m_HeaderBytes;
	pgFillBlock(&mask, sizeof(par_info), SET_MASK_BITS);
	pgSetParInfo(m_paigeRef, &sel, &info, &mask, best_way);

	//
	// Taboo the nasty headers and adjust caret position to beginning of body.
	//
	MakeAutoURLSpaghetti(0, m_HeaderBytes);
	SetTabooHeadersStyle();
	pgSetSelection( m_paigeRef, m_HeaderBytes, m_HeaderBytes, 0, TRUE );
	DisposeMemory( impRef );
}


void PgReadMsgView::SetTabooHeadersStyle()
{
	const CStringArray*	pTabooHeaderArray;
	char *szHeader;
	CString testHeader;

	CReadMessageDoc *pDoc = (CReadMessageDoc*) GetDocument();
	ASSERT_KINDOF(CReadMessageDoc, pDoc);
	pTabooHeaderArray = &(pDoc->GetTabooHeaderArray());

	text_ref	textRef;
	select_pair paraOffsets, tempOffsets;
	long		currentOffset = 0, textLength;
	long		endOfText = pgTextSize(m_paigeRef);
	BOOL		SetStyleSheet = FALSE;

	pgFindPar(m_paigeRef, currentOffset, &paraOffsets.begin, &paraOffsets.end);
	szHeader = (char *) pgExamineText(m_paigeRef, currentOffset, &textRef, &textLength);

	while (currentOffset < m_HeaderBytes-1) //endOfText-1)
	{

		//Can check for Taboo headers from the last Taboo header seen to the end of
		//the array, but if someone changes the Taboo headers array order then we'll
		//be hosed. So leave out that optimization and check against all the headers
		//all the time.
		for ( int i = 0, compareLength = ( (*pTabooHeaderArray )[i] ).GetLength(), numHeaders = pTabooHeaderArray->GetSize(); 
				i < numHeaders - 1; 
				++i,  compareLength = ( ( *pTabooHeaderArray )[i] ).GetLength() )
		{
			testHeader.Empty(); 
			testHeader = (*pTabooHeaderArray)[i];
			//ASSERT(textLength >= compareLength);
			if (textLength >= compareLength)
			{
				if ( !strnicmp( (*pTabooHeaderArray)[i], szHeader, compareLength) )
				{
					//pgSetStyleSheet(m_paigeRef, &paraOffsets, m_TabooHeadersStyleID, draw_none);
					SetStyleSheet = TRUE;
				}
			}
			else 
			{
				//We have to span over multiple blocks to get the text
				long diff = compareLength - textLength;
				char header[256];
				char *rest = NULL;

				strncpy(header, szHeader, textLength);
				header[textLength] = '\0';

				UnuseMemory(textRef);
				rest = (char *) pgExamineText(m_paigeRef, currentOffset + textLength, &textRef, &textLength);

				if ((!rest) || (textLength < diff))
					break;

				strncat(header, rest, diff);

				if ( !strnicmp( (*pTabooHeaderArray)[i], header, compareLength) )
				{
					//pgSetStyleSheet(m_paigeRef, &paraOffsets, m_TabooHeadersStyleID, draw_none);
					SetStyleSheet = TRUE;
				}
				
			}
		}

		currentOffset = paraOffsets.end; 

		while(currentOffset < m_HeaderBytes-1 ) //endOfText - 1)
		{
			UnuseMemory(textRef);
			pgFindPar(m_paigeRef, currentOffset, &tempOffsets.begin, &tempOffsets.end);
			szHeader = (char *) pgExamineText(m_paigeRef, currentOffset, &textRef, &textLength);
			if ( !( (*szHeader == ' ') || (*szHeader == '\t') ) )
				break;
			else 
				paraOffsets.end = tempOffsets.end;

			currentOffset = tempOffsets.end;
		}

		if (SetStyleSheet)
		{
			pgSetStyleSheet(m_paigeRef, &paraOffsets, m_TabooHeadersStyleID, draw_none);
			SetStyleSheet = FALSE;
		}

		paraOffsets.begin = tempOffsets.begin;
		paraOffsets.end = tempOffsets.end;
	}

	UnuseMemory(textRef);

}

// ExportMessage:
// Standard, generalized way of saving to a "message doc"

void PgReadMsgView::ExportMessage( CMessageDoc* pMsgDoc /* = NULL */ )
{
	CMessageDoc* pDoc = NULL;

	if ( pMsgDoc )
		pDoc = pMsgDoc;
	else
		pDoc = (CMessageDoc*) GetDocument();

	VERIFY( pDoc );
	ASSERT( pDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)) );
	
	CSummary* pSum = pDoc->m_Sum;
	VERIFY( pSum );
	
	select_pair bodySel;
	bodySel.begin = m_HeaderBytes;
	bodySel.end = pgTextSize(m_paigeRef);

	// figger out whatsup wi' dat data format!
	PgDataTranslator::FormatType txrType;

	if ( (PgHasStyledText( m_paigeRef, PgGlobalsPtr() )) || (SelectionHasHtmlSnippet(bodySel))) {
		pSum->SetFlag(MSF_XRICH);
		pSum->SetFlagEx(MSFEX_HTML);
		txrType = PgDataTranslator::kFmtHtml;
	}
	else {
		txrType = PgDataTranslator::kFmtText;
	}

	//If it is a plain text message, then we can just splat the
	//whole thing into the MsgDoc. But if it is a *HTML* message,
	//then we need to first save the headers as plain text and
	//then the body as HMTL
	if (txrType == PgDataTranslator::kFmtText)
	{
		// do the deed
		memory_ref pgmRef;
		PgDataTranslator theTxr( m_paigeRef );
		theTxr.ExportData( &pgmRef, txrType );

		// convert the PGMM reference to a c-string, and jam it in the doc. note
		// that anything in the paige instance (headers, whatever) is gonna
		// get saved as part of the message.
		char* pMem = (char*) UseMemoryToCStr( pgmRef );
		pDoc->SetText( pMem );
		// get out before someone gets hurt
		UnuseAndDispose( pgmRef );
	}
	else
	{
		select_pair hdrSel;

		//Get the Headers
		hdrSel.begin = 0;
		hdrSel.end = m_HeaderBytes;
		
		pg_ref pgHdrRef = pgCopy(m_paigeRef, &hdrSel);
		memory_ref pgHdr;
		PgDataTranslator hdrTxr(pgHdrRef);
		hdrTxr.ExportData(&pgHdr, PgDataTranslator::kFmtText);
		char *hdr = (char*)UseMemoryToCStr( pgHdr );

		//Get the Body
		//sel.begin = m_HeaderBytes;
		//sel.end = pgTextSize(m_paigeRef);

		// BOG: the copy we make here does not have one important attribute
		// of the original... The StuffBucket! since the lifetime of the copy is
		// limited to this routine, we can just "borrow" the original bucket.
		//
		// Note: this technique is something to do very carefully. the StuffBucket
		// contains mfc specific stuff like CWnd pointers, which might not be
		// safe to use in some scenarios. it's ok here, since they won't be used.
		pg_ref pgBodyRef = pgCopy(m_paigeRef, &bodySel);
		paige_rec_ptr pgrOrg = (paige_rec_ptr) UseMemory( m_paigeRef );
		paige_rec_ptr pgrCpy = (paige_rec_ptr) UseMemory( pgBodyRef );
		pgrCpy->user_refcon = pgrOrg->user_refcon;
		UnuseMemory( m_paigeRef );
		UnuseMemory( pgBodyRef );

		memory_ref pgBody;
		PgDataTranslator bodyTxr(pgBodyRef);
		bodyTxr.ExportData(&pgBody, PgDataTranslator::kFmtHtml);
		char *body = (char*)UseMemoryToCStr( pgBody );

		int hdrLen = strlen(hdr);
		int bodyLen = strlen(body);
		char *theMsg = new char[ hdrLen + bodyLen + 3];
		*theMsg = '\0';

		strncat(theMsg, hdr, hdrLen);

		//Paige Text translator has this bug where the last \r\n is
		//truncated to just \r. So add in the \n. This is just a temporary
		//fix until I correct the Text translator. === K
		//int msgLen = strlen(theMsg);
		//theMsg[msgLen] = '\n';
		//theMsg[msgLen] = '\0';
	
		strncat(theMsg, body, bodyLen);
		pDoc->SetText( theMsg );
		
		if (theMsg)
			delete [] theMsg;
		UnuseAndDispose( pgHdr );
		UnuseAndDispose( pgBody );
	}
	
	SaveChangeState();
}

void PgReadMsgView::OnEditMode()
{
	SetReadOnly( !m_fRO );
	//We do not allow the user to edit the headers
	//I think we need to display the headers a little differently
	//in this case.
}

void PgReadMsgView::OnBlahBlahBlah()
{
	style_info TabooHeaderStyleInfo;
	pgGetStyle(m_paigeRef, m_TabooHeadersStyleID, &TabooHeaderStyleInfo);

	font_info fontInfo;
	pgGetFontInfoRec(m_paigeRef, TabooHeaderStyleInfo.font_index, &fontInfo);

	//Change the style bits
	if (TabooHeaderStyleInfo.styles[hidden_text_var] == 1)
		TabooHeaderStyleInfo.styles[hidden_text_var] = 0;
	else 
		TabooHeaderStyleInfo.styles[hidden_text_var] = 1;

	CMessageDoc *pDoc = (CMessageDoc*) GetDocument();
	ASSERT_KINDOF(CMessageDoc, pDoc);
	pDoc->m_Sum->ToggleFlag( MSF_SHOW_ALL_HEADERS );

	//This will cause all the text using the stylesheet to be redrawn with
	//the new style info
	pgChangeStyle(m_paigeRef, m_TabooHeadersStyleID, &TabooHeaderStyleInfo,
		&fontInfo, best_way);

	//Set the cursor at the beginning of the document 
	pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);

	ScrollToCursor();
}


/*void PgReadMsgView::OnFileSaveAs() 
{
	CSaveAs SA;
	CReadMessageDoc* doc = (CReadMessageDoc*)GetDocument();
	
	//if (doc && doc->m_Sum && SA.Start(doc->m_Sum->m_Subject, TRUE))
	//	doc->SaveAs(SA);
}
*/

BOOL PgReadMsgView::GetMessageAsHTML( CString& szText, BOOL IncludeHeaders )
{
    szText.Empty();

    if ( IncludeHeaders ) {
        GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtHtml );
    }
    else {
        select_pair sel;
        sel.begin = m_HeaderBytes;
        sel.end = pgTextSize( m_paigeRef );

        pg_ref impRef = pgCopy( m_paigeRef, &sel );
        paige_rec_ptr ppgCopy = (paige_rec_ptr) UseMemory( impRef );
        paige_rec_ptr ppgReal = (paige_rec_ptr) UseMemory( m_paigeRef );
        ppgCopy->user_refcon = ppgReal->user_refcon;
        UnuseMemory( impRef );
        UnuseMemory( m_paigeRef );

        GetTextAs( impRef, szText, PgDataTranslator::kFmtHtml );
        UnuseAndDispose( impRef );
    }

    return TRUE;
}

BOOL PgReadMsgView::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
	//Should collapse this func and GetMessageAsHtml, but maybe too expensive.
	//Lets leave it for now.

	pg_ref impRef;

	szText.Empty();

	if (IncludeHeaders)
	{
		GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtText );
	}
	else
	{
		select_pair sel;
		sel.begin = m_HeaderBytes;
		sel.end = pgTextSize(m_paigeRef);

		impRef = pgCopy(m_paigeRef, &sel);

		GetTextAs( impRef, szText, PgDataTranslator::kFmtText );

		UnuseAndDispose(impRef);
	}

	return (TRUE);

}


BOOL PgReadMsgView::SetAllText( const char* szText,bool bSign )
{
	style_info TabooHeaderStyleInfo;
	pgGetStyle(m_paigeRef, m_TabooHeadersStyleID, &TabooHeaderStyleInfo);

	BOOL turnedOnBlah = FALSE;

	//Change the style bits
	if (TabooHeaderStyleInfo.styles[hidden_text_var] == 1)
	{
		turnedOnBlah = TRUE;		
		OnBlahBlahBlah();
	}
	select_pair sel;
	sel.begin = 0;
	sel.end = pgTextSize(m_paigeRef);
	pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
	ClearStyle();
	BOOL result = CPaigeEdtView::SetAllText(szText, bSign);
	SetTabooHeadersStyle();
	if (turnedOnBlah)
		OnBlahBlahBlah();
	return result;
}


BOOL PgReadMsgView::SetAllHTML( const char* szHTML,bool bSign )
{
	style_info TabooHeaderStyleInfo;
	pgGetStyle(m_paigeRef, m_TabooHeadersStyleID, &TabooHeaderStyleInfo);

	BOOL turnedOnBlah = FALSE;
	
	//Change the style bits
	if (TabooHeaderStyleInfo.styles[hidden_text_var] == 1)
	{
		turnedOnBlah = TRUE;
		OnBlahBlahBlah();
	}
	
	select_pair sel;
	sel.begin = 0;
	sel.end = pgTextSize(m_paigeRef);
	pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
	ClearStyle();

	BOOL result = CPaigeEdtView::SetAllHTML(szHTML, bSign);
	
	SetTabooHeadersStyle();
	if (turnedOnBlah)
		OnBlahBlahBlah();

	return result;
}


void PgReadMsgView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);	
}

void PgReadMsgView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_fRO)
		CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags);
	else
	{
		//Editing allowed only if not in the headers
		long selectLeft;
		pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
		if (selectLeft >= m_HeaderBytes)
			CPaigeEdtView::OnChar(nChar, nRepCnt, nFlags);
		else 
			CView::OnChar(nChar, nRepCnt, nFlags);
	}
}

void PgReadMsgView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_fRO)
		CPaigeEdtView::OnKeyDown(nChar, nRepCnt, nFlags);
	else
	{
		//If in the Headers don't process Delete and Backspace.
		//Navigation and selection is allowed in the headers.
		//Other characters are taken care of in OnChar
		long selectLeft;
		pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
		if (selectLeft < m_HeaderBytes)
		{
			switch(nChar)
			{
			case VK_DELETE :
			case VK_BACK :
				return;
			default :
				CPaigeEdtView::OnKeyDown(nChar, nRepCnt, nFlags);
			}
		}
		else
			CPaigeEdtView::OnKeyDown(nChar, nRepCnt, nFlags);
	}

}

void PgReadMsgView::OnFixedFont()
{
	//
	// Reset the style sheet for the body
	//	
	long styleID;
	if ( !(styleID = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
		return;

	font_info	namedFont;
	style_info	namedStyle;

	pgGetStyle(m_paigeRef, (short)styleID, &namedStyle);
	//pgGetNamedStyleInfo(m_paigeRef, styleID, &namedStyle, &namedFont, &namedPar);
	//pgGetFontInfoRec(m_paigeRef, (short)styleID, &fontInfo);

	//Get the font and style info
	CReadMessageFrame* Frame = ( CReadMessageFrame* ) GetParentFrame();
	ASSERT_KINDOF( CReadMessageFrame, Frame );

	BOOL bChecked = Frame->UseFixedWidthFont();

	LOGFONT lf;
	style_info styleInfo, styleInfoMask;
	
	if (bChecked)
		GetMessageFixedFont().GetLogFont(&lf);
	else
		GetMessageProportionalFont().GetLogFont(&lf);
	PgConvertLogFont( m_paigeRef, PgGlobalsPtr(), &lf, &namedFont, &styleInfo, &styleInfoMask );

	namedStyle.point = pgGetDefaultPointSize();
	if ( lf.lfWeight == FW_BOLD )
		namedStyle.styles[bold_var] = -1;
	if ( lf.lfItalic )
		namedStyle.styles[italic_var] = -1;
	if ( lf.lfUnderline )
		namedStyle.styles[underline_var] = -1;
	if ( lf.lfStrikeOut )
		namedStyle.styles[strikeout_var] = -1;
//	styleInfo.point <<= 16;

	//Now change the body style
	pgChangeStyle(m_paigeRef, (short)styleID, &namedStyle, &namedFont, draw_none);

	//Also need to change the font for the taboo headers style ID
	style_info tabooStyle;
	pgGetStyle(m_paigeRef, m_TabooHeadersStyleID, &tabooStyle);
	pgChangeStyle(m_paigeRef, m_TabooHeadersStyleID, &tabooStyle, &namedFont, best_way);

}
