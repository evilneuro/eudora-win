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
//  Description:	Declarations for SECCustomToolBar
//  Created:		August 1996
//
//	Modified:		March 1997 (Mark Isham)
//	Description:	Numerous "cool look" enhancements
//	

#ifdef WIN32

#ifndef __TBARCUST_H__
#define __TBARCUST_H__

#ifndef __SBARCORE_H__
#include "sbarcore.h"
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
struct SECBtnMapEntry;
class  SECToolBarManager;
class  SECStdBtn;
struct SECBtnDrawData;

// Define the height and width of a standard toolbar button
#define SEC_TOOLBAR_BTN_WIDTH       23
#define SEC_TOOLBAR_BTN_HEIGHT      22

// Height of a separator between two toolbar rows.
#define SEPARATOR_HEIGHT  ((m_nSepWidth*2)/3)

// Additional buttons styles
#define SEC_TBBS_FOCUS (MAKELONG(0, 0x0100))		// Btn has customize focus
#define SEC_TBBS_RAISED (MAKELONG(0, 0x0200))		// Btn is raised

// ID of first user defined toolbar. IDs between AFX_IDW_TOOLBAR and
// SEC_IDW_FIRST_USER_TOOLBAR are received for default/system toolbars
#define SEC_IDW_FIRST_USER_TOOLBAR		(AFX_IDW_TOOLBAR + 0x20)

// Helper macro - used for calculating no. of elements in an array.
#define NUMELEMENTS(array) (sizeof(array)/sizeof(array[0]))

// Define the custom toolbar type ID
// This value can be written to the registry (via SaveBarState)
// and a later retore (via LoadBarState) can use this value to
// determine which control bar type to reinstantiate.
#define CBT_SECCUSTOMTOOLBAR	14946  // Control Bar Type ID

///////////////////////////////////////////////////////////////////////////////
// SECCustomToolBar
//
class SECCustomToolBar : public SECControlBar
{
// Construction
public:
	SECCustomToolBar();
	
	BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, UINT nID,
		DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd = NULL,
		CCreateContext* pContext = NULL);

	BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_VISIBLE | WS_CHILD | 
				CBRS_TOP, UINT nID = AFX_IDW_TOOLBAR, LPCTSTR lpszTitle = NULL);

	BOOL CreateEx(DWORD dwExStyle, CWnd* pParentWnd, DWORD dwStyle = 
				WS_VISIBLE | WS_CHILD | CBRS_TOP, UINT nID = AFX_IDW_TOOLBAR, 
				LPCTSTR lpszTitle = NULL);

	DECLARE_DYNCREATE(SECCustomToolBar)

// Attributes
public:
	enum BarNotifyCode				// ToolBar Notification codes
	{
		BarShow,					// The toolbar is being shown
		BarHide,					// The toolbar is being hidden
		BarCreate,					// The toolbar has been created
		BarDelete,					// The toolbar is being deleted
		BarDesChange				// The config focus button has changed
	};

	// Structure used during wrapping. Details each wrap position
	struct Wrapped
	{
		int  m_nAfter;				// The index that we wrap after
		BOOL m_bAtSep;				// TRUE if we are wrapping on a separator
		int  m_nSize;				// Horizontal size of this row
	};
	virtual void GetInsideRect(CRect& rectInside) const;

