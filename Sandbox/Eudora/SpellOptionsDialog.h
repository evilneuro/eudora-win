#if !defined(AFX_SPELLOPTIONSDIALOG_H__4056D7C8_08FF_11D1_9818_00805FD2F268__INCLUDED_)
#define AFX_SPELLOPTIONSDIALOG_H__4056D7C8_08FF_11D1_9818_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SpellOptionsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpellOptionsDialog dialog

class CSpellOptionsDialog : public CDialog
{
	ULONG	m_ulOptionsMask;
	BOOL	m_bAlwaysSuggest;
// Construction
public:
	CSpellOptionsDialog(CWnd* pParent, ULONG ulMask, BOOL bAlwaysSuggest);   // standard constructor
	
	ULONG GetOptionsMask() { return m_ulOptionsMask; }
	BOOL GetAlwaysSuggest() { return m_bAlwaysSuggest; }

// Dialog Data
	//{{AFX_DATA(CSpellOptionsDialog)
	enum { IDD = IDD_SPELL_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlwaysSuggestBtn();
	afx_msg void OnIgnoreCapsBtn();
	afx_msg void OnIgnoreMixedCaseBtn();
	afx_msg void OnIgnoreMixedDigitsBtn();
	afx_msg void OnReportDoubledWordsBtn();
	afx_msg void OnIniIgnoreAllCaps();
	afx_msg void OnIniSuggestPhonetics();
	afx_msg void OnIniSuggestSplitWords();
	afx_msg void OnIniSuggestTypographic();
	//}}AFX_MSG
	afx_msg void OnReportMixedCaseBtn();
	afx_msg void OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg long OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLOPTIONSDIALOG_H__4056D7C8_08FF_11D1_9818_00805FD2F268__INCLUDED_)
