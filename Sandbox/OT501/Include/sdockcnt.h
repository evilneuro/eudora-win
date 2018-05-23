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
//  Author:			Dean Hallman
//  Description:	Declarations for SECDockContext
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __SDOCKCNT_H__
#define __SDOCKCNT_H__

// CDockBar is an private, undocumented MFC class that we subclass.
// Hence, the necessity to include afxpriv.h
#ifndef __AFXPRIV_H__
#include "afxpriv.h"
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

/////////////////////////////////////////////////////////////////////////////
// SECDockContext

// Forward Declarations
class SECDockBar;
class SECControlBar;

class SECDockContext : public CDockContext
{
// Construction
public:
    SECDockContext(CControlBar* pBar);

// Attributes
protected:
	CRect m_rectFocus;
	CPoint m_ptStartDrag;
	CPoint m_ptPrev;
	int m_nPosDockingRow;
	BOOL m_bPreviouslyFloating;  // After unfloat as MDI child,
									// should we dock or float it
// Operations
public:

// Double Click Operations
	virtual void ToggleDocking();
	virtual void ToggleMDIFloat();

// Drag Operations
	virtual void StartDrag(CPoint pt);
	void Move(CPoint pt);       // called when mouse has moved
	void EndDrag();             // drop

	// Resize Operations
	virtual void StartResize(int nHitTest, CPoint pt);
	void Stretch(CPoint pt);
	void EndResize();

// Operations
	void CancelLoop();

// Implementation
public:
    ~SECDockContext();
    BOOL Track();
    void DrawFocusRect(BOOL bRemoveRect = FALSE);
	DWORD CanDock();

	friend class SECControlBar;
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SDOCKCNT_H__

#endif // WIN32