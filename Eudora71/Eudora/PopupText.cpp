//	PopupText.cpp
//	
//	Tooltip-like window that can be used for displaying text, but
//	with more flexibility and easier use than standard tooltips.
//
//	Originally based on code by Paul DiLascia as published in
//	MSDN Magazine November 2003, but heavily modified for our
//	purposes.
//
//	Copyright (c) 2004 by QUALCOMM, Incorporated
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
#include "PopupText.h"

#include "DebugNewHelpers.h"

IMPLEMENT_DYNAMIC(CPopupText,CWnd)
BEGIN_MESSAGE_MAP(CPopupText,CWnd)
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_WM_TIMER()
END_MESSAGE_MAP()


//	Constants
const int		CPopupText::kDisplayTimer = 1;
const int		CPopupText::kCheckLocationTimer = 2;
const int		CPopupText::kMaxCharactersBeforeWrapping = 127;

//	Statics
CPopupText *	CPopupText::s_pPopupText = NULL;



// ---------------------------------------------------------------------------
//		* Initialize										[Static Public]
// ---------------------------------------------------------------------------
//	Use to create and initialize the popup text window.

bool
CPopupText::Initialize(
	CPoint			in_pt,
	CWnd *			in_pParentWnd,
	UINT			in_nID)
{
	TRACE("CPopupText::Initialize()\n");
	ASSERT(!s_pPopupText);

	if (s_pPopupText)
		delete s_pPopupText;

	bool	bInitializeGood = false;

	s_pPopupText = DEBUG_NEW_MFCOBJ_NOTHROW CPopupText;

	if (s_pPopupText)
	{
		bInitializeGood = ( s_pPopupText->CreateEx( 0, NULL, NULL,
													WS_POPUP|WS_VISIBLE,
													CRect(in_pt,CSize(0,0)),
													in_pParentWnd, in_nID ) != 0 );
	}

	return bInitializeGood;
}


// ---------------------------------------------------------------------------
//		* Destroy											[Static Public]
// ---------------------------------------------------------------------------
//	Use to destroy the popup text window before quitting.

bool
CPopupText::Destroy()
{
	TRACE("CPopupText::Destroy()\n");

	if (s_pPopupText)
	{
		delete s_pPopupText;
		s_pPopupText = NULL;
	}

	return true;
}


// ---------------------------------------------------------------------------
//		* CPopupText											[Protected]
// ---------------------------------------------------------------------------
//	Constructor

CPopupText::CPopupText()
	:	m_sizeTextMargins(5, 5), m_sizeWindowMargins(20, 20),
		m_nWindowWidthLeftOfCursor(25), m_nWindowWidthRightOfCursor(175)
{
	CNonClientMetrics ncm;
	m_font.CreateFontIndirect(&ncm.lfMenuFont);
}


// ---------------------------------------------------------------------------
//		* ~CPopupText											[Protected]
// ---------------------------------------------------------------------------
//	Destructor

CPopupText::~CPopupText()
{
}


// ---------------------------------------------------------------------------
//		* OnSetText												[Protected]
// ---------------------------------------------------------------------------
//	Text changed: resize window to fit

LRESULT CPopupText::OnSetText(WPARAM wp, LPARAM lp)
{
	GetWindowRect(&m_rectText);
	int h = m_rectText.Height();
	CClientDC dc(this);
	DrawText(dc, CString((LPCTSTR)lp), m_rectText, DT_CALCRECT | DT_WORDBREAK);

	CRect		rectWindow = m_rectText;
	rectWindow.InflateRect(m_sizeTextMargins);
	if (m_sizeTextMargins.cy)
		h = rectWindow.Height();
	SetWindowPos(NULL, 0, 0, rectWindow.Width(), h,
		SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);

	ScreenToClient(m_rectText);
	
	return Default();
}


// ---------------------------------------------------------------------------
//		* DrawText												[Protected]
// ---------------------------------------------------------------------------
//	Draw the text

