// TocFrame.h : header file
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#if !defined(AFX_TOCFRAME_H__CE6A8E92_1B2D_11D1_91DB_00805FD2B1FA__INCLUDED_)
#define AFX_TOCFRAME_H__CE6A8E92_1B2D_11D1_91DB_00805FD2B1FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "mdidrag.h"
#include "MsgSplitter.h"

class CSummary;

/////////////////////////////////////////////////////////////////////////////
// CTocFrame frame

// Forward declarations
class CTocDoc;
class CTocListBox;
class CTocView;

class CTocFrame : public CMDIChildTarget
{
	DWORD		m_dwStartPreviewTime;
	int			m_nTocListHeight;

	DECLARE_DYNCREATE(CTocFrame)
protected:
	CTocFrame();           // protected constructor used by dynamic creation


// Operations
public:
	typedef CArray<CSummary *, CSummary *>		SummaryArrayT;

	int GetLastSplitterPos() const { return m_nLastSplitterPos; }
	void RememberSplitterPos() { m_nLastSplitterPos = GetCurrentSplitterPos(); }
	
	const SummaryArrayT &		GetPreviewSummaryArray() { return m_bPreviewAllowed ? m_previewSummaryArray : m_emptySummaryArray; }
	CSummary *					GetSingleSelectionPreviewSummary() { return (m_bPreviewAllowed && (m_previewSummaryArray.GetSize() == 1)) ? m_previewSummaryArray[0] : NULL; }
	void						InvalidateCachedPreviewSummary(CSummary * in_pInvalidSummary = NULL, bool in_bRefreshPreviewDisplay = false);
	void						NotifyPreviewSelectionMayHaveChanged(bool in_bRefreshASAP = false, bool bSelectionResultDirectUserAction = true);
	void						SetSumListBox(CTocListBox * in_pSumListBox) { m_pSumListBox = in_pSumListBox; }
	CTocView *					GetTocView() const;
	virtual BOOL				IsReplyable();
	void						GetSelectedSummaries(SummaryArrayT & out_summaryArray);

	//	Composing and sending helpers
	void ComposeMessage(UINT nID, const char* pszDefaultTo, const char* pszStationery = NULL, const char* pszPersona = NULL);
	void SendAgain(const char* pszPersona = NULL);

// Overrides
	virtual BOOL OnCreateClient( LPCREATESTRUCT	lpcs, CCreateContext* pContext);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTocFrame)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCmdDelete();
	//}}AFX_VIRTUAL

	void ShowHideSplitter();
	void SwitchPane(int Index);

	bool						ShouldPreview();

	// Implementation
protected:
	virtual ~CTocFrame();

	INT							GetCurrentSplitterPos();
	bool						CheckPreviewSelection();
	bool						DoPreviewDisplay();

	void SwitchViewClass(CRuntimeClass* pNewViewRuntimeClass);

	//	Message map helpers
	void Print(BOOL bShowDialog);
	void SetJunkStatus(bool bJunk);

	//	Other message map functions
	BOOL OnComposeMessage(UINT nID);
	BOOL SetStatus(UINT nID);
	BOOL SetPriority(UINT nID);
	BOOL SetLabel(UINT nID);
	BOOL SetServerStatus(UINT nID);

#ifdef IMAP4 // Undelete messages
	void UnDelete();
	void ImapExpunge();
	BOOL OnImapRedownload(UINT nID);
	BOOL OnImapFetch(UINT nID);
	void OnImapClearCached();
	void OnImapResync();
