// PgCntrItem.cpp : implementation of the PgCntrItem class
//

#include "stdafx.h"
#include "afxole.h"

#include "paige.h"
#include "pgembed.h"
#include "pgtraps.h"

#include "OleDoc.h"
#include "PaigeEdtView.h"
#include "font.h"

// our own lazy ass
#include "PgCntrItem.h"

#include "DebugNewHelpers.h"



/////////////////////////////////////////////////////////////////////////////
// PgCntrItem implementation

IMPLEMENT_SERIAL(PgCntrItem, COleClientItem, 0)

PgCntrItem::PgCntrItem(COleDocument* pContainer)
	: COleClientItem(pContainer)
{
	m_pg = 0;
	activatePos.SetRect(0,0,0,0);
	m_canActivate = false;
	m_activateNow = false;;
}

PgCntrItem::~PgCntrItem()
{
}

void PgCntrItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
	ASSERT_VALID(this);

	COleClientItem::OnChange(nCode, dwParam);

	// When an item is being edited (either in-place or fully open)
	//  it sends OnChange notifications for changes in the state of the
	//  item or visual appearance of its content.

	// TODO: invalidate the item by calling UpdateAllViews
	//  (with hints appropriate to your application)

#if 0
	GetDocument()->UpdateAllViews(NULL);
		// for now just update ALL views/no hints
#endif

	if ( nCode == OLE_CHANGED ) {
		long pos;
		pg_embed_ptr pe;
		embed_ref er;

		if ( er = find_me( NULL, &pos, &pe ) ) {
			pgInvalEmbedRef( m_pg, pos, pe, best_way );
			UnuseMemory( er );
		}
	}
}

BOOL PgCntrItem::OnChangeItemPosition(const CRect& rectPos)
{
	ASSERT_VALID(this);

	// During in-place activation PgCntrItem::OnChangeItemPosition
	//  is called by the server to change the position of the in-place
	//  window.  Usually, this is a result of the data in the server
	//  document changing such that the extent has changed or as a result
	//  of in-place resizing.
	//
	// The default here is to call the base class, which will call
	//  COleClientItem::SetItemRects to move the item
	//  to the new position.

	if (!COleClientItem::OnChangeItemPosition(rectPos))
		return FALSE;

	// TODO: update any cache you may have of the item's rectangle/extent

	return TRUE;
}


inline void pgpoint2point( co_ordinate_ptr pgpt, POINT* pt ) {
	pt->x = pgpt->h;
	pt->y = pgpt->v;
}

void PgCntrItem::OnGetItemPosition(CRect& rPosition)
{
	ASSERT_VALID(this);

	// During in-place activation, PgCntrItem::OnGetItemPosition
	//  will be called to determine the location of this item.  The default
	//  implementation created from AppWizard simply returns a hard-coded
	//  rectangle.  Usually, this rectangle would reflect the current
	//  position of the item relative to the view used for activation.
	//  You can obtain the view by calling PgCntrItem::GetActiveView.

	rPosition = activatePos;
}

BOOL PgCntrItem::CanActivate()
{
	return TRUE;
//	return (m_canActivate == true);
}

void PgCntrItem::OnActivate()
{
	// Allow only one inplace activate item per frame
	CPaigeEdtView* pView = GetActiveView();
	ASSERT_VALID(pView);
	COleClientItem* pItem = GetDocument()->GetInPlaceActiveItem(pView);

	if (pItem != NULL && pItem != this)
		pItem->Close();

	COleClientItem::OnActivate();
}

void PgCntrItem::OnDeactivateUI(BOOL bUndoable)
{
	COleClientItem::OnDeactivateUI(bUndoable);

    // Hide the object if it is not an outside-in object
    DWORD dwMisc = 0;
    m_lpObject->GetMiscStatus(GetDrawAspect(), &dwMisc);
    if (dwMisc & OLEMISC_INSIDEOUT)
        DoVerb(OLEIVERB_HIDE, NULL);
}

void PgCntrItem::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	// Call base class first to read in COleClientItem data.
	// Since this sets up the m_pDocument pointer returned from
	//  PgCntrItem::GetDocument, it is a good idea to call
	//  the base class Serialize first.
	COleClientItem::Serialize(ar);

	// now store/retrieve data specific to PgCntrItem
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// PgCntrItem diagnostics

#ifdef _DEBUG
void PgCntrItem::AssertValid() const
{
	COleClientItem::AssertValid();
}

void PgCntrItem::Dump(CDumpContext& dc) const
{
	COleClientItem::Dump(dc);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Get/Set routines

// set an ole control's stock font property
void PgCntrItem::SetFont( LPFONTDISP pFontDisp )
{
	assert( pFontDisp != 0 );

	if ( pFontDisp ) {
		// get the control's dispatch interface
		IDispatch* pDisp = 0;
		HRESULT hr = m_lpObject->QueryInterface( IID_IDispatch, (void**)&pDisp );
		assert( SUCCEEDED(hr) && pDisp );

		if ( SUCCEEDED(hr) && pDisp ) {
			try {
				COleDispatchDriver dspDrv( pDisp );
				dspDrv.SetProperty( DISPID_FONT, VT_FONT, pFontDisp );
			}
			catch ( CException* e ) {
				// this probably just means that the control does not have
				// a stock font property---fail quietly.
				TRACE( "SetProperty: failed setting Stock Font.\n" );
				e->Delete();
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Help

// a little paige-stupidity spackle
embed_ref PgCntrItem::find_me( long* index, long* position, pg_embed_ptr* pe )
{
	bool found = false;

	embed_ref er = 0;
	select_pair sel;
	long pos;
	pg_embed_ptr embed_ptr;

	sel.begin = 0;
	sel.end = pgTextSize( m_pg );

	for ( long li =  1; !found && (er = pgGetIndEmbed( m_pg, &sel, li, &pos, 0 )); li++ ) {
		if ( er && (embed_ptr = (pg_embed_ptr)UseMemory( er )) ) {
			if ( embed_ptr->user_refcon == (long)this ) {
				if ( index )
					*index = li;
				if ( position )
					*position = pos;
				if ( pe )
					*pe = embed_ptr;
				else
					UnuseMemory( er );

				found = true;
			}
			else
				UnuseMemory( er );
		}
	}

	return found ? er : 0;
}