void
CPopupText::DrawText(
	CDC &			dc,
	LPCTSTR			lpText,
	CRect &			rectText,
	UINT			flags,
	CRect *			prectWindow)
{
	if (prectWindow)
	{
		CBrush		b( GetSysColor(COLOR_INFOBK) ); // use tooltip bg color
		dc.FillRect(prectWindow, &b);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor( GetSysColor(COLOR_INFOTEXT) ); // tooltip text color
	}

	CFont* pOldFont = dc.SelectObject(&m_font);
	dc.DrawText(lpText, &rectText, flags);
	dc.SelectObject(pOldFont);
}


// ---------------------------------------------------------------------------
//		* OnPaint												[Protected]
// ---------------------------------------------------------------------------
//	Paint text using system colors

void CPopupText::OnPaint()
{
	CRect rc;
	GetClientRect(&rc);
	CString s;
	GetWindowText(s);
	CPaintDC dc(this);

	DrawText(dc, s, m_rectText, DT_WORDBREAK | DT_VCENTER, &rc);
}


// ---------------------------------------------------------------------------
//		* PreCreateWindow										[Protected]
// ---------------------------------------------------------------------------
//	Register class if needed

BOOL CPopupText::PreCreateWindow(CREATESTRUCT& cs) 
{
	static CString sClassName;
	if (sClassName.IsEmpty())
		sClassName = AfxRegisterWndClass(0);
	cs.lpszClass = sClassName;
	cs.style = WS_POPUP|WS_BORDER;
	cs.dwExStyle |= WS_EX_TOOLWINDOW;
	return CWnd::PreCreateWindow(cs);
}


// ---------------------------------------------------------------------------
//		* PostNcDestroy											[Protected]
// ---------------------------------------------------------------------------
//	CPopupText is intended to be kept around and reused, so don't auto-delete.

void CPopupText::PostNcDestroy()
{
	// don't delete this
}


// ---------------------------------------------------------------------------
//		* SetText													[Public]
// ---------------------------------------------------------------------------
//	Set the text, inserting CRs as necessary to wrap lines that would
//	otherwise be too long because they do not contain any spaces.

void
CPopupText::SetText(
	LPCTSTR			pszText)
{
	CString		szText = pszText;


	int			nLength = szText.GetLength();
	int			i = 0;
	int			nCharactersBeforeSpace = 0;

	while (i < nLength)
	{
		if ( isspace((unsigned char)szText[i]) )
			nCharactersBeforeSpace = 0;
		else
			nCharactersBeforeSpace++;

		if (nCharactersBeforeSpace > kMaxCharactersBeforeWrapping)
		{
			szText.Insert(i, '\r');
			i++;
			nLength++;

			nCharactersBeforeSpace = 0;
		}

		i++;
	}

	SetWindowText(szText);
}


// ---------------------------------------------------------------------------
//		* PositionWindow											[Public]
// ---------------------------------------------------------------------------
//	Position the window based on the rect of the item, the window in which
//	the popup text will live, and the location of the cursor.

