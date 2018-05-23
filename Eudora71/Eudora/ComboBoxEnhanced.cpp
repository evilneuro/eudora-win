//	ComboBoxEnhanced.cpp
//
//	Provides enhanced CComboBoxEx.
//
//	Copyright (c) 2003 by QUALCOMM, Incorporated
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



#include "stdafx.h"
#include "ComboBoxEnhanced.h"
#include "font.h"
#include "rs.h"

#include "DebugNewHelpers.h"


//	Message maps
BEGIN_MESSAGE_MAP(CListBoxEncapsulated, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CComboBoxEnhanced, CComboBoxEx)

BEGIN_MESSAGE_MAP(CComboBoxEnhanced, CComboBoxEx)
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnListBoxCtlColor)
	ON_CONTROL_REFLECT_EX(CBN_SELCHANGE, OnComboBoxItemSelected)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


//	Constants
const CComboBoxEnhanced::kDefaultPadding = 3;
const CComboBoxEnhanced::kDroppedPadding = 10;
const CComboBoxEnhanced::kIndentPadding = 10;


// ------------------------------------------------------------------------------------------
//		* CListBoxEncapsulated														[Public]
// ------------------------------------------------------------------------------------------
//	Construct CListBoxEncapsulated.

CListBoxEncapsulated::CListBoxEncapsulated(
	CComboBoxEnhanced *			in_pParent)
	:	m_pParent(in_pParent)
{
}


// ------------------------------------------------------------------------------------------
//		* ~CListBoxEncapsulated														[Public]
// ------------------------------------------------------------------------------------------
//	Destruct CListBoxEncapsulated.

CListBoxEncapsulated::~CListBoxEncapsulated()
{
}


// ------------------------------------------------------------------------------------------
//		* PointIsInDisabledItem													 [Protected]
// ------------------------------------------------------------------------------------------
//	Determines whether or not a given point (in client coordinates) is currently inside
//	a disabled item.
//
//	Parameters:
//		in_point:		Location to check
//
//	Returns:
//		Whether or not the indicated point is inside a disabled item

bool
CListBoxEncapsulated::PointIsInDisabledItem(CPoint in_point)
{
	bool		bPointIsInDisabledItem = false;

	//	First check to see if the point is even inside us
	CRect		rectClient;
	GetClientRect(rectClient);

	if ( rectClient.PtInRect(in_point) )
	{
		//	The point is inside us - see which item it's in
		BOOL	bOutside = FALSE;
		int		nHitIndex = reinterpret_cast<CListBox *>(this)->ItemFromPoint(in_point, bOutside);

		//	Determine if the point is inside a disabled item
		bPointIsInDisabledItem = ( !bOutside && !m_pParent->IsItemEnabled(nHitIndex) );
	}
	
	return bPointIsInDisabledItem;
}


// ------------------------------------------------------------------------------------------
//		* OnLButtonDown															 [Protected]
// ------------------------------------------------------------------------------------------
//	Override of left mouse button down handling to reject any handling if the point
//	is inside a disabled item.
//
//	Parameters:
//		nFlags:		Not used
//		point:		Location of mouse click

void
CListBoxEncapsulated::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( !PointIsInDisabledItem(point) )
		CWnd::OnLButtonDown(nFlags, point);
}


// ------------------------------------------------------------------------------------------
//		* OnLButtonUp															 [Protected]
// ------------------------------------------------------------------------------------------
//	Override of left mouse button up handling to reject any handling if the point
//	is inside a disabled item.
//
//	Parameters:
//		nFlags:		Not used
//		point:		Location of mouse click

void
CListBoxEncapsulated::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( !PointIsInDisabledItem(point) )
		CWnd::OnLButtonUp(nFlags, point);
}


// ------------------------------------------------------------------------------------------
//		* OnLButtonDblClk														 [Protected]
// ------------------------------------------------------------------------------------------
//	Override of left mouse double click handling to reject any handling if the point
//	is inside a disabled item.
//
//	Parameters:
//		nFlags:		Not used
//		point:		Location of mouse double click

void
CListBoxEncapsulated::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ( !PointIsInDisabledItem(point) )
		CWnd::OnLButtonDblClk(nFlags, point);
}


// ------------------------------------------------------------------------------------------
//		* CComboBoxEnhanced															[Public]
// ------------------------------------------------------------------------------------------
//	Construct CComboBoxEnhanced.

CComboBoxEnhanced::CComboBoxEnhanced()
	:	m_ListBoxEncapsulated(this), m_nLastSelectedItem(0), m_nMaxWidth(0)
{
}


// ------------------------------------------------------------------------------------------
//		* ~CComboBoxEnhanced														[Public]
// ------------------------------------------------------------------------------------------
//	Destruct CComboBoxEnhanced.

CComboBoxEnhanced::~CComboBoxEnhanced()
{
}


// ------------------------------------------------------------------------------------------
//		* ResetContent																[Public]
// ------------------------------------------------------------------------------------------
//	Resets the content and resets the max width back to 0 so that when items are next
//	inserted we size the dropped width appropriately.

