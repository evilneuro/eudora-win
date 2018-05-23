// PgEmbeddedObject.h -- ole container support for Paige

#if !defined(_PG_EMBEDDED_OBJECT_H_)
#define _PG_EMBEDDED_OBJECT_H_

class PgCntrItem;
struct HtmlObjectSpec;

bool PgUserInsertObject( CView* pView, pg_ref pg, short draw_mode );
int PgLoadAllObjects( pg_ref pg );
PgCntrItem* PgBindToObject( paige_rec_ptr ppg, HtmlObjectSpec* pSpec );

PG_PASCAL (long) ActiveXCallbackProc( paige_rec_ptr pg, pg_embed_ptr embed_ptr,
		long embed_type, short command, long user_refcon, long param1,
			long param2 );
#endif
