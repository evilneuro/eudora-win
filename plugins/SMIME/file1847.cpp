/*
 *  Filename: file1847.cpp
 *
 *  Last Edited: Wednesday, October 2, 1996
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
 */

#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "ems-win.h" // The EMS API
#include "mimetype.h"
#include "encoding.h"
#include "rfc822.h"
#include "rfc1847.h"

#include "file1847.h"

/* ------------------------------------------------------------------------ */

typedef struct {
	int bFoundHeader;
	BufTypePtr pBuf, pSearchBuf;
	TrEncType cte;
	unsigned int nPrevEndMatch;
	char *preEncBuffer;
	Dec64Ptr d64state;
	DecQPPtr dQPstate;
} PartParseState;

typedef PartParseState *PartParseStatePtr;

/* ------------------------------------------------------------------------ */

static int DoCreateOnePart(emsMIMEtypeP InPartMimePtr,
					 emsMIMEtypeP OutMimePtr,
					 TrEncType InPartCTE,
					 FILE *fIn,
					 FILE *fOut,
					 int nPart,
					 long nPreLen,
					 long nTotalInLen,
					 createStatePtr pState,
					 emsProgress progress);

static int DoCreatePartsOutput(emsMIMEtypeP mimePtr,
				   BufTypePtr pInPart1,
				   BufTypePtr pInPart2,
				   createStatePtr pState,
				   FILE *fOut);

static int DoCreateStringOutput(const char *pStr,
				   int nPart,
				   emsMIMEtypeP mimePtr,
				   createStatePtr pState,
				   FILE *fOut);

static int DoParsePart(BufTypePtr pInBuf,
				 FILE *fOut,
				 emsMIMEtypeP *mimePtr,
				 int bDeMime,
				 PartParseStatePtr pState);

static PartParseStatePtr create_PartParseState();
static void delete_PartParseState(PartParseStatePtr p);

static void DoBufOutput(FILE *fOut, BufTypePtr pBuf);

/* ------------------------------------------------------------------------ */

#define kBufferSize (1024)

#define safefree(p) { if (p) { free(p); (p) = NULL; } }
#define safeclose(f) { if (f) { fclose(f); (f) = NULL; } }

const char *kMimeVersionStr = "Mime-Version: 1.0\r\n";

/* ------------------------------------------------------------------------ */

/*  CreateTmpFile
 *
 *  Creates a empty, unique, temporary file in the users 'TEMP'
 *    directory.
 *
 *  No arguments.
 *
 *  Returns a pointer to the filename string. This should be
 *    considered temporary data, and should never be free()'d.
 */
char *CreateTmpFile()
{
	static char tmpName[_MAX_PATH + 1];

#ifdef WIN32 // 32-bit
	char path[_MAX_PATH + 1];			
	GetTempPath(_MAX_PATH, path);
	GetTempFileName(path, "tmp", 0, tmpName);
#else // 16-bit
	BYTE btmpDrv = GetTempDrive(0);
	GetTempFileName(btmpDrv,"tmp", 0, tmpName);
#endif

	return (tmpName);
}

/* ------------------------------------------------------------------------ */

/*  rfc1847_file_create
 *
 *  Creates RFC1847 MIME structure.
 *
 *  Args:
 *    pOutFilename     [IN]     Output filename ** WRITE-ONLY **
 *    OutMimePtr       [IN/OUT] MIME type of output -- boundary param will be added
 *    InPart1MimePtr   [IN]     MIME type of part 1
 *    InPart1CTE       [IN]     Content-transfer-encoding for part 1
 *    pInPart1Filename [IN]     Input filename for part 1 ** READ-ONLY **
 *    InPart2MimePtr   [IN]     MIME type of part 2
 *    InPart2CTE       [IN]     Content-transfer-encoding for part 2
 *    pInPart2Filename [IN]     Input filename for part 2 ** READ-ONLY **
 *    progress         [IN]     EMS progress callback function
 *
 *  Returns boolean. TRUE for success, FALSE for failure.
 */
