#if !defined(AFX_EUGRAPHPPG_H__51E2C877_367B_11D4_8554_0008C7D3B6F8__INCLUDED_)
#define AFX_EUGRAPHPPG_H__51E2C877_367B_11D4_8554_0008C7D3B6F8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// EuGraphPpg.h : Declaration of the CEuGraphPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CEuGraphPropPage : See EuGraphPpg.cpp.cpp for implementation.

class CEuGraphPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CEuGraphPropPage)
	DECLARE_OLECREATE_EX(CEuGraphPropPage)

// Constructor
public:
	CEuGraphPropPage();

// Dialog Data
	//{{AFX_DATA(CEuGraphPropPage)
	enum { IDD = IDD_PROPPAGE_EUGRAPH };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CEuGraphPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EUGRAPHPPG_H__51E2C877_367B_11D4_8554_0008C7D3B6F8__INCLUDED)
