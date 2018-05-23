#if !defined(AFX_WAZOOWND_H__AA6D0E51_A6C9_11D0_97B4_00805FD2F268__INCLUDED_)
#define AFX_WAZOOWND_H__AA6D0E51_A6C9_11D0_97B4_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WazooWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWazooWnd window

extern UINT umsgActivateWazoo;		// registered Windows message


class CWazooWnd : public CWnd
{
	DECLARE_DYNAMIC(CWazooWnd)

// Construction
public:
	CWazooWnd(UINT nID);

// Attributes
public:

	UINT GetID() const
		{ return m_nID; }

	HICON GetTabIconHandle() const
		{ return m_hIcon; }

	CString GetTabLabel() const
		{ return m_strTabLabel; }

	BOOL HasActiveFocus() const;

	static void SaveInitialChildFocus(const CWnd* pChildWithFocus);

	virtual void OnActivateWazoo();
	virtual void OnDeactivateWazoo();

// Operations
public:

	void	DoContextMenu(
	CWnd*	pWnd, 
	CPoint	point,
	HMENU	hMenu );

	CControlBar* GetParentControlBar() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWazooWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWazooWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWazooWnd)
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	//}}AFX_MSG
	afx_msg void OnInitMenuPopup(CMenu* pMenu, UINT, BOOL bSysMenu);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnCmdSECAllowDocking();
	afx_msg void OnCmdSECHide();
	afx_msg void OnCmdSECMDIFloat();
	afx_msg void OnCmdUpdateSECAllowDocking(CCmdUI* pCmdUI);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL SaveChildWithFocus();

	UINT	m_nID;				// ID for Wazoo window
	CString m_strTabLabel;		// tab label string for Wazoo tab
	HICON	m_hIcon;			// HICON for Wazoo tab
	HWND	m_hwndFocus;		// last child window that had keyboard focus
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAZOOWND_H__AA6D0E51_A6C9_11D0_97B4_00805FD2F268__INCLUDED_)
