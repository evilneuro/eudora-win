//	SearchBar.cpp
//
//	Provides mini-dialog toolbar searching.
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
#include "SearchBar.h"
#include "font.h"
#include "guiutils.h"
#include "mainfrm.h"
#include "msgdoc.h"
#include "resource.h"
#include "rs.h"
#include "summary.h"
#include "tocdoc.h"

#include "DebugNewHelpers.h"


//	Constants
const short			CSearchBar::kSmallToolbarButtonHeight = 16 + 11;
const short			CSearchBar::kLargeToolbarButtonHeight = 32 + 11;
const short			CSearchBar::kSearchBarMinWidth = 100;
const short			CSearchBar::kSearchBarMaxWidth = 500;
const short			CSearchBar::kSearchComboHeight = 400;
const short			CSearchBar::kSearchComboPaddingFromGripper = 2;
const short			CSearchBar::kSearchComboPaddingFromEdgeDocked = 5;
const short			CSearchBar::kSearchComboPaddingFromEdgeFloating = 10;


//	Message maps
BEGIN_MESSAGE_MAP(CSearchBarComboBoxEdit, CEdit)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(CSearchBarComboBox, CComboBoxEnhanced)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


IMPLEMENT_DYNCREATE(CSearchBar, SECCustomToolBar)

