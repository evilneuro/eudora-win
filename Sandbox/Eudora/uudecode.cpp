// UUDECODE.CPP
//
// Routines for decoding uuencoded data

#include "stdafx.h"

#include "cursor.h"
#include "fileutil.h"
#include "lex822.h"
#include "header.h"
#include "resource.h"
#include "MIMEMap.h"
#include "mime.h"
#include "uudecode.h"
#include "pop.h"
#include "progress.h"
#include "resource.h"
#include "guiutils.h"
#include "rs.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


DecodeUU::DecodeUU(MIMEState* ms)
{
	m_ms = ms;
	Init();
}

DecodeUU::~DecodeUU()
{
	delete m_ATFile;
}

int DecodeUU::Init()
{
	m_State = AbDone;
	m_ATFile = NULL;

	return (0);
}

BOOL DecodeUU::IsAbLine(const char* text, LONG size)
{
	const char* spot;

	if (size < 11 || CompareRString(IDS_UUENCODE_BANNER, text, 6))
		return (FALSE);
	spot = text + 7;
	while (*spot >= '0' && *spot <= '7')
		spot++;
	if (*spot != ' ' || spot - text > 10)
		return (FALSE);
	if (*++spot == '\r')
		return (FALSE);

	strncpy(m_Name, spot, sizeof(m_Name) - 1);
	m_Name[sizeof(m_Name) - 1] = 0;

	// See if we can figure out the extension from the MIME headers
	if (m_ms)
	{
		MIMEMap mm;

		if (mm.Find(m_ms))
			mm.SetExtension(m_Name);
	}

	return (TRUE);
}

int DecodeUU::Decode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	char buf[256];
	
	// Only copy out if we aren't uudecoding
	if (m_State != AbWrite)
	{
		if (In != Out)
			strncpy(Out, In, InLen);
		OutLen = InLen;
	}

	switch (m_State)
	{
	case AbDone:
		if (IsAbLine(In, InLen))
		{
			m_State = NotAb;
			g_pPOPInboxFile->Tell(&m_lOrigOffset);  // save start
			ASSERT(m_lOrigOffset >= 0);
		}
		break;
		
	case NotAb:
		{
			// we just saw a line that looked like a begin line
			// if this line is the right length, we'll give it a go
			if (UURightLength(In, InLen) < 0 || !(m_ATFile = OpenAttachFile(m_Name)))
			{
				m_State = AbDone;
				break;
			}
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
			wsprintf(buf, CRString(IDS_DECODING_ATTACH), strrchr(strFilename, '\\') + 1);
			Progress(0, buf, g_lEstimatedMsgSize);
			g_pPOPInboxFile->ChangeSize(m_lOrigOffset);	// toss the saved bits
			m_State = AbWrite;
		}
		// Fall through to AbWrite
		
	case AbWrite:
		UULine(In, InLen);
		OutLen = 0;
		break;
	}
	
	if (m_State != AbDone)
	{
		if (m_ATFile)
			ProgressAdd(InLen);
		return (TRUE);
	}

	// We're done with the file
	if (m_ATFile)
	{
		// Write out attachment note
		WriteAttachNote(m_ATFile, IDS_ATTACH_CONVERTED);

		Progress(1, NULL, 1);
		delete m_ATFile;
		m_ATFile = NULL;
		PopProgress();
	}

	return (FALSE);
}

inline int UU(char c) { return ((c - ' ') & 077); }
inline BOOL InvalidChar(char c) { return (c < ' ' || c > '`'); }
/************************************************************************
 * UURightLength - find out if the current line is of the proper length
 *  Returns length if so, negative number if mismatch
 ************************************************************************/
int DecodeUU::UURightLength(const char* text, LONG size, BOOL Strict /*= TRUE*/)
{
	if (!size || *text == '\r' || *text == '\n')
		return (0);

	int length = UU(*text);

	if (InvalidChar(*text))
		return (-1);  // BZZZZT
	
	text++;
	size--;
	if (text[size - 1] == '\n')
		size--;
	if (text[size - 1] == '\r')
		size--;

	BOOL Matches = ((length + 2) / 3 == ((size * 3) / 4) / 3);
	if (!Matches)
	{
		if (Strict)
			return (-2);	// BZZZZT
//		g_lBadCoding++;
	}

	return (length);	// hurray!
}

/************************************************************************
 * UULine - handle a line of uuencoded stuff
 ************************************************************************/
