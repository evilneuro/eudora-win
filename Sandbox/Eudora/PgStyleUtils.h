#ifndef _PG_STYLE_UTILS_H_
#define _PG_STYLE_UTILS_H_

#include "paige.h"

// file: PgStyleUtils.h
//
// PgStyleUtils is a collection of routines for doing all sorts of interesting
// style-related stuff -- converting LOGFONTS to associated paige structs for
// instance.
//
// Naming convention for the routines varies from Paige's in two ways: an
// underscore precedes the routine name, and each word in the rouinte name is
// capitalized. The names are similar to Paige in that the substring "pg"
// appears at the beginning of each name; hence the underscore.


extern "C" {
void PgConvertLogFont( pg_ref pg, pg_globals_ptr pgGlobals, LOGFONT PG_FAR* logFont,
			font_info_ptr font, style_info_ptr style, style_info_ptr styleMask );

bool PgHasStyledText( pg_ref pg, pg_globals_ptr globals, select_pair_ptr sel=NULL, bool* pExcerptCausedStyle = NULL );

}

#endif	// _PG_STYLE_UTILS_H_
