
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
//  Authors:		Dean Hallman & Scot Wingo
//  Description:	SECWorkspaceManager declaration
//
// Change History:
// 2/24/97 Mark Isham - Restructured architecture for better extensibility
//                      and SECDockState tracking

#ifdef WIN32

#ifndef __SECWSMGR_H__
#define __SECWSMGR_H__

#include "lstbxedt.h"
class SECToolBarManager;

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

#define SEC_WORKSPACE_MGR_WORKSPACE_LIST	_T("Workspace List\\")
#define SEC_WORKSPACE_MGR_WORKSPACES   		_T("Workspaces\\")
#define SEC_WORKSPACE_MGR_WORKSPACES_COUNT  _T("worspace_count")
#define SEC_WORKSPACE_MGR_WORKSPACES_PREFIX _T("worspace_")


/////////////////////////////////////////////////////////////////////////////
// SECWorkspaceState

class SECWorkspaceState : public CObject  {

    DECLARE_DYNAMIC(SECWorkspaceState);

public:
    // Construction
    SECWorkspaceState();

public:
	void EmptyWindowInfoList();

	// Override these to implement your own storage medium/protocol.
	virtual BOOL SaveState(CString strBaseRegKey,CString strWorkspaceName);
	virtual BOOL LoadState(CString strBaseRegKey,CString strWorkspaceName);
	virtual void Serialize(CArchive& ar,CString strWorkspaceName);

	virtual BOOL RenameState(CString strBaseRegKey,CString strOldName,CString strNewName);
	virtual BOOL DeleteState(CString strBaseRegKey,CString strWorkspaceName);

   // Implementation    
public:
	CObList m_WindowInfoList;
	CDockState* m_pDockState;
	CString m_strStateName;
	SECControlBarManager* m_pManager;

protected:
  
// Implementation
public:
	virtual ~SECWorkspaceState();

protected:
};


/////////////////////////////////////////////////////////////////////////////
// SECWindowInfo

// Window type specifiers - use for bitwise comparisons
#define SEC_WININFO_TYPE_SEC				0x0001
#define SEC_WININFO_TYPE_TOP_LEVEL_FRAME	0x0002
#define SEC_WININFO_TYPE_MDI_FRAME			0x0004

// Registry keywords for use by SECWindowInfo
#define SEC_WININFO_REG_PATHNAME	_T("PathName")
#define SEC_WININFO_REG_DOCTYPE		_T("DocType")
#define SEC_WININFO_REG_WINTITLE	_T("WindowTitle")
#define SEC_WININFO_REG_TYPE		_T("Type")
#define SEC_WININFO_REG_WNDPLACE	_T("WindowPlacement")

class SECWindowInfo : public CObject {

    DECLARE_DYNAMIC(SECWindowInfo);
public:

    // Construction
    SECWindowInfo();
	virtual ~SECWindowInfo();
	virtual void SetWindowType(CWnd* pWnd);

	// override these if you have any additional information to load/store		
	virtual void SaveToRegistry(SECRegistry& rRegKey);
	virtual void LoadFromRegistry(SECRegistry& rRegKey);
	virtual void Serialize(CArchive& ar);

    // Attributes
public:
	CString m_strPathName;
	CString m_strDocType;
	CString m_strWindowTitle;
	DWORD m_dwType;
	WINDOWPLACEMENT m_wp;
};

/////////////////////////////////////////////////////////////////////////////
// SECWorkspaceManager

class SECWorkspaceManager : public CCmdTarget {

    DECLARE_DYNAMIC(SECWorkspaceManager);

public:
    // Construction
    
    SECWorkspaceManager();

	// It is recommended that strWorkspaceMgrKey be unique for different
	// applications using the SECWorkspaceMgr, otherwise invalid window
	// configurations may result.
	BOOL Create(CWinApp* pWinApp,CString strWorkspaceMgrKey);
	BOOL Create(CString strWorkspaceMgrKey);

    // Attributes
public:
    CString  GetActiveWorkspace() const;
    POSITION GetFirstWorkspacePosition() const;
    CString  GetNextWorkspace(POSITION& pos) const;
    int      GetWindowCount() const;
    POSITION GetFirstWindowPosition() const;
    CWnd*    GetNextWindow(POSITION& pos) const;
    BOOL     DoesWorkspaceExist(CString strTestWorkspace);
    
    // Operations
public:
    void AddWindow(CWnd* pWnd);
    void RemoveWindow(CWnd* pWnd);
    void CloseAllWindows();
	void SetActiveWorkspace(CString& strActiveWorkspace);
    
    // Overridables (operations too)
public:
	virtual void OpenWorkspace(CString strWorkspaceName);
	virtual void SaveWorkspace(CString strWorkspaceName);
    virtual BOOL RenameWorkspace(CString strOldName,CString strNewName);
    virtual BOOL CreateNewWorkspace(CString strWorkspaceName);
    virtual BOOL DeleteWorkspace(CString strWorkspaceName);
    virtual void CloseActiveWorkspace();
    virtual void ReadWorkspaceList();
	virtual void RefreshWorkspaceListInReg();

protected: 
	// Overridable implementation

