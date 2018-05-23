// FilterReportView.h : header file
//
#ifndef FILTERREPORTVIEW_H

#define FILTERREPORTVIEW_H

#include <afxcview.h>
#include "QCImageList.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterReportView view

class CFilterReportView : public CListView
{
	QCMailboxImageList	m_ImageList;
	INT					m_iSortIndex;
	BOOL				m_bAscendingOrder;
	

protected:
	DECLARE_DYNCREATE(CFilterReportView)

// Attributes
public:

// Operations
public:
	//
	// Normally, the constructor and destructor are protected since
	// the MFC document template stuff takes care of object creation
	// and destruction.  However, since this is now owned by a Wazoo
	// window, these need to be public.
	//
	CFilterReportView();           
	virtual ~CFilterReportView();

	struct CEntry : public CObject
	{
		CString	m_szMailbox;
		UINT	m_uCount;
		time_t	m_Time;
	};

	BOOL AddEntry( 
	UINT	uCount,
	LPCSTR	szMailbox,
	time_t	tNow );

	BOOL AddEntry(
	CEntry*	pEntry );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterReportView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFilterReportView)
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
