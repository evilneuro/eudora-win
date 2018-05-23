// FileLineReader.cpp
//
// Classes for MIME parsing input that reads a line at a time from a file

#include "stdafx.h"

#include "FileLineReader.h"
#include "QCUtils.h"
#include "guiutils.h"


#include "DebugNewHelpers.h"


long FileLineReader::ReadLine(char* buf, LONG bSize)
{
	////////////////////////////////////////////////////////////
	// START DEATH ROW CODE                                   //
	// Ok, this is just staying here for right now.  But when //
	// the MIME parsing reorg is done so that we can do it in //
	// a background thread, it must die.                      //
	////////////////////////////////////////////////////////////
	static BOOL bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;
	const BOOL bIsMainThread = ::IsMainThreadMT();

	if (bIsMainThread)
	{
		if (bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce)
		{
			// And what exactly do you think you're doing, Mister?
			// Does your mother know you're trying to get in to this
			// function more than once?  Well does she?!?!
			ASSERT(0);
			return -1;
		}
		bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = TRUE;
	}

	if( (m_nLinesRead++ % 100) == 0)
	{
		if(bIsMainThread && EscapePressed())//call escape pressed only if FileLineReader is used
											//in main thread.
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
		if(bIsMainThread)
			bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;
		return -1;
	}

	if (lNumBytesRead)
	{
		// If the last character is NULL, then JJFileMT chopped off a CR or a
		// CRLF (JJFileMT NULL terminates after all characters on error or EOF).
		if (!buf[lNumBytesRead-1])
		{
			// Check for stripped off CR or CRLF. For CRLF, the NULL terminating
			// logic is unchanged (for now) to decrease the changes made for
			// Eudora 6.1.1.
			// For CR, we'll make sure before we change anything that we have
			// room for \r\n\0.
			if (lNumBytesRead >= 2 && !buf[lNumBytesRead-2])
			{
				// Add back the stripped off CRLF
				buf[lNumBytesRead-2] = '\r';
				buf[lNumBytesRead-1] = '\n';

				// NULL terminate only if there's space - this makes me nervous,
				// but that's how the code always did it.
				if (lNumBytesRead < bSize)
					buf[lNumBytesRead] = 0;
			}
			else if (lNumBytesRead+1 < bSize)
			{
				// A CR was stripped off - add on a CRLF
				// We don't normally expect to see CR's by themselves, but certain
				// exploits such as those reported by Paul Szabo, relied on the fact
				// that previously CR's terminated our line processing, but didn't
				// actually persist. We now translate lone CR's to CRLF's to avoid
				// this situation.
				buf[lNumBytesRead-1] = '\r';
				buf[lNumBytesRead] = '\n';

				// Increase the count because we've added a LF
				lNumBytesRead++;

				// NULL terminate
				buf[lNumBytesRead] = '\0';
			}
		}
	}
	if(bIsMainThread)
		bOnlyYoooooooooouuuuuuuuuCanEnterThisFunctionOnce = FALSE;

	return lNumBytesRead;
}

long BufferLineReader::ReadLine(char* buf, LONG bSize)
{
	if(m_nCurrentPos >= m_nMaxSize)
		{
		buf = NULL;
		return 0;
		}

	if (m_nCurrentPos + bSize > m_nMaxSize)
		bSize = m_nMaxSize - m_nCurrentPos;

	if(bSize >0)
	{
		memcpy(buf,m_pFloatingBuffer + m_nCurrentPos,bSize);
		m_nCurrentPos += bSize;
	}
	else
	{
		buf = NULL;
		return 0;
	}

	return bSize;
}


