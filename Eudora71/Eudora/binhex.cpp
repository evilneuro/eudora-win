// BINHEX.CPP
//
// Contains the routines to binhex a file.
//

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "MIMEMap.h"
#include "binhex.h"
#include "QCWorkerSocket.h"
#include "guiutils.h"
#include "progress.h"
#include "QCUtils.h"
#include "fileutil.h"


#include "DebugNewHelpers.h"


// 6 to 8 bit conversion table
static const BYTE BinHexTable[64] =
{
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x30, 0x31, 0x32,
	0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x40, 0x41,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52,
	0x53, 0x54, 0x55, 0x56, 0x58, 0x59, 0x5a, 0x5b,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x68,
	0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x70, 0x71, 0x72
};


inline unsigned long LongToMac(unsigned long x)
{
	return ((unsigned long) ::SwapLongMT(x));
}

inline unsigned short WordToMac(unsigned short x)
{
	return ((unsigned short) ::SwapShortMT(x));
}



// EncodeDataChar
// Encodes an 8-bit data character into a 6-bit buffer
// returns the number of valid encoded characters generated
//
int CBinHexMT::EncodeDataChar(BYTE ch, char* toSpot)
{
	char* spotWas = toSpot;

	switch(State86++)
	{
	case 0:
		*toSpot++ = BinHexTable[(ch>>2) & 0x3f];
		SavedBits = (BYTE)((ch & 0x3) << 4);
		if (++LineLength == 64)
		{
			LineLength = 0;
			*toSpot++ = '\r';
			*toSpot++ = '\n';
		}
		break;
	case 1:
		*toSpot++ = BinHexTable[SavedBits | ((ch>>4) &0xf)];
		SavedBits = (BYTE)((ch & 0xf) << 2);
		if (++LineLength == 64)
		{
			LineLength = 0;
			*toSpot++ = '\r';
			*toSpot++ = '\n';
		}
		break;
	case 2:
		*toSpot++ = BinHexTable[SavedBits | ((ch>>6)&0x3)];
		if (++LineLength == 64)
		{	
			LineLength = 0;
			*toSpot++ = '\r';
			*toSpot++ = '\n';
		}
		*toSpot++ = BinHexTable[ch & 0x3f];
		if (++LineLength == 64)
		{	
			LineLength = 0;
			*toSpot++ = '\r';
			*toSpot++ = '\n';
		}
		State86 = 0;
		break;
	}

	return (toSpot - spotWas);
}


// CompCrcOut
//
// Compute CRC
//
const unsigned short BYTEMASK	= 0x00FF;
const unsigned short BYTEBIT	= 0x0100;
const unsigned long WORDMASK	= 0x0000FFFF;
const unsigned long WORDBIT		= 0x00010000;
const unsigned long CRCCONSTANT	= 0x00001021;

void CBinHexMT::CompCrcOut(unsigned short ch)
{
	unsigned long temp = CalcCrc;
	int x;

	for (x = 8; x ; x--)
	{
		ch <<= 1;
		if ((temp <<= 1) & WORDBIT)
			temp = (temp & WORDMASK) ^ CRCCONSTANT;
		temp ^= (ch >> 8);
		ch &= BYTEMASK;
	}
	CalcCrc = temp;
}


