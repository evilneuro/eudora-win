/* HTML definitions table,  used by HTML Import & Export. This file contains all the
tags and tag attribute keywords that we support (in some fashion) for HTML. */

#include "stdafx.h"

#include "CPUDEFS.H"
#include "pghtmdef.h"
#include "machine.h"

// BEGIN QUALCOMM
// CL: I really don't like putting these includes here, but...
// I really need to change the teletype style to reflect Eudora defaults
// and modifying the style in PaigeEditView after it was created caused
// all sorts of problems.
//
#include "paige_io.h"
#include "rs.h"
#include "resource.h"


// Added support for the <div> tag
pg_char PG_FAR control_commands[] = {
	"base basefont body br dir div dl head hr html li listing menu ol p style title ul wbr x-tab x-sigsep \0"
//	"base basefont body br dir dl head hr html li listing menu ol p title ul wbr \0"
};
// END QUALCOMM

pg_char PG_FAR control_attributes[] = {
	"align alink background bgcolor class color href leftmargin link noshade size start text topmargin type vlink width \0"
};


pg_char PG_FAR html_style_commands[] = {
	"address b big blink blockquote center cite code dd dfn dt em font h1 h2 h3 h4 h5 h6 i nobr pre s small strike strong sub sup tt u var \0"
};
// QUALCOMM - added type, style, class, cite
pg_char PG_FAR style_attributes[] = {
	"align color face size style type class cite \0"
};

pg_char PG_FAR table_data_commands[] = {
	"table td th tr \0"
};

pg_char PG_FAR data_commands[] = {
	"a embed img object param \0"
};
pg_char PG_FAR data_attributes[] = {
	"align alt bgcolor border cellpadding cellspacing classid colspan eudora height hspace href id ismap lowsrc name src title valign value valuetype vspace width \0"
};
pg_char PG_FAR data_values[] = {
	"absbottom absmiddle baseline center left middle right top \0"
};

pg_char PG_FAR data_color_values[] = {
	"aliceblue antiquewhite aqua aquamarine azure beige bisque black blanchedalmond blue blueviolet brown burlywood cadetblue chartreuse chocolate coral cornflowerblue cornsilk crimson cyan darkblue darkcyan darkgoldenrod darkgray darkgreen darkkhaki darkmagenta darkolivegreen darkorange darkorchid darkred darksalmon darkseagreen darkslateblue darkslategray darkturquoise darkviolet deeppink deepskyblue dimgray dodgerblue firebrick floralwhite forestgreen fuchsia gainsboro ghostwhite gold goldenrod gray green greenyellow honeydew hotpink indianred indigo ivory khaki lavender lavenderblush lawngreen lemonchiffon lightblue lightcoral lightcyan lightgoldenrodyellow lightgreen lightgrey lightpink lightsalmon lightseagreen lightskyblue lightslategray lightsteelblue lightyellow lime limegreen linen magenta maroon mediumaquamarine mediumblue mediumorchid mediumpurple mediumseagreen mediumslateblue mediumspringgreen mediumturquoise mediumvioletred midnightblue mintcream mistyrose moccasin navajowhite navy oldlace olive olivedrab orange orangered orchid palegoldenrod palegreen paleturquoise palevioletred papayawhip peachpuff peru pink plum powderblue purple red rosybrown royalblue saddlebrown salmon sandybrown seagreen seashell sienna silver skyblue slateblue slategray snow springgreen steelblue tan teal thistle tomato turquoise violet wheat white whitesmoke yellow yellowgreen \0"
};

