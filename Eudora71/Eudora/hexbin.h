// HEXBIN.H
//
// all of the defines etc required to make hexbin work
//

#ifndef _HEXBIN_H_
#define _HEXBIN_H_

#include "Decode.h"

class JJFile;

typedef enum 
{
	NotBanner, NotHex, CollectName, CollectInfo, DataWrite, DataCrc1, DataCrc2,
	RzWrite, RzCrc1, RzCrc2, HexDone
} HexBinStates;


class HexBinHead
{
public:
	char m_Type[4];
	char m_Author[4];
	short int m_Flags;
	long m_DataLength;
	long m_RzLength;
	unsigned short m_HCrc;
};

class DecodeBH : public Decoder
{
public:
	DecodeBH(LPCTSTR AltFilename = NULL);
	~DecodeBH();
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init();

private:
	HexBinStates m_State;
	char* m_Outp;
	long m_OSpot;
	long m_BSpot;
	char m_Name[40];
	BYTE m_LastData;
	BYTE m_State68;
	BYTE m_B8;
	BYTE m_RunCount;
	BOOL m_Run;
	BOOL m_Translate;
	long m_Count;
	unsigned long m_CalcCrc;
	unsigned long m_Crc;
	union
	{
		HexBinHead m_BxHead;
		BYTE m_BxhBytes[sizeof(HexBinHead)];
	};
	// the above struct HexBinHead pads so we need an array 
	// not unioned with a struct
	BYTE m_BxH[20];

	int m_ResultCode;
	BOOL m_DecodedHeader;
	long m_TotBytes;
	char m_Banner[64];
	long m_lBannerStart;
	JJFile* m_ATFile;
	TCHAR m_AltFilename[MAX_PATH + 1];

	void HexBinInputChar(BYTE ch);
	short int HexBinDecode(short int);
	void HexBinDataChar(short x);
	void CompCrc(unsigned short ch);
	void CrcError();
	void AbortHexBin();

	// This is the function that stuffs the sturct 
	void SetHeadFields();
};

#endif // #ifndef _HEXBIN_H_