int rfc1847_file_create(const char *pOutFilename,
				   emsMIMEtypeP OutMimePtr,

				   emsMIMEtypeP InPart1MimePtr,
				   TrEncType InPart1CTE,
				   const char *pInPart1Filename,

				   emsMIMEtypeP InPart2MimePtr,
				   TrEncType InPart2CTE,
				   const char *pInPart2Filename,

				   emsProgress progress)
{
	FILE *fOut = NULL, *fIn1 = NULL, *fIn2 = NULL;

	fOut = fopen(pOutFilename, "ab");     /* WRITE - Append: Binary */
	fIn1 = fopen(pInPart1Filename, "rb"); /* READ-ONLY: Binary */
	fIn2 = fopen(pInPart2Filename, "rb"); /* READ-ONLY: Binary */

	int nStatus = FILE1847_OK;

	if (fOut && fIn1 && fIn2)
	{
		fwrite(kMimeVersionStr, sizeof(char), strlen(kMimeVersionStr), fOut);

		long nLen1 = 0, nTotalInLen = 0;
		emsProgressData progData;

		progData.size = sizeof(emsProgressData);
		progData.value = 0L;
		progData.message = NULL;

		if (progress)
		{
			// get the file length of the two input files -- use for progress
			rewind(fIn1);
			fseek(fIn1, 0, SEEK_END);
			nLen1 = ftell(fIn1);
			rewind(fIn1);

			rewind(fIn2);
			fseek(fIn2, 0, SEEK_END);
			nTotalInLen = nLen1 + ftell(fIn2);
			rewind(fIn2);

			progress(&progData);
		}

		createStatePtr pState = new_create_state();

		if (!pState)
			nStatus = FILE1847_FAIL;

		/* ---------- PART 1 ---------- */

		if (nStatus == FILE1847_OK)
			nStatus = DoCreateOnePart(InPart1MimePtr, OutMimePtr, InPart1CTE, fIn1, fOut, 1, 0, nTotalInLen, pState, progress);

		/* ---------- PART 2 ---------- */

		if (nStatus == FILE1847_OK)
			nStatus = DoCreateOnePart(InPart2MimePtr, OutMimePtr, InPart2CTE, fIn2, fOut, 2, nLen1, nTotalInLen, pState, progress);

		// Do 1847 cleanup
		if (nStatus == FILE1847_OK)
		{
			if (!DoCreatePartsOutput(OutMimePtr, NULL, NULL, pState, fOut))
				nStatus = FILE1847_FAIL;
		}

		// Do final progress indicator
		if ((nStatus == FILE1847_OK) && (progress))
		{
			progData.value = (long) 100;
			if (progress(&progData) ? 1 : 0)
				nStatus = FILE1847_ABORT;
		}

		// Done with the state data
		delete_create_state(pState);
	}

	safeclose(fOut);
	safeclose(fIn1);
	safeclose(fIn2);

	return(nStatus);
}

/* ------------------------------------------------------------------------ */

/*  rfc1847_file_parse
 *
 *  Parses RFC1847 MIME structure.
 *
 *  Args:
 *    pInFilename       [IN]  Input filename of file to parse ** READ-ONLY **
 *    InMimePtr         [OUT] MIME type of input file
 *    pOutPart1Filename [IN]  Output filename for part 1 ** WRITE-ONLY **
 *    OutPart1MimePtr   [OUT] MIME type of created part 1
 *    bPart1DeMime      [IN]  Boolean: Remove header and transfer encoding, part 1?
 *    pOutPart2Filename [IN]  Output filename for part 2 ** WRITE-ONLY **
 *    OutPart2MimePtr   [OUT] MIME type of created part 2
 *    bPart2DeMime      [IN]  Boolean: Remove header and transfer encoding, part 2?
 *    progress          [IN]  EMS progress callback function
 *
 *  Returns boolean. TRUE for success, FALSE for failure.
 */
