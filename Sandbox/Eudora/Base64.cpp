// Base64.cpp
//
// Classes for doing Base64 encoding and decoding

#include "stdafx.h"

#include "Base64.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


const BYTE SKIP	= 0xFF;
const BYTE FAIL	= 0xFE;
const BYTE PAD	= 0xFD;

static const char g_Encode[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const BYTE g_Decode[] = 
{
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,SKIP,SKIP,FAIL,FAIL,SKIP,FAIL,FAIL,	// 0
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// 1
	SKIP,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,0x3e,FAIL,FAIL,FAIL,0x3f,	// 2
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,FAIL,FAIL,FAIL,PAD ,FAIL,FAIL,	// 3
	FAIL,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,	// 4
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,FAIL,FAIL,FAIL,FAIL,FAIL,	// 5
	FAIL,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,	// 6
	0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,FAIL,FAIL,FAIL,FAIL,FAIL,	// 7
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// 8
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// 9
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// A
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// B
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// C
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// D
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// E
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	// F
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F 
};


// Bit extracting macros
/*inline*/ BYTE Bot2(BYTE b) { return (BYTE)(b & 0x03); }
/*inline*/ BYTE Bot4(BYTE b) { return (BYTE)(b & 0x0F); }
/*inline*/ BYTE Bot6(BYTE b) { return (BYTE)(b & 0x3F); }
/*inline*/ BYTE Top2(BYTE b) { return (BYTE)(b >> 6); }
/*inline*/ BYTE Top4(BYTE b) { return (BYTE)(b >> 4); }
/*inline*/ BYTE Top6(BYTE b) { return (BYTE)(b >> 2); }

void EncodeThree64(const char *In, char*& Out, int& bpl)
{
	if (bpl==76)
	{
		*Out++ = '\r';
		*Out++ = '\n';
		bpl = 0;
	}
	bpl += 4;
	*Out++ = g_Encode[Top6(In[0])];
	*Out++ = g_Encode[Bot2(In[0])<<4 | Top4(In[1])];
	*Out++ = g_Encode[Bot4(In[1])<<2 | Top2(In[2])];
	*Out++ = g_Encode[Bot6(In[2])];
}

// Base64Encoder - convert binary data to base64
//
int Base64Encoder::Init(char* Out, LONG& OutLen, int flag)
{
	return (OutLen = m_PartialCount = m_BytesOnLine = 0);
}

int Base64Encoder::Done(char* Out, LONG& OutLen)
{
	char* OutSpot = Out;	// the spot to which to copy the encoded chars

	// we've been called to cleanup the leftovers
	if (m_PartialCount)
	{
		if (m_PartialCount < 2)
			m_Partial[1] = 0;
		m_Partial[2] = 0;
		EncodeThree64(m_Partial, OutSpot, m_BytesOnLine);

		// now, replace the unneeded bytes with ='s
		OutSpot[-1] = '=';
		if (m_PartialCount == 1)
			OutSpot[-2] = '=';
		OutLen = 4;
	}

	OutLen = OutSpot - Out;

	return (0);
}

int Base64Encoder::Encode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	const char* InSpot = In;	// the byte currently being decoded
	char* OutSpot = Out;		// the spot to which to copy the encoded chars
	
	if (!InLen)
		return (0);

	// do we have any stuff left from last time?
	if (m_PartialCount)
	{
		int needMore = 3 - m_PartialCount;
		if (InLen >= needMore)
		{
			// we can encode some bytes
			memcpy(m_Partial + m_PartialCount, In, needMore);
			InLen -= needMore;
			InSpot += needMore;
			EncodeThree64(m_Partial, OutSpot, m_BytesOnLine);
			m_PartialCount = 0;
		}
		// If we don't have enough bytes to complete the leftovers, we
		// obviously don't have 3 bytes.  So the encoding code will fall
		// through to the point where we copy the leftovers to the partial
		// buffer.  As long as we're careful to append and not copy blindly,
		// we'll be fine.
	}
		
	// We encode the integral multiples of three
	for (; InLen >= 3; InSpot += 3, InLen -= 3)
		EncodeThree64(InSpot, OutSpot, m_BytesOnLine);
		
	// now, copy the leftovers to the partial buffer
	if (InLen)
	{
		memcpy(m_Partial + m_PartialCount, InSpot, InLen);
		m_PartialCount += InLen;
	}

	OutLen = OutSpot - Out;

	return (0);
}



// Base64Decoder - convert base64 data to binary
//
int Base64Decoder::Init()
{
	m_DecoderState = 0;
	m_PadCount = 0L;
	return (0);
}

int Base64Decoder::Decode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	BYTE decode;			// the decoded byte
	char* OutSpot = Out;	// current output character
	int invalCount;			// how many bad chars found this time around?

	// fetch state from caller's buffer
	invalCount = 0;
	
	for (; InLen; In++, InLen--)
	{
		switch(decode = g_Decode[*In])
		{
		case SKIP: break;				// skip whitespace
		case FAIL: invalCount++; break;	// count invalid characters
		case PAD: m_PadCount++; break;	// count pad characters
		default:
			// found a non-pad character, so if we had previously found a
			// pad, that pad was an error
			if (m_PadCount)
			{
				invalCount += (int)m_PadCount;
				m_PadCount=0;
			}
					
			// extract the right bits
			switch (m_DecoderState)
			{
			case 0:
				m_Partial = (char)(decode << 2);
				m_DecoderState++;
				break;
			case 1:
				*OutSpot++ = (char)(m_Partial | Top4(decode));
				m_Partial =  (char)(Bot4(decode) << 4);
				m_DecoderState++;
				break;
			case 2:
				*OutSpot++ = (char)(m_Partial | Top6(decode));
				m_Partial =  (char)(Bot2(decode) << 6);
				m_DecoderState++;
				break;
			case 3:
				*OutSpot++ = (char)(m_Partial | decode);
				m_DecoderState = 0;
				break;
			}
		}
	}

	OutLen = OutSpot - Out;
	
	return (invalCount);
}
