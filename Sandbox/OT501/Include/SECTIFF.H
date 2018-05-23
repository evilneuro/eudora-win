//
// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC TIFF file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	6/22/95	Started
//    MSW       9/30/95 Code Review
//
//

#ifndef __SECTIFF_H__
#define __SECTIFF_H__

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

#ifndef WIN32
#pragma warning(disable : 4103)
#endif 

#include "tiffio.h"
#include "tiffiop.h"
#include "tiff.h"
#include "t4.h"
       
#ifdef WIN32
typedef LPBYTE* LPPBYTE;
typedef LPBYTE LPBITS;
#else
typedef LPBYTE FAR* LPPBYTE;
typedef BYTE huge * LPBITS;
#endif


typedef void (SECTiff::*tileSeparateRoutine)
	(LPBYTE, LPBYTE, LPBYTE, LPBYTE, LPBYTE, DWORD, DWORD, int, int);

typedef void (SECTiff::*tileContigRoutine)
    (LPBITS, LPBYTE, LPBYTE, DWORD, DWORD, int, int);

typedef void (SECTiff::*geti_tileSeparateRoutine)
	(LPDWORD, LPBYTE, LPBYTE, LPBYTE, LPRGBvalue, DWORD, DWORD, long, long);

typedef void (SECTiff::*geti_tileContigRoutine)
    (LPDWORD, LPBYTE, LPRGBvalue, DWORD, DWORD, long, long);
	 
class SECTiff: public SECImage
{
    DECLARE_SERIAL(SECTiff);

// Construction
    SECTiff();

// Operations
public:
    virtual BOOL LoadImage (LPCTSTR lpszFileName); /* Load native format & convert to DIB */
    virtual BOOL SaveImage (LPCTSTR lpszFileName); /* Save as native format */

// Overrides
protected:
    virtual BOOL DoLoadImage(CFile* pFile);
    virtual BOOL DoSaveImage(CFile* pFile);

// Implementation
public:
    int    m_nCompression;

protected:
    TIFF * m_lpTif;
    WORD	 m_wOrientation;
	WORD	m_wExtraSamples;
	WORD	m_wPlanarConfig;
    WORD   m_wBPS; // bits per sample
    WORD   m_wSPP;	// samples per pixel
    WORD   m_wPhoto;
    WORD   m_wBPP;
    LPBYTE m_lpPic;
    WORD   m_wColType;	// 8==8bit data, 24==true colors
    LPBYTE *m_lpBWmap;
	LPDWORD	*m_lpgetiBWmap;
    LPBYTE *m_lpPALmap;
	LPDWORD *m_lpgetiPALmap;
    LPBYTE m_lpMap;
    int	 m_nStopOnErr;
    WORD   m_wYCbCrHorizSampling;
    WORD   m_wYCbCrVertSampling;
    float *m_lpYCbCrCoeffs;
    float *m_lpRefBlackWhite;
	LPWORD	redcmap;
	LPWORD	greencmap;
	LPWORD 	bluecmap;
	float	m_D1, m_D2, m_D3, m_D4;	//used in packed RGB routines in tif_get.cpp

	//in use for 
	WORD wYCbCrVertSampling;
	WORD wYCbCrHorizSampling;
	float *lpRefBlackWhite;
	void pack1(int *pnByte, DWORD *pdwWdith, DWORD *pdwTotalBytes, LPBITS *cp, LPBYTE *bw, DWORD wMax, DWORD dwBytesInLine);
	LPPBYTE lppBWmap;
	LPPBYTE lppPALmap;
	tileSeparateRoutine SECpickTileSeparateCase(LPBYTE);
	geti_tileSeparateRoutine geti_pickTileSeparateCase(TIFF*, LPRGBvalue);
	tileSeparateRoutine m_fpTileSeparate;
	geti_tileSeparateRoutine m_fpgetiTileSeparate;

	tileContigRoutine SECpickTileContigCase(LPBYTE);
	geti_tileContigRoutine geti_pickTileContigCase(TIFF*, LPRGBvalue);
	tileContigRoutine m_fpContigRoutine;
	geti_tileContigRoutine m_fpgetiContigRoutine;
	void geti_initYCbCrConversion(void);

