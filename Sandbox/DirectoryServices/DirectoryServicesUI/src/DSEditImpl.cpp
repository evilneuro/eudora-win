//////////////////////////////////////////////////////////////////////////////
// DSEditImpl.cpp
// 
//
// Created: 09/13/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#endif

#define __DS_EDIT_INTERFACE_IMPLEMENTATION_
#include "DSEditImpl.h"

IMPLEMENT_DYNCREATE(DSEdit, AutoCompCEdit)

BEGIN_MESSAGE_MAP(DSEdit, AutoCompCEdit)
    ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#ifdef THIS_FILE
#undef THIS_FILE
#endif
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
static char THIS_FILE[] = __FILE__;
#endif

BOOL
DSEdit::PreTranslateMessage(MSG *pMsg)
{
    BOOL retVal = FALSE;
    ASSERT_VALID(this);

    switch(pMsg->message) {
    case WM_SYSCHAR:
	// A shortcut key.
	if (pMsg->lParam & 0x20000000) {
	    // Do we want this shortcut.
//	    if (IsThisAShortcutKeyWeWant(pMsg->wParam)) {
		::SendMessage(GetParent()->m_hWnd, pMsg->message,
			      pMsg->wParam, pMsg->lParam);
		retVal = TRUE;
//	    }
	}
	break;

	case WM_KEYDOWN:
		{
			if (pMsg->wParam == VK_UP && m_ACListBox && m_ACListBox->ContainsItems())
			{
				m_ACListBox->OnArrowKey(true);
				return TRUE;
			}
			else if(pMsg->wParam == VK_DOWN && m_ACListBox && m_ACListBox->ContainsItems())
			{
				m_ACListBox->OnArrowKey(false);
				return TRUE;
			}
			else if(pMsg->wParam == VK_RETURN && m_ACListBox && m_ACListBox->ContainsItems())
			{
				m_ACListBox->SetControlToCheck();
				KillACList();
				return FALSE;
			}
			else if(pMsg->wParam == VK_ESCAPE && m_ACListBox && m_ACListBox->ContainsItems())
			{
				KillACList();
				return TRUE;
			}
			break;
		}
	case WM_LBUTTONDOWN:
		{
			if (CWnd::GetFocus() != this)
			{
				SetFocus();
			}
			break;
		}
    default:
	break;
    }

    return(retVal);
}

BOOL
DSEdit::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg)
{
    if (nHitTest == HTCLIENT) {
	if (pvUserData) {
	    ::SetCursor((DSQueryParentQueryInProgress(pvUserData)) ?
			m_hArrowHour : m_hIBeam);
	}
	else {
	    ::SetCursor(m_hIBeam);
	}
	return(TRUE);
    }

    return(CWnd::OnSetCursor(pWnd, nHitTest, msg));
}

void DSEdit::GetACListWindowRect(LPRECT rct)
{
	GetWindowRect(rct);	
}

LRESULT
DSEdit::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_GETDLGCODE:
	return(DLGC_WANTALLKEYS);
    case WM_CHAR:
	if (ShouldPropagateMessageToParent(wMessage, wParam, lParam)) {
	   ::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
	   return(0);
	}
	else if (wParam == VK_RETURN) {
	    if (keyCB) {
		(*keyCB)(pvUserData, wParam);
		return(0);
	    }
	}
	break;
    default:
	break;
    }

    return(AutoCompCEdit::WindowProc(wMessage, wParam, lParam));
} 

void DSEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
 	AutoCompCEdit::OnChar(nChar, nRepCnt, nFlags);
	if (nChar == VK_DELETE || nChar == VK_BACK)
	{
		KillACList();
		return;
	}
	
	KillACList();
	DoQueryAutoComplete();
//	SetPoppedUpState(true);
}


