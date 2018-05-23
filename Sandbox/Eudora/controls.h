// controls.h : header file
//

#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CLabelButton button
class CLabelButton : public CButton
{
//	DECLARE_DYNCREATE(CLabelMenu)
// Construction
public:
	CLabelButton();

// Attributes
public:
	int m_Label;

// Operations
public:

// Implementation
public:
	~CLabelButton();
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	// Generated message map functions
//	//{{AFX_MSG(CLabelButton)
//		// NOTE - the ClassWizard will add and remove member functions here.
//	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CLabelMenu menu
class CLabelMenu : public CMenu
{
//	DECLARE_DYNCREATE(CLabelMenu)
// Construction
public:
	CLabelMenu();

// Attributes
public:
//	int m_Label;

// Operations
public:

// Implementation
public:
	~CLabelMenu();
	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	// Generated message map functions
//	//{{AFX_MSG(CLabelMenu)
//		// NOTE - the ClassWizard will add and remove member functions here.
//	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMyBitmapButton window

class CMyBitmapButton : public CBitmapButton
{
// Construction
public:
	CMyBitmapButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyBitmapButton)
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyBitmapButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyBitmapButton)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
class QCCommandObject;
/////////////////////////////////////////////////////////////////////////////
// CMailboxButton window
class CMailboxButton : public CButton
{
// Construction
public:
	CMailboxButton();

// Attributes
public:
	CString	m_ButtonDesc;
	CString	m_MailboxPath;

	// If this represents an IMAP mailbox, use the foll:
	//
	CString m_SelectedImapName;
	BOOL	m_bIsImap;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMailboxButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMailboxButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMailboxButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
public:
	afx_msg BOOL OnNewMailboxInRoot(void);
	afx_msg BOOL OnNewMailbox(QCCommandObject *pCommand);
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	afx_msg BOOL OnDynamicCommand(UINT uID );

	DECLARE_MESSAGE_MAP()

public:
	BOOL SelectMailbox(LPCSTR pPersonalityName = NULL);
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.


#endif //__CONTROLS_H__
