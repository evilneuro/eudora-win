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

#include "DebugNewHelpers.h"


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
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrintOne)
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

void DSPrintEditView::OnFilePrintOne()
{
	// This will do the right thing because the default handler looks at the
	// original message to see if it was ID_FILE_PRINT or ID_FILE_PRINT_DIRECT
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
	m_thePrintInfo.m_ulBufferSize =  GetEditCtrl().GetWindowTextLength();
	
	// make the printer font
	m_thePrintInfo.m_pPrinterFont = DEBUG_NEW_MFCOBJ_NOTHROW CFontInfo();
	
	if ( m_thePrintInfo.m_pPrinterFont != NULL )
	{
		// Lets get the user demanded font info
		m_thePrintInfo.m_pPrinterFont->CreateFontFromMapping(strName, (int)GetIniShort(IDS_INI_PF_SIZE), true, pDC );
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
		m_thePrintInfo.m_pHeaderFont = DEBUG_NEW_MFCOBJ_NOTHROW CFontInfo(FW_BOLD);
		
		if ( m_thePrintInfo.m_pHeaderFont != NULL )
		{
			// Lets get the user demanded font info
			m_thePrintInfo.m_pHeaderFont->CreateFontFromMapping(strName, (int)GetIniShort(IDS_INI_PF_SIZE), true, pDC );
			
		}
	
		// create the pen
		m_thePrintInfo.m_lPenWidth = ulYRes / 64; // 1/64 inch in pixels 
		m_thePrintInfo.m_pPen = DEBUG_NEW_MFCOBJ_NOTHROW CPen( PS_SOLID, ( int ) ( m_thePrintInfo.m_lPenWidth ), RGB( 0, 0, 0 ) );

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


//
// This is an ugly hack.  The PrintInsideRect function is an almost
// identical copy to the source code in MFC for
// CEditView::PrintInsideRect, except that it replaces the calls to
// GetBufferLength(), LockBuffer(), and UnlockBuffer() with calls
// that actually work.  It seems that MS broke something in the new
// XP-style common controls that makes calling LocalLock() return a
// buffer with a single space in it rather than the actual buffer
// of the text in the control.
//
#ifdef _MBCS
	extern AFX_DATA const BOOL _afxDBCS;
#else
	#define _afxDBCS FALSE
#endif

AFX_STATIC UINT AFXAPI _AfxEndOfLine(LPCTSTR lpszText, UINT nLen, UINT nIndex)
{
	ASSERT(AfxIsValidAddress(lpszText, nLen, FALSE));
	LPCTSTR lpsz = lpszText + nIndex;
	LPCTSTR lpszStop = lpszText + nLen;
	while (lpsz < lpszStop && *lpsz != '\r')
		++lpsz;
	return lpsz - lpszText;
}

AFX_STATIC UINT AFXAPI _AfxNextLine(LPCTSTR lpszText, UINT nLen, UINT nIndex)
{
	ASSERT(AfxIsValidAddress(lpszText, nLen, FALSE));
	LPCTSTR lpsz = lpszText + nIndex;
	LPCTSTR lpszStop = lpszText + nLen;
	while (lpsz < lpszStop && *lpsz == '\r')
		++lpsz;
	if (lpsz < lpszStop && *lpsz == '\n')
		++lpsz;
	return lpsz - lpszText;
}

AFX_STATIC UINT AFXAPI
_AfxClipLine(CDC* pDC, int aCharWidths[256], int cxLine, int nTabStop,
	LPCTSTR lpszText, UINT nIndex, UINT nIndexEnd)
{
	ASSERT_VALID(pDC);
	ASSERT(nIndex < nIndexEnd);
	ASSERT(AfxIsValidAddress(lpszText, nIndexEnd, FALSE));

	TEXTMETRIC tm;
	::GetTextMetrics(pDC->m_hDC, &tm);

	// make an initial guess on the number of characters that will fit
	int cx = 0;
	LPCTSTR lpszStart = lpszText + nIndex;
	LPCTSTR lpszStop = lpszText + nIndexEnd;
	LPCTSTR lpsz = lpszStart;
	while (lpsz < lpszStop)
	{
		if (*lpsz == '\t')
			cx += nTabStop - (cx % nTabStop);
		else
		{
#ifdef _UNICODE
			if (*lpsz <= 0xFF)
				cx += aCharWidths[(BYTE)*lpsz];
			else
				cx += tm.tmAveCharWidth;
#else //_UNICODE
			if (_afxDBCS && _istlead(*lpsz))
			{
				++lpsz;
				cx += tm.tmAveCharWidth;
			}
			else
				cx += aCharWidths[(BYTE)*lpsz];
#endif //!_UNICODE
		}
		++lpsz;
		if (cx > cxLine)
			break;
	}

	// adjust for errors in the guess
	cx = pDC->GetTabbedTextExtent(lpszStart, lpsz-lpszStart, 1, &nTabStop).cx;
	if (cx > cxLine)
	{
		// remove characters until it fits
		do
		{
			ASSERT(lpsz != lpszStart);
			if (_afxDBCS)
				lpsz = _tcsdec(lpszStart, lpsz);
			else
				--lpsz;
			cx = pDC->GetTabbedTextExtent(lpszStart, lpsz-lpszStart, 1, &nTabStop).cx;
		} while (cx > cxLine);
	}
	else if (cx < cxLine)
	{
		// add characters until it doesn't fit
		while (lpsz < lpszStop)
		{
			lpsz = _tcsinc(lpsz);
			ASSERT(lpsz <= lpszStop);
			cx = pDC->GetTabbedTextExtent(lpszStart, lpsz-lpszStart, 1, &nTabStop).cx;
			if (cx > cxLine)
			{
				if (_afxDBCS)
					lpsz = _tcsdec(lpszStart, lpsz);
				else
					--lpsz;
				break;
			}
		}
	}

	// return index of character just past the last that would fit
	return lpsz - lpszText;
}

UINT DSPrintEditView::PrintInsideRect(CDC* pDC, RECT& rectLayout,
	UINT nIndexStart, UINT nIndexStop)
	// worker function for laying out text in a rectangle.
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	BOOL bWordWrap = (GetStyle() & ES_AUTOHSCROLL) == 0;

	// get buffer and real starting and ending postions
	UINT nLen = GetEditCtrl().GetWindowTextLength();
	if (nIndexStart >= nLen)
		return nLen;
	CString TheText;
	GetEditCtrl().GetWindowText(TheText);
	LPCTSTR lpszText = TheText;
	if (nIndexStop > nLen)
		nIndexStop = nLen;
	ASSERT(nIndexStart < nLen);

	// calculate text & tab metrics
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int cyChar = tm.tmHeight + tm.tmExternalLeading;
	int nTabStop = m_nTabStops *
		pDC->GetTabbedTextExtent(_T("\t"), 1, 0, NULL).cx / 8 / 4;
	int aCharWidths[256];
	pDC->GetCharWidth(0, 255, aCharWidths);

	int y = rectLayout.top;
	UINT cx = rectLayout.right - rectLayout.left;
	UINT nIndex = nIndexStart;

	VERIFY(pDC->SaveDC() != 0);
	BOOL bLayoutOnly = pDC->IntersectClipRect(&rectLayout) == NULLREGION;

	do
	{
		UINT nIndexEnd = _AfxEndOfLine(lpszText, nIndexStop, nIndex);
		if (nIndex == nIndexEnd)
		{
			y += cyChar;
		}
		else if (bWordWrap)
		{
			// word-wrap printing
			do
			{
				UINT nIndexWrap = _AfxClipLine(pDC, aCharWidths,
					cx, nTabStop, lpszText, nIndex, nIndexEnd);
				UINT nIndexWord = nIndexWrap;
				if (nIndexWord != nIndexEnd)
				{
					while (nIndexWord > nIndex &&
					  !_istspace(lpszText[nIndexWord]))
					{
						nIndexWord--;
					}
					if (nIndexWord == nIndex)
						nIndexWord = nIndexWrap;
				}
				CRect rect(rectLayout.left, y, rectLayout.right, y+cyChar);
				if (!bLayoutOnly && pDC->RectVisible(rect))
				{
					pDC->TabbedTextOut(rect.left, y,
						(LPCTSTR)(lpszText+nIndex), nIndexWord-nIndex, 1,
						&nTabStop, rect.left);
				}
				y += cyChar;
				nIndex = nIndexWord;
				while (nIndex < nIndexEnd && _istspace(lpszText[nIndex]))
					nIndex++;
			} while (nIndex < nIndexEnd && y+cyChar <= rectLayout.bottom);

			nIndexEnd = nIndex;
		}
		else
		{
			// non-word wrap printing (much easier and faster)
			CRect rect(rectLayout.left, y, rectLayout.right, y+cyChar);
			if (!bLayoutOnly && pDC->RectVisible(rect))
			{
				UINT nIndexClip = _AfxClipLine(pDC, aCharWidths, cx, nTabStop,
					lpszText, nIndex, nIndexEnd);
				if (nIndexClip < nIndexEnd)
				{
					if (_istlead(*(lpszText+nIndexClip)))
						nIndexClip++;
					nIndexClip++;
				}
				pDC->TabbedTextOut(rect.left, y,
					(LPCTSTR)(lpszText+nIndex), nIndexClip-nIndex, 1,
					&nTabStop, rect.left);
			}
			y += cyChar;
		}
		nIndex = _AfxNextLine(lpszText, nIndexStop, nIndexEnd);
	}
	while (nIndex < nIndexStop && y+cyChar <= rectLayout.bottom);

	pDC->RestoreDC(-1);
	ASSERT_VALID(this);

	rectLayout.bottom = y;
	return nIndex;
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


IMPLEMENT_DYNCREATE(DSPrintTextView, DSPrintEditView)

 
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


