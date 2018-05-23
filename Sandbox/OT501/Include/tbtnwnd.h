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
//  Authors:		Don
//  Description:	Declarations for SECWndBtn
//  Created:		August 1996
//

#ifdef WIN32

#ifndef __TBTNWND_H__
#define __TBTNWND_H__

#ifndef __TBTNSTD_H__
#include "tbtnstd.h"
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

// Define the hotspot width which a SECWndBtn will use for resizing
#define SEC_WNDBTN_RESIZE_WIDTH		3

///////////////////////////////////////////////////////////////////////////////
// SECWndBtn class
//
class SECWndBtn : public SECStdBtn
{
// Construction
public:
	SECWndBtn();

// Attributes
public:
	enum NotifyCodes		// Notification codes sent with
	{						// wmSECToolBarWndNotify
		NBase   = 0x0100,
		WndInit = NBase + 1	// CWnd has been created and needs initialisation.
	};

	enum InformCode			// Codes informing us to take some kind of action
	{
		IBase   = 0x0100,
		WndMsg  = IBase + 1	// Dispatch message to CWnd
	};

// Operations
public:
	// SetMode informs the button when the button either enters/leaves a
	// vertically docked state
	virtual void SetMode(BOOL bVertical);

	// Modifies our window position
	virtual void SetPos(int x, int y);

	// Invalidates the button
	virtual void Invalidate(BOOL bErase = FALSE) const;

	// CWnd passes messages here first.
	virtual BOOL LButtonDown(UINT nFlags, CPoint point);
	BOOL LButtonUp(UINT nFlags, CPoint point);
	BOOL LButtonDblClk(UINT nFlags, CPoint point);
	BOOL RButtonDown(UINT nFlags, CPoint point);
	BOOL RButtonUp(UINT nFlags, CPoint point);
	BOOL RButtonDblClk(UINT nFlags, CPoint point);
	BOOL SetCursor();

protected:
	void PrePaint();
	void PostPaint();
	void ForwardMessage(UINT nMessage, WPARAM wParam = 0, LPARAM lParam = 0);

	// Pass on a notification to the toolbar owner
	void SendNotify(UINT nCode);

// Overrideables
public:
	// Initialises button after creation.
	virtual void Init(SECCustomToolBar* pToolBar, const UINT* pData);

	// Draw the button
	virtual void DrawButton(CDC& dc, SECBtnDrawData& data);

	// Tool-tip/flyby help hit-testing
	virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	// Informs button of some event
	virtual void InformBtn(UINT nCode, void* pData);

	// Functions called when saving/loading a toolbar state, allowing the
	// button to save state info.
	virtual void GetBtnInfo(BYTE* nSize, LPBYTE* ppInfo) const;
	virtual void SetBtnInfo(BYTE nSize, const LPBYTE pInfo);

	// Called when toolbar bitmap has changed ... buttons should now adjust
	// their size.
	virtual void AdjustSize();

protected:
	virtual CWnd* GetWnd() = 0;			// Returns pointer to our CWnd

	// Creates our CWnd
	virtual BOOL CreateWnd(CWnd* pParentWnd, DWORD dwStyle, CRect& rect, 
						   int nID) = 0;

// Implementation
public:
	virtual ~SECWndBtn() {}

protected:
	// Implementation, data members.
	static HCURSOR hCurHorzDrag;// Shared cursor - used when resizing
	BOOL  m_bWndVisible;		// CWnd is currently visible (not vert. docked)
	int   m_nSize;				// Width of CWnd.
	int   m_nMinSize;			// Minimum width of CWnd.
	int   m_nHeight;			// Wanted Height of CWnd.
	int   m_nRealHeight;		// Actual Height of CWnd.
};

// CWndBtn helper macro
#define WND_BUTTON(class, id, wndId, style, wndStyle, wndDefWidth, \
				   wndMinWidth, wndHeight) \
	{class, id, style|TBBS_BUTTON, wndId, wndStyle, MAKELONG(wndDefWidth, \
     wndMinWidth), wndHeight, 0},

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBTNWND_H__

#endif // WIN32