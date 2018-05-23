/*
 *  Functions to aide manipulation of MIME RFC822 headers.
 *
 *  Filename: RFC822.cpp
 *
 *  Last Edited: July 15, 1996
 *
 *  Author: Scott Manjourides
 *
 *  Portions adopted from code originally written by Stever Dorner.
 *  Copyright 1995, 1996 QUALCOMM Inc.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


 *
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

#include "rfc822.h"

/* ========================================================================= */

const char *kRFC822_MustBeQuoted = "()<>@,;:\\\"/[]?= ";
const char *kRFC822_MustBeEscaped = "\\\"";
const char *kRFC822_EOL = "\r\n";

/* ========================================================================= */

#define safefree(p) { if (p) { free(p); (p) = NULL; } }

/* ========================================================================= */

/*
 *  Find next RFC822 token in given string, copying it into a newly created
 *  string. Advance pointer past token and any following whitespace.
 *   
 *  NOTE: The user of this function is responsible for freeing returned
 *        string.
 *
 *  Args:
 *   cpp [IN/OUT] Handle (pointer-to-pointer) of RFC822 string to extract from
 *
 *  Returns: String containing next token, NULL if error.
 *         Moves cpp to first non-whitespace character AFTER extracted token
 */
char *rfc822_extract_token(char **cpp)
{
	char *start, *cp = *cpp;

	cp = rfc822_skipws(cp); // Skip white space
	start = cp;

	cp = rfc822_skipword(cp); // Skip word

	if (cp > start)
	{
		const int len = cp - start;
		char *buf = (char *) malloc(len + 1);

		if (buf)
		{
			rfc822_unquote_strcpy(buf, start, len);
			*cpp = rfc822_skipws(cp);

			return (buf);
		}
	}

	return (NULL); // No token to extract
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Advances to next non-whitespace character in string. This includes
 *  ignoring RFC822 comments.
 *   
 *  Args:
 *   cp [IN] RFC822 string
 *
 *  Returns: Pointer to next non-whitespace character.
 */
char *rfc822_skipws(char *cp)
{
	do {
		while (*cp == ' ') // Skip spaces
			cp++;

		if (*cp == '(') // A comment?
		{
			int nested = 1;

			while ((*++cp) && (nested)) // Find end of comment
			{
				switch (*cp)
				{
					case '(': nested++;
					break;

					case ')': nested--;
					break;

					case '\\': // Escape character
						if ((*(cp+1)) != '\0') // Check for end-of-string
							cp++;
					break;

					case '\"': // Quote inside comment
						while ((*cp) && (*++cp != '\"')) // Go 'til end of quote
							if ((*cp == '\\') && ((*(cp+1)) != '\0'))
								cp++; // Escaped character inside quote -- inside comment (eeek!)
					break;
				}
			}
		}
	} while (*cp == ' ');

	return cp;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Advances to next character in string directly after the current token.
 *  The first character must be the beginning of a valid token or end of
 *  string (ie. all whitespace must be skipped BEFORE calling this function).
 *
 *  Args:
 *   cp [IN] RFC822 string
 *
 *  Returns: Pointer to next valid whitespace character.
 */
char *rfc822_skipword(char *cp)
{
	int bInQuotes = 0;

	// First character must be a quote to be valid double-quoted string
	if (*cp == '\"')
	{
		bInQuotes = 1;
		cp++;
	}
	
	while (*cp)
	{
		if (strchr(kRFC822_MustBeQuoted, *cp))
		{
			if (!bInQuotes) // Found a character that should be double-quoted, but is not
				return (cp);

			// Now we know we are inside a double-quoted string

			switch (*cp)
			{
				case '\"': return (cp+1); // End double-quote
					break;
				case '\\': cp++; // Escape character, skip next
					break;
				default: // nothing
					break;
			}
		}

		cp++;
	}

	return (cp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Calculates the length of the given text string if it were converted
 *  to an RFC822 string.
 *
 *  Args:
 *   cp [IN] Text string
 *
 *  Returns: Equivalent RFC822 length of given text.
 */
unsigned int rfc822_quoted_strlen(char *cp)
{
	unsigned int len = 0, bQuoted = 0;

	while (*cp)
	{
		len++;

		if ((!bQuoted) && (strchr(kRFC822_MustBeQuoted, *cp) != NULL))
			bQuoted = 1;

		if (strchr(kRFC822_MustBeEscaped, *cp) != NULL)
			len++, bQuoted = 1;

		cp++;
	}

	if (bQuoted)
		len += 2; // Quotes

	return (len);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Copies and converts the source text string to a destination
 *  RFC822 string. The source must be NULL terminated, and the
 *  destination will be NULL terminated.
 *
 *  Args:
 *   dst [OUT] RFC822 string
 *   src [IN]  Text string
 *
 *  Returns: Pointer to destination NULL termination.
 */
char *rfc822_quote_strcpy(char *dst, const char *src)
{
	const unsigned int bQuoted = ( strpbrk(src, kRFC822_MustBeQuoted) ? 1 : 0 );

	if (bQuoted)
		*dst++ = '\"';

	while (*src)
	{
		if (strchr(kRFC822_MustBeEscaped, *src) != NULL)
			*dst++ = '\\';

		*dst++ = *src++;
	}

	if (bQuoted)
		*dst++ = '\"';

	*dst = '\0';

	return (dst);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Copies and converts the source RFC822 string to a destination
 *  text string. The source must be NULL terminated, and the
 *  destination will be NULL terminated.
 *
 *  Args:
 *   dst [OUT] Text string
 *   src [IN]  RFC822 string
 *   len [IN]  Maximum charcters to copy; Zero implies whole string
 *
 *  Returns: Pointer to destination NULL termination.
 */
char *rfc822_unquote_strcpy(char *dst, const char *src, unsigned int len)
{
	// If 'len' is provided (ie. >0) then only len chars will be copied
	// Otherwise, copy 'til end-of-string
	char *end;
	unsigned int bEscaped = 0;

	end = strchr(src, '\0');

	if ((len > 0) && ((src + len) < end))
		end = (char *) src + len;

	while (src < end)
	{
		if (bEscaped)
			bEscaped = 0;
		else
		{
			switch (*src)
			{
				case '\\':	bEscaped = 1;
				case '\"':	src++;
							continue;
				break;
			}
		}

		*dst++ = *src++;
	}

	*dst = '\0';

	return dst;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Finds and extracts a header line from a full multi-lined header. All
 *  unfolding (removing newlines) is done before header line is returned.
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   pFullHeader [IN] Pointer to a full RFC822 header, including newlines
 *   pLinePrefix [IN] Prefix of header line to extract
 *
 *  Returns: Extracted header line string; dynamically allocated.
 */
char *rfc822_extract_header(const char *pFullHeader, const char *pLinePrefix)
{
	const char *kNewline = "\r\n";
	const unsigned int kNewlineLen = (2);
	const unsigned int nPreLen = strlen(pLinePrefix);

	char *pStart, *pEnd, *pBuf, *pPos, *pRetBuf;

	if (nPreLen < 1)
		return (NULL);

	pStart = (char *) pFullHeader;

	/* Find first 'line' which matches prefix */
	while ((pStart) && (strnicmp(pStart, pLinePrefix, nPreLen) != 0))
		if (pStart = strstr(pStart, kNewline))
			pStart += kNewlineLen;

	if (!pStart)
		return (NULL); /* Not found */

	/* Find the end of this header line */
	for (pEnd = strstr(pStart, kNewline); (pEnd != NULL); pEnd = strstr(pEnd, kNewline))
	{
		pPos = pEnd + kNewlineLen;

		/* Does header line continue on next line? */
		if ((*pPos == ' ') || (*pPos == '\t'))
			pEnd = pPos;
		else
			break;
	}

	/* If we ran off the end of the string, then the end is the last char in the string */
	if (pEnd == NULL)
		pEnd = strchr(pStart, '\0');
	
	pBuf = pRetBuf = (char *) malloc(pEnd - pStart + 1); /* Max length of output string */

	for (pPos = pStart; pPos < pEnd; pPos++)
	{
		if (strncmp(pPos, kNewline, kNewlineLen) == 0) /* We're at a newline */
			pPos += kNewlineLen; /* Skip over newline */

		*pBuf++ = *pPos;
	}

	*pBuf = '\0';

	return (pRetBuf);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Reads and returns the RFC822 header from input file. This is defined
 *  here as reading until the first blank line from current file
 *  position. The blank line is discarded.
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   inFile [IN] Input file stream; already open
 *
 *  Returns: Complete multi-lined RFC822 header; dynamically allocated.
 *     The file pointer is left immediately after the first read blank line.
 */
char *rfc822_read_header(ifstream& inFile)
{
	// We don't know how big the header is, but we know each line
	// must be less than 1024 chars. We use 'kBlockSize' to
	// increase the memory of the header while ensuring that
	// there is at least kBlockSize available for each read.
	const unsigned int kBlockSize = 1024;

	char *pBuf, *pPos, *cp;
	unsigned int nBufSize, nStrLen;

	if ((!inFile.good()) || (inFile.eof()))
		return NULL;

	nStrLen = nBufSize = 0;
	pBuf = NULL;

	while ((inFile.good()) && (!inFile.eof()))
	{
		if ((nBufSize - nStrLen) < (kBlockSize + 1))
		{
			nBufSize += kBlockSize;
			if ((cp = (char *) realloc(pBuf, nBufSize)) == NULL)
			{
				safefree(pBuf);
				return (NULL); // Out of memory?
			}

			pPos = (pBuf = cp) + nStrLen;
		}

		// Get next line
		cp = pPos;
		while ((inFile.good()) && (!inFile.eof()))
			if ((*cp++ = inFile.get()) == '\n')
			{
				*cp = '\0';
				break;
			}

		/* inFile.getline(pPos, kBlockSize); */
		/* getline() strips '\n' chars! Yuck! */

		if (strcmp(pPos, kRFC822_EOL) == 0) /* Empty line */
			break;

		pPos = strchr(pPos, '\0');
		nStrLen = pPos - pBuf;
	}

	// Null terminate the header string
	*pPos = '\0';

	// Chop off any excess allocated memory
	if (cp = (char *) realloc(pBuf, strlen(pBuf) + 1))
		pBuf = cp;
	
	return (pBuf);
}
