// JJFILE.CPP
//
// File utility routines
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include <afxpriv.h>		// for T2COLE()

#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <direct.h>

#include <QCUtils.h>

#include "jjfile.h"
#include "QCError.h"

const char chrSLASH = '\\';
const char* pszSLASH = "\\";


#include "DebugNewHelpers.h"


JJFileMT::JJFileMT(int nSize /*= BUF_SIZE*/)
{
	ASSERT(this);
	
	m_pBuffer = DEBUG_NEW char[nSize];
	m_bIsOurBuffer = TRUE;
	m_nSize = nSize;
	m_pBufPtr = m_pBuffer;
	m_nValidBytes = 0;
	m_fd = -1;
	m_bIsIn = false;
}

JJFileMT::JJFileMT(char* pBuffer, int nSize)
{
	ASSERT(this);
	
	m_pBuffer = pBuffer;
	m_bIsOurBuffer = FALSE;
	m_nSize = nSize;
	m_pBufPtr = m_pBuffer;
	m_nValidBytes = 0;
	m_fd = -1;
	m_bIsIn = false;
}

JJFileMT::~JJFileMT()
{
	ASSERT(this);
	
	if (IsOpen() == S_OK)
		Close();
	if (m_bIsOurBuffer)
		delete [] m_pBuffer;
}
      

////////////////////////////////////////////////////////////////////////
// Open [public, virtual]
//
// Formerly returned 'm_fd', which was -1 on error.  New version returns
// HRESULT.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Open(const char* filename, int mode, long tries, long interval)
{
	ASSERT(this);

	// Check for both NULL and file names that are too big if we strip
	// off trailing periods.
	if ( !filename || (strlen(filename) >= _MAX_PATH) )
		m_strFName.Empty();
	else
	{
		char StrippedName[_MAX_PATH + 1];
		char* s = StrippedName;
		const char *f = filename;

		// Strip off trailing periods in directories for Novell bug 
		while (*f)
		{
			if (f[0] != '.' || f[1] != chrSLASH)
				*s++ = *f;
			f++;
		}
		*s = 0;

		//
		// We must set the filename before we return, even if we end up
		// failing on the open() call.  This is so that error handlers
		// will know which file we failed to open.
		//
		m_strFName = StrippedName;

retry:
		// If mode contains _O_CREAT, then _S_IWRITE is 
		// necessary so the file doesn't become read-only.
		// _O_APPEND seems to be broken with the Symantec C++ library.
		m_fd = open(StrippedName, (mode & ~O_APPEND) | O_BINARY, S_IWRITE);
		if (m_fd < 0)
		{
			if (errno==13 && --tries>0)	// 13 is access error, supposedly.
			{
				Sleep(interval);
				goto retry;	// honestly, this is the cleanest way.  SD 3/11/99
			}

			m_nLastError = errno;
		}
		else
		{
			// _O_APPEND seems to be broken with the Symantec C++ library.
			if (mode & O_APPEND)
				lseek(m_fd, 0L, SEEK_END);

			LPCTSTR pFilename = strrchr(m_strFName, '\\');
			if (pFilename && stricmp(pFilename + 1, "in.mbx") == 0)
			{
				m_bIsIn = true;
			}
			if (m_bIsIn && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
			{
				char		szLogBuf[256];
				long		lFSize = 0;
				GetFSize(&lFSize);
				sprintf(szLogBuf, "LOGNULL JJFileMT::Open() %s, Size: %d", (LPCTSTR)m_strFName, lFSize);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
			}

			return S_OK;
		}
	}

	if (mode != O_RDONLY)
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_OPEN_WRITING);

	return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_OPEN_READING);
}


////////////////////////////////////////////////////////////////////////
// Close [public]
//
// Formerly returned -1 on error, 0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Close(void)
{
	ASSERT(this);

	if (m_fd >= 0)
	{
		// close regardless of Flush() success
		HRESULT hrFlush = Flush();
		m_nLastError = errno;

		if (m_bIsIn && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char		szLogBuf[256];
			long		lFSize = 0;
			GetFSize(&lFSize);
			sprintf(szLogBuf, "LOGNULL JJFileMT::Close() %s, Size: %d", (LPCTSTR)m_strFName, lFSize);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		}

		// shut it down
		m_fd = ::close(m_fd);
		if (m_fd == 0)
		{
			// successful close()
			m_fd = -1;
			if (FAILED(hrFlush))
				return hrFlush;
			return S_OK;
		}
	}

	//
	// If we get here, either the close failed, or the file wasn't
	// open in the first place.
	//
	m_nLastError = errno;
	return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_CLOSE);
}


