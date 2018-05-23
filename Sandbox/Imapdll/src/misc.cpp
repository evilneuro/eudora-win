/*
 * Program:	Miscellaneous utility routines
 *
 */


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <ctype.h>
// #include "Network.h"
// #include "imapmail.h"
#include "exports.h"
#include "ImapDefs.h"
#include "osdep.h"
#include "misc.h"


// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


/* Convert string to all uppercase
 * Accepts: string pointer
 * Returns: string pointer
 */

char *ucase (char *s)
{
	char *t;
				/* if lowercase covert to upper */
	for (t = s; t && *t; t++)
	{
		if (islower (*t))
			*t = (char) toupper (*t);
	}

	return s;			/* return string */
}


/* Convert string to all lowercase
 * Accepts: string pointer
 * Returns: string pointer
 */

char *lcase (char *s)
{
	char *t;
				/* if uppercase covert to lower */
	for (t = s; t && *t; t++)
	{
		if (isupper (*t))
			*t = (char) tolower (*t);
	}

	return s;			/* return string */
}



/* Copy string to free storage
 * Accepts: source string
 * Returns: free storage copy of string
 */

char *cpystr (const char *string)
{
	return string ? strcpy ((char *) fs_get (1 + strlen (string)),string) : NIL;
}


/* Copy text/size to free storage as sized text
 * Accepts: destination sized text
 *	    pointer to source text
 *	    size of source text
 * Returns: text as a char *
 */

char *cpytxt (SIZEDTEXT *dst,char *text,unsigned long size)
{

	// Sanity:
	if (! (dst && text) )
		return NULL;

	/* flush old space */
	if (dst->data)
		fs_give ((void **) &dst->data);

	/* copy data in sized text */
	memcpy (dst->data = (char *) fs_get ((size_t) (dst->size = size) + 1),text,
	  (size_t) size);

	dst->data[size] = '\0';	/* tie off text */

	return dst->data;		/* convenience return */
}



/* Copy sized text to free storage as sized text
 * Accepts: destination sized text
 *	    source sized text
 * Returns: text as a char *
 */

char *textcpy (SIZEDTEXT *dst,SIZEDTEXT *src)
{

	// Sanity:
	if ( !(dst && src) )
		return NULL;

	/* flush old space */
	if (dst->data)
		fs_give ((void **) &dst->data);

	/* copy data in sized text */
	memcpy (dst->data = (char *) fs_get ((size_t) (dst->size = src->size) + 1),
	  src->data,(size_t) src->size);

	dst->data[dst->size] = '\0';	/* tie off text */

	return dst->data;		/* convenience return */
}


/* Copy stringstruct to free storage as sized text
 * Accepts: destination sized text
 *	    source stringstruct
 * Returns: text as a char *
 */

char *textcpystring (SIZEDTEXT *text,STRING *bs)
{
  unsigned long i = 0;

	// Sanity:
	if (! (text && bs) )
		return NULL;

	/* clear old space */
	if (text->data) fs_give ((void **) &text->data);

	/* make free storage space in sized text */
	text->data = (char *) fs_get ((size_t) (text->size = SIZE (bs)) + 1);

	while (i < text->size)
		 text->data[i++] = SNX (bs);

	text->data[i] = '\0';		/* tie off text */

	return text->data;		/* convenience return */
}


/* Copy stringstruct from offset to free storage as sized text
 * Accepts: destination sized text
 *	    source stringstruct
 *	    offset into stringstruct
 *	    size of source text
 * Returns: text as a char *
 */

char *textcpyoffstring (SIZEDTEXT *text,STRING *bs,unsigned long offset,
			unsigned long size)
{
	unsigned long i = 0;

	// Sanity:
	if (! (text && bs ) )
		return NULL;

	/* clear old space */
	if (text->data) fs_give ((void **) &text->data);

	SETPOS (bs,offset);		/* offset the string */

	/* make free storage space in sized text */
	text->data = (char *) fs_get ((size_t) (text->size = size) + 1);

	while (i < size)
		text->data[i++] = SNX (bs);

	text->data[i] = '\0';		/* tie off text */

	return text->data;		/* convenience return */
}


