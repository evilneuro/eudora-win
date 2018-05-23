// QCFindMgr.cpp: implementation of the QCFindMgr class.
//
// Copyright (c) 1998-2000 by QUALCOMM, Incorporated
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
#include <afxdlgs.h> // CFindReplaceDialog
#include "FindDlg.h"
#include "GuiUtils.h" // ErrorDialog
#include "resource.h"

#include "QCFindMgr.h"


#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
//
// QCFindState
//
/////////////////////////////////////////////////////////////////////////////

//
// SetState [PUBLIC]
//

void QCFindState::SetState(CFindReplaceDialog *pDlg)
{
	ASSERT(pDlg);
	
	if (pDlg)
	{
		m_FindStr = pDlg->GetFindString();
		m_bMatchCase = (pDlg->MatchCase() == TRUE);
		m_bWholeWord = (pDlg->MatchWholeWord() == TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// QCFindMgr
//
/////////////////////////////////////////////////////////////////////////////

QCFindMgr *QCFindMgr::m_pTheFindMgr = NULL;

QCFindMgr gTheFindMgr; // The one and only object

// --------------------------------------------------------------------------

//
// QCFindMgr [PUBLIC]
//

QCFindMgr::QCFindMgr()
	: m_pWnd(NULL), m_pDlg(NULL), m_bValidFindState(false), m_DlgRct(0,0,0,0)
{
	ASSERT(NULL == m_pTheFindMgr);
	
	if (!m_pTheFindMgr) // Only the first object is the manager
		m_pTheFindMgr = this;
}

// --------------------------------------------------------------------------

//
// ~QCFindMgr [PUBLIC, VIRTUAL]
//

QCFindMgr::~QCFindMgr()
{
	if (this == m_pTheFindMgr)
		m_pTheFindMgr = NULL;
}

// --------------------------------------------------------------------------

//
// GetFindMgr [PUBLIC, STATIC]
//

QCFindMgr *QCFindMgr::GetFindMgr()
{
	ASSERT(m_pTheFindMgr);
	return (m_pTheFindMgr);
}

CFindDlg * QCFindMgr::GetFindDlg()
{
	return m_pDlg;
}

// --------------------------------------------------------------------------

//
// Create [PUBLIC]
//
// Brings up dlg (init dlg). Client is allowed to call this multiple times.

bool QCFindMgr::Create()
{
	ASSERT(m_pTheFindMgr);
	if (!m_pTheFindMgr)
		return (false);

	if (!m_pDlg)
	{
		m_pDlg = DEBUG_NEW CFindDlg();

		DWORD dwFlags = FR_HIDEUPDOWN;

		if (m_DlgState.IsMatchCase())
			dwFlags |= FR_MATCHCASE;

		if (m_DlgState.IsWholeWord())
			dwFlags |= FR_WHOLEWORD;

		m_pDlg->Create(TRUE, m_DlgState.GetFindText(), NULL, dwFlags, AfxGetMainWnd());

		// This causes a slight UI flicker. The window is created and shown in
		// it's default position, then we move it where we want it.
		//
		// There seems to be no way to avoid the Create() from showing the window.
		//
		if ((m_DlgRct.Width() > 0) && (m_DlgRct.Height() > 0))
			m_pDlg->MoveWindow(m_DlgRct);
	}
	else
		m_pDlg->SetActiveWindow();

	return (true);
}

// --------------------------------------------------------------------------

//
// Destroy [PUBLIC]
//

bool QCFindMgr::Destroy() // Call when dlg is going away (saves state)
{

	CWnd *pWnd = GetFindWnd();
	ASSERT_VALID(pWnd);

	if (pWnd)
		pWnd->PostMessage(WM_ACTIVATE);


	ASSERT(m_pTheFindMgr);
	ASSERT(m_pDlg);
	if ((!m_pDlg) || (!m_pTheFindMgr))
		return (false);

	m_DlgState.SetState(m_pDlg);
	m_pDlg->GetWindowRect(m_DlgRct);

	ASSERT(m_pDlg->IsTerminating());
	
	// Don't delete anything, MFC cleans itself
	m_pDlg = NULL;

	return (true);
}

// --------------------------------------------------------------------------


//
// UpdateLastFindState [PUBLIC]
//

bool QCFindMgr::UpdateLastFindState(LPCTSTR sFindStr, bool bMatchCase, bool bWholeWord)
{
	ASSERT(m_pTheFindMgr);
	if (!m_pTheFindMgr)
		return (false);

	m_FindState.SetState(sFindStr, bMatchCase, bWholeWord);
	m_bValidFindState = true;

	return (true);
}

// --------------------------------------------------------------------------

//
// UpdateLastFindState [PUBLIC]
//

bool QCFindMgr::UpdateLastFindState() // Call this with each find -- to save state
{
	ASSERT(m_pTheFindMgr);
	ASSERT(m_pDlg);

	if ((!m_pDlg) || (!m_pTheFindMgr))
		return (false);

	m_FindState.SetState(m_pDlg);
	m_bValidFindState = true;

	return (true);
}

// --------------------------------------------------------------------------

//
// GetLastFindState [PUBLIC]
//

bool QCFindMgr::GetLastFindState(QCFindState &state) const // Call this to get Find Again info (F3)
{
	ASSERT(m_pTheFindMgr);

	if (!m_bValidFindState)
		return (false);

	state = m_FindState;
	return (true);
}

// --------------------------------------------------------------------------

//
// GetLastFindState [PUBLIC]
//

bool QCFindMgr::CanFindAgain() const
{
	return (m_bValidFindState);
}

// --------------------------------------------------------------------------

//
// SetFindWnd [PUBLIC]
//

bool QCFindMgr::SetFindWnd(CWnd *pWnd)
{
	m_pWnd = pWnd;
	
	return (true);
}

// --------------------------------------------------------------------------

//
// GetFindWnd [PUBLIC]
//

CWnd *QCFindMgr::GetFindWnd() const
{
	return (m_pWnd);
}

// --------------------------------------------------------------------------

//
// SendFindMsg [PROTECTED]
//

bool QCFindMgr::SendFindMsg(WPARAM wParam, LPARAM lParam)
{
	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if (!lpFR)
		return (false);


	CWnd *pWnd = GetFindWnd();
	ASSERT_VALID(pWnd);

	if (pWnd)
	{
		switch (pWnd->SendMessage(WM_FINDREPLACE, wParam, lParam))
		{
			case EuFIND_ERROR:
			{
				ASSERT(0); // Something went wrong
				return (false);
			}
			break;

			case EuFIND_NOTFOUND:
			{
				DisplayNotFound(lpFR->lpstrFindWhat);
			}
			break;
		}
	}
	else
		VERIFY(MessageBeep(MB_ICONEXCLAMATION)); // The user is trying to find in a wnd which does not support find

	return (true);
}

// --------------------------------------------------------------------------

//
// DoFind [PUBLIC]
//

bool QCFindMgr::DoFind(WPARAM wParam, LPARAM lParam)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr == this);

	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if ((!lpFR) || (pFindMgr != this))
		return (false);
	
	if (lpFR->Flags & FR_DIALOGTERM)
	{
		// If the find dlg is closing, then save some state (don't find)
		Destroy();
	}
	else
	{
		// Every time we do a find, save the find state to be used for
		// subsequent Find Again (F3)
		UpdateLastFindState();
		if (!SendFindMsg(wParam, lParam))
			return (false);
	}

	return (true);
}

// --------------------------------------------------------------------------

//
// DoFindAgain [PUBLIC]
//

bool QCFindMgr::DoFindAgain()
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr == this);

	if (pFindMgr != this)
		return (false);

	if (CanFindAgain())
	{
		UpdateFindFocus();

		CWnd *pWnd = GetFindWnd();
		if (pWnd)
		{
			FINDREPLACE FR;

			// Clear the structure
			FR.lStructSize = (DWORD) sizeof(FINDREPLACE);
			FR.hwndOwner = (HWND) NULL;
			FR.hInstance = (HINSTANCE) NULL;
			FR.Flags = (DWORD) 0;
			FR.lpstrFindWhat = (LPTSTR) NULL;
			FR.lpstrReplaceWith = (LPTSTR) NULL;
			FR.wFindWhatLen = (WORD) 0;
			FR.wReplaceWithLen = (WORD) 0;
			FR.lCustData = (LPARAM) 0;
			FR.lpfnHook = (LPFRHOOKPROC) NULL;
			FR.lpTemplateName = (LPCTSTR) NULL;

			// Get the find state info
			QCFindState FindState;
			VERIFY(GetLastFindState(FindState));

			// Now fill the block with the data
			FR.lpstrFindWhat = (LPTSTR) FindState.GetFindText();
			FR.wFindWhatLen = (WORD) FindState.GetFindTextLen();

			if (FindState.IsMatchCase())
				FR.Flags |= FR_MATCHCASE;

			if (FindState.IsWholeWord())
				FR.Flags |= FR_WHOLEWORD;

			LPFINDREPLACE lpFR = &FR;
			LPARAM lParam = (LPARAM) lpFR;;
			WPARAM wParam = (WPARAM) 0;

			// pWnd->SendMessage(WM_FINDREPLACE, wParam, lParam);
			if (!SendFindMsg(wParam, lParam))
				return (false);
		}
		else
		{
			// The user is trying to find again in a wnd which does not support find
			VERIFY(MessageBeep(MB_ICONEXCLAMATION));
		}
	}
	else
	{
		// Cannot Find Again -- probably means someone is incorrectly handling the ON_UPDATE_COMMAND_UI for ID_EDIT_FIND_FINDTEXTAGAIN
		VERIFY(MessageBeep(MB_ICONEXCLAMATION));
	}

	return (true);
}

