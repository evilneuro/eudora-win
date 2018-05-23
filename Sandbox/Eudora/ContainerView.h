// ContainerView.h : header file
//
// A CView-dervied class that acts as a simple container.
// Helpful for when you need a CView-wrapper around one
// or more controls but don't want the baggage of CFormView.
//

#if !defined(AFX_CONTAINERVIEW_H__0E45E31A_A72D_11D2_B8BA_00805F9B6C0A__INCLUDED_)
#define AFX_CONTAINERVIEW_H__0E45E31A_A72D_11D2_B8BA_00805F9B6C0A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CContainerView view

class CContainerView : public CView
{
	DECLARE_DYNCREATE(CContainerView)

public:
	CContainerView();
	virtual ~CContainerView();

	BOOL Create(CWnd* pParentWnd);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContainerView)
	protected:
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CContainerView)
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

	HWND	m_hwndFocus;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTAINERVIEW_H__0E45E31A_A72D_11D2_B8BA_00805F9B6C0A__INCLUDED_)
