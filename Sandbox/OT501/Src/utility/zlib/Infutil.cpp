/* inflate_util.c -- data and routines common to blocks and codes
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
// #include "infblock.h"
// #include "inftrees.h"
// #include "infcodes.h"
#include "infutil.h"


/* And'ing with mask[n] masks the lower n bits */
WORD inflate_mask[17] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};


/* copy as much as possible from the sliding window to the output area */
int SECComp::inflate_flush( inflate_blocks_statef *s, z_stream *z, int r)
{
  WORD n;
  BYTE FAR *p, *q;

  /* local copies of source and destination pointers */
  p = (BYTE FAR *)z->next_out;
  q = s->read;

  /* compute number of bytes to copy as far as end of window */
  n = (WORD)((q <= s->write ? s->write : s->end) - q);
  if (n > z->avail_out) n = (WORD)z->avail_out;
  if (n && r == Z_BUF_ERROR) r = Z_OK;

  /* update counters */
  z->avail_out -= n;
  z->total_out += n;

  /* update check information */
  if (s->checkfn != Z_NULL)
    s->check = (this->*(s->checkfn))(s->check, q, n);

  /* copy as far as end of window */
  zmemcpy(p, q, n);
  p += n;
  q += n;

  /* see if more to copy at beginning of window */
  if (q == s->end)
  {
    /* wrap pointers */
    q = s->window;
    if (s->write == s->end)
      s->write = s->window;

    /* compute bytes to copy */
    n = (WORD)(s->write - q);
    if (n > z->avail_out) n = (WORD)z->avail_out;
    if (n && r == Z_BUF_ERROR) r = Z_OK;

    /* update counters */
    z->avail_out -= n;
    z->total_out += n;

    /* update check information */
    if (s->checkfn != Z_NULL)
      s->check = (this->*(s->checkfn))(s->check, q, n);

    /* copy */
    zmemcpy(p, q, n);
    p += n;
    q += n;
  }

  /* update pointers */
  z->next_out = p;
  s->read = q;

  /* done */
  return r;
}