BEGIN_MESSAGE_MAP(CSearchBar, SECCustomToolBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(IDC_SEARCH_BAR_COMBO, UpdateSearchBarCombo)
	ON_CBN_SELCHANGE(IDC_SEARCH_BAR_COMBO, OnComboBoxItemSelected)
	ON_CBN_DROPDOWN(IDC_SEARCH_BAR_COMBO, OnComboBoxDropDown)
END_MESSAGE_MAP()


// ------------------------------------------------------------------------------------------
//		* CSearchBarComboBoxEdit													[Public]
// ------------------------------------------------------------------------------------------
//	Construct CSearchBarComboBoxEdit.
//
//	Parameters:
//		in_pSearchBar:	Pointer to search bar that owns this object

CSearchBarComboBoxEdit::CSearchBarComboBoxEdit(
	CSearchBar *		in_pSearchBar)
	:	m_pSearchBar(in_pSearchBar), m_bAllowSetText(false), m_bGetActualText(false),
		m_bIsEmpty(true)
{
}


// ------------------------------------------------------------------------------------------
//		* ~CSearchBarComboBoxEdit													[Public]
// ------------------------------------------------------------------------------------------
//	Destruct CSearchBarComboBoxEdit.

CSearchBarComboBoxEdit::~CSearchBarComboBoxEdit()
{
}


// ------------------------------------------------------------------------------------------
//		* PreTranslateMessage														[Public]
// ------------------------------------------------------------------------------------------
//	Override pretranslation of messages so that we can correctly handle the enter key.
//
//	Parameters:
//		pMsg:		Pointer to information about message
//
//	Returns:
//		Whether or not event was handled.

BOOL
CSearchBarComboBoxEdit::PreTranslateMessage(
	MSG *				pMsg)
{
	BOOL	bEventHandled = FALSE;

	if (pMsg->hwnd == m_hWnd)
	{
		if ( (pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP) )
		{
			switch(pMsg->wParam)
			{
				case VK_RETURN:
					if (pMsg->message == WM_KEYDOWN)
					{
						//	Just return that we handled the return keydown event so that
						//	no one else does.
						bEventHandled = TRUE;
					}
					else	//	(pMsg->message == WM_KEYUP)
					{
						//	Return means that we should search for the current text
						if (m_pSearchBar)
							m_pSearchBar->SearchForCurrentText();
						
						//	We handled the return keyup event.
						bEventHandled = TRUE;
					}
					break;
					
				case VK_UP:
				case VK_DOWN:
					//	Eat up and down arrows to avoid activating a menu change.
					//	(Would initiate a search without any indication of why, because
					//	unlike a normal combo box we don't allow the edit field to change
					//	when the selected item changes).
					bEventHandled = TRUE;
					break;
			}
		}
		
		//	If we didn't handle it do the default processing
		if (!bEventHandled)
			bEventHandled = CEdit::PreTranslateMessage(pMsg);
	}


	return bEventHandled;
}


// ------------------------------------------------------------------------------------------
//		* SetActualText																[Public]
// ------------------------------------------------------------------------------------------
//	Allows caller to set the actual text (normally we ignore setting text so that choosing
//	the combo box menu item doesn't change the text).
//
//	Parameters:
//		in_szText:				Pointer to new text value
//		in_bUpdateEmptyStatus:	Whether or not we should update our records

void
CSearchBarComboBoxEdit::SetActualText(
	const char *			in_szText,
	bool					in_bUpdateEmptyStatus)
{
	m_bAllowSetText = true;
	SetWindowText(in_szText);
	m_bAllowSetText = false;

	if (in_bUpdateEmptyStatus)
		m_bIsEmpty = (*in_szText == '\0');
}


// ------------------------------------------------------------------------------------------
//		* GetActualText																[Public]
// ------------------------------------------------------------------------------------------
//	Allows caller to get the actual text (normally we lie when someone asks for the text
//	so that when the combo box asks for the text what we tell it causes the correct menu
//	item to be selected).
//
//	Parameters:
//		out_szText:				CString in which we return the text

void
CSearchBarComboBoxEdit::GetActualText(
	CString &			out_szText)
{
	m_bGetActualText = true;
	GetWindowText(out_szText);
	m_bGetActualText = false;
}


// ------------------------------------------------------------------------------------------
//		* UpdateGrayTextStatus														[Public]
// ------------------------------------------------------------------------------------------
//	If we've recorded that we're empty, then we update whether we're displaying gray
//	text with the current search type or blank based on whether or not we're focused.

void
CSearchBarComboBoxEdit::UpdateGrayTextStatus()
{
	//	Are we empty?
	if (m_bIsEmpty)
	{
		if (GetFocus() == this)
		{
			//	We're focused so remove any gray text that we previously set
			CString		szText;

			GetActualText(szText);

			//	When setting the text, pass in false so that we don't bother
			//	updating our empty status - it hasn't changed
			if ( !szText.IsEmpty() )
				SetActualText("", false);
		}
		else
		{
			//	We're not focused so fill ourselves with the gray text of
			//	the current search type
			CRString	szSelectedMenuText( m_pSearchBar->GetCurrentSearchMenuStringID() );

			//	Pass in false so that we don't update our empty status - the gray
			//	text doesn't count, we're still officially empty
			SetActualText(szSelectedMenuText, false);
		}
	}
}


// ------------------------------------------------------------------------------------------
//		* OnSetFocus															 [Protected]
// ------------------------------------------------------------------------------------------
//	Override handling of WM_SETFOCUS so that we can update the status of the gray text
//	that we show when we're empty and not focused.
//
//	Parameters:
//		pOldWnd:	Pointer to window that last had focus

void
CSearchBarComboBoxEdit::OnSetFocus(
	CWnd *			pOldWnd )
{
	//	Update the gray text status because we're gaining focus
	UpdateGrayTextStatus();

	//	Let CEdit do its thing
	CEdit::OnSetFocus(pOldWnd);
}


// ------------------------------------------------------------------------------------------
//		* OnKillFocus															 [Protected]
// ------------------------------------------------------------------------------------------
//	Override handling of WM_KILLFOCUS so that we can update the status of the gray text
//	that we show when we're empty and not focused.
//
//	Parameters:
//		pNewWnd:	Pointer to window that just gained focus

void
CSearchBarComboBoxEdit::OnKillFocus(
	CWnd *			pNewWnd)
{
	//	Let CEdit do its thing
	CEdit::OnKillFocus(pNewWnd);

	//	We had focus - check to see if the user typed anything
	CString		szText;
	
	GetActualText(szText);
	
	m_bIsEmpty = (szText.IsEmpty() != FALSE);
	
	//	Update the gray text status because we're losing focus
	UpdateGrayTextStatus();
}


// ------------------------------------------------------------------------------------------
//		* OnSetText																 [Protected]
// ------------------------------------------------------------------------------------------
//	Override handling of WM_SETTEXT messages, which allows us to avoid having the contents
//	of the combo box edit field be blasted when the user chooses one of the combo box items.
//
//	Parameters:
//		wParam:		Not used
//		lParam:		Pointer to a null-terminated string that is the window text
//
//	Returns:
//		Whether or not operation succeeded.

LRESULT
CSearchBarComboBoxEdit::OnSetText(
	WPARAM				wParam,
	LPARAM				lParam)
{
	LRESULT lResult = TRUE;
	
	//	Only allow setting the text if we've been told to.
	if (m_bAllowSetText)
		lResult = DefWindowProc(WM_SETTEXT, wParam, lParam);

	return lResult;
}


// ------------------------------------------------------------------------------------------
//		* OnGetText																 [Protected]
// ------------------------------------------------------------------------------------------
//	Override handling of WM_GETTEXT messages, which allows us to maintain the combo
//	box selection despite the fact that we don't allow the edit field's actual contents
//	to be replaced with the combo box selection.
//
//	Parameters:
//		wParam:		Specifies the maximum number of TCHARs to be copied
//		lParam:		Pointer to the buffer that is to receive the text
//
//	Returns:
//		Length of text copied into lParam.

LRESULT
CSearchBarComboBoxEdit::OnGetText(
	WPARAM				wParam,
	LPARAM				lParam)
{
	LRESULT lResult = 0;
	
	if (m_bGetActualText)
	{
		//	We've been told to tell the truth so return the actual text
		lResult = DefWindowProc(WM_GETTEXT, wParam, lParam);
	}
	else if (lParam && m_pSearchBar)
	{
		//	Combo boxes are curious creatures. It appears to be the case that
		//	CBS_DROPDOWN combo boxes rely on getting the text of the embedded
		//	edit field and matching it to the menu item in order to choose the
		//	selected menu item (in fact they won't remember anything that the
		//	user selected or that was set via SetCurSel unless the edit field
		//	returns that text.
		//
		//	[Side Note: This seems to imply and some testing bears out that if
		//	it's owner drawn it probably needs to store the strings - i.e. must
		//	have CBS_HASSTRINGS set. Hopefully will never be an issue for us here,
		//	because although CComboBoxEx's are implied owner drawn they also
		//	automatically have CBS_HASSTRINGS set.]
		//
		//	Make the combo box happy by returning what it expects to see rather
		//	than what we actually contain.
		CRString	szSelectedMenuText( m_pSearchBar->GetCurrentSearchMenuStringID() );
		
		if ( wParam >= static_cast<UINT>(szSelectedMenuText.GetLength()) )
		{
			lResult = szSelectedMenuText.GetLength();
			strcpy(reinterpret_cast<char *>(lParam), szSelectedMenuText);
		}
	}

	return lResult;
}


// ------------------------------------------------------------------------------------------
//		* OnGetTextLength														 [Protected]
// ------------------------------------------------------------------------------------------
//	Handle WM_GETTEXTLENGTH message.
//
//	Parameters:
//		Not used
//
//	Returns:
//		Text length.

LRESULT
CSearchBarComboBoxEdit::OnGetTextLength(
	WPARAM				wParam,
	LPARAM				lParam)
{
	LRESULT		lResult = 0;
	
	if (m_bGetActualText)
	{
		//	We've been told to tell the truth so return the actual text
		lResult = DefWindowProc(WM_GETTEXTLENGTH, wParam, lParam);
	}
	else if (m_pSearchBar)
	{
		//	Make the combo box happy by returning what the lenght of what it expects
		//	to see rather than what we actually contain.
		//	See CSearchBarComboBoxEdit::OnGetText for more details.
		CRString	szSelectedMenuText( m_pSearchBar->GetCurrentSearchMenuStringID() );
			
		lResult = szSelectedMenuText.GetLength();
	}

	return lResult;
}


// ------------------------------------------------------------------------------------------
//		* CSearchBarComboBox														[Public]
// ------------------------------------------------------------------------------------------
//	Construct CSearchBarComboBox.
//
//	Parameters:
//		in_pSearchBar:	Pointer to search bar that owns this object

CSearchBarComboBox::CSearchBarComboBox(
	CSearchBarComboBoxEdit *		in_pSearchBarComboBoxEdit)
	:	m_pSearchBarComboBoxEdit(in_pSearchBarComboBoxEdit)
{
}


// ------------------------------------------------------------------------------------------
//		* ~CSearchBarComboBox														[Public]
// ------------------------------------------------------------------------------------------
//	Destruct CSearchBarComboBox.

CSearchBarComboBox::~CSearchBarComboBox()
{
}

// ------------------------------------------------------------------------------------------
//		* OnCtlColor															 [Protected]
// ------------------------------------------------------------------------------------------
//	Override WM_CTLCOLOR handling so that we can make the edit field text gray
//	when it's "officially" empty (i.e. filled with current search type).
//
//	Parameters
//		pDC:		Pointer to device context for which to set colors
//		pWnd:		Pointer to window for which we're being called
//		nCtlColor:	Control type - we don't bother using this
//
//	Returns:
//		A handle to the brush that is to be used for painting the control background

HBRUSH
CSearchBarComboBox::OnCtlColor(
	CDC *			pDC,
	CWnd *			pWnd,
	UINT			nCtlColor)
{
	//	We want the text to be gray if:
	//	* The search bar combo box is what we're being asked about
	//	* The search bar combo box is NOT focused
	//	* The search bar combo box is "officially" empty
	if ( m_pSearchBarComboBoxEdit && (pWnd == m_pSearchBarComboBoxEdit) &&
		 (GetFocus() != m_pSearchBarComboBoxEdit) && m_pSearchBarComboBoxEdit->IsEmpty() )
	{
		pDC->SetTextColor( GetSysColor(COLOR_GRAYTEXT) );

		return (GetSysColorBrush(COLOR_WINDOW));
	}

	//	If we made it this far, just return with the default colors
	return CComboBoxEnhanced::OnCtlColor(pDC, pWnd, nCtlColor);
}


// ------------------------------------------------------------------------------------------
//		* RecentSearch																[Public]
// ------------------------------------------------------------------------------------------
//	Default constructor for RecentSearch (keeps list class happy).

CSearchBar::RecentSearch::RecentSearch()
	:	m_nSearchType(kSearchWeb), m_szSearchText()
{
}


// ------------------------------------------------------------------------------------------
//		* RecentSearch																[Public]
// ------------------------------------------------------------------------------------------
//	Construct RecentSearch given value parameters.
//
//	Parameters:
//		in_nSearchType:		Type of search to remember
//		in_szSeachText:		Text of search to remember

CSearchBar::RecentSearch::RecentSearch(
	short				in_nSearchType,
	const char *		in_szSeachText)
	:	m_nSearchType(in_nSearchType), m_szSearchText(in_szSeachText)
{
}


// ------------------------------------------------------------------------------------------
//		* ~RecentSearch																[Public]
// ------------------------------------------------------------------------------------------
//	Destruct RecentSearch.

CSearchBar::RecentSearch::~RecentSearch()
{
}


// ------------------------------------------------------------------------------------------
//		* CSearchBar																[Public]
// ------------------------------------------------------------------------------------------
//	Construct CSearchBar.

CSearchBar::CSearchBar()
	:	SECCustomToolBar(), m_nCurrentSearch(kSearchWeb),
		m_nSearchBarWidth(kSearchBarMinWidth), m_nEditTextMinHeight(10),
		m_nToolbarHeight(kLargeToolbarButtonHeight), m_bWasFloating(FALSE),
		m_dwExOldStyle(m_dwExStyle), m_SearchCombo(&m_ComboEdit), m_ComboEdit(this)
{
	m_nCurrentSearch = GetIniShort(IDS_INI_SEARCH_BAR_SEARCH_TYPE);
	m_nSearchBarWidth = GetIniShort(IDS_INI_SEARCH_BAR_WIDTH);
}


// ------------------------------------------------------------------------------------------
//		* ~CSearchBar																[Public]
// ------------------------------------------------------------------------------------------
//	Destruct CSearchBar.

CSearchBar::~CSearchBar()
{
}


// ------------------------------------------------------------------------------------------
//		* Create																	[Public]
// ------------------------------------------------------------------------------------------
//	Create CSearchBar correctly setting up size, font, menu items, etc.
//
//	Parameters:
//		in_pParentWnd:		Parent window (probably dock bar)
//		in_dwAlignStyle:	Align style
//
//	Returns:
//		Whether or not creation succeeded.

BOOL
CSearchBar::Create(
	CWnd *				in_pParentWnd,
	DWORD				in_dwAlignStyle)
{
	//	Set up our styles appropriately.
	//	Yes, we're a little schizophrenic with both CBRS_SIZE_FIXED and
	//	CBRS_SIZE_DYNAMIC. I'm not actually sure CBRS_SIZE_FIXED is still necessary,
	//	but CBRS_SIZE_DYNAMIC definitely is in order for our floating size to be
	//	consistently different than our horizontal docked size (necessary when using
	//	small toolbar buttons).
	DWORD	dwStyle = in_dwAlignStyle | CBRS_SIZE_FIXED | CBRS_SIZE_DYNAMIC | CBRS_BORDER_3D | WS_VISIBLE;
	DWORD	dwExStyle = CBRS_EX_STDCONTEXTMENU | CBRS_EX_UNIDIRECTIONAL;
	
	//	Only set the cool styles if the setting is on
	if ( GetIniShort(IDS_INI_SHOW_COOLBAR) )
		dwExStyle |= CBRS_EX_COOLBORDERS | CBRS_EX_GRIPPER;
	
	m_cyTopBorder    = 1;
	m_cyBottomBorder = 1;

	//	Call our parent's Create
	if ( !SECControlBar::Create( NULL, _T(CRString(IDS_SEARCH_BAR_TITLE)), IDC_SEARCH_BAR,
								 dwStyle & CBRS_ALL, dwExStyle, CRect(0,0,0,0), in_pParentWnd, NULL) )
	{
		return FALSE;
	}

	//	Set all our non control bar styles
	const DWORD dwNonControlBarStyle = dwStyle & ~CBRS_ALL;
	if (dwNonControlBarStyle)
		VERIFY( ModifyStyle(0, dwNonControlBarStyle) );

	//	Remember what last extended styles we saw so that we can reposition
	//	our combo box if necessary
	m_dwExOldStyle = m_dwExStyle;

	//	Calculate initial sizes
	RefreshSettings();

	m_szFloat.cy = kLargeToolbarButtonHeight;
	m_szDockHorz.cy = m_nToolbarHeight;
	m_szDockVert.cy = kLargeToolbarButtonHeight;

	m_szFloat.cx = m_nSearchBarWidth;
	m_szDockHorz.cx = m_nSearchBarWidth;
	m_szDockVert.cx = m_nSearchBarWidth;

	//	Get the stock variable-withd font and corresponding text metrics
	//	that we'll use for the combo box
	CFont			ansiFont;
	TEXTMETRIC		tm;

	ansiFont.CreateStockObject(ANSI_VAR_FONT);
	::GetTextMetrics(&ansiFont, tm);
	
	//	Calculate the item height
	m_nEditTextMinHeight = tm.tmHeight + tm.tmExternalLeading;
	int					nMenuItemHeight = m_nEditTextMinHeight + 2;

	//	Figure out the initial orientation type
	OrientationType		eOrientationType = kDockedHorizontally;

	if (in_dwAlignStyle & CBRS_FLOATING)
		eOrientationType = kFloating;
	else if ( (in_dwAlignStyle & CBRS_ALIGN_LEFT) || (in_dwAlignStyle & CBRS_ALIGN_RIGHT) )
		eOrientationType = kDockedVertically;
	else if ( (in_dwAlignStyle & CBRS_ALIGN_TOP) || (in_dwAlignStyle & CBRS_ALIGN_BOTTOM) )
		eOrientationType = kDockedHorizontally;

	//	Calculate the rectangle
	CRect		rectCombo;
	CalculateComboBoxRect(eOrientationType, m_nEditTextMinHeight, rectCombo);

	DWORD		dwComboBoxStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP |
								  CBS_AUTOHSCROLL | CBS_DROPDOWN;
	
	if ( m_SearchCombo.Create(dwComboBoxStyle, rectCombo, this, IDC_SEARCH_BAR_COMBO) )
	{
		//	Setup font and item heights for combo box.
		m_SearchCombo.SetFont(&ansiFont, FALSE);
		m_SearchCombo.SetItemHeight(-1, m_nEditTextMinHeight);
		m_SearchCombo.SetItemHeight(0, nMenuItemHeight);

		//	Make the combo box's edit control be MFC based so that we can
		//	conveniently override portions of its operation.
		HWND		hwndComboBoxEx = m_SearchCombo.GetSafeHwnd();
		ASSERT(hwndComboBoxEx);

		if (hwndComboBoxEx)
		{
			//	Since our combo box is CComboBoxEx based we can get the edit
			//	control by sending the CBEM_GETEDITCONTROL message (with
			//	plain CComboBoxes you can do this via ChildWindowFromPoint,
			//	but that doesn't work as easily with CComboBoxEx - besides
			//	CBEM_GETEDITCONTROL is a better technique anyway).
			HWND		hwndEdit = reinterpret_cast<HWND>( ::SendMessage(hwndComboBoxEx, CBEM_GETEDITCONTROL, 0, 0) );
			ASSERT(hwndEdit);

			if (hwndEdit)
				m_ComboEdit.SubclassWindow(hwndEdit);
		}

		//	Read in the recent item list
		ReadRecentList();

		//	Fill in the combo box for the first time
		UpdateSearchComboItems();

		//	Initial update of the gray text - we're likely not focused
		m_ComboEdit.UpdateGrayTextStatus();
	}

	return TRUE;
}


// ------------------------------------------------------------------------------------------
//		* GetMaxNumberRecentItems													[Public]
// ------------------------------------------------------------------------------------------
//	Gets the maximum number of recent items allowed setting, while sanity checking it.
//
//	Returns:
//		The maximum number of recent items allowed.

short
CSearchBar::GetMaxNumberRecentItems()
{
	short		nMaxNumberRecentListItems = GetIniShort(IDS_INI_SEARCH_BAR_RECENT_COUNT);

	if (nMaxNumberRecentListItems < 0)
		nMaxNumberRecentListItems = 0;

	if (nMaxNumberRecentListItems > 99)
		nMaxNumberRecentListItems = 99;

	return nMaxNumberRecentListItems;
}


// ------------------------------------------------------------------------------------------
//		* ReadRecentList															[Public]
// ------------------------------------------------------------------------------------------
//	Reads the current recent items list from the INI file.

void
CSearchBar::ReadRecentList()
{
	//	Read in the current recent items list from the INI file
	CWinApp *	pApp = AfxGetApp();
	CRString	szRecentListINISection(IDS_SEARCH_RECENT_LIST_INI_SECTION);
	CString		szSearchSetting;
	int			nSettingsNumber = 0;
	int			nMaxNumberRecentItems = GetMaxNumberRecentItems();
	short		nSearchType;

	//	Erase any previous items in the list
	m_recentSearchList.erase( m_recentSearchList.begin(), m_recentSearchList.end() );

	while (nSettingsNumber < nMaxNumberRecentItems)
	{
		//	Read the search type setting for the current index 
		szSearchSetting.Format(IDS_SEARCH_RECENT_TYPE_INI_FORMAT, nSettingsNumber);
		nSearchType = static_cast<short>( pApp->GetProfileInt(szRecentListINISection, szSearchSetting, kSearchInvalid) );

		//	Sanity check the value (allows for newer versions of Eudora to add new values - we'll
		//	just ignore them).
		if ( (nSearchType >= kSearchWeb) && (nSearchType <= kSearchMailfolder) )
		{
			//	Read the search text for the current index
			szSearchSetting.Format(IDS_SEARCH_RECENT_TEXT_INI_FORMAT, nSettingsNumber);
			CString		szSearchText = pApp->GetProfileString(szRecentListINISection, szSearchSetting);
			ASSERT( !szSearchText.IsEmpty() );

			//	If everything went ok - add the recent search item to the end of the list
			if ( !szSearchText.IsEmpty() )
				m_recentSearchList.push_back( RecentSearch(nSearchType, szSearchText) );
		}
		else if (nSearchType == kSearchInvalid)
		{
			//	Setting missing - we must have hit the end
			break;
		}

		nSettingsNumber++;
	}
}


// ------------------------------------------------------------------------------------------
//		* WriteRecentList															[Public]
// ------------------------------------------------------------------------------------------
//	Writes the current recent items list into the INI file.

void
CSearchBar::WriteRecentList()
{
	//	Write out the current recent items list to the INI file
	CWinApp *	pApp = AfxGetApp();
	CRString	szRecentListINISection(IDS_SEARCH_RECENT_LIST_INI_SECTION);
	CString		szSearchSetting;
	CString		szSearchSettingValue;
	int			nSettingsNumber = 0;

	for ( RecentListIteratorT recentListIterator = m_recentSearchList.begin();
		  recentListIterator != m_recentSearchList.end();
		  recentListIterator++ )
	{
		const char *	szSearchText = (*recentListIterator).GetSearchText();
		short			nSearchType = (*recentListIterator).GetSearchType();

		ASSERT(szSearchText && *szSearchText);
		ASSERT( (nSearchType >= kSearchWeb) && (nSearchType <= kSearchMailfolder) );
		
		if ( (szSearchText && *szSearchText) &&
			 (nSearchType >= kSearchWeb) && (nSearchType <= kSearchMailfolder) )
		{
			//	Write the search type setting for the current index
			szSearchSetting.Format(IDS_SEARCH_RECENT_TYPE_INI_FORMAT, nSettingsNumber);
			szSearchSettingValue.Format("%d", nSearchType);
			pApp->WriteProfileString(szRecentListINISection, szSearchSetting, szSearchSettingValue);

			//	Write the search text setting for the current index
			szSearchSetting.Format(IDS_SEARCH_RECENT_TEXT_INI_FORMAT, nSettingsNumber);
			pApp->WriteProfileString(szRecentListINISection, szSearchSetting, szSearchText);

		}

		nSettingsNumber++;
	}

	//	Remove any extra items that are no longer part of the current recent list
	while (nSettingsNumber < 100)
	{
		//	Remove the search type setting for the current index
		szSearchSetting.Format(IDS_SEARCH_RECENT_TYPE_INI_FORMAT, nSettingsNumber);
		pApp->WriteProfileString(szRecentListINISection, szSearchSetting, NULL);

		//	Remove the search text setting for the current index
		szSearchSetting.Format(IDS_SEARCH_RECENT_TEXT_INI_FORMAT, nSettingsNumber);
		pApp->WriteProfileString(szRecentListINISection, szSearchSetting, NULL);

		nSettingsNumber++;
	}
}


// ------------------------------------------------------------------------------------------
//		* UpdateSearchComboItems													[Public]
// ------------------------------------------------------------------------------------------
//	Update the list of search combo items. Called when first created and before the list
//	is dropped down.

void
CSearchBar::UpdateSearchComboItems()
{
	//	Rebuild the items from scratch
	m_SearchCombo.ResetContent();

	const char *	szMailboxName = GetActiveMailboxName();
	int				nMenuItemFormatResID;
	int				nBaseInitiateSearchIndex = 0;
	bool			bIsDisabled;

	//	Add the recent search items
	for ( RecentListIteratorT recentListIterator = m_recentSearchList.begin();
		  recentListIterator != m_recentSearchList.end();
		  recentListIterator++ )
	{
		const char *	szSearchText = (*recentListIterator).GetSearchText();

		ASSERT(szSearchText && *szSearchText);
		
		if (szSearchText && *szSearchText)
		{
			bIsDisabled = false;
			
			//	Determine the menu item format string ID
			switch ( (*recentListIterator).GetSearchType() )
			{
				case kSearchWeb:
					nMenuItemFormatResID = IDS_SEARCH_WEB_COMBO_FORMAT;
					break;
					
				case kSearchEudora:
					nMenuItemFormatResID = IDS_SEARCH_EUDORA_COMBO_FORMAT;
					break;
					
				case kSearchMailbox:
					nMenuItemFormatResID = IDS_SEARCH_MAILBOX_COMBO_FORMAT;
					bIsDisabled = (szMailboxName == NULL);
					break;

				case kSearchMailfolder:
					nMenuItemFormatResID = IDS_SEARCH_MAILFOLDER_COMBO_FORMAT;
					bIsDisabled = (szMailboxName == NULL);
					break;

				default:
					//	What's going on here? We should recognize the search type!
					ASSERT(0);
					nMenuItemFormatResID = 0;
					break;
			}

			if (nMenuItemFormatResID)
			{
				//	Format and append the text for the recent search item
				CString			szMenuItemText;
				
				szMenuItemText.Format(nMenuItemFormatResID, szSearchText);
				m_SearchCombo.InsertItem( szMenuItemText, nBaseInitiateSearchIndex, szMenuItemText.GetLength(), bIsDisabled );
			}
		}

		nBaseInitiateSearchIndex++;
	}

	if (nBaseInitiateSearchIndex > 0)
	{
		//	We have recent items - insert a divider line between these and
		//	the "initiate search with current text" items
		m_SearchCombo.InsertItem(NULL, nBaseInitiateSearchIndex, 0, true, true);
		nBaseInitiateSearchIndex++;
	}
	
	//	Insert "initiate search with current text" items after recent search items
	m_SearchCombo.InsertItem(IDS_SEARCH_WEB_COMBO, kSearchWeb + nBaseInitiateSearchIndex);
	m_SearchCombo.InsertItem(IDS_SEARCH_EUDORA_COMBO, kSearchEudora + nBaseInitiateSearchIndex);
	if (szMailboxName)
	{
		m_SearchCombo.InsertItem(IDS_SEARCH_MAILBOX_COMBO, kSearchMailbox + nBaseInitiateSearchIndex);
		m_SearchCombo.InsertItem(IDS_SEARCH_MAILFOLDER_COMBO, kSearchMailfolder + nBaseInitiateSearchIndex);
	}
}


// ------------------------------------------------------------------------------------------
//		* GetCurrentSearchMenuStringID												[Public]
// ------------------------------------------------------------------------------------------
//	Translates from the current search type to the appropriate menu string ID.
//
//	Returns:
//		The current menu string ID.

UINT
CSearchBar::GetCurrentSearchMenuStringID()
{
	UINT			nCurrentSearchMenuStringID = 0;

	switch (m_nCurrentSearch)
	{
		case kSearchEudora:
			nCurrentSearchMenuStringID = IDS_SEARCH_EUDORA_COMBO;
			break;

		case kSearchMailbox:
			{
				//	If we don't have an active mailbox name, then the mailbox
				//	menu item won't exist
				const char *	szMailboxName = GetActiveMailboxName();
				nCurrentSearchMenuStringID = szMailboxName ? IDS_SEARCH_MAILBOX_COMBO : IDS_SEARCH_EUDORA_COMBO;
			}
			break;

		case kSearchMailfolder:
			{
				//	If we don't have an active mailbox name, then the mailfolder
				//	menu item won't exist
				const char *	szMailboxName = GetActiveMailboxName();
				nCurrentSearchMenuStringID = szMailboxName ? IDS_SEARCH_MAILFOLDER_COMBO : IDS_SEARCH_EUDORA_COMBO;
			}
			break;

		case kSearchWeb:
		default:
			nCurrentSearchMenuStringID = IDS_SEARCH_WEB_COMBO;
			break;
	}

	return nCurrentSearchMenuStringID;
}


// ------------------------------------------------------------------------------------------
//		* RefreshSettings															[Public]
// ------------------------------------------------------------------------------------------
//	Refreshes the toolbar height in case it changed.

void
CSearchBar::RefreshSettings()
{
	//	Figure out the toolbar height based on whether or not large buttons are being used
	short	nToolbarHeight;
	if ( GetIniShort(IDS_INI_SHOW_LARGEBUTTONS) )
		nToolbarHeight = kLargeToolbarButtonHeight;
	else
		nToolbarHeight = kSmallToolbarButtonHeight;

	if (nToolbarHeight != m_nToolbarHeight)
		m_nToolbarHeight = nToolbarHeight;

	//	Get the search bar width
	short	nSearchBarWidth = GetIniShort(IDS_INI_SEARCH_BAR_WIDTH);

	//	Sanity check the search bar width
	if (nSearchBarWidth < kSearchBarMinWidth)
	{
		nSearchBarWidth = kSearchBarMinWidth;
		SetIniShort(IDS_INI_SEARCH_BAR_WIDTH, nSearchBarWidth);
	}
	else if (nSearchBarWidth > kSearchBarMaxWidth)
	{
		nSearchBarWidth = kSearchBarMaxWidth;
		SetIniShort(IDS_INI_SEARCH_BAR_WIDTH, nSearchBarWidth);
	}

	if (nSearchBarWidth != m_nSearchBarWidth)
		m_nSearchBarWidth = nSearchBarWidth;
}

// ------------------------------------------------------------------------------------------
//		* GetActiveMailboxName														[Public]
// ------------------------------------------------------------------------------------------
//	Figure out the active mailbox name if possible
//
//	Returns:
//		Pointer to active mailbox name. Do not delete and do not store (won't stay valid).

const char *
CSearchBar::GetActiveMailboxName()
{
	const char *	szMailboxName = NULL;
	
	//	Get the main frame
	CMainFrame *	pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
 	ASSERT_KINDOF(CMainFrame, pMainFrame);

	//	Get the active document for the main frame
	CFrameWnd *		pActiveFrame = pMainFrame->GetActiveFrame();
	CDocument *		pActiveDoc = NULL;

	if (pActiveFrame)
		pActiveDoc = pActiveFrame->GetActiveDocument();
	
	if (pActiveDoc)
	{
		CTocDoc *		pTocDoc = NULL;
		UINT			nWindowID = GetTopWindowID();
		
		//	Look for the TocDoc for the mailbox that the user intends to search
		switch (nWindowID)
		{
 			case IDR_TOC:
				pTocDoc = reinterpret_cast<CTocDoc *>(pActiveDoc);
				break;

			case IDR_READMESS:
			case IDR_COMPMESS:
				{
					CSummary *		pSummary = reinterpret_cast<CMessageDoc *>(pActiveDoc)->m_Sum;
					if (pSummary)
						pTocDoc = pSummary->m_TheToc;
				}
				break;
		}
		
		//	Get the mailbox name if possible
		if (pTocDoc)
			szMailboxName = pTocDoc->GetMBFileName();
	}

	return szMailboxName;
}


// ------------------------------------------------------------------------------------------
//		* SearchForCurrentText														[Public]
// ------------------------------------------------------------------------------------------
//	Search for the current text in the edit control that's embedded in the
//	CComboBoxEnhanced, which we access via CSearchBarComboBoxEdit.
//	Initiated either by choosing a new combo box item or by pressing the enter key.
//
//	Parameters
//		in_bBeepIfEmpty:	Indicates whether or not we should beep if there's no text

void
CSearchBar::SearchForCurrentText(
	bool				in_bBeepIfEmpty)
{
	//	Remember the current search type for future Eudora sessions
	SetIniShort(IDS_INI_SEARCH_BAR_SEARCH_TYPE, m_nCurrentSearch);
	
	//	Get the actual text in the edit field by telling it to tell the truth
	CString		szText;

	m_ComboEdit.GetActualText(szText);

	//	Trim any leading or trailing spaces and tabs
	szText.TrimLeft();
	szText.TrimRight();

	//	Replace any CRLFs and tabs with spaces
	szText.Replace("\r\n\t", " ");

	if ( szText.IsEmpty() )
	{
		if (in_bBeepIfEmpty)
			MessageBeep(MB_OK);
	}
	else
	{
		//	Remove any identical items
		int						nNumberRecentListItems = 0;
		RecentListIteratorT		recentListIterator = m_recentSearchList.begin();

		while ( recentListIterator != m_recentSearchList.end() )
		{
			if ( (m_nCurrentSearch == (*recentListIterator).GetSearchType()) &&
				 (szText.CompareNoCase((*recentListIterator).GetSearchText()) == 0) )
			{
				//	We found an identical item to the one we're planning to add.
				//	Remove it and move on to the next item.
				recentListIterator = m_recentSearchList.erase(recentListIterator);
			}
			else
			{
				//	Just move on to the next item
				recentListIterator++;

				//	Count the recent list items while we iterate because with lists
				//	calling size could be O(n) anyway
				nNumberRecentListItems++;
			}
		}
		
		//	Add the search to the front of the recent list
		m_recentSearchList.push_front( RecentSearch(m_nCurrentSearch, szText) );
		nNumberRecentListItems++;

		//	Make sure that the size doesn't exceed the maximum size allowed
		int		nMaxNumberRecentListItems = GetMaxNumberRecentItems();
		if (nNumberRecentListItems > nMaxNumberRecentListItems)
			m_recentSearchList.resize(nMaxNumberRecentListItems);
		
		if (m_nCurrentSearch == kSearchWeb)
		{
			//	We're searching the web - that's easy enough
			LaunchURLWithQuery(NULL, ACTION_SEARCH_WEB, szText, NULL, NULL, NULL, NULL, ACTION_SEARCH_PARAM_LABEL);
		}
		else
		{
 			//	We're searching for text in all of Eudora, a specific mailbox, or a specific mailfolder
			//	I wasn't sure of the best way to do this since we need to initiate a command with
			//	specific data (most commands are dispatched without data). There may be a better,
			//	cleaner way to do this, but for now we'll just figure out the mailbox name if possible.
			bool			bSelectParent = false;
			bool			bSelectAll = true;

			const char *	szMailboxName = GetActiveMailboxName();
			
			if (szMailboxName)
			{
				//	We have the mailbox name - set the parameters appropriately
				if (m_nCurrentSearch != kSearchEudora)
					bSelectAll = false;
				
				if (m_nCurrentSearch == kSearchMailfolder)
					bSelectParent = true;
			}

			//	Open the find messages window with what we have
			OpenFindMessagesWindow(szMailboxName, szText, true, bSelectParent, bSelectAll, true);
		}

		//	The search is initiated, blank out the search text so that the gray
		//	text will display again.
		m_ComboEdit.SetActualText("");

		//	Update the gray text status so that we display the gray text again.
		m_ComboEdit.UpdateGrayTextStatus();

		//	Save the current recent list into the INI file
		WriteRecentList();
	}
}


// ------------------------------------------------------------------------------------------
//		* GetBarInfoEx																[Public]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and calls SECControlBar (grandparent)
//	implementation.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).
//
//	Parameters:
//		pInfo:		Control bar info

void
CSearchBar::GetBarInfoEx(
	SECControlBarInfo *		pInfo)
{
	SECControlBar::GetBarInfoEx(pInfo);
}


// ------------------------------------------------------------------------------------------
//		* SetBarInfoEx																[Public]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and calls SECControlBar (grandparent)
//	implementation.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).
//
//	Parameters:
//		pInfo:		Control bar info
//		pFrameWnd:	Frame window

void
CSearchBar::SetBarInfoEx(
	SECControlBarInfo *		pInfo,
	CFrameWnd *				pFrameWnd)
{
	SECControlBar::SetBarInfoEx(pInfo, pFrameWnd);
}



// ------------------------------------------------------------------------------------------
//		* OnBarStyleChange															[Public]
// ------------------------------------------------------------------------------------------
//	Hooks in after bar style changing to make sure that we maintain any styles that we feel
//	are important.  Needed because it's easy to make the toolbars persistent by relying on
//	the default toolbar INI code, but that code also saves and restores styles. That means
//	that if we add any styles after we've already released a version of Eudora, any users
//	that used the previous version won't get the new styles that we've added.
//
//	Parameters:
//		dwOldStyle:		Old style

void
CSearchBar::OnBarStyleChange(
	DWORD				dwOldStyle,
	DWORD				dwNewStyle)
{
	SECControlBar::OnBarStyleChange(dwOldStyle, dwNewStyle);
	
	if (dwOldStyle | CBRS_BORDER_3D)
		m_dwStyle |= CBRS_BORDER_3D;

	if (dwOldStyle | CBRS_SIZE_FIXED)
		m_dwStyle |= CBRS_SIZE_FIXED;

	if (dwOldStyle | CBRS_SIZE_DYNAMIC)
		m_dwStyle |= CBRS_SIZE_DYNAMIC;
}


// ------------------------------------------------------------------------------------------
//		* OnUpdateCmdUI																[Public]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and calls SECControlBar (grandparent)
//	implementation.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).

void
CSearchBar::OnUpdateCmdUI(
	CFrameWnd *				pTarget,
	BOOL					bDisableIfNoHndler)
{
	SECControlBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
}


// ------------------------------------------------------------------------------------------
//		* OnToolHitTest																[Public]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and calls SECControlBar (grandparent)
//	implementation.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).

int
CSearchBar::OnToolHitTest(
	CPoint					point,
	TOOLINFO *				pTI) const
{
	return SECControlBar::OnToolHitTest(point, pTI);
}


// ------------------------------------------------------------------------------------------
//		* CalcFixedLayout														 [Protected]
// ------------------------------------------------------------------------------------------
//	Calculate fixed size of SearchBar.
//
//	Parameters:
//		bStretch:	Since CSearchBar is always available for docking (i.e. it's always
//					docked or floating), this should always be 0. It's therefore not used.
//		bHorz:		Whether or not the calculation is for horizontal orientation.
//
//	Returns:
//		The size of the fixed layout

CSize
CSearchBar::CalcFixedLayout(
	BOOL				bStretch,
	BOOL				bHorz)
{
	RefreshSettings();
	
	CSize		size;

	if (bHorz)
	{
		m_szDockHorz.cx = m_nSearchBarWidth;
		m_szDockHorz.cy = m_nToolbarHeight;

		size = m_szDockHorz;
	}
	else
	{
		m_szDockVert.cx = m_nSearchBarWidth;
		m_szDockVert.cy = kLargeToolbarButtonHeight;

		size = m_szDockVert;
	}

	return size;
}


// ------------------------------------------------------------------------------------------
//		* CalcFixedLayout														 [Protected]
// ------------------------------------------------------------------------------------------
//	Calculate dynamic size of SearchBar.
//
//	Parameters:
//		nLength:	Requested size. We don't care - CSearchBar will be the size it needs
//					to be regardless.
//		dwMode:		Sizing flags.
//
//	Returns:
//		The size of the dynamic layout

CSize
CSearchBar::CalcDynamicLayout(
	int					nLength,
	DWORD				dwMode)
{
	//	Make sure CBRS_SIZE_DYNAMIC is turned on (in case a previous state was
	//	saved where it was not). This is necessary in order for our floating size
	//	to be consistently different than our horizontal docked size (necessary
	//	when using small toolbar buttons).
	m_dwStyle |= CBRS_SIZE_DYNAMIC;

	//	Modify our parent's frame style so that it's frame isn't resizable.
	//	We want the better dragging behavior that we get with CBRS_SIZE_DYNAMIC,
	//	but we don't want actual resizing to be allowed.
	BOOL	bIsFloating = IsFloating();
	if (bIsFloating && !m_bWasFloating)
	{
		CFrameWnd *		pParentFrame = GetParentFrame();
		
		pParentFrame->ModifyStyle(0, MFS_MOVEFRAME);
	}
	m_bWasFloating = bIsFloating;
	
	CSize		size;
	
	if ( !(dwMode & LM_MRUWIDTH) && (dwMode & (LM_HORZDOCK | LM_VERTDOCK)) )
	{
		size = CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZ);
	}
	else
	{
		m_szFloat.cx = m_nSearchBarWidth;
		m_szFloat.cy = kLargeToolbarButtonHeight;

		size = m_szFloat;
	}

	return size;
}


