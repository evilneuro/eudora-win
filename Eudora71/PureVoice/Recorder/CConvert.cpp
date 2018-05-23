/*////////////////////////////////////////////////////////////////////////////

NAME:
    CConvert - 

FILE:       CConvert.CPP
AUTHOR:     Livingstone Song (lss)
COMPANY:    QUALCOMM, Inc.
TYPE:       C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:
    Not meant to be derived. Copy ctor or assignment operator not allowed.

DEPENDENCIES:

NOTES:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/??/96   lss     -Initial
04/01/97   lss     -Added capability to adjust cpu usage for encoding
04/23/97   lss     -Modified to support directly writing, reading variable
                    length files
05/24/97   lss     -Only one thread creation/destruction per object. Uses
                    suspend/resumeThread instead.
05/28/97   lss     -Added critical section to Stop, commonEnDecode,
					and killConvertThread
06/04/97   lss     -Uses events to synchro threads. Much cleaner than that of
					05/24/97
06/09/97   lss     -Do ResetEvent for mp_eventThreadStop right after thread
					creation

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "QVoice.h"
#ifdef LS_DEBUG0
    #include "LsDebug.h"
#endif

// LOCAL INCLUDES
#include "CConvert.h"

// LOCAL DEFINES
#define NUMBLOCKS 2

#include "DebugNewHelpers.h"


//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CConvert::CConvert( LPCTSTR sSrcFile, LPCTSTR sDestFile, CACodec *codec, BOOL *pisDone, BOOL bYield )
{
    me_mode     = FILE2FILE;    // source: file, dest: file
    ctor_common( NULL, NULL, sSrcFile, sDestFile, codec, pisDone, bYield );   
}

CConvert::CConvert( CSndBuffer *srcSndBuf, LPCTSTR sDestFile, CACodec *codec, BOOL *pisDone, BOOL bYield )
{
    me_mode     = MEM2FILE;     // source: buffer, dest: file
    ctor_common( srcSndBuf, NULL, NULL, sDestFile, codec, pisDone, bYield );
}

CConvert::CConvert( LPCTSTR sSrcFile, CSndBuffer *destSndBuf, CACodec *codec, BOOL *pisDone, BOOL bYield )
{
    me_mode     = FILE2MEM;     // source: file, dest: buffer
    ctor_common( NULL, destSndBuf, sSrcFile, NULL, codec, pisDone, bYield );
}

CConvert::CConvert( CSndBuffer *srcSndBuf, CSndBuffer *destSndBuf, CACodec *codec, BOOL *pisDone, BOOL bYield )
{
    me_mode     = MEM2MEM;     // source: buffer, dest: buffer
    ctor_common( srcSndBuf, destSndBuf, NULL, NULL, codec, pisDone, bYield );
}

CConvert::~CConvert()
{
    killConvertThread();
    if (mp_mmioInFile)
     {
        mp_mmioInFile->Close();
        delete mp_mmioInFile;
     }
    if (mp_mmioOutFile) delete mp_mmioOutFile;
	if (mp_eventThreadStop) delete mp_eventThreadStop;
	if (mp_eventThreadResume) delete mp_eventThreadResume;
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CConvert::Encode( int nHogCpu /* = 1 */ )
{
    me_useType = ENCODE;
    mi_hogCpu = nHogCpu;
    return commonEnDecode();
}

BOOL CConvert::Decode( BOOL bRealtime /* = FALSE */ )
{
    me_useType = DECODE;
    mb_realtime = bRealtime;
    ml_packetIndex = 0;
    return commonEnDecode();
}

