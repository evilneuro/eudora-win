#if !defined(AFX_BLAHBLAHBLAHVIEW_H__5E5536C2_DC43_11D0_97E9_00805FD2F268__INCLUDED_)
#define AFX_BLAHBLAHBLAHVIEW_H__5E5536C2_DC43_11D0_97E9_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BlahBlahBlahView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBlahBlahBlahView view

class CBlahBlahBlahView : public CEditView
{
	// these are used to hack print preview
	BOOL	m_bIsIconized;
	BOOL	m_bIsMaximized;

protected:
	CBlahBlahBlahView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBlahBlahBlahView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlahBlahBlahView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBlahBlahBlahView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CBlahBlahBlahView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	afx_msg void OnEditMode();
	afx_msg void OnFilePrintPreview();

	virtual void OnEndPrintPreview( 
	CDC* pDC, 
	CPrintInfo* pInfo, 
	POINT point, 
	CPreviewView* pView );


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLAHBLAHBLAHVIEW_H__5E5536C2_DC43_11D0_97E9_00805FD2F268__INCLUDED_)
