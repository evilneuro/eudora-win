// URLEDIT.CPP
//
// Subclassed CEdit control for handling URLs
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>
#include <limits.h>

#include "urledit.h"
#include "resource.h"
#include "rs.h"
#include "font.h"
#include "utils.h"
#include "eudora.h"
#include "guiutils.h"
#include "summary.h"
#include "3dformv.h"
#include "doc.h"		// for MSGDOC.H
#include "msgdoc.h"		// for COMPMSGD.H
#include "compmsgd.h"
#include "address.h"

BOOL LookupQuery(const char* Server, const char* Command, BOOL IsPh);


#include "helpcntx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


UINT uPossibleFocusMoveMsg = RegisterWindowMessage( "UrlEditPossibleFocusChange" );

/////////////////////////////////////////////////////////////////////////////
// CURLEdit

IMPLEMENT_DYNCREATE(CURLEdit, CRichEditCtrl)

CURLEdit::CURLEdit() :
    _gotCapture(false),
    _mouseMoveOverURL(false),
    m_bEnableHighlighting(TRUE)
{
	m_bModalDlg = FALSE;
}

BEGIN_MESSAGE_MAP(CURLEdit, CRichEditCtrl)
	//{{AFX_MSG_MAP(CURLEdit)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
        ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_PASTE, OnPaste)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


LRESULT CURLEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = CRichEditCtrl::WindowProc(message, wParam, lParam);

	switch (message)
	{
		case WM_SETFOCUS:
		{
			if ( ! m_bModalDlg )
			{
				// this fixes the rich edit control bug that leaves
				// the parent frame deactivated in the background 
				// when the edit control is activated by a mouse click.
			
				CFrameWnd*	pParentFrame;				
				CWnd*		pTop;

				// get the parent frame
				pParentFrame = GetParentFrame();

				// get the window at the top of the Z order
				pTop = pParentFrame->GetWindow( GW_HWNDFIRST );
			

				// see if the parent frame is the top window
				if ( pParentFrame && pTop && ( pTop->m_hWnd != pParentFrame->m_hWnd ) )
				{
					// nope -- move the frame to the top of the Z order
					pParentFrame->SetWindowPos( &wndTop, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );

					// move the frame to the top resets the
					// focus so post another set focus message
					SetFocus();
				}
			}
		}
		break;

	case WM_SETTEXT:
	case WM_CUT:
	case WM_PASTE:
	case EM_UNDO:
	case EM_PASTESPECIAL:
	case EM_REPLACESEL:
		DoURLHighlight(this);
		break;
	}

	return (Result);
}

////////////////////////////////////////////////////////////////////////
// SubclassEdit [public]
//
// Subclass the standard Windows edit control to have MFC route stuff
// through our message map before it gets to the standard edit control
// message handlers.  This allows us to override behaviors of the
// standard Windows edit control.
//
////////////////////////////////////////////////////////////////////////
BOOL CURLEdit::SubclassEdit(UINT nID, CWnd* pParent)
{
	// SubclassWindow requires an HWND so we call the Windows GetDlgItem
	// and avoid creating a tempory CWnd and then calling GetSafeHwnd
	HWND hWndEdit = ::GetDlgItem(pParent->m_hWnd, nID);
	if (hWndEdit == NULL)
		return FALSE;
	BOOL status = SubclassWindow(hWndEdit);

	//
	// Force Rich Edit controls to send the EN_CHANGE notifications to
	// their parents.  This allows CRichEditCtrl to transparently
	// "drop in" in place of CEdit controls in dialogs, specifically
	// the nicknames window.
	//
	if (status)
	{
		DWORD mask = GetEventMask();
		mask |= ENM_CHANGE  | ENM_SELCHANGE;
		SetEventMask(mask);
	}
	
	return status;
}

////////////////////////////////////////////////////////////////////////
// DynamicDowncast [protected, static]
//
// Downcast the passed in control to a CRichEditCtrl or a CEdit,
// depending om whether we're doing 32- or 16-bit.
//
////////////////////////////////////////////////////////////////////////
CRichEditCtrl* CURLEdit::DynamicDowncast(CWnd* pEditCtrl)
{
	CRichEditCtrl* p_editctrl = NULL;

	p_editctrl = DYNAMIC_DOWNCAST(CRichEditCtrl, pEditCtrl);
	if (NULL == p_editctrl)
		p_editctrl = (CRichEditCtrl*)DYNAMIC_DOWNCAST(CRichEditView, pEditCtrl);
	ASSERT_VALID(p_editctrl);

	return (p_editctrl);
}

