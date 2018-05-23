#ifndef __DDESERVE_H__
#define __DDESERVE_H__
////////////////////////////////////////////////////////////////////////
//
// CDDEServer
// Class instantiated and used by DDE server side.
//
////////////////////////////////////////////////////////////////////////

#include <ddeml.h>

class CDDEString;

//
// Required prototype for external GetReturnAddress() function.  In
// Eudora, this function is implemented in ADDRESS.H, ADDRESS.CPP.
//
const char* GetReturnAddress();

class CDDEServer
{
public:
	CDDEServer(void);
	~CDDEServer(void);

    BOOL Initialize(void);
    void Uninitialize(void);

	//
	// DDE transaction handlers.
	//
	BOOL OnConnect(
		HSZ hszTopic,
		HSZ hszService,
		CONVCONTEXT* pContext,
		BOOL isSameInstance);
	HDDEDATA OnWildConnect(
		UINT wFmt,
		HSZ hszTopic,
		HSZ hszService,
		CONVCONTEXT* pContext,
		BOOL isSameInstance);
	void OnConnectConfirm(
		HCONV hConv,
		HSZ hszTopic,
		HSZ hszService,
		BOOL isSameInstance);
    void OnDisconnect(BOOL isSameInst);
	HDDEDATA OnRequest(
		UINT wFmt,
		HSZ hszTopic,
		HSZ hszService);
	void OnRegister(
		HSZ hszBaseServiceName,
		HSZ hszInstanceServiceName);
	void OnUnregister(
		HSZ hszBaseServiceName,
		HSZ hszInstanceServiceName);

private:
	CDDEServer(const CDDEServer&);		// not implemented
	void operator=(const CDDEServer&);	// not implemented

#ifdef WIN32
	static HDDEDATA CALLBACK DDEServerCallback
#else
	static HDDEDATA _export CALLBACK DDEServerCallback
#endif // WIN32
	(
		UINT wType, 
		UINT wFmt, 
		HCONV hConv,
		HSZ hsz1, 
		HSZ hsz2, 
		HDDEDATA hDDEData, 
		DWORD dwData1, 
		DWORD dwData2
	);

	BOOL ProcessLongParam(CString& paramList, unsigned long& longVal) const;

	DWORD m_InstId;
    static CDDEServer* m_pTheServer;
    PFNCALLBACK m_pCallbackFn;

	CDDEString* m_pServiceName;
	CDDEString* m_pSystemTopic;
	CDDEString* m_pWWWOpenURLTopic;
	CDDEString* m_pMAPITopic;
	CDDEString* m_pExpandNicknameTopic;
	CDDEString* m_pGetNextMessageIdTopic;
	CDDEString* m_pGetMessageByIdTopic;
	CDDEString* m_pPutMessageByIdTopic;
	CDDEString* m_pDeleteMessageByIdTopic;
	CDDEString* m_pMAPIOriginatorItem;
	CDDEString* m_pMAPINicknamesItem;
	CDDEString* m_pMAPIServerVersionItem;
};

#endif // __DDESERVE_H__



