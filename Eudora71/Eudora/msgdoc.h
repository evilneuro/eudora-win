// msgdoc.h
//
// Base document class for read and composition messages
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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

#ifndef MSGDOC_H
#define MSGDOC_H

#include "doc.h"
#include "MimeStorage.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageDoc document

class CSummary;
class JJFile;
class CCompStationeryDoc;

typedef enum
{
	MSE_MESSAGE_MISSING_HEADERS = 0x0200	// could not save message due to missing headers
} MESSAGE_SAVE_ERROR;

class CMessageDoc : public CDoc
{
	DECLARE_DYNAMIC(CMessageDoc)

protected:
	CMessageDoc();			// protected constructor used by dynamic creation

// Attributes
public:
	CSummary* m_Sum;
	QCMessage m_QCMessage;		// the forerunner of MIME Storage
	CString	m_MessageId;		// someday we'll just use m_QCMessage!!!

// Operations
public:
	bool		WasTranslated() const { return m_bWasTranslated; }
	void		SetWasTranslated(bool in_bWasTranslated = true) { m_bWasTranslated = in_bWasTranslated; }
	void		SetAssociatedFiles(CStringList * in_pAssociatedFiles);
	
	void ComposeMessage(UINT MenuItemID, const char* DefaultTo, const char* stationery = NULL,
			    const char* pSelText = NULL);
	void OnSendAgain();

	virtual BOOL OnOpenDocument(const char* pszPathName);
	BOOL OnOpenDocument(CSummary* Sum)
		{ return (OnOpenDocument((const char*)Sum)); }
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	virtual BOOL Read();
	virtual HRESULT Write() = 0;
//	virtual BOOL SaveAs(CSaveAs& SA) = 0;

	BOOL DidFindOpenView() const
		{ return m_DidFindOpenView; }

	char* GetText();
	const char* SetText(const char* NewText);
	const char* CatText(const char* NewText);
	const UINT GetTextSize() {return m_BufSize;}
    unsigned GetSelectedText( CString& szText );

	void AssignMessageId( void );

// Implementation
protected:
	virtual ~CMessageDoc();
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CMessageDoc)
	afx_msg void OnUpdateNoComp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReplyToAll(CCmdUI* pCmdUI);
	afx_msg void OnMakeNickname();
	afx_msg void OnRecalculateJunkScore();
	afx_msg BOOL OnChangeJunkStatus(UINT nID);
	afx_msg void OnFilterMessages();
	afx_msg void OnMakeFilter();
	afx_msg void OnCmdViewSource();
	afx_msg void OnUpdateSendToBrowser(CCmdUI* pCmdUI);
	afx_msg void OnSendToBrowser();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateJunk(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNotJunk(CCmdUI* pCmdUI);
	afx_msg BOOL OnDynamicCommand(UINT uID);
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnTransferNewInRoot();
	BOOL ComposeMessage(UINT MenuItemID);
	BOOL OnSwitchMessage(UINT Direction);
	BOOL OnLabel(UINT LabelMenuID);
	BOOL OnSetPriority(UINT uPriorityCmdID);
	DECLARE_MESSAGE_MAP()

protected:
	char*	m_Text;
	UINT	m_BufSize;

	long m_FindIndex;		// Find Engine's current search location within this message
	BOOL m_DidFindOpenView;	// Set TRUE when Find Engine was responsible for opening view

	bool			m_bWasTranslated;
	CStringList *	m_pAssociatedFiles;

	void WriteReadReceipt(BOOL WasAuto);
	void ViewMessageExternally(BOOL ForceText);

public:
#define RAW		TRUE
	bool	GetFullMessage(CString & out_szMessage, BOOL in_bRaw = FALSE, JJFile * in_pMailbox = NULL);
	char *	GetFullMessage(BOOL bRaw = FALSE, JJFile * in_pMailbox = NULL);

// Operations
public:
	virtual BOOL OnNewDocument();
	virtual void SetTitle(const char* pszTitle);
	void ReallySetTitle(const char* pszTitle)
		{ m_strTitle.Empty(); SetTitle(pszTitle); }
	virtual BOOL WriteAsText(JJFile* pFile, BOOL bIsStationery = FALSE ) = 0;
	virtual BOOL SaveAsFile(JJFile* pFile, const char* szPathname) = 0;
	virtual BOOL SaveModified();
};

#endif
