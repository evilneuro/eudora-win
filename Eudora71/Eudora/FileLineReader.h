// FileLineReader.h
//
// Class for MIME parsing input that reads a line at a time from a file

#ifndef _FileLineReader_h_
#define _FileLineReader_h_

#include "LineReader.h"

class JJFileMT;

class FileLineReader : public LineReader
{
public:
	FileLineReader() : m_nLinesRead(0), m_bEscapePressed(false)
		{ m_File = NULL; }
	void SetFile(JJFileMT* curFile)
		{ m_File = curFile; }
	bool IsEscapePressed()
		{ return m_bEscapePressed; }
	virtual long ReadLine(char* buf, LONG bSize);

protected:
	JJFileMT*	m_File;
	int			m_nLinesRead;
	bool		m_bEscapePressed;
};


class BufferLineReader : public LineReader
{
public: 
	BufferLineReader(long nMaxSize) : m_pFloatingBuffer(0), m_bEscapePressed(false),
										m_nCurrentPos(0),  m_nMaxSize(nMaxSize)
		{ ; }
	void SetFloatingBuffer(char* pBuffer)
		{ m_pFloatingBuffer = pBuffer; }
	bool IsEscapePressed()
		{ return m_bEscapePressed; }
	virtual long ReadLine(char* buf, LONG bSize);
protected:
	char * m_pFloatingBuffer;
    long m_nCurrentPos;
	long m_nMaxSize;
	bool m_bEscapePressed;
};


#endif // #ifndef _FileLineReader_h_
