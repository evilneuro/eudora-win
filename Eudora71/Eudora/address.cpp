// ADDRESS.CPP
//
// Utility routines for gleaning information from email address headers
// 
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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

#include <ctype.h>

#include <QCUtils.h>

#include "address.h"
#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "doc.h"
#include "nickdoc.h"
#include "lex822.h"
#include "guiutils.h"
#include "persona.h"

#include "DebugNewHelpers.h"


////////////////////////////////////////////////////////////////////////
// FindAddressEnd [extern]
//
// Returns a pointer to the character which terminates the address.
// This can be a comma, carriage return, newline, or NULL character.
// To support group syntax, we treat pairs of matched colon/semi-colon
// pairs as delimeters where commas are ignored.
////////////////////////////////////////////////////////////////////////
const char* FindAddressEnd(const char* AddressStart, BOOL bDisplayError /*=TRUE*/, BOOL bGroupSyntax /*=FALSE*/)
{
	const char* a = AddressStart;
	char Last = 0;
	int Quote = 0;
	int Paren = 0;
	int GroupSyntax = 0;

	//
	// Scan for the first null terminator, comma, carriage return, or
	// newline chacter that is not in a quoted string or in a
	// parenthesized comment string or in a group syntax address.
	//
	for (; *a && ((*a != ',' && *a != '\r' && *a != '\n') || Quote || Paren || (GroupSyntax && bGroupSyntax)); a++)
	{
		if (Last == '\\')
		{
			if (*a == '\\')
			{
				Last = 0;
				continue;
			}
		}
		else
		{
			if (!Quote && *a == '(')
				Paren++;
			else if (!Quote && *a == ')')
			{
				Paren--;
				if (Paren < 0)
				{
					a++;
					break;
				}
			}
			else if (!Paren && *a == '"')
				Quote = !Quote;
			else if (!Quote && !Paren && *a == ':' && a[1] != ':')
				GroupSyntax++;
			else if (!Quote && !Paren && *a == ';')
			{
				GroupSyntax--;
				if (GroupSyntax < 0)
				{
					a++;
					break;
				}
			}
		}
		Last = *a;
	}

	//
	// If we hit a terminating character while were processing a quoted,
	// parenthesized, or group syntax section, then optionally display
	// an error message.
	//
	if (Quote || Paren || (GroupSyntax && bGroupSyntax))
	{
		if (bDisplayError)
			ErrorDialog(IDS_ERR_ADDRESS_BAD_PARENS, AddressStart);

		return NULL;
	}
	
	return (a);
}


// GetRealName
// Find the human-readable part of an address
//
char* GetRealName(char* FromLine)
{
	if (!FromLine)
    	return (NULL);

	// Remove redirect comments
	char* Redir = strstr(FromLine, CRString(IDS_BY_WAY_OF));
	if (Redir)
		*Redir = 0;

	::TrimWhitespaceMT(FromLine);

	char* start = FromLine;
	char* end = FromLine + strlen(FromLine) - 1;
	int InAngle = 0, InParen = 0, InQuote = 0;
	char Last = 0;
	for (char* f = FromLine; *f; Last = *f++)
	{
		if (Last == '\\')
		{
			if (*f == '\\')
			{
				Last = 0;
				continue;
			}
		}
		else
		{
			if (*f == '<' && !InParen && !InQuote)
			{
				if (f != FromLine)
					end = f - 1;
				InAngle = TRUE;
			}
			else if (InAngle && *f == '>')
			{
				InAngle = FALSE;
				if (f == end)
				{
					// The entire line is <>ed
					start++;
					end--;
				}
				else if (end < f)
				{
					// There's some text before and maybe after the <>ed part
					strcpy(end + 1, f + 1);
					end = end + strlen(end) - 1;
				}
				else
				{
					// There's some text only after the <>ed part
					start = f + 1;
				}
				break;
			}
			else if (*f == '(' && !InAngle && !InQuote)
			{
				if (InParen++ == 0)
					start = f + 1;
			}
			else if (*f == ')' && InParen)
			{
				if (--InParen == 0)
				{
					end = f - 1;
					break;
				}
			}
			else if (*f == '"' && !InAngle && !InParen)
				InQuote = !InQuote;
		}
			
		Last = *f;
	}

	strncpy(FromLine, start, end - start + 1);
	FromLine[end - start + 1] = 0;

	// Remove quotes and parentheses from real names
	::TrimWhitespaceMT(FromLine);
	if (*FromLine == '"' || *FromLine == '(')
	{
		strcpy(FromLine, FromLine + 1);
		char* End = FromLine + strlen(FromLine) - 1;
		if (*End == '"' || *End == ')')
			*End = 0;
		::TrimWhitespaceMT(FromLine);
	}

	return (FromLine);
}

