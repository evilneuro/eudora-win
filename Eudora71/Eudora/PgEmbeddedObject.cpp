// PgEmbeddedObject.cpp -- support for ole-based embed refs in Paige.
//
// Note that this does not support ole objects in *any* paige ref, but only
// those with associated mfc view/doc. The doc must be derived from
// COleDocument. The embed ref points to a "PgCntrItem" [PgCntrItem.cpp],
// which provides a common interface for all objects.

#include "stdafx.h"
#include "afxole.h"
#include "afxodlgs.h"

#include "PaigeEdtView.h"
#include "OleDoc.h"
#include "PgCntrItem.h"
#include "ObjectSpec.h"

#include "paige.h"
#include "pgembed.h"
#include "pgtraps.h"

#define embed_type(p) (p->type & EMBED_TYPE_MASK)

#include "DebugNewHelpers.h"


// called by Paige for all OBJECTs [i.e. ActiveX controls]
PG_PASCAL (long) ActiveXCallbackProc( paige_rec_ptr pg, pg_embed_ptr embed_ptr,
		long embed_type, short command, long user_refcon, long param1,
			long param2 )
{
	long result = 0;
	PgCntrItem* pItem = 0;

	switch (command)
	{
		case EMBED_DRAW:
		{
			CRect theBox;
			RectangleToRect( (rectangle_ptr)param1, 0, &theBox );
			HDC hdc = (HDC)pg->globals->current_port->machine_ref;
			CDC* pdc = CDC::FromHandle( hdc );

			if ( !(pItem = (PgCntrItem*) embed_ptr->user_refcon) ) {
				// haven't bound to the activex control yet, so let's get busy.
				// if we fail we'll bind to our "something sucks" control,
				// which will handle ui for error conditions.
				HtmlObjectSpec* pSpec = (HtmlObjectSpec*) embed_ptr->user_data;
				pItem = PgBindToObject( pg, pSpec );

				if ( pItem ) {
					embed_ptr->user_refcon = (long) pItem;
				}
				else {
					// TODO: need some way to flag the embed as dead
					// break out now, we're completely screwed
					break;
				}
			}

			if ( !pItem->IsInPlaceActive() )
				pItem->Draw( pdc, &theBox );

			pItem->activatePos = theBox;
		}
			break;
		case EMBED_DOUBLECLICK:
		case EMBED_MOUSEDOWN:
		{
			pg_embed_click_ptr click = (pg_embed_click_ptr) param1;

			CRect rc;
			RectangleToRect( &click->bounds, 0, &rc );
			PgCntrItem* pItem = (PgCntrItem*) embed_ptr->user_refcon;
			if ( pItem )
				pItem->activatePos = rc;

			// fall through
		}
		case EMBED_MOUSEMOVE:
		case EMBED_MOUSEUP:
			result = pgDefaultEmbedCallback( pg, embed_ptr, embed_type,
				command, user_refcon, param1, param2 );
			break;
		case EMBED_DESTROY:
		{
			// smoke ole item and the object spec, let paige do the rest
			HtmlObjectSpec* pSpec = (HtmlObjectSpec*) embed_ptr->user_data;
			delete pSpec;

			PgCntrItem* pItem = (PgCntrItem*) embed_ptr->user_refcon;
			if ( pItem )
				pItem->Delete();

			result = pgDefaultEmbedCallback( pg, embed_ptr, embed_type, command,
						user_refcon, param1, param2 );
		}
			break;

		default:
			result = pgDefaultEmbedCallback( pg, embed_ptr, embed_type,
						command, user_refcon, param1, param2 );
			break;
	}

	return result;
}


///////////////////////////////////////////////////////////////////////////////
// public interfaces

// force immediate loading of all items
int PgLoadAllObjects( pg_ref pg )
{
	// might as well do this outside the loop
	paige_rec_ptr pgRec;
	if ( !(pgRec = (paige_rec_ptr) UseMemory( pg )) )
		return 0;

	int nObjects = 0;

	embed_ref er;
	pg_embed_ptr pe;
	HtmlObjectSpec* pSpec;
	PgCntrItem* pItem;

	for ( long pos = 0; (er = pgFindNextEmbed( pg, &pos, 0, 0 )); pos += 2 ) {
		if ( er && (pe = (pg_embed_ptr)UseMemory(er)) ) {
			if ( embed_type(pe) != embed_url_image )
				if ( pSpec = (HtmlObjectSpec*) pe->user_data )
					if ( pItem = PgBindToObject( pgRec, pSpec ) ) {
						pe->user_refcon = (long) pItem;
						nObjects++;
					}
		}

		UnuseMemory( er );
	}

	UnuseMemory( pg );
	return nObjects;
}


// handy-dandy "KindOf" boogie
#define is_kind_of(class_name, object) \
	((object)->IsKindOf(RUNTIME_CLASS(class_name)))


