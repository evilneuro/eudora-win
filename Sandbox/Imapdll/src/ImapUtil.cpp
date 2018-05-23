// ImapUtil.cpp - Utility functions.

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include <ctype.h>
#include <stdio.h>

#include "exports.h"
#include "ImapDefs.h"
#include "CramMd5.h"
#include "osdep.h"
#include "misc.h"

//#include "md5.h"

#include "ImapUtil.h"

// #include "CramMd5.h"
//#include "krbv4.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


// Used in the error database.
#define MAX_ERROR_LIST_SIZE		6


const DWORD dwErrorWaitTimeout = 2;		// seconds

//
// list of authenticators
//
// IMAP CRAM-MD5.
//extern long CramMD5Authenticator (authchallenge_t challenger,
//			      authrespond_t responder, NETMBX *mb, void *s,
//			      unsigned long *trial, char *user);

static AUTHENTICATOR CramMd5 =
{ 
	"CRAM-MD5",						// Name
	 CramMD5Authenticator,			// Client responder,
};


// Kerberos V4
extern long KrbV4Authenticator (authchallenge_t challenger,
			      authrespond_t responder, void *s,
			      unsigned long *trial, char *user);

static AUTHENTICATOR krb4 =
{ 
	"KERBEROS_V4",						// Name
	 KrbV4Authenticator,			// Client responder,
};


extern long  GssapiAuthenticator (authchallenge_t challenger,authrespond_t responder,
			void *stream, unsigned long *trial,
			char *user);


static AUTHENTICATOR gssapi =
{ 
	"GSSAPI",						// Name
	 GssapiAuthenticator,			// Client responder,
};


// Note: If no authenticators, set this to {NULL}.

// JOK - 8/18/98)
// Remove krb4 for now.

static AUTHENTICATOR *mailauthenticators [] = {&CramMd5, &krb4, &gssapi, NULL};

// static AUTHENTICATOR *mailauthenticators [] = {&CramMd5, &gssapi, NULL};


				/* pointer to alternate gets function */
static mailgets_t mailgets = NIL;

				/* pointer to read progress function */
static readprogress_t mailreadprogress = NIL;
				/* mail cache manipulation function */

/* Dummy string driver for complete in-memory strings */

STRINGDRIVER mail_string = {
	mail_string_init,		/* initialize string structure */
	mail_string_next,		/* get next byte in string structure */
	mail_string_setpos		/* set position in string structure */
};



//===================================================================//



/* Initialize mail string structure for in-memory string
 * Accepts: string structure
 *	    pointer to string
 *	    size of string
 */

void mail_string_init (STRING *s,void *data,unsigned long size)
{
				/* set initial string pointers */
	s->chunk = s->curpos = (char *) (s->data = data);
				/* and sizes */
	s->size = s->chunksize = s->cursize = size;
	s->data1 = s->offset = 0;	/* never any offset */
}


/* Get next character from string
 * Accepts: string structure
 * Returns: character, string structure chunk refreshed
 */

char mail_string_next (STRING *s)
{
	return *s->curpos++;		/* return the last byte */
}


/* Set string pointer position
 * Accepts: string structure
 *	    new position
 */

void mail_string_setpos (STRING *s,unsigned long i)
{
	s->curpos = s->chunk + i;	/* set new position */
	s->cursize = s->chunksize - i;/* and new size */
}



/* Mail garbage collect message
 * Accepts: message structure
 *	    garbage collection flags
 */

void mail_gc_msg (MESSAGE *msg,long gcflags)
{
  if (gcflags & GC_ENV) {	/* garbage collect envelopes? */
    mail_free_envelope (&msg->env);
    mail_free_body (&msg->body);
  }
  if (gcflags & GC_TEXTS) {	/* garbage collect texts */
    if (msg->full.text.data) fs_give ((void **) &msg->full.text.data);
    if (msg->header.text.data) {
      mail_free_stringlist (&msg->lines);
      fs_give ((void **) &msg->header.text.data);
    }
    if (msg->text.text.data) fs_give ((void **) &msg->text.text.data);
  }
}



/* JOK - Recurse through the given body and return the
 * nested body specified in "section"
 * Accepts: Top level BODY structure.
 *	    section specifier
 * Returns: pointer to nested body (can be same as what was passed in).
 */

