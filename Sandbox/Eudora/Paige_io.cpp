// Paige_io.cpp
// Input/Output/Data Translation services for Paige objects.

#include "stdafx.h"
#include "Paige_io.h"
#include "PgTextTranslator.h"
#include "PgEmbeddedImage.h"
#include "pgosutl.h"
#include "rs.h"
#include "eudora.h"
#include "PgHLinks.h"

// MyHTML
#include "PaigeStyle.h"
#include "pgHTMDef.h"
#include "resource.h"

//TEMP for old sources
#include "pgUtils.h"

#define DBGMSG_BAD_DATAFORMAT \
	OutputDebugString( _T("Dude! PgDataTranslator: Invalid Data Format.") )

#define PAIGE "PAIGE"
#define CF_HTML "HTML Format"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// these may need to be registered in some common place, so that non-paige-aware
// code can make use of them. they can just be "externed" from wherever.

const PgDataTranslator::FormatType PgDataTranslator::kFmtText = CF_TEXT;

const PgDataTranslator::FormatType PgDataTranslator::kFmtNative =
				RegisterClipboardFormat(_T(PAIGE));

const PgDataTranslator::FormatType PgDataTranslator::kFmtRtf =
				RegisterClipboardFormat(_T(CF_RTF));

const PgDataTranslator::FormatType PgDataTranslator::kFmtHtml =
				RegisterClipboardFormat(_T(CF_HTML));


const long g_importFlags = IMPORT_EVERYTHING_FLAG;


int pgGetDefaultPointSize()
{
	short currSize = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);
	
	return GetFontSizeMap()[currSize][2];
}

// temporary - remove with port to Eudora
//extern pgm_globals	g_pgMemoryGlobals;
//extern pg_globals		g_pgGlobals;

///////////////////////////////////////////////////////////////////////////////
// PgDataTranslator - Paige memory io

// ExportData:
// Translates Paige contents to a Paige allocated memory reference

void PgDataTranslator::ExportData( memory_ref* expMemRef,
					FormatType fmt /* = kFmtText */ )
{
	PaigeExportObject exportFilter;
	if ( MakeTranslator( &exportFilter, fmt ) ) {
	
		// Allocate zero byte sizes for the export memory-ref (1 byte rec-size, 0 recs)
		memory_ref pgmRef = MemoryAlloc( PgMemGlobalsPtr(), 1, 0, 0 );

		pg_error pgError = exportFilter->pgInitExportFile( PgGlobalsPtr(),
						NULL, pgmRef, NULL, 0);

		// Export from current document into paige memory references
		pgError = exportFilter->pgExportFile(m_pgRef, NULL,
						EXPORT_EVERYTHING_FLAG, FALSE);

		*expMemRef = pgmRef;
		delete exportFilter;
	}
}


// ImportData:
// Translates data from a Paige allocated memory reference

void PgDataTranslator::ImportData( memory_ref* impMemRef,
					FormatType fmt /* = kFmtText */ )
{
	PaigeImportObject importFilter;
	pg_error error;

	if ( MakeTranslator( &importFilter, fmt ) ) {

		//A NULL character is appended in case of text import. So copy until last-1
		if ( fmt == kFmtText ) {
			long lastPosition = GetByteSize(*impMemRef);
			error = importFilter->pgInitImportFile(PgGlobalsPtr(), NULL, 
							*impMemRef, NULL, 0, lastPosition-1);
		} 
		else {
			error = importFilter->pgInitImportFile(PgGlobalsPtr(), NULL, 
							*impMemRef, NULL, 0, UNKNOWN_POSITION);
		}

		long importFlags = g_importFlags;
		if ( fmt == kFmtHtml )
			importFlags ^= IMPORT_BKCOLOR_FLAG;
			
		error = importFilter->pgImportFile(m_pgRef, CURRENT_POSITION, 
							importFlags, FALSE, best_way );

		if ( fmt == kFmtHtml )
			PgInitHyperlinks( m_pgRef );

		delete importFilter;
	}
}


