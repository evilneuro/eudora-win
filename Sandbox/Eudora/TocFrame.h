#if !defined(AFX_TOCFRAME_H__CE6A8E92_1B2D_11D1_91DB_00805FD2B1FA__INCLUDED_)
#define AFX_TOCFRAME_H__CE6A8E92_1B2D_11D1_91DB_00805FD2B1FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// TocFrame.h : header file
//

#include "mdidrag.h"
#include "MsgSplitter.h"

class CSummary;

/////////////////////////////////////////////////////////////////////////////
// CTocFrame frame

class CTocFrame : public CMDIChildTarget
{
	INT			m_iMajorVersion;
	INT			m_iMinorVersion;
	CSummary*	m_pPreviewSummary;
	DWORD		m_dwStartPreviewTime;
	int			m_nTocListHeight;

	// for the automarkasread hack
	bool m_bSchmookieState;

	DECLARE_DYNCREATE(CTocFrame)
protected:
	CTocFrame();           // protected constructor used by dynamic creation


// Operations
public:

	INT GetSplitterPos();
	CSummary*	GetPreviewSummary() { return m_pPreviewSummary; }

// Overrides
	virtual BOOL OnCreateClient( LPCREATESTRUCT	lpcs, CCreateContext* pContext);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTocFrame)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL


	INT	GetMajorVersion() const { return m_iMajorVersion; }
	INT	GetMinorVersion() const { return m_iMinorVersion; }
	void ShowHideSplitter();

	// Implementation
protected:
	virtual ~CTocFrame();

	void DoPreviewDisplay();
	bool ShouldPreview();

	CMsgSplitterWnd m_wndSplitter;

	// Generated message map functions
	//{{AFX_MSG(CTocFrame)
	afx_msg void OnChildActivate();
	afx_msg long OnPreviewMessage( WPARAM wParam, LPARAM lParam );
	afx_msg long OnPurgeSummaryCache( WPARAM wParam, LPARAM lParam );
	afx_msg long OnInitPreview( WPARAM	wParam, LPARAM	lParam );
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCmdDelete();
	afx_msg void OnUpdateCmdDelete(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnClose();
	afx_msg void OnToggleHeader();
	afx_msg void OnEditFindFindMsg();
	afx_msg void OnUpdateCmdUndelete (CCmdUI* pCmdUI);
	afx_msg void OnUpdateCmdImapExpunge (CCmdUI* pCmdUI);
	afx_msg void OnCmdUnDelete();
	afx_msg void OnCmdImapExpunge();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CSummary* GetPreviewableSummaryFromDoc();
	BOOL m_bIsPreviewPaneEmpty;
	CSummary* m_pPreviousPreviewableSummaryFromDoc;
	BOOL m_bDoAutoMarkAsRead;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOCFRAME_H__CE6A8E92_1B2D_11D1_91DB_00805FD2B1FA__INCLUDED_)
