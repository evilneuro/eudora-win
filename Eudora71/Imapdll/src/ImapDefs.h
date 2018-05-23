// ImapDefs.h - Typedefs used by tye IMAP code.
//

#ifndef __IMAPDEFS_H
#define __IMAPDEFS_H

// These are defined in rfc822.cpp
//
extern char *body_types[];	/* defined body type strings */

extern char *body_encodings[];	/* defined body encoding strings */

/* Build parameters */

// (JOK) #define CACHEINCREMENT 250	/* cache growth increments */

#define MAILTMPLEN 1024		/* size of a temporary buffer */
#define MAXMESSAGESIZE 65000	/* MS-DOS: maximum text buffer size
				 * other:  initial text buffer size */
#define NUSERFLAGS 30		/* # of user flags (current servers 30 max) */
#define BASEYEAR 1970		/* the year time began on Unix (note: mx
				 * driver depends upon this matching Unix) */
				/* default for unqualified addresses */
#define BADHOST ".MISSING-HOST-NAME."
				/* default for syntax errors in addresses */
#define ERRHOST ".SYNTAX-ERROR."


/* Constants */

#define NIL 0			/* convenient name */
#define T 1			/* opposite of NIL */
#define LONGT (long) 1		/* long T */

#define WARN (long) 1		/* mm_log warning type */
#define IMAPERROR (long) 2		/* mm_log error type */
#define PARSE (long) 3		/* mm_log parse error type */
#define BYE (long) 4		/* mm_notify stream dying */

#define DELIM '\377'		/* strtok delimiter character */


/* Bits from mail_parse_flags().  Don't change these, since the header format
 * used by tenex, mtx, dawz, and tenexdos corresponds to these bits.
 */

#define fSEEN 1
#define fDELETED 2
#define fFLAGGED 4
#define fANSWERED 8
#define fOLD 16
#define fDRAFT 32

/* Driver flags */

#define DR_DISABLE (long) 1	/* driver is disabled */
#define DR_LOCAL (long) 2	/* local file driver */
#define DR_MAIL (long) 4	/* supports mail */
#define DR_NEWS (long) 8	/* supports news */
#define DR_READONLY (long) 16	/* driver only allows readonly access */
#define DR_NOFAST (long) 32	/* "fast" data is slow (whole msg fetch) */
#define DR_NAMESPACE (long) 64	/* driver has a special namespace */
#define DR_LOWMEM (long) 128	/* low amounts of memory available */


/* Cache management function codes */

#define CH_INIT (long) 10	/* initialize cache */
#define CH_SIZE (long) 11	/* (re-)size the cache */
#define CH_MAKEELT (long) 30	/* return elt, make if needed */
#define CH_ELT (long) 31	/* return elt if exists */
#define CH_FREE (long) 40	/* free space used by elt */
#define CH_EXPUNGE (long) 45	/* delete elt pointer from list */


/* Open options */

#define OP_DEBUG (long) 1		/* debug protocol negotiations */
#define OP_READONLY (long) 2	/* read-only open */
#define OP_ANONYMOUS (long) 4	/* anonymous open of newsgroup */
#define OP_SHORTCACHE (long) 8	/* short (elt-only) caching */
#define OP_SILENT (long) 16		/* don't pass up events (internal use) */
#define OP_PROTOTYPE (long) 32	/* return driver prototype */
#define OP_CONTROLONLY (long) 64	/* IMAP connect but no select) */
#define OP_EXPUNGE (long) 128	/* silently expunge recycle stream */


/* Close options */

#define CL_EXPUNGE (long) 1	/* expunge silently */


/* Fetch options */

#define FT_UID (long) 1		/* argument is a UID */
#define FT_PEEK (long) 2	/* peek at data */
#define FT_NOT (long) 4		/* NOT flag for header lines fetch */
#define FT_INTERNAL (long) 8	/* text can be internal strings */
#define FT_PREFETCHTEXT (long) 16 /* IMAP prefetch text when fetching header */


/* Flagging options */

#define ST_UID (long) 1		/* argument is a UID sequence */
#define ST_SILENT (long) 2	/* don't return results */
#define ST_SET (long) 4		/* set vs. clear */

/* Copy options */

#define CP_UID (long) 1		/* argument is a UID sequence */
#define CP_MOVE (long) 2	/* delete from source after copying */


/* Search/sort options */

