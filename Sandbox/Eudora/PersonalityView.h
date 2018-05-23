// PersonalityView.h : header file
//
#ifndef __PERSONALITYVIEW_H__
#define __PERSONALITYVIEW_H__

#include <afxcview.h>
#include "QCImageList.h"
#include "EudoraMsgs.h"

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

	afx_msg void OnUpdateSendMail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMultiReply(CCmdUI* pCmdUI);

	afx_msg void OnCmdNewPersonality();
	afx_msg void OnCmdModifyPersonality();
	afx_msg void OnCmdDeletePersonality();

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

private:
	QCMailboxImageList  m_ImageList;
	int					m_nSortIndex;
	COleDropTarget		m_dropTarget;
};

/////////////////////////////////////////////////////////////////////////////
#endif
