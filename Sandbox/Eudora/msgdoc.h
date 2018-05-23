// MSGDOC.H
//
// Base document class for read and composition messages
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
	void ComposeMessage(UINT MenuItemID, const char* DefaultTo, const char* stationery = NULL,
			    const char* pSelText = NULL);
	void OnSendAgain();

	virtual BOOL OnOpenDocument(const char* pszPathName);
	BOOL OnOpenDocument(CSummary* Sum)
		{ return (OnOpenDocument((const char*)Sum)); }
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	virtual BOOL Read();
	virtual BOOL Write() = 0;
//	virtual BOOL SaveAs(CSaveAs& SA) = 0;

	//
	// Required interface for built-in message Find Engine.
	//
	virtual BOOL FindFirst(const CString& searchStr, BOOL matchCase, BOOL wholeWord, BOOL bOpenWnd = TRUE);
	virtual BOOL FindNext(const CString& searchStr, BOOL matchCase, BOOL wholeWord);
	virtual BOOL FindAgain(const CString& searchStr, BOOL matchCase, BOOL wholeWord);
	BOOL DidFindOpenView() const
		{ return m_DidFindOpenView; }

	char* GetText();
	const char* SetText(const char* NewText);
	const char* CatText(const char* NewText);
	const char* SetText(CEdit* Ctrl);
	const UINT GetTextSize() {return m_BufSize;}

// Implementation
protected:
	virtual ~CMessageDoc();

    unsigned GetSelectedText( CString& szText );
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CMessageDoc)
	afx_msg void OnUpdateNoComp(CCmdUI* pCmdUI);
	afx_msg void OnMakeNickname();
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

	void WriteReadReceipt(BOOL WasAuto);
	void AssignMessageId( void );
	void ViewMessageExternally(BOOL ForceText);

public:
#define RAW		TRUE
	char *GetFullMessage( BOOL bRaw = FALSE );

#ifdef WIN32
// doc.h additions.  RichEdit inheritance requires
// that doc.h be subsumed into this class.
	// Operations
public:
	virtual BOOL OnNewDocument();
	virtual void SetTitle(const char* pszTitle);
	void ReallySetTitle(const char* pszTitle)
		{ m_strTitle.Empty(); SetTitle(pszTitle); }
	virtual BOOL WriteAsText( JJFile*	pFile, BOOL bIsStationery = FALSE ) = 0;
	virtual BOOL SaveAsFile( JJFile* pFile, CString szPathname) = 0;
	virtual BOOL SaveModified();
	/* added above from doc.h
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
   */
#endif


};

#endif
