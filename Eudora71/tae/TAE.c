// TAE.c

#include "TAE.h"

#include "TAECommon.h"
#include "TAEDictionary.h"
#include "TAEText.h"

#if defined(macintosh)
#pragma export on
#endif



#ifdef DO_MEMORY_DEBUGGING

#include <stdlib.h>
#include <time.h>
#endif



const short nSafeCollection = 1;

#if defined(macintosh)
int TAEInitDictionary(struct TAEDictState *ptaeds, FSSpecPtr pSpec)
{
	if (ptaeds && pSpec)
	{
		ptaeds->pvdict = TAEDictionary_New(pSpec);
		if (ptaeds->pvdict)
		{
			return 1;
		}
	}
	return 0;
}
#else
int TAEInitDictionary(struct TAEDictState *ptaeds, char *szDictName)
{
	#ifdef DO_MEMORY_DEBUGGING
	srand((unsigned)time(NULL));
	#endif
	if (ptaeds && szDictName)
	{
		ptaeds->pvdict = TAEDictionary_New(szDictName);
		if (ptaeds->pvdict)
		{
			return 1;
		}
	}
	return 0;
}
#endif

int TAECloseDictionary(struct TAEDictState *ptaeds)
{
	if (ptaeds && ptaeds->pvdict)
	{
		TAEDictionary_Delete((struct TAEDictionary *)(ptaeds->pvdict));
		return 1;
	}
	return 0;
}

void TAESetDictionaryDataLocation(struct TAEDictState *ptaeds,
								  void *pdata)
{
	if (ptaeds && ptaeds->pvdict)
	{
		TAEDictionary_SetDictionaryDataLocation((struct TAEDictionary *)(ptaeds->pvdict), pdata);
	}
}

void *TAEGetDictionaryDataLocation(struct TAEDictState *ptaeds)
{
	if (ptaeds && ptaeds->pvdict)
	{
		return TAEDictionary_GetDictionaryDataLocation((struct TAEDictionary *)(ptaeds->pvdict));
	}
	return NULL;
}

int TAEGetDictionaryDataSize(struct TAEDictState *ptaeds)
{
	if (ptaeds && ptaeds->pvdict)
	{
		return TAEDictionary_GetDictionaryDataSize((struct TAEDictionary *)(ptaeds->pvdict));
	}
	return 0;
}

void TAEInitAllMatches(struct TAEAllMatches *ptaeallmatches)
{
	if (ptaeallmatches)
	{
		ptaeallmatches->ptaematches = NULL;
		ptaeallmatches->iNumMatches = 0;
	}
}

void TAEFreeAllMatches(struct TAEAllMatches *ptaeallmatches)
{
	if (ptaeallmatches)
	{
		if (ptaeallmatches->ptaematches)
		{
			TAE_free(ptaeallmatches->ptaematches);
			ptaeallmatches->ptaematches = NULL;
		}
		ptaeallmatches->iNumMatches = 0;
	}
}

void TAEInitScoreData(struct TAEScoreData *ptaesd)
{
	if (ptaesd)
	{
		ptaesd->ptaecolscore = NULL;
		ptaesd->iNumCollections = 0;
	}
}

void TAEFreeScoreData(struct TAEScoreData *ptaesd)
{
	if (ptaesd && ptaesd->ptaecolscore)
	{
		TAE_free(ptaesd->ptaecolscore);
	}
}

int TAEStartSession(struct TAESessionState *ptaess, struct TAEDictState *ptaeds)
{
	if (ptaess && ptaeds && ptaeds->pvdict)
	{
		TAEDictionary_ResetScore((struct TAEDictionary *)(ptaeds->pvdict));
		ptaess->ptaedictstate = ptaeds;
		ptaess->pvtext = TAEText_New();
		if (ptaess->pvtext)
		{
			return 1;
		}
	}
	return 0;
}

