// TAEText.h

#ifndef _TAETEXT_
#define _TAETEXT_

#include "TAECommon.h"
#include "TAEDictionary.h"


//////////////////////////////////////////////////////////////////
// TAEText - Structure and methods for processing a piece of text.
//////////////////////////////////////////////////////////////////

typedef struct TAEText
{
	char					*szText;
	long					 lTextLen;
	unsigned int			 iNumTokens;
	int						 iTotalTokens;
	int						 iNumMatches;
	long					 lTextPos;
	long					 lCharPos;
	char					 cCurrChar;
	char					 cCurrCharType;
	tae_bool				 bSentSpace;
} TAEText;

typedef struct TAEDictionary TAEDictionary;

struct TAEText				*TAEText_New();
void						 TAEText_Delete(struct TAEText *ptaetext);

int							 TAEText_ResetText(struct TAEText *ptaetext,
											   char *szText,
											   long lTextLen);
int							 TAEText_AnalyzeText(struct TAEText *ptaetext,
												 TAEDictionary *ptaedict,
												 struct TAEAllMatches *ptaeallmatches,
												 unsigned long lOptions,
												 char **whitelist);
unsigned char				 TAEText_NextChar(struct TAEText *ptaetext,
											  unsigned long lOptions);

#endif	// #ifndef _TAETEXT_
