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
//  Description:	Declarations for SECToolBarRectTracker
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __TBARTRCK_H__
#define __TBARTRCK_H__

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

/////////////////////////////////////////////////////////////////////////////
// SECToolBarRectTracker
//
class SECToolBarRectTracker : public CRectTracker
{
public:
// Constructors
	SECToolBarRectTracker(LPCRECT lpSrcRect);

// Operations
	BOOL Track(CWnd* pWnd, CPoint point);
	BOOL HorzDrag(CWnd* pWnd, CPoint point);

	virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC,
								 CWnd* pWnd);

// Implementation
protected:
	BOOL TrackHandle(CWnd* pWnd, CPoint point);

private:
// Default constructor
	SECToolBarRectTracker();
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBARTRCK_H__

#endif // WIN32