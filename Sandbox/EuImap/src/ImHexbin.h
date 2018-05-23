// ImHexbin.h
//
// all of the defines etc required to make hexbin work
//

// Data Types

#ifndef _IMHEXBIN_H_
#define _IMHEXBIN_H_

#include "Decode.h"

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

class ImapDecodeBH : public Decoder
{
public:
	ImapDecodeBH();
	~ImapDecodeBH();
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init();

	// Access to setting internal directives and data:
	//
	void SetMustOutputFile (BOOL bValue)
		{ m_bMustWriteOutput = bValue; }
	void SetMailboxDirectory (LPCSTR pStr)
		{ m_szMailboxDir = pStr; }

	LPCTSTR GetEmbeddedFilename () { return m_szEmbeddedFilename; }

private:
	HexBinStates m_State;
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
	JJFile* m_ATFile;

	void HexBinInputChar(BYTE ch);
	short int HexBinDecode(short int);
	void HexBinDataChar(short x);
	void CompCrc(unsigned short ch);
	void CrcError();
	void AbortHexBin();

	// This is the function that stuffs the sturct 
	void SetHeadFields();

	JJFileMT* CreateAttachFilefromEmbeddedName ();

	// For embedded binhex handling:
	//
	BOOL m_bMustWriteOutput;

	// Caller must set this directory under which there's the Attach
	// directory.
	//
	CString m_szMailboxDir;

	JJFileMT*	m_pATFile;

	// Added for IMAP:
	// If an embedded filename is found, copy it here:
	//
	CString m_szEmbeddedFilename;

	// Buffer for accululating output strings, and it's size:
	LPSTR			m_pOutBuf;
	static int		m_OutBufsize;

	// Current index into m_OutBuf:
	int m_OutBufIndex;

};

#endif // #ifndef _IMHEXBIN_H_
