// PgStyleUtils.cpp
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

#include "stdafx.h"			// HACK!! this kills a precompiled header error

#include "pgtraps.h"		// for all paige text functions
#include "pgutils.h"		// paige utility routines
#include "machine.h"

#include "PgStyleUtils.h"	// header for this module
#include "paigestyle.h"
#include "pgdefstl.h"
#include "PgGlobals.h"
#include "PGEMBED.H"
#include "pghtext.h"
#include "pghtmdef.h"
#include "pgText.h"
#include "rs.h"
#include "resource.h"


#include "DebugNewHelpers.h"


// _pgConvertLogFont:
// Converts a windows logical font record to paige equiv
//
// Pass this baby a Windows LOGFONT structure, and it will fill out Paige
// font_info and style_info structures with the appropriate values.

extern "C"
void PgConvertLogFont( pg_ref pg, pg_globals_ptr pgGlobals, LOGFONT PG_FAR* logFont,
			font_info_ptr font, style_info_ptr style, style_info_ptr styleMask )
{
	// make a paige ref if the user doesn't have one
	pg_ref pgRef;
	BOOL CreatedPaigeRef = FALSE;
	if ( pg == 0 )
	{
		CreatedPaigeRef = TRUE;
		pgRef = pgNewShell( pgGlobals );
	}
	else
		pgRef = pg;
	
	// initialize the style to Paige default
	*style = pgGlobals->def_style;

	// initialize paige font record and style mask to appropriate values.
	// Note: don't use memset for this, allways use paige mem manager routines.
	pgFillBlock( font, sizeof(font_info), 0 );
	pgFillBlock( styleMask, sizeof(style_info), 0 );
	pgFillBlock( styleMask->styles, MAX_STYLES * sizeof(short), -1 );
	styleMask->point = (pg_fixed) -1;

	// some of the LOGFONT gets converted to paige font info, and some of it
	// gets converted to paige style info; first the font info:

	lstrcpy( (LPSTR)font->name, logFont->lfFaceName );
	font->environs |= NAME_IS_CSTR;		// this is a must!

	font->family_id = logFont->lfPitchAndFamily;
	font->machine_var[PG_OUT_PRECISION] = logFont->lfOutPrecision;
	font->machine_var[PG_CLIP_PRECISION] = logFont->lfClipPrecision;
	font->machine_var[PG_QUALITY] = logFont->lfQuality;
	font->machine_var[PG_CHARSET] = logFont->lfCharSet;

	if ( (style->point = (pg_fixed) logFont->lfHeight) < 0 ) {
		style->point = -style->point;
	}

	// make sure point size is 0x000n0000
	style->point <<= 16;

	// convert point size to fit the screen resolution
	style->point = pgScreenToPointsize( pgRef, style->point );

	// convert remaining style attributes (bold, italic, etc...)
	if ( logFont->lfWeight == FW_BOLD ) {
		style->styles[bold_var] = -1;
	}
	if ( logFont->lfItalic ) {
		style->styles[italic_var] = -1;
	}
	if ( logFont->lfUnderline ) {
		style->styles[underline_var] = -1;
	}
	if ( logFont->lfStrikeOut ) {
		style->styles[strikeout_var] = -1;
	}

	// blow off our home-made ref
	if ( CreatedPaigeRef )
		pgDispose( pgRef );
}