HRESULT JJFileMT::Reset()
{
	HRESULT hrSeek = Seek(0L, SEEK_SET);
	m_pBufPtr = m_pBuffer;
	m_nValidBytes = 0;
	m_nLastError = errno;
	return hrSeek;
}

////////////////////////////////////////////////////////////////////////
// Flush [public, virtual]
//
// Formerly returned -1 on error, else 0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Flush(void)
{
	ASSERT(this);
	
	if (m_nValidBytes <= 0)
	{
		m_nValidBytes = 0;
		return S_OK;
	}

	char		szLogBuf[256];
	long		lFSizeBefore = 0;
	long		lFSizeAfter = 0;

	if (m_bIsIn)
	{
		GetFSize(&lFSizeBefore);
	}
	int			iBytesWritten = ::write(m_fd, m_pBuffer, m_nValidBytes);
	if (m_bIsIn && (iBytesWritten != m_nValidBytes) && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
	{
		sprintf(szLogBuf, "LOGNULL JJFileMT::Flush() %s, Write %d bytes, %d actually written", (LPCTSTR)m_strFName, m_nValidBytes, iBytesWritten);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		GetFSize(&lFSizeAfter);
		sprintf(szLogBuf, "  %d + %d = %d (%d)", lFSizeBefore, iBytesWritten, lFSizeBefore + iBytesWritten, lFSizeAfter);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
	}
	if (m_fd < 0 || iBytesWritten < m_nValidBytes)
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_WRITE);
    }

	m_nValidBytes = 0;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Seek [public, virtual]
//
// Formerly returned -1 on error, else new offset from start of file.
// New interface optionally returns offset in 'pNewOffset'.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Seek(long lOffset, int nMode /*= SEEK_SET*/, long* plNewOffset /*= NULL*/)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);
	
	HRESULT hrFlush = Flush();
	if (FAILED(hrFlush))
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_WRITE);
	}

	long		 lFSize = 0;
	GetFSize(&lFSize);
	if (nMode == SEEK_SET)
	{
		if (lOffset > lFSize && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			LPCTSTR pFilename = strrchr(m_strFName, '\\');
			LPCTSTR pExt = NULL;
			if (pFilename)
				pExt = strrchr(++pFilename, '.');
			if (pFilename && stricmp(pExt + 1, "mbx") == 0)
			{
				char		szLogBuf[512];
				sprintf(szLogBuf, "LOGNULL Seeking past end of %s (SEEK_SET) %d > %d", pFilename, lOffset, lFSize);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
			}
		}
	}
	else if (nMode == SEEK_CUR)
	{
		long		 lTell = 0;
		if (Tell(&lTell) == S_OK)
		{
			if (lTell + lOffset > lFSize && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
			{
				LPCTSTR pFilename = strrchr(m_strFName, '\\');
				LPCTSTR pExt = NULL;
				if (pFilename)
					pExt = strrchr(++pFilename, '.');
				if (pFilename && stricmp(pExt + 1, "mbx") == 0)
				{
					char		szLogBuf[512];
					sprintf(szLogBuf, "LOGNULL Seeking past end of %s (SEEK_CUR) %d + %d > %d", pFilename, lTell, lOffset, lFSize);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
				}
			}
		}
	}
	else if (nMode == SEEK_END)
	{
		if (lOffset > 0 && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			LPCTSTR pFilename = strrchr(m_strFName, '\\');
			LPCTSTR pExt = NULL;
			if (pFilename)
				pExt = strrchr(++pFilename, '.');
			if (pFilename && stricmp(pExt + 1, "mbx") == 0)
			{
				char		szLogBuf[512];
				sprintf(szLogBuf, "LOGNULL Seeking past end of %s (SEEK_END) %d", pFilename, lOffset);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
			}
		}
	}
	
	long lNewOffset = ::lseek(m_fd, lOffset, nMode);
	if (lNewOffset < 0L)
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
	}

	if (plNewOffset)
		*plNewOffset = lNewOffset;		// caller wants the new offset

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Tell [public]
//
// Formerly returned -1 on error, else new offset from start of file.
// New interface optionally returns offset in 'pNewOffset'.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Tell(long* plOffset)
{
	if (NULL == plOffset)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	long lOffset = ::lseek(m_fd, 0L, SEEK_CUR);
	if (lOffset < 0L)
	{
		*plOffset = lOffset;
		m_nLastError = errno;
		return E_FAIL;
	}
	else
	{
		*plOffset = lOffset + m_nValidBytes;
		return S_OK;
	}
}


