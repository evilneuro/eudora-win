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
//  Description:	Declarations for SECZoomView
//  Created:		7/16/95
//
//  Got most logic from ScrollView, plus the tangram sample, which
//  shows how to use ANISTROPIC mode to do zooming.
//
//  Since we're aiming to be a drop-in replacement for scrollview,
//  we override some of the usual scrollview routines plus we
//  always start with zooming off and let the user set it up,
//  instead of zooming by default.
//

#ifndef __SECZMVW_H__
#define __SECZMVW_H__
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


class SECZoomView : public CScrollView
{
    DECLARE_DYNAMIC(SECZoomView);
public:
    enum {SEC_ZOOMOFF, SEC_ZOOMPANOVER, SEC_ZOOMNORMAL, SEC_ZOOMFIT};
// Construction    
protected:
    SECZoomView();         // protected constructor used by dynamic creation
    
// Operations
public:
    // Overridden CScrollView member functions
    // sizeDefault lives in scrollview?
    void   SetScrollSizes(int nMapMode,
			  SIZE sizeTotal, const SIZE& sizePage = sizeDefault,
			  const SIZE& sizeLine = sizeDefault);
    void   CenterOnLogicalPoint(CPoint ptCenter);
    CPoint GetLogicalCenterPoint(void);
    
    // Zooming functions
    UINT  GetZoomMode() {return m_zoomMode;};
    float GetZoomLevel() {return m_fZoomScale;};
    void  SetZoomMinMax(float fMin, float fMax);
    void  SetZoomMinMaxPercent(int nMin, int nMax);
    void  SetZoomMode(UINT zoomMode);
    
    void     ZoomIn    (CRect &rect);
    void     ZoomIn    (CPoint *point = NULL, float delta = 1.25);
    void     ZoomOut   (CPoint *point = NULL, float delta = 1.25);
    void     ZoomOriginal(CPoint *point = NULL);
    void     ZoomFit  ();
    
    //Arguments slightly different, so people can just enter percent
    //and have point as default instead of having to specify null point.
    void     ZoomPercent(int nPercent = 25,CPoint *point = NULL);
    
    // Zooming utility functions
    void      ViewDPtoLP (LPPOINT lpPoints, int nCount = 1);
    void      ViewLPtoDP (LPPOINT lpPoints, int nCount = 1);
    void      ClientToDevice(CPoint &point);

// Overrideables
protected:
    // Override this to get zoom scale change notifications.
    virtual void ZoomLevelChanged(void) {};
    virtual void ZoomOutOfRange(BOOL bUnderFlow, float fAttempt);
    virtual void SetZoomLevel(float);

// Implementation
protected:
    virtual ~SECZoomView();
#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif //_DEBUG
    virtual void OnDraw(CDC* pDC) = 0; // Pass on Pure Virtual from CScrollView
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
    // Printing support
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    
protected:
    void      PersistRatio(const CSize &orig, CSize &dest, CPoint &remainder);
    void      ReCalcBars(void);
    void      UpdateViewport(CPoint * ptNewCenter);
    
    // Private member variables
    UINT      m_zoomMode;      // The current zoom mode: normal/off/zoom to fit
    CSize     m_szOrigTotalDev;  // Origional total size in device units
    CSize     m_szOrigPageDev;   // Origional page scroll size in device units
    CSize     m_szOrigLineDev;   // Origional line scroll size in device units
    float     m_fZoomScale;      // Current ratio between device/logical units
    float     m_fZoomMin;
    float     m_fZoomMax;
    
public:
    // Generated message map functions
    //{{AFX_MSG(SECZoomView)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    
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

#endif // __SECZMVW_H__
