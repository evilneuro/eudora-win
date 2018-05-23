// MAINFRM.H
//
// Copyright (c) 1994-2001 by QUALCOMM, Incorporated
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


#ifndef	__MAINFRAME_H__
#define	__MAINFRAME_H__

#include "QICommandClient.h"
#include "statbar.h"
#include "WazooBarMgr.h"
#include "workbook.h"
#include "sbarmgr.h"
#include "PersonalityView.h"
#include "DynamicMailboxMenu.h"
#include "DynamicPersonalityMenu.h"
#include "DynamicPluginMenu.h"
#include "DynamicRecipientMenu.h"
#include "DynamicSavedSearchMenu.h"
#include "DynamicStationeryMenu.h"

//#include "plist_mgr.h"

// This is for the kerberos app "kerb16.exe"
#define LOAD_LIBRARY	51
#define FREE_LIBRARY	52
#define SET_USERNAME	53
#define GET_TICKET		54
#define	DELETE_TICKETS	55
#define	KERB_DLL_MODE	56
#define	SEND_TICKET		57
#define FAILED_SEND_TICKET 58

// Notification messages
#define WM_TRAY_CALLBACK		WM_USER+299
#define WM_USER_CHECKMAIL		WM_USER+300
#define WM_USER_REGISTER_MAILTO WM_USER+301
#define WM_USER_MCI_NOTIFY_MODE WM_USER+302

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


#define MAX_TOOLBAR_ADS		50


class QCCustomToolBar;
class DirectoryServicesWazooWndNew;
class CSearchBar;


class CAdInfo : public CObject
{
public:
	CAdInfo(UINT iCmd, CBitmap* pBmp, CBitmap* pBmpAlt, CString& strID, CString& strAdURL, CString& strAdTitle);
	virtual ~CAdInfo();

	UINT			 m_iCmd;
	CBitmap			*m_pBmp;
	CBitmap			*m_pBmpAlt;
	CString			 m_strURL;
	CString			 m_strID;
	CString			 m_strTitle;
};

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
class QCImapMailboxCommand;

extern enum COMMAND_ACTION_TYPE;
//extern UINT umsgRefreshTaskStatusPane;

class CMainFrame : public QCWorkbook, QICommandClient
{
	friend class CMainFrameOleDropTarget;			// mi casa es su casa
	friend class QComApplication;
	void FixMenuDrawingIfXP();
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
	static const UINT			kGeneralTimerPeriod = 1000;

	CMainFrame();
	bool	FinishInitAndShowWindow(int nWindowState, CWnd * pAboutDlg);

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
	DirectoryServicesWazooWndNew *m_pDirectoryServicesWazooWndNew;

