// PgTextTranslator.cpp
// Specialization of the PaigeImportFilter class

#include "stdafx.h"
#include "PgTextTranslator.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


///////////////////////////////////////////////////////////////////////////////

// pgPrepareImport:
// Set anything we need right before the data is imported. For now, all we
// need do is make sure that we always use the current font, style, and
// paragraph info.

pg_error PgTextImportFilter::pgPrepareImport()
{
	PaigeImportFilter::pgPrepareImport();

	font_info fontInfo, fiMask;
	style_info styleInfo, siMask;
	par_info parInfo, piMask;

	pg_ref pg = original_pg ? original_pg : import_pg;

	pgGetFontInfo( pg, NULL, false, &fontInfo, &fiMask );
	pgGetStyleInfo( pg, NULL, false, &styleInfo, &siMask );
	pgGetParInfo( pg, NULL, false, &parInfo, &piMask );

	translator.font = fontInfo;
	translator.format = styleInfo;
	translator.par_format = parInfo;

	return NO_ERROR;
}