void
CPopupText::PositionWindow(
	CRect &			rectItem,
	CWnd *			pwndKeepInsideWindow,
	CPoint &		ptCursor)
{
	CString		szText;

	GetWindowText(szText);

	CClientDC	dc(this);

	//	Position the popup text below the item and slightly to the right of the
	//	current cursor location (note that this currently does nothing to handle
	//	cases where the text would appear off the screen).
	m_rectItem = rectItem;

	m_rectText.top = rectItem.bottom + m_sizeWindowMargins.cy;
	m_rectText.bottom = m_rectText.top + 10;
	m_rectText.left = ptCursor.x - m_nWindowWidthLeftOfCursor;
	m_rectText.right = ptCursor.x + m_nWindowWidthRightOfCursor;

	//	Now calculate the size of the popup text rect so that it will fit all
	//	of the text.
	DrawText(dc, szText, m_rectText, DT_CALCRECT | DT_WORDBREAK);

	//	Inflate the rectangle so that there's some margin between the
	//	window rectangle and the text.
	m_rectText.InflateRect(m_sizeTextMargins);

	//	Get the parent window rect in screen coordinates
	CRect		rectWindow;

	pwndKeepInsideWindow->GetClientRect(&rectWindow);

	pwndKeepInsideWindow->ClientToScreen(&rectWindow);


	if (m_rectText.Width() > rectWindow.Width())
	{
		//	If the rectangle is entirely too wide, then limit it.
		//	Note that this doesn't actually handle drawing the text
		//	correctly.
		ASSERT(0);
		m_rectText.left = rectWindow.left + m_sizeWindowMargins.cx;
		m_rectText.right = rectWindow.right - m_sizeWindowMargins.cx;
	}
	else
	{
		//	If the rectangle would be too far to the right or left
		//	then shift it.
		int		nMoveBy;
		
		if (m_rectText.right > rectWindow.right)
		{
			nMoveBy = m_rectText.right - rectWindow.right + m_sizeWindowMargins.cx;
			
			m_rectText.left -= nMoveBy;
			m_rectText.right -= nMoveBy;
		}

		if (m_rectText.left < rectWindow.left)
		{
			nMoveBy = rectWindow.left - m_rectText.left + m_sizeWindowMargins.cx;
			
			m_rectText.left += nMoveBy;
			m_rectText.right += nMoveBy;
		}
	}

	//	Set the window position
	SetWindowPos( NULL,
				  m_rectText.left, m_rectText.top,
				  m_rectText.Width(), m_rectText.Height(),
				  SWP_NOZORDER | SWP_NOACTIVATE );

	//	Deflate the text rect and translate back to client coordinates to prepare
	//	for text drawing.
	m_rectText.DeflateRect(m_sizeTextMargins);
	ScreenToClient(m_rectText);
}


// ---------------------------------------------------------------------------
//		* ShowDelayed												[Public]
// ---------------------------------------------------------------------------
//	Show window with delay. No delay means show now.

void
CPopupText::ShowDelayed(
	UINT				in_nMsec)
{
	if (in_nMsec == 0)
	{
		// no delay: show it now
		OnTimer(kDisplayTimer);
	}
	else
	{
		// delay: set time
		SetTimer(kDisplayTimer, in_nMsec, NULL);
	}
}


// ---------------------------------------------------------------------------
//		* Cancel													[Public]
// ---------------------------------------------------------------------------
//	Cancel text: kill timers and hide window

void
CPopupText::Cancel()
{
	if (m_hWnd)
	{
		KillTimer(kDisplayTimer);
		KillTimer(kCheckLocationTimer);
		ShowWindow(SW_HIDE);
	}
}


// ---------------------------------------------------------------------------
//		* RelayEvent												[Public]
// ---------------------------------------------------------------------------
//	

void
CPopupText::RelayEvent(
	MSG *			in_pMsg)
{
	if ( !IsWindowVisible() )
		return;
	
	UINT	message = in_pMsg->message;

	if ( (message >= WM_KEYFIRST) && (message <= WM_KEYLAST) )
	{
		//	User is doing something - dismiss current popup text
		Cancel();
	}
	else
	{
		switch (message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONDBLCLK:
			case WM_NCLBUTTONDOWN:
			case WM_NCLBUTTONDBLCLK:
			case WM_NCRBUTTONDOWN:
			case WM_NCRBUTTONDBLCLK:
			case WM_NCMBUTTONDOWN:
			case WM_NCMBUTTONDBLCLK:
				//	User is doing something - dismiss current popup text
				Cancel();
				break;
		}
	}
}



// ---------------------------------------------------------------------------
//		* OnTimer												[Protected]
// ---------------------------------------------------------------------------
//	Timer popped

void
CPopupText::OnTimer(
	UINT			nIDEvent)
{
	if (nIDEvent == kDisplayTimer)
	{
		//	Display myself and kill timer
		ShowWindow(SW_SHOWNA);
		Invalidate();
		UpdateWindow();
		KillTimer(kDisplayTimer);
		SetTimer(kCheckLocationTimer, 500, NULL);
	}

	if (nIDEvent == kCheckLocationTimer)
	{
		//	Check to see if the cursor is still inside the last item rect.
		CPoint				ptCursor;

		GetCursorPos(&ptCursor);

		if ( !m_rectItem.PtInRect(ptCursor) )
		{
			//	The cursor moved outside the last item rect - cancel the window.
			Cancel();
			KillTimer(kCheckLocationTimer);
		}
	}
}
