// COMPMSGD.H
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

	BOOL WriteAsText(
	JJFile* pFile,
	BOOL	bIsStationery = FALSE );

	BOOL SaveAsFile(
		JJFile *theFile,
		CString szPathname);
	
	DECLARE_DYNCREATE(CCompMessageDoc)
protected:
	BOOL	m_bIsStationery;		//Is this a Stationery or just a 
								//Plain msg with/without stationery applied
	CString	m_RedirectFrom;
// Attributes
public:
	BOOL	m_HasBeenSaved;
	BOOL	m_StationeryApplied;
	unsigned long m_NextAutoSaveTime;

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

	void SetupRedirect(const char * oldFrom = NULL);
	BOOL IsRedirect()
		{ return !m_RedirectFrom.IsEmpty(); }

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
		const char* ECHeaders = NULL 
	);
	BOOL Queue( BOOL autoSend = FALSE);
	BOOL IsHeaderEmpty(int i);
	const char* SetHeaderLine(int i, const char* NewText);
	const char* GetHeaderLine(int i);
	void SetHeaderAsInvalid(int nHeader, BOOL bInvalid);

//	const char* GetBody()   
//		{ return (m_Text); }
	BOOL SaveAs(CSaveAs& SA);

	BOOL Write(JJFile* out);

	void ReadSystemInfo(CString &sAttach, CString &sBody);

	BOOL ApplyStationery(CCompStationeryDoc *statDoc);
	BOOL ApplyPersona( CString Persona );

	//
	// Implementation of required interface for built-in message Find Engine.
	//
	virtual BOOL FindFirst(const CString& searchStr, BOOL matchCase, BOOL wholeWord, BOOL bOpenWnd = TRUE);
	virtual BOOL FindNext(const CString& searchStr, BOOL matchCase, BOOL wholeWord);
	virtual BOOL FindAgain(const CString& searchStr, BOOL matchCase, BOOL wholeWord);

	long DoContextMenu(CWnd* pCaller, WPARAM wParam, LPARAM lParam);

	void InsertFCCInBCC(
	QCMailboxCommand* pCommand );

// Implementation
protected:
	BOOL OnNewDocument();

	BOOL Read();
	BOOL Write();

	void ReadPlatform(CString &sPlatform, CString &sVer, CString &sMachineType,
			CString &sProcessor, CString &sTotalPhys, CString &sTotalVirtual);
	void ReadEudoraInfo(CString &sEudoraIniV2);

	HRESULT OnCompletionTranslate(char* cooked);

	BOOL BadRecipient();

public:
	BOOL AddAttachment(const char* Filename); 

	//
	// Helper function for 16-bit and 32-bit Find Engines.
	//
	void FindSetSel32(/* CCompMessageView* pCompMessageView,*/ CHeaderView* pHeaderView, CEdit* pEditCtrl, int startIndex, int endIndex);

	// Generated message map functions
protected:
	//{{AFX_MSG(CCompMessageDoc)
	afx_msg void OnUpdateChangeQueueing(CCmdUI* pCmdUI);
	afx_msg void OnChangeQueueing();
	afx_msg void OnUpdateStatusSendable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusQueued(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusTimedQueue(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusSent(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusUnsent(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
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

	afx_msg BOOL OnDynamicCommand(
	UINT uID );
	
	afx_msg void OnUpdateDynamicCommand(
	CCmdUI* pCmdUI);

	afx_msg void OnFCCNewInRoot();

	DECLARE_MESSAGE_MAP()
	
protected:
	int m_FindHeaderIndex;				// for built-in Find Engine

public:	// so sendmail.cpp can access it
	CString m_ExtraHeaders;
	CString m_Headers[MaxHeaders];
	BOOL m_HeadersInvalidFlag[NumHeaders];
};


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
	const char* ECHeaders = NULL
);
CCompMessageDoc* NewCompDocumentWith(const char *fileName);
CCompMessageDoc* NewCompDocumentAs(const CString& strPersona);
CSummary* NewMessageFromFile(const char *fileName);
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
HRESULT SendMail(CString strPersona, int WhichToSend = QS_READY_TO_BE_SENT);
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
