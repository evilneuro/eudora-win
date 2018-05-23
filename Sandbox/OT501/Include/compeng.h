// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC Compression
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	12/29/95	Started
//
//


#ifndef __SECCOMP_H__
#define __SECCOMP_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#include "zconf.h"
#include "deflate.h"

#ifndef __SC__
class __single_inheritance SECComp;
#else
class SECComp;
#endif

typedef voidpf (SECComp::*alloc_func) (voidpf opaque, WORD items, WORD size);
typedef void   (SECComp::*comp_free_func)  (voidpf opaque, voidpf address);
typedef DWORD (SECComp::*check_func) (DWORD check, BYTE *buf, WORD len);
typedef voidp gzFile;

struct inflate_internal_state;
struct deflate_internal_state;
struct internal_state;

typedef struct z_stream_s {
    BYTE    *next_in;  /* next input byte */
    DWORD     avail_in;  /* number of bytes available at next_in */
    DWORD    total_in;  /* total nb of input bytes read so far */

    void FAR    *next_out; /* next output byte should be put there */
    DWORD     avail_out; /* remaining free space at next_out */
    DWORD    total_out; /* total nb of bytes output so far */

    TCHAR    *msg;      /* last error message, NULL if no error */
    struct internal_state *state; /* not visible by applications */
    // struct inflate_internal_state *inf_state; /* not visible by applications */
    // struct deflate_internal_state *def_state; /* not visible by applications */

    alloc_func zalloc;  /* used to allocate the internal state */
    comp_free_func  zfree;   /* used to free the internal state */
    voidp      opaque;  /* private data object passed to zalloc and zfree */

    BYTE     data_type; /* best guess about the data type: ascii or binary */

} z_stream; 

// from gzio.cpp
typedef struct gz_stream {
    z_stream stream;
    int      z_err;   /* error code for last stream operation */
    int      z_eof;   /* set if end of input file */
    CFile     *pFile;   /* .gz file */
    BYTE     *inbuf;  /* input buffer */
    BYTE     *outbuf; /* output buffer */
    DWORD    crc;     /* crc32 of uncompressed data */
    TCHAR    *msg;    /* error message */
    TCHAR    *path;   /* path name for debugging only */
    int      transparent; /* 1 if input file is not a .gz file */
    char     mode;    /* 'w' or 'r' */
} gz_stream;

struct inflate_blocks_state;
// typedef struct inflate_blocks_state FAR inflate_blocks_statef;
typedef struct inflate_blocks_state inflate_blocks_statef;
struct inflate_codes_state;
typedef struct inflate_codes_state FAR inflate_codes_statef;

typedef struct inflate_huft_s inflate_huft;
// from inftrees.cpp
struct inflate_huft_s {
  union {
    struct {
      BYTE Exop;        /* number of extra bits or operation */
      BYTE Bits;        /* number of bits in this code or subcode */
    } what;
    BYTE *pad;         /* pad structure to a power of 2 (4 bytes for */
  } word;               /*  16-bit, 8 bytes for 32-bit machines) */
  union {
    WORD Base;          /* literal, length base, or distance base */
    inflate_huft *Next; /* pointer to next level of table */
  } more;
};


// from infcodes.cpp
/* inflate codes private state */
struct inflate_codes_state {

  /* mode */
  enum {        /* waiting for "i:"=input, "o:"=output, "x:"=nothing */
      START,    /* x: set up for LEN */
      LEN,      /* i: get length/literal/eob next */
      LENEXT,   /* i: getting length extra (have base) */
      DIST,     /* i: get distance next */
      DISTEXT,  /* i: getting distance extra */
      COPY,     /* o: copying bytes in window, waiting for space */
      LIT,      /* o: got literal, waiting for output space */
      WASH,     /* o: got eob, possibly still output waiting */
      END,      /* x: got eob and all data flushed */
      BADCODE}  /* x: got error */
    mode;               /* current inflate_codes mode */

  /* mode dependent information */
  WORD len;
  union {
    struct {
      inflate_huft *tree;       /* pointer into tree */
      WORD need;                /* bits needed */
    } code;             /* if LEN or DIST, where in tree */
    WORD lit;           /* if LIT, literal */
    struct {
      WORD get;                 /* bits to get for extra */
      WORD dist;                /* distance back to copy from */
    } copy;             /* if EXT or COPY, where and how much */
  } sub;                /* submode */

