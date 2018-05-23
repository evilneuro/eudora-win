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


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



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
			if( szAttributeList[0].CompareNoCase( m_szBR ) == 0 )
			{
				AddNewLine(1, FALSE);
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szP ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szPARA ) == 0 ) )
			{
				AddNewLine(2, TRUE);
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szOffP ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffPARA ) == 0 ) )
			{
				AddNewLine(1, FALSE);
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szBLOCKQUOTE ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szBQ ) == 0 ) )
			{
				SetExcerptLevel(1);
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szOffBLOCKQUOTE ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffBQ ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				SetExcerptLevel(-1);
			}
			else if( ( szAttributeList[0].CompareNoCase( m_szA ) == 0 ) && 
						!uInHead && 
						!uInTitle )
			{
				NewAnchor(szAttributeList);
			}
			else if( szAttributeList[0].CompareNoCase( m_szIMG ) == 0 ) 
			{
				uMax = szAttributeList.GetSize();
				for (u = 1; u + 2 < uMax; u++)
				{
					if (szAttributeList[u].CompareNoCase(m_szALT) == 0 &&
						szAttributeList[u + 1] == "=")
					{
						CString sz = szAttributeList[u + 2];
						
						if (sz.GetLength() >= 2 && sz[0] == '"')
							sz = sz.Mid( 1, sz.GetLength() - 2 );

						AddText(sz, sz.GetLength());
						break;
					}
				}				
				szAttributeList.SetSize( 0 );
				u = 0;				
			}
			else if( ( szAttributeList[0].CompareNoCase( m_szHTML ) == 0 ) || 
				( szAttributeList[0].CompareNoCase( m_szXHTML ) == 0 ) )
			{
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szOffHTML ) == 0 ) || 
						( szAttributeList[0].CompareNoCase( m_szOffXHTML ) == 0 ) )
			{
			}
			else if( szAttributeList[0].CompareNoCase( m_szHEAD ) == 0 )
			{
				uInHead++;
			}
			else if( szAttributeList[0].CompareNoCase( m_szOffHEAD ) == 0 )
			{
				if (uInHead)
					uInHead--;
			}
			else if( szAttributeList[0].CompareNoCase( m_szTITLE ) == 0 )
			{
				uInTitle++;
			}
			else if( szAttributeList[0].CompareNoCase( m_szOffTITLE ) == 0 )
			{
				if (uInTitle)
					uInTitle--;
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szCENTER ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szDIV ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				SaveBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szOffCENTER ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffDIV ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szH1 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szH2 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szH3 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szH4 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szH5 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szH6 ) == 0 ) )	
			{
				AddNewLine(2, TRUE);
				SaveBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szOffH1 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffH2 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffH3 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffH4 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffH5 ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffH6 ) == 0 ) )	
			{
				AddNewLine(2, TRUE);
				RestoreBlockSettings();
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szUL ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOL ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				SaveBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( m_szLI ) == 0 ) 
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings(FALSE);
				m_lIndent += 1;
				AddText("* ", 2);
			}
			else if(	( szAttributeList[0].CompareNoCase( m_szOffUL ) == 0 ) ||
						( szAttributeList[0].CompareNoCase( m_szOffOL ) == 0 ) )
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( m_szDL ) == 0 )
			{
				SaveBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( m_szDD ) == 0 )
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( m_szDT ) == 0 ) 
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( m_szOffDL ) == 0 ) 
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( m_szTABLE ) == 0 )
			{
				++uInTable;
				AddNewLine(1, TRUE); 
				SaveBlockSettings();
				// push the default row settings
				SaveBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( m_szOffTABLE ) == 0 )
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
			else if( szAttributeList[0].CompareNoCase( m_szTR ) == 0 )
			{
				if( uInTable )
				{
					AddNewLine(1, TRUE);
					RestoreBlockSettings();

					// set and save the new ones
					SaveBlockSettings();
				}
			}
			else if( szAttributeList[0].CompareNoCase( m_szTD ) == 0 )
			{
				if (uInTable)
					RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( m_szTH ) == 0 )
			{
				if (uInTable)
					RestoreBlockSettings(FALSE);
			}
			else if( szAttributeList[0].CompareNoCase( m_szHR ) == 0 )
			{
				AddNewLine(1, FALSE);
				SaveBlockSettings();
				AddText("----------", 10);
				AddNewLine(1, FALSE);
				RestoreBlockSettings();
			}
			else if( szAttributeList[0].CompareNoCase( m_szPRE ) == 0 )
			{
				m_uInPre++;
			}
			else if( szAttributeList[0].CompareNoCase( m_szOffPRE ) == 0 )
			{
				if (m_uInPre)
					m_uInPre--;
			}
			else if( szAttributeList[0].CompareNoCase( m_szBASE ) == 0 ) 
			{
				uMax = szAttributeList.GetSize();
				for (u = 1 ; u + 2 < uMax; u++)
				{
					if (szAttributeList[u].CompareNoCase(m_szHREF) == 0 &&
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
			else if (szAttributeList[0].CompareNoCase( m_szDIR ) == 0)
			{
				AddNewLine(1, TRUE);
				SaveBlockSettings();
			}
			else if (szAttributeList[0].CompareNoCase( m_szOffDIR ) == 0)
			{
				AddNewLine(1, TRUE);
				RestoreBlockSettings();
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
	try
	{	
		m_szEudoraTag = CRString( IDS_EUDORA_TAG );
		m_szAutoURL = CRString( IDS_AUTOURL );

		m_szHTML = CRString( IDS_MIME_HTML );
		m_szOffHTML = "/" + m_szHTML;	
		m_szXHTML = CRString( IDS_MIME_XHTML );
		m_szOffXHTML = "/" + m_szXHTML;	
		m_szA = CRString( IDS_HTML_ANCHOR );
		m_szOffA = "/" + m_szA;
		m_szALIGN = CRString( IDS_HTML_ALIGN );
		m_szALT = CRString( IDS_HTML_ALT );
		m_szAMP = CRString( IDS_HTML_AMPERSAND );
		m_szB = CRString( IDS_HTML_BOLD );
		m_szBASE = CRString( IDS_HTML_BASE );
		m_szOffB = "/" + m_szB;
		m_szBIG = CRString( IDS_HTML_BIGGER );
		m_szOffBIG = "/" + m_szBIG;
		m_szBLOCKQUOTE = CRString( IDS_HTML_BLOCKQUOTE );
		m_szOffBLOCKQUOTE = "/" + m_szBLOCKQUOTE;
		m_szBQ = CRString( IDS_HTML_BQ );
		m_szOffBQ = "/" + m_szBQ;
		m_szBR = CRString( IDS_HTML_BREAK );
		m_szCENTER = CRString( IDS_HTML_CENTER );
		m_szOffCENTER = "/" + m_szCENTER;
		m_szCOLOR = CRString( IDS_HTML_COLOR );
		m_szDD = CRString( IDS_HTML_DD );
		m_szDIR = CRString( IDS_HTML_DIR );
		m_szOffDIR = "/" + m_szDIR;
		m_szDIV = CRString( IDS_HTML_DIV );
		m_szOffDIV = "/" + m_szDIV;
		m_szDL = CRString( IDS_HTML_DL );
		m_szOffDL = "/" + m_szDL;
		m_szDT = CRString( IDS_HTML_DT );
		m_szEM = CRString( IDS_HTML_EM );
		m_szOffEM = "/" + m_szEM;
		m_szFACE = CRString( IDS_HTML_FONTFACE );
		m_szFONT = CRString( IDS_HTML_FONT );
		m_szOffFONT = "/" + m_szFONT;
		m_szGT = CRString( IDS_HTML_GREATERTHAN );
		m_szH1 = CRString( IDS_HTML_HEADER1 );
		m_szOffH1 = "/" + m_szH1;
		m_szH2 = CRString( IDS_HTML_HEADER2 );
		m_szOffH2 = "/" + m_szH2;
		m_szH3 = CRString( IDS_HTML_HEADER3 );
		m_szOffH3 = "/" + m_szH3;
		m_szH4 = CRString( IDS_HTML_HEADER4 );
		m_szOffH4 = "/" + m_szH4;
		m_szH5 = CRString( IDS_HTML_HEADER5 );
		m_szOffH5 = "/" + m_szH5;
		m_szH6 = CRString( IDS_HTML_HEADER6 );
		m_szOffH6 = "/" + m_szH6;
		m_szHEAD = CRString( IDS_HTML_HEAD );
		m_szOffHEAD = "/" + m_szHEAD;
		m_szHR = CRString( IDS_HTML_HR );
		m_szHREF = CRString( IDS_HTML_HREF );
		m_szI = CRString( IDS_HTML_ITALIC );
		m_szIMG = CRString( IDS_HTML_IMG );
		m_szOffI = "/" + m_szI;
		m_szLEFT = CRString( IDS_HTML_LEFT );
		m_szLI = CRString( IDS_HTML_LISTITEM );
//		m_szLink = CRString( IDS_LINK );
		m_szLink = "<link>";
		m_szLT = CRString( IDS_HTML_LESSTHAN );
		m_szNAME = CRString( IDS_HTML_NAME );
		m_szNBSP = CRString( IDS_HTML_SPACE );
		m_szQUOT = CRString( IDS_HTML_QUOTE );	
		m_szOL = CRString( IDS_HTML_ORDEREDLIST );
		m_szOffOL = "/" + m_szOL;
		m_szP = CRString( IDS_HTML_P );
		m_szOffP = "/" + m_szP;
		m_szPARA = CRString( IDS_HTML_PARA );
		m_szOffPARA = "/" + m_szPARA;
		m_szPRE = CRString( IDS_HTML_PRE );
		m_szOffPRE = "/" + m_szPRE;
		m_szRIGHT = CRString( IDS_HTML_RIGHT );
		m_szSIZE = CRString( IDS_HTML_FONTSIZE );
		m_szSMALL = CRString( IDS_HTML_SMALLER );
		m_szOffSMALL = "/" + m_szSMALL;
		m_szSTRONG = CRString( IDS_HTML_STRONG );
		m_szOffSTRONG = "/" + m_szSTRONG;
		m_szTABLE = CRString( IDS_HTML_TABLE );
		m_szOffTABLE = "/" + m_szTABLE;
		m_szTD = CRString( IDS_HTML_TD );
		m_szTH = CRString( IDS_HTML_TH );
		m_szTITLE = CRString( IDS_HTML_TITLE );
		m_szOffTITLE = "/" + m_szTITLE;
		m_szTR = CRString( IDS_HTML_TR );
		m_szU = CRString( IDS_HTML_UNDERLINE );
		m_szOffU = "/" + m_szU;
		m_szUL = CRString( IDS_HTML_LIST );
		m_szOffUL = "/" + m_szUL;
			
		m_szBLACK = CRString( IDS_HTML_BLACK );
		m_szSILVER = CRString( IDS_HTML_SILVER );
		m_szGRAY = CRString( IDS_HTML_GRAY );
		m_szWHITE = CRString( IDS_HTML_WHITE );
		m_szMAROON = CRString( IDS_HTML_MAROON );
		m_szRED = CRString( IDS_HTML_RED );
		m_szPURPLE = CRString( IDS_HTML_PURPLE );
		m_szFUCHSIA = CRString( IDS_HTML_FUCHSIA );
		m_szGREEN = CRString( IDS_HTML_GREEN );
		m_szLIME = CRString( IDS_HTML_LIME );
		m_szOLIVE = CRString( IDS_HTML_OLIVE );
		m_szYELLOW = CRString( IDS_HTML_YELLOW );
		m_szNAVY = CRString( IDS_HTML_NAVY );
		m_szBLUE = CRString( IDS_HTML_BLUE );
		m_szTEAL = CRString( IDS_HTML_TEAL );
		m_szAQUA = CRString( IDS_HTML_AQUA );

		m_szAacuteCAP = CRString( IDS_HTML_AACUTE_CAP );
		m_szAgraveCAP = CRString( IDS_HTML_AGRAVE_CAP );
		m_szAcircCAP = CRString( IDS_HTML_ACIRC_CAP );
		m_szAtildeCAP = CRString( IDS_HTML_ATILDE_CAP );
		m_szAringCAP = CRString( IDS_HTML_ARING_CAP );
		m_szAumlCAP = CRString( IDS_HTML_AUML_CAP );
		m_szAEligCAP = CRString( IDS_HTML_AELIG_CAP );
		m_szCcedilCAP = CRString( IDS_HTML_CCEDIL_CAP );
		m_szEacuteCAP = CRString( IDS_HTML_EACUTE_CAP );
		m_szEgraveCAP = CRString( IDS_HTML_EGRAVE_CAP );
		m_szEcircCAP = CRString( IDS_HTML_ECIRC_CAP );
		m_szEumlCAP = CRString( IDS_HTML_EUML_CAP );
		m_szIacuteCAP = CRString( IDS_HTML_IACUTE_CAP );
		m_szIgraveCAP = CRString( IDS_HTML_IGRAVE_CAP );
		m_szIcircCAP = CRString( IDS_HTML_ICIRC_CAP );
		m_szIumlCAP = CRString( IDS_HTML_IUML_CAP );
		m_szETHCAP = CRString( IDS_HTML_ETH_CAP );
		m_szNtildeCAP = CRString( IDS_HTML_NTILDE_CAP );
		m_szOacuteCAP = CRString( IDS_HTML_OACUTE_CAP );
		m_szOgraveCAP = CRString( IDS_HTML_OGRAVE_CAP );
		m_szOcircCAP = CRString( IDS_HTML_OCIRC_CAP );
		m_szOtildeCAP = CRString( IDS_HTML_OTILDE_CAP );
		m_szOumlCAP = CRString( IDS_HTML_OUML_CAP );
		m_szOslashCAP = CRString( IDS_HTML_OSLASH_CAP );
		m_szUacuteCAP = CRString( IDS_HTML_UACUTE_CAP );
		m_szUgraveCAP = CRString( IDS_HTML_UGRAVE_CAP );
		m_szUcircCAP = CRString( IDS_HTML_UCIRC_CAP );
		m_szUumlCAP = CRString( IDS_HTML_UUML_CAP );
		m_szYacuteCAP = CRString( IDS_HTML_YACUTE_CAP );
		m_szThornCAP = CRString( IDS_HTML_THORN_CAP );
		
		m_szligLOWER = CRString( IDS_HTML_SZLIG_LOWER );

		m_szAacuteLOWER = CRString( IDS_HTML_AACUTE_LOWER );
		m_szAgraveLOWER = CRString( IDS_HTML_AGRAVE_LOWER );
		m_szAcircLOWER = CRString( IDS_HTML_ACIRC_LOWER );
		m_szAtildeLOWER = CRString( IDS_HTML_ATILDE_LOWER );
		m_szAringLOWER = CRString( IDS_HTML_ARING_LOWER );
		m_szAumlLOWER = CRString( IDS_HTML_AUML_LOWER );
		m_szAEligLOWER = CRString( IDS_HTML_AELIG_LOWER );
		m_szCcedilLOWER = CRString( IDS_HTML_CCEDIL_LOWER );
		m_szEacuteLOWER = CRString( IDS_HTML_EACUTE_LOWER );
		m_szEgraveLOWER = CRString( IDS_HTML_EGRAVE_LOWER );
		m_szEcircLOWER = CRString( IDS_HTML_ECIRC_LOWER );
		m_szEumlLOWER = CRString( IDS_HTML_EUML_LOWER );
		m_szIacuteLOWER = CRString( IDS_HTML_IACUTE_LOWER );
		m_szIgraveLOWER = CRString( IDS_HTML_IGRAVE_LOWER );
		m_szIcircLOWER = CRString( IDS_HTML_ICIRC_LOWER );
		m_szIumlLOWER = CRString( IDS_HTML_IUML_LOWER );
		m_szETHLOWER = CRString( IDS_HTML_ETH_LOWER );
		m_szNtildeLOWER = CRString( IDS_HTML_NTILDE_LOWER );
		m_szOacuteLOWER = CRString( IDS_HTML_OACUTE_LOWER );
		m_szOgraveLOWER = CRString( IDS_HTML_OGRAVE_LOWER );
		m_szOcircLOWER = CRString( IDS_HTML_OCIRC_LOWER );
		m_szOtildeLOWER = CRString( IDS_HTML_OTILDE_LOWER );
		m_szOumlLOWER = CRString( IDS_HTML_OUML_LOWER );
		m_szOslashLOWER = CRString( IDS_HTML_OSLASH_LOWER );
		m_szUacuteLOWER = CRString( IDS_HTML_UACUTE_LOWER );
		m_szUgraveLOWER = CRString( IDS_HTML_UGRAVE_LOWER );
		m_szUcircLOWER = CRString( IDS_HTML_UCIRC_LOWER );
		m_szUumlLOWER = CRString( IDS_HTML_UUML_LOWER );
		m_szYacuteLOWER = CRString( IDS_HTML_YACUTE_LOWER );
		m_szThornLOWER = CRString( IDS_HTML_THORN_LOWER );
		
		m_szYumlLOWER = CRString( IDS_HTML_YUML_LOWER );
		
		m_szCopy = CRString( IDS_HTML_COPY );
		m_szReg = CRString( IDS_HTML_REG );
		m_szTrade = CRString( IDS_HTML_TRADE );
		m_szShy = CRString( IDS_HTML_SHY );
		m_szCBSP = CRString( IDS_HTML_CBSP );
	}
	catch( CException* pExp )
	{
		pExp->Delete();
		return FALSE;
	}
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
		ASSERT(0);

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

	theArray.SetSize( 0 );

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
			while( iswspace( *szCurrent ) )
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
	for( ulLength = 0 ; ( (*pszText)[ulLength] != '\0' ) && ( (*pszText)[ulLength] != '>' ); ulLength ++ )
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
	while( ulLength && iswspace( *szStart ) ) 
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
								
				if( !bInQuote && ( ( szStart[u] == '=' ) || iswspace( szStart[u] ) ) )
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
			while( iswspace( szStart[u] ) && ( u < ulLength ) )
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

		if( !m_uInPre && iswspace( szText[0] ) )
		{
			// skip all white space
			for( ul = 0; ul < ulTotalLength ; ul++ )
			{
				if ( !iswspace( szText[ul] ) )
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
					if ( ul == 3 && (strncmp(szText, m_szGT, 3) == 0))
					{
						m_szDest += '>';
					}
					else if ( ul == 3 && (strncmp(szText, m_szLT, 3) == 0))
					{
						m_szDest += '<';
					}
					else if ( ul == 4 && (strncmp(szText, m_szAMP, 4) == 0))
					{
						m_szDest += '&';
					}
					else if ( ul == 5 && (strncmp(szText, m_szNBSP, 5) == 0))
					{
						m_szDest += ' ';
					}
					else if ( ul == 5 && (strncmp(szText, m_szQUOT, 5) == 0))
					{
						m_szDest += '"';
					}
					else if ( ul == 7 && (strncmp(szText, m_szAacuteCAP, 7) == 0))
					{
						m_szDest += 'Á';
					}
					else if ( ul == 7 && (strncmp(szText, m_szAgraveCAP, 7) == 0))
					{
						m_szDest += 'À';
					}
					else if ( ul == 6 && (strncmp(szText, m_szAcircCAP, 6) == 0))
					{
						m_szDest += 'Â';
					}
					else if ( ul == 7 && (strncmp(szText, m_szAtildeCAP, 7) == 0))
					{
						m_szDest += 'Ã';
					}
					else if ( ul == 6 && (strncmp(szText, m_szAringCAP, 6) == 0))
					{
						m_szDest += 'Å';
					}
					else if ( ul == 5 && (strncmp(szText, m_szAumlCAP, 5) == 0))
					{
						m_szDest += 'Ä';
					}
					else if ( ul == 6 && (strncmp(szText, m_szAEligCAP, 6) == 0))
					{
						m_szDest += 'Æ';
					}
					else if ( ul == 7 && (strncmp(szText, m_szCcedilCAP, 7) == 0))
					{
						m_szDest += 'Ç';
					}
					else if ( ul == 7 && (strncmp(szText, m_szEacuteCAP, 7) == 0))
					{
						m_szDest += 'É';
					}
					else if ( ul == 7 && (strncmp(szText, m_szEgraveCAP, 7) == 0))
					{
						m_szDest += 'È';
					}
					else if ( ul == 6 && (strncmp(szText, m_szEcircCAP, 6) == 0))
					{
						m_szDest += 'Ê';
					}
					else if ( ul == 5 && (strncmp(szText, m_szEumlCAP, 5) == 0))
					{
						m_szDest += 'Ë';
					}
					else if ( ul == 7 && (strncmp(szText, m_szIacuteCAP, 7) == 0))
					{
						m_szDest += 'Í';
					}
					else if ( ul == 7 && (strncmp(szText, m_szIgraveCAP, 7) == 0))
					{
						m_szDest += 'Ì';
					}
					else if ( ul == 6 && (strncmp(szText, m_szIcircCAP, 6) == 0))
					{
						m_szDest += 'Î';
					}
					else if ( ul == 5 && (strncmp(szText, m_szIumlCAP, 5) == 0))
					{
						m_szDest += 'Ï';
					}
					else if ( ul == 4 && (strncmp(szText, m_szETHCAP, 4) == 0))
					{
						m_szDest += 'Ð';
					}
					else if ( ul == 7 && (strncmp(szText, m_szNtildeCAP, 7) == 0))
					{
						m_szDest += 'Ñ';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOacuteCAP, 7) == 0))
					{
						m_szDest += 'Ó';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOgraveCAP, 7) == 0))
					{
						m_szDest += 'Ó';
					}
					else if ( ul == 6 && (strncmp(szText, m_szOcircCAP, 6) == 0))
					{
						m_szDest += 'Ô';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOtildeCAP, 7) == 0))
					{
						m_szDest += 'Õ';
					}
					else if ( ul == 5 && (strncmp(szText, m_szOumlCAP, 5) == 0))
					{
						m_szDest += 'Ö';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOslashCAP, 7) == 0))
					{
						m_szDest += 'Ø';
					}
					else if ( ul == 7 && (strncmp(szText, m_szUacuteCAP, 7) == 0))
					{
						m_szDest += 'Ú';
					}
					else if ( ul == 7 && (strncmp(szText, m_szUgraveCAP, 7) == 0))
					{
						m_szDest += 'Ù';
					}
					else if ( ul == 6 && (strncmp(szText, m_szUcircCAP, 6) == 0))
					{
						m_szDest += 'Û';
					}
					else if ( ul == 5 && (strncmp(szText, m_szUumlCAP, 5) == 0))
					{
						m_szDest += 'Ü';
					}
					else if ( ul == 7 && (strncmp(szText, m_szYacuteCAP, 7) == 0))
					{
						m_szDest += 'Ý';
					}
					else if ( ul == 6 && (strncmp(szText, m_szThornCAP, 6) == 0))
					{
						m_szDest += 'Þ';
					}
					else if ( ul == 6 && (strncmp(szText, m_szligLOWER, 6) == 0))
					{
						m_szDest += 'ß';
					}
					else if ( ul == 7 && (strncmp(szText, m_szAacuteLOWER, 7) == 0))
					{
						m_szDest += 'á';
					}
					else if ( ul == 7 && (strncmp(szText, m_szAgraveLOWER, 7) == 0))
					{
						m_szDest += 'à';
					}
					else if ( ul == 6 && (strncmp(szText, m_szAcircLOWER, 6) == 0))
					{
						m_szDest += 'â';
					}
					else if ( ul == 7 && (strncmp(szText, m_szAtildeLOWER, 7) == 0))
					{
						m_szDest += 'ã';
					}
					else if ( ul == 6 && (strncmp(szText, m_szAringLOWER, 6) == 0))
					{
						m_szDest += 'å';
					}
					else if ( ul == 5 && (strncmp(szText, m_szAumlLOWER, 5) == 0))
					{
						m_szDest += 'ä';
					}
					else if ( ul == 6 && (strncmp(szText, m_szAEligLOWER, 6) == 0))
					{
						m_szDest += 'æ';
					}
					else if ( ul == 7 && (strncmp(szText, m_szCcedilLOWER, 7) == 0))
					{
						m_szDest += 'ç';
					}
					else if ( ul == 7 && (strncmp(szText, m_szEacuteLOWER, 7) == 0))
					{
						m_szDest += 'é';
					}
					else if ( ul == 7 && (strncmp(szText, m_szEgraveLOWER, 7) == 0))
					{
						m_szDest += 'è';
					}
					else if ( ul == 6 && (strncmp(szText, m_szEcircLOWER, 6) == 0))
					{
						m_szDest += 'ê';
					}
					else if ( ul == 5 && (strncmp(szText, m_szEumlLOWER, 5) == 0))
					{
						m_szDest += 'ë';
					}
					else if ( ul == 7 && (strncmp(szText, m_szIacuteLOWER, 7) == 0))
					{
						m_szDest += 'í';
					}
					else if ( ul == 7 && (strncmp(szText, m_szIgraveLOWER, 7) == 0))
					{
						m_szDest += 'ì';
					}
					else if ( ul == 7 && (strncmp(szText, m_szIcircLOWER, 6) == 0))
					{
						m_szDest += 'î';
					}
					else if ( ul == 5 && (strncmp(szText, m_szIumlLOWER, 5) == 0))
					{
						m_szDest += 'ï';
					}
					else if ( ul == 4 && (strncmp(szText, m_szETHLOWER, 4) == 0))
					{
						m_szDest += 'ð';
					}
					else if ( ul == 7 && (strncmp(szText, m_szNtildeLOWER, 7) == 0))
					{
						m_szDest += 'ñ';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOacuteLOWER, 7) == 0))
					{
						m_szDest += 'ó';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOgraveLOWER, 7) == 0))
					{
						m_szDest += 'ò';
					}
					else if ( ul == 6 && (strncmp(szText, m_szOcircLOWER, 6) == 0))
					{
						m_szDest += 'ô';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOtildeLOWER, 7) == 0))
					{
						m_szDest += 'õ';
					}
					else if ( ul == 5 && (strncmp(szText, m_szOumlLOWER, 5) == 0))
					{
						m_szDest += 'ö';
					}
					else if ( ul == 7 && (strncmp(szText, m_szOslashLOWER, 7) == 0))
					{
						m_szDest += 'ø';
					}
					else if ( ul == 7 && (strncmp(szText, m_szUacuteLOWER, 7) == 0))
					{
						m_szDest += 'ú';
					}
					else if ( ul == 7 && (strncmp(szText, m_szUgraveLOWER, 7) == 0))
					{
						m_szDest += 'ù';
					}
					else if ( ul == 6 && (strncmp(szText, m_szUcircLOWER, 6) == 0))
					{
						m_szDest += 'û';
					}
					else if ( ul == 5 && (strncmp(szText, m_szUumlLOWER, 5) == 0))
					{
						m_szDest += 'ü';
					}
					else if ( ul == 7 && (strncmp(szText, m_szYacuteLOWER, 7) == 0))
					{
						m_szDest += 'ý';
					}
					else if ( ul == 6 && (strncmp(szText, m_szThornLOWER, 6) == 0))
					{
						m_szDest += 'þ';
					}
					else if ( ul == 5 && (strncmp(szText, m_szYumlLOWER, 5) == 0))
					{
						m_szDest += 'ÿ';
					}
					else if ( ul == 5 && (strncmp(szText, m_szCopy, 5) == 0))
					{
						m_szDest += '©';
					}
					else if ( ul == 4 && (strncmp(szText, m_szReg, 4) == 0))
					{
						m_szDest += '®';
					}
					else if ( ul == 6 && (strncmp(szText, m_szTrade, 6) == 0))
					{
						m_szDest += '™';
					}
					else if ( ul == 4 && (strncmp(szText, m_szShy, 4) == 0))
					{
						m_szDest += '–';
					}
					else if ( ul == 5 && (strncmp(szText, m_szCBSP, 5) == 0))
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

		if(	( szAttributeList[u].CompareNoCase( m_szEudoraTag ) == 0 ) &&
			( szAttributeList[u+1] == "=" ) &&
			( szValue.CompareNoCase( m_szAutoURL ) == 0 ) )
		{
			// don't include auto urls
			szHRef.Empty();
			break;
		}

		if(	( szAttributeList[u].CompareNoCase( m_szHREF ) == 0 ) &&
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