	// Called by workspace manager on the onset of a workspace state
	// operation. These are the entry points to all major state operations.
	// Override these to implement your own workspace protocol 
	// and/or storage medium.
    virtual BOOL OnWorkspaceStateLoad(SECWorkspaceState& rState,
									  CString strWorkspaceName);
    virtual BOOL OnWorkspaceStateSave(SECWorkspaceState& rState,
									  CString strWorkspaceName);
	virtual BOOL OnWorkspaceStateRename(SECWorkspaceState& rState,
									  CString strOldWorkspaceName,
									  CString strNewWorkspaceName);
	virtual BOOL OnWorkspaceStateDelete(SECWorkspaceState& rState,
									  CString strWorkspaceName);


	// These are called when the workspace state is to be loaded/saved.
	// Override *both* to perform your own custom workspace state 
	// implementation.
	virtual BOOL GetWorkspaceState(SECWorkspaceState& rState);
 	virtual BOOL SetWorkspaceState(SECWorkspaceState& rState);

	// Open document with specified path name
	virtual CDocument* OnOpenDocumentFile(CDocTemplate* pDocTempl,CString strPathName);
	virtual BOOL OnPromptSaveAsIfNecessary(CString& strPathName,CFrameWnd* pWnd,CDocument* pDoc);

	// Called immediately before a window will be set to the
	// workspace or loaded from the workspace. Override and return
	// FALSE to veto 1 or more windows.
	virtual BOOL OnSetWorkspaceStateWnd(SECWindowInfo* pWI);
	virtual BOOL OnGetWorkspaceStateWnd(CWnd* pWnd);

	// These are called just before SECDockState load/save.
	// Override and return FALSE to perform custom SECDockState load/save.
	virtual BOOL OnSetWorkspaceStateDockState();
	virtual BOOL OnGetWorkspaceStateDockState();

   // Implementation    
public:
    CStringList m_strListWorkspaces;
    CString m_strActiveWorkspace;
    CObList m_obListWindows;
    CWinApp * m_pWinApp;
	CString m_strWorkspaceRegBase;
	CString m_strWorkspaceList;
	CString m_strWorkspaceStorage;

protected:
	// return registry key of workspace list
	virtual CString GetWorkspaceListKey() { return m_strWorkspaceList; }
	virtual CString GetWorkspaceDataKey() { return m_strWorkspaceStorage; }
	virtual CString GetRegistryKey() { return GetWorkspaceListKey(); } // back compatible

protected:
	// Frame window for control bar state save/load
    SECFrameWnd* m_pFrameWnd;
    SECMDIFrameWnd* m_pMDIFrameWnd;

// Implementation
public:
	virtual ~SECWorkspaceManager();

protected:
	// Generated message map functions
	//{{AFX_MSG(SECWorkspaceManager)
	afx_msg BOOL OnWorkspace(UINT nID);
	afx_msg void OnUpdateWorkspaceMenu(CCmdUI* pCmdUI);
	afx_msg void OnWorkspaceNew();
	afx_msg void OnWorkspaceOpen();
	afx_msg void OnWorkspaceSaveAs();
	afx_msg void OnWorkspaceDelete();
	afx_msg void OnWorkspaceClose();
	afx_msg void OnWorkspaceSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// SECWorkspaceListBoxEditor
class SECWorkspaceManagerDialog;

class SECWorkspaceListBoxEditor : public SECListBoxEditor
{
public:
	SECWorkspaceListBoxEditor(SECWorkspaceManager* pWM,SECWorkspaceManagerDialog* pWMD);

protected:
	// virtual overrides
	virtual void OnEditingJustStarted();
	virtual void OnEditingStopped();
	virtual void OnItemRenamed(int nIndexRenamed,CString strOldName,CString NewName);
	virtual void OnItemAdded(int nIndexNewItem,CString strTextInserted);
	virtual void OnItemMoved(int nOldIndex,int nNewIndex,CString strTextMoved,BOOL WasDragged);
	virtual BOOL OnItemDelete(int nIndexToBeDeleted,CString strTextDeleted);

	// implementation
	BOOL ReplaceListBoxItem(CListBox* pLB,int iIndex,CString strNewText,DWORD dwItemData=0);
	SECWorkspaceManager* m_pWM;
	SECWorkspaceManagerDialog* m_pWMD;
};


/////////////////////////////////////////////////////////////////////////////
// SECWorkspaceManagerDialog

class SECWorkspaceManagerDialog : public CDialog
{
// Construction
public:
	SECWorkspaceManagerDialog(SECWorkspaceManager* pWSM,CWnd* pParent = NULL);
	~SECWorkspaceManagerDialog();
    BOOL UpdateWorkspaceRenameMap(CString strOldName,CString strNewName);
	void AddWorkspaceToKillList(CString strWorkspaceName);

// Dialog Data
	//{{AFX_DATA(SECWorkspaceManagerDialog)
	enum { IDD = IDD_OPEN_WORKSPACE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{Afx_VIRTUAL(SECWorkspaceManagerDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	SECWorkspaceListBoxEditor* m_pLBEditor;
    SECWorkspaceManager* m_pWSM;

	// workspace rename map
	void InitializeWorkspaceRenameMap();
	void ApplyWorkspaceRenameMap();
	CMapStringToString m_strWorkspaceRenameMap;
	CStringList m_strWorkspacesDeleted;

	void ApplySettings();

	// Generated message map functions
	//{{AFX_MSG(SECWorkspaceManagerDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenWorkspace();
	afx_msg void OnSaveWorkspace();
	afx_msg void OnDone();
	afx_msg void OnCancel();
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

#endif // __SECWSMGR_H__

#endif // WIN32