	void SECput8bitcmaptile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECput4bitcmaptile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECput2bitcmaptile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECput1bitcmaptile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECputgreytile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECput1bitbwtile(LPBITS cp, LPBYTE pp, LPBYTE pMap, DWORD w, DWORD h, int fromskew, int toskew);
	void SECput2bitbwtile(LPBITS cp, LPBYTE pp, LPBYTE pMap, DWORD w, DWORD h, int fromskew, int toskew);
	void SECput4bitbwtile(LPBITS cp, LPBYTE pp, LPBYTE pMap, DWORD w, DWORD h, int fromskew, int toskew);
	void SECputRGBcontig8bittile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECputRGBcontig16bittile(LPBITS cp, LPBYTE tpp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECputRGBseparate8bittile(LPBYTE cp, LPBYTE r, LPBYTE g, LPBYTE b, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECputRGBseparate16bittile(LPBYTE cp, LPBYTE pr, LPBYTE pg, LPBYTE pb, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);
	void SECputRGBContigYCbCrClump(LPBYTE cp, LPBYTE pp, WORD cw, WORD ch, DWORD w, int n, int fromskew, int toskew);
	void SECputcontig8bitYCbCrtile(LPBITS cp, LPBYTE pp, LPBYTE Map, DWORD w, DWORD h, int fromskew, int toskew);

	void geti_put8bitcmaptile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_put4bitcmaptile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_put2bitcmaptile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_put1bitcmaptile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putgreytile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_put1bitbwtile(LPDWORD cp, LPBYTE pp, LPRGBvalue pMap, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_put2bitbwtile(LPDWORD cp, LPBYTE pp, LPRGBvalue pMap, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_put4bitbwtile(LPDWORD cp, LPBYTE pp, LPRGBvalue pMap, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putRGBcontig8bittile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putRGBcontig16bittile(LPDWORD cp, LPBYTE tpp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putRGBseparate8bittile(LPDWORD cp, LPBYTE r, LPBYTE g, LPBYTE b, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putRGBseparate16bittile(LPDWORD cp, LPBYTE pr, LPBYTE pg, LPBYTE pb, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putRGBContigYCbCrClump(LPDWORD cp, LPBYTE pp, DWORD cw, DWORD ch, DWORD w, int n, long fromskew, long toskew);
	void geti_putcontig8bitYCbCrtile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	void geti_putRGBcontig8bitCMYKtile(LPDWORD cp, LPBYTE pp, LPRGBvalue Map, DWORD w, DWORD h, long fromskew, long toskew);
	
    BOOL loadPalette();
    BOOL loadColors();
    WORD loadImageData();
    void ConvertRGB();
    WORD checkcmap(int, LPWORD, LPWORD, LPWORD);
    WORD gt();
    int gtTileContig();
    int gtTileSeparate();
    int gtStripContig();
    int gtStripSeparate();
    int makebwmap();
    int makecmap();
    void setupColormap(TIFF *, LPBYTE, LPBYTE, LPBYTE);
    int WriteTIFF(FILE *, LPBYTE, int, LPBYTE, LPBYTE, LPBYTE, int, int, const TCHAR *, TCHAR *);
    void SECinitYCbCrConversion();
    DWORD setorientation(DWORD);
    virtual void Serialize(CArchive& ar);
public:   
    LPVOID _TIFFmalloc(size_t);
    LPVOID _TIFFrealloc(LPVOID, size_t);
    void _TIFFfree(LPVOID);
    UINT _TIFFgetMode(const TCHAR* mode, const TCHAR* module);
    const TIFFFieldInfo *TIFFFindFieldInfo(ttag_t, TIFFDataType);
    const TIFFFieldInfo *TIFFFieldWithTag(ttag_t);
    void _TIFFgetfield(TIFFDirectory*, ttag_t, ...);
    long TIFFNoRowEncode(TIFF*, tidata_t, tsize_t, tsample_t);
    long TIFFNoStripEncode(TIFF*, tidata_t, tsize_t, tsample_t);
    long TIFFNoTileEncode(TIFF*, tidata_t, tsize_t, tsample_t);
    long TIFFNoRowDecode(TIFF*, tidata_t, tsize_t, tsample_t);
    long TIFFNoStripDecode(TIFF*, tidata_t, tsize_t, tsample_t);
    long TIFFNoTileDecode(TIFF*, tidata_t, tsize_t, tsample_t);
    void TIFFNoPostDecode(TIFF*, tidata_t, tsize_t);
    void TIFFSwab16BitData(TIFF*, tidata_t, tsize_t);
    void TIFFSwab32BitData(TIFF*, tidata_t, tsize_t);
    long TIFFFlushData1(TIFF*);
    void TIFFFreeDirectory(TIFF*);
    long TIFFDefaultDirectory(TIFF*);
    long TIFFSetCompressionScheme(TIFF *, long);
    void TIFFClose(TIFF*);
    long TIFFFlush(TIFF*);
    long TIFFFlushData(TIFF*);
    long TIFFGetField(TIFF*, ttag_t, ...);
    long TIFFVGetField(TIFF*, ttag_t, va_list);
    long TIFFGetFieldDefaulted(TIFF*, ttag_t, ...);
    long TIFFVGetFieldDefaulted(TIFF*, ttag_t, va_list);
    long TIFFReadDirectory(TIFF*);
    tsize_t TIFFScanlineSize(TIFF*);
    tsize_t TIFFStripSize(TIFF*);
    tsize_t TIFFVStripSize(TIFF*, DWORD);
    tsize_t TIFFTileRowSize(TIFF*);
    tsize_t TIFFTileSize(TIFF*);
    tsize_t TIFFVTileSize(TIFF*, DWORD);
    long TIFFFileno(TIFF*);
    UINT TIFFGetMode(TIFF*);
    long TIFFIsTiled(TIFF*);
    long TIFFIsByteSwapped(TIFF*);
    DWORD TIFFCurrentRow(TIFF*);
    tdir_t TIFFCurrentDirectory(TIFF*);
    tstrip_t TIFFCurrentStrip(TIFF*);
    ttile_t TIFFCurrentTile(TIFF*);
    long TIFFReadBufferSetup(TIFF*, tdata_t, tsize_t);
    long TIFFLastDirectory(TIFF*);
    long TIFFSetDirectory(TIFF*, tdir_t);
    long TIFFSetField(TIFF*, ttag_t, ...);
    long TIFFVSetField(TIFF*, ttag_t, va_list);
    long TIFFWriteDirectory(TIFF *);
    void TIFFPrintDirectory(TIFF*, FILE*, long = 0);
    long TIFFReadScanline(TIFF*, tdata_t, DWORD, tsample_t = 0);
    long TIFFWriteScanline(TIFF*, tdata_t, DWORD, tsample_t = 0);
    long TIFFReadRGBAImage(TIFF*,
			   DWORD, DWORD, LPDWORD, long stop = 0);
    TIFF* TIFFOpen(const TCHAR*, const TCHAR*);
    TIFF* TIFFFdOpen(thandle_t, const TCHAR*, const TCHAR*);
    TIFF* TIFFClientOpen(const TCHAR* name, const TCHAR* mode,
			 thandle_t clientdata,
			 TIFFReadWriteProc readproc, TIFFReadWriteProc writeproc,
			 TIFFSeekProc seekproc, TIFFCloseProc closeproc,
			 TIFFSizeProc sizeproc,
			 TIFFMapFileProc mapproc, TIFFUnmapFileProc unmapproc);
    const TCHAR* TIFFFileName(TIFF*);
    void TIFFError(const _TCHAR*, const TCHAR*, ...);
    void TIFFWarning(const TCHAR*, const TCHAR*, ...);
    TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler handler);
    TIFFErrorHandler TIFFSetWarningHandler(TIFFErrorHandler handler);
    ttile_t TIFFComputeTile(TIFF*, DWORD, DWORD, DWORD, tsample_t);
    long TIFFCheckTile(TIFF*, DWORD, DWORD, DWORD, tsample_t);
    ttile_t TIFFNumberOfTiles(TIFF*);
    tsize_t TIFFReadTile(TIFF*,
			 tdata_t, DWORD, DWORD, DWORD, tsample_t);
    tsize_t TIFFWriteTile(TIFF*,
			  tdata_t, DWORD, DWORD, DWORD, tsample_t);
    tstrip_t TIFFComputeStrip(TIFF*, DWORD, tsample_t);
    tstrip_t TIFFNumberOfStrips(TIFF*);
    tsize_t TIFFReadEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
    tsize_t TIFFReadRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
    tsize_t TIFFReadEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
    tsize_t TIFFReadRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
    tsize_t TIFFWriteEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
    tsize_t TIFFWriteRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
    tsize_t TIFFWriteEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
    tsize_t TIFFWriteRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
    void TIFFSetWriteOffset(TIFF*, toff_t);
    void TIFFSwabShort(LPWORD);
    void TIFFSwabLong(LPDWORD);
    void TIFFSwabArrayOfShort(LPWORD, unsigned long);
    void TIFFSwabArrayOfLong(LPDWORD, unsigned long);
    void TIFFReverseBits(LPBYTE, DWORD);
    const unsigned char* TIFFGetBitRevTable(long);
    
//AABTEST #ifdef WIN32
    void TIFFModeCCITTFax3(TIFF* tif, long isClassF);	
    long Fax3Decode2DRow(TIFF*, BYTE*, DWORD);
    long Fax3Encode2DRow(TIFF*, BYTE*, BYTE*, DWORD);
    long Fax3Encode1DRow(TIFF*, BYTE*, long);
    long Fax3Decode1DRow(TIFF*, BYTE*, long);
    void Fax3PutBits(TIFF*, DWORD, long);
    void Fax3PutEOL(TIFF*);
    void *Fax3SetupState(TIFF*, long);
    long Fax3PreDecode(TIFF*);
    long Fax3PreEncode(TIFF*);
    long Fax3Decode(TIFF*, tidata_t, tsize_t, tsample_t);
    void fax3_putcode(TIFF*, const tableentry *);
    void fax3_putspan(TIFF*, long, const tableentry *);
    long Fax3Encode(TIFF*, tidata_t, tsize_t, tsample_t);
    long Fax3PostEncode(TIFF*);
    void Fax3Close(TIFF*);
    void Fax3Cleanup(TIFF*);
    long Fax4Decode(TIFF*, tidata_t, tsize_t, tsample_t);
    long Fax4Encode(TIFF*, tidata_t, tsize_t, tsample_t);
    long Fax4PostEncode(TIFF*);
//AABTEST #endif
    
    long TIFFInitDumpMode(TIFF*);
#ifdef PACKBITS_SUPPORT
    long TIFFInitPackBits(TIFF*);
#endif //PACKBITS_SUPPORT
//AABTEST #ifdef CCITT_SUPPORT
    long TIFFInitCCITTRLE(TIFF*);
long TIFFInitCCITTRLEW(TIFF*);
    long TIFFInitCCITTFax3(TIFF*);
    long TIFFInitCCITTFax4(TIFF*);
//AABTEST #endif //CCITT_SUPPORT   
#ifdef THUNDER_SUPPORT
    long TIFFInitThunderScan(TIFF*);
#endif //THUNDER_SUPPORT
#ifdef NEXT_SUPPORT
    long TIFFInitNeXT(TIFF*);
#endif //NEXT_SUPPORT
#ifdef LZW_SUPPORT
    long TIFFInitLZW(TIFF*);
#endif //LZW_SUPPORT
#ifdef JPEG_SUPPORT
    long TIFFInitJPEG(TIFF*);
#endif //JPEG_SUPPORT
    
//AAB holders for typedef pointer fxns
    TIFFVoidMethod m_fpVoid;
    TIFFBoolMethod m_fpBool;
    TIFFCodeMethod m_fpCode;
    TIFFSeekMethod m_fpSeek;
    TIFFPostMethod m_fpPost;
    decodeFunc m_fpDecode;
    
// void (SECTiff::*m_fpVoid)(TIFF *);
    
//AAB converted these from statics since they call member fxns...
//from tif_aux.cpp
    void TIFFDefaultTransferFunction(TIFFDirectory *);
    void TIFFDefaultRefBlackWhite(TIFFDirectory *);
//from tif_comp.cpp
    const cscheme_t *compfindScheme(long);
    long TIFFNoDecode(TIFF*, TCHAR*);
    long TIFFNoEncode(TIFF*, TCHAR*);
//from tif_dir.cpp
    void setShortArray(LPPWORD, LPWORD, long);
    void setLongArray(LPPDWORD, LPDWORD, long);
    void setFloatArray(LPPFLOAT, LPFLOAT, long);
#ifdef JPEG_SUPPORT
    void setJPEGQTable(LPPPBYTE, LPPBYTE, long);
    void setJPEGCTable(LPPPBYTE, LPPBYTE, long);
#endif
    long setExtraSamples(TIFFDirectory *, va_list, LPWORD);
    long TIFFSetField1(TIFF*, ttag_t, va_list);
    long OkToChangeTag(TIFF*, ttag_t);
    void TIFFGetField1(TIFFDirectory *, ttag_t, va_list);
//from tif_dirr.cpp
    LPBYTE CheckMalloc(TIFF*, size_t, const char *);
    void EstimateStripByteCounts(TIFF*, TIFFDirEntry FAR*, WORD);
    void MissingRequired(TIFF*, const char *);
    long CheckDirCount(TIFF*, TIFFDirEntry FAR*, DWORD);
    tsize_t TIFFFetchData(TIFF*, TIFFDirEntry FAR*, LPBYTE);
    tsize_t TIFFFetchString(TIFF*, TIFFDirEntry FAR*, LPBYTE);
    float TIFFFetchRational(TIFF*, TIFFDirEntry FAR*);
    long TIFFFetchNormalTag(TIFF*, TIFFDirEntry FAR*);
    long TIFFFetchPerSampleShorts(TIFF*, TIFFDirEntry FAR*, long FAR*);
    long TIFFFetchShortArray(TIFF*, TIFFDirEntry FAR*, LPWORD);
    long TIFFFetchShortPair(TIFF*, TIFFDirEntry FAR*);
    long TIFFFetchStripThing(TIFF*, TIFFDirEntry FAR*, long, LPPDWORD);
    long TIFFFetchExtraSamples(TIFF*, TIFFDirEntry FAR*);
    long TIFFFetchRefBlackWhite(TIFF*, TIFFDirEntry FAR*);
#ifdef JPEG_SUPPORT
    long TIFFFetchJPEGQTables(TIFF*, TIFFDirEntry FAR*);
    long TIFFFetchJPEGCTables(TIFF*, TIFFDirEntry FAR*, LPPPBYTE);
#endif
    long TIFFFetchFloatArray(TIFF*, TIFFDirEntry FAR*, LPFLOAT);
    float TIFFFetchFloat(TIFF*, TIFFDirEntry FAR*);
    long TIFFFetchByteArray(TIFF*, TIFFDirEntry FAR*, LPWORD);
    long TIFFFetchLongArray(TIFF*, TIFFDirEntry FAR*, LPDWORD);
    long TIFFFetchRationalArray(TIFF*, TIFFDirEntry FAR*, LPFLOAT);
#ifdef STRIPCHOP_SUPPORT
    void ChopUpSingleUncompressedStrip(TIFF*);
#endif
    long cvtRational(TIFF*, TIFFDirEntry FAR*, DWORD, DWORD, LPFLOAT);
    
//from tif_dirw.cpp
    long TIFFWriteNormalTag(TIFF*, TIFFDirEntry FAR*, const TIFFFieldInfo *);
    void TIFFSetupShortLong(TIFF*, ttag_t, TIFFDirEntry FAR*, DWORD);
    long TIFFWriteRational(TIFF*, TIFFDataType, ttag_t, TIFFDirEntry FAR*, float);
    long TIFFWritePerSampleShorts(TIFF*, ttag_t, TIFFDirEntry FAR*);
    long TIFFSetupShortPair(TIFF*, ttag_t, TIFFDirEntry FAR*);
    long TIFFWriteShortTable(TIFF*, ttag_t, TIFFDirEntry FAR*, DWORD, LPPWORD);
    long TIFFWriteString(TIFF*, ttag_t, TIFFDirEntry FAR*, TCHAR*);
    long TIFFWriteShortArray(TIFF*, TIFFDataType, ttag_t, TIFFDirEntry FAR*, DWORD, LPWORD);
    long TIFFWriteLongArray(TIFF*, TIFFDataType, ttag_t, TIFFDirEntry FAR*, DWORD, LPDWORD);
    long TIFFWriteRationalArray(TIFF*, TIFFDataType, ttag_t, TIFFDirEntry FAR*, DWORD, LPFLOAT);
    long TIFFWriteFloatArray(TIFF*, TIFFDataType, ttag_t, TIFFDirEntry FAR*, DWORD, LPFLOAT);
#ifdef JPEG_SUPPORT
    long TIFFWriteJPEGQTables(TIFF*, TIFFDirEntry FAR*);
    long TIFFWriteJPEGCTables(TIFF*, ttag_t, TIFFDirEntry FAR*, LPPBYTE);
#endif
    long TIFFWriteTransferFunction(TIFF*, TIFFDirEntry FAR*);
    long TIFFWriteData(TIFF*, TIFFDirEntry FAR*, LPBYTE);
    long TIFFLinkDirectory(TIFF*);

//from tif_dump.cpp
    int DumpModeEncode(TIFF*, tidata_t, tsize_t, tsample_t);
    int DumpModeDecode(TIFF*, tidata_t, tsize_t, tsample_t);
    int DumpModeSeek(TIFF*, DWORD);
    
//from tif_geti.cpp
    int geti_gt(TIFF*,DWORD,DWORD,LPDWORD);
    int geti_gtTileContig(TIFF*, LPDWORD, LPRGBvalue, DWORD, DWORD);
    int geti_gtTileSeparate(TIFF*, LPDWORD, LPRGBvalue, DWORD, DWORD);
    int geti_gtStripContig(TIFF*, LPDWORD, LPRGBvalue, DWORD, DWORD);
    int geti_gtStripSeparate(TIFF*, LPDWORD, LPRGBvalue, DWORD, DWORD);
    int geti_makebwmap(TIFF*, LPRGBvalue);
    int geti_makecmap(TIFF*, LPWORD, LPWORD, LPWORD);
    WORD geti_checkcmap(int, LPWORD, LPWORD, LPWORD);
    DWORD geti_setorientation(TIFF*, DWORD);
    int geti_setupMap(TIFF*, WORD, WORD, LPPRGBvalue);

//from tif_lzw.cpp
    long LZWEncodePredRow(TIFF*, tidata_t, tsize_t, tsample_t);
    long LZWEncodePredTile(TIFF*, tidata_t, tsize_t, tsample_t);
    long LZWDecode(TIFF*, tidata_t, tsize_t, tsample_t);
#ifdef LZW_COMPAT
    long LZWDecodeCompat(TIFF*, tidata_t, tsize_t, tsample_t);
#endif
    long LZWDecodePredRow(TIFF*, tidata_t, tsize_t, tsample_t);
    long LZWDecodePredTile(TIFF*, tidata_t, tsize_t, tsample_t);
    int LZWCheckPredictor(TIFF*, LZWState FAR*, predictorFunc, predictorFunc);
    void codeLoop(TIFF*);
    void LZWCleanup(TIFF*);
    int LZWPreDecode(TIFF*);
    long LZWDecodeCompat(TIFF*, tidata_t, tsize_t, tsample_t);
    int LZWPreEncode(TIFF*);
    int LZWEncode(TIFF*, tidata_t, tsize_t, tsample_t);
    int LZWPostEncode(TIFF*);
    
//include tif_pack.cpp
    int PackBitsPreEncode(TIFF *);
    int PackBitsEncode(TIFF*, tidata_t, tsize_t, tsample_t);
    int PackBitsEncodeChunk(TIFF*, tidata_t, tsize_t, tsample_t);
    int PackBitsDecode(TIFF*, tidata_t, tsize_t, tsample_t);
    
//tif_read.cpp
    int TIFFSeek(TIFF*, DWORD, tsample_t);
    void TIFFInitOrder(register TIFF*, long, long);
    int TIFFFillStrip(TIFF*, tstrip_t);
    int TIFFFillTile(TIFF*, ttile_t);
    int TIFFStartTile(TIFF*, ttile_t);
    int TIFFCheckRead(TIFF*, int);
    int TIFFStartStrip(TIFF*, tstrip_t);
    tsize_t TIFFReadRawStrip1(TIFF*, tstrip_t, tdata_t, tsize_t, const TCHAR*);
    tsize_t TIFFReadRawTile1(TIFF*, ttile_t, tdata_t, tsize_t, const TCHAR*);
    
//tif_vers.cpp
    const char* TIFFGetVersion(void);
    
//tif_writ.cpp
    long TIFFWriteCheck(TIFF*, long, const TCHAR*);
    long TIFFBufferSetup(TIFF*, const TCHAR*);
    long TIFFGrowStrips(TIFF*, long, const TCHAR*);
    long TIFFAppendToStrip(TIFF*, tstrip_t, tidata_t, tsize_t);
    long TIFFSetupStrips(TIFF*);
    
//tif_thun.cpp
    long ThunderDecode(TIFF*, tidata_t, tsize_t);
    long ThunderDecodeRow(TIFF*, tidata_t, tsize_t, tsample_t);

public:
    ~SECTiff();
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

#endif // __SECTIFF_H__
