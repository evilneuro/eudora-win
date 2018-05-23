// ValidEdit.cpp : implementation file
//

#include "stdafx.h"
#include "ValidEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// --------------------------------------------------------------------------
//
// CValidEdit class

IMPLEMENT_DYNAMIC(CValidEdit, CEdit)

BEGIN_MESSAGE_MAP(CValidEdit, CEdit)
  //{{AFX_MSG_MAP(CValidEdit)
  ON_WM_CHAR()
  //}}AFX_MSG_MAP
  ON_MESSAGE(WM_PASTE, OnPaste)
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

CValidEdit::CValidEdit()
{
	m_bUseValid = false;
	m_sValid.Empty();

	m_bUseInvalid = false;
	m_sInvalid.Empty();

	m_bSilent = false;
}

// --------------------------------------------------------------------------

void CValidEdit::SetInvalid(LPCSTR lpInvalid)
{
	m_bUseInvalid = false;
	m_sInvalid.Empty();

	if (lpInvalid)
	{
		m_sInvalid = lpInvalid;

		if (m_sInvalid.GetLength() > 0)
			m_bUseInvalid = true;
	}
}

// --------------------------------------------------------------------------

void CValidEdit::SetValid(LPCSTR lpValid)
{
	m_bUseValid = false;
	m_sValid.Empty();

	if (lpValid)
	{
		m_sValid = lpValid;

		if (m_sValid.GetLength() > 0)
			m_bUseValid = true;
	}
}

// --------------------------------------------------------------------------

bool CValidEdit::SetSilent(bool bSilent /* = true */)
{
	bool bOld = m_bSilent;
	m_bSilent = bSilent;

	return (bOld);
}

// --------------------------------------------------------------------------

void CValidEdit::SendChar(UINT nChar)
{
	m_bSilent = true;

#ifdef WIN32
	AfxCallWndProc(this, m_hWnd, WM_CHAR, nChar, 1);
#else
	SendMessage(WM_CHAR, nChar, 1);
#endif

	m_bSilent = false;
}

// --------------------------------------------------------------------------

void CValidEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!isprint((int)(unsigned char)nChar))
	{
		Default(); // Let all non-printable chars through
		return;
	}

	if (CheckChar(nChar))
	{
		Default();
	}
	else
	{
		if (!m_bSilent)
			::MessageBeep(0xFFFFFFFF);
	}
}

// --------------------------------------------------------------------------

bool CValidEdit::CheckChar(UINT nChar)
{
	if (!isprint((int)(unsigned char)nChar))
		return (true); // Let all non-printable char through

	bool bValid = true;

	if (m_bUseValid)
		bValid = (m_sValid.Find(char(nChar)) != (-1));

	if ((bValid) && (m_bUseInvalid))
		bValid = (m_sInvalid.Find(char(nChar)) == (-1));

	return (bValid);
}

// --------------------------------------------------------------------------

LRESULT CValidEdit::OnPaste(WPARAM, LPARAM)
{
	if (!OpenClipboard())
		return (0); // Can't get the clipboard data

	HANDLE hdle = ::GetClipboardData(CF_TEXT);

	if (hdle)
	{
		LPSTR sClipStr = (LPSTR) GlobalLock(hdle);

		bool bValid = true;
		while ((bValid) && (*sClipStr))
		{
			if (!CheckChar(*sClipStr))
				bValid = false;

			sClipStr++;
		}
		GlobalUnlock(hdle);
		CloseClipboard();


		if (bValid)
			Default();
		else
		{
			if (!m_bSilent)
				::MessageBeep(0xFFFFFFFF);
		}
	}
	else
		CloseClipboard();

	return (0);
}