// ------------------------------------------------------------------------------------------
//		* DoPaint																 [Protected]
// ------------------------------------------------------------------------------------------
//	Override of DoPaint so that we can check to see if our extended style has been changed.
//	Unfortunately no one will notify us if they change our extended style - they just
//	do it and invalidate us so that we redraw - this seemed as good a place as any to
//	notice the change and react.
//
//	Parameters:
//		pDC:		Pointer to device context for painting.

void
CSearchBar::DoPaint(
	CDC *				pDC)
{
	//	Check to see if our gripper has been changed
	if ( (m_dwExOldStyle & CBRS_EX_GRIPPER) != (m_dwExStyle & CBRS_EX_GRIPPER) )
	{
		//	Resize our combo box because our gripper has been changed
		if (m_dwStyle & CBRS_FLOATING)
			ResizeComboBox(kFloating);
		else if ( (m_dwStyle & CBRS_ALIGN_LEFT) || (m_dwStyle & CBRS_ALIGN_RIGHT) )
			ResizeComboBox(kDockedVertically);
		else if ( (m_dwStyle & CBRS_ALIGN_TOP) || (m_dwStyle & CBRS_ALIGN_BOTTOM) )
			ResizeComboBox(kDockedHorizontally);
	}

	m_dwExOldStyle = m_dwExStyle;
	
	SECCustomToolBar::DoPaint(pDC);
}


