//	LinkHistoryList.cpp
//
//	Custom subclass of CListView specifically designed for the Link History
//	Window. Draws large icons correctly, clips correctly, and does
//	full row selection.
//
//	Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "LinkHistoryList.h"

#include <QCUtils.h>

#include "guiutils.h"
#include "resource.h"

#include "QCSharewareManager.h"

#include "DebugNewHelpers.h"


IMPLEMENT_DYNCREATE(CLinkHistoryList, CListView)

//	Message mapped overrides
BEGIN_MESSAGE_MAP(CLinkHistoryList, CListView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeyDown)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateCopy)
	ON_UPDATE_COMMAND_UI(IDM_LH_EDIT_COPY, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(IDM_LH_VIEW_LINK, OnUpdateNeedSingleSelection)
	ON_UPDATE_COMMAND_UI(IDM_LH_REMOVE_LINK, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(IDM_LH_MARK_LINK_REMIND, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(IDM_LH_MARK_LINK_BOOKMARKED, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(IDM_LH_MARK_LINK_NORMAL, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(IDM_LH_EDIT_COPY, OnEditCopy)
	ON_COMMAND(IDM_LH_VIEW_LINK, OnLaunchSelectedItem)
	ON_COMMAND(IDM_LH_REMOVE_LINK, OnRemoveSelectedItems)
	ON_COMMAND(IDM_LH_MARK_LINK_REMIND, OnChangeSelItemsStatusToRemind)
	ON_COMMAND(IDM_LH_MARK_LINK_BOOKMARKED, OnChangeSelItemsStatusToBookmarked)
	ON_COMMAND(IDM_LH_MARK_LINK_NORMAL, OnChangeSelItemsStatusToNone)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_MESSAGE(LVM_SETBKCOLOR, OnSetBkColor)
END_MESSAGE_MAP()


//	Static pointer to an instance of CLinkHistoryList (should only be one)
CLinkHistoryList *  CLinkHistoryList::s_pLinkHistoryList = NULL;

// ---------------------------------------------------------------------------
//		* CLinkHistoryList
// ---------------------------------------------------------------------------
//	CLinkHistoryList constructor

CLinkHistoryList::CLinkHistoryList()
	:	m_iSortColumn(sc_DateVisited), m_bReverseOrder(FALSE),
		m_iPrevSortColumn(1), m_bPrevReverseOrder(FALSE),
		m_bStatusURLShown(false),
		m_szStatusRemind(IDS_LINK_HISTORY_REMIND_STATUS),
		m_szStatusBookmarked(IDS_LINK_HISTORY_BOOKMARKED_STATUS),
		m_szStatusAttempted(IDS_LINK_HISTORY_ATTEMPTED_STATUS),
		m_szStatusNotDisplayed(IDS_LINK_HISTORY_NOT_DISPLAYED_STATUS),
		m_bSelectionChangingKeyHit(false),
		m_bDrawTypeIcon(true)
{	
	//	Get the sort column settings from INI file
	m_iSortColumn = GetIniShort(IDS_INI_LH_SORT_COLUMN);
	m_bReverseOrder = ( GetIniShort(IDS_INI_LH_SORT_REV) != 0 );
	m_iPrevSortColumn = GetIniShort(IDS_INI_LH_PREVSORT_COLUMN);
	m_bPrevReverseOrder = ( GetIniShort(IDS_INI_LH_PREVSORT_REV) != 0 );
	m_bDrawTypeIcon = ( GetIniShort(IDS_INI_LH_DRAW_ICONS) != 0 );
	
	//	Get the window color for list drawing
	m_clrBkgnd = ::GetSysColor(COLOR_WINDOW);

	//	There should only be one link history view, but given the creation
	//	process it's not worth enforcing
	ASSERT(!s_pLinkHistoryList);
	s_pLinkHistoryList = this;
}


// ---------------------------------------------------------------------------
//		* ~CLinkHistoryList
// ---------------------------------------------------------------------------
//	CLinkHistoryList destructor

CLinkHistoryList::~CLinkHistoryList()
{
	if (s_pLinkHistoryList == this)
		s_pLinkHistoryList = NULL;
}


// ---------------------------------------------------------------------------
//		* AssertValid & Dump
// ---------------------------------------------------------------------------
//	CLinkHistoryList diagnostics

#ifdef _DEBUG
void CLinkHistoryList::AssertValid() const
{
	CListView::AssertValid();
}

void CLinkHistoryList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG


// ---------------------------------------------------------------------------
//		* AddURL													[Public]
// ---------------------------------------------------------------------------
//	Adds a URL to the list. Called from LinkHistoryManager.

bool
CLinkHistoryList::AddURL(
	LPUrlItemData	pURL)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

	//	Add the new item at the end
	lvi.iItem = GetListCtrl().GetItemCount(); 
	lvi.iSubItem = 0; 
	lvi.iImage = I_IMAGECALLBACK;
	lvi.pszText = LPSTR_TEXTCALLBACK; 
	lvi.lParam = (LPARAM) pURL;

	if ( GetListCtrl().InsertItem (&lvi) == -1)
		return false;

	ResortItems();

	return true;
}


//


// ---------------------------------------------------------------------------
//		* UpdateURL													[Public]
// ---------------------------------------------------------------------------
//	Updates a URL in the list. Called from LinkHistoryManager.

void
CLinkHistoryList::UpdateURL(
	LPUrlItemData	pURL,
	bool			bEnsureVisible)
{
	bool	bInvalidateSingleItem = true;

	//	Assume (for now) that updating URL will only update date visited
	//	Resort the items (if this ever proves to be too slow, we'll need
	//	to wait to do this at idle time)
	if ( ResortItemsIfSortedByDate() )
	{
		Invalidate();
		bInvalidateSingleItem = false;
	}

	if (bEnsureVisible || bInvalidateSingleItem)
	{
		LV_FINDINFO		lvFindInfo;
		lvFindInfo.flags = LVFI_PARAM;
		lvFindInfo.psz = NULL;
		lvFindInfo.lParam = reinterpret_cast<LPARAM>(pURL);

		int				nIndex = GetListCtrl().FindItem(&lvFindInfo);
		ASSERT(nIndex != -1);
		
		if (nIndex != -1)
		{
			if (bEnsureVisible)
			{
				//	Pass TRUE for partial ok in case we got called because user
				//	viewed a URL that was only partially visible (i.e. avoid
				//	unnecessary scrolling).
				GetListCtrl().EnsureVisible(nIndex, TRUE);
			}

			if (bInvalidateSingleItem)
				GetListCtrl().RedrawItems(nIndex, nIndex);
		}
	}
}


// ---------------------------------------------------------------------------
//		* RedrawRemovedItems									[Protected]
// ---------------------------------------------------------------------------
//	Invalidate every item from the first selected item down.
//	Call before calling DeleteItem so that any redrawing that occurs
//	in DeleteItem will redraw the entire row (rather than leaving gray
//	splotches to the right of selected items - caused by full row selection
//	not being redrawn).

void
CLinkHistoryList::RedrawRemovedItems(
	int				nFirstItemToRedraw)
{
	CRect	rcBounds, rcClient;
	GetClientRect(rcClient);
	
	GetListCtrl().GetItemRect(nFirstItemToRedraw, rcBounds, LVIR_BOUNDS);
	rcClient.top = rcBounds.top;
	GetListCtrl().InvalidateRect(rcClient, FALSE);
}


// ---------------------------------------------------------------------------
//		* RemoveURL													[Public]
// ---------------------------------------------------------------------------
//	Removes a URL from the list. Called from LinkHistoryManager.

void
CLinkHistoryList::RemoveURL(
	LPUrlItemData	pURL)
{
	LV_FINDINFO		lvFindInfo;
	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.psz = NULL;
	lvFindInfo.lParam = reinterpret_cast<LPARAM>(pURL);

	int				nIndex = GetListCtrl().FindItem(&lvFindInfo);
	ASSERT(nIndex != -1);
	
	if (nIndex != -1)
	{
		RedrawRemovedItems(nIndex);
		
		//	Delete the list control item
		GetListCtrl().DeleteItem(nIndex);
	}
}


// ---------------------------------------------------------------------------
//		* OnCreate												[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to initialize the list attributes, etc.

int CLinkHistoryList::OnCreate(LPCREATESTRUCT lpcs) 
{
	if ( CListView::OnCreate(lpcs) == -1 )
		return -1;

	//	Set the control to report style with icons
	BOOL	bModGood = ModifyStyle( 0xFFFF,
									LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON | LVS_ALIGNLEFT | LVS_OWNERDRAWFIXED );
	if (!bModGood)
		return -1;
	
	//	Setup the columns
	GetListCtrl().InsertColumn( 0, CRString(IDS_LINK_HISTORY_TYPE_LABEL), LVCFMT_LEFT,
								GetIniShort(IDS_INI_LH_TYPE_WIDTH) );
	GetListCtrl().InsertColumn( 1, CRString(IDS_LINK_HISTORY_NAME_LABEL), LVCFMT_LEFT,
								GetIniShort(IDS_INI_LH_NAME_WIDTH) );
	GetListCtrl().InsertColumn( 2, CRString(IDS_LINK_HISTORY_DATE_VISITED_LABEL), LVCFMT_LEFT,
								GetIniShort(IDS_INI_LH_DATE_VISITED_WIDTH) );	
	if (m_bDrawTypeIcon)
	{
		//	Set an image list of large icons as the "small" icon imagelist because
		//	"small" icons are all that are displayed in report view. CListView
		//	seems to handle the dimensions correctly.
		//
		//	Regarding GetIconImageList - it was a tough call of who should own the
		//	icon/thumbnail image list. Ideally CLinkHistoryList would own it, but the
		//	fact was that LinkHistoryManager was adding all the thumbnails to it. In
		//	particular LinkHistoryManager adds thumbnails to it when it loads which
		//	occurs before CLinkHistoryList exists.
		CImageList *	pImageList = LinkHistoryManager::Instance()->GetIconImageList();

		if (pImageList != NULL)
			GetListCtrl().SetImageList(pImageList, LVSIL_SMALL);
		else
			m_bDrawTypeIcon = false;
	}
	
	QCSharewareManager *	pSWM = GetSharewareManager();
	if (pSWM != NULL)
		pSWM->Register(this);

	if ( UsingFullFeatureSet() )
	{
		//	Notify the Link History Manager that we're ready for it to fill us with URLs
		LinkHistoryManager::Instance()->NotifyLHViewReady();
	}
	else
	{
		DisplayFeatureNotInFreeMessage();
	}

	return 0;
}


// ---------------------------------------------------------------------------
//		* OnDestroy												[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to save the view ini settings and
//	unregister with the shareware manager.

void CLinkHistoryList::OnDestroy() 
{
	//	Save column widths to INI file.
	ASSERT( ::IsWindow( GetSafeHwnd() ) );

	LV_COLUMN col;
	col.mask = LVCF_WIDTH;
	for (int i = 0; GetListCtrl().GetColumn(i, &col); i++)
	{
		ASSERT(col.cx > 0 && col.cx < 30000);
		switch (i)
		{
		case sc_Type:
			// Type column
			SetIniShort(IDS_INI_LH_TYPE_WIDTH, short(col.cx));
			break;
		case sc_Name:
			// Name column
			SetIniShort(IDS_INI_LH_NAME_WIDTH, short(col.cx));
			break;
		case sc_DateVisited:
			// Date Visited column
			SetIniShort(IDS_INI_LH_DATE_VISITED_WIDTH, short(col.cx));
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	QCSharewareManager *	pSWM = GetSharewareManager();
	if (pSWM != NULL)
		pSWM->UnRegister(this);

	CListView::OnDestroy();
}


// ---------------------------------------------------------------------------
//		* DisplayFeatureNotInFreeMessage							[Public]
// ---------------------------------------------------------------------------
//	Delete any items in the list and display the light message instead.

void CLinkHistoryList::DisplayFeatureNotInFreeMessage()
{
	//	Remove all the items, and display the light message.
	//	NOTE: Must be careful to never use lvi.lParam as a LPUrlItemData
	//		  when UsingFullFeatureSet() returns false.
	Invalidate();
	GetListCtrl().DeleteAllItems();
	GetListCtrl().InsertItem( 0, CRString(IDS_ERR_FEATURE_NOT_IN_FREE) );
}


// ---------------------------------------------------------------------------
//		* ResortItems												[Public]
// ---------------------------------------------------------------------------
//	Helper routine which resorts the list items.

BOOL CLinkHistoryList::ResortItems()
{
	BOOL	bResult = FALSE;

	if ( UsingFullFeatureSet() )
	{
		bResult = GetListCtrl().SortItems( SortCompareFunc, reinterpret_cast<DWORD>(this) );
	}

	return bResult;
}


// ---------------------------------------------------------------------------
//		* ResortItemsIfSortedByDate									[Public]
// ---------------------------------------------------------------------------
//	Helper routine which resorts the list items only if they are sorted by
//	date.

bool CLinkHistoryList::ResortItemsIfSortedByDate()
{
	bool	bResortItems =  (m_iSortColumn == sc_DateVisited);

	if (bResortItems)
		ResortItems();
	
	return bResortItems;
}



// ---------------------------------------------------------------------------
//		* Notify													[Public]
// ---------------------------------------------------------------------------
//	Notify us when program changes state between Light, Pro, and Adware.

void CLinkHistoryList::Notify(
	QCCommandObject*		/*pObject*/,
	COMMAND_ACTION_TYPE		theAction,
	void *					pData)
{
	if (theAction == CA_SWM_CHANGE_FEATURE)
	{
		bool	bIsFullFeatureSet = UsingFullFeatureSet();

		//	Were we using the full feature set? When in doubt assume the opposite of
		//	the current usage so that we make sure the list contains the correct info
		//	(i.e. list of items or "not in free" message).
		bool	bWasFullFeatureSet = (pData == NULL) ? !bIsFullFeatureSet :
									 (*reinterpret_cast<SharewareModeType *>(pData) != SWM_MODE_LIGHT);

		if (bWasFullFeatureSet != bIsFullFeatureSet)
		{
			if (bIsFullFeatureSet)
			{
				Invalidate();
				GetListCtrl().DeleteAllItems();

				//	Notify the Link History Manager that we're
				//	ready for it to fill us with URLs
				LinkHistoryManager::Instance()->NotifyLHViewReady();
			}
			else
			{
				DisplayFeatureNotInFreeMessage();
			}
		}
	}
}



// ---------------------------------------------------------------------------
//		* OnColumnClick											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to respond to the user clicking on
//	a column header.

void CLinkHistoryList::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *	pNMListView = (NM_LISTVIEW*)pNMHDR;

	if( pNMListView->iItem == -1 )
	{
		//	The user clicked on a header button
		BOOL	bResult = FALSE;

		//	Save the previous info in case we need to revert
		short	bSavePrevColumn = m_iPrevSortColumn;
		BOOL	bSavePrevReverseState = m_bPrevReverseOrder;

		//	Store the previous sort order for when we sort by type
		m_iPrevSortColumn = m_iSortColumn;
		m_bPrevReverseOrder = m_bReverseOrder;

		if(	m_iSortColumn == pNMListView->iSubItem )
		{
			//	If the index is the same, toggle between normal and reversed order 
			m_bReverseOrder = !m_bReverseOrder;
		}
		else
		{					
			//	Always do normal order first
			m_bReverseOrder = FALSE;
		}

		m_iSortColumn = static_cast<short>(pNMListView->iSubItem);

		bResult = ResortItems();

		if (!bResult)
		{
			//	Restore the previous sort info
			m_iSortColumn = m_iPrevSortColumn;
			m_bReverseOrder = m_bPrevReverseOrder;
			m_iPrevSortColumn = bSavePrevColumn;
			m_bPrevReverseOrder = bSavePrevReverseState;
		}
		
		//	Save sort column information to INI file.
		SetIniShort(IDS_INI_LH_SORT_COLUMN, m_iSortColumn);
		SetIniShort( IDS_INI_LH_SORT_REV, static_cast<short>(m_bReverseOrder ? 1 : 0) );
		SetIniShort(IDS_INI_LH_PREVSORT_COLUMN, m_iPrevSortColumn);
		SetIniShort( IDS_INI_LH_PREVSORT_REV, static_cast<short>(m_bPrevReverseOrder ? 1 : 0) );
	}
	
	*pResult = 0;
}


// ---------------------------------------------------------------------------
//		* OnKeyDown											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to set a flag which causes the status text
//	to be changed when the user navigates with the arrow keys.

void CLinkHistoryList::OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_KEYDOWN  *	pKeyDown = reinterpret_cast<LV_KEYDOWN *>(pNMHDR);
	
	if (pKeyDown != NULL)
	{
		switch (pKeyDown->wVKey)
		{
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
			m_bSelectionChangingKeyHit = true;
			break;
		}
	}

	*pResult = 0;
}


// ---------------------------------------------------------------------------
//		* OnLButtonDblClk										[Protected]
// ---------------------------------------------------------------------------
//	Message mapped method in which we respond to the user double
//	clicking on an item by launching the selected item.

void CLinkHistoryList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	OnLaunchSelectedItem();
}


// ---------------------------------------------------------------------------
//		* ShowStatusText										[Protected]
// ---------------------------------------------------------------------------
//	Helper method which sets the status text for a given index. Called
//	from either OnSetCursor or DrawItem (in response to OnKeyDown setting
//	flag).

void CLinkHistoryList::ShowStatusText(int nIndexToShow)
{
	if ( UsingFullFeatureSet() )
	{
		CFrameWnd *		pFrameWnd = GetTopLevelFrame();
		
		if (pFrameWnd != NULL)
		{
			LV_ITEM		lvi;
			lvi.mask = LVIF_PARAM;
			lvi.iItem = nIndexToShow;
			lvi.iSubItem = 0;

			if ( GetListCtrl().GetItem(&lvi) )
			{
				//	Set the status text
				LPUrlItemData	pListItemData = (LPUrlItemData) lvi.lParam;

				if (pListItemData == NULL)
				{
					ASSERT(0);
					return;
				}

				pFrameWnd->SetMessageText( GetDisplayURL(pListItemData) );
				pFrameWnd->GetMessageBar()->UpdateWindow();
				SetStatusURLShown(true);
			}
		}
	}
}


// ---------------------------------------------------------------------------
//		* OnSetCursor											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to set the status text for the
//	item under the mouse

BOOL CLinkHistoryList::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	CFrameWnd *		pFrameWnd = GetTopLevelFrame();
	
	if (pFrameWnd != NULL)
	{
		if ( (nHitTest == HTCLIENT) && (pWnd == this) )
		{
			POINT			pt;
			
			//	Determine if the mouse is over an item for which we should display
			//	the URL as the status text.
			if ( ::GetCursorPos(&pt) )
			{
				ScreenToClient(&pt);

				LV_HITTESTINFO		lvhti;
				lvhti.pt = pt;
		
				int nIndex = GetListCtrl().HitTest(&lvhti);
		
				if ( (nIndex != -1) && (lvhti.flags & LVHT_ONITEM) )
				{
					ShowStatusText(lvhti.iItem);
				}
			}
		}

		if ( !GetStatusURLShown() )
		{
			// Restore default status bar prompt.
			pFrameWnd->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
	}

	return CListView::OnSetCursor(pWnd, nHitTest, message);
}


// ---------------------------------------------------------------------------
//		* GetDisplayName									[Static Public]
// ---------------------------------------------------------------------------
//	Returns either the name or the URL to be used as the display name.
//
//	Note that this sometimes calls GetDisplayURL, which returns a CString &
//	that could be changed if this method or GetDisplayURL is called again
//	(szDisplayURL is static). As such either use or store a copy of the
//	return value immediately.

const CString &
CLinkHistoryList::GetDisplayName(LPUrlItemData pItemData)
{
	//	If there's no name, then use the URL as the name
	return (pItemData->m_szLinkName == "") ?
		   GetDisplayURL(pItemData) : pItemData->m_szLinkName;
}


// ---------------------------------------------------------------------------
//		* GetDisplayURL										[Static Public]
// ---------------------------------------------------------------------------
//	Returns either the raw URL or a pretty version for ads (given that
//	we're not doing clickbase).
//
//	Note that this returns a CString & that could be changed if this
//	method is called again (szDisplayURL is static). As such either use
//	or store a copy of the return value immediately.

const CString &
CLinkHistoryList::GetDisplayURL(LPUrlItemData pItemData)
{
	static CString szDisplayURL;

	if (!pItemData)
	{
		szDisplayURL.Empty();
		return szDisplayURL;
	}
	else if (pItemData->m_type == LinkHistoryManager::lt_Ad)
	{
		//	Remove everything up to and including "url="
		int		pos = pItemData->m_szURL.Find("url=");
		
		if (pos == -1)
			szDisplayURL = pItemData->m_szURL;
		else
			szDisplayURL = pItemData->m_szURL.Right(pItemData->m_szURL.GetLength() - pos - 4);

		//	Remove everything after "&distributorid"
		pos = szDisplayURL.Find("&distributorid");
		if (pos >= 0)
			szDisplayURL = szDisplayURL.Left(pos);

		//	Unescape for nicer display
		szDisplayURL = UnEscapeURL(szDisplayURL);

		return szDisplayURL;
	}
	else
	{
		return pItemData->m_szURL;
	}
}


// ---------------------------------------------------------------------------
//		* GetDisplayDateOrStatus									[Public]
// ---------------------------------------------------------------------------
//	Returns the date to display. Uses szDateString for storage only if
//	necessary. Returned CString & may or may not be szDateString.

const CString &
CLinkHistoryList::GetDisplayDateOrStatus(LPUrlItemData pItemData, CString & szDateString)
{
	CString &	szRetDateString = szDateString;
	
	switch (pItemData->m_status)
	{
	//	First three cases immediately return the appropriate string
	case LinkHistoryManager::ls_Remind:
		return m_szStatusRemind;

	case LinkHistoryManager::ls_Bookmarked:
		return m_szStatusBookmarked;

	case LinkHistoryManager::ls_NotVisited:
		return m_szStatusNotDisplayed;

	case LinkHistoryManager::ls_Attempted:
		{
			CString		szTempDateString;
			::TimeDateStringMT(szTempDateString.GetBuffer(256), pItemData->m_lLastVisitTime, TRUE);
			szTempDateString.ReleaseBuffer();

			szRetDateString = m_szStatusAttempted;
			szRetDateString += szTempDateString;
		}
		break;

	case LinkHistoryManager::ls_None:
	default:
		if (pItemData->m_lLastVisitTime == 0)
		{
			//	This shouldn't happen, if they visited the entry there should be a valid time.
			ASSERT(0);

			//	Return not displayed, since ls_NotVisited should probably be the value for m_status
			return m_szStatusNotDisplayed; 
		}

		::TimeDateStringMT(szRetDateString.GetBuffer(256), pItemData->m_lLastVisitTime, TRUE);
		szRetDateString.ReleaseBuffer();
		break;
	}

	return szRetDateString;
}


// ---------------------------------------------------------------------------
//		* OnGetDispInfo											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to provide the text and image as necessary.
//	This allows us to store the information ourselves separately without
//	wasting memory by duplicating the information in the list.

void CLinkHistoryList::OnGetDispInfo(NMHDR * pnmh, LRESULT * pResult)
{
	LV_DISPINFO *	plvdi = reinterpret_cast<LV_DISPINFO *>(pnmh);
	LPUrlItemData	pItemData = reinterpret_cast<LPUrlItemData>(plvdi->item.lParam);

	//	Check for bad case or when we're running light (in which case there
	//	is a single item, but it's *not* a LPUrlItemData).
	if (pItemData == NULL)
		return;
	
	if (plvdi->item.mask & LVIF_TEXT)
	{
		switch (plvdi->item.iSubItem)
		{
		case 0:
			plvdi->item.pszText[0] = '\0';
			break;
		
		case 1:
			//	If there's no name, then use the URL as the name
			strncpy(plvdi->item.pszText, GetDisplayName(pItemData), plvdi->item.cchTextMax);
			break;

		case 2:
			{
				CString		tempString;
				strncpy(plvdi->item.pszText, GetDisplayDateOrStatus(pItemData, tempString), plvdi->item.cchTextMax);
			}
			break;
		}
	}
	
	if (plvdi->item.mask & LVIF_IMAGE)
	{
		switch (pItemData->m_type)
		{
		case LinkHistoryManager::lt_Ad:
			//	To do: add true thumbnail ad support
			plvdi->item.iImage = pItemData->m_nIconIndex;
			break;

		case LinkHistoryManager::lt_Unknown:
			plvdi->item.iImage = 0;
			break;

		default:
			//	Types starting with http: are 1 based, convert to 0 based image index
			plvdi->item.iImage = pItemData->m_type - 1;
			break;
		}
	}
	
	*pResult = 0;
}


// ---------------------------------------------------------------------------
//		* OnUpdateNeedSelection									[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to enable commands based on whether or
//	not there's a selection.

void CLinkHistoryList::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if ( UsingFullFeatureSet() )
		pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 );
}


// ---------------------------------------------------------------------------
//		* OnUpdateNeedSingleSelection							[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to enable commands based on whether or
//	not there's exactly one item selected.

void CLinkHistoryList::OnUpdateNeedSingleSelection(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if ( UsingFullFeatureSet() )
		pCmdUI->Enable( GetListCtrl().GetSelectedCount() == 1 );
}


// ---------------------------------------------------------------------------
//		* OnUpdateCopy											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to enable copy based on whether or
//	not there's a selection.

void CLinkHistoryList::OnUpdateCopy(CCmdUI* pCmdUI)
{
	if ( UsingFullFeatureSet() )
		pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 );
	else
		pCmdUI->Enable(FALSE);
}