BODY *mail_sub_body (BODY *body, char *section)
{
	BODY *b;
	PART *pt;
	unsigned long i;
	char tmp[MAILTMPLEN];

	/* make sure have a body */
	if (!body)
		return NIL;

	// Any section specifier?
	if (!(section && *section))
		return body;

	// Initialize
	b = NIL;

	/* find desired section */
	for (section = ucase (strcpy (tmp,section)); *section;)
	{
		if (isdigit (*section))
		{	/* get section specifier */
			i = strtoul (section,&section,10);

			/* make sure what follows is valid */
			if (*section && ((*section++ != '.') || !*section)) return NIL;

				/* multipart content? */
			if (b->type == TYPEMULTIPART)
			{
				/* yes, find desired part */
				for (pt = b->nested.part; (--i && (pt = pt->next)););
				if (!pt) return NIL;	/* bad specifier */
				b = &pt->body;		/* note new body */
			}

			/* otherwise must be section 1 */
			else if (i != 1) return NIL;

			/* need to go down further? */
			if (*section) switch (b->type)
			{
				case TYPEMULTIPART:	/* multipart */
				break;

				case TYPEMESSAGE:		/* embedded message */
				if (!strcmp (b->subtype,"RFC822"))
				{
					b = b->nested.msg->body;
					break;
				}
				default:			/* bogus subpart specification */
				return NIL;
			}
		}
		else
			return NIL;		/* unknown section specifier */
	}

	return b;
}



/* Mail data structure instantiation routines */

/* Mail instantiate cache elt
 * Accepts: initial message number
 * Returns: new cache elt
 */

// NOTE (JOK) - No longer takes a msgno!!

MESSAGECACHE *mail_new_cache_elt ()
{
	MESSAGECACHE *elt;

	// Allocate.
	elt = (MESSAGECACHE *) fs_get (sizeof (MESSAGECACHE));
	if (elt)
	{
		memset (elt, 0, sizeof (MESSAGECACHE));

		// JOK - Do we still need to do this??
		if (elt)
			 elt->lockcount = 1;
	}

	return elt;
}


/* Mail instantiate envelope
 * Returns: new envelope
 */

ENVELOPE *mail_newenvelope (void)
{
  return (ENVELOPE *) memset (fs_get (sizeof (ENVELOPE)),0,sizeof (ENVELOPE));
}


/* Mail instantiate address
 * Returns: new address
 */

ADDRESS *mail_newaddr (void)
{
  return (ADDRESS *) memset (fs_get (sizeof (ADDRESS)),0,sizeof (ADDRESS));
}



/* Mail instantiate body
 * Returns: new body
 */

BODY *mail_newbody (void)
{
  return mail_initbody ((BODY *) fs_get (sizeof (BODY)));
}


/* Mail initialize body
 * Accepts: body
 * Returns: body
 */

BODY *mail_initbody (BODY *body)
{
  memset ((void *) body,0,sizeof (BODY));
  body->type = TYPETEXT;	/* content type */
  body->encoding = ENC7BIT;	/* content encoding */
  return body;
}


/* Mail instantiate body parameter
 * Returns: new body part
 */

PARAMETER *mail_newbody_parameter (void)
{
  return (PARAMETER *) memset (fs_get (sizeof(PARAMETER)),0,sizeof(PARAMETER));
}


/* Mail instantiate body part
 * Returns: new body part
 */

PART *mail_newbody_part (void)
{
  PART *part = (PART *) memset (fs_get (sizeof (PART)),0,sizeof (PART));
  mail_initbody (&part->body);	/* initialize the body */
  return part;
}


/* Mail instantiate body message part
 * Returns: new body message part
 */

MESSAGE *mail_newmsg (void)
{
  return (MESSAGE *) memset (fs_get (sizeof (MESSAGE)),0,sizeof (MESSAGE));
}



/* Mail instantiate string list
 * Returns: new string list
 */

STRINGLIST *mail_newstringlist (void)
{
  return (STRINGLIST *) memset (fs_get (sizeof (STRINGLIST)),0,
				sizeof (STRINGLIST));
}


/* Mail instantiate new search program
 * Returns: new search program
 */

SEARCHPGM *mail_newsearchpgm (void)
{
  return (SEARCHPGM *) memset (fs_get (sizeof(SEARCHPGM)),0,sizeof(SEARCHPGM));
}


/* Mail instantiate new search program
 * Accepts: header line name   
 * Returns: new search program
 */

