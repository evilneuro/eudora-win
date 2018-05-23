#if !defined(AFX_INSERTLINKDIALOG_H__30A918E3_49DA_11D1_8148_00805FD2622C__INCLUDED_)
#define AFX_INSERTLINKDIALOG_H__30A918E3_49DA_11D1_8148_00805FD2622C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// InsertLinkDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInsertLinkDialog dialog

class CInsertLinkDialog : public CDialog
{
// Construction
public:
	CInsertLinkDialog(const char* szURL, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInsertLinkDialog)
	enum { IDD = IDD_INSERT_LINK };
	CString m_szURL;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsertLinkDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
		CString GetURL() { return m_szURL; }

	

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsertLinkDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSERTLINKDIALOG_H__30A918E3_49DA_11D1_8148_00805FD2622C__INCLUDED_)
