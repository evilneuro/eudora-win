// HEXBIN.CPP
//
// Code for the hexbin decoder.
//

#include "stdafx.h"

#include "fileutil.h"
#include "resource.h"
#include "rs.h"
#include "progress.h"
#include "MIMEMap.h"
#include "mime.h"
#include "pop.h"
#include "guiutils.h"
#include "hexbin.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



const int HexBad = 0xFF;
const int HexGood = 0xF0;
const int HexAbort = 0xF5;

const BYTE RUNCHAR = 0x90;
const BYTE DONE = 0x7F;
const BYTE SKIP = 0x7E;
const BYTE FAIL = 0x7D;

static const BYTE HexBinTable[256] =
{
/* 0*/	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
/*					 \n				   \r		*/
		FAIL, FAIL, SKIP, FAIL, FAIL, SKIP, FAIL, FAIL,
/* 2*/	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
/* 4*/	FAIL, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, FAIL, FAIL,
/* 6*/	0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, FAIL,
/*					 :					*/
		0x14, 0x15, DONE, FAIL, FAIL, FAIL, FAIL, FAIL,
/* 8*/	0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
		0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, FAIL,
/*10*/	0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, FAIL,
		0x2C, 0x2D, 0x2E, 0x2F, FAIL, FAIL, FAIL, FAIL,
/*12*/	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, FAIL,
		0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, FAIL, FAIL,
/*14*/	0x3D, 0x3E, 0x3F, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
/*16*/	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
		FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
};

DecodeBH::DecodeBH()
{
	Init();
    *m_Banner = 0;
}

DecodeBH::~DecodeBH()
{
	delete m_ATFile;
}

int DecodeBH::Init()
{
	m_OSpot = m_BSpot = m_LastData = m_State68 = m_B8 = 0;
	m_RunCount = 0;
	m_Run = 0;
	m_Count = 0L;
	m_CalcCrc = m_Crc = 0;
	m_State = NotBanner;
	m_ResultCode = HexGood;
	m_DecodedHeader = FALSE;
	m_TotBytes = 0L;
	m_lBannerStart = -1L;
	m_ATFile = NULL;

	return (0);
}

int DecodeBH::Decode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	// Only compare up to version number
	const int BannerLen = 41;

	if (In != Out)
		memcpy(Out, In, InLen);
	OutLen = InLen;

	if (m_State == NotBanner)
	{
		m_lBannerStart = -1L;
		if (InLen < BannerLen)
			return (FALSE);
		if (!*m_Banner)
			strcpy(m_Banner, CRString(IDS_BINHEX_BANNER));
		if (strncmp(In, m_Banner, BannerLen))
			return (FALSE);
		Init();
		// Remember where we started
		g_pPOPInboxFile->Tell(&m_lBannerStart);
		ASSERT(m_lBannerStart >= 0);
//		OutLen = 0;
		m_State = NotHex;
		return (TRUE);
	}

	m_Outp = Out;

	for (; InLen && m_ResultCode != HexBad && m_ResultCode != HexAbort &&
		m_State != HexDone; In++, InLen--)
	{
		m_TotBytes++;
		HexBinInputChar((BYTE)*In);
	}
	if (m_ResultCode == HexGood && m_DecodedHeader)
		Progress(m_TotBytes);

//	OutLen = 0;

	if (m_State != HexDone)
		return (TRUE);

	// Search for ending colon.  We always do this because either we stopped
	// decoding early, or we got to the end of the data fork and we want to
	// ignore the resource fork.
	if (!strchr(In, ':'))
		return (TRUE);
	/*
	for (; InLen; InLen--)
		if (*In++ == ':')
			break;
	if (!InLen)
		while (NetConnection->GetCh(&ch) > 0 && ch != ':') {}
	*/

	// On success, get rid of binhex and write out attachment note
	if (m_ResultCode == HexGood)
	{
		if (m_lBannerStart > 0L)
			g_pPOPInboxFile->ChangeSize(m_lBannerStart);
		WriteAttachNote(m_ATFile, IDS_ATTACH_CONVERTED);

		OutLen = 0;
		char *InPtr = strchr(In, ':') + 1;
		while (InPtr - In < InLen)
		{
			*Out++ = *InPtr++;
			OutLen++;
		}
	}

	// We're done with the file
	if (m_ATFile)
	{
		Progress(1, NULL, 1);
		if (m_ResultCode != HexGood)
		{
			CString strFilename;
			BSTR bstrFilename = NULL;
			if (SUCCEEDED(m_ATFile->GetFName(&bstrFilename)))
			{
				strFilename = bstrFilename;
				::SysFreeString(bstrFilename);
			}
			::FileRemoveMT(strFilename);
		}
		delete m_ATFile;
		m_ATFile = NULL;
		PopProgress();
	}

	Init();

	return (FALSE);
}