	void Notify(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction, void* pData = NULL);

// Attributes
public:
	BOOL	m_bStartedIconic;

// Operations
public:
	BOOL LoadOpenWindows();
	BOOL SaveOpenWindows(BOOL Close);
	void SaveBarState(LPCTSTR lpszProfileName) const;
	CRect GetMainWindowPosFromIni();
	void GenerateNewName(const char* strName, char* szNewName);
	void ResetDSResize();

	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext*);

	void SetGeneralTimer(UINT nElapse = kGeneralTimerPeriod);
	bool CouldSetGeneralTimer() const { return (m_timerID != 0); }
	void SetIcon(int HaveMailStatus=-1, int msgCount =-1, BOOL bForceChange = FALSE);
	UINT GetTopWindowID();
	CWazooWnd* GetWazooWindowWithFocus();
	void SetActiveWazooWindows();
	void EnableDocking(DWORD dwDockStyle);
	void GetRealClientRect(LPRECT lpRect);
	void DragAcceptFiles(BOOL bAccept = TRUE);

	CEdit* GetCurrentEdit();
	CRichEditCtrl* GetCurrentRichEdit();
	BOOL CanModifyEdit();
	BOOL GetSelectedText(CString& SelText);

	BOOL IsToolbarVisible() const;
	BOOL IsSearchbarVisible() const;
	BOOL IsStatbarVisible() const;

	void ChangeSponsor(CString& strAdID, CString& strButtonAdFile, CString& strAdURL, CString& strAdTitle);
	void ResetSponsor();

	void UpdatePlaylists( bool bUpdateNow = false );

	void ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay);
	void RemoveBogusAdToolBars	();
	void AddAdToolBar();
	void RemoveAdToolBar();
	void RemoveAdToolBarFromItsDockBar(CControlBar * pAdToolBar);
	BOOL ToolBarAdAlreadyShowing(CString& strAdID);
	void ShowToolBarAd(CString& strAdID, CString& strButtonAdFile, CString& strAdURL, CString& strAdTitle);
	void HideToolBarAd(CString& strAdID);
	void RefreshAdToolBar();
	void SwitchAdToolBarImages();
	BOOL IsAdToolBar(QCCustomToolBar *pToolBar);
	BOOL IsToolBarAd(int nCmd);
	void GetToolBarAdToolTip(int nCmd, CString& strToolTip);
	void ToolBarClosed(QCCustomToolBar *pToolBar);
	void ToolBarDeleted(QCCustomToolBar *pToolBar);
	void ToolBarButtonDeleted(QCCustomToolBar *pToolBar, int nIndex);
	void FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);

	void InitJunkMenus();

	CCompMessageDoc* HuntForTopmostCompMessage(CWazooWnd* pWazooWnd);

	static CMainFrame *QCGetMainFrame();

	// Notifies various folks that a connection is happenin', dude
	afx_msg void OnConnectionWasMade();

	HMENU GetFileMenu()										{ return m_hFileMenu; }
	HMENU GetEditMenu()										{ return m_hEditMenu; }
	CDynamicRecipientMenu* GetEditInsertRecipientMenu()		{ return m_pEditInsertRecipientMenu; }
	CDynamicPluginMenu* GetEditMessagePluginsMenu()			{ return m_pEditMessagePluginsMenu; }
	CDynamicMailboxMenu* GetMailboxMenu()					{ return m_pMailboxMenu; }
	HMENU GetMessageMenu()									{ return m_hMessageMenu; }
	CDynamicRecipientMenu* GetMessageNewMessageToMenu()		{ return m_pMessageNewMessageToMenu; }
	CDynamicRecipientMenu* GetMessageForwardToMenu()		{ return m_pMessageForwardToMenu; }
	CDynamicRecipientMenu* GetMessageRedirectToMenu()		{ return m_pMessageRedirectToMenu; }
	CDynamicStationeryMenu* GetMessageNewMessageWithMenu()	{ return m_pMessageNewMessageWithMenu; }
	CDynamicStationeryMenu* GetMessageReplyWithMenu()		{ return m_pMessageReplyWithMenu; }
	CDynamicStationeryMenu* GetMessageReplyToAllWithMenu()	{ return m_pMessageReplyToAllWithMenu; }
	CDynamicPluginMenu* GetMessageAttachMenu()				{ return m_pMessageAttachMenu; }
	CDynamicPersonalityMenu* GetMessageChangePersonalityMenu()	{ return m_pMessageChangePersonalityMenu; }
	HMENU GetMessageChangeLabelMenu()						{ return m_hMessageChangeLabelMenu; }
	CDynamicMailboxMenu* GetTransferMenu()					{ return m_pTransferMenu; }
	HMENU GetSpecialMenu()									{ return m_hSpecialMenu; }
	CDynamicRecipientMenu* GetSpecialRemoveRecipientMenu()	{ return m_pSpecialRemoveRecipientMenu; }
	CDynamicSavedSearchMenu* GetSavedSearchMenu()			{ return m_pSavedSearchMenu; }
	HMENU GetToolsMenu()									{ return m_hToolsMenu; }
	HMENU GetWindowMenu()									{ return m_hWindowMenu; }
	HMENU GetHelpMenu()										{ return m_hHelpMenu; }
	
	void SetTransferMenuBeingUsed(BOOL bSet) { m_bTransferMenuBeingUsed = bSet; }