// Better mapping that takes into account any HTML 4 character entities
// that map to CP1252. Allows handling of euro and Outlook messages
// that use HTML 4 numeric character entities. Mapping adapted from Mac code.
// New arrangement allows us to more easily see how characters line up.
CharacterEntityInfo character_entity_arr[kLengthCharEntityArr] = {
	{ 0x00C6,	"AElig",	(pg_char) 'Æ' },
	{ 0x00C1,	"Aacute",	(pg_char) 'Á' },
	{ 0x00C2,	"Acirc",	(pg_char) 'Â' },
	{ 0x00C0,	"Agrave",	(pg_char) 'À' },
	{ 0x00C5,	"Aring",	(pg_char) 'Å' },
	{ 0x00C3,	"Atilde",	(pg_char) 'Ã' },
	{ 0x00C4,	"Auml",		(pg_char) 'Ä' },
	{ 0x00C7,	"Ccedil",	(pg_char) 'Ç' },
	{ 0x2021,	"Dagger",	(pg_char) '‡' },
	{ 0x00D0,	"ETH",		(pg_char) 'Ð' },
	{ 0x00C9,	"Eacute",	(pg_char) 'É' },
	{ 0x00CA,	"Ecirc",	(pg_char) 'Ê' },
	{ 0x00C8,	"Egrave",	(pg_char) 'È' },
	{ 0x00CB,	"Euml",		(pg_char) 'Ë' },
	{ 0x00CD,	"Iacute",	(pg_char) 'Í' },
	{ 0x00CE,	"Icirc",	(pg_char) 'Î' },
	{ 0x00CC,	"Igrave",	(pg_char) 'Ì' },
	{ 0x00CF,	"Iuml",		(pg_char) 'Ï' },
	{ 0x00D1,	"Ntilde",	(pg_char) 'Ñ' },
	{ 0x0152,	"OElig",	(pg_char) 'Œ' },
	{ 0x00D3,	"Oacute",	(pg_char) 'Ó' },
	{ 0x00D4,	"Ocirc",	(pg_char) 'Ô' },
	{ 0x00D2,	"Ograve",	(pg_char) 'Ó' },
	{ 0x00D8,	"Oslash",	(pg_char) 'Ø' },
	{ 0x00D5,	"Otilde",	(pg_char) 'Õ' },
	{ 0x00D6,	"Ouml",		(pg_char) 'Ö' },
	{ 0x0160,	"Scaron",	(pg_char) 'Š' },
	{ 0x00DE,	"THORN",	(pg_char) 'Þ' },
	{ 0x00DA,	"Uacute",	(pg_char) 'Ú' },
	{ 0x00DB,	"Ucirc",	(pg_char) 'Û' },
	{ 0x00D9,	"Ugrave",	(pg_char) 'Ù' },
	{ 0x00DC,	"Uuml",		(pg_char) 'Ü' },
	{ 0x00DD,	"Yacute",	(pg_char) 'Ý' },
	{ 0x0178,	"Yuml",		(pg_char) 'Ÿ' },
	{ 0x00E1,	"aacute",	(pg_char) 'á' },
	{ 0x00E2,	"acirc",	(pg_char) 'â' },
	{ 0x00B4,	"acute",	(pg_char) '´' },
	{ 0x00E6,	"aelig",	(pg_char) 'æ' },
	{ 0x00E0,	"agrave",	(pg_char) 'à' },
	{ 0x0026,	"amp",		(pg_char) '&' },
	{ 0x00E5,	"aring",	(pg_char) 'å' },
	{ 0x00E3,	"atilde",	(pg_char) 'ã' },
	{ 0x00E4,	"auml",		(pg_char) 'ä' },
	{ 0x201E,	"bdquo",	(pg_char) '„' },
	{ 0x00A6,	"brvbar",	(pg_char) '¦' },
	{ 0x2022,	"bull",		(pg_char) '•' },
	{ 0x00E7,	"ccedil",	(pg_char) 'ç' },
	{ 0x00B8,	"cedil",	(pg_char) '¸' },
	{ 0x00A2,	"cent",		(pg_char) '¢' },
	{ 0x02C6,	"circ",		(pg_char) 'ˆ' },
	{ 0x00A9,	"copy",		(pg_char) '©' },
	{ 0x00A4,	"curren",	(pg_char) '¤' },
	{ 0x2020,	"dagger",	(pg_char) '†' },
	{ 0x00B0,	"deg",		(pg_char) '°' },
	{ 0x00F7,	"divide",	(pg_char) '÷' },
	{ 0x00E9,	"eacute",	(pg_char) 'é' },
	{ 0x00EA,	"ecirc",	(pg_char) 'ê' },
	{ 0x00E8,	"egrave",	(pg_char) 'è' },
	{ 0x00F0,	"eth",		(pg_char) 'ð' },
	{ 0x00EB,	"euml",		(pg_char) 'ë' },
	{ 0x20AC,	"euro",		(pg_char) '€' },
	{ 0x0192,	"fnof",		(pg_char) 'ƒ' },
	{ 0x00BD,	"frac12",	(pg_char) '½' },
	{ 0x00BC,	"frac14",	(pg_char) '¼' },
	{ 0x00BE,	"frac34",	(pg_char) '¾' },
	{ 0x003E,	"gt",		(pg_char) '>' },
	{ 0x2026,	"hellip",	(pg_char) '…' },
	{ 0x00ED,	"iacute",	(pg_char) 'í' },
	{ 0x00EE,	"icirc",	(pg_char) 'î' },
	{ 0x00A1,	"iexcl",	(pg_char) '¡' },
	{ 0x00EC,	"igrave",	(pg_char) 'ì' },
	{ 0x00BF,	"iquest",	(pg_char) '¿' },
	{ 0x00EF,	"iuml",		(pg_char) 'ï' },
	{ 0x00AB,	"laquo",	(pg_char) '«' },
	{ 0x201C,	"ldquo",	(pg_char) '“' },
	{ 0x2039,	"lsaquo",	(pg_char) '‹' },
	{ 0x2018,	"lsquo",	(pg_char) '‘' },
	{ 0x003C,	"lt",		(pg_char) '<' },
	{ 0x00AF,	"macr",		(pg_char) '¯' },
	{ 0x2014,	"mdash",	(pg_char) '­' },
	{ 0x00B5,	"micro",	(pg_char) 'µ' },
	{ 0x00B7,	"middot",	(pg_char) '·' },
	{ 0x00A0,	"nbsp",		(pg_char) ' ' },
	{ 0x2013,	"ndash",	(pg_char) '–' },
	{ 0x00AC,	"not",		(pg_char) '¬' },
	{ 0x00F1,	"ntilde",	(pg_char) 'ñ' },
	{ 0x00F3,	"oacute",	(pg_char) 'ó' },
	{ 0x00F4,	"ocirc",	(pg_char) 'ô' },
	{ 0x0153,	"oelig",	(pg_char) 'œ' },
	{ 0x00F2,	"ograve",	(pg_char) 'ò' },
	{ 0x00AA,	"ordf",		(pg_char) 'ª' },
	{ 0x00BA,	"ordm",		(pg_char) 'º' },
	{ 0x00F8,	"oslash",	(pg_char) 'ø' },
	{ 0x00F5,	"otilde",	(pg_char) 'õ' },
	{ 0x00F6,	"ouml",		(pg_char) 'ö' },
	{ 0x00B6,	"para",		(pg_char) '¶' },
	{ 0x00B1,	"plusmn",	(pg_char) '±' },
	{ 0x2030,	"permil",	(pg_char) '‰' },
	{ 0x00A3,	"pound",	(pg_char) '£' },
	{ 0x0022,	"quot",		(pg_char) '"' },
	{ 0x00BB,	"raquo",	(pg_char) '»' },
	{ 0x201D,	"rdquo",	(pg_char) '”' },
	{ 0x00AE,	"reg",		(pg_char) '®' },
	{ 0x203A,	"rsaquo",	(pg_char) '›' },
	{ 0x2019,	"rsquo",	(pg_char) '’' },
	{ 0x201A,	"sbquo",	(pg_char) '‚' },
	{ 0x0161,	"scaron",	(pg_char) 'š' },
	{ 0x00A7,	"sect",		(pg_char) '§' },
	{ 0x00AD,	"shy",		(pg_char) '­' },
	{ 0x00DF,	"szlig",	(pg_char) 'ß' },
	{ 0x00FE,	"thorn",	(pg_char) 'þ' },
	{ 0x02DC,	"tilde",	(pg_char) '˜' },
	{ 0x2122,	"trade",	(pg_char) '™' },
	{ 0x00FA,	"uacute",	(pg_char) 'ú' },
	{ 0x00FB,	"ucirc",	(pg_char) 'û' },
	{ 0x00F9,	"ugrave",	(pg_char) 'ù' },
	{ 0x00A8,	"uml",		(pg_char) '¨' },
	{ 0x00FC,	"uuml",		(pg_char) 'ü' },
	{ 0x00FD,	"yacute",	(pg_char) 'ý' },
	{ 0x00A5,	"yen",		(pg_char) '¥' },
	{ 0x00FF,	"yuml",		(pg_char) 'ÿ' },
	{ 0x2013,	"endash",	(pg_char) '–' },	/* old endash name - comes 2nd so importing works, while exporting is unaffected */
	{ 0x2014,	"emdash",	(pg_char) '­' }  	/* old emdash name - comes 2nd so importing works, while exporting is unaffected */
};


