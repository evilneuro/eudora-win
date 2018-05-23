// Paige_io.cpp
// Input/Output/Data Translation services for Paige objects.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include "Paige_io.h"
#include "PgTextTranslator.h"
#include "PgEmbeddedImage.h"
#include "pgosutl.h"
#include "pgtxr.h"
#include "rs.h"
#include "eudora.h"
#include "PgHLinks.h"
#include "PgTextExport.h"
#include "msgutils.h"
#include "utils.h"

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



#include "DebugNewHelpers.h"

// Typedef a correct definition for a unicode character
typedef unsigned short UniChar; 

// these may need to be registered in some common place, so that non-paige-aware
// code can make use of them. they can just be "externed" from wherever.

const PgDataTranslator::FormatType PgDataTranslator::kFmtText = CF_TEXT;

const PgDataTranslator::FormatType PgDataTranslator::kFmtNative =
				RegisterClipboardFormat(_T(PAIGE));

const PgDataTranslator::FormatType PgDataTranslator::kFmtRtf =
				RegisterClipboardFormat(_T(CF_RTF));

const PgDataTranslator::FormatType PgDataTranslator::kFmtHtml =
				RegisterClipboardFormat(_T(CF_HTML));



UniChar UTF8ToUniChar(unsigned char *utf8, unsigned char **next)
{
	short r;
	/* Plain ASCII just return it */
	if(*utf8 <= 0x7F)
	{
		r = *utf8++;
	} else {
		int c;
		unsigned char b, m;
		
		/*
		 *	Count how many bytes there are; check how many high bits are set
		 *	and that'll be the byte count. Also, form the mask to grab the
		 *	low bits out of the first byte.
		 */
		for(c = 0, b = *utf8, m = 0x3F; ((b <<= 1) & 0x80); ++c)
			m >>= 1;
		
		/* Grab the low bits of the first byte */
		r =(short)( *utf8++ & m);
		while(c--)
		{
			/* Grab the low six bits of each byte and shift them into r */
			r <<= 6;
			r |= (*utf8++ & 0x3F);
		}
	}
	if(next) *next = utf8;
	return r;
}

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

