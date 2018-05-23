/*
 * Program:	RFC-822 routines (originally from SMTP)
 *
 */


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <ctype.h>
#include <stdio.h>
#include <time.h>
// #include "netstream.h"
// #include "imapmail.h"
#include "exports.h"
#include "ImapDefs.h"

#include "osdep.h"
#include "rfc822.h"
#include "misc.h"


// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


/* RFC-822 static data */


char *errhst = ERRHOST;		/* syntax error host string */


/* Body formats constant strings, must match definitions in mail.h */

char *body_types[TYPEMAX+1] = {
  "TEXT", "MULTIPART", "MESSAGE", "APPLICATION", "AUDIO", "IMAGE", "VIDEO",
  "MODEL", "X-UNKNOWN"
};


// NOTE: "X-BINHEX" is not really a valid encoding, but we need a place holder.
//
char *body_encodings[ENCMAX+1] = {
  "7BIT", "8BIT", "BINARY", "BASE64", "QUOTED-PRINTABLE", "X-UUENCODE", "X-BINHEX", "X-UNKNOWN"
};


/* Token delimiting special characters */

				/* full RFC-822 specials */
const char *rspecials =  "()<>@,;:\\\"[].";
				/* body token specials */
const char *tspecials = " ()<>@,;:\\\"[]./?=";


/* Once upon a time, CSnet had a mailer which assigned special semantics to
 * dot in e-mail addresses.  For the sake of that mailer, dot was added to
 * the RFC-822 definition of `specials', even though it had numerous bad side
 * effects:
 *   1)	It broke mailbox names on systems which had dots in user names, such as
 *	Multics and TOPS-20.  RFC-822's syntax rules require that `Admin . MRC'
 *	be considered equivalent to `Admin.MRC'.  Fortunately, few people ever
 *	tried this in practice.
 *   2) It required that all personal names with an initial be quoted, a widely
 *	detested user interface misfeature.
 *   3)	It made the parsing of host names be non-atomic for no good reason.
 * To work around these problems, the following alternate specials lists are
 * defined.  hspecials and wspecials are used in lieu of rspecials, and
 * ptspecials are used in lieu of tspecials.  These alternate specials lists
 * make the parser work a lot better in the real world.  It ain't politically
 * correct, but it lets the users get their job done!
 */

				/* parse-word specials */
const char *wspecials = " ()<>@,;:\\\"[]";
				/* parse-token specials for parsing */
const char *ptspecials = " ()<>@,;:\\\"[]/?=";



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


/* Convert BASE64 contents to binary
 * Accepts: source
 *	    length of source
 *	    pointer to return destination length
 * Returns: destination as binary
 */

void *rfc822_base64 (unsigned char *src,unsigned long srcl,unsigned long *len)
{
  char c;
  void *ret = fs_get ((size_t) (*len = 4 + ((srcl * 3) / 4)));
  char *d = (char *) ret;
  short e = 0;
  memset (ret,0,(size_t) *len);	/* initialize block */
  *len = 0;			/* in case we return an error */
  while (srcl--) {		/* until run out of characters */
    c = *src++;			/* simple-minded decode */
    if (isupper (c)) c -= 'A';
    else if (islower (c)) c -= 'a' - 26;
    else if (isdigit (c)) c -= '0' - 52;
    else if (c == '+') c = 62;
    else if (c == '/') c = 63;
    else if (c == '=') {	/* padding */
      switch (e++) {		/* check quantum position */
      case 3:
	e = 0;			/* restart quantum */
	break;
      case 2:
	if (*src == '=') break;
      default:			/* impossible quantum position */
	fs_give (&ret);
	return NIL;
      }
      continue;
    }
    else continue;		/* junk character */
    switch (e++) {		/* install based on quantum position */
    case 0:
      *d = (char) (c << 2);		/* byte 1: high 6 bits */
      break;
    case 1:
      *d++ |= c >> 4;		/* byte 1: low 2 bits */
      *d = (char) (c << 4);		/* byte 2: high 4 bits */
      break;
    case 2:
      *d++ |= c >> 2;		/* byte 2: low 4 bits */
      *d = (char) (c << 6);		/* byte 3: high 2 bits */
      break;
    case 3:
      *d++ |= c;		/* byte 3: low 6 bits */
      e = 0;			/* reinitialize mechanism */
      break;
    }
  }
  *len = d - (char *) ret;	/* calculate data length */
  return ret;			/* return the string */
}