void DSEdit::DoQueryAutoComplete(bool fullList)
{
	char buf[256];
	char Partial[256];
	Partial[0] = 0;
	unsigned int Len = 0;
	
	// Grab the line that the caret is in

	Len = GetLine(0, buf, sizeof(buf)-1);
	buf[Len] = 0;

	int SelStart, SelEnd;
	DWORD temp = GetSel();
	SelEnd = HIWORD(temp);
	SelStart = LOWORD(temp);

	char* Start = buf + SelStart;

	int a = SelStart;
	SelStart = (SelStart < SelEnd ? SelStart : SelEnd);
	SelEnd = (SelEnd > a ? SelEnd : a);

	// Find the beginning of the string if no text is selected
	if (SelStart == SelEnd)
	{
		if (buf[SelEnd] != 0)
			return;
		while (Start > buf && Start[-1] != ',')
			Start--;
	}
	else if (SelStart < SelEnd)
	{
		buf[SelStart] = 0;
		Start = buf;
	}

	// Skip by leading space
	while (*Start == ' ')
		Start++;

	int NumFound = 0;
	Len = strlen(Start);

	// Here we want to first check the history list for matches then we go on to the addressbook. 
	CStringList* Matches;

	if (fullList)
		Matches = m_DSQueryACList->FindListOfACStartingWith(_T(""));
	else if (*Start != 0)
		Matches= m_DSQueryACList->FindListOfACStartingWith(Start);
	else 
		Matches = NULL;

	int NumEntries = 0;

	if (Matches)
		NumEntries += Matches->GetCount();

	if (NumEntries > 0)
	{	
		m_ACListBox->DoACListBox();	
		if (Matches)
		{
			for (int i=0; i < Matches->GetCount(); i++)
			{
				CString temp = Matches->GetAt(Matches->FindIndex(i));

				char *tempChars = new char[temp.GetLength()+1];				
				strcpy(tempChars, temp);

				m_ACListBox->AddToEndOfACListBox(tempChars, 0);
				delete [] tempChars;
			}
		}

		if (fullList)
		{
			m_ACListBox->InitListDSDropDown();
			SetPoppedUpState(true);
		}
		else
			m_ACListBox->InitList();

		CRect rct;
		if (m_TheDropDownBtn)
		{
			m_TheDropDownBtn->GetClientRect(rct);
			m_TheDropDownBtn->InvalidateRect(rct);
		}
	}
	delete Matches;	
}

void DSEdit::InitListElements()
{
	if (!m_DSQueryACList){
		m_DSQueryACList = new AutoCompList(CRString(IDS_DSQUERY_HIST_LIST));
		if (m_DSQueryACList)
			m_DSQueryACList->ReadAutoCompleteList();
	}
	int width;
	CRect rct;
	GetClientRect(rct);
	width = rct.right-rct.left;
	if (!m_ACListBox)
	{
		m_ACListBox = new AutoCompleterListBox(this, width);
		m_ACListBox->SetHotRollover(false);
	}
    m_DoAutoComp = 0;
}

void DSEdit::DestroyListElements()
{
	if (m_DSQueryACList)
	{
		m_DSQueryACList->SaveToDisk();
		delete m_DSQueryACList;
		m_DSQueryACList = NULL;
	}
	if (m_ACListBox)
	{
		delete m_ACListBox;
		m_ACListBox = NULL;
	}
}

BOOL DSEdit::ACListPoppedUp()
{

	if (m_ACListBox)
		return m_ACListBox->ContainsMultItems();
	else return 0;
}
void DSEdit::KillACList()
{
	if (m_ACListBox)
		m_ACListBox->KillACListBox();

//	SetPoppedUpState(false);
	CRect rct;
	if (m_TheDropDownBtn)
	{
		m_TheDropDownBtn->GetClientRect(rct);
		m_TheDropDownBtn->InvalidateRect(rct);
	}
}