SEARCHHEADER *mail_newsearchheader (char *line)
{
  SEARCHHEADER *hdr = (SEARCHHEADER *) memset (fs_get (sizeof (SEARCHHEADER)),
					       0,sizeof (SEARCHHEADER));
  hdr->line = cpystr (line);	/* not defined */
  return hdr;
}


/* Mail instantiate new search set
 * Returns: new search set
 */

SEARCHSET *mail_newsearchset (void)
{
  return (SEARCHSET *) memset (fs_get (sizeof(SEARCHSET)),0,sizeof(SEARCHSET));
}


/* Mail instantiate new search or
 * Returns: new search or
 */

SEARCHOR *mail_newsearchor (void)
{
  SEARCHOR *or = (SEARCHOR *) memset (fs_get (sizeof (SEARCHOR)),0,
				      sizeof (SEARCHOR));
  or->first = mail_newsearchpgm ();
  or->second = mail_newsearchpgm ();
  return or;
}



/* Mail instantiate new searchpgmlist
 * Returns: new searchpgmlist
 */

SEARCHPGMLIST *mail_newsearchpgmlist (void)
{
  SEARCHPGMLIST *pgl = (SEARCHPGMLIST *)
    memset (fs_get (sizeof (SEARCHPGMLIST)),0,sizeof (SEARCHPGMLIST));
  pgl->pgm = mail_newsearchpgm ();
  return pgl;
}


/* Mail instantiate new sortpgm
 * Returns: new sortpgm
 */

SORTPGM *mail_newsortpgm (void)
{
  return (SORTPGM *) memset (fs_get (sizeof (SORTPGM)),0,sizeof (SORTPGM));
}



/* Mail garbage collection routines */


/* Mail garbage collect body
 * Accepts: pointer to body pointer
 */

void mail_free_body (BODY **body)
{
  if (*body) {			/* only free if exists */
    mail_free_body_data (*body);/* free its data */
    fs_give ((void **) body);	/* return body to free storage */
  }
}


/* Mail garbage collect body data
 * Accepts: body pointer
 */

void mail_free_body_data (BODY *body)
{
  switch (body->type) {		/* free contents */
  case TYPEMULTIPART:		/* multiple part */
    mail_free_body_part (&body->nested.part);
    break;
  case TYPEMESSAGE:		/* encapsulated message */
    if (body->subtype && !strcmp (body->subtype,"RFC822")) {
      mail_free_stringlist (&body->nested.msg->lines);
      mail_gc_msg (body->nested.msg,GC_ENV | GC_TEXTS);
      fs_give ((void **) &body->nested.msg);
    }
    break;
  default:
    break;
  }
  if (body->subtype) fs_give ((void **) &body->subtype);
  mail_free_body_parameter (&body->parameter);
  if (body->id) fs_give ((void **) &body->id);
  if (body->description) fs_give ((void **) &body->description);
  if (body->disposition.type) fs_give ((void **) &body->disposition);
  if (body->disposition.parameter)
    mail_free_body_parameter (&body->disposition.parameter);
  if (body->language) mail_free_stringlist (&body->language);
  if (body->mime.text.data) fs_give ((void **) &body->mime.text.data);
  if (body->contents.text.data) fs_give ((void **) &body->contents.text.data);
}




/* Mail garbage collect body parameter
 * Accepts: pointer to body parameter pointer
 */

void mail_free_body_parameter (PARAMETER **parameter)
{
  if (*parameter) {		/* only free if exists */
    if ((*parameter)->attribute) fs_give ((void **) &(*parameter)->attribute);
    if ((*parameter)->value) fs_give ((void **) &(*parameter)->value);
				/* run down the list as necessary */
    mail_free_body_parameter (&(*parameter)->next);
				/* return body part to free storage */
    fs_give ((void **) parameter);
  }
}


/* Mail garbage collect body part
 * Accepts: pointer to body part pointer
 */

void mail_free_body_part (PART **part)
{
  if (*part) {			/* only free if exists */
    mail_free_body_data (&(*part)->body);
				/* run down the list as necessary */
    mail_free_body_part (&(*part)->next);
    fs_give ((void **) part);	/* return body part to free storage */
  }
}



/* Mail garbage collect cache element
 * Accepts: pointer to cache element pointer
 */

void mail_free_elt (MESSAGECACHE **elt)
{
				/* only free if exists and no sharers */
  if (*elt && !--(*elt)->lockcount) {
    mail_gc_msg (&(*elt)->privat.msg,GC_ENV | GC_TEXTS);
    fs_give ((void **) elt);
  }
  else *elt = NIL;		/* else simply drop pointer */
}



