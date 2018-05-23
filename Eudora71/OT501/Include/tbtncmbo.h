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
//  Description:	Declarations for SECToolBar
//  Created:		August 1996
//

#ifdef WIN32

#ifndef __TBTNCMBO_H__
#define __TBTNCMBO_H__

#ifndef __TBTNWND_H__
#include "tbtnwnd.h"
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

// Forward Declarations
class SECComboBtn;

///////////////////////////////////////////////////////////////////////////////
// SECComboBtnEdit class
//
class SECComboBtnEdit : public CEdit
{
// Construction
public:
	SECComboBtnEdit();

// Implementation
public:
	virtual ~SECComboBtnEdit() {}

public:
	// Implementation, data members.
	SECWndBtn* m_pCombo;				// Parent combobox

protected:
	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

///////////////////////////////////////////////////////////////////////////////
// SECComboBtn class
//
class SECComboBtn : public CComboBox, public SECWndBtn
{
	DECLARE_BUTTON(SECComboBtn);

// Construction
public:
	SECComboBtn() {}

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
	virtual ~SECComboBtn() {}

	// Called when toolbar bitmap has changed ... buttons should 
	// now adjust their size.
	virtual void AdjustSize();

protected:
	// Implementation, data members.
	SECComboBtnEdit m_edit;				// Our subclassed child edit control
	CFont m_font;

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
#define COMBO_BUTTON(id, comboId, style, comboStyle, comboDefWidth, \
					 comboMinWidth, comboHeight) \
	WND_BUTTON(BUTTON_CLASS(SECComboBtn), id, comboId, style, comboStyle, \
		       comboDefWidth, comboMinWidth, comboHeight)

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBTNCMBO_H__

#endif // WIN32