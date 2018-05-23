/*////////////////////////////////////////////////////////////////////////////

NAME:
	CAWave - A pure abstract class for Wave In/Out

FILE:		CAWave.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

	CAWave is a pure abstract class. This is the parent of
	CWaveOut and CWaveIn classes

RESCRITICTIONS:

DEPENDENCIES:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////
#ifndef _CAWAVE_H_
#define _CAWAVE_H_

// INCLUDES
#include "stdafx.h"
#include "lscommon.h"
#include "QPL.H"
#include "LsConvertUnit.h"

/*
enum {
	UNIT_TIME_MS,
	UNIT_SAMPLES,
	UNIT_BYTES
};
*/

class CAWave
{
public:
//	LIFECYCLE
	CAWave();
	virtual ~CAWave(){};

//	OPERATIONS
	// play from current position for ns samples and 
	// return position start in samples
	virtual long Play( long lDuration=LS_ALL ) = 0;		
	
	// record for maxLength in samples
	// but not more than maxLength in samples
	virtual long Record( long lDuration=LS_INFINITE ) = 0;
	
	// pause and return current position in samples
	virtual long Pause() = 0;
	
	// stop and return current position in samples
	// returns the position to the beginning
	virtual long Stop() = 0;

//	ACCESS	
    // set gain
    // sets volume for wave output and record gain for wave input
	// setting range is 0..15
    virtual long Gain( long gain ) = 0;
    virtual long Gain() = 0;

	// goto position at index in units of 'type', see enum at
	// the top for choices
	// return index, note that returned index may not exactly
	// equal to parameter index if the unit is not sample
	virtual long setPosition( long index, int type=UNIT_SAMPLES ) = 0;
	
	// return current position in units of 'type'
	// see enum at the top for choices
	virtual long getPosition( int type=UNIT_SAMPLES ) = 0;

	// return total length of sound in units of 'type'
	virtual long getTotalLength( int type=UNIT_SAMPLES ) = 0;

	// copy our WAVEFORMATEX struct to *wvFormat
	virtual void getWaveFormat( LPWAVEFORMATEX wvFormat )
		{memcpy( wvFormat, &m_waveFormat, sizeof(m_waveFormat) );};

	virtual	DWORD Error(){DWORD ret = ml_error;ml_error=0;return ret;};

	// return converted index of type 'srcType'
	// in units of 'destType'
	virtual unsigned long convertUnits(
		int				destType,
		unsigned long	index,
		int				srcType );

// ENUMS
	enum {	
		LS_ALL = -1,
		LS_INFINITE = -1
	};

//	SYSTEM PURPOSES
	// for Windows call back function
	virtual int _record(LPVOID pParam)=0;
	virtual int _play(LPVOID pParam)=0;

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	virtual void Error( DWORD error ){ml_error = error;};

//	ATTRIBUTES
	BOOL		m_isOpen;			// device open?

	int			mi_sampleSize;		// sample size in bytes
	long		ml_position;		// current position in samples
	long		ml_length;			// length of sound in samples

	long		ml_waveHdrBufSize;	// buffer size for WAVEHDR.lpData [samples]

	WAVEFORMATEX m_waveFormat;		// Defines the Wave Format

	MMTIME		m_mmt;				// MM Time

	WAVEHDR		m_waveHdr1;			// Header for the 1st sample buffer
	WAVEHDR		m_waveHdr2;			// Header for the 2nd sample buffer
	LPWAVEHDR	mp_waveHdrCur;		// Current header

	DWORD		ml_error;			// error number

	DWORD		ml_gain;			// volume/record gain

	LsConvertUnit	m_ConvertUnits;	// for unit conversion
};

#endif
