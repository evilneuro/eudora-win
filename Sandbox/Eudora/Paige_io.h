// Paige_io.cpp
// Input/Output/Data Translation services for Paige objects.

#ifndef _PAIGE_IO_H
#define _PAIGE_IO_H

#include "paige.h"
#include "pgTxrCPP.h"
#include "pgTraps.h"
#include "PgGlobals.h"

#include <xstddef>
#pragma warning (disable: 4663 4244 4018 4146)
#include <stack>
#pragma warning (default: 4663 4244 4018 4146)
using namespace std;

typedef stack<int> INT_STACK;

enum blockQuoteStyle { plain = 1, cite};


#define NUM_FONT_SIZE_TABLES 5
#define NUM_FONT_SIZE_ENTRIES 7
typedef int FontSizeMapType[NUM_FONT_SIZE_TABLES][NUM_FONT_SIZE_ENTRIES];
FontSizeMapType& GetFontSizeMap();
int pgGetDefaultPointSize();


class PgDataTranslator : private PgSharedAccess
{
public:
	// data tranfer types - these are just clipboard formats. they are registered
	// in Paige_io.cpp, but that could be done elsewhere.
	typedef UINT FormatType;
	static const FormatType kFmtText, kFmtRtf, kFmtNative, kFmtHtml;

private:
	pg_ref	m_pgRef;

	// can't make it like this!
	PgDataTranslator(){};

	// factory function for Paige "Txr"s
	bool MakeTranslator( PaigeExportObject* pTxr, FormatType fmt = kFmtText );
	bool MakeTranslator( PaigeImportObject* pTxr, FormatType fmt = kFmtText );

public:
	// must use this constructor
	PgDataTranslator( pg_ref pg ){ ASSERT( pg != 0 ); m_pgRef = pg; }

	// Paige memory io
	void ExportData( memory_ref* expMemRef, FormatType fmt = kFmtText );
	void ImportData( memory_ref* impMemRef, FormatType fmt = kFmtText );

	// OLE Uniform Data Transfer
	void ExportData( COleDataSource* pDataSource, FormatType fmt = kFmtText );
	void ImportData( COleDataObject* pDataObj, FormatType fmt = kFmtText );

	// MFC file io
	void ExportData( CFile* pFile, FormatType fmt = kFmtText );
	void ImportData( CFile* pFile, FormatType fmt = kFmtText );

	// WinAPI file io
	void ExportData( HFILE hFile, FormatType fmt = kFmtText );
	void ImportData( HFILE hFile, FormatType fmt = kFmtText );

	// Stream-based io, or whatever MIME Storage will use
	// TBD
};

class CMyPaigeHTMLImportFilter : public PaigeHTMLImportFilter
{
public:
	INT_STACK blockQuoteStack;

//TEMP Remove after updating to new Paige srcs
protected:
	void SkipToEndTag(short the_tag);
public:
	CMyPaigeHTMLImportFilter();

	//HTML specific Virtual Overrides
	pg_boolean PreProcessTag(short tag_type, short tag, tag_attribute_ptr params, long num_params);
};

class CMyPaigeHTMLExportFilter : public PaigeHTMLExportFilter
{
public:
	CMyPaigeHTMLExportFilter();

	void PreProcessParChange (par_info_ptr old_style, par_info_ptr new_style,
			long doc_position);
	void EmulateTab (void);
};

#endif	// _PAIGE_IO_H