////////////////////////////////////////////////////////////////////////
// DoURLHighlight [public, static]
//
// Look at the entire contents of the edit control, and highlight
// the URLs in it.
//
////////////////////////////////////////////////////////////////////////
void CURLEdit::DoURLHighlight(CWnd* pEditCtrl)
{
	// Prevent recursion
//	static BOOL PreventURLHighlightRecurse = FALSE;
//	if (PreventURLHighlightRecurse)
//		return;
//	PreventURLHighlightRecurse = TRUE;

	if (GetIniShort(IDS_INI_URL_HIGHLIGHT) == FALSE)
		return;

	CRichEditCtrl* p_editctrl = DynamicDowncast(pEditCtrl);

	if (NULL == p_editctrl)
		return;
	else if (pEditCtrl->IsKindOf(RUNTIME_CLASS(CURLEdit)))
	{
		//
		// Performance hack.  If the user has disabled
		// URL highlighting on this control, then bail now.
		//
		CURLEdit* p_urledit = (CURLEdit *) pEditCtrl;
		if (! p_urledit->m_bEnableHighlighting)
			return;
	}

	FINDTEXTEX ft;
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;
	ft.lpstrText = ":";

	while (p_editctrl->FindText(FR_MATCHCASE, &ft) != -1)
	{
		// Set up next find
		ft.chrg.cpMin = ft.chrgText.cpMin + 1;

		int LineNum = p_editctrl->LineFromChar(ft.chrgText.cpMin + 1);
		int LineInd = p_editctrl->LineIndex(LineNum);
		CString URL(GetURLAtLineColumn(p_editctrl, LineNum, ft.chrgText.cpMin + 1 - LineInd));
		if (URL.IsEmpty() == FALSE)
		{
			FINDTEXTEX ftURL;
			const char* Colon = strchr(URL, ':');
			ftURL.chrg.cpMin = ft.chrgText.cpMin - (Colon - URL);
			ftURL.chrg.cpMax = -1;
			ftURL.lpstrText = (char*)(const char*)URL;
			if (p_editctrl->FindText(FR_MATCHCASE, &ftURL) == -1)
			{
				// Uh oh, we didn't find it!  How could that be!
				ASSERT(FALSE);
			}
			else
			{
				// Start next search after the end of this URL
				ft.chrg.cpMin = ftURL.chrgText.cpMax + 1;

				// Change the style to blue underline
				BOOL WasUnchanged = !p_editctrl->GetModify();
				CHARRANGE OldSel;
				p_editctrl->GetSel(OldSel);
				p_editctrl->HideSelection(TRUE, FALSE);

				p_editctrl->SetSel(ftURL.chrgText);
				CHARFORMAT cf;
				memset(&cf, 0, sizeof(cf));
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_COLOR | CFM_UNDERLINE;
				cf.dwEffects = CFE_UNDERLINE;
				cf.crTextColor = URL_COLOR;
				VERIFY(p_editctrl->SetSelectionCharFormat(cf));

				// We don't want to change style to be blue/underline for text around
				// this URL, so change the style right before and right after the URL
				cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
				cf.dwEffects = CFE_AUTOCOLOR;
				p_editctrl->SetSel(0,0);
				VERIFY(p_editctrl->SetSelectionCharFormat(cf));
				p_editctrl->SetSel(ftURL.chrgText.cpMin, ftURL.chrgText.cpMin);
				VERIFY(p_editctrl->SetSelectionCharFormat(cf));
//				p_editctrl->SetSel(ftURL.chrgText.cpMax + 1, ftURL.chrgText.cpMax + 1);
//				VERIFY(p_editctrl->SetSelectionCharFormat(cf));

				// The change in style will cause the modify flag to be set, so if this text was
				// unmodified before, then set it back to being unmodified
				if (WasUnchanged)
					p_editctrl->SetModify(FALSE);

				p_editctrl->SetSel(OldSel);
				p_editctrl->HideSelection(FALSE, FALSE);
			}
		}
	}

//	PreventURLHighlightRecurse = FALSE;
}

