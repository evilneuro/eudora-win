// PgTextTranslator.h
// Specialization of the PaigeImportFilter class

#ifndef _PG_TEXTTRANSLATOR_H_
#define _PG_TEXTTRANSLATOR_H_

#include "Paige.h"
#include "pgEmbed.h"
#include "pgTxr.h"
#include "pgTxrCPP.h"


class PgTextImportFilter : public PaigeImportFilter
{
public:
	PgTextImportFilter(){};
	~PgTextImportFilter(){};

	virtual pg_error pgPrepareImport();
};

#endif	// _PG_TEXTTRANSLATOR_H_