/* Convert binary contents to BASE64
 * Accepts: source
 *	    length of source
 *	    pointer to return destination length
 * Returns: destination as BASE64
 */

unsigned char *rfc822_binary (void *src,unsigned long srcl,unsigned long *len)
{
	unsigned char *ret,*d;
	unsigned char *s = (unsigned char *) src;
	char *v = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	unsigned long i = ((srcl + 2) / 3) * 4;

	*len = i += 2 * ((i / 60) + 1);

	d = ret = (unsigned char *) fs_get ((size_t) ++i);

	/* process tuplets */
	for (i = 0; srcl; s += 3)
	{	
		*d++ = v[s[0] >> 2];	/* byte 1: high 6 bits (1) */
				/* byte 2: low 2 bits (1), high 4 bits (2) */
		*d++ = v[((s[0] << 4) + (--srcl ? (s[1] >> 4) : 0)) & 0x3f];
				/* byte 3: low 4 bits (2), high 2 bits (3) */
		*d++ = srcl ? v[((s[1] << 2) + (--srcl ? (s[2] >> 6) : 0)) & 0x3f] : '=';
				/* byte 4: low 6 bits (3) */
		*d++ = srcl ? v[s[2] & 0x3f] : '=';
		if (srcl) srcl--;		/* count third character if processed */

		if ((++i) == 15)
		{		/* output 60 characters? */
			i = 0;			/* restart line break count, insert CRLF */
			*d++ = '\015'; *d++ = '\012';
		}
	}

	*d++ = '\015'; *d++ = '\012';	/* insert final CRLF */

	*d = '\0';			/* tie off string */

	if (((unsigned long) (d - ret)) != *len)
		fatal ("rfc822_binary logic flaw");

	return ret;			/* return the resulting string */
}


/* Write RFC822 address
 * Accepts: pointer to destination string
 *	    address to interpret
 */

void rfc822_write_address (char *dest,ADDRESS *adr)
{
	while (adr)
	{
				/* start of group? */
		if (adr->mailbox && !adr->host)
		{
				/* yes, write group name */
			rfc822_cat (dest,adr->mailbox,rspecials);
			strcat (dest,": ");	/* write group identifier */
			adr = adr->next;		/* move to next address block */
		}
		else
		{			/* end of group? */
			if (!adr->host) strcat (dest,";");
				/* simple case? */
			else if (!(adr->personal || adr->adl)) rfc822_address (dest,adr);
			else
			{			/* no, must use phrase <route-addr> form */
				if (adr->personal)
				{	/* in case have adl but no personal name */
					rfc822_cat (dest,adr->personal,rspecials);
					strcat (dest," ");
				}

				strcat (dest,"<");	/* write address delimiter */
				rfc822_address (dest,adr);/* write address */
				strcat (dest,">");	/* closing delimiter */
			}

				/* delimit if there is one */
			if ((adr = adr->next) && adr->mailbox)
				strcat (dest,", ");
		}
	}
}



/* Write RFC822 route-address to string
 * Accepts: pointer to destination string
 *	    address to interpret
 */

void rfc822_address (char *dest,ADDRESS *adr)
{
	/* no-op if no address */
	if (adr && adr->host)
	{
		/* have an A-D-L? */
		if (adr->adl)
		{
			strcat (dest,adr->adl);
			strcat (dest,":");
		}

		/* write mailbox name */
		rfc822_cat (dest,adr->mailbox,wspecials);

		/* unless null host (HIGHLY discouraged!) */
		if (*adr->host != '@')
		{	
			strcat (dest,"@");	/* host delimiter */
			strcat (dest,adr->host);	/* write host name */
		}
	}
}



/* Concatenate RFC822 string
 * Accepts: pointer to destination string
 *	    pointer to string to concatenate
 *	    list of special characters
 */

void rfc822_cat (char *dest,char *src,const char *specials)
{
	char *s;

	/* any specials present? */
	if (strpbrk (src,specials))
	{	
		strcat (dest,"\"");		/* opening quote */

		/* truly bizarre characters in there? */
		while (s = strpbrk (src,"\\\""))
		{
			strncat (dest,src,s-src);	/* yes, output leader */
			strcat (dest,"\\");	/* quoting */
			strncat (dest,s,1);	/* output the bizarre character */
			src = ++s;		/* continue after the bizarre character */
		}

		if (*src)
			strcat (dest,src);/* output non-bizarre string */

		strcat (dest,"\"");		/* closing quote */
	}
	else
	{
		strcat (dest,src);	/* otherwise it's the easy case */
	}
}


#endif // IMAP4

