/* inflate.c -- zlib interface to inflate modules
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




int SECComp::inflateReset( z_stream *z)
{
  DWORD c;
  inflate_internal_state *pState = (inflate_internal_state *)z->state;

  if (z == Z_NULL || pState == Z_NULL)
    return Z_STREAM_ERROR;
  z->total_in = z->total_out = 0;
  z->msg = Z_NULL;
  pState->mode = pState->nowrap ? inflate_internal_state::BLOCKS : inflate_internal_state::METHOD;
  inflate_blocks_reset(pState->blocks, z, &c);
  ZTrace((stderr, "inflate: reset\n"));
  return Z_OK;
}


int SECComp::inflateEnd( z_stream *z)
{
  DWORD c;
  inflate_internal_state *pState = (inflate_internal_state *)z->state;
  if (z == Z_NULL || pState == Z_NULL || z->zfree == Z_NULL)
    return Z_STREAM_ERROR;
  if (pState->blocks != Z_NULL)
    inflate_blocks_free(pState->blocks, z, &c);
  ZFREE(z, pState);
  pState = Z_NULL;
  ZTrace((stderr, "inflate: end\n"));
  return Z_OK;
}


int SECComp::inflateInit2( z_stream *z, int w)
{
  /* initialize state */
  inflate_internal_state *pState;

  if (z == Z_NULL)
    return Z_STREAM_ERROR;
  if (z->zalloc == Z_NULL) z->zalloc = zcalloc;
  if (z->zfree == Z_NULL) z->zfree = zcfree;
  if ((z->state= (internal_state *)
       ZALLOC(z,1,sizeof(struct inflate_internal_state))) == Z_NULL)
    return Z_MEM_ERROR;
  pState = (inflate_internal_state *)z->state;
  pState->blocks = Z_NULL;

  /* handle undocumented nowrap option (no zlib header or check) */
  pState->nowrap = 0;
  if (w < 0)
  {
    w = - w;
    pState->nowrap = 1;
  }

  /* set window size */
  if (w < 8 || w > 15)
  {
    inflateEnd(z);
    return Z_STREAM_ERROR;
  }
  pState->wbits = (WORD)w;

  /* create inflate_blocks state */
  if ((pState->blocks =
       inflate_blocks_new(z, pState->nowrap ? Z_NULL : adler32, (WORD)((WORD)1 << (WORD)w)))
      == Z_NULL)
  {
    inflateEnd(z);
    return Z_MEM_ERROR;
  }
  ZTrace((stderr, "inflate: allocated\n"));

  /* reset state */
  inflateReset(z);
  return Z_OK;
}


int SECComp::inflateInit( z_stream *z)
{
  return inflateInit2(z, DEF_WBITS);
}


#define NEEDBYTE {if(z->avail_in==0)return r;r=Z_OK;}
#define NEXTBYTE (z->avail_in--,z->total_in++,*z->next_in++)

