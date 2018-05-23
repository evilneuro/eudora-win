// JJFILE.H
//
// File utility routines
//

#ifndef __JJFILE_H__
#define __JJFILE_H__

#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

//FORNOWconst int kGenericDosError = 37;

// Data Types


class AFX_EXT_CLASS JJFileMT
{
//FORNOW	friend class CFileIO;
//FORNOW	friend class FileLineReader;
public:
	JJFileMT(char* buf, int size/*FORNOW, short ErrFlag = TRUE*/);
	JJFileMT(int _size = BUF_SIZE/*FORNOW, short ErrFlag = TRUE*/);
	virtual ~JJFileMT();

	enum
	{
		BUF_SIZE = 4096
	};

	virtual HRESULT Open(const char* filename, int mode, long tries=5L, long interval=250L);
	HRESULT Close();
	virtual HRESULT Flush();
	virtual HRESULT Seek(long lOffset, int nMode = SEEK_SET, long* plNewOffset = NULL);
	HRESULT Tell(long* plOffset);
	HRESULT Reset();
	
	HRESULT Stat(struct stat* sptr);

//FORNOW	void SetErrorReporting(BOOL NewValue)
//FORNOW		{ PostError = NewValue; }

	HRESULT IsOpen() const
		{ return (m_fd == -1 ? S_FALSE : S_OK); }
	int GetLastError_() const
		{ return m_nLastError; }
	HRESULT GetFName(BSTR* pBStr) const;
	HRESULT GetFSize(long* pFSize) const;

	//FORNOW	int Copy(long Start, long End, long To);

    // These are used for fast performance with the entire file
	HRESULT GetBuf(char** ppBuffer);
	HRESULT GetBufSize(int* pnSize) const;
	HRESULT GetNextBlock(long* plNumBytes);

	virtual HRESULT Delete();
	virtual HRESULT Rename(const char* NewName, BOOL bReplace = TRUE);
	virtual HRESULT JJBlockMove(long Start, long Length, JJFileMT *Target);
	HRESULT ChangeSize(long length);
	HRESULT Truncate(long lOffset);

	HRESULT Get(char* value)
		{ return (Read(value, sizeof(char))); }
	HRESULT Get(short* value)
		{ return (Read((char*)value, sizeof(short))); }
	HRESULT Get(BOOL* value)
		{ return (Read((char*)value, sizeof(BOOL))); }
	HRESULT Get(unsigned short* value)
		{ return (Read((char*)value, sizeof(unsigned short))); }
	HRESULT Get(long* value)
		{ return (Read((char*)value, sizeof(long))); }
	HRESULT Get(unsigned long* value)
		{ return (Read((char*)value, sizeof(unsigned long))); }
	HRESULT GetLine(char* pBuffer, long lBufferSize, long* plNumBytesRead = NULL);

	const char* GetFileName() { return (LPCTSTR)m_strFName; }

	virtual HRESULT Read(char* buf, long size);
	HRESULT RawRead(char* pBuffer, int nSize, long* plNumBytesRead = NULL);
		
	HRESULT Put(char value)
		{ return (Write_((char*)&value, sizeof(char))); }
	HRESULT Put(short value)
		{ return (Write_((char*)&value, sizeof(short))); }
	HRESULT Put(BOOL value)
		{ return (Write_((char*)&value, sizeof(BOOL))); }
	HRESULT Put(unsigned short value)
		{ return (Write_((char*)&value, sizeof(unsigned short))); }
	HRESULT Put(long value)
		{ return (Write_((char*)&value, sizeof(long))); }
	HRESULT Put(unsigned long value)
		{ return (Write_((char*)&value, sizeof(unsigned long))); }
	HRESULT Put(const char* pBuffer = NULL, long lNumBytesToWrite = -1L)
		{ return (Write_(pBuffer, lNumBytesToWrite)); }

	HRESULT PutLine(const char* pBuffer = NULL, long lNumBytesToWrite = -1L);

	HRESULT EnsureNewline(void);

protected:		
	// ErrorMessages
//FORNOW    void WriteError_();
//FORNOW	void ReadError_();
//FORNOW	void ReportError_(const char* Message);
	long ReadLine_(char* buf, long size);
	virtual HRESULT Write_(const char* pBuffer, long lNumBytesToWrite);

private:
	// Data members
	int m_fd;				// Descriptor
	int m_nSize;			// Buffer size
	char* m_pBuffer;		// Buffer
	char* m_pBufPtr;		// Pointer to current spot in buffer
	int m_nValidBytes;		// Valid # of bytes left in the buffer
							// It is negative if last read, positive if write
	BOOL m_bIsOurBuffer;	// Did we create the buffer?
	CString m_strFName;		// Filename -- we need this to delete a file we created
	int m_nLastError;		// cached value of ::errno
	bool m_bIsIn;			// Flag to indicate if this in in.mbx
//FORNOW	BOOL PostError;	// ErrorMessage Flag
//FORNOW	CCursor cur;	// Have wait cursor on during entire lifetime of this object
};




#endif // __JJFILE_H__
