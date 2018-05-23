////////////////////////////////////////////////////////////////////////
// Filename:  EmoticonToolbarButton.cpp
//
// Copyright (c) 2001 by QUALCOMM, Incorporated
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
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "EmoticonMenu.h"

#include "guiutils.h"
#include "mainfrm.h"
#include "QCGraphics.h"
#include "EmoticonToolbarButton.h"
#include "EmoticonManager.h"

#include "DebugNewHelpers.h"

const DWORD		MAGICROP		= 0xb8074a;

extern EmoticonDirector g_theEmoticonDirector;

IMPLEMENT_BUTTON(EmoticonToolbarButton)


EmoticonToolbarButton::EmoticonToolbarButton()
	:	SECTwoPartBtn(), m_pLastEmoticonDrawn(NULL)
{
}


void EmoticonToolbarButton::DrawButton(CDC& dc, SECBtnDrawData& data)
{
	// We want the emoticon toolbar button to reflect the most recently selected
	// emoticon rather than the default icon.
	//
	// We could do all the drawing ourselves by overriding both DrawFace and
	// DrawDisabled, but that mean replicating a lot of code that's already
	// been written (handling normal state, pressed state, and disabled
	// drawing).
	//
	// Instead, we'll be sneaky. SECTwoPartBtn already knows how to draw
	// a normal button where the icon doesn't change. All we need to do
	// is add the smarts to change the icon behind its back. SECTwoPartBtn
	// (and its parent classes) will do all the hard work of drawing and
	// be none the wiser.
	Emoticon *	pEmoticon = GetCurrentEmoticon();

	// Check to see if we have an emoticon and if it's different from the
	// last emoticon drawn.
	if ( pEmoticon && (pEmoticon != m_pLastEmoticonDrawn) )
	{
		// Remember the last emoticon drawn so that we can save time
		// and avoid doing this when it's not necesssary. (Please note
		// that m_pLastEmoticonDrawn should only be used for comparison
		// purposes - note used directly because it could potentially
		// be deleted).
		m_pLastEmoticonDrawn = pEmoticon;
		
		// Get the image associated with the current emoticon
		CImageList *	pEmoticonImage = pEmoticon->GetImage();
		
		if (pEmoticonImage)
		{
			// Calculate the location of the icon for this toolbar button
			int		nIconX = m_nImage * m_pToolBar->GetImgWidth();
			int		nIconY = 0;
			
			// Erase the previous icon
			extern SEC_AUX_DATA		secData;
			
			data.m_bmpDC.FillSolidRect(nIconX, nIconY, m_pToolBar->GetImgWidth(), m_pToolBar->GetImgHeight(), secData.clrBtnFace);
			
			// Draw the new emoticon icon transparently using the mask
			pEmoticonImage->Draw(&data.m_bmpDC, 0, CPoint(nIconX, nIconY), ILD_TRANSPARENT);
		}
	}
	
	SECTwoPartBtn::DrawButton(dc, data);
}


////////////////////////////////////////////////////////////////////////
// BtnPressDown [protected, virtual]
//
// Handle popup menu when right side clicked on
////////////////////////////////////////////////////////////////////////
BOOL EmoticonToolbarButton::BtnPressDown(CPoint point)
{
	BOOL bRet = SECTwoPartBtn::BtnPressDown(point);
	
	if (bRet && (m_nStyle & SEC_TBBS_PRESSED2))
	{
		// Draw the right button with the pressed style
		Invalidate();

		CPoint thePoint;

		thePoint.x = m_x;
		thePoint.y = m_y + m_cy;
		
		m_pToolBar->ClientToScreen(&thePoint);

		// Get Edit->Emoticon submenu
		CMenu* pMenu = CMenu::FromHandle(CMainFrame::QCGetMainFrame()->GetEditMenu());
		if (!pMenu || !(pMenu = pMenu->GetSubMenu(12)))
			ASSERT(0);
		else
		{
			UINT nID = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, thePoint.x, thePoint.y, AfxGetMainWnd());

			if (nID)
			{
				// TrackPopupMenu() doesn't post the command if you specify TPM_RETURNCMD,
				// so we have to do it ourselves
				PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_COMMAND, nID, 0);
	
				// Save this selected emoticon as the last emoticon used, but not if
				// the Shift key is down (user feature)
				if (!ShiftDown()) {
					Emoticon *emoticonObj = NULL;
					g_theEmoticonDirector.LookUpEmoticonIDMap(static_cast<unsigned short>(nID), emoticonObj);
					ASSERT(emoticonObj != NULL);
					SetIniString(IDS_INI_LAST_EMOTICON_TRIGGER, emoticonObj->GetTrigger());
				}
			}
		}

		// Get rid of the pressed style and redraw the right button so it displays normally
		// and the left button gets redrawn with the new color.
		m_nStyle &= ~(SEC_TBBS_PRESSED2 | SEC_TBBS_TRACKING2);
		Invalidate();

		// Act like nothing ever happened
		return FALSE;
	}

	return bRet;
}


Emoticon * EmoticonToolbarButton::GetCurrentEmoticon()
{
	Emoticon *		pEmoticon = NULL;
	CString			szLastEmoticonTrigger = GetIniString(IDS_INI_LAST_EMOTICON_TRIGGER);

	// Attempt to get the last emoticon, if any
	if ( !szLastEmoticonTrigger.IsEmpty() )
		g_theEmoticonDirector.LookUpEmoticonTriggerMap(szLastEmoticonTrigger, pEmoticon);

	if (!pEmoticon)
	{
		// Couldn't get the last emoticon, so check the emoticon list
		const EmoticonList *	pEmoticonList = g_theEmoticonDirector.GetEmoticonList();

		if ( pEmoticonList && !pEmoticonList->IsEmpty() )
		{
			// Default to the head of the list
			pEmoticon = pEmoticonList->GetHead();

			// We really should have one now - the list tested as non-empty above
			ASSERT(pEmoticon);

			if (pEmoticon)
			{
				// Set the last emoticon for future use
				ASSERT( pEmoticon->GetTrigger() );
				SetIniString( IDS_INI_LAST_EMOTICON_TRIGGER, pEmoticon->GetTrigger() );
			}
		}
	}

	return pEmoticon;
}
