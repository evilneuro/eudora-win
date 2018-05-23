#if !defined(AFX_SELECTPERSONALITYDLG_H__8A293953_254E_11D1_9CFE_00805F4A39FE__INCLUDED_)
#define AFX_SELECTPERSONALITYDLG_H__8A293953_254E_11D1_9CFE_00805F4A39FE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SelectPersonalityDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectPersonalityDlg dialog

class CSelectPersonalityDlg : public CDialog
{
// Construction
public:
	CSelectPersonalityDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectPersonalityDlg)
	enum { IDD = IDD_SELECT_PERSONALITY };
	CListCtrl	m_PersonalityList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectPersonalityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectPersonalityDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	QCMailboxImageList	m_ImageList;

public:
	CString	m_SelectedPersona;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPERSONALITYDLG_H__8A293953_254E_11D1_9CFE_00805F4A39FE__INCLUDED_)
