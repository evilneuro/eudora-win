#ifndef __DDECLIEN_H__
#define __DDECLIEN_H__
////////////////////////////////////////////////////////////////////////
//
// CDDEClient
// Class instantiated and used by DDE client side.
//
////////////////////////////////////////////////////////////////////////

#include <ddeml.h>

class CDDEClient
{
public:
	CDDEClient(void);
	~CDDEClient(void);

	DWORD QueryServers(const char* pServiceName, const char* pTopicName);
	BOOL RegisterNetscapeProtocol(const char* pProtocol, BOOL isRegister = TRUE);
	BOOL OpenNetscapeURL(const char* pCmd);
	BOOL GetEudoraOriginator(CString& originatorAddress);
	BOOL GetEudoraNicknames(CString& nickNames);
	BOOL ExpandEudoraNickname(const CString& nickName, CString& expandedNames);
	BOOL GetNextMessageId(CString& messageId, BOOL sortByDate, BOOL unreadOnly);
	BOOL GetMessageById(CString& messageData, const CString& messageId,
						BOOL bodyAsFile, BOOL envelopeOnly,
						BOOL markAsRead, BOOL wantAttachments);
	BOOL PutMessageById(const CString& messageData, CString& messageId);
	BOOL DeleteMessageById(const CString& messageId);
	BOOL GetEudoraMAPIServerVersion(CString& versionInfo);

private:
	CDDEClient(const CDDEClient&);		// not implemented
	void operator=(const CDDEClient&);	// not implemented

#ifdef WIN32
	static HDDEDATA CALLBACK DDECallback
#else
	static HDDEDATA _export CALLBACK DDECallback
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

	BOOL DoLogon_(void);
	void DoLogoff_(void);

	DWORD m_InstId;
};

#endif // __DDECLIEN_H__
