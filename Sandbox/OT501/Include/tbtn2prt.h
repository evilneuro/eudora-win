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
//  Description:	Declarations for SECTwoPartBtn
//  Created:		August 1996
//

#ifdef WIN32

#ifndef __TBTN2PRT_H__
#define __TBTN2PRT_H__

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


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// SECTwoPartBtn class
//

// Define the width of the SECTwoPartBtn when in two part mode.
#define SEC_TOOLBAR_TWOPART_WIDTH	37
#define SEC_TOOLBAR_TWOPART_DWIDTH	14

// Styles for SECTwoPartBtn		
#define SEC_TBBS_PRESSED2 (MAKELONG(0, 0x0400))		// 2nd part is pressed
#define SEC_TBBS_TRACKING2 (MAKELONG(0, 0x0800))	// We're tracking 2nd part

// Define a two-part toolbar button
#define TWOPART_BUTTON(id, id2, style, dispatchID) \
	{BUTTON_CLASS(SECTwoPartBtn), id, style|TBBS_BUTTON, id2, dispatchID, 0, \
	 0, 0},

class SECTwoPartBtn : public SECStdBtn
{
// Construction
public:
	SECTwoPartBtn();

// Attributes
public:
	BOOL m_bTwoPartMode;	// Button is horz. aligned - so in two part mode
	UINT m_nID2;			// ID of second part
	UINT m_nDispatchID2;	// Command ID dispatched for second part.
	UINT m_nImage2;			// Bitmap index of second part.

// Operations
public:
	// SetMode informs the button when the button either enters/leaves a
	// vertically docked state
	virtual void SetMode(BOOL bVertical);

// Overrideables
public:
	// Called when toolbar bitmap has changed ... buttons should now adjust
	// their size.
	virtual void AdjustSize();

protected:
	// Initialises button after creation.
	virtual void Init(SECCustomToolBar* pToolBar, const UINT* pData);

	// Draw the button
	virtual void DrawButton(CDC& dc, SECBtnDrawData& data);
	
	// Handle mouse events in the button's window rect.
	virtual BOOL BtnPressDown(CPoint point);
	virtual void BtnPressMouseMove(CPoint point);
	virtual void BtnPressCancel();
	virtual UINT BtnPressUp(CPoint point);

// Implementation
	DECLARE_BUTTON(SECTwoPartBtn)
};

// Define a two-part toolbar button
#define TWOPART_BUTTON(id, id2, style, dispatchID) \
	{BUTTON_CLASS(SECTwoPartBtn), id, style|TBBS_BUTTON, id2, dispatchID, 0, \
	 0, 0},

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBTN2PRT_H__

#endif // WIN32