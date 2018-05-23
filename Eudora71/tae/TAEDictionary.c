// TAEDictionary.c

#include "TAEDictionary.h"

#include "TAECommon.h"

#include <string.h>

const short							kCurrentCompiledDictionaryVersion = 2;

#define TAE_MAXSHORT				32767

#define TAE_ToLower(c)				(((c >= 'A') && (c <= 'Z')) ? (c + 32) : c)


// The following array represents words that are safe (won't match)
// if they are capitalized.
#define kNumSafeCaps				3

char cSafeIfCapitalized[kNumSafeCaps][16] = {"Dick","Dong","Cumming"};


////////////////////////////////////////////////////////////////
// TAEDictionary - Structure and methods for dictionary actions.
////////////////////////////////////////////////////////////////

#if defined(macintosh)
struct TAEDictionary *TAEDictionary_New(FSSpec *pDictSpec)
#else
struct TAEDictionary *TAEDictionary_New(char *szDictFileName)
#endif
{
	struct TAEDictionary *ptaedict = NULL;
	#ifdef DO_MEMORY_DEBUGGING
	// TAE_BeginMemoryDebugging();
	#endif

#if defined(macintosh)
	if (pDictSpec)
#else
	if (szDictFileName)
#endif
	{
		ptaedict = (struct TAEDictionary *)TAE_malloc(sizeof(struct TAEDictionary));
		if (ptaedict)
		{
			TAEVector_Blank(&(ptaedict->taevecCollections));
			ptaedict->iNumNodes = 0;
			ptaedict->iDictionaryRootPos = 0;
			ptaedict->lDataStart = 0;
			ptaedict->iDataLen = 0;
			
			TAEStream_Blank(&(ptaedict->taestream));

#if defined(macintosh)
			if (TAEDictionary_ReadCompiledDictionary(ptaedict, pDictSpec) != TAE_OK)
#else
			if (TAEDictionary_ReadCompiledDictionary(ptaedict, szDictFileName) != TAE_OK)
#endif
			{
				TAEDictionary_Delete(ptaedict);
				ptaedict = NULL;
			}
		}
	}
	return ptaedict;
}

void TAEDictionary_Delete(struct TAEDictionary *ptaedict)
{
	if (ptaedict)
	{
		struct TAECollection	*ptaecollection = NULL;
		int						 i = 0;
		for (i = 0; i < ptaedict->taevecCollections.iSize; ++i)
		{
			ptaecollection = (struct TAECollection *)TAEVector_ElementAt(&(ptaedict->taevecCollections), i);
			if (ptaecollection)
			{
				TAE_free(ptaecollection);
			}
		}

		TAEVector_FreeMemory(&(ptaedict->taevecCollections));

		TAEStream_Close(&(ptaedict->taestream));

		TAE_free(ptaedict);
		ptaedict = NULL;
	}
}


void TAEDictionary_SetDictionaryDataLocation(struct TAEDictionary *ptaedict,
											 void *pdata)
{
	if (ptaedict)
	{
		ptaedict->taestream.pBuf = (char*)pdata;
	}
}

void *TAEDictionary_GetDictionaryDataLocation(struct TAEDictionary *ptaedict)
{
	if (ptaedict)
	{
		return (void*)ptaedict->taestream.pBuf;
	}
	return NULL;
}

int TAEDictionary_GetDictionaryDataSize(struct TAEDictionary *ptaedict)
{
	if (ptaedict)
	{
		return ptaedict->taestream.iDataLen;
	}
	return 0;
}

