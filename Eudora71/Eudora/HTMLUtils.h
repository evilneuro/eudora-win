//	HTML utilities culled from Eudora's copy of PGHTMIMP.CPP, PgEmbeddedImage.cpp,
//	and PgHLinks.cpp.

#ifndef _HTMLUtils_H_
#define _HTMLUtils_H_

#include "PAIGE.H"


void					resolve_URL(
								pg_char_ptr				source_URL,
								pg_char_ptr				target_URL,
								size_t					nMaxLength);
pg_short_t				translate_hex(
								pg_char					hex_char);
void					unescape_url(
								char *					url);
char					x2c(
								char *					what);


#endif	//	_HTMLUtils_H_
