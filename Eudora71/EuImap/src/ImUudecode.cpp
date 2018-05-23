// UUDECODE.CPP
//
// Routines for decoding uuencoded data
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include "cursor.h"
#include "fileutil.h"
#include "lex822.h"
#include "header.h"
#include "resource.h"
#include "mime.h"
#include "ImUudecode.h"
#include "resource.h"
#include "guiutils.h"
#include "rs.h"

#include "ImapMailbox.h"
#include "imapmime.h"

#include "DebugNewHelpers.h"


ImapDecodeUU::ImapDecodeUU()
{
	m_pATFile = NULL;
	Init();

	m_bMustWriteOutput = FALSE;
}

ImapDecodeUU::~ImapDecodeUU()
{
	if (m_pATFile)
		delete m_pATFile;

	m_pATFile = NULL;

	// Added by JOK:
	m_szEmbeddedFilename.Empty();
}

int ImapDecodeUU::Init()
{
	m_State = AbDone;

	if (m_pATFile)
		delete m_pATFile;

	m_pATFile = NULL;

	// Added by JOK:
	m_szEmbeddedFilename.Empty();

	return (0);
}

BOOL ImapDecodeUU::IsAbLine(const char* text, LONG size)
{
	const char* spot;

	if (size < 11 || CompareRString(IDS_UUENCODE_BANNER, text, 6))
		return (FALSE);
	spot = text + 7;
	while (*spot >= '0' && *spot <= '7')
		spot++;
	if (*spot != ' ' || spot - text > 10)
		return (FALSE);
	if (*++spot == '\r')
		return (FALSE);

	strncpy(m_Name, spot, sizeof(m_Name) - 1);
	m_Name[sizeof(m_Name) - 1] = 0;


	return (TRUE);
}

int ImapDecodeUU::Decode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	// Verify arguments:
	//
	if (! (In && Out) )
		return FALSE;

	// JOK - Don't return anything unless we decoded some data:
	//	
	OutLen = 0;

	// declaring this up here because the sun compiler has scoping
	// problems if this is declared in one case and scope ends in
	// another
	CRString label(IDS_DECODING_ATTACH);

	switch (m_State)
	{
		case AbDone:
			if (IsAbLine(In, InLen))
			{
				m_State = NotAb;
			}

			break;
		
		case NotAb:
			{
				// we just saw a line that looked like a begin line
				// if this line is the right length, we'll give it a go
				if ( UURightLength(In, InLen) < 0 )
				{
					m_State = AbDone;

					break;
				}

				// At the least, return m_Name in m_szEmbeddedFilename:
				//
				m_szEmbeddedFilename = m_Name;

				// If we will be doing the outputting to the attach file, 
				// create the attach file now:
				//
				if (m_bMustWriteOutput)
				{
					m_pATFile = CreateAttachFilefromEmbeddedName();

					if (!m_pATFile)
					{
						m_State = AbDone;
						break;
					}

					// Get the filename that was actually created and copy it to 
					// m_szEmbeddedFilename. The caller can ask for this.
					//
					BSTR bstrFilename = NULL;

					if (SUCCEEDED(m_pATFile->GetFName(&bstrFilename)))
					{
						// This is the full path:
						//
						m_szEmbeddedFilename = bstrFilename;
						::SysFreeString(bstrFilename);
					}
				}

				// OK. Translate and write until we're one:
				//
				m_State = AbWrite;
			}
			// Fall through to AbWrite
		
		case AbWrite:
			UULine(In, InLen, Out, OutLen);

			break;
	}
	
	if (m_State != AbDone)
	{
		return (TRUE);
	}

	// We get here only when we're done.
	// If we were writing to a file, cleanup the file stuff.
	//

	if (m_pATFile)
	{
		delete m_pATFile;
		m_pATFile = NULL;
	}

	return (FALSE);
}

inline int UU(char c) { return ((c - ' ') & 077); }
inline BOOL InvalidChar(char c) { return (c < ' ' || c > '`'); }
/************************************************************************
 * UURightLength - find out if the current line is of the proper length
 *  Returns length if so, negative number if mismatch
 ************************************************************************/
