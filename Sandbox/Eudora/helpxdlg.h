// mydlg.h : header file
//

#ifndef HELPXDLG_H
#define HELPXDLG_H

/////////////////////////////////////////////////////////////////////////////
// CHelpxDlg dialog

class CHelpxDlg : public CDialog
{
// Construction
public:
	CHelpxDlg(UINT nID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHelpxDlg)
//	enum { IDD = nID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

#ifdef WIN32
	afx_msg long OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg long OnHelp(WPARAM wParam, LPARAM lParam);
#endif
	afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM lParam);
	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
#ifdef WIN32
	afx_msg long OnRightButton(WPARAM wParam, LPARAM lParam);
#endif

	

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHelpxDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
 
};

#endif // HELPXDLG_H