extern "C"
bool PgHasStyledText( pg_ref pg, pg_globals_ptr globals, select_pair_ptr pSel, bool* pInternalStylesOnly )
{
	int nBegin, nEnd;
	if (pSel)
	{
		nBegin = pSel->begin;
		nEnd = pSel->end;
	}
	else
	{
		nBegin = 0;
		nEnd = pgTextSize(pg);
	}

	if (pInternalStylesOnly)
		*pInternalStylesOnly = false;

	style_walk walker;
	paige_rec_ptr pgr = (paige_rec_ptr)UseMemory( pg );
	pgPrepareStyleWalk( pgr, nBegin, &walker, true );

	bool bStyled;
	bool bHasInternalStyles = false;
	bool bSendEmoticonsAsImages = (GetIniShort(IDS_INI_SEND_EMOTICONS_AS_IMAGES) != 0);

	//
	// Quick and Dirty Check:  If no styles exist then we can be pretty
	// darn sure that this is a plain text message.  
	//
	if ( !(bStyled = (walker.next_style_run->style_item != 0)) )
	{
		if ( !(bStyled = (walker.prev_style_run->style_item != 0)) )
			if ( !(bStyled = (walker.next_par_run->style_item != 0)) )
				if (!(bStyled = (walker.prev_par_run->style_item != 0)))
					bStyled=(walker.hyperlink->type != 0);
	}
	//
	// Ok, we've got a style/format change. Only the following count as valid changes:
	//		char styles: bold, italic, underline, strikeout
	//		paragraph format: indentions, right/left/center justification
	//		paragraph styles: bullet list
	//		paragraph stylesheets: headings 1-6, blockquote
	//		Font change: size, name, color
	//		URLs: Non eudora generated urls
	//
	if ( bStyled )
	{
		int iWalkerStart = 1, iDefFontStart = 1, fontLength;

		//pg_globals_ptr	globals=pgGetGlobals(pg);
		long style_bits, def_style_bits;
		//def_style_bits = PaigeToQDStyle(&globals->def_style);

		long styleID;
		font_info defFont;
		style_info defStyle;

		if ( (styleID = pgGetNamedStyleIndex (pg, body_style) ) )
		{
			pgGetNamedStyleInfo(pg, styleID, &defStyle, &defFont, NULL);
			def_style_bits = PaigeToQDStyle(&defStyle);
		}
		else
		{
			defFont = globals->def_font;
			defStyle = globals->def_style;
			def_style_bits = PaigeToQDStyle(&globals->def_style);
		}
		
		if ( defFont.environs & NAME_IS_CSTR )
			iDefFontStart = 0;

		for (;walker.current_offset<=nEnd;) {
			//
			// Character Styles: bold, italic, underline, strikeout
			//
			style_bits = PaigeToQDStyle(walker.cur_style);
			if ( bStyled = (style_bits != def_style_bits) )
				//TEMP : Due to old paige srcs
				//if ( bStyled = (( style_bits & (X_BOLD_BIT | X_ITALIC_BIT | X_UNDERLINE_BIT | X_LINERULE_BIT) ) !=0) )
				if ( bStyled = (( style_bits & (X_BOLD_BIT | X_ITALIC_BIT | X_UNDERLINE_BIT  | X_STRIKEOUT_BIT) ) !=0) )
					break;
			//
			// Text color and size
			//
			if ( bStyled = ((walker.cur_style->fg_color.red	  != defStyle.fg_color.red	  ) ||
							(walker.cur_style->fg_color.green != defStyle.fg_color.green ) ||
							(walker.cur_style->fg_color.blue  != defStyle.fg_color.blue  ) ||
							(walker.cur_style->fg_color.alpha != defStyle.fg_color.alpha ) ||
							(walker.cur_style->point		  != defStyle.point		   ) ) )
				break;

			if (walker.cur_style->embed_id != defStyle.embed_id)
			{
				if (bSendEmoticonsAsImages)
				{
					// We're sending emoticons as images - so this counts as styled
					// regardless of whether or not it's an emoticon.
					bStyled = true;
					break;
				}
				else
				{
					// We're sending emoticons as triggers - check to see if we have
					// an emoticon before assuming that we're styled.
					memory_ref			embed_ref = walker.cur_style->embed_object;
					pg_embed_ptr		embed_ptr = reinterpret_cast<pg_embed_ptr>( UseMemory(embed_ref) );
					ASSERT(embed_ptr);

					if (embed_ptr)
					{
						if ( !(embed_ptr->type & EMBED_EMOTICON_FLAG) )
							bStyled = true;
						UnuseMemory(embed_ref);

						if (bStyled)
							break;
					}
					else
					{
						bStyled = true;
						break;
					}
				}
			}

			//
			//  URLs: ignore attachments, plugins and auto generatted urls
			//
			if ( walker.hyperlink->type )
			{
				if ( bStyled = !((walker.hyperlink->type & HYPERLINK_EUDORA_ATTACHMENT)	||
						  		 (walker.hyperlink->type & HYPERLINK_EUDORA_PLUGIN	  )	||
								 (walker.hyperlink->type & HYPERLINK_EUDORA_AUTOURL   ) ))
					break;
			}
			//
			// Paragraph format	other than stylesheets
			//
			par_info mask;
			pgInitParMask(&mask,0);
			mask.justification = -1;
//			mask.user_id = -1;
			mask.html_bullet = -1;
			mask.table.border_info = -1;
			mask.html_numbers=-1;
			mask.indents.left_indent = mask.indents.right_indent = mask.indents.first_indent=-1;
			if (bStyled = !pgStyleMatchesCriteria(walker.cur_par_style, &(globals->def_par),&mask, NULL, SIGNIFICANT_PAR_STYLE_SIZE))
				break;

			pgInitParMask(&mask,0);
			mask.user_id = -1;
			if (bStyled = !pgStyleMatchesCriteria(walker.cur_par_style, &(globals->def_par),&mask, NULL, SIGNIFICANT_PAR_STYLE_SIZE))
			{
				// We want to know when it was a excerpt that caused a message to be "styled" so that
				// when we send the message when can send it out as "text/plain; format=flowed".
				if ((walker.cur_par_style->user_id == PAIGE_FORMAT_USER_ID_EXCERPT) ||
						(walker.cur_par_style->user_id == PAIGE_FORMAT_USER_ID_SIGNATURE))
				{
					bHasInternalStyles = true;
					bStyled = false;
				}
				else
					break;
			}

			//
			// Stylesheets: Blank or Body is ok
			//
			// ( bStyled = ( (walker.cur_par_style->style_sheet_id != 0) && (abs(walker.cur_par_style->style_sheet_id) != 1) && ) )
			if ( ( bStyled = ( ( abs(walker.cur_par_style->style_sheet_id) > 1 ) && ( abs(walker.cur_par_style->style_sheet_id) < 16 ) ) )	)
				break;
			//
			// Font format: Name and size
			//
			//if (bStyled= (strnicmp((const char*)&walker.cur_font->name[1],(const char*)globals->def_font.name,walker.cur_font->name[0])!=0))
			if ( walker.cur_font->environs & NAME_IS_CSTR )
			{
				iWalkerStart = 0;
				fontLength = lstrlen((LPSTR)&walker.cur_font->name);
			}
			else
			{
				iWalkerStart = 1;
				fontLength = (int)(walker.cur_font->name[0]);
			}

			if (bStyled= (strnicmp((const char*)&walker.cur_font->name[iWalkerStart],(const char*)&defFont.name[iDefFontStart],fontLength)!=0))
				break;

			//
			// Walk to the next style
			//
			register long		next_run_offset;
			next_run_offset = min(walker.next_style_run->offset,walker.next_par_run->offset);
			if (next_run_offset > walker.t_length)
				break;
			if ( !pgWalkStyle(&walker, next_run_offset - walker.current_offset))
				break;
		}
	}

		   
	pgPrepareStyleWalk( pgr, nBegin, NULL, true );
	UnuseMemory( pg );

	// We want to know when it was a excerpt that caused a message to be "styled" so that
	// when we send the message when can send it out as "text/plain; format=flowed".
	if (!bStyled && bHasInternalStyles)
	{
		if (pInternalStylesOnly)
			*pInternalStylesOnly = true;

		return true;
	}
	
	return bStyled;
}


