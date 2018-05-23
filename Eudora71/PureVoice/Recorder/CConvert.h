/*////////////////////////////////////////////////////////////////////////////

NAME:
    CConvert - 

FILE:       CConvert.h
AUTHOR:     Livingstone Song (lss)
COMPANY:    QUALCOMM, Inc.
TYPE:       C++-CLASS

DESCRIPTION:
    CConvert creates a thread that does the encoding/decoding

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     -Initial
04/14/97   lss     -Added PacketIndex(). Brought on by the need for correct
                    timing info for fast/slow playback

/*////////////////////////////////////////////////////////////////////////////
#ifndef _CCONVERT_H_
#define _CCONVERT_H_

// INCLUDES
#include "CAWave.h"
#include "CACodec.h"
#include "CSndBuf.h"
#include "mmioFile.h"
#include "QPL.H"
#include "LsConvertUnit.h"


class CConvert
{
public:
//  LIFECYCLE
    CConvert( LPCTSTR sSrcFile, LPCTSTR sDestFile, CACodec *codec,
		BOOL *pisDone = NULL, BOOL bYield = FALSE );
    CConvert( CSndBuffer *sndBuf, LPCTSTR sDestFile, CACodec *codec, 
		BOOL *pisDone = NULL, BOOL bYield = FALSE );
    CConvert( LPCTSTR sSrcFile, CSndBuffer *sndBuf, CACodec *codec, 
		BOOL *pisDone = NULL, BOOL bYield = FALSE );
    CConvert( CSndBuffer *srcSndBuf, CSndBuffer *sndBuf, CACodec *codec,
		BOOL *pisDone = NULL, BOOL bYield = FALSE );
    CConvert( const CConvert& rhs );

    virtual ~CConvert();

//  OPERATIONS
    BOOL Encode(  int nHogCpu  = 1 );
    BOOL Decode( BOOL bRealtime = FALSE );
    void Stop();

//  ACCESS
    unsigned long setPositionTime( unsigned long p );
    long getLength( int type = UNIT_BYTES );
    long getIndex(){return ml_index;};
    long PacketIndex(){return ml_packetIndex;};

//////////////////////////////////////////////////////////////////////////////
private:
//  METHODS
    CConvert(); // don't allow default ctor

    void ctor_common( 	CSndBuffer *srcSndBuf,
		CSndBuffer *destsndBuf, 
		LPCTSTR sSrcFile, 
		LPCTSTR sDestFile, 
		CACodec *codec, 
		BOOL *pisDone, 
		BOOL bYield );

	BOOL commonEnDecode();
    void killConvertThread();

    UINT static ConvertThread(LPVOID pParam);

    int encode_file2file();
    void encode_file2file_variable( int size, BYTE *buffer, BYTE *data );
    void encode_file2file_fixed( int size, BYTE *buffer, BYTE *data );

    int encode_mem2file();
    
    int decode_file2mem();
    int decode_file2file();
    int decode_mem2mem();

    int open();
    int close();

    long read( BYTE *buffer, int size );
    long write( BYTE *buffer, int size );

    // for me_mode
    enum {
        FILE2MEM,
        FILE2FILE,
        MEM2FILE,
        MEM2MEM,

        CODEC = 0,      // default mode
        FILTER = 0x1000
    };

    // for me_useType
    enum {
        ENCODE = 1,
        DECODE
    };
    
//  ATTRIBUTES
    CQVoiceApp  *mp_App;    // pointer to our app class so we can call YieldEvents
public:
    CmmioFile   *mp_mmioInFile;
private:
    CmmioFile   *mp_mmioOutFile;

    CString     ms_srcFile;
    CString     ms_destFile;
	CSndBuffer  *mp_srcSndBuf;
	CSndBuffer  *mp_destSndBuf;
	CACodec     *mp_codec;

    BOOL        mb_DoYield;

    CWinThread *mp_ConvertThread;   // pointer to our ConvertThread

    // these 3 have units in bytes
    long        ml_length;  // in bytes of source
    long        ml_index;   // current processing position (source)
    long        ml_offset;  // start reading from ml_offset
    
    volatile long ml_packetIndex;   // current processing position in packets

	CCriticalSection m_cs;

	CEvent*			mp_eventThreadStop;
	CEvent*			mp_eventThreadResume;

    volatile BOOL   *mp_isDone;
    volatile BOOL   mb_Cancel;
    volatile BOOL   mb_ThreadLive;

    BOOL            mb_realtime;
    
    t_qpl_codec_info   m_codecInfo;

    LsConvertUnit   m_ConvertUnits;

    int         me_mode;    // see above enum
    int         me_useType; // ENCODER/DECODER

    int         mi_hogCpu;

public:// fix this later
    long        ml_lengthVariable;  // length of dest but valid only
                                    // for variable rate codecs
    t_qpl_codec_option m_codecOption;
};

#endif