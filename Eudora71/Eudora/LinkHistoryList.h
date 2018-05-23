//	LinkHistoryList.h
//	
//	Custom subclass of CListCtrl specifically designed for the Link History
//	Window. Draws large icons correctly, clips correctly, and does
//	full row selection.
//
//	Copyright (c) 1999 by QUALCOMM, Incorporated
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



#ifndef __LinkHistoryList_H__
#define __LinkHistoryList_H__


#include <afxcview.h>
#include "QCImageList.h"
#include "rs.h"

#include "LinkHistoryManager.h"
#include "QICommandClient.h"


class CLinkHistoryList : public CListView, public QICommandClient
{
protected:
	DECLARE_DYNCREATE(CLinkHistoryList)

public:
	typedef LinkHistoryManager::LPUrlItemData	LPUrlItemData;

	typedef enum
	{
		sc_Type = 0,
		sc_Name,
		sc_DateVisited
	} SortColumnEnum;

	//
	// Normally, the constructor and destructor are protected since
	// the MFC document template stuff takes care of object creation
	// and destruction.  However, since this is now owned by a Wazoo
	// window, these need to be public.
	//
								CLinkHistoryList(); 
	virtual						~CLinkHistoryList();

	static CLinkHistoryList *	Instance()
									{
										return s_pLinkHistoryList;
									}
	bool						ResortItemsIfSortedByDate();
	BOOL						ResortItems();

	void						DisplayFeatureNotInFreeMessage();
	virtual void				Notify(
									QCCommandObject *		pObject,
									COMMAND_ACTION_TYPE		theAction,
									void *					pData = NULL);
	
	static const CString &		GetDisplayName(LPUrlItemData pItemData);
	static const CString &		GetDisplayURL(LPUrlItemData pItemData);
	const CString &				GetDisplayDateOrStatus(LPUrlItemData pItemData, CString & szDateString);

	bool						AddURL(
									LinkHistoryManager::LPUrlItemData	pURL);
	void						UpdateURL(
									LinkHistoryManager::LPUrlItemData	pURL,
									bool								bEnsureVisisble = true);
	void						RemoveURL(
									LinkHistoryManager::LPUrlItemData	pURL);

	void						OnLaunchSelectedItem();
	void						OnRemoveSelectedItems();
	void						OnChangeSelItemsStatusToRemind();
	void						OnChangeSelItemsStatusToBookmarked();
	void						OnChangeSelItemsStatusToNone();
	void						OnChangeSelItemsStatus(
									LinkHistoryManager::LinkStatusEnum	status);
	void						OnEditSelectAll();

 	static int CALLBACK			SortCompareFunc(
									LPARAM				lParam1, 
									LPARAM				lParam2,
									LPARAM				lParamSort);

	void						SetStatusURLShown(bool bStatusURLShown) { m_bStatusURLShown = bStatusURLShown; }
	bool						GetStatusURLShown() const { return m_bStatusURLShown; }

	virtual BOOL				PreTranslateMessage(MSG * pMsg);

	//	MFC is screwed up here. The virtual OnCmdMsg method is public in CCmdTarget, but protected
	//	in CView. Therefore, we need to to make a public wrapper to call the protected CView version.
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{ 
			return CListView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//	Message map overrides
	afx_msg int					OnCreate(
									LPCREATESTRUCT		lpcs);
	afx_msg void				OnDestroy();
	afx_msg void				OnPaint();
	afx_msg LRESULT				OnSetBkColor(
									WPARAM				wParam,
									LPARAM				lParam);
	afx_msg void				OnColumnClick(
									NMHDR *				pNMHDR,
									LRESULT *			pResult);
	afx_msg void				OnKeyDown(
									NMHDR *				pNMHDR,
									LRESULT *			pResult);
	afx_msg void				OnLButtonDblClk(
									UINT				nFlags,
									CPoint				point);
	afx_msg BOOL				OnSetCursor(
									CWnd *				pWnd,
									UINT				nHitTest,
									UINT				message );
	afx_msg void				OnGetDispInfo(
									NMHDR *				pnmh,
									LRESULT *			pResult);
	
	afx_msg void				OnUpdateNeedSelection(CCmdUI* pCmdUI);
	afx_msg void				OnUpdateNeedSingleSelection(CCmdUI* pCmdUI);
	afx_msg void				OnUpdateCopy(CCmdUI* pCmdUI);
	afx_msg void				OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void				OnEditCopy();
	DECLARE_MESSAGE_MAP()

	void						RedrawRemovedItems(
									int					nFirstItemToRedraw);
	virtual void				DrawItem(
									LPDRAWITEMSTRUCT	lpDrawItemStruct);
	void						ShowStatusText(
									int					nIndexToShow);

	static CLinkHistoryList *	s_pLinkHistoryList;

	short						m_iSortColumn;
	BOOL						m_bReverseOrder;
	short						m_iPrevSortColumn;
	BOOL						m_bPrevReverseOrder;
	bool						m_bStatusURLShown;
	CRString					m_szStatusRemind;
	CRString					m_szStatusBookmarked;
	CRString					m_szStatusAttempted;
	CRString					m_szStatusNotDisplayed;
	COLORREF					m_clrText;
	COLORREF					m_clrTextBk;
	COLORREF					m_clrBkgnd;
	bool						m_bSelectionChangingKeyHit;
	bool						m_bDrawTypeIcon;
};


#endif		//	__LinkHistoryList_H__
