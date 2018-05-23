#if !defined(AFX_PRINTPREVIEWVIEW_H__D45A7C10_6CFE_11D4_B56B_0008C7392A1C__INCLUDED_)
#define AFX_PRINTPREVIEWVIEW_H__D45A7C10_6CFE_11D4_B56B_0008C7392A1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrintPreviewView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewView view

class CPrintPreviewView : public CPreviewView
{
protected:
	CPrintPreviewView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPrintPreviewView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrintPreviewView)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPrintPreviewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPrintPreviewView)
	afx_msg void OnPreviewPrint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRINTPREVIEWVIEW_H__D45A7C10_6CFE_11D4_B56B_0008C7392A1C__INCLUDED_)
