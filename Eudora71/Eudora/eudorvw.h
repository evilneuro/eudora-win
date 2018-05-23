// eudorvw.h : interface of the CEudoraView class
//
/////////////////////////////////////////////////////////////////////////////

class CEudoraView : public CView
{
protected: // create from serialization only
	CEudoraView();
	DECLARE_DYNCREATE(CEudoraView)

// Attributes
public:
	CEudoraDoc* GetDocument();

// Operations
public:

// Implementation
public:
	virtual ~CEudoraView();
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Printing support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Generated message map functions
protected:
	//{{AFX_MSG(CEudoraView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in eudorvw.cpp
inline CEudoraDoc* CEudoraView::GetDocument()
   { return (CEudoraDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
