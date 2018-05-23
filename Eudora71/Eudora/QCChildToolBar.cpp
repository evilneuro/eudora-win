// QCChildToolBar.cpp: implementation of the QCChildToolBar class.
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "QCChildToolBar.h"
#include "QCToolBarManager.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "mainfrm.h"

#include "DebugNewHelpers.h"

extern QCCommandStack			g_theCommandStack;
extern QCPluginDirector			g_thePluginDirector;

class CDontFloatDockContext : public SECDockContext
{
public:
	CDontFloatDockContext(CControlBar* pBar) : SECDockContext(pBar) {}

	virtual void StartDrag(CPoint pt);
};

void CDontFloatDockContext::StartDrag( CPoint )
{
}

IMPLEMENT_DYNCREATE(QCChildToolBar, SECCustomToolBar )

BEGIN_MESSAGE_MAP(QCChildToolBar, SECCustomToolBar)
	//{{AFX_MSG_MAP(QCChildToolBar)
	ON_MESSAGE(WM_USER_QUERY_PALETTE, OnMyQueryNewPalette)
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCChildToolBar::QCChildToolBar()
	:	m_nLastIdleProcessing(0)
{

}

QCChildToolBar::~QCChildToolBar()
{

}

BOOL QCChildToolBar::AcceptDrop() const
{
	return FALSE;
}


BOOL QCChildToolBar::InitButtons()
{
//	for( i = 0; i < m_btns.GetCount(); i++ )
//	{
//		if( m_btns[i].m_nID == 
//
//		if( pBtn->IsKindOf
//	}
	return TRUE;
}

void QCChildToolBar::EnableDocking(DWORD dwDockStyle)
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
    ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

    m_dwDockStyle = dwDockStyle;
    if (m_pDockContext == NULL)
		m_pDockContext = DEBUG_NEW CDontFloatDockContext(this);
	
    // permanently wire the bar's owner to its current parent
	if (m_hWndOwner == NULL)
	m_hWndOwner = ::GetParent(m_hWnd);
}


void* QCChildToolBar::GetButton(
INT iIndex )
{
	return ( void* )( m_btns[ iIndex ] );
}



BOOL QCChildToolBar::GetDragMode() const
{
	// We are in Drag 'Add' mode if the CONTROL key is held down at the start
	// of the drag
	return FALSE;
}

BOOL QCChildToolBar::DragButton(int nIndex, CPoint point)
{
	m_bAltDrag = FALSE;
	return FALSE;
}


void QCChildToolBar::OnLButtonDblClk(UINT nFlags, CPoint pt)
{
	// ignore double click if inside our "gripper buttons"
	if( m_dwExStyle & CBRS_EX_GRIPPER_CLOSE) {
	   if(m_rcGripperCloseButton.PtInRect(pt))
	      return;
	   }
	if( m_dwExStyle & CBRS_EX_GRIPPER_EXPAND) {
	   if(m_rcGripperExpandButton.PtInRect(pt))
	      return;
	   }	

	CWnd::OnLButtonDblClk(nFlags, pt);
}


void QCChildToolBar::AddTranslatorButtons(int &startPos)
{
	LONG				lType;
	QCPluginCommand*	pCommand		= NULL;
	long				lastModID		= 0;


	POSITION pos = g_thePluginDirector.GetFirstPluginPosition();
	while( pos != NULL )
	{
		pCommand = g_thePluginDirector.GetNextPlugin( pos );

		switch(pCommand->GetType())
		{
			case CA_TRANSMISSION_PLUGIN:
				lType = ID_TRANSLATORS_QUEUE;
				break;
			default:
				continue;
		}
	
		// okay, add it in
		UINT uCommandID = g_theCommandStack.FindOrAddCommandID( pCommand, pCommand->GetType() );

		if( uCommandID == 0 )
		{
			continue;
		}
	
		int imageOffset = ((QCPluginCommand *)pCommand)->m_ImageOffset;
		BOOL isOnCompletion = ((QCPluginCommand *)pCommand)->IsOnCompletion();
		long modID = 0, trID = 0;
		((QCPluginCommand *)pCommand)->GetTransmissionID(&modID, &trID);
		
		CImageList *pImageList = &g_thePluginDirector.m_ImageList;
		( ( QCToolBarManagerWithBM* ) m_pManager )->CopyButtonImage( pImageList, imageOffset, uCommandID,  isOnCompletion);
	
			// Separator between groups
		if ((lastModID != 0)&&(modID != lastModID))
			AddButton( startPos++, 0, TRUE, TRUE );

		AddButton( startPos, uCommandID, FALSE, TRUE );
		SetButtonStyle( startPos, TBBS_CHECKBOX );

		m_btns[ startPos++ ]->m_ulData = lType;
		
		lastModID = modID;
	}
}