PgDataTranslator::PgDataTranslator( pg_ref pg, long flags /*= 0*/ )
{
	ASSERT(pg != 0);
	m_pgRef = pg;

	if ( flags )
		m_importFlags = flags;
	else {
		m_importFlags = IMPORT_EVERYTHING_FLAG;

		// stuff we don't do by default
		m_importFlags ^= IMPORT_SIG_FORMAT_FLAG;

		if ( !GetIniShort(IDS_INI_IMPORT_TABLES) )
			m_importFlags ^= IMPORT_TABLES_FLAG;
		if ( !GetIniShort(IDS_INI_FETCH_INLINE_CONTENT) )
			m_importFlags ^= IMPORT_HTTP_IMAGES_FLAG;
	}
}


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
					FormatType fmt /* = kFmtText */,
					short draw_mode /*= best_way */)
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

		if ( fmt == kFmtHtml )
			m_importFlags ^= IMPORT_BKCOLOR_FLAG;
			
		error = importFilter->pgImportFile(m_pgRef, CURRENT_POSITION, 
							m_importFlags, FALSE, draw_mode );

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
	if ( MakeTranslator( &exportFilter, fmt, true ) ) {
		//	CF_HTML prefix, the eight digit zeros allow us to more easily calculate
		//	offsets which would otherwise vary as we formatted the numbers.
		const char *	kCF_HTMLPrefix = "Version:0.9\r\n"
										 "StartHTML:00000000\r\n"
										 "EndHTML:00000000\r\n"
										 "StartFragment:00000000\r\n"
										 "EndFragment:00000000\r\n";
		const			kCF_HTMLPrefixLength = strlen(kCF_HTMLPrefix);

		// Allocate zero byte sizes for the export memory-ref (1 byte rec-size, 0 recs)
		memory_ref exportMemRef = MemoryAlloc( PgMemGlobalsPtr(), 1, 0, 0 );

		pg_error pgError = exportFilter->pgInitExportFile (PgGlobalsPtr(), NULL, exportMemRef, NULL, 0);

		// Export from current document into paige memory references
		pgError = exportFilter->pgExportFile(m_pgRef, NULL, EXPORT_EVERYTHING_FLAG, TRUE);

		// "Lock" the Paige memories and get the memory addresses
		void PG_FAR* exportMemPtr = UseMemory(exportMemRef);

		// Alloc required Global (share) memory and Lock it down
		long exportByteSize = GetByteSize(exportMemRef);
		long allocByteSize = exportByteSize;

		if (fmt == PgDataTranslator::kFmtText)
		{
			// Add one byte if text since the last char kept getting chopped off
			++allocByteSize;
		}
		else if (fmt == PgDataTranslator::kFmtHtml)
		{
			allocByteSize += kCF_HTMLPrefixLength;
		}


		HGLOBAL		pghGlobal= GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, allocByteSize + 1);
		char *		pgGlobalMemStart = (char *) GlobalLock(pghGlobal);
		char *		pgGlobalMemPtr = pgGlobalMemStart;

		if (fmt == PgDataTranslator::kFmtHtml)
		{
			//	Prepend the CF_HTML prefix
			//	Note that we only support the most basic CF_HTML, hence we use
			//	the absolute earliest version number of 0.9.
			memcpy(pgGlobalMemPtr, kCF_HTMLPrefix, kCF_HTMLPrefixLength);
			pgGlobalMemPtr += kCF_HTMLPrefixLength;
		}
		
		// Copy from Paige memory block to the Global memory block
		memcpy(pgGlobalMemPtr, exportMemPtr, exportByteSize);

		if (fmt == PgDataTranslator::kFmtHtml)
		{
			//	We're exporting CF_HTML, so we need to fill out the offsets
			//	that we left as zeros above.
			//
			//	Now go back, calculate all the lengths, and write out the
			//	necessary header information. Note, wsprintf() truncates the
			//	string when you overwrite it so you follow up with code to replace
			//	the 0 appended at the end with a '\r'...
			//
			//	Code based on sample MS code (search for CF_HTML in MSDN), with modifications
			//	for what we already know and some added safety.
			pgGlobalMemPtr = strstr(pgGlobalMemStart, "StartHTML");
			if (pgGlobalMemPtr)
			{
				//	HTML starts directly after prefix, so the offset is the prefix length
				wsprintf(pgGlobalMemPtr+10, "%08u", kCF_HTMLPrefixLength);
				*(pgGlobalMemPtr+10+8) = '\r';
			}

			pgGlobalMemPtr = strstr(pgGlobalMemStart, "EndHTML");
			if (pgGlobalMemPtr)
			{
				//	HTML ends at allocByteSize
				wsprintf(pgGlobalMemPtr+8, "%08u", allocByteSize);
				*(pgGlobalMemPtr+8+8) = '\r';
			}

			//	Search for start and end fragment comments
			char *		pStartFrag = strstr(pgGlobalMemPtr, "<!--StartFrag");
			char *		pEndFrag = strstr(pgGlobalMemPtr, "<!--EndFrag");

			if (pStartFrag && pEndFrag)
			{
				pgGlobalMemPtr = strstr(pgGlobalMemStart, "StartFragment");
				if (pgGlobalMemPtr)
				{
					wsprintf(pgGlobalMemPtr+14, "%08u", pStartFrag - pgGlobalMemStart);
					*(pgGlobalMemPtr+14+8) = '\r';
				}

				pgGlobalMemPtr = strstr(pgGlobalMemStart, "EndFragment");
				if (pgGlobalMemPtr)
				{
					wsprintf(pgGlobalMemPtr+12, "%08u", pEndFrag - pgGlobalMemStart);
					*(pgGlobalMemPtr+12+8) = '\r';
				}
			}
		}

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
	memory_ref importMemRef = MEM_NULL; 

	if ( MakeTranslator( &importFilter, fmt ) ) {

		HGLOBAL pgImportGlobal = 0;
		HGLOBAL pgNewImportGlobal;
		STGMEDIUM stg;

		//Data may be returned as HGLOBAL/ISTREAM-ptr. 
		//We do not handle the other storage mediums.

		pgImportGlobal = pDataObj->GetGlobalData( (CLIPFORMAT)fmt, NULL);

		if ( pgImportGlobal )
		{
			//	Make pgImportGlobal moveable to reduce the chances that a later GlobalReAlloc
			//	will fail (either called directly below or inside of HandleToMemory)
			pgNewImportGlobal = GlobalReAlloc(pgImportGlobal, 0, GMEM_MODIFY | GMEM_MOVEABLE);

			//	If this suceeded use the new global, otherwise continue with the old global
			if (pgNewImportGlobal)
				pgImportGlobal = pgNewImportGlobal;
			
			//if ((fmt == kFmtText) || (fmt == kFmtRtf))
			if ( /*( IsWin95() || (pgTextSize(m_pgRef)) ) && */( (fmt == kFmtText) || (fmt == kFmtRtf) ) )
			{
				//On win95, the Global size of the HGlobal returned from
				//the OLE function GetGlobalData is bogus. Hence the
				//Paige translator was translating more than it was
				//supposed to. So we go the CString way and copy
				//only upto the first NULL char in the memory.
				LPCSTR textStr = (LPCSTR) GlobalLock(pgImportGlobal);
				importMemRef = CString2Memory( PgMemGlobalsPtr(), (LPCSTR)textStr );
				GlobalUnlock(pgImportGlobal);
			}
			else if (fmt == kFmtHtml)
			{
				LPSTR textStr = (LPSTR) GlobalLock(pgImportGlobal);

				if (textStr)
				{
					char *					startHTML = strstr(textStr, "StartHTML:");
					char *					endHTML = strstr(textStr, "EndHTML:");
					char					szHTMLDescriptionDataBuffer[512];
					static const size_t		kBufferLastChar = sizeof(szHTMLDescriptionDataBuffer)-1;

					if (!startHTML || !endHTML)
					{
						GlobalUnlock(pgImportGlobal);
						GlobalFree(pgImportGlobal);
						delete importFilter;
						return;
					}

					// Skip past "StartHMTL:"
					startHTML += 10;

					// Copy start position to buffer, NULL terminate, and parse
					int i = 0;
					*szHTMLDescriptionDataBuffer = '\0';
					while ( (*startHTML <= '9') && (*startHTML >= '0') && (i < kBufferLastChar) )
					{
						szHTMLDescriptionDataBuffer[i] = *startHTML;
						startHTML++;
						i++;
					}
					szHTMLDescriptionDataBuffer[i] = '\0';
					long		startPos = atol(szHTMLDescriptionDataBuffer);

					// Skip past "EndHTML:"
					endHTML += 8;

					// Copy end position to buffer, NULL terminate, and parse
					i = 0;
					*szHTMLDescriptionDataBuffer = '\0';
					while ( (*endHTML <= '9') && (*endHTML >= '0') && (i < kBufferLastChar) )
					{
						szHTMLDescriptionDataBuffer[i] = *endHTML;
						endHTML++;
						i++;
					}
					szHTMLDescriptionDataBuffer[i] = '\0';
					long		endPos = atol(szHTMLDescriptionDataBuffer);

					// Sanity check positions (startPos can't be 0 because it should point
					// after the header information)
					if ( !startPos || !endPos || (endPos <= startPos) )
					{
						GlobalUnlock(pgImportGlobal);
						GlobalFree(pgImportGlobal);
						delete importFilter;
						return;
					}

					// Look for SourceURL parameter to provide the base URL for any relative paths.
					char *		pSourceURL = strstr(textStr, "SourceURL:");

					if (pSourceURL)
					{
						// Skip past "SourceURL:"
						pSourceURL += 10;

						char *	pScan = pSourceURL;
						char	cScan;

						// Copy the URL until we encounter whitespace (ASFAIK the SourceURL line is
						// CRLF terminated, but look for any whitespace to be sure).
						i = 0;
						*szHTMLDescriptionDataBuffer = '\0';
						while ( *pScan && (i < kBufferLastChar) )
						{
							cScan = *pScan;

							if ( (cScan == '\r') || (cScan == '\n') || (cScan == '\t') || (cScan == ' ') )
								break;

							szHTMLDescriptionDataBuffer[i] = cScan;

							pScan++;
							i++;
						}
						szHTMLDescriptionDataBuffer[i] = '\0';

						if (*szHTMLDescriptionDataBuffer)
						{
							// URL looks like this: http://www.some-website.com/some-path/web-doc.html
							// Chop off the web-doc.html so that we have just the base path.
							pScan = strrchr(szHTMLDescriptionDataBuffer, '/');
							if ( pScan && ((pScan-szHTMLDescriptionDataBuffer) < sizeof(szHTMLDescriptionDataBuffer)) )
								*(pScan+1) = '\0';

							CMyPaigeHTMLImportFilter *		pPaigeHTMLImportFilter = reinterpret_cast<CMyPaigeHTMLImportFilter *>(importFilter);

							pPaigeHTMLImportFilter->SetBaseURLString(szHTMLDescriptionDataBuffer);
						}
					}

					// Make sure endPos is no bigger than possible
					DWORD		nGlobalSize = GlobalSize(pgImportGlobal);
					if ( (DWORD) endPos > nGlobalSize )
						endPos = nGlobalSize;

					unsigned char *		startTemp = ((unsigned char*) textStr) + startPos;
					unsigned char *		endTemp = ((unsigned char*) textStr) + endPos;
					unsigned char *		temp = startTemp;
					bool				bHasUniChars = false;
					
					//	Check for the prescence of any unicode characters before bothering continuing with encoding work.
					while (temp < endTemp)
					{
						if (*temp > 0x7F)
						{
							//	Found a unicode character
							bHasUniChars = true;
							break;
						}

						temp++;
					}

					CString			asciiData;
					const char *	pHTMLBeforeStripping;
					long			nHTMLBeforeStrippingLength;

					if (bHasUniChars)
					{
						//	There were some unicode characters that we'll need to encode
						UniChar		tempChar;
						char		tempBuffer[16];

						//	Pre-allocate an approximation of the amount of memory that asciiData will need.
						//	We'll need one byte for each original character. We'll need extra space for
						//	unicode character encoding, so we'll allocate an extra 1k (probably enough for
						//	most circumstances).
						//
						//	Pre-allocating saves *a lot* of time over just allowing += to do all the allocating
						//	because += would call new & delete every time it is called. As we loop += may
						//	do that towards the end (after the 1k extra is exceeded), but that's ok (trade-off
						//	here between time to figure out the exact amount of space needed ahead of time vs.
						//	time to allocate extra space towards the end of the loop).
						//
						//	GetBuffer is the odd (but documented) way to pre-allocate a CString's buffer.
						asciiData.GetBuffer(endPos-startPos + 1024);

						temp = startTemp;

						while (temp < endTemp)
						{
							tempChar = UTF8ToUniChar(temp, &temp);
							if (tempChar <= 0x7F)
							{
								asciiData += (char) tempChar;
							}
							else
							{
								sprintf(tempBuffer, "&#%d;", tempChar);
								asciiData += tempBuffer;
							}
						}

						pHTMLBeforeStripping = asciiData;
						nHTMLBeforeStrippingLength = asciiData.GetLength();
					}
					else
					{
						pHTMLBeforeStripping = reinterpret_cast<char *>(startTemp);
						nHTMLBeforeStrippingLength = endPos-startPos;
					}

					CString			strippedHTML;

					//	Preallocate enough space for the HTML before stripping to avoid allocations in AddToBody
					strippedHTML.GetBuffer(nHTMLBeforeStrippingLength);

					//	Pass in true for third to last parameter so that executable content is always stripped
					//	from HTML. This doesn't matter for security reasons (since Paige won't execute anything),
					//	but it does matter for display reasons. Script, etc. tags aren't recognized by Paige's
					//	importing and so the contents of the script (or whatever) would be displayed
					//	(unless the coding of the HTML disguised them as a comment for backward compatibility
					//	with non-scripting browsers).
					//
					//	Pass in true for the last parameter so that local file ref stripping is relaxed.
					//	For now this means that img tags will be spared, but in the future we may want
					//	to allow all local file refs through when pasting - stripping pasted content
					//	was never meant to improve security - as stated above it was done for display reasons.
					AddToBody(strippedHTML, pHTMLBeforeStripping, nHTMLBeforeStrippingLength, IS_HTML, true, false, true);

					//	Unlock and reallocate to hold the new unicode encoded and executable stripped HTML
					GlobalUnlock(pgImportGlobal);
					pgNewImportGlobal = GlobalReAlloc(pgImportGlobal, strippedHTML.GetLength()+1, GMEM_ZEROINIT);

					if (pgNewImportGlobal)
					{
						//	ReAlloc succeeded - copy over the new data
						pgImportGlobal = pgNewImportGlobal;
						textStr = (LPSTR) GlobalLock(pgImportGlobal);
						memset(textStr,0, strippedHTML.GetLength()+1);
						temp = (unsigned char*)strippedHTML.GetBuffer(0);
						strcpy(textStr, (const char*)temp);
						GlobalUnlock(pgImportGlobal);
						importMemRef = HandleToMemory( PgMemGlobalsPtr(), pgImportGlobal, 1 );
					}
					else
					{
						//	ReAllocing pgImportGlobal failed - free the memory and set to NULL
						GlobalFree(pgImportGlobal);
						pgImportGlobal = NULL;
					}
				}
				else
				{
					//	Locking pgImportGlobal failed - free the memory and set to NULL
					GlobalFree(pgImportGlobal);
					pgImportGlobal = NULL;
				}
			}
			else
			{
				//All other formats can use this
				importMemRef = HandleToMemory( PgMemGlobalsPtr(), pgImportGlobal, 1 );
			}
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
				
				char *text = DEBUG_NEW char[static_cast<int>(diff.QuadPart)];
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
			}
		}

		if (importMemRef)
		{
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

			m_importFlags ^= IMPORT_HIDDENTEXT_FLAG;

			if ( fmt == kFmtHtml )
				m_importFlags ^= IMPORT_BKCOLOR_FLAG;

			error = importFilter->pgImportFile(m_pgRef, CURRENT_POSITION, 
								m_importFlags, FALSE, best_way);

			if ( fmt == kFmtHtml )
				PgInitHyperlinks( m_pgRef );

			// Clean up
			DisposeMemory(importMemRef);
		}

		delete importFilter;
	}
}

