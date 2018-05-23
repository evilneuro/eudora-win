// JJFILE.CPP
//
// File utility routines
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


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

JJFileMT::JJFileMT(int nSize /*= BUF_SIZE*/)
{
	ASSERT(this);
	
	m_pBuffer = new char[nSize];
	m_bIsOurBuffer = TRUE;
	m_nSize = nSize;
	m_pBufPtr = m_pBuffer;
	m_nValidBytes = 0;
	m_fd = -1;
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

#ifdef _DEBUG
	if( m_pBuffer)
	{
		fBaptizeBlockMT(m_pBuffer, filename);
	}
#endif
		
	
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

#ifndef WIN32
	AnsiToOem(StrippedName, StrippedName);
#endif

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
#ifndef unix
	m_fd = open(StrippedName, (mode & ~O_APPEND) | O_BINARY, S_IWRITE);
#else
	m_fd = open(StrippedName, (mode & ~O_APPEND) | O_BINARY, S_IREAD | S_IWRITE);
#endif
	if (m_fd < 0)
	{
		if (errno==13 && --tries>0)	// 13 is access error, supposedly.
		{
			Sleep(interval);
			goto retry;	// honestly, this is the cleanest way.  SD 3/11/99
		}

		m_nLastError = errno;

		if (mode & (O_CREAT | O_WRONLY))
			return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_OPEN_WRITING);
		else
			return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_OPEN_READING);
	}
    else
	{
		// _O_APPEND seems to be broken with the Symantec C++ library.
		if (mode & O_APPEND)
			lseek(m_fd, 0L, SEEK_END);
	}
	
	return S_OK;
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
	return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_CLOSE);
}


HRESULT JJFileMT::Reset()
{
	HRESULT hrSeek = Seek(0L, SEEK_SET);
	m_pBufPtr = m_pBuffer;
	m_nValidBytes = 0;
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

	if (m_fd < 0 || ::write(m_fd, m_pBuffer, m_nValidBytes) < m_nValidBytes)
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
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_WRITE);

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
		return E_FAIL;

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
		return E_FAIL;				// error
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
		return hrFlush;

	if ((::_chsize(m_fd, lLength) < 0) ||
		(::_lseek( m_fd, 0, SEEK_END) < 0))
	{
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
	ASSERT(lOffset >= 0);

	HRESULT hr = Seek(lOffset);
	if (FAILED(hr))
		return hr;

	long lNewOffset = 0;
	hr = Tell(&lNewOffset);
	if (FAILED(hr))
		return hr;
	ASSERT(lNewOffset >= 0);

	hr = ChangeSize(lNewOffset);
	if (FAILED(hr))
		return hr;

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
		return hrRemove;

    return S_OK;
}


////////////////////////////////////////////////////////////////////////
// Rename [public, virtual]
//
// Formerly returned -1 on error, >=0 on success.
////////////////////////////////////////////////////////////////////////
HRESULT JJFileMT::Rename(const char* NewName, BOOL bDeleteFirst /*= TRUE*/)
{
	ASSERT(this);
	
	if (IsOpen() == S_OK)
	{
		HRESULT hrClose = Close();
		if (FAILED(hrClose))
			return hrClose;
	} 

	// Make this case insesitive
	m_strFName = strlwr((char *)(const char *)m_strFName);
	NewName = strlwr((char *)NewName);

	char *pszDevice1, *pszDevice2;
	pszDevice1 = strchr(m_strFName, ':');
	pszDevice2 = strchr(NewName, ':');

	if ( pszDevice1 != NULL && pszDevice2 != NULL && *--pszDevice1 == *--pszDevice2 )
	{
		// same drive - we can just rename it
		// Get rid of destination file, otherwise rename will fail
		if (bDeleteFirst)
		{
			HRESULT hrRemove = ::FileRemoveMT(NewName);
			if (FAILED(hrRemove))
				return hrRemove;
		}
		
		HRESULT hrRename = ::FileRenameMT(m_strFName, NewName);
		if (FAILED(hrRename))
			return hrRename;

		return S_OK;
	}

	{
		// copy the file then delete original
		char dummy[1];
		struct stat st;

		JJFileMT fileTarget(dummy, sizeof(dummy));
		if (FAILED(fileTarget.Open(NewName, O_WRONLY | O_CREAT | O_TRUNC)) ||
			::stat(m_strFName, &st) ||
			st.st_size == -1L || 
			FAILED(Open(m_strFName, O_RDONLY)) || 
			FAILED(JJBlockMove(0L, st.st_size, &fileTarget)))
		{
			fileTarget.Delete();
			Close();
			return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_RENAME);
		}

		fileTarget.Close();
	}

	Delete();
	return S_OK;
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
	
	Seek(Start);
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
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_READ);
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
			if (::write(m_fd, m_pBuffer, m_nValidBytes) < m_nValidBytes)
			{
				m_nLastError = errno;
				return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_WRITE);
			}
			m_nValidBytes = 0;
		}
	}
	
	return S_OK;
}


