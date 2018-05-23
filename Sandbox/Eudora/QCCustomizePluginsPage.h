#if !defined(AFX_QCCUSTOMIZEPLUGINSPAGE_H__8DA9C245_F6E0_11D0_9804_00805FD2F268__INCLUDED_)
#define AFX_QCCUSTOMIZEPLUGINSPAGE_H__8DA9C245_F6E0_11D0_9804_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QCCustomizePluginsPage.h : header file
//

#include "QCToolBarCmdPage.h"

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePluginsPage dialog

class QCCustomizePluginsPage : public QCToolBarCmdPage
{
	UINT*	m_IDs;
	
	DECLARE_DYNCREATE(QCCustomizePluginsPage)

// Construction
public:
	QCCustomizePluginsPage();
	~QCCustomizePluginsPage();

protected:
	virtual void SetDescription(int nID);
public:
// Dialog Data
	//{{AFX_DATA(QCCustomizePluginsPage)
	enum { IDD = IDD_CUST_TOOLBAR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCCustomizePluginsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(QCCustomizePluginsPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QCCUSTOMIZEPLUGINSPAGE_H__8DA9C245_F6E0_11D0_9804_00805FD2F268__INCLUDED_)