void DSEdit::OnSize( UINT nType, int cx, int cy )
{
	int width;
	CRect rect;
	GetClientRect(rect);
	width = rect.right-rect.left;
	m_ACListBox->SetMinWidth(width+20); // 20 is a fudge value representing the width of the dropdown button...
	AutoCompCEdit::OnSize(nType,cx,cy);
}

void DSEdit::OnKillFocus( CWnd* blah)
{
	if (m_ACListBox && m_ACListBox->ContainsItems())
		KillACList();

	AutoCompCEdit::OnKillFocus(blah);

//	m_PoppedUpState = false;
}
void DSEdit::OnSetFocus( CWnd* blah)
{
	AutoCompCEdit::OnSetFocus(blah);
}

bool DSEdit::PoppedUpState()
{
	return m_PoppedUpState;
}

void DSEdit::SetPoppedUpState(bool newState)
{
	m_PoppedUpState = newState;
}

void DSEdit::OnACListSelectionMade()
{
	CRect rct;
	if (m_TheDropDownBtn)
	{
		m_TheDropDownBtn->GetClientRect(rct);
		m_TheDropDownBtn->InvalidateRect(rct);
	}
	SetFocus();

	if (m_ACListBox)
	{
		m_ACListBox->SetControlToItemDS(-1, false);
	}

//	int Start,End;
//	GetSel(Start, End);
	DWORD foo;
	foo = 0xFFFF0000;
	SetSel(foo, true);
//	SetSel((Start > End) ? Start :End , (Start > End) ? Start :End );
	SetPoppedUpState(false);
	KillACList();
}

void DSEdit::OnACListDisplayed()
{
	SetPoppedUpState(true);

	if (m_TheDropDownBtn)
	{
		CRect rct;
		m_TheDropDownBtn->GetClientRect(rct);
		m_TheDropDownBtn->InvalidateRect(rct);
	}
	if (m_ACListBox)
	{
		m_ACListBox->SetListToControlDS(true);
	}
	else
		ASSERT(0);
	SetFocus();
}

IMPLEMENT_DYNCREATE(DSQueryBtn, CButton)

BEGIN_MESSAGE_MAP(DSQueryBtn, CButton)

END_MESSAGE_MAP()

DSQueryBtn::DSQueryBtn(){}

BOOL DSQueryBtn::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		SendMessage(WM_LBUTTONDOWN, pMsg->wParam, pMsg->lParam);
		return true;
	}
	return false;
}

LRESULT DSQueryBtn::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
 //   switch (wMessage) {
// case WM_LBUTTONDBLCLK:
//		{
//			return SendMessage(WM_LBUTTONDOWN, wParam, lParam);
//			
//		}
  //  default:
//	break;
  //  }

    return(CButton::WindowProc(wMessage, wParam, lParam));
} 

void DSQueryBtn::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
	BOOL isPressed = lpDIS->itemState & ODS_SELECTED != 0;
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rect(lpDIS->rcItem);
	
	COLORREF clr;
	clr = GetSysColor(COLOR_3DFACE);

	int ThreeDBorderwidth = GetSystemMetrics(SM_CXEDGE);
	int ThreeDBorderheight = GetSystemMetrics(SM_CYEDGE);

	pDC->DrawFrameControl( rect, DFC_BUTTON, DFCS_PUSHED );

	CRect insideRect(rect);
	insideRect.right -= ThreeDBorderwidth;
	insideRect.top += ThreeDBorderheight;
	insideRect.bottom -= ThreeDBorderheight;
	if (!isPressed)
		pDC->DrawFrameControl( insideRect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX);
	if (isPressed)
		pDC->DrawFrameControl( insideRect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX | DFCS_FLAT | DFCS_PUSHED);


	m_EditControl->GetClientRect(rect);
	rect.left = rect.right - ThreeDBorderwidth;
	m_EditControl->ValidateRect(rect);

}

void DSQueryBtn::SetAssocField(DSEdit* Editcon)
{
	m_EditControl = Editcon;
}












 