  /* mode independent information */
  Byte lbits;           /* ltree bits decoded per branch */
  Byte dbits;           /* dtree bits decoder per branch */
  inflate_huft *ltree;          /* literal/length/eob tree */
  inflate_huft *dtree;          /* distance tree */

};

 struct inflate_blocks_state {

  /* mode */
  enum {
      TYPE,     /* get type bits (3, including end bit) */
      LENS,     /* get lengths for stored */
      STORED,   /* processing stored block */
      TABLE,    /* get table lengths */
      BTREE,    /* get bit lengths tree for a dynamic block */
      DTREE,    /* get length, distance trees for a dynamic block */
      CODES,    /* processing fixed or dynamic block */
      DRY,      /* output remaining window bytes */
      DONE,     /* finished last block, done */
      BAD}      /* got a data error--stuck here */
    mode;               /* current inflate_block mode */

  /* mode dependent information */
  union {
    WORD left;          /* if STORED, bytes left to copy */
    struct {
      WORD table;               /* table lengths (14 bits) */
      WORD index;               /* index into blens (or border) */
      WORD *blens;             /* bit lengths of codes */
      WORD bb;                  /* bit length tree depth */
      inflate_huft *tb;         /* bit length decoding tree */
    } trees;            /* if DTREE, decoding info for trees */
    struct {
      inflate_huft *tl, *td;    /* trees to free */
      inflate_codes_statef 
         *codes;
    } decode;           /* if CODES, current state */
  } sub;                /* submode */
  WORD last;            /* true if this block is the last block */

  /* mode independent information */
  WORD bitk;            /* bits in bit buffer */
  DWORD bitb;           /* bit buffer */
  BYTE *window;        /* sliding window */
  BYTE *end;           /* one byte after sliding window */
  BYTE *read;          /* window read pointer */
  BYTE *write;         /* window write pointer */
  check_func checkfn;   /* check function */
  DWORD check;          /* check on output */

};

/* inflate private state */
struct inflate_internal_state {

  /* mode */
  enum {
      METHOD,   /* waiting for method byte */
      FLAG,     /* waiting for flag byte */
      BLOCKS,   /* decompressing blocks */
      CHECK4,   /* four check bytes to go */
      CHECK3,   /* three check bytes to go */
      CHECK2,   /* two check bytes to go */
      CHECK1,   /* one check byte to go */
      DONE,     /* finished check, done */
      BAD}      /* got an error--stay here */
    mode;               /* current inflate mode */

  /* mode dependent information */
  union {
    WORD method;        /* if FLAGS, method byte */
    struct {
      DWORD was;                /* computed check value */
      DWORD need;               /* stream check value */
    } check;            /* if CHECK, check values to compare */
    WORD marker;        /* if BAD, inflateSync's marker bytes count */
  } sub;        /* submode */

