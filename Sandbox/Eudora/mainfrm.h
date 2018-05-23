// MAINFRM.H
//
/////////////////////////////////////////////////////////////////////////////

 
#ifndef	__MAINFRAME_H__
#define	__MAINFRAME_H__

#include "QICommandClient.h"
#include "statbar.h"
#include "WazooBarMgr.h"
#include "workbook.h"
#include "sbarmgr.h"
#include "PersonalityView.h"

// This is for the kerberos app "kerb16.exe"
#define LOAD_LIBRARY	51
#define FREE_LIBRARY	52
#define SET_USERNAME	53
#define GET_TICKET		54
#define	DELETE_TICKETS	55
#define	KERB_DLL_MODE	56
#define	SEND_TICKET		57
#define FAILED_SEND_TICKET 58

//Taskbar notification message
#define WM_TRAY_CALLBACK		WM_USER+200
#define WM_USER_CHECKMAIL		WM_USER+300
#define WM_USER_REGISTER_MAILTO WM_USER+301

#ifdef EXPIRING
#define WM_USER_EVAL_EXPIRED	WM_USER+302
#endif // EXPIRING

#define WM_USER_AUTOMATION_MSG  WM_USER+303
#define WM_USER_QUERY_PALETTE   WM_USER+304

// IDs for the toolbars
#define IDW_MAIN_TOOLBAR	AFX_IDW_TOOLBAR
#define IDW_STYLED_TEXT_TOOLBAR	IDW_MAIN_TOOLBAR + 1


// main menu entries -- pass to GetTopMenuByPosition()
#define MAIN_MENU_FILE		0
#define MAIN_MENU_EDIT		1
#define MAIN_MENU_MAILBOX	2
#define MAIN_MENU_MESSAGE	3
#define MAIN_MENU_TRANSFER	4
#define MAIN_MENU_SPECIAL	5
#define MAIN_MENU_TOOLS		6
#define MAIN_MENU_WINDOW	7
#define MAIN_MENU_HELP		8

class QCCustomToolBar;
class DirectoryServicesWazooWndNew;

///////////////////////////////////////////////////////////////////////
// CMainFrameOleDropTarget
//
// Private class that overrides CView-based implementation of 
// COleDropTarget to work with the CMainFrame.
///////////////////////////////////////////////////////////////////////
class CMainFrameOleDropTarget : public COleDropTarget
{
	friend class CMainFrame;		// private class

private:
	CMainFrameOleDropTarget()
	{
	}
	virtual ~CMainFrameOleDropTarget()
	{
	}

public:
	// Overridables
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
								   DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
								  DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
						DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
								DROPEFFECT dropDefault, DROPEFFECT dropList,
								CPoint point);
//	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);

private:
	CMainFrameOleDropTarget(const CMainFrameOleDropTarget&);	// not implemented
	void operator=(const CMainFrameOleDropTarget&);		// not implemented
};


class CTocView;
class CTocDoc;
class CMboxTreeCtrl;
class CFilterReportView;
class CLookupView; 
class CCompMessageDoc;
class QCCommandObject;
class QCMailboxCommand;
class QComApplication;
class CWazooWnd;
class CTaskStatusView;
class CWebBrowserView;
class QCImapMailboxCommand;

extern enum COMMAND_ACTION_TYPE;
//extern UINT umsgRefreshTaskStatusPane;

class CMainFrame : public QCWorkbook, QICommandClient
{
	friend class CMainFrameOleDropTarget;			// mi casa es su casa
	friend class QComApplication;
	void InitMenus();

	void HandleMailboxNotification(
	CMenu*				pMenu,
	COMMAND_ACTION_TYPE theNewAction,
	QCMailboxCommand*	pCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData);

// IMAP4
	// May need to change the way an IMAP mbox is displayed - fro a menu item to
	// a submenu.
	//
	CMenu* IMAPRedisplayAsSubMenu (QCImapMailboxCommand *pImapCommand, LPCSTR pNamedPath,
								 CMenu* pMenu, int *i, COMMAND_ACTION_TYPE theNewAction);

// END IMAP4

	void AddMailboxMenuItem(
	UINT	uID,
	LPCSTR	szMenuItem,
	CMenu*	pMenu,
	CMenu*	pSubMenu,
	INT		iStartPos,
	INT		iEndPos,
	BOOL	bTransferMenu );

	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	CMboxTreeCtrl* m_pActiveMboxTreeCtrl;			//FORNOW, a total hack
	CMboxTreeCtrl* GetActiveMboxTreeCtrl();			//FORNOW, for backwards compatibility
	CFilterReportView* m_pActiveFilterReportView;	//FORNOW, a total hack
	CFilterReportView* GetActiveFilterReportView();	//FORNOW, a total hack
	CLookupView* m_pActiveLookupView;				//FORNOW, a total hack
//	CLookupView* GetActiveLookupView();				//FORNOW, a total hack
	CPersonalityView* m_pActivePersonalityView;		//FORNOW, a total hack
	CPersonalityView* GetActivePersonalityView();	//FORNOW, a total hack
	CTaskStatusView* m_pActiveTaskStatusView;		//FORNOW, a total hack
	CTaskStatusView* GetActiveTaskStatusView();		//FORNOW, a total hack
	CWebBrowserView* m_pActiveWebBrowserView;		//FORNOW, a total hack
	CWebBrowserView* GetActiveWebBrowserView();		//FORNOW, a total hack
	DirectoryServicesWazooWndNew *m_pDirectoryServicesWazooWndNew;

