/////////////////////////////////////////////////////////////////////////////
// 	File: MDICHILD.CPP 
//
//  Purpose:  This file provides the declarations for the base window class
//			 for the eudora child windows...all should derive from here...
//
//	Author: Jeff Beckley
//	Created:   9/1/94
//  Modified:
//			  11/2/94	JCP		Added GetWindowType..
//            9/26/98   Scott Manjourides: Added WM_GETMINMAXINFO handler and SetMinTrackSize
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "mdichild.h"
#include "resource.h"
#include "rs.h"
#include "doc.h"
#include "mainfrm.h"
#include "signatur.h"

#include "DebugNewHelpers.h"

// CMDIChild

IMPLEMENT_DYNCREATE(CMDIChild, QCWorksheet)

BEGIN_MESSAGE_MAP(CMDIChild, QCWorksheet)
	//{{AFX_MSG_MAP(CMDIChild)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMDIChild::CMDIChild()
	: m_bMinTrackValid(FALSE)

{
}

CMDIChild::~CMDIChild()
{
}

BOOL CMDIChild::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle /*= WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE*/,
	CWnd* pParentWnd /*= NULL*/, CCreateContext* pContext /*= NULL*/)
{
	BOOL Status = QCWorksheet::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);
	
	if (Status)
	{
		UINT INIWindowPos = 0;
		switch (GetTemplateID())
		{
		case IDR_SEARCH_WND	:	INIWindowPos = IDS_INI_SEARCH_WIN_POS;			break;
		case IDR_TOC :			INIWindowPos = IDS_INI_MAILBOXES_WIN_POS;		break;
		case IDR_TEXTFILE :		INIWindowPos = IDS_INI_TEXT_FILE_WIN_POS;		break;
		case IDR_SIGNATURES :	INIWindowPos = IDS_INI_SIGNATURE_WIN_POS;		break;
#ifdef IMAP4 //
		case IDR_IMAP_FILTERS:  INIWindowPos = IDS_INI_IMAP_FILTERS_WIN_POS;	break;
#endif // IMAP4
		}
		
		CRect rect;
		if (INIWindowPos && GetIniWindowPos(INIWindowPos, rect))
		{
			if ((rect.Width() > 0) && (rect.Height() > 0))
				MoveWindow(rect, FALSE);
		}
	}
	
	return (Status);
}

int CMDIChild::m_nCmdShow = -1;

