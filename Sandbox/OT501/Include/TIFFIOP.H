/* $Header: /usr/people/sam/tiff/libtiff/RCS/tiffiop.h,v 1.44 93/08/26 14:29:34 sam Exp $ */

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992 Sam Leffler
 * Copyright (c) 1991, 1992 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef _TIFFIOP_
#define	_TIFFIOP_
/*
 * ``Library-private'' definitions.
 */
 
#ifndef WIN32
#pragma warning(disable : 4103)
#endif

#include <windows.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

#include "tiffconf.h"
#include "tiffcomp.h"
#include "tiffio.h"

//AAB made changes in this stuff
typedef BYTE RGBvalue;
#ifdef WIN32
typedef LPWORD* LPPWORD;
typedef LPDWORD* LPPDWORD;
typedef float* LPFLOAT;
typedef LPFLOAT* LPPFLOAT;
typedef LPBYTE* LPPBYTE;
typedef LPPBYTE* LPPPBYTE;
typedef RGBvalue* LPRGBvalue;
typedef LPRGBvalue* LPPRGBvalue;
#else
typedef LPWORD FAR* LPPWORD;
typedef LPDWORD FAR* LPPDWORD;
typedef float FAR* LPFLOAT;
typedef LPFLOAT FAR* LPPFLOAT;
typedef LPBYTE FAR* LPPBYTE;
typedef LPPBYTE FAR* LPPPBYTE;
typedef RGBvalue FAR* LPRGBvalue;
typedef LPRGBvalue FAR* LPPRGBvalue;
#endif

/*
 * Internal format of a TIFF directory entry.
 */
typedef	struct {
#define	FIELD_SETLONGS	2
	/* bit vector of fields that are set */
	DWORD	td_fieldsset[FIELD_SETLONGS];

	DWORD	td_imagewidth, td_imagelength, td_imagedepth;
	DWORD	td_tilewidth, td_tilelength, td_tiledepth;
	WORD	td_subfiletype;
	WORD	td_bitspersample;
	WORD	td_sampleformat;
	WORD	td_compression;
	WORD	td_photometric;
	WORD	td_threshholding;
	WORD	td_fillorder;
	WORD	td_orientation;
	WORD	td_samplesperpixel;
	WORD	td_predictor;
	DWORD	td_rowsperstrip;
	DWORD	td_minsamplevalue, td_maxsamplevalue;	/* XXX */
	float	td_xresolution, td_yresolution;
	WORD	td_resolutionunit;
	WORD	td_planarconfig;
	float	td_xposition, td_yposition;
	DWORD	td_group3options;
	DWORD	td_group4options;
	WORD	td_pagenumber[2];
	WORD	td_cleanfaxdata;
	WORD	td_badfaxrun;
	DWORD	td_badfaxlines;
	LPWORD	td_colormap[3];
	WORD	td_halftonehints[2];
	WORD	td_extrasamples;
	LPWORD	td_sampleinfo;
	TCHAR	*td_documentname;
	TCHAR	*td_artist;
	TCHAR	*td_datetime;
	TCHAR	*td_hostcomputer;
	TCHAR	*td_imagedescription;
	TCHAR	*td_make;
	TCHAR	*td_model;
	TCHAR	*td_software;
	TCHAR	*td_pagename;
	tstrip_t td_stripsperimage;
	tstrip_t td_nstrips;		/* size of offset & bytecount arrays */
	LPDWORD td_stripoffset;
	LPDWORD td_stripbytecount;
#ifdef YCBCR_SUPPORT
	LPFLOAT td_ycbcrcoeffs;
	WORD	td_ycbcrsubsampling[2];
	WORD	td_ycbcrpositioning;
#endif
#ifdef JPEG_SUPPORT
	WORD	td_jpegproc;
	WORD	td_jpegrestartinterval;
	LPPBYTE td_qtab;
	LPPBYTE td_dctab;
	LPPBYTE td_actab;
#endif
#ifdef COLORIMETRY_SUPPORT
	LPFLOAT td_whitepoint;
	LPFLOAT td_primarychromas;
	LPFLOAT td_refblackwhite;
	LPWORD td_transferfunction[3];
#endif

#ifdef CMYK_SUPPORT
	WORD	td_inkset;
	WORD	td_dotrange[2];
	TCHAR	*td_inknames;
	TCHAR	*td_targetprinter;
#endif
} TIFFDirectory;

