// TAEText.c

#include "TAEText.h"


#define		MAX_CHAR		256

#define		CT_UNKNOWN		-1
#define		CT_WORDCHAR		0
#define		CT_WHITESPACE	1
#define		CT_PUNCTUATION	2

char cCharType[MAX_CHAR] = {-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1,
							-1, -1, -1, -1, -1, -1, -1, -1};

#define TAE_CharType(c)			((c < MAX_CHAR) ? cCharType[c] : CT_UNKNOWN)


#define HTML_CHAR_ENTRIES		4

char cHTMLCharEntries[HTML_CHAR_ENTRIES][2][5] = {{"amp", "&"},
												  {"quot", "\""},
												  {"lt", "<"},
												  {"gt", ">"}};


//////////////////////////////////////////////////////////////////
// TAEText - Structure and methods for processing a piece of text.
//////////////////////////////////////////////////////////////////

struct TAEText *TAEText_New()
{
	struct TAEText *ptaetext = (struct TAEText *)TAE_malloc(sizeof(struct TAEText));
	if (ptaetext)
	{
		ptaetext->szText = NULL;
		ptaetext->iNumTokens = 0;
		ptaetext->iTotalTokens = 0;
		ptaetext->iNumMatches = 0;
		ptaetext->lTextLen = 0;
		ptaetext->lTextPos = -1;
		ptaetext->lCharPos = -1;
		ptaetext->bSentSpace = TAE_FALSE;
		ptaetext->cCurrChar = -1;
		ptaetext->cCurrCharType = -1;

		// If we haven't already initialized the char type table, do it now.
		if (cCharType[0] == -1)
		{
			int			i = 0;

			for (i = 0; i < MAX_CHAR; ++i)
			{
				if (TAE_IsLetter((char)i))
				{
					// Character is considered a letter.
					cCharType[i] = CT_WORDCHAR;
				}
				else if ((i >= '0') && (i <= '9'))
				{
					// Character is a digit.
					cCharType[i] = CT_WORDCHAR;
				}
				else if (i <= ' ')
				{
					// Unprintable characters and the space character are considered whitespace.
					cCharType[i] = CT_WHITESPACE;
				}
				else
				{
					// Character is anything else (essentially punctuation).
					cCharType[i] = CT_PUNCTUATION;
				}
			}
		}
	}
	return ptaetext;
}

void TAEText_Delete(struct TAEText *ptaetext)
{
	if (ptaetext)
	{
		TAE_free(ptaetext);
		ptaetext = NULL;
	}
}

int TAEText_ResetText(struct TAEText *ptaetext, char *szText, long lTextLen)
{
	if (!ptaetext || !szText)
	{
		return TAE_UNEXPECTEDNULL;
	}

	ptaetext->szText = szText;
	ptaetext->lTextPos = -1;
	ptaetext->lCharPos = -1;
	ptaetext->iNumTokens = 0;
	ptaetext->lTextLen = lTextLen;
	ptaetext->bSentSpace = TAE_FALSE;

	return TAE_OK;
}

int TAEText_AnalyzeText(struct TAEText *ptaetext,
						struct TAEDictionary *ptaedict,
						struct TAEAllMatches *ptaeallmatches,
						unsigned long lOptions,
						char **whitelist)
{
	int			iReturn = 0;

	if (!ptaetext || !ptaedict)
	{
		return TAE_UNEXPECTEDNULL;
	}

	iReturn = TAEDictionary_CategorizeWordsAndPhrases(ptaedict,
													  ptaetext,
													  ptaeallmatches,
													  lOptions,
													  whitelist);

	if ((lOptions & TAE_IGNORESAFETEXT) && (ptaetext->iNumMatches == 0))
	{
		// Don't add in the number of tokens if we are ignoring safe text
		// and there are no matches in this text.
	}
	else
	{
		// Add the number of tokens in this text to the total.
		ptaetext->iTotalTokens += ptaetext->iNumTokens;
	}

	return iReturn;
}

/*
 * Return the next character.
 *
 */
