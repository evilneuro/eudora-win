/*////////////////////////////////////////////////////////////////////////////

NAME:
	CWaveOut and CWavIn - Wave Play and Record Classes

FILE:		CWavePCM.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

	CWaveOut is a wave play class while CWaveIn is a wave record class.
	Both are derived from CAWave abstract class.

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
03/10/97   lss     Added LsMixerVolume
03/21/97   lss     Added GetMixerControl()
04/29/97   lss     Added AGC stuff for CWaveIn
  
/*////////////////////////////////////////////////////////////////////////////
#ifndef _CWAVEPCM_H_
#define _CWAVEPCM_H_

// INCLUDES
#include "CAWave.h"
#include "CSndBuf.h"
#include "mmioFile.h"
#include "LsMixer.h"

struct SSoundMsg
{
	WPARAM	wParam;
	LPARAM	lParam;
};

//////////////////////////////////////////////////////////////////////////////
// CWaveOut Declaration
//////////////////////////////////////////////////////////////////////////////

class CWaveOut : public CAWave
{
public:
//	LIFECYCLE
	CWaveOut( const CString& filename, HWND hWnd = NULL );
	CWaveOut( CSndBuffer *sndBuf, HWND hWnd = NULL );
protected:
	ctor_common();
public:
	virtual ~CWaveOut();

//	OPERATIONS
	virtual long Play( long lDuration=LS_ALL );		
	virtual	long Pause();
	virtual long Stop();

	virtual LsMixerControl* GetMixerControl( int type );

//	ACCESS
	virtual long setPosition( long index, int type=UNIT_SAMPLES );
	virtual long getPosition( int type=UNIT_SAMPLES );
	virtual long getTotalLength( int type=UNIT_SAMPLES );

    virtual long Gain( long gain );	// set volume
    virtual long Gain();			// get volume

//	SYSTEM PURPOSES
	virtual int _play( LPVOID pParam );

//////////////////////////////////////////////////////////////////////////////	
protected:
//	METHODS
	virtual int makeAddWaveBuffer( LPWAVEHDR lpHdr, long size, BOOL isfirstTime );
	virtual int destroyWaveBuffer( LPWAVEHDR lpHdr );

	virtual BOOL openAudioDevice();
	virtual BOOL closeAudioDevice();

	virtual BOOL open();
	virtual BOOL close();

	virtual void read( LPWAVEHDR lpHdr );

/*
	static void CALLBACK waveOutProc ( HWAVEOUT hwi, UINT uMsg, DWORD dwInstance,
									DWORD dwParam1, DWORD dwParam2 );
	void _waveOutProc ( HWAVEOUT hwi, UINT uMsg, DWORD dwInstance,
									DWORD dwParam1, DWORD dwParam2 );
*/
//	ENUMS
	enum{
		FROM_MEM = 1,
		FROM_FILE = 2,
	};

//	ATTRIBUTES
	// we will be sending our messages to this window
	HWND		m_hWnd;
	HWAVEOUT	m_hWaveOut;			// Handle of an open waveform-audio output device

	CSndBuffer *mp_SndBuf;
	CWaveFile	m_mmioFile;
	CString		ms_filename;
	int			me_mode;	// see above enum
	
	long		ml_origWaveHdrBufSize;
	long		ml_otherBufIndex;
	long		ml_offset;	
	long		ml_playLength;
		
	int			mi_playstop;

	BOOL		m_isPlaying;
	BOOL		m_isPaused;

protected:
	// don't allow default or copy ctor
	CWaveOut(){};
	CWaveOut(const CWaveOut& rhs);

private:
	// ain't gonna use these
	long Record(long ns=LS_ALL){return 0;};
	int	_record(LPVOID pParam){return 0;};
};


//////////////////////////////////////////////////////////////////////////////
// CWaveIn Declaration
//////////////////////////////////////////////////////////////////////////////

class CWaveIn : public CAWave
{
public:
//	LIFECYCLE
	CWaveIn( const CString& filename, HWND hWnd, BOOL bAgc = FALSE );
	CWaveIn( CSndBuffer *sndBuf,
			 const CString& filename,
			 HWND hWnd,
			 BOOL bAgc = FALSE );
	CWaveIn( CSndBuffer *sndBuf, HWND hWnd, BOOL bAgc = FALSE );
protected:
	ctor_common( BOOL bAgc );
public:
	virtual ~CWaveIn();

//	OPERATIONS
	virtual long Record( long lDuration=LS_INFINITE );
	virtual	long Pause();
	virtual long Stop();

	virtual LsMixerControl* GetMixerControl( int type );

//	ACCESS
	virtual long getPosition( int type=UNIT_SAMPLES );
	virtual long getTotalLength( int type=UNIT_SAMPLES );

	// not yet supported
    virtual long Gain( long gain ){return -1;};	// set record gain
    virtual long Gain(){return -1;}				// get record gain

//	SYSTEM PURPOSES
	int	_record( LPVOID pParam );

//////////////////////////////////////////////////////////////////////////////	
protected:
//	METHODS
	long	common_stop();
	virtual int makeAddWaveBuffer( LPWAVEHDR lpHdr, long size, BOOL isfirstTime );
	virtual int destroyWaveBuffer( LPWAVEHDR lpHdr );

	virtual BOOL openAudioDevice();
	virtual BOOL closeAudioDevice();

	virtual BOOL open();
	virtual BOOL close();

	virtual BOOL write( char* lpData, UINT nCount );

/*
	static void CALLBACK waveInProc ( HWAVEIN hwi, UINT uMsg, DWORD dwInstance,
									DWORD dwParam1, DWORD dwParam2 );
	void _waveInProc ( HWAVEIN hwi, UINT uMsg, DWORD dwInstance,
									DWORD dwParam1, DWORD dwParam2 );
*/
//	ENUMS
	enum {
		TO_MEM = 1,
		TO_FILE = 2,
	};

//	ATTRIBUTES
	// we will be sending our messages to this window
	HWND		m_hWnd;
	HWAVEIN		m_hWaveIn;	// Handle of an open waveform-audio input device

	CSndBuffer	*m_otherBuf;		
	CWaveFile	m_mmioFile;
	CString		ms_filename;
//	LsMixerVolume	m_gain;	// for gain control
	int			me_mode;	// see enum

	int			mi_stoprecording;

	long		ml_recordLength;
	long		ml_pausePos;	// position at hard pause in samples

	BOOL		mb_agcEnable;
	BOOL		m_isRecording;
	BOOL		m_isPaused;		// not currently used
	BOOL		mb_hardPause;	// meaning pause using stop but leave
								// file (or mem) open

	class Agc
	{
	public:
		Agc( CWaveIn* pContainer );
		~Agc();

		void DoAgc( short* pData, int nCount );

	//////////////////////////////////////////////////////////////////////////
	private:
		LsMixerVolume *mp_volume;
		int			m_agcVolume;
		int			m_oldVolume;
		int			m_agcFlag;
		int			m_agcDelay;
		int			m_origVolume;

		int	m_minVolume, m_maxVolume, m_lowerThresh;
		int	m_overFlowThHi, m_overFlowThLo, m_agcDnFast;
		int m_agcDnSlow, m_agcUpFast, m_agcUpSlow;	
	}	*mp_agc;

protected:
	// don't allow default or copy ctor
	CWaveIn();
	CWaveIn(const CWaveIn& rhs);

private:
	// ain't gonna use these
	long Play(long ns=LS_ALL){return 0;};
	int _play(LPVOID pParam){return 0;};
	virtual long setPosition( long index, int type=UNIT_SAMPLES )
		{return 0;};
};

#endif
