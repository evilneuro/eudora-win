// html2text.cpp : implementation file
//

/////////////////////////////////////////////////////////////////////////////
// CHtml2Text message handlers
//

#include "stdafx.h"
#include "resource.h"
#include <afxcmn.h>
#include <afxpriv.h>			// for MFC-defined WM_SETMESSAGESTRING
#include "rs.h"
#include "rescomm.h"
#include "eudora.h"
#include "html2text.h"


#include "DebugNewHelpers.h"

// Statics
bool		CHtml2Text::s_bStringsLoaded = false;
CString		CHtml2Text::s_szEudoraTag;
CString		CHtml2Text::s_szAutoURL;
CString		CHtml2Text::s_szSignature;

CString		CHtml2Text::s_szA;
CString		CHtml2Text::s_szOffA;
CString		CHtml2Text::s_szALIGN;
CString		CHtml2Text::s_szALT;
CString		CHtml2Text::s_szAMP;
CString		CHtml2Text::s_szB;
CString		CHtml2Text::s_szOffB;
CString		CHtml2Text::s_szBASE;
CString		CHtml2Text::s_szBIG;
CString		CHtml2Text::s_szOffBIG;
CString		CHtml2Text::s_szBLOCKQUOTE;
CString		CHtml2Text::s_szOffBLOCKQUOTE;
CString		CHtml2Text::s_szBQ;
CString		CHtml2Text::s_szOffBQ;
CString		CHtml2Text::s_szBR;
CString		CHtml2Text::s_szCENTER;
CString		CHtml2Text::s_szOffCENTER;
CString		CHtml2Text::s_szCOLOR;
CString		CHtml2Text::s_szDD;	
CString		CHtml2Text::s_szDIR;
CString		CHtml2Text::s_szOffDIR;
CString		CHtml2Text::s_szDIV;
CString		CHtml2Text::s_szOffDIV;
CString		CHtml2Text::s_szDL;
CString		CHtml2Text::s_szOffDL;
CString		CHtml2Text::s_szDT;
CString		CHtml2Text::s_szEM;
CString		CHtml2Text::s_szOffEM;
CString		CHtml2Text::s_szFACE;
CString		CHtml2Text::s_szFONT;
CString		CHtml2Text::s_szOffFONT;
CString		CHtml2Text::s_szGT;
CString		CHtml2Text::s_szH1;
CString		CHtml2Text::s_szOffH1;
CString		CHtml2Text::s_szH2;
CString		CHtml2Text::s_szOffH2;
CString		CHtml2Text::s_szH3;
CString		CHtml2Text::s_szOffH3;
CString		CHtml2Text::s_szH4;
CString		CHtml2Text::s_szOffH4;
CString		CHtml2Text::s_szH5;
CString		CHtml2Text::s_szOffH5;
CString		CHtml2Text::s_szH6;
CString		CHtml2Text::s_szOffH6;
CString		CHtml2Text::s_szHEAD;
CString		CHtml2Text::s_szOffHEAD;
CString		CHtml2Text::s_szHR;
CString		CHtml2Text::s_szHREF;
CString		CHtml2Text::s_szHTML;
CString		CHtml2Text::s_szOffHTML;
CString		CHtml2Text::s_szI;
CString		CHtml2Text::s_szOffI;
CString		CHtml2Text::s_szIMG;
CString		CHtml2Text::s_szLEFT;
CString		CHtml2Text::s_szLI;
CString		CHtml2Text::s_szLink;
CString		CHtml2Text::s_szLT;
CString		CHtml2Text::s_szNAME;
CString		CHtml2Text::s_szNBSP;
CString		CHtml2Text::s_szOL;
CString		CHtml2Text::s_szOffOL;
CString		CHtml2Text::s_szP;
CString		CHtml2Text::s_szOffP;
CString		CHtml2Text::s_szPARA;
CString		CHtml2Text::s_szOffPARA;
CString		CHtml2Text::s_szPRE;
CString		CHtml2Text::s_szOffPRE;
CString		CHtml2Text::s_szQUOT;
CString		CHtml2Text::s_szRIGHT;
CString		CHtml2Text::s_szSIZE;
CString		CHtml2Text::s_szSMALL;
CString		CHtml2Text::s_szOffSMALL;
CString		CHtml2Text::s_szSTRONG;
CString		CHtml2Text::s_szOffSTRONG;
CString		CHtml2Text::s_szTABLE;
CString		CHtml2Text::s_szOffTABLE;
CString		CHtml2Text::s_szTD;
CString		CHtml2Text::s_szTH;
CString		CHtml2Text::s_szTITLE;
CString		CHtml2Text::s_szOffTITLE;
CString		CHtml2Text::s_szTR;
CString		CHtml2Text::s_szU;
CString		CHtml2Text::s_szOffU;
CString		CHtml2Text::s_szUL;
CString		CHtml2Text::s_szOffUL;
CString		CHtml2Text::s_szXHTML;
CString		CHtml2Text::s_szOffXHTML;

CString		CHtml2Text::s_szBLACK;
CString		CHtml2Text::s_szSILVER;
CString		CHtml2Text::s_szGRAY;
CString		CHtml2Text::s_szWHITE;
CString		CHtml2Text::s_szMAROON;
CString		CHtml2Text::s_szRED;
CString		CHtml2Text::s_szPURPLE;
CString		CHtml2Text::s_szFUCHSIA;
CString		CHtml2Text::s_szGREEN;
CString		CHtml2Text::s_szLIME;
CString		CHtml2Text::s_szOLIVE;
CString		CHtml2Text::s_szYELLOW;
CString		CHtml2Text::s_szNAVY;
CString		CHtml2Text::s_szBLUE;
CString		CHtml2Text::s_szTEAL;
CString		CHtml2Text::s_szAQUA;
	
CString		CHtml2Text::s_szAacuteCAP;
CString		CHtml2Text::s_szAgraveCAP;
CString		CHtml2Text::s_szAcircCAP;
CString		CHtml2Text::s_szAtildeCAP;
CString		CHtml2Text::s_szAringCAP;
CString		CHtml2Text::s_szAumlCAP;
CString		CHtml2Text::s_szAEligCAP;
CString		CHtml2Text::s_szCcedilCAP;
CString		CHtml2Text::s_szEacuteCAP;
CString		CHtml2Text::s_szEgraveCAP;
CString		CHtml2Text::s_szEcircCAP;
CString		CHtml2Text::s_szEumlCAP;
CString		CHtml2Text::s_szIacuteCAP;
CString		CHtml2Text::s_szIgraveCAP;
CString		CHtml2Text::s_szIcircCAP;
CString		CHtml2Text::s_szIumlCAP;
CString		CHtml2Text::s_szETHCAP;
CString		CHtml2Text::s_szNtildeCAP;
CString		CHtml2Text::s_szOacuteCAP;
CString		CHtml2Text::s_szOgraveCAP;
CString		CHtml2Text::s_szOcircCAP;
CString		CHtml2Text::s_szOtildeCAP;
CString		CHtml2Text::s_szOumlCAP;
CString		CHtml2Text::s_szOslashCAP;
CString		CHtml2Text::s_szUacuteCAP;
CString		CHtml2Text::s_szUgraveCAP;
CString		CHtml2Text::s_szUcircCAP;
CString		CHtml2Text::s_szUumlCAP;
CString		CHtml2Text::s_szYacuteCAP;
CString		CHtml2Text::s_szThornCAP;
	