	void Notify(
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

// Attributes
public:
	BOOL	m_bStartedIconic;

// Operations
public:
	BOOL LoadOpenWindows();
	BOOL SaveOpenWindows(BOOL Close);
	void SaveBarState(LPCTSTR lpszProfileName) const;
	CRect GetMainWindowPosFromIni();
	void GenerateNewName(CString strName,char *szNewName);
        void ResetDSResize();

#ifndef COMMERCIAL
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext*);
#endif //COMMERCIAL

	void SetIcon(int HaveMailStatus=-1, int msgCount =-1);
	UINT GetTopWindowID();
	CWazooWnd* GetWazooWindowWithFocus();
	void SetActiveWazooWindows();
	void GetRealClientRect(LPRECT lpRect);

	CEdit* GetCurrentEdit();
	CRichEditCtrl* GetCurrentRichEdit();
	BOOL CanModifyEdit();

	BOOL IsToolbarVisible() const;
	BOOL IsStatbarVisible() const;

	CCompMessageDoc* HuntForTopmostCompMessage(CWazooWnd* pWazooWnd);

	static CMainFrame *QCGetMainFrame();

	CMenu *GetTopMenuByPosition(int nPos);

// Implementation
public:
	~CMainFrame();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	BOOL IsOnStatusBar(const CPoint& ptClient) const;
	BOOL SetStatusBarText(const char* szText);
	CStatusBarEx* GetStatusBar() {return &m_wndStatusBar;}
	//
	// Override of virtual functions from base class.
	//
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu);

#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg void OnShowToolBar();
	afx_msg void OnShowStatusBar();
	afx_msg void OnUpdateOvrIndicator(CCmdUI* pCmdUI);
	afx_msg void OnCustomizeLink();
	//	void OnProperties();

protected:
	static CMainFrame *m_pTheMainFrame;

	CStatusBarEx		m_wndStatusBar;
	QCCustomToolBar*	m_pToolBar;

	CWazooBarMgr	m_WazooBarMgr;
	
	void WriteToolBarMarkerToIni() const;

	bool m_bSwitchFindAccel;

protected:
	void SplitPOPACCOUNT() ; // splits POP_ACCOUNT into LOGIN_NAME and POP_SERVER

	void FlushIncompatibleToolBarState(void);

	enum
	{
		//
		// Hardcoded version numbers for determining state of toolbar
		// state info in the INI file.
		//
		COMMERCIAL_32_VERSION = 16,
		LIGHT_32_VERSION = 16,
		AUTOACTIVATE_TIMER_EVENT_ID = 1000,
		GENERAL_BACKGROUND_TIMER_ID = 1001
	};

	//
	// Registered object for OLE Drag and Drop callbacks.
	//
	CMainFrameOleDropTarget m_dropTarget;

	HICON			m_hIcon;
	UINT			m_IconID;
	UINT			m_timerID;
	UINT			m_idAutoActivateTimer;		// for Auto-Wazoo drag and drop auto-activation
	int				m_iNewMsgs;	// Counter for Win95 status pane

public:
	UINT	GetCurrentIconID() {return m_IconID;}
	UINT	GetGeneralTimerID() { return GENERAL_BACKGROUND_TIMER_ID; }
	
