// SingleReader.h
//
// MIMEReader class for reading in application/applefile MIME parts

#ifndef _SingleReader_h_
#define _SingleReader_h_

#include "MIMEReader.h"

class SingleReader : public MIMEReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize);
};

#endif // #ifndef _SingleReader_h_
