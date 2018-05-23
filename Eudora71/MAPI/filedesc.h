////////////////////////////////////////////////////////////////////////
//
// CMapiFileDesc
//
// Intelligent C++ object wrapper for dumb C-based MapiFileDesc structure.
//
////////////////////////////////////////////////////////////////////////

#include "..\Eudora\eumapi.h"

class CMapiFileDesc : public MapiFileDesc
{
	friend class CMapiMessage;

public:
	~CMapiFileDesc(void);
	BOOL SetFileName(const CString& fileName);
	BOOL SetPathName(const CString& pathName);

private:
	//
	// Privatized constructor to limit creators to friends.
	//
	CMapiFileDesc(void);

	CMapiFileDesc(const CMapiFileDesc&);	// not implemented
	void operator=(const CMapiFileDesc&);	// not implemented
};