///////////////////////////////////////////////////////////////////////////////
// PgDataTranslator - MFC File I/O

// ExportData:
// Translates Paige contents to specified CFile object

void PgDataTranslator::ExportData( CFile* pFile, FormatType fmt /* = kFmtText */ )
{
	ExportData( reinterpret_cast<HFILE>(pFile->m_hFile), fmt );
}


// ImportData:
// Translates from specified CFile object to Paige

void PgDataTranslator::ImportData( CFile* pFile, FormatType fmt /* = kFmtText */ )
{
	ImportData( reinterpret_cast<HFILE>(pFile->m_hFile), fmt );
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

		if ( fmt == kFmtHtml )
			m_importFlags ^= IMPORT_BKCOLOR_FLAG;

		error = importFilter->pgImportFile(m_pgRef, CURRENT_POSITION, 
										m_importFlags, FALSE, draw_none);

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

bool PgDataTranslator::MakeTranslator(
			PaigeExportObject *		ppTxr,
			FormatType				fmt,				// = kFmtText
			bool					bIsForClipboard)	// = false
{
	if ( fmt == kFmtText ) {
		*ppTxr = DEBUG_NEW_NOTHROW PaigeTextExportFilter(); 
	}
	else if ( fmt == kFmtRtf ) {
		*ppTxr = (PaigeExportObject) DEBUG_NEW_NOTHROW PaigeRTFExportFilter(); 
	}
	else if ( fmt == kFmtNative ) {
		*ppTxr = (PaigeExportObject) DEBUG_NEW_NOTHROW PaigeNativeExportFilter(); 
	}
	else if ( fmt == kFmtHtml ) {
		*ppTxr = (PaigeExportObject) DEBUG_NEW_NOTHROW CMyPaigeHTMLExportFilter(bIsForClipboard);
	}
	else {
		*ppTxr = NULL;
		DBGMSG_BAD_DATAFORMAT;
		ASSERT( 0 );
	}

	return (*ppTxr != NULL);
}


bool PgDataTranslator::MakeTranslator( PaigeImportObject* ppTxr,
					FormatType fmt /* = kFmtText */ )
{
	if ( fmt == kFmtText ) {
//		*ppTxr = DEBUG_NEW_NOTHROW PaigeImportFilter(); 
		*ppTxr = DEBUG_NEW_NOTHROW PgTextImportFilter(); 
	}
	else if ( fmt == kFmtRtf ) {
		*ppTxr = (PaigeImportObject) DEBUG_NEW_NOTHROW PaigeRTFImportFilter(); 
	}
	else if ( fmt == kFmtNative ) {
		*ppTxr = (PaigeImportObject) DEBUG_NEW_NOTHROW PaigeNativeImportFilter(); 
	}
	else if ( fmt == kFmtHtml ) {
		*ppTxr = (PaigeImportObject) DEBUG_NEW_NOTHROW CMyPaigeHTMLImportFilter();
	}
	else {
		*ppTxr = NULL;
		DBGMSG_BAD_DATAFORMAT;
		ASSERT( 0 );
	}

	return (*ppTxr != NULL);
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
					bool		bIsExcerpt = false;

					switch (param_ptr->code)
					{
						case style_attribute:
							{
								// Check for what Outlook generates
								CString		szOutlookExcerpt;

								GetIniString(IDS_INI_OUTLOOK_EXCERPT_TAG, szOutlookExcerpt);

								if ( !szOutlookExcerpt.IsEmpty() &&
									 (szOutlookExcerpt.CompareNoCase(reinterpret_cast<char *>(param_ptr->param)) == 0) )
								{
									bIsExcerpt = true;
								}
							}
							break;

						case type_attribute:
						case style_class_attribute:
							// Check for type="cite" and class="cite" (what we and many other mailers generate)
							if (strnicmp((char*)param_ptr->param,"cite",param_ptr->param_size) == 0)
								bIsExcerpt = true;
							break;

						case cite_attribute:
							// Also check for presence of cite attribute (what we and many other mailers generate)
							bIsExcerpt = true;
							break;
					}

					if (bIsExcerpt)
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
	else if ( !(import_bits & IMPORT_HTTP_IMAGES_FLAG) &&
			tag_type == DATA_COMMAND && tag == image_command)
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
	//
	// Strip images with unpleasant dimensions
	//
	else if ((import_bits & IMPORT_HTTP_IMAGES_FLAG) &&
			tag_type == DATA_COMMAND && tag == image_command)
	{
		tag_attribute_ptr height_arg, width_arg;
		
		ret = FALSE;	// assume the best

		if ((height_arg = find_parameter(params, num_params, image_height_attribute)) && height_arg->param_size)
		if ((width_arg = find_parameter(params, num_params, image_width_attribute)) && width_arg->param_size)
		{
			int height = atoi((const char*)width_arg->param);
			int width = atoi((const char*)width_arg->param);

			if (height*width < GetIniShort(IDS_INI_MIN_HTTP_IMAGE_SIZE))
				ret = TRUE;	// too small; probably a web bug!
		}
	}
	//
	// TABLES
	//
	//TEMP comment out later for new Paige srcs
	else if ( !(import_bits & IMPORT_TABLES_FLAG) &&
			tag_type == TABLE_DATA_COMMAND )
	{
		ret = TRUE;
		// Ensure that table rows begin and end on a new line
		if ( tag == table_row_command || tag == -table_row_command)
			BreakLine(FALSE, FALSE);
	}
	// Replace the starting div tag with a CRLF (the CRLF for the ending
	// div tag is handled in PaigeHTMLImportFilter::DoControlTag). 
	else if (tag_type == CONTROL_COMMAND && (tag == div_command))
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
		ScanStyleSheet();
		ret = TRUE;
	}
	else if ( (tag_type == CONTROL_COMMAND) && (tag == x_signature_command) )
	{
		// don't do this when importing quoted text, or if sig importing has
		// been explicitly turned off.
		if ( (import_bits & IMPORT_SIG_FORMAT_FLAG) &&
					!CPaigeStyle::IsExcerpt(&current_par) ) {

			DumpPendingText();
			CPaigeStyle::ApplySignature( &current_par, TRUE );

			if ( !doing_table )
				ApplyInputParagraph(&current_par, -1, FALSE);

			SkipToEndTag( x_signature_command );
			ret = TRUE;
		}
	}

	return ret;
}