/* Returns index of rightmost bit in word
 * Accepts: pointer to a 32 bit value
 * Returns: -1 if word is 0, else index of rightmost bit
 *
 * Bit is cleared in the word
 */

unsigned long find_rightmost_bit (unsigned long *valptr)
{
  register long value= *valptr;
  register long clearbit;	/* bit to clear */
  register bitno;		/* bit number to return */

	// Sanity:
	if (!valptr)
		return 0;

	/* no bits are set */
	if (!value) return (0xffffffff);

	if (value & 0xFFFF)
	{		/* low order halfword has a bit? */
	    bitno = 0;			/* yes, start with bit 0 */
		clearbit = 1;		/* which has value 1 */
	}
	else
	{			/* high order halfword has the bit */
	    bitno = 16;			/* start with bit 16 */
		clearbit = 0x10000;		/* which has value 10000h */
		value >>= 16;		/* and slide the halfword down */
	}

	if (!(value & 0xFF))
	{	/* low quarterword has a bit? */
		bitno += 8;			/* no, start 8 bits higher */
		clearbit <<= 8;		/* bit to clear is 2^8 higher */
		value >>= 8;		/* and slide the quarterword down */
	}

	while (T)
	{			/* search for bit in quarterword */
		if (value & 1) break;	/* found it? */

		value >>= 1;		/* now, slide the bit down */
		bitno += 1;			/* count one more bit */
		clearbit <<= 1;		/* bit to clear is 1 bit higher */
	}

	*valptr ^= clearbit;		/* clear the bit in the argument */

	return (bitno);		/* and return the bit number */
}



/* Return minimum of two integers
 * Accepts: integer 1
 *	    integer 2
 * Returns: minimum
 */

long min (long i,long j)
{
  return ((i < j) ? i : j);
}


/* Return maximum of two integers
 * Accepts: integer 1
 *	    integer 2
 * Returns: maximum
 */

long max (long i,long j)
{
	return ((i > j) ? i : j);
}


/* Case independent search
 * Accepts: base string
 *	    length of base string
 *	    pattern string
 *	    length of pattern string
 * Returns: T if pattern exists inside base, else NIL
 */

long search (char *s,long i,char *pat,long patc)
{
	long j;
	long ret = NIL;

				/* validate arguments, calculate # of tries */
  if (s && (i > 0) && pat && (patc > 0) && ((i -= (patc - 1)) > 0)) {
    union {			/* machine word */
      unsigned long w;		/* long form */
      char c[8];		/* octet form */
    } wd;
    char *p = (char *) fs_get ((size_t) (patc + 1));
    for (j = 0; j < patc; j++)	/* make lowercase copy of pattern */
      p[j] = isupper (pat[j]) ? (char)tolower (pat[j]) : pat[j];
    p[j] = '\0';		/* tie off just in case */
    memcpy (wd.c,"AAAA1234",8);	/* constant for word testing */
				/* do slow search if long is not 4 chars */
    ret = (wd.w == 0x41414141) ? fsrc (s,i,p,patc) : ssrc (&s,&i,p,patc,i);
    fs_give ((void **) &p);	/* flush copy */
  }
  return ret;
}


/* Case independent fast search (32-bit machines only)
 * Accepts: base string
 *	    number of tries left
 *	    pattern string
 *	    length of pattern string
 * Returns: T if pattern exists inside base, else NIL
 */

