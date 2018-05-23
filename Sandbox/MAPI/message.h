////////////////////////////////////////////////////////////////////////
//
// CMapiMessage
//
// Intelligent C++ object wrapper for dumb C-based MapiMessage structure.
//
////////////////////////////////////////////////////////////////////////

#include "..\Eudora\eumapi.h"

class CMapiMessage : public MapiMessage
{
	friend class CMapiSessionMgr;

public:
	~CMapiMessage(void);

	BOOL SetOriginatorName(const char* pName);
	BOOL ReadMessageData(const CString& messageData);
	BOOL WriteMessageData(CString& messageData, BOOL wantAutoSend, BOOL useShortFilenames);

private:
	//
	// Privatized constructor to limit creators to friends.
	//
	CMapiMessage(void);

	CMapiMessage(const CMapiMessage&);		// not implemented
	void operator=(const CMapiMessage&);	// not implemented
};

