#if !defined(AFX_MAKEFILTER_H__C23E4482_EE8C_11D0_9CE6_00805F4A39FE__INCLUDED_)
#define AFX_MAKEFILTER_H__C23E4482_EE8C_11D0_9CE6_00805F4A39FE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MakeFilter.h : header file
//


#include "controls.h"

//	Forward declarations
class CFilter;

/////////////////////////////////////////////////////////////////////////////
// CMakeFilter dialog

class CMakeFilter : public CHelpxDlg
{
	enum MATCH_COND
	{
		MATCH_FROM = 0,
		MATCH_RECIPIENT,
		MATCH_SUBJECT
	};
	
	enum ACTION
	{
		TRANS_NEW = 0,
		TRANS_DIFF,
		DELETE_MSG
	};

	CEdit	*m_pMailboxEdit;
	CEdit	*m_pDefFolder;
	BOOL	m_bUserNamedMbox;
	CString m_AllRecips;
	CString m_RootFolder;
	CString m_DiffFolder;

	// Construction
public:
	CMakeFilter(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMakeFilter)
	enum { IDD = IDD_MAKE_FILTER };
	CButton	m_DeleteRadio;
	CButton	m_NewMBoxRadio;
	CButton	m_SelMboxRadio;
	CButton	m_SubRadio;
	CButton	m_FromRadio;
	CButton	m_RecipRadio;
	BOOL	m_InCheck;
	BOOL	m_OutCheck;
	BOOL	m_ManualCheck;
	int		m_MatchRadio;
	CString	m_SubjectText;
	CEdit	m_SubjectEdit;
	CString	m_FromText;
	CEdit	m_FromEdit;
	int		m_ActionRadio;
	CString	m_NewMBoxName;
	CMailboxButton	m_MailboxButton;
	CButton	m_Details;
	CComboBox	m_RecipCombo;
	CString	m_RecipText;
	CString m_DefFolder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMakeFilter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMakeFilter)
	afx_msg void OnDetails();
	virtual BOOL OnInitDialog();
	afx_msg void OnSubjectRadio();
	afx_msg void OnFromRadio();
	afx_msg void OnRecipRadio();
	afx_msg void OnKillfocusNewMbox();
	afx_msg void OnMailbox();
	afx_msg void OnSetfocusNewMbox();
	afx_msg void OnSetfocusFromText();
	afx_msg void OnSetfocusRecipText();
	afx_msg void OnSetfocusSubText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void SetActionChoice(ACTION actionSel = TRANS_NEW);
	void SetMatchChoice(MATCH_COND matchSel = MATCH_FROM);
	void SetupRecipCombo();
	void EnableSelectedCtrl(MATCH_COND matchSel = MATCH_FROM);
	void SetupMailboxSuggest(const char *sugName, UINT defFolder, BOOL isAddress = TRUE);
	int CompareAddress(const char *add1, const char *add2);

public:
	BOOL GoFigure(CSumList *p_SummaryList);
	BOOL GoFigureRecip(CSummary *p_Summary);
	BOOL GoFigureSubject(CSummary *p_Summary);
	BOOL GoFigureFrom(CSummary *p_Summary);
	BOOL IntersectRecip(CString& addList);
	CFilter *SetupFilter();
	virtual void OnOK();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAKEFILTER_H__C23E4482_EE8C_11D0_9CE6_00805F4A39FE__INCLUDED_)
