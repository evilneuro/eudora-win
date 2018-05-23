// SplitHlp.h : header file
//
#ifndef SPLITHLP_H
#define SPLITHLP_H
/////////////////////////////////////////////////////////////////////////////
// CSplitterHelpWnd frame with splitter

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CSplitterHelpWnd : public CSplitterWnd
{
	DECLARE_DYNCREATE(CSplitterHelpWnd)
protected:

// Attributes
protected:
	
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitterHelpWnd)
	//}}AFX_VIRTUAL

	virtual CWnd* GetActivePane(int* pRow = NULL, int* pCol = NULL);
	void CSplitterHelpWnd::TrackColumnSize(int x, int col);

// Implementation
public:
	CSplitterHelpWnd();           // protected constructor used by dynamic creation
	virtual ~CSplitterHelpWnd();

	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CSplitterHelpWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