long fsrc (char *s,long i,char *pat,long patc)
{
  long j;
  unsigned long mask;
				/* any chars before word boundary? */
  if ((j = ((long) s & 3)) && ssrc (&s,&i,pat,patc,(long) 4 - j)) return T;
  /*
   * Fast search algorithm XORs the mask with each word from the base string
   * and complements the result. This will give bytes of all ones where there
   * are matches.  We then mask out the high order and case bits in each byte
   * and add 21 (case + overflow) to all the bytes.  If we have a resulting
   * carry, then we have a match.
   */
  mask = *pat * 0x01010101;	/* search mask */
  while (i > 0) {		/* interesting word? */
    if (0x80808080&(0x21212121+(0x5F5F5F5F&~(mask^*(unsigned long *) s)))) {
				/* yes, commence a slow search through it */
      if (ssrc (&s,&i,pat,patc,(long) 4)) return T;
    }
    else s += 4,i -= 4;		/* try next word */
  }
  return NIL;			/* string not found */
}


/* Case independent slow search
 * Accepts: base string
 *	    number of tries left
 *	    pattern string
 *	    length of pattern string
 *	    maximum number of tries
 * Returns: T if pattern exists inside base, else NIL
 */

long ssrc (char **s,long *i,char *pat,long patc,long maxtries)
{
  char *t;
  long j;
  while (maxtries--)		/* search each position */
    for (t = (*s)++,(*i)--,j = patc - 1;
	 (pat[j] == (isupper (t[j]) ? tolower (t[j]) : t[j])); j--)
      if (!j) return T;		/* found pattern */
  return NIL;			/* not found */
}



/* Wildcard pattern match
 * Accepts: base string
 *	    pattern string
 *	    delimiter character
 * Returns: T if pattern matches base, else NIL
 */

long pmatch_full (char *s,char *pat,char delim)
{
  switch (*pat) {
  case '%':			/* non-recursive */
				/* % at end, OK if no inferiors */
    if (!pat[1]) return (delim && strchr (s,delim)) ? NIL : T;
                                /* scan remainder of string until delimiter */
    do if (pmatch_full (s,pat+1,delim)) return T;
    while ((*s != delim) && *s++);
    break;
  case '*':			/* match 0 or more characters */
    if (!pat[1]) return T;	/* * at end, unconditional match */
				/* scan remainder of string */
    do if (pmatch_full (s,pat+1,delim)) return T;
    while (*s++);
    break;
  case '\0':			/* end of pattern */
    return *s ? NIL : T;	/* success if also end of base */
  default:			/* match this character */
    return (*pat == *s) ? pmatch_full (s+1,pat+1,delim) : NIL;
  }
  return NIL;
}


/* Directory pattern match
 * Accepts: base string
 *	    pattern string
 *	    delimiter character
 * Returns: T if base is a matching directory of pattern, else NIL
 */

long dmatch (char *s,char *pat,char delim)
{
  switch (*pat) {
  case '%':			/* non-recursive */
    if (!*s) return T;		/* end of base means have a subset match */
    if (!*++pat) return NIL;	/* % at end, no inferiors permitted */
				/* scan remainder of string until delimiter */
    do if (dmatch (s,pat,delim)) return T;
    while ((*s != delim) && *s++);
    if (*s && !s[1]) return T;	/* ends with delimiter, must be subset */
    return dmatch (s,pat,delim);/* do new scan */
  case '*':			/* match 0 or more characters */
    return T;			/* unconditional match */
  case '\0':			/* end of pattern */
    break;
  default:			/* match this character */
    if (*s) return (*pat == *s) ? dmatch (s+1,pat+1,delim) : NIL;
				/* end of base, return if at delimiter */
    else if (*pat == delim) return T;
    break;
  }
  return NIL;
}


/* Added by JOK */
/* Trim leading and trailing blanks from the given string. */
void Trim (char *string)
{
	char *p, *q;

	if (string)
	{
		p = q = string;

		// Look for first non-blank char.
		while (q && (*q) && (*q == ' ' || *q == '\t'))
			q++;

		// All blank?
		if (!*q)
			*p = 0;	// We're done.
		else
		{
			while (*q)
			{
				*p++ = *q++;
			}
			*p = 0;	// Tie off.

			// Strip trailing;
			q = p + strlen (p) - 1;
			while (q && (q >= p) && !(*q == ' ' || *q == '\t'))
				*q-- = 0;
		}
	}
}


#endif // IMAP4

