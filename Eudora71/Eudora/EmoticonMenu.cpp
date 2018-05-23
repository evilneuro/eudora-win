// EmoticonMenu.cpp : implementation file

#include "stdafx.h"
#include "resource.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "EmoticonMenu.h"
#include "EmoticonManager.h"
#include "rs.h"
#include "fileutil.h"
#include "QCGraphics.h"


#include "DebugNewHelpers.h"

extern CString EudoraDir;
extern EmoticonDirector g_theEmoticonDirector;
extern QCCommandStack	g_theCommandStack;


const short EmoticonMenu::kTextPadding = 10;
unsigned int EmoticonMenu::m_maxImageWidth;
unsigned int EmoticonMenu::m_maxImageHeight;
unsigned int EmoticonMenu::m_xPosMeaning;

/////////////////////////////////////////////////////////////////////////////
// EmoticonMenu


EmoticonMenu::EmoticonMenu()
{

	VERIFY(CreatePopupMenu());
	
	ASSERT(GetMenuItemCount()==0);

	const COMMAND_ACTION_TYPE theAction = CA_EMOTICON;

	const EmoticonList *pEmoticonList = g_theEmoticonDirector.GetEmoticonList();

	m_maxImageWidth = 0;
	m_maxImageHeight = 0;
	m_xPosMeaning = 0;

	CWindowDC	screenDC(NULL);

	for(POSITION pos=pEmoticonList->GetHeadPosition(); pos!=NULL; ) {
		Emoticon *		pEmoticon = pEmoticonList->GetNext(pos);

		// Only load images and put them in the menu, if the emoticon
		// is not a synonym.
		if ( !pEmoticon->IsSynonym() ) {
			// Attempt to load the image that we'll put in the menu
			SECImage *	pTempEmoticonImage = LoadImage( pEmoticon->GetImageFullPathForMenu() );

			if (pTempEmoticonImage) {
				// Convert the image into a bitmap
				CBitmap *	pTempEmoticonBitmap = pTempEmoticonImage->MakeBitmap(&screenDC);

				if (pTempEmoticonBitmap) {
					// Create the image list. We won't actually be keeping an actual *list*
					// of emoticon images (i.e. they'll only be one emoticon image).
					// CImageList just happens to contain some mask creation and transparent
					// image drawing functionality. We could do transparent drawing other ways,
					// but this way is nice and easy.
					CImageList *	pEmoticonImage = DEBUG_NEW_MFCOBJ_NOTHROW CImageList();

					if (pEmoticonImage) {
						// Get the image width and height
						DWORD			nImageWidth = pTempEmoticonImage->dwGetWidth();
						DWORD			nImageHeight = pTempEmoticonImage->dwGetHeight();

						// Finish construction:
						// * We use ILC_COLOR24 so that we can handle emoticons with lots of colors.
						// * We use ILC_MASK so that the CImageList will create a mask for each image
						//   we add (again in our case there will only be one image)
						// * The last two parameters 1, 1 - specify an initial and grow size of one
						if ( pEmoticonImage->Create(nImageWidth, nImageHeight, ILC_COLOR24 | ILC_MASK, 1, 1) ) {
							// Add our emoticon bitmap with the correct transparent color
							pEmoticonImage->Add( pTempEmoticonBitmap, pTempEmoticonImage->GetTransparentColor() );

							// Set the background color of the image to none so that it will always be
							// drawn transparently (even if drawn with ILD_NORMAL).
							pEmoticonImage->SetBkColor(CLR_NONE);

							// Stash the image we've created into the emoticon object.
							pEmoticon->SetImage(pEmoticonImage);
							
							// Stash the image dimensions into the emoticon object (it seemed easier
							// than trying to figure them out from the CImageList later).
							pEmoticon->SetImageWidth(nImageWidth);
							pEmoticon->SetImageHeight(nImageHeight);
							
							// Add the command
							WORD	wID = g_theCommandStack.AddCommand(pEmoticon, theAction);

							if (wID) {
								// Add to the menu
								VERIFY( AppendMenu(MF_OWNERDRAW, wID, (LPCTSTR)(wID)) );

								// Associate the command ID with the emoticon
								pEmoticon->SetID(wID);
								g_theEmoticonDirector.SetEmoticonIDMap(wID, pEmoticon);
							}
						}
						else {
							// Create failed - delete the CImageList
							delete pEmoticonImage;
						}
					}

					// We're done with the temporary bitmap - delete it
					delete pTempEmoticonBitmap;
				}

				// We're done with the temporary SECImage - delete it
				delete pTempEmoticonImage;
			}
		}
	}
}