// SendBinHex
// Drives the binhex process.
//
int CBinHexMT::SendBinHex(const char* Name, MIMEMap& mm, long NeedTrans, long TotalSize, const char* ContentID, BOOL IsInline)
{
	char FileBuf[2048];
	JJFile InFile(FileBuf, sizeof(FileBuf));
	unsigned short tempCrc;
	char TransName[32];
	//char cbuf[512];
	BYTE* ptr;
	long lNumBytesRead = 0;
	BYTE x;

	//codedBuffer = cbuf;
	memset(codedBuffer, 0, sizeof(codedBuffer));

	LineLength = 1;
	State86 = 0;
	CalcCrc = 0;
	codedSpot = 0;

	// Open the file to be processed
	if (FAILED(InFile.Open(Name, O_RDONLY)))
	{
		ErrorDialog(IDS_ERR_OPEN_ATTACH, Name);
    	return (-1);
	}

	// Content-Type: application/binhex
	LPCTSTR Filename = strrchr(Name, '\\') + 1;
	sprintf(codedBuffer, "%s %s/%s; %s=\"%s\"",
		(const char*)CRString(IDS_MIME_HEADER_CONTENT_TYPE),
		(const char*)CRString(IDS_MIME_APPLICATION),
		(const char*)CRString(IDS_MIME_BINHEX),
		(const char*)CRString(IDS_MIME_NAME),
		Filename);
	if (m_Connection->PutLine(codedBuffer) < 0) return (-1);
	
	// Content-ID: header (if there is one)
	if (ContentID && *ContentID)
	{
		sprintf(codedBuffer, "%s <%s>",
			(const char*)CRString(IDS_MIME_HEADER_CONTENT_ID),
			ContentID);
		if (m_Connection->PutLine(codedBuffer) < 0) return (-1);
	}

	// Content-Disposition: header
	CRString ContentDispType(IsInline? IDS_MIME_CONTENT_DISP_INLINE : IDS_MIME_CONTENT_DISP_ATTACH);
	sprintf(codedBuffer, "%s %s; %s=\"%s\"",
		(const char*)CRString(IDS_MIME_HEADER_CONTENT_DISP),
		(const char*)ContentDispType,
		(const char*)CRString(IDS_MIME_CONTENT_DISP_FILENAME),
		Filename);
	if (m_Connection->PutLine(codedBuffer) < 0) return (-1);
	
	// Blank line between MIME headers and binhex stuff
	if (m_Connection->PutLine() < 0) return (-1);

	// Send the binhex banner
	if (m_Connection->PutLine() < 0) return (-1);
	if (m_Connection->Put(CRString(IDS_BINHEX_BANNER)) < 0) return (-1);

	// The first data to be coded is the length of the filename
	strncpy(TransName, Filename, 31);
	TransName[31] = 0;
	x = (BYTE)strlen(TransName);
	CODE(x);

	// Encode the filename we are hexing
	for (ptr = (BYTE*)TransName; *ptr; ptr++)
		CODE(*ptr);

	// Creator and type values
	CODE(0);
	for (x = 0, ptr = (BYTE*)mm.m_Type; x < 4; x++, ptr += (*ptr? 1 : 0))
		CODE(*ptr);
	for (x = 0, ptr = (BYTE*)mm.m_Creator; x < 4; x++, ptr += (*ptr? 1 : 0))
		CODE(*ptr);
	CODE(0);	// Flags
	CODE(0);

	// Data fork size
	TotalSize = LongToMac(TotalSize);
	for (x = 0, ptr = (BYTE *)&TotalSize; x < 4; x++, ptr++)
		CODE(*ptr);

	// Resource fork size
	for (x = 0; x < 4; x++)
		CODE(0);

	CompCrcOut(0);
	CompCrcOut(0);
	tempCrc = (unsigned short)(CalcCrc & 0xFFFF);
	tempCrc = WordToMac(tempCrc);
	for (x = 0, ptr = (BYTE*)&tempCrc; x < 2; x++, ptr++)
		CODE(*ptr);
	CalcCrc = 0;
	
	// Send what we have so far....that is codedSpot bytes contained in
	// the coding buffer
	if (m_Connection->Put(codedBuffer, codedSpot) < 0) return (-1);

	// Read out of the file and encode the actual data now..
	BYTE LastChar = 0;
	codedSpot = 0;
	while (SUCCEEDED(InFile.GetNextBlock(&lNumBytesRead)) && (lNumBytesRead > 0))
	{
		for (BYTE *b = (BYTE *)FileBuf; lNumBytesRead; lNumBytesRead--, LastChar = *b, b++)
		{
			BYTE ch = *b;

			// If this is a text file, we may need to do some end-of-line
			// convention translation
			if (NeedTrans && ch == '\n')
			{
				// If we saw a CR then a LF, then just skip the LF
				if (LastChar == '\r')
					continue;

				// If it was just a LF, then translate to a CR
				ch = '\r';
			}

			CODE(ch);
			if (codedSpot > sizeof(codedBuffer) - 6)
			{
				if (m_Connection->Put(codedBuffer, codedSpot) < 0) return (-1);
				ProgressAdd((codedSpot * 3) / 4);
				codedSpot = 0;
			}
		}
	}

	// Send last remaining bytes
	if (!lNumBytesRead)
	{
		if (m_Connection->Put(codedBuffer, codedSpot) < 0) return (-1);
	}
	else
	{
		ErrorDialog(IDS_ERR_BINHEX_OPEN_ATTACH, Name);
		return (-1);
	}
	
	// Add some trailing stuff....
	codedSpot = 0;	
	CompCrcOut(0);
	CompCrcOut(0);
	tempCrc = (unsigned short)(CalcCrc & 0xFFFF);
	tempCrc = WordToMac(tempCrc);
	for (ptr = (BYTE*)&tempCrc, x=2; x>0; x--,ptr++)
		CODE(*ptr);
	CalcCrc = 0;

	CompCrcOut(0);
	CompCrcOut(0);
	tempCrc = (unsigned short)(CalcCrc & 0xFFFF);
	tempCrc = WordToMac(tempCrc);
	for (ptr = (BYTE*)&tempCrc, x=2; x>0; x--,ptr++)
		CODE(*ptr);

	if (State86) CODE(0);
	codedBuffer[codedSpot++] = ':';
	codedBuffer[codedSpot++] = '\r';
	codedBuffer[codedSpot++] = '\n';
	if (m_Connection->Put(codedBuffer, codedSpot) < 0) return (-1);

	Progress(1, NULL, 1);

	return (1);
}