// ------------------------------------------------------------------------------------------
//		* CalculateComboBoxRect													 [Protected]
// ------------------------------------------------------------------------------------------
//	Calculate the combo box rectangle when being first created, when being resized,
//	or when our style has been changed.
//
//	Parameters:
//		in_eOrientationType:	Type of orientation with which to calculate.
//		in_nComboHeight:		Height of combo box so that we can center it correctly
//		out_rectCombo:			Combo box rectangle that we fill in

void
CSearchBar::CalculateComboBoxRect(
	OrientationType			in_eOrientationType,
	int						in_nComboHeight,
	CRect &					out_rectCombo)
{
	if (in_eOrientationType == kDockedHorizontally)
	{
		out_rectCombo.left = m_cxLeftBorder;
		if (m_dwExStyle & CBRS_EX_GRIPPER)
			out_rectCombo.left += m_GripperInfo.GetWidth() + kSearchComboPaddingFromGripper;
		else
			out_rectCombo.left += m_GripperInfo.m_nGripperOffSidePadding;
		out_rectCombo.right = m_nSearchBarWidth - m_cxRightBorder - m_GripperInfo.m_nGripperOffSidePadding - kSearchComboPaddingFromEdgeDocked;

		//	Center combo box vertically
		//	Extra - 1 is a fudge factor. I'm not sure why it helps, but it
		//	looks better, particularly when dealing with small toolbar buttons.
		out_rectCombo.top = (m_nToolbarHeight - in_nComboHeight) / 2 - 1;
	}
	else if (in_eOrientationType == kDockedVertically)
	{
		int		nSpaceForGripper = 0;
		
		if (m_dwExStyle & CBRS_EX_GRIPPER)
			nSpaceForGripper = m_GripperInfo.GetHeight() + 2;
		
		out_rectCombo.left = m_GripperInfo.m_nGripperOffSidePadding + m_cxLeftBorder + kSearchComboPaddingFromEdgeDocked;			
		out_rectCombo.right = m_nSearchBarWidth - m_cxRightBorder - m_GripperInfo.m_nGripperOffSidePadding - kSearchComboPaddingFromEdgeDocked;

		//	Center combo box vertically
		out_rectCombo.top = (kLargeToolbarButtonHeight - in_nComboHeight - nSpaceForGripper - kSearchComboPaddingFromEdgeDocked) / 2 + nSpaceForGripper;
	}
	else	//	in_eOrientationType == kFloating
	{
		out_rectCombo.left = kSearchComboPaddingFromEdgeFloating;
		out_rectCombo.right = m_nSearchBarWidth - kSearchComboPaddingFromEdgeFloating;

		//	Center combo box vertically
		out_rectCombo.top = (kLargeToolbarButtonHeight - in_nComboHeight) / 2;
	}

	//	Leave enough space for combo box menu items
	out_rectCombo.bottom = out_rectCombo.top + kSearchComboHeight;
}


