// TEXTFILE.H
//
#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

/////////////////////////////////////////////////////////////////////////////
// CTextFileDoc document

class CTextFileDoc : public CDoc
{
	DECLARE_SERIAL(CTextFileDoc)
protected:
	CTextFileDoc();			// protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	BOOL OnSaveDocument(const char* pszPathName);
	virtual BOOL SaveModified();
	//Public because BigEditView also uses these
 	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSave();
                                   
// Implementation
protected:
	~CTextFileDoc();
	void Serialize(CArchive& ar);	// overridden for document i/o 
	BOOL	m_SaveAs;

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextFileDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg BOOL OnDynamicCommand(UINT uID );
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#endif