// Stop the current convert thread (actually it suspends the thread) and 
// don't return until we have suspended (actually very close to it)
void CConvert::Stop()
{
	CSingleLock sLock(&m_cs); sLock.Lock();
    mb_Cancel = TRUE;
	CSingleLock sLockStop(mp_eventThreadStop); sLockStop.Lock(5000);
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

// set position to p ms from the beginning
// works only in 20 ms units
unsigned long CConvert::setPositionTime( unsigned long p )
{ 
    unsigned int min = m_codecInfo.samplesPerBlock * 1000 / m_codecInfo.samplesPerSec;
    ml_offset = p;
    p /= min;
//  ml_offset = p * (m_codecInfo.bytesPerPacket);
    return (p*min);
}

// return data length of source
long CConvert::getLength( int type /* = UNIT_BYTES */ )
{
    if ( ((me_mode == FILE2MEM) || (me_mode == FILE2FILE)) && mp_mmioInFile )
        return mp_mmioInFile->getDataLen( type );

    return m_ConvertUnits.Convert( type, ml_length, UNIT_BYTES );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void CConvert::ctor_common(
	CSndBuffer *srcSndBuf,
	CSndBuffer *destSndBuf, 
	LPCTSTR sSrcFile, 
	LPCTSTR sDestFile, 
	CACodec *codec, 
	BOOL *pisDone, 
	BOOL bYield 
)
{
	mp_eventThreadStop = new CEvent( TRUE, TRUE );
	mp_eventThreadResume = new CEvent( FALSE, TRUE );

    mp_srcSndBuf	= srcSndBuf;
    mp_destSndBuf	= destSndBuf;
    ms_srcFile		= sSrcFile;
    ms_destFile		= sDestFile;
    mp_codec		= codec;
    mb_DoYield		= bYield;
    mp_isDone		= pisDone;

    mb_realtime = FALSE;
    me_useType  = 0;

    mp_ConvertThread = NULL;

    if ( mb_DoYield )
        mp_App = (CQVoiceApp *)AfxGetApp();
    else
        mp_App = NULL;

    // these are all in bytes
    ml_length   = 0;
    ml_index    = 0;
    ml_offset   = 0;

    ml_packetIndex  = 0;
    ml_lengthVariable = 0;  // only valid for variable rate codecs

    mb_Cancel       = FALSE;
    mb_ThreadLive   = FALSE;

    mp_mmioInFile   = NULL;
    mp_mmioOutFile  = NULL;

    memset(&m_codecOption, 0, sizeof(m_codecOption) );
    if ( mp_codec )
        mp_codec->Info( &m_codecInfo ); // copy codec info

    // get length of relevant data in bytes
    if ( (me_mode == FILE2MEM) || (me_mode == FILE2FILE))
     {
        mp_mmioInFile = new CQcelpFile;
        // is it an encoded file
        if ( mp_mmioInFile->Open( LPCTSTR(ms_srcFile), CmmioFile::READ ) )
         {// yep, it's an encoded file
            me_useType = DECODE;
            ml_length = mp_mmioInFile->getDataLen();
            mp_mmioInFile->getInfo( &m_codecInfo );
            m_ConvertUnits.SetInfo( m_codecInfo );
         }
        else
         {
			delete mp_mmioInFile;
            mp_mmioInFile = new CWaveFile;
            // nope, then is it a wave file?
            if ( mp_mmioInFile->Open( LPCTSTR(ms_srcFile), CmmioFile::READ ) )
             {// yep, it's a wave file
                me_useType = ENCODE;
                ml_length = mp_mmioInFile->getDataLen();
                m_ConvertUnits.SetInfo( 
                    *(WAVEFORMATEX *)(mp_mmioInFile->getInfo()) );
             }
			else
			 {
				delete mp_mmioInFile; mp_mmioInFile = NULL;
			 }
         }

//      delete mp_mmioInFile;
//      mp_mmioInFile = NULL;
     }
    else
     {
        // for real-time, need to generalize this later
        m_ConvertUnits.SetInfo( m_codecInfo.bitsPerSample>>3, 
                                    m_codecInfo.samplesPerSec);

        ml_length = mp_srcSndBuf->getRecordedLen();
     }
    // force fixed rate since I'll include variable rate in
    // the preprocessing unit some time in the future.
//  m_codecInfo.variableRate.isVariableRate = FALSE;
}

BOOL CConvert::commonEnDecode()
{
	Stop();
	CSingleLock sLock(&m_cs); sLock.Lock();
	ml_lengthVariable = 0;
	mb_Cancel = FALSE;	
    if ( !mb_DoYield )
     {
        if ( !mb_ThreadLive )
         {
			// need to initially set it to true since the thread
			// could be checking this variable before we get to
			// set it in the if loop below
			mb_ThreadLive = TRUE;
            mp_ConvertThread = AfxBeginThread(ConvertThread, this);
            if ( mp_ConvertThread && mp_ConvertThread->m_hThread )
			 {
   				mp_eventThreadStop->ResetEvent();
			 }
			else
			 {
				mb_ThreadLive = FALSE;
				return FALSE;
			 }
			TRACE("===Launch Thread = %X\n", mp_ConvertThread->m_nThreadID);
         }
        else
         {
			mp_eventThreadResume->SetEvent();
			mp_eventThreadStop->ResetEvent();
         }
		return TRUE;
     }
    else
        ConvertThread(this);

	return TRUE;
}

void CConvert::killConvertThread()
{
	CSingleLock sLock(&m_cs); sLock.Lock();

    if ( !mb_ThreadLive ) return;
    mb_ThreadLive = FALSE; 
    mb_Cancel = TRUE;
	mp_eventThreadResume->SetEvent();
	WaitForSingleObject( mp_ConvertThread->m_hThread, 5000 );
	TRACE0("===Exited Thread\n");
}

UINT CConvert::ConvertThread(LPVOID pParam) // static
{
    if ( !pParam )
     {
        TRACE0("CConvert::ConvertThread - pThis = NULL\n");
        return UINT(-1);
     }

    CConvert* pThis = (CConvert*)pParam;

    while ( pThis->mb_ThreadLive )
     {
        switch( pThis->me_mode )
         {
/*
			case FILE2MEM:
                if ( pThis->me_useType == ENCODE )
                    pThis->mp_mmioInFile = new CWaveFile;
                else
                    pThis->mp_mmioInFile = new CQcelpFile;
             break;
*/
            case FILE2FILE: 
/*
                if ( pThis->me_useType == ENCODE )
                    pThis->mp_mmioInFile = new CWaveFile;
                else
                    pThis->mp_mmioInFile = new CQcelpFile;
*/
			// fall through
            case MEM2FILE: 
                if ( pThis->me_useType == ENCODE )
                    pThis->mp_mmioOutFile = new CQcelpFile;
                else
                    pThis->mp_mmioOutFile = new CWaveFile;
             break;
         }
        if ( pThis->me_useType == ENCODE )
         {
            switch( pThis->me_mode )
            {
                case FILE2FILE: 
                    pThis->encode_file2file();
                 break;
                case MEM2FILE: 
                    pThis->encode_mem2file();
                 break;
            }
         }
        else
         {
            switch( pThis->me_mode )
            {
				case MEM2MEM:
					pThis->decode_mem2mem();
				 break;
                case FILE2MEM:
                    pThis->decode_file2mem();
                 break;
                case FILE2FILE:
                    pThis->decode_file2file();
                 break;
            }
         }
        if (pThis->mp_isDone && !pThis->mb_Cancel)
            *(pThis->mp_isDone) = TRUE;

    /*
        if (pThis->mp_mmioInFile)
         {
            delete pThis->mp_mmioInFile;
            pThis->mp_mmioInFile = NULL;
         }
    */
        if (pThis->mp_mmioOutFile)
         {
            delete pThis->mp_mmioOutFile;
            pThis->mp_mmioOutFile = NULL;
         }

		pThis->mp_eventThreadStop->SetEvent();
		TRACE1("==Suspending Thread %X\n", pThis->mp_ConvertThread->m_nThreadID );
		CSingleLock sLock(pThis->mp_eventThreadResume); sLock.Lock();
		TRACE1("==Resuming Thread %X\n", pThis->mp_ConvertThread->m_nThreadID);
		pThis->mp_eventThreadResume->ResetEvent();
		pThis->mp_eventThreadStop->ResetEvent();
     }

    return 0;
}

/*
int CConvert::encode_file2file()
{
    int     err;
    int     size = m_codecInfo.samplesPerBlock *
                  (m_codecInfo.bitsPerSample>>3) * NUMBLOCKS;
    char *buffer = new char[size];
    char *data   = new char[m_codecInfo.bytesPerPacket * NUMBLOCKS];
    int     n    = size;
    int  datasize;
    
    if ((err = open()) != TRUE ) 
        return err;
    mp_codec->init_encoder();   // init our encoder
    
    while ( (n == size) && !mb_Cancel )
     {
        // we will throw away, at most, the last 40 ms worth
        if ((ml_index + size) > ml_length) break;
        n = read((char*)buffer, size);
        datasize = mp_codec->encode( (BYTE*)data, (BYTE*)buffer, 
                            NUMBLOCKS * m_codecInfo.samplesPerBlock );
        if (write( data, datasize ) != datasize )
         {
            break;
         }
        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method so yield process time
     }
    delete [] data;
    delete [] buffer;
    mp_codec->done_encoder();   // done with our encoder
    close();

    return TRUE;
}
*/

int CConvert::encode_file2file()
{
#ifndef LS_PLAYER_ONLY
    int     err;
    int     size = m_codecInfo.samplesPerBlock *
                   (m_codecInfo.bitsPerSample>>3);
    BYTE *buffer = new BYTE[size];
    BYTE *data   = new BYTE[m_codecInfo.bytesPerPacket];
    
	if ( m_codecInfo.variableRate.numOfRates > 0 )
		// yep, inform the file that the packets will be variable
		((CQcelpFile*)mp_mmioOutFile)->VariableRate(1);
	else
		((CQcelpFile*)mp_mmioOutFile)->VariableRate(0);

    if ((err = open()) != TRUE ) 
        return err;
    mp_codec->Reset( &m_codecOption );          // init our encoder
    
	if ( m_codecInfo.variableRate.numOfRates > 0 )
		encode_file2file_variable( size, buffer, data );
	else
		encode_file2file_fixed( size, buffer, data );

    delete [] data;
    delete [] buffer;
    close();
#endif
    return TRUE;
}

void CConvert::encode_file2file_variable( int size, BYTE *buffer, BYTE *data )
{
#ifndef LS_PLAYER_ONLY
    int n = size;
    int datasize, packet_size;
    int i;

    unsigned long   nPackets = 0;
    int packetsPerSec = m_codecInfo.samplesPerSec / m_codecInfo.samplesPerBlock;

    WORD *bytesPerPacket = m_codecInfo.variableRate.bytesPerPacket;
    WORD *formatWord = (WORD *)data;

    WORD *rate = m_codecInfo.variableRate.bytesPerPacket;
    BYTE bytes_per_packet[256];
    
    memset( bytes_per_packet, 0, 256 );
    for ( i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
     {
        bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i])+1;// plus 1 for header byte
     }

    ((CQcelpFile*)mp_mmioOutFile)->OffsetStepSize( 10 );    // 1 second increments

    while ( (n == size) && !mb_Cancel )
     {
        // we will throw away, at most, the last one block
        if ( (ml_index + size) > ml_length ) break;
        n = read(buffer, size);
        datasize = mp_codec->Encode( data, buffer, 
                                m_codecInfo.samplesPerBlock, &m_codecOption );
		nPackets++;
        if ( !(nPackets % packetsPerSec) )
            ((CQcelpFile*)mp_mmioOutFile)->MarkOffset( ml_lengthVariable );
        packet_size = bytes_per_packet[*data];
        write( data, packet_size );
        ml_lengthVariable += packet_size;

        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method so yield process time
     }
    ((CQcelpFile*)mp_mmioOutFile)->setPacketCount( nPackets );

#endif
}

void CConvert::encode_file2file_fixed( int size, BYTE *buffer, BYTE *data )
{
#ifndef LS_PLAYER_ONLY
    int n = size;
    int datasize;

    while ( (n == size) && !mb_Cancel )
     {
        // we will throw away, at most, the last one block
        if ((ml_index + size) > ml_length) return;
        n = read( buffer, size );
        datasize = mp_codec->Encode( data, buffer, 
                                m_codecInfo.samplesPerBlock, &m_codecOption );
        if (write( data, datasize ) != datasize )
         {
            return;
         }
        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method so yield process time
     }
#endif
}

int CConvert::encode_mem2file()
{
#ifndef LS_PLAYER_ONLY
    int     nHogCpu = 0;
    int     datasize, packet_size;
    int     err, i;//, j;
    unsigned long   nPackets = 0;
    int packetsPerSec = m_codecInfo.samplesPerSec / m_codecInfo.samplesPerBlock;
    int numofbytes = m_codecInfo.samplesPerBlock * 
                    (m_codecInfo.bitsPerSample>>3) * NUMBLOCKS;
    BYTE *index;
    BYTE *buffer = new BYTE[numofbytes];
    BYTE *data   = new BYTE[m_codecInfo.bytesPerPacket * NUMBLOCKS];

    int  num_of_rates       = m_codecInfo.variableRate.numOfRates;
    WORD *rate              = m_codecInfo.variableRate.bytesPerPacket;
    BYTE bytes_per_packet[256];
    
    memset( bytes_per_packet, 0, 256 );
    for ( i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
     {
        bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i])+1;// plus 1 for header byte
     }

	if ( num_of_rates > 0 )
		// yep, inform the file that the packets will be variable
		((CQcelpFile*)mp_mmioOutFile)->VariableRate(1);
	else
		((CQcelpFile*)mp_mmioOutFile)->VariableRate(0);


    if ((err = open()) != TRUE ) 
        return err;
    mp_codec->Reset( &m_codecOption );  // init our encoder

    ((CQcelpFile*)mp_mmioOutFile)->OffsetStepSize( 10 );    // 1 second increments

    // Use this for real time encoding. Saves memory by deleting
    // PCM data after it has been encoded.
    while (1)
     {
        if ( mp_srcSndBuf->getCurLen() >= numofbytes )
         {
            if ( mp_srcSndBuf->GetFrameDel((char*)buffer, 1) == 0)
             {
                TRACE0("Encoding: sndBuf->GetFrameDel() returned = 0!\n");
                continue;
             }
            ml_length = mp_srcSndBuf->getRecordedLen();
            ml_index += numofbytes;
            datasize = mp_codec->Encode( data, buffer, 
                    NUMBLOCKS * m_codecInfo.samplesPerBlock, &m_codecOption );
            if ( num_of_rates > 0 )
             {
                index = data;
                
                for ( i=0; i < NUMBLOCKS; ++i)
                 {
                    nPackets++;
                    if ( !(nPackets % packetsPerSec) )
                        ((CQcelpFile*)mp_mmioOutFile)->MarkOffset( ml_lengthVariable );
                    index += (i*datasize/NUMBLOCKS);
                    packet_size = bytes_per_packet[*index];
                    ml_lengthVariable += packet_size;
                    write( index, packet_size );
                 }
             }
            else
                if (write( data, datasize) != datasize)
                 {
                    break;
                 }
            // if we're using non-thread method then yield process time
            if ( mb_DoYield )
                mp_App->YieldEvents();
         }
        else if ((mp_srcSndBuf->getCurLen() < numofbytes) && 
                    mp_srcSndBuf->isDone() )
         {
            break;
         }
        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method so yield process time
        else
         {
            if ( mi_hogCpu > 0 )
             {
                nHogCpu++;
                if ( nHogCpu == mi_hogCpu )
                 {
                    nHogCpu = 0;
                    Sleep(1);
                 }
             }
         }
        if (mb_Cancel ) break; // user interruption
     }

    delete [] data;
    delete [] buffer;
    ((CQcelpFile*)mp_mmioOutFile)->setPacketCount( nPackets );
    close();
#endif
    return TRUE;
}