BOOL QCChildToolBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction;
	INT					iCurrentButton;
	WORD				wCommandID;
	CString				szToolTip;

	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*) lParam;

	switch (pNMHDR->code)
	{
	case TTN_NEEDTEXTA:
	case TTN_NEEDTEXTW:
		{
			//
			// The toolbar button command id is stuffed into the 'idFrom' field.
			//
			wCommandID = ( WORD ) pNMHDR->idFrom;
			ASSERT( wCommandID != ID_SEPARATOR );
			for( iCurrentButton = GetBtnCount() - 1; iCurrentButton >= 0; iCurrentButton-- )
			{
				if( m_btns[ iCurrentButton ]->m_nID == wCommandID )
				{
					break;
				}
			}

			if( iCurrentButton < 0 )
			{
				break;
			}

			SECStdBtn* pButton = m_btns[ iCurrentButton ];

			if (pButton->m_ulData != 0 && pButton->m_nID != pButton->m_ulData)
			{
				if( g_theCommandStack.Lookup( wCommandID, &pObject, &theAction ) )
				{
					szToolTip = pObject->GetToolTip( theAction );				
					szToolTip = szToolTip.Left(79);		// truncate at 79 chars, if necessary

					if (TTN_NEEDTEXTW == pNMHDR->code)
					{
						TOOLTIPTEXTW *pTTT = (TOOLTIPTEXTW *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));

						USES_CONVERSION;
						wcscpy(pTTT->szText, A2W( szToolTip ) );
					}
					else
					{
						TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));
						strcpy(pTTT->szText, szToolTip);
					}
					return TRUE;
				}
			}
		}
		break;
		default:
		break;
	}
	
	return SECCustomToolBar::OnNotify( wParam, lParam, pResult );
}


BOOL QCChildToolBar::CreateEx(DWORD dwExStyle, CWnd* pParentWnd, 
								DWORD dwStyle, UINT nID, LPCTSTR lpszTitle)
{
	m_cyTopBorder    = 1;
	m_cyBottomBorder = 1;

	if (!SECCustomToolBar::CreateEx(dwExStyle, pParentWnd, dwStyle & CBRS_ALL, nID, lpszTitle))
		return FALSE;

	const DWORD dwNonControlBarStyle = dwStyle & ~CBRS_ALL;
	if (dwNonControlBarStyle)
		ModifyStyle(0, dwNonControlBarStyle);

	return TRUE;
}

LRESULT QCChildToolBar::OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam)
{            
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void QCChildToolBar::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged((CWnd*)this, pFocusWnd);
}

void QCChildToolBar::OnSysColorChange()
{
	m_Palette.DoSysColorChange((CWnd*)this);
}


LRESULT QCChildToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	// Get our parent frame
	CWnd *		pToolBarOwnerFrame = GetParentFrame();

	// Get the focused frame
	CWnd *		pFocusedFrame = GetFocus();

	if (pFocusedFrame)
		pFocusedFrame = pFocusedFrame->GetParentFrame();

	// Get the current tick count
	DWORD	nCurrentTickCount = GetTickCount();
	
	// Check to see if the tick count wrapped, which it will do if the system
	// is run continuously for 49.7 days.
	if (nCurrentTickCount < m_nLastIdleProcessing)
		m_nLastIdleProcessing = 0;

	LRESULT		lResult = 0;

	// We throttle how often we'll update toolbars during idle time,
	// because we can end up with a *lot* of toolbars (one for each
	// read and composition window) and updating them all during every
	// idle causes Eudora to spend too much time in each idle.
	//
	// We'll continue processing the idle if:
	// * We're inside the focused frame (because we may need to update
	//   immediately - for example to update as the result of something
	//   typed)
	// * It's been almost 2 seconds since our last idle (this could
	//   probably be longer, but it was chosen to work well with Eudora's
	//   idle timer that fires once every second)
	if ( ((nCurrentTickCount - m_nLastIdleProcessing) > 1900) ||
		 (pToolBarOwnerFrame == pFocusedFrame) )
	{
		// Continue processing the idle by calling our parent class
		lResult = SECCustomToolBar::OnIdleUpdateCmdUI(wParam, lParam);

		// Update when we last allowed idle processing
		m_nLastIdleProcessing = nCurrentTickCount;
	}
	
	return lResult;
}