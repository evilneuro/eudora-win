#include "stdafx.h"
#include "eudora.h"
#include "rs.h"
#include "striprich.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CString	CStripRichString::m_szExcerptPrefix;
CString	CStripRichString::m_szExcerptStart;
CString	CStripRichString::m_szExcerptEnd;
CString	CStripRichString::m_szPARAM_ON;
CString	CStripRichString::m_szEXCERPT_ON;
CString	CStripRichString::m_szPARAGRAPH_ON;
CString	CStripRichString::m_szNOFILL_ON;
CString	CStripRichString::m_szPARAM_OFF;
CString	CStripRichString::m_szEXCERPT_OFF;
CString	CStripRichString::m_szPARAGRAPH_OFF;
CString	CStripRichString::m_szNOFILL_OFF;

CStripRichString::CStripRichString(
UINT	uExcerptLevel,
BOOL	bNoFill )
{
	m_bNoFill = bNoFill;
	m_uLastLineIndex = 0;
	m_uLastLineLength = 0;
	m_iLastSpacePos = -1;
	SetExcerptLevel( uExcerptLevel );
}


void CStripRichString::Append(
const char*	pBuf,
ULONG		ulLength)
{
	char*	pTemp;
	char*	pInsert;

	while( ulLength )
	{
		// make sure the excerpt chars have been added to this line 
		// before we start adding chars
		if( ( m_uLastLineLength == 0 ) && ( m_szExcerpt != "" ) )
		{
			*this += m_szExcerpt;
			m_uLastLineLength = m_szExcerpt.GetLength();
		}

		// check for \r\n pair
		if( ( pBuf[0] == '\r' ) && ( pBuf[1] == '\n' ) && ( ulLength >= 2 ) )
		{
			// see if it's followed by one more \r\n pairs
			if ( ( pBuf[2] == '\r' ) && ( pBuf[3] == '\n' ) && ( ulLength >= 4 ) )
			{
				// yep -- add the remaining pairs
				pBuf += 2;
				ulLength -= 2;

				while( ( pBuf[0] == '\r' ) && ( pBuf[1] == '\n' ) && ( ulLength >= 2 ) )
				{
					// add the \r\n and the excerpt characters ( if any )
					*this += "\r\n" + m_szExcerpt;
					pBuf += 2;
					ulLength -= 2;
				}
				// set the last line length to be the # of excerpt chars
				m_uLastLineLength = m_szExcerpt.GetLength();
				// set the last line index to the begining of the last line
				m_uLastLineIndex = GetLength() - m_uLastLineLength;
				// clear the last space pos
				m_iLastSpacePos = -1;
			}
			else
			{
				// see if nofill is on
				if( m_bNoFill )
				{
					// just add the \r\n pair & the excerpt chars
					// add the \r\n and the excerpt characters ( if any )
					*this += "\r\n" + m_szExcerpt;
					pBuf += 2;
					ulLength -= 2;

					// set the last line length to be the # of excerpt chars
					m_uLastLineLength = m_szExcerpt.GetLength();
					// set the last line index to the begining of the last line
					m_uLastLineIndex = GetLength() - m_uLastLineLength;
					// clear the last space pos
					m_iLastSpacePos = -1;
				}		
				else
				{
					// replace the \r\n pair with a space
					*this += ' ';
					// get the next char
					pBuf += 2;
					ulLength -= 2;

					// set the last space index - if it's not past the wrap column
					if( m_uLastLineLength + 1 <= m_uWWColumn )
					{
						m_iLastSpacePos = m_uLastLineIndex + m_uLastLineLength;
					}
					// increment the length counter
					m_uLastLineLength ++;
				}
			}
		}
		else
		{
			// add the character
			*this += *pBuf;

			// set the last space index - if it's not past the wrap column
			if( ( *pBuf == ' ' ) && ( m_uLastLineLength + 1 <= m_uWWColumn ) )
			{
				m_iLastSpacePos = m_uLastLineIndex + m_uLastLineLength;
			}

			// get the next char
			pBuf++;
			ulLength --;

			// increment the length counter
			m_uLastLineLength ++;
		}

		// now see if we should wrap
		if( m_uLastLineLength >= m_uWWMax ) 
		{
			// we didn't hit a \r\n before the max value, so wrap at iWWColumn
			// first see if we hit a space
			if( m_iLastSpacePos >= 0 )
			{
				// create space for \n & excerpt
				pTemp = GetBufferSetLength( GetLength() + 1 + m_szExcerpt.GetLength() );
				
				// set the pointer
				pInsert = pTemp + m_iLastSpacePos;

				// move everything but the space
				memmove(	pInsert + m_szExcerpt.GetLength() + 2, 
							pInsert + 1, 
							m_uLastLineLength - ( m_iLastSpacePos - m_uLastLineIndex ) - 1 );
			}
			else
			{
				// there were no spaces
				// create space for \r & \n & excerpt
				pTemp = GetBufferSetLength( GetLength() + 2 + m_szExcerpt.GetLength() );

				// set the pointer
				pInsert = pTemp + m_uWWColumn;

				// move everything
				memmove( pInsert + m_szExcerpt.GetLength() + 2, pInsert, m_uLastLineLength - m_uWWColumn);
			}

			// insert the \r\n & the excerpt string
			*pInsert++ = '\r';
			*pInsert++ = '\n';
			memcpy( pInsert, m_szExcerpt, m_szExcerpt.GetLength() ); 
			
			// set the last line length and last line index
			m_uLastLineLength = strlen( pInsert );
			m_uLastLineIndex = pInsert - pTemp;
						
			// find the index of the last space less than m_uWWColumn
			if ( m_uLastLineLength >= m_uWWColumn )
			{
				m_iLastSpacePos = m_uLastLineIndex + m_uWWColumn - 1;
			}
			else
			{
				m_iLastSpacePos = m_uLastLineIndex + m_uLastLineLength - 1;
			}
			
			while( m_iLastSpacePos >= (int) m_uLastLineIndex )
			{
				if ( pTemp[ m_iLastSpacePos ] == ' ' )
				{
					break;
				}
				
				--m_iLastSpacePos;
			}
			
			if( m_iLastSpacePos < (int) m_uLastLineIndex )
			{
				m_iLastSpacePos = -1;
			}
					
			// release the buffer
			ReleaseBuffer();		
		}
	}
}



