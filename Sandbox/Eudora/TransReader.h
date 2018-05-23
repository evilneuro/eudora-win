// TransReader.h
//
// MIMEReader classes for handling EMSAPI translations

#ifndef _TransReader_h_
#define _TransReader_h_

#include "MIMEReader.h"

class HeaderDesc;
class MIMEState;
class JJFile;

//==============================================================================
class TransReader : public MIMEReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize) = 0;

protected:
	BoundaryType ReadTL(CObArray& MimeStates, char* buf, LONG bSize, JJFile* file);
	BOOL WriteHeaders(HeaderDesc* hd, JJFile* file);
	CString GetTransIDString(MIMEState* ms);
};

//==============================================================================
class NowTransReader : public TransReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize);
};

//==============================================================================
class NotNowTransReader : public TransReader
{
public:
	BoundaryType ReadIt(CObArray& MimeStates, char* buf, LONG bSize);
};

#endif // #ifndef _TransReader_h_
