// FILTERSV.H
//
// Copyright (c) 1993-2000 by QUALCOMM, Incorporated
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

#ifndef _EUD_FILTERSV_H_
#define _EUD_FILTERSV_H_


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "QICommandClient.h"
#include "QCImageList.h"		// c_dalew 9/30/99 - Added so we can use QCImageList.
#include "filtersd.h"
#include "controls.h"
#include "QCProtocol.h"


#define NUM_FILT_ACTS	5

// Filter Menu ID's
#define FM_FILTER_ACTIONS	0
#define FM_PRIORITY 		1
#define FM_STATUS			2

// MesSummary priorites
const short PRIOR_HIGHEST	= 0;
const short PRIOR_HIGH		= 1;
const short PRIOR_NORMAL	= 2;
const short PRIOR_LOW		= 3;
const short PRIOR_LOWEST	= 4;
const short PRIOR_LOWER		= 5;
const short PRIOR_RAISE		= 6;


// Check button action contrl values
const int CBA_LBUTTON	= 1;
const int CBA_RBUTTON	= 2;
			

typedef enum
{
	FUT_UPDATEVIEW,
	FUT_UPDATEDOC,
	FUT_DIRTYSEL,
	FUT_CHANGESEL,
	FUT_CHANGENAME,
	FUT_REFRESH,
	FUT_REDRAW,
	FUT_NEWITEM
} FilterUpdateType;


/////////////////////////////////////////////////////////////////////////////
// CFilterLabelButton button
class CFilterLabelButton : public CButton
{
// Construction
public:
	CFilterLabelButton();

// Attributes
public:
	int m_Label;

// Operations
public:

// Implementation
public:
	~CFilterLabelButton();
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	// Generated message map functions
	//{{AFX_MSG(CFilterLabelButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// c_dalew 9/30/99 - Changed CFilterImageList to subclass QCImageList not CImageList.
class CFilterImageList : public QCImageList
{	

public:

	CFilterImageList(){}
	virtual ~CFilterImageList(){}

	BOOL Create()
	{
		return CImageList::Create( IDB_FILTERS, 16, 0, RGB (192, 192, 192) );
	}


	enum ImageOffset
	{
		IMAGE_NOTHING = 0,
		IMAGE_CHECK,
		IMAGE_DISABLE_FREE,
		IMAGE_TRANSFER,
		IMAGE_TRANSFER_TRASH,
		IMAGE_SKIP_REST,
		IMAGE_PERSONALITY,
		IMAGE_COPY_TO,
		IMAGE_PLAY_SOUND,
		IMAGE_PRINT,
		IMAGE_PRIORITY,
		IMAGE_JUNK
	};
};


/////////////////////////////////////////////////////////////////////////////
// CFiltListBox window
class CFilter;

class CFiltListBox : public CDragListBox
{
// Construction
public:
	CFiltListBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFiltListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFiltListBox();

	// Overrides
	int ItemFromPt( CPoint pt, BOOL bAutoScroll = TRUE ) const;
	BOOL DrawSingle(int nIndex);
	BOOL EraseSingle(int nIndex);
	void GetDividerRgn(int nIndex, CRgn &rgn);


	virtual void DrawInsert(int nItem);
	virtual BOOL BeginDrag(CPoint pt);
	virtual void CancelDrag(CPoint pt);
	virtual UINT Dragging(CPoint pt);
	virtual void Dropped(int nSrcIndex, CPoint pt);

	// Generated message map functions
protected:
	bool m_bFirstDragging;
	int m_nStartDragIndex;

	//{{AFX_MSG(CFiltListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CFilterImageList	m_FiltImageList;
	int	DrawIcons( CDC *pDC, POINT pt, CFilter * pFilt );
};


/////////////////////////////////////////////////////////////////////////////
// CFiltersViewLeft form view

class CFiltersViewLeft : public CFormView, public QCProtocol
{
protected:
	CFiltersViewLeft();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFiltersViewLeft)

// Form Data
public:
	//{{AFX_DATA(CFiltersViewLeft)
	enum { IDD = IDR_FILTERS_LEFT };
	CFiltListBox	m_FiltersList;
	CButton	m_Up;
	CButton	m_Remove;
	CButton	m_New;
	CButton	m_Down;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFiltersViewLeft)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFiltersViewLeft();
	void OnInitialUpdate();

	//
	// The following very impolitely hides the public, non-virtual
	// CView::GetDocument() method.  In Eudora 3.x or earlier, this
	// was just a convenient way to type cast the CDocument pointer to
	// a derived CFiltersDoc pointer.  In Eudora 4.x or later, this
	// override actually hooks all views up to the One True CFiltersDoc(tm).
	// 
	CFiltersDoc* GetDocument();

	void DoRefresh();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CFiltersViewLeft)
	afx_msg void OnSelChange();
	afx_msg void OnDown();
	afx_msg void OnNew();
//public:
	afx_msg void OnRemove();
//protected:
	afx_msg void OnUp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
