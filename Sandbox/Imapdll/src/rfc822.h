/*
 * Program:	RFC-822 routines (originally from SMTP)
 *
 */

/* Function prototypes */

void rfc822_write_address (char *dest,ADDRESS *adr);
void rfc822_address (char *dest,ADDRESS *adr);
void rfc822_cat (char *dest,char *src,const char *specials);
void *rfc822_base64 (unsigned char *src,unsigned long srcl,unsigned long *len);
unsigned char *rfc822_binary (void *src,unsigned long srcl,unsigned long *len);