int rfc1847_file_parse(const char *pInFilename,
					   emsMIMEtypeP *InMimePtr,

					   const char *pOutPart1Filename,
					   emsMIMEtypeP *OutPart1MimePtr,
					   int bPart1DeMime,

					   const char *pOutPart2Filename,
					   emsMIMEtypeP *OutPart2MimePtr,
					   int bPart2DeMime,
				  
					   emsProgress progress)
{
	FILE *fIn = NULL, *fOut1 = NULL, *fOut2 = NULL;

	fIn = fopen(pInFilename, "rb");       /* READ-ONLY: Binary */
	fOut1 = fopen(pOutPart1Filename, "ab"); /* WRITE - Append: Binary */
	fOut2 = fopen(pOutPart2Filename, "ab"); /* WRITE - Append: Binary */

	int nStatus = FILE1847_OK;

	if (fIn && fOut1 && fOut2)
	{
		BufTypePtr pOutPart1= makesize_buf(kBufferSize),
			pOutPart2 = makesize_buf(kBufferSize),
			pInBuf = makesize_buf(kBufferSize);

		if ((!pOutPart1) || (!pOutPart2) || (!pInBuf))
			nStatus = FILE1847_FAIL;

		parseStatePtr pState;
		PartParseStatePtr pParsePart1State, pParsePart2State;
		unsigned int nRead;
		int ret;

		long nTotalInLen = 0;
		int nPercentComplete = 0;
		emsProgressData progData;

		progData.size = sizeof(emsProgressData);
		progData.value = 0L;
		progData.message = NULL;

		// Check to see if we need to parse the parts
		int bReallyParse1 = FALSE, bReallyParse2 = FALSE;
		if ((OutPart1MimePtr) || (bPart1DeMime))
			bReallyParse1 = TRUE;
		if ((OutPart2MimePtr) || (bPart2DeMime))
			bReallyParse2 = TRUE;

		if ((nStatus == FILE1847_OK) && progress)
		{
			// get the file length of the two input files -- use for progress
			rewind(fIn);
			fseek(fIn, 0, SEEK_END);
			nTotalInLen = ftell(fIn);
			rewind(fIn);

			progData.value = (long) nPercentComplete;
			if (progress(&progData) ? 1 : 0)
				nStatus = FILE1847_ABORT;
		}

		pParsePart1State = create_PartParseState();
		pParsePart2State = create_PartParseState();
		pState = new_parse_state();

		if ((nStatus == FILE1847_OK) && (pParsePart1State) && (pParsePart2State) && (pState))
		{
			do {
				/* Fill the input buffer from the input file */
				emptybuf_buf(pInBuf);
				if ((nRead = fread(getbuf_buf(pInBuf), sizeof(char), (bufsize_buf(pInBuf)), fIn)) > 0)
					setlen_buf(pInBuf, nRead);

				/* Parse the input, producing output until completed input */
				do {
					emptybuf_buf(pOutPart1);
					emptybuf_buf(pOutPart2);

					ret = rfc1847_parse(InMimePtr, pOutPart1, pOutPart2, pInBuf, pState);

					if (ret != RFC1847_FAIL)
					{
						if (bReallyParse1) /* Parse part #1 */
						{
							if (!DoParsePart(pOutPart1, fOut1, OutPart1MimePtr, bPart1DeMime, pParsePart1State))
								nStatus = FILE1847_FAIL;
						}
						else /* Raw output part #1 */
							DoBufOutput(fOut1, pOutPart1);

						if (bReallyParse2) /* Parse part #2 */
						{
							if (!DoParsePart(pOutPart2, fOut2, OutPart2MimePtr, bPart2DeMime, pParsePart2State))
								nStatus = FILE1847_FAIL;
						}
						else /* Raw output part #2 */
							DoBufOutput(fOut2, pOutPart2);
					}
				} while ((nStatus == FILE1847_OK) && (ret == RFC1847_BUFFERFULL));

				if (ret == RFC1847_FAIL)
					nStatus = FILE1847_FAIL;

				if ((nStatus == FILE1847_OK) && (progress))
				{
					// Update the progress and check for abort
					nPercentComplete = (int) ((((double)ftell(fIn))/nTotalInLen) * 100.0);
					progData.value = (long) nPercentComplete;
					if (progress(&progData) ? 1 : 0)
						nStatus = FILE1847_ABORT;
				}
			} while ((nStatus == FILE1847_OK) && (!feof(fIn)) && (!ferror(fIn)));

			if (nStatus == FILE1847_OK)
			{
				if (bReallyParse1) /* Parse part #1 */
					if (!DoParsePart(NULL, fOut1, OutPart1MimePtr, bPart1DeMime, pParsePart1State))
						nStatus = FILE1847_FAIL;

				if (bReallyParse2) /* Parse part #2 */
					if (!DoParsePart(NULL, fOut2, OutPart2MimePtr, bPart2DeMime, pParsePart2State))
						nStatus = FILE1847_FAIL;
			}
		}
		else
			nStatus = FILE1847_FAIL;

		delete_PartParseState(pParsePart1State);
		delete_PartParseState(pParsePart2State);
		
		delete_parse_state(pState);
	
		delete_buf(pOutPart1);
		delete_buf(pOutPart2);
		delete_buf(pInBuf);
	}
	else
		nStatus = FILE1847_FAIL; // Files did not open -- error

	safeclose(fIn);
	safeclose(fOut1);
	safeclose(fOut2);

	return(nStatus);
}

