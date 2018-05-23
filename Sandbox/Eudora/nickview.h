// NICKVIEW.H
//
// "Left-hand" and "Right-hand" CFormView classes for the Address Book
// splitter window.
//

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CNicknamesViewRight form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CNicknamesDoc;

class CNicknamesViewRight : public CFormView
{
	DECLARE_DYNCREATE(CNicknamesViewRight)
protected:
	CNicknamesViewRight();			// protected constructor used by dynamic creation

// Form Data
public:
	//{{AFX_DATA(CNicknamesViewRight)
	//}}AFX_DATA

// Attributes
public:
	CNicknamesPropSheet	m_PropSheet;

// Operations
public:
    CNicknamesDoc* GetDocument();
	void ClearAllControls();
	void EnableAllControls(BOOL bEnable);

	void ResizeCurrentPropertyPage(const CRect& pageRect);

	//FORNOWvoid OnToggleRecipient();
	//FORNOWvoid ToggleRecipientFlag(const char *name);
	
// Implementation
protected: 
	~CNicknamesViewRight();
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	// Generated message map functions
	//{{AFX_MSG(CNicknamesViewRight)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//FORNOWCEdit* GetCurrentEdit();
	//FORNOWvoid ChangesMade();
	
	//FORNOWCNickname* m_CurrentNickname;	// Last nickname with focus
	//FORNOWint m_CurrentNicknameIndex;		// Index of last nickname with focus
	//FORNOWint m_OldWidth, m_OldHeight;	// Previous width and height of window

private:
	BOOL m_bIsInitialized;
};


////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft form view
//
// This is the platform-independent base class which handles the common
// behaviors for the derived 16-bit and 32-bit specific implementations.
// This is not instantiated directly.
//

class CNicknamesViewLeft : public CFormView
{
	DECLARE_DYNAMIC(CNicknamesViewLeft)

protected:
	CNicknamesViewLeft(UINT uTemplateID);   // protected constructor used by dynamic creation

// Form Data
public:
	//{{AFX_DATA(CNicknamesViewLeft)
	CButton				m_DelButton;
	CButton				m_GrowButton;
	CButton				m_NewButton;
	CButton				m_ToButton;
	CButton				m_CcButton;
	CButton				m_BccButton;
	CButton				m_ExpandCheckBox;
	CComboBox			m_ViewByCombo;
	CStatic				m_ViewByComboLabel;
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void AutoClickNewButton();
	void AutoClickRemoveButton();

	void OnInsertNickname(int HeaderNum);
	void MoveOrCopyNicknames(BOOL bIsMove, CObList& nicknameList, CNicknameFile* pTargetNicknameFile = NULL);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesViewLeft)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CNicknamesViewLeft();
    CNicknamesDoc* GetDocument();

	// Generated message map functions
	//{{AFX_MSG(CNicknamesViewLeft)
	afx_msg void OnNewClicked();
	afx_msg void OnDelClicked();
	afx_msg void OnGrowClicked();
	afx_msg void OnToClicked();
	afx_msg void OnCcClicked();
	afx_msg void OnBccClicked();
	afx_msg void OnViewBySelchange();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnMakeNickname();
	afx_msg void OnUpdateMakeNickname(CCmdUI* pCmdUI);
	afx_msg void OnCmdDblClkNickname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual BOOL GetSelectedNicknames(CObList& nicknameList, BOOL bGetNicknameFiles = FALSE) = 0;
	virtual CNicknameFile* FindSingleSelectedNicknameFile(void) = 0;
	virtual void RefreshRecipListDisplay(void) = 0;
	virtual void RemoveSelectedItems(void) = 0;
	virtual void SetViewByMode(int nIndex, const char* pFieldName) = 0;
	virtual BOOL DeleteExistingNickname(CNicknameFile* pNicknameFile, CNickname* pNickname, BOOL bLastOne = TRUE) = 0;
	char* AppendSelectedNicknames(char* pNicknames);
};


/////////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft32 form view

class CNicknamesViewLeft32 : public CNicknamesViewLeft
{
protected:
	CNicknamesViewLeft32();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CNicknamesViewLeft32)

// Form Data
public:
	//{{AFX_DATA(CNicknamesViewLeft32)
	enum { IDD = IDR_NICKNAMES_LEFT32 };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesViewLeft32)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CNicknamesViewLeft32();
	virtual BOOL GetSelectedNicknames(CObList& nicknameList, BOOL bGetNicknameFiles = FALSE);
	virtual CNicknameFile* FindSingleSelectedNicknameFile(void);
	virtual void RefreshRecipListDisplay(void);
	virtual void RemoveSelectedItems(void);
	virtual void SetViewByMode(int nIndex, const char* pFieldName);
	virtual BOOL DeleteExistingNickname(CNicknameFile* pNicknameFile, CNickname* pNickname, BOOL bLastOne = TRUE);
	virtual void OnEnterKeyPressed();
	virtual void OnEscapeKeyPressed();

	void DoRefresh();

	// Generated message map functions
	//{{AFX_MSG(CNicknamesViewLeft32)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg long OnActivateWazoo(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CNicknamesTreeCtrl m_NicknamesTree;
	BOOL m_bTreeCtrlInitialized;		// for Just In Time display initialization
};

