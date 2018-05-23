// AboutDlg.cpp : implementation file

//   Date: Wednesday, September 24, 1996
// Author: Scott Manjourides
//
// Copyright 1996, QUALCOMM Incorporated
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

#ifdef WIN32
#	include <afxcmn.h>
#	include <afxrich.h>
#endif

#include "resource.h"
#include "guiutils.h" // ShiftDown()
#include "utils.h" // QCLoadBitmap()
#include "rs.h" // CRString
#include "AboutDlg.h"
#include "SafetyPal.h"
#include "Automation.h"
#include "QCGraphics.h"
#include "TraceFile.h"
#include "QCUtils.h" // GotoURL()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/* ------------------------------------------------------------------------ */

#define kTimerDelay (50) // n/1000-ths delay bewteen timer events
#define kInitSpeed (1)   // Number of pixels to scroll with each timer event
#define kNumColumns (3)  // Can be 1,2, or 3
#define kInsetPixels (3) // Must be 3 for correct 3D border placement

#define safefree(p) { if (p) { free(p); (p) = NULL; } }

/* ------------------------------------------------------------------------ */

inline BOOL ControlDown()
  	{ return ((GetKeyState(VK_CONTROL) < 0) ? TRUE : FALSE); }

/* ------------------------------------------------------------------------ */

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnClickedOK)
	ON_BN_CLICKED(IDC_CREDITS, OnClickedCredits)
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_BN_CLICKED(IDC_HYPERLINK_BTN, OnHyperlinkBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

CAboutDlg::CAboutDlg(BOOL IsSplash, CWnd* pParentWnd)
	: CDialog(CAboutDlg::IDD, pParentWnd),
	m_LogoBitmapWidth(0),
	m_LogoBitmapHeight(0),
	m_pLogoBitmap0(NULL),
	m_InitializedCredits(FALSE),
	m_dcCredits(),
	m_dcRealCredits(),
	m_hCreditsBitmap(NULL),
	m_hOldCreditsBitmap(NULL),
	m_hRealCreditsBitmap(NULL),
	m_hOldRealCreditsBitmap(NULL),
	m_nLeftPos(kInsetPixels),
	m_nTopPos(kInsetPixels),
	m_nTextLineHeight(0),
	m_InvalidateRct(),
	m_TimerID(0),
	m_CurrentScrollPosition(0),
	m_MaxScroll(0),
	m_ScrollSpeed(0),
	m_CreditsHeight(0),
	m_CreditsWidth(0),
	m_TitleFont(),
	m_NameFont()
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_Version = CRString(IDS_VERSION);
	//}}AFX_DATA_INIT
	
	// Look up the reg number
	CString regNum = GetIniString(IDS_INI_REG_NUMBER);
	// And append it if there
	if ( regNum.GetLength( ) != 0 )
	{
		m_Version += " Reg #";
		m_Version += regNum;
	}

	QCLoadBitmap(MAKEINTRESOURCE(IDB_LOGO), m_LogoBitmap);

	BITMAP bm;
	if (!m_LogoBitmap.GetBitmap(&bm)) return;
	m_LogoBitmapWidth = bm.bmWidth;
	m_LogoBitmapHeight = bm.bmHeight;

	if (m_IsSplash = IsSplash)
	{
		// Create modeless dialog
		Create(CAboutDlg::IDD);
		
		// Set and get rid of some styles we don't want for splash screens
		SetWindowLong(m_hWnd, GWL_STYLE, GetStyle() & ~(WS_SYSMENU));
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetExStyle() | WS_EX_TOOLWINDOW);
			
		// Make window stay app-topmost
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		
		// Hide buttons
		CWnd* p_button = GetDlgItem(IDOK);
		if (p_button)
			p_button->ShowWindow(SW_HIDE);
		p_button = GetDlgItem(IDC_CREDITS);
		if (p_button)
			p_button->ShowWindow(SW_HIDE);

		//
		// Resize height of window to just show bitmap.
		//
		CRect rect;
		GetWindowRect(&rect);
		rect.bottom = rect.top + m_LogoBitmapHeight + (kInsetPixels * 2) + 2;
		SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
			SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOMOVE | SWP_NOZORDER);

		// Center it in the main window (or screen in app is minimized)
		CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);

		//
		// Set a 4 second timer to destroy the splash screen.
		//
		if ((m_TimerID = SetTimer(42, 4000, NULL)) == 0)
		{
			DestroyWindow();
			return;
		}

		// Show the window
		ShowWindow(SW_SHOW);
		
		// Make sure it gets displayed right away
		UpdateWindow();
	}
}