// ------------------------------------------------------------------------------------------
//		* ResizeComboBox														 [Protected]
// ------------------------------------------------------------------------------------------
//	Handles resizing the combo box for the current docking or floating configuration.
//
//	Parameters:
//		in_eOrientationType:	The current type of docking or floating orientation

void
CSearchBar::ResizeComboBox(
	OrientationType			in_eOrientationType)
{
	if ( (in_eOrientationType == kDockedHorizontally) &&
		 (m_nToolbarHeight == kSmallToolbarButtonHeight) )
	{
		//	We need to be as small as possible when docked horizontally with
		//	small toolbar buttons.
		m_SearchCombo.SetItemHeight(-1, m_nEditTextMinHeight);
	}
	else
	{
		//	We can be a little bigger and look a little better.
		m_SearchCombo.SetItemHeight(-1, m_nEditTextMinHeight + 2);
	}

	CRect			rectCombo;

	m_SearchCombo.GetWindowRect(&rectCombo);

	int				nComboHeight = rectCombo.Height();

	CalculateComboBoxRect(in_eOrientationType, nComboHeight, rectCombo);

	//	Adjust combo box position
	m_SearchCombo.MoveWindow(&rectCombo);
}


// ------------------------------------------------------------------------------------------
//		* BalanceWrap															 [Protected]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and does nothing.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).

