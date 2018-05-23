#ifndef PAIGESTYLE_H
#define PAIGESTYLE_H

// These should be used as mask vaules for the  style_info->user_id field
// in a style_info struct.
//
#define PAIGE_STYLE_USER_ID_SPELL            1
#define PAIGE_STYLE_USER_ID_NEEDSSPELLCHECK  2

//
// These should be used as mask vaules for the  par_info->user_id field
// in a style_info struct.
//
#define PAIGE_FORMAT_USER_ID_EXCERPT    1
#define PAIGE_FORMAT_USER_ID_SIGNED     2


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
    bool ApplyExcerpt(bool bOn);
    bool IsExcerpt(select_pair_ptr sel);
        
    bool ApplyMisspelled(bool bOn, select_pair_ptr pSel = NULL);
    bool IsMisspelled(style_info_ptr info);
    bool IsMisspelled(select_pair_ptr sel);
    void ClearMisspellingAndRecheck(select_pair_ptr sel = NULL);
        
    bool IsBullet(select_pair_ptr sel);
    bool ApplyBullet(bool bOn);
        
    bool ApplySigned(par_info_ptr info, bool bOn);
    bool ApplySigned(bool bOn);
    bool IsSigned(par_info_ptr par);
        

    bool SetNeedsSpellCheck( bool bOn, select_pair_ptr pSel );
    static bool CPaigeStyle::NeedsSpellCheck( style_info_ptr pStyle )
        {return (pStyle->user_id == PAIGE_STYLE_USER_ID_NEEDSSPELLCHECK);}

    bool NeedsSpellCheck( select_pair_ptr sel );
};	

#endif