// ---------------------------------------------------------------------------
//		* OnUpdateEditSelectAll									[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to enable select all based on whether or
//	not there are any items.

void CLinkHistoryList::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( UsingFullFeatureSet() && (GetListCtrl().GetItemCount() > 0) );
}


// ---------------------------------------------------------------------------
//		* OnEditCopy											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to copy the URLs of the selected Link History
//	items.

void CLinkHistoryList::OnEditCopy()
{
	if ( !UsingFullFeatureSet() )
	{
		FeatureNotInFree();
	}
	else
	{
		CString			szSelectedText;
		LV_ITEM			lvi;
		LPUrlItemData	pListItemData;
		int				itemIndex = GetListCtrl().GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

		while (itemIndex != -1)
		{
			lvi.mask = LVIF_PARAM;
			lvi.iItem = itemIndex;
			lvi.iSubItem = 0;

			itemIndex = GetListCtrl().GetNextItem(itemIndex, LVNI_ALL | LVNI_SELECTED);

			if ( GetListCtrl().GetItem(&lvi) )
			{
				pListItemData = (LPUrlItemData) lvi.lParam;
				
				if (pListItemData->m_szLinkName != "")
				{
					szSelectedText += pListItemData->m_szLinkName;
					szSelectedText += " ";
				}
				szSelectedText += "<";
				szSelectedText += pListItemData->m_szURL;
				szSelectedText += ">";

				if (itemIndex != -1)
				{
					szSelectedText += "\r\n";
				}
			}
		}

		CopyTextToClipboard( szSelectedText, GetSafeHwnd() );
	}
}


