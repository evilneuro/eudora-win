// CramMd5.h - Include in Imap4rv1.cpp.
//
#ifndef __CRAMMD5_H
#define __CRAMMD5_H

long CramMD5Authenticator (authchallenge_t challenger,
			      authrespond_t responder, void *s,
			      unsigned long *trial, char *user);



#endif // __CRAMMD5_H
