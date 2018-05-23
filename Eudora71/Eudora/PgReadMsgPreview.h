#if !defined(AFX_PGREADMSGPREVIEW_H__80A47E51_0AD1_11D1_AC51_00805FD2626C__INCLUDED_)
#define AFX_PGREADMSGPREVIEW_H__80A47E51_0AD1_11D1_AC51_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PgReadMsgPreview.h : header file
//

#include "PgMsgView.h"


/////////////////////////////////////////////////////////////////////////////
// PgReadMsgPreview view

class PgReadMsgPreview : public PgMsgView
{

protected:
	PgReadMsgPreview();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(PgReadMsgPreview)

// Attributes
public:

// Operations
protected:

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PgReadMsgPreview)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

	virtual CTocDoc *		GetTocDoc();

	virtual BOOL			OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	
	virtual BOOL GetMessageAsText( CString& szText, BOOL IncludeHeaders );
	virtual BOOL SetAssociatedFiles(CStringList * in_pAssociatedFiles);
	virtual BOOL RefreshMessage();

// Implementation
protected:
	virtual ~PgReadMsgPreview();
	
	void				DeleteAssociatedFiles();

	virtual void		SetMoreMessageStuff(
							PgStuffBucket *						pStuffBucket);
	void				FinalizeDisplay(bool bNeedToDraw);
	LRESULT				LoadNewPreview();

	CStringList *		m_pAssociatedFiles;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(PgReadMsgPreview)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	afx_msg LRESULT LoadNewPreview(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGREADMSGPREVIEW_H__80A47E51_0AD1_11D1_AC51_00805FD2626C__INCLUDED_)