// ---------------------------------------------------------------------------
//		* PreTranslateMessage										[Public]
// ---------------------------------------------------------------------------
//	Converts pressing enter key to double-clicking the selection

BOOL CLinkHistoryList::PreTranslateMessage(MSG* pMsg)
{
	if ( (WM_KEYDOWN == pMsg->message) && (VK_RETURN == pMsg->wParam) )
	{
		OnLButtonDblClk(0, CPoint(-1, -1));
		return TRUE;
	}

	// do default processing
	return CListView::PreTranslateMessage(pMsg);
}


// ---------------------------------------------------------------------------
//		* DrawItem												[Protected]
// ---------------------------------------------------------------------------
//	Override to draw the image correctly, and do full row selection

void CLinkHistoryList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	//	Constants
	const int		nOffsetFirst = 2;
	const int		nColumnOffset = 6;
	const UINT		nFormat = DT_LEFT | DT_SINGLELINE | DT_NOPREFIX |
							  DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS;

	static _TCHAR	szBuff[MAX_PATH];
	
	int				nItem = lpDrawItemStruct->itemID;
	CDC *			pOriginalDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	int				nSavedDC = pOriginalDC->SaveDC();

	//	Default to drawing in the original DC
	CDC *			pDC = pOriginalDC;
	CDC *			pAltDC = pOriginalDC;
	
	//	Get rects
	CRect		rcBounds, rcLabel, rcIcon, rcClient;
	GetListCtrl().GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetListCtrl().GetItemRect(nItem, rcLabel, LVIR_LABEL);	
	GetListCtrl().GetItemRect(nItem, rcIcon, LVIR_ICON);

	GetClientRect(&rcClient);
	rcClient.top = rcBounds.top;
	rcClient.bottom = rcBounds.bottom;
	
	if ( !UsingFullFeatureSet() )
	{
		//	Just draw the light message and then return immediately
		int nRetLen = GetListCtrl().GetItemText(nItem, 0, szBuff, sizeof(szBuff));
		if (nRetLen > 0)
		{
			rcClient.left += nColumnOffset;
			
			//	Make sure colors are set up correctly for drawing
			pOriginalDC->FillSolidRect(rcClient, ::GetSysColor(COLOR_WINDOW));
			pOriginalDC->SelectStockObject(BLACK_PEN);		// Set default pen
			pOriginalDC->SelectStockObject(ANSI_VAR_FONT);	// Choose the default 'skinny' font
			pOriginalDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));

			pOriginalDC->DrawText(szBuff, -1, rcClient, nFormat);
		}

		// Restore dc	
		pOriginalDC->RestoreDC(nSavedDC);
		return;
	}
	
	CRect			rcItem(lpDrawItemStruct->rcItem);
	UINT			uiFlags = ILD_TRANSPARENT;
	BOOL			bFocus = (GetFocus() == this);
	LPUrlItemData	pListItemData = reinterpret_cast<LPUrlItemData>(lpDrawItemStruct->itemData);

	if (pListItemData == NULL)
	{
		ASSERT(0);
		return;
	}

	//	Setup the offscreen bitmap
	CRect		rcDestItem = rcClient;
	int			nBitmapHeight = rcBounds.Height();
	int			nBitmapWidth = rcBounds.Width();
	CDC			dcAlt;
	CBitmap		bm;
	HBITMAP		hSaveOldBitmap = NULL;

	//	Attempt to create a compatible device context
	BOOL		bUseOffscreenBitmap = dcAlt.CreateCompatibleDC(pOriginalDC);
	
	if (bUseOffscreenBitmap)
	{
		if (rcClient.Width() > nBitmapWidth)
			nBitmapWidth = rcClient.Width(); // this extends the bitmap past the righ-most column when needed
	
		//	Attempt to create an offscreen bitmap
		bUseOffscreenBitmap = bm.CreateCompatibleBitmap(pOriginalDC, nBitmapWidth, nBitmapHeight);
	}

	if (bUseOffscreenBitmap)
	{
		HBITMAP hAltBitmap = HBITMAP(bm);
		hSaveOldBitmap = HBITMAP(dcAlt.SelectObject(hAltBitmap));
		ASSERT(hSaveOldBitmap);

		pAltDC = &dcAlt;
		pDC = pAltDC;

		//	Offset the rects so that they match the coordinate system of the bitmap we created
		int nX = -(rcBounds.left);
		int nY = -(rcBounds.top);
		rcBounds.OffsetRect(nX, nY);
		rcLabel.OffsetRect(nX, nY);
		rcIcon.OffsetRect(nX, nY);
		rcClient.OffsetRect(nX, nY);
		rcItem.OffsetRect(nX, nY);
	}

	//	Make sure that there is no clipping initially
	pDC->SelectClipRgn(NULL);
	
	pDC->FillSolidRect(rcClient, ::GetSysColor(COLOR_WINDOW));
	pDC->SelectStockObject(BLACK_PEN);		// Set default pen
	pDC->SelectStockObject(ANSI_VAR_FONT);	// Choose the default 'skinny' font

	//	Get item data
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.pszText = szBuff;
	lvi.cchTextMax = sizeof(szBuff);
	lvi.stateMask = 0xFFFF;		//	Get all state flags
	GetListCtrl().GetItem(&lvi);

	BOOL bSelected = (bFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
	bSelected = bSelected || (lvi.state & LVIS_DROPHILITED);

	//	Set colors if item is selected
	int			nColZeroRight = rcBounds.left + GetListCtrl().GetColumnWidth(0);
	CRect		rcAll = rcBounds;
	rcAll.right = rcClient.right;

	CRgn		rgnClip;
	COLORREF	backColor = m_clrBkgnd;
	if (bSelected)
	{
		if (GetFocus() == this)
		{
			pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			backColor = ::GetSysColor(COLOR_HIGHLIGHT);
		}
		else
		{
			pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
			backColor = RGB(0xC0, 0xC0, 0xC0);
		}

		//	FillSolidRect calls SetBkColor, so we don't bother
		pDC->FillSolidRect(rcAll, backColor);
	}
	else
	{
		//	Not selected, choose the normal colors. FillSolidRect call with COLOR_WINDOW
		//	above (before if/else) called SetBkColor, so we don't bother 
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	}

	//	Draw normal and overlay icon
	CImageList *	pImageList = GetListCtrl().GetImageList(LVSIL_SMALL);
	if (pImageList)
	{
		UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
		if (rcItem.left<rcItem.right-1)
		{
			//	Set color and mask for the icon
			if (bSelected)
			{
				uiFlags |= ILD_BLEND50;
			}
			else
			{
				uiFlags |= ILD_NORMAL;
			}
			
			rcItem = rcBounds;
			rcItem.right = nColZeroRight;
			rgnClip.CreateRectRgnIndirect(&rcItem);	
			pDC->SelectClipRgn(&rgnClip);
			rgnClip.DeleteObject();	
			
			//	When we do ad thumbnails, we may want to draw them differently
			//	so that they look good when selected.
			ImageList_DrawEx( pImageList->m_hImageList, lvi.iImage, pDC->m_hDC,
							  rcIcon.left, rcIcon.top,
							  LinkHistoryManager::kLinkTypeIconSize, LinkHistoryManager::kLinkTypeIconPaddedHeight,
							  m_clrBkgnd, backColor, uiFlags | nOvlImageMask );
		}
	}

	//	Don't clip for drawing for the rest of this method
	pDC->SelectClipRgn(NULL);

	//	Draw column labels, only draws 0th column if we're not drawing the "Type" image
	rcItem = rcLabel;

	if ( !m_bDrawTypeIcon || (pImageList == NULL) )
	{
		rcLabel.left += nOffsetFirst;
		rcLabel.right -= nOffsetFirst;
		
		//	Determine which string to use
		UINT			nStrID;
		switch (pListItemData->m_type)
		{
		case LinkHistoryManager::LinkTypeEnum::lt_Ad:					nStrID = IDS_LINK_HISTORY_AD_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_Http:					nStrID = IDS_LINK_HISTORY_HTTP_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_Ftp:					nStrID = IDS_LINK_HISTORY_FTP_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_Mail:					nStrID = IDS_LINK_HISTORY_MAIL_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_DirectoryServices:	nStrID = IDS_LINK_HISTORY_DIRSERV_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_Setting:				nStrID = IDS_LINK_HISTORY_SETTING_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_File:					nStrID = IDS_LINK_HISTORY_FILE_LINK_NAME; break;
		case LinkHistoryManager::LinkTypeEnum::lt_Unknown:
		default:
			 nStrID = IDS_LINK_HISTORY_UNKOWN_LINK_NAME;
			 break;
		}

		pDC->DrawText(CRString(nStrID), -1, rcLabel, nFormat);
	}

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	//	Draw the name column. Access the info directly to be sure to work
	//	under 95 and avoid unnecessary copying of the strings.
	if ( GetListCtrl().GetColumn(1, &lvc) )
	{
		rcItem.left = rcItem.right;
		rcItem.right += lvc.cx;
		
		rcLabel = rcItem;
		rcLabel.left += nColumnOffset;
		rcLabel.right -= nColumnOffset;

		pDC->DrawText(GetDisplayName(pListItemData), -1, rcLabel, nFormat);
	}

	//	Draw the date column. Access the info directly to be sure to work
	//	under 95 and avoid unnecessary copying of the strings.
	if ( GetListCtrl().GetColumn(2, &lvc) )
	{
		rcItem.left = rcItem.right;
		rcItem.right += lvc.cx;
		
		rcLabel = rcItem;
		rcLabel.left += nColumnOffset;
		rcLabel.right -= nColumnOffset;
		
		CString		szTempDate;
		pDC->DrawText(GetDisplayDateOrStatus(pListItemData, szTempDate), -1, rcLabel, nFormat);
	}

	//	Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && bFocus)
	{
		pDC->DrawFocusRect(rcAll);
	}

	if (bUseOffscreenBitmap)
	{
		pOriginalDC->BitBlt( rcDestItem.left, rcDestItem.top,
							 nBitmapWidth, nBitmapHeight,
							 pAltDC, rcClient.left, rcClient.top,
							 SRCCOPY );
		ASSERT(hSaveOldBitmap);
		::SelectObject(dcAlt.GetSafeHdc(), hSaveOldBitmap);
		hSaveOldBitmap = NULL;
	}

	// Restore dc	
	pOriginalDC->RestoreDC(nSavedDC);

	if ( m_bSelectionChangingKeyHit && (GetListCtrl().GetSelectedCount() == 1) )
	{
		//	Show the status text for the new singly selected item. Must do this
		//	here or some other place that processes after the selection has been
		//	changed (in OnKeyDown the new selection is not in effect yet).
		int		nSelItemIndex = GetListCtrl().GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		
		if (nSelItemIndex != -1)
		{
			ShowStatusText(nSelItemIndex);
		}
		m_bSelectionChangingKeyHit = false;
	}
}