long PG_FAR data_colors[140] = {

	0xF0F8FF, 0xFAEBD7, 0x00FFFF, 0x7FFFD4, 0xF0FFFF, 0xF5F5DC, 0xFFE4C4, 0x000000,
	0xFFEBCD, 0x0000FF, 0x8A2BE2, 0xA52A2A, 0xDEB887, 0x5F9EA0, 0x7FFF00, 0xD2691E,
	0xFF7F50, 0x6495ED, 0xFFF8DC, 0xDC143C, 0x00FFFF, 0x00008B, 0x008B8B, 0xB8860B,
	0xA9A9A9, 0x006400, 0xBDB76B, 0x8B008B, 0x556B2F, 0xFF8C00, 0x9932CC, 0x8B0000,
	0xE9967A, 0x8FBC8F, 0x483D8B, 0x2F4F4F, 0x00CED1, 0x9400D3, 0xFF1493, 0x00BFFF,
	0x696969, 0x1E90FF, 0xB22222, 0xFFFAF0, 0x228B22, 0xFF00FF, 0xDCDCDC, 0xF8F8FF,
	0xFFD700, 0xDAA520, 0x808080, 0x008000, 0xADFF2F, 0xF0FFF0, 0xFF69B4, 0xCD5C5C,
	0x4B0082, 0xFFFFF0, 0xF0E68C, 0xE6E6FA, 0xFFF0F5, 0x7CFC00, 0xFFFACD, 0xADD8E6,
	0xF08080, 0xE0FFFF, 0xFAFAD2, 0x90EE90, 0xD3D3D3, 0xFFB6C1, 0xFFA07A, 0x20B2AA,
	0x87CEFA, 0x778899, 0xB0C4DE, 0xFFFFE0, 0x00FF00, 0x32CD32, 0xFAF0E6, 0xFF00FF,
	0x800000, 0x66CDAA, 0x0000CD, 0xBA55D3, 0x9370DB, 0x3CB371, 0x7B68EE, 0x00FA9A,
	0x48D1CC, 0xC71585, 0x191970, 0xF5FFFA, 0xFFE4E1, 0xFFE4B5, 0xFFDEAD, 0x000080,
	0xFDF5E6, 0x808000, 0x6B8E23, 0xFFA500, 0xFF4500, 0xDA70D6, 0xEEE8AA,
	0x98FB98, 0xAFEEEE, 0xDB7093, 0xFFEFD5, 0xFFDAB9, 0xCD853F, 0xFFC0CB, 0xDDA0DD,
	0xB0E0E6, 0x800080, 0xFF0000, 0xBC8F8F, 0x4169E1, 0x8B4513, 0xFA8072, 0xF4A460,
	0x2E8B57, 0xFFF5EE, 0xA0522D, 0xC0C0C0, 0x87CEEB, 0x6A5ACD, 0x708090, 0xFFFAFA,
	0x00FF7F, 0x4682B4, 0xD2B48C, 0x008080, 0xD8BFD8, 0xFF6347, 0x40E0D0, 0xEE82EE,
	0xF5DEB3, 0xFFFFFF, 0xF5F5F5, 0xFFFF00, 0x9ACD32
};

