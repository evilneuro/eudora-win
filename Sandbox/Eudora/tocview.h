// TOCVIEW.H
//
#ifndef TOCVIEW_H
#define TOCVIEW_H

#include "3dformv.h"
#include "tocdoc.h"
#include "QICommandClient.h"

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

	// Generated message map functions
protected:
	//{{AFX_MSG(CTocHeaderButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	static BOOL LoadOneButtonImage(int nResourceID, HBITMAP& hBitmap);

	static HBITMAP m_hBitmapSortByStatus;
	static HBITMAP m_hBitmapSortByPriority;
	static HBITMAP m_hBitmapSortByAttachment;
	static HBITMAP m_hBitmapSortByLabel;
	static HBITMAP m_hBitmapSortBySender;
	static HBITMAP m_hBitmapSortByDate;
	static HBITMAP m_hBitmapSortBySize;
	static HBITMAP m_hBitmapSortByServerStatus;
	static HBITMAP m_hBitmapSortBySubject;

	static CImageList s_ilDigits4x5;
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
	int GetHorizWindowOrg()
		{ return m_HorizWindowOrg; }

// Implementation
public:
	virtual ~CTocListBox();
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	FieldWidthType FindSelectedField(const CPoint& point);
	BOOL GetItemText(int nIndex, CString& rString) const;
	BOOL IsDraggingOver(const CPoint& clientPt) const;
	void SameItemSelect(const CPoint& ClickPoint, int Index);
	int SameItemSelectCompare(FieldWidthType Field, CSummary* Sum1, CSummary* Sum2);
	void AlphaCharacterPressed(UINT nChar);
	void SetSelCenter(int idx, bool bUnSelAll = true);

	//
	// Make this public so that the context menu can be displayed by
	// the preview pane.
	//
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	
protected:
	// Generated message map functions
	//{{AFX_MSG(CTocListBox)
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
	BOOL m_GotReturnKeyDown;
	int  m_HorizWindowOrg;
	
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
	int				m_nStartIndex, m_nLastMatchIndex;
	bool			m_bTTSSubject;
	bool			m_bInCanceledMode;
	
	bool MatchTTS(LPCTSTR pStr, LPCTSTR pSubStr);
	LPSTR SkipSubjectPrefix(LPCSTR pSubject);
	int GetNextWrapIndex(int nCurIdx, int nItemCount, int nStartIdx);
	void ResetTTSPos();
	void FindNextTTS();

	CSafetyPalette m_Palette;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CTocView dialog

class CTocView : public C3DFormView, public QICommandClient
{
	DECLARE_DYNCREATE(CTocView)
protected:
	CTocView();			// protected constructor used by dynamic creation

// Form Data
public:
	//{{AFX_DATA(CTocView)
	enum { IDD = IDD_TOC };
	CCompactButton	m_Compact;
	CBitmapButton	m_PeanutButton;
	CScrollBar	m_HScrollBar;
	CTocListBox	m_SumListBox;
	//}}AFX_DATA
	CTocHeaderButton	m_HeaderButton[CTocDoc::NumFields + 1];	// Sort buttons
	short				m_FieldSep[CTocDoc::NumFields];			// Positions of field separators

// Attributes
public:

// Operations
public:
    CTocDoc* GetDocument()
    	{ return ((CTocDoc*)C3DFormView::GetDocument()); }
    BOOL Selected(const CSummary* Sum) const
    	{ return (m_SumListBox.GetSel(m_SumListBox.GetIndex(Sum))); }
    int Select(CSummary* Sum, BOOL bSelect = TRUE)
    	{ return (m_SumListBox.SetSel(m_SumListBox.GetIndex(Sum), bSelect)); }
    int GetSelCount() const
    	{ return (m_SumListBox.GetSelCount()); }
	void SelectAll(BOOL bSelect = TRUE, BOOL bRedisplay = TRUE);
	BOOL GetSelText(CString& rString);
    	 
	void Delete();
	BOOL Sort(UINT nID);
	BOOL SortNow();
	void SetCompact();
	BOOL SetStatus(UINT nID);
	BOOL SetPriority(UINT nID);
	BOOL SetLabel(UINT nID);
	BOOL SetServerStatus(UINT nID);
	void OpenMessages();
	void ComposeMessage(UINT nID, const char* pszDefaultTo, const char* pszStationery = NULL, const char* pszPersona = NULL);
	void SendAgain(const char* pszPersona = NULL);
	void Fumlub();
	BOOL OnEnterKey(UINT nID);
	int TopVisibleItem();

#ifdef IMAP4 // Undelete messages
	void UnDelete();
	void ImapExpunge();
	void ImapHandleServerStatus (UINT nID);
	BOOL OnImapRedownload(UINT nID);
	BOOL OnImapFetch(UINT nID);
	void OnImapClearCached();
	void OnImapResync ();
#endif // IMAP4

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

	virtual BOOL IsReplyable();

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
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	//}}AFX_MSG
	
	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFileSaveAs();
	afx_msg void OnFilePrint();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnSelectAll();
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToggleStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSelNoOut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateResponse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSendable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSentUnsent(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSendImmediately(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChangeQueueing(CCmdUI* pCmdUI);
	afx_msg void OnUpdateServerStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
	afx_msg void OnGroupBySubject();
	afx_msg void OnUpdateGroupBySubject(CCmdUI* pCmdUI);
	afx_msg void OnSendImmediately();
	afx_msg void OnChangeQueueing();
	afx_msg void OnMakeNickname();
	afx_msg void FilterMessages();
	afx_msg void OnToggleStatus();

	afx_msg BOOL OnDynamicCommand(
	UINT uID );
	
	afx_msg void OnUpdateDynamicCommand(
	CCmdUI* pCmdUI);
	
	afx_msg void OnTransferNewInRoot();
	afx_msg void OnUpdateTransferNewInRoot(
	CCmdUI* pCmdUI);

	afx_msg void OnMakeFilter();
	afx_msg void OnPeanutContextMenu();

#ifdef IMAP4 // Undelete messages.
	afx_msg void OnUpdateUndelete (CCmdUI* pCmdUI);
	afx_msg void OnUpdateImapExpunge (CCmdUI* pCmdUI);
	afx_msg	void OnUpdateImapRedownload (CCmdUI* pCmdUI);
	afx_msg	void OnUpdateImapFetch (CCmdUI* pCmdUI);
	afx_msg	void OnUpdateImapClearCached (CCmdUI* pCmdUI);
#endif // IMAP4 
	

	DECLARE_MESSAGE_MAP()

protected:
	bool FindTextInSum(CSummary *pSum, LPCTSTR pStr, bool bMatchCase, bool bWholeWord);

	BOOL OnComposeMessage(UINT nID);
	void OnSendAgain();
	void RedisplayHeaderSortChange(SortType Header, BOOL bAdding);
	void SizeColumn(int FieldNum);
	int HeaderSplitColumn(const CPoint& point);
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    
	BOOL CopyTextToClipboard(const CString& rString) const;

	int m_OldWidth, m_OldHeight;

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
