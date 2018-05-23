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
//  Authors:		Dean Hallman
//  Description:	Declarations for SECControlBarManager
//  Created:		August 1996
//

#ifdef WIN32

#ifndef __SBARMGR_H__
#define __SBARMGR_H__

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
class SECControlBarInfo;
class SECControlBarInfoEx;

/////////////////////////////////////////////////////////////////////////////
// SECControlBarManager
//

class SECControlBarManager : public CCmdTarget
{
	DECLARE_DYNAMIC(SECControlBarManager)

// Construction
public:
	SECControlBarManager();
	SECControlBarManager(CFrameWnd* pFrameWnd);

// Operations
public:
	// Returns the a pointer to the main frame.
	CFrameWnd* GetFrameWnd() const;

	// Save/Restore manager state
	virtual void LoadState(LPCTSTR lpszProfileName);
	virtual void SaveState(LPCTSTR lpszProfileName) const;

	// Support for dynamic control bar save/restore. Use when
	// a variable number of control bars of a particular type
	// must be saved and restored.  (i.e. User-defined toolbars)
	virtual DWORD GetBarTypeID(CControlBar* pBar) const;
	virtual SECControlBarInfoEx* CreateControlBarInfoEx(SECControlBarInfo*) const;
	virtual CControlBar* DynCreateControlBar(SECControlBarInfo* pBarInfo);

// Overrideables
protected:

// Implementation
public:
	virtual ~SECControlBarManager();

// Attributes
protected:
	CFrameWnd* m_pFrameWnd;				// The main frame.

protected:
	// Generated message map functions
    //{{AFX_MSG(SECControlBarManager)
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SBARMGR_H__

#endif // WIN32