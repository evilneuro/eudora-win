/*
 *  Functions to aide manipulation of MIME RFC822 headers.
 *
 *  Filename: RFC822.h
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
 *
 */

#ifndef EMS_RFC822_H
#define EMS_RFC822_H

#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

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
char *rfc822_extract_token(char **cpp);

/*
 *  Advances to next non-whitespace character in string. This includes
 *  ignoring RFC822 comments.
 *   
 *  Args:
 *   cp [IN] RFC822 string
 *
 *  Returns: Pointer to next non-whitespace character.
 */
char *rfc822_skipws(char *cp);

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
char *rfc822_skipword(char *cp);

/*
 *  Calculates the length of the given text string if it were converted
 *  to an RFC822 string.
 *
 *  Args:
 *   cp [IN] Text string
 *
 *  Returns: Equivalent RFC822 length of given text.
 */
unsigned int rfc822_quoted_strlen(char *cp);

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
char *rfc822_quote_strcpy(char *dst, const char *src);

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
char *rfc822_unquote_strcpy(char *dst, const char *src, unsigned int len);

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
char *rfc822_extract_header(const char *pFullHeader, const char *pLinePrefix);

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
char *rfc822_read_header(ifstream& inFile);

#endif /* EMS_RFC822_H */