int TAEProcessText(struct TAESessionState *ptaess, char *szText, long lTextLen,
				   struct TAEAllMatches *ptaeallmatches, unsigned long lOptions,
				   char **whitelist)
{
	if (ptaess && ptaess->pvtext && ptaess->ptaedictstate && ptaess->ptaedictstate->pvdict)
	{
		struct TAEAllMatches		*ptaeallmatchestmp = NULL;
		struct TAEAllMatches		 taeallmatches;
		taeallmatches.ptaematches = NULL;
		taeallmatches.iNumMatches = 0;
		taeallmatches.iMaxMatches = 0;
		ptaeallmatchestmp = &taeallmatches;

		if (TAEText_ResetText((struct TAEText *)(ptaess->pvtext), szText, lTextLen) != TAE_OK)
		{
			return 0;
		}
		if (TAEText_AnalyzeText((struct TAEText *)(ptaess->pvtext), 
											(struct TAEDictionary *)(ptaess->ptaedictstate->pvdict),
											ptaeallmatchestmp,
											lOptions,
											whitelist) != TAE_OK)
		{
			if (ptaeallmatchestmp && taeallmatches.ptaematches)
			{
				TAE_free(taeallmatches.ptaematches);
			}
			return 0;
		}

		// The internal match structure is too large and is defined internally
		// so allocate the external structure to the exact size needed and transfer
		// the data from the internal match structure to the external match structure.
		if (ptaeallmatches && taeallmatches.iNumMatches > 0)
		{
			int			 i = 0;
			ptaeallmatches->ptaematches = (struct TAEMatch *)TAE_malloc(sizeof(struct TAEMatch) * taeallmatches.iNumMatches);
			if (!ptaeallmatches->ptaematches)
			{
				return 0;
			}
			ptaeallmatches->iMaxMatches = taeallmatches.iNumMatches;
			for (i = 0; i < taeallmatches.iNumMatches; ++i)
			{
				if (taeallmatches.ptaematches[i].nCollection != nSafeCollection)
				{
					ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches].lStart = taeallmatches.ptaematches[i].lStart;
					ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches].lLength = taeallmatches.ptaematches[i].lLength;
					ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches].nCollection = taeallmatches.ptaematches[i].nCollection;
					++ptaeallmatches->iNumMatches;
				}
			}
		}

		// Free the internal match structure.
		if (ptaeallmatchestmp && taeallmatches.ptaematches)
		{
			TAE_free(taeallmatches.ptaematches);
		}

		return 1;
	}
	return 0;
}

int TAEEndSession(struct TAESessionState *ptaess)
{
	if (ptaess && ptaess->pvtext)
	{
		TAEText_Delete((struct TAEText *)(ptaess->pvtext));
		return 1;
	}
	return 0;
}