// --------------------------------------------------------------------------

//
// DisplayNotFound [PROTECTED]
//

void QCFindMgr::DisplayNotFound(LPCTSTR pStr)
{
	::ErrorDialog(IDS_FIND_NOT_FOUND, pStr);
}

// --------------------------------------------------------------------------

//
// IsAck [PUBLIC, STATIC]
//

bool QCFindMgr::IsAck(WPARAM wParam, LPARAM lParam)
{
	return ((1 == wParam) && (0 == lParam));
}

// --------------------------------------------------------------------------

//
// SendAck [PUBLIC, STATIC]
//

LONG QCFindMgr::SendAck(CWnd *pWnd)
{
	ASSERT(pWnd);
	if (!pWnd)
		return (EuFIND_ERROR);

	return (pWnd->SendMessage(WM_FINDREPLACE, 1, 0));
}

// --------------------------------------------------------------------------

//
// UpdateFindFocus [PUBLIC]
//

void QCFindMgr::UpdateFindFocus()
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr == this);

	if (pFindMgr != this)
		return;

	// Get the focus from where the user is coming from (before find)
	CWnd *pFocus = CWnd::GetFocus();

	// Walk through parent ptrs until someone handles WM_FINDREPLACE
	while (pFocus)
	{
		if (SendAck(pFocus) == EuFIND_ACK_YES)
			break;

		pFocus = pFocus->GetParent();
	}

	// pFocus may be NULL, meaning find is not supported
	SetFindWnd(pFocus);
}
