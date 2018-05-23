/*
 *  Functions to aide manipulation and parsing of MIME RFC1847 objects.
 *
 *  Filename: RFC1847.cpp
 *  Original Author: Scott Manjourides
 *
 *  Copyright 1995-2003 QUALCOMM, Incorporated
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
 *  Send comments and questions to <eudora-emsapi@qualcomm.com>
 *
 */

#include <stdlib.h> /* rand() */
#include <time.h> /* time() */
#include <stdio.h> /* sprintf() */
#include <string.h>
#include <windows.h> /* Must be included for ems-win.h */

#include "emsapi-win.h"
#include "RFC822.h"
#include "RFC1847.h"
#include "BufType.h"
#include "MimeType.h"

/* ------------------------------------------------------------------------- */

static char *kBoundaryParameterName = "boundary";
static char *kBoundaryPrefixSuffix = "=-=-=-=";

/* ------------------------------------------------------------------------- */

#define safefree(p) { if (p) { free(p); (p) = NULL; } }

/* ------------------------------------------------------------------------- */

static int rfc1847_create_init(createStatePtr pState);
static int rfc1847_parse_init(parseStatePtr pState);

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
createStatePtr new_create_state()
{
	createStatePtr p;
	p = (createStatePtr) malloc(sizeof(createState));

	if (!p)
		return (NULL);

	p->stage = p->next_stage = p->afterout_stage = CSE_Fail;

	p->pBoundaryStr = NULL;
	p->current_part = 0;

	p->pTmpBuf = make_buf();
	p->pOutputBuf = NULL;

	p->bInitialized = FALSE;

	return (p);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
void delete_create_state(createStatePtr p)
{
	if (!p)
		return;

	delete_buf(p->pTmpBuf);
	safefree(p->pBoundaryStr);

	safefree(p);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
parseStatePtr new_parse_state()
{
	parseStatePtr p;
	p = (parseStatePtr) malloc(sizeof(parseState));

	if (!p)
		return (NULL);

	p->stage = p->next_stage = p->afterout_stage = PSE_Fail;

	p->current_part = 0;

	p->nOutputCount = 0;
	p->nAfteroutSkip = 0;
	p->nLeftSpanCount = 0;

	p->pOutputBuf = NULL;
	p->pSearchBuf = NULL;

//	p->pTmpBuf = make_buf();
	p->pPrevBuf = make_buf();
	p->pBoundaryBuf = make_buf();
	p->pDblNewlineBuf = make_buf();
	p->pNewlineBuf = make_buf();

	p->bInitialized = FALSE;

	return (p);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
void delete_parse_state(parseStatePtr p)
{
	if (!p)
		return;
		
//	delete_buf(p->pTmpBuf);
	delete_buf(p->pPrevBuf);
	delete_buf(p->pBoundaryBuf);
	delete_buf(p->pDblNewlineBuf);
	delete_buf(p->pNewlineBuf);

	safefree(p);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Performs necessary initialization for rfc1847_create().
 *   
 *  Args:
 *    pState [IN] Pointer to the state structure.
 *
 *  Return value: BOOLEAN indicating success.
 */
static int rfc1847_create_init(createStatePtr pState)
{
	char *pBoundaryTxt = (char *) malloc(128);

	if (!pBoundaryTxt)
		return (FALSE);

	/* Create a random boundary; max 68 chars */
	srand((unsigned)time(NULL));

	sprintf(pBoundaryTxt, "%s%d%d%d%s",
		kBoundaryPrefixSuffix,
		rand(),
		rand(),
		rand(),
		kBoundaryPrefixSuffix);

	pState->pBoundaryStr = pBoundaryTxt;
	pState->stage = CSE_Start;

	pState->bInitialized = TRUE;

	return (TRUE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
				   createStatePtr pState)
{
	if (!pState->bInitialized)
		if (!rfc1847_create_init(pState))
			pState->stage = CSE_Fail;

	while (pState->stage != CSE_Done)
	{
		switch (pState->stage)
		{
			case CSE_Start:
				pState->stage = CSE_DoHeader; /* Just an entry point */
			break;

			case CSE_DoHeader:
			{
				pState->stage = CSE_Fail; // If not changed, then something failed

				if ((mimePtr) && (pState->pBoundaryStr))
				{
					/* Remove the boundary parameter from the MIME struct -- if it's there */
					remove_mime_parameter(mimePtr, kBoundaryParameterName);

					/* Add the boundary to the MIME type struct */
					if (add_mime_parameter(mimePtr, kBoundaryParameterName, pState->pBoundaryStr))
					{
						char *pCT = NULL;

						/* Create an RFC822 Content-Type header line from the MIME struct*/
						if ((pCT = string_mime_type(mimePtr)) != NULL)
						{
							strcpy_buf(pState->pTmpBuf, pCT); /* Content-Type: */
							strcat_buf(pState->pTmpBuf, "\r\n"); /* Not the blank line, just end of CT: */
							resetpos_buf(pState->pTmpBuf);

							safefree(pCT);

							pState->pOutputBuf = pState->pTmpBuf;
							pState->afterout_stage = CSE_DoBoundary;
							pState->stage = CSE_DoOutput;
						}
					}
				}
			}
			break;

			case CSE_DoBoundary:
			{
				strcpy_buf(pState->pTmpBuf, "\r\n");
				strcat_buf(pState->pTmpBuf, "--");
				strcat_buf(pState->pTmpBuf, pState->pBoundaryStr);

				if ((pState->current_part) < 2) /* End the header and first part */
				{
					pState->current_part++;
					pState->afterout_stage = CSE_DoPart;
				}
				else /* End of second part */
				{
					strcat_buf(pState->pTmpBuf, "--");
					pState->afterout_stage = CSE_Done;
				}

				strcat_buf(pState->pTmpBuf, "\r\n");
				resetpos_buf(pState->pTmpBuf);

				pState->stage = CSE_DoOutput;
				pState->pOutputBuf = pState->pTmpBuf;
			}
			break;

			case CSE_DoPart:
			{
				switch (pState->current_part)
				{
					case 1: /* We're doing part 1 now */
					{
						if (poslen_buf(pInPart1) > 0)
						{
							pState->stage = CSE_DoOutput;
							pState->pOutputBuf = pInPart1;

							if (pInPart2)
								pState->afterout_stage = CSE_DoBoundary;
							else
								pState->afterout_stage = CSE_DoPart;
						}
						else /* pInPart1 is empty/completed/non-existant */
						{
							if (pInPart2)
								pState->stage = CSE_DoBoundary;
							else /* !pInPart2 */
								return (RFC1847_COMPLETED); /* We've eaten all of part 1, and there is no part 2 */
						}
					}
					break;

					case 2: /* We're doing part 2 now */
					{
						if (poslen_buf(pInPart2) > 0)
						{
							pState->stage = CSE_DoOutput;
							pState->pOutputBuf = pInPart2;
							pState->afterout_stage = CSE_DoPart;
						}
						else /* pInPart1 is empty/completed/non-existant */
						{
							if (pInPart2)
								return (RFC1847_COMPLETED); /* We've eaten all of part 2 */
							else /* !pInPart2 */
								pState->stage = CSE_DoBoundary;
						}
					}
					break;
				}
			}
			break;

			case CSE_DoOutput:
			{
				if (pOutBuf)
				{
					unsigned int nLen = poslen_buf(pState->pOutputBuf);
					unsigned int nOut = bufins_buf(pOutBuf, pState->pOutputBuf);

					if (nOut < nLen) /* Anything left to output */
						return (RFC1847_BUFFERFULL);

					pState->stage = pState->afterout_stage;
				}
				else
					return (RFC1847_BUFFERFULL); /* No output buffer */
			}
			break;

			case CSE_Fail:
				return (RFC1847_FAIL);
			break;
		} /* switch (stage) */
	} /* while */

	return (RFC1847_COMPLETED);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Performs necessary initialization for rfc1847_parse().
 *
 *  Args:
 *    pState [IN] Pointer to the state structure.
 *
 *  Return value: BOOLEAN indicating success.
 */
static int rfc1847_parse_init(parseStatePtr pState)
{
	if (!pState)
		return (FALSE);

	strcpy_buf(pState->pNewlineBuf, "\r\n");
	strcpy_buf(pState->pDblNewlineBuf, "\r\n\r\n");

	resetpos_buf(pState->pNewlineBuf);
	resetpos_buf(pState->pDblNewlineBuf);
	
	pState->stage = PSE_Start;
	pState->bInitialized = TRUE;

	return (TRUE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
				  parseStatePtr pState)
{
	int bForceJumpState = FALSE;

	if (!pState->bInitialized)
		if (!rfc1847_parse_init(pState))
			pState->stage = PSE_Fail;

	while ((bForceJumpState) || (poslen_buf(pInBuf) > 0))
	{
		bForceJumpState = FALSE;

		switch (pState->stage)
		{
			case PSE_Start:
			{
				/* Just an entry point */
				pState->stage = PSE_SkipHeader;
			}
			break;

			case PSE_SkipHeader:
			{
				pState->stage = PSE_KeepUntil;
				pState->next_stage = PSE_ParseHeader;
				pState->pSearchBuf = pState->pDblNewlineBuf;
			}
			break;

			case PSE_ParseHeader: /* Do all the mimePtr stuff */
			{
				resetpos_buf(pState->pPrevBuf);

				unsigned int nPrevLen = poslen_buf(pState->pPrevBuf);
				char *pHeader = (char *) malloc(nPrevLen + 1);

				if (!pHeader)
				{
					pState->stage = PSE_Fail;
					{ bForceJumpState = TRUE; continue; };
				}

				strncpy(pHeader, getpos_buf(pState->pPrevBuf), nPrevLen);
				*(pHeader + nPrevLen) = '\0';

				char *pCT = rfc822_extract_header(pHeader, "Content-Type:");
				safefree(pHeader);

				if (!pCT)
				{
					pState->stage = PSE_Fail;
					{ bForceJumpState = TRUE; continue; };
				}

				*mimePtr = parse_make_mime_type(pCT);
				safefree(pCT);

				/* get boundary info */
				const char *cp = get_mime_parameter(*mimePtr, kBoundaryParameterName);

				if (!cp)
				{
					pState->stage = PSE_Fail;
					{ bForceJumpState = TRUE; continue; };
				}

				strcpy_buf(pState->pBoundaryBuf, "\r\n--");
				strcat_buf(pState->pBoundaryBuf, cp);
				// DO NOT free() cp


				resetpos_buf(pState->pBoundaryBuf);

				unsigned int nSkip = skipcount_buf(pState->pPrevBuf, pState->pBoundaryBuf);
				unsigned int nRemain = nPrevLen - nSkip;

				pState->nLeftSpanCount = nRemain;

				free_buf(pState->pPrevBuf);

				pState->stage = PSE_SkipFirst;
			}
			break;

			case PSE_SkipFirst:
			{
				pState->stage = PSE_IgnoreUntil;
				pState->pSearchBuf = pState->pBoundaryBuf;
				pState->next_stage = PSE_PrePart;
			}
			break;

			case PSE_PrePart:
			{
				pState->stage = PSE_IgnoreUntil;
				pState->pSearchBuf = pState->pNewlineBuf;
				pState->next_stage = PSE_OutPart;
			}
			break;

			case PSE_OutPart:
			{	
				pState->current_part++;

				if (pState->current_part <= 2)
				{
					pState->stage = PSE_OutputUntil;
					pState->pSearchBuf = pState->pBoundaryBuf;
					pState->next_stage = PSE_PrePart;
				}
				else
					pState->stage = PSE_Done;
			}
			break;

			case PSE_Done:
			{
				delpos_buf(pInBuf);
			}
			break;

			case PSE_Fail:
			{
				return (RFC1847_FAIL);
			}
			break;

			/* ----- ----- */

			case PSE_KeepUntil:
			case PSE_IgnoreUntil:
			case PSE_OutputUntil:
			{
				int bFound = FALSE;
				unsigned int nPrefixCount = 0, nAfterSkip = 0;

				resetpos_buf(pState->pSearchBuf);

				const unsigned int nInLen = poslen_buf(pInBuf);
				const unsigned int nSearchLen = poslen_buf(pState->pSearchBuf);

				/* Did we have a partial span at the end of last buffer? */
				if (pState->nLeftSpanCount > 0)
				{
					//unsigned int nRemainingCount = nSearchLen - (pState->nLeftSpanCount);
					
					/* Check for completion of span */
					// If doesn't continue match, returns zero
					// otherwise returns number of chars of pSearchBuf that have been matched
					unsigned int nNewMatched = completecount_buf(pInBuf, pState->pSearchBuf, pState->nLeftSpanCount);

					if (nNewMatched == nSearchLen) /* complete match made */
					{
						bFound = TRUE;
						nPrefixCount = 0; /* No chars in this buffer are before the match */
						nAfterSkip = nSearchLen - (pState->nLeftSpanCount); /* Move buffer position to AFTER match */

						pState->nLeftSpanCount = 0;
					}
					else if (nNewMatched == 0) /* match failed */
					{
						/* need to 'do' pState->nLeftSpanCount of pState->pSearchBuf */

						switch (pState->stage)
						{
							case PSE_KeepUntil:
								bufncat_buf(pState->pPrevBuf, pState->pSearchBuf, pState->nLeftSpanCount);
							break;

							case PSE_IgnoreUntil:
								/* Ignore -- do nothing */
							break;
							
							case PSE_OutputUntil:
								pState->pOutputBuf = pState->pSearchBuf;
								pState->nOutputCount = pState->nLeftSpanCount;
								pState->afterout_stage = pState->stage; /* Stay in same stage */
								pState->stage = PSE_DoOutput;
							break;
						}

						pState->nLeftSpanCount = 0;

						if (pState->stage == PSE_DoOutput)
							{ bForceJumpState = TRUE; continue; };
					}
					else /* Continued to match, but not completed yet -- the input buffer is smaller than the pSearchBuf */
					{
						// Remainder of pInBuf matches, so we need to gobble it
						delpos_buf(pInBuf);
						pState->nLeftSpanCount = nNewMatched;
						continue; // We want to 'return', because we are done with input
					}
				}

				resetpos_buf(pState->pSearchBuf);

				if (!bFound)
				{
					if (pState->pSearchBuf) // nSearchLen?
					{
						// Find match of pSearchBuf, either complete or end-spanning
						// return number of chars to skip before match
						unsigned int nSkip = skipcount_buf(pInBuf, pState->pSearchBuf);
						unsigned int nRemain = nInLen - nSkip;

						if (nRemain > nSearchLen) /* Found 'complete' */
						{
							bFound = TRUE;
							nPrefixCount = nSkip;
							nAfterSkip = nSearchLen;
						}
						else /* Either not found or partial possible */
						{
							nPrefixCount = nSkip;
							nAfterSkip = nRemain; /* Gobble up the remaining (known to match) */
							pState->nLeftSpanCount = nRemain;
						}
					}
					else
					{
						nPrefixCount = nInLen;
						nAfterSkip = 0;
					}
				}

				resetpos_buf(pState->pSearchBuf);

				if (bFound) /* Found */
				{
					switch (pState->stage)
					{
						case PSE_KeepUntil:
							bufncat_buf(pState->pPrevBuf, pInBuf, nPrefixCount + nAfterSkip);
							pState->stage = pState->next_stage;
						break;

						case PSE_IgnoreUntil: /* Ignore -- do nothing */
							incpos_buf(pInBuf, nPrefixCount + nAfterSkip);
							pState->stage = pState->next_stage;
						break;
						
						case PSE_OutputUntil:
							pState->pOutputBuf = pInBuf;
							pState->nOutputCount = nPrefixCount;
							pState->afterout_stage = pState->next_stage;
							pState->nAfteroutSkip = nAfterSkip;
							pState->stage = PSE_DoOutput;
						break;

						default:
							pState->stage = PSE_Fail;
						break;
					}
				}
				else /* not found */
				{
					switch (pState->stage)
					{
						case PSE_KeepUntil:
							bufncat_buf(pState->pPrevBuf, pInBuf, nPrefixCount + nAfterSkip);
						break;

						case PSE_IgnoreUntil: /* Ignore -- do nothing */
							incpos_buf(pInBuf, nPrefixCount + nAfterSkip);
						break;
						
						case PSE_OutputUntil:
							pState->pOutputBuf = pInBuf;
							pState->nOutputCount = nPrefixCount;
							pState->afterout_stage = pState->stage; /* Same stage */
							pState->nAfteroutSkip = nAfterSkip;
							pState->stage = PSE_DoOutput;
						break;

						default:
							pState->stage = PSE_Fail;
						break;
					}
				}
			}
			break;

			case PSE_DoOutput:
			{
				// Need to distiguish between the two output paths
				BufTypePtr p = NULL;

				switch (pState->current_part)
				{
					case 1: p = pOutPart1;
					break;

					case 2: p = pOutPart2;
					break;

					default: pState->stage = PSE_Fail;
					break;
				}

				if (p)
				{
					unsigned int nOut = bufnins_buf(p, pState->pOutputBuf, pState->nOutputCount);

					// Need to check if not everything was outputted
					// If not -- buffer is full, adjust 'nOutputCount', return BUFFERFULL;
					pState->nOutputCount -= nOut;

					if ((pState->nOutputCount) > 0) /* Anything left to output */
						return (RFC1847_BUFFERFULL);

					pState->stage = pState->afterout_stage;
					if (pState->nAfteroutSkip > 0)
						incpos_buf(pState->pOutputBuf, pState->nAfteroutSkip);
				}
				else
					return (RFC1847_BUFFERFULL); /* No output buffer */
				
			}
			break;
		}
	}

	return (RFC1847_COMPLETED);
}