// ---------------------------------------------------------------------------
//		* OnPaint												[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to support full row selection

void CLinkHistoryList::OnPaint() // WM_PAINT 
{	
	//	In order to do full row select, we need to extend the update region
	//	so that we can paint a selection all the way to the right
	CRect rcBounds;
	GetListCtrl().GetItemRect(0, rcBounds, LVIR_BOUNDS);
	CRect rcClient;
	GetClientRect(&rcClient);

	if (rcBounds.right < rcClient.right)
	{
		CPaintDC dc(this);
		CRect	rcClip;
		dc.GetClipBox(rcClip);

		rcClip.left = __min(rcBounds.right-1, rcClip.left);
		rcClip.right = rcClient.right;
		InvalidateRect(rcClip, FALSE);
	}

	CListView::OnPaint();
}


// ---------------------------------------------------------------------------
//		* OnSetBkColor											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to ensure that m_clrBkgnd stays up to date.

LRESULT CLinkHistoryList::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	m_clrBkgnd = static_cast<COLORREF>(lParam);
	return Default();
}


// ---------------------------------------------------------------------------
//		* LaunchSelectedItem										[Public]
// ---------------------------------------------------------------------------
//	Launches the selected item. Only one item is allowed because of
//	a limitation of LaunchURL and external links.

