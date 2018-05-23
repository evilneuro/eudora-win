/////////////////////////////////////////////////////////////////////////////
// 	File: PRINTEDIT.CPP
//
//  Purpose:  This file provides the declarations for the huge
//			edit view class.  This class is subclassed from the
//			CTerView class...which provides these huge edit regions
//			This class also provides print and print preview 
//			functions, as well as standard headers/footers for the
//			document being printed
//
/////////////////////////////////////////////////////////////////////////////

//	Includes
#include "stdafx.h"

#include "mdichild.h"
#include "resource.h"
#include "rs.h"
#include "DSPrintEdit.h"
#include "font.h"
#include "address.h"
#include "saveas.h"
#include "doc.h"
#include "textfile.h"


// Defines and other constants..
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// set up the inherited define for easier coding                
#undef	inherited
#define	inherited CEditView


/////////////////////////////////////////////////////////////////////////////
// DSPrintEditView


IMPLEMENT_SERIAL(DSPrintEditView, CEditView, 0)

//	======================================================================================== 
DSPrintEditView::DSPrintEditView()
    : sppfCB(NULL), pvUserData(NULL)
{
	// Initialize the printer variables
	m_includeHeaderFooters	= GetIniShort(IDS_INI_PRINT_HEADERS);
	memset( &m_thePrintInfo, 0, sizeof( m_thePrintInfo ) );
}


//	========================================================================================
DSPrintEditView::~DSPrintEditView()
{
}

BOOL DSPrintEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (inherited::PreCreateWindow(cs) == FALSE)
		return (FALSE);
		
	cs.style &= ~(WS_HSCROLL | ES_AUTOHSCROLL);// | ES_NOHIDESEL);
	
	return (TRUE);
}

void DSPrintEditView::OnInitialUpdate()
{
	inherited::OnInitialUpdate();

	GetEditCtrl().SetFont(&ScreenFont, FALSE);
}

BEGIN_MESSAGE_MAP(DSPrintEditView, inherited)
	//{{AFX_MSG_MAP(DSPrintEditView)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//	========================================================================================
// DSPrintEditView serialization
void 
DSPrintEditView::Serialize(CArchive& ar)
{
	inherited::SerializeRaw(ar);
}

//	========================================================================================
// DSPrintEditView message handlers

void DSPrintEditView::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	// File->Save As... can always be done
	pCmdUI->Enable(TRUE);
}

void DSPrintEditView::OnFileSaveAs()
{
	CSaveAs SA;    
	
	if (GetDocument()->IsKindOf(RUNTIME_CLASS(CTextFileDoc)))
	{
	   ((CTextFileDoc *)GetDocument())->OnFileSaveAs();
	   return;
	}
	
	if (SA.Start(GetDocument()->GetPathName()))
	{   
		SA.PutText(LockBuffer());
		UnlockBuffer();
	}
}

void DSPrintEditView::OnFilePrint()
{
	inherited::OnFilePrint();
}

void DSPrintEditView::OnFilePrintPreview()
{
	inherited::OnFilePrintPreview();
}

void DSPrintEditView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
  	CEditView::OnPrepareDC( pDC, pInfo );
 }

