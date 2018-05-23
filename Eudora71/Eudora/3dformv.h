// 3DFORMV.H
//
// Base class for CFormViews that need 3D look

/////////////////////////////////////////////////////////////////////////////
// C3DFormView form view

#ifndef THREEDFORMV_H
#define THREEDFORMV_H

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class C3DFormView : public CFormView
{
	DECLARE_DYNAMIC(C3DFormView)
protected:
	C3DFormView(UINT nID);			// protected constructor used by dynamic creation

// Form Data
public:

// Attributes
public:

// Operations
public:
	virtual void OnInitialUpdate();
	
// Implementation
protected:
	virtual ~C3DFormView();

protected:
	// Generated message map functions
	//{{AFX_MSG(C3DFormView)
	afx_msg LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