void CLinkHistoryList::OnLaunchSelectedItem()
{
	if ( !UsingFullFeatureSet() )
	{
		FeatureNotInFree();
	}
	else
	{
		INT		iSelected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
		if (iSelected >= 0)
		{
			LV_ITEM lvi;
			lvi.mask = LVIF_PARAM;
			lvi.iItem = GetListCtrl().GetNextItem(-1, LVNI_SELECTED);
			lvi.iSubItem = 0;

			if ( GetListCtrl().GetItem(&lvi) )
			{
				LPUrlItemData	pListItemData = (LPUrlItemData) lvi.lParam;
				
				if (pListItemData != NULL)
				{
					bool			bIsAd = (pListItemData->m_type == LinkHistoryManager::lt_Ad);
					const char *	szAdID = bIsAd ? static_cast<const char *>(pListItemData->m_szAdID) : NULL;
					
					if ( LinkHistoryManager::Instance()->ShouldLaunchURL(pListItemData->m_szURL, bIsAd, NULL, szAdID) )
					{
						BOOL	bLaunchGood = LaunchURLNoHistory(pListItemData->m_szURL);

						//	Update the LinkHistoryManager's info directly to avoid an unnecessary search
						LinkHistoryManager::Instance()->UpdateVisitedURLInfo(pListItemData, bLaunchGood);

						//	Dirty the LinkHistoryManager so that it saves at idle
						LinkHistoryManager::Instance()->SetDirtyState(true);
					}
				}
			}
		}
	}
}