/*
 * Field flags used to indicate fields that have
 * been set in a directory, and to reference fields
 * when manipulating a directory.
 */
/* multi-entry fields */
#define	FIELD_IMAGEDIMENSIONS		0
#define FIELD_TILEDIMENSIONS		1
#define	FIELD_CELLDIMENSIONS		2		/* XXX */
#define	FIELD_RESOLUTION		3
#define	FIELD_POSITION			4
/* single-entry fields */
#define	FIELD_SUBFILETYPE		5
#define	FIELD_BITSPERSAMPLE		6
#define	FIELD_COMPRESSION		7
#define	FIELD_PHOTOMETRIC		8
#define	FIELD_THRESHHOLDING		9
#define	FIELD_FILLORDER			10
#define	FIELD_DOCUMENTNAME		11
#define	FIELD_IMAGEDESCRIPTION		12
#define	FIELD_MAKE			13
#define	FIELD_MODEL			14
#define	FIELD_ORIENTATION		15
#define	FIELD_SAMPLESPERPIXEL		16
#define	FIELD_ROWSPERSTRIP		17
#define	FIELD_MINSAMPLEVALUE		18
#define	FIELD_MAXSAMPLEVALUE		19
#define	FIELD_PLANARCONFIG		20
#define	FIELD_PAGENAME			21
#define	FIELD_GROUP3OPTIONS		22
#define	FIELD_GROUP4OPTIONS		23
#define	FIELD_RESOLUTIONUNIT		24
#define	FIELD_PAGENUMBER		25
#define	FIELD_STRIPBYTECOUNTS		26
#define	FIELD_STRIPOFFSETS		27
#define	FIELD_COLORMAP			28
#define FIELD_PREDICTOR			29
#define FIELD_ARTIST			30
#define FIELD_DATETIME			31
#define FIELD_HOSTCOMPUTER		32
#define FIELD_SOFTWARE			33
#define	FIELD_EXTRASAMPLES		34
#define	FIELD_BADFAXLINES		35
#define	FIELD_CLEANFAXDATA		36
#define	FIELD_BADFAXRUN			37
#define FIELD_SAMPLEFORMAT		38
#define	FIELD_SMINSAMPLEVALUE		39
#define	FIELD_SMAXSAMPLEVALUE		40
#define FIELD_IMAGEDEPTH		41
#define FIELD_TILEDEPTH			42
#define	FIELD_HALFTONEHINTS		43
#ifdef YCBCR_SUPPORT
#define FIELD_YCBCRCOEFFICIENTS		44
#define FIELD_YCBCRSUBSAMPLING		45
#define FIELD_YCBCRPOSITIONING		46
#endif
#ifdef JPEG_SUPPORT
#define FIELD_JPEGPROC			47
#define FIELD_JPEGRESTARTINTERVAL	48
#define FIELD_JPEGQTABLES		49
#define FIELD_JPEGDCTABLES		50
#define FIELD_JPEGACTABLES		51
#endif
#ifdef COLORIMETRY_SUPPORT
#define	FIELD_REFBLACKWHITE		52
#define	FIELD_WHITEPOINT		53
#define	FIELD_PRIMARYCHROMAS		54
#define	FIELD_TRANSFERFUNCTION		55
#endif
#ifdef CMYK_SUPPORT
#define	FIELD_INKSET			56
#define	FIELD_INKNAMES			57
#define	FIELD_DOTRANGE			58
#define	FIELD_TARGETPRINTER		59
#endif
#define	FIELD_LAST			59

#define	TIFFExtractData(tif, type, v) \
    ((tif)->tif_header.tiff_magic == TIFF_BIGENDIAN ? \
        ((v) >> (tif)->tif_typeshift[type]) & (tif)->tif_typemask[type] : \
	(v) & (tif)->tif_typemask[type])
