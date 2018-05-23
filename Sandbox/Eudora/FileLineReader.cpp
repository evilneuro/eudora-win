// FileLineReader.cpp
//
// Classes for MIME parsing input that reads a line at a time from a file

#include "stdafx.h"

#include "FileLineReader.h"
#include "QCUtils.h"
#include "guiutils.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


long FileLineReader::ReadLine(char* buf, long bSize)
{
	////////////////////////////////////////////////////////////
	// START DEATH ROW CODE                                   //
	// Ok, this is just staying here for right now.  But when //
	// the MIME parsing reorg is done so that we can do it in //
	// a background thread, it must die.                      //
	////////////////////////////////////////////////////////////
	static BOOL bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;

	if (bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce)
	{
		// And what exactly do you think you're doing, Mister?
		// Does your mother know you're trying to get in to this
		// function more than once?  Well does she?!?!
		ASSERT(0);
		return -1;
	}
	bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = TRUE;

	if( (m_nLinesRead++ % 100) == 0)
	{
		if( EscapePressed())
		{
			m_bEscapePressed = true;
			TRACE("Escape pressed while doing MIME parsing");
			bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;
			return -1;
		}
	}
	////////////////////////////////////////////////////////////
	// END DEATH ROW CODE                                     //
	////////////////////////////////////////////////////////////



	long lNumBytesRead = 0;
	if (FAILED(m_File->GetLine(buf, bSize, &lNumBytesRead)))
	{
		bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;
		return -1;
	}

	if (lNumBytesRead >= 2)
	{
		// Add back the stripped off CRLF
		if (!buf[lNumBytesRead-2] && !buf[lNumBytesRead-1]) 
		{
			buf[lNumBytesRead-2] = '\r';
			buf[lNumBytesRead-1] = '\n';
		}
	}

	bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;

	return lNumBytesRead;
}
