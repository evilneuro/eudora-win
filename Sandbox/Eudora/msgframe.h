// MSGFRAME.H
//

#ifndef _MSGFRAME_H_
#define _MSGFRAME_H_

#include "MDIchild.h"
#include "MsgSplitter.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageFrame frame

class CMessageFrame : public CMDIChild
{
	DECLARE_DYNAMIC(CMessageFrame)

protected:
	CMsgSplitterWnd m_wndSplitter;

	CMessageFrame();			// protected constructor used by dynamic creation

// Attributes
public:

	// HATE THIS!!!
	CRect			m_InitialSize;

// Operations
public:
	
	virtual void ActivateFrame(int nCmdShow = -1);

	virtual void EnableTooltips(BOOL bEnable) = 0;
	
// Implementation
protected:
	virtual ~CMessageFrame();

	// Generated message map functions
	//{{AFX_MSG(CMessageFrame)
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	virtual BOOL OnCreateClient(
	LPCREATESTRUCT	lpcs, 
	CCreateContext* pContext) = 0;
	
	virtual BOOL OnCmdMsg( 
	UINT				nID, 
	int					nCode, 
	void*				pExtra, 
	AFX_CMDHANDLERINFO* pHandlerInfo );

	virtual BOOL PreTranslateMessage(
	MSG*	pMsg );
	
	DECLARE_MESSAGE_MAP()
};

#endif		// _MSGFRAME_H_
