// TAEPublic.h

#ifndef _TAEPUBLIC_
#define _TAEPUBLIC_


// Function return values
#define TAE_OK				0
#define TAE_UNKNOWN			1
#define TAE_OUTOFMEMORY		2
#define TAE_UNEXPECTEDNULL	3
#define TAE_DICTCORRUPTED	4
#define TAE_DICTNOTFOUND	5
#define TAE_DICTBADVERSION	6


// Processing options
#define TAE_IGNORESAFETEXT		0x0001		// Ignore total of tokens in a pass if no matches are found
#define TAE_MATCHSAFECAP		0x0002		// Match words even if they are considered safe if capitalized
#define TAE_CONTAINSHTML		0x0004		// Text contains HTML tags and formatting characters
#define TAE_NOIGNOREHTMLTAGS	0x0008		// Do count contents of HTML tags as part of the text
// Currently unsupported options
#define TAE_DOTOKENMATCH		0x0010		// Do token based processing rather than character based


typedef short	tae_bool;

#define TAE_FALSE		0
#define TAE_TRUE		1


#define TAECollectionScore	TAECollection


// TAECollection - Structure for holding data for a collection.
struct TAECollection
{
	int						 iCollectionID;
	int						 iTypeID;
	int						 iCategoryID;
	int						 iHitCount;
	int						 iFreq;
};


// TAEMatch - Structure for holding the information about words or phrases 
// within the text that matched words or phrases in the dictionary.
struct TAEMatch
{
	long					 lStart;
	long					 lLength;
	short					 nCollection;
};


// TAEAllMatches - Structure for holding the list of all matching words and phrases.
struct TAEAllMatches
{
	struct TAEMatch			*ptaematches;
	int						 iNumMatches;
	int						 iMaxMatches;
};


#endif	// #ifndef _TAEPUBLIC_