pg_char PG_FAR errorlog_tags[] = { // tags that are not supported but can affect content
	"applet form frame \0"
};


// Stylesheet names:

pg_char PG_FAR body_style[] = {"HTML Body\0"};
pg_char PG_FAR pre_tag_style[] = {"HTML Pre-tag\0"};
pg_char PG_FAR teletype_style[] = {"HTML Teletype\0"};
pg_char PG_FAR blockquote_style[] = {"HTML Blockquote\0"};
pg_char PG_FAR address_style[] = {"HTML Address\0"};
pg_char PG_FAR cite_style[] = {"HTML Cite\0"};

pg_char PG_FAR heading1_style[] = {"HTML Heading 1\0"};
pg_char PG_FAR heading2_style[] = {"HTML Heading 2\0"};
pg_char PG_FAR heading3_style[] = {"HTML Heading 3\0"};
pg_char PG_FAR heading4_style[] = {"HTML Heading 4\0"};
pg_char PG_FAR heading5_style[] = {"HTML Heading 5\0"};
pg_char PG_FAR heading6_style[] = {"HTML Heading 6\0"};

pg_char PG_FAR code_style[] = {"HTML Code\0"};
pg_char PG_FAR code_def_style[] = {"HTML Code Def\0"};
pg_char PG_FAR code_var_style[] = {"HTML Var\0"};


static long make_style (pg_ref import_pg, pg_char_ptr name, long html_element, long stylebits,
				short pointsize, pg_char_ptr fontname, par_info_ptr par);


/* CreateHTMLStyles creates default named stylesheets for the various HTML tags.
If style_array is non-NULL then it contains an array of NUM_HTML_STYLES. Upon entry,
each value of zero in that array is created, otherwise the stylesheet is NOT created
(the array element is assumed to be an existing named style index). On return
all the elements are filled in. */

PG_PASCAL (void) CreateHTMLStyles (pg_ref pg, pg_char_ptr def_font, long PG_FAR *style_array)
{
	par_info		par_heading;
	long			html_styles[NUM_HTML_STYLES];
	
	pgGetParInfoRec(pg, 0, &par_heading);
	par_heading.indents.left_indent = par_heading.indents.first_indent = 0;

	if (style_array)
		pgBlockMove(style_array, html_styles, sizeof(long) * NUM_HTML_STYLES);
	else
		pgFillBlock(html_styles, sizeof(long) * NUM_HTML_STYLES, 0);

	if (!html_styles[body_stylesheet])
		html_styles[body_stylesheet] = make_style(pg, body_style, body_stylesheet, X_PLAIN_TEXT, 12, def_font, &par_heading);
	
#ifdef WINDOWS_PLATFORM
	if (!html_styles[pre_tag_stylesheet])
		html_styles[pre_tag_stylesheet] = make_style(pg, pre_tag_style, pre_tag_stylesheet, X_PLAIN_TEXT, PRETAG_DEF_POINTSIZE, PRETAG_DEF_FONT, &par_heading);
	if (!html_styles[teletype_stylesheet])
//		html_styles[teletype_stylesheet] = make_style(pg, teletype_style, teletype_stylesheet, X_PLAIN_TEXT, 12, (pg_char_ptr)"Courier New\0", &par_heading);
		html_styles[teletype_stylesheet] = make_style(pg, teletype_style, teletype_stylesheet, X_PLAIN_TEXT, static_cast<short>(pgGetDefaultPointSize()>>16), (pg_char_ptr)GetIniString(IDS_INI_MESSAGE_FIXED_FONT), &par_heading);
	if (!html_styles[code_stylesheet])
		html_styles[code_stylesheet] = make_style(pg, code_style, code_stylesheet, X_PLAIN_TEXT, CODETAG_DEF_POINTSIZE, CODETAG_DEF_FONT, &par_heading);
	
	if (!html_styles[code_def_stylesheet])
		html_styles[code_def_stylesheet] = make_style(pg, code_def_style, code_def_stylesheet, X_ITALIC_BIT, 9, (pg_char_ptr)"Courier New\0", &par_heading);
#else

	if (!html_styles[pre_tag_stylesheet])
		html_styles[pre_tag_stylesheet] = make_style(pg, pre_tag_style, pre_tag_stylesheet, X_PLAIN_TEXT, PRETAG_DEF_POINTSIZE, (pg_char_ptr)"Monaco\0", &par_heading);
	if (!html_styles[teletype_stylesheet])
		html_styles[teletype_stylesheet] = make_style(pg, teletype_style, teletype_stylesheet, X_PLAIN_TEXT, 12, (pg_char_ptr)"Monaco\0", &par_heading);
	if (!html_styles[code_stylesheet])
		html_styles[code_stylesheet] = make_style(pg, code_style, code_stylesheet, X_PLAIN_TEXT, CODETAG_DEF_POINTSIZE, (pg_char_ptr)"Monaco\0", &par_heading);
	if (!html_styles[code_def_stylesheet])
		html_styles[code_def_stylesheet] = make_style(pg, code_def_style, code_def_stylesheet, X_ITALIC_BIT, 9, CODETAG_DEF_FONT, &par_heading);
#endif

	if (!html_styles[code_var_stylesheet])
		html_styles[code_var_stylesheet] = make_style(pg, code_var_style, code_var_stylesheet, X_ITALIC_BIT, 12, def_font, &par_heading);

	if (!html_styles[blockquote_stylesheet])
		html_styles[blockquote_stylesheet] = make_style(pg, blockquote_style, blockquote_stylesheet, X_PLAIN_TEXT, 12, def_font, &par_heading);
	if (!html_styles[address_stylesheet])
		html_styles[address_stylesheet] = make_style(pg, address_style, address_stylesheet, X_ITALIC_BIT, 12, def_font, &par_heading);
	if (!html_styles[cite_stylesheet])
		html_styles[cite_stylesheet] = make_style(pg, cite_style, cite_stylesheet, X_ITALIC_BIT, 12, def_font, &par_heading);

	if (!html_styles[heading1_stylesheet]) {
		
		par_heading.outline_level += 1;
		html_styles[heading1_stylesheet] = make_style(pg, heading1_style, heading1_stylesheet, X_BOLD_BIT, 24, def_font, &par_heading);
	}
	
	if (!html_styles[heading2_stylesheet]) {
	
		par_heading.outline_level += 1;
		html_styles[heading2_stylesheet] = make_style(pg, heading2_style, heading2_stylesheet, X_BOLD_BIT, 18, def_font, &par_heading);
	}
	
	if (!html_styles[heading3_stylesheet]) {

		par_heading.outline_level += 1;
		html_styles[heading3_stylesheet] = make_style(pg, heading3_style, heading3_stylesheet, X_BOLD_BIT, 14, def_font, &par_heading);
	}
	
	if (!html_styles[heading4_stylesheet]) {

		par_heading.outline_level += 1;
		html_styles[heading4_stylesheet] = make_style(pg, heading4_style, heading4_stylesheet, X_BOLD_BIT, 12, def_font, &par_heading);
	}
	
	par_heading.outline_level = 0;

	if (!html_styles[heading5_stylesheet])
		html_styles[heading5_stylesheet] = make_style(pg, heading5_style, heading5_stylesheet, X_BOLD_BIT, 10, def_font, &par_heading);
	
	if (!html_styles[heading6_stylesheet])
		html_styles[heading6_stylesheet] = make_style(pg, heading6_style, heading6_stylesheet, X_BOLD_BIT, 9, def_font, &par_heading);

	if (style_array)
		pgBlockMove(html_styles, style_array, sizeof(long) * NUM_HTML_STYLES);
}


