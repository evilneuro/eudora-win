// PaigeStyle.cpp : implementation file
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

#include "stdafx.h"
#include "resource.h"

#include "defprocs.h"
#include "pgutils.h"
#include "paige.h"
#include "pgtraps.h"
#include "machine.h"
#include "pggrafx.h"
#include "PaigeStyle.h"
#include "pghtmdef.h"
#include "PgStuffBucket.h"
#include "font.h"
#include "rs.h"
#include "machine.h"
// high-level style interface
#include "pghlevel.h"
#include "moodwatch.h"
#include "pgcompmsgview.h"

#include "PGDEFSTL.H"
#include "PgStyleUtils.h"

//class PgCompMsgView;

#include "DebugNewHelpers.h"
#include "BossProtector.h"


#define EXCERPT_OFFSET        8
#define VBAR_LINEWIDTH        3
#define SEPARATOR_LINEWIDTH   3
#define LIST_OFFSET           16


#define ID_UNDERLINE_DISTANCE 4 //3 drawing pixel and one blank
// pattern bmps for making brushes
CBitmap g_bmpSpell, g_bmpHWord, g_bmpNonHWord;

// hard-coded heights for the brushes (a pox on win95!)
enum {
	kSpellBrushHeight = 3,
	kHWordBrushHeight = 4,
	kNonHWordBrushHeight = 3
};


// protected one-time init of moodwatch drawing globals
void PGS_InitDrawGlobals()
{
	static bool initialized = false;

	if ( !initialized ) {
		assert( (HBITMAP)g_bmpHWord == 0 );
		assert( (HBITMAP)g_bmpNonHWord == 0 );
		assert( (HBITMAP)g_bmpSpell == 0 );
		// TODO: match background color to document
		g_bmpHWord.LoadBitmap( IDB_CHILIBRUSH );
		g_bmpNonHWord.LoadBitmap( IDB_NONHWORDBRUSH );
		g_bmpSpell.LoadBitmap( IDB_SPELLBRUSH );
		initialized = true;
	}
}

void PGS_DrawUnderline( HDC hDC, POINT* ptStart, int length, underline_type kind )
{
	HPEN           underline_pen = NULL, old_pen = NULL, second_underline_pen = NULL;
	COLORREF       pen_color = 0, second_pen_color = 0;
	const int      pen_size = 1;
	const int      pen_type = PS_SOLID;

	if (length <= 0)
		return;
			
	if ( kind == kSpellUnderline ) {
		pen_color = RGB(255,0,0);
	}
	else if (kind == kBPUnderline ) {
		pen_color = CBossProtector::GetBPColorCode();	
	}
	else if ( kind == kHWordUnderline ) {
		pen_color = RGB(128,0,0);//maroon
		second_pen_color = RGB(0,255,0);
		
	}
	else if ( kind == kNonHWordUnderline ) {
		pen_color = RGB(0,255,0);
	}
	else {
		// oops, looks like somebody's screwing up---for safety's sake we'll
		// draw the spell underline, but just use the current text color.
		assert(0);
		pen_color = RGB(255,0,0);
	}

	underline_pen = CreatePen(pen_type, pen_size, pen_color);
	if ( kind == kHWordUnderline )
		second_underline_pen = CreatePen(pen_type, pen_size, second_pen_color);
	
	old_pen = (HPEN) SelectObject(hDC, underline_pen);
//first row of underline
	int nIntervals = length/ID_UNDERLINE_DISTANCE;
	int nRemainder = length%ID_UNDERLINE_DISTANCE;
	MoveToEx(hDC, ptStart->x, ++ptStart->y, NULL);
	int distance = ptStart->x;
	int pen_toggle = 0;
	for(int j =0; j<nIntervals; j++)
	{
		distance +=(ID_UNDERLINE_DISTANCE-2);
		LineTo(hDC,distance,ptStart->y);
		distance+=2;
		MoveToEx(hDC, distance, ptStart->y, NULL);
		//toggle the colors if it h word
		if ( kind == kHWordUnderline )
		{
			if(pen_toggle==0)
			{
				SelectObject(hDC, second_underline_pen);
				pen_toggle = 1;
			}
			else 
			{
				SelectObject(hDC, underline_pen);
				pen_toggle = 0;
			}
		}
	}
	if (nRemainder > 1)
		LineTo(hDC,distance+nRemainder-2,ptStart->y);
////

///second row of underline
	if ( kind == kHWordUnderline )
	   SelectObject(hDC, underline_pen);
	
	nIntervals = (length-2)/ID_UNDERLINE_DISTANCE;
	nRemainder = (length-2)%ID_UNDERLINE_DISTANCE;
	ptStart->y+=1;
	MoveToEx(hDC, ptStart->x+2, ptStart->y, NULL);
	distance = ptStart->x+2;
	pen_toggle=1;
	for(j =0; j<nIntervals; j++)
	{
		distance +=(ID_UNDERLINE_DISTANCE-2);
		LineTo(hDC,distance,ptStart->y);
		distance+=2;
		MoveToEx(hDC, distance, ptStart->y, NULL);
	}
	if (nRemainder > 1)
		LineTo(hDC,distance+nRemainder-2,ptStart->y);
///////////////////////////////

	SelectObject(hDC, old_pen);
	DeleteObject(underline_pen);
	if ( kind == kHWordUnderline )
		DeleteObject(second_underline_pen);
}



PG_PASCAL (void) pgDrawSpellUnderline (paige_rec_ptr pg, Point from_pt, short distance )
{
	// No misspelling underlines when printing
	if ((pg->flags & PRINT_MODE_BIT) != 0)
		return;

	HDC hdc = pgGetPlatformDevice(pg->globals->current_port);
	CPoint pt( from_pt.h, from_pt.v );
	PGS_DrawUnderline( hdc, &pt, distance, kSpellUnderline );
	pgReleasePlatformDevice(pg->globals->current_port);
}