void CMDIChild::ActivateFrame(int nCmdShow /*= -1*/)
{
	WINDOWPLACEMENT		wp;
	BOOL				bGetWindowPlacedSuceeded = GetWindowPlacement(&wp);
	CMainFrame*			Main = (CMainFrame*)AfxGetMainWnd();
	CMDIChildWnd*		Win = Main->MDIGetActive();

	//	It's harmful (bug 5196) to call MoveWindow when the CMDIChild is already
	//	active and maximized. MoveWindow was making the window smaller but
	//	QCWorksheet::ActivateFrame was not re-maximizing it. This resulted
	//	in a bizarre inconsistent state (in the case of 5196 every time the
	//	user did Find Text in a maximized window).
	//	So avoid that particular case, but allow MoveWindow in every other case
	//	(for now - we may want to change this in the future because MoveWindow is
	//	often superfluous for maximized windows, but if the window isn't active
	//	it's not harmful).
	if ( (Win->GetSafeHwnd() != this->GetSafeHwnd()) ||
		 (bGetWindowPlacedSuceeded && (wp.showCmd != SW_SHOWMAXIMIZED)) )
	{
		CRect MainRect;
		BOOL NoSize = ((GetStyle() & WS_THICKFRAME) == 0);
		
		// Check window size to make sure it's reasonable
		wp.length = sizeof(wp);
		if (!bGetWindowPlacedSuceeded)
		{
			GetWindowRect(&wp.rcNormalPosition);
			Main->ScreenToClient(&wp.rcNormalPosition);
		}
		Main->GetRealClientRect(MainRect);
		CRect rect(wp.rcNormalPosition);
		if (NoSize)
		{
			// If this window can't be resized, then save the fixed width and height
			// in order to move the window to be visible
			int Width = rect.Width();
			int Height = rect.Height();

			if (rect.left < MainRect.left)
				rect.left = MainRect.left;
			else if (rect.left > MainRect.right)
				rect.left = MainRect.right - Width;
			if (rect.top < MainRect.top)
				rect.top = MainRect.top;
			else if (rect.top > MainRect.bottom)
				rect.top = MainRect.bottom - Height;
				
			rect.right = rect.left + Width + GetSystemMetrics(SM_CXVSCROLL);
			rect.bottom = rect.top + Height + GetSystemMetrics(SM_CYHSCROLL);
			
	//		// Don't maximize this window if previous MDI child window was maximized
	//		BOOL Maxed;
	//		Main->MDIGetActive(&Maxed);
	//		if (Maxed)
	//		{
	//			if (nCmdShow == -1 && m_nCmdShow == -1)
	//				nCmdShow = SW_SHOWNORMAL;
	//		}
		}
		else
		{
			if (rect.left < MainRect.left || rect.left > MainRect.right)
				rect.left = MainRect.left;
			if (rect.right < MainRect.left || rect.right > MainRect.right)
				rect.right = MainRect.right;
			if (rect.top < MainRect.top || rect.top > MainRect.bottom)
				rect.top = MainRect.top;
			if (rect.bottom < MainRect.top || rect.bottom > MainRect.bottom)
				rect.bottom = MainRect.bottom;
		}

		MoveWindow(&rect, FALSE);
	}
	
	// If there isn't a specific window state to open in, and if there are no open windows, and
	// the last window was maximized when closed, then open up the next window maximized as well.
	if (nCmdShow == -1 && m_nCmdShow == -1 &&
		(Win == NULL || Win->IsWindowVisible() == FALSE) &&
		GetIniShort(IDS_INI_LAST_WINDOW_MAX))
	{
		nCmdShow = SW_SHOWMAXIMIZED;
	}

	QCWorksheet::ActivateFrame(nCmdShow == -1? m_nCmdShow : nCmdShow);
}
 

/////////////////////////////////////////////////////////////////////////////
// CMDIChild message handlers

void CMDIChild::OnDestroy()
{
	UINT INIWindowPos = 0;
	switch (GetTemplateID())
	{
		case IDR_SEARCH_WND	:	INIWindowPos = IDS_INI_SEARCH_WIN_POS;			break;
		case IDR_TEXTFILE:		INIWindowPos = IDS_INI_TEXT_FILE_WIN_POS;		break;
#ifdef IMAP4
		case IDR_IMAP_FILTERS:	INIWindowPos = IDS_INI_IMAP_FILTERS_WIN_POS;	break;
#endif // IMAP4
		case IDR_SIGNATURES:
		{
			CSignatureDoc* doc = (CSignatureDoc*)GetActiveDocument();
			if (doc)
			{
				INIWindowPos = IDS_INI_SIGNATURE_WIN_POS;
			}
		}
		break;
	}

	if (INIWindowPos)
	{
		WINDOWPLACEMENT wp;
		
		wp.length = sizeof(wp);
		if (GetWindowPlacement(&wp))
			SetIniWindowPos(INIWindowPos, wp.rcNormalPosition);
	}
	
	QCWorksheet::OnDestroy();
}

UINT CMDIChild::GetTemplateID() 
{ 
	if(IsKindOf(RUNTIME_CLASS(CMDIChild)))
		return (m_nIDHelp); 
	return (0);
}

void CMDIChild::OnGetMinMaxInfo(MINMAXINFO* lpMMI) 
{
	CFrameWnd::OnGetMinMaxInfo(lpMMI);

	if (m_bMinTrackValid)
		lpMMI->ptMinTrackSize = m_ptMinTrackSize;
}

void CMDIChild::SetMinTrackSize(POINT pt)
{
	m_ptMinTrackSize = pt;
	m_bMinTrackValid = TRUE;
}

POINT CMDIChild::GetMinTrackSize() const
{
	if (m_bMinTrackValid)
		return (m_ptMinTrackSize);

	return (CPoint(0,0));
}