#if defined(macintosh)
int TAEDictionary_ReadCompiledDictionary(struct TAEDictionary *ptaedict, FSSpecPtr pDictSpec)
#else
int TAEDictionary_ReadCompiledDictionary(struct TAEDictionary *ptaedict, char *szDictFileName)
#endif
{
	short					 nFileVersion = 0;
	short					 nObjectKindToCreate = 0;
	short					 nClusterUniqueID = 0;
	short					 nCategoryUniqueID = 0;
	short					 nCollectionUniqueID = 0;
	struct TAECollection	*ptaecollection = NULL;
#if !defined(macintosh)
	char					 szMsg[512];
#endif

#if defined(macintosh)
	if (!ptaedict || !pDictSpec)
#else
	if (!ptaedict || !szDictFileName)
#endif
	{
		return TAE_UNEXPECTEDNULL;
	}

#if defined(macintosh)
	if ((ptaedict->iDataLen = TAEStream_Open(&(ptaedict->taestream), pDictSpec)) == 0)
#else
	if ((ptaedict->iDataLen = TAEStream_Open(&(ptaedict->taestream), szDictFileName)) == 0)
#endif
	{
#if defined(macintosh)
#else
		sprintf(szMsg, "Couldn't open dictionary %s.\n", szDictFileName);
		TAE_ShowError(szMsg);
#endif
		return TAE_DICTNOTFOUND;
	}

	// Check dictionary version number.
	nFileVersion = TAEStream_ReadShort(&(ptaedict->taestream));
	if (nFileVersion != kCurrentCompiledDictionaryVersion)
	{
		TAE_ShowError("Incorrect dictionary version.\n");
		return TAE_DICTBADVERSION;
	}

	nObjectKindToCreate = (short)TAEStream_ReadChar(&(ptaedict->taestream));

	TAEVector_Init(&(ptaedict->taevecCollections), 10, 2);

	while (nObjectKindToCreate != kEndOfObjects)
	{
		switch (nObjectKindToCreate)
		{
			case kClusterObject:
			{
				++nClusterUniqueID;
				break;
			}

			case kCategoryObject:
			{
				++nCategoryUniqueID;
				break;
			}

			case kCollectionObject:
			{
				nCollectionUniqueID++;
				ptaecollection = (struct TAECollection *)TAE_malloc(sizeof(struct TAECollection));
				if (!ptaecollection)
				{
					return TAE_OUTOFMEMORY;
				}
				ptaecollection->iCollectionID = nCollectionUniqueID;
				ptaecollection->iTypeID = nCategoryUniqueID;
				ptaecollection->iCategoryID = nClusterUniqueID;
				ptaecollection->iHitCount = 0;
				ptaecollection->iFreq = 0;
				// Add collection to master collection list.
				TAEVector_AddElement(&(ptaedict->taevecCollections), (long)ptaecollection);
				break;
			}
		}

		nObjectKindToCreate = (short)TAEStream_ReadChar(&(ptaedict->taestream));
	}

	return TAEDictionary_ReattachNodes(ptaedict);
}

int TAEDictionary_ReattachNodes(struct TAEDictionary *ptaedict)
{
	short						 nTotalEdges = 0;
	int							 nNodeID = 0;
	unsigned int				 iCurrPos = 0;
	int							*piAddresses = NULL;
	char						 szMsg[512];
	short						 nEndMatchCollectionID = kNoEndMatchID;

	if (!ptaedict)
	{
		return TAE_UNEXPECTEDNULL;
	}

	//	Read the number of nodes
	ptaedict->iNumNodes = TAEStream_ReadInt(&(ptaedict->taestream));

	if (ptaedict->iNumNodes == 0)
	{
		TAE_ShowError("\nError: 0 nodes in compact dictionary.\n");
		return TAE_DICTCORRUPTED;
	}

	piAddresses = (int*)TAE_malloc(sizeof(int) * ptaedict->iNumNodes);
	if (!piAddresses)
	{
		return TAE_OUTOFMEMORY;
	}

	ptaedict->lDataStart = ptaedict->taestream.iPos;

	// For debugging only:
	// TAEDictionary_ShowAllNodes(ptaedict);

	iCurrPos = ptaedict->lDataStart;

	// Find actual locations of nodes.
	for (nNodeID = 0; nNodeID < ptaedict->iNumNodes; nNodeID++)
	{
		if (nNodeID == ptaedict->iNumNodes - 1)
		{
			ptaedict->iDictionaryRootPos = iCurrPos;
		}

		nEndMatchCollectionID = kNoEndMatchID;

		// Actual location is offset of node from start of data block.
		piAddresses[nNodeID] = iCurrPos - ptaedict->lDataStart;

		//	Read in the number of edges
		nTotalEdges = TAEStream_ReadShortAt(&(ptaedict->taestream), &iCurrPos);

		//	Read in the end match collection ID if it exists
		if ((nTotalEdges & kEndMatchToRead) != 0)
		{
			// Read just to move past the data.  We don't use this here.
			nEndMatchCollectionID = TAEStream_ReadShortAt(&(ptaedict->taestream), &iCurrPos);
		}
		
		//	Discard the end match exists bit from the number of edges
		nTotalEdges &= kNumberEdgesMask;

		if ((nTotalEdges < 0) || (nTotalEdges > TAE_MAXSHORT))
		{
			//	Illegal number of edges, the file is corrupted
			//	Note that we aren't actually limited to less than 32,767 edges,
			//	this is just a sanity check that can be adjusted.
			sprintf(szMsg, "\nIllegal number of edges: %s the file is corrupted\n", nTotalEdges);
			TAE_ShowError(szMsg);
			return TAE_DICTCORRUPTED;
		}
		else if (nTotalEdges > 0)
		{
			//	Read in the edges
			int							 i = 0;
			char						 cEdgeCharacter = '\0';
			int							 iDestinationNodeID = 0;

			for (i = 0; i < nTotalEdges; i++)
			{
				cEdgeCharacter = TAEStream_ReadCharAt(&(ptaedict->taestream), &iCurrPos);
				iDestinationNodeID = TAEStream_ReadIntAt(&(ptaedict->taestream), &iCurrPos);
			}
		}
	}
	// Convert ID's to node locations.
	iCurrPos = ptaedict->lDataStart;
	for (nNodeID = 0; nNodeID < ptaedict->iNumNodes; nNodeID++)
	{
		//	Read in the number of edges
		nTotalEdges = TAEStream_ReadShortAt(&(ptaedict->taestream), &iCurrPos);

		//	Read in the end match collection ID if it exists
		if ((nTotalEdges & kEndMatchToRead) != 0)
		{
			// Read just to move past the data.  We don't use this here.
			nEndMatchCollectionID = TAEStream_ReadShortAt(&(ptaedict->taestream), &iCurrPos);
		}
		
		//	Discard the end match exists bit from the number of edges
		nTotalEdges &= kNumberEdgesMask;

		if ((nTotalEdges < 0) || (nTotalEdges > TAE_MAXSHORT))
		{
			//	Illegal number of edges, the file is corrupted
			//	Note that we aren't actually limited to less than 32,767 edges,
			//	this is just a sanity check that can be adjusted.
			sprintf(szMsg, "\nIllegal number of edges: %s the file is corrupted\n", nTotalEdges);
			TAE_ShowError(szMsg);
			return TAE_DICTCORRUPTED;
		}
		else if (nTotalEdges > 0)
		{
			//	Read in the edges
			int							 i = 0;
			char						 cEdgeCharacter = '\0';
			int							 iDestinationNodeID = 0;
			long						 iPos = 0;

			for (i = 0; i < nTotalEdges; i++)
			{
				cEdgeCharacter = TAEStream_ReadCharAt(&(ptaedict->taestream), &iCurrPos);
				iDestinationNodeID = TAEStream_ReadIntAt(&(ptaedict->taestream), &iCurrPos);

				iPos = piAddresses[iDestinationNodeID];

				// printf("%d %04x\n", iDestinationNodeID, iPos);

				if (iPos == -1)
				{
					sprintf(szMsg, "\nUnable to find destination node for ID: %d the file is probably corrupted.\n", iDestinationNodeID);
					TAE_ShowError(szMsg);
					return TAE_DICTCORRUPTED;
				}

				TAEStream_SetBytesAt(&(ptaedict->taestream), iCurrPos - 4, iPos, sizeof(int));
			}
		}
	}

	if (piAddresses)
	{
		TAE_free(piAddresses);
	}

	return TAE_OK;
}