///////////////////////////////////////////////////////////////////////////////
// PgDataTranslator - UDT (OLE)

// Export:
// Translates Paige contents/selection as OLE data source

void PgDataTranslator::ExportData( COleDataSource* pDataSource,
						FormatType fmt /* = kFmtText */ )
{
	PaigeExportObject exportFilter;
	if ( MakeTranslator( &exportFilter, fmt ) ) {
	
		// Allocate zero byte sizes for the export memory-ref (1 byte rec-size, 0 recs)
		memory_ref exportMemRef = MemoryAlloc( PgMemGlobalsPtr(), 1, 0, 0 );

		pg_error pgError = exportFilter->pgInitExportFile (PgGlobalsPtr(), NULL, exportMemRef, NULL, 0);

		// Export from current document into paige memory references
		pgError = exportFilter->pgExportFile(m_pgRef, NULL, EXPORT_EVERYTHING_FLAG, TRUE);

		// "Lock" the Paige memories and get the memory addresses
		void PG_FAR* exportMemPtr = UseMemory(exportMemRef);

		// Alloc required Global (share) memory and Lock it down
		long exportByteSize = GetByteSize(exportMemRef);

		//Add one byte if text since the last char keeps getting chopped off
		if (fmt == PgDataTranslator::kFmtText)
			++exportByteSize;

		HGLOBAL pghGlobal= GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, exportByteSize + 1);
		void PG_FAR* pgGlobalMem = GlobalLock(pghGlobal);
		
		// Copy from Paige memory block to the Global memory block
		if (fmt == PgDataTranslator::kFmtText)
			memcpy(pgGlobalMem, exportMemPtr, exportByteSize-1);
		else
			memcpy(pgGlobalMem, exportMemPtr, exportByteSize);

		// Create COleDataSource objects to copy to clipboard
		pDataSource->CacheGlobalData( (CLIPFORMAT)fmt, pghGlobal, NULL );

		// Cleanup Paige Block and unlock the global memory
		GlobalUnlock(pghGlobal);
		UnuseAndDispose(exportMemRef);
		delete exportFilter;
	}
}


// ImportData:
// Translates OLE data object into Paige format

