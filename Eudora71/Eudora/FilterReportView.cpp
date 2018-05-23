// FilterReportView.cpp : implementation file
//
// Copyright (c) 1993-2000 by QUALCOMM, Incorporated
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

//

#include "stdafx.h"

#include <QCUtils.h>	// for TimeDateStringMT

#include "eudora.h"
#include "FilterReportView.h"
#include "FilterReportWazooWnd.h"

#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

extern QCMailboxDirector	g_theMailboxDirector;

#include "DebugNewHelpers.h"


int CALLBACK SortByMailbox(
LPARAM lParam1,
LPARAM lParam2, 
LPARAM lParamSort)
{
	CFilterReportView::CEntry*	pEntry1;
	CFilterReportView::CEntry*	pEntry2;
	
	pEntry1 = ( CFilterReportView::CEntry* ) lParam1;
	pEntry2 = ( CFilterReportView::CEntry* ) lParam2;
	
	if( lParamSort )
	{
		return ( stricmp( pEntry1->m_szMailbox, pEntry2->m_szMailbox ) );
	}
	else
	{
		return ( stricmp( pEntry2->m_szMailbox, pEntry1->m_szMailbox ) );
	}
}


int CALLBACK SortByCount(
LPARAM lParam1,
LPARAM lParam2, 
LPARAM lParamSort)
{
	CFilterReportView::CEntry*	pEntry1;
	CFilterReportView::CEntry*	pEntry2;
	
	pEntry1 = ( CFilterReportView::CEntry* ) lParam1;
	pEntry2 = ( CFilterReportView::CEntry* ) lParam2;
	
	if( lParamSort )
	{
		return ( pEntry1->m_uCount - pEntry2->m_uCount );
	}
	else
	{
		return ( pEntry2->m_uCount - pEntry1->m_uCount );
	}
}