unsigned char TAEText_NextChar(struct TAEText *ptaetext, unsigned long lOptions)
{
	unsigned char		c = '\0';
	char				cType = CT_UNKNOWN;
	char				cLastChar = '\0';
	char				cLastCharType = CT_UNKNOWN;
	long				lTextPos = -1;
	tae_bool			bSentSpace = TAE_FALSE;
	tae_bool			bHandled = TAE_FALSE;

	if (!ptaetext || !ptaetext->szText)
	{
		return '\0';
	}

	lTextPos = ptaetext->lTextPos;
	bSentSpace = ptaetext->bSentSpace;

	cLastChar = ptaetext->cCurrChar;
	cLastCharType = ptaetext->cCurrCharType;

	// There are some cases (e.g., the HTML coded character "&lt;") where the character pos differs
	// from the text pos, so we keep a separate index for this case.
	ptaetext->lCharPos = -1;

	do
	{
		++lTextPos;

		if (lTextPos < ptaetext->lTextLen)
		{
			c = ptaetext->szText[lTextPos];
			cType = (char)TAE_CharType(c);

			// The dictionary acknowledges two units of comparison: words and punctuation.  The delimiter
			// that separates words and punctuation is a space.
			// Any time we encounter two consecutive characters that are of different character types,
			// we send a space separator between them.
			// Exceptions: apostrophe

			// If we are at whitespace, eat it.
			if (cType == CT_WHITESPACE)
			{
				tae_bool		bAtEOF = TAE_FALSE;
				tae_bool		bIsWhiteSpace = TAE_TRUE;
				if (ptaetext->lCharPos == -1)
				{
					ptaetext->lCharPos = lTextPos - 1;
				}
				while (!bAtEOF && bIsWhiteSpace)
				{
					c = ptaetext->szText[++lTextPos];
					if (lTextPos == ptaetext->lTextLen)
					{
						bAtEOF = TAE_TRUE;
					}
					else
					{
						cType = (char)TAE_CharType(c);
						if (cType != CT_WHITESPACE)
						{
							bIsWhiteSpace = TAE_FALSE;
						}
					}
				}
				if (!bSentSpace)
				{
					// We haven't sent a space separator, send it now.
					c = ' ';
					bSentSpace = TAE_TRUE;
					bHandled = TAE_TRUE;
					--lTextPos;
				}
				else if (bAtEOF)
				{
					// We are at the end of the text, send a final space separator.
					c = ' ';
					bSentSpace = TAE_TRUE;
					bHandled = TAE_TRUE;
				}
			}

			if (!bSentSpace && (lTextPos > 0))
			{
				// We didn't just send a space separator.  See if we should.
				if ((c == '\'') && (cLastCharType == CT_WORDCHAR) &&
					((lTextPos + 1) < ptaetext->lTextLen) &&
					(TAE_CharType(ptaetext->szText[lTextPos + 1]) == CT_WORDCHAR))
				{
					// Current character is an apostrophe, just keep going.
				}
				else if ((cLastChar == '\'') && (cType == CT_WORDCHAR) &&
						 ((lTextPos - 2) >= 0) &&
						 (TAE_CharType(ptaetext->szText[lTextPos - 2]) == CT_WORDCHAR))
				{
					// Last character was an apostrophe, just keep going.
				}
				else if ((cType != cLastCharType) ||
						 ((cType == CT_PUNCTUATION) && (cLastCharType == CT_PUNCTUATION)))
				{
					// We are at a transition point -- either a point between two different types of
					// characters or between two punctuation marks.  Send a space separator
					c = ' ';
					bSentSpace = TAE_TRUE;
					bHandled = TAE_TRUE;
					--lTextPos;
				}
			}
			else if (!bHandled && (lOptions & TAE_CONTAINSHTML) && (c == '&'))
			{
				// If the text contains HTML and we found '&' indicating a special character.
				int					i = 0;
				long				lCharPos = lTextPos + 1;
				long				lCharLen = 0;

				for (i = lCharPos; (i < ptaetext->lTextLen) && (lCharLen == 0); ++i)
				{
					if (ptaetext->szText[i] == ';')
					{
						lCharLen = i - lCharPos;
					}
				}

				if (lCharLen > 0)
				{
					tae_bool			bFound = TAE_FALSE;
					for (i = 0; (i < HTML_CHAR_ENTRIES) && !bFound; ++i)
					{
						if (strncmp(ptaetext->szText + lCharPos, cHTMLCharEntries[i][0], lCharLen) == 0)
						{
							bFound = TAE_TRUE;
							c = cHTMLCharEntries[i][1][0];
							bSentSpace = TAE_FALSE;
							bHandled = TAE_TRUE;
						}
					}
					ptaetext->lCharPos = lTextPos;
					lTextPos += lCharLen + 1;
					if (!bFound)
					{
						// We found a special character that isn't one we handle.  Skip over it.
						continue;
					}
				}
			}
			else if (!bHandled && (lOptions & TAE_CONTAINSHTML) && ((lOptions & TAE_NOIGNOREHTMLTAGS) == 0) && (c == '<'))
			{
				// We have HTML, we want to remove HTML tags and we are at a '<'.
				// Skip over this tag.
				long				i = 0;
				long				lTagPos = lTextPos + 1;
				tae_bool			bDone = TAE_FALSE;
				tae_bool			bInsideQuote = TAE_FALSE;
				tae_bool			bInsideComment = TAE_FALSE;
				if ((lTagPos + 3 < ptaetext->lTextLen) && (ptaetext->szText[lTagPos] == '!') &&
					 (ptaetext->szText[lTagPos + 1] == '-') && (ptaetext->szText[lTagPos + 2] == '-'))
				{
					bInsideComment = TAE_TRUE;
				}
				for (i = lTagPos; (i < ptaetext->lTextLen) && !bDone; ++i)
				{
					if (!bInsideComment && (ptaetext->szText[i] == '"'))
					{
						bInsideQuote = (tae_bool)(bInsideQuote ? TAE_FALSE : TAE_TRUE);
					}
					if (!bInsideQuote && (ptaetext->szText[i] == '>'))
					{
						if (bInsideComment)
						{
							if( (ptaetext->szText[i - 1] == '-') && (ptaetext->szText[i - 2] == '-'))
							{
								bDone = TAE_TRUE;
							}
						}
						else
						{
							bDone = TAE_TRUE;
						}
					}
				}
				lTextPos = i - 1;
				continue;
			}

			if (!bHandled)
			{
				// If we reach here, we are not at a transition point -- either we just sent the space separator or
				// we are in the middle of a word.  Send the current character.
				bSentSpace = TAE_FALSE;
				bHandled = TAE_TRUE;
			}
		}
		else
		{
			// We are at the end of the text.
			if (bSentSpace)
			{
				// We already sent the space to end the token, send '\0'.
				c = '\0';
				bHandled = TAE_TRUE;
			}
			else
			{
				// Send the space separator to end the token.
				--lTextPos;
				c = ' ';
				bSentSpace = TAE_TRUE;
				bHandled = TAE_TRUE;
			}
		}
	} while (!bHandled);

	ptaetext->cCurrChar = c;
	ptaetext->cCurrCharType = cType;
	ptaetext->lTextPos = lTextPos;
	ptaetext->bSentSpace = bSentSpace;

	// If we haven't specified a char pos, assume it is the same as the text pos.
	if (ptaetext->lCharPos == -1)
	{
		ptaetext->lCharPos = lTextPos;
	}

	return c;
}