#define	TIFFInsertData(tif, type, v) \
    ((tif)->tif_header.tiff_magic == TIFF_BIGENDIAN ? \
        ((v) & (tif)->tif_typemask[type]) << (tif)->tif_typeshift[type] : \
	(v) & (tif)->tif_typemask[type])

 typedef struct {
	ttag_t	field_tag;		/* field's tag */
	short	field_readcount;	/* read count (-1 for unknown) */
	short	field_writecount;	/* write count (-1 for unknown) */
	TIFFDataType field_type;	/* type of associated data */
	WORD	field_bit;		/* bit in fieldsset bit vector */
	WORD	field_oktochange;	/* if true, can change while writing */
	TCHAR	*field_name;		/* ASCII name */
#ifdef WIN32
}  TIFFFieldInfo;
#else // WIN16
} TIFFFieldInfo;
#endif  // WIN32

#define	FIELD_IGNORE	((WORD)-1)	/* tags processed but ignored */

#define	TIFF_ANY	TIFF_NOTYPE	/* for field descriptor searching */
#define	TIFF_VARIABLE	-1		/* marker for variable length tags */
#define	TIFF_SPP	-2		/* marker for SamplesPerPixel tags */

extern	const TIFFFieldInfo tiffFieldInfo[100];/* table of field descriptors */

#ifdef WIN32
extern	const long tiffDataWidth[];	/* table of tag datatype widths */
#else
extern const long __far tiffDataWidth[]; /* table of tag datatype widths */
#endif

#define BITn(n)				(((DWORD)1L)<<((n)&0x1f))
#define BITFIELDn(tif, n)		((tif)->tif_dir.td_fieldsset[(n)/32])
#define TIFFFieldSet(tif, field)	(BITFIELDn(tif, field) & BITn(field))
#define TIFFSetFieldBit(tif, field)	(BITFIELDn(tif, field) |= BITn(field))
#define TIFFClrFieldBit(tif, field)	(BITFIELDn(tif, field) &= ~BITn(field))

#define	FieldSet(fields, f)		(fields[(f)/32] & BITn(f))
#define	ResetFieldBit(fields, f)	(fields[(f)/32] &= ~BITn(f))

/*
 * Typedefs for ``method pointers'' used internally.
 */
#ifdef WIN32
//AAB? typedef	char* tidata_t;	/* internal image data type */
typedef LPBYTE tidata_t;
#else
//AAB CHECK 12/10 typedef LPBYTE tidata_t; 
typedef BYTE huge * tidata_t;
#endif

#ifndef __SC__
class __single_inheritance SECTiff;
#else
class SECTiff;
#endif

//AAB typedef	void (*TIFFVoidMethod)(TIFF*);
typedef	void (SECTiff::*TIFFVoidMethod)(TIFF*);
typedef	long (SECTiff::*TIFFBoolMethod)(TIFF*);
typedef	long (SECTiff::*TIFFCodeMethod)(TIFF*, tidata_t, tsize_t, tsample_t);
typedef	long (SECTiff::*TIFFSeekMethod)(TIFF*, DWORD);
typedef	void (SECTiff::*TIFFPostMethod)(TIFF*, tidata_t, tsize_t);