/* ------------------------------------------------------------------------ */

CAboutDlg::~CAboutDlg()
{
	// nothing to do
}

/* ------------------------------------------------------------------------ */

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// display the hourglass if this is the splash screen
	if ((m_IsSplash) && (m_pParentWnd))
	{
		m_pParentWnd->BeginWaitCursor();
	}

	// Get temporary DC for dialog - Will be released in dc destructor
	CClientDC dc(this);

	// Create compatible memory DC using the dialogs DC
	m_dcLogo.CreateCompatibleDC(&dc);
 
	//
	// Select bitmap into memory DC.  
	//
	m_pLogoBitmap0 = m_dcLogo.SelectObject(&m_LogoBitmap);
	ASSERT(m_pLogoBitmap0);
	//
	// Note local CClientDC destructor for 'dc' will release device context
	//

	m_InvalidateRct.SetRect(m_nLeftPos, m_nTopPos, m_nLeftPos + m_LogoBitmapWidth, m_nTopPos + m_LogoBitmapHeight);

	//
	// Step 1.  Resize the width of the dialog's client area to exactly
	// fit the bitmap width.
	//
	CRect rect;
	GetWindowRect(&rect);
	SetWindowPos(NULL, 0, 0, m_LogoBitmapWidth + (kInsetPixels * 2) + 2, rect.Height(),
		SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOMOVE | SWP_NOZORDER);

	//
	// Step 2.  Center the static text labels (and while we're
	// at it, set the labels to use the standard "thin" font.
	//
	CenterControl(GetDlgItem(IDC_ABOUT_COPYRIGHT));
	CenterControl(GetDlgItem(IDC_VERSION));
	CenterControl(GetDlgItem(IDC_ABOUT_EXTRA_INFO));

	//
	// Step 3.  Position the buttons.
	//
	CWnd* p_button = GetDlgItem(IDC_CREDITS);
	if (p_button)
	{
		p_button->GetWindowRect(&rect);
		ScreenToClient(&rect);
		p_button->SetWindowPos(NULL, 40, rect.top, 0, 0,
			SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER);
	}
	else
		ASSERT(0);

	p_button = GetDlgItem(IDOK);

	if (p_button)
	{
		p_button->GetWindowRect(&rect);
		ScreenToClient(&rect);
		p_button->SetWindowPos(NULL, m_LogoBitmapWidth - (rect.Width() + 40), rect.top, 0, 0,
			SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER);
	}
	else
		ASSERT(0);

	//
	// Step 4.  Position the dialog relative to the main Eudora window.
	//
	if (!m_IsSplash)
		CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_VERSION, m_Version);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate == FALSE)
	{
	}
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::PostNcDestroy()
{
	if (m_IsSplash)
		delete this;		// to cover modeless dialog (splash screen) cleanup
	else
		CDialog::PostNcDestroy();
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::OnPaint()
{
	CPaintDC dc(this);			// device context for painting

	// Four cases
	//   1: Logo only (normal)
	//   2: Logo on top, credits on bottom
	//   3: Only credits
	//   4: Credits on top, logo on bottom

	ASSERT(m_CurrentScrollPosition <= m_MaxScroll);

	CDC *pWhichCreditsDC = NULL;

	if (ControlDown())
		pWhichCreditsDC = &m_dcRealCredits;
	else
		pWhichCreditsDC = &m_dcCredits;

	if (m_CurrentScrollPosition == 0) // Case 1: Logo only (normal)
	{
		dc.BitBlt(m_nLeftPos, m_nTopPos, m_LogoBitmapWidth, m_LogoBitmapHeight, &m_dcLogo, 0, 0, SRCCOPY);
	}
	else if (m_CurrentScrollPosition < m_LogoBitmapHeight) // Case 2: Logo on top, credits on bottom
	{
		UINT nTopHieght = (m_LogoBitmapHeight - m_CurrentScrollPosition);

		// Do partial logo (TOP)
		dc.BitBlt(m_nLeftPos, m_nTopPos,
			m_LogoBitmapWidth, nTopHieght,
			&m_dcLogo,
			0, m_CurrentScrollPosition,
			SRCCOPY);

		// Do credits (BOTTOM)
		dc.BitBlt(m_nLeftPos, nTopHieght + m_nTopPos,
			m_LogoBitmapWidth, m_LogoBitmapHeight - nTopHieght,
			pWhichCreditsDC,
			0, 0,
			SRCCOPY);
	}
	else if (m_CurrentScrollPosition < m_CreditsHeight) // Case 3: Only credits
	{
		dc.BitBlt(m_nLeftPos, m_nTopPos,
			m_LogoBitmapWidth, m_LogoBitmapHeight,
			pWhichCreditsDC,
			0, m_CurrentScrollPosition - m_LogoBitmapHeight,
			SRCCOPY);
	}
	else // Case 4: Credits on top, logo on bottom
	{
		UINT nTopHieght = (m_LogoBitmapHeight + m_CreditsHeight) - m_CurrentScrollPosition;

		// Do credits (TOP)
		dc.BitBlt(m_nLeftPos, m_nTopPos,
			m_LogoBitmapWidth, nTopHieght,
			pWhichCreditsDC,
			0, (m_CurrentScrollPosition - m_LogoBitmapHeight),
			SRCCOPY);

		// Do partial logo (BOTTOM)
		dc.BitBlt(m_nLeftPos, m_nTopPos + nTopHieght,
			m_LogoBitmapWidth, m_LogoBitmapHeight - nTopHieght,
			&m_dcLogo,
			0, 0,
			SRCCOPY);
	}

	// Draw the 3D border
	CRect BorderRct = m_InvalidateRct;
	BorderRct.InflateRect(2,2);
	CBrush whiteBrush(RGB(255,255,255)), *pBrush;
	pBrush = &whiteBrush;
	dc.FrameRect(LPCRECT(BorderRct), pBrush );
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// Put back the old logo-DC bitmap
	if (m_pLogoBitmap0)
	{
		m_dcLogo.SelectObject(m_pLogoBitmap0);
		m_pLogoBitmap0 = NULL;
	}

	// If the credits stuff was created
	if (m_InitializedCredits)
	{
		// ----- MAIN CREDITS -----

		// Put back the old bitmap
		ASSERT(m_hOldCreditsBitmap);
		::SelectObject(m_dcCredits.GetSafeHdc(), m_hOldCreditsBitmap);
		m_hOldCreditsBitmap = NULL;

		// Delete our credits bitmap
		ASSERT(m_hCreditsBitmap);
		::DeleteObject(m_hCreditsBitmap);
		m_hCreditsBitmap = NULL;

		// ----- ALTERNATE CREDITS -----

		// Put back the old bitmap
		ASSERT(m_hOldRealCreditsBitmap);
		::SelectObject(m_dcRealCredits.GetSafeHdc(), m_hOldRealCreditsBitmap);
		m_hOldRealCreditsBitmap = NULL;

		// Delete our credits bitmap
		ASSERT(m_hRealCreditsBitmap);
		::DeleteObject(m_hRealCreditsBitmap);
		m_hRealCreditsBitmap = NULL;


		m_InvalidateRct.SetRect(0,0,0,0);

		m_CurrentScrollPosition = 0;
		m_MaxScroll = 0;
		m_ScrollSpeed = 0;
		m_CreditsHeight = 0;
		m_CreditsWidth = 0;

		m_InitializedCredits = FALSE;
	}

	if (m_TimerID != 0)
	{
		KillTimer(m_TimerID);
		m_TimerID = 0;
	}

	// Reset the cursor, if needed
	if ((m_IsSplash) && (m_pParentWnd))
		m_pParentWnd->EndWaitCursor();
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::OnTimer(UINT nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);

	if (m_IsSplash)
	{
		//
		// Got splash screen timer
		//
		if (m_TimerID != 0)
		{
			ASSERT(42 == m_TimerID);
			KillTimer(m_TimerID);		// just in case
		}

		// Maximum time has exceeded, so hide the splash screen
//		ShowWindow(SW_HIDE);
		return;
	}

	// Hold down shift while scrolling to scroll faster
	if (ShiftDown())
		m_CurrentScrollPosition += (m_ScrollSpeed * 4);
	else
		m_CurrentScrollPosition += m_ScrollSpeed;

	// Check for "wrap around"
	if (m_CurrentScrollPosition > m_MaxScroll)
		StopScrollTimer();

	// Make the dialog update the scrolled region
	InvalidateRect(LPCRECT(m_InvalidateRct), FALSE);
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::OnClickedCredits()
{
	// If there is no timer running
	if (m_TimerID == 0)
	{
		// If we actually get the timer started
		if ((m_TimerID = StartScrollTimer()) != 0)
		{
			// If there is no credits bitmap, then this
			// must be the first time they clicked "more"
			if (!m_InitializedCredits)
			{
				CClientDC dc(this);
				m_dcCredits.CreateCompatibleDC(&dc);
				m_dcRealCredits.CreateCompatibleDC(&dc);

				m_dcCredits.SetBkColor(RGB(0,0,0)); // Set background color to black
				m_dcCredits.SelectStockObject(WHITE_PEN); // Set pen to white
				m_dcCredits.SetTextColor(RGB(255,255,255)); // White text
				m_dcCredits.SelectStockObject(ANSI_VAR_FONT); // Choose the default 'skinny' font

				m_dcRealCredits.SetBkColor(RGB(0,0,0)); // Set background color to black
				m_dcRealCredits.SelectStockObject(WHITE_PEN); // Set pen to white
				m_dcRealCredits.SetTextColor(RGB(255,255,255)); // White text
				m_dcRealCredits.SelectStockObject(ANSI_VAR_FONT); // Choose the default 'skinny' font

				// Create both normal and bold fonts for later use
				CFont ANSIFont;
				ANSIFont.CreateStockObject(ANSI_VAR_FONT);
				LOGFONT fontdata;
				ANSIFont.GetObject(sizeof(LOGFONT), &fontdata);
				
				m_NameFont.CreateFontIndirect(&fontdata); // NORMAL
				fontdata.lfWeight = 700; 
				m_TitleFont.CreateFontIndirect(&fontdata); // BOLD

				// Calc the height of a line of text
				LPTEXTMETRIC pTextStats = (LPTEXTMETRIC) malloc(sizeof(TEXTMETRIC));
				ASSERT(pTextStats);
				m_dcCredits.GetOutputTextMetrics(pTextStats);
				m_nTextLineHeight = pTextStats->tmHeight + 4;
				safefree(pTextStats);

				char *buffer = NULL, *realbuffer = NULL;

				char **pCreditsList = LoadCreditsList(&buffer, IDT_CREDITS_LIST);
				char **pRealCreditsList = LoadCreditsList(&realbuffer, IDT_REAL_CREDITS_LIST);

				m_CreditsHeight = CalcCreditsHeight(pCreditsList);
				m_CreditsWidth = m_LogoBitmapWidth;
								
				m_hRealCreditsBitmap = CreateCompatibleBitmap(dc.GetSafeHdc(), m_CreditsWidth, m_CreditsHeight);
				m_hOldRealCreditsBitmap = HBITMAP(::SelectObject(m_dcRealCredits.GetSafeHdc(), m_hRealCreditsBitmap));
				ASSERT(m_hOldRealCreditsBitmap);

				m_hCreditsBitmap = CreateCompatibleBitmap(dc.GetSafeHdc(), m_CreditsWidth, m_CreditsHeight);
				m_hOldCreditsBitmap = HBITMAP(::SelectObject(m_dcCredits.GetSafeHdc(), m_hCreditsBitmap));
				ASSERT(m_hOldCreditsBitmap);

				DrawCredits(&m_dcCredits, pCreditsList);
				DrawCredits(&m_dcRealCredits, pRealCreditsList);

				safefree(pCreditsList);
				safefree(buffer);

				safefree(pRealCreditsList);
				safefree(realbuffer);

				m_MaxScroll = m_LogoBitmapHeight + m_CreditsHeight;

				m_InitializedCredits = TRUE;
			}

			m_CurrentScrollPosition = 0;
			m_ScrollSpeed = kInitSpeed;
		}
	}
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::OnClickedOK()
{
	StopScrollTimer();
	CDialog::OnOK();
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::DrawCredits(CDC *pDC, char **pCreditsList)
{
	const int nWidth = m_LogoBitmapWidth;
	const int nHeight = m_CreditsHeight;

	// Erase the bitmap (BLACK)
	CRect rct(0, 0, nWidth, nHeight);
	CBrush blackBrush(RGB(0,0,0)), *pBrush;
	pBrush = &blackBrush;
	pDC->FillRect(LPCRECT(rct), pBrush);

	// All text will be centered
	pDC->SetTextAlign(TA_CENTER);
	pDC->SetTextColor(RGB(255,255,255)); // White text

	// Find the center of the bitmap -- used for group titles
	const int nCenterPos = nWidth / 2;
	int ColPos[10];

	// Evenly space the columns horizontally
	for (int n=0; n < kNumColumns; n++)
		ColPos[n] = (((2 * n) + 1) * nWidth)/(2 * kNumColumns);

	int nVertPos = 20; // Leave some space from top before start text
	char **cpp = pCreditsList;

	// We have to start with a group -- so make sure we're at a group header
	while ((*cpp) && (*cpp[0] != '.'))
		cpp++;
	
	while (*cpp)
	{
		// Skip to the start of the next group
		while ((*cpp) && (*cpp[0] != '.'))
			cpp++;

		if (*cpp) // Starting new section
		{
			// Switch font to BOLD
			pDC->SelectObject(&m_TitleFont);

			// Output the group title
			pDC->TextOut(nCenterPos, nVertPos, CString((*cpp) + 1), strlen((*cpp) + 1));

			// Jump past text we just outputted
			nVertPos += m_nTextLineHeight;

			// Draw separator line under group title
			pDC->MoveTo(10,nVertPos);
			pDC->LineTo(nWidth-11, nVertPos);

			// Skip some space after line
			nVertPos += (m_nTextLineHeight/2);

			// Find the next group -- calc this group size
			char **next = cpp + 1;
			while ((*next) && (*next[0] != '.'))
				next++;
			int nGroupCount = (next - cpp) - 1;

			// Calc how many names (max) will be in each column
			int nPerColumn = ((nGroupCount + (kNumColumns - 1)) / kNumColumns);

			// Keep the top vert pos to calc name positions from
			int nTopVert = nVertPos;

			// Set the actual vert pos to past this group
			nVertPos += ((nPerColumn + 2) * m_nTextLineHeight);

			int nThisColumn = 0;
			int nInColumn = 0;

			// Switch font to NORMAL
			pDC->SelectObject(&m_NameFont);

			// Do the group
			while (nGroupCount--)
			{
				// Move to next name in this group
				cpp++;

				// Put out this name in the correct position
				pDC->TextOut(ColPos[nThisColumn],
					nTopVert + (nInColumn * m_nTextLineHeight),
					CString(*cpp), strlen(*cpp));

				// Fills down each column first, then moves to next column
				
				if ((++nInColumn) >= nPerColumn)
				{
					nInColumn = 0;
					nThisColumn++;
				}
			}
		}
	}
}

/* ------------------------------------------------------------------------ */

int CAboutDlg::CalcCreditsHeight(char **pCreditsList)
{
	int nVertPos = 20; // Leave some space from top before start text
	char **cpp = pCreditsList;

	// We have to start with a group -- so make sure we're at a group header
	while ((*cpp) && (*cpp[0] != '.'))
		cpp++;
	
	while (*cpp)
	{
		// Skip to the start of the next group
		while ((*cpp) && (*cpp[0] != '.'))
			cpp++;

		if (*cpp) // Starting new section
		{
			nVertPos += m_nTextLineHeight;
			nVertPos += (m_nTextLineHeight/2);

			// Find the next group -- calc this group size
			char **next = cpp + 1;
			while ((*next) && (*next[0] != '.'))
				next++;
			int nGroupCount = (next - cpp) - 1;

			cpp = next;

			// Calc how many names (max) will be in each column
			int nPerColumn = ((nGroupCount + (kNumColumns - 1)) / kNumColumns);

			nVertPos += ((nPerColumn + 2) * m_nTextLineHeight);
		}
	}

	return (nVertPos);
}

////////////////////////////////////////////////////////////////////////
// CenterControl [protected]
//
// Center the textual label with respect to the current size of the
// dialog window.  Also, set the font to the standard system "thin"
// font.
////////////////////////////////////////////////////////////////////////
void CAboutDlg::CenterControl(CWnd* pControl)
{
	if (NULL == pControl)
	{
		ASSERT(0);
		return;
	}

	// Force the standard "thin" font for all static text labels.
	pControl->SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));

	CRect dlgrect;
	GetWindowRect(&dlgrect);
	CRect labelrect;
	pControl->GetWindowRect(&labelrect);
	const int DELTA = (dlgrect.left + (dlgrect.Width() / 2)) -
					  (labelrect.left + (labelrect.Width() / 2));

	ScreenToClient(&labelrect);
	pControl->SetWindowPos(NULL, labelrect.left + DELTA, labelrect.top, 0, 0,
		SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER);
}

/* ------------------------------------------------------------------------ */

char **CAboutDlg::LoadCreditsList(char **buffer, int nResourceID)
{
	HINSTANCE hInst = QCFindResourceHandle( (LPCSTR)nResourceID, "TEXTDATA" );
	HRSRC hResInfo;
	HGLOBAL hResText;
	const char *pResStr = NULL;
	
	if ((hResInfo = ::FindResource(hInst, (LPCSTR)nResourceID, "TEXTDATA")) &&
		(hResText = ::LoadResource(hInst, hResInfo)))
	{
		if (pResStr = (const char*)::LockResource(hResText))
		{
			DWORD nResLen = ::SizeofResource(hInst, hResInfo);

			*buffer = (char *) malloc(nResLen + 1);
			ASSERT(*buffer);

			strncpy(*buffer, pResStr, nResLen);
			(*buffer)[nResLen] = '\0';

			::UnlockResource(hResText);
		}
		::FreeResource(hResText);
	}

	char *cp;
	UINT nLineCount = 0;

	// Just count the lines first
	cp = *buffer;
	do {
		if ((*cp) == '#')
			break;

		nLineCount++;

		// Move to end of line
		while ((*cp) && (*cp != '\r') && (*cp != '\n'))
			cp++;

		// Move to beg of next line
		while ((*cp) && ((*cp == '\r') || (*cp == '\n')))
			cp++;
	} while (*cp);

	// Now that we know the number of lines, setup a buffer for 'em
	char **pBufLines = (char **) malloc((nLineCount + 1) * sizeof(char *));
	ASSERT(pBufLines);

	char **pPos = pBufLines;
	
	// Now parse the buffer and make hooks into each line
	cp = *buffer;
	do {
		if ((*cp) == '#')
			break;

		*pPos++ = cp;

		// Move to end of line
		while ((*cp) && (*cp != '\r') && (*cp != '\n'))
			cp++;

		if (*cp)
		{
			*cp++ = '\0';

			// Move to beg of next line
			while ((*cp) && ((*cp == '\r') || (*cp == '\n')))
				cp++;
		}
	} while (*cp);

	*pPos = NULL;

	return (pBufLines);
}

/* ------------------------------------------------------------------------ */

UINT CAboutDlg::StartScrollTimer()
{
	UINT ret;

	if (ret = SetTimer(1, kTimerDelay, NULL))
	{
		// Disable the button
		CWnd *pButton = GetDlgItem(IDC_CREDITS);
		pButton->EnableWindow(FALSE);

		// Move the focus to the OK button
		pButton = GetDlgItem(IDOK);
		pButton->SetFocus();
	}

	return (ret);
}

/* ------------------------------------------------------------------------ */

void CAboutDlg::StopScrollTimer()
{
	if (m_TimerID != 0)
	{
		KillTimer(m_TimerID);
		m_TimerID = 0;
		m_CurrentScrollPosition = 0;

		// Enable the button
		CWnd *pButton = GetDlgItem(IDC_CREDITS);
		pButton->EnableWindow(TRUE);
	}
}

BOOL CAboutDlg::OnQueryNewPalette()
{
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void CAboutDlg::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged((CWnd*)this, pFocusWnd);
}

void CAboutDlg::OnSysColorChange()
{
	m_Palette.DoSysColorChange((CWnd*)this);
}

void CAboutDlg::OnHyperlinkBtn() 
{
	CRString sURL(IDS_MAIN_WEB_URL);
	GotoURL(sURL, SW_SHOW);
}