int TAEGetScoreData(struct TAESessionState *ptaess, struct TAEScoreData *ptaesd, short scores[][7], short nNumEntries)
{
	int						 iScore = 0;
	int						 iHcount = 0;
	int						 iNonHcount = 0;
	int						 iHfreq = 0;
	int						 iNonHfreq = 0;

	#ifdef SHOW_DEBUG_OUTPUT
	char					 szMsg[512];
	#endif

	if (ptaess && ptaess->ptaedictstate && ptaess->ptaedictstate->pvdict && ptaess->pvtext)
	{
		struct TAEText			*ptaetext = (struct TAEText *)(ptaess->pvtext);
		struct TAEDictionary	*ptaedict = (struct TAEDictionary *)(ptaess->ptaedictstate->pvdict);

		if (ptaedict && ptaetext)
		{
			struct TAECollection	*ptaeds = NULL;
			int						 i = 0;

			TAEDictionary_ComputeFreqScore(ptaedict, ptaetext);

			if (ptaesd)
			{
				ptaesd->iNumCollections = ptaedict->taevecCollections.iSize;
				ptaesd->ptaecolscore = (struct TAECollectionScore *)TAE_malloc(sizeof(struct TAECollectionScore) * 
																			   ptaesd->iNumCollections);
			}

			for (i = 0; i < ptaedict->taevecCollections.iSize; ++i)
			{
				ptaeds = (struct TAECollection *)(ptaedict->taevecCollections.plData[i]);
				if (ptaesd && ptaesd->ptaecolscore)
				{
					ptaesd->ptaecolscore[i].iCollectionID = ptaeds->iCollectionID;
					ptaesd->ptaecolscore[i].iTypeID = ptaeds->iTypeID;
					ptaesd->ptaecolscore[i].iCategoryID = ptaeds->iCategoryID;
					ptaesd->ptaecolscore[i].iHitCount = ptaeds->iHitCount;
					ptaesd->ptaecolscore[i].iFreq = ptaeds->iFreq;
				}
				// Proceed only if the match is not in the safe collection.
				if (ptaeds->iCollectionID != nSafeCollection)
				{
					if (ptaeds->iCollectionID % 2 == 0)
					{
						iHfreq += ptaeds->iFreq;
						iHcount += ptaeds->iHitCount;
					}
					else
					{
						iNonHfreq += ptaeds->iFreq;
						iNonHcount += ptaeds->iHitCount;
					}
				}
			}

			return TAE_AnalScore(iHcount, iNonHcount, ptaetext->iTotalTokens, scores, nNumEntries);
		}
	}


	#ifdef SHOW_DEBUG_OUTPUT
	sprintf(szMsg, "\n\nH: %d, non-H: %d\n", iHfreq, iNonHfreq);
	TAE_ShowMessage(szMsg);
	#endif

	return iScore;
}

short TAE_AnalScore(int h, int nonH, int total, short scoreArray[][7], short nNumEntries)
{
	// Default score array

	short scores[9][7] =	{/*	H words		non-H 		total		score */
								{1, 100,	0, 0,	 	0, 0, 		3},
								{2, 1000,	0, 0,		0, 0, 		3},
								{1, 1000,	2, 100,		0, 0, 		3},
								{1, 500,	0, 0, 		0, 0, 		2},
								{1, 1000, 	1, 100,		0, 0,		2},
								{0, 0,		3, 500, 	0, 0,		2},
								{0, 0,		0, 0, 		2, 50,		2},
								{0, 0,		0, 0, 		2, 100,		1},
								{1, 8000,	0, 0, 		0, 0,		1}
							};
	short n = 8;
	short i = 0;
	int hFreq = 0, nonHFreq = 0, totalFreq = 0;
	#ifdef SHOW_DEBUG_OUTPUT
	char szMsg[512];
	#endif
	
	if (!h && !nonH) return 0;	// nothing there

 	if (!scoreArray)
	{
  		// Caller didn't specify an array; use ours
		scoreArray = scores;
 		nNumEntries = n;
	}

	hFreq = h ? total/h : 0;
	nonHFreq = nonH ? total/nonH : 0;
	totalFreq = total/(h+nonH);
	
	#ifdef SHOW_DEBUG_OUTPUT
	sprintf(szMsg, "\n\nH: %d, non-H: %d\n", hFreq, nonHFreq);
	TAE_ShowMessage(szMsg);
	#endif

	for (i=0;i<nNumEntries;i++)
	{
		if ((!scoreArray[i][0] || h>=scoreArray[i][0]) &&
			(!scoreArray[i][1] || hFreq<scoreArray[i][1]) &&
			(!scoreArray[i][2] || nonH>=scoreArray[i][2]) &&
			(!scoreArray[i][3] || nonHFreq<scoreArray[i][3]) &&
			(!scoreArray[i][4] || (h+nonH)>=scoreArray[i][4]) &&
			(!scoreArray[i][5] || totalFreq<scoreArray[i][5]))
		{
			return scoreArray[i][6];
		}
	}
	return 0;
}

#if defined(macintosh)
#pragma export reset
#endif