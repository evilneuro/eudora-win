#include "TAE.h"

#include <stdio.h>

#include <string.h>

#include <malloc.h>

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#ifndef _timeb
#define _timeb timeb
#define _ftime ftime
#endif


#define kBufferSize		1000000


// To use an alternate scoring array:
// #define USE_ALT_SCORING

// To test the memory moving capability:
#define TEST_MEMORY_MOVE

main(int argc, char **argv)
{
	struct TAEDictState		taeds;

	// Diagnostics: Variables for timing
	struct _timeb	 tstruct;
	long			 lBeginTime = 0;
	long			 lEndTime = 0;

	// Diagnostics: Get start time
	_ftime( &tstruct );
	lBeginTime = tstruct.millitm;

	if (TAEInitDictionary(&taeds, "dict.dat"))
	{
		struct TAESessionState	 taess;
		FILE					*fileIn = NULL;
		char					 cText[kBufferSize];
		int						 i = 0;

		#ifdef TEST_MEMORY_MOVE
		int						 iDictionaryLength = 0;
		void					*pDictionaryData = NULL;
		void					*pDictionaryDataCopy = NULL;
		#endif

		#ifdef USE_ALT_SCORING
		short					 scoreArray[10][7] =	{/*	 H words	non-H		total		score */
															{1, 1000,	0, 0,		0, 0, 		1},
															{1, 2000,	0, 0,		0, 0, 		1},
															{1, 3000,	2, 1000,	0, 0, 		1},
															{1, 4000,	1, 1000,	0, 0,		1},
															{0, 0,		3, 5000,	0, 0,		1},
															{0, 0,		0, 0,		2, 50,		1},
															{0, 0,		0, 0,		3, 100,		1},
															{1, 4000,	0, 0,		0, 0,		1}
														};
		#endif

		// Diagnostics: Get end time
		_ftime( &tstruct );
		lEndTime = tstruct.millitm;
		printf("\nDone loading compact dictionary [load time: %f sec.]\n", ((double)(lEndTime - lBeginTime))/1000.0);

		#ifdef TEST_MEMORY_MOVE
		iDictionaryLength = TAEGetDictionaryDataSize(&taeds);
		pDictionaryData = TAEGetDictionaryDataLocation(&taeds);
		pDictionaryDataCopy = malloc(iDictionaryLength);
		if (pDictionaryDataCopy)
		{
			memcpy(pDictionaryDataCopy, pDictionaryData, iDictionaryLength);
			free(pDictionaryData);
			TAESetDictionaryDataLocation(&taeds, pDictionaryDataCopy);
		}
		#endif

		if (argc > 1)
		{
			fileIn = fopen(argv[1], "r");
		}
		else
		{
			fileIn = fopen("test.txt", "r");
		}

		if (!fileIn)
		{
			if (argc > 1)
			{
				printf("Error opening %s\n", argv[1]);
				return 0;
			}
			else
			{
				printf("Error opening test.txt\n");
				return 0;
			}
		}

		if (fileIn)
		{
			for (i = 0; i < kBufferSize; ++i)
			{
				if (fread(&(cText[i]), sizeof(unsigned char), 1, fileIn) == 0)
				{
					if (feof(fileIn))
					{
						cText[i] = '\0';
						break;
					}
				}
			}
			fclose(fileIn);
		}

		if (TAEStartSession(&taess, &taeds))
		{
			struct TAEScoreData			 taesd;
			int							 iScore = 0;
			int							 i = 0;
			char						*ptr = NULL;
			int							 idx = 0;

			struct TAEAllMatches		 taeallmatches;
			struct TAEMatch				*ptaematch;

			taeallmatches.iNumMatches = 0;
			taeallmatches.ptaematches = NULL;

			taesd.iNumCollections = 0;
			taesd.ptaecolscore = NULL;

			// Diagnostics: Variables for timing
			// Diagnostics: Get start time
			_ftime( &tstruct );
			lBeginTime = tstruct.millitm;

			TAEInitAllMatches(&taeallmatches);
			if (TAEProcessText(&taess, cText, strlen(cText), &taeallmatches,
							   /*TAE_CONTAINSHTML|TAE_NOIGNOREHTMLTAGS|TAE_MATCHSAFECAP||TAE_IGNORESAFETEXT*/0) == 0)
			{
				printf("Failure\n");
				return 0;
			}

			for (i = 0; i < taeallmatches.iNumMatches; ++i)
			{
				ptaematch = (struct TAEMatch *)(taeallmatches.ptaematches + i);
				if (ptaematch)
				{
					printf("Match: %ld %ld %d", ptaematch->lStart, ptaematch->lLength, ptaematch->nCollection);
					printf("  ->");
					for (ptr = &cText[ptaematch->lStart], idx = ptaematch->lLength; idx; idx--, ptr++)
					{
						putc(*ptr,stdout);
					}
					printf("<-\n");
				}
			}

			TAEFreeAllMatches(&taeallmatches);

			// Diagnostics: Get end time
			_ftime( &tstruct );
			lEndTime = tstruct.millitm;
			printf("[Total: %f sec, %f sec/word]", (lEndTime - lBeginTime)/1000.0, (lEndTime - lBeginTime)/1000.0);

			#ifdef USE_ALT_SCORING
			iScore = TAEGetScoreData(&taess, &taesd, scoreArray, 8);
			#else
			iScore = TAEGetScoreData(&taess, &taesd, NULL, 0);
			#endif
			printf("\n\nFlame level: ");
			switch (iScore)
			{
				case 3: printf("high\n"); break;
				case 2: printf("medium\n"); break;
				case 1: printf("low\n"); break;
				case 0: printf("none\n"); break;
			}
			if (1 || iScore > 0)
			{
				int							 i = 0;
				struct TAECollectionScore	*ptaecolscore = NULL;

				for (i = 0; i < taesd.iNumCollections; ++i)
				{
					ptaecolscore = (struct TAECollectionScore *)(taesd.ptaecolscore + i);
					if (ptaecolscore)
					{
						printf("Cluster: %d, Category: %d, Collection: %d, Hits: %d, Frequency: %d/100000 words\n", 
								taesd.ptaecolscore[i].iCategoryID,
								taesd.ptaecolscore[i].iTypeID,
								taesd.ptaecolscore[i].iCollectionID,
								taesd.ptaecolscore[i].iHitCount,
								taesd.ptaecolscore[i].iFreq);
					}
				}
			}

			TAEFreeScoreData(&taesd);

			TAEEndSession(&taess);
		}
		TAECloseDictionary(&taeds);
	}
	else
	{
		fprintf(stderr,"ERROR: failed to open dictionary\n");
	}
	return 0;
}
