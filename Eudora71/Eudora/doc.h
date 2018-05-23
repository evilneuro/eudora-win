// DOC.H
//
// Derived CDocument that does some handy functions
//
#ifndef DOC_H
#define DOC_H

#include "OleDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CDoc document

class CDoc : public COleDoc
{
	DECLARE_DYNCREATE(CDoc)
protected:
	CDoc();			// protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	virtual BOOL OnNewDocument();
	virtual void SetTitle(const char* pszTitle);
	void ReallySetTitle(const char* pszTitle)
		{ m_strTitle.Empty(); SetTitle(pszTitle); }
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual BOOL SaveModified();

// Implementation
protected:
	virtual ~CDoc();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDoc)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif
