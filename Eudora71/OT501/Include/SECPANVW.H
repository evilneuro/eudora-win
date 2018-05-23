// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	Declarations for SECPanView
//  Created:		Fri Jun 16 13:43:57 1995
//
//  CHANGELOG
//
//   MSW      9/30/95  Code Review
//
//

#ifndef __SECPANVW_H__
#define __SECPANVW_H__

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

//CR MSW: move this into the class.
typedef enum {SEC_PANDELAY,SEC_PANINSTANT} SECPanMode;

class SECPanWnd; //lives in secpanwn.h/panwnd.cpp

class SECPanView : public SECZoomView
{
    friend class SECPanWnd;
    DECLARE_DYNAMIC(SECPanView);

// Construction    
protected:
    SECPanView();         // protected constructor used by dynamic creation

// Attributes
public:    
    void SetPanMode(SECPanMode panMode);
    SECPanMode GetPanMode() {return m_panMode;};
    BOOL IsPanning(){ return m_bPanning;}; //Are we in a pan mode?
    BOOL IsOverviewShown() { return m_pOverview != NULL;};
    
// Operations - virtual so you can change behavior or enhance it.
public:
    //Panning operations
    virtual void StartPan(CPoint * ptPan,
			  UINT nCursorID = 0);
    virtual void ContinuePan(CPoint *ptPan);
    virtual void EndPan(CPoint *ptPan);
    
    //Overview Window operations
    virtual BOOL ShowOverviewWnd(CRect);
    virtual BOOL HideOverviewWnd();
    
    virtual void UpdateOverviewWnd(CView * pSender,
				   LPARAM lHint = 0L,
				   CObject * pHint = NULL);
    virtual void OnUpdateOverview(CView * pSender,
				  LPARAM lHint,CObject * pHint,
				  CDC * pOverviewDC, CWnd * pWndOverview);

// Overrideables - OverRide this to get notifications of zoom level change
protected:    
    virtual void ZoomLevelChanged(void); //From SECZoomView
    
// Implementation
public:
    virtual ~SECPanView();
#ifdef _DEBUG
    void Dump(CDumpContext&) const;
    void AssertValid() const;
#endif //_DEBUG

protected:
    virtual void OnDraw(CDC* pDC) = 0; // Pass on Pure Virtual from SECZoomView
    //Internal functions
    void PerformPan(CPoint *ptEndPan,BOOL bReversePan = FALSE);
    void UpdateOverviewRect();
    void PreOverview();
    void PostOverview();
    void OverviewDestroyed();
    void UpdateViewport(CPoint * lpPtNewLog);
    
protected:
    // Member variables
    UINT       m_nCursorID;
    SECPanMode m_panMode;
    BOOL       m_bPanning;
    HCURSOR    m_hOrigCursor;
    HCURSOR    m_hDragCursor;
    CPoint     m_ptLogStartDrag; //Logical coords of pan start
    SECPanWnd *  m_pOverview;    //The overview window.

    //These member store state needed for the overview window.
    CSize       m_orig_szTotalDev;
    BOOL        m_orig_bCenter;
    UINT        m_orig_zoomMode;    
    BOOL        DoScroll(CSize sizeScroll, BOOL bDoScroll);
    
public:
    // Generated message map functions
    //{{AFX_MSG(SECPanView)
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnHScroll(UINT,UINT,CScrollBar*);
    afx_msg void OnVScroll(UINT,UINT,CScrollBar*);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG
    //}}AFX_MSG
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

#endif // __SECPANVW_H__


