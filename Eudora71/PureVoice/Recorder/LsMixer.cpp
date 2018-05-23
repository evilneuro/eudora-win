/*////////////////////////////////////////////////////////////////////////////

NAME:
	Mixer - Contains very basic mixer capability of volume and peak meter

FILE:		LsMixer.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:
	This file contains 3 classes: LsMixerControl, LsMixerVolume, and
	LsMixerVolumeMeter. LsMixerControl is the base class for both
	LsMixerVolume and LsMixerMeter. For now, this is a quick and dirty
	way of doing it but since I don't need other mixer capabilities, this
	will do fine.

RESCRITICTIONS:

DEPENDENCIES:

FILES:
	STDAFX.H
	MEMORY.H
	LSCOMMON.H
	LSMIXER.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/22/97   lss     Initial
03/11/97   lss     Added window callback support
03/26/97   lss     Separated LsMixerControl::Open into initMixerLine, 
				   openDst, openSrc, and openControl so that if a control was
				   not found then in a src then it would look in its dst.
				   This modification came about because toshiba laptops listed
				   WaveIn peak meter under WaveIn dst instead of microphone 
				   src
04/24/97   lss     Check if the device is open before doing any operations
06/04/97   lss     Check each wave[In/Out] device and make sure it's the one
07/10/97   lss     Scratch the above modification and search via each mixer
				   devices

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"
#include "memory.h"

// PROJECT INCLUDES
#include "lscommon.h"

// LOCAL INCLUDES
#include "LsMixer.h"

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// LsMixerControl Implementation
//////////////////////////////////////////////////////////////////////////////

BOOL LsMixerControl::Open(
	UINT	muxId,
	int		ctlType,
	DWORD	flag,
	HWND	hWnd // = NULL
)
{
	BOOL		ret = TRUE;
	MMRESULT	err;
	DWORD		desiredComponentType;
	DWORD		desiredControlType;

	if ( hWnd ) flag |= CALLBACK_WINDOW;

	err = ::mixerOpen( &m_hmx, muxId, DWORD(hWnd), 0, flag );
	if ( MMSYSERR_NOERROR != err ) 
		return FALSE;

	mb_Open = TRUE;

//	find correct dst
	MIXERLINE mxl;
	if ( !initMixerLine( mxl, ctlType, desiredComponentType, desiredControlType )
		|| !openDst( mxl ) )
	 {
		Close();
		return FALSE;
	 }

//	find correct src
	if ( !openSrc( mxl, desiredComponentType ) )
	 {
		Close();
		return FALSE;
	 }

//	find correct control
	if ( !openControl( mxl, desiredControlType ) )
	 {
		initMixerLine( mxl, ctlType, desiredComponentType, desiredControlType );
		openDst( mxl );
		// check the dst controls
		if ( !openControl( mxl, desiredControlType ) )
		 {
			Close();
			return FALSE;
		 }
	 }

	return ret;
}

BOOL LsMixerControl::Close()
{
	if ( !mb_Open ) return TRUE;
	delete mp_details; mp_details = NULL;
	if ( MMSYSERR_NOERROR == ::mixerClose( m_hmx ) );
	 {
		 mb_Open = FALSE;
		 return TRUE;
	 }
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOL LsMixerControl::initMixerLine(
	MIXERLINE&	mxl, 
	int			ctlType,
	DWORD&		desiredComponentType,
	DWORD&		desiredControlType
)
{
	::ZeroMemory( &mxl, sizeof( mxl ) );
	mxl.cbStruct = sizeof ( mxl );
	switch( ctlType )
	 {
		case waveInVolume: // fall through
		case waveInMeter:
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
			desiredComponentType = MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
		 break;
		case waveOutVolume: // fall through
		case waveOutMeter:
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
			desiredComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
		 break;
		default:
			ASSERT(0);
		 return FALSE;
	 }
	switch( ctlType )
	 {
		case waveInVolume: // fall through
		case waveOutVolume:	
			desiredControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
//			desiredControlType = MIXERCONTROL_CT_CLASS_FADER;
		 break;
		case waveInMeter: // fall through
		case waveOutMeter:
			desiredControlType = MIXERCONTROL_CONTROLTYPE_PEAKMETER;
//			desiredControlType = MIXERCONTROL_CT_CLASS_METER;
		 break;
		default:
			ASSERT(0);
		 return FALSE;
	 }
	return TRUE;
}

BOOL LsMixerControl::openDst( MIXERLINE& mxl )
{
	if ( MMSYSERR_NOERROR == 
		::mixerGetLineInfo( HMIXEROBJ(m_hmx), &mxl, MIXER_OBJECTF_HMIXER |
							MIXER_GETLINEINFOF_COMPONENTTYPE ) )
		return TRUE;
	else
		return FALSE;
}

BOOL LsMixerControl::openSrc( MIXERLINE& mxl, DWORD desiredComponentType )
{
	int destination = mxl.dwDestination;
	int connections = mxl.cConnections;

	for( int i=0; i < connections; ++i )
	 {
		::ZeroMemory( &mxl, sizeof( mxl ) );
		mxl.cbStruct = sizeof ( mxl );
		mxl.dwDestination = destination;
		mxl.dwSource = i;
		::mixerGetLineInfo( HMIXEROBJ(m_hmx), &mxl, MIXER_OBJECTF_HMIXER |
							MIXER_GETLINEINFOF_SOURCE );
		if ( desiredComponentType == mxl.dwComponentType )
		 {
			return TRUE;
		 }
	 }

	return FALSE;
}

BOOL LsMixerControl::openControl( MIXERLINE& mxl, DWORD desiredControlType )
{
	MMRESULT	err;
	MIXERLINECONTROLS mxlc;
	BYTE* buf = new BYTE[sizeof(MIXERCONTROL) * mxl.cControls];
	::ZeroMemory( &mxlc, sizeof( mxlc ) );
	::ZeroMemory( buf, sizeof(MIXERCONTROL) * mxl.cControls );
	mxlc.cbStruct  = sizeof( mxlc );
	mxlc.dwLineID  = mxl.dwLineID;
	mxlc.cControls = mxl.cControls;
	mxlc.cbmxctrl  = sizeof( MIXERCONTROL );
	mxlc.pamxctrl  = LPMIXERCONTROL( buf );

	err = ::mixerGetLineControls( HMIXEROBJ(m_hmx), &mxlc, MIXER_OBJECTF_HMIXER |
							MIXER_GETLINECONTROLSF_ALL);
	if ( MMSYSERR_NOERROR != err )
	 {
		delete buf;
		return FALSE;
	 }

	LPMIXERCONTROL pmxc = LPMIXERCONTROL( buf );
	int foundCtlIndex = -1;
	for( int i=0; i < mxlc.cControls; ++i )
	 {	
		if ( desiredControlType == pmxc[i].dwControlType )
		 {
			foundCtlIndex = i;
			break;			
		 }
	 }

	if ( -1 == foundCtlIndex )
	 {
		delete buf;
		return FALSE;
	 }
//
	::ZeroMemory( &m_mxcd, sizeof( m_mxcd ) );
	DWORD type = pmxc[foundCtlIndex].dwControlType & MIXERCONTROL_CT_UNITS_MASK;
	int size = 0;
	switch( type )
	{
		case MIXERCONTROL_CT_UNITS_SIGNED:
		case MIXERCONTROL_CT_UNITS_DECIBELS:
			size = sizeof( MIXERCONTROLDETAILS_SIGNED );
		 break;
		case MIXERCONTROL_CT_UNITS_UNSIGNED:
		case MIXERCONTROL_CT_UNITS_PERCENT:
			size = sizeof( MIXERCONTROLDETAILS_UNSIGNED );
		 break;
		default:
			ASSERT(0);
		 break;
	}

	mp_details			  = new BYTE[size];
	m_mxcd.cbStruct       = sizeof( m_mxcd );
	m_mxcd.dwControlID    = pmxc[foundCtlIndex].dwControlID;
	m_mxcd.cChannels      = 1; // want uniform
	m_mxcd.cMultipleItems = 0;
	m_mxcd.cbDetails      = size;
	m_mxcd.paDetails      = mp_details;
	
	delete buf;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// LsMixerVolume Implementation
//////////////////////////////////////////////////////////////////////////////

BOOL LsMixerVolume::Open( HWAVEIN hWaveIn, HWND hWnd )
{
	return LsMixerControl::Open(
		UINT(hWaveIn), waveInVolume, MIXER_OBJECTF_HWAVEIN, hWnd );
}

BOOL LsMixerVolume::Open( HWAVEOUT hWaveOut, HWND hWnd )
{
	return LsMixerControl::Open(
		UINT(hWaveOut), waveOutVolume, MIXER_OBJECTF_HWAVEOUT, hWnd );
}

BOOL LsMixerVolume::Open( int ctlType, HWND hWnd )
{
	int maxMixer = 0;
	DWORD flag;

	if ( waveIn == ctlType )
	 {
		ctlType = waveInVolume;
	 }
	else if ( waveOut == ctlType )
	 {
		ctlType = waveOutVolume;
	 }
	else return FALSE;
	
	flag = MIXER_OBJECTF_MIXER;
	maxMixer = mixerGetNumDevs();

	if ( maxMixer < 1 ) return FALSE;
 
	BOOL ret;
	int i = 0;
	do
	 {
		ret = LsMixerControl::Open( i++, ctlType, flag, hWnd );

	 } while ( !ret &&  (i < maxMixer) );

	return ret;
}

DWORD LsMixerVolume::Volume()
{
	if ( !mb_Open ) return 0;
	::mixerGetControlDetails( HMIXEROBJ(m_hmx), &m_mxcd, MIXER_OBJECTF_HMIXER |
						MIXER_GETCONTROLDETAILSF_VALUE );

	return 99.0 * ((MIXERCONTROLDETAILS_UNSIGNED*)mp_details)->dwValue / 0xFFFF;
}

void LsMixerVolume::Volume( DWORD volume  )
{
	if ( !mb_Open ) return;
	LS_RANGE( volume, 0, 99 );
	volume = volume / 99.0 * 0xFFFF;

	((MIXERCONTROLDETAILS_UNSIGNED*)mp_details)->dwValue = volume;
	::mixerSetControlDetails( HMIXEROBJ(m_hmx), &m_mxcd, MIXER_OBJECTF_HMIXER |
						MIXER_SETCONTROLDETAILSF_VALUE );
}

//////////////////////////////////////////////////////////////////////////////
// LsMixerVolumeMeter Implementation
//////////////////////////////////////////////////////////////////////////////

BOOL LsMixerVolumeMeter::Open( HWAVEIN hWaveIn, HWND hWnd )
{
	return LsMixerControl::Open( UINT(hWaveIn), waveInMeter,
								MIXER_OBJECTF_HWAVEIN, hWnd );
}

BOOL LsMixerVolumeMeter::Open( HWAVEOUT hWaveOut, HWND hWnd )
{
	return LsMixerControl::Open( UINT(hWaveOut), waveOutMeter,
								MIXER_OBJECTF_HWAVEOUT, hWnd );
}

BOOL LsMixerVolumeMeter::Open( int ctlType, HWND hWnd )
{
	int maxMixer = 0;
	DWORD flag;

	if ( waveIn == ctlType )
	 {
		ctlType = waveInMeter;
	 }
	else if ( waveOut == ctlType )
	 {
		ctlType = waveOutMeter;
	 }
	else return FALSE;
	
	flag = MIXER_OBJECTF_MIXER;
	maxMixer = mixerGetNumDevs();

	if ( maxMixer < 1 ) return FALSE;
 
	BOOL ret;
	int i = 0;
	do
	 {
		ret = LsMixerControl::Open( i++, ctlType, flag, hWnd );

	 } while ( !ret &&  (i < maxMixer) );

	return ret;
}

long LsMixerVolumeMeter::Meter()
{
	if ( !mb_Open ) return 0;
	::mixerGetControlDetails( HMIXEROBJ(m_hmx), &m_mxcd, MIXER_OBJECTF_HMIXER|
						MIXER_GETCONTROLDETAILSF_VALUE );

	long meter = 99.0 * 
				((MIXERCONTROLDETAILS_SIGNED*)mp_details)->lValue / 0x8000;
	return ( meter < 0 ? -meter : meter );
}
