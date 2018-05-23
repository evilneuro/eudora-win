#if !defined(AFX_PICTURELINKDLG_H__30A918E3_49DA_11D1_8148_00805FD2622C__INCLUDED_)
#define AFX_PICTURELINKDLG_H__30A918E3_49DA_11D1_8148_00805FD2622C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PictureLinkDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPictureLinkDlg dialog

class CPictureLinkDlg : public CDialog
{
// Construction
public:
	CPictureLinkDlg(LPCTSTR szURL = NULL, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPictureLinkDlg)
	enum { IDD = IDD_INSERT_PICTURE_LINK };
	CString m_szURL;
	CString m_szAltText;
	CString m_szClickLink;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CString GetHtmlCode();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureLinkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
		CString GetURL() { return m_szURL; }

	

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPictureLinkDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTURELINKDLG_H__30A918E3_49DA_11D1_8148_00805FD2622C__INCLUDED_)