////////////////////////////////////////////////////////////////////////
// GetURLAtLineColumn [public, static]
//
// Retrieve the URL string, if any from the given line and column
// coordinate of the given edit control.
//
// From the current passed position check for the colon. If it is not on
// the passed in line go to the previous line until it is found or one of
// the delimitors (<,(,",',),>,\n) is found.  If no colon was found it is
// not a URL.  Otherwise we scan backward from the passed position to find
// one of the delimitors or a space.  As we are scanning we check for a 
// colon. If we find a space we check forward for the colon if we do not find
// it and find a space we set a flag that states the current position is
// within a word seperated by space and resume the backward search. If we
// find the colon the first delimitor or space to the left of the colon 
// indicates wether it is space seperated or delimitor seperated. If it 
// is space seperated and the flag states position on a word with space
// seperation then the position does not contain a URL otherwise it is a
// delimited URL and regardless of the space it is part of the URL.
//						modified 11/19/96 by ms. 
////////////////////////////////////////////////////////////////////////
CString CURLEdit::GetURLAtLineColumn(CWnd* pEditCtrl, int LineNum, int ColNum, 
									 BOOL bSelect /*=FALSE*/)
{
	CRichEditCtrl* p_editctrl = DynamicDowncast(pEditCtrl);

	if (NULL == p_editctrl)
		return "";

	int Len;
	const char* pURL = NULL;
	char buf[_MAX_PATH + 64];

	// Gets the current line
	if ((Len = p_editctrl->GetLine(LineNum, buf, sizeof(buf) - 1)) <= 0 || ColNum > Len)
		return ("");

	if (ColNum < 0)
		ColNum = 0;

	BOOL bTerminator=FALSE;
	buf[Len]=0;
	char buf2[_MAX_PATH + 64];
	int Len2=ColNum;
	int totalLen =Len;
	int sLineNum = LineNum;
	char buf3[_MAX_PATH + 64]={0};
	int Len3;
	char* pTemp=NULL;
	int rc=0;
	const char* Colon;
	CString sURL(buf);
	strcpy( buf3, buf);
	char searchString[]={'\n','<', '>', '"', '(', ')','\'','\0'};

	// If the current line does not have a colon search the line above it
	while ((Colon = strchr(buf3, ':')) == NULL)
	{
		LineNum--;
		if (LineNum <= 0)
			return CString("");

		if ((Len3= p_editctrl->GetLine(LineNum, buf3, sizeof(buf3))) <= 0)
			return CString("");

		buf3[Len3]=0;
		pTemp=buf3 + Len3;
		while ((rc= strcmp(pTemp, buf3)) != 0 && pTemp[0] != ':' &&
				pTemp[0] != '<' && pTemp[0] != '"' && pTemp[0] != '\'' && pTemp[0] != '(' &&
				pTemp[0] != '\n' && pTemp[0] != ')' && pTemp[0] != '>')
		{
			pTemp--;
		}

		sURL = CString(buf3) + sURL;
		totalLen += Len3;
		Len2 += Len3;
		if (pTemp[0] == ':')
			break;
		// if pTemp has one of the searchString then this is not a URL
		if (strchr(searchString, pTemp[0])!= NULL || isspace(pTemp[0]) || rc != 0)
			return CString("");
	}

	if (sURL.GetLength() < sizeof(buf))
    	strcpy(buf, (const char *)sURL);
    else
    	return CString("");

	pURL = buf + Len2;

	const char* pTempURL = pURL;	
	BOOL bPastColon=FALSE;
	BOOL bSpaceSeperated=FALSE;

	while (pURL != buf)
	{
		if (pURL[-1] == ':')
		{
			bPastColon = TRUE;
			pTempURL = pURL;
		}

		if (pURL[-1] == '<' || pURL[-1] == '"' || pURL[-1] == '\'' || pURL[-1] == '(')
		{
			bTerminator = TRUE;
			break;
		}
		else if (pURL[-1] == '\n' || pURL[-1] == ')' || pURL[-1] == '>')
		{
			bTerminator = FALSE;
			break;
		}
		else if (isspace(pURL[-1]))
		{
			// it reached a space on the LHS but has not come across a ':' then
			// check if the colon was after the column position
			if (!bPastColon)
			{
				while (pTempURL[0] != '<' && pTempURL[0] != '"' && pTempURL[0] != '\'' &&
						!isspace(pTempURL[0]) && pTempURL[0] != '(' && 
						pTempURL[0] != ')' && pTempURL[0] != '>' && pTempURL[0] != '\n')
				{
					if (pTempURL[0] == ':')
					{
						bPastColon = TRUE;
						break;
					}
					pTempURL++;
				}
				//there is space on both sides of the word(s) check a flag and keep
				//looking for the colon in the LHS of the string 'cause if this string
				//is terminated this word(s) is (are) still part of the URL.
				if (isspace(pTempURL[0]) || pTempURL[0] == '\n')
					bSpaceSeperated = TRUE;
			}

			// we already found the colon and this space is left of the colon so it is
			// not terminated by one of the valid characters other than space.
			if (bPastColon)
			{
				bTerminator = FALSE;
				break;
			}
		}

		pURL--;
	}

	// if the position does not have terminators other than space and the Column is 
	// at a word without ':' then there is no URL to return;
	if (bSpaceSeperated && !bTerminator)
		return CString("");

	char* End = (char*)pURL;
	strcpy(buf2, pURL);
	Len = strlen(buf2);
	if (Len > totalLen)
		Len = totalLen;

	// loop for finding the end of the URL
	while (buf2[Len] != '\n' && buf2[Len-1] != '\n' && (strlen(buf) < sizeof(buf)))
	{
		sLineNum++;
		End = buf + totalLen - Len;
		BOOL bterminatorFound = TRUE;
		while (*End )
		{
			if (*End == '>' || *End == '<' || *End == '"' || *End == ')' || *End == '\'')
				break;
			if (!bTerminator && isspace(*End))
				break;
			End++;
			if (*End == 0)
				bterminatorFound = FALSE;
		}
		if (bterminatorFound && *End)
			break;
		Len2 = p_editctrl->GetLine(sLineNum, buf2, sizeof(buf2));
		if (Len2 <= 0)
			break;
		buf2[Len2]=0;
		buf[totalLen] = 0;
		if ((strlen(buf)+Len2) >= sizeof(buf))
			break;
		strcat(buf, buf2);
		Len = Len2;
		totalLen += Len;
	}
	buf[totalLen] = 0;

	while (*End)
	{
		if (*End == '>' || *End == '<' || *End == '"' || *End == ')' || *End == '\'')
			break;
		if (!bTerminator && isspace(*End))
			break;
		End++;
	}

	// The following if statement is to strip a period at the end of the URL if a space is the terminator
	// of the URL.
	if (isspace(*End) && (End[-1] == '.' || End[-1] == ',' || End[-1] == ';'
		|| End[-1] == '!' || End[-1] == '?'))
		End--;

	*End = 0;

	if (*pURL == '<')
		pURL++;
	
	// Strings starting with "<URL:" are defintely URLs
	if (!strnicmp(pURL, "URL:", 4))
		pURL += 4;
	else
	{
		const char* u = pURL;
		// Schemes only contain alphas, digits, '+', '-', and '.'
		while (isalnum(*u) || *u == '+' || *u == '-' || *u == '.')
			u++;
		if (*u != ':' || u == pURL || u[1] == 0)
			return CString("");
		else
		{
			CRString SchemeList(IDS_URL_SCHEMES);
			const char* Scheme = SchemeList;
			const char* EndScheme;
			while (Scheme && *Scheme)
			{
				EndScheme = strchr(Scheme, ':');
				if (EndScheme)
				{
					if (!strnicmp(pURL, Scheme, EndScheme - Scheme + 1))
						break;
					Scheme = EndScheme + 1;
				}
				else
				{
					if (!strnicmp(pURL, Scheme, strlen(Scheme)))
						break;
					return CString("");
				}
			}
			if (!Scheme || !*Scheme)
				return CString("");
		}
	}
	if (bSelect)
	{
		long start=0;
		long end=0;

		start = p_editctrl->LineIndex(LineNum);
		start = start + (pURL - buf);
		end = start + strlen(pURL);
		p_editctrl->SetSel(start, end);
	}

	return (CString(pURL));
}



