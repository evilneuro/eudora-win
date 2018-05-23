// AdView.h : header file
//

#ifndef ADVIEW_H
#define ADVIEW_H

#include "PaigeEdtView.h"

/////////////////////////////////////////////////////////////////////////////
// CAdView view

class CAdView : public CPaigeEdtView
{
protected:
	DECLARE_DYNCREATE(CAdView)

// Attributes
public:

// Operations
public:
	//
	// Normally, the constructor and destructor are protected since
	// the MFC document template stuff takes care of object creation
	// and destruction.  However, since this is now owned by a Wazoo
	// window, these need to be public.
	//
	CAdView();           
	virtual ~CAdView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CAdView)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif //ADVIEW_H
