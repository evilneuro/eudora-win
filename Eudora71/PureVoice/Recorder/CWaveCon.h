#ifndef _CWAVECON_H_
#define _CWAVECON_H_

#include "CAWave.h"
#include "CWavePCM.h"
#include "CACodec.h"
#include "CConvert.h"
//#include "mmioFile.h"

//************************************************************************
// Class  : CWaveOutDecode
// Parent : CWaveOut
//
//************************************************************************
class CWaveOutDecode : public CWaveOut
{
public:
//	LIFECYCLE
	CWaveOutDecode(const CString& filename, HWND hWnd, CACodec *codec,
			BOOL bUseYield = FALSE );
	CWaveOutDecode(CSndBuffer *sndBuf, HWND hWnd, CACodec *codec,
			BOOL bUseYield = FALSE );
	virtual ~CWaveOutDecode();

//	OPERATIONS
	virtual long Play( long lDuration=LS_ALL );		

//	ACCESS
	virtual long setPosition( long index, int type=UNIT_SAMPLES );
	virtual long getPosition( int type=UNIT_SAMPLES );
	virtual int  PlaySpeed( int speed );
	virtual int  PlaySpeed();


//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	ctor_common (CSndBuffer *sndBuf, LPCTSTR filename, 
							HWND hWnd, CACodec *codec, BOOL bUseYield );

	virtual BOOL openAudioDevice();
	virtual BOOL closeAudioDevice();

	virtual void read( LPWAVEHDR lpHdr );

//	ATTRIBUTES
	int		mi_codecFrameSize;		// in samples
	int		mi_sndPageSize;			// in bytes

	long	ml_packetCnt;
	float	mf_timeFactor;

	int		m_playSpeed;			// 0 = norm, -1 = slow, 1 = fast
public:
	CConvert *mp_converter;

private:
	// don't allow default ctor, copy, assignment
	CWaveOutDecode();
	CWaveOutDecode(const CWaveOutDecode& rhs);
	CWaveOutDecode& operator=( const CWaveOutDecode& rhs );
};

/*
//************************************************************************
// Class  : CWaveInEncode
// Parent : CWaveIn
//
//************************************************************************
class CWaveInEncode : public CWaveIn
{
public:
	// ctor
	CWaveInEncode(const CString& filename, HWND hWnd, CACodec *codec,
			BOOL bUseYield = FALSE, BOOL bDelete = FALSE);
	CWaveInEncode(CSndBuffer *sndBuf, const CString& filename, HWND hWnd,
			CACodec *codec,	BOOL bUseYield = FALSE, BOOL bDelete = FALSE);
	CWaveInEncode(CSndBuffer *sndBuf, HWND hWnd, CACodec *codec,
			BOOL bUseYield = FALSE, BOOL bDelete = FALSE);

	// dctor
	virtual ~CWaveInEncode();

	virtual long Record( long ml_recordLength=LS_INFINITE );
	virtual	long Pause();
	virtual long Stop();

	virtual long getPosition( int type=UNIT_SAMPLES );
	virtual long getTotalLength( int type=UNIT_SAMPLES );

	// called by MSWindows
	virtual int	_record( LPVOID pParam );
	
protected:
	ctor_common();

	int makeAddWaveBuffer( LPWAVEHDR lpHdr, long size, BOOL isfirstTime );
	int destroyWaveBuffer( LPWAVEHDR lpHdr );

	BOOL openAudioDevice();
	BOOL closeAudioDevice();

	BOOL open();
	BOOL close();

	void write( char* lpData, UINT nCount );

private:
	// we will be sending our messages to this window
	HWND		m_hWnd;
	HWAVEIN		m_hWaveIn;	// Handle of an open waveform-audio input device

	CSndBuffer	*m_otherBuf;		
	CWaveFile	mmioFile;
	CString		ms_filename;
	int			me_mode;	// see enum

	int			mi_stoprecording;
	long		ml_totalSamples;

	BOOL		m_isRecording;
	BOOL		m_isPaused;

	BOOL		m_isConvDone;
	BOOL		m_isConvInterrupt;

private:
	// don't allow default or copy ctor
	CWaveInEncode();
	CWaveInEncode(const CWaveIn& rhs);
};
*/
#endif