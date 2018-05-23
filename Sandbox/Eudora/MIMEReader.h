// MIMEReader.h
//
// Virtual base class for reading MIME parts

#ifndef _MIMEReader_h_
#define _MIMEReader_h_

typedef enum
{
	btNotBoundary,
	btInnerBoundary,
	btOuterBoundary,
	btEndOfMessage,
	btError
} BoundaryType;


class MIMEReader
{
public:
	virtual ~MIMEReader() {}
	virtual BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize) = 0;
};

#endif // #ifndef _MIMEReader_h_