//	========================================================================================
// OnBeginPrinting
void
DSPrintEditView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{   
    (*sppfCB)(pvUserData, true);

	TEXTMETRIC	theTM;
	CFont*		pOldFont;
	ULONG		ulXRes;
	ULONG		ulYRes;
	ULONG		ulPageWidthPixels;
	ULONG		ulPageHeightPixels;
	ULONG		ulOneHalfInchX;
	ULONG		ulOneHalfInchY;
	ULONG		ulThreeQuartersInchX;
	ULONG		ulThreeQuartersInchY;
	CString strName = GetIniString(IDS_INI_PRINTER_FONT);

	// initialize the pointer to the old font
	pOldFont = NULL;

	// call the base class
	inherited::OnBeginPrinting(pDC,pInfo); 

	// get the buffersize
	m_thePrintInfo.m_ulBufferSize =  GetBufferLength();
	
	// make the printer font
	m_thePrintInfo.m_pPrinterFont = new CFontInfo();
	
	if ( m_thePrintInfo.m_pPrinterFont != NULL )
	{
		// Lets get the user demanded font info
		m_thePrintInfo.m_pPrinterFont->CreateFontFromMapping(strName, (int)GetIniShort(IDS_INI_PF_SIZE), pDC );
		SetPrinterFont( m_thePrintInfo.m_pPrinterFont );
	}
	
	// save the mapping mode
	m_thePrintInfo.m_uOldMappingMode = pDC->SetMapMode( MM_TEXT );

	// get the x & y resolutions
	ulXRes = pDC->GetDeviceCaps( LOGPIXELSX );
	ulYRes = pDC->GetDeviceCaps( LOGPIXELSY );

	// get the size of the page in pixels
	ulPageWidthPixels = pDC->GetDeviceCaps( HORZRES );
	ulPageHeightPixels = pDC->GetDeviceCaps( VERTRES );
  	
	// save some handy values
	ulOneHalfInchX = ( ulXRes / 2 );
	ulOneHalfInchY = ( ulYRes / 2 );
	ulThreeQuartersInchX = ( ulXRes * 3UL ) / 4;
	ulThreeQuartersInchY = ( ulYRes * 3UL ) / 4;
                                         
	// get the body rect
	m_thePrintInfo.m_theBodyRect.left = ( int ) ulThreeQuartersInchX;
	m_thePrintInfo.m_theBodyRect.right = ( int ) ( ulPageWidthPixels - ulThreeQuartersInchX );
	
	if ( !m_includeHeaderFooters )
	{
		m_thePrintInfo.m_theBodyRect.top = ( int ) ulThreeQuartersInchY;
		m_thePrintInfo.m_theBodyRect.bottom = ( int ) ( ulPageHeightPixels - ulThreeQuartersInchY );
	}
	else
	{
		// create the header font 
		m_thePrintInfo.m_pHeaderFont = new CFontInfo(FW_BOLD);
		
		if ( m_thePrintInfo.m_pHeaderFont != NULL )
		{
			// Lets get the user demanded font info
			m_thePrintInfo.m_pHeaderFont->CreateFontFromMapping(strName, (int)GetIniShort(IDS_INI_PF_SIZE), pDC );
			
		}
	
		// create the pen
		m_thePrintInfo.m_lPenWidth = ulYRes / 64; // 1/64 inch in pixels 
		m_thePrintInfo.m_pPen = new CPen( PS_SOLID, ( int ) ( m_thePrintInfo.m_lPenWidth ), RGB( 0, 0, 0 ) );

		if ( m_thePrintInfo.m_pHeaderFont != NULL )
		{
			// select the font
			pOldFont = pDC->SelectObject( m_thePrintInfo.m_pHeaderFont );
		}
	
		// get the metrics
		pDC->GetTextMetrics( &theTM );
		
		// get the position for the header text in pixels
		m_thePrintInfo.m_HeaderTextRect.top = ( int ) ulOneHalfInchY;
		m_thePrintInfo.m_HeaderTextRect.bottom = ( int )(	m_thePrintInfo.m_HeaderTextRect.top + 
															theTM.tmHeight + 
															theTM.tmExternalLeading );
		m_thePrintInfo.m_HeaderTextRect.left = ( int ) ulOneHalfInchX;
		m_thePrintInfo.m_HeaderTextRect.right =	( int ) ( ulPageWidthPixels - ulOneHalfInchX );

		// get the position for the footer text
		m_thePrintInfo.m_FooterTextRect.bottom = ( int )( ulPageHeightPixels - ulOneHalfInchY );
		m_thePrintInfo.m_FooterTextRect.top =	( int )(	m_thePrintInfo.m_FooterTextRect.bottom - 
															theTM.tmHeight );
		m_thePrintInfo.m_FooterTextRect.left = ( int ) m_thePrintInfo.m_HeaderTextRect.left;
		m_thePrintInfo.m_FooterTextRect.right = ( int ) m_thePrintInfo.m_HeaderTextRect.right;

		
		// reset the font
		if ( m_thePrintInfo.m_pHeaderFont != NULL )
		{
			// select the font
			pDC->SelectObject( pOldFont );
		}
					
		// set the top margin in pixels
		m_thePrintInfo.m_theBodyRect.top = ( int )(	m_thePrintInfo.m_HeaderTextRect.bottom +
													( 2 * m_thePrintInfo.m_lPenWidth ) + 
													( ulOneHalfInchY / 2 ) );

		m_thePrintInfo.m_theBodyRect.bottom = ( int ) ( ulPageHeightPixels - m_thePrintInfo.m_theBodyRect.top );
	}	
}