int TAEDictionary_NodeAt(struct TAEDictionary *ptaedict, struct TAEDictionaryNode *ptaedn, unsigned int *iPos)
{
	char				 szMsg[512];

	if (!ptaedict || !ptaedn)
	{
		return TAE_UNEXPECTEDNULL;
	}

	if (*iPos <= ptaedict->iDataLen)
	{
		short						 nTranslatedID = 0;
		short						 nEndMatchCollectionID = kNoEndMatchID;

		ptaedn->nTotalEdges = 0;
		ptaedn->nNodeInfo = 0;
		ptaedn->iDestNodeID = 0;

		//	Read in the number of edges
		ptaedn->nTotalEdges = TAEStream_ReadShortAt(&(ptaedict->taestream), iPos);

		nTranslatedID = (short)(kNoEndMatchID + 1);
		ptaedn->nNodeInfo &= ~kEndMatchCollectionIDMask;
		ptaedn->nNodeInfo |= (nTranslatedID & kEndMatchCollectionIDMask);

		//	Read in the end match collection ID if it exists
		if ((ptaedn->nTotalEdges & kEndMatchToRead) != 0)
		{
			nEndMatchCollectionID = TAEStream_ReadShortAt(&(ptaedict->taestream), iPos);
		}
		
		//	External numbering scheme is 0 based, with -1 as an illegal value,
		//	but internally we offset it up by one (1 based, 0 illegal)
		nTranslatedID = (short)(nEndMatchCollectionID + 1);
		ptaedn->nNodeInfo &= ~kEndMatchCollectionIDMask;
		ptaedn->nNodeInfo |= (nTranslatedID & kEndMatchCollectionIDMask);

		//	Discard the end match exists bit from the number of edges
		ptaedn->nTotalEdges &= kNumberEdgesMask;

		if ((ptaedn->nTotalEdges < 0) || (ptaedn->nTotalEdges > TAE_MAXSHORT))
		{
			//	Illegal number of edges, the file is corrupted
			//	Note that we aren't actually limited to less than 32,767 edges,
			//	this is just a sanity check that can be adjusted.
			sprintf(szMsg, "\nIllegal number of edges: %s the file is corrupted\n", ptaedn->nTotalEdges);
			TAE_ShowError(szMsg);
			return TAE_DICTCORRUPTED;
		}
		else if (ptaedn->nTotalEdges > 0)
		{
			//	Read in the edges
			int							 i = 0;
			char						 cEdgeCharacter = '\0';
			int							 iDestinationNodeID = 0;

			for (i = 0; i < ptaedn->nTotalEdges; i++)
			{
				cEdgeCharacter = TAEStream_ReadCharAt(&(ptaedict->taestream), iPos);
				iDestinationNodeID = TAEStream_ReadIntAt(&(ptaedict->taestream), iPos);
			}
		}
	}
	return TAE_OK;
}

