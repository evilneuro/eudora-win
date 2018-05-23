// ImSingle.cpp - DECODER FOR APPLESINGLE ATTACHMENTS.
// 

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP

#include <QCUtils.h>

#include "cursor.h"
#include "fileutil.h"
#include "lex822.h"
#include "header.h"
#include "resource.h"
#include "mime.h"
#include "ImHexbin.h"
#include "pop.h"
#include "resource.h"
#include "guiutils.h"
#include "rs.h"

#include "ImSingle.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



// =========== STATICS =============/



//=================== ImapDecodeAS class implementation ==============//

ImapDecodeAS::ImapDecodeAS()
{
	// Initialize stuff:
	//
	m_HeaderCount	= 0;
	m_NameOffset	= 0x7FFFFFFF;
	m_NameEnd		= 0;
	m_TypeOffset	= 0x7FFFFFFF;
	m_TypeCreatorOffset = 0;
	m_DataOffset	= 0;
	m_LastType		= 0;
	m_Number		= 0;

	// Make sure of this:
	//
	m_pSecondaryDecoder = NULL;

	//
	m_bDoneWithHeader = FALSE;

    *m_Filename = 0;

	// Accumulate filename into "m_Filename".
	m_pNamePtr = m_Filename;
	*m_ProgBuf = 0;
	*m_TypeCreator = 0;
}


ImapDecodeAS::~ImapDecodeAS()
{
	// Delete any secondary decoder:
	if (m_pSecondaryDecoder)
		delete m_pSecondaryDecoder;

	m_pSecondaryDecoder = NULL;
}


int ImapDecodeAS::Init()
{
	// Initialize stuff:
	//
	m_HeaderCount	= 0;
	m_NameOffset	= 0x7FFFFFFF;
	m_NameEnd		= 0;
	m_TypeOffset	= 0x7FFFFFFF;
	m_TypeCreatorOffset = 0;
	m_DataOffset	= 0;
	m_LastType		= 0;
	m_Number		= 0;


	// If there is a secondary deoceder, initialize it:
	//
	if (m_pSecondaryDecoder)
		m_pSecondaryDecoder->Init();

	//
	m_bDoneWithHeader	= FALSE;
    *m_Filename			= 0;

	// Accumulate filename into "m_Filename".
	//
	m_pNamePtr			= m_Filename;
	*m_ProgBuf			= 0;
	*m_TypeCreator		= 0;

	return (0);
}



// Decode [PUBLIC]
// 
// This looks for the creator and type and discards them, and passes back 
// the decoded data.
// This also does any content-transfer decoding because the caller needs to
// set that decoder as a member of this object (using "SetDecondaryDecoder()")
// before the first call to this method.
//

int ImapDecodeAS::Decode(const char* In, LONG InLen, char* Out, LONG& OutLen)
{
	LONG size;

	// If we've got a secondary decoder, decode string in-place:
	//
	size = InLen;

	if (m_pSecondaryDecoder)
		m_pSecondaryDecoder->Decode(In, size, Out, size);
	else
	{
		// Copy to output.
		if (In != Out)
			memcpy(Out, In, size);
	}

	// Go through the header to find pertinent info about file.
	// Note: m_bDoneWithHeader is initialized to FALSE in the Init() 
	// method, and may be set to TRUE in here.
	//
	if (!m_bDoneWithHeader)
	{
		for (BYTE* b = (BYTE*)Out; size; m_HeaderCount++, b++, size--)
		{
			// Note: m_HeaderCount initialized in Init().
			//
			if (m_HeaderCount <= 0x19)
			{
				;
			}
			// Note: m_NameOffset initialized in Init().
			else if (m_HeaderCount < m_NameOffset)
			{
				int MapOffset = (int)((m_HeaderCount - 0x19 - 1) % 12 );
				int NumByte = MapOffset % 4;

				if (!NumByte)
					m_Number = 0L;

				m_Number |= ((long)*b) << ((3 - NumByte) * 8);

				if (MapOffset == 3)			// type
					m_LastType = (int)m_Number;
				else if (MapOffset == 7)	// offset
				{
					switch (m_LastType)
					{
					case 1: m_DataOffset = m_Number; break;
					case 3: m_NameOffset = m_Number; break;
					case 9: m_TypeCreatorOffset = m_Number; break;
					}
				}
				else if (MapOffset == 11)	// length
				{
					if (m_LastType == 3)
						m_NameEnd = m_NameOffset + m_Number;
				}
			}
			else if (m_HeaderCount >= m_NameOffset && m_HeaderCount < m_NameEnd)
			{
				*m_pNamePtr++ = *b;
				if (m_HeaderCount == m_NameEnd - 1)
				{
					// Ok. We've got the mapped name into "m_Filename". Tie off.
					//
					*m_pNamePtr++ = 0;
				}
			}
			else if (m_HeaderCount >= m_TypeCreatorOffset &&
				m_HeaderCount < m_TypeCreatorOffset + 8)
			{
				m_TypeCreator[m_HeaderCount - m_TypeCreatorOffset] = *b;

#if 0 // (JOK - Do we really want to do this??

				if (m_HeaderCount == m_TypeCreatorOffset + 7 &&
						mm.Find(TypeCreator + 4, TypeCreator))
				{
					mm.SetExtension(hd->attributeName);
				}
#endif //
			}
			else if (m_HeaderCount == m_DataOffset)
			{
				// We're at the start of the data offset. Copy what we have left
				// to the output.
				//
				memcpy(Out, b, size);

				// Signal for next that we're done with the header.
				m_bDoneWithHeader = TRUE;

				break;
			}
		}
	}

	// Return any bytes left. If we've looped through the complete data we
	// were given, "size" will be 0.
	//
	OutLen = size;
	
	return (TRUE);
}



#endif // IMAP4

