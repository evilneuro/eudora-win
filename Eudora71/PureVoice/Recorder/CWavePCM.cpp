/*////////////////////////////////////////////////////////////////////////////

NAME:
	CWaveOut and CWavIn - Wave Play and Record Classes

FILE:		CWavePCM.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:
	Implementation of CWaveOut and CWaveIn.
	CWaveOut is a wave play class while CWaveIn is a wave record class.

RESCRITICTIONS:

DEPENDENCIES:

FILES:
	STDAFX.H
	STDLIB.H
	MEMORY.H
	LSCOMMON.H
	QPL.H
	CWAVEPCM.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
08/18/97   lss     If WAVE_MAPPER somehow fails, try all waveOut/In manually
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"
#include "stdlib.h"
#include "memory.h"

// PROJECT INCLUDES
#include "QVoice.h"
#include "lscommon.h"
#include "qpl.h"
#include "LsMixer.h"

// LOCAL INCLUDES
#include "CWavePCM.h"

#include "DebugNewHelpers.h"

#ifdef _DEBUG
void ERRTRACE( int err )
{
	if (err == MMSYSERR_NOERROR)
		TRACE("No MM error\n");
	else if (err == MMSYSERR_INVALHANDLE) 
		TRACE("Specified device handle is invalid.\n");
	else if (err == MMSYSERR_NODRIVER)
		TRACE("No device driver is present.\n");
	else if (err == MMSYSERR_NOMEM)
		TRACE("Unable to allocate or lock memory.\n");
	else if (err == WAVERR_STILLPLAYING)
		TRACE("The buffer pointed to by the pwh parameter is still in the queue.\n");
	else if (err == MMSYSERR_ALLOCATED)
		TRACE("Specified resource is already allocated.\n");
	else if (err == MMSYSERR_BADDEVICEID)
		TRACE("Specified device identifier is out of range.\n");
	else if (err == WAVERR_BADFORMAT)
		TRACE("Attempted to open with an unsupported waveform-audio format.\n");
	else
		TRACE("MM unknown\n");
}
#else
#define ERRTRACE(err)
#endif

//////////////////////////////////////////////////////////////////////////////
// CWaveOut Class Implementation
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CWaveOut::ctor_common()
{
    m_hWaveOut          = 0;
    m_hWnd              = 0;
//  m_mmioFile          =  ;
	ml_otherBufIndex	= 0;
    me_mode             = 0;
	mi_playstop			= 0;
	m_isPlaying			= FALSE;
	m_isPaused			= FALSE;
	ml_offset			= 0;
    ml_playLength       = 0;
    mp_SndBuf           = 0;
	ml_origWaveHdrBufSize = ml_waveHdrBufSize;
    ms_filename.Empty();

	return 0;
}

CWaveOut::CWaveOut( CSndBuffer *sndBuf, HWND hWnd )
{
	ctor_common();

	me_mode			= FROM_MEM;	// play from sndBuf
	m_hWnd			= hWnd;
	mp_SndBuf		= sndBuf;
	ml_length		= mp_SndBuf->getCurLen()/mi_sampleSize;
}

CWaveOut::CWaveOut( const CString& filename, HWND hWnd )
{
	ctor_common();

	me_mode		= FROM_FILE;	// play from file
	ms_filename = LPCTSTR (filename);
	if ( m_mmioFile.Open( LPCTSTR(filename), CmmioFile::WAVE_SUPPORTED ) )
	 {
		m_mmioFile.getInfo( &m_waveFormat );
		mi_sampleSize = m_waveFormat.wBitsPerSample >> 3;
		ml_length = m_mmioFile.getDataLen()/mi_sampleSize;
//		m_waveFormat.nSamplesPerSec  = 16000;
		ml_origWaveHdrBufSize = m_waveFormat.nSamplesPerSec / 2; // .5 sec worth	
		m_waveFormat.nAvgBytesPerSec =
				m_waveFormat.nSamplesPerSec * mi_sampleSize ;
		m_ConvertUnits.SetInfo( m_waveFormat );
	 }
	else
		Error( ERROR_MM_BADFORMAT );
	m_hWnd = hWnd;
}

CWaveOut::~CWaveOut()
{
	if ( m_isPlaying ) Stop();
	if ( m_isOpen ) closeAudioDevice();
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CWaveOut::openAudioDevice()
{
	MMRESULT err;

	if ( m_isOpen ) 
	 {
//		Error( ERROR_MM_ALREADY_OPENED );
	 	return TRUE;
     }
	
	if ( m_hWnd )
	{
	int maxWaveOut = waveOutGetNumDevs();
	// indx of -1 is WAVE_MAPPER, try WAVE_MAPPER first
	for ( int indx = -1; indx < maxWaveOut; indx++ )
	 {
		err = waveOutOpen( &m_hWaveOut, indx, &m_waveFormat, (DWORD) m_hWnd,
					(DWORD) 0, (DWORD) CALLBACK_WINDOW );
		if ( err == MMSYSERR_NOERROR )
			break;
	 }
	}
	else
	err = waveOutOpen( &m_hWaveOut, WAVE_MAPPER, &m_waveFormat, (DWORD) 0,
				(DWORD) 0, (DWORD) CALLBACK_NULL );

	ERRTRACE(err);
	if ( err == MMSYSERR_NOERROR )
		m_isOpen = TRUE;
	else
	 {
		switch( err )
		{
			case MMSYSERR_NODRIVER:
				err = ERROR_MM_NODRIVER;
			 break;
			case MMSYSERR_ALLOCATED:
				err = ERROR_MM_INUSE;
			 break;
			case MMSYSERR_BADDEVICEID:
				err = ERROR_MM_BADDEVICEID;
			 break;
			case WAVERR_BADFORMAT:
				err = ERROR_MM_BADFORMAT;
			 break;
			default:
				err = ERROR_MM_CANT_OPEN_DEVICE;
			 break;
		}
		Error( err );
		return FALSE;
	 }

	if ( me_mode == FROM_FILE ) 
	 {
		if ( !open() )
		 {
			Error( ERROR_FILE_CANT_OPEN );
			return FALSE;
		 }
	 }

	::waveOutGetVolume( m_hWaveOut, &ml_gain );

	return TRUE;
}

BOOL CWaveOut::closeAudioDevice()
{
	MMRESULT err;
	BOOL ret = TRUE;

	if ( !m_isOpen ) 
	 {
//		Error( ERROR_MM_ALREADY_CLOSED );
	 	return TRUE;
     }
	
	err = waveOutClose( m_hWaveOut );
	ERRTRACE(err);
	
	if ( err == MMSYSERR_NOERROR )
	 {
		m_isOpen    = FALSE;
		m_isPlaying = FALSE;
		m_isPaused	= FALSE;
	 }
	else
	 {
		Error( ERROR_MM_CANT_CLOSE_DEVICE );
		ret = FALSE;
	 }

	if ( me_mode == FROM_FILE ) 
		close();

	return ret;
}

long CWaveOut::Play( long lDuration )
{
	// if paused then unpause and return
	if ( m_isPaused )
	 {
		m_isPaused = FALSE;
		waveOutRestart ( m_hWaveOut );
		return ml_position;
	 }
	
	if ( m_isPlaying || (lDuration == 0) ||
        (ml_length <= 0) || ((ml_length - ml_position) < 2) )
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

	if ( (lDuration < 0) || (lDuration > ml_length) )
		ml_playLength = ml_length;
	else
		ml_playLength = lDuration;

    if ( ml_playLength > (ml_length - ml_offset))
     {
        ml_playLength = ml_length - ml_offset;
     }

	ml_waveHdrBufSize = ml_origWaveHdrBufSize;

	if ( ml_playLength < ml_waveHdrBufSize )
		ml_waveHdrBufSize = ml_playLength >> 1;

	// first pause it so we can stuff our double buffer
	waveOutPause( m_hWaveOut );

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
		if ( !m_isPlaying )	break;
		if ( mi_playstop < 2 ) ((CQVoiceApp*)AfxGetApp())->YieldEvents();
	 }

	return ml_position;
}

long CWaveOut::Stop()
{
	if ( !m_isPlaying ) return 0;

    ml_position = getPosition();

	waveOutReset(m_hWaveOut);

	waveOutUnprepareHeader(m_hWaveOut, &m_waveHdr1, sizeof(WAVEHDR));
	waveOutUnprepareHeader(m_hWaveOut, &m_waveHdr2, sizeof(WAVEHDR));

	// ok, delete our double buffer
	delete [] m_waveHdr1.lpData;
	delete [] m_waveHdr2.lpData;
	
    m_isPlaying = FALSE;
	m_isPaused	= FALSE;

	if ( !closeAudioDevice() )
        return -1;

	return ml_position;
}

long CWaveOut::Pause()
{
	if ( !m_isPlaying ) return 0;

	m_isPaused = TRUE;
	waveOutPause ( m_hWaveOut );
	
	return ml_position;
}

LsMixerControl* CWaveOut::GetMixerControl( int type )
{
	LsMixerControl* pMc = NULL;
	if ( !m_isOpen ) return pMc;
	switch( type )
	 {
		case LsMixerControl::volume:
			pMc = new LsMixerVolume;
			if ( !((LsMixerVolume*)pMc)->Open( m_hWaveOut ) )
			 {
				delete pMc;
				pMc = NULL;
			 }
		 break;
		case LsMixerControl::meter:
			pMc = new LsMixerVolumeMeter;
			if ( !((LsMixerVolumeMeter*)pMc)->Open( m_hWaveOut ) )
			 {
				delete pMc;
				pMc = NULL;
			 }		 
		 break;
		default:
			TRACE0("Unknown type - CWaveOut::GetMixerControl\n");
	 }
	return pMc;
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

long CWaveOut::setPosition( long index, int type )
{
	index = convertUnits( UNIT_SAMPLES, index, type );

	if ( index < 0 ) 
		index = 0;
	else
		if ( index > getTotalLength() )
			index = getTotalLength();

	ml_position = index;

	return convertUnits( type, ml_position, UNIT_SAMPLES );
}

long CWaveOut::getPosition( int type )
{
	if ( m_isPlaying )
	 {
		waveOutGetPosition( m_hWaveOut, &m_mmt, sizeof (MMTIME));
		return convertUnits( type, (m_mmt.u.sample+ml_offset), UNIT_SAMPLES );
	 }
	else
		return convertUnits( type, ml_position, UNIT_SAMPLES );
}

long CWaveOut::getTotalLength( int type )
{
	return convertUnits( type, ml_length, UNIT_SAMPLES );
}

// set volume and return orig volume
long CWaveOut::Gain( long volume )
{
	LS_RANGE( volume, 0, 100 );
	volume = volume / 100.0 * 0xFFFF;
	long v = 100.0 * (0xFFFF & ml_gain) / 0xFFFF ;
	ml_gain = (DWORD(volume) << 16) | DWORD(volume);
	if ( m_isOpen )
		waveOutSetVolume( m_hWaveOut, ml_gain );

	return v;
}

long CWaveOut::Gain()
{
	return 100.0 * (0xFFFF & ml_gain) / 0xFFFF;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// size in samples
int CWaveOut::makeAddWaveBuffer( LPWAVEHDR lpHdr, long size, BOOL isfirstTime )
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

	read( lpHdr );

	TRACE3("PLAY: [&WaveBuf, Size] = [%X, %d], Cur Pos(bytes) = %d\n",
		lpHdr->lpData, lpHdr->dwBufferLength, ml_otherBufIndex );

	ml_otherBufIndex += lpHdr->dwBufferLength; 
	ml_position += (lpHdr->dwBufferLength / mi_sampleSize);

	if ( lpHdr->dwBufferLength < 1 ) 
     {
        mi_playstop++;
        return TRUE;
     }
	
	err = waveOutPrepareHeader(m_hWaveOut, lpHdr, sizeof(WAVEHDR));
	if ( err != MMSYSERR_NOERROR )
     {
		Error( ERROR_MM_BUF_ADD );
		destroyWaveBuffer( lpHdr );
		return FALSE;
     } 

	err = waveOutWrite(m_hWaveOut, lpHdr, sizeof(WAVEHDR));

	if ( err != MMSYSERR_NOERROR )
     {
		Error( ERROR_MM_BUF_ADD );
		destroyWaveBuffer( lpHdr );
		return FALSE;
     }  

	return TRUE;
}

int CWaveOut::destroyWaveBuffer( LPWAVEHDR lpHdr )
{
	waveOutUnprepareHeader( m_hWaveOut, lpHdr, sizeof(WAVEHDR) );
	delete [] lpHdr->lpData; lpHdr->lpData = NULL;
	
	return TRUE;
}

int CWaveOut::_play(LPVOID pParam)
{ 
	SSoundMsg *msg  = (SSoundMsg *)pParam;
    LPWAVEHDR lpHdr = (LPWAVEHDR) msg->lParam;

	// update our current header
	if (lpHdr == &m_waveHdr1)
		mp_waveHdrCur = &m_waveHdr2;
	else 
		mp_waveHdrCur = &m_waveHdr1;

	// last buffer was stuffed and wait until we finish
	// playing the buffers.. wouldn't want to stop prematurely now..
	if (mi_playstop)
	 {
		mi_playstop++;
		if (mi_playstop > 2)
		 {
			Stop();
		 }
		return 0L;
	 }
		
	// unprepare immediately
	if (waveOutUnprepareHeader(m_hWaveOut, lpHdr, sizeof(WAVEHDR)) !=
		MMSYSERR_NOERROR ) 
		return 0L;

	if (makeAddWaveBuffer(lpHdr, ml_waveHdrBufSize, FALSE) == FALSE)
	 {                   
		return 0L;             
	 }

	return 0L;
}

BOOL CWaveOut::open()
{ 
	BOOL ret = m_mmioFile.Open( LPCTSTR(ms_filename), CWaveFile::READ );
	if (ret)
		m_mmioFile.Seek( ml_position*mi_sampleSize, CFile::current );
	return ret;
} 

BOOL CWaveOut::close()
{
	m_mmioFile.Close();
	return TRUE;
}

void CWaveOut::read( LPWAVEHDR lpHdr )
{
	if ((ml_otherBufIndex + lpHdr->dwBufferLength) >
		(ml_playLength * mi_sampleSize) )
	 {
		lpHdr->dwBufferLength = (ml_playLength * mi_sampleSize) - ml_otherBufIndex;
		mi_playstop=1; // eh-hm, this is the last buffer we're stuffing
	 }	
	if ( me_mode == FROM_FILE )  // from file
	 {
		m_mmioFile.Read( lpHdr->lpData, lpHdr->dwBufferLength );
	 }
	else if ( me_mode == FROM_MEM )
	 {
		mp_SndBuf->Get( lpHdr->lpData, ml_otherBufIndex + 
                (ml_offset * mi_sampleSize), lpHdr->dwBufferLength);
	 }
}
/*
void CALLBACK CWaveOut::waveOutProc ( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, 
									DWORD dwParam1, DWORD dwParam2 )
{
	CWaveOut *pseudoThis = (CWaveOut*) dwInstance;
	if ( pseudoThis != (CWaveOut*) NULL )
		pseudoThis->_waveOutProc ( hwo, uMsg, dwInstance, dwParam1, dwParam2 );
}

void CWaveOut::_waveOutProc ( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, 
									DWORD dwParam1, DWORD dwParam2 )
{
	SSoundMsg msg;
	msg.lParam = dwParam1;

	switch(uMsg)
     {
		case WOM_CLOSE:
			::PostMessage( m_hWnd, MM_WOM_CLOSE, 0, 0 );
        break;
        case WOM_DONE:
			TRACE("ENTER: _play\n");
			_play(&msg);
			TRACE("EXIT: _play\n");
        break;
        case WOM_OPEN:
        break;
     }   
}
*/

