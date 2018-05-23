// FONT.H
//
// Routines for dealing with fonts, especially the user-defined Screen 
//
#ifndef _FONT_H_
#define _FONT_H_

// Data Types
typedef enum
{
	DONT_CARE = 0,
	FIXED_ONLY = 1,
	PROPORTIONAL_ONLY = 2
} FONT_TYPE_ENUM;


class CFontInfo : public CFont
{
public:
	CFontInfo(int Weight = FW_NORMAL, BOOL Italic = FALSE);

// Attributes
public:
	TEXTMETRIC m_tm;
	int m_Weight;
	BOOL m_Italic;

	int CellHeight() const
		{ return (m_tm.tmHeight + m_tm.tmExternalLeading); }
	int CharHeight() const
		{ return (m_tm.tmHeight); }
	int CharWidth() const
		{ return (m_tm.tmAveCharWidth); }

// Operations
public:
	BOOL SetTextMetrics();
	int TextWidth(LPCTSTR Text);
	BOOL CreateFont(LPCTSTR Name, int Points, CDC* pDC = NULL);
	BOOL CreateFontFromMapping(LPCTSTR Name, int nUnMappedSize, CDC* pDC = NULL);
};


extern CFontInfo ScreenFont;
extern CFontInfo ScreenFontBold;
extern CFontInfo ScreenFontItalic;
extern CFontInfo PrinterFont;
extern CFontInfo PrinterFontBold;

CFontInfo& GetMessageFont();
CFontInfo& GetMessageProportionalFont();
CFontInfo& GetMessageFixedFont();


BOOL GetTextMetrics(HFONT hFont, TEXTMETRIC& tm);
inline BOOL GetTextMetrics(CFont* cFont, TEXTMETRIC& tm)
	{ ASSERT_VALID(cFont); return (GetTextMetrics((HFONT)cFont->m_hObject, tm)); }
int TextWidth(CFont* Font, LPCTSTR Text);
BOOL SetupFonts();

void EnumFontFaces( CStringArray& theArray, FONT_TYPE_ENUM enType = DONT_CARE );
void UpdateFontSettings();

#define FONTSIZE_COUNT 5
extern int	theFontSizeMap[];

#endif	// _FONT_H_