/* ------------------------------------------------------------------------ */

/*  DoCreatePartsOutput (static)
 *
 *  Private function used to pipe 1847 output to a file.
 */
static int DoCreatePartsOutput(emsMIMEtypeP mimePtr,
				   BufTypePtr pInPart1,
				   BufTypePtr pInPart2,
				   createStatePtr pState,
				   FILE *fOut)
{
	BufTypePtr pOutBuf = makesize_buf(kBufferSize);
	unsigned int nOutLen, ret;
	char *cp;

	if (pOutBuf)
	{
		if (pInPart1)
			resetpos_buf(pInPart1);

		if (pInPart2)
			resetpos_buf(pInPart2);

		do {
			// Clear the output
			emptybuf_buf(pOutBuf);

			// Do the 1847-dance
			ret = rfc1847_create(mimePtr, pOutBuf, pInPart1, pInPart2, pState);

			// Output whatever we got
			if ((nOutLen = buflen_buf(pOutBuf)) > 0)
			{
				cp = getbuf_buf(pOutBuf);
				fwrite(cp, 1, nOutLen, fOut);
			}
		} while (ret == RFC1847_BUFFERFULL);
	}
	else
		return (FALSE); // Could not create buffer

	delete_buf(pOutBuf);

	return (ret != RFC1847_FAIL);
}

/* ------------------------------------------------------------------------ */

/*  DoCreateStringOutput (static)
 *
 *  Private function used to send a C-string through the 1847 creator and
 *  eventually pipe the 1847 output to a file.
 */
static int DoCreateStringOutput(const char *pStr,
				   int nPart,
				   emsMIMEtypeP mimePtr,
				   createStatePtr pState,
				   FILE *fOut)
{
	BufTypePtr pTmpBuf = make_buf();
	int ret = RFC1847_FAIL;

	if (pTmpBuf)
	{
		// Manually setup the buffer structure -- kind of ugly, but
		// more effiecient than copying the string
		pTmpBuf->buf = (char *) pStr;
		pTmpBuf->len = strlen(pStr);
		pTmpBuf->buf_size = pTmpBuf->len + 1;
		resetpos_buf(pTmpBuf);

		// Now output the buffer
		ret = DoCreatePartsOutput(mimePtr,
			((nPart == 1) ? pTmpBuf : NULL), /* Part 1 */
			((nPart == 2) ? pTmpBuf : NULL), /* Part 2 */
			pState,
			fOut);

		clear_buf(pTmpBuf); // Unassociate buffer with string
		delete_buf(pTmpBuf); // Delete buffer struct -- AFTER we clear it's contents
	}
	else
		return (FALSE); // Could not create buffer

	return (ret != RFC1847_FAIL);
}

/* ------------------------------------------------------------------------ */

/*  DoCreateOnePart (static)
 *
 *  Private function used to pipe a given file through the 1847 creator as
 *  either part 1 or part 2. All De-mime stuff is also done here.
 */
