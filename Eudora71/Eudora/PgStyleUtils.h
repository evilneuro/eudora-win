// PgStyleUtils.h
//
// PgStyleUtils is a collection of routines for doing all sorts of interesting
// style-related stuff -- converting LOGFONTS to associated paige structs for
// instance.
//
// Naming convention for the routines varies from Paige's in two ways: an
// underscore precedes the routine name, and each word in the rouinte name is
// capitalized. The names are similar to Paige in that the substring "pg"
// appears at the beginning of each name; hence the underscore.
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#ifndef _PG_STYLE_UTILS_H_
#define _PG_STYLE_UTILS_H_

#include "paige.h"


extern "C" {
void PgConvertLogFont( pg_ref pg, pg_globals_ptr pgGlobals, LOGFONT PG_FAR* logFont,
			font_info_ptr font, style_info_ptr style, style_info_ptr styleMask );

bool PgHasStyledText( pg_ref pg, pg_globals_ptr globals, select_pair_ptr sel=NULL, bool* pInternalStylesOnly = NULL );
long PgScanBackwardsToJustAfterBlank(pg_ref paigeRef, long nStartOffset);

}

#endif	// _PG_STYLE_UTILS_H_