/* Mail garbage collect envelope
 * Accepts: pointer to envelope pointer
 */

void mail_free_envelope (ENVELOPE **env)
{
  if (*env) {			/* only free if exists */
    if ((*env)->remail) fs_give ((void **) &(*env)->remail);
    mail_free_address (&(*env)->return_path);
    if ((*env)->date) fs_give ((void **) &(*env)->date);
    mail_free_address (&(*env)->from);
    mail_free_address (&(*env)->sender);
    mail_free_address (&(*env)->reply_to);
    if ((*env)->subject) fs_give ((void **) &(*env)->subject);
    mail_free_address (&(*env)->to);
    mail_free_address (&(*env)->cc);
    mail_free_address (&(*env)->bcc);
    if ((*env)->in_reply_to) fs_give ((void **) &(*env)->in_reply_to);
    if ((*env)->message_id) fs_give ((void **) &(*env)->message_id);
    if ((*env)->newsgroups) fs_give ((void **) &(*env)->newsgroups);
    if ((*env)->followup_to) fs_give ((void **) &(*env)->followup_to);
    if ((*env)->references) fs_give ((void **) &(*env)->references);
    fs_give ((void **) env);	/* return envelope to free storage */
  }
}


/* Mail garbage collect address
 * Accepts: pointer to address pointer
 */

void mail_free_address (ADDRESS **address)
{
  if (*address) {		/* only free if exists */
    if ((*address)->personal) fs_give ((void **) &(*address)->personal);
    if ((*address)->adl) fs_give ((void **) &(*address)->adl);
    if ((*address)->mailbox) fs_give ((void **) &(*address)->mailbox);
    if ((*address)->host) fs_give ((void **) &(*address)->host);
    if ((*address)->error) fs_give ((void **) &(*address)->error);
    mail_free_address (&(*address)->next);
    fs_give ((void **) address);/* return address to free storage */
  }
}


/* Mail garbage collect stringlist
 * Accepts: pointer to stringlist pointer
 */

void mail_free_stringlist (STRINGLIST **string)
{
  if (*string) {		/* only free if exists */
    if ((*string)->text.data) fs_give ((void **) &(*string)->text.data);
    mail_free_stringlist (&(*string)->next);
    fs_give ((void **) string);	/* return string to free storage */
  }
}




/* Mail garbage collect searchpgm
 * Accepts: pointer to searchpgm pointer
 */

void mail_free_searchpgm (SEARCHPGM **pgm)
{
  if (*pgm) {			/* only free if exists */
    mail_free_searchset (&(*pgm)->msgno);
    mail_free_searchset (&(*pgm)->uid);
    mail_free_searchor (&(*pgm)->or);
    mail_free_searchpgmlist (&(*pgm)->not);
    mail_free_searchheader (&(*pgm)->header);
    mail_free_stringlist (&(*pgm)->bcc);
    mail_free_stringlist (&(*pgm)->body);
    mail_free_stringlist (&(*pgm)->cc);
    mail_free_stringlist (&(*pgm)->from);
    mail_free_stringlist (&(*pgm)->keyword);
    mail_free_stringlist (&(*pgm)->subject);
    mail_free_stringlist (&(*pgm)->text);
    mail_free_stringlist (&(*pgm)->to);
    fs_give ((void **) pgm);	/* return program to free storage */
  }
}


/* Mail garbage collect searchheader
 * Accepts: pointer to searchheader pointer
 */

void mail_free_searchheader (SEARCHHEADER **hdr)
{
  if (*hdr) {			/* only free if exists */
    fs_give ((void **) &(*hdr)->line);
    if ((*hdr)->text) fs_give ((void **) &(*hdr)->text);
    mail_free_searchheader (&(*hdr)->next);
    fs_give ((void **) hdr);	/* return header to free storage */
  }
}


/* Mail garbage collect searchset
 * Accepts: pointer to searchset pointer
 */

void mail_free_searchset (SEARCHSET **set)
{
  if (*set) {			/* only free if exists */
    mail_free_searchset (&(*set)->next);
    fs_give ((void **) set);	/* return set to free storage */
  }
}



/* Mail garbage collect searchor
 * Accepts: pointer to searchor pointer
 */