static PG_PASCAL (void) spell_draw_proc(paige_rec_ptr pg, style_walk_ptr walker, 
									  pg_byte_ptr data, pg_short_t offset, pg_short_t length,
									  draw_points_ptr draw_position, long extra, short draw_mode)
{
	Point start_pt;
	Point end_pt;

	pgDrawProc( pg, walker, data, offset, length, draw_position, extra, draw_mode);

	if (walker->cur_style->user_id & PAIGE_STYLE_USER_ID_MISSPELLED)
	{
		start_pt.h = pgLongToShort(draw_position->from.h);
		start_pt.v = pgLongToShort(draw_position->from.v);

		end_pt.h = pgLongToShort(draw_position->to.h);
		end_pt.v = pgLongToShort(draw_position->to.v);

		pgDrawSpellUnderline(pg, start_pt, (short)(end_pt.h-start_pt.h+1));
	}
}

static PG_PASCAL (void) draw_h_word_proc(paige_rec_ptr pg, style_walk_ptr walker, 
									  pg_byte_ptr data, pg_short_t offset, pg_short_t length,
									  draw_points_ptr draw_position, long extra, short draw_mode)
{
	pgDrawProc( pg, walker, data, offset, length, draw_position, extra, draw_mode);

    PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;
    CWnd* pwnd = pSB->pWndOwner;
	bool bDrawBadWords = false;
    // UGLY: should really do this with type/subtype "stuff"; leave it for now.
    if ( pwnd && pwnd->IsKindOf(RUNTIME_CLASS(PgCompMsgView)))
	{
	   if((((PgCompMsgView*)pwnd)->GetMoodScore() >1)&&
		   (((PgCompMsgView*)pwnd)->GetMoodScore() <5))
	   {
			bDrawBadWords = true;
	   }
	}

	if ( (walker->cur_style->user_id & PAIGE_STYLE_USER_ID_BAD_MOOD_WORD) && bDrawBadWords ) 
	{
		HDC hdc = pgGetPlatformDevice(pg->globals->current_port);

		Point start_pt;
		Point end_pt;

		start_pt.h = pgLongToShort(draw_position->from.h);
		start_pt.v = pgLongToShort(draw_position->from.v);

		end_pt.h = pgLongToShort(draw_position->to.h);
		end_pt.v = pgLongToShort(draw_position->to.v);

		CPoint pt( start_pt.h, start_pt.v );
		
		int distance = end_pt.h - start_pt.h + 1;
		PGS_DrawUnderline( hdc, &pt, distance, kHWordUnderline );

		pgReleasePlatformDevice(pg->globals->current_port);
	}
}

static PG_PASCAL (void) draw_non_h_word_proc(paige_rec_ptr pg, style_walk_ptr walker, 
									  pg_byte_ptr data, pg_short_t offset, pg_short_t length,
									  draw_points_ptr draw_position, long extra, short draw_mode)
{
	pgDrawProc( pg, walker, data, offset, length, draw_position, extra, draw_mode);

    PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;
    CWnd* pwnd = pSB->pWndOwner;
	bool bDrawBadWords = false;
    // UGLY: should really do this with type/subtype "stuff"; leave it for now.
    if ( pwnd && pwnd->IsKindOf(RUNTIME_CLASS(PgCompMsgView)))
	{
	   if((((PgCompMsgView*)pwnd)->GetMoodScore() >1)&&
		   (((PgCompMsgView*)pwnd)->GetMoodScore() <5))
	   {
			bDrawBadWords = true;
	   }
	}

	if ( walker->cur_style->user_id & PAIGE_STYLE_USER_ID_BAD_MOOD_WORD && bDrawBadWords ) 
	{
		HDC hdc = pgGetPlatformDevice(pg->globals->current_port);

		Point start_pt;
		Point end_pt;

		start_pt.h = pgLongToShort(draw_position->from.h);
		start_pt.v = pgLongToShort(draw_position->from.v);

		end_pt.h = pgLongToShort(draw_position->to.h);
		end_pt.v = pgLongToShort(draw_position->to.v);

		CPoint pt( start_pt.h, start_pt.v );

		int distance = end_pt.h - start_pt.h + 1;
		PGS_DrawUnderline( hdc, &pt, distance, kNonHWordUnderline );

		pgReleasePlatformDevice(pg->globals->current_port);
	}
}

static PG_PASCAL (void) spell_init_proc (paige_rec_ptr pg, style_info_ptr style, font_info_ptr font)
{
//	register short distance;

	pgStyleInitProc(pg, style, font);
//	distance = 0; //style->styles[boxed_var];
//	style->ascent += distance;
//	style->descent += distance;
	style->class_bits |= NO_SMART_DRAW_BIT;
}

 
static PG_PASCAL(void) excerpt_glitter_proc
(
    paige_rec_ptr      pg,
    style_walk_ptr     walker,
    long               line_number,
    long               par_number,
    text_block_ptr     block,
    point_start_ptr    first_line,
    point_start_ptr    last_line,
    point_start_ptr    previous_first,
    point_start_ptr    previous_last,
    co_ordinate_ptr    offset_extra,
    rectangle_ptr      vis_rect,
    short              call_verb
)
{
    co_ordinate begin, end;
	COLORREF lineColor;

    if (call_verb == glitter_post_bitmap_draw)
        return;

    color_value bk_color;
    bk_color = pg->bk_color;

	if ( GetIniShort(IDS_INI_USE_SYSCOLORS) )
		lineColor = GetSysColor( COLOR_BTNSHADOW );
	else
		lineColor = pgGetGrayScale(&bk_color, -54); //BORDER_DARK_SCALE);

    begin.v = first_line->bounds.top_left.v + offset_extra->v;
    end.v   = first_line->bounds.bot_right.v + offset_extra->v - VBAR_LINEWIDTH;
    begin.h = end.h = EXCERPT_OFFSET + offset_extra->h;  

    long n = walker->cur_par_style->user_data;

    while ( n-- > 0 ) {
        pgLineDraw(pg->globals->current_port, &begin, &end, lineColor, VBAR_LINEWIDTH);
        begin.h += EXCERPT_OFFSET;
    }
}