void
CSearchBar::BalanceWrap(
	int						nRow,
	Wrapped *				pWrap)
{
	
}


// ------------------------------------------------------------------------------------------
//		* GetDragMode															 [Protected]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and returns FALSE.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).

BOOL
CSearchBar::GetDragMode() const
{
	return FALSE;
}


// ------------------------------------------------------------------------------------------
//		* AcceptDrop															 [Protected]
// ------------------------------------------------------------------------------------------
//	Overrides SECCustomToolBar implementation and returns FALSE.
//
//	We mostly just want the functionality of being an SECControlBar because we don't want
//	to support additional buttons. Unfortunately being an SECCustomToolBar has two important
//	advantages:
//	* We'll draw more like a toolbar with the drop shadows
//	* SECDockBar::CalcFixedLayout makes a layout decision based on whether or not
//	  we're a toolbar (it shifts the origin up a pixel to y=-2 instead of y=-1, which
//	  throws off more drawing in subtle ways than you'd expect).

BOOL
CSearchBar::AcceptDrop() const
{
	return FALSE;
}


// ------------------------------------------------------------------------------------------
//		* OnSize																 [Protected]
// ------------------------------------------------------------------------------------------
//	Resize any controls.
//
//	Parameters:
//		nType:		Specifies the type of resizing requested
//		cx:			Specifies the new width of the client area
//		cy:			Specifies the new height of the client area

