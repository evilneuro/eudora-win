// QP.cpp
//
// Classes for doing quoted-printable encoding and decoding
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "QP.h"


#include "DebugNewHelpers.h"


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
//	BOOL bCurlyQuotes	= m_Flag & ENCODE_CURLYQUOTES;

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