//////////////////////////////////////////////////////////////////////////////
// CWaveIn Class Implementation
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CWaveIn::ctor_common( BOOL bAgc )
{
	mi_stoprecording	= 0;
	m_isRecording		= FALSE;
	m_isPaused			= FALSE;
	mb_hardPause		= FALSE;
	ml_pausePos			= 0;

	mb_agcEnable		= bAgc;
	mp_agc				= NULL;

    ml_waveHdrBufSize	= 1600;   // 1600 samples = 0.2 sec worth

	return 0;
}

CWaveIn::CWaveIn( CSndBuffer *sndBuf, HWND hWnd, BOOL bAgc ) 
{
	ctor_common( bAgc );
	me_mode			= TO_MEM; // record to sndBuf 
	m_hWnd			= hWnd;
	m_otherBuf		= sndBuf;
}

CWaveIn::CWaveIn( const CString& filename, HWND hWnd, BOOL bAgc )
{
	ctor_common( bAgc );
	me_mode			= TO_FILE; // record to file 
	ms_filename		= LPCTSTR (filename);
	m_hWnd			= hWnd;
}

CWaveIn::CWaveIn(
	CSndBuffer *sndBuf,
	const CString& filename,
	HWND hWnd,
	BOOL bAgc 
)
{
	ctor_common( bAgc );
	me_mode			= TO_FILE | TO_MEM; // record to file and sndBuf
	m_otherBuf		= sndBuf;
	ms_filename		= LPCTSTR (filename);
	m_hWnd			= hWnd;
}