////////////////////////////////////////////////////////////////////////
// GetURLAtCaret [public, static, overloaded]
//
// Retrieve the URL string, if any from the current mouse cursor
// location.
//
////////////////////////////////////////////////////////////////////////
CString CURLEdit::GetURLAtCaret(CWnd* pEditCtrl, BOOL bSelect /*=FALSE*/)
{
	CRichEditCtrl* p_editctrl = DynamicDowncast(pEditCtrl);

	if (NULL == p_editctrl)
		return "";

	long Start, End;

	p_editctrl->GetSel(Start, End);

	return CURLEdit::GetURLAtLineColumn(pEditCtrl, p_editctrl->LineFromChar(-1), 
			Start - p_editctrl->LineIndex(), bSelect);
}


////////////////////////////////////////////////////////////////////////
// GetURLAtCursor [public, static]
//
// Retrieve the URL string, if any from the current mouse cursor
// location.
//
////////////////////////////////////////////////////////////////////////
CString CURLEdit::GetURLAtCursor(CWnd* pEditCtrl, BOOL bSelect /*=FALSE*/)
{
	CRichEditCtrl* p_editctrl = DynamicDowncast(pEditCtrl);

	if (NULL == p_editctrl)
		return "";

	CPoint point;
	::GetCursorPos(&point);
	CRect rect;
	p_editctrl->GetWindowRect(&rect);

	CFont* p_font = NULL;
	CFont win32_font;
	if (IsVersion4())
	{
		//
		// Win 95 has the cool EM_CHARFROMPOS message.  BTW, the Win32
		// SDK documentation for EM_CHARFROMPOS under MSVC 4.x is wrong.  
		// See Knowledge Base article Q137805 for the corrected info.
		//
		p_editctrl->ScreenToClient(&point);
		DWORD idx = p_editctrl->SendMessage(EM_CHARFROMPOS, 0, LPARAM(&point));
		if (idx != -1)
		{
			long Line = p_editctrl->LineFromChar(idx);
			int Col = idx - p_editctrl->LineIndex(Line);
			ASSERT(Col >= 0);
			return CURLEdit::GetURLAtLineColumn(pEditCtrl, Line, Col, bSelect);
		}
	}
	else
	{
		CHARFORMAT cf;
		cf.dwMask = CFM_SIZE;
		DWORD mask = p_editctrl->GetDefaultCharFormat(cf);
		ASSERT(mask & CFM_SIZE);

		// Create a font object so that we can convert point size to TWIPS to pixels.
		if (win32_font.CreatePointFont(cf.yHeight / 2, cf.szFaceName))
			p_font = &win32_font;
	}

	//
	// Get text metrics for the current edit control font rendered 
	// on the screen.  We need this to figure out the average
	// character width and the line height.
	//
	if (p_font)
	{
		CDC dc;
		if (dc.CreateIC("DISPLAY", NULL, NULL, NULL))
		{
			dc.SelectObject(p_font);
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);

			if ((tm.tmHeight + tm.tmExternalLeading) && tm.tmAveCharWidth)			// avoid divide by zero
			{
				int LineNum = (point.y - rect.top - 5) / (tm.tmHeight + tm.tmExternalLeading) + p_editctrl->GetFirstVisibleLine();
				return CURLEdit::GetURLAtLineColumn(pEditCtrl, LineNum, (point.x - rect.left - 3) / tm.tmAveCharWidth);
			}
		}
	}

	return ("");
}


