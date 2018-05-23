// TAE.h

#ifndef _TAE_
#define _TAE_

#if defined(macintosh)
#include <MacTypes.h>
#include <Files.h>
#include <Memory.h>
#endif


// The basic process for analyzing a text is:

/*
	struct TAEDictState		taeds;
	if (TAEInitDictionary(&taeds, "dict.dat"))
	{
		struct TAESessionState	 taess;
		if (TAEStartSession(&taess, &taeds))
		{
			// To keep the overall score:
			int							 iScore = 0;
			// Optional: Use this only if you want detailed score info.
			struct TAEScoreData			 taesd;

			// Call this as many times as you like with different
			// sections of the text.  Don't pass in the same text more
			// than once or pass in overlapping portions of text or
			// the scoring will not be accurate.
			TAEProcessText(&taess, cText);

			// Get the score data.  Do this after you are done calling
			// TAEProcessText() and before you call TAEEndSession().
			// If you do not care about the detailed scoring data (most of
			// the time you won't) simply pass NULL for the second parameter.
			iScore = TAEGetScoreData(&taess, &taesd);

			printf("\n\nFlame level: ");
			switch (iScore)
			{
				case 3: printf("high\n"); break;
				case 2: printf("medium\n"); break;
				case 1: printf("low\n"); break;
				case 0: printf("none\n"); break;
			}

			TAEFreeScoreData(&taesd);

			TAEEndSession(&taess);
		}
		TAECloseDictionary(&taeds);
	}
*/


// Include publicly available definitions.
#include "TAEPublic.h"


// Structure for holding the state of the dictionary.
// 
// The calling code does not need to know anything about the contents of 
// this structure, it just needs to keep a reference to this and pass it 
// to any functions that require it.
// 
// If you want to use the same dictionary in more than one session, this 
// structure reference can be shared between any number of sessions.  If 
// you want to use more than one dictionary you will need to have a 
// separate copy of this structure for each dictionary.
struct TAEDictState
{
	void					*pvdict;				// Pointer to a TAEDictionary struct.
													// Typed as a void* so we don't need
													// to include any other TAE files.
};

// Structure for holding the state of a text analysis session.
// 
// The calling code does not need to know anything about the contents of 
// this structure, it just needs to keep a reference to this and pass it 
// to any functions that require it.
// 
// The calling code needs to maintain a separate copy of this structure
// for each text analysis session that is happening at a given time.
struct TAESessionState
{
	struct TAEDictState		*ptaedictstate;
	void					*pvtext;
};

// Structure for holding the detailed information about a session score.
// 
// This structure is used by the TAEScoreData structure and the calling 
// code will never create an instance of this.  The calling code may
// want to examine the fields of this structure for more detailed information
// about the session scoring.
struct TAEScoreData
{
	int							 iNumCollections;
	struct TAECollectionScore	*ptaecolscore;
};


// Call once per Eudora session per type of dictionary (mood mail, etc.) to initialize the dictionary.
// Input:
//		struct TAEDictState *ptaeds - A valid pointer to a TAEDictState struct.
//			This call will fill this struct in the data for the specified dictionary.
//			This structure will be needed as the parameter for calls to TAECloseDictionary()
//			and TAEStartSession().  This pointer may be shared by any number of TAE sessions
//			(that is, you may pass this as a parameter to any number of calls to TAEStartSession()).
//		char *szDictName - A valid pointer to a string containing the name of the dictionary
//			to open.  In the absence of a full path name, the current working directory is assumed.
// Output:
//		struct TAEDictState *ptaeds - This structure will be filled in with the data for the
//			specified dictionary.
// Return value:
//		Returns 1 if initialization was successful, 0 otherwise.
#if defined(macintosh)
int						 TAEInitDictionary(struct TAEDictState *ptaeds, FSSpec *dictSpec);
#else
int						 TAEInitDictionary(struct TAEDictState *ptaeds, char *szDictName);
#endif

// Call once per opened dictionary to close the dictionary when it is no longer needed.
// Input:
//		struct TAEDictState *ptaeds - A valid pointer to a TAEDictState struct to be closed.
// Output:
//		struct TAEDictState *ptaeds - This pointer no longer contains valid data.
// Return value:
//		Returns 1 if the closing was successful, 0 otherwise.
int						 TAECloseDictionary(struct TAEDictState *ptaeds);


// Call to set the starting point of the dictionary data.
void					 TAESetDictionaryDataLocation(struct TAEDictState *ptaeds,
													  void *pdata);

// Call to get the starting point of the dictionary data.
void					*TAEGetDictionaryDataLocation(struct TAEDictState *ptaeds);

// Call to get the length of the data allocated for the dictionary data.
int						 TAEGetDictionaryDataSize(struct TAEDictState *ptaeds);


// Call before passing match structure to TAEProcessText().
void					 TAEInitAllMatches(struct TAEAllMatches *ptaeallmatches);

// Call after you are done with the match structure filled in by TAEProcessText().
void					 TAEFreeAllMatches(struct TAEAllMatches *ptaeallmatches);

// Initialize the score data.
void					 TAEInitScoreData(struct TAEScoreData *ptaesd);

// Free the score data.
void					 TAEFreeScoreData(struct TAEScoreData *ptaesd);


// Call once when you are about to begin processing a new text.  This call will reset all current 
// processing and scoring data, so call this
// 
// Even if you are calling TAEProcessText() on multiple blocks
// within the same text, call this only once per 
int						 TAEStartSession(struct TAESessionState	*ptaess, struct TAEDictState *ptaeds);

// Call this any number of times to process pieces of a text.  The state and scoring information
// will be preserved between calls.  If the calling code calls this routine more than once using 
// the same piece of text or a piece of text that "overlaps" a previous piece of text the statistics
// and scoring will not be accurate.
int						 TAEProcessText(struct TAESessionState *ptaess, char *szText, long lTextLen, 
										struct TAEAllMatches *ptaeallmatches, unsigned long lOptions,
										char **whitelist);

// Obtain the score data.
int						 TAEGetScoreData(struct TAESessionState *ptaess, struct TAEScoreData *ptaesd,
										 short scores[][7], short nNumEntries);

// Internal scoring routine.
short					 TAE_AnalScore(int h, int nonH, int total, short scoreArray[][7], short nNumEntries);

// Free the score data.
void					 TAEFreeScoreData(struct TAEScoreData *ptaesd);

// End a session.
int						 TAEEndSession(struct TAESessionState *ptaess);

#endif	// #ifndef _TAE_
