// AdView.cpp : implementation file
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "rs.h"
#include "guiutils.h"
#include "pgosutl.h"

// playlist subsystem interface
#include "plist_mgr.h"

// our own sorry butt
#include "AdView.h"

#include "AdWazooWnd.h"
#include "AdWazooBar.h"
#include "LinkHistoryManager.h"
#include "QCSharewareManager.h"
#include "AuditEvents.h"
#include "FaceTime.h"


#include "DebugNewHelpers.h"


////////////////////////////////////////////////////////////////////////////////
// local genius

// just a temporary hack---i think
#define ADHTML_TMPL "<div align=center><a href=\"%s\">"\
                        "<img src=\"%s\" alt=\"%s\">"\
					"</a></div>"


////////////////////////////////////////////////////////////////////////////////
// CAdView

IMPLEMENT_DYNCREATE(CAdView, CPaigeEdtView)

CAdView* CAdView::s_pOneAndOnlyView = NULL;

// Dr. Frankenstein
CAdView::CAdView()
{
	if (s_pOneAndOnlyView)
		ASSERT(0);	// Uh oh, two CAdViews!
	else
		s_pOneAndOnlyView = this;

	m_bIsBlankTime = TRUE;
	m_bOneAdShown = FALSE;
	m_bWasInitialized = FALSE;

	SetReadOnly();
}

// Dr. Kevorkian
CAdView::~CAdView()
{
}

void CAdView::loadEntry( ENTRY_INFO* pe )
{
	CAuditEvents* pAuditEvents = CAuditEvents::GetAuditEvents();
	
	if (m_CurrentAdID.IsEmpty() == FALSE)
	{
		if (pe && m_CurrentAdID == pe->id)
		{
			TRACE1("The ad window was told to show the same ad it's already showing: %s\n", (LPCTSTR)m_CurrentAdID);
			return;
		}

		if (pAuditEvents)
			pAuditEvents->AdCloseEvent(m_CurrentAdID);
		m_CurrentAdID.Empty();
	}

	if (!pe)
	{
		if (!m_bIsBlankTime)
		{
			m_bIsBlankTime = TRUE;
			Invalidate();

			EnableWindow(FALSE);
//			ShowWindow(SW_HIDE);
		}

		return;
	}

	const char* alt = "Dees eez dee ad!";

	// BOG: is the right thing? might be overkill, then again...
	SetRedraw(FALSE);
	NewPaigeObject();

	if ( pe ) {
		//	Image file URL (if applicable) to pass to LinkHistoryManager
		const char *	szImageFileURL = NULL;
		
		if ( strcmp( pe->mime_type, "text/html" ) == 0 ) {
			HFILE hf = _lopen( pe->src, OF_READ | OF_SHARE_DENY_NONE );

			PgDataTranslator theTxr( m_paigeRef );
			theTxr.ImportData( hf, PgDataTranslator::kFmtHtml );

			_lclose( hf );
		}
		else {
			szImageFileURL = pe->src;
			
			CString AdHTML;
			AdHTML.Format(ADHTML_TMPL, pe->href, pe->src, pe->title? pe->title : alt);
			unsigned char* pucstr = (unsigned char*)(const char*) AdHTML;
			memory_ref impRef = CString2Memory(PgMemGlobalsPtr(), pucstr);

			PgDataTranslator theTxr(m_paigeRef);
			theTxr.ImportData(&impRef, PgDataTranslator::kFmtHtml, draw_none);

			DisposeMemory(impRef);
		}
		
		CWazooWnd* pParent = DYNAMIC_DOWNCAST(CAdWazooWnd, GetParent());
		CAdWazooBar* pBar = DYNAMIC_DOWNCAST(CAdWazooBar, pParent->GetParentControlBar());
		if (!pBar)
			ASSERT(0);
		else
			pBar->NewAdSize(pe->width, pe->height);

		m_bOneAdShown = TRUE;
		LinkHistoryManager::Instance()->AddAdToLinkHistory(pe->title ? pe->title : alt, szImageFileURL, pe->href, pe->id);

		m_CurrentAdID = pe->id;
		if (pAuditEvents)
			pAuditEvents->AdOpenEvent(m_CurrentAdID);
	}

	// Tell Paige not to display the caret
	paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
	pgRecPtr->flags2 |= NO_CARET_BIT;
	UnuseMemory(m_paigeRef);

	m_bIsBlankTime = FALSE;
	SetRedraw(TRUE);
	Invalidate();

	EnableWindow(TRUE);
//	ShowWindow(SW_SHOWNA);
}