// ---------------------------------------------------------------------------
//		* OnRemoveSelectedItems										[Public]
// ---------------------------------------------------------------------------
//	Removes the selected items and tells the LinkHistoryManager to
//	do the same (LinkHistoryManager frees the memory).

void CLinkHistoryList::OnRemoveSelectedItems()
{
	if ( !UsingFullFeatureSet() )
	{
		FeatureNotInFree();
	}
	else
	{
		int				itemIndex;
		LV_ITEM			lvi;
		LPUrlItemData	pListItemData;

		itemIndex = GetListCtrl().GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

		if (itemIndex != -1)
		{
			RedrawRemovedItems(itemIndex);

			do
			{
				//	Delete the item from the link history manager
				lvi.mask = LVIF_PARAM;
				lvi.iItem = itemIndex;
				lvi.iSubItem = 0;

				if ( GetListCtrl().GetItem(&lvi) )
				{
					pListItemData = (LPUrlItemData) lvi.lParam;
					
					LinkHistoryManager::Instance()->RemoveURL(pListItemData);
				}

				//	Delete the list control item
				GetListCtrl().DeleteItem(itemIndex);

				itemIndex = GetListCtrl().GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
			} while (itemIndex != -1);
		}
	}
}


// ---------------------------------------------------------------------------
//		* OnChangeSelItemsStatusToRemind							[Public]
// ---------------------------------------------------------------------------
//	Allows user to change the status of the selected item or items
//	to Remind Me.