CString		CHtml2Text::s_szligLOWER;

CString		CHtml2Text::s_szAacuteLOWER;
CString		CHtml2Text::s_szAgraveLOWER;
CString		CHtml2Text::s_szAcircLOWER;
CString		CHtml2Text::s_szAtildeLOWER;
CString		CHtml2Text::s_szAringLOWER;
CString		CHtml2Text::s_szAumlLOWER;
CString		CHtml2Text::s_szAEligLOWER;
CString		CHtml2Text::s_szCcedilLOWER;
CString		CHtml2Text::s_szEacuteLOWER;
CString		CHtml2Text::s_szEgraveLOWER;
CString		CHtml2Text::s_szEcircLOWER;
CString		CHtml2Text::s_szEumlLOWER;
CString		CHtml2Text::s_szIacuteLOWER;
CString		CHtml2Text::s_szIgraveLOWER;
CString		CHtml2Text::s_szIcircLOWER;
CString		CHtml2Text::s_szIumlLOWER;
CString		CHtml2Text::s_szETHLOWER;
CString		CHtml2Text::s_szNtildeLOWER;
CString		CHtml2Text::s_szOacuteLOWER;
CString		CHtml2Text::s_szOgraveLOWER;
CString		CHtml2Text::s_szOcircLOWER;
CString		CHtml2Text::s_szOtildeLOWER;
CString		CHtml2Text::s_szOumlLOWER;
CString		CHtml2Text::s_szOslashLOWER;
CString		CHtml2Text::s_szUacuteLOWER;
CString		CHtml2Text::s_szUgraveLOWER;
CString		CHtml2Text::s_szUcircLOWER;
CString		CHtml2Text::s_szUumlLOWER;
CString		CHtml2Text::s_szYacuteLOWER;
CString		CHtml2Text::s_szThornLOWER;
CString		CHtml2Text::s_szYumlLOWER;
CString		CHtml2Text::s_szCopy;
CString		CHtml2Text::s_szReg;
CString		CHtml2Text::s_szTrade;
CString		CHtml2Text::s_szShy;
CString		CHtml2Text::s_szCBSP;