#define SE_UID (long) 1		/* return UID */
#define SE_FREE (long) 2	/* free search program after finished */
#define SE_NOPREFETCH (long) 4	/* no search prefetching */
#define SO_FREE (long) 8	/* free sort program after finished */

/* Mailgets flags */

#define MG_UID (long) 1		/* message number is a UID */
#define MG_COPY (long) 2	/* must return copy of argument */


/* Garbage collection flags */

#define GC_ELT (long) 1		/* message cache elements */
#define GC_ENV (long) 2		/* envelopes and bodies */
#define GC_TEXTS (long) 4	/* cached texts */

/* Sort functions */

#define SORTDATE 0		/* date */
#define SORTARRIVAL 1		/* arrival date */
#define SORTFROM 2		/* from */
#define SORTSUBJECT 3		/* subject */
#define SORTTO 4		/* to */
#define SORTCC 5		/* cc */
#define SORTSIZE 6		/* size */

/* Sort program */

#define SORTPGM struct sort_program

SORTPGM {
  unsigned int reverse : 1;	/* sort function is to be reversed */
  short function;		/* sort function */
  SORTPGM *next;		/* next function */
};



/* Entry in the message cache array */

typedef struct message_cache
{
	unsigned long msgno;		/* message number */
	unsigned int lockcount : 8;	/* non-zero if multiple references */
	unsigned long rfc822_size;	/* # of bytes of message as raw RFC822 */
	struct
	{			/* c-client internal use only */
		unsigned long uid;		/* message unique ID */
		PARTTEXT special;		/* special text pointers */
		MESSAGE msg;		/* internal message pointers */
	} privat;
			/* internal date */
	unsigned int day : 5;		/* day of month (1-31) */
	unsigned int month : 4;	/* month of year (1-12) */
	unsigned int year : 7;	/* year since BASEYEAR (expires in 127 yrs) */
	unsigned int hours: 5;	/* hours (0-23) */
	unsigned int minutes: 6;	/* minutes (0-59) */
	unsigned int seconds: 6;	/* seconds (0-59) */
	unsigned int zoccident : 1;	/* non-zero if west of UTC */
	unsigned int zhours : 4;	/* hours from UTC (0-12) */
	unsigned int zminutes: 6;	/* minutes (0-59) */
			/* system flags */
	unsigned int seen : 1;	/* system Seen flag */
	unsigned int deleted : 1;	/* system Deleted flag */
	unsigned int flagged : 1; 	/* system Flagged flag */
	unsigned int answered : 1;	/* system Answered flag */
	unsigned int draft : 1;	/* system Draft flag */
	unsigned int recent : 1;	/* system Recent flag */
			/* message status */
	unsigned int valid : 1;	/* elt has valid flags */
	unsigned int searched : 1;	/* message was searched */
	unsigned int sequence : 1;	/* message is in sequence */
			/* reserved for use by main program */
	unsigned int spare : 1;	/* first spare bit */
	unsigned int spare2 : 1;	/* second spare bit */
	unsigned int spare3 : 1;	/* third spare bit */
	void *sparep;			/* spare pointer */
	unsigned long user_flags;	/* user-assignable flags */
} MESSAGECACHE;


/* Stringstruct access routines */

#define INIT(s,d,data,size) ((*((s)->dtb = &d)->init) (s,data,size))
#define SIZE(s) ((s)->size - GETPOS (s))
#define CHR(s) (*(s)->curpos)
#define SNX(s) (--(s)->cursize ? *(s)->curpos++ : (*(s)->dtb->next) (s))
#define GETPOS(s) ((s)->offset + ((s)->curpos - (s)->chunk))
#define SETPOS(s,i) (*(s)->dtb->setpos) (s,i)



/* Mail Access I/O stream */


/* Structure for mail driver dispatch */

#define DRIVER struct driver	


// USER NOTIFICATION messages. Duplicated in imap.h.
#define IMAP_NOTIFY_SET_READONLY_STATUS		1
#define IMAP_NOTIFY_SET_READWRITE_STATUS	2
#define IMAP_NOTIFY_ESCAPE_PRESSED			3
#define IMAP_NOTIFY_INVALID_PASSWORD		4


// User callback: Passes messages out of DLL to caller:
// Duplicated in imap.h!!
//
typedef void (*ImapNotificationCallback_p) (unsigned long ulData, unsigned short NotifierType, unsigned long ulValue);

// Error callback type declaration: (Also defined in imap.h!!!)
//
typedef QCNetworkSettings::ErrorCallback ImapErrorFunc;


