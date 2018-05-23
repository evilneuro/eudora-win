#ifndef __QCCUSTOMIZEPERSONALITIESPAGE_H__
#define __QCCUSTOMIZEPERSONALITIESPAGE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QCCustomizePersonalitiesPage.h : header file
//

#include "QCToolBarCmdPage.h"
#include "QCImageList.h"

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePersonalitiesPage dialog

class QCCustomizePersonalitiesPage : public QCToolBarCmdPage
{
	QCMailboxImageList	m_ImageList;

	DECLARE_DYNCREATE(QCCustomizePersonalitiesPage)

// Construction
public:
	QCCustomizePersonalitiesPage();
	~QCCustomizePersonalitiesPage();

// Dialog Data
	//{{AFX_DATA(QCCustomizePersonalitiesPage)
	enum { IDD = IDD_CUST_TOOLBAR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCCustomizePersonalitiesPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	virtual	UINT	TranslateID(
	UINT	uID );

protected:
	// Generated message map functions
	//{{AFX_MSG(QCCustomizePersonalitiesPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // __QCCUSTOMIZEPERSONALITIESPAGE_H__
