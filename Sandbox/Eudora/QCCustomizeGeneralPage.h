// QCCustomizeGeneral.h: interface for the QCCustomizeGeneral class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCUSTOMIZEGENERAL_H__0051D9A1_6DB0_11D1_A814_0060972F7459__INCLUDED_)
#define AFX_QCCUSTOMIZEGENERAL_H__0051D9A1_6DB0_11D1_A814_0060972F7459__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCToolBarCmdPage.h"

class QCCustomizeGeneralPage : public QCToolBarCmdPage  
{
	DECLARE_DYNCREATE(QCCustomizeGeneralPage)

public:
	// Dialog Data
	//{{AFX_DATA(QCCustomizeGeneralPage)
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCCustomizeGeneralPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(QCCustomizeGeneralPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	QCCustomizeGeneralPage();
	
	virtual ~QCCustomizeGeneralPage();

};

#endif // !defined(AFX_QCCUSTOMIZEGENERAL_H__0051D9A1_6DB0_11D1_A814_0060972F7459__INCLUDED_)