// Operations
public:
	// Load/Save the toolbar configuration
	virtual void GetBarInfoEx(SECControlBarInfo* pInfo);
	virtual void SetBarInfoEx(SECControlBarInfo* pInfo, CFrameWnd* pFrameWnd);

	// Set/Get the style of a button
	void SetButtonStyle(int nIndex, UINT nStyle);
	UINT GetButtonStyle(int nIndex) const;

	// Remove/Add a button from/to the toolbar
	virtual BOOL RemoveButton(int nIndex, BOOL bNoUpdate = FALSE, INT* pIndex = NULL );
	virtual void AddButton(int nIndex, int nID, BOOL bSeparator = FALSE, 
				   BOOL bNoUpdate = FALSE);

	// Returns the no. of buttons on the toolbar
	inline int GetBtnCount() const { return m_btns.GetSize(); }

	// Returns TRUE if this toolbar is in customize mode.
	inline BOOL InConfigMode() const { return m_bConfig; }

	// Returns TRUE when we are performing a drag with ALT key down (note that
	// we are not in customize mode).
	inline BOOL InAltDragMode() const { return m_bAltDrag; }

	// Converts the command ID to a button index.
	int CommandToIndex(UINT nID) const;

	// Returns the command ID for the given button index.
	UINT GetItemID(int nIndex) const;

	// Returns the index of the currently active button
	int GetCurBtn() const;

	// Converts a command ID to an index in the button bitmap.
	int IDToBmpIndex(UINT nID);

	// Loads the toolbar bitmap resource
	BOOL LoadToolBar(LPCTSTR lpszResourceName);
	BOOL LoadToolBar(UINT nIDResource);
	BOOL LoadBitmap(UINT nIDResource, const UINT* lpIDArray, int nIDCount);
	BOOL LoadBitmap(LPCTSTR lpszResourceName, const UINT* lpIDArray, 
						   int nIDCount);
	void SetToolBarInfo(HBITMAP hBmp, UINT* pBmpItems, UINT nBmpItems, 
						int nImgWidth, int nImgHeight);

	// Sets the buttons on the current toolbar
	BOOL SetButtons(const UINT* lpIDArray, int nIDCount);

	// Returns the rect. of the given button index.
	void GetItemRect(int nIndex, LPRECT lpRect) const;

	// Passes notification through to all buttons of nID
	void InformBtns(UINT nID, UINT nCode, void* pData, BOOL bPass = FALSE);

// Overrideables
public:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout( int nLength, DWORD dwMode );
	virtual void  OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void  OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);
	virtual int   OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

protected:
	virtual void  DoPaint(CDC* pDC);
	virtual void  DelayShow(BOOL bShow);

	// Balance the wrapping of the toolbar.
	virtual void  BalanceWrap(int nRow, Wrapped* pWrap);

	// Returns TRUE if the default drag mode is add.
	virtual BOOL  GetDragMode() const;

	// Returns TRUE if this toolbar accepts dropped buttons.
	virtual BOOL  AcceptDrop() const;

// Overrides
	// override	for custom sizing of toolbar
	virtual void AdjustInsideRectForGripper(CRect& rect,BOOL bHorz);

// Implementation
public:
	virtual ~SECCustomToolBar();

	// override of SECControlBar::DrawBorders
	virtual void DrawBorders(CDC* pDC, CRect& rect);

	// Puts the toolbar into config mode
	void EnableConfigMode(BOOL bEnable);

	// Returns TRUE if "Cool" look is enabled.
	inline BOOL CoolLookEnabled() const { return (m_dwExStyle & CBRS_EX_COOLBORDERS) ? TRUE : FALSE; }

	// Returns TRUE if buttons are to be drawn transparently
	BOOL IsTransparent() const;

	// Functions returning button dimensions
	inline int GetImgWidth() const { return m_nImgWidth; }
	inline int GetImgHeight() const { return m_nImgHeight; }
	inline int GetStdBtnWidth() const { return (m_nImgWidth + 7); }
	inline int GetStdBtnHeight() const { return (m_nImgHeight + 7); }
	inline int GetSepWidth() const { return ((m_nImgWidth + 8) / 3); }
	inline int GetSepHeight() const { return (m_nImgHeight + 7); }

	// Returns the cached drawing data
	SECBtnDrawData& GetDrawData();
	void SetBitmap(HBITMAP hBmp);

