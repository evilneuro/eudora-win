#include "stdafx.h"

#include "defprocs.h"
#include "pgutils.h"
#include "paige.h"
#include "pgtraps.h"
#include "machine.h"
#include "pggrafx.h"
#include "PaigeStyle.h"
#include "pghtmdef.h"
#include "PgStuffBucket.h"

#define EXCERPT_OFFSET 10




#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


PG_PASCAL (void) pgDrawSpellUnderline (paige_rec_ptr pg, Point from_pt,
      short distance)
{
	Point          start_pt;
	HDC            hdc;
	HPEN           underline_pen, old_pen;
	COLORREF       pen_color;
	long           pen_size;
//	short          ascent;
	int               pen_type;

	// No misspelling underlines when printing
	if ((pg->flags & PRINT_MODE_BIT) != 0)
		return;

	//ascent = (short)style->ascent;
	start_pt = from_pt;

	hdc = pgGetPlatformDevice(pg->globals->current_port);

	pen_type = PS_SOLID;

			
//	pgColorToOS(&style->fg_color, &pen_color);
	pen_color = 0x000000FF; 
	pen_color |= pg->port.palette_select;
	pen_size = 1;
	pgScaleLong(pg->scale_factor.scale, 0, &pen_size);

	if (!pen_size)
	  pen_size = 1;

	underline_pen = CreatePen(pen_type, (short)pen_size, pen_color);
	old_pen = (HPEN) SelectObject(hdc, underline_pen);

	MoveToEx(hdc, start_pt.h, start_pt.v, NULL);
	LineTo(hdc, start_pt.h + distance, start_pt.v);

//	if (draw_bits & DOUBLE_UNDERLINE) 
	{
	  start_pt.v = (short)(start_pt.v + pen_size * 2);
	  MoveToEx(hdc, start_pt.h, start_pt.v, NULL);
	  LineTo(hdc, start_pt.h + distance, start_pt.v);
	}

	SelectObject(hdc, old_pen);
	DeleteObject(underline_pen);

	pgReleasePlatformDevice(pg->globals->current_port);
}