////////////////////////////////////////////////////////////////////////////////
// Handler for playlist callbacks

/* static */ void CAdView::PlaylistCallback( int cmd, long userRef, long lParam )
{
	// DRW - Most of the guts of this method are now in CMainFrame::PlaylistCallback().
	switch ( cmd )
	{
		case PLIST_BLANK:
		{
			s_pOneAndOnlyView->loadEntry(0);
			break;
		}
		case PLIST_SHOWTHIS:
		{
			ENTRY_INFO* pei = (ENTRY_INFO*) lParam;
			switch ( pei->entry_type )
			{
				case kAd:
				case kRunout:
				{
					s_pOneAndOnlyView->loadEntry( pei );
					break;
				}
			}
			break;
		}
	}
}

bool CAdView::NewPaigeObject(long AddFlags /*= 0*/, long AddFlags2 /*= 0*/)
{
	const bool bRet = CPaigeEdtView::NewPaigeObject(AddFlags, AddFlags2 | NO_CARET_BIT);

	// There are times when a paige ref needs to know that this is an ad
	paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
	PgStuffBucket* pSB = (PgStuffBucket*) pgRecPtr->user_refcon;
	pSB->kind = PgStuffBucket::kAd;
	UnuseMemory(m_paigeRef);

	// Set background color to user's system color for 3D background
	COLORREF Sys3DColor = GetSysColor(COLOR_3DFACE);
	color_value BackColor;
	pgOSToPgColor(&Sys3DColor, &BackColor);
	pgSetPageColor(m_paigeRef, &BackColor);

	return (bRet);
}

/*virtual*/ BOOL CAdView::LaunchURL(LPCSTR szURL)
{
	CAuditEvents* pAuditEvents = CAuditEvents::GetAuditEvents();

	if (pAuditEvents)
		pAuditEvents->AdHitEvent(m_CurrentAdID, IsOffline() == FALSE);

	BOOL bLaunchGood = TRUE;

	//	Launch URLs in ad windows separately so that we can do the
	//	Link History Manager record keeping differently.
	//	Check ShouldLaunchURL first, which checks offline status and
	//	user offline URL preferences (i.e. visit, remind, bookmark)
	if ( LinkHistoryManager::Instance()->ShouldLaunchURL(szURL, true, NULL, m_CurrentAdID) )
	{
		bLaunchGood = LaunchURLNoHistory(szURL);

		//	Last parameter true because this URL is an ad
		LinkHistoryManager::Instance()->AddVisitedURL(szURL, bLaunchGood, true, m_CurrentAdID);
	}

	return bLaunchGood;
}

/*virtual*/ BOOL CAdView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (s_pOneAndOnlyView != this)
	{
		// Already ASSERTed in the constructor.  Now, just don't create the window.
		return FALSE;
	}
	
	// use plain CView instead of PaigeEdtView. only difference is classname,
	// and we don't want to be mistaken for an editor.
	return CView::PreCreateWindow(cs);
}


////////////////////////////////////////////////////////////////////////////////
// Daaa... Map.

BEGIN_MESSAGE_MAP(CAdView, CPaigeEdtView)
	//{{AFX_MSG_MAP(CAdView)
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
// CAdView message handlers

