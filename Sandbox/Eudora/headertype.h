///////////////////////////////////////////////////////////////////////////////
// headertype.h
//
// This is taken verbatim from compmsgd.h. I needed the header types
// HEADER_TO, HEADER_CC, and HEADER_BCC for setting the header line
// in the composition window when a user selects To, Cc, or Bcc from
// the Directory Services window, but did not see why I had to include
// all of compmsgd.h. compmsgd.h includes this file now.
//
// Created: 09/27/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __EUDORA_HEADERTYPE_H_
#define __EUDORA_HEADERTYPE_H_

#define HEADER_TO		0
#define HEADER_FROM		1
#define HEADER_SUBJECT		2
#define HEADER_CC		3
#define HEADER_BCC		4
#define HEADER_ATTACHMENTS	5
#define HEADER_IN_REPLY_TO	6
#define HEADER_REFERENCES	7
#define NumHeaders              6 // this is the number of headers displayed to the user
#define MaxHeaders 8			// this includes the reply threading headers

#define HEADER_CURRENT	99		// do the current header, please

#endif // __EUDORA_HEADERTYPE_H_