void
CSearchBar::OnSize(
	UINT				nType,
	int					cx,
	int					cy)
{
	SECControlBar::OnSize(nType, cx, cy);

	if (cx && cy)
	{
		if (m_dwStyle & CBRS_FLOATING)
			ResizeComboBox(kFloating);
		else if ( (m_dwStyle & CBRS_ALIGN_LEFT) || (m_dwStyle & CBRS_ALIGN_RIGHT) )
			ResizeComboBox(kDockedVertically);
		else if ( (m_dwStyle & CBRS_ALIGN_TOP) || (m_dwStyle & CBRS_ALIGN_BOTTOM) )
			ResizeComboBox(kDockedHorizontally);
	}
}


// ------------------------------------------------------------------------------------------
//		* OnEraseBkgnd															 [Protected]
// ------------------------------------------------------------------------------------------
//	Strangely, we need to handle the drawing of the control bar background ourselves.
//
//	Parameters:
//		pDC:	Specifies the device-context object
//
//	Returns:
//		TRUE to indicate that no further erasing is required.

BOOL
CSearchBar::OnEraseBkgnd(
	CDC *				pDC)
{
	CBrush	backBrush;

	backBrush.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);
	CRect rect;

	pDC->GetClipBox(&rect);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);

	return TRUE;
}


