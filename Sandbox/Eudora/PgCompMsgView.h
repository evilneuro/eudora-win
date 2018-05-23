#if !defined(AFX_PGCOMPMSGVIEW_H__35E93281_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
#define AFX_PGCOMPMSGVIEW_H__35E93281_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PgCompMsgView.h : header file
//

#include "PgMsgView.h"
#include "headervw.h"

class CMessageDoc;

/////////////////////////////////////////////////////////////////////////////
// PgCompMsgView view

class PgCompMsgView : public PgMsgView
{
protected:
	PgCompMsgView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(PgCompMsgView)

// Implementation
protected:
	virtual ~PgCompMsgView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// Attributes
public:

// Operations
public:

	BOOL GetMessageAsHTML(
		CString& szText,
		BOOL includeHeaders);

	BOOL GetMessageAsText(
		CString& szText,
		BOOL includeHeaders);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PgCompMsgView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

protected:
    CHeaderView* m_pHeader;

// printing specific stuff
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    void InsertPrintHeader();
    void RemovePrintHeader();

    virtual BOOL PreTranslateMessage( MSG* pMsg );

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
	//{{AFX_MSG(PgCompMsgView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGCOMPMSGVIEW_H__35E93281_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
