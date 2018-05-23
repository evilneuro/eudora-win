// SIGNATUR.H
//

#ifndef _SIGNATUR_H_
#define _SIGNATUR_H_

#include "doc.h"
#include "PaigeEdtView.h"

class CSignatureView;

class CSignatureDoc : public CDoc
{
	DECLARE_SERIAL(CSignatureDoc )
protected:
	CSignatureDoc ();			// protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	//CSignatureView* GetView();

	//BOOL OnSaveDocument(const char* pszPathName);
 	afx_msg void OnFileSaveAs();

	//Virtual overrides
	virtual void SetTitle(const char* pszTitle);

	BOOL GetSignature(
		CString&	szText ) { return (FALSE); }

// Implementation
protected:
	~CSignatureDoc ();
	void Serialize(CArchive& ar);	// overridden for document i/o 
	BOOL	m_SaveAs;

	// Generated message map functions
protected:
	//{{AFX_MSG(CSignatureDoc )
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CSignatureView view

class CSignatureView : public CPaigeEdtView
{
protected:
	CSignatureView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSignatureView)
// Attributes
public:

	//CString	GetText();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSignatureView)
	public:
	//virtual void OnInitialUpdate();

	protected:
	//virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL


// Implementation
protected:
	virtual ~CSignatureView();
	void Serialize(CArchive& ar);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSignatureView)
	//afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif