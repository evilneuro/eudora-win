#if !defined(AFX_TABLEDLGS_H__797C38D4_27D2_11D1_8BC1_00805FD2B1C8__INCLUDED_)
#define AFX_TABLEDLGS_H__797C38D4_27D2_11D1_8BC1_00805FD2B1C8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TableDlgs.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewTableDlg dialog

class CNewTableDlg : public CDialog
{
// Construction
public:
	CNewTableDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewTableDlg)
	enum { IDD = IDD_NEW_TABLE };
	float	m_columnWidth;
	int		m_rows;
	int		m_columns;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewTableDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewTableDlg)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLEDLGS_H__797C38D4_27D2_11D1_8BC1_00805FD2B1C8__INCLUDED_)
