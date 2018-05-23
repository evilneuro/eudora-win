// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	Declaration of the panning overview window
//  Created:		Tue Jun 27 15:13:35 1995
//
//   CHANGELOG
//
//      MSW    9/30/95   Code Review
//
//
//
#ifndef __SECPANWN_H__
#define __SECPANWN_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

class SECPanView;
/////////////////////////////////////////////////////////////////////////////
// SECPanWnd window

class SECPanWnd : public CWnd
{
    // We're friends with the pan view. 
    friend class SECPanView;
    
// Construction
public:
    SECPanWnd(SECPanView *);

// Attributes
public:
    virtual BOOL Create(CRect rectCreate);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    
// Operations
public:
    void SetPanRect(CRect rectNewPan);
    void DrawPanRect(CDC * pDC);
    void InvalidateOverviewWnd(CRect * rectUpdate  = NULL );
    void OnUpdate(CView * pSender, LPARAM lHint, CObject * pHint);
    
    
// Overridables
protected:
    
// Implementation
public:
    virtual ~SECPanWnd();
#ifdef _DEBUG
	void Dump(CDumpContext  &dc);
	void AssertValid();
#endif //_DEBUG
    SECPanView * m_pPanView;        //Back pointer to view
    CRect        m_rectLogPan;      //The current viewport
    CSize        m_szLogDrag;       //The size of the rect being dragged
    BOOL         m_bCaptured;       //Is the mosue captured
    HCURSOR      m_hOverviewCursor; //Cursor for dragging the overview rect
 
    //Impementation specific helper functions for overview wnd.
    void ClipOverviewCursor();
    void OverviewDPtoLP(CPoint *);
    
    // Generated message map functions
protected:
    //{{AFX_MSG(SECPanWnd)
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnClose();
	//}}AFX_MSG
    afx_msg void OnPaint();
    
    DECLARE_MESSAGE_MAP();
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif // __SECPANWN_H__



