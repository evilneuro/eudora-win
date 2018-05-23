#if !defined(AFX_QCMAILBOXCUSTOMIZETOOLBARPAGE_H__FB91E7E2_EF1D_11D0_9801_00805FD2F268__INCLUDED_)
#define AFX_QCMAILBOXCUSTOMIZETOOLBARPAGE_H__FB91E7E2_EF1D_11D0_9801_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QCMbxCustomizeToolbarPage.h : header file
//

#include "QCToolBarCmdPage.h"
#include "QCMailboxTreeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// QCMailboxCustomizeToolbarPage dialog

class QCMailboxCustomizeToolbarPage : public QCToolBarCmdPage
{
	INT					m_nCurSel;
	QCMailboxTreeCtrl	m_theTreeCtrl;

	DECLARE_DYNCREATE(QCMailboxCustomizeToolbarPage)

// Construction
public:
	QCMailboxCustomizeToolbarPage();
	~QCMailboxCustomizeToolbarPage();

// Dialog Data
	//{{AFX_DATA(QCMailboxCustomizeToolbarPage)
	enum { IDD = IDD_CUST_TOOLBAR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCMailboxCustomizeToolbarPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	UINT	TranslateID(
	UINT	uID );

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(QCMailboxCustomizeToolbarPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QCMAILBOXCUSTOMIZETOOLBARPAGE_H__FB91E7E2_EF1D_11D0_9801_00805FD2F268__INCLUDED_)