void
CComboBoxEnhanced::ResetContent()
{
	ResetMaxWidth();

	CComboBoxEx::ResetContent();
}


// ------------------------------------------------------------------------------------------
//		* InsertItem																[Public]
// ------------------------------------------------------------------------------------------
//	Provide easier way to insert an item into a CComboBoxEx.
//
//	Parameters:
//		in_nStringResID:	Resource ID of menu item string
//		in_nItemID:			ID/location for item to add
//		in_bIsDisabled:		Whether or not menu item should be disabled (default false)
//		in_bIsDividerLine:	Whether or not menu item is a divider line (default false)

void
CComboBoxEnhanced::InsertItem(
	UINT				in_nStringResID,
	int					in_nItemID,
	bool				in_bIsDisabled,
	bool				in_bIsDividerLine)
{
	CRString			szItemText(in_nStringResID);

	InsertItem( szItemText, in_nItemID, szItemText.GetLength() );
}


// ------------------------------------------------------------------------------------------
//		* InsertItem																[Public]
// ------------------------------------------------------------------------------------------
//	Provide easier way to insert an item into a CComboBoxEx.
//
//	Parameters:
//		in_szString:		Pointer to string data (must be NULL terminated if
//							in_nStringLength is -1
//		in_nItemID:			ID/location for item to add
//		in_nStringLength:	String length or -1 to calculate string length
//		in_bIsDisabled:		Whether or not menu item should be disabled (default false)
//		in_bIsDividerLine:	Whether or not menu item is a divider line (default false)

void
CComboBoxEnhanced::InsertItem(
	const char *		in_szString,
	int					in_nItemID,
	int					in_nStringLength,
	bool				in_bIsDisabled,
	bool				in_bIsDividerLine)
{
	if (in_nStringLength == -1)
		in_nStringLength = strlen(in_szString);
	
	//	Fill out the item info
	COMBOBOXEXITEM		itemInfo;

	memset( &itemInfo, 0, sizeof(itemInfo) );
	itemInfo.mask = CBEIF_TEXT | CBEIF_LPARAM;

	itemInfo.pszText = const_cast<LPTSTR>(in_szString);
	itemInfo.cchTextMax = in_nStringLength;
	itemInfo.iItem = in_nItemID;

	if (in_bIsDisabled)
		itemInfo.lParam |= kDisabled;

	if (in_bIsDividerLine)
		itemInfo.lParam |= kDividerLine;

	//	Calculate the width of the current item (note that this like the rest
	//	of CComboBoxEnhanced does not handle icons)
	CFont *		pFont = GetFont();
	int			nCurrentItemWidth = 2 * kDefaultPadding + TextWidth(pFont, in_szString) + kDroppedPadding;

	if (nCurrentItemWidth > m_nMaxWidth)
	{
		//	We have a new maximum width item - remember it and set the dropped width
		//	so that it will fit.
		m_nMaxWidth = nCurrentItemWidth;

		SetDroppedWidth(m_nMaxWidth);
	}

	int			nInsertItemResult = CComboBoxEx::InsertItem(&itemInfo);
	ASSERT(nInsertItemResult >= 0);
}

// ------------------------------------------------------------------------------------------
//		* IsItemEnabled																[Public]
// ------------------------------------------------------------------------------------------
//	Determines if indicated item is enabled or not.
//
//	Parameters:
//		itemInfo:		Information about item - must contain lParam information
//
//	Returns:
//		Whether or not the indicated item is enabled

bool
CComboBoxEnhanced::IsItemEnabled(
	COMBOBOXEXITEM &	itemInfo)
{
	return ( (itemInfo.lParam & (kDisabled | kDividerLine)) == 0 );
}


// ------------------------------------------------------------------------------------------
//		* IsItemEnabled																[Public]
// ------------------------------------------------------------------------------------------
//	Determines if indicated item is enabled or not.
//
//	Parameters:
//		in_nItemIndex:		Index of the item to check
//
//	Returns:
//		Whether or not the indicated item is enabled

bool
CComboBoxEnhanced::IsItemEnabled(
	int					in_nItemIndex)
{
	COMBOBOXEXITEM		itemInfo;

	memset( &itemInfo, 0, sizeof(itemInfo) );
	itemInfo.mask = CBEIF_LPARAM;
	itemInfo.iItem = in_nItemIndex;

	return ( GetItem(&itemInfo) && IsItemEnabled(itemInfo) );
}


// ------------------------------------------------------------------------------------------
//		* OnDrawItem															 [Protected]
// ------------------------------------------------------------------------------------------
//	Override of drawing.
//
//	Parameters:
//		nIDCtl:				ID of the control to draw
//		lpDrawItemStruct:	Information about the list item currently being drawn

