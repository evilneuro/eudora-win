// TaskErrorView.h : header file

#ifndef _TASKERRORVIEW_H_
#define _TASKERRORVIEW_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"
#include "3dformv.h"
#include "TaskInfo.h"
#include "TaskErrorMT.h"
#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CTaskErrorView : public C3DFormView
{
protected:
	DECLARE_DYNCREATE(CTaskErrorView)

	CTaskErrorView();           // protected constructor used by dynamic creation
	virtual ~CTaskErrorView();

	//
	// MFC is screwed up here.  The virtual Create method is public 
	// in CWnd, but protected in CFormView.  Therefore, we need to
	// to make a public wrapper to call the protected CFormView version.
	//
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
						DWORD dwRequestedStyle, const RECT& rect, 
						CWnd* pParentWnd, UINT nID,
						CCreateContext* pContext = NULL)
		{ 
			return C3DFormView::Create(lpszClassName, lpszWindowName, 
										dwRequestedStyle, rect,
										pParentWnd, nID, pContext);
		}

protected:
	//{{AFX_DATA(CTaskErrorView)
	enum { IDD = IDR_TASK_ERROR };
	CListCtrlEx m_ErrorList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskErrorView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateStatusBar();
	bool m_bErrorStatus;
	void ShowErrorDlg(CTaskErrorMT *);
	void SwitchAndHighlight(unsigned int nIdx);


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Column order
	enum { COL_ICON = 0, COL_TEXT };

	// Task ImageList order
	enum { TIL_ERROR = 0 };

	// Generated message map functions
	//{{AFX_MSG(CTaskErrorView)
	afx_msg long OnActivateWazoo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LONG OnMsgNewError( WPARAM, LPARAM );
	afx_msg LONG OnMsgShowDlg( WPARAM, LPARAM );
	afx_msg LONG OnMsgDelError( WPARAM, LPARAM );
	afx_msg LONG OnMsgRemoveAll( WPARAM, LPARAM );
	afx_msg LONG OnMsgListDblClk(WPARAM, LPARAM);
	afx_msg LONG OnMsgListDeleteKey(WPARAM, LPARAM);
	afx_msg LONG OnMsgListReturnKey(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnInitialUpdate();
	bool m_bInitilized;
	bool m_bDlgShown;

	int FindTaskIndex(unsigned int UID);
	int m_DlgMargin;
	int m_nOldCX, m_nOldCY;
};

CTaskErrorView* QCGetTaskErrorView();

/////////////////////////////////////////////////////////////////////////////

#endif // _TASKERRORVIEW_H_