// Efficiently scan backwards to just after a blank character
long PgScanBackwardsToJustAfterBlank(pg_ref paigeRef, long nStartOffset)
{
	long			nOffset = nStartOffset;
	paige_rec_ptr	pPaige = reinterpret_cast<paige_rec_ptr>( UseMemory(paigeRef) );
	
	if (pPaige)
	{
		//	Find the text block in which we want to scan
		text_block_ptr	pTextBlock = pgFindTextBlock(pPaige, nOffset, NULL, FALSE, TRUE);

		ASSERT(pTextBlock);
		
		if (pTextBlock)
		{
			//	Get the text for the text block
			pg_char_ptr		pText = reinterpret_cast<pg_char_ptr>( UseMemory(pTextBlock->text) );

			ASSERT(pText);
			
			if (pText)
			{
				//	Prepare our style walk
				style_walk		walker;
				pgPrepareStyleWalk(pPaige, nOffset, &walker, FALSE);
				
				//	Keep track of where we are locally in the text block
				long	nCurrentLocalOffset = nOffset - pTextBlock->begin;
				long	nEndLocalOffset = pTextBlock->end - pTextBlock->begin;

				//	Flags for the current character
				long	infoFlags;
				
				//	Scan until we hit the beginning of the text block or find a
				//	blank character.
				while (nCurrentLocalOffset && nOffset)
				{
					//	Move back a character
					pgWalkStyle(&walker, -1);
					--nCurrentLocalOffset;
					
					//	Is the character a blank?
					infoFlags = walker.cur_style->procs.char_info( pPaige, &walker, pText,
																   pTextBlock->begin, 0,
																   nEndLocalOffset, nCurrentLocalOffset,
																   BLANK_BIT );
					if (infoFlags & BLANK_BIT)
						break;
					
					//	Character wasn't blank - move our return value back too
					--nOffset;
				}
				
				//	Free style walk memory
				pgPrepareStyleWalk(pPaige, 0, NULL, FALSE);

				//	Release text
				UnuseMemory(pTextBlock->text);
			}
		}

		//	Must release t_blocks because pgFindTextBlock calls UseMemory with it.
		UnuseMemory(pPaige->t_blocks);
		
		//	Release Paige info
		UnuseMemory(paigeRef);
	}

	return nOffset;
}