// HexBinInputChar
//
void DecodeBH::HexBinInputChar(BYTE ch)
{
	short int ch2;

	switch (m_State)
	{
	case HexDone:
		break;
		
	case NotHex:
		if (ch == ':') 
			m_State = CollectName;
		break;

	case CollectInfo:
		if ((ch2 = HexBinDecode(ch)) >= 0)	
			HexBinDataChar(ch2);
		break;

	case CollectName:
//		*m_Outp++ = ch;
		// fall through

	default:
		if ((ch2 = HexBinDecode(ch)) >= 0)	
			HexBinDataChar(ch2);
		break;
	}
}


inline unsigned long MacToLong(unsigned long x)
{ 
	return (unsigned long) ::SwapLongMT(x); 
}
inline unsigned short MacToWord(unsigned short x)
{ 
	 return (unsigned short) ::SwapShortMT(x); 
}

//
// HexBinDataChar 
//
void DecodeBH::HexBinDataChar(short d)
{
//	short HBHeadSize = sizeof(HexBinHead);
	// don't use sizeof because of padding
	short HBHeadSize = 20;

	switch (m_State)
	{
	// Collect the decoded values that make up the filename
	case CollectName:
		CompCrc(d);
		m_Name[m_OSpot] = (char)d;
		if (m_Name[m_OSpot])
			m_OSpot++;
		else
		{
			m_State = CollectInfo;
			m_BSpot = 0;
		}
		break;
	
	case CollectInfo:
		m_BxH[m_BSpot++] = (unsigned char)d;
		switch (HBHeadSize - m_BSpot)
		{
		case 0:
			this->SetHeadFields();
			// First, get all of the bytes right
			m_BxHead.m_DataLength = MacToLong(m_BxHead.m_DataLength);
			m_BxHead.m_RzLength = MacToLong(m_BxHead.m_RzLength);

			// Test the length of the data to
			// see if we think its too short.
//			if (m_BxHead.m_DataLength < (g_lEstimatedMsgSize *4)/5)
			{
				MIMEMap mm;

				m_Crc = MacToWord(m_BxHead.m_HCrc);
				CrcError();
				if (m_State == HexDone)
					break;
				if (strncmp(m_BxHead.m_Type, CRString(IDS_ATTACH_TEXT_TYPE), 4))
					m_Translate = FALSE;
				else
					m_Translate = TRUE;
				if (mm.Find(m_BxHead.m_Author, m_BxHead.m_Type))
					mm.SetExtension(m_Name);
				if (!(m_ATFile = OpenAttachFile(m_Name + 1)))
					AbortHexBin();
				else
				{
					m_State = DataWrite;
					m_OSpot = m_BxHead.m_DataLength;
					m_DecodedHeader = TRUE;
					PushProgress();

					CString strFilename("???");
					{
						BSTR bstrFilename = NULL;
						if (SUCCEEDED(m_ATFile->GetFName(&bstrFilename)))
						{
							strFilename = bstrFilename;
							::SysFreeString(bstrFilename);
						}
					}

					char buf[256];
					wsprintf(buf, CRString(IDS_DECODING_ATTACH), strrchr(strFilename, '\\') + 1);
					ASSERT(strlen(buf) < sizeof(buf));
					Progress(0, buf, (m_OSpot * 4) / 3);
				}
			}
//			else
//				AbortHexBin();
			break;

		case 1:
			break;

		default:
			CompCrc(d);
			break;
		}
		break;

	case DataWrite:
	case RzWrite:
		// m_OSpot contains remaining bytes left in data fork
		if (m_OSpot)
		{
			// Put decoded byte in Out buffer, newline translation if needed
			if (m_Translate && d == '\r')
				m_ATFile->PutLine();
            else
				m_ATFile->Put((char)d);
			CompCrc(d);
			m_OSpot--;
			break;
		}
		m_State = (m_State == DataWrite? DataCrc1 : RzCrc1);
		// No more data, so fall through to check the CRC of the data fork

	case DataCrc1:
	case RzCrc1:
		// Compute the first part of the CRC
		m_Crc = d << 8;
		m_State = (m_State == DataCrc1? DataCrc2 : RzCrc2);
		break;

	case DataCrc2:
	case RzCrc2:
		m_Crc = m_Crc | d;
		CrcError();

		// If someone sends us a resource only file
		// treat it like its data (stupid, huh?).
		// Otherwise, we're done 'cuz PCs don't care about the resource fork.
		if (m_State == DataCrc2 && m_BxHead.m_DataLength == 0)
		{
			m_OSpot = m_BxHead.m_RzLength;
			m_State = RzWrite;
		}
		else
			m_State = HexDone;
		break;
	}
}

