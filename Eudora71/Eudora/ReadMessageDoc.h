// ReadMessageDoc.h : interface of the CReadMessageDoc class
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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


#ifndef READMESSAGEDOC_H
#define READMESSAGEDOC_H

#include "msgdoc.h"

class CSaveAs;

class CReadMessageDoc : public CMessageDoc
{
	static 	CStringArray	m_theTabooHeaderArray;

	BOOL	m_bInFind;
	BOOL	m_bDidFindOpenView;
public:

protected: // create from serialization only
	CReadMessageDoc();
	DECLARE_DYNCREATE(CReadMessageDoc)

// Attributes
public:
// Operations
private:
	static void BuildTabooHeaderArray();

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReadMessageDoc)
	public:
	//}}AFX_VIRTUAL

	virtual HRESULT Write();
	virtual BOOL WriteAsText(JJFile* pFile, BOOL bIsStationery = FALSE);
	HRESULT Write(JJFile* mboxFile);

	static const CStringArray& GetTabooHeaderArray();

	static void StripTabooHeaders(char* szMessage);

	virtual BOOL SaveAsFile(JJFile* pFile, const char* szPathname);

	BOOL SaveAs(CSaveAs& SA);

// Implementation
public:
	virtual ~CReadMessageDoc();

protected:
	CView* CReadMessageDoc::GetView();

// Generated message map functions
protected:
	//{{AFX_MSG(CReadMessageDoc)
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	afx_msg BOOL OnDynamicCommand(UINT uID);
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);

	afx_msg void OnUpdateStatusUnread(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusRead(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusReplied(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusForwarded(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusRedirected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusRecovered(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusToggle(CCmdUI* pCmdUI);

	afx_msg BOOL OnMessageStatus(UINT StatusMenuID);

	afx_msg void OnUpdateMessageServer(CCmdUI* pCmdUI);

	afx_msg void OnMessageServerLeave();
	afx_msg void OnMessageServerFetch();
	afx_msg void OnMessageServerDelete();
	afx_msg void OnMessageServerFetchDelete();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
