// Exports.h - Typedefs and macros exported from the DLL
//

#ifndef __IMAPEXPORTS_H
#define __IMAPEXPORTS_H

#include "QCNetSettings.h"


// TYPEDEFS for the exported IMAP structures.

typedef unsigned long UIDVALIDITY;
typedef unsigned long IMAPUID;

// USER NOTIFICATION messages. Duplicated in imapmail.h.
#define IMAP_NOTIFY_SET_READONLY_STATUS		1
#define IMAP_NOTIFY_SET_READWRITE_STATUS	2
#define IMAP_NOTIFY_ESCAPE_PRESSED			3
#define IMAP_NOTIFY_INVALID_PASSWORD		4


// Notification callback type for network and IMAP asynchronous notifications.
// Must duplicate this in "imapmail.h".
//
typedef void (*ImapNotificationCallback_p) (unsigned long ulData, unsigned short NotifierType, unsigned long ulValue);

//
// Error message types exported to the user.
//
// BAD error code:
#define IMAPERR_BAD_CODE			-1
#define IMAPERR_BASE				100
#define IMAPERR_USER_CANCELLED		IMAPERR_BASE + 1		// User pressed the stop button.
#define IMAPERR_LOCAL_ERROR			IMAPERR_BASE + 2		// Failed at our end.
#define IMAPERR_COMMAND_FAILED		IMAPERR_BASE + 3		// Has a server response.
#define IMAPERR_CONNECTION_CLOSED	IMAPERR_BASE + 4		// Error cause connection to close.


/* Primary body types */

#define TYPETEXT 0		/* unformatted text */
#define TYPEMULTIPART 1		/* multiple part */
#define TYPEMESSAGE 2		/* encapsulated message */
#define TYPEAPPLICATION 3	/* application data */
#define TYPEAUDIO 4		/* audio */
#define TYPEIMAGE 5		/* static image */
#define TYPEVIDEO 6		/* video */
#define TYPEMODEL 7		/* model */
#define TYPEOTHER 8		/* unknown */
#define TYPEBOGUSMULTI 9		/* badly parsed multipart type; Courier IMAP server */
#define TYPEMAX 15		/* maximum type code */


/* Body encodings */

#define ENC7BIT 0		/* 7 bit SMTP semantic data */
#define ENC8BIT 1		/* 8 bit SMTP semantic data */
#define ENCBINARY 2		/* 8 bit binary data */
#define ENCBASE64 3		/* base-64 encoded data */
#define ENCQUOTEDPRINTABLE 4	/* human-readable 8-as-7 bit data */
#define ENCUUENCODE	5	/* X-UUENCODE */
#define ENCBINHEX   6	// mac-binhex
#define ENCOTHER 7		/* unknown */
#define ENCMAX 10		/* maximum encoding code */


/* Bits for folder attributes. */

#define LATT_NOINFERIORS (long) 1
#define LATT_NOSELECT (long) 2
#define LATT_MARKED (long) 4
#define LATT_UNMARKED (long) 8


// Structures.
#define BODY struct mail_bodystruct
#define MESSAGE struct mail_body_message
#define PARAMETER struct mail_body_parameter
#define PART struct mail_body_part
#define PARTTEXT struct mail_body_text
#define SIZEDTEXT struct mail_text

/* Sized text */

SIZEDTEXT {
  char *data;			/* text */
  unsigned long size;		/* size of text in octets */
};


/* Message body text */

PARTTEXT {
  unsigned long offset;		/* offset from body origin */
  SIZEDTEXT text;		/* text */
};


/* String list */

typedef struct string_list
{
  SIZEDTEXT text;		/* string text */
  struct string_list *next;
} STRINGLIST;


typedef struct mail_address {
	char	*personal;			/* personal name phrase */
	char	*adl;				/* at-domain-list source route */
	char	*mailbox;			/* mailbox name */
	char	*host;				/* domain name of mailbox's host */
	char	*error;				/* error in address from SMTP module */
	struct mail_address *next;	/* pointer to next address in list */
} MAILADDRESS;


// We export an ENVELOPE!
typedef struct mail_envelope
{
	char			*remail;		/* remail header if any */
	MAILADDRESS		*return_path;	/* error return address */
	char			*date;			/* message composition date string */
	MAILADDRESS		*from;			/* originator address list */
	MAILADDRESS		*sender;		/* sender address list */
	MAILADDRESS		*reply_to;		/* reply address list */
	char			*subject;		/* message subject string */
	MAILADDRESS		*to;			/* primary recipient list */
	MAILADDRESS		*cc;			/* secondary recipient list */
	MAILADDRESS		*bcc;			/* blind secondary recipient list */
	char			*in_reply_to;	/* replied message ID */
	char			*message_id;	/* message ID */
	char			*newsgroups;	/* USENET newsgroups */
	char			*followup_to;	/* USENET reply newsgroups */
	char			*references;	/* USENET references */
} ENVELOPE;



/* Message body structure */

BODY {
	unsigned short type;		/* body primary type */
	char *sztype;				/* body primary type as string */
	unsigned short encoding;	/* body transfer encoding */
	char *subtype;		/* subtype string */
	PARAMETER *parameter;		/* parameter list */
	char *id;			/* body identifier */
	char *description;		/* body description */
	struct {			/* body disposition */
		char *type;			/* disposition type */
		PARAMETER *parameter;	/* disposition parameters */
	} disposition;
	STRINGLIST *language;		/* body language */
	PARTTEXT mime;		/* MIME header */
	PARTTEXT contents;		/* body part contents */
	union {			/* different ways of accessing contents */
		PART *part;			/* body part list */
		MESSAGE *msg;		/* body encapsulated message */
	} nested;
	struct {
		unsigned long lines;	/* size of text in lines */
		unsigned long bytes;	/* size of text in octets */
	} size;
	char *md5;			/* MD5 checksum */
};


