// TextReader.h
//
// MIMEReader class for reading in text/* MIME parts

#ifndef _TextReader_h_
#define _TextReader_h_

#include "MIMEReader.h"

class TextReader : public MIMEReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize);

	LONG StripRich(LPTSTR buf, LONG size);
	LONG StripHtml(LPTSTR buf, LONG size);
};

#endif // #ifndef _TextReader_h_