/************************************** Local Functions *********************************/


/* make_style builds a named stylesheet returning the stylesheet index number. (If named style
already exists then that style is returned). */

static long make_style (pg_ref import_pg, pg_char_ptr name, long html_element, long stylebits,
				short pointsize, pg_char_ptr fontname, par_info_ptr par)
{
	long			index;
	
	if ((index = pgGetNamedStyleIndex(import_pg, name)) == 0) {
		paige_rec_ptr		pg_rec;
		pg_globals_ptr		paige_globals;
		style_info			info;
		font_info			font;
		font_info_ptr		use_font;
		
		pg_rec = /*Kusuma*/(paige_rec_ptr)/*Kusuma*/UseMemory(import_pg);
		paige_globals = pg_rec->globals;
		UnuseMemory(import_pg);

		info = paige_globals->def_style;
		info.point = (long)pointsize;
		info.point <<= 16;
		
		QDStyleToPaige(stylebits, &info);
		
		info.html_style = html_element;

		if (fontname) {
			pg_short_t		font_size;
			
			pgFillBlock(&font, sizeof(font_info), 0);
			
			for (font_size = 0; font_size < (FONT_SIZE + 2); ++font_size)
				if ((font.name[font_size + 1] = fontname[font_size]) == 0)
					break;
			
			font.name[0] = (pg_char)font_size;
			use_font = &font;
		}
		else
			use_font = &paige_globals->def_font;

		index = pgNewNamedStyle(import_pg, name, &info, use_font, par);
	}
	
	return		index;
}


