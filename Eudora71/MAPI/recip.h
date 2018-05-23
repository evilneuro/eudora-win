////////////////////////////////////////////////////////////////////////
//
// CMapiRecipDesc
//
// Intelligent C++ object wrapper for dumb C-based MapiRecipDesc structure.
//
////////////////////////////////////////////////////////////////////////

#include "..\Eudora\eumapi.h"

class CMapiRecipDesc : public MapiRecipDesc
{
	friend class CMapiSessionMgr;
	friend class CMapiMessage;

public:
	~CMapiRecipDesc(void);

	BOOL SetClass(ULONG recipClass);
	BOOL SetName(const char* pName);
	BOOL SetAddress(const char* pAddress);
	BOOL SetNameAndAddress(const char* pName, const char* pAddress);
	BOOL ResolveName();

private:
	//
	// Privatized constructor to limit creators to friends.
	//
	CMapiRecipDesc(void);

	CMapiRecipDesc(const CMapiRecipDesc&);	// not implemented
	void operator=(const CMapiRecipDesc&);	// not implemented
};


// Handy little routine for combining a lpszName and a lpszAddress from
// a MapiRecipDesc structure.  Note that since CMapiRecipDesc is derived
// from MapiRecipDesc, you can use one of those as well.
CString MakeFullAddress(MapiRecipDesc* RecipDesc);