/* Mail I/O stream */

// Forward declaration:
class CNetStream;
class QCNetworkSettings;
	

// Structure for allowing a callback to read from a string.
typedef struct
{
	char *s;
	unsigned long size;
} ParenStrData;


/* Message overview */

typedef struct mail_overview
{
	char *subject;		/* message subject string */
	MAILADDRESS *from;		/* originator address list */
	char *date;			/* message composition date string */
	char *message_id;		/* message ID */
	char *references;		/* USENET references */
	struct
	{			/* may be 0 or NUL if unknown/undefined */
		unsigned long octets;	/* message octets (probably LF-newline form) */
		unsigned long lines;	/* message lines */
		char *xref;			/* cross references */
	} optional;
} OVERVIEW;


// Driver notifications.
enum NOTIFY_TYPE
{
	NETSTREAM_CLOSED,
	NETSTREAM_DESTROYED,
	NETSTREAM_ESCAPE_PRESSED,

};



/* Parse results from mail_valid_net_parse */

#define NETMAXHOST 64
#define NETMAXUSER 64
#define NETMAXMBX 255
#define NETMAXSRV 20

typedef struct net_mailbox
{
	char host[NETMAXHOST+1];	/* host name */
	char user[NETMAXUSER+1];	/* user name */
	char mailbox[NETMAXMBX+1];	/* mailbox name */
	char service[NETMAXSRV+1];	/* service name */
	unsigned long port;		/* TCP port number */
	unsigned int anoflag : 1;	/* anonymous */
	unsigned int dbgflag : 1;	/* debug flag */
} NETMBX;


/* Network access I/O stream */

/* Mailgets data identifier */

typedef struct GETS_DATA
{
	void *stream;			// A CProtocol object!!
	unsigned long msgno;
	char *what;
	STRINGLIST *stl;
	unsigned long first;
	unsigned long last;
	long flags;
} GETS_DATA;

#define INIT_GETS(md,s,m,w,f,l) \
  md.stream = s, md.msgno = m, md.what = w, md.first = f, md.last = l, \
  md.stl = NIL, md.flags = NIL;




// ======================= FROM imap4v1.h =========================//
//

#define MAXLOGINTRIALS 3	/* maximum number of login trials */


// Don't automatically look ahead! (JOK)
#define IMAPLOOKAHEAD 0			/* envelope lookahead */
#define IMAPUIDLOOKAHEAD 0		/* UID lookahead */
// END JOK

#define IMAPTCPPORT (long) 143	/* assigned TCP contact port */


/* Parsed reply message from imap_reply */

typedef struct imap_parsed_reply
{
	char *line;			/* original reply string pointer */
	char *tag;			/* command tag this reply is for */
	char *key;			/* reply keyword */
	char *text;			/* subsequent text */
} IMAPPARSEDREPLY;


#define IMAPTMPLEN 16*MAILTMPLEN
#define MAXAUTHENTICATORS 8

#define AUTHENTICATOR struct mail_authenticator

typedef void *(*authchallenge_t) (void *protocol, unsigned long *len);

typedef BOOL (*authrespond_t) (void *protocol, char *response, unsigned long size);

typedef long (*authclient_t) (authchallenge_t challenger,
			      authrespond_t responder, void *s,
			      unsigned long *trial, char *user);

typedef char *(*authresponse_t) (void *challenge,unsigned long clen,
				 unsigned long *rlen);

AUTHENTICATOR {
  char *name;			/* name of this authenticator */
  authclient_t client;		/* client function that supports it */
};

/* Arguments to imap_send() */

typedef struct imap_argument
{
	int type;			/* argument type */
	void *text;			/* argument text */
} IMAPARG;


typedef char *(*mailgets_t) (readfn_t f,void *protocol,unsigned long size,
			     GETS_DATA* md);

typedef char *(*readprogress_t) (GETS_DATA *md,unsigned long octets);


/* imap_send() argument types */

#define ATOM 0
#define NUMBER 1
#define FLAGS 2
#define ASTRING 3
#define LITERAL 4
#define LIST 5
#define SEARCHPROGRAM 6
#define BODYTEXT 7
#define BODYPEEK 8
#define BODYCLOSE 9
#define SEQUENCE 10
#define LISTMAILBOX 11
#define ENVELOPEPLUS  12
#define ENVELOPEPLUSCLOSE  13



// ============================== END stuff from imap4r1.h ============================//


#endif //  __IMAPDEFS_H


