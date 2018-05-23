// UUDECODE.H
//
// Routines for decoding uuencoded data

#ifndef _UUDECODE_H_
#define _UUDECODE_H_

#include "Encode.h"
#include "Decode.h"

// Forward declarations
class JJFile;
class MIMEState;


// Data Types

typedef enum
{
	NotAb, AbWrite, AbDone
} AbStates;

class DecodeUU : public Decoder
{
public:
	DecodeUU(MIMEState* ms = NULL);
	~DecodeUU();
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init();

protected:
	AbStates	m_State;
	long		m_lOrigOffset;
	JJFile*		m_ATFile;
	MIMEState*	m_ms;
	char		m_Name[255];

	BOOL IsAbLine(const char* text, LONG size);
	int UURightLength(const char* text, LONG size, BOOL Strict = TRUE);
	AbStates UULine(const char* text, LONG size);
};

class EncodeUU : public Encoder
{
public:
	EncodeUU();
	~EncodeUU();
	int Init(char* Out, LONG& OutLen, int flag);
	int Encode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Done(char* Out, LONG& OutLen);

protected:
	char	m_buf[80];
	char*	m_OutBuf;
	int		m_ByteNum;
	BYTE	m_LastByte;
};

#endif // #ifndef _UUDECODE_H_
