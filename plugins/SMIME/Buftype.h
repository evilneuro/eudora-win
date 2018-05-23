/*
 * File: BUFTYPE.H
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

#ifndef BUFTYPE_H
#define BUFTYPE_H

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef struct {
	char *buf;
	unsigned int buf_size;
	unsigned int len;
	char *pos;
} BufType;

typedef BufType *BufTypePtr;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

BufTypePtr makesize_buf(unsigned int n);
BufTypePtr make_buf();

/* boolean */ int isempty_buf(BufTypePtr bp);
unsigned int availspace_buf(BufTypePtr bp);
unsigned int buflen_buf(BufTypePtr bp); /* Number of valid chars in buffer. Always < bufsize_buf() */
char *getbuf_buf(BufTypePtr bp);

char *getend_buf(BufTypePtr bp);
unsigned int bufsize_buf(BufTypePtr bp); /* The size of the buffer -- used or not */
unsigned int buflen_buf(BufTypePtr bp);
/* boolean */ int setlen_buf(BufTypePtr bp, const unsigned int n);

unsigned int bufncat_buf(BufTypePtr dst, BufTypePtr src, unsigned int n);
unsigned int bufins_buf(BufTypePtr b1, BufTypePtr b2);
unsigned int bufnins_buf(BufTypePtr b1, BufTypePtr b2, unsigned int n);

unsigned int poslen_buf(BufTypePtr bp); /* Number of valid chars, starting at current position */

void incpos_buf(BufTypePtr bp, unsigned int n);
void delpos_buf(BufTypePtr bp);
char *getpos_buf(BufTypePtr bp); /* NULL problems */
void resetpos_buf(BufTypePtr bp);

unsigned int strcpy_buf(BufTypePtr dst, const char *src);
unsigned int strncpy_buf(BufTypePtr dst, const char *src, const unsigned int n);
unsigned int strcat_buf(BufTypePtr dst, const char *src);
unsigned int strncat_buf(BufTypePtr dst, const char *src, const unsigned int n);

void delete_buf(BufTypePtr bp);
void free_buf(BufTypePtr bp);
void clear_buf(BufTypePtr bp);
void emptybuf_buf(BufTypePtr bp);

unsigned int completecount_buf(BufTypePtr src, BufTypePtr find, const unsigned int n);
unsigned int skipcount_buf(BufTypePtr buf, BufTypePtr find);

#endif
