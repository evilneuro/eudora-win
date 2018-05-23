/*////////////////////////////////////////////////////////////////////////////

NAME:
	Mixer - Contains very basic mixer capability of volume and peak meter

FILE:		LsMixer.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

	This file contains 3 classes: LsMixerControl, LsMixerVolume, and
	LsMixerVolumeMeter. LsMixerControl is the base class for both
	LsMixerVolume and LsMixerMeter. For now, this is a quick and dirty
	way of doing it but since I don't need other mixer capabilities, this
	will do fine.

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/22/97   lss     Initial
03/11/97   lss     Added window callback support

/*////////////////////////////////////////////////////////////////////////////
#ifndef _LSMIXER_H_
#define _LSMIXER_H_

// INCLUDES
#include <mmsystem.h>

//////////////////////////////////////////////////////////////////////////////
// LsMixerControl Declaration
//////////////////////////////////////////////////////////////////////////////

class LsMixerControl
{
public:
//	LIFECYCLE
	LsMixerControl() : mb_Open(FALSE), m_hmx(0), mp_details(NULL) {};
	virtual ~LsMixerControl(){ Close(); };

//	OPERATIONS
	virtual BOOL Open( UINT muxId, int ctlType, DWORD flag, HWND hWnd = NULL );
	virtual BOOL Close();

//	INQUIRY
	virtual IsOpen(){ return mb_Open; };

//	ENUMS
	enum {
		waveInVolume = 1,
		waveInMeter,
		waveOutVolume,
		waveOutMeter,
		waveIn,
		waveOut
	};

	enum {
		volume = 1,
		meter
	};

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	BOOL				initMixerLine(
									MIXERLINE&	mxl, 
									int			ctlType,
									DWORD&		desiredComponentType,
									DWORD&		desiredControlType );
	BOOL				openDst( MIXERLINE& mxl );
	BOOL				openSrc( MIXERLINE& mxl, DWORD desiredComponentType );
	BOOL				openControl( MIXERLINE& mxl, DWORD desiredControlType );

//	ATTRIBUTES
	HMIXER				m_hmx;
	MIXERCONTROLDETAILS m_mxcd;
	BYTE*				mp_details;

	BOOL				mb_Open;
};

//////////////////////////////////////////////////////////////////////////////
// LsMixerVolume Declaration
//////////////////////////////////////////////////////////////////////////////

class LsMixerVolume : public LsMixerControl
{
public:
//	LIFECYCLE
	LsMixerVolume(){};
	~LsMixerVolume(){};

//	OPERATIONS
	BOOL Open( HWAVEIN hWaveIn, HWND hWnd = NULL );
	BOOL Open( HWAVEOUT hWaveOut, HWND hWnd = NULL );
	BOOL Open( int ctlType, HWND hWnd = NULL );

//	ACCESS
	DWORD Volume();
	void  Volume( DWORD volume );

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
};

//////////////////////////////////////////////////////////////////////////////
// LsMixerVolumeMeter Declaration
//////////////////////////////////////////////////////////////////////////////

class LsMixerVolumeMeter : public LsMixerControl
{
public:
//	LIFECYCLE
	LsMixerVolumeMeter(){};
	~LsMixerVolumeMeter(){};

//	OPERATIONS
	BOOL Open( HWAVEIN hWaveIn, HWND hWnd = NULL );
	BOOL Open( HWAVEOUT hWaveOut, HWND hWnd = NULL );
	BOOL Open( int ctlType, HWND hWnd = NULL );

//	ACCESS
	long Meter();

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
};

#endif