void PgDataTranslator::ImportData( COleDataObject* pDataObj,
						FormatType fmt /* = kFmtText */ )
{
	PaigeImportObject importFilter;
	pg_error error;
	memory_ref importMemRef; 

	if ( MakeTranslator( &importFilter, fmt ) ) {

		HGLOBAL pgImportGlobal = 0;
		STGMEDIUM stg;

		//Data may be returned as HGLOBAL/ISTREAM-ptr. 
		//We do not handle the other storage mediums.

		pgImportGlobal = pDataObj->GetGlobalData( (CLIPFORMAT)fmt, NULL);

		if ( pgImportGlobal )
		{
			//if ((fmt == kFmtText) || (fmt == kFmtRtf))
			if ( /*( IsWin95() || (pgTextSize(m_pgRef)) ) && */( (fmt == kFmtText) || (fmt == kFmtRtf) ) )
			{
				//On win95, the Global size of the HGlobal returned from
				//the OLE function GetGlobalData is bogus. Hence the
				//Paige translator was translating more than it was
				//supposed to. So we go the CString way and copy
				//only upto the first NULL char in the memory.
				LPCSTR textStr = (LPCSTR) GlobalLock(pgImportGlobal);
				GlobalUnlock(pgImportGlobal);
				importMemRef = CString2Memory( PgMemGlobalsPtr(), (LPCSTR)textStr );
			}
			else
				//All other formats can use this
				importMemRef = HandleToMemory( PgMemGlobalsPtr(), pgImportGlobal, 1 );
		}
		else
		{
			pDataObj->GetData( (CLIPFORMAT)fmt, &stg, NULL );
		
/*			if (stg.tymed == TYMED_HGLOBAL)
			{
				if (! pgImportGlobal)
					pgImportGlobal = stg.hGlobal;
				
				importMemRef = HandleToMemory( PgMemGlobalsPtr(), pgImportGlobal, 1 );

				if (pgImportGlobal == NULL) 
				{
					ASSERT(0);
					return;
				}

			}
			else */if (stg.tymed == TYMED_ISTREAM)
			{
				IStream *piStream = stg.pstm;
				
				HRESULT hr = S_OK;
				LARGE_INTEGER pOffset;
				ULARGE_INTEGER pBegin, pEnd, diff;
				ULONG bytesRead;

				pOffset.QuadPart = 0;

				hr = piStream->Seek(pOffset, SEEK_SET, &pBegin);
				ASSERT(S_OK == hr);

				piStream->Seek(pOffset, SEEK_END, &pEnd);
				ASSERT(S_OK == hr);

				diff.QuadPart = pEnd.QuadPart - pBegin.QuadPart;
				
				//Set the Seek ptr to the beginning before reading
				hr = piStream->Seek(pOffset, SEEK_SET, &pBegin);
				ASSERT(S_OK == hr);
				
				char *text = new char[static_cast<int>(diff.QuadPart)];
				piStream->Read(text, static_cast<unsigned long>(diff.QuadPart), &bytesRead);

				if (! bytesRead)
				{
					ASSERT(0);
					return;
				}

				piStream->Release();

				importMemRef = Bytes2Memory( PgMemGlobalsPtr(),
						(pg_char_ptr)text, sizeof(pg_char), static_cast<long>(diff.QuadPart));

				delete [] text;
			}
			else 
			{
				//We do not handle other storage mediums
				ASSERT(0);
				return;
			}
		}	
		// Paige implementation doesn't seem to handle selected text correctly, 
		//  so let's perform the deletion ourselves.
		//if (pgNumSelections(m_pgRef))
		//	pgDelete(m_pgRef, NULL, draw_none);

		//A NULL character is appended in case of text import. So copy until last-1
		if ( fmt == kFmtText ) {
			long lastPosition = GetByteSize(importMemRef);
			error = importFilter->pgInitImportFile(PgGlobalsPtr(), NULL, 
							importMemRef, NULL, 0, lastPosition-1);
		} 
		else {
			error = importFilter->pgInitImportFile(PgGlobalsPtr(), NULL, 
							importMemRef, NULL, 0, UNKNOWN_POSITION);
		}

;
		long importFlags = g_importFlags;
		importFlags ^= IMPORT_HIDDENTEXT_FLAG;
		if ( fmt == kFmtHtml )
			importFlags ^= IMPORT_BKCOLOR_FLAG;

		error = importFilter->pgImportFile(m_pgRef, CURRENT_POSITION, 
							importFlags, FALSE, best_way);

		if ( fmt == kFmtHtml )
			PgInitHyperlinks( m_pgRef );

		// Clean up
		DisposeMemory(importMemRef);	
		delete importFilter;
	}
}

///////////////////////////////////////////////////////////////////////////////
// PgDataTranslator - MFC File I/O

// ExportData:
// Translates Paige contents to specified CFile object

void PgDataTranslator::ExportData( CFile* pFile, FormatType fmt /* = kFmtText */ )
{
	ExportData( pFile->m_hFile, fmt );
}


// ImportData:
// Translates from specified CFile object to Paige

void PgDataTranslator::ImportData( CFile* pFile, FormatType fmt /* = kFmtText */ )
{
	ImportData( pFile->m_hFile, fmt );
}

///////////////////////////////////////////////////////////////////////////////
// PgDataTranslator - WinAPI File I/O

// ExportData:
// Translates Paige contents to HFILE return from OpenFile

