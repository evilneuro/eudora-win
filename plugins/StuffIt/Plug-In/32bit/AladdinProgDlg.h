#if !defined(AFX_ALADDINPROGDLG_H__A702DD31_E3F9_11D0_83D3_00805FD26222__INCLUDED_)
#define AFX_ALADDINPROGDLG_H__A702DD31_E3F9_11D0_83D3_00805FD26222__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxcmn.h> // CProgressCtrl

/* The keyword 'bool' is used throughout the Stuffit engine, so let's ignore the warning. */
#pragma warning(disable:4237) /* warning C4237: nonstandard extension used : 'bool' keyword is reserved for future use */

#include "Engine.h"
using namespace StuffItEngine;

// AladdinProgDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAladdinProgDlg dialog

class CAladdinProgDlg : public CDialog
{
// Construction
public:
	CAladdinProgDlg(CWnd* pParent = NULL);   // standard constructor

	void DoCreateArchive(const char** fileList, archiveWriter writer);
	bool DoExpandArchive(archiveReader reader);

// Dialog Data
	//{{AFX_DATA(CAladdinProgDlg)
	enum { IDD = IDD_ALADDIN_PROGRESS_DLG };
	CProgressCtrl	m_progress_ctl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAladdinProgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
//	const char **m_filelist;
//	archiveWriter m_writer;

	// Generated message map functions
	//{{AFX_MSG(CAladdinProgDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALADDINPROGDLG_H__A702DD31_E3F9_11D0_83D3_00805FD26222__INCLUDED_)
