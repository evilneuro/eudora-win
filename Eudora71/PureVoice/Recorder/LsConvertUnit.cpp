/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsConvertUnit - 

FILE:		LsConvertUnit.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

DEPENDENCIES:


FILES:


REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/12/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "QPL.h"

// LOCAL INCLUDES
#include "LsConvertUnit.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

LsConvertUnit::LsConvertUnit() :
	mi_bps(0),	mi_sps(0),	mi_bpp(0),	mi_spb(0), me_mode(NONE)
{
}

LsConvertUnit::LsConvertUnit( WAVEFORMATEX& wvFmt )
{
	LsConvertUnit();

	me_mode = WAVE;

	mi_bps = wvFmt.nBlockAlign;
	mi_sps = wvFmt.nSamplesPerSec;
}

LsConvertUnit::LsConvertUnit( t_qpl_codec_info& cdcFmt ) 
{
	LsConvertUnit();
	
	me_mode = CODEC;

	mi_bpp = cdcFmt.bytesPerPacket;
	mi_sps = cdcFmt.samplesPerSec;
	mi_spb = cdcFmt.samplesPerBlock;
}

LsConvertUnit::~LsConvertUnit()
{
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

unsigned long LsConvertUnit::Convert(
	int				destType, 
	unsigned long	srcNum, 
	int				srcType )
{
	ASSERT(me_mode != NONE);
	if ( WAVE == me_mode )
		return ConvertWave( destType, srcNum, srcType );
	else if ( CODEC == me_mode )
		return ConvertCodec( destType, srcNum, srcType );
	
	return srcNum;
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

void LsConvertUnit::SetInfo( WAVEFORMATEX& wvFmt )
{
	me_mode = WAVE;

	mi_bps = wvFmt.nBlockAlign;
	mi_sps = wvFmt.nSamplesPerSec;
}

void LsConvertUnit::SetInfo( int bps, int sps )
{
	me_mode = WAVE;

	mi_bps = bps;
	mi_sps = sps;
}

void LsConvertUnit::SetInfo( t_qpl_codec_info& cdcFmt ) 
{
	me_mode = CODEC;

	mi_bpp = cdcFmt.bytesPerPacket;
	mi_sps = cdcFmt.samplesPerSec;
	mi_spb = cdcFmt.samplesPerBlock;
}

void LsConvertUnit::SetInfo( int bpp, int sps, int spb ) 
{
	me_mode = CODEC;

	mi_bpp = bpp;
	mi_sps = sps;
	mi_spb = spb;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

unsigned long LsConvertUnit::ConvertWave(
	int				destType, 
	unsigned long	srcNum, 
	int				srcType )
{
	if ( destType == srcType )
		return srcNum;

	if ( destType == UNIT_TIME_MS )
	 {
		if ( srcType == UNIT_SAMPLES )
			return (unsigned long)((float)srcNum * 1000 / mi_sps);
		if ( srcType == UNIT_BYTES )
			return (unsigned long)((float)srcNum * 1000 / (mi_bps * mi_sps) );
	 }
	if ( destType == UNIT_SAMPLES )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (unsigned long)((float)srcNum * mi_sps / 1000);
		if ( srcType == UNIT_BYTES )
			return (unsigned long)((float)srcNum / mi_bps );
	 }
	if ( destType == UNIT_BYTES )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (unsigned long)((float)srcNum * mi_sps * mi_bps / 1000);
		if ( srcType == UNIT_SAMPLES )
			return (unsigned long)((float)srcNum * mi_bps );
	 }
	
	return srcNum; // to satify "not all control paths return a value"
}

unsigned long LsConvertUnit::ConvertCodec(
	int				destType, 
	unsigned long	srcNum, 
	int				srcType )
{
	// first make sure the number we want to convert is
	// aligned correctly. this is done by rounding up
	// to the next atomic number
	unsigned long min;
	switch ( srcType )
	 {
		case UNIT_TIME_MS:
			min = mi_spb * 1000 / mi_sps;
		  break;
		case UNIT_BYTES:
			min = mi_bpp;
		 break;
		case UNIT_SAMPLES:
			min = mi_spb;
		 break;
		case UNIT_PACKETS:
			min = 1;
		break;
		default:
			return srcNum;
	 }
	if ( srcNum % min ) 
	 {
		srcNum += min;
		srcNum /= min;
		srcNum *= min;
	 }
	if ( destType == srcType )
		return srcNum;

	if ( destType == UNIT_TIME_MS )
	 {
		if ( srcType == UNIT_SAMPLES )
			return (unsigned long)((float)srcNum * 1000 / mi_sps);
		if ( srcType == UNIT_PACKETS )
			return (unsigned long)((float)srcNum * 1000 * mi_spb / mi_sps );
		if ( srcType == UNIT_BYTES )
			return (unsigned long)((float)srcNum * 1000 * mi_spb / (mi_bpp * mi_sps));
	 }
	if ( destType == UNIT_SAMPLES )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (unsigned long)((float)srcNum * mi_sps / 1000);
		if ( srcType == UNIT_PACKETS )
			return (unsigned long)((float)srcNum * mi_spb);
		if ( srcType == UNIT_BYTES )
			return (unsigned long)((float)mi_spb / mi_bpp * srcNum);
	 }
	if ( destType == UNIT_PACKETS )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (unsigned long)((float)srcNum * mi_sps / (1000 * mi_spb));
		if ( srcType == UNIT_SAMPLES )
			return (unsigned long)((float)srcNum / mi_spb);
		if ( srcType == UNIT_BYTES )
			return (unsigned long)((float)srcNum / mi_bpp);
	 }
	if ( destType == UNIT_BYTES )
	 {
		if ( srcType == UNIT_TIME_MS )
			return (unsigned long)((float)mi_bpp * srcNum * mi_sps / (1000 * mi_spb));
		if ( srcType == UNIT_SAMPLES )
			return (unsigned long)((float)mi_bpp * srcNum / mi_spb);
		if ( srcType == UNIT_PACKETS )
			return (unsigned long)((float)srcNum * mi_bpp);
	 }	
	return srcNum; // to satify "not all control paths return a value"
}
