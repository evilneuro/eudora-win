// MultiReader.h
//
// MIMEReader class for reading in multipart/* MIME parts

#ifndef _MultiReader_h_
#define _MultiReader_h_

#include "MIMEReader.h"

class MultiReader : public MIMEReader
{
public:
	MultiReader();
	
	BoundaryType	ReadIt(CObArray& MimeStates, char* buf, LONG bSize);
	void			SetExplodingDigest(){ m_bExplodeDigest = true;};
	void			ResetExplodingDigest() { m_bExplodeDigest = false;} ;

private:
	bool m_bExplodeDigest;
};

#endif // #ifndef _MultiReader_h_