#ifdef WIN32
	afx_msg long OnRightButton(WPARAM wParam, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()


protected:
	int m_Sel;
	BOOL Find(CFilter *pSearchFilter, const char* szSearch, BOOL bMatchCase, BOOL bWholeWord);
public:
	BOOL FindText(CFilter *pSearchFilter, const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, CStringArray *csaField);
	BOOL MoveItem(int src, int dest, BOOL insertBefore = TRUE );
	void UpdateName();
	void NewItem( CFilter *pFilter);
	void DoOnRemove(); // Public interface to OnRemove()
	virtual BOOL DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);

};


/////////////////////////////////////////////////////////////////////////////
// 
class CFiltItem
{
	UINT	m_CommandID;
	UINT	m_ImageOffset;
	CString	m_ItemText;

public:
	CFiltItem() : m_CommandID(0), m_ImageOffset(0), m_ItemText("") {}
	virtual ~CFiltItem(){}
	
	CFiltItem(UINT commandID, LPCTSTR itemText);
	
	LPCTSTR GetItemText() const {return (LPCTSTR)m_ItemText; }
	UINT GetCommandID() const {return m_CommandID; }
	UINT GetImageOffset() const {return m_ImageOffset; }
};

/////////////////////////////////////////////////////////////////////////////
// OWNER DRAW Combobox
class CFiltComboBox : public CComboBox
{
public:
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
};

/////////////////////////////////////////////////////////////////////////////
// OWNER DRAW Menu
typedef CList<CFiltItem *, CFiltItem *> CFiltItemList;

class CFiltActionMenu : public CMenu
{
	CFilterImageList	m_FiltImageList;
public:
	CFiltActionMenu();
	void ModifyItems(CFiltItemList *p_FiltItemList);
	virtual ~CFiltActionMenu();
	
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
	int	DrawIcons( CDC *pDC, long& Left, long Top, BOOL bChecked, BOOL bSelected, CFiltItem *p_Item);
};

/////////////////////////////////////////////////////////////////////////////
// CFiltersViewRight form view

class CFiltersViewRight : public CFormView, public QICommandClient
{
protected:
	CFiltersViewRight();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFiltersViewRight)

// Form Data
public:
	//{{AFX_DATA(CFiltersViewRight)
	enum { IDD = IDR_FILTERS_RIGHT };
	CComboBox	m_Verb1Combo;
	CComboBox	m_Verb0Combo;
	CComboBox	m_Header1Combo;
	CComboBox	m_Header0Combo;
	CComboBox	m_AddBook1Combo;
	CComboBox	m_AddBook0Combo;
	CComboBox	m_ConjunctionCombo;
	int		m_Conjunction;
	CString	m_Header0;
	CString	m_Header1;
	CString	m_AddBook0;
	CString	m_AddBook1;
	BOOL	m_Incoming;
	BOOL	m_Manual;
	BOOL	m_Outgoing;
	CString	m_Value0;
	CString	m_Value1;
	int		m_Verb0;
	int		m_Verb1;
	CButton	m_Action1Button;
	int		m_Action0;
	int		m_Action1;
	int		m_Action2;
	int		m_Action3;
	int		m_Action4;
	//}}AFX_DATA
	CFiltComboBox	m_ActionCombo[NUM_FILT_ACTS];
	CFilterLabelButton m_LabelButton[NUM_FILT_ACTS];
	CMailboxButton	m_Mailbox[NUM_FILT_ACTS];
	CButton	m_SoundButton[NUM_FILT_ACTS];

	CEdit	m_FiltEdit[NUM_FILT_ACTS];
	CString	m_EditValue[NUM_FILT_ACTS];

	CButton m_LCheckBox[NUM_FILT_ACTS];
	BOOL	m_LCheckValue[NUM_FILT_ACTS];
	CButton m_RCheckBox[NUM_FILT_ACTS];
 	BOOL	m_RCheckValue[NUM_FILT_ACTS];

	CComboBox	m_PriorityCombo[NUM_FILT_ACTS];
	int			m_PriorityValue[NUM_FILT_ACTS];

	CComboBox	m_StatusCombo[NUM_FILT_ACTS];
	int			m_StatusValue[NUM_FILT_ACTS];

	CButton		m_AppBrz[NUM_FILT_ACTS];

	CComboBox	m_StationeryCombo[NUM_FILT_ACTS];
	CString		m_StationeryValue[NUM_FILT_ACTS];

	CComboBox	m_PersonaCombo[NUM_FILT_ACTS];
	CString		m_PersonaValue[NUM_FILT_ACTS];

	CComboBox	m_VoicesCombo[NUM_FILT_ACTS];
	int			m_VoicesValue[NUM_FILT_ACTS];
	bool		m_bHasSpeechEngine;

	bool		m_bHeader0IsJunkScore;
	bool		m_bHeader1IsJunkScore;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFiltersViewRight)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL


