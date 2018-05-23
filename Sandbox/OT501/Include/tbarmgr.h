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
//  Description:	Declarations for SECToolBarManager
//  Created:		August 1996
//

#ifdef WIN32

#ifndef __TBARMGR_H__
#define __TBARMGR_H__

#ifndef __SBARMGR_H__
#include "sbarmgr.h"
#endif

#ifndef __TBARCUST_H__
#include "tbarcust.h"
#endif

#ifndef __TBTNSTD_H__
#include "tbtnstd.h"
#endif

#ifndef __SDOCKSTA_H__
#include "sdocksta.h"
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
// SECToolBarManager
//

// Registered messages forwarded by ToolBar manager
extern const int wmSECNotifyMsg;			// ToolBar notification
extern const int wmSECToolBarWndNotify;		// SECWndBtn derivative notification

class SECToolBarManager : public SECControlBarManager
{
	DECLARE_DYNAMIC(SECToolBarManager)

// Construction
public:
	SECToolBarManager();
	SECToolBarManager(CFrameWnd* pFrameWnd);

// Operations
public:
	// Define everything about a default toolbar.
	void DefineDefaultToolBar(UINT nID, 
							  LPCTSTR lpszTitle, 
							  UINT    nBtnCount, 
							  UINT*   lpBtnIDs,
							  DWORD   dwAlignment   = CBRS_ALIGN_ANY,
							  UINT    nDockBarID    = AFX_IDW_DOCKBAR_TOP,
							  UINT    nDockNextToID = NULL,
							  BOOL    bDocked       = TRUE,
							  BOOL    bVisible      = TRUE);

	// Returns TRUE if the mainframe is currently enabled. FALSE if we are
	// in customize-mode and we are pretending that it is disabled.
	BOOL IsMainFrameEnabled();

	// Put's all toolbars in/out of customize mode.
	void EnableConfigMode(BOOL bEnable=TRUE);
	BOOL InConfigMode() const;

	// Returns TRUE (with the btn rect. in question) if we are currently
	// processing a button command.
	BOOL IsToolBarCommand(CRect& rect) const;

	// Returns the toolbar under the given window rect.
	SECCustomToolBar* ToolBarUnderRect(const CRect& rect) const;

	// Returns the buttons for a given default toolbar ID.
	BOOL GetDefaultBtns(UINT nID, int& nCount, UINT*& lpBtnIDs);

	// QC Change: made this virtual
	// Creates a new 'user' toolbar
	virtual SECCustomToolBar* CreateUserToolBar(LPCTSTR lpszTitle = NULL);

	// Loads the toolbar resource (to be shared by all toolbars)
	BOOL LoadToolBarResource(LPCTSTR lpszStdBmpName, 
							 LPCTSTR lpszLargeBmpName = NULL);

	// Returns the rectangle within which toolbar buttons cannot be dropped
	void GetNoDropRect(CRect& rect) const;

	// Runs the view toolbars dialog.
	int ExecViewToolBarsDlg();

	// Passes notification through to all buttons of nID
	void InformBtns(UINT nID, UINT nCode, void* pData);

	// Functions for enabling/disabling tooltips and flyby help
	void EnableToolTips(BOOL bEnable=TRUE);
	void EnableFlyBy(BOOL bEnable=TRUE);
	BOOL ToolTipsEnabled() const;
	BOOL FlyByEnabled() const;

	// Functions for enabling/disabling large buttons
	void EnableLargeBtns(BOOL bEnable=TRUE);
	BOOL LargeBtnsEnabled() const;

	// Functions for enabling/disabled "Cool" look mode
	void EnableCoolLook(BOOL bEnable=TRUE,
					DWORD dwExCoolLookStyles=CBRS_EX_COOLBORDERS|CBRS_EX_GRIPPER);
	BOOL CoolLookEnabled() const;

	// Function to initialise info in toolbar.
	void SetToolBarInfo(SECCustomToolBar* pToolBar);

	// Set/Get the current button map.
	void SetButtonMap(const SECBtnMapEntry* pMap);
	const SECBtnMapEntry* GetButtonMap() const;