// StripAddress
// Gets only the pertinent parts from an address
//
char* StripAddress(char* line)
{
	return ::StripAddressMT(line);
}

// AddressIsMe
// Tells whether or not an address is "me" 
//
BOOL AddressIsMe(const char* Address, const char* Me)
{
	if (!Address || !Me)
		return (FALSE);

	BOOL Success = FALSE;
	const char* Start = Me;
	const char* End;
	char Addr[256];
	int Length;

	while (*Start)
	{
		if (!(End = FindAddressEnd(Start)))
			break;
		Length = End - Start;
		strncpy(Addr, Start, Length);
		Addr[Length] = 0;
		StripAddress(Addr);
		if (!stricmp(Address, Addr))
		{
			Success = TRUE;
			break;
		}
		Start = End;
		while (*Start == ',' || *Start == '\r' || *Start == '\n')
			Start++;
	}
	
	return (Success);
}

// StripMe
// From a list of addresses, removes all addresses deemed as being "me".
// If StripAll is FALSE, the first instance of "me" is left in the list.
//
// Retuns TRUE if one or more instances of "me" was found, FALSE if none found.
//
BOOL StripMe(char* line, BOOL StripAll /*= TRUE*/)
{
	if (!line || !*line)
		return FALSE;

	char AdrBuf[1024];
	BOOL bAddedOneAddres = FALSE;
	BOOL bFoundOne = FALSE;
	char* LineStart = line;
	const char *AdrStart = line;
	CNickname* MeAlias = NULL;

	CString csMe = CRString(IDS_ME_NICKNAME);
	CString Persona = g_Personalities.GetCurrent();
	if ( ! Persona.IsEmpty() )
	{
		csMe += "-";
		csMe += Persona;
	}

	ASSERT(g_Nicknames);
	JJFile in;
	MeAlias = g_Nicknames->ReadNickname( csMe, in );
	in.Close();

	while (1)
	{
		// Find the end of the next address in the list
		const char* AdrEnd = FindAddressEnd(AdrStart);
		if (!AdrEnd)
		{
			break;
		}

		BOOL bIsMe = FALSE;
		int len = AdrEnd - AdrStart;

		// Only check to see if address is "me" if it's small enough
		// to fit in our buffer and strip.
		if (len < 1024)
		{
			strncpy(AdrBuf, AdrStart, len);
			AdrBuf[len] = 0;
			StripAddress(AdrBuf);
			if (AddressIsMe(AdrBuf, GetIniString(IDS_INI_RETURN_ADDRESS)) ||
				AddressIsMe(AdrBuf, GetIniString(IDS_INI_POP_ACCOUNT)) ||
				(MeAlias && AddressIsMe(AdrBuf, MeAlias->GetAddresses())))
			{
				bIsMe = TRUE;
			}
		}

		// Only include this address in the list if it's not "me"
		// or it's the first instance of "me" when we're told
		// to only include the first instance of "me"
		if (!bIsMe || (!StripAll && !bFoundOne))
		{
			if (bAddedOneAddres)
				*line++ = ',';
			for (; len; len--)
				*line++ = *AdrStart++;
			bAddedOneAddres = TRUE;
		}

		if (bIsMe)
			bFoundOne = TRUE;

		if (!*AdrEnd)
			break;
		AdrStart = AdrEnd + 1;
		while (*AdrStart == ',' || *AdrStart == '\r' || *AdrStart == '\n')
			AdrStart++;
	}
	*line = 0;

	::TrimWhitespaceMT(LineStart);

	return bFoundOne;
}

