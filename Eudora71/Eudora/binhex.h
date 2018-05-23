// BINHEX.H
// 
#ifndef _BINHEX_H_
#define _BINHEX_H_

class Network;
class MIMEMap;

class CBinHexMT
{
public:
	CBinHexMT(Network *conn) : m_Connection(conn) {}
	int SendBinHex(const char* Name, MIMEMap& mm, long NeedTrans, long TotalBytes, const char* ContentID, BOOL IsInline);

	int EncodeDataChar(BYTE ch, char* toSpot);
	void CompCrcOut(unsigned short ch);

	void LCODE(BYTE ch)
		{ codedSpot += EncodeDataChar(ch, codedBuffer + codedSpot); }
	
	void CODE(BYTE ch)
		{ LCODE(ch); if (ch == 0x90) LCODE(0); CompCrcOut(ch); }

protected:	
	unsigned long CalcCrc;
	BYTE State86;
	BYTE SavedBits;
	BYTE LineLength;

	char	codedBuffer[512];
	int		codedSpot;

	Network* m_Connection;
};

#endif // #ifndef _BINHEX_H_
