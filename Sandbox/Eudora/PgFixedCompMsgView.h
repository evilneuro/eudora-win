#if !defined(AFX_PgFixedCompMsgView_H__35E93281_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
#define AFX_PgFixedCompMsgView_H__35E93281_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PgFixedCompMsgView.h : header file
//

#include "PgMsgView.h"
#include "headervw.h"

class CMessageDoc;

/////////////////////////////////////////////////////////////////////////////
// PgFixedCompMsgView view

class PgFixedCompMsgView : public PgMsgView
{
protected:
	PgFixedCompMsgView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(PgFixedCompMsgView)

// Attributes
public:
	INT m_nWordWrapColumn;
	CHeaderView* m_pHeader;

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PgFixedCompMsgView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	virtual void SetPgAreas( /*CPaigeEdtView *view*/ );

	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~PgFixedCompMsgView();
	void ExportMessage( CMessageDoc* pMsgDoc  = NULL );
	void SaveInfo();
	void UpdateScrollBars( bool bSetPage = false );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:

// printing specific stuff
	virtual	void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void InsertPrintHeader();
	void RemovePrintHeader();

private:
	CHARRANGE	chrgHeader;

	//
	// This stores the relative wall clock time for when we get
	// WM_SYSKEYDOWN messages for shortcut keys which set the focus to
	// a header line.  See OnSysKeyDown() and WindowProc() for usage.
	//
	DWORD		m_SysKeyDownTime;

	// Generated message map functions
protected:
	//{{AFX_MSG(PgFixedCompMsgView)

	afx_msg void OnUpdateTextBold( CCmdUI* pCmdUI );
	afx_msg void OnUpdateTextItalic( CCmdUI* pCmdUI );
	afx_msg void OnUpdateTextUnderline( CCmdUI* pCmdUI );
	afx_msg void OnUpdateIndent( CCmdUI* pCmdUI );
	afx_msg void OnUpdateParaAlign( CCmdUI* pCmdUI );
	afx_msg void OnUpdateEditInsertTable(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PgFixedCompMsgView_H__35E93281_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
