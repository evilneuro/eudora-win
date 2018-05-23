#if !defined(AFX_TEXTFILEVIEW_H__A0F4BC86_FC41_11D3_AD61_0008C7D3A427__INCLUDED_)
#define AFX_TEXTFILEVIEW_H__A0F4BC86_FC41_11D3_AD61_0008C7D3A427__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TextFileView.h : header file

#include "PaigeEdtView.h"

/////////////////////////////////////////////////////////////////////////////
// CTextFileView view

class CTextFileView : public CPaigeEdtView
{
public:
	CTextFileView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextFileView)
	void Serialize(CArchive& ar) ;
// Attributes

	virtual ~CTextFileView();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextFileView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
//	virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextFileView)
		// NOTE - the ClassWizard will add and remove member functions here.
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsertPicture(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTFILEVIEW_H__A0F4BC86_FC41_11D3_AD61_0008C7D3A427__INCLUDED_)
