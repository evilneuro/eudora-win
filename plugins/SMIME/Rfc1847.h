/*
 *  Functions to aide creation and parsing of MIME RFC1847 objects.
 *
 *  Filename: RFC1847.H
 *
 *  Last Edited: Monday, September 30, 1996
 *
 *  Author: Scott Manjourides
 *
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

/* =========================================================================

   - WHAT -

   The functions provided here allow easy creation and parsing of RFC1847
   MIME structure using buffer-based piping. For file-based RFC1847
   interactions use FILE1847.CPP.


   - DESIGN -

   The intent of the design of these functions is to allow easy integration
   with most other buffer-based libraries. The buffers used here are all of
   the 'BufType' type, as defined in BUFTYPE.CPP. This design was chosen to
   make easy transition to other common buffers, and functions are provided
   to do just such conversations (see BUFTYPE.CPP).

   Another major concern in the design of these functions is that of piping.
   We tried to assure that the buffer-based approach we chose would allow
   easy transition to and from existing buffer-based libraries. All buffers
   handled here have inherent size, max size, and position. The
   combination of these three state variables for each buffer gives the
   flexibility for either push (producer driven) or pull (consumer driven)
   piping of data. This flexibility should allow these functions to easily
   be used in many applications.

   The mechanism for producing and consuming data is identical for both
   the parse and create functions.


   - HOW TO USE -

   =============
   Creating 1847
   =============

   You will provide the top-most MIME type in an emsMIMEtype structure. This
   MIME type structure should contain all required content-type parameters,
   except "boundary", which will be created and added for you.

   You also need to provide two binary byte streams, one for each part of
   the MIME structure. These binary streams are presented to rfc1847_create
   using buffer structures defined in BUFTYPE.H. Each buffer should be
   filled using the appropriate _buf() call from BUFTYPE.H and passed to
   rfc1847_create to be consumed.

   The formatted MIME rfc1847 structure is returned in the output buffer,
   without changing the buffer's size. This means that it is possible to
   have the output buffer fill before the input buffer(s) are completely
   consumed. In this case rfc1847_create() will return RFC1847_BUFFERFULL.
   When this value is returned, the caller should simply consume the output
   buffer and re-call rfc1847_create(). The input buffers will retain any
   partially consumed status, and thus should not be altered.

   Because the byte streams are presented in pieces through the use of a
   buffer, the question of indicating the end of a stream needs to be
   addressed. The implied ordering of the two streams (part 1, then part 2)
   gives some help in solving this problem. If you need to chop part 1 into
   multiple buffer pieces, then you will need to NOT supply part 2 until
   part 1 is completed. This is done by passing NULL in place of the buffer.
   As soon as the part 2 is not NULL, part 1 is assumed to be completed.
   Part 2 is assumed to be completed when the part 2 buffer is NULL after
   is was non-NULL.

   Example with part 1 using 3 buffer blocks:

   Part 1       Part 2
   ------       ------
   Block 1      NULL
   Block 2      NULL
   Final block  Part 2
   NULL         NULL    <-- Indicates you are finished, does cleanup

   Of course, if everything fits into one buffer, the entire creation can
   be done in two calls. First call with both valid parts, then finish with
   the required NULL-NULL cleanup call.

   The state parameter should be created before the first call, passed
   (untouched) between calls, and deleted after the final call.

   ============
   Parsing 1847
   ============
   
   In much the same way as creating 1847, you must provide the input 1847
   MIME structure in the form of a buffer or buffers as defined by BUFTYPE.H.

   When the parser returns RFC1847_BUFFERFULL, you should clear both output
   buffer (one each for both parts) before re-calling.

   The MIME type of the parsed 1847 structure is returned as an emsMIMEtype
   structure.

   The state parameter should be created before the first call, passed
   (untouched) between calls, and deleted after the final call.

   ========================================================================= */

#ifndef RFC1847_H
#define RFC1847_H

#include <windows.h> /* Must be included for ems-win.h */
#include "ems-win.h"

#include "BUFTYPE.H"