protected:
	// Can't save some of these as CMenus because they are temporaries, and
	// it's a really bad idea to hold on to a pointer to a temporary object
	HMENU GetTopMenuByPosition(int nPos);

	HMENU						m_hFileMenu;
	HMENU						m_hEditMenu;
	CDynamicRecipientMenu*		m_pEditInsertRecipientMenu;
	CDynamicPluginMenu*			m_pEditMessagePluginsMenu;
	CDynamicMailboxMenu*		m_pMailboxMenu;
	HMENU						m_hMessageMenu;
	CDynamicRecipientMenu*		m_pMessageNewMessageToMenu;
	CDynamicRecipientMenu*		m_pMessageForwardToMenu;
	CDynamicRecipientMenu*		m_pMessageRedirectToMenu;
	CDynamicStationeryMenu*		m_pMessageNewMessageWithMenu;
	CDynamicStationeryMenu*		m_pMessageReplyWithMenu;
	CDynamicStationeryMenu*		m_pMessageReplyToAllWithMenu;
	CDynamicPluginMenu*			m_pMessageAttachMenu;
	CDynamicPersonalityMenu*	m_pMessageChangePersonalityMenu;
	HMENU						m_hMessageChangeLabelMenu;
	CDynamicMailboxMenu*		m_pTransferMenu;
	HMENU						m_hSpecialMenu;
	CDynamicRecipientMenu*		m_pSpecialRemoveRecipientMenu;
	CDynamicSavedSearchMenu*	m_pSavedSearchMenu;
	HMENU						m_hToolsMenu;
	HMENU						m_hWindowMenu;
	HMENU						m_hHelpMenu;

	BOOL m_bTransferMenuBeingUsed;

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
	afx_msg void OnShowSearchBar();
	afx_msg void OnShowStatusBar();
	afx_msg void OnUpdateOvrIndicator(CCmdUI* pCmdUI);
	afx_msg void OnCustomizeLink();
	//	void OnProperties();

protected:
	static CMainFrame *m_pTheMainFrame;

	CStatusBarEx		m_wndStatusBar;
	QCCustomToolBar*	m_pToolBar;
	CSearchBar *		m_pSearchBar;
	BOOL				m_bFlushBars;

	SECCustomToolBar	*m_pToolBarAd;
	CObArray			 m_arrayToolBarAds;
	CBitmap				*m_pBitmapToolBar;
	int					 m_iAdsShown;
	CAdInfo				*m_padSponsor;
	int					 m_nSponsorBitmapWidth;
	int					 m_nSponsorBitmapHeight;

	virtual void LogoClicked();
	virtual CString GetLogoMessageText();
	virtual void OnDrawBorder(CDC* pDC);

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

	HICON	m_hIcon;
	UINT	m_IconID;
	UINT	m_timerID;
	UINT	m_idAutoActivateTimer;	// for Auto-Wazoo drag and drop auto-activation
	int		m_iNewMsgs;				// Counter for Win95 status pane

	long	m_ldb;   // playlist db handle

public:
	long	GetLDB() {return m_ldb;}
	UINT	GetCurrentIconID() {return m_IconID;}
	UINT	GetGeneralTimerID() { return GENERAL_BACKGROUND_TIMER_ID; }
	