int TAEDictionary_NextNode(struct TAEDictionary *ptaedict, 
						   struct TAEDictionaryNode *ptaedn, 
						   unsigned int *iPos, char c)
{
	char			 szMsg[512];

	if (!ptaedict || !ptaedn)
	{
		return 0;
	}

	if (*iPos <= ptaedict->iDataLen)
	{
		short						 nEndMatchCollectionID = 0;
		short						 nTranslatedID = 0;

		//	Read in the number of edges
		ptaedn->nTotalEdges = TAEStream_ReadShortAt(&(ptaedict->taestream), iPos);

		//	Read in the end match collection ID if it exists
		if ((ptaedn->nTotalEdges & kEndMatchToRead) != 0)
		{
			ptaedn->nNodeInfo = kNoEndMatchID;
			nEndMatchCollectionID = TAEStream_ReadShortAt(&(ptaedict->taestream), iPos);
			nTranslatedID = (short)(nEndMatchCollectionID + 1);
			ptaedn->nNodeInfo &= ~kEndMatchCollectionIDMask;
			ptaedn->nNodeInfo |= (nTranslatedID & kEndMatchCollectionIDMask);
		}

		//	Discard the end match exists bit from the number of edges
		ptaedn->nTotalEdges &= kNumberEdgesMask;

		if ((ptaedn->nTotalEdges < 0) || (ptaedn->nTotalEdges > TAE_MAXSHORT))
		{
			//	Illegal number of edges, the file is corrupted
			//	Note that we aren't actually limited to less than 32,767 edges,
			//	this is just a sanity check that can be adjusted.
			sprintf(szMsg, "\nIllegal number of edges: %s the file is corrupted\n", ptaedn->nTotalEdges);
			TAE_ShowError(szMsg);
			return TAE_DICTCORRUPTED;
		}
		else if (ptaedn->nTotalEdges > 0)
		{
			//	Read in the edges
			int							 i = 0;
			char						 cEdgeCharacter = '\0';
			int							 first = 0;
			int							 last = ptaedn->nTotalEdges-1;
			int							 oldPos = *iPos;

			while (first<=last)
			{
				i = (first + last) / 2;
				*iPos = oldPos + (i * (2 + sizeof(int)));
				cEdgeCharacter = TAEStream_ReadCharAt(&(ptaedict->taestream), iPos);
				if (c > cEdgeCharacter)
				{
					first = i + 1;
				}
				else if (c < cEdgeCharacter)
				{
					last = i - 1;
				}
				else
				{
					// Found match.  Position is an offset, convert it to the actual pointer.
					*iPos = TAEStream_ReadIntAt(&(ptaedict->taestream), iPos) + ptaedict->lDataStart;
					return 1;
				}
			}
			
			// didn't find it.  Position after node.  Don't know if necessary
			*iPos = oldPos + ptaedn->nTotalEdges * (2 + sizeof(int));
		}
	}
	return 0;
}

int TAEDictionary_InitMatches(struct TAEDictionary *ptaedict,
								struct TAEText *ptaetext,
								struct TAEAllMatches **ptaeallmatches)
{
	if (!ptaedict || !ptaetext || !ptaeallmatches)
	{
		return TAE_UNEXPECTEDNULL;
	}

	// We need some starting point for allocating the structure for matches.  Now that we no longer
	// pre-tokenize we don't even know the number of tokens.  I did some testing on email messages
	// and I find that the average email message contains 1 token for each 4 characters (headers
	// average 1 token for every 3 characters).  So I divide the text length by 4 to determine the
	// approximate number of tokens then divide by 10 for one match for each 10 tokens.  This is
	// probably overkill but this data is very temporary and we would rather allocate too much once
	// than too little multiple times.  If this number is too small, we will reallocate when needed.
	(*ptaeallmatches)->iMaxMatches = (ptaetext->lTextLen / 4) / 5;
	if ((*ptaeallmatches)->iMaxMatches < 10)
	{
		// Never allow less than 10 allocations.  Later we reallocate 10% of the total, so don't start
		// with less than 10.
		(*ptaeallmatches)->iMaxMatches = 10;
	}
	(*ptaeallmatches)->ptaematches = (struct TAEMatch *)TAE_malloc(sizeof(struct TAEMatch) *
																	(*ptaeallmatches)->iMaxMatches);

	if (!(*ptaeallmatches)->ptaematches)
	{
		return TAE_OUTOFMEMORY;
	}
	(*ptaeallmatches)->iNumMatches = 0;

