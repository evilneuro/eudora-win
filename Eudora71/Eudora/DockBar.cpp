// DockBar.cpp : implementation file
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
#include "DockBar.h"
#include "AdWazooBar.h"
#include "QCSharewareManager.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// QCDockBar

QCDockBar::QCDockBar()
{
}

QCDockBar::~QCDockBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// IsControlBarAtMaxWidthInRow [public, virtual]
//
// Override to handle non-sizing Ad window.
/////////////////////////////////////////////////////////////////////////////
BOOL QCDockBar::IsControlBarAtMaxWidthInRow(SECControlBar* pBar)
{
	if (GetSharewareMode() == SWM_MODE_ADWARE)
	{
		CAdWazooBar* pAdWazooBar = NULL;
		SECControlBar* pTempBar;
		float fLargestPctWidth = 0.0;
		CPtrList rowList;
		POSITION pos;
		
		VERIFY(GetControlBarRow(rowList, pBar));

		pos = rowList.GetHeadPosition();
		while (pos)
		{
			pTempBar = (SECControlBar*) rowList.GetNext(pos);
			if (!pAdWazooBar)
				pAdWazooBar = DYNAMIC_DOWNCAST(CAdWazooBar, pTempBar);
			if (pTempBar != pAdWazooBar && pTempBar != pBar && pTempBar->m_fPctWidth > fLargestPctWidth)
				fLargestPctWidth = pTempBar->m_fPctWidth;
		}

		if (pAdWazooBar)
			return (fLargestPctWidth <= m_fAdjustedMinPctWidth);
	}

	return SECDockBar::IsControlBarAtMaxWidthInRow(pBar);
}