void CLinkHistoryList::OnChangeSelItemsStatusToRemind()
{
	OnChangeSelItemsStatus(LinkHistoryManager::ls_Remind);
}


// ---------------------------------------------------------------------------
//		* OnChangeSelItemsStatusToBookmarked						[Public]
// ---------------------------------------------------------------------------
//	Allows user to change the status of the selected item or items
//	to bookmarked.

void CLinkHistoryList::OnChangeSelItemsStatusToBookmarked()
{
	OnChangeSelItemsStatus(LinkHistoryManager::ls_Bookmarked);
}


// ---------------------------------------------------------------------------
//		* OnChangeSelItemsStatusToNone								[Public]
// ---------------------------------------------------------------------------
//	Allows user to change the status of the selected item or items
//	to none (i.e. sort by date). If there is no date stamp,
//	then the implementation actually changes the status to ls_NotDisplayed
//	(see below implementation).

void CLinkHistoryList::OnChangeSelItemsStatusToNone()
{
	OnChangeSelItemsStatus(LinkHistoryManager::ls_None);
}


// ---------------------------------------------------------------------------
//		* OnChangeSelItemsStatus									[Public]
// ---------------------------------------------------------------------------
//	Implements status changing for OnChangeSelItemsStatusToBookmarked and
//	OnChangeSelItemsStatusToNone. Dirties the Link History Manager,
//	resorts, and updates the list display as necessary.

