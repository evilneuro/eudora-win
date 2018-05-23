#ifndef PGREGION_H
#define PGREGION_H

/* This file provides cross-platform macros for region manipulation.
PORTING NOTE: If there is no equivilent API for any of these macros, make it a function
and created the emulating code in your platform-specific code. */


#include "CPUDEFS.H"

#ifdef MAC_PLATFORM
#define pgCreateRgn()  NewRgn()
#define pgSectRgn(rgn1, rgn2, rgn3) SectRgn(rgn1, rgn2, rgn3)
#define pgUnionRgn(rgn1, rgn2, rgn3) UnionRgn(rgn1, rgn2, rgn3)
#define pgSubRgn(rgn1, rgn2, rgn3) DiffRgn(rgn1, rgn2, rgn3)
#define pgDisposeRgn(rgn) DisposeRgn(rgn)
#define pgEmptyRgn(rgn) EmptyRgn(rgn)
#define pgSetEmptyRgn(rgn) SetEmptyRgn(rgn)
#define pgCopyRgn(rgn1, rgn2) CopyRgn(rgn1, rgn2)
#define pgRectToRgn(rgn, r) RectRgn(rgn, r)
#define pgOffsetRgn(rgn, h, v) OffsetRgn(rgn, h, v)
#define pgInsetRgn(rgn, h, v) InsetRgn(rgn, h, v)

#endif


#ifdef WINDOWS_PLATFORM

#define pgCreateRgn() CreateRectRgn(0,0,0,0)
#define pgSectRgn(rgn1, rgn2, rgn3) CombineRgn((HRGN)rgn3, (HRGN)rgn1, (HRGN)rgn2, RGN_AND)
#define pgUnionRgn(rgn1, rgn2, rgn3) CombineRgn((HRGN)rgn3, (HRGN)rgn1, (HRGN)rgn2, RGN_OR)
#define pgSubRgn(rgn1, rgn2, rgn3) CombineRgn((HRGN)rgn3, (HRGN)rgn1, (HRGN)rgn2, RGN_DIFF)
#define pgDisposeRgn(rgn) DeleteObject(rgn)
#define pgSetEmptyRgn(rgn) SetRectRgn((HRGN) rgn, 0, 0, 0, 0)
#define pgCopyRgn(rgn1, rgn2) CombineRgn((HRGN) rgn2, (HRGN) rgn1, (HRGN) rgn1, RGN_COPY)
#define pgOffsetRgn(rgn, h, v) OffsetRgn((HRGN)rgn, h, v)

#ifdef __cplusplus
extern	"C" {
#endif

extern PG_PASCAL (void)pgRectToRgn(pg_region rgn, RECT PG_FAR *r);
extern PG_PASCAL (void) pgInsetRgn (pg_region rgn, short dh, short dv);
extern PG_PASCAL (pg_boolean) pgEmptyRgn(pg_region rgn);

#ifdef __cplusplus
	}
#endif

#endif
#endif