int CALLBACK SortByTime(
LPARAM lParam1,
LPARAM lParam2, 
LPARAM lParamSort)
{
	CFilterReportView::CEntry*	pEntry1;
	CFilterReportView::CEntry*	pEntry2;
	
	pEntry1 = ( CFilterReportView::CEntry* ) lParam1;
	pEntry2 = ( CFilterReportView::CEntry* ) lParam2;
	
	CTime	time1( pEntry1->m_Time );
	CTime	time2( pEntry2->m_Time );
	
	if( !lParamSort )
	{
		// swap 'em
		CTime time3;

		time3 = time2;
		time2 = time1;
		time1 = time3;
	}

	if( time1 < time2 )
	{
		return -1;
	}
	else if ( time2 < time1 )
	{
		return 1;
	}
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CFilterReportView

IMPLEMENT_DYNCREATE(CFilterReportView, CListView)

CFilterReportView*	CFilterReportView::s_pOneAndOnlyView = NULL;

CFilterReportView::CFilterReportView()
{
	ASSERT(s_pOneAndOnlyView == NULL);
	s_pOneAndOnlyView = this;

	m_iSortIndex = 2;
	m_bAscendingOrder = TRUE;
}

CFilterReportView::~CFilterReportView()
{
	s_pOneAndOnlyView = NULL;
}

/*static*/ CFilterReportView* CFilterReportView::GetFilterReportView()
{
	if (!s_pOneAndOnlyView)
		CFilterReportWazooWnd::CreateFilterReportView();

	return s_pOneAndOnlyView;
}

BEGIN_MESSAGE_MAP(CFilterReportView, CListView)
	//{{AFX_MSG_MAP(CFilterReportView)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterReportView drawing

void CFilterReportView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFilterReportView diagnostics

#ifdef _DEBUG
void CFilterReportView::AssertValid() const
{
	CListView::AssertValid();
}

void CFilterReportView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

BOOL CFilterReportView::AddEntry(UINT uCount, LPCSTR szMailbox, time_t tNow)
{
	CEntry* pEntry = DEBUG_NEW_MFCOBJ_NOTHROW CEntry;

	if( pEntry == NULL )
		return FALSE;

	pEntry->m_szMailbox = szMailbox;
	pEntry->m_uCount = uCount;
	pEntry->m_Time = tNow;

	( ( CEudoraApp* ) AfxGetApp() )->AddFilterListEntry( pEntry );
	
	return AddEntry( pEntry );
}



BOOL CFilterReportView::AddEntry(CEntry* pEntry)
{							
	CListCtrl&	theCtrl = GetListCtrl();
	char		szTime[256];
	LV_ITEM		theItem;
	INT			iRet;
	CString		szCount;

	// format the strings
	*szTime = 0;

	if( pEntry->m_Time > 0L )
	{
		::TimeDateStringMT( szTime, pEntry->m_Time, TRUE );
	}

	szCount.Format( "%u", pEntry->m_uCount );

	// setup the item structure
	theItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	theItem.iItem = theCtrl.GetItemCount();	
	theItem.iSubItem = 0;
	theItem.pszText = ( char* ) ( const char* ) ( pEntry->m_szMailbox );
	theItem.iImage = QCMailboxImageList::IMAGE_NORMAL_MBOX;
	theItem.lParam = ( LPARAM ) pEntry;

	// to do:	
	// IMAGE_TRASH_MBOX
	// IMAGE_IN_MBOX;
	//IMAGE_OUT_MBOX;
	
	// insert the item
	iRet = theCtrl.InsertItem( &theItem );

	if( iRet < 0 )
		return FALSE;

	//
	// Set the "focus" to the newly added item and make sure it
	// is visible in the display.
	//
	theCtrl.SetItemState(iRet, LVIS_FOCUSED, LVIS_FOCUSED);
	theCtrl.EnsureVisible(iRet, TRUE);

	// set the message column
	theCtrl.SetItemText( theItem.iItem, 1, szCount );

	// set the time column
	theCtrl.SetItemText( theItem.iItem, 2, szTime );

	return TRUE;
}
  

/////////////////////////////////////////////////////////////////////////////
// CFilterReportView message handlers

void CFilterReportView::OnInitialUpdate() 
{
	CListCtrl		&theCtrl = GetListCtrl();
	LONG			lStyle;
	INT				iRet;
	CDC*			pDC;
	UINT			wMailbox;
	UINT			wMessages;
	UINT			wTime;
	CString			szTemp;
	CImageList		theList;
	const CObList*	pFilterList;
	POSITION		pos;
	CEntry*			pEntry;

	if (theCtrl.GetImageList(LVSIL_NORMAL))
	{
		ASSERT(0);
		return;			// guard against bogus double initializations
	}

	CListView::OnInitialUpdate();

	// set the control to report style with icons
	lStyle = GetWindowLong( theCtrl.GetSafeHwnd(), GWL_STYLE );
	lStyle &= ~( LVS_ICON | LVS_SMALLICON | LVS_LIST );
	lStyle |= LVS_REPORT | LVS_ICON;
	SetWindowLong( theCtrl.GetSafeHwnd(), GWL_STYLE, lStyle );
	
	// create the imagelist
	if (! m_ImageList.Load() )
	{
		return;
	}

	theCtrl.SetImageList( &m_ImageList, LVSIL_NORMAL );
	theCtrl.SetImageList( &m_ImageList, LVSIL_SMALL );

	pDC = theCtrl.GetDC();
	
	// setup the columns
	
	szTemp.LoadString( IDS_FILTER_OPEN_MAILBOX );
	wMailbox = 4 * ( pDC->GetTextExtent( szTemp ) ).cx ;
	iRet = theCtrl.InsertColumn( 0, szTemp, LVCFMT_LEFT, wMailbox );	
	if( iRet < 0 )
	{
		theCtrl.ReleaseDC( pDC );
		return;
	}

	szTemp.LoadString( IDS_MESSAGES );
	wMessages = ( pDC->GetTextExtent( szTemp ) ).cx;
	iRet = theCtrl.InsertColumn( 1, szTemp, LVCFMT_RIGHT, wMessages );
	if( iRet < 0 )
	{
		theCtrl.ReleaseDC( pDC );
		return;
	}
	
	szTemp.LoadString( IDS_TIME );
	wTime = 4 * ( pDC->GetTextExtent( szTemp ) ).cx;
	iRet = theCtrl.InsertColumn( 2, szTemp, LVCFMT_LEFT, wTime );
	theCtrl.ReleaseDC( pDC );

	// add any previous filter entries
	pFilterList = &( ( ( CEudoraApp* ) AfxGetApp() )->GetFilterList() );
	pos = pFilterList->GetHeadPosition();

	while( pos != NULL )
	{
		pEntry = ( CEntry* ) pFilterList->GetNext( pos );
		AddEntry( pEntry );
	}
} 



void CFilterReportView::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	BOOL			bResult;
	CListCtrl&		theCtrl = GetListCtrl();
	NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)pNMHDR;
	BOOL			bSaveOrder;

	if( pNMListView->iItem == -1 )
	{
		// the user clicked on a header button
			
		bResult = FALSE;
		
		bSaveOrder = m_bAscendingOrder;

		if(	m_iSortIndex == pNMListView->iSubItem )
		{
			// if the index is the same, toggle between 
			// ascending and descending order
			m_bAscendingOrder = !m_bAscendingOrder;
		}
		else
		{					
			// always do ascending first
			m_bAscendingOrder = TRUE;
		}

		if( pNMListView->iSubItem == 0 )
		{
			// sort by mailbox
			bResult = theCtrl.SortItems( SortByMailbox, m_bAscendingOrder );
		}
		else if( pNMListView->iSubItem == 1 )
		{
			// sort by count
			bResult = theCtrl.SortItems( SortByCount, m_bAscendingOrder );
		}
		else if( pNMListView->iSubItem == 2 )
		{
			// sort by time
			bResult = theCtrl.SortItems( SortByTime, m_bAscendingOrder );
		}

		if( bResult )
		{
			// set the sort index
			m_iSortIndex = pNMListView->iSubItem;
		}
		else
		{
			// restore the order
			m_bAscendingOrder = bSaveOrder;
		}
	}
	
	*pResult = 0;
}


void CFilterReportView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	CListCtrl&			theCtrl = GetListCtrl();

//FORNOW	CListView::OnLButtonDblClk(nFlags, point);

	INT iSelected = theCtrl.GetNextItem( -1, LVNI_SELECTED );
	while ( iSelected >= 0 )
	{
		CString szMailboxName = theCtrl.GetItemText( iSelected, 0 );

		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByNamedPath( szMailboxName );
		
		if( pCommand != NULL )
		{
			pCommand->Execute( CA_OPEN ); 
		}

		iSelected = theCtrl.GetNextItem( iSelected, LVNI_SELECTED );
	}
}


////////////////////////////////////////////////////////////////////////
// PreTranslateMessage [public, virtual]
//
// Pressing the Enter key is a shortcut for double-clicking the selection.
////////////////////////////////////////////////////////////////////////
BOOL CFilterReportView::PreTranslateMessage(MSG* pMsg)
{
	if ((WM_KEYDOWN == pMsg->message) && (VK_RETURN == pMsg->wParam))
	{
		OnLButtonDblClk(0, CPoint(-1, -1));
		return TRUE;
	}

	// do default processing
	return CListView::PreTranslateMessage(pMsg);
}


