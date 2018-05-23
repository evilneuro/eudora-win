#include "stdafx.h"			// HACK!! this kills a precompiled header error
//#include "paige.h"			// standard paige header
#include "pgtraps.h"		// for all paige text functions
#include "pgutils.h"		// paige utility routines
#include "machine.h"

#include "PgStyleUtils.h"	// header for this module
#include "paigestyle.h"
#include "pgdefstl.h"
#include "PgGlobals.h"
#include "pghtext.h"
#include "pghtmdef.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


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
bool PgHasStyledText( pg_ref pg, pg_globals_ptr globals, select_pair_ptr pSel, bool* pExcerptCausedStyle )
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

	if (pExcerptCausedStyle)
		*pExcerptCausedStyle = false;

	style_walk walker;
	paige_rec_ptr pgr = (paige_rec_ptr)UseMemory( pg );
	pgPrepareStyleWalk( pgr, nBegin, &walker, true );

	bool bStyled;
	bool bHasExcerpt = false;
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
	//		char styles: bold, italic, underline
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
			// Character Styles: bold, italic, underline
			//
			style_bits = PaigeToQDStyle(walker.cur_style);
			if ( bStyled = (style_bits != def_style_bits) )
				//TEMP : Due to old paige srcs
				//if ( bStyled = (( style_bits & (X_BOLD_BIT | X_ITALIC_BIT | X_UNDERLINE_BIT | X_LINERULE_BIT) ) !=0) )
				if ( bStyled = (( style_bits & (X_BOLD_BIT | X_ITALIC_BIT | X_UNDERLINE_BIT) ) !=0) )
					break;
			//
			// Text color, size	and embedded images
			//
			if ( bStyled = ((walker.cur_style->fg_color.red	  != defStyle.fg_color.red	  ) ||
							(walker.cur_style->fg_color.green != defStyle.fg_color.green ) ||
							(walker.cur_style->fg_color.blue  != defStyle.fg_color.blue  ) ||
							(walker.cur_style->fg_color.alpha != defStyle.fg_color.alpha ) ||
							(walker.cur_style->embed_id		  != defStyle.embed_id	   ) ||
							(walker.cur_style->point		  != defStyle.point		   ) ) )
				break;

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
				if (walker.cur_par_style->user_id == PAIGE_FORMAT_USER_ID_EXCERPT)
				{
					bHasExcerpt = true;
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
	if (!bStyled && bHasExcerpt)
	{
		if (pExcerptCausedStyle)
			*pExcerptCausedStyle = true;
		return true;
	}
	
	return bStyled;
}