void
CComboBoxEnhanced::OnDrawItem(
	int						nIDCtl,
	LPDRAWITEMSTRUCT		lpDrawItemStruct)
{
	if (!lpDrawItemStruct)
	{
		ASSERT(0);
		return;
	}
	
	CDC *				pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CString				szText;
	COMBOBOXEXITEM		itemInfo;

	ASSERT(pDC);

	memset( &itemInfo, 0, sizeof(itemInfo) );
	itemInfo.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_INDENT;
	itemInfo.iItem = lpDrawItemStruct->itemID;

	//	We currently support drawing menu items with 256 or less characters
	itemInfo.pszText = szText.GetBuffer(256);
	itemInfo.cchTextMax = 256;

	if ( pDC && GetItem(&itemInfo) )
	{
		szText.ReleaseBuffer();
		
		if (itemInfo.lParam & kDividerLine)
		{
			//	Draw divider line
			COLORREF	crShadow = ::GetSysColor(COLOR_3DSHADOW);
			
			CBrush		shadowBrush(crShadow);
			CRect		rectDivider = lpDrawItemStruct->rcItem;
			
			rectDivider.top = (rectDivider.top + rectDivider.bottom) / 2;
			rectDivider.bottom = rectDivider.top+1;
			
			pDC->FillRect(&rectDivider, &shadowBrush);
		}
		else
		{
			CRect		rectItem = lpDrawItemStruct->rcItem;
			
			//	Set up some color values
			COLORREF	crBackColor, crTextColor;

			bool		bIsItemEnabled = IsItemEnabled(itemInfo);
			
			if ( (lpDrawItemStruct->itemState & ODS_SELECTED) && GetDroppedState() && bIsItemEnabled )
			{
				crBackColor = ::GetSysColor(COLOR_HIGHLIGHT);
				crTextColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			}
			else
			{
				crBackColor = ::GetSysColor(COLOR_WINDOW);
				crTextColor = ::GetSysColor( bIsItemEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT );
			}
			
			//	Draw background
			CBrush		backBrush(crBackColor);
			pDC->FillRect(&rectItem, &backBrush);
			
			if ( !(lpDrawItemStruct->itemState & ODS_COMBOBOXEDIT) )
				rectItem.left += kDefaultPadding + itemInfo.iIndent * kIndentPadding;
			
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(crTextColor);
			pDC->SetTextAlign(TA_LEFT | TA_TOP);
			
			//	Center the item vertically
			int			nRectHeight = rectItem.bottom - rectItem.top;
			CSize		textSize = pDC->GetTextExtent( szText, szText.GetLength() );
			int			nHeightDiff = nRectHeight - textSize.cy;

			if (nHeightDiff > 0)
				nHeightDiff++;
			else if (nHeightDiff < 0)
				nHeightDiff--;
			
			rectItem.top += nHeightDiff / 2;

			//	Check new rect height vs. text height
			nRectHeight = rectItem.bottom - rectItem.top;

			if (nRectHeight < textSize.cy)
			{
				//	We're dealing with an extra small combo box.
				//	Adjust the bottom down to handle the actual text height.
				rectItem.bottom = rectItem.top + textSize.cy;

				//	Redo the clip region
				CRgn		rgnClip;
				
				rgnClip.CreateRectRgnIndirect(&rectItem);	
				pDC->SelectClipRgn(&rgnClip);
				rgnClip.DeleteObject();
			}
			
			//	Draw the text
			pDC->ExtTextOut(rectItem.left, rectItem.top, ETO_CLIPPED, &rectItem, szText, szText.GetLength(), 0);
		}
	}
}


// ------------------------------------------------------------------------------------------
//		* OnComboBoxItemSelected												 [Protected]
// ------------------------------------------------------------------------------------------
//	Override of combo box item selection that avoids the selection of disabled items
//	while allowing other item selection handling to still occur.

BOOL
CComboBoxEnhanced::OnComboBoxItemSelected()
{
	int		nSelectedItem = GetCurSel();

	if ( !IsItemEnabled(nSelectedItem) )
	{
		//	Don't allow the selection of disabled items - revert back to the last selection
		SetCurSel(m_nLastSelectedItem);
	}
	else
	{
		//	The item is enabled - remeber it so that we can revert to it in the
		//	future if necessary.
		m_nLastSelectedItem = nSelectedItem;
	}

	//	Return FALSE so that other handling of item selection still occurs
	return FALSE;
}


// ------------------------------------------------------------------------------------------
//		* OnListBoxCtlColor														 [Protected]
// ------------------------------------------------------------------------------------------
//	Handle on list box control color so that we can grab and subclass the listbox, which
//	allows us to correctly support disabled items so that the don't react to clicking, etc.
//
//	Parameters:
//		WPARAM:			Ignored
//		lParam:			HWND of the list box

LRESULT
CComboBoxEnhanced::OnListBoxCtlColor(WPARAM, LPARAM lParam)
{
	if ( (m_ListBoxEncapsulated.m_hWnd == NULL) && (lParam != 0) &&
		 (lParam != reinterpret_cast<LPARAM>(m_hWnd)) )
	{
		m_ListBoxEncapsulated.SubclassWindow((HWND)lParam);
	}

	return Default();
}