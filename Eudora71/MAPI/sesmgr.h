////////////////////////////////////////////////////////////////////////
//
// CMapiSessionMgr
//
// Handles MAPI sessions and memory management for multiple MAPI 
// client applications.
//
////////////////////////////////////////////////////////////////////////

#include "..\Eudora\eumapi.h"

class CMapiMessage;
class CMapiRecipDesc;



class CMapiSessionMgr
{
public:
	CMapiSessionMgr(void);
	~CMapiSessionMgr(void);

	//
	// Session management.
	//
	LHANDLE CreateSession(void);
	BOOL DestroySession(LHANDLE sessionId);
	BOOL IsValidSessionId(LHANDLE sessionId);

	//
	// Memory management.
	//
	CMapiMessage* NewMapiMessage(void);
	CMapiRecipDesc* NewMapiRecipDescArray(ULONG numElements);
	BOOL FreeBuffer(void* pBuffer);

private:
	CMapiSessionMgr(const CMapiSessionMgr&);	// not implemented
	void operator=(const CMapiSessionMgr&);		// not implemented

	LHANDLE m_NextAvailableSessionID;
	CPtrList m_SessionList;		// LHANDLE cast to void*

	CObList m_BufferList;
};

