#ifndef __NICKNAMESWAZOOWND_H__
#define __NICKNAMESWAZOOWND_H__

// NicknamesWazooWnd.h : header file
//
// CNicknamesWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "SplitHlp.h"
#include "urledit.h"	// for NICKPAGE.H

/////////////////////////////////////////////////////////////////////////////
// CNicknamesWazooWnd window
#define NICKPAGE_EDIT_NAME			_T("name")
#define NICKPAGE_EDIT_FIRSTNAME		_T("first")
#define NICKPAGE_EDIT_LASTNAME		_T("last")
#define NICKPAGE_EDIT_ADDRESSES		_T("addresses")
#define NICKPAGE_EDIT_ADDRESS		_T("address")
#define NICKPAGE_EDIT_CITY			_T("city")
#define NICKPAGE_EDIT_STATE			_T("state")
#define NICKPAGE_EDIT_COUNTRY		_T("country")
#define NICKPAGE_EDIT_ZIP			_T("zip")
#define NICKPAGE_EDIT_PHONE			_T("phone")
#define NICKPAGE_EDIT_FAX			_T("fax")
#define NICKPAGE_EDIT_MOBILE		_T("mobile")
#define NICKPAGE_EDIT_WEB			_T("web")
#define NICKPAGE_EDIT_COMPANY		_T("company")
#define NICKPAGE_EDIT_TITLE			_T("title")
#define NICKPAGE_EDIT_ADDRESS2		_T("address2")
#define NICKPAGE_EDIT_CITY2			_T("city2")
#define NICKPAGE_EDIT_STATE2		_T("state2")
#define NICKPAGE_EDIT_COUNTRY2		_T("country2")
#define NICKPAGE_EDIT_ZIP2			_T("zip2")
#define NICKPAGE_EDIT_PHONE2		_T("phone2")
#define NICKPAGE_EDIT_FAX2			_T("fax2")
#define NICKPAGE_EDIT_MOBILE2		_T("mobile2")
#define NICKPAGE_EDIT_WEB2			_T("web2")
#define NICKPAGE_EDIT_O_EMAIL		_T("otheremail")
#define NICKPAGE_EDIT_O_PHONE		_T("otherphone")
#define NICKPAGE_EDIT_O_WEB			_T("otherweb")

class CNicknamesWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CNicknamesWazooWnd)

public:
	CNicknamesWazooWnd();
	virtual ~CNicknamesWazooWnd();

	// Override of base class implementation
	virtual void OnDeactivateWazoo();

	void ToggleRHSDisplay();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnActivateWazoo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNicknamesWazooWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd,UINT nHitTest,UINT message);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL CreateViews();

	CSplitterHelpWnd	m_wndSplitter;

	//
	// Cached width of the RHS pane.
	//
	int m_nRHSWidth;
private:
	CURLEdit * m_pWndPreviousFind;
	void SelectField(CURLEdit *pEdit, const CString & szTerm);
};


#endif //__NICKNAMESWAZOOWND_H__
