// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:			John Williston
//  Description:	Declarations for SECListBoxEdit, SECListBoxEditorHelper,
//                   SECListBoxEditor, SECListBoxDirEditor and SECListBoxFileEditor
//  Created:		August 1996
//
// Change History:
// 2/26/97 Mark Isham - Added some much needed virtual callbacks for significant events
//

#ifndef __LSTBXEDT_H__
#define __LSTBXEDT_H__

#if defined(WIN32) && !defined(__AFXCMN_H__)
#include "afxcmn.h"
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

// Use the following flags when initializing a SECListBoxEditor object
// to configure it.  Note:  the LBE_TOOLTIPS flag is only valid when
// the LBE_BUTTONS flag is in use.

const int LBE_BROWSE	= 0x1;  // edit has a browse button?
const int LBE_AUTOEDIT	= 0x2;	// alphanumeric key starts edit?
const int LBE_BUTTONS	= 0x4;	// supply command buttons?
const int LBE_TOOLTIPS	= 0x8;	// supply tooltips for buttons ?
const int LBE_SHORTCUTS	= 0x10;	// add, delete, and move from keyboard?
const int LBE_DRAGNDROP	= 0x20;	// drag and drop items in list?

const int LBE_DEFAULT = LBE_AUTOEDIT | LBE_BUTTONS | 
						LBE_TOOLTIPS | LBE_SHORTCUTS |
						LBE_DRAGNDROP;

#define WM_SECLSTBXEDIT_ESCAPE	WM_USER+0x0100	// VK_ESCAPE notification

// Forward declarations to avoid header #include's.
class SECListBoxEdit;
class SECListBoxEditorHelper;

class SECListBoxEditor : public CWnd
{
public:
	SECListBoxEditor();
	virtual ~SECListBoxEditor();
	virtual BOOL Initialize(CWnd* pwndParent, UINT uiID, 
							int iFlags = LBE_DEFAULT);

	BOOL IsEditing() const
		{ return m_pEdit != NULL; }
	virtual void StopEditing();
	CListBox* GetListBoxPtr() const;
	void SetEscape(BOOL bSet) { m_bEscape=bSet; }
	BOOL GetEscape() { return m_bEscape; }

protected:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECListBoxEditor)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SECListBoxEditor)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnNcHitTest(CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Useful constants -- hidden in class scope.

	enum { NUM_BUTTONS = 4, IDC_EDIT = 100, IDC_BUTTON = 101 };

	// Object members.

	CButton m_btnBrowse;				// browse button (if used)
#ifdef WIN32
	CToolTipCtrl m_ToolTip;				// tooltip control
#endif
	SECListBoxEdit* m_pEdit;				// pointer to editing control
	SECListBoxEditorHelper* m_pHelper;	// pointer to helper object
	CRect m_rcButtons[NUM_BUTTONS];		// button rectangles
	int m_iFlags;						// flags controlling operation

	BOOL m_bAddInProgress;
	CString m_strRenameOldName;
	BOOL m_bEscape;						// escape key handler

	// Feedback overridables
	virtual void OnEditingJustStarted() {}
	virtual void OnEditingStopped() {}
	virtual void OnItemRenamed(int nIndexRenamed,CString strOldName,CString strNewName);
	virtual void OnItemAdded(int nIndexNewItem,CString strTextInserted);
	virtual void OnItemMoved(int nOldIndex,int nNewIndex,CString strTextMoved,BOOL WasDragged);

	// item about to be deleted, return FALSE if it should not be deleted
	virtual BOOL OnItemDelete(int nIndexToBeDeleted,CString strTextDeleted);

	// The following methods may be overridden in descendant classes.
	virtual BOOL DoAdd();
	virtual BOOL DoDelete();
	virtual BOOL DoMoveDown();
	virtual BOOL DoMoveUp();
	virtual BOOL StartEditing(TCHAR c = NULL);
	virtual int CalculateHeight();
	virtual void CalculateButtonRectangles();
	BOOL IsFlagSet(int i)
		{ return m_iFlags & i; }
	BOOL AreFlagsSet(int i)
		{ return (m_iFlags & i) == i; }
	BOOL EditCore(int iIndex, BOOL bDelOnCancel, LPCTSTR p = NULL);
	void MoveUpCore(int iIndex);

	// Descendants should override this method to provide browse
	// functionality.  Access to the edit control is available
	// through the protected class members.

	virtual void OnBrowse() { }

	// Grant friendship to a class used in the implementation.

	friend class SECListBoxEditorHelper;
};

class SECListBoxDirEditor : public SECListBoxEditor
{
protected:
	virtual void OnBrowse();
public:
	virtual BOOL Initialize(CWnd* pwndParent, UINT uiID, int iFlags = LBE_DEFAULT)
		{ return SECListBoxEditor::Initialize(pwndParent, uiID, iFlags | LBE_BROWSE); }
};

class SECListBoxFileEditor : public SECListBoxEditor
{
protected:
	CString m_strCaption;
	CString m_strFilters;
	CString m_strInitialFile;
	CString m_strExt;
	virtual void OnBrowse();
public:
	void SetBrowseCaption(const TCHAR* pcszCaption)
		{ m_strCaption = pcszCaption; }
	void SetFileFilters(const TCHAR* pcszFilters)
		{ m_strFilters = pcszFilters; }
	void SetDefaultExtension(const TCHAR* pcszExt)
		{ m_strExt = pcszExt; }
	void SetInitialFileName(const TCHAR* pcszName)
		{ m_strInitialFile = pcszName; }
	virtual BOOL Initialize(CWnd* pwndParent, UINT uiID, int iFlags = LBE_DEFAULT)
		{ return SECListBoxEditor::Initialize(pwndParent, uiID, iFlags | LBE_BROWSE); }
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __LSTBXEDT_H__