static PG_PASCAL(void) signed_glitter_proc
(
    paige_rec_ptr      pg,
    style_walk_ptr     walker,
    long               line_number,
    long               par_number,
    text_block_ptr     block,
    point_start_ptr    first_line,
    point_start_ptr    last_line,
    point_start_ptr    previous_first,
    point_start_ptr    previous_last,
    co_ordinate_ptr    offset_extra,
    rectangle_ptr      vis_rect,
    short              call_verb
)
{
    co_ordinate begin, end;
    COLORREF pen_color;

    if (call_verb == glitter_post_bitmap_draw)
        return;

    color_value bk_color;
    bk_color  = pg->bk_color;
    pen_color = 0x000000FF; 
    pen_color |= pg->port.palette_select;

    begin.v = first_line->bounds.top_left.v + offset_extra->v;
    end.v   = first_line->bounds.bot_right.v + offset_extra->v - VBAR_LINEWIDTH;
    begin.h = end.h = EXCERPT_OFFSET + offset_extra->h;  

    pgLineDraw(pg->globals->current_port, &begin, &end, pen_color, VBAR_LINEWIDTH);
}

PG_PASCAL (void) DrawSigSeparator( paige_rec_ptr pg, Point fromPt,
      short lineLength, long lineWidth )
{
	// no sig separator when printing [?]
	if ( (pg->flags & PRINT_MODE_BIT) != 0 )
		return;

	HDC hdc = pgGetPlatformDevice(pg->globals->current_port);

	// draw the "lit" part of the line
	COLORREF penColor = GetSysColor( COLOR_BTNFACE ); 
	penColor |= pg->port.palette_select;
	int penType = PS_SOLID;
	long penSize = lineWidth - 1;

	pgScaleLong( pg->scale_factor.scale, 0, &penSize );

	if ( !penSize )
	  penSize = 1;

	HPEN pen = CreatePen( penType, (short)penSize, penColor );
	HPEN oldPen = (HPEN) SelectObject( hdc, pen );

	MoveToEx( hdc, fromPt.h, fromPt.v, NULL );
	LineTo( hdc, fromPt.h + lineLength, fromPt.v );

	SelectObject( hdc, oldPen );
	DeleteObject( pen );

	// now give it a shadow---source from the top
	fromPt.v = (short)(fromPt.v - (lineWidth - 1));

	penColor = GetSysColor( COLOR_BTNSHADOW ); 
	penColor |= pg->port.palette_select;
	penType = PS_SOLID;
	penSize = 1;

	pen = CreatePen( penType, (short)penSize, penColor );
	oldPen = (HPEN) SelectObject( hdc, pen );

	MoveToEx( hdc, fromPt.h, fromPt.v, NULL );
	LineTo( hdc, fromPt.h + lineLength, fromPt.v );

	SelectObject( hdc, oldPen );
	DeleteObject( pen );

	pgReleasePlatformDevice(pg->globals->current_port);
}


// how much space between the sig and body?
inline long sig_spacer() {
	return GetMessageFont().CellHeight();
}

static PG_PASCAL(void) signature_glitter_proc
(
    paige_rec_ptr      pg,
    style_walk_ptr     walker,
    long               line_number,
    long               par_number,
    text_block_ptr     block,
    point_start_ptr    first_line,
    point_start_ptr    last_line,
    point_start_ptr    previous_first,
    point_start_ptr    previous_last,
    co_ordinate_ptr    offset_extra,
    rectangle_ptr      vis_rect,
    short              call_verb
)
{
    if (call_verb == glitter_post_bitmap_draw)
        return;

	if ( first_line->flags & NEW_PAR_BIT ) {
		select_pair sp;
		sp.begin = sp.end = first_line->offset - 1;

		if ( !CPaigeStyle(pg->doc_pg).IsSignature(&sp) ) {
			long sigExtra = sig_spacer() / 2;

			rectangle visBounds;
			pgAreaBounds( pg->doc_pg, NULL, &visBounds );
			long lineLength = visBounds.bot_right.h - visBounds.top_left.h;

			Point pt;
			pt.h = (short)(first_line->bounds.top_left.h - 1 + offset_extra->h);
			pt.v = (short)(first_line->bounds.top_left.v + sigExtra + offset_extra->v);

			DrawSigSeparator( pg, pt, (short)(lineLength + 1), SEPARATOR_LINEWIDTH );
		}
	}
}

extern "C" PG_PASCAL(void) PGS_MainGlitterProc
(
    paige_rec_ptr      pg,
    style_walk_ptr     walker,
    long               line_number,
    long               par_number,
    text_block_ptr     block,
    point_start_ptr    first_line,
    point_start_ptr    last_line,
    point_start_ptr    previous_first,
    point_start_ptr    previous_last,
    co_ordinate_ptr    offset_extra,
    rectangle_ptr      vis_rect,
    short              call_verb
)
{
    // first call the default paige impl---for par borders and tables
    pgLineGlitterProc( pg, walker, line_number, par_number, block, first_line, last_line,
                       previous_first, previous_last, offset_extra, vis_rect, call_verb );

    //
    // Now pass off to our own procs for a little extra fluff. Currently, we are doing one
    // of these to the exclusion of the other, but that's just a signed vs. excerpt thing.
    // Do as many as ya want---as long as everybody respects everybody else's space!
    //

    par_info_ptr parInfo = walker->cur_par_style;

    if ( parInfo->user_id & PAIGE_FORMAT_USER_ID_EXCERPT ) {
        excerpt_glitter_proc( pg, walker, line_number, par_number, block, first_line, last_line,
                              previous_first, previous_last, offset_extra, vis_rect, call_verb );
    }
    else if ( parInfo->user_id & PAIGE_FORMAT_USER_ID_SIGNED ) {
        signed_glitter_proc( pg, walker, line_number, par_number, block, first_line, last_line,
                             previous_first, previous_last, offset_extra, vis_rect, call_verb );
    }
    else if ( parInfo->user_id & PAIGE_FORMAT_USER_ID_SIGNATURE ) {
        signature_glitter_proc( pg, walker, line_number, par_number, block, first_line, last_line,
                             previous_first, previous_last, offset_extra, vis_rect, call_verb );
    }
}


