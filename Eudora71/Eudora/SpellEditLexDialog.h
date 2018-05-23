#if !defined(AFX_SPELLEDITLEXDIALOG_H__1969F8A2_09CF_11D1_9818_00805FD2F268__INCLUDED_)
#define AFX_SPELLEDITLEXDIALOG_H__1969F8A2_09CF_11D1_9818_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define SpellCheck_DLL_ FALSE

#include "ssce.h"

// SpellEditLexDialog.h : header file
//

class CSpellDialog;

/////////////////////////////////////////////////////////////////////////////
// CSpellEditLexDialog dialog

class CSpellEditLexDialog : public CDialog
{
	CSpellDialog*	m_pParent;
	short			m_sEditLexID;

	void copyLexToLb();

// Construction
public:
	
	CSpellEditLexDialog(
	CSpellDialog*	pParent,
	short			sEditLexID );
	
// Dialog Data
	//{{AFX_DATA(CSpellEditLexDialog)
	enum { IDD = IDD_SPELL_EDIT_LEX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellEditLexDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellEditLexDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnLexClearBtn();
	afx_msg void OnLexAddBtn();
	afx_msg void OnLexDelBtn();
	afx_msg void OnSelchangeLexLb();
	//}}AFX_MSG

	afx_msg LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg long OnHelp(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLEDITLEXDIALOG_H__1969F8A2_09CF_11D1_9818_00805FD2F268__INCLUDED_)
