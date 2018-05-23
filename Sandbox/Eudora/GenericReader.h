// GenericReader.h
//
// MIMEReader class for reading in unknown MIME parts

#ifndef _GenericReader_h_
#define _GenericReader_h_

#include "MIMEReader.h"

class GenericReader : public MIMEReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize);
};

#endif // #ifndef _GenericReader_h_