  /* mode independent information */
  int  nowrap;          /* flag for no wrapper */
  WORD wbits;           /* log2(window size)  (8..15, defaults to 15) */
  inflate_blocks_statef 
    *blocks;            /* current inflate_blocks state */

};
/* Data structure describing a single value and its code string. */
typedef struct ct_data_s {
    union {
        ush  freq;       /* frequency count */
        ush  code;       /* bit string */
    } fc;
    union {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} ct_data;

typedef struct static_tree_desc_s  static_tree_desc;

typedef struct tree_desc_s {
    ct_data *dyn_tree;           /* the dynamic tree */
    int     max_code;            /* largest code with non zero frequency */
    static_tree_desc *stat_desc; /* the corresponding static tree */
} FAR tree_desc;

typedef ush Pos;
typedef Pos FAR Posf;
typedef int IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */
typedef struct deflate_internal_state {
    z_stream *strm;      /* pointer back to this zlib stream */
    int   status;        /* as the name implies */
    BYTE *pending_buf;  /* output still pending */
    BYTE *pending_out;  /* next pending byte to output to the stream */
    int   pending;       /* nb of bytes in the pending buffer */
    DWORD adler;         /* adler32 of uncompressed data */
    int   noheader;      /* suppress zlib header and adler32 */
    Byte  data_type;     /* UNKNOWN, BINARY or ASCII */
    Byte  method;        /* STORED (for zip only) or DEFLATED */

                /* used by deflate.c: */

    WORD  w_size;        /* LZ77 window size (32K by default) */  
    WORD  w_bits;        /* log2(w_size)  (8..16) */
    WORD  w_mask;        /* w_size - 1 */

    BYTE *window;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg window_size;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *head; /* Heads of the hash chains or NIL. */

    DWORD  ins_h;          /* hash index of string to be inserted */
    DWORD  hash_size;      /* number of elements in hash table */
    DWORD  hash_bits;      /* log2(hash_size) */
    DWORD  hash_mask;      /* hash_size-1 */

    DWORD  hash_shift;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    long block_start;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    DWORD match_length;           /* length of best match */
    IPos prev_match;             /* previous match */
    int match_available;         /* set if previous match exists */
    WORD strstart;               /* start of string to insert */
    WORD match_start;            /* start of matching string */
    WORD lookahead;              /* number of valid bytes ahead in window */

    WORD prev_length;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    WORD max_chain_length;

    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    WORD max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_insert_length  max_lazy_match
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    int level;    /* compression level (1..9) */
    int strategy; /* favor or force Huffman coding*/

    DWORD good_match;
    /* Use a faster search when the previous match is longer than this */

     int nice_match; /* Stop searching when current match exceeds this */

                /* used by trees.c: */

    /* Didn't use ct_data typedef below to supress compiler warning */
    struct ct_data_s dyn_ltree[HEAP_SIZE];   /* literal and length tree */
    struct ct_data_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    struct ct_data_s bl_tree[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    struct tree_desc_s l_desc;               /* desc. for literal tree */
    struct tree_desc_s d_desc;               /* desc. for distance tree */
    struct tree_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_count[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    int heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    int heap_len;               /* number of elements in the heap */
    int heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch depth[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */
    
    BYTE filler;	//AAB: Needed for packing warning
    uchf *l_buf;          /* buffer for literals or lengths */

    DWORD  lit_bufsize;
    /* Size of match buffer for literals/lengths.  There are 4 reasons for
     * limiting lit_bufsize to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input
     *     data is still in the window so we can still emit a stored block even
     *     when input comes from standard input.  (This can also be done for
     *     all blocks if lit_bufsize is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *     This is applicable only for zip (not gzip or zlib).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting
     *     trees more frequently.
     *   - I can't count above 4
     */

    DWORD last_lit;      /* running index in l_buf */

    ushf *d_buf;
    /* Buffer for distances. To simplify the code, d_buf and l_buf have
     * the same number of elements. To use different lengths, an extra flag
     * array would be necessary.
     */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    ulg compressed_len; /* total bit length of compressed file */
    WORD matches;       /* number of string matches in current block */
    int last_eob_len;   /* bit length of EOB code for last block */

#ifdef DEBUG
    ulg bits_sent;      /* bit length of the compressed data */
#endif

    ush bi_buf;

    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    int bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */

} deflate_state;

   
// struct internal_state      {int dummy;}; /* for buggy compilers */
// struct inflate_blocks_state {int dummy;}; /* for buggy compilers */


/* constants */
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1
#define Z_FULL_FLUSH    2
#define Z_SYNC_FLUSH    3 /* experimental: partial_flush + byte align */
#define Z_FINISH        4
/* See deflate() below for the usage of these constants */

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
/* error codes for the compression/decompression functions */

#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Used to set the data_type field */

#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

#define FIXEDH 530


class SECComp: public CObject
{
    DECLARE_SERIAL(SECComp);

// Construction
public:
    SECComp();

// Attributes
	int crc_table_empty;
	int fixed_lock;
	int fixed_built;
	WORD fixed_left;
	inflate_huft fixed_mem[FIXEDH];
	WORD fixed_bl;
	WORD fixed_bd;
	inflate_huft *fixed_tl;
	inflate_huft *fixed_td;

// Operations
public:

// Implementation
public:
	int deflateInit (z_stream *strm, int level);
	int deflate (z_stream *strm, int flush);
	int deflateEnd (z_stream *strm);
	int inflateInit (z_stream *strm);
	int inflate (z_stream *strm, int flush); 
	int inflateEnd (z_stream *strm);
	int deflateInit2 (z_stream *strm, int level, int method, int windowBits,
                            int memLevel, int strategy); 
    int deflateCopy (z_stream *dest, z_stream *source); 
    int deflateReset (z_stream *strm); 
    int inflateInit2 (z_stream *strm, int windowBits); 
    int inflateSync (z_stream *strm);
    int inflateReset (z_stream *strm); 
    int compress (BYTE *dest,   DWORD *destLen,
                        BYTE *source, DWORD sourceLen); 
    int uncompress (BYTE *dest,   DWORD *destLen,
                          BYTE *source, DWORD sourceLen); 
    gzFile gzopen (TCHAR *path, TCHAR *mode); 
    gzFile gzdopen (int fd, TCHAR *mode); 
    DWORD gzread (gzFile file, void FAR *buf, long len);
    long gzwrite (gzFile file, BYTE *buf, long len); 
    int gzflush (gzFile file, int flush); 
    int gzclose (gzFile file);
    char* gzerror (gzFile file, int *errnum); 
    DWORD adler32 (DWORD adler, BYTE *buf, WORD len); 
    // DWORD crc32(DWORD crc, BYTE *buf, WORD len); 
    DWORD crc32(DWORD crc, void FAR *buf, WORD len);

public:
	// routines that used to be static
	void make_crc_table();

	// from deflate.cpp
	void fill_window(deflate_state *s);
	int deflate_fast(deflate_state *s, int flush);
	int deflate_slow(deflate_state *s, int flush);
	// AAB int lm_init (deflate_state *s, int flush);
	void lm_init (deflate_state *s);
	int longest_match(deflate_state *s, IPos cur_match);
	void putShortMSB(deflate_state *s, WORD b);
	void flush_pending(z_stream *strm);
	long read_buf(z_stream *strm, char *buf, long size);

	// from trees.cpp
	void ct_init (deflate_state *s);
	int ct_tally (deflate_state *s, long dist, long lc);
	ulg ct_flush_block (deflate_state *s, char *buf, ulg stored_len, int eof);
	void ct_align (deflate_state *s);
	void ct_stored_block (deflate_state *s, char *buf, ulg stored_len, int eof);

	// from gzio.cpp	
	int destroy (gz_stream *s);
	gzFile gz_open (TCHAR *path, TCHAR *mode, int  fd, CFile *pFile=0);
	void   putLong (CFile *pFile, DWORD x);
	DWORD getLong (BYTE *buf);   
	
	// from infblock.cpp

	inflate_blocks_state* inflate_blocks_new ( z_stream *z, check_func c, WORD w);
	int inflate_blocks ( inflate_blocks_statef *, z_stream *, int); 
	void inflate_blocks_reset ( inflate_blocks_statef *, z_stream *, DWORD *);
	int inflate_blocks_free ( inflate_blocks_statef *, z_stream *, DWORD *);
	
	// from infcodes.cpp
	inflate_codes_statef *inflate_codes_new (WORD, WORD, inflate_huft *, inflate_huft *, z_stream *);
	int inflate_codes ( inflate_blocks_statef *, z_stream *, int);
	void inflate_codes_free ( inflate_codes_statef *, z_stream *);  
	
	// from inffast.cpp
	int inflate_fast ( WORD, WORD, inflate_huft *, inflate_huft *, inflate_blocks_statef *, z_stream *);
   
   	// from inftrees.cpp 
#ifdef DEBUG
  WORD inflate_hufts;
#endif

	int inflate_trees_bits ( WORD *, WORD *, inflate_huft **, z_stream *);            
	int inflate_trees_dynamic ( WORD, WORD, WORD *, WORD *, WORD *, inflate_huft **, 
		inflate_huft **, z_stream *);       
	int inflate_trees_fixed ( WORD *, WORD *, inflate_huft **, inflate_huft **);
	int inflate_trees_free ( inflate_huft *, z_stream *);
    
    
    int huft_build ( WORD *, WORD, WORD, WORD *, WORD *, inflate_huft **, WORD *, z_stream *);
	voidpf falloc ( voidpf, WORD, WORD);
	void ffree (voidpf q, voidpf p);

	// infutil.cpp
	int inflate_flush ( inflate_blocks_statef *, z_stream *, int); 

	// inflate.cpp	
	void ct_static_init (void);
	void init_block     (deflate_state *s);
	void pqdownheap     (deflate_state *s, ct_data *tree, int k);
	void gen_bitlen     (deflate_state *s, tree_desc *desc);
	void gen_codes      (ct_data *tree, int max_code, ushf *bl_count);
	void build_tree     (deflate_state *s, tree_desc *desc);
	void scan_tree      (deflate_state *s, ct_data *tree, int max_code);
	void send_tree      (deflate_state *s, ct_data *tree, int max_code);
	int  build_bl_tree  (deflate_state *s);
	void send_all_trees (deflate_state *s, int lcodes, int dcodes,
                              int blcodes);
	void compress_block (deflate_state *s, ct_data *ltree,
                              ct_data *dtree);
	void set_data_type  (deflate_state *s);
	unsigned bi_reverse (unsigned value, int length);
	void bi_windup      (deflate_state *s);
	void bi_flush       (deflate_state *s);
	void copy_block     (deflate_state *s, char *buf, unsigned len,
                              int header);
#ifdef DEBUG
	void send_bits (deflate_state *s, int value, int length);    
	void check_match(deflate_state *s, IPos start, IPos match, int length);
#endif   

	// zutil.h
	void z_error    (char *m);
	voidpf zcalloc (voidpf opaque, WORD items, WORD size);
	void   zcfree  (voidpf opaque, voidpf ptr);
	   
public:
    virtual ~SECComp();
    virtual void Serialize(CArchive& ar);
#ifdef _DEBUG
	void AssertValid() const;
#endif

};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif // __SECCOMP_H__
