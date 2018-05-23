/*
 *  Functions to encode and decode using BASE64 or Quoted-Printable
 *  Also a transfer-encoding header line parser
 *
 *  Filename: encoding.h
 *
 *  Last Edited: September 24, 1996
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

#ifndef EMS_ENCODING_H
#define EMS_ENCODING_H

/*
 * state buffer for base64 encoding. Initialize all values to zero.
 */
typedef struct {
	unsigned char partial[4];
	short partialCount;
	short bytesOnLine;
} Enc64, *Enc64Ptr;

/*
 * state buffer for base64 decoding. Initialize all values to zero.
 */
typedef struct {
	short decoderState;		/* which of 4 bytes are we seeing now? */
	long invalCount;		/* how many bad chars found so far? */
	long padCount;			/* how many pad chars found so far? */
	unsigned char partial;	/* partially decoded byte from/for last/next time */
	short wasCR;			/* was the last character a carriage return? */
} Dec64, *Dec64Ptr;

/*
 * used internally to track parsing state
 */
typedef enum {
	qpNormal,
	qpEqual,
	qpEncoded,
	qpTrailingWhitespace,
	qpError
} QPStates;

/*
 * state buffer for QP encoding. Initialize all values to zero.
 */
typedef struct {
	int nCurLineLen; /* Number of characters outputed to current line */
	char cLastChar;  /* Last character read in */
} EncQP, *EncQPPtr;

/*
 * state buffer for QP decoding. Initialize CurState to qpNormal.
 */
typedef struct {
	QPStates CurState;
	char cLastChar;
} DecQP, *DecQPPtr;

/*
 * Valid return values for rfc822_parse_cte which specify the trasnfer
 * encoding found in the parsed header line
 */
typedef enum {
	CTE_Error,
	CTE_Base64,
	CTE_QP,
	CTE_7bit,
	CTE_8bit,
	CTE_Binary,
	CTE_NONE
} TrEncType;

/*****************************************************************************/
/*                                B A S E 6 4                                */
/*****************************************************************************/

/*
 *  Convert binary data to base64
 *
 *  Args:
 *   binPtr     [IN]     the binary data (or NULL to close encoder)
 *   binLen     [IN]     the length of the binary data
 *   sixFourPtr [IN]     pointer to buffer for the base64 data
 *   e64        [IN/OUT] state; caller must preserve
 *
 *  Returns: The length of the base64 data
 */
long Encode64(char *bin, long len, char *sixFour, Enc64Ptr e64);

/*
 *  Convert base64 data to binary
 *
 *  Args:
 *   sixFourPtr [IN]     the base64 data (or nil to close decoder)
 *   sixFourLen [IN]     the length of the base64 data
 *   binPtr     [IN]     pointer to buffer to hold binary data
 *   d64        [IN/OUT] pointer to decoder state; caller must preserve
 *   decErrCnt  [OUT]    the number of decoding errors found
 *
 *  Returns: The length of the binary data
 */
long Decode64(char *sixFour, long sixFourLen, char *bin, Dec64Ptr d64, long *decErrCnt);

/*****************************************************************************/
/*                      Q U O T E D - P R I N T A B L E                      */
/*****************************************************************************/

/* ------------------------------------------------------------------------- */
/* NOTE: To handle BINARY data, you must always quote newline characters,    */
/*   this implementation assumes TEXT data and thus does not encode newlines */
/* ------------------------------------------------------------------------- */

/*
 *  Convert binary data to quoted-printable
 *
 *  Args:
 *   pBin     [IN]     the binary data (or NULL to close the encoder)
 *   nLen     [IN]     the length of the binary data (or 0 to close the encoder)
 *   pQP      [IN]     pointer to buffer for the quoted-printable data
 *   eQPstate [IN/OUT] state; caller must preserve
 *
 *  Returns: The length of the quoted-printable data
 */
long EncodeQP(char *bin, long len, char *qp, EncQPPtr eQP);

/*
 *  Convert quoted printable data to binary
 *
 *  Args:
 *   pQP       [IN]     the quoted printable data (or NULL to close the decoder)
 *   nLen      [IN]     the length of the quoted printable data (or 0 to close the decoder)
 *   pBin      [IN]     pointer to buffer to hold binary data
 *   dQPstate  [IN/OUT] pointer to decoder state; caller must preserve
 *   decErrCnt [OUT]    the number of decoding errors found
 *
 *  Returns: The length of the binary data
 */
long DecodeQP(char *qp, long qpLen, char *bin, DecQPPtr dqp, long *decErrCnt);

/*****************************************************************************/
/*                             CTE   P A R S E R                             */
/*****************************************************************************/

/*
 *  Parse Content-Transer-Encoding header line
 *
 *  Args:
 *   src [IN] Valid Transfer-Encoding header.
 *
 *  Returns: enumerated integer 'TrEncType' type specifying CTE (see above).
 */
TrEncType rfc822_parse_cte(const char *src);

/*
 *  Create Content-Transer-Encoding header line
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   mechanism [IN] Enumerated integer 'TrEncType' type specifying CTE.
 *
 *  Returns: Content tranfer encoding header line string.
 */
char *rfc822_make_cte(TrEncType mechanism);

/*
 *  Finds and extracts the content transfer encoding header line from a full
 *  multi-lined header. All unfolding (removing newlines) is done before
 *  header line is returned.
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   pFullHeader [IN] Pointer to a full RFC822 header, including newlines
 *
 *  Returns: Extracted header line string; dynamically allocated.
 */
char *rfc822_extract_cte(const char *pFullHeader);

#endif /* EMS_ENCODING_H */
