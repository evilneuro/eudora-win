/*////////////////////////////////////////////////////////////////////////////

NAME:
	CWaveOutDecode - 

FILE:		CWaveCon.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/??/96   lss     -Initial
05/23/97   lss     -In read(), correctly handle buffer length at the end

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"
#include "stdlib.h"
#include "memory.h"

// PROJECT INCLUDES
#include "QVoice.h"
#ifdef LS_DEBUG0
	#include "LsDebug.h"
	int LsDebug::m_EmptyBuffer;
#endif

// LOCAL INCLUDES
#include "CWaveCon.h"

// LOCAL DEFINES
#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CWaveOutDecode::CWaveOutDecode( CSndBuffer *sndBuf, HWND hWnd,
							   CACodec *codec, BOOL bUseYield )
{
	ctor_common( sndBuf, NULL, hWnd, codec, bUseYield );	
}

CWaveOutDecode::CWaveOutDecode( const CString& filename, HWND hWnd,
							   CACodec *codec, BOOL bUseYield )
{
	ctor_common( NULL, filename, hWnd, codec, bUseYield );	
}

CWaveOutDecode::~CWaveOutDecode()
{
	if (m_isPlaying) Stop();
	if (m_isOpen) closeAudioDevice();
	// first important that we delete mp_converter first before mp_SndBuf
	// or we could be referring to destructed mp_SndBuf
	delete mp_converter;
	delete mp_SndBuf;
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

long CWaveOutDecode::Play( long lDuration )
{
	// if paused then unpause and return
	if ( m_isPaused )
	 {
		m_isPaused = FALSE;
		waveOutRestart ( m_hWaveOut );
		return ml_position;
	 }

    int min = mi_sndPageSize/mi_sampleSize;
	
	if ( m_isPlaying || (lDuration == 0) || (ml_length <= 0) || 
        ((ml_length - ml_position) < (min * 2)) )
     {
        return 0;
     }

    if ( !openAudioDevice() )
	{
		closeAudioDevice();
		return -1;
	}
	
	ml_offset			= ml_position;
	ml_otherBufIndex	= 0;
	mi_playstop			= 0;
	
	if ( (lDuration < 0) || (lDuration > ml_length) ||
		 ((min+lDuration) > ml_length) )
     {
		ml_playLength = ml_length;
     }
	else if ( lDuration < min )
     {
		ml_playLength = min;
     }
	else
	 {	
		// truncate the play length so it's multiple of 'min'
		ml_playLength = int(lDuration/min) * min ;
	 }
    
    if ( ml_playLength > (ml_length - ml_offset) )
     {
        ml_playLength = ml_length - ml_offset;
     }

	ml_waveHdrBufSize = ml_origWaveHdrBufSize;

    // make it multiple of 'min'
	if ( ml_playLength < ml_waveHdrBufSize )
     {
		ml_waveHdrBufSize = ml_playLength / 2;
        ml_waveHdrBufSize /= min;
        ml_waveHdrBufSize *= min;
     }

//	ml_waveHdrBufSize = 2880;

//	CString tstStr;
//	tstStr.Format( "Len = %d, Play_Len = %d", ml_length, ml_playLength );
//	AfxMessageBox( LPCTSTR(tstStr) );
//

	// first pause it so we can stuff our double buffer
	waveOutPause ( m_hWaveOut );

	TRACE0("SndBuf Reset Better Had Happened!!!!\n");
	// make sure we have at least 2 buffers length ready
	while ( (mp_SndBuf->getCurLen() < ml_waveHdrBufSize * mi_sampleSize * 2)
		    && !mp_SndBuf->isDone() )
		Sleep(1);

    // make sure we really do have 2 buffers length
    if ( (mp_SndBuf->getCurLen() < ml_waveHdrBufSize * mi_sampleSize * 2) )
     {
		if ( mp_SndBuf->getCurLen() >= min*mi_sampleSize*2 )
		 {
			ml_playLength = mp_SndBuf->getCurLen() / mi_sampleSize;
			ml_waveHdrBufSize = ml_playLength / 2;
			ml_waveHdrBufSize /= min;
			ml_waveHdrBufSize *= min;
		 }
		else
		 {
			closeAudioDevice();
			return ml_position;
		 }
     }

	// make and add 1st buffer
    if( makeAddWaveBuffer( &m_waveHdr1, ml_waveHdrBufSize, TRUE ) == FALSE )
     {     
		closeAudioDevice();
		return -1;               
     }

    // make and add 2nd buffer
    if( makeAddWaveBuffer( &m_waveHdr2, ml_waveHdrBufSize, TRUE ) == FALSE )
     {                   
		closeAudioDevice();
		return -1;               
     }

	// we have our double buffer so let's go
	waveOutRestart ( m_hWaveOut );

	mp_waveHdrCur = &m_waveHdr1;
	m_isPlaying = TRUE;
	
	// if no valid HWND then we will not return until we finish playing
	SSoundMsg msg;
	msg.lParam = LPARAM(mp_waveHdrCur);
	while( !m_hWnd )
	 {
		if ( mp_waveHdrCur->dwFlags & WHDR_DONE )
		 {
			_play(&msg);
			msg.lParam = LPARAM(mp_waveHdrCur);
		 }
		if ( !m_isPlaying ) break;
		if ( mi_playstop < 2 ) ((CQVoiceApp*)AfxGetApp())->YieldEvents();
	 }

	return ml_position;
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

long CWaveOutDecode::setPosition( long index, int type )
{
	index = mp_converter->setPositionTime( 
	  convertUnits( UNIT_TIME_MS, 
				CWaveOut::setPosition( index, type ), type ) );
	return CWaveOut::setPosition( index, UNIT_TIME_MS );
}

long CWaveOutDecode::getPosition( int type )
{
//	if ( 0 == m_playSpeed )
//		return CWaveOut::getPosition( type );

	if ( m_isPlaying )
	 {
		ml_packetCnt = mp_converter->PacketIndex();
		mf_timeFactor = ml_packetCnt * mi_codecFrameSize *
				(float)mi_sampleSize / (float)mp_SndBuf->getRecordedLen();
		waveOutGetPosition( m_hWaveOut, &m_mmt, sizeof (MMTIME));
		TRACE("CWaveOutDecode: ml_packetCnt = %d, playfactor = %f\n", ml_packetCnt, mf_timeFactor );
		return convertUnits( type, (mf_timeFactor*m_mmt.u.sample+ml_offset),
						UNIT_SAMPLES );
	 }
	else
		return convertUnits( type, ml_position, UNIT_SAMPLES );
}

int CWaveOutDecode::PlaySpeed( int speed )
{
	int old_speed = m_playSpeed;

	LS_RANGE( speed, -3, 3 );
	m_playSpeed = speed;
	TRACE1("PLAY SPEED = %d\n", m_playSpeed);
	if (mp_converter)
		mp_converter->m_codecOption.Playback = m_playSpeed;	

	return old_speed;
}

int CWaveOutDecode::PlaySpeed()
{
	return m_playSpeed;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CWaveOutDecode::ctor_common( CSndBuffer *sndBuf, LPCTSTR filename, 
							HWND hWnd, CACodec *codec, BOOL bUseYield )
{
    CWaveOut::ctor_common();

	// these are used for correct timing info during fast/slow playback
	mf_timeFactor = 1.0F;
	ml_packetCnt  = 0;

	m_playSpeed	  = 0;	// normal speed

	const t_qpl_codec_info *pCodecInfo = codec->Info();

	m_waveFormat.nSamplesPerSec  = pCodecInfo->samplesPerSec;
	m_waveFormat.nBlockAlign     = 2;
	m_waveFormat.wBitsPerSample  = pCodecInfo->bitsPerSample;
	mi_sampleSize				 = m_waveFormat.wBitsPerSample >> 3;
	m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * mi_sampleSize ;
	mi_codecFrameSize			 = pCodecInfo->samplesPerBlock; // this is in samples
	// make the CSndBuf page size to be 2 times the codec's frame size
	mi_sndPageSize				 = mi_codecFrameSize * mi_sampleSize * 2;

	mp_SndBuf = new CSndBuffer( TRUE, mi_sndPageSize );
	if ( sndBuf )
		mp_converter = new CConvert( sndBuf, mp_SndBuf, codec, NULL, bUseYield);
	else
		mp_converter = new CConvert( filename, mp_SndBuf, codec, NULL, bUseYield);

	// make size of wave bufs in multiples of mi_sndPageSize
	// let's make it in the range of .5 secs
	int t = convertUnits( UNIT_TIME_MS, mi_sndPageSize/mi_sampleSize, UNIT_SAMPLES );
	if ( int(500/t) ) t = int(500/t)*t;
	ml_origWaveHdrBufSize	= convertUnits( UNIT_SAMPLES, t, UNIT_TIME_MS );

	ml_length = mp_converter->getLength( UNIT_SAMPLES );
	
	m_hWnd = hWnd;

	return 0;
}

int CWaveOutDecode::openAudioDevice()
{
	BOOL ret = CWaveOut::openAudioDevice();
	if ( ret )
	 {
		mp_SndBuf->Reset();
		mp_converter->Decode( TRUE );
	 }

#ifdef LS_DEBUG0
	LsDebug::m_EmptyBuffer = 0;
#endif

	return ret;
}

int CWaveOutDecode::closeAudioDevice()
{
	BOOL ret = CWaveOut::closeAudioDevice();
	mp_converter->Stop();
#ifdef LS_DEBUG0
//	if (  LsDebug::m_EmptyBuffer )
//	 {
//		LsDebug debug;
//		char buf[80];
//		sprintf( buf, "%d empty cases", LsDebug::m_EmptyBuffer );
//		debug.DebugMsg( buf );
//	 }
#endif
	return ret;
}

void CWaveOutDecode::read( LPWAVEHDR lpHdr )
{
	char *pStart = lpHdr->lpData;
	char *pStop	 = lpHdr->lpData + lpHdr->dwBufferLength;
	
	while (pStart < pStop)
	 {
		int cnt = mp_SndBuf->GetFrameDel( pStart, 1 );
		if ( cnt == 0 )
		 {
			if ( mp_SndBuf->isDone() )
			 {
				// we're done
				mi_playstop = 1; 
				int old = lpHdr->dwBufferLength;
				long diff = pStart - lpHdr->lpData;// - mi_sndPageSize;
				LS_RANGE( diff, 0, old );
				lpHdr->dwBufferLength = diff;
				memset( lpHdr->lpData+lpHdr->dwBufferLength, 
								0, old-lpHdr->dwBufferLength );
				return;
			 }
			else
			 {
#ifdef LS_DEBUG0
				LsDebug::m_EmptyBuffer++;
#endif
				memset( pStart, 0, pStop - pStart );
				break;
			 }
		 }
		pStart += mi_sndPageSize;
	 }
	TRACE1("PLAY DECODE: SndBuf Size = %d\n", mp_SndBuf->getCurLen() );
}
/*
//************************************************************************
// CWaveInDecode Class Implementation
//************************************************************************
CWaveInEncode::ctor_common( CSndBuffer *sndBuf, const CString& filename, 
							HWND hWnd, CACodec *codec, BOOL bUseYield )
{
	m_waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
	m_waveFormat.nChannels       = 1;

	mi_stoprecording	= 0;
	m_isRecording		= FALSE;
	m_isPaused			= FALSE;

	m_isConvDone		= FALSE;	// for converter
	m_isConvInterrupt	= FALSE;	// for converter

	m_mmt.wType			= TIME_SAMPLES;

	mp_waveHdrCur		= NULL;

	t_qpl_codec_info *pCodecInfo = codec->Info();

	m_waveFormat.nSamplesPerSec  = pCodecInfo->nSamplesPerSec;
	m_waveFormat.nBlockAlign     = 2;
	m_waveFormat.wBitsPerSample  = pCodecInfo->wBitsPerSample;
	mi_sampleSize				 = m_waveFormat.wBitsPerSample >> 3;
	m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * mi_sampleSize ;
	mi_codecFrameSize			 = pCodecInfo->frameSize; // this is in samples

	// make the CSndBuf page size to be 2 times the codec's frame size
	mi_sndPageSize				 = mi_codecFrameSize * mi_sampleSize * 2;

	// let's make it in the range of .5 secs
	ml_waveHdrBufSize	= mi_sampleSize * convertUnits( UNIT_SAMPLES, 500, UNIT_TIME_MS );

	mp_SndBuf = new CSndBuffer( FALSE, mi_sndPageSize );
	if ( sndBuf )
		mp_converter = new CConvert( mp_SndBuf, sndBuf, codec, &m_isConvDone, bUseYield );
	else
		mp_converter = new CConvert( mp_SndBuf, LPCTSTR(filename), codec, &m_isConvDone, bUseYield );

	m_hWnd = hWnd;

	return 0;
}

CWaveInDecode::CWaveInDecode(CSndBuffer *sndBuf, HWND hWnd, 
				 CACodec *codec, BOOL bUseYield )
{
	ctor_common( sndBuf, NULL, hWnd, codec, bUseYield );	
	me_mode	= TO_MEM; // record to sndBuf 
}

CWaveInDecode::CWaveInDecode( const CString& filename, HWND hWnd,
				 CACodec *codec, BOOL bUseYield )
{
	ctor_common( NULL, filenam, hWnd, codec, bUseYield );	
	me_mode	= TO_FILE; // record to file 
}

CWaveInDecode::CWaveInDecode( CSndBuffer *sndBuf, const CString& filename, 
							HWND hWnd, CACodec *codec, BOOL bUseYield )
{
	ctor_common( sndBuf, filename, hWnd, codec, bUseYield );	
	me_mode	= TO_FILE | TO_MEM; // record to file and sndBuf
}

CWaveInDecode::~CWaveInDecode()
{
	if ( m_isRecording ) Stop();
	if ( m_isOpen ) closeAudioDevice();
	// first important that we delete mp_converter first before mp_SndBuf
	// or we could be referring to destructed mp_SndBuf
	delete mp_converter;
	delete mp_SndBuf;
}

long CWaveInDecode::getPosition( int type )
{
	waveInGetPosition( m_hWaveIn, &m_mmt, sizeof (MMTIME));

	return convertUnits( type, m_mmt.u.sample, UNIT_SAMPLES );
}

long CWaveInDecode::getTotalLength( int type )
{
	return convertUnits( type, ml_length, UNIT_SAMPLES );
}

BOOL CWaveInDecode::openAudioDevice()
{
	MMRESULT err;

	if ( m_isOpen ) 
	 {
		setError( ERROR_MM_ALREADY_OPENED );
	 	return FALSE;
     }
	
	err = waveInOpen( &m_hWaveIn, WAVE_MAPPER, &m_waveFormat, (DWORD) m_hWnd,
				(DWORD) 0, (DWORD) CALLBACK_WINDOW );
	
	if ( err == MMSYSERR_NOERROR )
		m_isOpen = TRUE;
	else
	 {
		setError( ERROR_MM_CANT_OPEN_DEVICE );
		return FALSE;
	 }

	if (me_mode & TO_FILE) 
	 {
		if ( !open() )
		 {
			setError( ERROR_FILE_CANT_OPEN );
			return FALSE;
		 }
	 }

	return TRUE;
}

BOOL CWaveInDecode::closeAudioDevice()
{
	MMRESULT err;
	BOOL ret = TRUE;

	if ( !m_isOpen ) 
	 {
		setError( ERROR_MM_ALREADY_CLOSED );
	 	return FALSE;
     }
	err = waveInClose( m_hWaveIn );
	
	if ( err == MMSYSERR_NOERROR )
	 {
		m_isOpen		= FALSE;
		m_isRecording	= FALSE;
		m_isPaused		= FALSE;
	 }
	else
	 {
		setError( ERROR_MM_CANT_CLOSE_DEVICE );
		ret = FALSE;
	 }

	if (me_mode & TO_FILE) 
		close();

	return ret;
}

long CWaveInDecode::Record( long lDuration )
{
	MMRESULT err;
	
	// if paused then unpause and return
	if ( m_isPaused )
	 {
		m_isPaused = FALSE;
		waveInStart ( m_hWaveIn );
		return ml_position;
	 }

	if ( !openAudioDevice() )
	 {
		closeAudioDevice();
		return -1;
	 }

	if ( m_isRecording && (lDuration == 0) ) return 0;
	mi_stoprecording = 0;

	if ( lDuration < 0 )
		ml_recordLength = 0;
	else
		ml_recordLength = lDuration;

	if ( ml_recordLength < ml_waveHdrBufSize )
		ml_waveHdrBufSize = ml_recordLength;

	// start the recording off first...
	if ( err = waveInStart(m_hWaveIn) )
     {
		ERRTRACE( err );
		setError( ERROR_MM_CANT_START_RECORD );
		closeAudioDevice();
		return -1;
	 }

	// make and add 1st buffer
    if(makeAddWaveBuffer( &m_waveHdr1, ml_waveHdrBufSize, TRUE ) == FALSE)
     {                   
		closeAudioDevice();
		return -1;               
     }

    // make and add 2nd buffer
    if(makeAddWaveBuffer( &m_waveHdr2, ml_waveHdrBufSize, TRUE )== FALSE)
     {                   
		closeAudioDevice();
		return -1;               
     }

	mp_waveHdrCur = &m_waveHdr1;
	m_isRecording = TRUE;
	
	return 1;
}

long CWaveInDecode::Stop()
{
	MMRESULT err;

	if ( m_isConvInterrupt )
	 {
		m_isConvInterrupt = FALSE;
		mp_converter->Stop();
	 }

	if ( !m_isRecording ) return 0;

	m_isConvInterrupt = !m_isConvDone;
	mi_stoprecording = 1;

	err = waveInStop(m_hWaveIn);
	ERRTRACE(err);
	err = waveInReset(m_hWaveIn);
	ERRTRACE(err);

	// it's ok if we unprepare already unprepared wavehdr
	err = waveInUnprepareHeader(m_hWaveIn, &m_waveHdr1, sizeof(WAVEHDR));
	ERRTRACE(err);
	err = waveInUnprepareHeader(m_hWaveIn, &m_waveHdr2, sizeof(WAVEHDR));
	ERRTRACE(err);

	write ( mp_waveHdrCur->lpData, mp_waveHdrCur->dwBytesRecorded );

	if (me_mode & TO_MEM) 
		m_otherBuf->Done(); // ok, tell buffer we're done

	ml_length += (mp_waveHdrCur->dwBytesRecorded/mi_sampleSize);	

	delete [] m_waveHdr1.lpData;
	delete [] m_waveHdr2.lpData;
		
	closeAudioDevice();

	m_isRecording	= FALSE;
	m_isPaused		= FALSE;

	return 0;
}

long CWaveInDecode::Pause()
{
	if ( !m_isRecording ) return 0;

	m_isPaused = TRUE;
	waveInStop(m_hWaveIn);
	
	return ml_position;
}

// size in samples
int CWaveInDecode::makeAddWaveBuffer( LPWAVEHDR lpHdr, long size, BOOL isfirstTime )
{
	MMRESULT err;

	if (isfirstTime)
	 {
		lpHdr->dwBufferLength	= size * mi_sampleSize;
		lpHdr->lpData			= (LPSTR)(new char[lpHdr->dwBufferLength]);
	 }
	lpHdr->dwUser			= 0;
	lpHdr->dwFlags			= 0;
	lpHdr->dwLoops			= 0;	
	//TRACE("lpHdr->dwBufferLength = %d\n", lpHdr->dwBufferLength);
	err = waveInPrepareHeader( m_hWaveIn, lpHdr, sizeof(WAVEHDR) );
	if ( err != MMSYSERR_NOERROR )
		return FALSE;

	err = waveInAddBuffer( m_hWaveIn, lpHdr, sizeof(WAVEHDR) );
	if ( err != MMSYSERR_NOERROR )
     {
		setError( ERROR_MM_BUF_ADD );
		destroyWaveBuffer( &m_waveHdr1 );
		return FALSE;
     }  

	return TRUE;
}

int CWaveInDecode::destroyWaveBuffer( LPWAVEHDR lpHdr )
{
	waveInUnprepareHeader( m_hWaveIn, lpHdr, sizeof(WAVEHDR) );
	delete [] lpHdr->lpData;
	lpHdr->lpData = 0;
	
	return TRUE;
}

int CWaveInDecode::_record( LPVOID pParam )
{ 
	SSoundMsg *msg  = (SSoundMsg *)pParam;
    LPWAVEHDR lpHdr = (LPWAVEHDR) msg->lParam;
    MMRESULT err;

	if ( mi_stoprecording ) return 0L;
	
	// check to see if request record time is up
	// if so then stop and return
	if ( (ml_recordLength > 0) && (ml_recordLength >= ml_length) )
	 {
		Stop();
		return 0L;
	 } 

	// unprepare immediately
	if ((err = waveInUnprepareHeader( m_hWaveIn, lpHdr, sizeof(WAVEHDR) )
		MMSYSERR_NOERROR ) 
	 {
		ERRTRACE( err );
		return 0L;
	 }   

	write( lpHdr->lpData, lpHdr->dwBytesRecorded ); 

	ml_length += ( lpHdr->dwBytesRecorded/mi_sampleSize );

	if ( lpHdr->dwBytesRecorded == lpHdr->dwBufferLength )
	 { 
		// ok, we need to add another buffer
		// get the used WAVEHDR and re-use it
		// and update current WAVEHDR being used
		
		if (lpHdr == &m_waveHdr1)
			mp_waveHdrCur = &m_waveHdr2;
		else 
			mp_waveHdrCur = &m_waveHdr1;
		
		if(makeAddWaveBuffer( lpHdr, ml_waveHdrBufSize, FALSE) == FALSE )
		 {                   
			return 0L;             
		 }    
	 }
	return 0L;
}

BOOL CWaveInDecode::open()
{ 
	m_mmioFile.setInfo( &m_waveFormat );
	return m_mmioFile.Open( LPCTSTR(ms_filename), CWaveFile::WRITE );
} 

BOOL CWaveInDecode::close()
{
	m_mmioFile.Close();
	return TRUE;
}

void CWaveInDecode::write( char* lpData, UINT nCount )
{
	if ( me_mode & TO_FILE )  // to file
	 {
		// Write the waveform data subchunk.
		m_mmioFile.Write( lpData, nCount ); 
	 }
	if ( me_mode & TO_MEM )
	 {
		long i = m_otherBuf->Put( lpData, nCount );
		TRACE("lpData = %X, nCount = %d, %d\n", lpData, nCount, i);
	 }
}

*/