void CAdView::OnIdleUpdateCmdUI()
{
// No more warning about hiding the Ad window
//
//	if (GetSharewareMode() != SWM_MODE_ADWARE)
//		return;
//
//	static int LastHiddenCheckTime = 0;
//	static int HiddenCount = 0;
//
//	const BOOL bIsForeground = IsForegroundApp();
//	
//	if (time(NULL) >= LastHiddenCheckTime + 60)
//	{
//		// Don't check for hidden window when we're in blank time or the
//		// first time through here (which only does initialization)
//		if (!m_bIsBlankTime && LastHiddenCheckTime)
//		{
//			if (bIsForeground)
//			{
//				BOOL bSpotCovered = FALSE;
//				CPoint point;
//				HWND hWndSpot;
//				CRect rect;
//
//				GetWindowRect(rect);
//
//#define CHECK_SPOT(p)	hWndSpot = ::WindowFromPoint(p);\
//						if (hWndSpot != m_hWnd && !::IsChild(m_hWnd, hWndSpot))\
//							bSpotCovered = TRUE;
//
//				// Check center point
//				point = rect.CenterPoint();
//				CHECK_SPOT(point);
//
//				// Check middle of the 4 quadrants
//				point.x -= rect.Width() / 4;
//				point.y -= rect.Height() / 4;
//				CHECK_SPOT(point);
//				point.x += rect.Width() / 2;
//				CHECK_SPOT(point);
//				point.y += rect.Height() / 2;
//				CHECK_SPOT(point);
//				point.x -= rect.Width() / 2;
//				CHECK_SPOT(point);
//
//				if (bSpotCovered)
//				{
//					// Give the user at least a buffer of 60 seconds to have the Ad window covered.
//					// If we didn't use this latch, then the user could move a window over the Ad
//					// window just before the check is done, and would get dinged right away.
//					if (++HiddenCount > 1)
//					{
//						CRString Message(IDS_ERR_HIDDEN_AD_WINDOW);
//						AlertDialog(IDD_HIDDEN_AD_WINDOW, (LPCTSTR)Message);
//					}
//					LastHiddenCheckTime = time(NULL);
//					return;
//				}
//			}
//			HiddenCount= 0;
//		}
//
//		LastHiddenCheckTime = time(NULL);
//	}
}

void CAdView::InitBlankBitmap()
{
	m_BlankTimeBitmap.DeleteObject();
	VERIFY(SECLoadSysColorBitmap(m_BlankTimeBitmap, MAKEINTRESOURCE(IDB_AD_BLANK_TIME), FALSE));
}

void CAdView::OnSysColorChange()
{
	InitBlankBitmap();
	InvalidateRect(NULL);

	CPaigeEdtView::OnSysColorChange();
}

void CAdView::OnDraw(CDC* pDC)
{
	// Display the blank time bitmap if we're in that mode
	if (m_bIsBlankTime)
	{
		if (m_BlankTimeBitmap.GetSafeHandle())
		{
			CPaletteDC memDC;
			if (memDC.CreateCDC())
			{
				CBitmap* pOldBmp = memDC.SelectObject(&m_BlankTimeBitmap);
				CRect rect;
				BITMAP bm;
				VERIFY(m_BlankTimeBitmap.GetBitmap(&bm));
				GetClientRect(&rect);
				VERIFY(pDC->BitBlt(rect.Width()/2 - bm.bmWidth/2, rect.Height()/2 - bm.bmHeight/2,
						bm.bmWidth, bm.bmHeight, &memDC, 0, 0, SRCCOPY));
				memDC.SelectObject(pOldBmp);
			}
		}

		return;
	}

	CPaigeEdtView::OnDraw(pDC);
}

void CAdView::OnInitialUpdate()
{
	if (m_bWasInitialized)
	{
		// Guard against double initialization
		return;
	}
	m_bWasInitialized = TRUE;

	m_pgBorder.SetRectEmpty();

	CPaigeEdtView::OnInitialUpdate();

	// DRW - the MGR_Init() call has been moved to CMainFrame.

//	// Start out hidden so that the caret doesn't show
//	ShowWindow(SW_HIDE);

	InitBlankBitmap();

	ShowScrollBar(SB_BOTH, FALSE);
}

void CAdView::OnMouseMove(UINT nFlags, CPoint point)
{
	// Bypass base CPaigeEdtView implementation of WM_MOUSEMOVE so that
	// we don't do any drag selecting or drag and drop of ad window contents
	CView::OnMouseMove(nFlags, point);
}

void CAdView::OnSetFocus(CWnd* pOldWnd)
{
	// Bypass PaigeEdtView because it mucks with the caret, which we want to hide
	CView::OnSetFocus(pOldWnd);
}


void CAdView::OnKillFocus(CWnd* pNewWnd)
{
	// Bypass PaigeEdtView because it mucks with the caret, which we want to hide
	CView::OnKillFocus(pNewWnd);
}
