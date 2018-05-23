#if !defined(AFX_QCCUSTOMIZERECIPIENTSPAGE_H__8DA9C244_F6E0_11D0_9804_00805FD2F268__INCLUDED_)
#define AFX_QCCUSTOMIZERECIPIENTSPAGE_H__8DA9C244_F6E0_11D0_9804_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QCCustomizeRecipientsPage.h : header file
//

#include "QCToolBarCmdPage.h"
#include "QCImageList.h"

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeRecipientsPage dialog

class QCCustomizeRecipientsPage : public QCToolBarCmdPage
{
	QCMailboxImageList	m_ImageList;

	DECLARE_DYNCREATE(QCCustomizeRecipientsPage)

// Construction
public:
	QCCustomizeRecipientsPage();
	~QCCustomizeRecipientsPage();

// Dialog Data
	//{{AFX_DATA(QCCustomizeRecipientsPage)
	enum { IDD = IDD_CUST_TOOLBAR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCCustomizeRecipientsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	virtual	UINT	TranslateID(
	UINT	uID );

protected:
	// Generated message map functions
	//{{AFX_MSG(QCCustomizeRecipientsPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QCCUSTOMIZERECIPIENTSPAGE_H__8DA9C244_F6E0_11D0_9804_00805FD2F268__INCLUDED_)