// Generated message map functions
protected:

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	afx_msg void OnViewStationery();
	afx_msg void OnViewFilterReport();
	afx_msg void OnViewFileBrowser();
	afx_msg void OnViewLookup();
	afx_msg void OnViewPersonalities();
	afx_msg void OnViewTaskStatus();
	afx_msg void OnViewTaskError();
	afx_msg void OnViewWebBrowser();

	//{{AFX_MSG(CMainFrame)
	afx_msg void OnCtrlF();
	afx_msg void OnShiftCtrlF();
	afx_msg void OnEditFindFindMsg();
	afx_msg void OnEditFindFindText();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnFileCheckMail();
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindMsg(CCmdUI* pCmdUI);
	afx_msg void OnEditFindFindTextAgain();
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSelEdit(CCmdUI* pCmdUI);
	afx_msg void OnMessageNewMessage();
	afx_msg void OnMessageReplyCtrlR();
	afx_msg void OnPurchaseInfo();
	afx_msg void OnSpecialEmptyTrash();
	afx_msg void OnSpecialCompactMailboxes();
	afx_msg void OnSpecialSettings();
	afx_msg void OnSpecialNewAccount() ;
	afx_msg void OnSpecialChangePassword();
	afx_msg void OnSpecialForgetPassword();
	afx_msg void OnWindowFilters();
	afx_msg void OnWindowMailboxes();
	afx_msg void OnWindowNicknames();
	afx_msg void OnHelpSearchForHelpOn();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateSpecialForgetPassword(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();
	afx_msg void OnSendQueuedMessages();
	afx_msg void OnUpdateSendQueuedMessages(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMessageReply(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMessageReplyAll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMessageSendImmediately(CCmdUI* pCmdUI);
	afx_msg void OnDisableMessageStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMessageLabel(CCmdUI* pCmdUI);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSendToBack();
	afx_msg void OnUpdateSendToBack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateNeedClip(CCmdUI* pCmdUI);
	afx_msg void OnEditPasteAsQuotation();
	afx_msg void OnEditClear();
	afx_msg void OnEditWrapSelection();
	afx_msg void OnUpdateEditFinishNickname(CCmdUI* pCmdUI);
	afx_msg void OnEditFinishNickname();
	afx_msg void OnSpecialAddAsRecipient();
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAboutEmsPlugins();
	afx_msg void OnUpdateAttach(CCmdUI* pCmdUI);
	afx_msg void OnAttachFile();
	afx_msg void OnProcessAllofThem();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	//}}AFX_MSG
	
	// Function prototypes not generated by ClassWizard
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateMailbox(CCmdUI* pCmdUI);
	afx_msg BOOL OnWindowSignature(UINT nID);	
	afx_msg	void OnDeleteSignature();
	afx_msg void OnNewSignature();
	afx_msg void OnImportMail();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnCloseAll();
	afx_msg void OnSaveAll();
	afx_msg void OnFakeAppExit();
	afx_msg void OnUpdateFakeAppExit(CCmdUI* pCmdUI);
	afx_msg LRESULT OnTrayCallback( WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUserCheckMail( WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRegisterMailto(WPARAM wParam, LPARAM lParam);

#ifdef EXPIRING
	afx_msg LRESULT OnUserEvalExpired(WPARAM wParam, LPARAM lParam);
#endif // EXPIRING

	afx_msg LRESULT OnUserAutomationMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowProgress(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditable(CCmdUI* pCmdUI);
	afx_msg void OnMoreHelp();
	afx_msg BOOL OnMailingList(UINT nID);
	afx_msg long OnRefreshTaskStatusPane(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	
	BOOL QuerySendQueuedMessages();
	
#ifdef OLDSTUFF	
	BOOL OnNewMessageTo(UINT nID);
	BOOL OnResponseTo(UINT nID, UINT RepsonseType);
#endif

	BOOL TrayItem( int iIcon, int iOperation);
	BOOL CloseDown();

	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);

	void OnHelp();
	void OnHelpTopics();
	void OnTechSupport();

public:
	// The following non-ClassWiz handlers are normally protected,
	// but are made public here so that the CMboxTreeCtrl can
	// access them as if it was an alternate menu.
	void NewMessage(char *to, char *from, char *sub, char *cc, 
					char *bcc, char *attach, char *body);				
	void CreateMessage(char* cmdLine);				
	void GetAttachmentLine(char* cmdLine, char *attachLine);				

	afx_msg void CheckSpelling();

	LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);

	afx_msg BOOL OnDynamicCommand(
	UINT uID );
	
	afx_msg void OnUpdateDynamicCommand(
	CCmdUI* pCmdUI);

	afx_msg void OnViewSignatures();

	afx_msg void OnNewMailboxInRoot();
	afx_msg void OnTransferNewMailboxInRoot();

	afx_msg void OnMailboxIn();
	afx_msg void OnMailboxOut();
	afx_msg void OnMailboxTrash();

	afx_msg void OnNewStationery();
	afx_msg void OnDeleteStationery();

// IMAP4:
	afx_msg void OnImapResyncOpened() ;
// End IMAP4

#ifdef _DEBUG
	afx_msg void OnSharewareChangeStateLight();
	afx_msg void OnUpdateSharewareChangeStateLight(CCmdUI* pCmdUI);
	afx_msg void OnSharewareChangeStatePro();
	afx_msg void OnUpdateSharewareChangeStatePro(CCmdUI* pCmdUI);
	afx_msg void OnSharewareChangeStateAdware();
	afx_msg void OnUpdateSharewareChangeStateAdware(CCmdUI* pCmdUI);
#endif // _DEBUG
	// factors out Notify events for SWMstate management
	virtual void SWMNotify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData); 

}; 

/////////////////////////////////////////////////////////////////////////////


inline UINT GetTopWindowID()
	{ return (((CMainFrame*)AfxGetMainWnd())->GetTopWindowID()); }

#endif	// __MAINFRAME_H__