CWaveIn::~CWaveIn()
{
	if ( m_isRecording ) Stop();
	if ( m_isOpen ) closeAudioDevice();
	delete mp_agc;
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

long CWaveIn::getPosition( int type )
{
	if ( m_isRecording )
	 {
		waveInGetPosition( m_hWaveIn, &m_mmt, sizeof (MMTIME));
		return convertUnits( type, m_mmt.u.sample+ml_pausePos, UNIT_SAMPLES );
	 }
	else
		return convertUnits( type, ml_position, UNIT_SAMPLES );
}

long CWaveIn::getTotalLength( int type )
{
	return convertUnits( type, ml_length, UNIT_SAMPLES );
}

//////////////////////////////////////////////////////////////////////////////
// OPERATION
////////////

BOOL CWaveIn::openAudioDevice()
{
	MMRESULT err;

	if ( m_isOpen ) 
	 {
//		Error( ERROR_MM_ALREADY_OPENED );
	 	return TRUE;
     }

	int maxWaveIn = waveInGetNumDevs();
	// indx of -1 is WAVE_MAPPER, try WAVE_MAPPER first
	for ( int indx = -1; indx < maxWaveIn; indx++ )
	 {
		err = waveInOpen( &m_hWaveIn, indx, &m_waveFormat, (DWORD) m_hWnd,
				(DWORD) 0, (DWORD) CALLBACK_WINDOW );
		if ( err == MMSYSERR_NOERROR )
			break;
	 }

//	err = waveInOpen( &m_hWaveIn, WAVE_MAPPER, &m_waveFormat, (DWORD) waveInProc,
//				(DWORD)this, (DWORD) CALLBACK_FUNCTION );
	
	if ( err == MMSYSERR_NOERROR )
		m_isOpen = TRUE;
	else
	 {
		switch( err )
		{
			case MMSYSERR_NODRIVER:
				err = ERROR_MM_NODRIVER;
			 break;
			case MMSYSERR_ALLOCATED:
				err = ERROR_MM_INUSE;
			 break;
			case MMSYSERR_BADDEVICEID:
				err = ERROR_MM_BADDEVICEID;
			 break;
			case WAVERR_BADFORMAT:
				err = ERROR_MM_BADFORMAT;
			 break;
			default:
				err = ERROR_MM_CANT_OPEN_DEVICE;
			 break;
		}
		Error( err );
		return FALSE;
	 }

	if ( mb_hardPause == FALSE )
	 {
		if (me_mode & TO_FILE)
		 {
			if ( !open() )
			 {
				Error( ERROR_FILE_CANT_OPEN );
				return -1;
			 }
		 }
		if (me_mode & TO_MEM) 
			m_otherBuf->Reset(); 
		
		ml_length = 0;
		ml_pausePos = 0;

//		m_gain.Open( m_hWaveIn );
	 }
	
	if ( mb_agcEnable )
	 {
		delete mp_agc;
		mp_agc = new Agc(this);
	 }
	else
	 {
		mp_agc = NULL;
	 }

	return TRUE;
}

BOOL CWaveIn::closeAudioDevice()
{
	MMRESULT err;
	BOOL ret = TRUE;

	if ( m_isOpen ) 
	 {
		err = waveInClose( m_hWaveIn );

		if ( err == MMSYSERR_NOERROR )
		 {
			m_isOpen		= FALSE;
			m_isRecording	= FALSE;
			m_isPaused		= FALSE;
			delete mp_agc; mp_agc = NULL;
		 }
		else
		 {
			Error( ERROR_MM_CANT_CLOSE_DEVICE );
			ret = FALSE;
		 }
	 }
	if ( mb_hardPause == FALSE )
	 {
		if ( me_mode & TO_FILE ) 
			close();
		if (me_mode & TO_MEM) 
			m_otherBuf->Done(); // ok, tell buffer we're done
	 }

	return ret;
}

long CWaveIn::Record( long lDuration )
{
	MMRESULT err;
	
	// if paused then unpause and return
	if ( m_isPaused )
	 {
		m_isPaused = FALSE;
		waveInStart ( m_hWaveIn );
		return ml_position;
	 }

	// return if lDuration is zero or less or we are still recording
	if ( m_isRecording ||
		 ((lDuration < 0) && (lDuration != LS_INFINITE)) )
		return 0;

	if ( !openAudioDevice() )
	 {
		closeAudioDevice();
		return -1;
	 }

	mi_stoprecording = 0;

	if ( lDuration == LS_INFINITE )
		ml_recordLength = 0;
	else
		ml_recordLength = lDuration;

    ml_waveHdrBufSize	= 1600;   // 1600 samples = 0.2 sec worth

	if ( (lDuration != LS_INFINITE) && (ml_recordLength < ml_waveHdrBufSize) )
		ml_waveHdrBufSize = ml_recordLength;
/*
	// start the recording off first...
	if ( err = waveInStart(m_hWaveIn) )
     {
		ERRTRACE( err );
		Error( ERROR_MM_CANT_START_RECORD );
		closeAudioDevice();
		return -1;
	 }
*/
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

	// now start the recording
	if ( err = waveInStart(m_hWaveIn) )
     {
		ERRTRACE( err );
		Error( ERROR_MM_CANT_START_RECORD );
		closeAudioDevice();
		return -1;
	 }

	mp_waveHdrCur = &m_waveHdr1;
	m_isRecording = TRUE;
	
	return 1;
}

long CWaveIn::Stop()
{
	mb_hardPause = FALSE;
	m_isPaused = FALSE;
	
	common_stop();

	return ml_position;
}

long CWaveIn::Pause()
{
	if ( !m_isRecording ) return 0;

	mb_hardPause = TRUE;
	common_stop();
	ml_pausePos = ml_position;
//	m_isPaused = TRUE;
//	waveInStop(m_hWaveIn);

	return ml_position;
}

LsMixerControl* CWaveIn::GetMixerControl( int type )
{
	LsMixerControl* pMc = NULL;
	if ( !m_isOpen ) return pMc;
	switch( type )
	 {
		case LsMixerControl::volume:
			pMc = new LsMixerVolume;
			if ( !((LsMixerVolume*)pMc)->Open( m_hWaveIn ) )
			 {
				delete pMc;
				pMc = NULL;
			 }	
		 break;
		case LsMixerControl::meter:
			pMc = new LsMixerVolumeMeter;
			if ( !((LsMixerVolumeMeter*)pMc)->Open( m_hWaveIn ) )
			 {
				delete pMc;
				pMc = NULL;
			 }	
		 break;
		default:
			TRACE0("Unknown type - CWaveIn::GetMixerControl\n");
	 }
	return pMc;
}

/*
// set gain and return orig gain
long CWaveIn::Gain( long volume )
{
	LS_RANGE( volume, 0, 100 );
	volume = volume / 100.0 * 0xFFFF;
	long v = 100.0 * (0xFFFF & ml_gain) / 0xFFFF ;
	ml_gain = (DWORD(volume) << 16) | DWORD(volume);
	if ( m_isOpen )
		m_gain.Volume( ml_gain );

	return v;
}

long CWaveIn::Gain()
{
	return 100.0 * (0xFFFF & ml_gain) / 0xFFFF;
}
*/
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

long CWaveIn::common_stop()
{
	MMRESULT err;
	
	if ( m_isRecording )
	 {
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

		ml_length += (mp_waveHdrCur->dwBytesRecorded/mi_sampleSize);	
		ml_position = ml_length;

		delete [] m_waveHdr1.lpData;
		delete [] m_waveHdr2.lpData;
	 }

	closeAudioDevice();
	m_isRecording	= FALSE;

	return 0;
}

// size in samples
int CWaveIn::makeAddWaveBuffer( LPWAVEHDR lpHdr, long size, BOOL isfirstTime )
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
	 {
		Error( ERROR_MM_BUF_ADD );
		destroyWaveBuffer( lpHdr );
		return FALSE;
	 }

	err = waveInAddBuffer( m_hWaveIn, lpHdr, sizeof(WAVEHDR) );
	if ( err != MMSYSERR_NOERROR )
     {
		Error( ERROR_MM_BUF_ADD );
		destroyWaveBuffer( lpHdr );
		return FALSE;
     }  

	return TRUE;
}