static int DoCreateOnePart(emsMIMEtypeP InPartMimePtr,
					 emsMIMEtypeP OutMimePtr,
					 TrEncType InPartCTE,
					 FILE *fIn,
					 FILE *fOut,
					 int nPart,
					 long nPreLen,
					 long nTotalInLen,
					 createStatePtr pState,
					 emsProgress progress)
{
	BufTypePtr pInBuf = makesize_buf(kBufferSize);

	if (!pInBuf)
		return (FILE1847_FAIL); // Could not create buffer -- error

	int nStatus = FILE1847_OK;

	// Do InPartMimePtr (Content-Type) header
	if ((nStatus == FILE1847_OK) && (InPartMimePtr))
	{
		char *pCT = string_mime_type(InPartMimePtr);
		
		if (pCT)
		{
			if (!DoCreateStringOutput(pCT, nPart, OutMimePtr, pState, fOut))
				nStatus = FILE1847_FAIL;
			else
				if (!DoCreateStringOutput("\r\n", nPart, OutMimePtr, pState, fOut))
					nStatus = FILE1847_FAIL;
		}
		else
			nStatus = FILE1847_FAIL;

		safefree(pCT);
	}

	// Do InPartCTE (Content-Transfer-Encoding) header
	if ((nStatus == FILE1847_OK) && (InPartCTE != CTE_NONE))
	{
		char *pCTE = rfc822_make_cte(InPartCTE);
		
		if (pCTE)
		{
			if (!DoCreateStringOutput(pCTE, nPart, OutMimePtr, pState, fOut))
				nStatus = FILE1847_FAIL;
			else
				if (!DoCreateStringOutput("\r\n", nPart, OutMimePtr, pState, fOut))
					nStatus = FILE1847_FAIL;
		}
		else
			nStatus = FILE1847_FAIL;

		safefree(pCTE);
	}

	// Do blank line -- only if we put out some header already
	if ((nStatus == FILE1847_OK) && ((InPartMimePtr) || (InPartCTE != CTE_NONE)))
	{
		if (!DoCreateStringOutput("\r\n", nPart, OutMimePtr, pState, fOut))
			nStatus = FILE1847_FAIL;
	}

	char *preEncBuffer = NULL;
	unsigned int preEncBufLen;

	Enc64Ptr e64state = NULL; // Used by Encode64()
	EncQPPtr eQPstate = NULL; // Used by EucodeQP()

	if (nStatus == FILE1847_OK)
	{
		switch (InPartCTE)
		{
			// BASE64
			case CTE_Base64:
			{
				// BASE64 expands about 1/3
				preEncBufLen = ((kBufferSize * 6) / 10);
				preEncBuffer = (char *) malloc(preEncBufLen);

				if (!preEncBuffer)
					nStatus = FILE1847_FAIL; // Could not create buffer

				e64state = (Enc64Ptr) malloc(sizeof(Enc64)); // Used by Encode64()
				e64state->partialCount = e64state->bytesOnLine = 0;
			}
			break;

			// QUOTED-PRINTABLE
			case CTE_QP:
			{
				// QP expands max of 3 times
				preEncBufLen = (kBufferSize / 4);
				preEncBuffer = (char *) malloc(preEncBufLen);

				if (!preEncBuffer)
					nStatus = FILE1847_FAIL; // Could not create buffer

				eQPstate = (EncQPPtr) malloc(sizeof(EncQP)); // Used by Encode64()
				eQPstate->nCurLineLen = 0;
				eQPstate->cLastChar = '\0';
			}
			break;

			// Otherwise, no encoding
			default:
			{
				preEncBufLen = 0;
			}
			break;
		}
	}

	size_t nReadLen = 0;
	int nPercentComplete = 0;
	emsProgressData progData;

	progData.size = sizeof(emsProgressData);
	progData.value = 0L;
	progData.message = NULL;

	// Do pInPartFilename
	while ((nStatus == FILE1847_OK) && (!feof(fIn)) && (!ferror(fIn)))
	{
		emptybuf_buf(pInBuf);

		switch (InPartCTE)
		{
			case CTE_Base64:
			{
				nReadLen = fread(preEncBuffer, sizeof(char), preEncBufLen, fIn);
				nReadLen = Encode64(preEncBuffer, nReadLen, getbuf_buf(pInBuf), e64state);
			}
			break;

			case CTE_QP:
			{
				nReadLen = fread(preEncBuffer, sizeof(char), preEncBufLen, fIn);
				nReadLen = EncodeQP(preEncBuffer, nReadLen, getbuf_buf(pInBuf), eQPstate);
			}
			break;

			default: /* 7bit, 8bit, binary, none */
			{
				nReadLen = fread(getbuf_buf(pInBuf), sizeof(char), bufsize_buf(pInBuf), fIn);
			}
			break;
		}

		if (nReadLen > 0)
		{
			setlen_buf(pInBuf, nReadLen);

			if (!DoCreatePartsOutput(OutMimePtr,
					((nPart == 1) ? pInBuf : NULL), /* Part 1 */
					((nPart == 2) ? pInBuf : NULL), /* Part 2 */
					pState,
					fOut))
				nStatus = FILE1847_FAIL;
		}

		if ((nStatus == FILE1847_OK) && (progress))
		{
			// Update the progress and check for abort
			nPercentComplete = (int) (((((double)ftell(fIn) + nPreLen))/nTotalInLen) * 100.0);
			progData.value = (long) nPercentComplete;
			if (progress(&progData) ? 1 : 0)
				nStatus = FILE1847_ABORT;
		}
	}

	if (nStatus == FILE1847_OK)
	{
		// Finish part -- if needed
		switch (InPartCTE)
		{
			case CTE_Base64:
			{
				nReadLen = Encode64(NULL, 0, getbuf_buf(pInBuf), e64state);
			}
			break;

			case CTE_QP:
			{
				nReadLen = EncodeQP(NULL, 0, getbuf_buf(pInBuf), eQPstate);
			}
			break;

			default: /* 7bit, 8bit, binary, none */
			{
				nReadLen = 0;
			}
			break;
		}

		if (nReadLen > 0)
		{
			setlen_buf(pInBuf, nReadLen);

			if (!DoCreatePartsOutput(OutMimePtr,
					((nPart == 1) ? pInBuf : NULL), /* Part 1 */
					((nPart == 2) ? pInBuf : NULL), /* Part 2 */
					pState,
					fOut))
				nStatus = FILE1847_FAIL;
		}
	}

	safefree(e64state);
	safefree(eQPstate);
	safefree(preEncBuffer);
	delete_buf(pInBuf);

	return (nStatus);
}

