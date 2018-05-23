/*////////////////////////////////////////////////////////////////////////////

NAME:
	 CACodec - An abstract class for Codec

FILE:		CACODEC.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

	A wrapper for IQPLCodec

FILES:
	QPL.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef _CACODEC_H_
#define _CACODEC_H_

// PROJECT INCLUDES
#ifndef _QPL_H_
#include "QPL.H"				// For type stuff
#endif

class CACodec
{
public:
//	LIFECYCLE
	CACodec( IQPLCodec* pCodec ) : mp_Codec(pCodec)
			{if (mp_Codec) 
				{ mp_Codec->AddRef(); mp_Codec->Info(&m_Info,NULL);}}				
	virtual ~CACodec()
			{if (mp_Codec) mp_Codec->Release();}
	// Copy ctor or assignment not allowed

//	OPERATIONS
	virtual BOOL	Reset( t_qpl_codec_option *pOption )
						{return (mp_Codec->Reset( pOption )== S_OK);}
	
	virtual long	Encode( 
						BYTE *destBuf,
						const BYTE *srcBuf, 
						int nSamples,
						t_qpl_codec_option *pOption )
						{return mp_Codec->Encode(
							destBuf, srcBuf, nSamples, pOption );};
	virtual long	Decode(
						BYTE *destBuf, 
						const BYTE *srcBuf,
						int nPackets,
						t_qpl_codec_option *pOption )
						{return mp_Codec->Decode( 
							destBuf, srcBuf, nPackets, pOption );};
	virtual	void	Info( t_qpl_codec_info	*pInfo )
						{if ( !pInfo ) return;
						memcpy ( pInfo, &m_Info, sizeof( t_qpl_codec_info ) );}

	const t_qpl_codec_info* Info() {return &m_Info;}

private:
//	ATTRIBUTE
	IQPLCodec*		mp_Codec;
	t_qpl_codec_info	m_Info;

// Do not allow default ctor, copy ctor or assignment operator
	CACodec();
	CACodec( const CACodec& );
	CACodec& operator=( const CACodec& );
};

#endif
