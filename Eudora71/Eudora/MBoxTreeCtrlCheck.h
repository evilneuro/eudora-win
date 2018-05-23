// MBoxTreeCtrlCheck.h : header file
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

#ifndef _MBOXTREECTRLCHECK_H_
#define _MBOXTREECTRLCHECK_H_

#include "mboxtree.h"
#include <deque>

extern UINT msgMBoxTreeCtrlCheck_CheckChange; // registered Windows message

/////////////////////////////////////////////////////////////////////////////
// CMBoxTreeCtrlCheck window

#define STATETOINDEXIMAGEMASK(i) ((i) >> 12)

class CMBoxTreeCtrlCheck : public CMboxTreeCtrl
{
public:
	// Typedefs
	typedef enum
	{
		TCS_NOSTATE = 0,
		TCS_UNCHECKED,
		TCS_PARTIAL_CHECK,
		TCS_CHECKED
	} TCT_CheckState;

	typedef enum
	{
		TCA_CHECK = 0,
		TCA_PARTIAL_CHECK,
		TCA_UNCHECK,
		TCA_TOGGLE,
		TCA_REFRESH
	} TCT_CheckAction;

// Construction
	CMBoxTreeCtrlCheck();
	virtual ~CMBoxTreeCtrlCheck();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMBoxTreeCtrlCheck)
	//}}AFX_VIRTUAL

	// Override from QCMailboxTreeCtrl
	virtual BOOL GetItemStruct(TV_INSERTSTRUCT &tvstruct,
								ItemType itemType,
								const char* itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);

	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(LPTV_INSERTSTRUCT lpInsertStruct);
	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage,
							int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,
							HTREEITEM hParent, HTREEITEM hInsertAfter);
	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT,
							HTREEITEM hInsertAfter = TVI_LAST);
	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage,
							HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	HTREEITEM GetNextTraversalItem(HTREEITEM hItem, bool bWantChildren = true) const;

	BOOL IsItemChecked(HTREEITEM hItem) const;
	HTREEITEM GetFirstCheckedItem() const;
	HTREEITEM GetNextCheckedItem(HTREEITEM hItem) const;
	void SetCheck(HTREEITEM hItem, BOOL bCheck = true);
	void CheckAll(bool bCheck = true);
	UINT GetCheckCount() const;

	BOOL GetListMailboxPathnamesWithSelectedState(
			bool						in_bAddFolders,
			TCT_CheckState				in_eState,
			std::deque<CString> &		out_listPathnames) const;

	BOOL SetContainerFlag(BOOL bContainers = true);
	BOOL SetSingleCheck(BOOL bSingleCheck = true);

	BOOL ExpandAll();

	BOOL InitImageLists(UINT nImageBitmapID, int cImageX, COLORREF crImageMask,
							UINT nStateBitmapID, int cStateX, COLORREF crStateMask );
	BOOL InitItemImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask);
	BOOL InitStateImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask);

	HTREEITEM FindCheck(QCMailboxCommand *pMbxCmd, bool bClearCheck = true, bool bCheckParent = false);

// Generated message map functions
protected:
	HTREEITEM m_hCheckedItem; // Only valid if m_bSingleCheck == true
	BOOL m_bSingleCheck;

	void RefreshTreeChecks();
	bool DoAction(HTREEITEM hItem, TCT_CheckAction nAction);
	void RefreshNode(HTREEITEM hItem);
	TCT_CheckState GetRefreshState(HTREEITEM hItem, TCT_CheckState CurrentState = TCS_NOSTATE);
	BOOL m_bContainers;

	BOOL m_bInitStateImageList;
	CImageList m_imageState; // This contains the image list of bitmaps for the state (checkboxes)

	BOOL m_bInitItemImageList;
	CImageList m_imageItem; // This contains the image list of bitmaps for each item

	//{{AFX_MSG(CMBoxTreeCtrlCheck)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _MBOXTREECTRLCHECK_H_