void EmoticonMenu::CalculateMaxMenuDimensions(CDC &dc)
{

	const EmoticonList *pEmoticonList;

	pEmoticonList = g_theEmoticonDirector.GetEmoticonList();
	for(POSITION pos=pEmoticonList->GetHeadPosition(); pos!=NULL; ) {
		Emoticon* emoticonObj = pEmoticonList->GetNext(pos);

		if (!emoticonObj) {
			ASSERT(0);
			return;
		}	

		//get the maximum width of the image and trigger
		CString strTrigger = emoticonObj->GetTrigger();
		CSize triggerSize = dc.GetTextExtent(strTrigger, strTrigger.GetLength());	
		unsigned int currXPosMeaning = emoticonObj->GetImageWidth() + triggerSize.cx + kTextPadding;
		if(currXPosMeaning > m_xPosMeaning)
			m_xPosMeaning = currXPosMeaning;
	}

	pEmoticonList = g_theEmoticonDirector.GetEmoticonList();
	for(POSITION pos=pEmoticonList->GetHeadPosition(); pos!=NULL; ) {
		Emoticon* emoticonObj = pEmoticonList->GetNext(pos);

		if (!emoticonObj) {
			ASSERT(0);
			return;
		}	

		//get the maximum width and height of the entire line
		CString strMeaning = emoticonObj->GetMeaning();
		CSize meaningSize = dc.GetTextExtent(strMeaning, strMeaning.GetLength());	
		unsigned int currEmoticonMenuWidth = m_xPosMeaning + meaningSize.cx;
		unsigned int currEmoticonMenuHeight = emoticonObj->GetImageHeight();
		if(currEmoticonMenuWidth > m_maxImageWidth)
			m_maxImageWidth = currEmoticonMenuWidth;
		if(currEmoticonMenuHeight > m_maxImageHeight)
			m_maxImageHeight = currEmoticonMenuHeight;
	}
}

void EmoticonMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	DoDrawItem( lpDIS );
}


void EmoticonMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	DoMeasureItem( lpMIS );
}


void EmoticonMenu::DoDrawItem( LPDRAWITEMSTRUCT lpDIS )
{

	ASSERT(lpDIS->CtlType == ODT_MENU);

	UINT id = (UINT)(WORD)lpDIS->itemID;
	ASSERT(id == lpDIS->itemData);
	
	CDC dc;
	dc.Attach(lpDIS->hDC);

	CRect rc(lpDIS->rcItem);

	if (lpDIS->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&rc);

	COLORREF cr = (lpDIS->itemState & ODS_SELECTED) ? 
		::GetSysColor(COLOR_HIGHLIGHT) :
		dc.GetBkColor();

	CBrush brushFill(cr);
	cr = dc.GetTextColor();

	if (lpDIS->itemState & ODS_SELECTED)
	{
		dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else if ( lpDIS->itemState & ODS_GRAYED )
	{
		// set the text color to gray
		dc.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	}

	int nBkMode = dc.SetBkMode(TRANSPARENT);
	dc.FillRect(&rc, &brushFill);

	//	We might prefer HALFTONE, but some msdn docs say that it only works on NT.
	//	COLORONCOLOR seems to work well enough.
	dc.SetStretchBltMode(COLORONCOLOR);

	// Stretch to the same size (essentially BitBlt()).

	//get the image and draw the same
	Emoticon *		pEmoticon = NULL;
	g_theEmoticonDirector.LookUpEmoticonIDMap((WORD) id, pEmoticon);

	if (!pEmoticon) {
		ASSERT(0);
		return;
	}

	CImageList *	pEmoticonImage = pEmoticon->GetImage();

	if	(!pEmoticonImage) {
		ASSERT(0);
		return;
	}

	pEmoticonImage->Draw(&dc, 0, CPoint(rc.left+2, rc.top+1), ILD_TRANSPARENT);

	rc.left += pEmoticon->GetImageWidth() + kTextPadding;
	CString		strTrigger = pEmoticon->GetTrigger();
	dc.TextOut(rc.left, rc.top, strTrigger, strTrigger.GetLength());		
	
	rc.left = lpDIS->rcItem.left + m_xPosMeaning + kTextPadding;
	CString		strMeaning = pEmoticon->GetMeaning();
	dc.TextOut(rc.left, rc.top, strMeaning, strMeaning.GetLength());	

	dc.SetTextColor(cr);
	dc.SetBkMode(nBkMode);

	dc.Detach();
}


void EmoticonMenu::DoMeasureItem( LPMEASUREITEMSTRUCT lpMIS )
{
	ASSERT(lpMIS->CtlType == ODT_MENU);
	UINT id = (UINT)(WORD)lpMIS->itemID;
	ASSERT(id == lpMIS->itemData);

	if((m_maxImageWidth == 0) || (m_maxImageHeight == 0))
	{
		CDC dc;
		dc.CreateIC(("DISPLAY"), NULL, NULL, NULL);

		CalculateMaxMenuDimensions(dc);
	}

	lpMIS->itemWidth = m_maxImageWidth + 12; 
	lpMIS->itemHeight = m_maxImageHeight + 2;
}