	return TAE_OK;
}

void TAEDictionary_ProcessMatch(struct TAEDictionary *ptaedict,
								struct TAEText *ptaetext,
								struct TAEAllMatches *ptaeallmatches,
								int iMatchStartPos,
								int iMatchEndPos,
								short nMatchedCollectionID)
{
	struct TAECollection		*ptaecolLastMatchedCollection = NULL;

	if (!ptaedict || !ptaetext)
	{
		return;
	}

	if (nMatchedCollectionID <= ptaedict->taevecCollections.iSize)
	{
		ptaecolLastMatchedCollection = (struct TAECollection *)TAEVector_ElementAt(&(ptaedict->taevecCollections),
																			nMatchedCollectionID - 1);
		if (!ptaecolLastMatchedCollection)
		{
			TAE_ShowError("Matched a non-existent collection!\n");
			return;
		}
	}
	else
	{
		TAE_ShowError("Matched a non-existent collection!\n");
		return;
	}

	// Tally the hit
	TAEDictionary_TallyOneHit(ptaedict, ptaecolLastMatchedCollection);

	// If match data is requested, note the offsets
	if (ptaeallmatches)
	{
		if (ptaeallmatches->iNumMatches == ptaeallmatches->iMaxMatches)
		{
			// We need to allocate memory for more matches.  Allocate 10% of the total memory used.
			int						 iNewMaxMatches = ptaeallmatches->iMaxMatches + 
													  (ptaeallmatches->iMaxMatches / 10);
			struct TAEMatch			*ptaematches = ptaeallmatches->ptaematches;

			ptaeallmatches->ptaematches = (struct TAEMatch *)TAE_malloc(sizeof(struct TAEMatch) * 
																		 iNewMaxMatches);
			if (ptaeallmatches->ptaematches != NULL)
			{
				int					 i = 0;
				for (i = 0; i < ptaeallmatches->iMaxMatches; ++i)
				{
					ptaeallmatches->ptaematches[i].lStart = ptaematches[i].lStart;
					ptaeallmatches->ptaematches[i].lLength = ptaematches[i].lLength;
					ptaeallmatches->ptaematches[i].nCollection = ptaematches[i].nCollection;
				}
				ptaeallmatches->iMaxMatches = iNewMaxMatches;
				TAE_free(ptaematches);
			}
			else
			{
				// We should be more graceful here, but the fact is, if we overflow our original
				// allocation and we can't reallocate we are looking at a large number of hits
				// and failing to note any further matches probably isn't the greatest sin we
				// could commit.
				ptaeallmatches->ptaematches = ptaematches;
				return;
			}
		}
		ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches].lStart = iMatchStartPos;
		ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches].lLength = iMatchEndPos - iMatchStartPos + 1;
		ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches].nCollection = nMatchedCollectionID;
		++ptaeallmatches->iNumMatches;
	}
}

/*
 * Categorize the words in the wordList.
 * 
 */