////////////////////////////////////////////////////////////////////////
// OnSetCursor [protected]
//
// Change to a "launch cursor" if we're over a URL.
//
////////////////////////////////////////////////////////////////////////
BOOL CURLEdit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT && (pWnd->GetSafeHwnd() == m_hWnd))
	{
		if ((_url.IsEmpty() == FALSE) && _mouseMoveOverURL) {
		    HCURSOR hC = ::LoadCursor(0, IDC_NO);
		    if (hC != NULL) {
			HCURSOR hC2 = ::SetCursor(hC);
			return TRUE;
		    }
		    else {
			// If could not load appropropriate cursor,
			// default to CRichEditCtrl::OnSetCursor(),
			// but first assert.
			assert(0);
		    }
		}
		else if (! CURLEdit::GetURLAtCursor(this).IsEmpty())
		{
		    QCSetCursor(IDC_APP_LAUNCH);
		    return TRUE;
		}

	}

	return (CRichEditCtrl::OnSetCursor(pWnd, nHitTest, message));
}

////////////////////////////////////////////////////////////////////////
// OnLButtonDown [protected]
//
////////////////////////////////////////////////////////////////////////
void CURLEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
    _gotCapture = false;
    _url.Empty();
    _mouseMoveOverURL = false;
    // Need to get what's at the caret before the selection is changed
    _url = GetURLAtCursor(this, FALSE);

    if (_url.IsEmpty() == FALSE) {
	_clickPoint = point;
	if (GetCapture() != this) {
	    SetCapture();
	    _gotCapture = true;
	}
    }
    else {
	CRichEditCtrl::OnLButtonDown(nFlags, point);
    }
}

////////////////////////////////////////////////////////////////////////
// OnLButtonUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CURLEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (_url.IsEmpty() == FALSE) {
	if (point == _clickPoint) {
	    OpenURL(_url);
	}
	if ((_gotCapture == true) && (GetCapture() == this)) {
	    ReleaseCapture();
	}
    }
    else {
	CRichEditCtrl::OnLButtonUp(nFlags, point);
    }
    _url.Empty();
    _mouseMoveOverURL = false;
}

