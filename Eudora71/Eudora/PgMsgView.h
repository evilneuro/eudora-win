// PgMsgView.h : header file
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#if !defined(AFX_PGMSGVIEW_H__30F69821_1551_11D1_AC54_00805FD2626C__INCLUDED_)
#define AFX_PGMSGVIEW_H__30F69821_1551_11D1_AC54_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PaigeEdtView.h"
#include "PgStuffBucket.h"

/////////////////////////////////////////////////////////////////////////////
// PgMsgView view

//	Forward declarations
class CMessageDoc;


class PgMsgView : public CPaigeEdtView
{
    bool m_bEnableAttachmentMenus;
	select_pair m_attachmentOffset;

	bool enum_attachments( char** ndlist, select_pair_ptr within = NULL );
	void copy_attachments( select_pair_ptr pSel, COleDataSource* pDS );
	bool point_in_attach(CPoint ptClient, long* pOffset = NULL);

protected:
    PgMsgView();           // protected constructor used by dynamic creation
    DECLARE_DYNCREATE(PgMsgView)

// Attributes
protected:
    bool fDiscardMessage;

// Operations
public:
    virtual HRESULT		SaveInfo();

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

	virtual void		GetMessageForDisplay(
								CMessageDoc *						in_pMsgDoc,
								ContentConcentrator::ContextT		in_context,
								bool								in_bStripDocumentLevelTags,
								bool								in_bRelaxLocalFileRefStripping,
								CString &							out_szBody,
								CString *							out_szHeaders = NULL,
								bool *								out_bWasConcentrated = NULL,
								bool								in_bMorphHTML = false);
    virtual void		ImportMessage(
								CString &					szBody,
								PgDataTranslator *			pTxr = NULL );
    virtual void ExportMessage( CMessageDoc* pMsgDoc = NULL );

    void SetMessageStuff( CMessageDoc* pMsgDoc );
	virtual void		SetMoreMessageStuff(
								PgStuffBucket *						pStuffBucket);

    BOOL SelectionHasHtmlSnippet(select_pair &sel);
	int EnumEmbeddedObjects( char** eoList );
	virtual void OnCopy(COleDataSource* pDS );

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Generated message map functions
protected:
    //{{AFX_MSG(PgMsgView)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnUpdateCmdViewSource(CCmdUI* pCmdUI);
    afx_msg void OnInvisibles();
    afx_msg void OnCopyAttachment();
	afx_msg void OnAttachmentAction(UINT nID);
    afx_msg void OnUpdateAttachmentAction(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu( CWnd*, CPoint );
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:
    void OpenFileAtOffset(long offset, int command = 0);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGMSGVIEW_H__30F69821_1551_11D1_AC54_00805FD2626C__INCLUDED_)
