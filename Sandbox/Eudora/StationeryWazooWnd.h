#ifndef __STATIONERYWAZOOWND_H__
#define __STATIONERYWAZOOWND_H__

// StationeryWazooWnd.h : header file
//
// CStationeryWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "StationeryTree.h"


/////////////////////////////////////////////////////////////////////////////
// CStationeryWazooWnd window

class CContainerView;
class QCStationeryCommand;
extern enum COMMAND_ACTION_TYPE;

class CStationeryWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CStationeryWazooWnd)
	
	QCStationeryCommand*	GetSelected();

	void DoReply(
	COMMAND_ACTION_TYPE theAction );
	
	void DoUpdateReply(
	COMMAND_ACTION_TYPE	theAction,
	CCmdUI*				pCmdUI);

public:
	CStationeryWazooWnd();
	virtual ~CStationeryWazooWnd();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStationeryWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CStationeryWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg LRESULT OnInitialUpdate(WPARAM, LPARAM);

	afx_msg void EnableIfSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReplyWith(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReplyToAllWith(CCmdUI* pCmdUI);
	afx_msg void OnNewStationery();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnNewMessageWith();	
	afx_msg void OnReplyWith();	
	afx_msg void OnReplyToAllWith();
	DECLARE_MESSAGE_MAP()

	CContainerView*	m_pStationeryView;
	CStationeryTree	m_StationeryTree;
};


#endif //__STATIONERYWAZOOWND_H__