// user inserts ole item from common dialog interface
//
// BOG: this is just some hacked up stuff used for prototyping. it does most
// of what you need to do user-inserts of ole objects, but i'm thinking that's
// not gonna fly for 5.0 anyway, so let's just "def" it.
#if 0
bool PgUserInsertObject( CView* pView, pg_ref pg, short draw_mode )
{
//    - must be able to get COleDocument from view [pv->GetDocument()]
//    - for now we let object dictate aspect, display bounds, etc.

	bool bRet = false;

#if 1
	// Invoke the standard Insert Object dialog box to obtain information
	//  for new PgCntrItem object.
	COleInsertDialog dlg;
	if (dlg.DoModal() != IDOK)
		return bRet;
#endif

	COleDoc* pDoc = (COleDoc*) pView->GetDocument();
	ASSERT_KINDOF( COleDocument, pDoc );

	if ( is_kind_of(COleDoc, pDoc) ) {
		TRY
		{
			pView->BeginWaitCursor();
			pItem = DEBUG_NEW PgCntrItem( pDoc );

#if 1
			// Initialize the item from the dialog data.
			if (!dlg.CreateItem(pItem))
				AfxThrowMemoryException();  // any exception will do
			ASSERT_VALID(pItem);

			// If item created from class list (not from file) then launch
			//  the server to edit the item.
			if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
				pItem->m_activateNow = true;
			else
				pItem->m_activateNow = false;
#else
			CLSID uid;
			USES_CONVERSION;
			CLSIDFromProgID( A2COLE("SNUFFLELUMPS.SnufflelumpsCtrl.1"), &uid );
//			CLSIDFromProgID( A2COLE("MediaPlayer.MediaPlayer.1"), &uid );
			pItem->CreateNewItem( uid );
			pItem->Run();
			pItem->m_activateNow = false;
#endif

			// Create a custom ref, but if we specify embed_user_data
			// then OpenPaige will attach the data to the ref.
			paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( pg );
			embed_ref ref = pgNewEmbedRef( pgRec->globals->mem_globals, embed_user_box,
					0, 0, 0, 0, 0, FALSE );

			UnuseMemory( pg );

// TODO: need a memory exception here

			// The following code is vital for a "custom" user type since
			// OpenPaige has no idea how tall our embed item is, nor does
			// it know how wide it is:

			CSize ext;
			pItem->GetExtent( &ext );
			pView->GetDC()->HIMETRICtoDP( &ext );

			pg_embed_ptr embed_ptr = (pg_embed_ptr) UseMemory( ref );
			embed_ptr->height = ext.cy;
			embed_ptr->width = ext.cx;

			// stash a ptr to our ole object
			embed_ptr->user_data = (long) pItem;
			UnuseMemory(ref);

			// Insert the ref. (Also add pgPrepareUndo() here if desired).
			pgInsertEmbedRef( pg, ref, CURRENT_POSITION, 0, ActiveXCallbackProc,
						/*refCon*/0, draw_mode );

			bRet = true;
			pView->EndWaitCursor();
		}
		CATCH(CException, e)
		{
			if (pItem != NULL)
			{
				ASSERT_VALID(pItem);
				pItem->Delete();
			}
		}
		END_CATCH
	}

	if ( pItem->m_activateNow )
		pItem->DoVerb(OLEIVERB_SHOW, pView);

	pItem->m_activateNow = false;
	return bRet;
}
#endif

// PgBindToObject: this tries to load and initialize an activex control based
// on an HtmlObjectSpec. If we fail, we bind to our "something sucks"
// activex control instead [not impl yet]
//
// BOG: if you don't pass this thing a paige record with a good StuffBucket,
// it ain't gonna work worth stuff. in this case, "good StuffBucket" means
// one with a back-ptr to a CPaigeEdtView derrived view.

PgCntrItem* PgBindToObject( paige_rec_ptr ppg, HtmlObjectSpec* pSpec )
{
	PgCntrItem* pItem = 0;   // return this
	CView* pView = 0;        // our window on the world
	COleDoc* pDoc = 0;  // view's doc: holds list of all embedded objects

	// get a good (ole) doc ptr
	PgStuffBucket* pSB = (PgStuffBucket*) ppg->user_refcon;
	assert( pSB );

	if ( pSB ) {
		pView = (CView*) pSB->pWndOwner;
		ASSERT_KINDOF( CView, pView );

		if ( pView ) {
			pDoc = (COleDoc*) pView->GetDocument();
			ASSERT_KINDOF( COleDoc, pDoc );
		}
	}

	// ok, provided that all went well, we'll try to load the control and add
	// it too the doc's container items.
	if ( pDoc && is_kind_of(COleDoc, pDoc) ) {
		pItem = DEBUG_NEW_MFCOBJ_NOTHROW PgCntrItem( pDoc );

		if ( pItem ) {
			USES_CONVERSION;

			CLSID uid;
			CLSIDFromString( A2OLE(pSpec->classid), &uid );

			if ( pItem->CreateNewItem( uid ) ) {
				pItem->m_activateNow = false;
				pItem->m_idString = pSpec->id;
				pItem->m_pg = ppg->doc_pg;
				pItem->SetFont( pDoc->GetFontDispatch() );

				// if size isn't specified, let's hope the control has a clue
				if ( !pSpec->width || !pSpec->height ) {
					CSize defExt;
					pItem->GetExtent( &defExt );
					pView->GetDC()->HIMETRICtoDP( &defExt );

					if ( !pSpec->width )
						pSpec->width = defExt.cx;

					if ( !pSpec->height )
						pSpec->height = defExt.cy;
				}

				// set/reset the control's extent
				CSize ext( pSpec->width, pSpec->height );
				pView->GetDC()->DPtoHIMETRIC( &ext );
				pItem->SetExtent( ext );
			}
			else {
				// TODO: bind to "default" control for error handling
				// for now just bail, leaving an unhandled user_box
				pItem->Delete();
				pItem = 0;
			}
		}
	}

	return pItem;
}
