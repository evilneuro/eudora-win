// BidentReadMessageView.h : interface of the CBidentReadMessageView class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BIDENTREADMESSAGEVIEW_H
#define BIDENTREADMESSAGEVIEW_H

#include "BidentView.h"

class CReadMessageDoc;
class CReadMessageFrame;

class CBidentReadMessageView : public CBidentView
{
	CReadMessageFrame*	m_pParentFrame;

protected: // create from serialization only
	CBidentReadMessageView();
	DECLARE_DYNCREATE(CBidentReadMessageView)

public:
	CReadMessageDoc* GetDocument();

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBidentReadMessageView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBidentReadMessageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBidentReadMessageView)
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSelectAll();
	afx_msg void OnUpdateSelectAll(CCmdUI* pCmdUI);

	afx_msg void OnRefreshFile();
	afx_msg void OnUpdateAlwaysDisabled(CCmdUI* pCmdUI);

public:

	void	SaveInfo();

	virtual BOOL GetSelectedText(
	CString& szText );

	virtual BOOL GetAllText(
	CString& szText );

	virtual BOOL GetSelectedHTML(
	CString& szHTML );

	virtual BOOL GetAllHTML(
	CString& szHTML );

	virtual BOOL SetAllText(
	const char* szText );

	virtual BOOL SetAllHTML(
	const char* szHTML );

	BOOL WriteTempFile( 
	CFile&	theFile,
	BOOL	bShowTabooHeaders );

	virtual BOOL GetMessageAsHTML(
	CString& msg,
	BOOL IncludeHeaders = FALSE);

	virtual BOOL GetMessageAsText(
	CString& msg,
	BOOL IncludeHeaders = FALSE); 

};

#ifndef _DEBUG  // debug version in CarrotView.cpp
inline CReadMessageDoc* CBidentReadMessageView::GetDocument()
   { return (CReadMessageDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
#endif
