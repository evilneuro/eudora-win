/* $Header: /usr/people/sam/tiff/libtiff/RCS/tiffio.h,v 1.73 93/08/26 14:59:03 sam Exp $ */

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

#ifndef _TIFFIO_
#define	_TIFFIO_

/*
 * TIFF I/O Library Definitions.
 */
#include <windows.h>
#include "tiff.h"

/*
 * TIFF is defined as an incomplete type to hide the
 * library's internal data structures from clients.
 */
typedef	struct tiff TIFF;

/*
 * The following typedefs define the intrinsic size of
 * data types used in the *exported* interfaces.  These
 * definitions depend on the proper definition of types
 * in tiff.h.  Note also that the varargs interface used
 * pass tag types and values uses the types defined in
 * tiff.h directly.
 *
 * NB: ttag_t is unsigned int and not unsigned short because
 *     ANSI C requires that the type before the ellipsis be a
 *     promoted type (i.e. one of int, unsigned int, pointer,
 *     or double).
 * NB: tsize_t is int32 and not uint32 because some functions
 *     return -1.
 * NB: toff_t is not off_t for many reasons; TIFFs max out at
 *     32-bit file offsets being the most important
 */
typedef	unsigned int ttag_t;	/* directory tag */
typedef	short tdir_t;		/* directory index */
typedef	WORD tsample_t;	/* sample number */
typedef	long tstrip_t;	/* strip number */
typedef DWORD ttile_t;		/* tile number */
typedef	long	tsize_t;		/* i/o size in bytes */
#ifdef WIN32
typedef	void* tdata_t;		/* image data ref */
#else
// typedef	void FAR* tdata_t;		/* image data ref */ 
typedef void huge * tdata_t;  // AAB 12/10
// typedef	void FAR* thandle_t;	/* client data handle */
#endif
typedef HFILE thandle_t;
typedef	long toff_t;		/* file offset */

#ifndef NULL
#define	NULL	0
#endif

/*
 * Flags to pass to TIFFPrintDirectory to control
 * printing of data structures that are potentially
 * very large.   Bit-or these flags to enable printing
 * multiple items.
 */
#define	TIFFPRINT_NONE		0x0		/* no extra info */
#define	TIFFPRINT_STRIPS	0x1		/* strips/tiles info */
#define	TIFFPRINT_CURVES	0x2		/* color/gray response curves */
#define	TIFFPRINT_COLORMAP	0x4		/* colormap */
#define	TIFFPRINT_JPEGQTABLES	0x100		/* JPEG Q matrices */
#define	TIFFPRINT_JPEGACTABLES	0x200		/* JPEG AC tables */
#define	TIFFPRINT_JPEGDCTABLES	0x200		/* JPEG DC tables */

/*
 * Macros for extracting components from the
 * packed ABGR form returned by TIFFReadRGBAImage.
 */
#define	TIFFGetR(abgr)	((abgr) & 0xff)
#define	TIFFGetG(abgr)	(((abgr) >> 8) & 0xff)
#define	TIFFGetB(abgr)	(((abgr) >> 16) & 0xff)
#define	TIFFGetA(abgr)	(((abgr) >> 24) & 0xff)

#include <stdio.h>
#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif
typedef	void (*TIFFErrorHandler)(const TCHAR* module, const _TCHAR* fmt, va_list);
typedef	tsize_t (*TIFFReadWriteProc)(thandle_t, tdata_t, tsize_t);
typedef	toff_t (*TIFFSeekProc)(thandle_t, toff_t, short);
//AAB typedef	long (*TIFFCloseProc)(thandle_t);
typedef	thandle_t (*TIFFCloseProc)(thandle_t);
typedef	toff_t (*TIFFSizeProc)(thandle_t);
typedef	long (*TIFFMapFileProc)(thandle_t, tdata_t*, toff_t*);
typedef	void (*TIFFUnmapFileProc)(thandle_t, tdata_t, toff_t);

extern	const char* TIFFGetVersion(void);

