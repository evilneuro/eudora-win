/*////////////////////////////////////////////////////////////////////////////

NAME:
	CDummyDlg - Use this dummy modal dialog to launch our PureVoice

FILE:		dummydlg.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

  Use this dummy modal dialog to launch our PureVoice. Derived from CDialog

RESCRITICTIONS:

FILES:


REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/??/96   lss     -Initial
04/30/97   lss     -Separate exe name and command line.
05/02/97   lss     -Added code to receive filename from PureVoice via user msg
					and shared mem.
 					
/*////////////////////////////////////////////////////////////////////////////

#ifndef _DUMMYDLG_H_
#define _DUMMYDLG_H_

class CDummyDlg : public CDialog
{
public:
//	LIFECYCLE
	CDummyDlg( CString& exeName, CString& cmdLine, CWnd* pParent = NULL );

//	ACCESS
	CString m_FileName;
	BOOL	mb_AttachReadMe;

//////////////////////////////////////////////////////////////////////////////
protected:
//	ATTRIBUTES

//	METHODS
	CString		m_exeName;
	CString		m_cmdLine;

//////////////////////////////////////////////////////////////////////////////
// ClassWizard, Window Message Handler, etc... stuff
//////////////////////////////////////////////////////////////////////////////
public:
// Dialog Data
	//{{AFX_DATA(CDummyDlg)
	enum { IDD = IDD_DUMMY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDummyDlg)
	//}}AFX_VIRTUAL
protected:
	// Generated message map functions
	//{{AFX_MSG(CDummyDlg)
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnGetFileName(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif