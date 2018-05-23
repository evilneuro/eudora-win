#include "stdafx.h"
#include "eudora.h"
#include "rs.h"
#include "Text2Html.h"
#include "guiutils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static CString	szEudoraTag;
static CString	szAUTOURL;
static CString	szAMP;
static CString	szNBSP;
static CString	szQUOT;
static CString	szBR;

static CString	szAacuteCAP;
static CString	szAgraveCAP;
static CString	szAcircCAP;
static CString	szAtildeCAP;
static CString	szAringCAP;
static CString	szAumlCAP;
static CString	szAEligCAP;
static CString	szCcedilCAP;
static CString	szEacuteCAP;
static CString	szEgraveCAP;
static CString	szEcircCAP;
static CString	szEumlCAP;
static CString	szIacuteCAP;
static CString	szIgraveCAP;
static CString	szIcircCAP;
static CString	szIumlCAP;
static CString	szETHCAP;
static CString	szNtildeCAP;
static CString	szOacuteCAP;
static CString	szOgraveCAP;
static CString	szOcircCAP;
static CString	szOtildeCAP;
static CString	szOumlCAP;
static CString	szOslashCAP;
static CString	szUacuteCAP;
static CString	szUgraveCAP;
static CString	szUcircCAP;
static CString	szUumlCAP;
static CString	szYacuteCAP;
static CString	szThornCAP;

static CString	szligLOWER;
static CString	szAacuteLOWER;
static CString	szAgraveLOWER;
static CString	szAcircLOWER;
static CString	szAtildeLOWER;
static CString	szAringLOWER;
static CString	szAumlLOWER;
static CString	szAEligLOWER;
static CString	szCcedilLOWER;
static CString	szEacuteLOWER;
static CString	szEgraveLOWER;
static CString	szEcircLOWER;
static CString	szEumlLOWER;
static CString	szIacuteLOWER;
static CString	szIgraveLOWER;
static CString	szIcircLOWER;
static CString	szIumlLOWER;
static CString	szETHLOWER;
static CString	szNtildeLOWER;
static CString	szOacuteLOWER;
static CString	szOgraveLOWER;
static CString	szOcircLOWER;
static CString	szOtildeLOWER;
static CString	szOumlLOWER;
static CString	szOslashLOWER;
static CString	szUacuteLOWER;
static CString	szUgraveLOWER;
static CString	szUcircLOWER;
static CString	szUumlLOWER;
static CString	szYacuteLOWER;
static CString	szThornLOWER;
static CString	szYumlLOWER;

static CString	szCopy;
static CString	szReg;
static CString	szTrade;
static CString	szShy;
static CString	szCBSP;

static CString	szA;
static CString	szOffA;
static CString	szDIV;
static CString	szOffDIV;
static CString	szHref;
static CString	szLT;
static CString	szGT;

static CStringArray	szURLSchemes;
static CString		szSchemeStart;
static BOOL HTMLStringsWasInited = FALSE;
	

void InitHtmlStrings()
{
	CString		szTemp;
	INT			iEnd;
	INT			iSize;
	INT			i;
	CString		szNew;

	if (HTMLStringsWasInited)
		return;

	HTMLStringsWasInited = TRUE;

	szEudoraTag.LoadString(IDS_EUDORA_TAG);
	szAUTOURL.LoadString(IDS_AUTOURL);

	szAacuteCAP.LoadString(IDS_HTML_AACUTE_CAP);
	szAgraveCAP.LoadString(IDS_HTML_AGRAVE_CAP);
	szAcircCAP.LoadString(IDS_HTML_ACIRC_CAP);
	szAtildeCAP.LoadString(IDS_HTML_ATILDE_CAP);
	szAringCAP.LoadString(IDS_HTML_ARING_CAP);
	szAumlCAP.LoadString(IDS_HTML_AUML_CAP);
	szAEligCAP.LoadString(IDS_HTML_AELIG_CAP);
	szCcedilCAP.LoadString(IDS_HTML_CCEDIL_CAP);
	szEacuteCAP.LoadString(IDS_HTML_EACUTE_CAP);
	szEgraveCAP.LoadString(IDS_HTML_EGRAVE_CAP);
	szEcircCAP.LoadString(IDS_HTML_ECIRC_CAP);
	szEumlCAP.LoadString(IDS_HTML_EUML_CAP);
	szIacuteCAP.LoadString(IDS_HTML_IACUTE_CAP);
	szIgraveCAP.LoadString(IDS_HTML_IGRAVE_CAP);
	szIcircCAP.LoadString(IDS_HTML_ICIRC_CAP);
	szIumlCAP.LoadString(IDS_HTML_IUML_CAP);
	szETHCAP.LoadString(IDS_HTML_ETH_CAP);
	szNtildeCAP.LoadString(IDS_HTML_NTILDE_CAP);
	szOacuteCAP.LoadString(IDS_HTML_OACUTE_CAP);
	szOgraveCAP.LoadString(IDS_HTML_OGRAVE_CAP);
	szOcircCAP.LoadString(IDS_HTML_OCIRC_CAP);
	szOtildeCAP.LoadString(IDS_HTML_OTILDE_CAP);
	szOumlCAP.LoadString(IDS_HTML_OUML_CAP);
	szOslashCAP.LoadString(IDS_HTML_OSLASH_CAP);
	szUacuteCAP.LoadString(IDS_HTML_UACUTE_CAP);
	szUgraveCAP.LoadString(IDS_HTML_UGRAVE_CAP);
	szUcircCAP.LoadString(IDS_HTML_UCIRC_CAP);
	szUumlCAP.LoadString(IDS_HTML_UUML_CAP);
	szYacuteCAP.LoadString(IDS_HTML_YACUTE_CAP);
	szThornCAP.LoadString(IDS_HTML_THORN_CAP);
	
	szligLOWER.LoadString(IDS_HTML_SZLIG_LOWER);
	szAacuteLOWER.LoadString(IDS_HTML_AACUTE_LOWER);
	szAgraveLOWER.LoadString(IDS_HTML_AGRAVE_LOWER);
	szAcircLOWER.LoadString(IDS_HTML_ACIRC_LOWER);
	szAtildeLOWER.LoadString(IDS_HTML_ATILDE_LOWER);
	szAringLOWER.LoadString(IDS_HTML_ARING_LOWER);
	szAumlLOWER.LoadString(IDS_HTML_AUML_LOWER);
	szAEligLOWER.LoadString(IDS_HTML_AELIG_LOWER);
	szCcedilLOWER.LoadString(IDS_HTML_CCEDIL_LOWER);
	szEacuteLOWER.LoadString(IDS_HTML_EACUTE_LOWER);
	szEgraveLOWER.LoadString(IDS_HTML_EGRAVE_LOWER);
	szEcircLOWER.LoadString(IDS_HTML_ECIRC_LOWER);
	szEumlLOWER.LoadString(IDS_HTML_EUML_LOWER);
	szIacuteLOWER.LoadString(IDS_HTML_IACUTE_LOWER);
	szIgraveLOWER.LoadString(IDS_HTML_IGRAVE_LOWER);
	szIcircLOWER.LoadString(IDS_HTML_ICIRC_LOWER);
	szIumlLOWER.LoadString(IDS_HTML_IUML_LOWER);
	szETHLOWER.LoadString(IDS_HTML_ETH_LOWER);
	szNtildeLOWER.LoadString(IDS_HTML_NTILDE_LOWER);
	szOacuteLOWER.LoadString(IDS_HTML_OACUTE_LOWER);
	szOgraveLOWER.LoadString(IDS_HTML_OGRAVE_LOWER);
	szOcircLOWER.LoadString(IDS_HTML_OCIRC_LOWER);
	szOtildeLOWER.LoadString(IDS_HTML_OTILDE_LOWER);
	szOumlLOWER.LoadString(IDS_HTML_OUML_LOWER);
	szOslashLOWER.LoadString(IDS_HTML_OSLASH_LOWER);
	szUacuteLOWER.LoadString(IDS_HTML_UACUTE_LOWER);
	szUgraveLOWER.LoadString(IDS_HTML_UGRAVE_LOWER);
	szUcircLOWER.LoadString(IDS_HTML_UCIRC_LOWER);
	szUumlLOWER.LoadString(IDS_HTML_UUML_LOWER);
	szYacuteLOWER.LoadString(IDS_HTML_YACUTE_LOWER);
	szThornLOWER.LoadString(IDS_HTML_THORN_LOWER);
	szYumlLOWER.LoadString(IDS_HTML_YUML_LOWER);

	szCopy.LoadString(IDS_HTML_COPY);
	szReg.LoadString(IDS_HTML_REG);
	szTrade.LoadString(IDS_HTML_TRADE);
	szShy.LoadString(IDS_HTML_SHY);
	szCBSP.LoadString(IDS_HTML_CBSP);

	szLT.LoadString(IDS_HTML_LESSTHAN);
	szGT.LoadString(IDS_HTML_GREATERTHAN);
	szAMP.LoadString(IDS_HTML_AMPERSAND);
	szNBSP.LoadString(IDS_HTML_SPACE);
	szQUOT.LoadString(IDS_HTML_QUOTE);

	szA.LoadString(IDS_HTML_ANCHOR);
	szOffA = "</" + szA + '>';
	// Leave off closing angle bracket since we're going to add a HREF later
	szA = '<' + szA;

	szDIV.LoadString(IDS_HTML_DIV);
	szOffDIV = "</" + szDIV +'>';
	szDIV = '<' + szDIV + '>';

	szHref.LoadString(IDS_HTML_HREF);
	szBR = '<' + CRString( IDS_HTML_BREAK ) + '>';

	iSize = 0;
	szTemp.LoadString(IDS_URL_SCHEMES);
	szTemp.MakeLower();

	while( szTemp != "" )
	{
		iEnd = szTemp.Find( ':' ); 
		
		
		if( iEnd < 0 ) 
		{
			szNew = szTemp;
			szTemp = "";
		}
		else
		{
			szNew = szTemp.Left( iEnd + 1 );
			szTemp = szTemp.Right( szTemp.GetLength() - iEnd - 1 );
		}

		for( i = 0; i < iSize; i++ )
		{
			if( szURLSchemes[ i ] > szNew )
			{
				break;
			}
		}

		if ((szNew != "mailto:") && (szNew != "news:") && (szNew != "file:") && (szNew != "x-eudora-option:"))
			szNew += "//";

		szURLSchemes.InsertAt( i, szNew );

		iSize ++;

		if( szSchemeStart.Find( szNew[0] ) < 0 ) 
		{
			szSchemeStart += szNew[0];
		}
	}

	szTemp = szSchemeStart;
	szTemp.MakeUpper();
	szSchemeStart += szTemp;
}