// Generated message map functions
protected:

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	afx_msg void OnCtrlJ();
	afx_msg void OnShiftCtrlJ();
	afx_msg void OnShiftCtrlL();

	afx_msg void OnViewStationery();
	afx_msg void OnViewFilterReport();
	afx_msg void OnViewFileBrowser();
	afx_msg void OnViewLookup();
	afx_msg void OnViewLinkHistory();
	afx_msg void OnViewPersonalities();
	afx_msg void OnViewTaskStatus();
	afx_msg void OnViewTaskError();
	afx_msg void OnViewUsageStatistics();

	afx_msg void OnResetTools();

	//{{AFX_MSG(CMainFrame)
	afx_msg void OnCtrlF();
	afx_msg void OnShiftCtrlF();
	afx_msg void OnEditFindFindMsg();
	afx_msg void OnEditFindFindText();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnTimeChange();
	afx_msg void OnFileCheckMail();
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToggleOffline(CCmdUI* pCmdUI);
	afx_msg void OnFileToggleOffline();
	afx_msg void OnUpdateEditFindFindMsg(CCmdUI* pCmdUI);
	afx_msg void OnEditFindFindTextAgain();
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSelEdit(CCmdUI* pCmdUI);
	afx_msg void OnMessageNewMessage();
	afx_msg void OnMessageReplyCtrlR();
	afx_msg void OnUpdateSpecialEmptyTrash(CCmdUI* pCmdUI);
	afx_msg void OnSpecialEmptyTrash();
	afx_msg void OnUpdateTrimJunk(CCmdUI* pCmdUI);
	afx_msg void OnSpecialTrimJunk();
	afx_msg void OnSpecialCompactMailboxes();
	afx_msg void OnSpecialSettings();
	afx_msg void OnSpecialNewAccount() ;
	afx_msg void OnSpecialChangePassword();
	afx_msg void OnSpecialForgetPassword();
	afx_msg void OnWindowFilters();
	afx_msg void OnWindowMailboxes();
	afx_msg void OnWindowNicknames();
	afx_msg void OnHelpSearchForHelpOn();
	afx_msg void OnToolBarAd(UINT uCmdID);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateSpecialForgetPassword(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
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
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnPaymentAndRegistration();
	afx_msg void OnUpdatePaymentAndRegistration(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatistics(CCmdUI* pCmdUI);
	afx_msg void OnOpenEudora();
	afx_msg void OnMinimizeToSystemTray();
	afx_msg void OnUpdateMinimizeToSystemTray(CCmdUI* pCmdUI);
	afx_msg void OnAlwaysShowTrayIcon();
	afx_msg void OnUpdateAlwaysShowTrayIcon(CCmdUI* pCmdUI);
	//}}AFX_MSG
	
	// Function prototypes not generated by ClassWizard
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateMailbox(CCmdUI* pCmdUI);
	afx_msg BOOL OnWindowSignature(UINT nID);	
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
	void NewMessage(LPCTSTR To, LPCTSTR From, LPCTSTR Subject, LPCTSTR Cc,
					LPCTSTR Bcc, LPCTSTR Attach, LPCTSTR Body);
	void CreateMessage(LPCTSTR cmdLine);
	void GetAttachmentLine(LPCTSTR cmdLine, CString& csAttachLine);

	LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);

	// The following non-ClassWiz handlers are normally protected,
	// but are made public here so that the CMboxTreeCtrl can
	// access them as if it was an alternate menu.
	afx_msg void CheckSpelling();

	afx_msg BOOL OnDynamicCommand(UINT uID);
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	afx_msg BOOL OnSelectedTextURLCommand(UINT nID);

	afx_msg void OnViewSignatures();

	afx_msg void OnNewMailboxInRoot();
	afx_msg void OnTransferNewMailboxInRoot();

	afx_msg void OnMailboxIn();
	afx_msg void OnMailboxOut();
	afx_msg void OnMailboxTrash();
	afx_msg void OnMailboxJunk();

// IMAP4:
	afx_msg void OnImapResyncOpened() ;
// End IMAP4

	afx_msg void OnIdleUpdateCmdUI();
	static int PlaylistCallback( int cmd, long userRef, long lParam );

	afx_msg LRESULT OnMCINotifyMode(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserMCINotifyMode(WPARAM wParam, LPARAM lParam);

#ifdef _DEBUG
	afx_msg void OnSharewareChangeStateLight();
	afx_msg void OnUpdateSharewareChangeStateLight(CCmdUI* pCmdUI);
	afx_msg void OnSharewareChangeStatePro();
	afx_msg void OnUpdateSharewareChangeStatePro(CCmdUI* pCmdUI);
	afx_msg void OnSharewareChangeStateAdware();
	afx_msg void OnUpdateSharewareChangeStateAdware(CCmdUI* pCmdUI);
	afx_msg void OnRequestBrandNewPlaylist();
	afx_msg void OnUpdateRequestBrandNewPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnRequestNewPlaylist();
	afx_msg void OnUpdateRequestNewPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnDumpAdState();
	afx_msg void OnUpdateDumpAdState(CCmdUI* pCmdUI);
	afx_msg void OnChangePlaylistServer(UINT uCmdID);
	afx_msg void OnUpdateChangePlaylistServer(CCmdUI* pCmdUI);

#endif // _DEBUG
	// factors out Notify events for SWMstate management
	virtual void SWMNotify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData); 
	static int GetAdFailure( );

protected:
	static void SetAdFailure( int days );
}; 

/////////////////////////////////////////////////////////////////////////////


inline UINT GetTopWindowID()
	{ return (((CMainFrame*)AfxGetMainWnd())->GetTopWindowID()); }

#endif	// __MAINFRAME_H__
