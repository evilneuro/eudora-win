#if !defined(AFX_COLEDOC_H__1DDE4B71_EF4D_11D3_87CD_000000000000__INCLUDED_)
#define AFX_COLEDOC_H__1DDE4B71_EF4D_11D3_87CD_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// COleDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COleDoc document

class COleDoc : public COleDocument
{
	// initialized from "MessageFont" logfont
	IFontDisp* m_pDefFont;
	void init_default_font();

protected:
	COleDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(COleDoc)

// Attributes
public:
	LPFONTDISP GetFontDispatch();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COleDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COleDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
			{return COleDocument::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );}

	// Generated message map functions
protected:
	//{{AFX_MSG(COleDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLEDOC_H__1DDE4B71_EF4D_11D3_87CD_000000000000__INCLUDED_)
