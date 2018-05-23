// TOCVIEW.H
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#ifndef TOCVIEW_H
#define TOCVIEW_H

#include "3dformv.h"
#include "tocdoc.h"
#include "ComboBoxEnhanced.h"
#include "QICommandClient.h"
#include "QCSharewareManager.h"
#include "QCProtocol.h"

/////////////////////////////////////////////////////////////////////////////
// CTocHeaderButton window

class CTocHeaderButton : public CButton
{
// Construction
public:
	CTocHeaderButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTocHeaderButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTocHeaderButton();
	static BOOL LoadButtonImageCache();
	static void PurgeButtonImageCache();
	static UINT GetSortPopupID() { return s_nSortPopupID; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CTocHeaderButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	static BOOL LoadOneButtonImage(int nResourceID, HBITMAP& hBitmap);

	static HBITMAP m_hBitmapSortByStatus;
	static HBITMAP m_hBitmapSortByJunk;
	static HBITMAP m_hBitmapSortByPriority;
	static HBITMAP m_hBitmapSortByAttachment;
	static HBITMAP m_hBitmapSortByLabel;
	static HBITMAP m_hBitmapSortBySender;
	static HBITMAP m_hBitmapSortByDate;
	static HBITMAP m_hBitmapSortBySize;
	static HBITMAP m_hBitmapSortByServerStatus;
	static HBITMAP m_hBitmapSortByMood;
	static HBITMAP m_hBitmapSortBySubject;

	static CImageList s_ilDigits4x5;

	static UINT s_nSortPopupID;
};

/////////////////////////////////////////////////////////////////////////////

class CCompactButton : public CButton
{
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};

/////////////////////////////////////////////////////////////////////////////
// CTocListBox window
class CTocListBox : public CListBox
{
	DECLARE_DYNCREATE(CTocListBox)

// Construction
public:
	CTocListBox();


// Attributes
public:

// Operations
public:
	CSummary* GetItemDataPtr(int nIndex) const
		{ return ((CSummary*)CListBox::GetItemDataPtr(nIndex)); }
	int GetIndex(const CSummary* Sum) const;
	int GetSelItemIndex() const;
	int SetSel(int nIndex, BOOL bSelect = TRUE, bool in_bResultDirectUserAction = false);
	int GetHorizWindowOrg()
		{ return m_HorizWindowOrg; }

	int GetSelIndices(CDWordArray *pcdwaSelIndices, int *piTopIdx, BOOL bHideDeletedIMAPMsgs);
	void SetSelFromIndices(CDWordArray *pcdwaSelIndices, int iTopIdx, BOOL bHideDeletedIMAPMsgs);

	void			SetSelectionResultDirectUserAction(bool in_bSelectionResultDirectUserAction) { m_bSelectionResultDirectUserAction = in_bSelectionResultDirectUserAction; }
	void			SetAllowSelChangeNotifications(bool in_bAllowSelChangeNotifications) { m_bAllowSelChangeNotifications = in_bAllowSelChangeNotifications; }

// Implementation
public:
	virtual ~CTocListBox();
	void ResetTTSPhrase();

	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	FieldWidthType FindSelectedField(const CPoint& point);
	BOOL GetItemText(int nIndex, CString& rString) const;
	BOOL IsDraggingOver(const CPoint& clientPt) const;
	void SameItemSelect(const CPoint& ClickPoint, int Index);
	int SameItemSelectCompare(FieldWidthType Field, CSummary* Sum1, CSummary* Sum2);
	void TTSCharacterPressed(UINT nChar);
	void MaybeSetSelCenter(int idx, bool bUnSelAll = true);
	void SetSelCenter(int idx, bool bUnSelAll = true);
	void ResetContent();
	void HandleSelectionMayHaveChanged(bool bSelectionResultDirectUserAction = true);

	//
	// Make this public so that the context menu can be displayed by
	// the preview pane.
	//
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	
protected:
	// Generated message map functions
	//{{AFX_MSG(CTocListBox)
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnKillfocus();
	afx_msg void OnSetfocus();
	afx_msg void OnSelChange();
	afx_msg LRESULT OnSetSel(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();


	DECLARE_MESSAGE_MAP()
	
protected:
	void RedisplaySelection();
	int  m_HorizWindowOrg;

	// Whether or not a given programmatic selection change is the result of
	// direct user action. Temporarily set to true just before calling
	// CListBox::SetSel as appropriate.
	bool			m_bSelectionResultDirectUserAction;
	
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:    
    // Needed to make this control an OLE data SOURCE (see OnMouseMove)
    COleDataSource m_OleDataSource;
	CRect m_DataSourceRect;
	CPoint m_SavedMouseDownPoint;
	enum
	{
		MOUSE_IDLE,
		MOUSE_DOWN_ON_SELECTION,
		MOUSE_DOWN_OFF_SELECTION,
		MOUSE_DOWN_FAKE,
		MOUSE_IS_MOVING,
		MOUSE_IS_DRAGGING
	} m_MouseState;
	BOOL m_AllowDragAndDrop;

	// Type To Select Stuff
	DWORD			m_nLastKeyTime;
	char			m_pFindPhrase[33];
	unsigned int	m_nFindPhraseLen;
	bool			m_bTTSSubject;

	// Whether or not HandleSelectionMayHaveChanged should be called. Set to
	// true in constructor and false in OnDestroy so that we return quickly
	// when DeleteItem is called as each and every list item is deleted.
	bool			m_bAllowSelChangeNotifications;
		
	LPSTR SkipSubjectPrefix(LPCSTR pSubject);
	int GetNextWrapIndex(int nCurIdx, int nItemCount, int nStartIdx);
	void FindTTS();

	CSafetyPalette m_Palette;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CTocView dialog

class CTocView : public C3DFormView, public QICommandClient, public QCProtocol
{
	DECLARE_DYNCREATE(CTocView)
protected:
	CTocView();			// protected constructor used by dynamic creation

// Form Data
public:
	//{{AFX_DATA(CTocView)
	enum { IDD = IDD_TOC };
	CCompactButton			m_Compact;
	CComboBoxEnhanced		m_ProfileCombo;
	CScrollBar				m_HScrollBar;
	CTocListBox				m_SumListBox;
	//}}AFX_DATA
	CTocHeaderButton	m_HeaderButton[CTocDoc::NumFields];		// Sort buttons
	short				m_FieldSep[CTocDoc::NumFields - 1];		// Positions of field separators

// Attributes
public:

// Operations
public:
    CTocDoc* GetDocument()
    	{ return ((CTocDoc*)C3DFormView::GetDocument()); }
	bool IsLastSortBySubject() const { return m_bLastSortBySubject; }
    BOOL Selected(const CSummary* Sum) const
    	{ return (m_SumListBox.GetSel(m_SumListBox.GetIndex(Sum))); }
    int Select(CSummary* Sum, BOOL bSelect = TRUE, bool in_bResultDirectUserAction = false)
		{ return m_SumListBox.SetSel(m_SumListBox.GetIndex(Sum), bSelect, in_bResultDirectUserAction); }
    int GetSelCount() const
    	{ return (m_SumListBox.GetSelCount()); }
	void SelectAll(BOOL bSelect = TRUE, BOOL bRedisplay = TRUE, bool in_bResultDirectUserAction = false);
	BOOL GetSelText(CString& rString);
    	 
	BOOL Sort(UINT nID);
	BOOL Sort(UINT nButtonID, UINT nSortDirectionID);
	BOOL SortNow();
	void MaybeResetContents();
	void ResetContents();
	void FillInContents();
	void SetCompact();
	void UpdateProfileComboBox(int in_nNumItemsSelected);
	void OpenMessages();
	void Fumlub();
	BOOL OnEnterKey(UINT nID);
	int TopVisibleItem();
	
public:
	// Overrides for default CView Drag 'n Drop implementations
	virtual BOOL OnDrop(COleDataObject* pDataObject,
						DROPEFFECT dropEffect, 
						CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject,
								   DWORD dwKeyState, 
								   CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
								  DWORD dwKeyState, 
								  CPoint point);
	virtual void OnDragLeave();

	virtual void RefreshView() {	OnSize(SIZE_RESTORED, -1, -1); }

	//
	// MFC is screwed up here.  The virtual OnCmdMsg method is public 
	// in CCmdTarget, but protected in CView.  Therefore, we need to
	// to make a public wrapper to call the protected base class version.
	//
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{ 
			return C3DFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

	// QCProtocol Overrides.
	virtual BOOL DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	virtual BOOL DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);

// Implementation
protected:
	~CTocView();
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void OnInitialUpdate();
	
	// Generated message map functions
	//{{AFX_MSG(CTocView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnCompact();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	afx_msg void OnFileSaveAs();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnSelectAll();
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
	afx_msg void OnGroupBySubject();
	afx_msg void OnUpdateGroupBySubject(CCmdUI* pCmdUI);
	afx_msg BOOL OnSortPopup(UINT nID);
	afx_msg void OnUpdateSortPopup(CCmdUI* pCmdUI);	
	afx_msg void OnProfileComboBoxItemSelected();

	DECLARE_MESSAGE_MAP()

protected:
	BOOL FindTextInSum(CSummary *pSum, LPCTSTR pStr, BOOL bMatchCase, BOOL bWholeWord);

	void RedisplayHeaderSortChange(SortType Header, BOOL bAdding);
	void SizeBottomOfWindowControls(CRect * out_pRectControl = NULL);
	void SizeColumn(int FieldNum);
	int HeaderSplitColumn(const CPoint& point);
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	int m_OldWidth, m_OldHeight;
	QCSharewareManager* m_pSWM;
	bool	m_bProfileComboBoxIsForMultipleSel;
	bool	m_bLastSortBySubject;

public:
	void SetFieldSeparators();
	void SizeHeaders();
	SortType GetSortMethod(unsigned Column)
		{ return GetDocument()->GetSortMethod(Column); }
	void SetSortMethod(unsigned Column, SortType NewSortMethod)
		{ GetDocument()->SetSortMethod(Column, NewSortMethod); }
	BOOL GetGroupBySubject()
		{ return GetDocument()->m_bGroupBySubject; }

	virtual void Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	virtual BOOL PreTranslateMessage( MSG* msg );

private:
	COleDropTarget m_dropTarget;
};


// Bitmaps preloaded for performance
extern CBitmap g_AttachmentBitmap;

//
// Preloaded Change Server Status menu bitmaps.
//
extern CBitmap g_ServerLeaveBitmap;
extern CBitmap g_ServerFetchBitmap;
extern CBitmap g_ServerDeleteBitmap;
extern CBitmap g_ServerFetchDeleteBitmap;

//
// Preloaded Change Priority menu bitmaps.
//
extern CBitmap g_PriorityLowestMenuBitmap;
extern CBitmap g_PriorityLowMenuBitmap;
extern CBitmap g_PriorityNormalMenuBitmap;
extern CBitmap g_PriorityHighMenuBitmap;
extern CBitmap g_PriorityHighestMenuBitmap;

//
// Preloaded bitmap objects for Change Status menu graphics.
//
extern CBitmap g_StatusUnreadBitmap;
extern CBitmap g_StatusReadBitmap;
extern CBitmap g_StatusRepliedBitmap;
extern CBitmap g_StatusForwardedBitmap;
extern CBitmap g_StatusRedirectBitmap;
extern CBitmap g_StatusUnsendableBitmap;
extern CBitmap g_StatusSendableBitmap;
extern CBitmap g_StatusQueuedBitmap;
extern CBitmap g_StatusSentBitmap;
extern CBitmap g_StatusUnsentBitmap;
extern CBitmap g_StatusTimeQueuedBitmap;
extern CBitmap g_StatusRecoveredBitmap;

// Current cascade position for multiple openned message windows
extern int MessageCascadeSpot;
#endif