protected:
	// Returns TRUE if the given row can be wrapped to reduce toolbar width.
	BOOL CanWrapRow(int nRowToWrap, int nMaxSize, int nRows, Wrapped* pWrap);

	// Perform a wrap of the toolbar to the specified horz. size.
	int PerformWrap(int nLength, Wrapped* pWrap);

	// Calculates the size of the current wrap.
	CSize CalcSize(int nRows, Wrapped* pWrap);

	// Wraps a toolbar to the given horz/vert constraint
	CSize SizeToolBar(int nLength, BOOL bVert);

	// Re-calcs the layout of the toolbar.
	CSize CalcLayout(int nLength, DWORD dwMode);

	// Passes a notification onto the manager.
	void NotifyManager(BarNotifyCode nCode);

	// Invalidates the specified button
	void InvalidateButton(int nIndex);

	// Returns the index of the button under the specified point
	int  ItemFromPoint(CPoint point, CRect& rect) const;

	// Executes the command handler for the specified button index
	void UpdateButton(int nIndex);

	// Sets the mode of the toolbar (TRUE=vert. docked)
	void SetMode(BOOL bVertDock);

	// Removes the config focus from the specified button
	void RemoveConfigFocus(BOOL bRedraw = FALSE);

	// Starts a drag of the specified button
	virtual BOOL DragButton(int nIndex, CPoint point);

	// Handles a button drop
	virtual void DropButton(CRect& dropRect, SECStdBtn* pDragBtn, BOOL bAdd);

	// Finds the row over which the button has been dropped
	void FindRow(CRect& clientRect, CRect& dropRect, int& nRowStart, 
				 int& nRowEnd);

	// Returns reference to pointer to toolbar with current cofnig focus
	SECCustomToolBar* GetConfigFocus() const;
	void SetConfigFocus(SECCustomToolBar* pFocus);

	// Function to provide "Cool" style button with raised look
	void RaiseButton();

	// Creates a new toolbar
	void NewToolBar(CPoint pt, const SECStdBtn& btn);

	// Handles the case of a slight drag of the button (dropped over sep.
	// next to button).
	BOOL SmallDrag(CRect& dropRect, int nRowStart, int nRowEnd, BOOL bAdd);

	// Creates a new button
	virtual SECStdBtn* CreateButton(UINT nID, SECCustomToolBar* pToolBar = NULL);

	// Creates a new separator
	virtual SECStdBtn* CreateSeparator(SECCustomToolBar* pToolBar = NULL);

	// Returns the toolbar under the specified point
	SECCustomToolBar* ToolBarFromPoint(int x, int y);

	// Implementation, data members.
	BOOL                         m_bOwnBitmap;		// TRUE if we own m_hBmp
	UINT                         m_nBmpItems;		// No. of buttons in bmp
	HBITMAP                      m_hBmp;			// The toolbar bitmap
	LPCTSTR                      m_lpszResourceName;// ID of toolbar bitmap
	UINT*                        m_pBmpItems;		// Array mapping Cmd IDs to
													// bitmap indexes
	int                          m_nImgHeight;		// Height of bitmap image
	int                          m_nImgWidth;		// Width of bitmap image
	int							 m_nSepWidth;		// Width of separator
	int							 m_nDown;			// Button pressed/config
	int							 m_nRaised;			// Index of raised button
													// "Cool" look only.
	BOOL						 m_bConfig;			// TRUE if in customize mode
	BOOL						 m_bAltDrag;		// TRUE if we are performing
													// a drag with Alt key down
	SECBtnDrawData*				 m_pDrawData;		// Cached info for drawing
													// btns.
	BOOL						 m_bVertical;		// TRUE if vert. docked
	BOOL 						 m_bFloatSizeUnknown;
	BOOL						 m_bHorzDockSizeUnknown;
	BOOL						 m_bVertDockSizeUnknown;
	CSize						 m_szTbarDockHorz;	// custom toolbar copy of m_szDockHorz
	CSize						 m_szTbarDockVert;	// custom toolbar copy of m_szDockVert
	CSize						 m_szTbarFloat;		// custom toolbar copy of m_szFloat

public:
	int                          m_nMaxBtnWidth;	// Max. width of button

