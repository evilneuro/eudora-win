/*////////////////////////////////////////////////////////////////////////////

NAME:
	 QPL - Qualcomm PureVoice Library Header File

FILE:		QPL.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C-INTERFACE

DESCRIPTION:

	This header file contains the necessary struct and COM interface
	information

FILES:
	WINDOWS.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/09/97   lss     -Initial, currently contains only codec object
04/22/97   lss     -Added variableRate to t_qpl_codec_option

/*////////////////////////////////////////////////////////////////////////////

#ifndef _QPL_H_
#define _QPL_H_

// SYSTEMS INCLUDES
#include <windows.h>						
#include <ole2.h>

// DEFINES
#define INT8	char
#define UINT8	unsigned char
#define INT16	short
#define UINT16	unsigned short
#define INT32	long
#define UINT32	unsigned long

#define STDCALL	__stdcall

#define NUM_OF_CODECS	2	// currently we have two codecs

inline void SetGUID(GUID& rguidDest, const GUID& rguidSrc)
{
    memcpy(&rguidDest, &rguidSrc, sizeof(GUID));
}

DEFINE_GUID(IID_NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
//	INT16	variableRate;	// in  : 0 = fixed, 1 = variable
	INT16	Playback;		// in  : 0 = normal, 1,2,3 = fast, faster, fastest
							//       -1,-2,-3 = slow, slower, slowest
	UINT32	NumOfSamples;	// out : # of samples outputted
	INT16	bAgc;			// in  : enable/disable agc
	INT16	AgcStep;		// out : up/down record gain decision
} t_qpl_codec_option;

// our codec variable rate info struct
typedef struct
{
//	UINT32	isVariableRate;	// is this file variable length
//	UINT32	sizeInPackets;  // what's the file (data chunk) size in packets?
	UINT32	numOfRates;		// # of entries for below
	// hi byte = rate index, lo byte = size of following packet
	UINT16	bytesPerPacket[8];
} t_qpl_variable_rate;

// our codec info struct (header format)
typedef struct
{
	IID		Id;				// unique ID of this codec
    UINT16	Version;		// version # of the codec
	BYTE    Name[80];		// proper name of the codec
    UINT16	avgBitsPerSec;	// avg bps of the codec
	UINT16	bytesPerPacket;	// size of a packet in bytes (max rate)
							// including header byte for that packet
	UINT16	samplesPerBlock;// size of a block in samples
							// encoder encodes one block into one packet
							// decoder decodes one packet into one block
    UINT16	samplesPerSec;	// sps of input needed for the encoder
							// and output from the decoder
    UINT16	bitsPerSample;  // bits per sample of the above

	// contains rate header format info
	t_qpl_variable_rate	variableRate;

    UINT32	reserved[5];	// reserved
} t_qpl_codec_info;

// this struct contains name and description for use in GUI display
typedef struct
{
	char    menuName[80];		// name the user sees (use layman's term)
	char    Description[256];	// description of this codec 
} t_qpl_codec_gui_info;

// Entry Point
HRESULT WINAPI QPLCreateInstance( REFIID riid, void** ppvObj );

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Codec Ids for QCELP13k 
// {5E7F6D41-B115-11d0-BA91-00805FB4B97E}
DEFINE_GUID(CID_Qcelp13k, 
0x5e7f6d41, 0xb115, 0x11d0, 0xba, 0x91, 0x0, 0x80, 0x5f, 0xb4, 0xb9, 0x7e);

//////////////////////////////////////////////////////////////////////////////
// Codec Ids for QCELP13k with Reduced Rate Mode = 3
// {5E7F6D42-B115-11d0-BA91-00805FB4B97E}
DEFINE_GUID(CID_Qcelp13kMinor, 
0x5e7f6d42, 0xb115, 0x11d0, 0xba, 0x91, 0x0, 0x80, 0x5f, 0xb4, 0xb9, 0x7e);


////////////////////////////////////////////////////////////////////////////// 
//  INTERFACE:
//
//		IUnknown
//
//  PURPOSE:
//
//		Base class of all interfaces. Defines life time management and
//		support for dynamic cast.
//
//  IID_IUnknown:
//
//		{00000000-0000-0000-C000000000000046}
//

DEFINE_GUID(IID_IUnknown,
0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#ifndef _WIN32
class IUnknown
{
public:
    virtual ULONG STDCALL	AddRef() = 0;
    virtual ULONG STDCALL	Release() = 0;
    virtual HRESULT STDCALL	QueryInterface( REFIID riid, void** ppvObj ) = 0;
};
#endif

////////////////////////////////////////////////////////////////////////////// 
//  INTERFACE:
//
//		IQPLCodecFactory
//
//  PURPOSE:
//
//		Base class for Codec Plugin. 
//
//  IID_IQPLGetCodec:
//
//		{8EB95DD8-B10E-11d0-BA91-00805FB4B97E}
DEFINE_GUID(IID_IQPLGetCodec, 
0x8eb95dd8, 0xb10e, 0x11d0, 0xba, 0x91, 0x0, 0x80, 0x5f, 0xb4, 0xb9, 0x7e);

class IQPLGetCodec : public IUnknown
{
public:
//	IUnknown Interface
    ULONG STDCALL	AddRef() = 0;
    ULONG STDCALL	Release() = 0;
    HRESULT STDCALL	QueryInterface( REFIID iid, void** ppvObj ) = 0;

	virtual UINT32	STDCALL NumOfCodecs() = 0;
	virtual HRESULT	STDCALL CreateCodec( UINT32 nIndex,		// in
										 void** ppvObj		// out
										) = 0;
	virtual HRESULT STDCALL CreateCodecById( REFGUID rcid,	// in
											void** ppvObj	// out
										) = 0;
};

////////////////////////////////////////////////////////////////////////////// 
//  INTERFACE:
//
//		IQPLCodec
//
//  PURPOSE:
//
//		Base class for all codec object. 
//
//  IID_IQPLCodec:
//
//		{8EB95DCA-B10E-11d0-BA91-00805FB4B97E}

DEFINE_GUID(IID_IQPLCodec, 
0x8eb95dca, 0xb10e, 0x11d0, 0xba, 0x91, 0x0, 0x80, 0x5f, 0xb4, 0xb9, 0x7e);

class IQPLCodec : public IUnknown
{
public:
//	IUnknown Interface
    ULONG STDCALL	AddRef() = 0;
    ULONG STDCALL	Release() = 0;
    HRESULT STDCALL	QueryInterface( REFIID riid, void** ppvObj ) = 0;

	virtual HRESULT STDCALL	Info( t_qpl_codec_info* pInfo,			// out
								  t_qpl_codec_gui_info* pMiscInfo	// out
							) = 0;
	virtual HRESULT STDCALL	Reset( t_qpl_codec_option* pOption ) = 0;
	virtual UINT32 STDCALL	Encode( 
								BYTE* pDestBuf,				// out
								const BYTE* pSrcBuf,		// in
								UINT32 nBlocks,				// in
								t_qpl_codec_option* pOption	// in-out
							) = 0;
	virtual UINT32 STDCALL	Decode( 
								BYTE* pDestBuf,				// out
								const BYTE* pSrcBuf,		// in
								UINT32 nPackets,			// in
								t_qpl_codec_option* pOption	// in-out
							) = 0;
};

#endif