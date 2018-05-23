// LineReader.h
//
// Abstract base class for MIME parsing input that reads a line at a time

#ifndef _LineReader_h_
#define _LineReader_h_

class LineReader
{
public:
	virtual ~LineReader() {}
	virtual long ReadLine(char* buf, LONG bSize) = 0;
};

#endif // #ifndef _LineReader_h_
