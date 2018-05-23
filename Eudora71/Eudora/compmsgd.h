// compmsgd.h
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

#ifndef COMPMSGD_H
#define COMPMSGD_H

#include "msgdoc.h"

/////////////////////////////////////////////////////////////////////////////
// CCompMessageDoc document
#include "headertype.h"

class JJFile;
class CSaveAs;
class CHeaderView;
class QCMailboxCommand;

class CCompMessageDoc : public CMessageDoc
{
	friend class CCompMessageView;

public:

	virtual BOOL WriteAsText(JJFile* pFile, BOOL bIsStationery = FALSE);
	virtual BOOL SaveAsFile(JJFile* pFile, const char* szPathname);
	
	DECLARE_DYNCREATE(CCompMessageDoc)
protected:
	BOOL	m_bIsStationery;		//Is this a Stationery or just a 
								//Plain msg with/without stationery applied
	BOOL	m_StationeryApplied;	// Was stationery applied?
	CString	m_RedirectFrom;
// Attributes
public:
	BOOL	m_HasBeenSaved;
	unsigned long m_NextAutoSaveTime;
	char    m_ResponseType;

// Operations
public:
	CCompMessageDoc();			// public constructor used by dynamic creation
	~CCompMessageDoc();
	
	CView* GetView();
	CView* GetCompView();
	CHeaderView* GetHeaderView();
	BOOL GetMessageHeaders(CString& hdrs);

	void SetIsStationery(BOOL isStationery = TRUE)
		{ m_bIsStationery = isStationery; }
	BOOL IsStationery()
		{ return m_bIsStationery; }
	BOOL WasStationeryApplied() const
		{ return m_StationeryApplied; }

	void SetupRedirect(const char * oldFrom = NULL);
	BOOL IsRedirect()
		{ return !m_RedirectFrom.IsEmpty(); }

	virtual BOOL OnNewDocument();
	virtual void SetTitle(const char* pszTitle);

    BOOL SaveModified();
	BOOL InitializeNew
	(
		const char* To, 
		const char* From, 
		const char* Subject, 
		const char* Cc,
		const char* Bcc, 
		const char* Attachments, 
		const char* Body,
		const char* ECHeaders = NULL,
		const char  ResponseType = 0
	);
	BOOL Queue( BOOL autoSend = FALSE);
	BOOL IsHeaderEmpty(int i);
	const char* SetHeaderLine(int i, const char* NewText);
	const char* GetHeaderLine(int i);
	void SetHeaderAsInvalid(int nHeader, BOOL bInvalid);

//	const char* GetBody()   
//		{ return (m_Text); }
	BOOL SaveAs(CSaveAs& SA);

	HRESULT Write(JJFile* out);

	void ReadSystemInfo(CString &sAttach, CString &sBody);

	BOOL ApplyStationery(CCompStationeryDoc *statDoc, bool bExpandAliases = false);
	BOOL ApplyPersona(const char* Persona);


	long DoContextMenu(CWnd* pCaller, WPARAM wParam, LPARAM lParam);

	void InsertFCCInBCC(
	QCMailboxCommand* pCommand );

// Implementation
protected:
	virtual BOOL Read();
	virtual HRESULT Write();

	void ReadEudoraInfo(CString &sEudoraIniV2);

	HRESULT OnCompletionTranslate(char* cooked);

	BOOL BadRecipient();

	int GetNumberOfAddressesInMsg();

public:
	BOOL AddAttachment(const char* Filename); 

	//
	// Helper function for 16-bit and 32-bit Find Engines.
	//
	void FindSetSel32(/* CCompMessageView* pCompMessageView,*/ CHeaderView* pHeaderView, CEdit* pEditCtrl, int startIndex, int endIndex);

	// Generated message map functions
protected:
	//{{AFX_MSG(CCompMessageDoc)
	afx_msg void OnUpdateQueueingCommand(CCmdUI* pCmdUI);
	afx_msg void OnChangeQueueing();
	afx_msg void OnUpdateStatusSendable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusQueued(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusTimedQueue(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusSent(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusUnsent(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintOne();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnCanModify(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSaveAsStationery();
	afx_msg void OnSend();
	afx_msg void OnMessageDelete();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	BOOL OnStatus(UINT StatusMenuID);

	afx_msg BOOL OnDynamicCommand(UINT uID);
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
	
protected:
	int m_FindHeaderIndex;				// for built-in Find Engine

	BOOL m_bDoingFccContextMenu;

public:	// so sendmail.cpp can access it
	CString m_ExtraHeaders;
	CString m_Headers[MaxHeaders];
	BOOL m_HeadersInvalidFlag[NumHeaders];
};


typedef enum {
	NN_OBEY_INI,
	NN_ALWAYS_EXPAND,
	NN_NEVER_EXPAND
} NicknameOption;


// Function prototypes
CCompMessageDoc* NewCompDocument
(
	const char* To = NULL, 
	const char* From = NULL, 
	const char* Subject = NULL,
	const char* Cc = NULL, 
	const char* Bcc = NULL, 
	const char* Attachments = NULL, 
	const char* Body = NULL,
	const char* Stationery = NULL,
	const char* Persona = NULL,
	const char	ResponseType = 0,
	const char* ECHeaders = NULL,
	bool bExpandAliases	= false
);
CCompMessageDoc* NewCompDocumentWith(const char *fileName, bool bExpandAliases = false);
CCompMessageDoc* NewCompDocumentAs(const char* strPersona);
CSummary* NewMessageFromFile(const char *fileName, NicknameOption nnOption = NN_OBEY_INI);
void StartGroup( void );
void EndGroup( void );

#define QS_NONE_QUEUED				0
#define QS_DELAYED_MORE_THAN_12		1
#define QS_DELAYED_WITHIN_12		2
#define QS_READY_TO_BE_SENT			4
extern int QueueStatus;
void SetQueueStatus();

extern BOOL FlushQueue;
typedef enum {
	SQR_ALL_OK,
	SQR_UNSENT_MESSAGE,
	SQR_MAJOR_ERROR
} SendQueuedResult;

//Helper functions 
HRESULT SendMail(const char* strPersona, int WhichToSend = QS_READY_TO_BE_SENT);
SendQueuedResult SendQueuedMessages(int WhichToSend = QS_READY_TO_BE_SENT,
									BOOL bMultiPersona = TRUE);
HRESULT SendQueuedMessagesAndQuit(int WhichToSend = QS_READY_TO_BE_SENT, BOOL bMultiPersona = TRUE);

class CCompStationeryDoc : public CCompMessageDoc
{
	friend class CCompMessageView;
	
	DECLARE_DYNCREATE(CCompStationeryDoc)
protected:

// Attributes
public:
	JJFile *m_File;

// Operations
public:
	CCompStationeryDoc();			// public constructor used by dynamic creation
	~CCompStationeryDoc();

	BOOL Read(const char *Filename);
};



#endif