int CWaveIn::destroyWaveBuffer( LPWAVEHDR lpHdr )
{
	waveInUnprepareHeader( m_hWaveIn, lpHdr, sizeof(WAVEHDR) );
	delete [] lpHdr->lpData; lpHdr->lpData = NULL;
	
	return TRUE;
}

int CWaveIn::_record( LPVOID pParam )
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
	if ((err = waveInUnprepareHeader( m_hWaveIn, lpHdr, sizeof(WAVEHDR) )) !=
		MMSYSERR_NOERROR ) 
	 {
		ERRTRACE( err );
		return 0L;
	 }   

	if ( !write( lpHdr->lpData, lpHdr->dwBytesRecorded ) )
	 {
		Stop();
		return 0L;
	 }

	ml_length += ( lpHdr->dwBytesRecorded/mi_sampleSize );
	ml_position = ml_length;

	/*
	// are we done yet???
	if (mi_stoprecording)
	 {
		// it's ok if we unprepare already unprepared wavehdr
		waveInUnprepareHeader(m_hWaveIn, &m_waveHdr1, sizeof(WAVEHDR));
		waveInUnprepareHeader(m_hWaveIn, &m_waveHdr2, sizeof(WAVEHDR));

		delete [] m_waveHdr1.lpData;
		delete [] m_waveHdr2.lpData;
		
		m_otherBuf->Done();
		
		closeAudioDevice();
		
		return 0L;
	 }
	*/
	if ( lpHdr->dwBytesRecorded == lpHdr->dwBufferLength )
	 { 
		// ok, we need to add another buffer
		// get the used WAVEHDR and re-use it
		// and update current WAVEHDR being used
		
		if (lpHdr == &m_waveHdr1)
			mp_waveHdrCur = &m_waveHdr2;
		else 
			mp_waveHdrCur = &m_waveHdr1;
		
		if( makeAddWaveBuffer(lpHdr, ml_waveHdrBufSize, FALSE) == FALSE )
		 {                   
			return 0L;             
		 }    
	 }
	return 0L;
}