int CConvert::decode_file2mem()
{
    int     datasize;
    int     err;

    if ((err = open()) != TRUE ) 
        return err;

    int blockSize = m_codecInfo.samplesPerBlock * (m_codecInfo.bitsPerSample>>3);
    int     size = m_codecInfo.bytesPerPacket;
    BYTE *buffer = new BYTE[size];
    BYTE *data   = new BYTE[blockSize*3]; // 3 block size, for slow playback
    int        n = size, i;
    // 1 second worth
    int waitSize = blockSize * (m_codecInfo.samplesPerSec/m_codecInfo.samplesPerBlock);

    WORD *rate   = m_codecInfo.variableRate.bytesPerPacket;
    BYTE bytes_per_packet[256];
    
    memset( bytes_per_packet, 0, 256 );
    for ( i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
     {
        bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i]);
     }

    mp_codec->Reset( &m_codecOption );  // init our decoder

    while ( (ml_offset + ml_index) < ml_length )
     {
        if ( mb_realtime )
         {
            // if buf size reaches waitSize, then wait until
            // it decreases
            while ( mp_destSndBuf->getCurLen() > waitSize )
             {
                if ( mb_Cancel ) break;
                Sleep(1);
             }
         }
        // if variable rate, then load appropriately
        if ( ((CQcelpFile*)mp_mmioInFile)->VariableRate() )
         { 
            n = read( buffer, 1 );
            if ( (n != 1) || mb_Cancel ) 
             {
                break;
             }
            // get the packet size to read
            datasize = bytes_per_packet[*buffer];
            if ( datasize == 0 )
             {
#ifdef _DEBUG
                TRACE("Error at packet #%d - snagged on %d\n",
                    ml_packetIndex+1, *buffer);
                mp_mmioInFile->Seek( -32, CFile::current );
                BYTE debug[64];
                n = read( debug, 64 );
#endif
                break;
             }
            n = read( buffer+1, datasize );
            if ( n != datasize ) 
             {
                break;
             }
            //inBuf += m_codecInfo.bytesPerPacket;
         }
        else
         {
            n = read( buffer, size );
            if ( (n != size) || mb_Cancel )
			 {
				break;
			 }
         }
        datasize = mp_codec->Decode( (BYTE*)data, (BYTE*)buffer, 1, 
                    &m_codecOption );
        ml_packetIndex++;

#ifdef _DEBUG
//      if ( datasize != (m_codecOption.NumOfSamples*2) )
//       {
//          TRACE1("CConvert::decode_file2mem - datasize = %d\n", datasize);
//          datasize = m_codecOption.NumOfSamples*2;
//       }
#endif
        mp_destSndBuf->Put( (char*)data, datasize );
        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method
                                    // so yield process time
     }
    mp_destSndBuf->Done();
    delete data;
    delete buffer;
    close();

    return TRUE;
}

