////////////////////////////////////////////////////////////////////////
// Filename:  ColorToolbarButton.cpp
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
#include "ColorLis.h"

#include "guiutils.h"
#include "mainfrm.h"

#include "ColorToolbarButton.h"


#include "DebugNewHelpers.h"


IMPLEMENT_BUTTON(CColorToolbarButton)


////////////////////////////////////////////////////////////////////////
// DrawFace [protected, virtual]
//
// Override of SECStdBtn::DrawFace() implemenation.  Handles the details
// of drawing the colored square on top of the standard button.
////////////////////////////////////////////////////////////////////////
void CColorToolbarButton::DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						 int& nWidth, int& nHeight, int nImgWidth)
{
	SECTwoPartBtn::DrawFace(data, bForce, x, y, nWidth, nHeight, nImgWidth);

	// Draw the colored rectangle, but only for the left side of the button.
	// The only way you can tell the left side from the right is the width.
	if (nImgWidth > 10)
	{
		COLORREF theColor = CColorMenu::GetColor(GetIniShort(IDS_INI_LAST_TEXT_COLOR) + ID_COLOR0);

		const int ColorRectWidth = 14;
		const int ColorRectHeight = 5;
		int ColorRectX = x + 1 + (8 - ColorRectWidth/2);
		int ColorRectY = y + 2 + (16 - ColorRectHeight);

		if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED) && !bForce)
		{
			ColorRectX++;
			ColorRectY++;
		}

		data.m_drawDC.Draw3dRect(ColorRectX, ColorRectY, ColorRectWidth, ColorRectHeight, 0, 0);
		data.m_drawDC.FillSolidRect(ColorRectX + 1, ColorRectY + 1, ColorRectWidth - 2, ColorRectHeight - 2, theColor);
	}
}

////////////////////////////////////////////////////////////////////////
// BtnPressDown [protected, virtual]
//
// Handle popup menu when right side clicked on
////////////////////////////////////////////////////////////////////////
BOOL CColorToolbarButton::BtnPressDown(CPoint point)
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

		// Get Edit->Text->Color submenu
		CMenu* pMenu = CMenu::FromHandle(CMainFrame::QCGetMainFrame()->GetEditMenu());
		if (!pMenu || !(pMenu = pMenu->GetSubMenu(10)) || !(pMenu = pMenu->GetSubMenu(11)))
			ASSERT(0);
		else
		{
			UINT nID = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, thePoint.x, thePoint.y, AfxGetMainWnd());

			if (nID)
			{
				// TrackPopupMenu() doesn't post the command if you specify TPM_RETURNCMD,
				// so we have to do it ourselves
				PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_COMMAND, nID, 0);
	
				// Save this selected color as the last used color, but not if
				// the Shift key is down (user feature)
				int nColor = nID - ID_COLOR0;
				if (!ShiftDown() && nColor >= 0 && nColor < 16)
					SetIniLong(IDS_INI_LAST_TEXT_COLOR, nColor);
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