int SECComp::inflate( z_stream *z, int f)
{
  inflate_internal_state *pState = (inflate_internal_state *)z->state;

  int r = f;    /* to avoid warning about unused f */
  WORD b;

  if (z == Z_NULL || z->next_in == Z_NULL)
    return Z_STREAM_ERROR;
  r = Z_BUF_ERROR;
  while (1) switch (pState->mode)
  {
    case inflate_internal_state::METHOD:
      NEEDBYTE
      if (((pState->sub.method = NEXTBYTE) & 0xf) != DEFLATED)
      {
        pState->mode = inflate_internal_state::BAD;
        z->msg = _T("unknown compression method");
        pState->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      if ((pState->sub.method >> 4) + 8 > pState->wbits)
      {
        pState->mode = inflate_internal_state::BAD;
        z->msg = _T("invalid window size");
        pState->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      pState->mode = inflate_internal_state::FLAG;
    case inflate_internal_state::FLAG:
      NEEDBYTE
      if ((b = NEXTBYTE) & 0x20)
      {
        pState->mode = inflate_internal_state::BAD;
        z->msg = _T("invalid reserved bit");
        pState->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      if (((pState->sub.method << 8) + b) % 31)
      {
        pState->mode = inflate_internal_state::BAD;
        z->msg = _T("incorrect header check");
        pState->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      ZTrace((stderr, "inflate: zlib header ok\n"));
      pState->mode = inflate_internal_state::BLOCKS;
    case inflate_internal_state::BLOCKS:
      r = inflate_blocks(pState->blocks, z, r);
      if (r == Z_DATA_ERROR)
      {
        pState->mode = inflate_internal_state::BAD;
        pState->sub.marker = 0;       /* can try inflateSync */
        break;
      }
      if (r != Z_STREAM_END)
        return r;
      r = Z_OK;
      inflate_blocks_reset(pState->blocks, z, &pState->sub.check.was);
      if (pState->nowrap)
      {
        pState->mode = inflate_internal_state::DONE;
        break;
      }
      pState->mode = inflate_internal_state::CHECK4;
    case inflate_internal_state::CHECK4:
      NEEDBYTE
      pState->sub.check.need = (DWORD)NEXTBYTE << 24;
      pState->mode = inflate_internal_state::CHECK3;
    case inflate_internal_state::CHECK3:
      NEEDBYTE
      pState->sub.check.need += (DWORD)NEXTBYTE << 16;
      pState->mode = inflate_internal_state::CHECK2;
    case inflate_internal_state::CHECK2:
      NEEDBYTE
      pState->sub.check.need += (DWORD)NEXTBYTE << 8;
      pState->mode = inflate_internal_state::CHECK1;
    case inflate_internal_state::CHECK1:
      NEEDBYTE
      pState->sub.check.need += (DWORD)NEXTBYTE;

      if (pState->sub.check.was != pState->sub.check.need)
      {
        pState->mode = inflate_internal_state::BAD;
        z->msg = _T("incorrect data check");
        pState->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      ZTrace((stderr, "inflate: zlib check ok\n"));
      pState->mode = inflate_internal_state::DONE;
    case inflate_internal_state::DONE:
      return Z_STREAM_END;
    case inflate_internal_state::BAD:
      return Z_DATA_ERROR;
    default:
      return Z_STREAM_ERROR;
  }
}


int SECComp::inflateSync(z_stream *z)
{
  WORD n;       /* number of bytes to look at */
  BYTE *p;     /* pointer to bytes */
  WORD m;       /* number of marker bytes found in a row */
  DWORD r, w;   /* temporaries to save total_in and total_out */
  inflate_internal_state *pState = (inflate_internal_state *)z->state;

  /* set up */
  if (z == Z_NULL || pState == Z_NULL)
    return Z_STREAM_ERROR;
  if (pState->mode != inflate_internal_state::BAD)
  {
    pState->mode = inflate_internal_state::BAD;
    pState->sub.marker = 0;
  }
  if ((n = (WORD)z->avail_in) == 0)
    return Z_BUF_ERROR;
  p = z->next_in;
  m = pState->sub.marker;

  /* search */
  while (n && m < 4)
  {
    if (*p == (BYTE)(m < 2 ? 0 : 0xff))
      m++;
    else if (*p)
      m = 0;
    else
      m = (WORD)((WORD)4 - (WORD)m);
    p++, n--;
  }

  /* restore */
  z->total_in += p - z->next_in;
  z->next_in = p;
  z->avail_in = n;
  pState->sub.marker = m;

  /* return no joy or set up to restart on a new block */
  if (m != 4)
    return Z_DATA_ERROR;
  r = z->total_in;  w = z->total_out;
  inflateReset(z);
  z->total_in = r;  z->total_out = w;
  pState->mode = inflate_internal_state::BLOCKS;
  return Z_OK;
}