void CLinkHistoryList::OnChangeSelItemsStatus(
	LinkHistoryManager::LinkStatusEnum	status)
{
	if ( !UsingFullFeatureSet() )
	{
		FeatureNotInFree();
	}
	else
	{
		LV_ITEM									lvi;
		LPUrlItemData							pListItemData;
		LinkHistoryManager::LinkStatusEnum		statusToSet;
		long									lNumberItemsChanged = 0;
		
		for ( int itemIndex = GetListCtrl().GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
			  itemIndex != -1;
			  itemIndex = GetListCtrl().GetNextItem(itemIndex, LVNI_ALL | LVNI_SELECTED) )
		{
			lvi.mask = LVIF_PARAM;
			lvi.iItem = itemIndex;
			lvi.iSubItem = 0;

			if ( GetListCtrl().GetItem(&lvi) )
			{
				pListItemData = (LPUrlItemData) lvi.lParam;
				
				ASSERT(pListItemData);

				if (pListItemData != NULL)
				{
					//	Determine the appropriate status to set
					statusToSet = status;
					if (statusToSet == LinkHistoryManager::ls_None)
					{
						//	Handle special case of no visit date stamp
						if (pListItemData->m_lLastVisitTime == 0)
							statusToSet = LinkHistoryManager::ls_NotVisited;
						else
							statusToSet = LinkHistoryManager::ls_None;
					}

					if (pListItemData->m_status != statusToSet)
					{
						pListItemData->m_status = statusToSet;
						lNumberItemsChanged++;
					}
					
					//	This link was touched, update touch time accordingly
					pListItemData->m_lLastTouchTime = time(NULL);
				}
			}
		}
		
		if (lNumberItemsChanged > 0)
		{
			LinkHistoryManager::Instance()->SetDirtyState(true);
			
			//	Items were changed, if the status is remind then tell the LinkHistoryManager to remind
			if (status == LinkHistoryManager::ls_Remind)
				LinkHistoryManager::Instance()->SetShouldRemind();

		
			//	Resort the items immediately if necessary (if this ever proves to be too slow,
			//	we'll need to wait to do this at idle time)
			if ( ResortItemsIfSortedByDate() )
			{
				//	Scroll the first selected item into view (so that the selected item
				//	or items don't suddenly seem to disappear).
				int		firstItemIndex = GetListCtrl().GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
				if (firstItemIndex != -1)
					GetListCtrl().EnsureVisible(firstItemIndex, FALSE);
			}

			//	Make sure list refreshes, because the information has changed
			Invalidate();
		}
	}
}


// ---------------------------------------------------------------------------
//		* OnEditSelectAll										[Public]
// ---------------------------------------------------------------------------
//	Implements selecting all the items in the Link History list.

void CLinkHistoryList::OnEditSelectAll()
{
	if ( !UsingFullFeatureSet() )
	{
		FeatureNotInFree();
	}
	else
	{
		int		nCount = GetListCtrl().GetItemCount();

		for (int i = 0; i < nCount; i++)
			GetListCtrl().SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);

		//	Set the focus on the first item.
		GetListCtrl().SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
	}
}


// ---------------------------------------------------------------------------
//		* SortCompareFunc
// ---------------------------------------------------------------------------
//	Callback routine for sorting list

int CALLBACK
CLinkHistoryList::SortCompareFunc(
	LPARAM		lParam1, 
	LPARAM		lParam2,
	LPARAM		lParamSort)
{
	INT		nSortResult = 0;
	
	if ( UsingFullFeatureSet() && lParam1 && lParam2 && lParamSort)
	{
		LPUrlItemData		pListItemData1 = (LPUrlItemData) lParam1;
		LPUrlItemData		pListItemData2 = (LPUrlItemData) lParam2;
		CLinkHistoryList *	pLinkHistoryList = reinterpret_cast<CLinkHistoryList *>(lParamSort);
		INT					iSortColumn = pLinkHistoryList->m_iSortColumn;
		BOOL				bReverseOrder = pLinkHistoryList->m_bReverseOrder;

		if (iSortColumn == sc_Type)
		{
			if (pListItemData1->m_type == pListItemData2->m_type)
			{
				//	Type is the same for both items, so sort by the secondary criteria
				iSortColumn = pLinkHistoryList->m_iPrevSortColumn;
				bReverseOrder = pLinkHistoryList->m_bPrevReverseOrder;
			}
			else
			{
				//	Lower type should sort to the top when the sort order is normal
				nSortResult = pListItemData1->m_type - pListItemData2->m_type;
			}
		}

		if (iSortColumn == sc_Name)
		{
			const CString &		szLeftName = GetDisplayName(pListItemData1);
			const CString &		szRightName = GetDisplayName(pListItemData2);
			
			nSortResult = szLeftName.CompareNoCase(szRightName);
		}

		if (iSortColumn == sc_DateVisited)
		{
			if (pListItemData1->m_status == pListItemData2->m_status)
			{
				//	More recent dates should sort to the top when the sort order is normal
				//	So prefer higher values and do the subtraction in reverse order
				nSortResult = pListItemData2->m_lLastVisitTime - pListItemData1->m_lLastVisitTime;
			}
			else
			{
				//	Lower status number should sort to the top when the sort order is normal
				nSortResult = pListItemData1->m_status - pListItemData2->m_status;
			}
		}

		if (bReverseOrder)		//	reverse sort
			nSortResult = -nSortResult;
	}

	return nSortResult;
}
