// ImapUtil.h - Utility suport functions.
//

#ifndef __IMAPUTIL_H
#define __IMAPUTIL_H

extern STRINGDRIVER mail_string;

void mail_string_init (STRING *s,void *data,unsigned long size);
char mail_string_next (STRING *s);
void mail_string_setpos (STRING *s,unsigned long i);
void mail_link (DRIVER *driver);

void mail_gc_msg (MESSAGE *msg,long gcflags);

BODY *mail_sub_body (BODY *body, char *section);

MESSAGECACHE *mail_new_cache_elt (void);

ENVELOPE *mail_newenvelope (void);
MAILADDRESS *mail_newaddr (void);
BODY *mail_newbody (void);
BODY *mail_initbody (BODY *body);
PARAMETER *mail_newbody_parameter (void);
PART *mail_newbody_part (void);
MESSAGE *mail_newmsg (void);
STRINGLIST *mail_newstringlist (void);
SEARCHPGM *mail_newsearchpgm (void);
SEARCHHEADER *mail_newsearchheader (char *line);
SEARCHSET *mail_newsearchset (void);
SEARCHOR *mail_newsearchor (void);
SEARCHPGMLIST *mail_newsearchpgmlist (void);
SORTPGM *mail_newsortpgm (void);
void mail_free_body (BODY **body);
void mail_free_body_data (BODY *body);
void mail_free_body_parameter (PARAMETER **parameter);
void mail_free_body_part (PART **part);

// JOK - We still have elts.
void mail_free_elt (MESSAGECACHE **elt);

void mail_free_envelope (ENVELOPE **env);
void mail_free_address (MAILADDRESS **address);
void mail_free_stringlist (STRINGLIST **string);
void mail_free_searchpgm (SEARCHPGM **pgm);
void mail_free_searchheader (SEARCHHEADER **hdr);
void mail_free_searchset (SEARCHSET **set);
void mail_free_searchor (SEARCHOR **orl);
void mail_free_searchpgmlist (SEARCHPGMLIST **pgl);
void mail_free_sortpgm (SORTPGM **pgm);

char *mail_auth (char *mechanism,authresponse_t resp,int argc,char *argv[]);
AUTHENTICATOR *mail_lookup_auth (unsigned long i);
unsigned int mail_lookup_auth_name (char *mechanism);

MESSAGECACHE *mail_new_cache_elt ();

long mail_parse_date (MESSAGECACHE *elt,char *s);

#endif // __IMAPUTIL_H
