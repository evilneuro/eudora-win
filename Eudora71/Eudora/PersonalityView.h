// PersonalityView.h : header file
//
// Copyright (c) 1997-1999 by QUALCOMM, Incorporated
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

#ifndef __PERSONALITYVIEW_H__
#define __PERSONALITYVIEW_H__

#include <afxcview.h>
#include "QCImageList.h"
#include "EudoraMsgs.h"


//	Forward declarations
class CSummary;
class CTocFrame;

/////////////////////////////////////////////////////////////////////////////
// CPersonalityView view

class CPersonalityView : public CListView
{
protected:
	DECLARE_DYNCREATE(CPersonalityView)

// Attributes
public:

// Operations
public:
	//
	// Normally, the constructor and destructor are protected since
	// the MFC document template stuff takes care of object creation
	// and destruction.  However, since this is now owned by a Wazoo
	// window, these need to be public.
	//
	CPersonalityView();           
	virtual ~CPersonalityView();

	BOOL PopulateView();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPersonalityView)
public:
	virtual void OnInitialUpdate();

	//
	// MFC is screwed up here.  The virtual OnCmdMsg method is public 
	// in CCmdTarget, but protected in CView.  Therefore, we need to
	// to make a public wrapper to call the protected CView version.
	//
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{ 
			return CListView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

	//
	// Overrides for drag drop implementation, inherited from the CView 
	// base class.
	//
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
								  DWORD dwKeyState, 
								  CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject,
						DROPEFFECT dropEffect, 
						CPoint point);

protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPersonalityView)
	afx_msg void OnDestroy();
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClick(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg BOOL OnCmdTransferMail(UINT nID);

	afx_msg void OnCmdNewMessageAs();
	afx_msg BOOL OnCmdMultiReplyAs(UINT nID);

	afx_msg void OnUpdateCheckMail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSendMail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMultiReply(CCmdUI* pCmdUI);

	afx_msg void OnCmdNewPersonality();
	afx_msg void OnCmdModifyPersonality();
	afx_msg void OnCmdDeletePersonality();

	afx_msg void OnUpdateNewPersonality(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDeletePersonality(CCmdUI* pCmdUI);

	afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSingleSelection(CCmdUI* pCmdUI);

	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg LONG OnMsgAdvNew(WPARAM, LPARAM);
	afx_msg LONG OnMsgAdvEditDom(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int GetSelectedPersonalities(CStringList& strListPersonalities);
	UINT MapToGlobalCommandId(UINT uPersonaCmdId);
	void GetCommandTarget(CSummary *& out_pSummary, CTocFrame *& out_pTocFrame);

private:
	QCMailboxImageList  m_ImageList;
	int					m_nSortIndex;
	COleDropTarget		m_dropTarget;
};

/////////////////////////////////////////////////////////////////////////////
#endif