////////////////////////////////////////////////////////////////////////
// Stat [public]
//
// Formerly returned -1 on error, else 0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Stat(struct stat* sptr)
{
	ASSERT(IsOpen() == S_OK);

	if (NULL == sptr)
	{
		ASSERT(0);
		return E_INVALIDARG;		// std OLE error code for bad arg
	}

	int nStatus = ::fstat(m_fd, sptr);
	if (nStatus < 0)
	{
		m_nLastError = errno;
		return E_FAIL;
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// GetFName [public]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::GetFName(BSTR* pBStr) const
{ 
	if (NULL == pBStr)
	{
		ASSERT(0);
		return E_INVALIDARG;		// std OLE error code for bad arg
	}

	USES_CONVERSION;
	*pBStr = ::SysAllocString(T2COLE(m_strFName));
	return S_OK;
}

HRESULT JJFileMT::GetFSize(long* pFSize) const
{ 
	if (NULL == pFSize)
	{
		ASSERT(0);
		return E_INVALIDARG;		// std OLE error code for bad arg
	}
	if (IsOpen() != S_OK)
	{
		return S_FALSE;
	}
	else
	{
		*pFSize = _filelength(m_fd);
		return S_OK;
	}

}

////////////////////////////////////////////////////////////////////////
// GetBuf [public]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::GetBuf(char** ppBuffer)
{
	if (NULL == ppBuffer)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}
	
	*ppBuffer = m_pBuffer; 
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// GetBufSize [public]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::GetBufSize(int* pnSize) const
{
	if (NULL == pnSize)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}
	
	*pnSize = m_nSize; 
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// GetNextBlock [public]
//
// For callers who really know what they're doing...
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::GetNextBlock(long* plNumBytes)
{
	if (NULL == plNumBytes)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	*plNumBytes = ::read(m_fd, m_pBuffer, m_nSize); 
	if (*plNumBytes < 0)
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
	}
	else if (0 == *plNumBytes)
		return S_FALSE;				// EOF
	else
		return S_OK;				// success
}


////////////////////////////////////////////////////////////////////////
// ChangeSize [public]
//
// Formerly returned -1 on error, 0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::ChangeSize(long lLength)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);
	
	HRESULT hrFlush = Flush();
	if (FAILED(hrFlush))
	{
		m_nLastError = errno;
		return hrFlush;
	}

	long		 lFSize = 0;
	GetFSize(&lFSize);
	if (lLength > lFSize && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
	{
		char		szLogBuf[512];
		sprintf(szLogBuf, "LOGNULL _chsize() growing file %s %d > %d", (LPCTSTR)m_strFName, lLength, lFSize);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
	}

	if ((::_chsize(m_fd, lLength) < 0) ||
		(::_lseek( m_fd, 0, SEEK_END) < 0))
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_CHANGESIZE);
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Truncate [public]
//
// Truncates the file at the given offset into the file.
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Truncate(long lOffset)
{
	long lOldOffset = 0;
	HRESULT hr = Tell(&lOldOffset);
	if (FAILED(hr))
	{
		m_nLastError = errno;
		return hr;
	}
	if (lOffset > lOldOffset)
	{
		TRACE2("Error in truncating file, trying to grow file: old offset=%ld   new offset=%ld\n", lOldOffset, lOffset);
		ASSERT(0);
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_CHANGESIZE);
	}

	hr = Seek(lOffset);
	if (FAILED(hr))
	{
		m_nLastError = errno;
		return hr;
	}

	long lNewOffset = 0;
	hr = Tell(&lNewOffset);
	if (FAILED(hr))
	{
		m_nLastError = errno;
		return hr;
	}
	ASSERT(lNewOffset >= 0);

	hr = ChangeSize(lNewOffset);
	if (FAILED(hr))
	{
		m_nLastError = errno;
		return hr;
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Delete [public, virtual]
//
// Formerly returned -1 on error, 0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Delete() 
{
	ASSERT(this);
	
	if (IsOpen() == S_OK)
		Close();

	HRESULT hrRemove = ::FileRemoveMT(m_strFName);
    if (FAILED(hrRemove))
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_DELETE);
	}

    return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Rename [public, virtual]
//
// Formerly returned -1 on error, >=0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Rename(const char* NewName, BOOL bReplace /*= TRUE*/)
{
	ASSERT(this);

	HRESULT		hr;
	
	// Close the file if it's open
	if (IsOpen() == S_OK)
	{
		hr = Close();
		if ( FAILED(hr) )
		{
			m_nLastError = errno;
			return hr;
		}
	}

	hr = FileRenameReplaceMT(m_strFName, NewName, bReplace);

	// If renaming file succeeded, remember our new name
	if ( SUCCEEDED(hr) )
	{
		m_strFName = NewName;
		
		// Keep track of whether or not we're dealing with the In mailbox for logging purposes
		LPCTSTR pFilename = strrchr(m_strFName, '\\');
		if (pFilename && stricmp(pFilename + 1, "in.mbx") == 0)
		{
			m_bIsIn = true;
		}
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
// JJBlockMove [public, virtual]
//
// Formerly returned -1 on error, 0 if we hit EOF in the source file,
// or 1 if successful.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::JJBlockMove(long Start, long Length, JJFileMT* Target)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);
	
	HRESULT hrSeek = Seek(Start);
	if ( FAILED(hrSeek) )
	{
		m_nLastError = errno;
		return hrSeek;
	}

	while (Length)
	{
		if (!m_nValidBytes)
		{
			if ((m_nValidBytes = ::read(m_fd, m_pBuffer, m_nSize)) <= 0)
			{
				if (m_nValidBytes < 0)
				{
					m_nLastError = errno;
					return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
				}

				return S_FALSE;		// successfully hit EOF 
			}
			m_nValidBytes = -m_nValidBytes;
			m_pBufPtr = m_pBuffer;
		}

		int copied = (int)min(Length, (long)-m_nValidBytes);
		if ((copied = ::write(Target->m_fd, m_pBufPtr, copied)) <= 0)
		{
			m_nLastError = errno;
			return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_WRITE);
        }
		m_nValidBytes += copied;
		m_pBufPtr += copied;
		Length -= copied;
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Read [public, virtual]
//
// Formerly returned -1 on error, 0 on EOF, else 1 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Read(char* pBuffer, long lSize)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);
	
	while (lSize)
	{
		if (!m_nValidBytes)
		{
			if ((m_nValidBytes = ::read(m_fd, m_pBuffer, m_nSize)) <= 0)
            {
				if (m_nValidBytes < 0)
				{
					m_nLastError = errno;
					return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
				}

				return S_FALSE;			// Successfully hit EOF.
			}
			m_nValidBytes = -m_nValidBytes;
			m_pBufPtr = m_pBuffer;
		}
		*pBuffer++ = *m_pBufPtr++;
		m_nValidBytes++;
		lSize--;
	}
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// RawRead [public]
//
// Formerly returned -1 on error, 0 on EOF, else number of bytes read
// on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::RawRead(char* pBuffer, int nSize, long* plNumBytesRead /* = NULL*/)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);

	if (NULL == pBuffer || nSize <= 0)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	int nNumBytesRead = ::read(m_fd, pBuffer, nSize);
	if (plNumBytesRead)
		*plNumBytesRead = long(nNumBytesRead);

	if (nNumBytesRead < 0)
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
	}
	else if (0 == nNumBytesRead)
		return S_FALSE;			// successfully hit EOF
	
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// GetLine [public]
//
// Formerly called Get().  Formerly returned -1 on error, 0 on EOF, or
// >0 if successful.  On success, the "num bytes read" value
// (optionally returned to the caller) is the number of bytes copied
// to the caller's buffer.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::GetLine(char* pBuffer, long lBufferSize, long* plNumBytesRead /*= NULL*/)
{
	long lNumBytesRead = ReadLine_(pBuffer, lBufferSize);
	if (plNumBytesRead)
		*plNumBytesRead = lNumBytesRead;

	if (lNumBytesRead < 0)
	{
		m_nLastError = errno;
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
	}
	else if (0 == lNumBytesRead)
		return S_FALSE;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// ReadLine_ [private]
//
// Do buffered read from our internal buffer, refilling the internal
// buffer if necessary.  Copy stuff from our internal buffer to the
// caller-provided buffer until either 1) the caller's buffer fills up
// or 2) we hit a CRLF or an EOF.  The caller's buffer is *not* NULL
// terminated, although terminating CRLF characters, if found, are
// replaced with NULLs.
//
// Returns -1 on error, 0 on EOF, otherwise the number of bytes 
// (including optional NULLs) copied to the caller's buffer.
////////////////////////////////////////////////////////////////////////
long JJFileMT::ReadLine_(char* pBuffer, long lSize)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);

	BOOL bFoundCR = FALSE;
	
	for (long lNumRead = 0; lNumRead < lSize; pBuffer++)
	{
		if (!m_nValidBytes)
		{
			// read() returns -1 on error, 0 for EOF
			if ((m_nValidBytes = ::read(m_fd, m_pBuffer, m_nSize - 1)) <= 0)
			{
            	*pBuffer = 0;
				if (m_nValidBytes < 0)
				{
					m_nLastError = errno;
					return (-1);
				}
				return (lNumRead);
			}
			m_nValidBytes = -m_nValidBytes;
			m_pBufPtr = m_pBuffer;
		}

		// Found a CR without a LF directly following.  Truncate and bail.
		if (bFoundCR && *m_pBufPtr != '\n')
		{
			pBuffer[-1] = 0;
			return (lNumRead);
		}

		*pBuffer = *m_pBufPtr++;
		m_nValidBytes++;
		lNumRead++;

		if (*pBuffer == '\r')
			bFoundCR = TRUE;
		else
		{
			if (*pBuffer == '\n')
			{
				if (bFoundCR)
					pBuffer[-1] = 0;
				*pBuffer = 0;
				break;
			}
			bFoundCR = FALSE;
		}
	}

	return (lNumRead);
}


////////////////////////////////////////////////////////////////////////
// EnsureNewline [public]
//
// Formerly returned -1 on error, else returned 1 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::EnsureNewline(void)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);
	
	// I know, I know... If there was a newline at the end of the buffer when
	// it got flushed we're going to write an extra newline.  But hey, the
	// name of the function is EnsureNewline, not EnsureOneNewline.
	if (!m_nValidBytes || m_pBuffer[m_nValidBytes - 1] != '\n')
		return PutLine();
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// PutLine [public]
//
// Write caller-provided data (which may already include CRLF), then
// forcibly add another CRLF.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::PutLine(const char* pBuffer /*= NULL*/, long lNumBytesToWrite /*= -1L*/)
{
	HRESULT hrWrite = Write_(pBuffer, lNumBytesToWrite);
	if (FAILED(hrWrite))
		return hrWrite;

	return Write_("\r\n", 2); 
}


////////////////////////////////////////////////////////////////////////
// Write_ [private]
//
// Formerly returned -1 on error, else the number of bytes actually copied
// to the internal buffer. 
//
// Does buffered write.  Copies data from source buffer into an internal
// buffer.  While the source buffer contains more data than the
// internal buffer can hold, then write the internal buffer out and
// keep going until the source buffer is emptied.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Write_(const char* pBuffer, long lNumBytesToWrite)
{
	ASSERT(this);
	ASSERT(IsOpen() == S_OK);
	
	if (NULL == pBuffer)
		return S_OK;		// successfully wrote nothing :-)
	
	long count = 0;

	if (lNumBytesToWrite < 0)
		lNumBytesToWrite = strlen(pBuffer);
	while (lNumBytesToWrite)
	{
		int nBytesToCopy = int(min(lNumBytesToWrite, long(m_nSize - m_nValidBytes)));
		memcpy(m_pBuffer + m_nValidBytes, pBuffer, nBytesToCopy);
		m_nValidBytes += nBytesToCopy;
		pBuffer += nBytesToCopy;
		lNumBytesToWrite -= nBytesToCopy;
		count += nBytesToCopy;
		if (lNumBytesToWrite >= m_nSize - m_nValidBytes)
		{
			int			iBytesWritten = ::write(m_fd, m_pBuffer, m_nValidBytes);
			if (iBytesWritten < m_nValidBytes && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
			{
				if (m_bIsIn)
				{
					char		szLogBuf[256];
					sprintf(szLogBuf, "LOGNULL JJFileMT::Write_() %s, Write %d bytes, %d actually written", (LPCTSTR)m_strFName, m_nValidBytes, iBytesWritten);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
				}

				m_nLastError = errno;
				return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_WRITE);
			}
			m_nValidBytes = 0;
		}
	}
	
	return S_OK;
}


