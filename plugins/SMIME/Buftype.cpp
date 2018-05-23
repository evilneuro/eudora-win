/*
 * File: BUFTYPE.CPP
 *
 * Date: Wednesday, August 28, 1996
 *
 * Author: Scott Manjourides
 *
 * Copyright (c) 1996 QUALCOMM, Incorporated
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
 */
#include "stdafx.h"

#include <stdlib.h>
#include <string.h>

#include "BUFTYPE.H"

#define safefree(p) { if (p) { free(p); (p) = NULL; } }

#ifndef TRUE
#	define TRUE (1)
#endif

#ifndef FALSE
#	define FALSE (0)
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int resizebuf_buf(BufTypePtr dst, unsigned int newSize);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

BufTypePtr makesize_buf(unsigned int n)
{
	BufTypePtr bp = (BufTypePtr) malloc(sizeof(BufType));

	if (!bp)
		return (NULL);

	clear_buf(bp);

	bp->buf = (char *) malloc(n * sizeof(char));

	if (bp->buf)
		bp->buf_size = n;
	else
	{
		delete_buf(bp);
		return (NULL);
	}

	return (bp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

BufTypePtr make_buf()
{
	BufTypePtr bp = (BufTypePtr) malloc(sizeof(BufType));

	if (!bp)
		return (NULL);

	clear_buf(bp);

	return (bp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* boolean */ int isempty_buf(BufTypePtr bp)
{
	if ((!bp) || (bp->buf == NULL) || (bp->len == 0))
		return (TRUE);

	return (FALSE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int availspace_buf(BufTypePtr bp)
{
	if ((!bp) || (bp->buf == NULL))
		return (0);

	return (bp->buf_size - bp->len);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int buflen_buf(BufTypePtr bp) /* Number of valid chars in buffer. Always < bufsize_buf() */
{
	if (isempty_buf(bp))
		return (0);

	return (bp->len);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *getbuf_buf(BufTypePtr bp)
{
	if (!bp)
		return (NULL);

	return (bp->buf);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *getend_buf(BufTypePtr bp)
{
	if (isempty_buf(bp))
		return (NULL);

	return ((bp->buf) + (bp->len - 1));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int bufsize_buf(BufTypePtr bp)
{
	if (!bp)
		return (0);

	return (bp->buf_size);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* boolean */ int endspan_buf(BufTypePtr buf, BufTypePtr find)
{
	if (isempty_buf(buf) || isempty_buf(find))
		return (FALSE);

	char cEnd = *getend_buf(buf); /* buf->buf[buf->len - 1]; */
	char *pPos = getpos_buf(find); /* find->buf */
	char *pEnd = getend_buf(find); /* (find->buf + find->len - 1); */

	/* Return TRUE if the last char in 'buf' is anywhere in 'find' */
	while (pPos <= pEnd)
	{
		if (*pPos == cEnd)
			return (TRUE);

		pPos++;
	}

	return (FALSE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* boolean */ int setlen_buf(BufTypePtr bp, const unsigned int n)
{
	if ((!bp) || (bp->buf == NULL) || (n > (bp->buf_size)))
		return (FALSE);

	bp->len = n;

	return (TRUE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int bufncat_buf(BufTypePtr dst, BufTypePtr src, unsigned int n)
{
	if ((isempty_buf(src)) || (n < 1) || (!dst))
		return (0);

	unsigned int oldSize = buflen_buf(dst);
	unsigned int addSize = poslen_buf(src);

	if (n < addSize)
		addSize = n;

	unsigned int newSize = oldSize + addSize;

	if (newSize > bufsize_buf(dst))
	{
		if (!resizebuf_buf(dst, newSize))
			return (0);
	}

	if (oldSize == 0)
		dst->pos = dst->buf;

	char *cpDst, *cpSrc = getpos_buf(src);
	unsigned int nCopy = addSize;

	if (oldSize > 0)
		cpDst = getend_buf(dst) + 1;
	else
		cpDst = getpos_buf(dst);

	while (nCopy--)
		*cpDst++ = *cpSrc++;

	incpos_buf(src, addSize);
	dst->len = newSize;

	return (addSize);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int bufins_buf(BufTypePtr dst, BufTypePtr src)
{
	if (isempty_buf(src))
		return (0);

	unsigned int oldLen = poslen_buf(dst);
	unsigned int bufSize = bufsize_buf(dst);
	unsigned int addLen = poslen_buf(src);
	unsigned int newLen = oldLen + addLen;

	if (newLen > bufSize)
		addLen = bufSize - oldLen;

	if (addLen < 1)
		return (0); /* Buffer is already full */

	return (bufncat_buf(dst, src, addLen));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int bufnins_buf(BufTypePtr dst, BufTypePtr src, unsigned int n)
{
	if ((isempty_buf(src)) || (n < 1))
		return (0);

	unsigned int oldLen = poslen_buf(dst);
	unsigned int bufSize = bufsize_buf(dst);
	unsigned int addLen = poslen_buf(src);
	unsigned int newLen = oldLen + addLen;

	if (newLen > bufSize)
		addLen = bufSize - oldLen;

	if (n < addLen)
		addLen = n;

	if (addLen < 1)
		return (0); /* Buffer is already full */

	return (bufncat_buf(dst, src, addLen));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int poslen_buf(BufTypePtr bp) /* Number of valid chars, starting at current position */
{
	if ((isempty_buf(bp)) || ((bp->pos) == NULL))
		return (0);

	return ( (bp->len) - ((bp->pos) - (bp->buf)) );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void incpos_buf(BufTypePtr bp, unsigned int n)
{
	if (!bp)
		return;
	
	if (poslen_buf(bp) > n)
		bp->pos += n;
	else
		bp->pos = NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void delpos_buf(BufTypePtr bp)
{
	if (!bp)
		return;

	bp->pos = NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void resetpos_buf(BufTypePtr bp)
{
	if (bp)
		bp->pos = bp->buf;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *getpos_buf(BufTypePtr bp)
{
	if (!bp)
		return (NULL);

	if ((bp->pos) == NULL)
		bp->pos = bp->buf;
	
	return (bp->pos);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int strcpy_buf(BufTypePtr dst, const char *src)
{
	if ((!src) || (!dst))
		return (0);

	unsigned int nLen = strlen(src);

	if (nLen > 0)
	{
		if (nLen > bufsize_buf(dst))
		{
			if (!resizebuf_buf(dst, nLen))
				return (0);
		}

		char *cp = dst->buf;

		while (*src)
			*cp++ = *src++;

		dst->pos = dst->buf;
	}
	else /* nLen == 0 */
	{
		dst->pos = NULL;
	}

	dst->len = nLen;

	return (nLen);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int strncpy_buf(BufTypePtr dst, const char *src, const unsigned int n)
{
	if ((!src) || (!dst))
		return (0);

	unsigned int nLen = strlen(src);

	if (n < nLen)
		nLen = n;

	if (nLen > 0)
	{
		if (nLen > bufsize_buf(dst))
		{
			if (!resizebuf_buf(dst, nLen))
				return (0);
		}

		char *cp = dst->buf;

		while (*src)
			*cp++ = *src++;

		dst->pos = dst->buf;
	}
	else /* nLen == 0 */
	{
		dst->pos = NULL;
	}

	dst->len = nLen;

	return (nLen);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int strcat_buf(BufTypePtr dst, const char *src)
{
	if ((!src) || (!dst))
		return (0);

	unsigned int oldSize = buflen_buf(dst);
	unsigned int addSize = strlen(src);
	unsigned int newSize = oldSize + addSize;

	if (newSize > bufsize_buf(dst))
	{
		if (!resizebuf_buf(dst, newSize))
			return (0);
	}

	if (oldSize == 0)
		dst->pos = dst->buf;

	char *cpDst, *cpSrc = (char *) src;
	unsigned int nCopy = addSize;

	if (oldSize > 0)
		cpDst = getend_buf(dst) + 1;
	else
		cpDst = getpos_buf(dst);

	while (nCopy--)
		*cpDst++ = *cpSrc++;

	dst->len = newSize;

	return (addSize);

}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned int strncat_buf(BufTypePtr dst, const char *src, const unsigned int n)
{
	if ((!src) || (!dst))
		return (0);

	unsigned int oldSize = buflen_buf(dst);
	unsigned int addSize = strlen(src);
	
	if (n < addSize)
		addSize = n;
	
	unsigned int newSize = oldSize + addSize;

	if (newSize > bufsize_buf(dst))
	{
		if (!resizebuf_buf(dst, newSize))
			return (0);
	}

	if (oldSize == 0)
		dst->pos = dst->buf;

	char *cpDst, *cpSrc = (char *) src;
	unsigned int nCopy = addSize;

	if (oldSize > 0)
		cpDst = getend_buf(dst) + 1;
	else
		cpDst = getpos_buf(dst);

	while (nCopy--)
		*cpDst++ = *cpSrc++;

	dst->len = newSize;

	return (addSize);

}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void delete_buf(BufTypePtr bp)
{
	if (!bp)
		return;

	free_buf(bp);
	safefree(bp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void free_buf(BufTypePtr bp)
{
	if (!bp)
		return;

	safefree(bp->buf);
	clear_buf(bp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void clear_buf(BufTypePtr bp)
{
	if (!bp)
		return;

	bp->buf = NULL;
	bp->buf_size = 0;

	emptybuf_buf(bp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void emptybuf_buf(BufTypePtr bp)
{
	if (!bp)
		return;

	bp->pos = bp->buf;
	bp->len = 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


static /* boolean */ int resizebuf_buf(BufTypePtr bp, unsigned int newSize)
{
	unsigned int oldSize = bp->buf_size;

	if (oldSize == newSize)
		return (TRUE);
	
	char *tmpPtr = (char *) realloc(bp->buf, newSize);

	if (tmpPtr == NULL)
		return (FALSE);

	bp->buf = tmpPtr;
	bp->buf_size = newSize;

	if (newSize < oldSize) /* Getting smaller */
	{
		/* Check for valid length */
		if (bp->len > newSize)
			bp->len = newSize;

		/* Check pos */
		unsigned int nPos = ((bp->pos) - (bp->buf));
		if (nPos >= bp->len)
			bp->pos = NULL;
	}
	else if (oldSize == 0)
	{
		bp->pos = bp->buf;
	}

	return (TRUE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// If doesn't continue match, returns zero
// otherwise returns number of chars of pSearchBuf that have been matched
unsigned int completecount_buf(BufTypePtr buf, BufTypePtr find, const unsigned int nPreMatched)
{
	if (isempty_buf(buf) || isempty_buf(find))
		return (0);

	if (nPreMatched >= poslen_buf(find))
		return (0);

	char *pBuf = getpos_buf(buf);
	char *pBufEnd = getend_buf(buf);

	char *pFind = getpos_buf(find) + nPreMatched;
	char *pFindEnd = getend_buf(find);

	unsigned int nSkipCount = 0;

	while ((pBuf <= pBufEnd) && (pFind <= pFindEnd) && ((*pBuf) == (*pFind)))
		pBuf++, pFind++, nSkipCount++;

	if ((pBuf > pBufEnd) || (pFind > pFindEnd))
		return (nPreMatched + nSkipCount);

	return (0); /* match failed */
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Find match of pSearchBuf, either complete or end-spanning
// return number of chars to skip before match
// No match: return length of buf (skip the whole buffer)
unsigned int skipcount_buf(BufTypePtr buf, BufTypePtr find)
{
	if (isempty_buf(buf))
		return (0);

	if (isempty_buf(find))
		return (poslen_buf(buf));

	char *pBuf = getpos_buf(buf);
	char *pFind = getpos_buf(find);
	char *pBufEnd = getend_buf(buf);
	char *pFindEnd = getend_buf(find);

	char *s1 = NULL, *s2 = NULL;

	unsigned int nSkipCount = 0;

	while (pBuf <= pBufEnd)
	{
		s1 = pBuf;
		s2 = pFind;

		while ((s1 <= pBufEnd) && (s2 <= pFindEnd) && ((*s1) == (*s2)))
			s1++, s2++;

		if ((s1 > pBufEnd) || (s2 > pFindEnd))
			return (nSkipCount);

		pBuf++;
		nSkipCount++;
	}

	return (poslen_buf(buf));
}