void	CStripRichString::SetExcerptLevel(
UINT	uExcerptLevel )
{
	// set the column width and column max values

	m_uWWColumn = GetIniLong(IDS_INI_WORD_WRAP_COLUMN) - uExcerptLevel;
	
	if ( m_uWWColumn < MIN_COLUMN )
	{
		m_uWWColumn = MIN_COLUMN;
	}

	m_uWWMax = GetIniLong(IDS_INI_WORD_WRAP_MAX) - uExcerptLevel;
	
	if( m_uWWMax >= m_uWWColumn * 2L ) 
	{
		m_uWWMax = ( m_uWWColumn * 2 ) - 1;
	}
	
	// save the excerpt level
	m_uExcerptLevel = uExcerptLevel;

	// build the excerpt prefix
	m_szExcerpt = "";
		
	while( uExcerptLevel -- )
	{
		m_szExcerpt += m_szExcerptPrefix;
	}
}

const CString&	CStripRichString::Strip(
const char*	pBuf,
UINT		uExcerptLevel,
BOOL		bNoFill )
{
	LPCSTR				pStartDirective;
	LPCSTR				pEndDirective;
	LPCSTR				pEndParam;
	ULONG				ulLength;
	LPCSTR				pSearchFrom;

	SetExcerptLevel( uExcerptLevel );
	m_bNoFill = bNoFill;

	if( m_szEXCERPT_OFF == "" )
	{
		// build some frequently used strings from resources
		m_szPARAM_ON.LoadString( IDS_MIMERICH_PARAM );
		m_szEXCERPT_ON.LoadString( IDS_MIMERICH_EXCERPT );
		m_szPARAGRAPH_ON.LoadString( IDS_MIMERICH_EXCERPT );
		m_szNOFILL_ON.LoadString( IDS_MIMERICH_NOFILL );
		m_szEXCERPT_OFF = "/" + m_szEXCERPT_ON;
		m_szPARAGRAPH_OFF = "/" + m_szPARAGRAPH_ON;
		m_szPARAM_OFF = "/" + m_szPARAM_ON;
		m_szNOFILL_OFF = "/" + m_szNOFILL_ON;

		m_szExcerptStart = GetIniString( IDS_INI_REPLY_START );
		m_szExcerptEnd = GetIniString( IDS_INI_REPLY_END );	
		m_szExcerptPrefix = GetIniString( IDS_INI_REPLY_PREFIX );
	
	}
		
	while( *pBuf != '\0' )
	{
		// find the next directive
		pStartDirective = strchr( pBuf, '<' );
		
		if( pStartDirective == NULL )
		{
			ulLength = strlen( pBuf );
			
			// there's no more rich text - add the current span
			Append( pBuf, ulLength );

			// bail
			return *this;
		}

		// see if this is really just a character
		if( pStartDirective[1] == '<' )
		{
			// add the text before and 1 of the "<" chars
			Append( pBuf, pStartDirective - pBuf + 1 );

			// skip the << pair
			pBuf = pStartDirective + 2;

			continue;
		}

		// find the closing '>'
		pEndDirective = strchr( pStartDirective + 1, '>' );
		
		if( pEndDirective == NULL )
		{
			// no closing directive
			// add the '<' as a character and continue
			Append( "<", 1 );
			pBuf += 1;
			
			continue;	
		}

		// add the text before the directive
		Append( pBuf, pStartDirective - pBuf ) ;

		// get the directive
		CStripRichString	szDirective( 0, m_bNoFill );

		szDirective.Append( pStartDirective + 1, pEndDirective - pStartDirective - 1 );
		szDirective.TrimLeft();
		szDirective.TrimRight();
		
		if ( szDirective.CompareNoCase( m_szEXCERPT_OFF ) == 0 )
		{
			// we're at the end of this level
			// set the next buffer position	
			pBuf = pEndDirective + 1;
		
			*this += "\r\n";
			
			if( m_szExcerptEnd != "" )
			{
				*this += m_szExcerpt + m_szExcerptEnd + "\r\n";		
			}	

			// reset the line info
			m_iLastSpacePos = -1;
			m_uLastLineLength = 0;
			m_uLastLineIndex = GetLength();

			// set the excerpt info
			if ( m_uExcerptLevel )
			{
				SetExcerptLevel( m_uExcerptLevel - 1 );
			}
			
			continue;
		}
		
		if ( szDirective.CompareNoCase( m_szEXCERPT_ON ) == 0 )
		{
			// set the buffer pointer to the beginning of the next excerpt
			pBuf = pEndDirective + 1;
			
			// make sure there's a \r\n before the excerpt
			if( ( GetLength() < 2 ) || (  Right(2) != "\r\n" ) )
			{
				*this += "\r\n";
			}

			if( m_szExcerptStart != "" )
			{
				*this += m_szExcerpt + m_szExcerptStart + "\r\n";
			}

			// set the excerpt stuff
			SetExcerptLevel( m_uExcerptLevel + 1 );

			// reset the line info
			m_iLastSpacePos = -1;
			m_uLastLineLength = 0;
			m_uLastLineIndex = GetLength();			

			continue;
		}

		if ( szDirective.CompareNoCase( m_szPARAM_ON ) == 0 )
		{
			// find the next /param  TODO -- make this case insensitive
			pSearchFrom = pEndDirective + 1;

			while( 1 )
			{
				pEndParam = strstr( pSearchFrom, "</" );
				
				if( ( pEndParam == NULL ) ||  ( strnicmp( pEndParam + 1, m_szPARAM_OFF, 6 ) == 0 ) )
				{
					// one way or another, we're done
					break;
				}
				
				pSearchFrom += 2;
			} 

			if( pEndParam != NULL )
			{
				// skip to the closing '>'
				while( *pEndParam != '\0' )
				{
					if ( *pEndParam == '>' ) 
					{
						pEndParam ++;
						break;
					}
					pEndParam ++;
				}

				// skip everything between the directives
				pBuf = pEndParam;
				continue;
			}
		}
		else
		if( szDirective.CompareNoCase( m_szPARAGRAPH_OFF ) == 0 )
		{		
			// add two crlf pairs
			Append( "\r\n\r\n\r\n" , 6 );
		}
		else					
		if ( szDirective.CompareNoCase( m_szNOFILL_ON ) == 0 )
		{
			// turn on nofill
			m_bNoFill = TRUE;
		}
		else
		if ( szDirective.CompareNoCase( m_szNOFILL_OFF ) == 0 )
		{
			// turn off nofill
			m_bNoFill = FALSE;
		}

		// set the buffer pointer to the next char past the ending '>'
		pBuf = pEndDirective + 1;
	}

	return *this;
}
