/////////////////////////////////////////////////////////////////////////////
// Panel.h : header file for the SECPanel component of the SECCalendar 
//            custom control.
//
// Stingray Software Extension Classes
// Copyright (C) 1996 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG:
//
//    PCB	01/19/96	Started
//
//


#ifndef __SEC_PANEL_H__
#define __SEC_PANEL_H__

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

#define SEC_PANEL_SCHEMA					0x0100

#define SEC_PANEL_DEBUG						FALSE
//	Drawing mode components.
#define SECPDM_FILLIN						0x0001L
#define SECPDM_BEVELPRESSED					0x0002L
#define SECPDM_BEVELRELEASED				0x0004L
#define SECPDM_OUTLINEPRESSED				0x0008L
#define SECPDM_OUTLINERELEASED				0x0010L
#define SECPDM_HIGHLIGHTPRESSED				0x0020L
#define SECPDM_HIGHLIGHTRELEASED			0x0040L
#define SECPDM_TEXTCENTERED					0x0080L
#define SECPDM_PREPOSTGRAYED				0x0100L

// Behavior mode components
#define SECPBM_TEXTSCALEABLEHEIGHT			0x1000L
#define SECPBM_TEXTSCALEABLEWIDTH			0x2000L
#define SECPBM_SELECTABLE					0x4000L
#define SECPBM_HIGHLIGHTABLE				0x8000L
#define SECPBM_AUTOCLOSEABLE				0x0100L
#define SECPBM_PPSELECTABLE					0x0200L


#define SECPDM_DRAW1 \
			SECPDM_FILLIN | SECPDM_BEVELPRESSED | \
			SECPDM_BEVELRELEASED | SECPDM_OUTLINEPRESSED | \
			SECPDM_OUTLINERELEASED | SECPDM_HIGHLIGHTPRESSED | \
			SECPDM_HIGHLIGHTRELEASED | \
			SECPDM_PREPOSTGRAYED

#define SECPDM_DRAW2 SECPDM_FILLIN | SECPDM_BEVELPRESSED | \
			SECPDM_HIGHLIGHTPRESSED | SECPDM_HIGHLIGHTRELEASED | \
			SECPDM_PREPOSTGRAYED

#define SECPDM_DRAW3 SECPDM_FILLIN | SECPDM_BEVELPRESSED | \
			SECPDM_TEXTCENTERED 

#define SECPDM_DEFAULT_VIEW_DRAW SECPDM_DRAW1
#define SECPDM_DEFAULT_DIALOG_DRAW SECPDM_DRAW1
#define SECPDM_DEFAULT_POPUP_DRAW SECPDM_DRAW3

#define SECPDM_DEFAULT_VIEW_PPDRAW SECPDM_FILLIN | SECPDM_PREPOSTGRAYED
#define SECPDM_DEFAULT_DIALOG_PPDRAW SECPDM_FILLIN | SECPDM_PREPOSTGRAYED
#define SECPDM_DEFAULT_POPUP_PPDRAW SECPDM_FILLIN | SECPDM_PREPOSTGRAYED | \
			SECPDM_TEXTCENTERED

#define SECPBM_DEFAULT_VIEW_BEHAVIOR \
			SECPBM_SELECTABLE|SECPBM_HIGHLIGHTABLE
#define SECPBM_DEFAULT_DIALOG_BEHAVIOR \
			SECPBM_SELECTABLE | SECPBM_HIGHLIGHTABLE
#define SECPBM_DEFAULT_POPUP_BEHAVIOR \
			SECPBM_SELECTABLE

#define SECP_DEFAULTBEVELLINES		  		2

/////////////////////////////////////////////////////////////////////////////
// SECPanel:
// See SECPanel.cpp for the implementation of this class
//
class SECCalendar;

class SECPanel	: public CObject
{
	DECLARE_SERIAL( SECPanel ) 
// Construction
public:
	BOOL HitTest( const CPoint &pt );
	SECPanel( );
	BOOL	Create( CRect r, 
					CWnd *pParent,
					DWORD dwStyle,
					UINT nID,
					BOOL bSelected = FALSE, 
					BOOL bHighlighted = FALSE, 
					long nDrawMode = SECPDM_DEFAULT_DIALOG_DRAW,
					long nBehaviorMode = SECPBM_DEFAULT_DIALOG_BEHAVIOR,
					UINT nBevelLines = SECP_DEFAULTBEVELLINES);

// Attributes
public:
	COLORREF GetGrayTextColor( void );
	void SetGrayTextColor( COLORREF &clr );
	COLORREF SetTextColor( COLORREF &clr );
	CRect SetContainerRect( CRect &rect );
	CRect GetContainerRect( void );
	CRect GetFaceRect( void );
	CString SetTextLabel( CString &theLabel );
	CString SetTextLabel( TCHAR *pszText );
	CString GetTextLabel( void );
	long SetDrawMode( long nMode );
	long GetDrawMode( void );
	UINT SetBevelLines( UINT nLines );
	UINT GetBevelLines( void );
	UINT SetHighlightLines( UINT nLines );
	UINT GetHighlightLines( void );
	BOOL SetSelected( BOOL bSelected, BOOL bRedraw = TRUE );
	BOOL GetSelected( void );
	BOOL SetHighlight( BOOL bHighlight, BOOL bRedraw = TRUE );
	BOOL GetHighlight( void );
	long SetBehaviorMode(long iBehaviorMode);

// Operations
public:
protected:
	void GetMetrics( void );
#ifndef WIN32
	int  GetLogFont(CFont &theFont, LOGFONT* pLogFont);
#endif // WIN32	

// Overrides
public:
	virtual BOOL OnDrawHighlight( CDC *pDC );
	virtual BOOL OnDraw( CDC *pDC );
	virtual BOOL OnDrawBevel( CDC *pDC );
	virtual BOOL OnDrawOutline( CDC *pDC );
	virtual BOOL OnDrawFace( CDC *pDC );
	virtual BOOL OnDrawLabel( CDC *pDC );

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECPanel)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECPanel( );

	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point); 
	void OnRButtonDown(UINT nFlags, CPoint point); 
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
	COLORREF m_clrGrayText;
	void	TextCenteredInRect( CDC *pDC, CRect &theRect, CString& strText);
	void	DrawBevelLines(	CDC *pDC, 
							COLORREF &topLeft, 
							COLORREF &bottomRight,
							UINT nStartOfOffset,
							UINT nEndOfOffset );
	void	DrawRect(CDC *pDC, 
					 CRect theRect,
					 COLORREF theLineColor);
	
	CWnd *m_pParent;
 	long m_iBehaviorMode;
	long m_iDrawMode;
	CString	m_sLabel;
	CRect m_rOuterRect;
	UINT m_nBevelLines;
	UINT m_nHighlightLines;
	BOOL m_bSelected;
	BOOL m_bHighlighted;
	COLORREF m_clrOutline;
	COLORREF m_clrHighlight;
	COLORREF m_clrText;

protected:
	BOOL m_bNeedMetrics;
	BOOL m_bCreated;
	UINT m_iStartOutline;
	UINT m_iStartBevel;
	UINT m_iStartHighlight;
	UINT m_iStartFace;
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SEC_PANEL_H__

