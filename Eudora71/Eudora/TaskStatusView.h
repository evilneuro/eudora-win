#if !defined(AFX_TASKSTATUSVIEW_H__0BCF1F03_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_)
#define AFX_TASKSTATUSVIEW_H__0BCF1F03_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TaskStatusView.h : header file
//

#include "resource.h"
#include "3dformv.h"
#include "TaskInfo.h"
#include "TaskErrorMT.h"
#include "ListCtrlEx.h"
#include "TaskStatus.h"
#include "TaskSummary.h"
#include <list> // STL list

/////////////////////////////////////////////////////////////////////////////
// CTaskStatusView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CTaskStatusView : public C3DFormView
{
protected:
	DECLARE_DYNCREATE(CTaskStatusView)

	CTaskStatusView();           // protected constructor used by dynamic creation
	virtual ~CTaskStatusView();

// Form Data
protected:
	//{{AFX_DATA(CTaskStatusView)
	enum { IDD = IDR_TASK_STATUS };
	CListCtrlEx m_TaskList;
	//}}AFX_DATA

public:
// Overrides
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

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskStatusView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//	int m_ColStateIdx, m_ColPersonaIdx, m_ColStatusIdx, m_ColDetailsIdx, m_ColProgressIdx;

	// Column Position
	//
	// Index into this array using the column IDs. The value is the position (order).
	// Value of -1 indicates the column is not visible.
	int m_pColPosition[5];

	// Column order (column IDs)
	enum { COL_STATE = 0, COL_PERSONA, COL_STATUS, COL_DETAILS, COL_PROGRESS };

	// Task ImageList order
	enum { TIL_UNKNOWN = 0, TIL_ERROR, TIL_CREATED, TIL_COMPLETE,
			TIL_SEND_QUEUED, TIL_SEND_RUNNING, TIL_SEND_POSTWAIT, TIL_SEND_POSTRUN,
			TIL_REC_QUEUED, TIL_REC_RUNNING, TIL_REC_POSTWAIT, TIL_REC_POSTRUN,
			TIL_INDEXING };

	// Generated message map functions
	//{{AFX_MSG(CTaskStatusView)
	afx_msg void OnDestroy();
	afx_msg long OnActivateWazoo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LONG OnMsgNewInfo( WPARAM, LPARAM);
	afx_msg LONG OnMsgChangeInfo( WPARAM, LPARAM );
	afx_msg LONG OnMsgDeleteInfo( WPARAM, LPARAM );
	afx_msg LONG OnMsgUpdateCols( WPARAM, LPARAM );
	afx_msg LONG OnMsgListRBtn( WPARAM, LPARAM );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnInitialUpdate();
	bool m_bInitilized;

	CString m_sRecvSuffix; // Suffix str added to receive tasks (indicates color)

	int FindInsertIndex(int nColID);
	void DoInsertedCol(const int idx);
	void DoDeletedCol(const int idx);
	void UpdateColumns();
	void NotifyStatusBar();
	void SwitchAndHighlight(unsigned int nIdx);

	bool UpdateFullListInfo();
	bool UpdateListInfo(unsigned int nIdx, CTaskInfoMT *pInfo);
	int GetImageIndex(TaskStatusState, TaskType);
	int FindTaskIndex(unsigned int UID);

	int m_DlgSpacing;
	int m_DlgMargin;
	int m_nOldCX, m_nOldCY;
	CTaskStatus m_TheStatus;
	
	// This is ugly so it's fast. We need to pump integers into the list
	// ctrl very, very, very often. Because the list only takes strings,
	// we keep strings around for integers 0 through 100. This array
	// allows near instant conversion from int to string for this range.
	char m_IntStrCache[101*4 + 1];
	void InitCacheIntStrs(); // Setup all the strings
	const char *GetCacheIntStr(int i); // Get the string, "" if out of range
};


CTaskStatusView* QCGetTaskStatusView();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKSTATUSVIEW_H__0BCF1F03_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_)