// just for my own special purpose
int CConvert::decode_mem2mem()
{
    int     datasize;
    int     err;

    if ((err = open()) != TRUE ) 
        return err;
	
	char* notEnd = NULL;

    int blockSize = m_codecInfo.samplesPerBlock * (m_codecInfo.bitsPerSample>>3);
    int     size = m_codecInfo.bytesPerPacket;
    BYTE *buffer = new BYTE[size];
    BYTE *data   = new BYTE[blockSize*3]; // 3 block size, for slow playback
    int        n = size, i;
    // 1 second worth
    int waitSize = blockSize * (m_codecInfo.samplesPerSec/m_codecInfo.samplesPerBlock);

    WORD *rate   = m_codecInfo.variableRate.bytesPerPacket;
    BYTE bytes_per_packet[256];
    
    memset( bytes_per_packet, 0, 256 );
    for ( i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
     {
        bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i]);
     }

    mp_codec->Reset( &m_codecOption );  // init our decoder

    while (1)
     {
        if ( mb_realtime )
         {
            // if buf size reaches waitSize, then wait until
            // it decreases
            while ( mp_destSndBuf->getCurLen() > waitSize )
             {
                if ( mb_Cancel ) break;
                Sleep(1);
             }
         }
        // if variable rate, then load appropriately
         { 
            notEnd = mp_srcSndBuf->GetNext( (char*)buffer, 1 );
            if ( !notEnd || mb_Cancel ) 
             {
                break;
             }
            // get the packet size to read
            datasize = bytes_per_packet[*buffer];
            if ( datasize == 0 )
			 {
				break;
			 }
            notEnd = mp_srcSndBuf->GetNext( (char*)(buffer+1), datasize );
            if ( !notEnd ) 
             {
                break;
             }
         }
        datasize = mp_codec->Decode( (BYTE*)data, (BYTE*)buffer, 1, 
                    &m_codecOption );
        ml_packetIndex++;
        mp_destSndBuf->Put( (char*)data, datasize );
        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method
                                    // so yield process time
     }
    mp_destSndBuf->Done();
    delete data;
    delete buffer;
    close();

    return TRUE;
}

