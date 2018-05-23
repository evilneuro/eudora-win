// MultiReader.h
//
// MIMEReader class for reading in multipart/* MIME parts

#ifndef _MultiReader_h_
#define _MultiReader_h_

#include "MIMEReader.h"

class MultiReader : public MIMEReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize);
};

#endif // #ifndef _MultiReader_h_