int TAEDictionary_CategorizeWordsAndPhrases(struct TAEDictionary *ptaedict, 
											struct TAEText *ptaetext,
											struct TAEAllMatches *ptaeallmatches,
											unsigned long lOptions,
											char **whitelist)
{
	unsigned char				 c = '\0';						// Current character
	int							 iTokenStart = -1;				// Starting point of current token
	int							 iTokenEnd = -1;				// Ending point of current token
	int							 iMatchStartPos = -1;			// Starting point of matching word/phrase
	int							 iMatchEndPos = -1;				// End point of matching word/phrase
	tae_bool					 bStillMatches = TAE_TRUE;		// Flag to indicate if the text still matches
	int							 iTokensInMatch = 0;			// The number of tokens in the current match
	int							 iTokensInCurrRun = 0;			// The number of tokens in the current run of words
	short						 nMatchedCollectionID = kNoEndMatchID;	// ID of last collection matched
	int							 iResumePos = -1;				// The position at which to resume the search
	tae_bool					 bInRunout = TAE_FALSE;			// Flag to indicate we are in runout mode
	unsigned int				 iDictIndex = 0;				// The index into the dictionary data
	struct TAEDictionaryNode	 taednCurNode;					// Current node within the dictionary data

	if (!ptaedict || !ptaetext)
	{
		return TAE_UNEXPECTEDNULL;
	}

	iDictIndex = ptaedict->iDictionaryRootPos;

	// If match data is requested, prepare the data list.
	if (ptaeallmatches)
	{
		int			iReturn = TAEDictionary_InitMatches(ptaedict, ptaetext, &ptaeallmatches);
		if (iReturn != TAE_OK)
		{
			return iReturn;
		}
	}

	// Zero out the matches for this text.
	ptaetext->iNumMatches = 0;

	nMatchedCollectionID = kNoEndMatchID;

	// Iterate through the text character by character looking for matching words and phrases.
	do
	{
		c = TAEText_NextChar(ptaetext, lOptions);
		// At this point all characters are lumped into two categories: meaningful characters
		// (letters, numbers, punctuation) and delimiters (we only see spaces).  One or more
		// consecutive meaningful characters surrounded by spaces (or the beginning or ending
		// of the text) constitute a token.  Although we examine the text character by character
		// we need to have a notion of a token for two reasons: 1) matches only happen on whole
		// tokens (if "abc" is in the dictionary, the text "abcd" will not be a match) and 2)
		// the number of tokens in the text is a factor in the scoring algorithm.

		// This "if"/"else" examines the current character and its context and handles it
		// accordingly.  It maintains the indices indicating the start and end of the individual
		// tokens and the current matching tokens (if any).
		if (c == ' ')
		{
			// We found a space which is the token delimiter.
			if (iTokenStart == -1)
			{
				// There is no token start, so this is a leading space before anything meaningful.
			}
			else
			{
				// We have a token in progress and this space is the end of that token.
				++(ptaetext->iNumTokens);
				++iTokensInCurrRun;
				iTokenEnd = ptaetext->lCharPos;

				if ((iMatchStartPos > -1) && bStillMatches)
				{
					// The token that just ended is part of a match.  See if the end of this
					// token is also the end of a match.
					unsigned int		iTmpDictIndex = iDictIndex;		// Temporary variable so we don't change iDictIndex.

					// Get the node info for the last node in the current match.
					if (TAEDictionary_NodeAt(ptaedict, &taednCurNode, &iTmpDictIndex) == TAE_OK)
					{
						// See if the node is the end of a match.
						if ((taednCurNode.nNodeInfo != kNoEndMatchID) &&
							(taednCurNode.nNodeInfo != kNextDictionaryNodeID))
						{
							// The end of the current word is a match, remember where we matched.
							// If we look for a longer and none is found, this is the final match.
							
							// Match end is last token pos plus the length of the current token.
							// (The current pos includes trailing whitespace, so we can't simply use that.)
							iMatchEndPos = ptaetext->lCharPos;
							// Remember the ID of the collection we matched.
							nMatchedCollectionID = taednCurNode.nNodeInfo;
							// Note the number of tokens in the match.
							iTokensInMatch = iTokensInCurrRun;
							#ifndef ALLOW_OVERLAPPING_MATCHES
							iResumePos = ptaetext->lTextPos;
							#endif
						}
					}
					else
					{
						// Shouldn't happen.  If we got to a node we should be able to retrieve its info.
						TAE_ShowError("Requested info for a node but got an error!\n");
						bStillMatches = TAE_FALSE;
					}

					// After the end of the token see if there is a space.
					if (!TAEDictionary_NextNode(ptaedict, &taednCurNode, &iDictIndex, ' '))
					{
						// There is no space after the current node, so the match ends here.
						bStillMatches = TAE_FALSE;
					}

					// See if we need to set a resume pos to return to after the match fails.
					if ((iTokenStart == iMatchStartPos) && (iResumePos == -1))
					{
						// If we just started a match, note that we should resume the search at the
						// current location if the current match fails.
						iResumePos = ptaetext->lTextPos;
					}
				}

				// If we are in runout reset the run values.
				if (bInRunout)
				{
					iTokensInCurrRun = 0;
					bStillMatches = TAE_TRUE;
					bInRunout = TAE_FALSE;
				}
			}
			// Reset the token values.
			iTokenStart = -1;
			iTokenEnd = -1;
		}
		else if (c != '\0')
		{
			// We found a meaningful character.
			if ((iTokenStart > -1) && !bStillMatches)
			{
				// We are in the middle of a token, but the token no longer matches.  Run it out.
			}
			else
			{
				// Either we are at the start of a new token or we are in the middle of a
				// token and we still have a match in progress.
				if (iTokenStart == -1)
				{
					// This character is the start of a new token.  Note its starting point.
					iTokenStart = ptaetext->lCharPos;
				}

				// See if the current character matches.
				if (TAEDictionary_NextNode(ptaedict, &taednCurNode, &iDictIndex, (char)TAE_ToLower(c)))
				{
					// This character and any previous matches before it are in the dictionary.
					bStillMatches = TAE_TRUE;
					if (iMatchStartPos == -1)
					{
						// There is no current match in progress, start a new match.
						iMatchStartPos = ptaetext->lCharPos;
					}
				}
				else
				{
					// The current character is not in the dictionary.  Reset things.
					if (bStillMatches && (iTokensInCurrRun == 0))
					{
						// We are in the middle of the first token in a run, everything
						// in this token previously matched but the current character 
						// doesn't match.  Therefore we are in runout mode.
						bInRunout = TAE_TRUE;
						// The match failed before a complete match was round.  Reset the match indices.
						iMatchStartPos = -1;
						iMatchEndPos = -1;
					}
					// Set the flag to indicate text no longer matches.
					bStillMatches = TAE_FALSE;
					// Reset dictionary index for next search.
					iDictIndex = ptaedict->iDictionaryRootPos;
				}
			}
		}
		else
		{
			// Current character is end of text.  Any match in progress ends here.
			bStillMatches = TAE_FALSE;
		}

		// This "if" is reached once for each character, but we only need to do something if:
		// 1) a match was previously started, 2) the current character doesn't match and
		// 3) there is a location to resume the search.  If bStillMatches is false and the
		// resume pos is -1 then we never had a match and we just keep going from where we are.
		if (!bStillMatches && (iResumePos > -1))
		{
			if (nMatchedCollectionID != kNoEndMatchID)
			{
				// We have found an end match.  Process the match.
				tae_bool			 bCountMatch = TAE_TRUE;

				#ifdef ALLOW_OVERLAPPING_MATCHES
				// Even if we allow overlapping matches, we still don't count matches that
				// completely overlap ("a b c" will match "a b" and "b c" but not "b" which
				// is totally enclosed in "a b").
				int					iLastMatchEnd = 0;
				if (ptaeallmatches->iNumMatches > 0)
				{
					iLastMatchEnd = ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches - 1].lStart +
									ptaeallmatches->ptaematches[ptaeallmatches->iNumMatches - 1].lLength;
				}
				if (iMatchEndPos <= iLastMatchEnd)
				{
					// The current match ends before the last match, so it is totally enveloped by it.
					// Even with overlapping we don't count matches that completely overlap.
					bCountMatch = TAE_FALSE;
				}
				#endif

				// I'm not particularly happy with this, but as a quick and dirty solution to the problem
				// of the name "Dick" matching we maintain a hard-coded list of words that should not be
				// matched and if the TAE_NOMATCHSAFECAP option is specified we check against this list
				// and do not match if that item is in the list.
				if (bCountMatch && (iTokensInMatch == 1) && ((lOptions & TAE_MATCHSAFECAP) == 0))
				{
					// We are not matching words that are safe when capitalized.
					int					i = 0;
					tae_bool			bFound = TAE_FALSE;
					for (i = 0; (i < kNumSafeCaps) && !bFound; ++i)
					{
						if (strncmp(ptaetext->szText + iMatchStartPos, cSafeIfCapitalized[i], iMatchEndPos - iMatchStartPos) == 0)
						{
							bFound = TAE_TRUE;
							bCountMatch = TAE_FALSE;
						}
					}
				}

				// process the user-specified whitelist, just like the fixed whitelist above
				if (whitelist && bCountMatch && (iTokensInMatch == 1))
				{
					int					i = 0;
					tae_bool			bFound = TAE_FALSE;
					for (i = 0; whitelist[i]!=NULL && !bFound; ++i)
					{
						if (strnicmp(ptaetext->szText + iMatchStartPos, whitelist[i], iMatchEndPos - iMatchStartPos) == 0)
						{
							bFound = TAE_TRUE;
							bCountMatch = TAE_FALSE;
						}
					}
				}

				if (bCountMatch)
				{
					// Process the match.
					TAEDictionary_ProcessMatch(ptaedict, ptaetext, ptaeallmatches,
												iMatchStartPos, iMatchEndPos, nMatchedCollectionID);
					++ptaetext->iNumMatches;
				}

			}

			// Reset the dictionary pointer.
			iDictIndex = ptaedict->iDictionaryRootPos;
			// If there is a resume pos, go there and reset.
			if (iResumePos > -1)
			{
				#ifdef ALLOW_OVERLAPPING_MATCHES
				// Correct the total word count by subtracting out any overlap.
				ptaetext->iNumTokens -= (iTokensInMatch - 1);
				#else
				// If necessary, adjust the number of tokens.
				if ((iTokensInCurrRun > 1) && (iTokensInCurrRun != iTokensInMatch))
				{
					// If there is only one token in the current run we are only advancing
					// by one token so no adjustment is necessary.  If the number of tokens in
					// the current run equals the number of tokens in the match the entire
					// run is a match and again no adjustment is necessary.

					// Correct the total word count by subtracting out any overlap.
					ptaetext->iNumTokens -= (iTokensInCurrRun - iTokensInMatch - 1);
				}
				#endif

				// Housekeeping.  Tell the text it already sent a space so it won't
				// send another space right away.
				ptaetext->bSentSpace = TAE_TRUE;

				// Reset the text pointer to resume at the desired point.
				ptaetext->lTextPos = iResumePos;
				iResumePos = -1;
			}

			if (nMatchedCollectionID != kNoEndMatchID)
			{
				iTokensInMatch = 0;
				// Reset the matching collection.
				nMatchedCollectionID = kNoEndMatchID;
			}

			// Clear the last match.
			iMatchStartPos = -1;
			iMatchEndPos = -1;

			// End the current token run.
			iTokenStart = -1;
			iTokenEnd = -1;
			iTokensInCurrRun = 0;

			// Reset the matching collection.
			nMatchedCollectionID = kNoEndMatchID;

			bStillMatches = TAE_TRUE;
		}
	} while (c != '\0');

	return TAE_OK;
}