static PG_PASCAL (void) spell_draw_proc(paige_rec_ptr pg, style_walk_ptr walker, 
									  pg_byte_ptr data, pg_short_t offset, pg_short_t length,
									  draw_points_ptr draw_position, long extra, short draw_mode)
{
	style_info_ptr original_style = walker->cur_style;
	pg_scale_factor scale = pg->scale_factor;
	Point start_pt;
	Point end_pt;

	pgDrawProc( pg, walker, data, offset, length, draw_position, extra, draw_mode);

	if (walker->cur_style->user_id & PAIGE_STYLE_USER_ID_SPELL)
	{
		start_pt.h = pgLongToShort(draw_position->from.h);
		start_pt.v = pgLongToShort(draw_position->from.v);

		end_pt.h = pgLongToShort(draw_position->to.h);
		end_pt.v = pgLongToShort(draw_position->to.v);

		pgDrawSpellUnderline(pg, start_pt, (short)(end_pt.h-start_pt.h));
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
    par_info_ptr par;
    co_ordinate begin, end;
    long grayscale_dark;

    if (call_verb == glitter_post_bitmap_draw)
        return;

    color_value bk_color;
    bk_color = pg->bk_color;
    grayscale_dark = pgGetGrayScale(&bk_color, -54); //BORDER_DARK_SCALE);

    begin.v = first_line->bounds.top_left.v + offset_extra->v;
    end.v   = first_line->bounds.bot_right.v + offset_extra->v -3;
    begin.h = end.h = EXCERPT_OFFSET + offset_extra->h;  

    par = walker->cur_par_style;
        
    long n=0;
    n = par->user_data;
    while ( n-- > 0 ) {
        pgLineDraw(pg->globals->current_port, &begin, &end, grayscale_dark, 3);
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
    end.v   = first_line->bounds.bot_right.v + offset_extra->v -3;
    begin.h = end.h = EXCERPT_OFFSET + offset_extra->h;  

    pgLineDraw(pg->globals->current_port, &begin, &end, pen_color, 3);
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
}


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

bool CPaigeStyle::SetNeedsSpellCheck( bool bOn, select_pair_ptr pSel )
{
    style_info mask, info;

    pgInitStyleMask( &info, 0 );
    pgInitStyleMask( &mask, 0 );
	//SDSpellTRACE("SetNeedsSpell %d %d.%d\n",bOn,pSel->begin,pSel->end);

    mask.user_id    = -1;
    mask.class_bits = -1;
//    mask.procs.init = (style_init_proc) -1;
//    mask.procs.draw = (text_draw_proc) -1;
//    info.procs.init = pgStyleInitProc;
//    info.procs.draw = pgDrawProc;
    
    if ( bOn )
        info.user_id = PAIGE_STYLE_USER_ID_NEEDSSPELLCHECK;	// This used to be |=.  However, we don't use &= ~ below, so why?
    else
        info.user_id = 0;

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
    return (pStyle->user_id == PAIGE_STYLE_USER_ID_NEEDSSPELLCHECK);
}
#endif

bool CPaigeStyle::NeedsSpellCheck( select_pair_ptr sel )
{
    pgInitStyleMask( &m_style, 0 );
    pgInitStyleMask( &m_styleMask, 0 );
    pgGetStyleInfo( m_paigeRef, sel, FALSE, &m_style, &m_styleMask );

    return ((m_style.user_id == PAIGE_STYLE_USER_ID_NEEDSSPELLCHECK) &&
	    (m_styleMask.user_id != 0));
}

// end BOG


bool CPaigeStyle::ApplyMisspelled(bool bOn, select_pair_ptr pSel)
{
	style_info	mask, info;
	pgInitStyleMask(&info,0);
	pgInitStyleMask(&mask,0);

	/*SDSpell*/TRACE("ApplyMisspelled %d %d.%d\n",bOn,pSel?pSel->begin:-1,pSel?pSel->end:-1);
	mask.user_id=-1;
	mask.class_bits = -1;
	mask.procs.init = (style_init_proc) -1;
	mask.procs.draw = (text_draw_proc) -1;
		
	if ( bOn )
	{
		info.user_id |= PAIGE_STYLE_USER_ID_SPELL;
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

bool CPaigeStyle::IsMisspelled(style_info_ptr pStyle)
{
	return (pStyle->user_id & PAIGE_STYLE_USER_ID_SPELL);
}

bool CPaigeStyle::IsMisspelled(select_pair_ptr sel)
{
	pgInitStyleMask(&m_style,0);
	pgInitStyleMask(&m_styleMask,0);
	// This is how we check if ANYTHING in the selection is misspelled;
	// fill in the user_id we like, and call pgGSI with TRUE
	m_style.user_id = PAIGE_STYLE_USER_ID_SPELL;
	pgGetStyleInfo(m_paigeRef, sel, TRUE, &m_style, &m_styleMask);

	//SDSpellTRACE("IsMisspelled %d.%d %d\n", sel->begin, sel->end, (m_style.user_id & PAIGE_STYLE_USER_ID_SPELL ) && (m_styleMask.user_id != 0 ) );
	return ( (m_style.user_id & PAIGE_STYLE_USER_ID_SPELL ) && (m_styleMask.user_id != 0 ) );
}

void CPaigeStyle::ClearMisspellingAndRecheck(select_pair_ptr sel)
{
	select_pair sel2, sel3;
	bool checkBefore = false;

	//SDSpell{static int n; n++; TRACE("ClearM&R%d %x\n",n,sel);}

	// Use current selection if no selection given
	if (sel) sel2 = *sel;
	else pgGetSelection(m_paigeRef,&sel2.begin,&sel2.end);
	
	// mark the whole word for checking
	sel3 = sel2;
    pgFindWord(m_paigeRef, sel2.begin, &sel2.begin,&sel2.end,TRUE,FALSE);
	// if the original range was more than one word, make sure we get it all
	if (sel3.begin != sel3.end && sel3.end > sel2.end)
	{
		pgFindWord(m_paigeRef,sel3.end,&sel3.begin,&sel3.end,TRUE,FALSE);
		sel2.end = sel3.end;
	}	
    SetNeedsSpellCheck( TRUE, &sel2 );

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
	oldStyleMask.fg_color.red = 0xFFFF;
	oldStyleMask.fg_color.green = 0xFFFF;
	oldStyleMask.fg_color.blue = 0xFFFF;
	oldStyleMask.bk_color.red = 0xFFFF;
	oldStyleMask.bk_color.green = 0xFFFF;
	oldStyleMask.bk_color.blue = 0xFFFF;
	oldStyleMask.font_index = -1;
	oldStyleMask.point = -1;
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
        info->user_data = max(info->user_data,0);
        info->left_extra = info->user_data * EXCERPT_OFFSET;

        // If no more exerpt levels then turn excerpting off
        if ( info->user_data == 0 )
        {
            info->user_id = 0;          // Change this to an and/or operation
        }
    }

    return TRUE;
}

//
// Applies the Excerpt style on the current selection
//
bool CPaigeStyle::ApplyExcerpt(bool bOn)
{
    par_info    mask, info;
    pgInitParMask(&info,0);
    pgInitParMask(&mask,0);

    mask.user_id=-1;
    mask.user_data=-1;
    mask.left_extra =-1;

    pgGetParInfo( m_paigeRef, NULL, false, &info, &mask );

    if ( ApplyExcerpt(&info, bOn) )
        pgSetParInfo(m_paigeRef,NULL,&info,&mask, best_way);

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
		info.html_bullet = 0;
		info.indents.left_indent += DEFLIST_INDENT_VALUE;
		info.html_style |= html_unordered_list;
	} else
	{
		info.html_bullet = 0; //max(0,info.html_bullet-2);
		info.indents.left_indent -= DEFLIST_INDENT_VALUE;
		if ( !info.html_bullet )
		{
			info.html_style = 0;
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
