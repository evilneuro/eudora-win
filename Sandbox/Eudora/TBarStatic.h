
#ifndef TBARSTATIC_H
#define TBARSTATIC_H

#ifndef __TBTNWND_H__
#include "tbtnwnd.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// CTBarStaticBtn class
//
class CTBarStaticBtn : public CStatic, public SECWndBtn
{
	DECLARE_BUTTON(CTBarStaticBtn);

// Construction
public:
	CTBarStaticBtn() {}

	// Attributes
public:
	enum NotifyCodes		// Notification codes sent with
	{						// wmSECToolBarWndNotify
		NBase   = 0x0200,
		Entered = NBase + 1	// The user hit return
	};

	// Operations
public:
	// SetMode informs the button when the button either enters/leaves a
	// vertically docked state
	virtual void SetMode(BOOL bVertical);

// Overrideables
protected:
	virtual CWnd* GetWnd();
	virtual BOOL CreateWnd(CWnd* pParentWnd, DWORD dwStyle, CRect& rect, int nID);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnFontCreateAndSet();

// Implementation
public:
	virtual ~CTBarStaticBtn() {}

	// Called when toolbar bitmap has changed ... buttons should 
	// now adjust their size.
	virtual void AdjustSize();
	
protected:
	// Implementation, data members.
//	CFont m_font;

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

// Define a combo button
#define STATIC_BUTTON(id, staticId, style, staticStyle, staticDefWidth, \
					 staticMinWidth, staticHeight) \
	WND_BUTTON(BUTTON_CLASS(CTBarStaticBtn), id, staticId, style, staticStyle, \
		       staticDefWidth, staticMinWidth, staticHeight)


#endif 