protected:
	// Array of all my buttons
	//
	// Note that this would normally be declared using the template 
	// CTypedPtrArray, but some Unix compilers don't support templates.
	//
	class SECBtnArray : public CPtrArray
	{
	public:
		// Accessing elements
		SECStdBtn* GetAt(int nIndex) const
			{ return (SECStdBtn*)CPtrArray::GetAt(nIndex); }
		SECStdBtn*& ElementAt(int nIndex)
			{ return (SECStdBtn*&)CPtrArray::ElementAt(nIndex); }
		void SetAt(int nIndex, SECStdBtn* ptr)
			{ CPtrArray::SetAt(nIndex, ptr); }

		// Potentially growing the array
		void SetAtGrow(int nIndex, SECStdBtn* newElement)
		   { CPtrArray::SetAtGrow(nIndex, newElement); }
		int Add(SECStdBtn* newElement)
		   { return CPtrArray::Add(newElement); }
		int Append(const SECBtnArray& src)
		   { return CPtrArray::Append(src); }
		void Copy(const SECBtnArray& src)
			{ CPtrArray::Copy(src); }

		// Operations that move elements around
		void InsertAt(int nIndex, SECStdBtn* newElement, int nCount = 1)
			{ CPtrArray::InsertAt(nIndex, newElement, nCount); }
		void InsertAt(int nStartIndex, SECBtnArray* pNewArray)
		   { CPtrArray::InsertAt(nStartIndex, pNewArray); }

		// overloaded operator helpers
		SECStdBtn* operator[](int nIndex) const
			{ return (SECStdBtn*)CPtrArray::operator[](nIndex); }
		SECStdBtn*& operator[](int nIndex)
			{ return (SECStdBtn*&)CPtrArray::operator[](nIndex); }
	} m_btns;

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual afx_msg void OnSysColorChange();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg LONG OnNoDelayHide(UINT nID, LONG lParam);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);

public:
	// QC CHANGE: made this public
	// Sets the config focus to the specified button
	void SetConfigFocus(int nBtn, BOOL bRedraw = FALSE);

};

/////////////////////////////////////////////////////////////////////////////
// SECCustomToolBarInfoEx

class SECCustomToolBarInfoEx : public SECControlBarInfoEx
{
	DECLARE_DYNCREATE(SECCustomToolBarInfoEx)

// Constructors
public:
	SECCustomToolBarInfoEx();

// Attributes
public:

	CString m_strBarName;		// Title of toolbar

	struct BtnInfo			
	{
		UINT  m_nID;			// ID of button
		BYTE  m_nSizeExtraInfo;	// Size of additional data
		BYTE* m_pExtraInfo;		// Ptr to additional data

		BtnInfo() : m_nSizeExtraInfo(0), m_pExtraInfo(NULL) {}
		~BtnInfo();
 	};

	// Array of button information
	CTypedPtrArray<CPtrArray, BtnInfo*> m_btnInfo;

// Operations
public:
	virtual void Serialize(CArchive& ar, SECDockState* pDockState);
	virtual BOOL LoadState(LPCTSTR lpszSection, SECDockState* pDockState);
	virtual BOOL SaveState(LPCTSTR lpszSection);

// Implementation
public:
	virtual ~SECCustomToolBarInfoEx();
};

/////////////////////////////////////////////////////////////////////////////
// inline functions

#ifndef __TBARMGR_H__
#include "tbarmgr.h"
#endif

inline SECCustomToolBar* SECCustomToolBar::GetConfigFocus() const
{
	if(m_pManager) {
		ASSERT_KINDOF(SECToolBarManager, m_pManager);
		return ((SECToolBarManager*)m_pManager)->GetConfigFocus();
	}

	return NULL;
}

inline void SECCustomToolBar::SetConfigFocus(SECCustomToolBar* pFocus)
{
	if(m_pManager) {
		ASSERT_KINDOF(SECToolBarManager, m_pManager);
		((SECToolBarManager*)m_pManager)->SetConfigFocus(pFocus);
	}
}

inline BOOL SECCustomToolBar::IsTransparent() const
{
	return (BOOL) ((m_dwExStyle & CBRS_EX_COOLBORDERS) &&
				   (m_dwExStyle & CBRS_EX_TRANSPARENT));
}


// Loads a toolbar resource into the given bitmap and variables
BOOL SECLoadToolBarResource(LPCTSTR lpszResourceName, CBitmap& bmp, 
							UINT*& pBmpItems, UINT& nBmpItems,
							int& nImgWidth, int& nImgHeight,
							UINT* pBtnCount = NULL, 
							UINT** ppBtnArray = NULL);

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBARCUST_H__

#endif // WIN32