	// Save/Restore manager state (i.e. tooltips, cool look etc.)
	virtual void LoadState(LPCTSTR lpszProfileName);
	virtual void SaveState(LPCTSTR lpszProfileName) const;

	// This is called by LoadState to establish default toolbar
	// layout when no configured workspace was loaded. If you
	// are not using a workspace manager, call this directly
	// to establish a default toolbar layout.
	virtual void SetDefaultDockState();

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
	virtual ~SECToolBarManager();

	// Enable/Disable the main frame while a customize dialog is up (disabling
	// all child windows of the mainframe).
	void EnableMainFrame();
	void DisableMainFrame();

	// Function which is executed when a toolbar is about to dispatch WM_COMMAND
	void SetInToolBarCommand(BOOL bIn, CRect& rect);

	// Function which receives notifications from toolbar.
	void BarNotify(int nID, SECCustomToolBar::BarNotifyCode notify);

	// Function specifing which window should receive the toolbar notifications
	void AddNotify(CWnd* pWnd);
	void RemoveNotify(CWnd* pWnd);

	// Sets a window within which buttons cannot be dropped.
	void SetNoDropWnd(CWnd* pWnd);

	// Returns the reference to pointer to toolbar with current config focus.
	SECCustomToolBar* GetConfigFocus() const { return m_pConfigFocus; }
	void SetConfigFocus(SECCustomToolBar* pFocus) { m_pConfigFocus = pFocus; }

	SECBtnDrawData& GetDrawData() { return m_drawData; }

	// Handles system color changes
	void SysColorChange();

	void ChangeBarStyle(BOOL bAdd, DWORD dwStyle);
	void ChangeExBarStyle(BOOL bAdd, DWORD dwExStyle);

protected:
	BOOL ChangeBitmap();


	// Implementation, data members.
	struct SECDefaultToolBar			// Record for a default toolbar
	{
		UINT    nID;					// The toolbar ID
		LPCTSTR lpszTitle;				// The toolbar title
		UINT    nBtnCount;				// No. of default buttons
		UINT*   lpBtnIDs;				// List of default buttons
		DWORD   dwAlignment;			// Default alignment mode 
		UINT    nDockBarID;				// Default bar to dock in
		UINT    nDockNextToID;			// Default bar to dock next to
		BOOL    bDocked;				// FALSE if bar is initialially float
		BOOL    bVisible;				// TRUE if bar is visible
	};

	CDWordArray m_enabledList;			// List of windows which have been
										// disabled (EnableMainFrame)
	CPtrArray m_defaultBars;			// Array of default toolbars
	CPtrArray m_notifyWnds;				// Wnd's to pass toolbar notifications
	CWnd* m_pNoDropWnd;					// Wnd that won't accept dropped btns
	BOOL m_bMainFrameEnabled;			// TRUE if EnableMainFrame called
	BOOL m_bConfig;						// TRUE if in toolbar customize mode
	BOOL m_bToolTips;					// TRUE if tooltips enabled
	BOOL m_bFlyBy;						// TRUE if flyby help enabled
	BOOL m_bCoolLook;					// TRUE if "Cool" look enabled
	BOOL m_bLargeBmp;					// TRUE if using large bitmaps
	BOOL m_bInToolBarCommand;			// TRUE while toolbar sends WM_COMMAND
	const SECBtnMapEntry* m_pBtnMap;	// Maps ID's to button types.
	SECCustomToolBar* m_pConfigFocus;	// Toolbar with current config focus
	CRect m_btnRect;					// Rect of btn sending WM_COMMAND
	CBitmap m_bmp;						// The bitmap used by all the toolbars
	UINT m_nBmpItems;					// No. of buttons in bmp
	LPCTSTR m_lpszStdBmpName;			// ID of standard toolbar bitmap
	LPCTSTR m_lpszLargeBmpName;			// ID of large toolbar bitmap
	UINT* m_pBmpItems;					// Array mapping Cmd IDs to bitmap indexes
	int m_nImgHeight;					// Height of bitmap image
	int m_nImgWidth;					// Width of bitmap image
	SECBtnDrawData m_drawData;			// Shared drawing resources

public:
	CRuntimeClass* m_pToolBarClass;		// Used to create toolbars.

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

#endif // __TBARMGR_H__

#endif // WIN32