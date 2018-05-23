/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsConvertUnit - 

FILE:		LsConvertUnit.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/12/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef _LSCONVERTUNIT_H_
#define _LSCONVERTUNIT_H_

class LsConvertUnit
{
public:
//	LIFECYCLE
	LsConvertUnit();
	LsConvertUnit( WAVEFORMATEX& wvFmt );
	LsConvertUnit( t_qpl_codec_info& cdcFmt );
	~LsConvertUnit();
	// Copy ctor or assignment operator not allowed

//	OPERATIONS
	unsigned long Convert( int destType, unsigned long srcNum, int srcType );

//	ACCESS
	void SetInfo( WAVEFORMATEX& wvFmt );
	void SetInfo( int bps, int sps );
	void SetInfo( t_qpl_codec_info& cdcFmt );
	void SetInfo( int bpp, int sps, int spb ); 

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	unsigned long ConvertWave( 
		int destType,
		unsigned long srcNum,
		int srcType );

	unsigned long ConvertCodec(
		int destType,
		unsigned long srcNum,
		int srcType );

//	ENUM
	enum{
		NONE,
		WAVE,
		CODEC
	};

//	ATTRIBUTES
	int		me_mode;	// see above enum

	int		mi_bps;		// bytes per sample
	int		mi_bpp;		// bytes per packet		(CODEC ONLY)
	int		mi_sps;		// samples per second
	int		mi_spb;		// samples per block	(CODEC ONLY)

// Do not allow copy ctor or assignment
	LsConvertUnit( const LsConvertUnit& );
	LsConvertUnit& operator=( const LsConvertUnit& );
};

#endif