// QualifyAddresses
//
// Qualifies addresses without domains if the switch is on.
// WARNING: This function may delete the memory associated with Addresses
//          and return a pointer to a newly created buffer, so only use
//          dynamically allocated memory for Addresses.
//
char* QualifyAddresses(char* Addresses)
{
	if (!Addresses || !*Addresses)
		return (Addresses);

	char Domain[128];
	GetIniString(IDS_INI_DOMAIN_QUALIFIER, Domain, sizeof(Domain));

	// Only do this if there's something to qualify with
	if (!*Domain)
		return (Addresses);

	// Need to have a leading @
	if (*Domain != '@')
	{
		memmove(Domain + 1, Domain, strlen(Domain) + 1);
		*Domain = '@';
	}

	int DomainLen = strlen(Domain);
	char* a = Addresses;
	int Quote = 0;
	int Paren = 0;
	
	while (*a)
	{
		// Find where the real address part starts
		char* AddressStart = NULL;
		char Last = 0;
		for (; *a && (*a != ',' || Quote || Paren); a++)
		{
			if (Last == '\\')
			{
				if (*a == '\\')
				{
					Last = 0;
					continue;
				}
			}
			else
			{
				if (!Quote && *a == '(')
					Paren++;
				else if (!Quote && *a == ')')
					Paren--;
				else if (!Paren && *a == '"')
					Quote = !Quote;
				else if (!Quote && !Paren &&
					(*a == '<' || (!AddressStart && *a != ' ')))
				{
					AddressStart = a;
				}
			}
			Last = *a;
		}

		if (AddressStart)
		{
			const char* NonBlank = AddressStart;
			while (isspace((int)(unsigned char)*NonBlank))
				NonBlank++;
			if ((unsigned char)*NonBlank != 131)	// 131 for FCC
			{
				// Find either the end of the address, or the @ character
				char* AddressEnd = AddressStart;
				while (AddressEnd < a && *AddressEnd != '@' &&
					*AddressEnd != ' ' && *AddressEnd != '>' && *AddressEnd != '(' &&
					*AddressEnd != ';' && *AddressEnd != ':')
				{
					AddressEnd++;
				}

				// Hack solution here.
				// If we're pointing at a colon, we're looking at group syntax.
				// Pretend we never got into this address by going back up to the
				// top of the loop and looking for the start of the first included address
				// SD 5/5/99
				if (AddressEnd < a && *AddressEnd==':')
				{
					a = AddressEnd+1;	// start over past the colon
					continue;
				}

				// If we didn't find the @ character, then it needs to be qualified
				if (AddressEnd > AddressStart && (AddressEnd >= a || *AddressEnd != '@'))
				{
					char* NewBuf = DEBUG_NEW_NOTHROW char[strlen(Addresses) + DomainLen + 1];
					if (NewBuf)
					{
						int InitialLen = AddressEnd - Addresses;
						strncpy(NewBuf, Addresses, InitialLen);
						strcpy(NewBuf + InitialLen, Domain);
						strcpy(NewBuf + InitialLen + DomainLen, Addresses + InitialLen);
						a = NewBuf + (a - Addresses) + DomainLen;
						delete [] Addresses;
						Addresses = NewBuf;
					}
				}
			}
		}

		if (*a == ',')
			a++;
	}

	return (Addresses);
}


CString ReturnAddress;

// GetReturnAddress
//
// Forms a return address out of the return address field (or the pop account
// if the return address field is empty) and the realname.
//
const char* GetReturnAddress()
{
	const char* Address = GetIniString(IDS_INI_RETURN_ADDRESS);
	while (Address && isspace((int)(unsigned char)*Address))
		Address++;
	if (!Address || !*Address)
    {
		Address = GetIniString(IDS_INI_POP_ACCOUNT);
		if (!Address)
        	return (NULL);
		while (isspace((int)(unsigned char)*Address))
			Address++;
    }
    
	char RealName[128];
	GetIniString(IDS_INI_REAL_NAME, RealName, sizeof(RealName));
	::TrimWhitespaceMT(RealName);
	if (*RealName)
		FormatAddress(ReturnAddress, RealName, Address);
    else
		ReturnAddress = Address;
	

    return (ReturnAddress);
}

// GetPersRealName
//
// Gets the realname for the current personality falling back on the
// return address or the address if necessary.
//
void GetPersRealName(CString & RealName)
{
	//	Get and trim the real name for the current personality
	GetIniString(IDS_INI_REAL_NAME, RealName);
	::TrimWhitespaceMT(RealName);

	if ( RealName.IsEmpty() )
	{
		//	There is no realname - get and trim the return address instead
		GetIniString(IDS_INI_RETURN_ADDRESS, RealName);
		::TrimWhitespaceMT(RealName);

		if ( RealName.IsEmpty() )
		{
			//	There is no return address - get and trim the address
			GetIniString(IDS_INI_POP_ACCOUNT);
			::TrimWhitespaceMT(RealName);
		}
	}
}

const char* FormatAddress(CString& Result, const char* RealName, const char* Address)
{
	char QRealName[128];
	const char* Format = GetIniString(IDS_INI_RETURN_ADDRESS_FORMAT);
	LPCSTR rglpsz[2];

	rglpsz[0] = RealName;
	rglpsz[1] = Address;
	
	// If we're using the "%1 <%2>" addressing format, then some specials need to be quoted
	if (strchr(Format, '<'))
	{
		Quote822(QRealName, RealName);
		rglpsz[0] = QRealName;
	}
	
	AfxFormatStrings(Result, Format, rglpsz, 2);

	return (Result);
}
