/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* $Id: zutil.h,v 1.9 1995/05/03 17:27:12 jloup Exp $ */

#ifndef _Z_UTIL_H
#define _Z_UTIL_H

//AAB #include "zlib.h"

#if defined(MSDOS) || defined(VMS)
#   include <stddef.h>
#   include <errno.h>
#else
#include <errno.h>
//     extern int errno;
#endif
#ifdef STDC
#  include <string.h>
#endif

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

extern TCHAR *z_errmsg[8]; /* indexed by 1-zlib_error */

#define ERR_RETURN(strm,err) return (strm->msg=z_errmsg[1-err], err)
/* To be used only when the state is known to be valid */

        /* common constants */

#define DEFLATED   8

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

        /* target dependencies */

#ifdef MSDOS
#  define OS_CODE  0x00
#  ifdef __TURBOC__
//AAB #    include <alloc.h>
#  else /* MSC or DJGPP */
#    include <malloc.h>
#  endif
#endif

#ifdef OS2
#  define OS_CODE  0x06
#endif

#ifdef WIN32 /* Windows NT */
#  define OS_CODE  0x0b
#endif

#if defined(VAXC) || defined(VMS)
#  define OS_CODE  0x02
#  define FOPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef AMIGA
#  define OS_CODE  0x01
#endif

#if defined(ATARI) || defined(atarist)
#  define OS_CODE  0x05
#endif

#ifdef MACOS
#  define OS_CODE  0x07
#endif

#ifdef __50SERIES /* Prime/PRIMOS */
#  define OS_CODE  0x0F
#endif

#ifdef TOPS20
#  define OS_CODE  0x0a
#endif

        /* Common defaults */

#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif

#ifndef FOPEN
#  define FOPEN(name, mode) fopen((name), (mode))
#endif

         /* functions */

#ifdef HAVE_STRERROR
   extern TCHAR *strerror ((int));
#  define zstrerror(errnum) strerror(errnum)
#else
#  define zstrerror(errnum) _T("")
#endif

#if defined(pyr)
#  define NO_MEMCPY
#endif
#if (defined(M_I86SM) || defined(M_I86MM)) && !defined(_MSC_VER)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  */
#  define NO_MEMCPY
#endif
#if defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  if defined(M_I86SM) || defined(M_I86MM) /* MSC small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define zmemcpy memcpy
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   extern void zmemcpy  ((Bytef* dest, Bytef* source, uInt len));
   extern void zmemzero ((Bytef* dest, uInt len));
#endif

/* Diagnostic functions */
#ifdef DEBUG
#  include <stdio.h>
#  ifndef verbose
#    define verbose 0
#  endif
#  define Assert(cond,msg) {if(!(cond)) z_error(msg);}
#  define ZTrace(x) fprintf x
#  define ZTracev(x) {if (verbose) fprintf x ;}
#  define ZTracevv(x) {if (verbose>1) fprintf x ;}
#  define ZTracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define ZTracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define ZTrace(x)
#  define ZTracev(x)
#  define ZTracevv(x)
#  define ZTracec(c,x)
#  define ZTracecv(c,x)
#endif



#ifdef ORIG
#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#endif
#define ZALLOC(strm, items, size) \
           ((this->*((strm)->zalloc)))(((strm)->opaque), (items), (size))

#define ZFREE(strm, addr)  (this->*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}

#endif /* _Z_UTIL_H */