/////////////////////////////////////////////////////////////////////////////////////////
//
//	CHtmlBlockInfo 
// 
CHtmlBlockInfo&	CHtmlBlockInfo::operator = ( const CHtmlBlockInfo& src )
{
	m_lIndent = src.m_lIndent;
	return *this;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//	CHtml2Text
//

CHtml2Text::CHtml2Text(BOOL bDoFormatFlowed)
{
	m_bDoFormatFlowed = bDoFormatFlowed;
}

///////////////////////////////////////////////////////////////////////////////
//
//	BOOL CHtml2Text::Html2Text(LPCTSTR szText)
//
//	The main conversion function.
//

CString CHtml2Text::Html2Text(LPCTSTR szText)
{
	LPCTSTR			szStartTag;
	LPCTSTR			szEndTag;
	ULONG			ulLength;
	CString			szElement;
	CString			szBase;
	UINT			u;
	UINT			uMax;
	UINT			uInHead;
	UINT			uInTitle;
	CStringArray	szAttributeList;
	ULONG			ulTotalLength;
	UINT			uInTable;
	INT				i;

	m_uExcerptLevel = 0;
	m_bTextOnLine = FALSE;
	m_uInPre = 0;
	m_lIndent = 0;

	// Empty any previous results
	m_szDest.Empty();

	if( ( szText == NULL ) || ( *szText == '\0' ) )
		return m_szDest;


	uInHead = 0;
	uInTitle = 0;
	ulTotalLength = strlen( szText );
	uInTable = 0;

	// Optimization to prevent lots of small buffer allocations.
	// Resulting plain text should never be larger than the original HTML.
	m_szDest.GetBuffer(ulTotalLength);

	if ( ! LoadMyStrings() )
	{
		AddText(szText, ulTotalLength);
		return m_szDest;
	}

	for (szStartTag = szText; *szStartTag;)
	{
		// get the current postion
		ulLength = ( szStartTag - szText );

		// find the first directive
		ulLength = strcspn( szStartTag, "<" );
		
		if( !uInHead && !uInTitle )
		{
			// display the text before the next directive
			AddText(szStartTag, ulLength);
		}
			
		// initialize the end pointer to the 
		szEndTag = szStartTag + ulLength;
		
		if (!*szEndTag)
		{
			// we're done!
			break;
		}

		// read in the directive and parse it 
		GetAttributeList(&szEndTag, szAttributeList);
		
		if (szAttributeList.GetSize())
		{
			if( szAttributeList[0].CompareNoCase( s_szBR ) == 0 )
			{
				AddNewLine(1, FALSE);
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szP ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szPARA ) == 0 ) )
			{
				AddNewLine(2, TRUE);
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szOffP ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffPARA ) == 0 ) )
			{
				AddNewLine(1, FALSE);
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szBLOCKQUOTE ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szBQ ) == 0 ) )
			{
				SetExcerptLevel(1);
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szOffBLOCKQUOTE ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffBQ ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				SetExcerptLevel(-1);
			}
			else if( ( szAttributeList[0].CompareNoCase( s_szA ) == 0 ) && 
						!uInHead && 
						!uInTitle )
			{
				NewAnchor(szAttributeList);
			}
			else if( szAttributeList[0].CompareNoCase( s_szIMG ) == 0 ) 
			{
				bool	bIsEmoticon = false;
				uMax = szAttributeList.GetSize();

				// Search attributes to see if the embedded image is an emoticon
				for (u = 1; u + 2 < uMax; u++)
				{
					CString	szAttributeName = szAttributeList[u];
					CString	szEqual = szAttributeList[u + 1];
					CString szAttributeValue = szAttributeList[u + 2];
					
					if ( (szAttributeList[u].CompareNoCase(s_szEudoraTag) == 0) &&
						 (szAttributeList[u + 1] == "=") )
					{
						CString		szAttributeValue = szAttributeList[u + 2];

						szAttributeValue.TrimLeft('"');
						szAttributeValue.TrimRight('"');

						if (szAttributeValue.CompareNoCase("emoticon") == 0)
						{
							bIsEmoticon = true;
							break;
						}
					}
				}

				for (u = 1; u + 2 < uMax; u++)
				{
					if (szAttributeList[u].CompareNoCase(s_szALT) == 0 &&
						szAttributeList[u + 1] == "=")
					{
						CString sz = szAttributeList[u + 2];
						
						if (sz.GetLength() >= 2 && sz[0] == '"')
							sz = sz.Mid( 1, sz.GetLength() - 2 );

						// Add a new line before and after the alternate text
						// (unless the embedded image is an emoticon). Because
						// non-emoticon embedded images could be smack up against
						// other text like this:
						// before embedded image1eda886a.jpgafter embedded image
						// Such cases look better like this:
						// before embedded image
						// 1eda886a.jpg
						// after embedded image
						// Setting the 2nd parameter to TRUE should help avoid double lines before or
						// after the embedded image text.
						if (!bIsEmoticon)
							AddNewLine(1, TRUE);
						AddText(sz, sz.GetLength());
						if (!bIsEmoticon)
							AddNewLine(1, TRUE);
						break;
					}
				}				
				szAttributeList.SetSize( 0 );
				u = 0;				
			}
			else if( ( szAttributeList[0].CompareNoCase( s_szHTML ) == 0 ) || 
				( szAttributeList[0].CompareNoCase( s_szXHTML ) == 0 ) )
			{
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szOffHTML ) == 0 ) || 
						( szAttributeList[0].CompareNoCase( s_szOffXHTML ) == 0 ) )
			{
			}
			else if( szAttributeList[0].CompareNoCase( s_szHEAD ) == 0 )
			{
				uInHead++;
			}
			else if( szAttributeList[0].CompareNoCase( s_szOffHEAD ) == 0 )
			{
				if (uInHead)
					uInHead--;
			}
			else if( szAttributeList[0].CompareNoCase( s_szTITLE ) == 0 )
			{
				uInTitle++;
			}
			else if( szAttributeList[0].CompareNoCase( s_szOffTITLE ) == 0 )
			{
				if (uInTitle)
					uInTitle--;
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szCENTER ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szDIV ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				SaveBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szOffCENTER ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffDIV ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szH1 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szH2 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szH3 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szH4 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szH5 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szH6 ) == 0 ) )	
			{
				AddNewLine(2, TRUE);
				SaveBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szOffH1 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffH2 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffH3 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffH4 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffH5 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffH6 ) == 0 ) )	
			{
				AddNewLine(2, TRUE);
				RestoreBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szUL ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOL ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				SaveBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szLI ) == 0 ) 
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings(FALSE);
				m_lIndent += 1;
				AddText("* ", 2);
			}
			else if(	( szAttributeList[0].CompareNoCase( s_szOffUL ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( s_szOffOL ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szDL ) == 0 )
			{
				SaveBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szDD ) == 0 )
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( s_szDT ) == 0 ) 
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( s_szOffDL ) == 0 ) 
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szTABLE ) == 0 )
			{
				++uInTable;
				AddNewLine(1, TRUE); 
				SaveBlockSettings();
				// push the default row settings
				SaveBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szOffTABLE ) == 0 )
			{
				if( uInTable )
				{
					-- uInTable;

					AddNewLine(1, TRUE);
					
					// toss the row settings
					if (!m_BlockList.IsEmpty())
						m_BlockList.RemoveTail();

					RestoreBlockSettings();
				}
			}
			else if( szAttributeList[0].CompareNoCase( s_szTR ) == 0 )
			{
				if( uInTable )
				{
					AddNewLine(1, TRUE);
					RestoreBlockSettings();

					// set and save the new ones
					SaveBlockSettings();
				}
			}
			else if( szAttributeList[0].CompareNoCase( s_szTD ) == 0 )
			{
				if (uInTable)
					RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( s_szTH ) == 0 )
			{
				if (uInTable)
					RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( s_szHR ) == 0 )
			{
				AddNewLine(1, FALSE);
				SaveBlockSettings();
				AddText("----------", 10);
				AddNewLine(1, FALSE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szPRE ) == 0 )
			{
				m_uInPre++;
			}
			else if( szAttributeList[0].CompareNoCase( s_szOffPRE ) == 0 )
			{
				if (m_uInPre)
					m_uInPre--;
			}
			else if( szAttributeList[0].CompareNoCase( s_szBASE ) == 0 ) 
			{
				uMax = szAttributeList.GetSize();
				for (u = 1 ; u + 2 < uMax; u++)
				{
					if (szAttributeList[u].CompareNoCase(s_szHREF) == 0 &&
						szAttributeList[u + 1] == "=")
					{
						CString szURL = szAttributeList[u + 2];
						
						if (szURL.GetLength() >= 2 && szURL[0] == '"')
							szURL = szURL.Mid( 1, szURL.GetLength() - 2 );
						
						i = szURL.Find( ":/" );
						
						if (i > 0)
						{
							i += 2;
							
							// skip the additional forward slash
							if (i < szURL.GetLength() && szURL[i] == '/')
								i ++;

							// find the ending '/' 
							while (i < szURL.GetLength() && szURL[i] != '/')
								i++;

							if (i == szURL.GetLength())
								szBase = szURL;
							else
								szBase = szURL.Left(i);
						}
					}
				}
			}
			else if (szAttributeList[0].CompareNoCase( s_szDIR ) == 0)
			{
				AddNewLine(1, TRUE);
				SaveBlockSettings();
			}
			else if (szAttributeList[0].CompareNoCase( s_szOffDIR ) == 0)
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( s_szSignature ) == 0 )
			{
				AddText("-- ", 3);
				AddNewLine(1, FALSE);
			}
		}

		// set the start pointer
		szStartTag = szEndTag;
	}

	m_BlockList.RemoveAll();

	return m_szDest;
}

