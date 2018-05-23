// PgCntrItem.h : interface of the PgCntrItem class
//

#if !defined(AFX_CNTRITEM_H__40C28750_EE32_11D3_87CA_00E098743E20__INCLUDED_)
#define AFX_CNTRITEM_H__40C28750_EE32_11D3_87CA_00E098743E20__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


// fwd decls
struct HtmlObjectSpec;
class COleDoc;
class CPaigeEdtView;

class PgCntrItem : public COleClientItem
{
	DECLARE_SERIAL(PgCntrItem)

	friend PgCntrItem* PgBindToObject(paige_rec_ptr ppg, HtmlObjectSpec* pSpec);

public:
	CRect activatePos;
	bool m_canActivate;
	bool m_activateNow;

private:
	pg_ref m_pg;
	CString m_idString;

	void init_default_font();
	embed_ref find_me( long* index, long* position, pg_embed_ptr* pe );

// Constructors
public:
	PgCntrItem(COleDocument* pContainer = NULL);
//	PgCntrItem(COleDocument* pContainer, pg_ref pg );
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CString& GetIDString(){return m_idString;}
	
	COleDoc* GetDocument()
		{ return (COleDoc*)COleClientItem::GetDocument(); }
	CPaigeEdtView* GetActiveView()
		{ return (CPaigeEdtView*)COleClientItem::GetActiveView(); }

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PgCntrItem)
	public:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	virtual void OnActivate();
	protected:
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
	virtual BOOL CanActivate();
	//}}AFX_VIRTUAL

	// Get/Set for stock properties -- makes up for the fact the we aren't
	// doing ambient properties yet.
	void SetFont( LPFONTDISP pFontDisp );

// Implementation
public:
	~PgCntrItem();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CNTRITEM_H__40C28750_EE32_11D3_87CA_00E098743E20__INCLUDED_)