/////////////////////////////////////////////////////////////////////////////
// IsOnlyControlBarInRow [public, virtual]
//
// Override to handle non-sizing Ad window.
/////////////////////////////////////////////////////////////////////////////
BOOL QCDockBar::IsOnlyControlBarInRow(SECControlBar* pBar)
{
	if (GetSharewareMode() != SWM_MODE_ADWARE)
		return SECDockBar::IsOnlyControlBarInRow(pBar);

	SECControlBar* pTempBar;
	CPtrList rowList;
	POSITION pos;
	
	VERIFY(GetControlBarRow(rowList, pBar));

	pos = rowList.GetHeadPosition();
	while (pos)
	{
		pTempBar = (SECControlBar*) rowList.GetNext(pos);
		if (pTempBar->IsVisible() && pTempBar != pBar && !DYNAMIC_DOWNCAST(CAdWazooBar, pTempBar))
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// FindControlBarLocation [public]
//
// Returns in what column and row a given control bar is located.
/////////////////////////////////////////////////////////////////////////////
bool QCDockBar::FindControlBarLocation(SECControlBar* in_pBar, int & out_nCol, int & out_nRow)
{
	bool	bFound = false;

	out_nCol = 0;
	out_nRow = -1;
	
	for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		CControlBar *		pBar = GetDockedControlBar(nPos);

		if (pBar == in_pBar)
		{
			bFound = true;
			break;
		}

		void* pVoid = m_arrBars[nPos];
		if (pBar != NULL && pBar->IsVisible())
		{
			out_nCol++;
		}
		else if (pBar == NULL && pVoid == NULL)
			// end of row because pBar == NULL
		{
			out_nCol = 0;
			out_nRow++;
		}
	}

	return bFound;
}

/////////////////////////////////////////////////////////////////////////////
// CalcTrackingLimits [protected, virtual]
//
// Used to make sure a splitter is never moved further than a fixed-size
// Ad window will allow.
/////////////////////////////////////////////////////////////////////////////
void QCDockBar::CalcTrackingLimits(Splitter* pSplitter)
{
	SECDockBar::CalcTrackingLimits(pSplitter);

	if (GetSharewareMode() != SWM_MODE_ADWARE)
		return;

	CAdWazooBar* pAdWazooBar = NULL;
	BOOL bIsBefore = FALSE;
	int nRowStartPos =0;

	// Figure out if the Ad window comes before or after this splitter in this row
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		CControlBar* pBar = GetDockedControlBar(i);
		if (!pBar)
		{
			if (m_arrBars[i] == NULL)
			{
				// If the Ad wazoo bar isn't in the row of this splitter, there's nothing more to do
				if (pAdWazooBar)
				{
					if (pSplitter->m_nPos >= i)
						return;
					break;
				}
				else if (pSplitter->m_nPos < i)
					return;
				nRowStartPos = i;
			}
		}
		if (!pAdWazooBar)
		{
			pAdWazooBar = DYNAMIC_DOWNCAST(CAdWazooBar, pBar);
			if (pAdWazooBar)
			{
				if (pSplitter->m_type == Splitter::RowSplitter)
					bIsBefore = (m_dwStyle & (CBRS_ALIGN_LEFT | CBRS_ALIGN_TOP));
				else
					bIsBefore = (i <= pSplitter->m_nPos);
			}
		}
	}

	if (pAdWazooBar)
	{
		int nMinWidth = pAdWazooBar->GetMinWidth();
		int nMinHeight = pAdWazooBar->GetMinHeight();

		CRect rectInside;
		GetClientRect(&rectInside);

		if (pSplitter->m_orientation == Splitter::Vertical)
		{
			if (bIsBefore)
				pSplitter->m_nMin = __max(rectInside.left + nMinWidth, pSplitter->m_nMin);
			else
				pSplitter->m_nMax = __min(rectInside.right - nMinWidth, pSplitter->m_nMax);
		}
		else
		{
			if (bIsBefore)
				pSplitter->m_nMin = __max(rectInside.top + nMinHeight, pSplitter->m_nMin);
			else
				pSplitter->m_nMax = __min(rectInside.bottom - nMinHeight, pSplitter->m_nMax);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// NormalizeRow [protected, virtual]
//
// Handle this override to make sure Ad windows stay the size they are
/////////////////////////////////////////////////////////////////////////////
void QCDockBar::NormalizeRow(int nPos, CControlBar* pBarDocked, int& nBarsBidirectional, int& nBarsUnidirectional)
{
	SECDockBar::NormalizeRow(nPos, pBarDocked, nBarsBidirectional, nBarsUnidirectional);

	if (GetSharewareMode() != SWM_MODE_ADWARE)
		return;

	CAdWazooBar* pAdWazooBar = NULL;
	SECControlBar* pBarToResize = NULL;
	int nRowStartPos = 0;

	// See if the Ad wazoo bar is in the row of bars specified by nPos.
	// If it is, then remember the bar before it, or the bar after it if
	// there is no bar before it.
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		SECControlBar* pBar = DYNAMIC_DOWNCAST(SECControlBar, GetDockedControlBar(i));
		if (!pBar)
		{
			if (m_arrBars[i] == NULL)
			{
				if (i > nPos)
					break;
				nRowStartPos = i;
				pAdWazooBar = NULL;
				pBarToResize = NULL;
			}
		}
		else
		{
			if (!pAdWazooBar)
				pAdWazooBar = DYNAMIC_DOWNCAST(CAdWazooBar, pBar);
			if (!pBarToResize && pBar != pAdWazooBar)
				pBarToResize = pBar;
		}
	}

	if (pAdWazooBar && pBarToResize)
	{
		CRect rect;
		GetClientRect(&rect);
		float fOldPctWidth = pAdWazooBar->m_fPctWidth;
		float fNewPctWidth = (m_dwStyle & (CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT))?
									pAdWazooBar->GetMinHeight() / (float)rect.Height() :
									pAdWazooBar->GetMinWidth() / (float)rect.Width();
		float fDiffPctWidth = fNewPctWidth - fOldPctWidth;

//		if (fDiffPctWidth > 0)
		{
			if (fDiffPctWidth > 0 && pBarToResize->m_fPctWidth < fDiffPctWidth)
			{
				// This bar is too small to shrink, find the biggest one
				float fLargestPctWidth = 0.0;
				for (int i = nRowStartPos + 1; i < m_arrBars.GetSize(); i++)
				{
					SECControlBar* pBar = DYNAMIC_DOWNCAST(SECControlBar, GetDockedControlBar(i));
					if (!pBar)
					{
						if (m_arrBars[i] == NULL)
							break;
					}
					else if (pBar->m_fPctWidth > fLargestPctWidth)
					{
						pBarToResize = pBar;
						fLargestPctWidth = pBar->m_fPctWidth;
					}
				}
			}

			pBarToResize->m_fPctWidth -= fDiffPctWidth;
			pAdWazooBar->m_fPctWidth += fDiffPctWidth;
		}
	}
}


BEGIN_MESSAGE_MAP(QCDockBar, SECDockBar)
	//{{AFX_MSG_MAP(QCDockBar)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// QCDockBar message handlers