int ImapDecodeUU::UURightLength(const char* text, LONG size, BOOL Strict /*= TRUE*/)
{
	if (!size || *text == '\r' || *text == '\n')
		return (0);

	int length = UU(*text);

	if (InvalidChar(*text))
		return (-1);  // BZZZZT
	
	text++;
	size--;
	if (text[size - 1] == '\n')
		size--;
	if (text[size - 1] == '\r')
		size--;

	BOOL Matches = ((length + 2) / 3 == ((size * 3) / 4) / 3);
	if (!Matches)
	{
		if (Strict)
			return (-2);	// BZZZZT
	}

	return (length);	// hurray!
}

/************************************************************************
 * UULine - handle a line of uuencoded stuff
 ************************************************************************/
AbStates ImapDecodeUU::UULine(const char* text, LONG size, char *Out, LONG& Outlen)
{
	int length;

	// JOK - Verify argments:
	if (! (text && Out) )
		return (m_State = AbDone);

	// Initialize out parameters (JOK).
	*Out = 0;
	Outlen = 0;
	
	// check for end line
	if (size >= 3 && size <= 5 && !strnicmp("end\r\n", text, size))
		return (m_State = AbDone);
	
	// check length of line against line count
	if ((length = UURightLength(text, size, FALSE)) == -1)
	{
//		WarnUser(UU_BAD_LENGTH,(length+2)/3-((size*3)/4)/3);
		return (m_State = AbDone);
	}
	
	// empty lines mean nothing
	if (length == 0)
		return (m_State);
	
	// skip length byte, and trailing newline
	text++;
	size--;
	if (text[size - 1] == '\n')
		size--;
	if (text[size - 1] == '\r')
		size--;
	
	// hey!  we're ready to decode!
	for (; length > 0; text += 4, length -= 3)
	{
		if (InvalidChar(text[0]) || InvalidChar(text[1]) ||
			length > 1 && InvalidChar(text[2]) ||
			length > 2 && InvalidChar(text[3]))
		{
			break;
		}

		// JOK - Add decoded output to "Out".
		// If we must output to the file ourselves, do it now also.:
		//

		char cToOutput;

		cToOutput = ((char)(UU(text[0]) << 2 | UU(text[1]) >> 4));
		
		Out[Outlen++] = cToOutput;

		if (m_bMustWriteOutput && m_pATFile)
		{
			if (FAILED(m_pATFile->Put(cToOutput)))
				break;
		}

		if (length > 1)
		{
			cToOutput = ((char)(UU(text[1]) << 4 | UU(text[2]) >> 2));

			Out[Outlen++] = cToOutput;

			if (m_bMustWriteOutput && m_pATFile)
			{
				if (FAILED(m_pATFile->Put(cToOutput)))
					break;
			}
		}

		if (length > 2)
		{
			cToOutput = ((char)(UU(text[2])<<6 | UU(text[3])));

			Out[Outlen++] = cToOutput;

			if (m_bMustWriteOutput && m_pATFile)
			{
				if (FAILED(m_pATFile->Put(cToOutput)))
					break;
			}
		}
	}

	if (length <= 0)
		return (m_State);

	return (m_State = AbDone);
}



// CreateAttachFilefromEmbeddedName [PRIVATE]
//
// Use m_szMailboxDir and m_Name to get a unique filename in the "<szMaiboxDir>\Attach"
// directory and create the file.
//
JJFileMT* ImapDecodeUU::CreateAttachFilefromEmbeddedName ()
{
	// Must have a mailbox directory:
	//
	if (m_szMailboxDir.IsEmpty())
		return NULL;

	// Need to do this for "OpenLocalAttachFile()!!!"
	//
	TCHAR szBuf [2048];

	if (m_szMailboxDir.Right ( 1 ) != DirectoryDelimiter)
	{
		wsprintf (szBuf, "%s%c", m_szMailboxDir, DirectoryDelimiter);
	}
	else
	{
		strcpy (szBuf, (LPCSTR)m_szMailboxDir);
	}

	return OpenLocalAttachFile(szBuf, m_Name, false);
}

