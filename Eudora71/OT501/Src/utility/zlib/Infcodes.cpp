/* infcodes.c -- process literals and length/distance pairs
 * Copyright (C) 1995 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
#include "stdafx.h"

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL

#include "compeng.h"
#include "zutil.h"
// #include "inftrees.h"
// #include "infblock.h"
#include "infcodes.h"
#include "infutil.h"
#include "inffast.h"

/* simplify the use of the inflate_huft type with some defines */
#define base more.Base
#define next more.Next
#define exop word.what.Exop
#define bits word.what.Bits

inflate_codes_statef* SECComp::inflate_codes_new( WORD bl, WORD bd, inflate_huft *tl, 
	inflate_huft *td,  z_stream *z)
{
  inflate_codes_statef *c;

  if ((c = (inflate_codes_statef *)
       ZALLOC(z,1,sizeof(struct inflate_codes_state))) != Z_NULL)
  {
    c->mode = inflate_codes_state::START;
    c->lbits = (Byte)bl;
    c->dbits = (Byte)bd;
    c->ltree = tl;
    c->dtree = td;
    ZTracev((stderr, "inflate:       codes new\n"));
  }
  return c;
}


int SECComp::inflate_codes( inflate_blocks_statef *s, z_stream *z, int r)
{
  WORD j;               /* temporary storage */
  inflate_huft *t;      /* temporary pointer */
  WORD e;               /* extra bits or operation */
  DWORD b;              /* bit buffer */
  WORD k;               /* bits in bit buffer */
  BYTE *p;             /* input data pointer */
  // WORD n;                /* bytes available there */  
  DWORD n;
  BYTE *q;             /* output window write pointer */
  WORD m;               /* bytes to end of window or read pointer */
  BYTE *f;             /* pointer to copy strings from */
  inflate_codes_statef *c = s->sub.decode.codes;  /* codes state */

  /* copy input/output information to locals (UPDATE macro restores) */
  LOAD

  /* process input and output based on current state */
  while (1) switch (c->mode)
  {             /* waiting for "i:"=input, "o:"=output, "x:"=nothing */
    case inflate_codes_state::START:         /* x: set up for LEN */
#ifndef SLOW
      if (m >= 258 && n >= 10)
      {
        UPDATE
        r = inflate_fast(c->lbits, c->dbits, c->ltree, c->dtree, s, z);
        LOAD
        if (r != Z_OK)
        {
          c->mode = r == Z_STREAM_END ? inflate_codes_state::WASH : inflate_codes_state::BADCODE;
          break;
        }
      }
#endif /* !SLOW */
      c->sub.code.need = c->lbits;
      c->sub.code.tree = c->ltree;
      c->mode = inflate_codes_state::LEN;
    case inflate_codes_state::LEN:           /* i: get length/literal/eob next */
      j = c->sub.code.need;
      NEEDBITS(j)	  
      t = c->sub.code.tree + ((WORD)b & inflate_mask[j]);
      DUMPBITS(t->bits)	  
      e = (WORD)(t->exop);
      if (e == 0)               /* literal */
      {
        c->sub.lit = t->base;
        ZTracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                 "inflate:         literal '%c'\n" :
                 "inflate:         literal 0x%02x\n", t->base));
        c->mode = inflate_codes_state::LIT;
        break;
      }
      if (e & 16)               /* length */
      {
		WORD wMask = 15;
        c->sub.copy.get = (WORD)(e & wMask);
        c->len = t->base;
        c->mode = inflate_codes_state::LENEXT;
        break;
      }
      if ((e & 64) == 0)        /* next table */
      {
        c->sub.code.need = e;
        c->sub.code.tree = t->next;
        break;
      }
      if (e & 32)               /* end of block */
      {
        ZTracevv((stderr, "inflate:         end of block\n"));
        c->mode = inflate_codes_state::WASH;
        break;
      }
      c->mode = inflate_codes_state::BADCODE;        /* invalid code */
      z->msg = _T("invalid literal/length code");
      r = Z_DATA_ERROR;
      LEAVE
    case inflate_codes_state::LENEXT:        /* i: getting length extra (have base) */
      j = c->sub.copy.get;
      NEEDBITS(j)
      c->len = (WORD)(c->len + (WORD)(b & inflate_mask[j]));
      DUMPBITS(j)
      c->sub.code.need = c->dbits;
      c->sub.code.tree = c->dtree;
      ZTracevv((stderr, "inflate:         length %u\n", c->len));
      c->mode = inflate_codes_state::DIST;
    case inflate_codes_state::DIST:          /* i: get distance next */
      j = c->sub.code.need;
      NEEDBITS(j)
      t = c->sub.code.tree + ((WORD)b & inflate_mask[j]);
      DUMPBITS(t->bits)
      e = (WORD)(t->exop);
      if (e & 16)               /* distance */
      {
		WORD wMask = 15;
        c->sub.copy.get = (WORD)(e & wMask);
        c->sub.copy.dist = t->base;
        c->mode = inflate_codes_state::DISTEXT;
        break;
      }
      if ((e & 64) == 0)        /* next table */
      {
        c->sub.code.need = e;
        c->sub.code.tree = t->next;
        break;
      }
      c->mode = inflate_codes_state::BADCODE;        /* invalid code */
      z->msg = _T("invalid distance code");
      r = Z_DATA_ERROR;
      LEAVE
    case inflate_codes_state::DISTEXT:       /* i: getting distance extra */
      j = c->sub.copy.get;
      NEEDBITS(j)
      c->sub.copy.dist = (WORD)(c->sub.copy.dist + ((WORD)b & inflate_mask[j]));
      DUMPBITS(j)
      ZTracevv((stderr, "inflate:         distance %u\n", c->sub.copy.dist));
      c->mode = inflate_codes_state::COPY;
    case inflate_codes_state::COPY:          /* o: copying bytes in window, waiting for space */
#ifndef __TURBOC__ /* Turbo C bug for following expression */
      f = (WORD)(q - s->window) < c->sub.copy.dist ?
          s->end - (c->sub.copy.dist - (q - s->window)) :
          q - c->sub.copy.dist;
#else
      f = q - c->sub.copy.dist;
      if ((WORD)(q - s->window) < c->sub.copy.dist)
        f = s->end - (c->sub.copy.dist - (q - s->window));
#endif
      while (c->len)
      {
        NEEDOUT
        OUTBYTE(*f++)
        if (f == s->end)
          f = s->window;
        c->len--;
      }
      c->mode = inflate_codes_state::START;
      break;
    case inflate_codes_state::LIT:           /* o: got literal, waiting for output space */
      NEEDOUT
      OUTBYTE(c->sub.lit)
      c->mode = inflate_codes_state::START;
      break;
    case inflate_codes_state::WASH:          /* o: got eob, possibly more output */
      FLUSH
      if (s->read != s->write)
        LEAVE
      c->mode = inflate_codes_state::END;
    case inflate_codes_state::END:
      r = Z_STREAM_END;
      LEAVE
    case inflate_codes_state::BADCODE:       /* x: got error */
      r = Z_DATA_ERROR;
      LEAVE
    default:
      r = Z_STREAM_ERROR;
      LEAVE
  }
}


void SECComp::inflate_codes_free( inflate_codes_statef *c, z_stream *z)
{
  ZFREE(z, c);
  ZTracev((stderr, "inflate:       codes free\n"));
}
