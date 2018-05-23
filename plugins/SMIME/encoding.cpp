/*
 *  Functions to encode and decode using BASE64 or Quoted-Printable
 *  Also a transfer-encoding header line parser
 *
 *  Filename: encoding.cpp
 *
 *  Last Edited: Friday, August 30, 1996
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

#include "stdafx.h"

#include <windows.h>
#include "ems-win.h"

#include "encoding.h"
#include "rfc822.h"    
#include "string.h" 
#include "ctype.h"   
#include "stdlib.h"

/* ========================================================================= */

#define safefree(p) { if (p) { free(p); (p) = NULL; } }

/* ========================================================================= */

/* Local util functions */
static char *newline_copy(char *dst);
static int newline_test(const char prev, const char curr);
static int hex2dec(const char ch);

/*****************************************************************************/
/*                                B A S E 6 4                                */
/*****************************************************************************/

/* Base64 encoder/decoder ported from Macintosh source by Myra Callen */

#define SKIP (-1)
#define FAIL (-2)
#define PAD  (-3)

#define kNewLine			"\015\012"
#define kNewLineLength (2)

#define ABS(x)   ((x)<0 ? -(x) : (x))

static char *g64EncodeChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static short g64DecodeArr[] = 
{
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,SKIP,SKIP,FAIL,FAIL,SKIP,FAIL,FAIL,	/* 0 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 1 */
	SKIP,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,0x3e,FAIL,FAIL,FAIL,0x3f,	/* 2 */
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,FAIL,FAIL,FAIL,PAD ,FAIL,FAIL,	/* 3 */
	FAIL,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,	/* 4 */
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 5 */
	FAIL,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,	/* 6 */
	0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 7 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 8 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 9 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* A */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* B */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* C */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* D */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* E */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* F */
  /* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F  */
};

/*
 * Bit extracting macros
 */
#define Bot2(b) ((b)&0x3)
#define Bot4(b) ((b)&0xf)
#define Bot6(b) ((b)&0x3f)
#define Top2(b) Bot2((b)>>6)
#define Top4(b) Bot4((b)>>4)
#define Top6(b) Bot6((b)>>2)

/*
 * the decoder
 */
#define EncodeThree64(bin,b64,bpl)	EncodeThreeFour(bin,b64,bpl,g64EncodeChars)

#define EncodeThreeFour(bin,b64,bpl,vector)                     \
	do                                                          \
	{                                                           \
		if ((bpl)==68)                                          \
		{                                                       \
			(b64) = newline_copy(b64);                          \
			(bpl) = 0;                                          \
		}                                                       \
		(bpl) += 4;                                             \
		*(b64)++ = vector[Top6((bin)[0])];                      \
		*(b64)++ = vector[Bot2((bin)[0])<<4 | Top4((bin)[1])];  \
		*(b64)++ = vector[Bot4((bin)[1])<<2 | Top2((bin)[2])];  \
		*(b64)++ = vector[Bot6((bin)[2])];                      \
	}                                                           \
	while (0)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
