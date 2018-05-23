// URLEDIT.H
//
// Subclassed CEdit class that knows how to deal with URLs
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

	static BOOL ParseMailtoArgs(const CString& strMailto,	//(i) raw mailto URL
								CString& strTo,				//(o) To: recipients
								CString& strCc,				//(o) Cc: recipients
								CString& strBcc, 			//(o) Bcc: recipients
								CString& strSubject,		//(o) subject
								CString& strBody);			//(o) body

	static CString DecodeMailtoURLChars(const char* pURL);

	static CString GetSelEditText(CRichEditCtrl *edtCtrl, UINT flagPos);
	static void SetSelEditText(CRichEditCtrl *edtCtrl, const char* text, UINT flagPos);
	static CString GetSelEditText(CEdit *edtCtrl, UINT flagPos);
	static void SetSelEditText(CEdit *edtCtrl, const char* text, UINT flagPos);

	BOOL m_bModalDlg;

private:

    CPoint   _clickPoint;
    bool     _gotCapture;
    bool     _mouseMoveOverURL;
    CString  _url;

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