extern	void TIFFClose(TIFF*);
extern	long TIFFFlush(TIFF*);
extern	long TIFFFlushData(TIFF*);
extern	long TIFFGetField(TIFF*, ttag_t, ...);
extern	long TIFFVGetField(TIFF*, ttag_t, va_list);
extern	long TIFFGetFieldDefaulted(TIFF*, ttag_t, ...);
extern	long TIFFVGetFieldDefaulted(TIFF*, ttag_t, va_list);
extern	long TIFFReadDirectory(TIFF*);
extern	tsize_t TIFFScanlineSize(TIFF*);
extern	tsize_t TIFFStripSize(TIFF*);
extern	tsize_t TIFFVStripSize(TIFF*, DWORD);
extern	tsize_t TIFFTileRowSize(TIFF*);
extern	tsize_t TIFFTileSize(TIFF*);
extern	tsize_t TIFFVTileSize(TIFF*, DWORD);
extern	long TIFFFileno(TIFF*);
extern	UINT TIFFGetMode(TIFF*);
//AAB extern	long TIFFGetMode(TIFF*);
extern	long TIFFIsTiled(TIFF*);
extern	long TIFFIsByteSwapped(TIFF*);
extern	DWORD TIFFCurrentRow(TIFF*);
extern	tdir_t TIFFCurrentDirectory(TIFF*);
extern	tstrip_t TIFFCurrentStrip(TIFF*);
extern	ttile_t TIFFCurrentTile(TIFF*);
extern	long TIFFReadBufferSetup(TIFF*, tdata_t, tsize_t);
extern	long TIFFLastDirectory(TIFF*);
extern	long TIFFSetDirectory(TIFF*, tdir_t);
extern	long TIFFSetField(TIFF*, ttag_t, ...);
extern	long TIFFVSetField(TIFF*, ttag_t, va_list);
extern	long TIFFWriteDirectory(TIFF *);
#if defined(c_plusplus) || defined(__cplusplus)
extern	void TIFFPrintDirectory(TIFF*, FILE*, long = 0);
extern	long TIFFReadScanline(TIFF*, tdata_t, DWORD, tsample_t = 0);
extern	long TIFFWriteScanline(TIFF*, tdata_t, DWORD, tsample_t = 0);
//AAB extern	long TIFFReadRGBAImage(TIFF*,
//AAB	    unsigned long, unsigned long, unsigned long*, long stop = 0);
extern	long TIFFReadRGBAImage(TIFF*,
	    DWORD, DWORD, LPDWORD, long stop = 0);
#else
extern	void TIFFPrintDirectory(TIFF*, FILE*, long);
extern	long TIFFReadScanline(TIFF*, tdata_t, DWORD, tsample_t);
extern	long TIFFWriteScanline(TIFF*, tdata_t, DWORD, tsample_t);
extern	long TIFFReadRGBAImage(TIFF*,
	    DWORD, DWORD, LPDWORD, long stop);
#endif
extern	TIFF* TIFFOpen(const TCHAR*, const TCHAR*);
extern	TIFF* TIFFFdOpen(thandle_t, const char*, const char*);
extern	TIFF* TIFFClientOpen(const TCHAR* name, const TCHAR* mode,
	    thandle_t clientdata,
	    TIFFReadWriteProc readproc, TIFFReadWriteProc writeproc,
	    TIFFSeekProc seekproc, TIFFCloseProc closeproc,
	    TIFFSizeProc sizeproc,
	    TIFFMapFileProc mapproc, TIFFUnmapFileProc unmapproc);
extern	const TCHAR* TIFFFileName(TIFF*);
extern	void TIFFError(const _TCHAR*, const TCHAR*, ...);
extern	void TIFFWarning(const TCHAR*, const TCHAR*, ...);
extern	TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler handler);
extern	TIFFErrorHandler TIFFSetWarningHandler(TIFFErrorHandler handler);
extern	ttile_t TIFFComputeTile(TIFF*, DWORD, DWORD, DWORD, tsample_t);
extern	long TIFFCheckTile(TIFF*, DWORD, DWORD, DWORD, tsample_t);
extern	ttile_t TIFFNumberOfTiles(TIFF*);
extern	tsize_t TIFFReadTile(TIFF*,
	    tdata_t, DWORD, DWORD, DWORD, tsample_t);
extern	tsize_t TIFFWriteTile(TIFF*,
	    tdata_t, DWORD, DWORD, DWORD, tsample_t);
extern	tstrip_t TIFFComputeStrip(TIFF*, DWORD, tsample_t);
extern	tstrip_t TIFFNumberOfStrips(TIFF*);
extern	tsize_t TIFFReadEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	void TIFFSetWriteOffset(TIFF*, toff_t);
extern	void TIFFSwabShort(LPWORD);
extern	void TIFFSwabLong(LPDWORD);
extern	void TIFFSwabArrayOfShort(LPWORD, unsigned long);
extern	void TIFFSwabArrayOfLong(LPDWORD, unsigned long);
//AAB extern	void TIFFReverseBits(unsigned char *, unsigned long);
extern	void TIFFReverseBits(LPBYTE, DWORD);
extern	const unsigned char* TIFFGetBitRevTable(long);

extern	void TIFFModeCCITTFax3(TIFF* tif, long isClassF);	/* XXX */
#if defined(__cplusplus)
}
#endif
#endif /* _TIFFIO_ */
