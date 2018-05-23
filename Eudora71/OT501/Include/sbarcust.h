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
//
//  Author:			John B. Williston
//  Description:	Definition of SECCustomStatusBar
//  Created:		July 1996
//

#ifndef __SBARCUST_H__
#define __SBARCUST_H__

#ifdef WIN32

#ifndef __AFXCMN_H__
#include "afxcmn.h"
#endif

#ifndef __SBARSTAT_H__
#include "sbarstat.h"
#endif

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


const int SBP_ID			= 0x1;
const int SBP_STYLE			= 0x2;
const int SBP_WIDTH			= 0x4;
const int SBP_TEXT			= 0x8;
const int SBP_TEXT_ALIGN	= 0x10;
const int SBP_FOREGROUND	= 0x20;
const int SBP_BACKGROUND	= 0x40;
const int SBP_BITMAP		= 0x80;
const int SBP_CURSOR		= 0x100;
const int SBP_ALL			= 0xFFFFFFFF;

typedef struct tagPaneInfoEx
{
	// Basic information supported by SECStatusBar::SetPaneInfo.

	int iIndex;
	UINT uiID;
	UINT uiStyle;
	int cxWidth;

	// Extra information supported via SECStatusBar::SetPaneText.

	CString strText;

	// Custom pane handling provided by SECCustomStatusBar.

	COLORREF crTextForeground;
	COLORREF crTextBackground;
	CBitmap* pBitmap;
	HCURSOR hCursor;
	int iTextAlign;
	int iFlags;

	tagPaneInfoEx()
	{
		iIndex = 0;
		uiID = 0;
		uiStyle = 0;
		cxWidth = 0;
		crTextForeground = 0;
		crTextBackground = 0;
		pBitmap = NULL;
		hCursor = 0;
		iTextAlign = TA_LEFT;
		iFlags = 0;
	}

	tagPaneInfoEx& operator=(const tagPaneInfoEx& rhs)
	{
		if (&rhs != this)
		{
			iIndex = rhs.iIndex;
			uiID = rhs.uiID;
			uiStyle = rhs.uiStyle;
			cxWidth = rhs.cxWidth;
			crTextForeground = rhs.crTextForeground;
			crTextBackground = rhs.crTextBackground;
			pBitmap = rhs.pBitmap;
			iTextAlign = rhs.iTextAlign;
			iFlags = rhs.iFlags;
		}

		return *this;
	}

} PANEINFOEX;

class SECCustomStatusBar : public SECStatusBar
{
// Construction
public:
	SECCustomStatusBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECCustomStatusBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECCustomStatusBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(SECCustomStatusBar)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetMinHeight(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM);

	DECLARE_MESSAGE_MAP()

// Overridden members.

protected:

	typedef struct tagWCPane
	{
		COLORREF crFore, crBack;	// foreground and background text colors
		CBitmap* pBitmap;			// pointer to bitmap to display
		HCURSOR hCursor;			// used for setting the cursor
		int iTextAlign;				// text alignment flags
		int iFlags;					// flags indicating which data to use

		tagWCPane()
		{
			crFore = crBack = 0;
			pBitmap = NULL;
			hCursor = 0;
			iTextAlign = TA_LEFT;
			iFlags = 0;
		}

	} WCPane;

	CProgressCtrl* m_pProgressCtrl;
	PANEINFOEX* m_pFirstPaneData;
	WCPane* m_pPanes;
	BOOL ResetIndicators();
	BOOL SimpleRect(CDC* pDC, LPRECT p, COLORREF c);
	CSize GetPaneTextExtent(int iIndex, const CString& str);
	void AllocatePaneInfo();
	void ForcePaneOwnerdraw();
	void ForceOnSizeParent();
	void FreePaneInfo()
		{ delete m_pPanes; }
	void GetPaneRect(int iIndex, LPRECT p) const;
	WCPane* PaneFromPoint(const CPoint& pt) const;
	int PaneIndexFromPoint(const CPoint& pt) const;

public:
	// Overridden methods.

	BOOL SetIndicators(const UINT* lpIDArray, int nIDCount);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

	// Custom pane information API.
	
	void GetPaneInfoEx(PANEINFOEX* pex) const;
	void SetPaneInfoEx(const PANEINFOEX* pex, BOOL bFit = TRUE);

	// Progress control API.

	BOOL InitializeProgressControl(LPCTSTR pcszMsg = NULL, int iLow = 0, 
								   int iHigh = 100, BOOL bResetPos = TRUE);
	BOOL IsProgressControlActive() const
		{ return m_pProgressCtrl != NULL; }
	void StepProgress();
	void SetProgress(int iPos);
	void UninitializeProgressControl();

	// Resets horizontal pane sizing for non-stretchy panes; very
	// useful when changing fonts or using bitmaps.

	void FitPanesToContent();

	// Useful for processing WM_NOTIFY messages to further
	// customize control response.

	int GetIndexFromPoint(const CPoint& pt) const
		{ return PaneIndexFromPoint(pt); }
};


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // WIN32

#endif //__SBARCUST_H__

/////////////////////////////////////////////////////////////////////////////