void mail_free_searchor (SEARCHOR **orl)
{
  if (*orl) {			/* only free if exists */
    mail_free_searchpgm (&(*orl)->first);
    mail_free_searchpgm (&(*orl)->second);
    mail_free_searchor (&(*orl)->next);
    fs_give ((void **) orl);	/* return searchor to free storage */
  }
}


/* Mail garbage collect search program list
 * Accepts: pointer to searchpgmlist pointer
 */

void mail_free_searchpgmlist (SEARCHPGMLIST **pgl)
{
  if (*pgl) {			/* only free if exists */
    mail_free_searchpgm (&(*pgl)->pgm);
    mail_free_searchpgmlist (&(*pgl)->next);
    fs_give ((void **) pgl);	/* return searchpgmlist to free storage */
  }
}


/* Mail garbage collect sort program
 * Accepts: pointer to sortpgm pointer
 */

void mail_free_sortpgm (SORTPGM **pgm)
{
  if (*pgm) {			/* only free if exists */
    mail_free_sortpgm (&(*pgm)->next);
    fs_give ((void **) pgm);	/* return sortpgm to free storage */
  }
}




/* Lookup authenticator index
 * Accepts: authenticator index
 * Returns: authenticator, or 0 if not found
 */

AUTHENTICATOR *mail_lookup_auth (unsigned long i)
{
	AUTHENTICATOR *auth;
	int j = 0;

	auth = mailauthenticators [j];

	while (auth && --i)
	{
		j++;
		auth = mailauthenticators [j];
	}

	return auth;
}


/* Lookup authenticator name
 * Accepts: authenticator name
 * Returns: i-based index in authenticator chain, or 0 if not found
 */

unsigned int mail_lookup_auth_name (char *mechanism)
{
	char tmp[MAILTMPLEN];
	int i, index;
	AUTHENTICATOR *auth = *mailauthenticators;

				/* make upper case copy of mechanism name */
	ucase (strcpy (tmp,mechanism));

	index = 0;
	auth = mailauthenticators [0];

	for (i = 1; auth; i++)
	{
		if (!strcmp (auth->name,tmp))
		{
			index = i;
			break;
		}

		auth = mailauthenticators [i];
	}

	return index;
}