/* ------------------------------------------------------------------------- */

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* THESE STRCUTURES AND TYPES ARE TO BE CONSIDERED OPAQUE TO THE USER */
	/* Internally used values -- irrelevant to the avaerae user */
	typedef enum {
		CSE_Start,
		CSE_DoHeader,
		CSE_DoBoundary,
		CSE_DoPart,
		CSE_DoOutput,
		CSE_Done,
		CSE_Fail
	} CreateStageEnum;

	/* Internally used values -- irrelevant to the avaerae user */
	typedef enum {
		PSE_Start,
		PSE_SkipHeader,
		PSE_ParseHeader,
		PSE_SkipFirst,
		PSE_PrePart,
		PSE_OutPart,
		PSE_Done,
		PSE_Fail,
		PSE_KeepUntil,
		PSE_IgnoreUntil,
		PSE_OutputUntil,
		PSE_DoOutput
	} ParseStageEnum;

	/* Create-state strucutre -- irrelevant to the avaerae user */
	typedef struct createState {
		int bInitialized;
		CreateStageEnum stage, next_stage, afterout_stage;
		unsigned int current_part;
		BufTypePtr pTmpBuf, pOutputBuf;
		char *pBoundaryStr;
	} createState;

	typedef createState *createStatePtr;

	/* Parse-state strucutre -- irrelevant to the avaerae user */
	typedef struct parseState {
		int bInitialized;
		ParseStageEnum stage, next_stage, afterout_stage;
		unsigned int current_part, nOutputCount, nAfteroutSkip, nLeftSpanCount;
		BufTypePtr /*pTmpBuf,*/ pOutputBuf, pSearchBuf, pPrevBuf, pBoundaryBuf,
			pDblNewlineBuf, pNewlineBuf;
	} parseState;

	typedef parseState *parseStatePtr;
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

/* Return values for parse and create functions */
enum { RFC1847_FAIL, RFC1847_COMPLETED, RFC1847_BUFFERFULL };

/* ------------------------------------------------------------------------- */

/*
 *  Allocates and returns a pointer to an initialized createState structure.
 *   
 *  NOTE: The user of this function is responsible for properly
 *        deleting the returned data by calling delete_create_state()
 *        when the state data is no longer needed.
 *
 *  No arguments.
 *
 *  Pointer to newly created createState structure. NULL if error.
 */
createStatePtr new_create_state();

/*
 *  Frees all memory within the state structure, including the structure
 *  itself.
 *   
 *  NOTE: The pointer argument should be assumed invalid after this call.
 *
 *  Args:
 *    pState [IN] Pointer to the state structure to be deleted.
 *
 *  No return value.
 */
void delete_create_state(createStatePtr pState);

/*
 *  Allocates and returns a pointer to an initialized parseState structure.
 *   
 *  NOTE: The user of this function is responsible for properly
 *        deleting the returned data by calling delete_create_state()
 *        when the state data is no longer needed.
 *
 *  No arguments.
 *
 *  Pointer to newly created parseState structure. NULL if error.
 */
parseStatePtr new_parse_state();

/*
 *  Frees all memory within the state structure, including the structure
 *  itself.
 *   
 *  NOTE: The pointer argument should be assumed invalid after this call.
 *
 *  Args:
 *    pState [IN] Pointer to the state structure to be deleted.
 *
 *  No return value.
 */
void delete_parse_state(parseStatePtr pState);

/*
 *  Creates RFC1847 MIME structure.
 *   
 *  Args:
 *    mimePtr  [IN]     MIME content info, used to create the MIME headers.
 *    pOutBuf  [OUT]    Output buffer (RFC1847 MIME structure).
 *    pInPart1 [IN]     Input buffer used for first part.
 *    pInPart2 [IN]     Input buffer used for second part.
 *    pState   [IN/OUT] Pointer to state info, caller must preserve.
 *
 *  Return values:
 *    RFC1847_COMPLETED   Input was sucessfully and completely parsed.
 *    RFC1847_BUFFERFULL  The output buffer is full; input partially parsed.
 *    RFC1847_FAIL        Parsing error.
 */
int rfc1847_create(emsMIMEtypeP mimePtr,
				   BufTypePtr pOutBuf,
				   BufTypePtr pInPart1,
				   BufTypePtr pInPart2,
				   createStatePtr pState);

/*
 *  Parses RFC1847 MIME structure, returning MIME info and separated parts.
 *   
 *  Args:
 *    mimePtr   [OUT]    Returns a pointer to a newly created emsMIMEtype.
 *    pOutPart1 [OUT]    Output buffer, part 1.
 *    pOutPart2 [OUT]    Output buffer, part 2.
 *    pInBuf    [IN]     Input buffer containing full RFC1847 MIME structure.
 *    pState    [IN/OUT] Pointer to state info, caller must preserve.
 *
 *  Return values:
 *    RFC1847_COMPLETED   Input was sucessfully and completely parsed.
 *    RFC1847_BUFFERFULL  The output buffer is full; input partially parsed.
 *    RFC1847_FAIL        Parsing error.
 */
int rfc1847_parse(emsMIMEtypeP *mimePtr,
				  BufTypePtr pOutPart1,
				  BufTypePtr pOutPart2,
				  BufTypePtr pInBuf,
				  parseStatePtr pState);

#endif /* RFC1847_H */