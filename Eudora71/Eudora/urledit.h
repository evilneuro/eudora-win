// URLEDIT.H
//
// Subclassed CEdit class that knows how to deal with URLs
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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

#ifndef URLEDIT_H
#define URLEDIT_H

#ifndef __ST__
#define __ST__

#define ST_BEGIN	0
#define ST_END		1
#define ST_CARET	2
#define ST_ALL		3

#define ST_FORMAT_NONE 0
#define ST_FORMAT_RICH 1
#define ST_FORMAT_HTML 2
#define ST_FORMAT_DONTKNOW 3

#endif

/////////////////////////////////////////////////////////////////////////////
// CURLEdit enhanced edit control

#define URL_COLOR RGB( 0, 0, 0xFE )

class CURLEdit : public CRichEditCtrl
{
	DECLARE_DYNCREATE(CURLEdit)
public:
// Construction
	CURLEdit();
	BOOL IsLiteralChar(TCHAR ch);

	void EnableHighlighting(BOOL bEnable)
		{ m_bEnableHighlighting = bEnable; }

	// subclassed construction
	BOOL SubclassEdit(UINT nID, CWnd* pParent);

	BOOL CURLEdit::OpenURL(const char* pURL);

	static CString GetURLAtLineColumn(CWnd* pEditCtrl, int LineNum, int ColNum, 
					BOOL bSelect=FALSE);
	static CString GetURLAtCaret(CWnd* pEditCtrl, BOOL bSelect=FALSE);
	static CString GetURLAtCursor(CWnd* pEditCtrl, BOOL bSelect=FALSE);
	static void DoURLHighlight(CWnd* pEditCtrl);

	static BOOL ParseMailtoArgs(const char* strMailto,		//(i) raw mailto URL
								CString& strTo,				//(o) To: recipients
								CString& strCc,				//(o) Cc: recipients
								CString& strBcc, 			//(o) Bcc: recipients
								CString& strXEudora,		//(o) X-Eudora fields
								CString& strSubject,		//(o) subject
								CString& strBody);			//(o) body

	static CString DecodeMailtoURLChars(const char* pURL);

	static CString GetSelEditText(CRichEditCtrl *edtCtrl, UINT flagPos);
	static void SetSelEditText(CRichEditCtrl *edtCtrl, const char* text, UINT flagPos);
	static CString GetSelEditText(CEdit *edtCtrl, UINT flagPos);
	static void SetSelEditText(CEdit *edtCtrl, const char* text, UINT flagPos);
	void CURLEdit::SetKillFocus(bool bKillFocus);

	BOOL m_bModalDlg;

private:

    CPoint   _clickPoint;
    bool     _gotCapture;
    bool     _mouseMoveOverURL;
    CString  _url;

	bool m_bKillFocus;

// Implementation
protected:
	//{{AFX_MSG(CURLEdit)
	// Rich edit control don't support default context menus.
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg long OnPaste(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT, CPoint);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg long OnCopy(WPARAM wParam, LPARAM lParam);
	afx_msg long OnCut(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	static CRichEditCtrl* DynamicDowncast(CWnd* pEditCtrl);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	//
	// Performance hack to allow turning off the expensive URL
	// highlighting action when we know that we're in the middle
	// of a bunch of edits to the edit buffer.
	//
	BOOL m_bEnableHighlighting;
};


// registered windows message
extern UINT uPossibleFocusMoveMsg;
#endif
