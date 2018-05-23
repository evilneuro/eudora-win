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
//  Description:	Declarations for SECToolBarSheet/Page
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __TBARPAGE_H__
#define __TBARPAGE_H__

#ifndef __SECRES_H__
#include "secres.h"
#endif

#ifndef __TBARCUST_H__
#include "tbarcust.h"
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
// SECToolBarSheet
//
class SECToolBarSheet : public CPropertySheet
{
// Construction
public:
	SECToolBarSheet(UINT nIDCaption = IDS_TOOLBAR_CUSTOMIZE, 
					CWnd* pParentWnd = NULL, UINT iSelectPage = 0) :
				CPropertySheet(nIDCaption, pParentWnd, iSelectPage) {}

	SECToolBarSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, 
					UINT iSelectPage = 0) :
				CPropertySheet(pszCaption, pParentWnd, iSelectPage) {}

	virtual BOOL OnInitDialog();

protected:
	// Implementation, data members.

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnFloatStatus(WPARAM, LPARAM);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};

/////////////////////////////////////////////////////////////////////////////
// SECToolBarCmdPage
//

class SECToolBarManager;
class SECCustomizeToolBar;
class SECCustomToolBar;

class SECToolBarCmdPage : public CPropertyPage
{
	DECLARE_DYNCREATE(SECToolBarCmdPage)

// Construction
public:
	SECToolBarCmdPage();
	SECToolBarCmdPage(UINT nIDTemplate, UINT nIDCaption = 0);
	SECToolBarCmdPage(LPCTSTR lpszTemplate, UINT nIDCaption = 0);

protected:
	void Initialize();

// Attributes
public:
	enum { IDD = IDD_TOOLBAR_CUSTOMIZE };

// Operations
public:
	void SetManager(SECToolBarManager* pManager);
	void DefineBtnGroup(LPCTSTR lpszTitle, int nBtnCount, UINT* lpBtnIDs);

// Overrideables
protected:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void OnReset();

// Implementation
public:
	~SECToolBarCmdPage();

protected:
	// This has be 'virtualized' so the description id can be changed
	// This is a QUALCOMM change!!!
	virtual void SetDescription(int nID);
	SECCustomToolBar* GetControlBar(UINT nID);

	// Implementation, data members.
	struct SECBtnGroup						// Describes a button group
	{
		SECCustomizeToolBar* pToolBar;		// Pointer to groups toolbar
		LPCTSTR lpszTitle;					// Btn group title
		UINT    nBtnCount;					// No. of btns in group
		UINT*   lpBtnIDs;					// List of btn IDs.

		SECBtnGroup() : pToolBar(NULL), lpszTitle(NULL), nBtnCount(0), 
						lpBtnIDs(0) {}
		~SECBtnGroup();
	};

	CPtrArray          m_btnGroups;			// Array of button groups
	SECToolBarManager* m_pManager;			// My toolbar manager
	int                m_nCurSel;			// The currently selected btn group

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnSelChange();
	afx_msg LONG OnBarNotify(UINT nID, LONG lParam);
	afx_msg LONG OnToolBarStyleChanging(UINT nEvent, LONG lParam);
};

/////////////////////////////////////////////////////////////////////////////
// SECCustomizeToolBar
//
class SECCustomizeToolBar : public SECCustomToolBar
{
// Construction
public:
	SECCustomizeToolBar();

	DECLARE_DYNAMIC(SECCustomizeToolBar);

// Operations
public:
	void DoSize(CRect& rect);

// Overrideables
protected:
	virtual void BalanceWrap(int nRow, Wrapped* pWrap);
	virtual BOOL GetDragMode() const;
	virtual BOOL AcceptDrop() const;
};

// Function to convert Dialog Base Units into pixels.
extern void SECConvertDialogUnitsToPixels(HFONT hFont, int& cx, int& cy);

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBARPAGE_H__

#endif // WIN32