//	========================================================================================
// OnEndPrinting
void DSPrintEditView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{

	//pInfo->m_bContinuePrinting = 0;

	inherited::OnEndPrinting(pDC, pInfo);

	SetPrinterFont(NULL);
	
	// reset things back to the previous settings
//	GetDocument()->SetModifiedFlag( m_thePrintInfo.m_bIsModified );

	// delete the printer font
	delete m_thePrintInfo.m_pPrinterFont;
	m_thePrintInfo.m_pPrinterFont = NULL;
	
	// delete the header font
	delete m_thePrintInfo.m_pHeaderFont;
	m_thePrintInfo.m_pHeaderFont = NULL;

	// delete the pen
	delete m_thePrintInfo.m_pPen;
	m_thePrintInfo.m_pPen = NULL;

	// reset the mapping mode
	pDC->SetMapMode( m_thePrintInfo.m_uOldMappingMode );

    (*sppfCB)(pvUserData, false);

}


//	========================================================================================
// OnPrint
void DSPrintEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CFont*	pOldFont;
	CRect	theRect;
	ULONG	ulNextIndex;
	
	// initialize the pointer to the current font
	pOldFont = NULL;

	// print the page header and footer
	if (m_includeHeaderFooters)
	{
		//pDC->SetBkMode( OPAQUE );
		DoPageHeader(pDC, pInfo);
		DoPageFooter(pDC, pInfo);
	}

	// select the printer font
	if ( m_thePrintInfo.m_pPrinterFont != NULL )
	{
		pOldFont = pDC->SelectObject( m_thePrintInfo.m_pPrinterFont );
	}


	// copy the rect since PrintInsideRect will alter it
	theRect = m_thePrintInfo.m_theBodyRect;

	// print
	ulNextIndex = PrintInsideRect(	pDC, 
									theRect,		
									m_aPageStart[ pInfo->m_nCurPage - 1 ] , 
									( UINT ) ( m_thePrintInfo.m_ulBufferSize ) );

	if ( pInfo->m_nCurPage == ( UINT ) m_aPageStart.GetSize() )
	{
		if( ulNextIndex < m_thePrintInfo.m_ulBufferSize )
		{
			// add this index to the start index array
			m_aPageStart.Add( ulNextIndex );
		}                     
		else
		{
			// tell mfc that this is the last page
			pInfo->SetMaxPage( pInfo->m_nCurPage );
		}
	}

	// reset the font
	if ( ( m_thePrintInfo.m_pPrinterFont != NULL ) && ( pOldFont != NULL ) )
	{
		pDC->SelectObject( pOldFont );
	}
} 	