/*
 * Reset the scoring data.
 *
 */
int TAEDictionary_ResetScore(struct TAEDictionary *ptaedict)
{
	struct TAECollection	*ptaecollection = NULL;
	int						 i = 0;

	if (!ptaedict)
	{
		return TAE_UNEXPECTEDNULL;
	}

	for (i = 0; i < ptaedict->taevecCollections.iSize; ++i)
	{
		ptaecollection = (struct TAECollection *)TAEVector_ElementAt(&(ptaedict->taevecCollections), i);
		if (ptaecollection)
		{
			ptaecollection->iHitCount = 0;
			ptaecollection->iFreq = 0;
		}
	}

	return TAE_OK;
}


/*
 * Tally one more hit for the TAECollection struct that corresponds to
 * the specified TAECollection.
 *
 */
int TAEDictionary_TallyOneHit(struct TAEDictionary *ptaedict, struct TAECollection *ptaecolDict)
{
	tae_bool				 bDone = TAE_FALSE;
	struct TAECollection	*ptaecol = NULL;
	int						 i = 0;

	if (!ptaedict)
	{
		return TAE_UNEXPECTEDNULL;
	}

	for (i = 0 ; (i < ptaedict->taevecCollections.iSize) && !bDone; ++i)
	{
		ptaecol = (struct TAECollection *)TAEVector_ElementAt(&(ptaedict->taevecCollections), i);
		if (ptaecol && (ptaecol->iCollectionID == ptaecolDict->iCollectionID))
		{
			ptaecol->iHitCount += 1;
			bDone = TAE_TRUE;
		}
	}
	return TAE_OK;
}