long Encode64(
	char *binPtr,
	long binLen,
	char *sixFourPtr,
	Enc64Ptr e64)
{
	char *binSpot;									/* the byte currently being decoded */
	char *sixFourSpot = sixFourPtr;	/* the spot to which to copy the encoded chars */
	short bpl;
	char *end;											/* end of integral decoding */

	bpl = e64->bytesOnLine;	/* in inner loop; want local copy */
	
	if (binLen)
	{
		
		/*
		 * do we have any stuff left from last time?
		 */
		if (e64->partialCount)
		{
			short needMore = 3 - e64->partialCount;
			if (binLen >= needMore)
			{
				/*
				 * we can encode some bytes
				 */
				memcpy(e64->partial+e64->partialCount,binPtr,needMore);
				binLen -= needMore;
				binPtr += needMore;
				EncodeThree64(e64->partial,sixFourSpot,bpl);
				e64->partialCount = 0;
			}
			/*
			 * if we don't have enough bytes to complete the leftovers, we
			 * obviously don't have 3 bytes.  So the encoding code will fall
			 * through to the point where we copy the leftovers to the partial
			 * buffer.  As long as we're careful to append and not copy blindly,
			 * we'll be fine.
			 */
		}
		
		/*
		 * we encode the integral multiples of three
		 */
		end = binPtr + 3*(binLen/3);
		for (binSpot = binPtr; binSpot < end; binSpot += 3)
		{
			EncodeThree64(binSpot,sixFourSpot,bpl);
			/* *sixFourLen = sixFourSpot - sixFourPtr; */
		}
		
		/*
		 * now, copy the leftovers to the partial buffer
		 */
		binLen = binLen % 3;
		if (binLen)
		{
			memcpy(e64->partial+e64->partialCount,binSpot,binLen);
			e64->partialCount += (short)binLen;
		}
	}
	else
	{
		/*
		 * we've been called to cleanup the leftovers
		 */
		if (e64->partialCount)
		{
			if (e64->partialCount<2) e64->partial[1] = 0;
			e64->partial[2] = 0;
			EncodeThree64(e64->partial,sixFourSpot,bpl);
			
			/*
			 * now, replace the unneeded bytes with ='s
			 */
			sixFourSpot[-1] = '=';
			if (e64->partialCount==1) sixFourSpot[-2] = '=';
		}
	}

	e64->bytesOnLine = bpl;	/* copy back to state buffer */

	return (sixFourSpot - sixFourPtr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
long Decode64(
	char *sixFourPtr,
	long sixFourLen,
	char *binPtr,
	Dec64Ptr d64,
	long *decErrCnt)
{
	short 	decode;													/* the decoded short */
	unsigned char 		c;															/* the decoded byte */
	/* we separate the short & the byte to the compiler can worry about byteorder */
	char *end = sixFourPtr + sixFourLen;	/* stop decoding here */
	char *binSpot = binPtr;								/* current output character */
	short 	decoderState;		/* which of 4 bytes are we seeing now? */
	long 		invalCount;			/* how many bad chars found this time around? */
	long 		padCount;				/* how many pad chars found so far? */
	unsigned char 		partial;				/* partially decoded byte from/for last/next time */
	short wasCR;
	char *sixFourStartPtr;
	
	/*
	 * fetch state from caller's buffer
	 */
	decoderState = d64->decoderState;
	invalCount = 0;	/* we'll add the invalCount to the buffer later */
	padCount = d64->padCount;
	partial = d64->partial;
	wasCR = d64->wasCR;
	
	if (sixFourLen)
	{
		sixFourStartPtr = sixFourPtr;
		for (;sixFourPtr<end;sixFourPtr++)
		{
			switch(decode=g64DecodeArr[*sixFourPtr])
			{
				case SKIP: break;									/* skip whitespace */
				case FAIL: invalCount++; break;		/* count invalid characters */
				case PAD: padCount++; break;			/* count pad characters */
				default:
					/*
					 * found a non-pad character, so if we had previously found a pad,
					 * that pad was an error
				 	*/
					if (padCount) {invalCount+=padCount;padCount=0;}
					
					/*
					 * extract the right bits
					 */
					c = (char) decode;
					switch (decoderState)
					{
						case 0:
							partial = c<<2;
							decoderState++;
							break;
						case 1:
							*binSpot++ = partial|Top4(c);
							partial = Bot4(c)<<4;
							decoderState++;
							break;
						case 2:
							*binSpot++ = partial|Top6(c);
							partial = Bot2(c)<<6;
							decoderState++;
							break;
						case 3:
							*binSpot++ = partial|c;
							decoderState=0;
							break;
					} /* switch decoderState */
			} /* switch decode */
		} /* for sixFourPtr */
	} /* if sixFourLen */
	else
	{
		/*
		 * all done.  Did all end up evenly?
		 */
		switch (decoderState)
		{
			case 0:
				invalCount += padCount;		/* came out evenly, so should be no pads */
				break;
			case 1:
				invalCount++;							/* data missing */
				invalCount += padCount;		/* since data missing; should be no pads */
				break;
			case 2:
				invalCount += ABS(padCount-2);	/* need exactly 2 pads */
				break;
			case 3:
				invalCount += ABS(padCount-1);	/* need exactly 1 pad */
				break;
		}
	}
	
	/*
	 * save state in caller's buffer
	 */
	d64->decoderState = decoderState;
	d64->invalCount += invalCount;
	d64->padCount = padCount;
	d64->partial = partial;
	d64->wasCR = wasCR;

	*decErrCnt = invalCount;

	return (binSpot - binPtr);
}

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
long EncodeQP(
	char *pBin,
	long nLen,
	char *pQP,
	EncQPPtr eQPstate)
{
	/* These characters are legal to leave UNQUOTED, everything else must be quoted */
	const char *gQPEncodeChars
		= "\t\n\r %&'()*+,-./0123456789:;<>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

	/* The hex digits -- in order */
	const char *pHexArr = "0123456789ABCDEF";
	const char *pQPStart = pQP; /* Remember the start pos */
	char *pEnd, ch, cLastChar;
	int nLineChars, bEncode;

	/* Check if this call is to close the encoding */
	if ((pBin == NULL) || (nLen < 1))
	{
		/* If there is anything on the current line, cap it with an equal */
		/* This avoids having trailing whitespace that would be ignored */
		/* Newline is added for good measure, because we assume TEXT, this */
		/*  shouldn't be a problem */
		if ((eQPstate->nCurLineLen) > 0)
		{
			*pQP++ = '=';
			pQP = newline_copy(pQP);

			eQPstate->nCurLineLen = 0;
			return (pQP - pQPStart);
		}

		return (0);
	}

	/* Restore the state from the caller data */
	nLineChars = eQPstate->nCurLineLen;
	cLastChar = eQPstate->cLastChar;

	/* Loop through the binary data */
	for (pEnd = pBin + nLen; pBin < pEnd; pBin++)
	{
		/* Check if the binary character must be encoded */
		bEncode = (strchr(gQPEncodeChars, (ch = *pBin)) ? 0 : 1);

		/* Will this action put us past the 76 char limit? */
		if ((nLineChars + (bEncode ? 3 : 1)) > 76)
		{
			/* Cap the line, and add a newline */
			*pQP++ = '=';
			pQP = newline_copy(pQP);

			nLineChars = 0; /* Starting new line */
		}

		if (bEncode) /* Encode the character using hex (ie. "A" -> "=41") */
		{
			*pQP++ = '=';
			*pQP++ = pHexArr[(ch>>4) & 0xF];
			*pQP++ = pHexArr[ch & 0xF];
			
			nLineChars += 3;
		}
		else /* No encoding needed, just copy character over */
		{
			*pQP++ = ch;

			/* If we copied a newline, then we need to reset the line char count */
			if (newline_test(cLastChar, ch))
				nLineChars = 0;
			else
				nLineChars++;
		}

		/* Keep track of the last character */
		cLastChar = ch;
	}

	/* Save the state */
	eQPstate->nCurLineLen = nLineChars;
	eQPstate->cLastChar = cLastChar;

	/* Return the number of characters we but in the buffer */
	return (pQP - pQPStart);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
long DecodeQP(
	char *pQP,
	long nLen,
	char *pBin,
	DecQPPtr dQPstate,
	long *decErrCnt)
{
	const char *pBinStart = pBin;
	char *pEnd, ch;
	
	/* Restore the state from the caller */
	QPStates CurState = dQPstate->CurState;
	char cLastChar = dQPstate->cLastChar;

	/* Check if this call is to close the decoding */
	if ((pQP == NULL) || (nLen < 1))
	{
		/* If the state is in the middle of doing something, then error */
		if ((CurState == qpEqual) || (CurState == qpEncoded))
		{
			(*decErrCnt)++;
			CurState = qpNormal;
		}

		return (0);
	}

	/* Loop through the QP data */
	for (pEnd = pQP + nLen; pQP < pEnd; pQP++)
	{
		/* Get the current character */
		ch = *pQP;

		/* What state are we in? */
		switch (CurState)
		{
			case qpNormal: /* Normal: copy everything until an 'equal' */
				if (ch == '=')
					CurState = qpEqual; /* Found an 'equal' char */
				else
					*pBin++ = ch;
			break;
					
			case qpEqual: /* qpEqual: Last char was an equal */
				if (isxdigit(ch))                     /* This char should be a hex digit */
					CurState = qpEncoded;
				else if (newline_test(cLastChar, ch)) /* Or it could be a newline */
					CurState = qpNormal;
				else if (isspace(ch))                 /* Or some whitespace before the newline */
					CurState = qpTrailingWhitespace;
				else
					CurState = qpError;               /* Otherwise, an error */
			break;
				
			case qpEncoded: /* qpEncoded: Last char was the first hex digit of an encoding */
				if ((isxdigit(ch)) && (isxdigit(cLastChar)))
				{
					int left = hex2dec(cLastChar);
					int right = hex2dec(ch);
					int leftshift = left << 4;
					int final = leftshift | right;
					/* Decode the hex digits into a character */
					*pBin++ = final;
					CurState = qpNormal;
				}
				else
					CurState = qpError; /* This char is not a hex digit: an error */
			break;

			case qpTrailingWhitespace: /* qpTrailingWhitespace: whitespace is allowed after */
				                       /*  an equals and before the newline */
				if (newline_test(cLastChar, ch))
					CurState = qpNormal; /* When we find the newline, don't copy it */
				else if (!isspace(ch))
					CurState = qpError; /* If we get to some char BEFORE a newline: an error */
			break;
		}

		if (CurState == qpError) /* Count the errors, reset to normal state (keep trying) */
		{
			(*decErrCnt)++;
			CurState = qpNormal;
		}

		/* Keep track of the last character */
		cLastChar = ch;
	}

	/* Save the state for next time */
	dQPstate->CurState = CurState;
	dQPstate->cLastChar = cLastChar;

	/* Return the number of characters we but in the buffer */
	return (pBin - pBinStart);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Parse Content-Transer-Encoding header line
 *
 *  Args:
 *   src [IN] Valid Transfer-Encoding header.
 *
 *  Returns: enumerated integer 'TrEncType' type specifying CTE.
 */
TrEncType rfc822_parse_cte(const char *src)
{
	const char *kPrefixStr = "Content-Transfer-Encoding:";
	const unsigned int kPrefixStrLen = strlen(kPrefixStr);

	char *cp = (char *) src + kPrefixStrLen, *mechanism = NULL;
	TrEncType cte = CTE_Error;

	// Check prefix
	if (strnicmp(src, kPrefixStr, kPrefixStrLen) == 0)
	{
		// Get first token (skips whitespace/comments)
		mechanism = rfc822_extract_token(&cp);

		// If we got something
		if ((mechanism) && (strlen(mechanism) > 0))
		{
			if (stricmp(mechanism, "base64") == 0)
				cte = CTE_Base64;
			else if (stricmp(mechanism, "quoted-printable") == 0)
				cte = CTE_QP;
			else if (stricmp(mechanism, "7bit") == 0)
				cte = CTE_7bit;
			else if (stricmp(mechanism, "8bit") == 0)
				cte = CTE_8bit;
			else if (stricmp(mechanism, "binary") == 0)
				cte = CTE_Binary;
		}

		safefree(mechanism);
	}

	return (cte);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
char *rfc822_make_cte(TrEncType mechanism)
{
	const char *kPrefix = "Content-Transfer-Encoding: ";
	char pBuf[40], *pCTE;

	switch (mechanism)
	{
		case CTE_Base64:	strcpy(pBuf, "base64");				break;
		case CTE_QP:		strcpy(pBuf, "quoted-printable");	break;
		case CTE_7bit:		strcpy(pBuf, "7bit");				break;
		case CTE_8bit:		strcpy(pBuf, "8bit");				break;
		case CTE_Binary:	strcpy(pBuf, "binary");				break;

		default:
			return (NULL);
	}

	pCTE = (char *) malloc(strlen(pBuf) + strlen(kPrefix) + 1);
	strcpy(pCTE, kPrefix);
	strcat(pCTE, pBuf);

	return (pCTE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
char *rfc822_extract_cte(const char *pFullHeader)
{
	return rfc822_extract_header(pFullHeader, "Content-Transfer-Encoding:");
}

/* ========================================================================== */
/*                              LOCAL FUNCTIONS                               */
/* ========================================================================== */

/*  NEWLINE STUFF: Used locally to copy and test for newlines  */
static const char gNewlineCh1 = '\r';
static const char gNewlineCh2 = '\n';

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Insert a newline, returning position after newline */
/* static */ char *newline_copy(char *dst)
{
	*dst++ = gNewlineCh1;
	*dst++ = gNewlineCh2;

	return dst;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Check if these two characters represent a newline */
/* static */ int newline_test(const char prev, const char curr)
{
	return ((prev == gNewlineCh1) && (curr == gNewlineCh2));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Convert a single HEX character-digit (0123456789ABCDEF) to the decimal */
/* value.  NOTE: This function assumes the character is a valid hex digit */
/* static */ int hex2dec(const char ch)
{
	if (isdigit(ch))
		return (ch -'0');

	return ((toupper(ch) - 'A') + 10);
}