// ------------------------------------------------------------------------------------------
//		* UpdateSearchBarCombo													 [Protected]
// ------------------------------------------------------------------------------------------
//	Enable SearchBar combo box - currently always enabled.
//
//	Parameters:
//		fill out

void
CSearchBar::UpdateSearchBarCombo(
	CCmdUI *			pCmdUI )
{
	pCmdUI->Enable(TRUE);
}


// ------------------------------------------------------------------------------------------
//		* OnComboBoxItemSelected												 [Protected]
// ------------------------------------------------------------------------------------------
//	Handle combo box selection.

void
CSearchBar::OnComboBoxItemSelected()
{
	int		nSel = m_SearchCombo.GetCurSel();

	if (nSel >= 0)
	{
		int				nNumberRecentSearchItems = m_recentSearchList.size();

		if (nSel < nNumberRecentSearchItems)
		{
			//	User chose a recent search item - find the information for the item selected
			int					nItemIndex = 0;

			for ( RecentListIteratorT recentListIterator = m_recentSearchList.begin();
				  recentListIterator != m_recentSearchList.end();
				  recentListIterator++, nItemIndex++ )
			{
				if (nItemIndex == nSel)
				{
					//	We found the information for the item selected
					const char *	szSearchText = (*recentListIterator).GetSearchText();
										
					if (szSearchText && *szSearchText)
					{
						//	Set the text in the combo box
						m_ComboEdit.SetActualText(szSearchText);

						//	The current search type is now whatever the recent list item says
						m_nCurrentSearch = (*recentListIterator).GetSearchType();

						//	Remove the recent list item because it will be added to the front
						//	again anyway
						m_recentSearchList.erase(recentListIterator);

						//	Initiate the search which will work off of m_nCurrentSearch and
						//	the text that we placed in the combo box
						SearchForCurrentText();
					}
					else
					{
						ASSERT(0);
						m_recentSearchList.erase(recentListIterator);
					}

					break;
				}
			}
		}
		else
		{
			int		nBaseInitiateSearchIndex = 0;

			//	If we have recent search items - then the base for the
			//	"initiate search with current text" items is after them
			//	and the divider line
			if (nNumberRecentSearchItems > 0)
				nBaseInitiateSearchIndex = nNumberRecentSearchItems + 1;
			
			//	Remember the user's selection so that we can display it selected
			//	any time the combo box is dropped down.
			m_nCurrentSearch = static_cast<short>(nSel - nBaseInitiateSearchIndex);
			SearchForCurrentText(false);
		}
		
		//	Remember the current search type for future Eudora sessions
		SetIniShort(IDS_INI_SEARCH_BAR_SEARCH_TYPE, m_nCurrentSearch);
	}
}


// ------------------------------------------------------------------------------------------
//		* OnComboBoxDropDown													 [Protected]
// ------------------------------------------------------------------------------------------
//	Handle combo box drop down by updating the menu items.

void
CSearchBar::OnComboBoxDropDown()
{
	UpdateSearchComboItems();
}
