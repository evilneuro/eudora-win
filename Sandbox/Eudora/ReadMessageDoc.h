// ReadMessageDoc.h : interface of the CReadMessageDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef READMESSAGEDOC_H
#define READMESSAGEDOC_H

#include "msgdoc.h"

class CSaveAs;

class CReadMessageDoc : public CMessageDoc
{
	static 	CStringArray	m_theTabooHeaderArray;

	BOOL	m_bInFind;
	BOOL	m_bDidFindOpenView;

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
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

	virtual BOOL Write();
	virtual BOOL WriteAsText( JJFile*	pFile, BOOL bIsStationery = FALSE );
	BOOL Write(JJFile* mboxFile);

	static const CStringArray&	GetTabooHeaderArray();

	static void StripTabooHeaders(
		char*	szMessage );

	virtual BOOL SaveAsFile( JJFile* pFile, CString szPathname );

	BOOL FindFirst(
	const CString&	searchStr, 
	BOOL			matchCase, 
	BOOL			wholeWord,
	BOOL			bOpenWnd = TRUE );

	BOOL FindNext(
	const CString&	searchStr, 
	BOOL			matchCase, 
	BOOL			wholeWord);

	BOOL FindAgain(
	const CString&	searchStr, 
	BOOL			matchCase, 
	BOOL			wholeWord);

	BOOL SaveAs(CSaveAs& SA);

// Implementation
public:
	virtual ~CReadMessageDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

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
