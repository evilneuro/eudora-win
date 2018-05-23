/* infblock.c -- interpret and process block types to last block
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
//AAB #include "infblock.h"
//AAB #include "inftrees.h"
#include "infcodes.h"
#include "infutil.h"

//AAB struct inflate_codes_state {int dummy;}; /* for buggy compilers */

/* Table for deflate from PKZIP's appnote.txt. */
static WORD border[] = { /* Order of the bit length code lengths */
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/*
   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
      stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)  If two
      codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.  Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.  Similarily, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).  Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.  (Pretty obvious.)
  11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.  The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.  It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.
 */


void SECComp::inflate_blocks_reset( inflate_blocks_statef *s, z_stream *z, DWORD *c)
{
  if (s->checkfn != Z_NULL)
    *c = s->check;
  if ((s->mode == inflate_blocks_state::BTREE) || (s->mode == inflate_blocks_state::DTREE))
    ZFREE(z, s->sub.trees.blens);
  if (s->mode == inflate_blocks_state::CODES)
  {
    inflate_codes_free(s->sub.decode.codes, z);
    inflate_trees_free(s->sub.decode.td, z);
    inflate_trees_free(s->sub.decode.tl, z);
  }
  s->mode = inflate_blocks_state::TYPE;
  s->bitk = 0;
  s->bitb = 0;
  s->read = s->write = s->window;
  if (s->checkfn != Z_NULL)
    s->check = (this->*(s->checkfn))(0L, Z_NULL, 0);
  ZTrace((stderr, "inflate:   blocks reset\n"));
}


inflate_blocks_statef* SECComp::inflate_blocks_new(z_stream *z, check_func c, WORD w)
{
  inflate_blocks_statef *s;

  if ((s = (inflate_blocks_statef *)ZALLOC
       (z,1,sizeof(struct inflate_blocks_state))) == Z_NULL)
    return s;
  if ((s->window = (Byte *)ZALLOC(z, 1, w)) == Z_NULL)
  {
    ZFREE(z, s);
    return Z_NULL;
  }
  s->end = s->window + w;
  s->checkfn = c;
  s->mode = inflate_blocks_state::TYPE;
  ZTrace((stderr, "inflate:   blocks allocated\n"));
  inflate_blocks_reset(s, z, &s->check);
  return s;
}


int SECComp::inflate_blocks(inflate_blocks_statef *s, z_stream *z, int r)
{
  WORD t;               /* temporary storage */
  DWORD b;              /* bit buffer */
  WORD k;               /* bits in bit buffer */
  BYTE *p;             /* input data pointer */
  DWORD n;               /* bytes available there */
  BYTE *q;             /* output window write pointer */
  WORD m;               /* bytes to end of window or read pointer */

  /* copy input/output information to locals (UPDATE macro restores) */
  LOAD

  /* process input based on current state */
  while (1) switch (s->mode)
  {
    case inflate_blocks_state::TYPE:
      NEEDBITS(3)
      t = (WORD)((WORD)b & (WORD)7);
      s->last = (WORD)((WORD)t & (WORD)1);
      switch (t >> 1)
      {
        case 0:                         /* stored */
          ZTrace((stderr, "inflate:     stored block%s\n",
                 s->last ? " (last)" : ""));
          DUMPBITS(3)
          t = (WORD)((WORD)k & (WORD)7);                    /* go to byte boundary */
          DUMPBITS(t)
          s->mode = inflate_blocks_state::LENS;               /* get length of stored block */
          break;
        case 1:                         /* fixed */
          ZTrace((stderr, "inflate:     fixed codes block%s\n",
                 s->last ? " (last)" : ""));
          {
            WORD bl, bd;
            inflate_huft *tl, *td;

            inflate_trees_fixed(&bl, &bd, &tl, &td);
            s->sub.decode.codes = inflate_codes_new(bl, bd, tl, td, z);
            if (s->sub.decode.codes == Z_NULL)
            {
              r = Z_MEM_ERROR;
              LEAVE
            }
            s->sub.decode.tl = Z_NULL;  /* don't try to free these */
            s->sub.decode.td = Z_NULL;
          }
          DUMPBITS(3)
          s->mode = inflate_blocks_state::CODES;
          break;
        case 2:                         /* dynamic */
          ZTrace((stderr, "inflate:     dynamic codes block%s\n",
                 s->last ? " (last)" : ""));
          DUMPBITS(3)
          s->mode = inflate_blocks_state::TABLE;
          break;
        case 3:                         /* illegal */
          DUMPBITS(3)
          s->mode = inflate_blocks_state::BAD;
          z->msg = _T("invalid block type");
          r = Z_DATA_ERROR;
          LEAVE
      }
      break;
    case inflate_blocks_state::LENS:
      NEEDBITS(32)
      if (((~b) >> 16) != (b & 0xffff))
      {
        s->mode = inflate_blocks_state::BAD;
        z->msg = _T("invalid stored block lengths");
        r = Z_DATA_ERROR;
        LEAVE
      }
      s->sub.left = (WORD)((WORD)b & (WORD)0xffff);
      b = k = 0;                      /* dump bits */
      ZTracev((stderr, "inflate:       stored length %u\n", s->sub.left));
      s->mode = s->sub.left ? inflate_blocks_state::STORED : inflate_blocks_state::TYPE;
      break;
    case inflate_blocks_state::STORED:
      if (n == 0)
        LEAVE
      NEEDOUT
      t = s->sub.left;
      if (t > n) t = (WORD)n;
      if (t > m) t = m;
      zmemcpy(q, p, t);
      p += t;  n = (WORD)((WORD)n - (WORD)t);
      q += t;  m = (WORD)((WORD)m - (WORD)t);
      s->sub.left = (WORD)((WORD)(s->sub.left) - (WORD)t);
	  if (s->sub.left != 0)
        break;
      ZTracev((stderr, "inflate:       stored end, %lu total out\n",
              z->total_out + (q >= s->read ? q - s->read :
              (s->end - s->read) + (q - s->window))));
      s->mode = s->last ? inflate_blocks_state::DRY : inflate_blocks_state::TYPE;
      break;
    case inflate_blocks_state::TABLE:
      NEEDBITS(14)
      s->sub.trees.table = t = (WORD)((WORD)b & (WORD)0x3fff);
#ifndef PKZIP_BUG_WORKAROUND
      if ((t & 0x1f) > 29 || ((t >> 5) & 0x1f) > 29)
      {
        s->mode = inflate_blocks_state::BAD;
        z->msg = _T("too many length or distance symbols");
        r = Z_DATA_ERROR;
        LEAVE
      }
#endif
      t = (WORD)((WORD)258 + (WORD)(t & 0x1f) + (WORD)((t >> 5) & 0x1f));
      if (t < 19)
        t = 19;
      if ((s->sub.trees.blens = (WORD *)ZALLOC(z, t, sizeof(WORD))) == Z_NULL)
      {
        r = Z_MEM_ERROR;
        LEAVE
      }
      DUMPBITS(14)
      s->sub.trees.index = 0;
      ZTracev((stderr, "inflate:       table sizes ok\n"));
      s->mode = inflate_blocks_state::BTREE;
    case inflate_blocks_state::BTREE:
      while (s->sub.trees.index < 4 + (s->sub.trees.table >> 10))
      {
        NEEDBITS(3)
        s->sub.trees.blens[border[s->sub.trees.index++]] = (WORD)((WORD)b & (WORD)7);
        DUMPBITS(3)
      }
      while (s->sub.trees.index < 19)
        s->sub.trees.blens[border[s->sub.trees.index++]] = 0;
      s->sub.trees.bb = 7;
      t = (WORD)inflate_trees_bits(s->sub.trees.blens, &s->sub.trees.bb,
                             &s->sub.trees.tb, z);
      if (t != Z_OK)
      {
        r = t;
        if (r == Z_DATA_ERROR)
          s->mode = inflate_blocks_state::BAD;
        LEAVE
      }
      s->sub.trees.index = 0;
      ZTracev((stderr, "inflate:       bits tree ok\n"));
      s->mode = inflate_blocks_state::DTREE;
    case inflate_blocks_state::DTREE:
      while (t = s->sub.trees.table,
             s->sub.trees.index < 258 + (t & 0x1f) + ((t >> 5) & 0x1f))
      {
        inflate_huft *h;
        WORD i, j, c;

        t = s->sub.trees.bb;
        NEEDBITS(t)
        h = s->sub.trees.tb + ((WORD)b & inflate_mask[t]);
        t = h->word.what.Bits;
        c = h->more.Base;
        if (c < 16)
        {
          DUMPBITS(t)
          s->sub.trees.blens[s->sub.trees.index++] = c;
        }
        else /* c == 16..18 */
        {
          i = (WORD)(c == (WORD)18 ? (WORD)7 : c - (WORD)14);
          j = (WORD)(c == (WORD)18 ? (WORD)11 : (WORD)3);
          NEEDBITS(t + i)
          DUMPBITS(t)
          j = (WORD)((WORD)j + (WORD)((WORD)b & (WORD)inflate_mask[i]));
          DUMPBITS(i)
          i = s->sub.trees.index;
          t = s->sub.trees.table;
          if (i + j > 258 + (t & 0x1f) + ((t >> 5) & 0x1f) ||
              (c == 16 && i < 1))
          {
            s->mode = inflate_blocks_state::BAD;
            z->msg = _T("invalid bit length repeat");
            r = Z_DATA_ERROR;
            LEAVE
          }
          c = (WORD)(c == 16 ? s->sub.trees.blens[i - 1] : 0);
          do {
            s->sub.trees.blens[i++] = c;
			--j;
          } while (j);
          s->sub.trees.index = i;
        }
      }
      inflate_trees_free(s->sub.trees.tb, z);
      s->sub.trees.tb = Z_NULL;
      {
        WORD bl, bd;
        inflate_huft *tl, *td;
        inflate_codes_statef *c;

        bl = 9;         /* must be <= 9 for lookahead assumptions */
        bd = 6;         /* must be <= 9 for lookahead assumptions */
        t = s->sub.trees.table;
        t = (WORD)inflate_trees_dynamic( (WORD)((WORD)257 + (WORD)((WORD)t & (WORD)0x1f)), 
		(WORD)((WORD)1 + (WORD)(((WORD)t >> (WORD)5) & (WORD)0x1f)),
                (s->sub.trees.blens), &bl, &bd, &tl, &td, z);
        if (t != Z_OK)
        {
          if (t == (WORD)Z_DATA_ERROR)
            s->mode = inflate_blocks_state::BAD;
          r = t;
          LEAVE
        }
        ZTracev((stderr, "inflate:       trees ok\n"));
        if ((c = inflate_codes_new(bl, bd, tl, td, z)) == Z_NULL)
        {
          inflate_trees_free(td, z);
          inflate_trees_free(tl, z);
          r = Z_MEM_ERROR;
          LEAVE
        }
        ZFREE(z, s->sub.trees.blens);
        s->sub.decode.codes = c;
        s->sub.decode.tl = tl;
        s->sub.decode.td = td;
      }
      s->mode = inflate_blocks_state::CODES;
    case inflate_blocks_state::CODES:
      UPDATE
      if ((r = inflate_codes(s, z, r)) != Z_STREAM_END)
        return inflate_flush(s, z, r);
      r = Z_OK;
      inflate_codes_free(s->sub.decode.codes, z);
      inflate_trees_free(s->sub.decode.td, z);
      inflate_trees_free(s->sub.decode.tl, z);
      LOAD
      ZTracev((stderr, "inflate:       codes end, %lu total out\n",
              z->total_out + (q >= s->read ? q - s->read :
              (s->end - s->read) + (q - s->window))));
      if (!s->last)
      {
        s->mode = inflate_blocks_state::TYPE;
        break;
      }
      if (k > 7)              /* return unused byte, if any */
      {
        Assert(k < 16, "inflate_codes grabbed too many bytes")
        k -= 8;
        n++;
        p--;                    /* can always return one */
      }
      s->mode = inflate_blocks_state::DRY;
    case inflate_blocks_state::DRY:
      FLUSH
      if (s->read != s->write)
        LEAVE
      s->mode = inflate_blocks_state::DONE;
    case inflate_blocks_state::DONE:
      r = Z_STREAM_END;
      LEAVE
    case inflate_blocks_state::BAD:
      r = Z_DATA_ERROR;
      LEAVE
    default:
      r = Z_STREAM_ERROR;
      LEAVE
  }
}


int SECComp::inflate_blocks_free( inflate_blocks_statef *s, z_stream *z, DWORD *c)
{
  inflate_blocks_reset(s, z, c);
  ZFREE(z, s->window);
  ZFREE(z, s);
  ZTrace((stderr, "inflate:   blocks freed\n"));
  return Z_OK;
}
