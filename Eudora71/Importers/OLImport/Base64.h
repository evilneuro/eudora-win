// Base64.h
//
// Classes for doing Base64 encoding and decoding

#ifndef _BASE64_H_
#define _BASE64_H_


class Base64Encoder
{
public:
	int Encode(const char* In, LONG InLen, char*Out, LONG& OutLen);
	int Init(char* Out, LONG& OutLen, int flag);
	int Done(char* Out, LONG& OutLen);

private:
	char m_Partial[4];
	int m_PartialCount;
	int m_BytesOnLine;
};


class Base64Decoder
{
public:
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init();

private:
	int m_DecoderState;		// which of 4 bytes are we seeing now?
	long m_PadCount;		// how many pad chars found so far?
	char m_Partial;			// partially decoded byte from/for last/next time
};

#endif // #ifndef _BASE64_H_