/* ------------------------------------------------------------------------ */

/*  DoParsePart (static)
 *
 *  Private function used to pipe the output of 1847 parsing into a file.
 */
static int DoParsePart(BufTypePtr pInBuf,
				 FILE *fOut,
				 emsMIMEtypeP *mimePtr,
				 int bDeMime,
				 PartParseStatePtr pState)
{
	if (!pInBuf) // Input buf NULL indicates 'cleanup'
	{
		if (!pState->bFoundHeader)
			return (FALSE);
		else
			return (TRUE);
	}

	if (!pState->bFoundHeader)
	{
		resetpos_buf(pState->pSearchBuf);

		if (pState->nPrevEndMatch > 0)
		{
			/* Check for completion of span */

			// If doesn't continue match, returns zero
			// otherwise returns number of chars of pSearchBuf that have been matched
			unsigned int nNewMatched = completecount_buf(pInBuf, pState->pSearchBuf, pState->nPrevEndMatch);

			if (nNewMatched == buflen_buf(pState->pSearchBuf)) /* complete match made */
			{
				pState->bFoundHeader = TRUE;
				pState->nPrevEndMatch = 0;
				bufncat_buf(pState->pBuf, pInBuf, (buflen_buf(pState->pSearchBuf) - (pState->nPrevEndMatch)));
			}
			else if (nNewMatched != 0) /* Continued to match, but not completed yet -- the input buffer is smaller than the pSearchBuf */
			{
				bufncat_buf(pState->pBuf, pInBuf, poslen_buf(pInBuf));
				pState->nPrevEndMatch = nNewMatched;
				return (TRUE);
			}
			else /* No match continuation */
			{
				pState->nPrevEndMatch = 0;
			}
		}

		resetpos_buf(pState->pSearchBuf);

		// Still not found -- no span
		if (!pState->bFoundHeader)
		{
			// Find match of pSearchBuf, either complete or end-spanning
			// return number of chars to skip before match
			unsigned int nSkip = skipcount_buf(pInBuf, pState->pSearchBuf);
			unsigned int nRemain = poslen_buf(pInBuf) - nSkip;

			if (nRemain == 0) // Not found
			{
				bufncat_buf(pState->pBuf, pInBuf, poslen_buf(pInBuf));
				return (TRUE);
			}
			else
			if (nRemain > buflen_buf(pState->pSearchBuf)) /* Found 'complete' */
			{
				pState->bFoundHeader = TRUE;
				bufncat_buf(pState->pBuf, pInBuf, (nSkip + buflen_buf(pState->pSearchBuf)) );
			}
			else // Partial possible
			{
				pState->nPrevEndMatch = nRemain;
				bufncat_buf(pState->pBuf, pInBuf, poslen_buf(pInBuf));
				return (TRUE);
			}
		}

		// ---------- Now we know it is found ----------

		unsigned int nLen = buflen_buf(pState->pBuf);
		char *pHeader = (char *) malloc(nLen + 1);

		strncpy(pHeader, getbuf_buf(pState->pBuf), nLen);

		{
			char *pCT = rfc822_extract_header(pHeader, "Content-Type:");

			if (mimePtr)
			{
				if (pCT)
				{
					*mimePtr = parse_make_mime_type(pCT);
				}
				else
				{
					*mimePtr = make_mime_type("text", "plain", "1.0");
				}
			}

			safefree(pCT);
		}

		if (bDeMime)
		{
			{
				char *pCTE = rfc822_extract_cte(pHeader);
				if (pCTE)
					pState->cte = rfc822_parse_cte(pCTE);
				else
					pState->cte = CTE_NONE; /* No CTE header, so no encoding */
				safefree(pCTE);
			}

			switch (pState->cte)
			{
				// BASE64
				case CTE_Base64:
				{
					// BASE64 expands about 1/3
					unsigned int preEncBufLen = ((kBufferSize * 10) / 6);
					pState->preEncBuffer = (char *) malloc(preEncBufLen);

					pState->d64state = (Dec64Ptr) malloc(sizeof(Dec64)); // Used by Decode64()
					pState->d64state->decoderState = 0;
					pState->d64state->invalCount = 0;
					pState->d64state->padCount = 0;
					pState->d64state->partial = 0;
					pState->d64state->wasCR = FALSE;
				}
				break;

				// QUOTED-PRINTABLE
				case CTE_QP:
				{
					// QP expands max of 3 times
					unsigned int preEncBufLen = (kBufferSize * 4);
					pState->preEncBuffer = (char *) malloc(preEncBufLen);

					pState->dQPstate = (DecQPPtr) malloc(sizeof(EncQP));
					pState->dQPstate->CurState = qpNormal;
					pState->dQPstate->cLastChar = 0;
				}
				break;

				// Otherwise, no encoding
				default:
				break;
			}
		}
		else
		{
			fwrite(getbuf_buf(pState->pBuf), sizeof(char), nLen, fOut);
		}

		safefree(pHeader)
		free_buf(pState->pBuf);
	}

	// ---------- Now we know it is found and header is init'd ----------

	long errorcnt;

	switch (pState->cte)
	{
		case CTE_Base64:
		{
			unsigned int nReadLen = Decode64(getpos_buf(pInBuf), poslen_buf(pInBuf), pState->preEncBuffer, pState->d64state, &errorcnt);
			fwrite(pState->preEncBuffer, sizeof(char), nReadLen, fOut);
		}
		break;

		case CTE_QP:
		{
			unsigned int nReadLen = DecodeQP(getpos_buf(pInBuf), poslen_buf(pInBuf), pState->preEncBuffer, pState->dQPstate, &errorcnt);
			fwrite(pState->preEncBuffer, sizeof(char), nReadLen, fOut);
		}
		break;

		default: /* 7bit, 8bit, binary, none */
		{
			fwrite(getpos_buf(pInBuf), sizeof(char), poslen_buf(pInBuf), fOut);
		}
		break;
	}

	return (TRUE);
}