/* Parameter list */

PARAMETER {
	char *attribute;		/* parameter attribute name */
	char *value;			/* parameter value */
	PARAMETER *next;		/* next parameter in list */
};


/* Multipart content list */
PART {
	BODY body;			/* body information for this part */
	PART *next;			/* next body part */
};


/* RFC-822 Message */

MESSAGE {
	ENVELOPE *env;		/* message envelope */
	BODY *body;			/* message body */
	PARTTEXT full;		/* full message */
	STRINGLIST *lines;		/* lines used to filter header */
	PARTTEXT header;		/* header text */
	PARTTEXT text;		/* body text */
};


// STRINGDRIVER declarations.

#define STRINGDRIVER struct string_driver

typedef struct mailstring {
	void *data;			/* driver-dependent data */
	unsigned long data1;		/* driver-dependent data */
	unsigned long size;		/* total length of string */
	char *chunk;			/* base address of chunk */
	unsigned long chunksize;	/* size of chunk */
	unsigned long offset;		/* offset of this chunk in base */
	char *curpos;			/* current position in chunk */
	unsigned long cursize;	/* number of bytes remaining in chunk */
	STRINGDRIVER *dtb;		/* driver that handles this type of string */
} STRING;


/* Dispatch table for string driver */

STRINGDRIVER {
				/* initialize string driver */
	void (*init) (STRING *s,void *data,unsigned long size);
				/* get next character in string */
	char (*next) (STRING *s);
				/* set position in string */
	void (*setpos) (STRING *s,unsigned long i);
};

// Reader type declaration. Reads a chunk at a time from the network.
typedef BOOL (*readfn_t) (void *stream,unsigned long size,char *buffer);


/* Mailbox status */

#define SA_MESSAGES		(long) 1	/* number of messages */
#define SA_RECENT		(long) 2	/* number of recent messages */
#define SA_UNSEEN		(long) 4	/* number of unseen messages */
#define SA_UIDNEXT		(long) 8	/* next UID to be assigned */
#define SA_UIDVALIDITY	(long) 16	/* UID validity value */

typedef struct mbx_status {
	long flags;				/* validity flags */
	unsigned long messages;	/* number of messages */
	unsigned long recent;		/* number of recent messages */
	unsigned long unseen;		/* number of unseen messages */
	unsigned long uidnext;	/* next UID to be assigned */
	unsigned long uidvalidity;	/* UID validity value */
} MAILSTATUS;




//================= Search program =====================//

/* Search program */

#define SEARCHPGM struct search_program
#define SEARCHHEADER struct search_header
#define SEARCHSET struct search_set
#define SEARCHOR struct search_or
#define SEARCHPGMLIST struct search_pgm_list


SEARCHHEADER
{			/* header search */
	char *line;			/* header line */
	char *text;			/* text in header */
	SEARCHHEADER *next;		/* next in list */
};


SEARCHSET
{			/* message set */
	unsigned long first;		/* sequence number */
	unsigned long last;		/* last value, if a range */
	SEARCHSET *next;		/* next in list */
};


SEARCHOR
{
	SEARCHPGM *first;		/* first program */
	SEARCHPGM *second;		/* second program */
	SEARCHOR *next;		/* next in list */
};


SEARCHPGMLIST
{
	SEARCHPGM *pgm;		/* search program */
	SEARCHPGMLIST *next;		/* next in list */
};


SEARCHPGM
{			/* search program */
	SEARCHSET *msgno;		/* message numbers */
	SEARCHSET *uid;		/* unique identifiers */
	SEARCHOR *or;			/* or'ed in programs */
	SEARCHPGMLIST *not;		/* and'ed not program */
	SEARCHHEADER *header;		/* list of headers */
	STRINGLIST *bcc;		/* bcc recipients */
	STRINGLIST *body;		/* text in message body */
	STRINGLIST *cc;		/* cc recipients */
	STRINGLIST *from;		/* originator */
	STRINGLIST *keyword;		/* keywords */
	STRINGLIST *unkeyword;	/* unkeywords */
	STRINGLIST *subject;		/* text in subject */
	STRINGLIST *text;		/* text in headers and body */
	STRINGLIST *to;		/* to recipients */
	unsigned long larger;		/* larger than this size */
	unsigned long smaller;	/* smaller than this size */
	unsigned short sentbefore;	/* sent before this date */
	unsigned short senton;	/* sent on this date */
	unsigned short sentsince;	/* sent since this date */
	unsigned short before;	/* before this date */
	unsigned short on;		/* on this date */
	unsigned short since;		/* since this date */
	unsigned int answered : 1;	/* answered messages */
	unsigned int unanswered : 1;	/* unanswered messages */
	unsigned int deleted : 1;	/* deleted messages */
	unsigned int undeleted : 1;	/* undeleted messages */
	unsigned int draft : 1;	/* message draft */
	unsigned int undraft : 1;	/* message undraft */
	unsigned int flagged : 1;	/* flagged messages */
	unsigned int unflagged : 1;	/* unflagged messages */
	unsigned int recent : 1;	/* recent messages */
	unsigned int old : 1;		/* old messages */
	unsigned int seen : 1;	/* seen messages */
	unsigned int unseen : 1;	/* unseen messages */
};




// Error callback type declaration:
//
typedef QCNetworkSettings::ErrorCallback ImapErrorFunc;


#endif // ifndef __IMAPEXPORTS_H
