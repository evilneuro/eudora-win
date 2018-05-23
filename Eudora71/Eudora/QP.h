// QP.h
//
// Classes for doing quoted-printable encoding and decoding

#ifndef _QP_H_
#define _QP_H_

#include "Encode.h"
#include "Decode.h"

typedef enum
{
	qpNormal,
	qpEqual,
	qpByte1
} QPStates;

class QPDecoder : public Decoder
{
public:
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init() { m_State = qpNormal; return (0); }

protected:
	QPStates	m_State;
	char		m_LastChar;
};


#define ENCODE_NONE			0x0000
#define ENCODE_QP			0x0001
//#define ENCODE_RICH			0x0002
#define ENCODE_CURLYQUOTES	0x0004

class QPEncoder : public Encoder
{
public:
	int Encode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init(char* Out, LONG& OutLen, int flag);
	int Done(char* Out, LONG& OutLen);

protected:
	short	m_BytesOnLine;
	int		m_Flag;
	BOOL	m_bCR;

	void RoomFor(int x, char*& out)
	{
		if (m_BytesOnLine + x > 76)
			{ *out++ = '='; *out++ = '\r'; *out++ ='\n' ; m_BytesOnLine = 0; }
	}
};

#endif // #ifndef _QP_H_