////////////////////////////////////////////////////////////////////////
// OnMouseMove [protected]
//
////////////////////////////////////////////////////////////////////////
void CURLEdit::OnMouseMove(UINT nFlags, CPoint point)
{
    if (_url.IsEmpty() == FALSE) {
	if ((_gotCapture == true) && (GetCapture() == this)) {
	    CString aURL = GetURLAtCursor(this, FALSE);
	    if (aURL.IsEmpty() == FALSE) {
		_mouseMoveOverURL = true;
	    }
	    ::SendMessage(m_hWnd, WM_SETCURSOR, (WPARAM) m_hWnd,
			  MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
	}	
    }
    else {
	CRichEditCtrl::OnMouseMove(nFlags, point);
    }
}



////////////////////////////////////////////////////////////////////////
// OpenURL [public, static]
//
////////////////////////////////////////////////////////////////////////
BOOL CURLEdit::OpenURL(const char* pURL)
{
	if (!pURL || !*pURL)
		return (FALSE);

	return LaunchURL( pURL );
}

////////////////////////////////////////////////////////////////////////
// SetSelText [public, static]     CRichEditCtrl Version
//
////////////////////////////////////////////////////////////////////////
void CURLEdit::SetSelEditText(CRichEditCtrl *edtCtrl, const char* text, UINT flagPos)
{
	//
	// Adjust the selection so that the text is set
	// in the correct place.
	//
	switch ( flagPos ) {
	case ST_BEGIN :
		edtCtrl->SetSel(0,0);
		break;
	case ST_END :
		{
			long lLen = edtCtrl->GetTextLength();
			edtCtrl->SetSel(lLen, lLen);
		}
		break;
	case ST_ALL :
		edtCtrl->SetSel(0,-1);
		//edtCtrl->ReplaceSel("");
		break;
	case ST_CARET:
		// Leave the selection alone
		break;
	}

	edtCtrl->ReplaceSel(text);
}  

////////////////////////////////////////////////////////////////////////
// GetSelText [public, static]    CRichEditCtrl Version
//
////////////////////////////////////////////////////////////////////////
CString CURLEdit::GetSelEditText(CRichEditCtrl *edtCtrl, UINT flagPos)
{
	CString		selText;
	int			start = 0;
	int			end = 0;
	CHARRANGE	cr;
	LPSTR		pString;

	//
	// Set the selection in the control so that the proper text
	// is extracted. ST_CARET means get the current selection.  
	// Anything else is assume to mean get all the text in the
	// control.
	//
	if  ( flagPos != ST_CARET )
	{
		edtCtrl->SetSel(0, edtCtrl->GetTextLength());
 	}

	edtCtrl->GetSel( cr );
	pString = selText.GetBufferSetLength( ( cr.cpMax - cr.cpMin + 1 ) * 2 );	
	pString[0] = NULL;
	::SendMessage( edtCtrl->m_hWnd, EM_GETSELTEXT, 0, (LPARAM) pString );
	selText.ReleaseBuffer();
	
	return selText;
}


////////////////////////////////////////////////////////////////////////
// SetSelText [public, static] ... CEdit Version
//
////////////////////////////////////////////////////////////////////////
void CURLEdit::SetSelEditText(CEdit *edtCtrl, const char* text, UINT flagPos)
{
	//
	// Adjust the selection so that the text is set
	// in the correct place.
	//
	switch ( flagPos ) {
	case ST_BEGIN :
		edtCtrl->SetSel(0,0);
		break;
	case ST_END :
		{
			int start, end;
			edtCtrl->SetSel(0, -1, TRUE);
			edtCtrl->GetSel(start, end);
			edtCtrl->SetSel(end, end, TRUE);
		}
		break;
	case ST_ALL :
		edtCtrl->SetSel(0,-1);
		//edtCtrl->ReplaceSel("");
		break;
	case ST_CARET:
		// Leave the selection alone
		break;
	}
	edtCtrl->SendMessage(EM_REPLACESEL, 1, (LPARAM)text);
}  

////////////////////////////////////////////////////////////////////////
// GetSelText [public, static]
//
////////////////////////////////////////////////////////////////////////
CString CURLEdit::GetSelEditText(CEdit *edtCtrl, UINT flagPos)
{
	CString selText;
	int start = 0, end = 0;
	//
	// Set the selection in the control so that the proper text
	// is extracted. ST_CARET means get the current selection.  
	// Anything else is assume to mean get all the text in the
	// control.
	//
	switch  ( flagPos ) 
	{
		case ST_CARET :
			edtCtrl->GetSel(start, end);
			selText.Empty();
			if (start != end)
			{
				int startLine = edtCtrl->LineFromChar(start); 
				int endLine = edtCtrl->LineFromChar(end); 
				int lineIndex = edtCtrl->LineIndex(startLine); 
				int stIndex = start - edtCtrl->LineIndex(startLine); 
						
				while (startLine <= endLine)
				{
					char *line = new char[edtCtrl->LineLength(lineIndex) + 1];
					memset(line, 0,  edtCtrl->LineLength(lineIndex) + 1);
					edtCtrl->GetLine(startLine++, line, edtCtrl->LineLength(lineIndex));
					if (line)
					{ 
						if (!selText.IsEmpty())
							selText += "\r\n";
						selText += line;
						delete []line;
					}
					lineIndex = edtCtrl->LineIndex(startLine); 
				}
						
				// Strip off Leading & Trailing stuff 
				selText = selText.Right(selText.GetLength() - stIndex);
				selText = selText.Left(end - start);  
			} 
			break;
		default:
			edtCtrl->SetSel(0, -1);  
			edtCtrl->GetWindowText(selText); 
			
	 		break;
 	}

	return selText;
}


////////////////////////////////////////////////////////////////////////
// DecodeMailtoURLChars [public, static]
//
// Characters that aren't URL delimiters or plain text must be encoded
// as a "%XX" triplet where XX is a hex byte code.
//
// This method walks the given string and decodes the "%XX" triplets
// back into the corresponding byte code.  See RFC1738 for the bloody
// details.
//
// If for some reason we're not given a valid "%XX" triplet, we just
// leave the bogus triplet alone and don't decode it.
//
// It also does parameter substitution that we came up with:
//		$Name$		Real name
//		$Email$		Return address (without real name)
//		$User$		Only username portion of email address
// 
// The return value is the decoded string. 
////////////////////////////////////////////////////////////////////////
CString CURLEdit::DecodeMailtoURLChars(const char* pURL)	//(io) do "in place" replacement
{
	CString Result;

	if (pURL && *pURL)
	{
		const char* p_src = pURL;

		// Optimization so that we're not doing lots of small buffer allocations
		Result.GetBuffer(strlen(pURL));

		while (*p_src)
		{
			if ('%' == *p_src)
			{
				//
				// Look ahead to the next two characters.  If they are both
				// non-NULL (didn't hit end of string) and they are both
				// valid hex digits, then we convert the encoded triplet
				// into a single hex byte code and copy that back to the
				// target string.
				//
				if (*(p_src + 1))
				{
					char hexdigits[3];
					hexdigits[0] = *(p_src + 1);
					if (*(p_src + 2))
					{
						hexdigits[1] = *(p_src + 2);

						if (isxdigit(hexdigits[0]) && isxdigit(hexdigits[1]))
						{
							hexdigits[2] = 0;
							int hexvalue = -1;
							VERIFY(sscanf(hexdigits, "%x", &hexvalue) == 1);
							ASSERT(hexvalue >= 0 && hexvalue <= 0xFF);

							// Heuristic to add CR if we get a LF by itself
							if (hexvalue == '\n' && (Result.IsEmpty() || Result[Result.GetLength() - 1] != '\r'))
								Result += '\r';

							//
							// Found and decoded a valid hex triplet,
							// so replace the triplet with the single-byte
							// decoded value.
							//
							Result += char(hexvalue);
							p_src += 3;
							continue;
						}
					}
				}
			}
			else if ('$' == *p_src)
			{
				const char Name[] = "$Name$";
				const char Email[]= "$Email$";
				const char User[] = "$User$";
				int NameLen = strlen(Name);
				int EmailLen= strlen(Email);
				int UserLen = strlen(User);

				if (strnicmp(p_src, Name, NameLen) == 0)
				{
					Result += GetIniString(IDS_INI_REAL_NAME);
					p_src += NameLen;
					continue;
				}
				if (strnicmp(p_src, Email, EmailLen) == 0)
				{
					char EmailAddress[256];
					strcpy(EmailAddress, GetReturnAddress());
					StripAddress(EmailAddress);
					Result += EmailAddress;
					p_src += EmailLen;
					continue;
				}
				if (strnicmp(p_src, User, UserLen) == 0)
				{
					Result += GetIniString(IDS_INI_LOGIN_NAME);
					p_src += UserLen;
					continue;
				}
			}

			//
			// If we get this far, we didn't find a valid hex triplet.
			//
			char c = *p_src++;

			// Heuristic to add CR if we get a LF by itself
			if (c == '\n' && (Result.IsEmpty() || Result[Result.GetLength() - 1] != '\r'))
				Result += '\r';

			Result += c;
		}
	}

	return Result;
}


////////////////////////////////////////////////////////////////////////
// ParseMailtoArgs [public, static]
//
// Parse the given mailto parameter list into two pieces: a recipient
// part and a subject part, returning TRUE if successful.  If the
// recipient part cannot be empty -- if the recipient part is empty,
// then return FALSE.
//
// The typical mailto URL supported by this method looks like:
//
//     mailto:fred@bogus.com
//
//             or
//
//     mailto:wilma@bedrock.com?Subject=Whatever&cc=fred&bcc=barney
//
// This routine assumes that question mark characters separates the
// basic mailto URL from a Netscape-style parameter list.
//
// We have seen that Netscape can generate funky mailto strings like:
//
//     mailto:?junk?morejunk?andstillmorejunk
//
// when responding to stuff like newsgroup postings.  Since there is
// no recipient list in front of the initial question mark, we return
// FALSE to indicate an empty recipient string.
//
// If we don't recognize the subject part of the mailto string, we
// just ignore it and return an empty subject without error.
// 
////////////////////////////////////////////////////////////////////////
BOOL CURLEdit::ParseMailtoArgs
(
	const CString& strMailto,	//(i) raw mailto URL
	CString& strTo,				//(o) To: recipients
	CString& strCc,				//(o) Cc: recipients
	CString& strBcc, 			//(o) Bcc: recipients
	CString& strSubject,		//(o) subject
	CString& strBody			//(o) body
)
{
	ASSERT(strTo.IsEmpty());
	strTo.Empty();
	ASSERT(strCc.IsEmpty());
	strCc.Empty();
	ASSERT(strBcc.IsEmpty());
	strBcc.Empty();
	ASSERT(strSubject.IsEmpty());
	strSubject.Empty();
	ASSERT(strBody.IsEmpty());
	strBody.Empty();

	//
	// Make working copy of caller's string so that we can muck with it.
	//
	CString mailto(strMailto);

	//
	// Strip off leading "mailto:" token, if any.
	//
	if (strnicmp(mailto, "mailto:", 7) == 0)
		mailto = mailto.Right(mailto.GetLength() - 7);

	int idx = mailto.Find('?');

	if (-1 == idx)
	{
		//
		// Easy case.  No parameter list to worry about.
		//
		strTo = mailto;

		// Convert "%XX" hex encoding to real byte codes.
		strTo = DecodeMailtoURLChars(strTo);
	}
	else
	{
		//
		// Hard case.  Treat everything to the left of the param marker
		// as recipient stuff, then parse the stuff to the right of
		// param marker in search of to/cc/bcc/subject/body parameters
		// of the form:
		//
		//     subject=something
		//     to=recipient
		//     cc=recipient
		//     bcc=recipient
		//     body=something
		//
		// Multiple parameters are supposed to be separated by 
		// ampersands (&) as in
		//
		//     subject=fred was here&cc="Flintstone, Wilma" <wilma@bedrock.com>
		//
		// But since we're nice programmers, we'll also accept more
		// question marks (?) as parameter separators for those people
		// who don't read URL specs.  :-)
		//
		strTo = mailto.Left(idx);
		mailto = mailto.Right(mailto.GetLength() - idx - 1);

		// Convert "%XX" hex encoding to real byte codes.
		strTo = DecodeMailtoURLChars(strTo);

		while (! mailto.IsEmpty())
		{
			CString param;
			if ((idx = mailto.FindOneOf("?&")) != -1)
			{
				//
				// Found a parameter separator, so fetch the next
				// parameter string.
				//
				param = mailto.Left(idx);
				mailto = mailto.Right(mailto.GetLength() - idx - 1);
			}
			else
			{
				// last parameter (or one and only parameter)
				param = mailto;
				mailto.Empty();
			}

			ASSERT(! param.IsEmpty());

			//
			// Break a parameter string of the form:
			//
			//      name=value
			//
			// into a "name" part and a "value" part, then save any
			// recognized parameters as To/Cc/Bcc/Subject addressing
			// info.
			//
			if ((idx = param.Find('=')) != -1)
			{
				CString name(param.Left(idx));
				CString value(param.Right(param.GetLength() - idx - 1));

				// Convert "%XX" hex encoding to real byte codes.
				value = DecodeMailtoURLChars(value);

				if (stricmp(name, "subject") == 0)
					strSubject += value;
				else if (stricmp(name, "to") == 0)
				{
					if (! strTo.IsEmpty())
						strTo += ", ";
					strTo += value;
				}
				else if (stricmp(name, "cc") == 0)
				{
					if (! strCc.IsEmpty())
						strCc += ", ";
					strCc += value;
				}
				else if (stricmp(name, "bcc") == 0)
				{
					if (! strBcc.IsEmpty())
						strBcc += ", ";
					strBcc += value;
				}
				else if (stricmp(name, "body") == 0)
					strBody += value;

				//
				// Unrecognized parameters are silently ignored.
				//
			}
		}
	}

	if (strTo.IsEmpty() &&
		strCc.IsEmpty() && 
		strBcc.IsEmpty() && 
		strSubject.IsEmpty() &&
		strBody.IsEmpty())
	{
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Called when the user releases the right button in the rich edit control.
////////////////////////////////////////////////////////////////////////
void CURLEdit::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	if (GetCapture() == this)
		ReleaseCapture();

	// Get the menu that contains the CURLEdit context popup
	CMenu popup_menus;
	HMENU hMenu = QCLoadMenu(IDR_CURLEDIT_POPUPS);
	if ( ! hMenu || ! popup_menus.Attach( hMenu ) )
	{
		ASSERT(0);
		return;
	}
	CMenu* p_temp_popup_menu = popup_menus.GetSubMenu(0);
	if (p_temp_popup_menu != NULL)
	{
		//
		// Since the popup menu we get from GetSubMenu() is a pointer
		// to a temporary object, let's make a local copy of the
		// object so that we have explicit control over its lifetime.
		//
		CMenu temp_popup_menu;
		temp_popup_menu.Attach(p_temp_popup_menu->GetSafeHmenu());
		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
		CContextMenu(&temp_popup_menu, ptScreen.x, ptScreen.y);
		VERIFY(temp_popup_menu.Detach());
	}
	else
	{
		ASSERT(0);
	}

	popup_menus.DestroyMenu();
}


////////////////////////////////////////////////////////////////////////
// OnRButtonUp [protected]
//
// Manually generate a WM_CONTEXTMENU message for Rich Edit controls, since
// it doesn't generate its own message.
////////////////////////////////////////////////////////////////////////
void CURLEdit::OnRButtonUp(UINT nFlags, CPoint point)
{
	Default();

	ClientToScreen(&point);
	OnContextMenu(this, point);
}


////////////////////////////////////////////////////////////////////////
// OnPaste [protected]
//
// Convert Paste commands to "Paste Special As CF_TEXT" commands.
//
////////////////////////////////////////////////////////////////////////
long CURLEdit::OnPaste(WPARAM wParam, LPARAM lParam)
{
	PasteSpecial(CF_TEXT);
	return 0;
}


