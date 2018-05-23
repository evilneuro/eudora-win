#if !defined(AFX_PGMSGVIEW_H__30F69821_1551_11D1_AC54_00805FD2626C__INCLUDED_)
#define AFX_PGMSGVIEW_H__30F69821_1551_11D1_AC54_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PgMsgView.h : header file
//

#include "PaigeEdtView.h"

/////////////////////////////////////////////////////////////////////////////
// PgMsgView view

class CMessageDoc;
class PgMsgView : public CPaigeEdtView
{
    bool m_bEnableCopyAttachment;
    long m_lAttachmentOffset;

protected:
    PgMsgView();           // protected constructor used by dynamic creation
    DECLARE_DYNCREATE(PgMsgView)

// Attributes
protected:
    bool fDiscardMessage;

// Operations
public:
    void SaveInfo();

    // Big Hack!!
    static void FlagAsDiscardable( PgMsgView*, bool bDiscard = true );

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(PgMsgView)
public:
    virtual void OnInitialUpdate();
    //}}AFX_VIRTUAL

// Implementation
protected:
    virtual ~PgMsgView();

    virtual void ImportMessage( CMessageDoc* pMsgDoc = NULL );
    virtual void ExportMessage( CMessageDoc* pMsgDoc = NULL );

    void SetMessageStuff( CMessageDoc* pMsgDoc );

    BOOL SelectionHasHtmlSnippet(select_pair &sel);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Generated message map functions
protected:
    //{{AFX_MSG(PgMsgView)
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnUpdateCmdViewSource(CCmdUI* pCmdUI);
    afx_msg void OnInvisibles();
    afx_msg void OnCopyAttachment();
    afx_msg void OnUpdateCopyAttachment(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu( CWnd*, CPoint );
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:
    void OpenFileAtOffset(long offset);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGMSGVIEW_H__30F69821_1551_11D1_AC54_00805FD2626C__INCLUDED_)