// Parse much like SkipToEndTag, but with a few important differences:
// * Doesn't skip comments (since style sheet contents may be inside
// * Accumulates lines while looking for the end tag and parses them
//   for style sheet contents
void CMyPaigeHTMLImportFilter::ScanStyleSheet()
{
	pg_char		next_char;
	short	access;

	access = GetAccessCtr(param_ref);

	if (access)
		UnuseMemory(param_ref);

	bool		bSawParBreakStyleName = false;
	bool		bSawDivStyleName = false;
	bool		bInsideStyleDefinition = false;
	bool		bLookingForStyleDefFields = false;
	int			nFound, nEnd;
	int			nParBreakStart, nDivStart, nStyleDefNameStart;
	CString		sLine;
	CString		sStyleName;

	sLine.GetBuffer(1024);

	while (pgGetImportChar(&next_char) == NO_ERROR)
	{
		if ( (next_char == TAG_COMMAND_CHAR) || (next_char == 0x0D) || (next_char == 0x0A) )
		{
			//	Parse line we've accumulated
			while ( !sLine.IsEmpty() )
			{
				if (!bSawParBreakStyleName && !bSawDivStyleName)
				{
					//	Any style definition name must be before the start of the style definition
					nEnd = sLine.Find('{');
					if (nEnd < 0)
						nEnd = sLine.GetLength();

					//	Look for style definition name
					nParBreakStart = sLine.Find("p.");
					bSawParBreakStyleName = ( (nParBreakStart >= 0) && (nParBreakStart < nEnd) );

					nDivStart = sLine.Find("div.");
					bSawDivStyleName = ( (nDivStart >= 0) && (nDivStart < nEnd) );

					if (bSawParBreakStyleName || bSawDivStyleName)
					{
						if (bSawParBreakStyleName)
							nStyleDefNameStart = nParBreakStart + 2;
						else
							nStyleDefNameStart = nDivStart + 4;
						
						//	Skip past the "p." or "div."
						sLine.Delete(0, nStyleDefNameStart);

						//	Name is either terminated by a comma or EOL
						nFound = sLine.Find(',');
						if (nFound > 0)
							sStyleName = sLine.Left(nFound);
						else
							sStyleName = sLine;
					}
					else
					{
						//	Didn't find style definition name, so we're done with the line
						sLine.Empty();
					}
				}

				if (bSawParBreakStyleName || bSawDivStyleName)
				{
					if (!bInsideStyleDefinition)
					{
						//	Found style defintion name, now look for curly brackets
						nFound = sLine.Find('{');

						if (nFound >= 0)
						{
							//	We're now inside the style definition, looking for any fields we're interested in
							bInsideStyleDefinition = true;
							bLookingForStyleDefFields = true;

							//	Skip past the '{'
							sLine.Delete(0, nFound+1);
						}
						else
						{
							//	Didn't find start of style definition, so we're done with the line
							sLine.Empty();
						}
					}

					if (bInsideStyleDefinition)
					{
						//	Look for the end of the style definition
						nEnd = sLine.Find('}');
						if (nEnd >= 0)
						{
							bSawParBreakStyleName = false;
							bSawDivStyleName = false;
							bInsideStyleDefinition = false;
						}
						
						if (bLookingForStyleDefFields)
						{
							nFound = sLine.Find("margin-bottom:.0");
							
							//	If we found something, make sure that we found it soon enough - i.e. still inside
							//	the current style definition.
							if ( (nFound >= 0) && (bInsideStyleDefinition || (nFound <= nEnd)) )
							{
								//	Trim any space before or after the actual name
								sStyleName.TrimLeft();
								sStyleName.TrimRight();
								
								//	We found "margin-bottom:.0" before the end of the current style definition
								PaigeHTMLStyleSheetDef *	pStyleSheetDef = DEBUG_NEW_MFCOBJ_NOTHROW PaigeHTMLStyleSheetDef(sStyleName, bSawParBreakStyleName, bSawDivStyleName, true);

								if (pStyleSheetDef)
									style_sheet_defs.AddTail(pStyleSheetDef);

								//	In case we're still inside a style definition, remember that we've already
								//	found all the fields in which we're interested.
								bLookingForStyleDefFields = false;
							}
						}

						if ( bInsideStyleDefinition || (sLine.GetLength() == (nEnd + 1)) )
						{
							//	We already looked for any style definition fields that we cared about and either:
							//	* This line didn't end the style definition OR
							//	* This line ended the style definition, but there's nothing else on the line (EOL)
							sLine.Empty();
						}
						else
						{
							//	We're no longer in the current style definition, but there may
							//	be more style defs on this line, so skip past the '}' and continue
							//	parsing.
							sLine.Delete(0, nEnd+1);
						}
					}
				}
			}

			if (next_char == TAG_COMMAND_CHAR)
			{
				next_char = pgNextImportChar();
				
				if (next_char == '!')
				{
					//	A comment - *don't* call ReadHTMLTag which would ignore the contents. We want to
					//	parse the comment contents because style sheet information is likely inside it.
					pgGetImportChar(&next_char);
				}
				else
				{
					//	Parse HTML tag and stop if it's the end of the style command
					long			command_id;
					pg_boolean		end_tag;
					
					command_id = ReadHTMLTag(&end_tag);
					
					if (command_id )
					{
						short		table_id, tag;
						
						table_id = pgHiWord(command_id);
						tag = pgLoWord(command_id);
						
						if ( end_tag && (tag == style_command) )
							break;
					}
				}
			}
		}
		else
		{
			//	Not EOL or start of HTML tag - accumulate the line
			sLine += (char) next_char;
		}
	}

	if (access)
		UseMemory(param_ref);
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


CMyPaigeHTMLExportFilter::CMyPaigeHTMLExportFilter(bool bIsForClipboard)
{
	//	Initialize PaigeHTMLImportFilter member (simplified changing Paige)
	m_bIsForClipboard = bIsForClipboard;
	
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
	// check for signature---write our own proprietary "x-signature" tag. note
	// that we don't write a closing tag since everything past the opening tag
	// is part of the sig.
	//
	// BOO: also note that if something is a sig, it can't be an excerpt.
	// there's only one per, so how could it possibly be excerpted? unless,
	// of course...  nah, why would ya want to do that!

	if ( !CPaigeStyle::IsSignature(old_style) &&
				CPaigeStyle::IsSignature(new_style) ) {

		// well here we are writing an html "style" tag during OutputParChanges.
		// kinda hard to tell how all this stuff is supposed to work.
		if ( CPaigeStyle::IsExcerpt(old_style) )
			WriteTag( html_style_commands, -blockquote_style_command, TRUE, TRUE );
	
		WriteTag( control_commands, x_signature_command, TRUE, FALSE);
		pgWriteString( (unsigned char*)"<p>", 0, 0 );
		WriteTag( control_commands, -x_signature_command, TRUE, TRUE );
	}
	else {
		bool bOld =  CPaigeStyle::IsExcerpt(old_style);
		bool bNew =  CPaigeStyle::IsExcerpt(new_style);

		if ( !bOld && !bNew )
			return;

		CRString szCite(IDS_HTML_BLOCKQUOTE_CITE);
		CString szBegin;
		const char* szEnd = "</blockquote>";
		const char* s;

		szBegin = '<' + szCite + '>';
		
		s = NULL;
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
	}

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