// Implementation
protected:
	virtual ~CFiltersViewRight();

    CFiltersDoc* GetDocument();

	void OnInitialUpdate();
	BOOL SetLabel(UINT nID);

	void DoRefresh();

	void RefreshVerbCombo(CComboBox *pCombo, bool bJunkScore);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CFiltersViewRight)
    afx_msg void OnDestroy();
	afx_msg void OnKillFocusCondition0();
	afx_msg void OnKillFocusValue0();
	afx_msg void OnKillFocusValue1();
	afx_msg void OnKillFocusValueCmb0();
	afx_msg void OnKillFocusValueCmb1();
	afx_msg void OnDropdownValueCmb0();
	afx_msg void OnDropdownValueCmb1();
	afx_msg void OnChangeVerb0();
	afx_msg void OnChangeVerb1();
	afx_msg void OnChangeHeader0();
	afx_msg void OnChangeHeader1();
	afx_msg void OnChange();
	afx_msg void OnLowerPriority();
	afx_msg void OnRaisePriority();
	afx_msg void OnTransfer();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDropdownActionCombo0();
	afx_msg void OnDropdownActionCombo1();
	afx_msg void OnDropdownActionCombo2();
	afx_msg void OnDropdownActionCombo3();
	afx_msg void OnDropdownActionCombo4();
	afx_msg void OnSelChangeCombo0();
	afx_msg void OnSelChangeCombo1();
	afx_msg void OnSelChangeCombo2();
	afx_msg void OnSelChangeCombo3();
	afx_msg void OnSelChangeCombo4();
	afx_msg void OnSelOKCombo0();
	afx_msg void OnSelOKCombo1();
	afx_msg void OnSelOKCombo2();
	afx_msg void OnSelOKCombo3();
	afx_msg void OnSelOKCombo4();
	afx_msg void OnDropdownPrio0();
	afx_msg void OnDropdownPrio1();
	afx_msg void OnDropdownPrio2();
	afx_msg void OnDropdownPrio3();
	afx_msg void OnDropdownPrio4();
	afx_msg void OnDropdownPersona0();
	afx_msg void OnDropdownPersona1();
	afx_msg void OnDropdownPersona2();
	afx_msg void OnDropdownPersona3();
	afx_msg void OnDropdownPersona4();
	afx_msg void OnMailboxSel0();
	afx_msg void OnMailboxSel1();
	afx_msg void OnMailboxSel2();
	afx_msg void OnMailboxSel3();
	afx_msg void OnMailboxSel4();
	afx_msg void OnLabelSel0();
	afx_msg void OnLabelSel1();
	afx_msg void OnLabelSel2();
	afx_msg void OnLabelSel3();
	afx_msg void OnLabelSel4();
	afx_msg void OnSoundSel0();
	afx_msg void OnSoundSel1();
	afx_msg void OnSoundSel2();
	afx_msg void OnSoundSel3();
	afx_msg void OnSoundSel4();
	afx_msg void OnAppSel0();
	afx_msg void OnAppSel1();
	afx_msg void OnAppSel2();
	afx_msg void OnAppSel3();
	afx_msg void OnAppSel4();

	//}}AFX_MSG
	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
#ifdef WIN32
	afx_msg long OnRightButton(WPARAM wParam, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()
protected:
	int m_Sel;
	CFilter* m_CurFilter;
	int m_ActionIndex;
	UINT m_ActionCommands[NUM_FILT_ACTS];
	CBitmap m_priorBM[5];

	void DropdownAction();
	void DropdownPriority();
	void DropdownPersona();
	void OnSelOKAction();
	void ValidChange(int index);
	void RemoveControl(UINT ActionID, int index);
	void DataEx(CDataExchange* pDX, UINT actionID, int index);
	void LoadFilter(CFilter* filt, UINT actionID, int index);
	BOOL DidFiltersChange(CFilter* filt, UINT actionID, int index);
	void SaveIntoFilter(CFilter* filt, UINT actionID, int index);
	void ClearAction(CFilter* filt, UINT actionID, int index);
	void ActivateAddressBookCombo(int nID, bool bActivate);


	void GetLargeRectPos(CRect &ctrlRect);
	CFont *GetSysFont(int whichFont = -1);

	void SetupEdit(const char* text);
	void SetupMailbox(const char* text);
	void SetupLabel(int sel);
	void SetupCheckboxes(LPCTSTR leftLabel, BOOL bIsLeftChecked, 
		 				LPCTSTR rightLabel, BOOL bIsRightChecked);
	void SetupAddressBookCombo(int nID);
	
	void SetupStatus();
	void SetupPriority();
	void SetupPersonaAction(const char* persona);
	void SetupSound();
	void SetupSpeak();
	void SetupNotifyApp();
	void SetupReplyWith(const char* text);

	void OnMailbox();
	void OnLabel();
	void OnSound();
	void OnApp();
	virtual void Notify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData); 
protected:
	CFiltItemList	m_FiltItemList;
public:
	CFiltActionMenu m_ActionMenu;
};

/////////////////////////////////////////////////////////////////////////////
void StripAccel(CString &str);




/////////////////////////////////////////////////////////////////////////////

#endif