long mail_parse_date (MESSAGECACHE *elt, char *s)
{
	unsigned long d,m,y;
	int mi,ms;
	struct tm *t;
	time_t tn;
	char tmp[MAILTMPLEN];

				/* clear elt */
	elt->zoccident = elt->zhours = elt->zminutes =
			elt->hours = elt->minutes = elt->seconds =
			elt->day = elt->month = elt->year = 0;

				/* make a writeable uppercase copy */
	if (s && *s && (strlen (s) < (size_t)MAILTMPLEN))
		s = ucase (strcpy (tmp,s));
	else
		return NIL;

				/* skip over possible day of week */
	if (isalpha (*s) && isalpha (s[1]) && isalpha (s[2]) && (s[3] == ',') && (s[4] == ' '))
		 s += 5;

	while (*s == ' ')
		s++;	/* parse first number (probable month) */

	if (!(m = strtoul ((const char *) s,&s,10)))
		return NIL;

	switch (*s)
	{			/* different parse based on delimiter */
		case '/':			/* mm/dd/yy format */
			if (!((d = strtoul ((const char *) ++s,&s,10)) && *s == '/' &&
				(y = strtoul ((const char *) ++s,&s,10)) && *s == '\0'))
			{
				return NIL;
			}

			break;

			case ' ':			/* dd mmm yy format */
				while (s[1] == ' ')
					s++;	/* slurp extra whitespace */

			case '-':			/* dd-mmm-yy format */
				d = m;			/* so the number we got is a day */

				/* make sure string long enough! */
				if (strlen (s) < (size_t) 5)
					return NIL;

				/* Some compilers don't allow `<<' and/or longs in case statements. */
				/* slurp up the month string */
				ms = ((s[1] - 'A') * 1024) + ((s[2] - 'A') * 32) + (s[3] - 'A');
				switch (ms)
				{		/* determine the month */
					case (('J'-'A') * 1024) + (('A'-'A') * 32) + ('N'-'A'): m = 1; break;
					case (('F'-'A') * 1024) + (('E'-'A') * 32) + ('B'-'A'): m = 2; break;
					case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('R'-'A'): m = 3; break;
					case (('A'-'A') * 1024) + (('P'-'A') * 32) + ('R'-'A'): m = 4; break;
					case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('Y'-'A'): m = 5; break;
					case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('N'-'A'): m = 6; break;
					case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('L'-'A'): m = 7; break;
					case (('A'-'A') * 1024) + (('U'-'A') * 32) + ('G'-'A'): m = 8; break;
					case (('S'-'A') * 1024) + (('E'-'A') * 32) + ('P'-'A'): m = 9; break;
					case (('O'-'A') * 1024) + (('C'-'A') * 32) + ('T'-'A'): m = 10; break;
					case (('N'-'A') * 1024) + (('O'-'A') * 32) + ('V'-'A'): m = 11; break;
					case (('D'-'A') * 1024) + (('E'-'A') * 32) + ('C'-'A'): m = 12; break;

					default:
						return NIL;	/* unknown month */
				}

				if ((s[4] == *s) &&	(y = strtoul ((const char *) s+5,&s,10)) &&
					(*s == '\0' || *s == ' '))
					break;

		default:
			return NIL;		/* unknown date format */
	}
				/* minimal validity check of date */
  if ((d > 31) || (m > 12)) return NIL; 
				/* two digit year */
  if (y < 100) y += (y >= (BASEYEAR - 1900)) ? 1900 : 2000;
				/* set values in elt */
  elt->day = d; elt->month = m; elt->year = y - BASEYEAR;

  ms = '\0';			/* initially no time zone string */
  if (*s) {			/* time specification present? */
				/* parse time */
    d = strtoul ((const char *) s+1,&s,10);
    if (*s != ':') return NIL;
    m = strtoul ((const char *) ++s,&s,10);
    y = (*s == ':') ? strtoul ((const char *) ++s,&s,10) : 0;
				/* validity check time */
    if ((d > 23) || (m > 59) || (y > 59)) return NIL; 
				/* set values in elt */
    elt->hours = d; elt->minutes = m; elt->seconds = y;
    switch (*s) {		/* time zone specifier? */
    case ' ':			/* numeric time zone */
      while (s[1] == ' ') s++;	/* slurp extra whitespace */
      if (!isalpha (s[1])) {	/* treat as '-' case if alphabetic */
				/* test for sign character */
	if ((elt->zoccident = (*++s == '-')) || (*s == '+')) s++;
				/* validate proper timezone */
	if (isdigit(*s) && isdigit(s[1]) && isdigit(s[2]) && isdigit(s[3])) {
	  elt->zhours = (*s - '0') * 10 + (s[1] - '0');
	  elt->zminutes = (s[2] - '0') * 10 + (s[3] - '0');
	}
	return T;		/* all done! */
      }
				/* falls through */
    case '-':			/* symbolic time zone */
      if (!(ms = *++s)) ms = 'Z';
      else if (*++s) {		/* multi-character? */
	ms -= 'A'; ms *= 1024;	/* yes, make compressed three-byte form */
	ms += ((*s++ - 'A') * 32);
	if (*s) ms += *s++ - 'A';
	if (*s) ms = '\0';	/* more than three characters */
      }
    default:			/* ignore anything else */
      break;
    }
  }



  /*  This is not intended to be a comprehensive list of all possible
   * timezone strings.  Such a list would be impractical.  Rather, this
   * listing is intended to incorporate all military, North American, and
   * a few special cases such as Japan and the major European zone names,
   * such as what might be expected to be found in a Tenex format mailbox
   * and spewed from an IMAP server.  The trend is to migrate to numeric
   * timezones which lack the flavor but also the ambiguity of the names.
   *
   *  RFC-822 only recognizes UT, GMT, 1-letter military timezones, and the
   * 4 CONUS timezones and their summer time variants.  [Sorry, Canadian
   * Atlantic Provinces, Alaska, and Hawaii.]
   *
   *  Timezones that are not valid in RFC-822 are under #if 1 conditionals.
   * Timezones which are frequently encountered, but are ambiguous, are
   * under #if 0 conditionals for documentation purposes.
   */
  switch (ms) {			/* determine the timezone */
				/* Universal */
  case (('U'-'A')*1024)+(('T'-'A')*32):
#if 1
  case (('U'-'A')*1024)+(('T'-'A')*32)+'C'-'A':
#endif
				/* Greenwich */
  case (('G'-'A')*1024)+(('M'-'A')*32)+'T'-'A':
  case 'Z': elt->zhours = 0; break;

    /* oriental (from Greenwich) timezones */
#if 1
				/* Middle Europe */
  case (('M'-'A')*1024)+(('E'-'A')*32)+'T'-'A':
#endif
#if 0	/* conflicts with Bering */
				/* British Summer */
  case (('B'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#endif
  case 'A': elt->zhours = 1; break;
#if 1
				/* Eastern Europe */
  case (('E'-'A')*1024)+(('E'-'A')*32)+'T'-'A':
#endif
  case 'B': elt->zhours = 2; break;
  case 'C': elt->zhours = 3; break;
  case 'D': elt->zhours = 4; break;
  case 'E': elt->zhours = 5; break;
  case 'F': elt->zhours = 6; break;
  case 'G': elt->zhours = 7; break;
  case 'H': elt->zhours = 8; break;
#if 1
				/* Japan */
  case (('J'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#endif
  case 'I': elt->zhours = 9; break;
  case 'K': elt->zhours = 10; break;
  case 'L': elt->zhours = 11; break;
  case 'M': elt->zhours = 12; break;

	/* occidental (from Greenwich) timezones */
  case 'N': elt->zoccident = 1; elt->zhours = 1; break;
  case 'O': elt->zoccident = 1; elt->zhours = 2; break;
#if 1
  case (('A'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
#endif
  case 'P': elt->zoccident = 1; elt->zhours = 3; break;
#if 0	/* conflicts with Nome */
				/* Newfoundland */
  case (('N'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
    elt->zoccident = 1; elt->zhours = 3; elt->zminutes = 30; break;
#endif
#if 1
				/* Atlantic */
  case (('A'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#endif
	/* CONUS */
  case (('E'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
  case 'Q': elt->zoccident = 1; elt->zhours = 4; break;
				/* Eastern */
  case (('E'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
  case (('C'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
  case 'R': elt->zoccident = 1; elt->zhours = 5; break;
				/* Central */
  case (('C'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
  case (('M'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
  case 'S': elt->zoccident = 1; elt->zhours = 6; break;
				/* Mountain */
  case (('M'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
  case (('P'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
  case 'T': elt->zoccident = 1; elt->zhours = 7; break;
				/* Pacific */
  case (('P'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#if 1
  case (('Y'-'A')*1024)+(('D'-'A')*32)+'T'-'A':
#endif
  case 'U': elt->zoccident = 1; elt->zhours = 8; break;
#if 1
				/* Yukon */
  case (('Y'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#endif
  case 'V': elt->zoccident = 1; elt->zhours = 9; break;
#if 1
				/* Hawaii */
  case (('H'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#endif
  case 'W': elt->zoccident = 1; elt->zhours = 10; break;
#if 0	/* conflicts with Newfoundland, British Summer, and Singapore */
				/* Nome/Bering/Samoa */
  case (('N'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
  case (('B'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
  case (('S'-'A')*1024)+(('S'-'A')*32)+'T'-'A':
#endif
  case 'X': elt->zoccident = 1; elt->zhours = 11; break;
  case 'Y': elt->zoccident = 1; elt->zhours = 12; break;

  default:			/* unknown time zones treated as local */
    tn = time (0);		/* time now... */
    t = localtime (&tn);	/* get local minutes since midnight */
    mi = t->tm_hour * 60 + t->tm_min;
    ms = t->tm_yday;		/* note Julian day */
    if (t = gmtime (&tn)) {	/* minus UTC minutes since midnight */
      mi -= t->tm_hour * 60 + t->tm_min;
	/* ms can be one of:
	 *  36x  local time is December 31, UTC is January 1, offset -24 hours
	 *    1  local time is 1 day ahead of UTC, offset +24 hours
	 *    0  local time is same day as UTC, no offset
	 *   -1  local time is 1 day behind UTC, offset -24 hours
	 * -36x  local time is January 1, UTC is December 31, offset +24 hours
	 */
      if (ms -= t->tm_yday)	/* correct offset if different Julian day */
	mi += ((ms < 0) == (abs (ms) == 1)) ? -24*60 : 24*60;
      if (mi < 0) {		/* occidental? */
	mi = abs (mi);		/* yup, make positive number */
	elt->zoccident = 1;	/* and note west of UTC */
      }
      elt->zhours = mi / 60;	/* now break into hours and minutes */
      elt->zminutes = mi % 60;
    }
    break;
  }
  return T;
}




#endif // IMAP4