struct tiff {
	TCHAR*		tif_name;	/* name of open file */
	long		tif_fd;		/* open file descriptor */
	//short 	tif_mode;	/* open mode (O_*) */
	UINT		tif_mode;	/* open mode (O_*) */
	char		tif_fillorder;	/* natural bit fill order for machine */
	char		tif_options;	/* compression-specific options */
	short		tif_flags;
#define	TIFF_DIRTYHEADER	0x1	/* header must be written on close */
#define	TIFF_DIRTYDIRECT	0x2	/* current directory must be written */
#define	TIFF_BUFFERSETUP	0x4	/* data buffers setup */
#define	TIFF_BEENWRITING	0x8	/* written 1+ scanlines to file */
#define	TIFF_SWAB		0x10	/* byte swap file information */
#define	TIFF_NOBITREV		0x20	/* inhibit bit reversal logic */
#define	TIFF_MYBUFFER		0x40	/* my raw data buffer; free on close */
#define	TIFF_ISTILED		0x80	/* file is tile, not strip- based */
#define	TIFF_MAPPED		0x100	/* file is mapped into memory */
#define	TIFF_POSTENCODE		0x200	/* need call to postencode routine */
	toff_t		tif_diroff;	/* file offset of current directory */
	toff_t		tif_nextdiroff;	/* file offset of following directory */
	TIFFDirectory	tif_dir;	/* internal rep of current directory */
	TIFFHeader	tif_header;	/* file's header block */
	const long*	tif_typeshift;	/* data type shift counts */
	const long*	tif_typemask;	/* data type masks */
	//AAB orig was unsigned; code uses -1 vals though DWORD	tif_row;	/* current scanline */
	long 	tif_row;	/* current scanline */
	tdir_t		tif_curdir;	/* current directory (index) */
	tstrip_t	tif_curstrip;	/* current strip for read/write */
	toff_t		tif_curoff;	/* current offset for read/write */
	toff_t		tif_dataoff;	/* current offset for writing dir */
/* tiling support */
	//AAB see tif_row comment DWORD	tif_col;	/* current column (offset by row too) */
	long	tif_col;	/* current column (offset by row too) */
	ttile_t		tif_curtile;	/* current tile for read/write */
	tsize_t		tif_tilesize;	/* # of bytes in a tile */
/* compression scheme hooks */
	TIFFBoolMethod	tif_predecode;	/* pre row/strip/tile decoding */
	TIFFBoolMethod	tif_preencode;	/* pre row/strip/tile encoding */
	TIFFBoolMethod	tif_postencode;	/* post row/strip/tile encoding */
	TIFFCodeMethod	tif_decoderow;	/* scanline decoding routine */
	TIFFCodeMethod	tif_encoderow;	/* scanline encoding routine */
	TIFFCodeMethod	tif_decodestrip;/* strip decoding routine */
	TIFFCodeMethod	tif_encodestrip;/* strip encoding routine */
	TIFFCodeMethod	tif_decodetile;	/* tile decoding routine */
	TIFFCodeMethod	tif_encodetile;	/* tile encoding routine */
	TIFFVoidMethod	tif_close;	/* cleanup-on-close routine */
	TIFFSeekMethod	tif_seek;	/* position within a strip routine */
	TIFFVoidMethod	tif_cleanup;	/* routine called to cleanup state */
	tidata_t	tif_data;	/* compression scheme private data */
/* input/output buffering */
	tsize_t		tif_scanlinesize;/* # of bytes in a scanline */
	tsize_t		tif_scanlineskew;/* scanline skew for reading strips */
	tidata_t	tif_rawdata;	/* raw data buffer */
	tsize_t		tif_rawdatasize;/* # of bytes in raw data buffer */
	tidata_t	tif_rawcp;	/* current spot in raw buffer */
	tsize_t		tif_rawcc;	/* bytes unread from raw buffer */
/* memory-mapped file support */
	tidata_t	tif_base;	/* base of mapped file */
	toff_t		tif_size;	/* size of mapped file region (bytes) */
	TIFFMapFileProc	tif_mapproc;	/* map file method */
	TIFFUnmapFileProc tif_unmapproc;/* unmap file method */
/* input/output callback methods */
	thandle_t	tif_clientdata;	/* callback parameter */
	TIFFReadWriteProc tif_readproc;	/* read method */
	TIFFReadWriteProc tif_writeproc;/* write method */
	TIFFSeekProc	tif_seekproc;	/* lseek method */
	TIFFCloseProc	tif_closeproc;	/* close method */
	TIFFSizeProc	tif_sizeproc;	/* filesize method */
/* post-decoding support */
	TIFFPostMethod	tif_postdecode;/* post decoding routine */
};

#define	isTiled(tif)	(((tif)->tif_flags & TIFF_ISTILED) != 0)
#define	isMapped(tif)	(((tif)->tif_flags & TIFF_MAPPED) != 0)
#define	TIFFReadFile(tif, buf, size) \
	((*(tif)->tif_readproc)((tif)->tif_clientdata,buf,size))
#define	TIFFWriteFile(tif, buf, size) \
	((*(tif)->tif_writeproc)((tif)->tif_clientdata,buf,size))
#define	TIFFSeekFile(tif, off, whence) \
	((*(tif)->tif_seekproc)((tif)->tif_clientdata,off,whence))