// HexBinDecode
//
// Takes the input character data and decodes it
//
short DecodeBH::HexBinDecode(short ch)
{
	short data;
	BYTE b6;

	if ((b6 = HexBinTable[ch]) > 64)
	{
		switch(b6)
		{	
		case SKIP:
			return (-1);
		case DONE:
			// warn user about short binhex
			ErrorDialog(IDS_ERR_SHORT_BINHEX);
			g_lBadCoding++;
			AbortHexBin();
			return (-1);
		default:
			// warn user about bad character
			ErrorDialog(IDS_ERR_BAD_BINHEX_CHAR, ch);
			g_lBadCoding++;
			AbortHexBin();
			return (-1);
		}
	}
	else
	{
		switch(m_State68++)
		{
		case 0:	
			m_B8 = (BYTE)(b6 << 2);
			return (-1);
		case 1:
			data = (short)(m_B8 | (b6 >> 4));
			m_B8 = (BYTE)((b6 & 0xF) << 4);
			break;
		case 2:
			data = (short)(m_B8 | (b6 >> 2));
			m_B8 = (BYTE)((b6 & 0x3) << 6);
			break;
		case 3:
			data = (short)(m_B8 | b6);
			m_State68 = 0;
			break;
		}

		if (!m_Run)
		{
			if (data == RUNCHAR)
			{
				m_Run = TRUE;
				m_RunCount = 0;
				return (-1);
			}
			else 
			{
				m_LastData = (BYTE)data;
				return (short)m_LastData;
			}
		}
		else
		{
			m_Run = FALSE;
			if (!data)
				return(m_LastData = RUNCHAR);
			while (--data > 0) HexBinDataChar(m_LastData);
			return (-1);
		}
	}
}

// CompCrc
//
// Compute the crc data to compare to the incoming CRC
//
//const unsigned long BYTEMASK	= 0x000000FF;
const unsigned long BYTEMASK	= 0x00FF;
const unsigned long WORDMASK	= 0x0000FFFF;
const unsigned long WORDBIT		= 0x00010000;
const unsigned long CRCCONSTANT	= 0x00001021;

void DecodeBH::CompCrc(unsigned short c)
{
	unsigned long temp = m_CalcCrc;
	int x = 8;

	for ( ; x ; x--)
	{
		c <<= 1;
		if ((temp <<= 1) & WORDBIT)
			temp = (temp & WORDMASK) ^ CRCCONSTANT;
		temp ^= (c >> 8);
		c &= BYTEMASK;
	}
	m_CalcCrc = temp;
}


// CrcError
//
// Compares computed and received CRC to see if they are the same
//
void DecodeBH::CrcError()
{
	CompCrc(0);
	CompCrc(0);
	if ((m_Crc & WORDMASK) != (m_CalcCrc & WORDMASK))
	{
		ErrorDialog(IDS_ERR_CRC_IN_ATTACH);
		g_lBadCoding++;
		AbortHexBin();
	}
	m_CalcCrc = 0;
}


// AbortHexBin
//
// Does misc clean up and ends the hexbin routine
//
void DecodeBH::AbortHexBin()
{
	m_State = HexDone;
	m_ResultCode = HexBad;
}


void DecodeBH::SetHeadFields()
{
	int offset = 0;
	//Copy in the Data one field at a time

	memcpy((void *)m_BxHead.m_Type,(void *)(m_BxH+offset),sizeof(m_BxHead.m_Type));
	offset += sizeof(m_BxHead.m_Type);

	memcpy((void *)m_BxHead.m_Author, (void *)(m_BxH+offset),sizeof(m_BxHead.m_Author));
	offset += sizeof(m_BxHead.m_Author);

	memcpy((void *)&m_BxHead.m_Flags, (void *)(m_BxH+offset),sizeof(short));
	offset += sizeof(short);

	memcpy((void *)&m_BxHead.m_DataLength , (void *)(m_BxH+offset), sizeof(long));
	offset += sizeof(long);

	memcpy((void *)&m_BxHead.m_RzLength , (void *)(m_BxH+offset), sizeof(long));
	offset += sizeof(long);

	memcpy((void *)&m_BxHead.m_HCrc , (void *)(m_BxH+offset), sizeof(unsigned short));

}