void PgDataTranslator::ExportData( HFILE hFile, FormatType fmt /* = kFmtText */ )
{
	// get a filter object for the target datatype
	PaigeExportObject exportFilter;
	if ( MakeTranslator( &exportFilter, fmt ) ) {

		// set-up the export filter
		pg_error pgError = exportFilter->pgInitExportFile( PgGlobalsPtr(), hFile,
						NULL, NULL, 0 );

		// Export from current document into paige memory references
		pgError = exportFilter->pgExportFile( m_pgRef, NULL,
						EXPORT_EVERYTHING_FLAG, FALSE );

		delete exportFilter;
	}
}


// ImportData:
// Translates from HFILE returned from OpenFile to Paige

void PgDataTranslator::ImportData( HFILE hFile, FormatType fmt /* = kFmtText */ )
{
	pg_error error;
	PaigeImportObject importFilter;
	if ( MakeTranslator( &importFilter, fmt ) ) {

		// Paige implementation doesn't seem to work, so let's do it ourselves.
		//pgDelete(m_pgRef, NULL, draw_none);
		
		error= importFilter->pgInitImportFile(PgGlobalsPtr(), hFile, 
											NULL, NULL, 0, UNKNOWN_POSITION);

		long importFlags = g_importFlags;
		if ( fmt == kFmtHtml )
			importFlags ^= IMPORT_BKCOLOR_FLAG;

		error = importFilter->pgImportFile(m_pgRef, CURRENT_POSITION, 
										importFlags, FALSE, draw_none);

		if ( fmt == kFmtHtml )
			PgInitHyperlinks( m_pgRef );

		delete importFilter;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Private Parts

// MakeTranslator:
// Constructs the actual Paige Import Filter -- caller is responsible for
// destroying this object when done with it.

bool PgDataTranslator::MakeTranslator( PaigeExportObject* ppTxr,
					FormatType fmt /* = kFmtText */ )
{
	if ( fmt == kFmtText ) {
		*ppTxr = new PaigeExportFilter(); 
	}
	else if ( fmt == kFmtRtf ) {
		*ppTxr = (PaigeExportObject) new PaigeRTFExportFilter(); 
	}
	else if ( fmt == kFmtNative ) {
		*ppTxr = (PaigeExportObject) new PaigeNativeExportFilter(); 
	}
	else if ( fmt == kFmtHtml ) {
		*ppTxr = (PaigeExportObject) new CMyPaigeHTMLExportFilter();
	}
	else {
		DBGMSG_BAD_DATAFORMAT;
		ASSERT( 0 );
	}

	return true;
}


bool PgDataTranslator::MakeTranslator( PaigeImportObject* ppTxr,
					FormatType fmt /* = kFmtText */ )
{
	if ( fmt == kFmtText ) {
//		*ppTxr = new PaigeImportFilter(); 
		*ppTxr = new PgTextImportFilter(); 
	}
	else if ( fmt == kFmtRtf ) {
		*ppTxr = (PaigeImportObject) new PaigeRTFImportFilter(); 
	}
	else if ( fmt == kFmtNative ) {
		*ppTxr = (PaigeImportObject) new PaigeNativeImportFilter(); 
	}
	else if ( fmt == kFmtHtml ) {
		*ppTxr = (PaigeImportObject) new CMyPaigeHTMLImportFilter();
	}
	else {
		DBGMSG_BAD_DATAFORMAT;
		ASSERT( 0 );
	}

	return true;
}

// ===================================================

CMyPaigeHTMLImportFilter::CMyPaigeHTMLImportFilter()
{
	current_point_level = base_font_index = 3;
	
	short currSize = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);

	FontSizeMapType& FontSizeMap = GetFontSizeMap();
		
	point_levels[0] = FontSizeMap[currSize][0];
	point_levels[1] = FontSizeMap[currSize][1];
	point_levels[2] = FontSizeMap[currSize][2];
	point_levels[3] = FontSizeMap[currSize][3];
	point_levels[4] = FontSizeMap[currSize][4];
	point_levels[5] = FontSizeMap[currSize][5];
	point_levels[6] = FontSizeMap[currSize][6];
	
	base_url_string[0] = '\0';
	return;
}


#define P2F	<<16L
static FontSizeMapType aFontSizeMap = {
	{  7 P2F,	 8 P2F,	 9 P2F,	10 P2F,	12 P2F, 16 P2F, 24 P2F},
	{  7 P2F,	 9 P2F,	10 P2F,	12 P2F, 16 P2F, 22 P2F, 28 P2F},
	{  8 P2F,	10 P2F,	11 P2F,	14 P2F, 18 P2F, 24 P2F, 36 P2F},
	{ 10 P2F,	12 P2F,	14 P2F,	16 P2F, 20 P2F, 26 P2F, 42 P2F},
	{ 11 P2F,	14 P2F,	16 P2F,	18 P2F, 22 P2F, 28 P2F, 48 P2F} };

// Reload the table of font sizes, and return a pointer to it
FontSizeMapType& GetFontSizeMap()
{
	short tableIDs[NUM_FONT_SIZE_TABLES] = {
		IDS_INI_FONT_SIZE_TABLE_SMALLEST,
		IDS_INI_FONT_SIZE_TABLE_SMALL,
		IDS_INI_FONT_SIZE_TABLE_MEDIUM,
		IDS_INI_FONT_SIZE_TABLE_LARGE,
		IDS_INI_FONT_SIZE_TABLE_LARGEST,
	};

	// Read the table, one line at a time
	for (int idIndex=0;idIndex<NUM_FONT_SIZE_TABLES;idIndex++)
	{
		char iniString[NUM_FONT_SIZE_ENTRIES*5];
		
		GetIniString(tableIDs[idIndex],iniString,sizeof(iniString));
		if (*iniString)
		{
			// ok, we found a string.  Now read the (comma-delimited) sizes
			for (int spotIndex=0;spotIndex<NUM_FONT_SIZE_ENTRIES;spotIndex++)
			{
				char *token = strtok(spotIndex?NULL:iniString,",");
				if (!token) break;	// if no entry, give up
				int theSize = atoi(token);
				if (!theSize) continue;	// if zero, go on to next one
				aFontSizeMap[idIndex][spotIndex] = (long)theSize P2F;	// Paige likes Fixed
			}
		}
	}

	return aFontSizeMap;
}

static tag_attribute_ptr find_parameter (tag_attribute_ptr params, long num_params, short wanted_code)
{
   register tag_attribute_ptr param_ptr;
   long                 index;

   param_ptr = params;

   for (index = 0; index < num_params; ++index, ++param_ptr)
      if (param_ptr->code == wanted_code)
         return   param_ptr;


   return   NULL;
}

pg_boolean CMyPaigeHTMLImportFilter::PreProcessTag(short tag_type, short tag, tag_attribute_ptr params, long num_params)
{
	pg_boolean		ret = FALSE;
	
	if (tag_type == STYLE_COMMAND && 
		((tag == blockquote_style_command) || (tag == -blockquote_style_command)) )
	{
		blockQuoteStyle bqStyle = plain;

		// Differentiate this end blockquote from an excerpt blockquote
		if (tag == -blockquote_style_command )
		{
			if ((blockQuoteStack.size()) && (blockQuoteStack.top() != bqStyle))
			{
			DumpPendingText();
			if (ret = CPaigeStyle::ApplyExcerpt(&current_par, FALSE))
			{
				if ( !doing_table )
					ApplyInputParagraph(&current_par, -1, FALSE);
//		        if (!table_level)
//			        ApplyInputParagraph(&current_par, -1, FALSE);

			}
			}
			if ( blockQuoteStack.size() )
				blockQuoteStack.pop();
		}
		else
		{
			long param_index;
			tag_attribute_ptr param_ptr;
			for (param_index = 0, param_ptr = params; param_index < num_params; ++param_ptr, ++param_index) 
			{ 
				if (tag == blockquote_style_command)
				{
					BOOL bIsOutlookExcerpt = FALSE;

					if (param_ptr->code == 5)
					{
						char outlookExcerpt[255];
						outlookExcerpt[0] = 0;
						GetIniString(IDS_INI_OUTLOOK_EXCERPT_TAG, outlookExcerpt, 254);
						int size = strlen(outlookExcerpt);

						if (size && (strnicmp( (char*)param_ptr->param,outlookExcerpt,size) == 0))
							bIsOutlookExcerpt = TRUE;
					}
					//Code is the offset(1 based) of "type"(6) OR "style"(5) in style_attributes defined in pghtmdef.c
					//type = cite is what we generate and style = border-left.... is what Outlook generates
					if ( bIsOutlookExcerpt ||
						 ( param_ptr->code == 6 && (strnicmp((char*)param_ptr->param,"cite",param_ptr->param_size) == 0 )) )
					{
						bqStyle = cite;
         				DumpPendingText();
						CPaigeStyle::ApplyExcerpt(&current_par,TRUE);
						translator.format = current_style;
   						if ( !doing_table )
//						if (!table_level)
							ApplyInputParagraph(&current_par, -1, FALSE);
						ret = TRUE;
						break;
					}
				}
			}

			blockQuoteStack.push(bqStyle);
		}
	}
	//
	// Strip images with http...
	//
#if 0
	else if (tag_type == DATA_COMMAND && tag == image_command)
	{
		tag_attribute_ptr argument;
		
		if ((argument = find_parameter(params, num_params, image_url_attribute)) && argument->param_size)
		{
			const char* prm = (const char*) argument->param;
			int prmSize = argument->param_size;

			if (!prmSize || (strnicmp(prm, "file:", 5) && strnicmp(prm, "cid:", 4)))
				ret = TRUE;
		}
	}
#endif
	//
	// TABLES
	//
	//TEMP comment out later for new Paige srcs
	else if ( tag_type == TABLE_DATA_COMMAND )
	{
		ret = TRUE;
		// Ensure that table rows begin and end on a new line
		if ( tag == table_row_command || tag == -table_row_command)
			BreakLine(FALSE, FALSE);
	}
	// Strip <DIV> tag and replace with CRLF
	else if (tag_type == CONTROL_COMMAND && (tag == div_command || tag == -div_command))
		BreakLine(FALSE, FALSE);
	else if ((tag_type == CONTROL_COMMAND) && (tag == base_url_command))
	{
         anchor_ref = MemoryDuplicate(param_ref);
		 
		 tag_attribute_ptr	params, argument;
		 long				num_params;

		 num_params = GetMemorySize(anchor_ref);
		 params = (tag_attribute_ptr)UseMemory(anchor_ref);

		 if ((argument = find_parameter(params, num_params, URL_attribute)) != NULL) 
		 {
			if (argument->param_size) 
			{
				strcpy( (char*)base_url_string, (const char*)argument->param );
			}
		 }

		 UnuseMemory(anchor_ref);
		 ret = TRUE;
	}
	//Dump all text between the style commands
	else if ( (tag_type == CONTROL_COMMAND) && (tag == style_command) )
	{
		SkipToEndTag(style_command);
		ret = TRUE;
	}

	return ret;
}


//TEMP : Implemented in new Paige Srcs
void CMyPaigeHTMLImportFilter::SkipToEndTag (short the_tag)
{
   pg_char           next_char;
   short			 access;
   
   access = GetAccessCtr(param_ref);
   
   if (access)
   		UnuseMemory(param_ref);

   while (pgGetImportChar(&next_char) == NO_ERROR) {
      
      if (next_char == TAG_COMMAND_CHAR) {
         long        	command_id;
         pg_boolean     end_tag;

         command_id = ReadHTMLTag(&end_tag);
         
         if (command_id ) {
            short                	table_id, tag;
            
            table_id = pgHiWord(command_id);
            tag = pgLoWord(command_id);
            
            if (end_tag && tag == the_tag)
            	break;
         }        
      }
   }
   
   if (access)
   		UseMemory(param_ref);
}


CMyPaigeHTMLExportFilter::CMyPaigeHTMLExportFilter()
{
	short current_point_level = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);
	
	FontSizeMapType& FontSizeMap = GetFontSizeMap();

	point_levels[0] = FontSizeMap[current_point_level][0];
	point_levels[1] = FontSizeMap[current_point_level][1];
	point_levels[2] = FontSizeMap[current_point_level][2];
	point_levels[3] = FontSizeMap[current_point_level][3];
	point_levels[4] = FontSizeMap[current_point_level][4];
	point_levels[5] = FontSizeMap[current_point_level][5];
	point_levels[6] = FontSizeMap[current_point_level][6];

	return;
}

