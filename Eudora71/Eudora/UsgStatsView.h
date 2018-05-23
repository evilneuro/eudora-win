// UsgStatsView.h: interface for the CUsageStatisticsView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USGSTATSVIEW_H__2705E758_3BFD_11D4_8559_0008C7D3B6F8__INCLUDED_)
#define AFX_USGSTATSVIEW_H__2705E758_3BFD_11D4_8559_0008C7D3B6F8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "UsgStatsDoc.h"
#include "PaigeEdtView.h"
#include "statmng.h"

#include "eugraph.h"

#include "QICommandClient.h"

extern UINT uRefreshUsageStatisticsWindow;

class CUsageStatisticsView : public CPaigeEdtView , public QICommandClient
{
	DECLARE_DYNCREATE(CUsageStatisticsView)

public:
	CUsageStatisticsView();
	virtual ~CUsageStatisticsView();

	CUsageStatisticsDoc* GetDocument();
	void RefreshUsageStatisticsWindow(long wParam = 0, long lParam = 0);
	void RefreshGraphicalStatistics(long wParam = 0, long lParam = 0);
	void ReloadData(short sSel, BOOL bMoreStatistics = FALSE);
	void SetPeriodSelection(short sSel);
	void ShowMoreStatistics(BOOL bMoreStatistics);

	void DrawGraph(_DEuGraph *pGraph, short sSeriesCount, CString csXString, short sXIntervals, long**ppData, CString csLegendString, short sLabelDisplayStartOffset, short sLabelDisplaySkipCount);

	virtual void			Clear();
	virtual void			ResetCaret();
	virtual void			HideCaret();

	// QICommandClient function.
	virtual void Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUsageStatisticsView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	void ReplaceSpaceFillers();
	static UpdateFT() { };


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


private:
	StatData*	m_pStatData;
	short		m_GraphStyles[3];
	COLORREF	m_GraphColors[3];
	_DEuGraph	m_Graph[STATCOUNT /* For tracking the basic statistics */	+ 2 /* For tracking detailed statistics */ ];

	ULONG		m_nTimerEventID;

	CString		m_csHourString;
	CString		m_csWeekString;
	CString		m_csMonthString;
	CString		m_csYearString;

	BOOL		m_bGraphOCXRegistered;

	void FilterString(CString &csHTML,const char *szBegin,const char *szEnd);

	// Generated message map functions
protected:
	//{{AFX_MSG(CUsageStatisticsView)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		//}}AFX_MSG

	afx_msg long OnRefreshUsageStatisticsWindow( WPARAM	wParam, LPARAM	lParam );
	afx_msg LRESULT OnDisplayChange(WPARAM, LPARAM);

	LPDISPATCH GetIDispatch(COleClientItem *pItem);
public:
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_USGSTATSVIEW_H__2705E758_3BFD_11D4_8559_0008C7D3B6F8__INCLUDED_)
