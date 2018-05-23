#if !defined(AFX_QCCUSTOMIZESTATIONERYPAGE_H__8DA9C242_F6E0_11D0_9804_00805FD2F268__INCLUDED_)
#define AFX_QCCUSTOMIZESTATIONERYPAGE_H__8DA9C242_F6E0_11D0_9804_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QCCustomizeStationeryPage.h : header file
//

#include "QCToolBarCmdPage.h"
#include "QCImageList.h"

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeStationeryPage dialog

class QCCustomizeStationeryPage : public QCToolBarCmdPage
{
	QCMailboxImageList	m_ImageList;

	DECLARE_DYNCREATE(QCCustomizeStationeryPage)

// Construction
public:
	QCCustomizeStationeryPage();
	~QCCustomizeStationeryPage();

	virtual	UINT	TranslateID(
	UINT	uID );

// Dialog Data
	//{{AFX_DATA(QCCustomizeStationeryPage)
	enum { IDD = IDD_CUST_TOOLBAR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCCustomizeStationeryPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(QCCustomizeStationeryPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	afx_msg LONG OnBarNotify(UINT nID, LONG lParam);

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QCCUSTOMIZESTATIONERYPAGE_H__8DA9C242_F6E0_11D0_9804_00805FD2F268__INCLUDED_)