///////////////////////////////////////////////////////////////////////////////
//
// BOOL CHtml2Text::LoadStrings()
//
// Loads recognized HTML strings.  
//
// Input: 
//		None
//
// Return Value:  
//		TRUE on success, otherwise FALSE
//
BOOL CHtml2Text::LoadMyStrings()
{
	if (s_bStringsLoaded)
		return TRUE;
	
	try
	{	
		s_szEudoraTag = CRString( IDS_EUDORA_TAG );
		s_szAutoURL = CRString( IDS_AUTOURL );
		s_szSignature = CString( "x-signature" );

		s_szHTML = CRString( IDS_MIME_HTML );
		s_szOffHTML = "/" + s_szHTML;	
		s_szXHTML = CRString( IDS_MIME_XHTML );
		s_szOffXHTML = "/" + s_szXHTML;	
		s_szA = CRString( IDS_HTML_ANCHOR );
		s_szOffA = "/" + s_szA;
		s_szALIGN = CRString( IDS_HTML_ALIGN );
		s_szALT = CRString( IDS_HTML_ALT );
		s_szAMP = CRString( IDS_HTML_AMPERSAND );
		s_szB = CRString( IDS_HTML_BOLD );
		s_szBASE = CRString( IDS_HTML_BASE );
		s_szOffB = "/" + s_szB;
		s_szBIG = CRString( IDS_HTML_BIGGER );
		s_szOffBIG = "/" + s_szBIG;
		s_szBLOCKQUOTE = CRString( IDS_HTML_BLOCKQUOTE );
		s_szOffBLOCKQUOTE = "/" + s_szBLOCKQUOTE;
		s_szBQ = CRString( IDS_HTML_BQ );
		s_szOffBQ = "/" + s_szBQ;
		s_szBR = CRString( IDS_HTML_BREAK );
		s_szCENTER = CRString( IDS_HTML_CENTER );
		s_szOffCENTER = "/" + s_szCENTER;
		s_szCOLOR = CRString( IDS_HTML_COLOR );
		s_szDD = CRString( IDS_HTML_DD );
		s_szDIR = CRString( IDS_HTML_DIR );
		s_szOffDIR = "/" + s_szDIR;
		s_szDIV = CRString( IDS_HTML_DIV );
		s_szOffDIV = "/" + s_szDIV;
		s_szDL = CRString( IDS_HTML_DL );
		s_szOffDL = "/" + s_szDL;
		s_szDT = CRString( IDS_HTML_DT );
		s_szEM = CRString( IDS_HTML_EM );
		s_szOffEM = "/" + s_szEM;
		s_szFACE = CRString( IDS_HTML_FONTFACE );
		s_szFONT = CRString( IDS_HTML_FONT );
		s_szOffFONT = "/" + s_szFONT;
		s_szGT = CRString( IDS_HTML_GREATERTHAN );
		s_szH1 = CRString( IDS_HTML_HEADER1 );
		s_szOffH1 = "/" + s_szH1;
		s_szH2 = CRString( IDS_HTML_HEADER2 );
		s_szOffH2 = "/" + s_szH2;
		s_szH3 = CRString( IDS_HTML_HEADER3 );
		s_szOffH3 = "/" + s_szH3;
		s_szH4 = CRString( IDS_HTML_HEADER4 );
		s_szOffH4 = "/" + s_szH4;
		s_szH5 = CRString( IDS_HTML_HEADER5 );
		s_szOffH5 = "/" + s_szH5;
		s_szH6 = CRString( IDS_HTML_HEADER6 );
		s_szOffH6 = "/" + s_szH6;
		s_szHEAD = CRString( IDS_HTML_HEAD );
		s_szOffHEAD = "/" + s_szHEAD;
		s_szHR = CRString( IDS_HTML_HR );
		s_szHREF = CRString( IDS_HTML_HREF );
		s_szI = CRString( IDS_HTML_ITALIC );
		s_szIMG = CRString( IDS_HTML_IMG );
		s_szOffI = "/" + s_szI;
		s_szLEFT = CRString( IDS_HTML_LEFT );
		s_szLI = CRString( IDS_HTML_LISTITEM );
//		s_szLink = CRString( IDS_LINK );
		s_szLink = "<link>";
		s_szLT = CRString( IDS_HTML_LESSTHAN );
		s_szNAME = CRString( IDS_HTML_NAME );
		s_szNBSP = CRString( IDS_HTML_SPACE );
		s_szQUOT = CRString( IDS_HTML_QUOTE );	
		s_szOL = CRString( IDS_HTML_ORDEREDLIST );
		s_szOffOL = "/" + s_szOL;
		s_szP = CRString( IDS_HTML_P );
		s_szOffP = "/" + s_szP;
		s_szPARA = CRString( IDS_HTML_PARA );
		s_szOffPARA = "/" + s_szPARA;
		s_szPRE = CRString( IDS_HTML_PRE );
		s_szOffPRE = "/" + s_szPRE;
		s_szRIGHT = CRString( IDS_HTML_RIGHT );
		s_szSIZE = CRString( IDS_HTML_FONTSIZE );
		s_szSMALL = CRString( IDS_HTML_SMALLER );
		s_szOffSMALL = "/" + s_szSMALL;
		s_szSTRONG = CRString( IDS_HTML_STRONG );
		s_szOffSTRONG = "/" + s_szSTRONG;
		s_szTABLE = CRString( IDS_HTML_TABLE );
		s_szOffTABLE = "/" + s_szTABLE;
		s_szTD = CRString( IDS_HTML_TD );
		s_szTH = CRString( IDS_HTML_TH );
		s_szTITLE = CRString( IDS_HTML_TITLE );
		s_szOffTITLE = "/" + s_szTITLE;
		s_szTR = CRString( IDS_HTML_TR );
		s_szU = CRString( IDS_HTML_UNDERLINE );
		s_szOffU = "/" + s_szU;
		s_szUL = CRString( IDS_HTML_LIST );
		s_szOffUL = "/" + s_szUL;
			
		s_szBLACK = CRString( IDS_HTML_BLACK );
		s_szSILVER = CRString( IDS_HTML_SILVER );
		s_szGRAY = CRString( IDS_HTML_GRAY );
		s_szWHITE = CRString( IDS_HTML_WHITE );
		s_szMAROON = CRString( IDS_HTML_MAROON );
		s_szRED = CRString( IDS_HTML_RED );
		s_szPURPLE = CRString( IDS_HTML_PURPLE );
		s_szFUCHSIA = CRString( IDS_HTML_FUCHSIA );
		s_szGREEN = CRString( IDS_HTML_GREEN );
		s_szLIME = CRString( IDS_HTML_LIME );
		s_szOLIVE = CRString( IDS_HTML_OLIVE );
		s_szYELLOW = CRString( IDS_HTML_YELLOW );
		s_szNAVY = CRString( IDS_HTML_NAVY );
		s_szBLUE = CRString( IDS_HTML_BLUE );
		s_szTEAL = CRString( IDS_HTML_TEAL );
		s_szAQUA = CRString( IDS_HTML_AQUA );

		s_szAacuteCAP = CRString( IDS_HTML_AACUTE_CAP );
		s_szAgraveCAP = CRString( IDS_HTML_AGRAVE_CAP );
		s_szAcircCAP = CRString( IDS_HTML_ACIRC_CAP );
		s_szAtildeCAP = CRString( IDS_HTML_ATILDE_CAP );
		s_szAringCAP = CRString( IDS_HTML_ARING_CAP );
		s_szAumlCAP = CRString( IDS_HTML_AUML_CAP );
		s_szAEligCAP = CRString( IDS_HTML_AELIG_CAP );
		s_szCcedilCAP = CRString( IDS_HTML_CCEDIL_CAP );
		s_szEacuteCAP = CRString( IDS_HTML_EACUTE_CAP );
		s_szEgraveCAP = CRString( IDS_HTML_EGRAVE_CAP );
		s_szEcircCAP = CRString( IDS_HTML_ECIRC_CAP );
		s_szEumlCAP = CRString( IDS_HTML_EUML_CAP );
		s_szIacuteCAP = CRString( IDS_HTML_IACUTE_CAP );
		s_szIgraveCAP = CRString( IDS_HTML_IGRAVE_CAP );
		s_szIcircCAP = CRString( IDS_HTML_ICIRC_CAP );
		s_szIumlCAP = CRString( IDS_HTML_IUML_CAP );
		s_szETHCAP = CRString( IDS_HTML_ETH_CAP );
		s_szNtildeCAP = CRString( IDS_HTML_NTILDE_CAP );
		s_szOacuteCAP = CRString( IDS_HTML_OACUTE_CAP );
		s_szOgraveCAP = CRString( IDS_HTML_OGRAVE_CAP );
		s_szOcircCAP = CRString( IDS_HTML_OCIRC_CAP );
		s_szOtildeCAP = CRString( IDS_HTML_OTILDE_CAP );
		s_szOumlCAP = CRString( IDS_HTML_OUML_CAP );
		s_szOslashCAP = CRString( IDS_HTML_OSLASH_CAP );
		s_szUacuteCAP = CRString( IDS_HTML_UACUTE_CAP );
		s_szUgraveCAP = CRString( IDS_HTML_UGRAVE_CAP );
		s_szUcircCAP = CRString( IDS_HTML_UCIRC_CAP );
		s_szUumlCAP = CRString( IDS_HTML_UUML_CAP );
		s_szYacuteCAP = CRString( IDS_HTML_YACUTE_CAP );
		s_szThornCAP = CRString( IDS_HTML_THORN_CAP );
		
		s_szligLOWER = CRString( IDS_HTML_SZLIG_LOWER );

		s_szAacuteLOWER = CRString( IDS_HTML_AACUTE_LOWER );
		s_szAgraveLOWER = CRString( IDS_HTML_AGRAVE_LOWER );
		s_szAcircLOWER = CRString( IDS_HTML_ACIRC_LOWER );
		s_szAtildeLOWER = CRString( IDS_HTML_ATILDE_LOWER );
		s_szAringLOWER = CRString( IDS_HTML_ARING_LOWER );
		s_szAumlLOWER = CRString( IDS_HTML_AUML_LOWER );
		s_szAEligLOWER = CRString( IDS_HTML_AELIG_LOWER );
		s_szCcedilLOWER = CRString( IDS_HTML_CCEDIL_LOWER );
		s_szEacuteLOWER = CRString( IDS_HTML_EACUTE_LOWER );
		s_szEgraveLOWER = CRString( IDS_HTML_EGRAVE_LOWER );
		s_szEcircLOWER = CRString( IDS_HTML_ECIRC_LOWER );
		s_szEumlLOWER = CRString( IDS_HTML_EUML_LOWER );
		s_szIacuteLOWER = CRString( IDS_HTML_IACUTE_LOWER );
		s_szIgraveLOWER = CRString( IDS_HTML_IGRAVE_LOWER );
		s_szIcircLOWER = CRString( IDS_HTML_ICIRC_LOWER );
		s_szIumlLOWER = CRString( IDS_HTML_IUML_LOWER );
		s_szETHLOWER = CRString( IDS_HTML_ETH_LOWER );
		s_szNtildeLOWER = CRString( IDS_HTML_NTILDE_LOWER );
		s_szOacuteLOWER = CRString( IDS_HTML_OACUTE_LOWER );
		s_szOgraveLOWER = CRString( IDS_HTML_OGRAVE_LOWER );
		s_szOcircLOWER = CRString( IDS_HTML_OCIRC_LOWER );
		s_szOtildeLOWER = CRString( IDS_HTML_OTILDE_LOWER );
		s_szOumlLOWER = CRString( IDS_HTML_OUML_LOWER );
		s_szOslashLOWER = CRString( IDS_HTML_OSLASH_LOWER );
		s_szUacuteLOWER = CRString( IDS_HTML_UACUTE_LOWER );
		s_szUgraveLOWER = CRString( IDS_HTML_UGRAVE_LOWER );
		s_szUcircLOWER = CRString( IDS_HTML_UCIRC_LOWER );
		s_szUumlLOWER = CRString( IDS_HTML_UUML_LOWER );
		s_szYacuteLOWER = CRString( IDS_HTML_YACUTE_LOWER );
		s_szThornLOWER = CRString( IDS_HTML_THORN_LOWER );
		
		s_szYumlLOWER = CRString( IDS_HTML_YUML_LOWER );
		
		s_szCopy = CRString( IDS_HTML_COPY );
		s_szReg = CRString( IDS_HTML_REG );
		s_szTrade = CRString( IDS_HTML_TRADE );
		s_szShy = CRString( IDS_HTML_SHY );
		s_szCBSP = CRString( IDS_HTML_CBSP );
	}
	catch( CException* pExp )
	{
		pExp->Delete();
		return FALSE;
	}

	s_bStringsLoaded = true;

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//
//	void CHtml2Text::RestoreBlockSettings()
//
//	Restores the last block settings
//
void CHtml2Text::RestoreBlockSettings(BOOL bRemoveSettings /*= TRUE*/)
{
	CString			szAlign;
	CHtmlBlockInfo	theBlockInfo;

	if (m_BlockList.IsEmpty())
	{
		// Why are we restoring when we did not save?
		// This ASSERTs all the time these days, particularly when scanning
		// to index email. I'd guess that there's just lots of invalid
		// HTML out there.
		//ASSERT(0);

		// default to left aligned, not indented
		theBlockInfo.m_lIndent = 0;
	}
	else
	{	
		// get the last settings
		theBlockInfo = m_BlockList.GetTail();		

		if (bRemoveSettings)
			m_BlockList.RemoveTail();
	}

	// restore the indent
	m_lIndent = theBlockInfo.m_lIndent;
}


///////////////////////////////////////////////////////////////////////////////
//
//	void CHtml2Text::SaveBlockSettings() 
//
//	Adds the current block settings to the list.
//
void CHtml2Text::SaveBlockSettings()
{
	CHtmlBlockInfo	theInfo;

	// get the current indent
	theInfo.m_lIndent = m_lIndent;

	// add it to the list
	m_BlockList.AddTail(theInfo);
}	


///////////////////////////////////////////////////////////////////////////////
//
//	void CHtml2Text::AddNewLine(UINT nCount, BOOL bEnsureFirst);
//
//	Adds line breaks.
//
//	Input:
//		UINT	nCount			-	the number of new lines to add
//		BOOL	bEnsureFirst	-	if TRUE, the first line break will be added
//									only if the last characters displayed were not
//									a "\r\n" pair
//
//	Return Value:
//		None, but if a new line was added, m_bTextOnLine is set to FALSE.
//
void CHtml2Text::AddNewLine(UINT nCount, BOOL bEnsureFirst)
{
	UINT	u;
	
	if( bEnsureFirst && nCount )
	{
		// see if the last 2 characters are a "\r\n" pair

		if( ( m_szDest.GetLength() >= 2 ) &&
			( m_szDest[ m_szDest.GetLength() - 2 ] == '\r' ) &&
			( m_szDest[ m_szDest.GetLength() - 1 ] == '\n' ) )
		{
			// yep -- at least 1 "\r\n" can be eliminated
			nCount --;
		}
	}

	if( nCount == 0 )
	{
		return;
	}

	if( m_bTextOnLine )
	{
		m_szDest += "\r\n";
		--nCount;
	}

	if (nCount)
	{
		CString	szExcerpt( '>', m_uExcerptLevel );
		szExcerpt += "\r\n";

		// add new lines
		for( u = 0 ; u < nCount; u++ )
		{
			m_szDest += szExcerpt;
		}
	}

	m_bTextOnLine = FALSE;
}



///////////////////////////////////////////////////////////////////////////////
//
//	void CHtml2Text::GetAttributeList(LPCTSTR* pszText, CStringArray& theArray)
//
//	Parses the current HTML element and it's attributes.
//
//	Input:
//		LPCTSTR*		pszText		-	The pointer to the source text pointer.
//										This pointer must be pointing to the 
//										opening '<' sign.  On return, this pointer
//										will be advanced to the first character
//										after the ending '>'. 
//		CStringArray&	theArray	-	the destination array.  The first 
//										element will be the HTML element.  The
//										rest will be the attribute statements.
//
//	Return Value:
//		None -- but the text pointer is advanced.
//
void CHtml2Text::GetAttributeList(LPCTSTR* pszText, CStringArray& theArray)
{
	LPTSTR	szCurrent;
	BOOL	bInQuote;
	UINT	u;
	LPCTSTR	szStart;
	ULONG	ulLength;
	TCHAR	cSave;

	theArray.SetSize(0, 32);

	// skip the opening less than sign
	( *pszText ) ++;

	// first, see if this is a comment
	if( ( (*pszText)[0] == '!' ) && ( (*pszText)[1] == '-' ) && ( (*pszText)[2] == '-' ) )
	{
		szStart = *pszText + 3;
		
		while( *szStart )
		{
			// find the next "--" sequence
			szCurrent = strstr( szStart, "--" );
			
			// set the next starting place if necessary			
			szStart = szCurrent + 1;

			if( szCurrent == NULL )
			{
				// bad html
				break;			
			}
			
			// skip the --
			szCurrent += 2;

			// skip ws
			while( iswspace( (unsigned char)*szCurrent ) )
			{
				szCurrent ++;
			}

			if( *szCurrent == '>' ) 
			{
				// we're done -- set the return pointer and bail
				*pszText = szCurrent + 1;
				return;
			}		
		}							
	}
	
	bInQuote = FALSE;

	// first, get the substring to work on
	// don't break on '>' unless we're not inside a quoted section
	for( ulLength = 0 ; ((*pszText)[ulLength] != '\0') && (bInQuote || ((*pszText)[ulLength] != '>')); ulLength ++ )
	{
		if( (*pszText)[ulLength] == '"' )
		{
			bInQuote = !bInQuote;
		}
	}
	
	// set the start pointer
	szStart = *pszText;
	
	// set the return pointer
	if ( (*pszText)[ulLength] == '\0' )
	{
		*pszText += ulLength;
	}
	else
	{
		*pszText += ulLength + 1;
	}

	// skip whitespace
	while( ulLength && iswspace( (unsigned char)*szStart ) ) 
	{
		szStart ++;
		ulLength --;
	}

	while( ulLength )
	{
		if ( *szStart == '=' )
		{
			// add the equal sign to the array
			theArray.Add( "=" );
			szStart++;
			--ulLength;
		}
		else			
		{
			bInQuote = FALSE;

			// find the next white space char or '='
			for( u = 0; u < ulLength; u++ )
			{
				if( szStart[u] == '"' )
				{
					bInQuote = !bInQuote;
				}
								
				if( !bInQuote && ( ( szStart[u] == '=' ) || iswspace( (unsigned char)szStart[u] ) ) )
				{
					// found a break
					break;
				}
			}
			// save the ending character
			cSave = szStart[u];
			// create the substring for adding
			((LPTSTR)szStart)[u] = 0;
			// add it
			theArray.Add( szStart );
			// restore the character
			((LPTSTR)szStart)[u] = cSave;
			// skip white space
			while( iswspace( (unsigned char)szStart[u] ) && ( u < ulLength ) )
			{
				u++;
			}
			// set the start pointer and length counter
			szStart += u;
			ulLength -= u;
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
//
//	void CHtml2Text::AddText(LPCTSTR szText, ULONG ulTotalLength)
//
//	Outputs text to the rich edit control. 
//
//	Input:
//		const char*	szText			-	a pointer to the display text
//		ULONG		ulTotalLength	-	the length of the span to display
//
//	Return Value:
//		None, but if at least one character is displayed on the current line, m_bTextOnLine is set to TRUE.
//
void CHtml2Text::AddText(LPCTSTR szText, ULONG ulTotalLength)
{
	ULONG	ul;
	CString	szTemp;
	INT		i;
	CString	szExcerpt( '>', m_uExcerptLevel );

	for( i = 0; i < m_lIndent; i++ )
	{
		szExcerpt += "    ";
	}

	while( ( ulTotalLength > 0 ) && ( *szText != 0 ) )
	{
		BOOL bAddSpace = FALSE;

		if( !m_uInPre && iswspace( (unsigned char)szText[0] ) )
		{
			// skip all white space
			for( ul = 0; ul < ulTotalLength ; ul++ )
			{
				if ( !iswspace( (unsigned char)szText[ul] ) )
				{
					break;
				}
				
				if( m_bTextOnLine || ( ( szText[ ul ] != '\r' ) && ( szText[ ul ] != '\n' ) ) )
				{
					bAddSpace = TRUE;
				}
			}
			
			szText += ul;
			ulTotalLength -= ul;
		}

		if (!m_bTextOnLine && (bAddSpace || ulTotalLength > 0))
		{
			m_szDest += szExcerpt;
			m_bTextOnLine = TRUE;

			if (m_bDoFormatFlowed && !bAddSpace)
			{
				// Leading space, ">" and "From " on a line need to be space-stuffed,
				// if we're doing output for format=flowed.
				if ((ulTotalLength >= 5 && strnicmp(szText, "&nbsp", 5) == 0) ||
					(ulTotalLength >= 3 && strnicmp(szText, "&gt", 3) == 0) ||
					(ulTotalLength >= 5 && strncmp(szText, "From ", 5) == 0))
				{
					m_szDest += ' ';
				}
			}
		}

		if( bAddSpace )
			m_szDest += ' ';

		if (ulTotalLength <= 0)
			break;

		if( szText[0] == '&' )
		{
			if ( ulTotalLength == 1 )
			{
				m_szDest += '&';
				ulTotalLength --;
			}
			else
			{
				szTemp.Empty();
				
				if ( szText[1] == '#' )
				{
					// get the number string	
					for( ul = 2; ( ul < ulTotalLength ) && isdigit( (int)(unsigned char)(szText[ul]) ); ul++ )
					{
						szTemp += szText[ul];
					}
					
					// convert the number to a character
					m_szDest += ( char ) ( atoi( szTemp ) & 0xFF );
				}
				else
				{
					// get the string
					ul = 1;
					while (ul < ulTotalLength && isalpha((int)(unsigned char)(szText[ul])))
					{
						 ul++ ;
					}
					
					// map the string
					//hardcoded the tag len for speed
					if ( ul == 3 && (strncmp(szText, s_szGT, 3) == 0))
					{
						m_szDest += '>';
					}
					else if ( ul == 3 && (strncmp(szText, s_szLT, 3) == 0))
					{
						m_szDest += '<';
					}
					else if ( ul == 4 && (strncmp(szText, s_szAMP, 4) == 0))
					{
						m_szDest += '&';
					}
					else if ( ul == 5 && (strncmp(szText, s_szNBSP, 5) == 0))
					{
						m_szDest += ' ';
					}
					else if ( ul == 5 && (strncmp(szText, s_szQUOT, 5) == 0))
					{
						m_szDest += '"';
					}
					else if ( ul == 7 && (strncmp(szText, s_szAacuteCAP, 7) == 0))
					{
						m_szDest += 'Á';
					}
					else if ( ul == 7 && (strncmp(szText, s_szAgraveCAP, 7) == 0))
					{
						m_szDest += 'À';
					}
					else if ( ul == 6 && (strncmp(szText, s_szAcircCAP, 6) == 0))
					{
						m_szDest += 'Â';
					}
					else if ( ul == 7 && (strncmp(szText, s_szAtildeCAP, 7) == 0))
					{
						m_szDest += 'Ã';
					}
					else if ( ul == 6 && (strncmp(szText, s_szAringCAP, 6) == 0))
					{
						m_szDest += 'Å';
					}
					else if ( ul == 5 && (strncmp(szText, s_szAumlCAP, 5) == 0))
					{
						m_szDest += 'Ä';
					}
					else if ( ul == 6 && (strncmp(szText, s_szAEligCAP, 6) == 0))
					{
						m_szDest += 'Æ';
					}
					else if ( ul == 7 && (strncmp(szText, s_szCcedilCAP, 7) == 0))
					{
						m_szDest += 'Ç';
					}
					else if ( ul == 7 && (strncmp(szText, s_szEacuteCAP, 7) == 0))
					{
						m_szDest += 'É';
					}
					else if ( ul == 7 && (strncmp(szText, s_szEgraveCAP, 7) == 0))
					{
						m_szDest += 'È';
					}
					else if ( ul == 6 && (strncmp(szText, s_szEcircCAP, 6) == 0))
					{
						m_szDest += 'Ê';
					}
					else if ( ul == 5 && (strncmp(szText, s_szEumlCAP, 5) == 0))
					{
						m_szDest += 'Ë';
					}
					else if ( ul == 7 && (strncmp(szText, s_szIacuteCAP, 7) == 0))
					{
						m_szDest += 'Í';
					}
					else if ( ul == 7 && (strncmp(szText, s_szIgraveCAP, 7) == 0))
					{
						m_szDest += 'Ì';
					}
					else if ( ul == 6 && (strncmp(szText, s_szIcircCAP, 6) == 0))
					{
						m_szDest += 'Î';
					}
					else if ( ul == 5 && (strncmp(szText, s_szIumlCAP, 5) == 0))
					{
						m_szDest += 'Ï';
					}
					else if ( ul == 4 && (strncmp(szText, s_szETHCAP, 4) == 0))
					{
						m_szDest += 'Ð';
					}
					else if ( ul == 7 && (strncmp(szText, s_szNtildeCAP, 7) == 0))
					{
						m_szDest += 'Ñ';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOacuteCAP, 7) == 0))
					{
						m_szDest += 'Ó';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOgraveCAP, 7) == 0))
					{
						m_szDest += 'Ó';
					}
					else if ( ul == 6 && (strncmp(szText, s_szOcircCAP, 6) == 0))
					{
						m_szDest += 'Ô';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOtildeCAP, 7) == 0))
					{
						m_szDest += 'Õ';
					}
					else if ( ul == 5 && (strncmp(szText, s_szOumlCAP, 5) == 0))
					{
						m_szDest += 'Ö';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOslashCAP, 7) == 0))
					{
						m_szDest += 'Ø';
					}
					else if ( ul == 7 && (strncmp(szText, s_szUacuteCAP, 7) == 0))
					{
						m_szDest += 'Ú';
					}
					else if ( ul == 7 && (strncmp(szText, s_szUgraveCAP, 7) == 0))
					{
						m_szDest += 'Ù';
					}
					else if ( ul == 6 && (strncmp(szText, s_szUcircCAP, 6) == 0))
					{
						m_szDest += 'Û';
					}
					else if ( ul == 5 && (strncmp(szText, s_szUumlCAP, 5) == 0))
					{
						m_szDest += 'Ü';
					}
					else if ( ul == 7 && (strncmp(szText, s_szYacuteCAP, 7) == 0))
					{
						m_szDest += 'Ý';
					}
					else if ( ul == 6 && (strncmp(szText, s_szThornCAP, 6) == 0))
					{
						m_szDest += 'Þ';
					}
					else if ( ul == 6 && (strncmp(szText, s_szligLOWER, 6) == 0))
					{
						m_szDest += 'ß';
					}
					else if ( ul == 7 && (strncmp(szText, s_szAacuteLOWER, 7) == 0))
					{
						m_szDest += 'á';
					}
					else if ( ul == 7 && (strncmp(szText, s_szAgraveLOWER, 7) == 0))
					{
						m_szDest += 'à';
					}
					else if ( ul == 6 && (strncmp(szText, s_szAcircLOWER, 6) == 0))
					{
						m_szDest += 'â';
					}
					else if ( ul == 7 && (strncmp(szText, s_szAtildeLOWER, 7) == 0))
					{
						m_szDest += 'ã';
					}
					else if ( ul == 6 && (strncmp(szText, s_szAringLOWER, 6) == 0))
					{
						m_szDest += 'å';
					}
					else if ( ul == 5 && (strncmp(szText, s_szAumlLOWER, 5) == 0))
					{
						m_szDest += 'ä';
					}
					else if ( ul == 6 && (strncmp(szText, s_szAEligLOWER, 6) == 0))
					{
						m_szDest += 'æ';
					}
					else if ( ul == 7 && (strncmp(szText, s_szCcedilLOWER, 7) == 0))
					{
						m_szDest += 'ç';
					}
					else if ( ul == 7 && (strncmp(szText, s_szEacuteLOWER, 7) == 0))
					{
						m_szDest += 'é';
					}
					else if ( ul == 7 && (strncmp(szText, s_szEgraveLOWER, 7) == 0))
					{
						m_szDest += 'è';
					}
					else if ( ul == 6 && (strncmp(szText, s_szEcircLOWER, 6) == 0))
					{
						m_szDest += 'ê';
					}
					else if ( ul == 5 && (strncmp(szText, s_szEumlLOWER, 5) == 0))
					{
						m_szDest += 'ë';
					}
					else if ( ul == 7 && (strncmp(szText, s_szIacuteLOWER, 7) == 0))
					{
						m_szDest += 'í';
					}
					else if ( ul == 7 && (strncmp(szText, s_szIgraveLOWER, 7) == 0))
					{
						m_szDest += 'ì';
					}
					else if ( ul == 7 && (strncmp(szText, s_szIcircLOWER, 6) == 0))
					{
						m_szDest += 'î';
					}
					else if ( ul == 5 && (strncmp(szText, s_szIumlLOWER, 5) == 0))
					{
						m_szDest += 'ï';
					}
					else if ( ul == 4 && (strncmp(szText, s_szETHLOWER, 4) == 0))
					{
						m_szDest += 'ð';
					}
					else if ( ul == 7 && (strncmp(szText, s_szNtildeLOWER, 7) == 0))
					{
						m_szDest += 'ñ';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOacuteLOWER, 7) == 0))
					{
						m_szDest += 'ó';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOgraveLOWER, 7) == 0))
					{
						m_szDest += 'ò';
					}
					else if ( ul == 6 && (strncmp(szText, s_szOcircLOWER, 6) == 0))
					{
						m_szDest += 'ô';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOtildeLOWER, 7) == 0))
					{
						m_szDest += 'õ';
					}
					else if ( ul == 5 && (strncmp(szText, s_szOumlLOWER, 5) == 0))
					{
						m_szDest += 'ö';
					}
					else if ( ul == 7 && (strncmp(szText, s_szOslashLOWER, 7) == 0))
					{
						m_szDest += 'ø';
					}
					else if ( ul == 7 && (strncmp(szText, s_szUacuteLOWER, 7) == 0))
					{
						m_szDest += 'ú';
					}
					else if ( ul == 7 && (strncmp(szText, s_szUgraveLOWER, 7) == 0))
					{
						m_szDest += 'ù';
					}
					else if ( ul == 6 && (strncmp(szText, s_szUcircLOWER, 6) == 0))
					{
						m_szDest += 'û';
					}
					else if ( ul == 5 && (strncmp(szText, s_szUumlLOWER, 5) == 0))
					{
						m_szDest += 'ü';
					}
					else if ( ul == 7 && (strncmp(szText, s_szYacuteLOWER, 7) == 0))
					{
						m_szDest += 'ý';
					}
					else if ( ul == 6 && (strncmp(szText, s_szThornLOWER, 6) == 0))
					{
						m_szDest += 'þ';
					}
					else if ( ul == 5 && (strncmp(szText, s_szYumlLOWER, 5) == 0))
					{
						m_szDest += 'ÿ';
					}
					else if ( ul == 5 && (strncmp(szText, s_szCopy, 5) == 0))
					{
						m_szDest += '©';
					}
					else if ( ul == 4 && (strncmp(szText, s_szReg, 4) == 0))
					{
						m_szDest += '®';
					}
					else if ( ul == 6 && (strncmp(szText, s_szTrade, 6) == 0))
					{
						m_szDest += '™';
					}
					else if ( ul == 4 && (strncmp(szText, s_szShy, 4) == 0))
					{
						m_szDest += '–';
					}
					else if ( ul == 5 && (strncmp(szText, s_szCBSP, 5) == 0))
					{
						m_szDest += ' ';
					}
					else
					{
						// display unknown special chars 
						m_szDest += CString(szText, ul);
					}
				}				
				
				szText += ul;
				ulTotalLength -= ul;

				if( ( szText[0] == ';' ) && ulTotalLength )
				{
					szText++;
					ulTotalLength --;
				}
			}
		}
		else
		{
			// plain old text
			if ( m_uInPre && '\n' == *szText )
				m_bTextOnLine = FALSE;
			else
				m_bTextOnLine = TRUE;

			m_szDest += szText[0];
			szText++;
			ulTotalLength --;
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
//
//	void CHtml2Text::SetExcerptLevel(INT iInc)
//
//	Sets up the display for a specific excerpt level
//
//	Input:
//		INT					iInc			-	The increment.  Excerpts should
//												only be incremented by +1 or -1.
//
//	Return Value:
//		None
//
void CHtml2Text::SetExcerptLevel(INT iInc)
{
	// add a new line if necessary

	AddNewLine(1, TRUE);

	if( ( iInc < 0 ) && !m_uExcerptLevel )
	{
		// bad input
		return;
	}

	if( iInc > 0 ) 
	{
		// add the block settings
		SaveBlockSettings();
		// set the excerpt level
		m_uExcerptLevel += iInc;
	}
	else
	{
		// set the excerpt level
		m_uExcerptLevel += iInc;

		// reset the block settings	
		RestoreBlockSettings();
	}
}


void CHtml2Text::NewAnchor(CStringArray& szAttributeList)
{
	CString		szValue;
	UINT		uMax;
	CString		szHRef;

	uMax = szAttributeList.GetSize();	
	szHRef.Empty();
	
	for (UINT u = 1; u + 2 < uMax; u++)
	{
		szValue = szAttributeList[u+2];
		
		if( ( szValue.GetLength() >= 2 ) && ( szValue[0] == '"' ) )
		{
			szValue = szValue.Mid( 1, szValue.GetLength() - 2 );	
		}

		if(	( szAttributeList[u].CompareNoCase( s_szEudoraTag ) == 0 ) &&
			( szAttributeList[u+1] == "=" ) &&
			( szValue.CompareNoCase( s_szAutoURL ) == 0 ) )
		{
			// don't include auto urls
			szHRef.Empty();
			break;
		}

		if(	( szAttributeList[u].CompareNoCase( s_szHREF ) == 0 ) &&
			( szAttributeList[u+1] == "=" ) &&
			( szValue.IsEmpty() == FALSE ) )
		{
			// Only makes sense to display full URLs (not relative ones), and a full URL
			// has to start with a scheme, which consists only of letters, digits, '+', '-', and '.'
			TCHAR c = szValue[0];
			if ( !isalnum((int)(unsigned char)c) && '+' != c && '-' != c && '.' != c )
				break;
			szHRef = szValue;
		}
	}

	if( szHRef.IsEmpty() == FALSE ) 
	{
		// display the url 
		AddText("<", 1);
		AddText(szHRef, szHRef.GetLength());
		AddText(">", 1);
	}
}




CString	Html2Text(LPCTSTR szSource, BOOL bDoFormatFlowed /*= FALSE*/)
{
	CHtml2Text h2t(bDoFormatFlowed);

	return h2t.Html2Text(szSource);
}

