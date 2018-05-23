// STATBAR.H
//

#ifndef _STATUSBAREX_H_
#define _STATUSBAREX_H_

/////////////////////////////////////////////////////////////////////////////
// CStatusBarEx window

#include "TaskStatus.h"

class CStatusBarEx : public SECStatusBar
{
// Construction
public:
	CStatusBarEx();
	virtual ~CStatusBarEx();

	BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
		UINT nID = AFX_IDW_STATUS_BAR);

	static CStatusBarEx *QCGetStatusBar();

protected:
	static CStatusBarEx *m_pStatBar; // One and only global pointer to status bar
	bool m_bBlink;
	int m_nRunningAniRate, m_nWaitingAniRate, m_nErrorAniRate;

	COLORREF m_GraphBorderColor; // Always used

	COLORREF m_DefaultCompleteColor, m_DefaultRemainColor; // Used when both send & recv

	COLORREF m_GraphCompletedColor, m_GraphRemainColor; // Sending color
	COLORREF m_RecvCompleteColor, m_RecvRemainColor; // Receiving color

	CTaskStatus m_TheTaskStatus;
	bool m_bTaskError;

	UINT m_RunningTimerID, m_WaitingTimerID, m_ErrorTimerID;
	unsigned int m_RunningImageIdx, m_WaitingImageIdx, m_ErrorImageIdx; // Cycle for animation

	CImageList *m_TaskImageList;

	bool m_bInitToolTip;
	CToolTipCtrl m_ToolTip;

	void SetStatusToolTipText(UINT nToolID);
	void OnChangeRunningStatus();
	void OnChangeWaitingStatus();
	void OnChangeErrorStatus();

	bool HandleLeftClick(UINT nFlags, CPoint point);
	bool HandleRightClick(UINT nFlags, CPoint point);

	void SetTooltipRect(int nIndex, UINT nIDTool);
	int FindPaneIndex(const CPoint &point);

	//{{AFX_VIRTUAL(CStatusBarEx)
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CStatusBarEx)
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LONG OnMsgTaskStatus(WPARAM, LPARAM);
	afx_msg LONG OnMsgErrorStatus(WPARAM, LPARAM);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

#endif // _STATUSBAREX_H_