void CMyPaigeHTMLExportFilter::PreProcessParChange (par_info_ptr old_style, par_info_ptr new_style,
				long doc_position)
{
	bool bOld =  CPaigeStyle::IsExcerpt(old_style);
	bool bNew =  CPaigeStyle::IsExcerpt(new_style);

	if ( !bOld && !bNew )
		return;

	char* szEnd = "</blockquote>";
	char* szBegin = "<blockquote type=cite cite>"; //LoadString(IDS_HTML_BLOCKQUOTE_CITE); //<blockquote type=cite>";
	char* s=NULL;
	BOOL bDoBreak = FALSE;
	if ( bOld && !bNew)
	{
		s = szEnd;
		bDoBreak = TRUE;
	}
	else if ( !bOld && bNew )
		s = szBegin;
	else if ( bOld && bNew )
	{
		if (old_style->user_data > new_style->user_data )
			s = szEnd;
		else if (old_style->user_data < new_style->user_data )
			s = szBegin;
	}

	int nDiff = abs(old_style->user_data - new_style->user_data );
	while ( nDiff-- > 0 )
		pgWriteString((unsigned char*)s,0,0);
	//
	// This represents part 2 of a hack to the paige export routines.
	// At the point where an excerpt ends and two carriage returns follow
	// (e.g. when we split an excerpt), paige wants to translate those to 
	// carriage return tags to a <p> tag.  When interpreted, the <p> belongs
	// the excerpt style and thus, being two carriage returns, makes the excerpt
	// extend too long.  The fix is to terminate the paragraph with a <br> (part 1)
	// insert the <blockquote> tag, then insert another <br> tag (part 2).
	//
	// CL: 11/16:  Paige no longer outputs consecutive carriage returns to end an excerpt.
	// Change was made in PGEXPORT.CPP.  This nullifies the need for this correction.
	//if ( bDoBreak )
	//	pgWriteString((unsigned char*)"<br>",0,0);
	return ;
}

/* EmulateTab sends out space char(s) to emulate a tab condition.
The chars_since_cr indicate the number of characters since the last hard break. */

void CMyPaigeHTMLExportFilter::EmulateTab (void)
{
	//Depending on the number of chars already input, add the required number of nbsp's
	int numTabStops = GetIniShort(IDS_INI_TAB_STOP);
	int numSpaces = numTabStops - (chars_since_cr % numTabStops);

	if (numSpaces == 0)
		numSpaces = numTabStops;

	WriteTag(control_commands, x_tab_command, TRUE, FALSE);
	
	//Output n nbsp's
	for (int i = 0; i < numSpaces; i++)
	{
		OutputSpecialChar(0xA0);
		chars_since_cr += 1;
	}

	WriteTag(control_commands, -x_tab_command, TRUE, FALSE);
}