//	========================================================================================
// DoPageHeader
void DSPrintEditView::DoPageHeader(CDC* pDC, CPrintInfo* pInfo)
{
	char 	temp[133];
	CFont	*pOldFont;
	CPen	*pOldPen;
	CRect	rcWhoCares;
	BOOL	bLayoutOnly = (pDC->GetClipBox( &rcWhoCares ) == NULLREGION);

	(pInfo);
	// this is better than the above line of code. If all you want to do is
	// test for a null-region, don't also modify the drawing rect!

	if ( bLayoutOnly ) 
	{
		return;
	}
 
	// Get the old font...and install our emboldend one
	if ( m_thePrintInfo.m_pHeaderFont != NULL )
	{
		pOldFont = ( CFont* ) pDC->SelectObject( m_thePrintInfo.m_pHeaderFont ) ;
	}
	
	if ( m_thePrintInfo.m_pPen )
	{
		// set the pen
		pOldPen = pDC->SelectObject( m_thePrintInfo.m_pPen );
	}

	// Develope the text for the header
	GetHeaderText(temp);  
		
	// Draw the text and line, but only if we're not just checking the layout
	pDC->DrawText(	temp, 
					-1, 
					&( m_thePrintInfo.m_HeaderTextRect ), 
					DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

	pDC->MoveTo(	m_thePrintInfo.m_HeaderTextRect.left, 
					( int ) ( m_thePrintInfo.m_HeaderTextRect.bottom + m_thePrintInfo.m_lPenWidth ) );
	pDC->LineTo(	m_thePrintInfo.m_HeaderTextRect.right, 
					( int ) ( m_thePrintInfo.m_HeaderTextRect.bottom + m_thePrintInfo.m_lPenWidth ) );
}  

//	========================================================================================
// DoPageFooter
void DSPrintEditView::DoPageFooter(CDC* pDC, CPrintInfo* pInfo)
{
	char 		temp[256];
	CDocument*	pDoc	= GetDocument();
	long		tempSize;
	CFont*		pOldFont;
	CPen*		pOldPen;
	CSize		size;
	CRect		rectPage;

	// initialize the pointer to the current font and pen
	pOldFont = NULL;
	pOldPen = NULL;

	// this is better than the above line of code. If all you want to do is
	// test for a null-region, don't also modify the drawing rect!
	CRect rcWhoCares;
	BOOL bLayoutOnly = (pDC->GetClipBox( &rcWhoCares ) == NULLREGION);
	
	if ( bLayoutOnly ) 
	{
		return;
	}

	if ( m_thePrintInfo.m_pHeaderFont )
	{
		// Set the font for our operation here...
		pOldFont = pDC->SelectObject( m_thePrintInfo.m_pHeaderFont );
	}

	if ( m_thePrintInfo.m_pPen )
	{
		// set the pen
		pOldPen = pDC->SelectObject( m_thePrintInfo.m_pPen );
	}

	// Lets get the footer text and put it in the footer
	GetFooterText(temp);
	tempSize = strlen(temp);
	
	if (tempSize > 60)
	{
		strcpy(temp + 60, "...");
	}

	// Draw the text
	pDC->DrawText(	temp, 
					-1, 
					&( m_thePrintInfo.m_FooterTextRect ), 
					DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

	// print the page #
	wsprintf(temp, "%d  ", pInfo->m_nCurPage);
	
	rectPage = m_thePrintInfo.m_FooterTextRect;
	size = pDC->GetTextExtent(temp, strlen(temp));
	
	rectPage.left = rectPage.right - size.cx;

	pDC->DrawText(temp, -1, &rectPage, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

	// draw the line
	pDC->MoveTo(	m_thePrintInfo.m_FooterTextRect.left, 
					( int ) ( m_thePrintInfo.m_FooterTextRect.top - ( m_thePrintInfo.m_lPenWidth * 2 ) ) );
	pDC->LineTo(	m_thePrintInfo.m_FooterTextRect.right, 
					( int ) ( m_thePrintInfo.m_FooterTextRect.top - ( m_thePrintInfo.m_lPenWidth * 2 ) ) );

	// Restore the original font & pen
	if( m_thePrintInfo.m_pHeaderFont ) 
	{
		pDC->SelectObject( pOldFont );
	}

	if( m_thePrintInfo.m_pPen ) 
	{
		pDC->SelectObject( pOldPen );
	}
}

//	========================================================================================
// GetHeaderText
void DSPrintEditView::GetHeaderText(char* theText)
{
	ASSERT(theText != NULL);

	CDocument* pDoc = GetDocument();
	if (pDoc)
		strcpy(theText, pDoc->GetTitle());
	else
		*theText = '\0';
}


//	========================================================================================
// GetFooterText
void DSPrintEditView::GetFooterText(char* theText)
{
	sprintf(theText, (const char*)CRString(IDS_PRINT_FOOTER), GetReturnAddress());
}

void
DSPrintEditView::SetCB(SetPrintingPreviewingFlagCB _sppfCB, void *data)
{
    sppfCB = _sppfCB;
    pvUserData = data;
}


//	========================================================================================
// DSPrintTextView
//	This is the overridden view for the TextFile template...it essentially provides
//	the full pathname in the header...
//

#ifdef new
#undef new
#endif

IMPLEMENT_DYNCREATE(DSPrintTextView, DSPrintEditView)

#define new DEBUG_NEW
 
// set up the inherited define for easier coding                
#undef	inherited
#define	inherited  DSPrintEditView

//	========================================================================================
// GetHeaderText
void
DSPrintTextView::GetHeaderText(char* theText)
{
	CDocument* pDoc = GetDocument();
  	// The behavior is to use the complete path/filename in the textfile header
    wsprintf(theText,"  %s",((LPCSTR)pDoc->GetPathName()));
}