int CConvert::decode_file2file()
{
    int     datasize;
    int     err;

    if ((err = open()) != TRUE ) 
        return err;

    int blockSize = m_codecInfo.samplesPerBlock * (m_codecInfo.bitsPerSample>>3);
    int     size = m_codecInfo.bytesPerPacket;
    BYTE *buffer = new BYTE[size];
    BYTE *data   = new BYTE[blockSize*3]; // 3 block size, for slow playback
    int        n = size, i;
    WORD *rate   = m_codecInfo.variableRate.bytesPerPacket;
    BYTE bytes_per_packet[256];
    
    memset( bytes_per_packet, 0, 256 );
    for ( i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
     {
        bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i]);
     }

    mp_codec->Reset( &m_codecOption );  // init our decoder

    while ( (ml_offset + ml_index) < ml_length )
     {
        // if variable rate, then load appropriately
        if ( ((CQcelpFile*)mp_mmioInFile)->VariableRate() )
         { 
            n = read( buffer, 1 );
            if ( (n != 1) || mb_Cancel ) 
             {
                break;
             }
            // get the packet size to read
            datasize = bytes_per_packet[*buffer];
            if ( datasize == 0 )
             {
#ifdef _DEBUG
                TRACE("Error at packet #%d - snagged on %d\n",
                    ml_packetIndex+1, *buffer);
                mp_mmioInFile->Seek( -32, CFile::current );
                BYTE debug[64];
                n = read( debug, 64 );
#endif
                break;
             }
            n = read( buffer+1, datasize );
            if ( n != datasize ) 
             {
                break;
             }
            //inBuf += m_codecInfo.bytesPerPacket;
         }
        else
         {
            n = read( buffer, size );
            if ( (n != size) || mb_Cancel ) break;
         }
        datasize = mp_codec->Decode( (BYTE*)data, (BYTE*)buffer, 1, 
                    &m_codecOption );
        ml_packetIndex++;
        write( data, datasize );
		ml_lengthVariable += datasize;
        if ( mb_DoYield )
            mp_App->YieldEvents();  // we're using non-thread method
                                    // so yield process time
     }
    delete data;
    delete buffer;
    close();

    return TRUE;
}

