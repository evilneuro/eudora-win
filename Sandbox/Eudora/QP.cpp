// QP.cpp
//
// Classes for doing quoted-printable encoding and decoding

#include "stdafx.h"

#include "QP.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// QPEncoder - convert binary data to quoted-printable
//
int QPEncoder::Init(char* Out, LONG& OutLen, int flag = 0)
{
	m_Flag = flag;
	m_bCR = FALSE;
	return (OutLen = m_BytesOnLine = 0);
}

int QPEncoder::Done(char* Out, LONG& OutLen)
{
	return (OutLen = 0);
}

int QPEncoder::Encode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	// Modified EncodeQP so that it now encodes Text/Enriched as well.
	
	const char* InSpot = In;			// the byte currently being decoded
	const char* end = InSpot + InLen;	// end of decoding
	char* OutSpot = Out;				// the spot to which to copy the encoded chars
	static const char hex[] = "0123456789ABCDEF";
	const char* nextSpace;
	BOOL encode;
	char c;
//	BOOL bRich			= m_Flag & ENCODE_RICH;
	BOOL bQP			= m_Flag & ENCODE_QP;
	BOOL bCurlyQuotes	= m_Flag & ENCODE_CURLYQUOTES;

	for (; InLen; m_bCR = (*InSpot == '\r'), InSpot++, InLen--)
	{
		// make copy of char
		c = *InSpot;
		
//		if ( bRich )
//		{
//			if ( c == '\n' )
//			{
//				if (!m_bCR) 
//				{ 
//				//	RoomFor(2, OutSpot);
//					*OutSpot++ = '\r';
//					*OutSpot++ = '\n';
//					m_bCR=TRUE;
//					m_BytesOnLine = 0;
//				}
//			}
//			else
//			if ( c != '\r' )
//			{
//				m_bCR=FALSE;
//				if ( c == '<' )
//				{
//				//	RoomFor(1,OutSpot);
//					*OutSpot++ = '<';
//					m_BytesOnLine++;
//				}
//			}
//		}		

		if ( bQP )
		{
			// handle newlines
			if (c == '\n' && m_bCR)
			{
				*OutSpot++ = '\n';
				m_BytesOnLine = 0;
			}
			else if (c == '\r' && (InLen == 1 || InSpot[1] == '\n'))
			{
				*OutSpot++ = '\r';
			}
			else
			{
				if (c == ' ' || c == '\t')
				{
					// trailing white space
					encode = (InLen == 1 || InSpot[1] == '\r');
					if (!encode)
					{
						for (nextSpace = InSpot + 1; nextSpace < end; nextSpace++)
							if (*nextSpace == ' ' || *nextSpace == '\r')
							{
								RoomFor(nextSpace - InSpot, OutSpot);
								break;
							}
					}
				}
				else
				{
					encode = (c == '=' || c < 33 || c > 126);	// weird characters
					if (!encode && !m_BytesOnLine)
					{
						// Check for "From " at the beginning of a line.  If found,
						// then encode the 'F' so it MTAs don't corrupt it.
						int CharsToCompare = min(5, InLen);
						if (strncmp(InSpot, "From ", CharsToCompare) == 0)
							encode = TRUE;
					}
				}

				if (encode)
				{
					RoomFor(3, OutSpot);
					*OutSpot++ = '=';
					*OutSpot++ = hex[(c >> 4) & 0xF];
					*OutSpot++ = hex[c & 0xF];
					m_BytesOnLine += 3;
				}
				else
				{
					RoomFor(1, OutSpot);
					*OutSpot++ = c;
					m_BytesOnLine++;
				}
			}
		}
		else // no Quoted Printable
		{
//			if ( !bRich )
//				RoomFor(1, OutSpot);
			*OutSpot++ = c;
			m_BytesOnLine++;
		}

		OutLen = OutSpot - Out;
	}

	return (0);
}

inline char HEX(char c)
{
	if (c >= '0' && c <= '9')
		return ((char)(c - '0'));
	if (c >= 'A' && c <= 'F')
		return ((char)(c - 'A' + 10));
	if (c >= 'a' && c <= 'f')
		return ((char)(c - 'a' + 10));
	return (-1);
}


// QPDecoder - convert quoted-printable data to binary
//
int QPDecoder::Decode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	char c;
	char* OutSpot = Out;
	short errs = 0;
	char upperNib, lowerNib;

	for (; InLen; In++, InLen--)
	{
		c = *In;
		switch (m_State)
		{
			case qpNormal:
				if (c == '=')
					m_State = qpEqual;
				else
					*OutSpot++ = c;
				break;
					
			case qpEqual:
				// Skip CRs, fold LFs
				if (c != '\r')
					m_State = (c == '\n'? qpNormal : qpByte1);
				break;
				
			case qpByte1:
				upperNib = HEX(m_LastChar);
				lowerNib = HEX(c);
				if (upperNib < 0 || lowerNib < 0)
					errs++;
				else
					*OutSpot++ = (char)((upperNib << 4) | lowerNib);
				m_State = qpNormal;
				break;
		}
		m_LastChar = c;
	}

	OutLen = OutSpot - Out;
	
	return (errs);
}