AbStates DecodeUU::UULine(const char* text, LONG size)
{
	int length;
	BOOL result = TRUE;
	
	// check for end line
	if (size >= 3 && size <= 5 && !strnicmp("end\r\n", text, size))
		return (m_State = AbDone);
	
	// check length of line against line count
	if ((length = UURightLength(text, size, FALSE)) == -1)
	{
//		WarnUser(UU_BAD_LENGTH,(length+2)/3-((size*3)/4)/3);
		return (m_State = AbDone);
	}
	
	// empty lines mean nothing
	if (length == 0)
		return (m_State);
	
	// skip length byte, and trailing newline
	text++;
	size--;
	if (text[size - 1] == '\n')
		size--;
	if (text[size - 1] == '\r')
		size--;
	
	// hey!  we're ready to decode!
	for (; length > 0; text += 4, length -= 3)
	{
		if (InvalidChar(text[0]) || InvalidChar(text[1]) ||
			length > 1 && InvalidChar(text[2]) ||
			length > 2 && InvalidChar(text[3]))
		{
//			WarnUser(BINHEX_BADCHAR,0);
			break;
		}
		if (FAILED(m_ATFile->Put((char)(UU(text[0]) << 2 | UU(text[1]) >> 4))))
			break;
		if (length > 1 &&
			FAILED(m_ATFile->Put((char)(UU(text[1]) << 4 | UU(text[2]) >> 2))))
		{
			break;
		}
		if (length > 2 && FAILED(m_ATFile->Put((char)(UU(text[2])<<6 | UU(text[3])))))
			break;
	}

	if (length <= 0)
		return (m_State);

	g_lBadCoding++;
	return (m_State = AbDone);
}


EncodeUU::EncodeUU()
{
	m_OutBuf = m_buf + 1;
	m_ByteNum = 0;
	m_LastByte = 0;
}

EncodeUU::~EncodeUU()
{
}

int EncodeUU::Init(char* Out, LONG& OutLen, int flag)
{
	char fname[_MAX_PATH + 1];

	strcpy(fname, Out);
	*m_buf = 'M';
	m_ByteNum = 0;

	// Send the uuencode banner
	wsprintf(Out, "\r\n%s %s\r\n", (const char*)CRString(IDS_UUENCODE_BANNER), fname);
	OutLen = strlen(Out);

	return (0);
}


inline char CODE(int c)
{
	int ch = (c & 0x3F);
	return ((char)(ch? (ch + ' ') : '`'));
}

int EncodeUU::Encode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	OutLen = 0;

	for (BYTE *ch = (BYTE *)In; InLen; InLen--, ch++, m_ByteNum++)
	{
		switch (m_ByteNum & 3)
		{
		case 0:
			m_OutBuf[m_ByteNum] = CODE(*ch >> 2);
			break;
		case 1:
			m_OutBuf[m_ByteNum] = CODE((m_LastByte << 4) | (*ch >> 4));
			break;
		case 2:
			m_OutBuf[m_ByteNum] = CODE((m_LastByte << 2) | (*ch >> 6));
			m_ByteNum++;
			m_OutBuf[m_ByteNum] = CODE(*ch);
			if (m_ByteNum == 59)
			{
				strcpy(m_OutBuf + m_ByteNum + 1, "\r\n");
				strcpy(Out + OutLen, m_buf);
				OutLen += 63;
				m_ByteNum = -1;
			}
			break;
		}
		m_LastByte = *ch;
	}

	return (0);
}

int EncodeUU::Done(char* Out, LONG &OutLen)
{
	*Out = 0;

	// Deal with left overs
	if (m_ByteNum)
	{
		*m_buf = CODE(m_ByteNum / 4 * 3 + (m_ByteNum & 3));
		switch (m_ByteNum & 3)
		{
		case 1:
			m_OutBuf[m_ByteNum++] = CODE(m_LastByte << 4);
			m_OutBuf[m_ByteNum++] = CODE(0);
			m_OutBuf[m_ByteNum++] = CODE(0);
			break;
		case 2:
			m_OutBuf[m_ByteNum++] = CODE(m_LastByte << 2);
			m_OutBuf[m_ByteNum++] = CODE(0);
			break;
		}
		strcpy(m_OutBuf + m_ByteNum, "\r\n");
		strcpy(Out, m_buf);
	}

	// Send uuencode closing
	strcat(Out, CRString(IDS_UUENCODE_CLOSING));
	strcat(Out, "\r\n");

	OutLen = strlen(Out);

	return (0);
}