/*
 * Take the raw hit data and convert it into frequency scores.
 *
 */
int TAEDictionary_ComputeFreqScore(struct TAEDictionary *ptaedict, struct TAEText *ptaetext)
{
	struct TAECollection	*ptaecol = NULL;
	int						 i = 0;

	if (!ptaedict || !ptaetext)
	{
		return TAE_UNEXPECTEDNULL;
	}

	for (i = 0; i < ptaedict->taevecCollections.iSize; ++i)
	{
		ptaecol = (struct TAECollection *)TAEVector_ElementAt(&(ptaedict->taevecCollections), i);
		if (ptaecol)
		{
			if (ptaetext->iTotalTokens > 0)
			{
				ptaecol->iFreq = ((ptaecol->iHitCount  * 100000) / ptaetext->iTotalTokens);
			}
		}
	}

	return TAE_OK;
}

#ifdef SHOW_DEBUG_OUTPUT
/*
 * This is for debugging only.  Use this to print a text list of all of the nodes.
 *
 */
void TAEDictionary_ShowAllNodes(struct TAEDictionary *ptaedict)
{
	if (ptaedict)
	{
		int							 nNodeID = 0;
		short						 nTotalEdges = 0;
		short						 nEndMatchCollectionID = 0;
		unsigned int				 iCurrPos = ptaedict->lDataStart;

		for (nNodeID = 0; nNodeID < ptaedict->iNumNodes; nNodeID++)
		{
			printf("0x%08x\n", iCurrPos);
			//	Read in the number of edges
			nTotalEdges = TAEStream_ReadShortAt(&(ptaedict->taestream), &iCurrPos);

			//	Read in the end match collection ID if it exists
			if ((nTotalEdges & kEndMatchToRead) != 0)
			{
				nEndMatchCollectionID = TAEStream_ReadShortAt(&(ptaedict->taestream), &iCurrPos);
			}
			
			printf("%04d\n", nTotalEdges);

			//	Discard the end match exists bit from the number of edges
			nTotalEdges &= kNumberEdgesMask;

			if ((nTotalEdges < 0) || (nTotalEdges > TAE_MAXSHORT))
			{
				return;
			}
			else if (nTotalEdges > 0)
			{
				//	Read in the edges
				int							 i = 0;
				char						 cEdgeCharacter = '\0';
				int							 iDestinationNodeID = 0;

				for (i = 0; i < nTotalEdges; i++)
				{
					cEdgeCharacter = TAEStream_ReadCharAt(&(ptaedict->taestream), &iCurrPos);
					iDestinationNodeID = TAEStream_ReadIntAt(&(ptaedict->taestream), &iCurrPos);

					printf("     %02X %08X\n", cEdgeCharacter, iDestinationNodeID + ptaedict->lDataStart);
				}
			}
		}
		printf("Showed %d nodes\n", nNodeID);
	}
}
#endif
