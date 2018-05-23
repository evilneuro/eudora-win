// TAEDictionary.h

#ifndef _TAEDICTIONARY_
#define _TAEDICTIONARY_

#if defined(macintosh)
#include <MacTypes.h>
#include <Files.h>
#include <Memory.h>
#endif

#include "TAECommon.h"

#include "TAEText.h"


#define kClusterObject				0x0000
#define kCategoryObject				0x0001
#define kCollectionObject			0x0002
#define kEndOfObjects				0x0003

#define kNumberEdgesMask			0x7FFF
#define kEndMatchToRead				0x8000


#define kNoEndMatchID				-1
#define kNextDictionaryNodeID		 0

#define kEndMatchCollectionIDMask	((short)0x7FFF)

typedef struct TAEText TAEText;


//////////////////////////////////////////////////////////////////
// TAEDictionaryNode - Structure for holding dictionary node info.
//////////////////////////////////////////////////////////////////

struct TAEDictionaryNode
{
	short						 nTotalEdges;
	short						 nNodeInfo;
	unsigned int				 iDestNodeID;
};


////////////////////////////////////////////////////////////////
// TAEDictionary - Structure and methods for dictionary actions.
////////////////////////////////////////////////////////////////

typedef struct TAEDictionary
{
	struct TAEVector			 taevecCollections;
	struct TAEStream			 taestream;
	int							 iNumNodes;
	unsigned int				 iDictionaryRootPos;
	unsigned long				 lDataStart;
	unsigned int				 iDataLen;
} TAEDictionary;

#if defined(macintosh)
struct TAEDictionary			*TAEDictionary_New(FSSpecPtr dictSpec);
#else
struct TAEDictionary			*TAEDictionary_New(char *szDictFileName);
#endif
void							 TAEDictionary_Delete(struct TAEDictionary *ptaedict);

// Dictionary relocations functions
void							 TAEDictionary_SetDictionaryDataLocation(struct TAEDictionary *ptaedict,
																		 void *pdata);
void							*TAEDictionary_GetDictionaryDataLocation(struct TAEDictionary *ptaedict);
int								 TAEDictionary_GetDictionaryDataSize(struct TAEDictionary *ptaedict);
// Dictionary initialization functions
#if defined(macintosh)
int								 TAEDictionary_ReadCompiledDictionary(struct TAEDictionary *ptaedict,
																	  FSSpecPtr pDictSpec);
#else
int								 TAEDictionary_ReadCompiledDictionary(struct TAEDictionary *ptaedict,
																	  char *szDictFileName);

#endif
int								 TAEDictionary_ReattachNodes(struct TAEDictionary *ptaedict);

// Dictionary navigation functions
int								 TAEDictionary_NodeAt(struct TAEDictionary *ptaedict, 
													  struct TAEDictionaryNode *ptaedn, 
													  unsigned int *iPos);

int								 TAEDictionary_NextNode(struct TAEDictionary *ptaedict, 
														struct TAEDictionaryNode *ptaedn, 
														unsigned int *iPos, 
														char c);

// Dictionary matching functions
int								 TAEDictionary_InitMatches(struct TAEDictionary *ptaedict,
															TAEText *ptaetext,
															struct TAEAllMatches **ptaematchlist);
void							 TAEDictionary_ProcessMatch(struct TAEDictionary *ptaedict,
															TAEText *ptaetext,
															struct TAEAllMatches *ptaematchlist,
															int iMatchStartPos,
															int iMatchEndPos,
															short nLastMatchedCollectionID);
int								 TAEDictionary_CategorizeWordsAndPhrases(struct TAEDictionary *ptaedict, 
																		 TAEText *ptaetext,
																		 struct TAEAllMatches *ptaematchlist,
																		 unsigned long lOptions,
																		 char **whitelist);

// Scoring functions
int								 TAEDictionary_ResetScore(struct TAEDictionary *ptaedict);
int								 TAEDictionary_TallyOneHit(struct TAEDictionary *ptaedict,
														   struct TAECollection *ptaecolDict);
int								 TAEDictionary_ComputeFreqScore(struct TAEDictionary *ptaedict,
																TAEText *ptaetext);

#ifdef SHOW_DEBUG_OUTPUT
void							 TAEDictionary_ShowAllNodes(struct TAEDictionary *ptaedict);
#endif

#endif	// #ifndef _TAEDICTIONARY_