#define	TIFFCloseFile(tif) \
	((*(tif)->tif_closeproc)((tif)->tif_clientdata))
#define	TIFFGetFileSize(tif) \
	((*(tif)->tif_sizeproc)((tif)->tif_clientdata))
#define	TIFFMapFileContents(tif, paddr, psize) \
	((*(tif)->tif_mapproc)((tif)->tif_clientdata,paddr,psize))
#define	TIFFUnmapFileContents(tif, addr, size) \
	((*(tif)->tif_unmapproc)((tif)->tif_clientdata,addr,size))

/*
 * Default Read/Seek/Write definitions.
 */
#ifndef ReadOK
#define	ReadOK(tif, buf, size) \
	(TIFFReadFile(tif, (tdata_t) buf, (tsize_t) size) == size)
#endif
#ifndef SeekOK
#define	SeekOK(tif, off) \
	(TIFFSeekFile(tif, (toff_t) off, L_SET) == (toff_t) off)
#endif
#ifndef WriteOK
#define	WriteOK(tif, buf, size) \
	(TIFFWriteFile(tif, (tdata_t) buf, (tsize_t) size) == (tsize_t) size)
#endif

/* generic option bit names */
#define	TIFF_OPT0	0x1
#define	TIFF_OPT1	0x2
#define	TIFF_OPT2	0x4
#define	TIFF_OPT3	0x8
#define	TIFF_OPT4	0x10
#define	TIFF_OPT5	0x20
#define	TIFF_OPT6	0x40
#define	TIFF_OPT7	0x80

/* NB: the u_long casts are to silence certain ANSI-C compilers */
#ifdef howmany
#undef howmany
#endif
#define	howmany(x, y)	((((DWORD)(x))+(((DWORD)(y))-1))/((DWORD)(y)))
#ifdef roundup
#undef roundup
#endif
#define	roundup(x, y)	(howmany(x,y)*((DWORD)(y)))

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__cplusplus)
}
#endif

//AAB additions needed for "class-ification"
typedef struct {
	TCHAR* name;
	long scheme;
	TIFFBoolMethod init;
} cscheme_t;

//AAB from tif_lzw.cpp
#define LZW_CHECKEOS
#define MAXCODE(n) ((1<<(n))-1)
#define BITS_MIN 9
#define BITS_MAX 12
#define CODE_CLEAR 256
#define CODE_EOI 257
#define CODE_FIRST 258
#define CODE_MAX MAXCODE(BITS_MAX)

#define HSIZE 10
#define HSHIFT (13-8)
#ifdef LZW_COMPAT
#define CSIZE	(MAXCODE(BITS_MAX)+1024)
#else
#define CSIZE	(MAXCODE(BITS_MAX)+1)
#endif
typedef void (*predictorFunc)(LPBYTE data, DWORD nbytes, long stride);
typedef struct {
	predictorFunc hordiff;
	DWORD rowsize;
	long stride;
	long nbits;
	long maxcode;
	long free_ent;
	long nextdata;
	long nextbits;
} LZWState;
typedef struct code_ent {
	struct code_ent *next;
	WORD length;
	BYTE value;
	BYTE firstchar;
} code_t;
typedef long (SECTiff::*decodeFunc)(TIFF*, tidata_t, tsize_t, tsample_t);
typedef struct {
	LZWState base;
	long dec_nbitsmask;
	long dec_restart;
#ifdef LZW_CHECKEOS
	long dec_bitsleft;
#endif
	decodeFunc dec_decode;
	code_t *dec_codep;
	code_t *dec_oldcodep;
	code_t *dec_free_entp;
	code_t *dec_maxcodep;
	code_t dec_codetab[CSIZE];
} LZWDecodeState;

typedef struct {
	long hash;
	long code;
} hash_t;
typedef struct {
	LZWState base;
	long enc_oldcode;
	long enc_checkpoint;
#define CHECK_GAP 10000
	long enc_ratio;
	long enc_incount;
	long enc_outcount;
	tidata_t enc_rawlimit;
	hash_t enc_hashtab[HSIZE];
}LZWEncodeState;
#endif /* _TIFFIOP_ */