#endif // IMAP4

	static const UINT			kCheckPreviewSelection;
	static const UINT			kCheckAutoMarkAsRead;

	CToolBar* m_PeanutToolBar;
	CTabCtrl* m_PeanutTabCtrl;
	SummaryArrayT				m_previewSummaryArray;
	SummaryArrayT				m_emptySummaryArray;

	CTocListBox *				m_pSumListBox;
	UINT						m_nCheckPreviewSelectionTimerID;
	UINT						m_nCheckAutoMarkAsReadTimerID;

	int							m_nLastSplitterPos;

	//	Whether or not we're currently inside DoPreviewDisplay for this CTocFrame.
	//	Used to prevent reentrancy - replaces static that incorrectly stopped other
	//	CTocFrames from drawing their preview display correctly.
	bool						m_bInsideDoPreviewDisplay;

	//	Whether or not we should ignore calls to InvalidateCachedPreviewSummary and CheckPreviewSelection
	bool						m_bIgnorePreviewSelectionChanges;

	//	Whether or not preview selection changes were attempted
	bool						m_bPreviewSelectionChangesAttempted;

	//	Whether or not cache has been invalidated since we last checked the selection
	bool						m_bCacheInvalidated;

	//	Whether or not preview is allowed with a given selection of messages
	bool						m_bPreviewAllowed;

	//	Whether or not we're currently displaying a preview (i.e. did last load work, etc.)
	bool						m_bDisplayingPreview;

	//	Whether or not the current selection is the result of direct user action
	bool						m_bSelectionResultDirectUserAction;

	CMsgSplitterWnd m_wndSplitter;

	// Generated message map functions
	//{{AFX_MSG(CTocFrame)
	afx_msg void OnChildActivate();
	afx_msg long OnCheckPreviewSelection(WPARAM wParam, LPARAM lParam);
	afx_msg long OnInitPreview( WPARAM	wParam, LPARAM	lParam );
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateCmdDelete(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPeanutActions();
	afx_msg void OnUpdatePeanutActions(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnToggleHeader();
	afx_msg void OnEditFindFindMsg();
	afx_msg void OnUpdateCmdUndelete (CCmdUI* pCmdUI);
	afx_msg void OnUpdateCmdImapExpunge (CCmdUI* pCmdUI);
	afx_msg void OnCmdUnDelete();
	afx_msg void OnCmdImapExpunge();
	//}}AFX_MSG
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);	
	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateNeedSel(CCmdUI * pCmdUI);
	afx_msg void OnUpdateResponse(CCmdUI * pCmdUI);
	afx_msg void OnUpdateSendImmediately(CCmdUI * pCmdUI);
	afx_msg void OnUpdateChangeQueueing(CCmdUI * pCmdUI);
	afx_msg void OnUpdateNeedSelNoOut(CCmdUI * pCmdUI);
	afx_msg void OnUpdateToggleStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSendable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSentUnsent(CCmdUI* pCmdUI);
	afx_msg void OnUpdateJunkCommands(CCmdUI* pCmdUI);
	afx_msg void OnUpdateServerStatus(CCmdUI* pCmdUI);

	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintOne();
	afx_msg void OnRecheckMessagesForJunk();
	afx_msg void OnDeclareMessagesJunk();
	afx_msg void OnDeclareMessagesNotJunk();
	afx_msg void OnMakeNickname();
	afx_msg void OnSendAgain();
	afx_msg void OnSendImmediately();
	afx_msg void OnChangeQueueing();
	afx_msg void OnToggleStatus();
	afx_msg void OnFilterMessages();
	afx_msg void OnMakeFilter();
	afx_msg void OnTransferNewInRoot();	
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	afx_msg BOOL OnDynamicCommand(UINT uID);

#ifdef IMAP4 // Undelete messages.
	afx_msg void OnUpdateUndelete (CCmdUI* pCmdUI);
	afx_msg void OnUpdateImapExpunge (CCmdUI* pCmdUI);
	afx_msg	void OnUpdateImapRedownload (CCmdUI* pCmdUI);
	afx_msg	void OnUpdateImapFetch (CCmdUI* pCmdUI);
	afx_msg	void OnUpdateImapClearCached (CCmdUI* pCmdUI);
#endif // IMAP4 

	DECLARE_MESSAGE_MAP()

private:
	CTocDoc* GetTocDoc();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOCFRAME_H__CE6A8E92_1B2D_11D1_91DB_00805FD2B1FA__INCLUDED_)
