/*////////////////////////////////////////////////////////////////////////////

NAME:
	CAWave - A pure abstract class for Wave In/Out

FILE:		CAWave.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

	Implementation of CAWave. CAWave is a pure abstract class. 
	This is the parent of CWaveOut and CWaveIn classes.

RESCRITICTIONS:

FILES:
	MMIOFILE.h
	CAWAVE.h

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "mmioFile.h"

// LOCAL INCLUDES
#include "CAWave.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CAWave::CAWave()
{
	m_isOpen		= FALSE;
	m_mmt.wType		= TIME_SAMPLES;
//  m_waveHdr1      = ;
//  m_waveHdr2      = ;
    ml_error        = 0;
    ml_length       = 0;
	ml_position		= 0;
	mp_waveHdrCur	= 0;
	ml_gain			= 0;

	// by default, use PCM format of 8KHz rate with 16 bit samples
	m_waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
	m_waveFormat.nChannels       = 1;
	m_waveFormat.nSamplesPerSec  = 8000;
	m_waveFormat.nAvgBytesPerSec = 16000;
	m_waveFormat.nBlockAlign     = 2;
	m_waveFormat.wBitsPerSample  = 16;

	mi_sampleSize	  = m_waveFormat.wBitsPerSample >> 3;

    ml_waveHdrBufSize = 4000;   // 4000 samples = 0.5 sec worth

	m_ConvertUnits.SetInfo( m_waveFormat );	// init our unit conversion member
} // CAWAVE


//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

unsigned long CAWave::convertUnits(
	int				destType,
	unsigned long	index,
	int				srcType )
{
/*
	if ( destType == srcType )
		return index;

	if ( destType == UNIT_TIME_MS )
	 {
		if ( srcType == UNIT_SAMPLES )
			return (long)((float)index * 1000 / m_waveFormat.nSamplesPerSec);
		if ( srcType == UNIT_BYTES )
			return (long)((float)index * 1000 /
			(m_waveFormat.nBlockAlign * m_waveFormat.nSamplesPerSec) );
	 }
	if ( destType == UNIT_SAMPLES )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (long)((float)index * m_waveFormat.nSamplesPerSec / 1000);
		if ( srcType == UNIT_BYTES )
			return (long)((float)index / m_waveFormat.nBlockAlign );
	 }
	if ( destType == UNIT_BYTES )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (long)((float)index * m_waveFormat.nSamplesPerSec *
							m_waveFormat.nBlockAlign / 1000);
		if ( srcType == UNIT_SAMPLES )
			return (long)((float)index * m_waveFormat.nBlockAlign );
	 }
	
	return index; // to satify "not all control paths return a value"
*/
	return m_ConvertUnits.Convert( destType, index, srcType );
} // convertUnits

/*
void CAWave::Error( DWORD error )
{
	switch( error )
	{
		case MMSYSERR_NODRIVER:
			error = ERROR_MM_NODRIVER;
		 break;
		case MMSYSERR_ALLOCATED:
			error = ERROR_MM_INUSE;
		 break;
		case MMSYSERR_BADDEVICEID:
			error = ERROR_MM_BADDEVICEID;
		 break;
		case WAVERR_BADFORMAT:
			error = ERROR_MM_BADFORMAT;
		 break;
		default:
			error = ERROR_MM_CANT_OPEN_DEVICE;
		 break;
	}
	ml_error = error;
} // Error
*/
