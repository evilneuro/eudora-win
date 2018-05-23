#ifndef PAIGESTYLE_H
#define PAIGESTYLE_H

// These should be used as mask vaules for the  style_info->user_id field
// in a style_info struct.
//
#define PAIGE_STYLE_USER_ID_MISSPELLED       0x00000001
#define PAIGE_STYLE_USER_ID_NEEDSSCANNED     0x00000002
#define PAIGE_STYLE_USER_ID_URL				 0x00000004
#define PAIGE_STYLE_USER_ID_BAD_MOOD_WORD	 0x00000008

//
// These should be used as mask vaules for the  par_info->user_id field
// in a style_info struct.
//
#define PAIGE_FORMAT_USER_ID_EXCERPT      0x00000001
#define PAIGE_FORMAT_USER_ID_SIGNED       0x00000002
#define PAIGE_FORMAT_USER_ID_SIGNATURE    0x00000004


enum underline_type {
	kSpellUnderline,
	kHWordUnderline,
	kNonHWordUnderline,
	kBPUnderline
};

// draws the appropriate line for misspelling, mood-words, etc.
void PGS_DrawUnderline( HDC hDC, POINT* ptStart, int length, underline_type kind );
void PGS_InitDrawGlobals();


extern "C" PG_PASCAL(void) PGS_MainGlitterProc
(
    paige_rec_ptr pg,
    style_walk_ptr walker,
    long line_number,
    long par_number,
    text_block_ptr block,
    point_start_ptr first_line,
    point_start_ptr last_line,
    point_start_ptr previous_first,
    point_start_ptr previous_last,
    co_ordinate_ptr offset_extra,
    rectangle_ptr vis_rect,
    short call_verb
);

extern "C" PG_PASCAL(void) PGS_MainLineAdjustProc
(
	paige_rec_ptr      pg,
	pg_measure_ptr     measure,
	point_start_ptr    starts,
	pg_short_t         num_starts,
	rectangle_ptr      line_fit,
	par_info_ptr       par_format
);

class CPaigeStyle
{
public:
    CPaigeStyle(pg_ref pg);
    ~CPaigeStyle();

    static bool ApplyExcerpt(par_info_ptr info, bool bOn);
    static bool IsExcerpt(par_info_ptr par);

private:
    par_info m_par, m_parMask;
    style_info m_style, m_styleMask;
    pg_ref m_paigeRef;

public:
    bool SetExcerpt(int nLevel);
    bool IsExcerpt(select_pair_ptr sel);
        
    bool ApplyMisspelled(bool bOn, select_pair_ptr pSel = NULL, bool bRedrawImmediately = true);
    bool IsMisspelled(style_info_ptr info);
    bool IsMisspelled(select_pair_ptr sel);

//Wrote following 3 functions while fixing some bug#5874 Hyperlinking Test with spelling errrow causes broken link.
//Turns out we don't need this functions at this moment but might be useful in future so doing #if 0.
#if 0 
    bool ApplyURL(bool bOn, select_pair_ptr pSel = NULL);
    bool IsURL(style_info_ptr info);
    bool IsURL(select_pair_ptr sel);
#endif        

    bool ApplyBadMoodWord(bool bOn, select_pair_ptr pSel = NULL,int nWordType = 0);
    bool IsBadMoodWord(style_info_ptr info);
    bool IsBadMoodWord(select_pair_ptr sel);

    void ClearMisspellingAndRecheck(select_pair_ptr sel = NULL);
    bool IsBullet(select_pair_ptr sel);
    long GetBulletLevel(select_pair_ptr sel);
    bool ApplyBullet(bool bOn);
        
    bool ApplySigned(par_info_ptr info, bool bOn);
    bool ApplySigned(bool bOn);
    bool IsSigned(par_info_ptr par);

	bool ApplySignature( bool bOn = true, select_pair_ptr sel = 0 );
	static bool ApplySignature( par_info_ptr info, bool bOn = true );
	static bool IsSignature( par_info_ptr pPar );
	bool IsSignature( select_pair_ptr sel );

	bool IsWithinURLRange( select_pair_ptr within = NULL );
    bool SetNeedsScanned( bool bOn, select_pair_ptr pSel );
    static bool CPaigeStyle::NeedsScanned( style_info_ptr pStyle )
					{
						if(pStyle->user_id & PAIGE_STYLE_USER_ID_NEEDSSCANNED)
							return true;
						else
							return false;
					}

    bool NeedsScanned( select_pair_ptr sel );
};	

#endif