/* ------------------------------------------------------------------------ */

/*  create_PartParseState (static)
 *
 *  Private function used initialize the parse state.
 */
static PartParseStatePtr create_PartParseState()
{
	PartParseStatePtr p = (PartParseStatePtr) malloc(sizeof(PartParseState));

	if (p)
	{
		p->bFoundHeader = FALSE;
		p->pBuf = make_buf();
		p->pSearchBuf = make_buf();
		strcpy_buf(p->pSearchBuf, "\r\n\r\n");
		p->cte = CTE_NONE;
		p->nPrevEndMatch = 0;
		p->preEncBuffer = NULL;
		p->d64state = NULL;
		p->dQPstate = NULL;
	}

	return (p);
}

/* ------------------------------------------------------------------------ */

/*  delete_PartParseState (static)
 *
 *  Private function used to delete the parse state.
 */
static void delete_PartParseState(PartParseStatePtr p)
{
	if (p)
	{
		delete_buf(p->pBuf);
		delete_buf(p->pSearchBuf);
		safefree(p->preEncBuffer);
		safefree(p->d64state);
		safefree(p->dQPstate);
		
		safefree(p);
	}
}

/* ------------------------------------------------------------------------ */

/*  DoBufOutput (static)
 *
 *  Private function used to output a buffer.
 */
static void DoBufOutput(FILE *fOut, BufTypePtr pBuf)
{
	fwrite(getpos_buf(pBuf), sizeof(char), poslen_buf(pBuf), fOut);
}