extern "C" PG_PASCAL(void) PGS_MainLineAdjustProc
(
	paige_rec_ptr      pg,
	pg_measure_ptr     measure,
	point_start_ptr    starts,
	pg_short_t         num_starts,
	rectangle_ptr      line_fit,
	par_info_ptr       par_format
)
{
	ASSERT(par_format);

	//	We can't do anything without a valid par_format
	if (!par_format)
		return;
	
	//	Detect the beginning or end of any HTML list type (bullets, indents, etc.)
	bool				bIsOffsetItemChange = false;
	style_walk_ptr		walker = measure->styles;
	long				nLineStartOffset = measure->block->begin + starts->offset;

	if ( walker && walker->par_base && (starts->flags & NEW_PAR_BIT) && (nLineStartOffset > 0) )
	{
		//	We only offset the top, so we only care about paragraphs that have
		//	a previous paragraph (i.e. not the first paragraph)
		par_info_ptr	prev_par_format = pgFindParStyle(pg, nLineStartOffset - 1);
		
		if (prev_par_format)
		{
			//	Check to see if the paragraph formats are different
			if (prev_par_format != par_format)
			{
				long	cur_html_bullet = par_format->html_bullet;
				long	prev_html_bullet = prev_par_format->html_bullet;
				long	cur_html_style = par_format->html_style & 0x0000FFFF;
				long	prev_html_style = prev_par_format->html_style & 0x0000FFFF;
				
				//	It's an offset change if:
				//	* Current item is bulleted and previous item was not bulleted
				//	* Current item is not bulleted and previous item was bulleted
				//	* Current item is a list type (i.e. indented, etc.) and previous item was not a list type
				//	* Current item is not a list type (i.e. indented, etc.) and previous item was a list type
				bIsOffsetItemChange =
					( (cur_html_bullet > 0) && (prev_html_bullet == 0) ) ||
					( (cur_html_bullet == 0) && (prev_html_bullet > 0) ) ||
					( (cur_html_style != html_par_normal) && (prev_html_style == html_par_normal) ) ||
					( (cur_html_style == html_par_normal) && (prev_html_style != html_par_normal) );
			}

			UnuseMemory(pg->par_formats);
		}

		//	Space HTML list types (e.g. bullets and indented blocks) down by LIST_OFFSET
		if (bIsOffsetItemChange)
			par_format->top_extra += LIST_OFFSET;
	}
		
	// first call default paige impl to do a whole lot of nasty-lookin' stuff!
	pgLineAdjustProc( pg, measure, starts, num_starts, line_fit, par_format );

	//	The paragraph formats are often shared among multiple paragraphs. If we kept
	//	the offset in place it would incorrectly offset subsequent HTML list items
	//	rather than just offsetting before the first item and after the last item.
	//	Restore top_extra to its previous value.
	if (bIsOffsetItemChange)
		par_format->top_extra -= LIST_OFFSET;

	// not a whole lot to this routine for now. the only thing we adjust at
	// this point is the first line of an inline sig, by moving it down a
	// little to create some padding between it and the message body
	if ( !GetIniShort( IDS_INI_INLINE_SIGNATURE ) )
		return;

	if ( CPaigeStyle::IsSignature(par_format) && starts->flags & NEW_PAR_BIT ) {
		select_pair sp;
		sp.begin = sp.end = starts->offset - 1;

		if ( !CPaigeStyle(pg->doc_pg).IsSignature(&sp) ) {
			co_ordinate top_left;
			long sigExtra = sig_spacer();

			for ( pg_short_t i = 0; i < num_starts; ++i ) {
				top_left = starts[i].bounds.top_left;
				pgOffsetRect( &starts[i].bounds, 0, sigExtra );
				starts[i].bounds.top_left = top_left;
			}

			top_left = line_fit->top_left;
			pgOffsetRect( line_fit, 0, sigExtra );
			line_fit->top_left = top_left;

			top_left = measure->actual_rect.top_left;
			pgOffsetRect( &measure->actual_rect, 0, sigExtra );
			measure->actual_rect.top_left = top_left;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// CPaigeStyle -- class for handling custom message styles

CPaigeStyle::CPaigeStyle(pg_ref pg)
{
	m_paigeRef = pg;
}

CPaigeStyle::~CPaigeStyle()
{
	return;
}


// BOG -- code for marking items that need to be spell-checked, in addition
// to items done on the fly. the custom style being applied is a "hidden"
// style; it differs from the current style only in the `user_id' field.

bool CPaigeStyle::SetNeedsScanned( bool bOn, select_pair_ptr pSel )
{
    style_info mask, info;

    pgInitStyleMask( &info, 0 );
    pgInitStyleMask( &mask, 0 );
	//SDSpellTRACE("SetNeedsSpell %d %d.%d\n",bOn,pSel->begin,pSel->end);
    mask.user_id    = -1;
    mask.class_bits = -1;
    pgGetStyleInfo( m_paigeRef, pSel, FALSE, &info, &mask );
    pgInitStyleMask( &mask, 0 );
    mask.user_id    = -1;
    mask.class_bits = -1;

//    mask.procs.init = (style_init_proc) -1;
//    mask.procs.draw = (text_draw_proc) -1;
//    info.procs.init = pgStyleInitProc;
//    info.procs.draw = pgDrawProc;
    
    if ( bOn )
	{
		info.user_id |= PAIGE_STYLE_USER_ID_NEEDSSCANNED;
		info.user_id &= ~PAIGE_STYLE_USER_ID_MISSPELLED;
	}
	else
	{
        //info.user_id = 0;
        if ( !(info.user_id & PAIGE_STYLE_USER_ID_NEEDSSCANNED) )
            return false;
	    info.user_id ^= PAIGE_STYLE_USER_ID_NEEDSSCANNED;
	}	
	// Set the style.  If it was misspelled before, we'll have to draw.
	// If it wasn't misspelled before, there's no reason to draw, because
	// the needspellcheck style isn't visible anyway.
    pgSetStyleInfo( m_paigeRef, pSel, &info, &mask, IsMisspelled(pSel) ? (short)best_way : (short)draw_none );

    return TRUE;	// How useful.  It always returns true, no matter what.
}

// two ways of checking for the "needs spell-check" style.
//
// the easiest possible check is to set a select_pair to begin=0, end=pgTextSize(...),
// and pass it to NeedsSpellCheck( select_pair_ptr ) to return whether the style
// exists in the document at all. to find exact text ranges to check, one must use
// a style-walker.
#if 0    // this is in the header file now
static bool CPaigeStyle::NeedsSpellCheck( style_info_ptr pStyle )
{
    return (pStyle->user_id == PAIGE_STYLE_USER_ID_NEEDSSCANNED);
}
#endif

bool CPaigeStyle::NeedsScanned( select_pair_ptr sel )
{
    pgInitStyleMask( &m_style, 0 );
    pgInitStyleMask( &m_styleMask, 0 );
    m_styleMask.user_id    = -1;
    pgGetStyleInfo( m_paigeRef, sel, FALSE, &m_style, &m_styleMask );

    return ((m_style.user_id & PAIGE_STYLE_USER_ID_NEEDSSCANNED) &&
	    (m_styleMask.user_id != 0));
}


bool CPaigeStyle::ApplyBadMoodWord(bool bOn, select_pair_ptr pSel,int nWordType)
{
	style_info	mask, info;
	pgInitStyleMask(&info,0);
	pgInitStyleMask(&mask,0);

	//SDSpellTRACE("ApplyMisspelled %d %d.%d\n",bOn,pSel?pSel->begin:-1,pSel?pSel->end:-1);
	mask.user_id=-1;
	mask.class_bits = -1;
	//mask.procs.init = (style_init_proc) -1;
	mask.procs.draw = (text_draw_proc) -1;
    pgGetStyleInfo( m_paigeRef, pSel, FALSE, &info, &mask );
	pgInitStyleMask(&mask,0);
	mask.user_id=-1;
	mask.class_bits = -1;
	mask.procs.draw = (text_draw_proc) -1;
	//mask.bot_extra = -1;
		
	if ( bOn )
	{
		// make sure the drawing globals are happy
		PGS_InitDrawGlobals();

		info.user_id |= PAIGE_STYLE_USER_ID_BAD_MOOD_WORD;
		//info.procs.init= spell_init_proc;
//		BITMAP bm;
		int brushHeight = 0;

		if (nWordType == MOOD_H_WORD)
		{
//			g_bmpHWord.GetBitmap( &bm );
			brushHeight = kHWordBrushHeight;
			info.procs.draw = draw_h_word_proc;
		}
		else if (nWordType == MOOD_NON_H_WORD)
		{
//			g_bmpNonHWord.GetBitmap( &bm );
			brushHeight = kNonHWordBrushHeight;
			info.procs.draw = draw_non_h_word_proc;
		}

//		info.bot_extra = bm.bmHeight - 1;,
		//info.bot_extra = brushHeight - 1;

	}
	else
	{
        if ( !(info.user_id & PAIGE_STYLE_USER_ID_BAD_MOOD_WORD) )
            return false;
        info.user_id ^= PAIGE_STYLE_USER_ID_BAD_MOOD_WORD;

		//info.user_id=0;
		//info.procs.init= pgStyleInitProc;
		info.procs.draw = pgDrawProc;
		//info.bot_extra = 0;
	}

	// all these shenanigans figure out if the window is visible
	paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
	PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
	bool isVisible = 0!=(pSB->pWndOwner->GetStyle() & WS_VISIBLE);
	UnuseMemory( m_paigeRef );

	pgSetStyleInfo(m_paigeRef,pSel,&info,&mask, (short)(isVisible? best_way:draw_none));

	return TRUE;
}

bool CPaigeStyle::IsBadMoodWord(style_info_ptr pStyle)
{
	return (pStyle->user_id & PAIGE_STYLE_USER_ID_BAD_MOOD_WORD) != 0;
}

bool CPaigeStyle::IsBadMoodWord(select_pair_ptr sel)
{
	pgInitStyleMask(&m_style,0);
	pgInitStyleMask(&m_styleMask,0);
	// This is how we check if ANYTHING in the selection is misspelled;
	// fill in the user_id we like, and call pgGSI with TRUE
	//m_styleMask.user_id = -1;
	//m_style.user_id = PAIGE_STYLE_USER_ID_BAD_MOOD_WORD;
	//pgGetStyleInfo(m_paigeRef, sel, TRUE, &m_style, &m_styleMask);
	//	return ( (m_style.user_id & PAIGE_STYLE_USER_ID_BAD_MOOD_WORD ) && (m_styleMask.user_id != 0 ) );
	m_styleMask.user_id = -1;
	pgGetStyleInfo(m_paigeRef, sel, FALSE, &m_style, &m_styleMask);
	return ((m_style.user_id & PAIGE_STYLE_USER_ID_BAD_MOOD_WORD) != 0);

}



// end BOG

//Wrote following 3 functions while fixing some bug#5874 Hyperlinking Test with spelling errrow causes broken link.
//Turns out we don't need this functions at this moment but might be useful in future so doing #if 0.
#if 0
bool CPaigeStyle::ApplyURL(bool bOn, select_pair_ptr pSel)
{
	style_info	mask, info;
	pgInitStyleMask(&info,0);
	pgInitStyleMask(&mask,0);

	//SDSpellTRACE("ApplyMisspelled %d %d.%d\n",bOn,pSel?pSel->begin:-1,pSel?pSel->end:-1);
	mask.user_id=-1;
	mask.class_bits = -1;
	mask.procs.init = (style_init_proc) -1;
	mask.procs.draw = (text_draw_proc) -1;
		
	if ( bOn )
	{
		info.user_id |= PAIGE_STYLE_USER_ID_URL;
		info.procs.init= spell_init_proc;
		info.procs.draw = spell_draw_proc;
	}
	else
	{
		info.user_id=0;
		info.procs.init= pgStyleInitProc;
		info.procs.draw = pgDrawProc;
	}

	// all these shenanigans figure out if the window is visible
	paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
	PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
	bool isVisible = 0!=(pSB->pWndOwner->GetStyle() & WS_VISIBLE);
	UnuseMemory( m_paigeRef );

	pgSetStyleInfo(m_paigeRef,pSel,&info,&mask, (short)(isVisible? best_way:draw_none));

	return TRUE;
}

bool CPaigeStyle::IsURL(style_info_ptr pStyle)
{
	return (pStyle->user_id & PAIGE_STYLE_USER_ID_URL);
}

bool CPaigeStyle::IsURL(select_pair_ptr sel)
{
	pgInitStyleMask(&m_style,0);
	pgInitStyleMask(&m_styleMask,0);
	// This is how we check if ANYTHING in the selection is misspelled;
	// fill in the user_id we like, and call pgGSI with TRUE
	m_style.user_id = PAIGE_STYLE_USER_ID_URL;
	pgGetStyleInfo(m_paigeRef, sel, TRUE, &m_style, &m_styleMask);

	return ( (m_style.user_id & PAIGE_STYLE_USER_ID_URL ) && (m_styleMask.user_id != 0 ) );
}
#endif 

bool CPaigeStyle::ApplyMisspelled(bool bOn, select_pair_ptr pSel, bool bRedrawImmediately)
{
	style_info	mask, info;
	pgInitStyleMask(&info,0);
	pgInitStyleMask(&mask,0);

	//SDSpellTRACE("ApplyMisspelled %d %d.%d\n",bOn,pSel?pSel->begin:-1,pSel?pSel->end:-1);
	mask.user_id=-1;
	mask.class_bits = -1;
	mask.procs.init = (style_init_proc) -1;
	mask.procs.draw = (text_draw_proc) -1;

    pgGetStyleInfo( m_paigeRef, pSel, FALSE, &info, &mask );
	pgInitStyleMask(&mask,0);

	mask.user_id=-1;
	mask.class_bits = -1;
	mask.procs.init = (style_init_proc) -1;
	mask.procs.draw = (text_draw_proc) -1;
	//mask.bot_extra = -1;

	if ( bOn )
	{
		// Don't want to mark URLs, bad mood words, or embedded objects as mispelled
		if ( IsWithinURLRange(pSel)|| IsBadMoodWord(pSel) || info.embed_object )
			return false;
		PGS_InitDrawGlobals();
#if 0
		BITMAP bm;
		g_bmpSpell.GetBitmap( &bm );
#endif
		info.user_id |= PAIGE_STYLE_USER_ID_MISSPELLED;
		info.procs.init= spell_init_proc;
		info.procs.draw = spell_draw_proc;

		// BOG: the underline gets drawn starting at the last pixel row of
		// the actual text, so we pad the line for one less pixel.
//		info.bot_extra = bm.bmHeight - 1;
		//info.bot_extra = kSpellBrushHeight - 1;
	}
	else
	{
        info.user_id &= ~PAIGE_STYLE_USER_ID_MISSPELLED;
		info.procs.init= pgStyleInitProc;
		info.procs.draw = pgDrawProc;
		//info.bot_extra = 0;
	}

	bool	bRedraw = false;
	
	if (bRedrawImmediately)
	{
		// all these shenanigans figure out if the window is visible
		paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
		PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;

		bRedraw = ( (pSB->pWndOwner->GetStyle() & WS_VISIBLE) != 0 );
		
		UnuseMemory( m_paigeRef );
	}

	pgSetStyleInfo( m_paigeRef,pSel,&info,&mask, static_cast<short>(bRedraw ? best_way : draw_none) );

	return true;
}

bool CPaigeStyle::IsMisspelled(style_info_ptr pStyle)
{
	return (pStyle->user_id & PAIGE_STYLE_USER_ID_MISSPELLED);
}

bool CPaigeStyle::IsMisspelled(select_pair_ptr sel)
{
	pgInitStyleMask(&m_style,0);
	pgInitStyleMask(&m_styleMask,0);
	// This is how we check if ANYTHING in the selection is misspelled;
	// fill in the user_id we like, and call pgGSI with TRUE
	m_style.user_id = PAIGE_STYLE_USER_ID_MISSPELLED;
	pgGetStyleInfo(m_paigeRef, sel, TRUE, &m_style, &m_styleMask);

	//SDSpellTRACE("IsMisspelled %d.%d %d\n", sel->begin, sel->end, (m_style.user_id & PAIGE_STYLE_USER_ID_SPELL ) && (m_styleMask.user_id != 0 ) );
	return ( (m_style.user_id & PAIGE_STYLE_USER_ID_MISSPELLED ) && (m_styleMask.user_id != 0 ) );
}

void CPaigeStyle::ClearMisspellingAndRecheck(select_pair_ptr sel)
{
	select_pair sel2, sel3;

	//SDSpell{static int n; n++; TRACE("ClearM&R%d %x\n",n,sel);}

	// Use current selection if no selection given
	if (sel) sel2 = *sel;
	else pgGetSelection(m_paigeRef,&sel2.begin,&sel2.end);
	
	// mark the whole word for checking
	sel3 = sel2;
    pgFindWord(m_paigeRef, sel2.begin, &sel2.begin, &sel2.end, TRUE, FALSE);
	// if the original range was more than one word, make sure we get it all
	if (sel3.begin != sel3.end && sel3.end > sel2.end)
	{
		pgFindWord(m_paigeRef, sel3.end, &sel3.begin, &sel3.end, TRUE, FALSE);
		sel2.end = sel3.end;
	}

	// Back up until just after a blank character so that we can be sure
	// that we're marking entire emoticon trigger potentials as needing
	// to be scanned (otherwise trigger characters are often consider word
	// breaking characters and so we would incorrectly skip scanning them).
	sel2.begin = PgScanBackwardsToJustAfterBlank(m_paigeRef, sel2.begin);

    SetNeedsScanned( TRUE, &sel2 );

#ifndef VOODOO_IS_DEAD
	// The following is voodoo, pure and simple.
	// For some reason, this incantation, while it should be
	// a complete no-op, fixes a big big big performance problem
	// during typing.  Someone should probably figure out why, but that
	// someone ain't me, at least not now.  SD 4/19/99
	
	// all this virgin goat slaying is hard on fonts & styles in the insertion point, so save them
	style_info oldStyleInfo, oldStyleMask;
	pgInitStyleMask( &oldStyleInfo, 0 );
    pgInitStyleMask( &oldStyleMask, 0 );
    pgGetStyleInfo( m_paigeRef, NULL, FALSE, &oldStyleInfo, &oldStyleMask );

	// Get the current selection
	pgGetSelection(m_paigeRef, &sel2.begin, &sel2.end);			// first, we get a virgin chicken
	// Set the selection to just what it was
	pgSetSelection(m_paigeRef, sel2.begin, sel2.end, 0, FALSE);	// then we slit its throat and pour the blood on the fire

	// now we clean up some of the offal, hoping that it wasn't really the intestines that placate the gods
	pgInitStyleMask( &oldStyleMask, 0);	// clear the mask we just got
	// restore what we believe is good
	oldStyleMask.styles[bold_var] = -1;
	oldStyleMask.styles[italic_var] = -1;
	oldStyleMask.styles[underline_var] = -1;
	oldStyleMask.styles[strikeout_var] = -1;
	oldStyleMask.fg_color.red = 0xFFFF;
	oldStyleMask.fg_color.green = 0xFFFF;
	oldStyleMask.fg_color.blue = 0xFFFF;
	oldStyleMask.bk_color.red = 0xFFFF;
	oldStyleMask.bk_color.green = 0xFFFF;
	oldStyleMask.bk_color.blue = 0xFFFF;
	oldStyleMask.font_index = -1;
	oldStyleMask.point = -1;
	//dont restore the bad mood word id 
	oldStyleMask.user_id = -1;
	oldStyleInfo.user_id  &= ~PAIGE_STYLE_USER_ID_BAD_MOOD_WORD;
	pgSetStyleInfo( m_paigeRef, NULL, &oldStyleInfo, &oldStyleMask, best_way);

	// Ok, we now return to the rational universe
#endif

    // Make sure the insertion style does not contain misspelling
	if (IsMisspelled((select_pair_ptr)NULL))
	{
		ApplyMisspelled(FALSE,NULL);
	}
}

bool CPaigeStyle::IsExcerpt(par_info_ptr pPar)
{
	return ( pPar->user_id & PAIGE_FORMAT_USER_ID_EXCERPT );
}

bool CPaigeStyle::IsExcerpt(select_pair_ptr sel)
{
	pgInitParMask(&m_par,0);
	pgInitParMask(&m_parMask,0);
	pgGetParInfo( m_paigeRef, sel, false, &m_par, &m_parMask );
	
	return ( IsExcerpt(&m_par) );
}

//
// Accepts a currently filled paragraph format structure and 
// modifies it to the appropriate style.
//
bool CPaigeStyle::ApplyExcerpt(par_info_ptr info, bool bOn)
{
    if ( bOn )
    {
        info->user_id |= PAIGE_FORMAT_USER_ID_EXCERPT;
        info->user_data++;
        info->left_extra = info->user_data * EXCERPT_OFFSET;
    }
    else
    {
        // If we weren't in an excerpt, then bail
        if (!(info->user_id & PAIGE_FORMAT_USER_ID_EXCERPT))
            return FALSE;

        info->user_data--;
        //info->user_data = max(info->user_data,0);
		info->user_data = (((info->user_data) > (0)) ? (info->user_data) : (0)); 
        info->left_extra = info->user_data * EXCERPT_OFFSET;

        // If no more exerpt levels then turn excerpting off
        if ( info->user_data == 0 )
        {
            info->user_id = 0;          // Change this to an and/or operation
        }
    }

    return TRUE;
}

bool CPaigeStyle::SetExcerpt(int n)
{
    pgInitParMask(&m_par,0);
    pgInitParMask(&m_parMask,0);

    m_parMask.user_id=-1;
    m_parMask.user_data=-1;
    m_parMask.left_extra =-1;

    pgGetParInfo( m_paigeRef, NULL, false, &m_par, &m_parMask );

    if ( n == 0 )
    {
        m_par.user_data = 0;
        m_par.left_extra = 0;
        m_par.user_id = 0;              // Change this to an and/or operation
    }
    else
    {
        m_par.user_id |= PAIGE_FORMAT_USER_ID_EXCERPT;
        m_par.user_data = n;
        m_par.left_extra = m_par.user_data * EXCERPT_OFFSET;
    }

    pgSetParInfo(m_paigeRef,NULL,&m_par,&m_parMask, best_way);
    return TRUE;
}

bool CPaigeStyle::IsBullet(select_pair_ptr sel)
{
	pgInitParMask(&m_par,0);
	pgInitParMask(&m_parMask,0);
	pgGetParInfo( m_paigeRef, sel, false, &m_par, &m_parMask );

	return ( (m_par.class_info & BULLETED_LINE) && (m_par.html_style & html_unordered_list) );
}

long CPaigeStyle::GetBulletLevel(select_pair_ptr sel)
{
	long	nBulletLevel = 0;

	if ( IsBullet(sel) )
	{
		//	We retrieved information into m_par in IsBullet
		nBulletLevel = m_par.html_bullet;

		//	We know there's a bullet so make sure that the level is at least 1
		if (nBulletLevel <= 0)
			nBulletLevel = 1;
	}

	return nBulletLevel;
}

bool CPaigeStyle::ApplyBullet(bool bOn)
{
 	par_info	info, mask;
	pgInitParMask(&info,0);
	pgInitParMask(&mask,0);
	pgGetParInfo( m_paigeRef, NULL, false, &info, &mask );

	mask.html_bullet = -1;
	mask.class_info = -1;
	mask.indents.left_indent = -1;
	mask.html_style = -1;
	
	if ( bOn )
	{
		info.class_info |= BULLETED_LINE;
		info.html_bullet = 1;
		info.indents.left_indent += DEFLIST_INDENT_VALUE;
		info.html_style = html_unordered_list;
	} else
	{
		info.html_bullet = 0; //max(0,info.html_bullet-2);
		info.indents.left_indent -= DEFLIST_INDENT_VALUE;
		if ( !info.html_bullet )
		{
			info.html_style = (info.indents.left_indent >= DEFLIST_INDENT_VALUE) ? html_definition_list : 0;
			info.class_info = 0;
		}
	}
	
	pgSetParInfo(m_paigeRef, NULL, &info, &mask, best_way);

	return TRUE;
}

bool CPaigeStyle::ApplySigned(bool bOn)
{
    par_info    mask, info;
    pgInitParMask(&info,0);
    pgInitParMask(&mask,0);

    mask.user_id=-1;
    mask.left_extra =-1;

    pgGetParInfo( m_paigeRef, NULL, false, &info, &mask );

    if ( ApplySigned(&info, bOn) )
        pgSetParInfo(m_paigeRef,NULL,&info,&mask, best_way);

    return TRUE;
}

bool CPaigeStyle::ApplySigned(par_info_ptr info, bool bOn)
{
    if ( bOn )
    {
        info->user_id |= PAIGE_FORMAT_USER_ID_SIGNED;
        info->left_extra = EXCERPT_OFFSET;
    }
    else
    {
        // If we weren't in an excerpt, then bail
        if (!(info->user_id & PAIGE_FORMAT_USER_ID_EXCERPT))
            return FALSE;

        m_par.left_extra = 0;
        m_par.user_id = 0;              // Change this to an and/or operation
    }

    return TRUE;
}


bool CPaigeStyle::IsSigned(par_info_ptr pPar)
{
	return ( pPar->user_id & PAIGE_FORMAT_USER_ID_EXCERPT );
}


// BOG: inline signature support
bool CPaigeStyle::ApplySignature( bool bOn /*= true*/, select_pair_ptr sel /*= 0*/ )
{
    par_info mask, info;
    pgInitParMask( &info, 0 );
    pgInitParMask( &mask, 0 );

    mask.user_id = -1;
    pgGetParInfo( m_paigeRef, sel, false, &info, &mask );

    pgInitParMask( &mask, 0 );
    mask.user_id = -1;

    if ( ApplySignature( &info, bOn ) )
        pgSetParInfo( m_paigeRef, sel, &info, &mask, best_way );

    return true;
}

bool CPaigeStyle::ApplySignature( par_info_ptr info, bool bOn /*= true*/ )
{
    if ( bOn ) {
        info->user_id |= PAIGE_FORMAT_USER_ID_SIGNATURE;
    }
    else {
        // If we aren't in a sig, then bail
        if ( !(info->user_id & PAIGE_FORMAT_USER_ID_SIGNATURE) )
            return false;

        info->user_id ^= PAIGE_FORMAT_USER_ID_SIGNATURE;
    }

    return true;
}

bool CPaigeStyle::IsSignature( par_info_ptr pPar )
{
	return (pPar->user_id & PAIGE_FORMAT_USER_ID_SIGNATURE) != 0;
}

bool CPaigeStyle::IsSignature( select_pair_ptr sel )
{
    par_info mask, info;
    pgInitParMask( &info, 0 );
    pgInitParMask( &mask, 0 );
    mask.user_id = -1;
    pgGetParInfo( m_paigeRef, sel, false, &info, &mask );
    return IsSignature( &info );
}


bool CPaigeStyle::IsWithinURLRange( select_pair_ptr within /*= NULL*/ )
{
	bool bRetVal = false;
	long startOffset, searchLimit;

	if ( within ) {
		startOffset = within->begin;
		searchLimit = within->end;
	}
	else {
		/*startOffset = 0;
		searchLimit = pgTextSize( m_paigeRef );*/
		return bRetVal;
	}

	style_walk sw;
	paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
	pgPrepareStyleWalk( prp, startOffset, &sw, false );
	UnuseMemory( m_paigeRef );
	
	for ( ; sw.current_offset < searchLimit; ) 
	{
		if ( sw.hyperlink->unique_id ) 
		{
			if (within->begin >= sw.hyperlink->applied_range.begin &&
 				within->begin <= sw.hyperlink->applied_range.end)
			{
				bRetVal = true;
				break;
			}
		}
		else
			break;
		if ( sw.next_style_run->offset <= sw.t_length )
			pgWalkStyle( &sw, sw.hyperlink->applied_range.end - sw.current_offset );
		else
			break;
	}

	pgPrepareStyleWalk( prp, 0, NULL, false );
	return bRetVal;
}

