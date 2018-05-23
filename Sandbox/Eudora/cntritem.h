
class CMessageDoc;
class CMyRichEditView;

class CRichCntrItem : public CRichEditCntrItem
{
	DECLARE_SERIAL(CRichCntrItem)

// Constructors
public:
	CRichCntrItem(REOBJECT* preo = NULL, CMessageDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CMessageDoc* GetDocument()
		{ return (CMessageDoc*)COleClientItem::GetDocument(); }
	CMyRichEditView* GetActiveView()
		{ return (CMyRichEditView*)COleClientItem::GetActiveView(); }

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWordPadCntrItem)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