BOOL IsURLScheme(const char* Text)
{
	INT	i;
	INT	iVal;

	if (!strchr(szSchemeStart, *Text)) 
		return FALSE;

//	int len = strlen(Text);
	int schemeLen = 0;

	for( i = szURLSchemes.GetUpperBound(); i >= 0; i-- )
	{
		schemeLen = szURLSchemes[ i ].GetLength();

		if( ( iVal = strnicmp( szURLSchemes[ i ], Text, schemeLen) ) == 0 )
		{
//			if (len > schemeLen)
				return TRUE;
//			else
//				return FALSE;
		}
		
		if( iVal < 0 )
		{
			return FALSE;
		}
	}
	
	return FALSE;
}



CString	Text2Html(LPCTSTR szSource, BOOL bConvertURLs, BOOL bProcessAttachmentAndPlugins, BOOL bConvertQuotesToExcerpt /*=TRUE*/)
{
	if (!szSource)
		return "";

	
	CString		szDest;
	int	uLen;
	int	nSpaces;
	int nTabs;
	int	numTabSpaces = GetIniShort(IDS_INI_TAB_STOP);
	const char* s;
	//	clock_t		t1;
	//	clock_t		t2;
	//	t1 = clock();
	
	for( s = szSource, uLen=0, nSpaces=0,nTabs=0; *s != 0; uLen++, s++ )
	{
		if( *s == ' ' ){	nSpaces ++;	}
		if( *s == '\t'){	nTabs ++;	}
	}

	// Optimization so that only one large memory allocation
	// is done instead of lots of small ones.
	// <x-tab>&nbsp;&nbsp;&nbsp;&nbsp;<\x-tab> == 39
	szDest.GetBuffer( ((uLen-nSpaces-nTabs)*3) + (nSpaces * 6) + (nTabs * (6*numTabSpaces + 15)) );

	if(!HTMLStringsWasInited)
		InitHtmlStrings();

	const char*	pLine;
	const char*	pChar;
	const char*	pEnd;
	const char*	pUrlEnd;
	CString		szUrl;
	CString		szHrefVal;
	BOOL		bDivTag;
	char		cLast;
	INT			i;
	BOOL		bUrlEndFound;
	UINT		urlEndPos;
	char*		tempStr;
	BOOL		bUrlCloseDivTag = FALSE;
	CString		temp;
	const char*	urlTemp;
	char*		urlNextCR;
	ULONG		charsSinceCR;
	CString		ExcerptOn;
	CString		ExcerptOff;
	int			OldQuoteLevel = 0;

	if (bConvertQuotesToExcerpt)
	{
		ExcerptOn.Format("<%s>", (LPCTSTR)CRString(IDS_HTML_BLOCKQUOTE_CITE));
		ExcerptOff.Format("</%s>", (LPCTSTR)CRString(IDS_HTML_BLOCKQUOTE));
	}
	
	pLine = szSource;

	while( *pLine )
	{
		if (bConvertQuotesToExcerpt)
		{
			const char* QuoteEnd = pLine;

			while (*QuoteEnd == '>')
				QuoteEnd++;
			int NewQuoteLevel = QuoteEnd - pLine;

			// Lines that begin with one or more quotes that are immediately followed
			// by "From " are mangled From lines, so remove one level of quotes.
			if (NewQuoteLevel && strncmp(QuoteEnd, "From ", 5) == 0)
				NewQuoteLevel--;

			if (*QuoteEnd == ' ')
				QuoteEnd++;
			pLine = QuoteEnd;

			if (NewQuoteLevel > OldQuoteLevel)
			{
				for (int i = OldQuoteLevel; i < NewQuoteLevel; i++)
					szDest += ExcerptOn;
			}
			else if (NewQuoteLevel < OldQuoteLevel)
			{
				for (int i = OldQuoteLevel; i > NewQuoteLevel; i--)
					szDest += ExcerptOff;
			}

			OldQuoteLevel = NewQuoteLevel;
		}

		pEnd = strchr( pLine, '\r' );

		bDivTag = FALSE;

		if( pEnd == NULL )
		{
			pEnd = pLine + strlen( pLine );
		}
		else if( pEnd > pLine )
		{
			bDivTag = TRUE;
			szDest += szDIV;
		}

		//If the line starts with "From ", add a space at the beginning
		if (strncmp(pLine, "From ", 5) == 0)
		{
			szDest += ' ';
		}

		pChar = pLine;
		
		cLast = 0;
		charsSinceCR = 0;

		while( pChar < pEnd )
		{
			char ch = *pChar;
			
			if( ch == ' ' )
			{
				//Space at the beginning of the line must be converted else
				//the display engines will ignore them.
				if (charsSinceCR == 0)
					szDest += "&nbsp;";
				else
				{
					if (cLast == ' ' && charsSinceCR != 1 )
					{
						// convert the last space to an & -- then add nbsp;
						szDest.SetAt( szDest.GetLength() - 1, '&' );
						szDest += "nbsp;";
					}
					szDest += ' ';
				}
			}
			else if( ch == '\t' )
			{
				// insert the header
				szDest += "<x-tab>";

				// find the number of nbsp's
				//nSpaces = 4 - ( ( pChar - pLine ) % 4 );
				nSpaces = numTabSpaces - ( charsSinceCR % numTabSpaces );
				charsSinceCR += nSpaces - 1;

				while( nSpaces-- )
				{
					szDest += szNBSP;
				}

				szDest += "</x-tab>";
			}
			else if( ch == '>' )
			{
				szDest += szGT;	
			}
			else if( ch == '<' )
			{
				szDest += szLT;
			}
			else if( ch == '&' )
			{
				szDest += szAMP;
			}
			else if( ch == '"' )
			{
				szDest += szQUOT;
			}
			else if( ch >= 0xC0 && ch <= 0xFF) //optimization for 8 bit characters <192-255>.  SK
			{
				if( ch == 'À' )			szDest += szAgraveCAP;	// 0xC0
				else if( ch == 'Á' )	szDest += szAacuteCAP;	// 0xC1
				else if( ch == 'Â' )	szDest += szAcircCAP;	// 0xC2
				else if( ch == 'Ã' )	szDest += szAtildeCAP;	// 0xC3
				else if( ch == 'Ä' )	szDest += szAumlCAP;	// 0xC4
				else if( ch == 'Å' )	szDest += szAringCAP;	// 0xC5
				else if( ch == 'Æ' )	szDest += szAEligCAP;	// 0xC6
				else if( ch == 'Ç' )	szDest += szCcedilCAP;	// 0xC7
				else if( ch == 'È' )	szDest += szEgraveCAP;	// 0xC8
				else if( ch == 'É' )	szDest += szEacuteCAP;	// 0xC9
				else if( ch == 'Ê' )	szDest += szEcircCAP;	// 0xCA
				else if( ch == 'Ë' )	szDest += szEumlCAP;	// 0xCB
				else if( ch == 'Ì' )	szDest += szIgraveCAP;	// 0xCC
				else if( ch == 'Í' )	szDest += szIacuteCAP;	// 0xCD
				else if( ch == 'Î' )	szDest += szIcircCAP;	// 0xCE
				else if( ch == 'Ï' )	szDest += szIumlCAP;	// 0xCF
				else if( ch == 'Ð' )	szDest += szETHCAP;		// 0xD0
				else if( ch == 'Ñ' )	szDest += szNtildeCAP;	// 0xD1
				else if( ch == 'Ò' )	szDest += szOgraveCAP;	// 0xD2
				else if( ch == 'Ó' )	szDest += szOacuteCAP;	// 0xD3
				else if( ch == 'Ô' )	szDest += szOcircCAP;	// 0xD4
				else if( ch == 'Õ' )	szDest += szOtildeCAP;	// 0xD5
				else if( ch == 'Ö' )	szDest += szOumlCAP;	// 0xD6
//				else if( ch == '×' )							// 0xD7
				else if( ch == 'Ø' )	szDest += szOslashCAP;	// 0xD8
				else if( ch == 'Ù' )	szDest += szUgraveCAP;	// 0xD9
				else if( ch == 'Ú' )	szDest += szUacuteCAP;	// 0xDA
				else if( ch == 'Û' )	szDest += szUcircCAP;	// 0xDB
				else if( ch == 'Ü' )	szDest += szUumlCAP;	// 0xDC
				else if( ch == 'Ý' )	szDest += szYacuteCAP;	// 0xDD
				else if( ch == 'Þ' )	szDest += szThornCAP;	// 0xDE
				else if( ch == 'ß' )	szDest += szligLOWER;	// 0xDF

				else if( ch == 'à' )	szDest += szAgraveLOWER;// 0xE0
				else if( ch == 'á' )	szDest += szAacuteLOWER;// 0xE1
				else if( ch == 'â' )	szDest += szAcircLOWER;	// 0xE2
				else if( ch == 'ã' )	szDest += szAtildeLOWER;// 0xE3
				else if( ch == 'ä' )	szDest += szAumlLOWER;	// 0xE4
				else if( ch == 'å' )	szDest += szAringLOWER;	// 0xE5
				else if( ch == 'æ' )	szDest += szAEligLOWER;	// 0xE6
				else if( ch == 'ç' )	szDest += szCcedilLOWER;// 0xE7
				else if( ch == 'è' )	szDest += szEgraveLOWER;// 0xE8
				else if( ch == 'é' )	szDest += szEacuteLOWER;// 0xE9
				else if( ch == 'ê' )	szDest += szEcircLOWER;	// 0xEA
				else if( ch == 'ë' )	szDest += szEumlLOWER;	// 0xEB
				else if( ch == 'ì' )	szDest += szIgraveLOWER;// 0xEC
				else if( ch == 'í' )	szDest += szIacuteLOWER;// 0xED
				else if( ch == 'î' )	szDest += szIcircLOWER;	// 0xEE
				else if( ch == 'ï' )	szDest += szIumlLOWER;	// 0xEF
				else if( ch == 'ð' )	szDest += szETHLOWER;	// 0xF0
				else if( ch == 'ñ' )	szDest += szNtildeLOWER;// 0xF1
				else if( ch == 'ò' )	szDest += szOgraveLOWER;// 0xF2
				else if( ch == 'ó' )	szDest += szOacuteLOWER;// 0xF3
				else if( ch == 'ô' )	szDest += szOcircLOWER;	// 0xF4
				else if( ch == 'õ' )	szDest += szOtildeLOWER;// 0xF5
				else if( ch == 'ö' )	szDest += szOumlLOWER;	// 0xF6
//				else if( ch == '÷' )							// 0xF7
				else if( ch == 'ø' )	szDest += szOslashLOWER;// 0xF8
				else if( ch == 'ù' )	szDest += szUgraveLOWER;// 0xF9
				else if( ch == 'ú' )	szDest += szUacuteLOWER;// 0xFA
				else if( ch == 'û' )	szDest += szUcircLOWER;	// 0xFB
				else if( ch == 'ü' )	szDest += szUumlLOWER;	// 0xFC
				else if( ch == 'ý' )	szDest += szYacuteLOWER;// 0xFD
				else if( ch == 'þ' )	szDest += szThornLOWER;	// 0xFE
				else if( ch == 'ÿ' )	szDest += szYumlLOWER;	// 0xFF
				else
				{
					// In case we forgot anything, and to
					// handle the '×' and '÷' characters.
					szDest += ch;
				}
			}
			else if( ch == '©' )
			{
				szDest += szCopy;
			}							
			else if( ch == '®' )
			{
				szDest += szReg;
			}							
			else if( ch == '™' )
			{
				szDest += szTrade;
			}							
			else if( ch == '–' )
			{
				szDest += szShy;
			}	
			else if( bConvertURLs &&
					( strnicmp(pChar, "www.", 4) == 0 ||
					  strnicmp(pChar, "ftp.", 4) == 0 ||
					  IsURLScheme(pChar) ))
			{
				bUrlEndFound = FALSE;

				// first, see if it's in quotes
				if( cLast == '"' )
				{
					pUrlEnd = strchr( pChar, '"' );

					if( pUrlEnd != NULL )
					{
						//pUrlEnd = pEnd;
						bUrlEndFound = TRUE;
					}
				}
				else if ( cLast == '<' )
				{
					i = 1;

					pUrlEnd = pChar;
					while ((i != 0) && (*pUrlEnd != NULL))
					{
						urlEndPos = strcspn( pUrlEnd, "<>");

						pUrlEnd += urlEndPos;

						if (*pUrlEnd)
						{
							if (*pUrlEnd == '<')
								i++;
							else if (*pUrlEnd == '>')
								i--;

							pUrlEnd += 1;

						}
					}

					if (i == 0)
					{
						bUrlEndFound = TRUE;
						pUrlEnd -= 1;
					}

				}
				//else
				if (! bUrlEndFound )
				{
					pUrlEnd = pChar + 1;
					
					i = 0;

					while(	( pUrlEnd < pEnd ) && 
							( *pUrlEnd != ' ' ) &&
							( *pUrlEnd != '\t' ) &&
							( *pUrlEnd != '(' ) &&
							( *pUrlEnd != '"' ) &&
							( *pUrlEnd != '\'' ) &&
							( *pUrlEnd != ')' ) &&
							( *pUrlEnd != '\n' ) &&
							( i >= 0 ) )
					{
						if( *pUrlEnd == '<' ) // ignore embedded '<'
						{
							++i;
						}
						else if( *pUrlEnd == '>' )
						{
							--i;
						}
						pUrlEnd++;
					}

					if( i < 0 )
					{ 
						pUrlEnd --;
					}

					// The following if statement is to strip a period at the end of the URL if a space is the terminator
					// of the URL.
					while ((( pUrlEnd[-1] == '.' ) || 
							( pUrlEnd[-1] == ',' ) || 
							( pUrlEnd[-1] == ';' ) || 
							( pUrlEnd[-1] == '!' ) || 
							( pUrlEnd[-1] == '[' ) || 
							( pUrlEnd[-1] == ']' ) || 
							( pUrlEnd[-1] == '?' ))
							&&
							(pUrlEnd > pChar)) 
					{
						pUrlEnd--;
					}

				}

				szHrefVal = "";
				
				if( strnicmp(pChar, "www.", 4) == 0 ) 
				{
					szHrefVal = "http://";
				}
				else if( strnicmp(pChar, "ftp.", 4) == 0)
				{
					szHrefVal = "ftp://";
				}

				if ( (strnicmp(pChar, "file:", 5) == 0) && (strnicmp(pChar, "file://", 7) != 0) )
				{
				//Allow file: besides file://
				//But the URL validation scheme looks for "//" also, so add those in the href value.
				szUrl = CString( pChar, pUrlEnd - pChar );

				CString temp = CString(pChar, 5);
				szHrefVal += temp;
				szHrefVal += "//";
				temp = CString( pChar + 5, pUrlEnd - (pChar+5) );
				szHrefVal += temp;
				}
				else
				{
				szUrl = CString( pChar, pUrlEnd - pChar );
				szHrefVal += szUrl;
				}

				szHrefVal = EscapeURL( szHrefVal );

				if( !URLIsValid( szHrefVal ) )
				{
					// just add the character as usual
					szDest += ch;
				}
				else
				{
					szDest += szA + ' ' + szHref + "=\"";				

					//FRAGILE = Kusuma
					//Text2Html always adds a CR-LF pair at the end, so need to remove those
					//If the parsing code changes in the future, this need to be checked
					szHrefVal = Text2Html(szHrefVal, FALSE, FALSE);
					if (strncmp(szHrefVal.Right(2),"\r\n", 2) == 0)
					{
						tempStr = szHrefVal.LockBuffer();
						tempStr[strlen(tempStr)-2] = '\0';
						szHrefVal.ReleaseBuffer();
					}
					

					//Html-ize the szUrl text. We shouldn't have div tags for the first line
					//and we should not close the div tag for the last line until 
					//the whole line has been read
					bUrlCloseDivTag = FALSE;

					if ((*pEnd != NULL) && (pUrlEnd > pEnd))
					{
						//We have atleast 2 lines. Html-ize the szHrefval
						temp = CString(pChar, pEnd - pChar);
						urlTemp = pEnd + ((pEnd[1] == '\n') ? 2 : 1);
						szUrl = Text2Html(temp, FALSE, FALSE);
						if (strncmp(szUrl.Right(2),"\r\n", 2) == 0)
						{
							tempStr = szUrl.LockBuffer();
							tempStr[strlen(tempStr)-2] = '\0';
							szUrl.ReleaseBuffer();
						} 

						while (urlTemp < pUrlEnd)
						{
							if (bUrlCloseDivTag)
								szUrl += szOffDIV;
							else
								bUrlCloseDivTag = TRUE;

							urlNextCR = strchr(urlTemp, '\r');
							if ((urlNextCR != NULL) && (urlNextCR < pUrlEnd))
							{
								temp = CString(urlTemp, urlNextCR - urlTemp);
								urlTemp = urlNextCR + (urlNextCR[1] == '\n' ? 2 : 1);
							}
							else
							{
								temp = CString(urlTemp, pUrlEnd - urlTemp);
								urlTemp = pUrlEnd;
							}
							
							temp = Text2Html(temp, FALSE, FALSE);
							if (strncmp(temp.Right(2),"\r\n", 2) == 0)
							{
								tempStr = temp.LockBuffer();
								tempStr[strlen(tempStr)-2] = '\0';
								temp.ReleaseBuffer();
							}

							szUrl += szDIV + temp;
						}

						bUrlCloseDivTag = TRUE;

					}
					else
					{					
						szUrl = Text2Html(szUrl, FALSE, FALSE);
						if (strncmp(szUrl.Right(2),"\r\n", 2) == 0)
						{
							tempStr = szUrl.LockBuffer();
							tempStr[strlen(tempStr)-2] = '\0';
							szUrl.ReleaseBuffer();
						} 
					}
					
					szDest += szHrefVal + "\" " + szEudoraTag + '=' + szAUTOURL + '>' + szUrl + szOffA;

					if ( strchr(pChar, '\r') != strchr(pUrlEnd, '\r') )
					{
						//Url extended over multiple lines, need to adjust some values.
						pEnd = strchr(pUrlEnd, '\r');
						if (pEnd == NULL)
							pEnd = pUrlEnd + strlen(pUrlEnd);
					}

					charsSinceCR += pUrlEnd - pChar - 1;
					pChar = pUrlEnd - 1;
				}
			}
			else
			{
				szDest += ch;
			}		
			cLast = ch;
			pChar++;
			charsSinceCR++;
		}

		if (bUrlCloseDivTag)
		{
			szDest += szOffDIV;
			bUrlCloseDivTag = FALSE;
		}

		if( bDivTag )
		{
			if( szDest.Right( szDIV.GetLength() + 1 ) == szDIV + ' ' )
			{
				szDest.SetAt( szDest.GetLength() - 1, '&' );
				szDest += "nbsp;";
			}

			szDest += szOffDIV;
		}
		else if ( *pEnd == '\r' )
		{
			szDest += szBR;
		}

		szDest += "\r\n";

		if( *pEnd == '\r' )
		{
			pLine = pEnd + (pEnd[1] == '\n'? 2 : 1);
		}
		else
		{
			// we're done
			break;
		}
	}

	if (OldQuoteLevel)
	{
		for (int i = OldQuoteLevel; i; i--)
			szDest += ExcerptOff;
	}

	szDest.FreeExtra();

//	t2 = clock();
//	szUrl.Format( "%ld\n", ( LONG )( t2 - t1 ) );
//	TRACE( szUrl );
		
	return szDest;
}				