int CConvert::open()
{
    if (((me_mode == FILE2FILE) ||
         (me_mode == MEM2FILE)) && ms_destFile.IsEmpty())
        return FALSE;

    if (((me_mode == FILE2FILE) ||
         (me_mode == FILE2MEM)) && ms_srcFile.IsEmpty())
        return FALSE;

	WAVEFORMATEX waveFormat;

	if ( DECODE == me_useType )
	 {
		waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
		waveFormat.nChannels       = 1;
		waveFormat.nSamplesPerSec  = 8000;
		waveFormat.nAvgBytesPerSec = 16000;
		waveFormat.nBlockAlign     = 2;
		waveFormat.wBitsPerSample  = 16;
	 }

    ml_index = 0;
    switch( me_mode )
    {
        case MEM2FILE:
            if ( ENCODE == me_useType ) 
				mp_mmioOutFile->setInfo(&m_codecInfo);
			else
				mp_mmioOutFile->setInfo(&waveFormat);
            if ( !mp_mmioOutFile->Open( LPCTSTR(ms_destFile), CmmioFile::WRITE ) )
                return FALSE;
		// fall through
		case MEM2MEM:
			ml_length = mp_srcSndBuf->getRecordedLen();
		break;
        case FILE2FILE:
            if ( ENCODE == me_useType ) 
				mp_mmioOutFile->setInfo(&m_codecInfo);
			else
				mp_mmioOutFile->setInfo(&waveFormat);
            if ( !mp_mmioOutFile->Open( LPCTSTR(ms_destFile), CmmioFile::WRITE ) )
                return FALSE;
        case FILE2MEM:
//          if ( !mp_mmioInFile->Open( LPCTSTR(ms_srcFile), CmmioFile::READ ) )
//              return FALSE;
			if ( !mp_mmioInFile )
				return FALSE;
            ml_length = mp_mmioInFile->getDataLen();
            if ( ml_offset )
//              mp_mmioInFile->Seek( ml_offset, CFile::current );
            if ( DECODE == me_useType ) 
                ((CQcelpFile*)mp_mmioInFile)->SeekInTime( (ULONG*)&ml_offset );
            else
                mp_mmioInFile->SeekInTime( ml_offset );
        break;
    }

    return TRUE;
}

int CConvert::close()
{
    if ( mp_mmioOutFile ) mp_mmioOutFile->Close();
//  if ( mp_mmioInFile ) mp_mmioInFile->Close();
    if ( mp_mmioInFile ) mp_mmioInFile->Seek( 0, CFile::begin );

    return 0;
}

long CConvert::read( BYTE *buffer, int size )
{
    long n = mp_mmioInFile->Read( buffer, size );
    ml_index += n;
    return n;
}

long CConvert::write( BYTE *buffer, int size )
{
    mp_mmioOutFile->Write( buffer, size );
    return size;
}