BOOL CWaveIn::open()
{ 
	m_mmioFile.setInfo( &m_waveFormat );
	return m_mmioFile.Open( LPCTSTR(ms_filename), CWaveFile::WRITE );
} 

BOOL CWaveIn::close()
{
	m_mmioFile.Close();
	return TRUE;
}

BOOL CWaveIn::write( char* lpData, UINT nCount )
{
	if ( mp_agc ) mp_agc->DoAgc( (short*)lpData, nCount/2 );
	if ( me_mode & TO_FILE )  // to file
	 {
		// Write the waveform data subchunk.
		m_mmioFile.Write( lpData, nCount ); 
		TRACE2("RECORD: &waveBuf, size(bytes) = (%X, %d)\n", lpData, nCount );
	 }
	if ( me_mode & TO_MEM )
	 {
		long i = m_otherBuf->Put( lpData, nCount );
		if ( i < 0 )
		 {
//			Error( ERROR_NO_MEMORY );
			return FALSE;
		 }
		TRACE3("RECORD: [&WaveBuf, Size] = [%X, %d], SndBuf Size = %d\n",
				lpData, nCount, m_otherBuf->getCurLen() );
	 }
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CWaveIn::Agc stuff

CWaveIn::Agc::Agc( CWaveIn* pContainer ) :
	mp_volume(NULL), m_agcVolume(90), m_agcFlag(0), m_agcDelay(0),
	m_oldVolume(0), m_minVolume(10), m_maxVolume(99),
	m_overFlowThHi(28000), m_overFlowThLo(24000), m_lowerThresh(20000),
	m_agcDnFast(-8), m_agcDnSlow(-2), m_agcUpFast(3),
	m_agcUpSlow(1), m_origVolume(99)
{
	CStdioFile f;
	if ( f.Open(LPCTSTR("agc.txt"), CFile::modeRead) )
	 {
		CString parse;
		char buf[256], *stopstring;
		int data[10], i = 0;
		while ( f.ReadString(buf, 255) || (i < 10) )
		 {
			if ( buf[0] == ';' ) continue; // skip comments
			parse = buf;
			parse = parse.Right(parse.GetLength()-parse.Find('=')-1);
			data[i++] = strtod( LPCTSTR(parse), &stopstring );
		 }
		if ( i == 10 )
		 {
			m_agcVolume = data[0];
			m_maxVolume = data[1];
			m_minVolume = data[2];
			m_overFlowThHi = data[3];
			m_overFlowThLo = data[4];
			m_lowerThresh  = data[5];
			m_agcDnFast = data[6];
			m_agcDnSlow = data[7];
			m_agcUpFast = data[8];
			m_agcUpSlow = data[9];
		 }
	 }
	f.Close();
	mp_volume = (LsMixerVolume*)pContainer->GetMixerControl( LsMixerControl::volume );
	if (mp_volume) m_origVolume = mp_volume->Volume();
}

CWaveIn::Agc::~Agc()
{
	if (mp_volume) mp_volume->Volume(m_origVolume);
	delete mp_volume;
}

void CWaveIn::Agc::DoAgc( short* pData, int nCount )
{
	short tmp;
	m_agcFlag = 0;
	for ( int i = 0; i < nCount; i++ )
	 {
		tmp = pData[i];
		tmp = tmp > -1 ? tmp : -tmp;
		if ( tmp > m_overFlowThHi )
		 {
			m_agcFlag = 2;
			break;
		 }
		if ( tmp > m_lowerThresh )
		 {
			m_agcFlag = 1;
		 }
		if ( tmp > m_overFlowThLo )
		 {
			m_agcFlag = 3;
		 }
	 }
	if ( !m_agcFlag )
	 {
		m_agcVolume += m_agcUpFast;
	 }
	else if ( 1 == m_agcFlag )
	 {
		m_agcVolume += m_agcUpSlow;
	 }
	else if ( 2 == m_agcFlag )
	 {
		m_agcVolume += m_agcDnFast;
	 }
	else if ( 3 == m_agcFlag )
	 {
		m_agcVolume += m_agcDnSlow;
	 }
	if ( m_agcVolume > m_maxVolume ) m_agcVolume = m_maxVolume;
	else if ( m_agcVolume <= m_minVolume ) m_agcVolume = m_minVolume;
	if (mp_volume && (m_agcVolume != m_oldVolume) )
	 {
		mp_volume->Volume( m_agcVolume );
		m_oldVolume = m_agcVolume;
	 }
	TRACE2("Volume = %d, Flag = %d\n", m_agcVolume, m_agcFlag );
}

/*
void CALLBACK CWaveIn::waveInProc ( HWAVEIN hwi, UINT uMsg, DWORD dwInstance,
									DWORD dwParam1, DWORD dwParam2 )
{
	CWaveIn *pseudoThis = (CWaveIn*) dwInstance;
	if ( pseudoThis != (CWaveIn*) NULL )
		pseudoThis->_waveInProc ( hwi, uMsg, dwInstance, dwParam1, dwParam2 );
}

void CWaveIn::_waveInProc ( HWAVEIN hwi, UINT uMsg, DWORD dwInstance,
									DWORD dwParam1, DWORD dwParam2 )
{
	SSoundMsg msg;
	msg.lParam = dwParam1;	

	switch(uMsg)
	 {
        case WIM_CLOSE:
        break;
        case WIM_DATA:
			_record(&msg);
        break;
        case WIM_OPEN:
		break;
	 }       
}
*/