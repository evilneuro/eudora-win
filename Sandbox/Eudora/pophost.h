#ifndef _POP_HOST_
#define _POP_HOST_

//Forward declarations
class JJFile;
class CMsgRecord;
class CPOPSession;

#include "MsgRecord.h"  //due to some weird template dependency ..hate MS

#include "afxmt.h"

enum { kLMOSBuffSize	= 4000};


#include <xstddef>
#pragma warning (disable: 4663 4244)
#include <algorithm>
#include <map>
#pragma warning (default: 4663 4244)

struct CompareMsgRecord 
{
		bool operator()(const CMsgRecord *left, const CMsgRecord *right) const
		{
			return (left->GetMsgNum() - right->GetMsgNum())?true:false;
		}
};


class CPopHost //: public CObject
{
	friend class CLMOSRecord;  //for access to locking and unlocking
public:
	CPopHost();
	CPopHost(CString Persona, CString strCurrentAccount);  //Thread-safe constructor
	//CPopHost(CString strLMOSEntryName);  //Thread-safe constructor given LMOS entry name
	~CPopHost();    
	
	BOOL CanUIDL() const								{ return m_bCanUIDL; }
	void SetCanUIDL(BOOL bCanUIDL);

	bool SetAccountNameAndLastPurge(char* pszBuffer);

	//The return value is true if messages got deleted
	//BOOL PurgeMessages(unsigned long bitflags);
	BOOL PurgeMessages(CPOPSession *pop, unsigned long ulBitFlags, BOOL bLeaveMailOnServer, 
							BOOL bDeleteMailFromServer, unsigned long ulPurgePeriod);

	
	bool CPopHost::IsMe(const CString& Persona) const { return (m_strPersona.CompareNoCase(Persona) == 0)?true:false;}

	//BOOL IsMe(const char *) const;   
	//BOOL IsMe(const CString& Persona, CString acct) const; 
    
	long GetLastPurge() const;
	void SetLastPurge(const char* pszLastPurge);
	void GetLastPurgeString(char *) const;

	//CMsgNumList* GetMsgNumList(){return m_pMsgListByMsgNum;}
	int GetCount();
	void GetTransferOptions(BOOL& SendingFlag, BOOL& DeleteMarkedExist, BOOL& DeleteMarked, 
							 BOOL& RetrieveMarkedExist, BOOL& RetrieveMarked );

	
	void GetAccountName(char* pszAccountName) const		{strcpy(pszAccountName, m_szAccountName);}
	void SetAccountName(const char* pszAccountName)		{strcpy(m_szAccountName, pszAccountName);}
	
	CMsgRecord* GetMsgByHash(unsigned long ulHash);
	CMsgRecord* GetMsgByIDString(const char* pszMessageId, BOOL bCanUIDl);
	CMsgRecord* GetMsgByMsgNum(int);
	
	CMsgRecord* GetNextFetchableMsgNum(unsigned long ulBitFlags, BOOL bFirstTime = FALSE);
	
	void InsertMsgRecord(CMsgRecord *); 
	void UpdateHashValue(CMsgRecord *pMsgRecord, unsigned long ulHash);	
	BOOL ReadLMOSData(JJFile* pLMOSFile);


	//Load data from binary file(new format)
	bool LoadLMOSData();
	bool LoadLMOSHeader(CFile *pLMOSFile);


	//Write out LMOS Data
	bool WriteLMOSHeader(CFile* pLMOSFile);
	bool WriteLMOSData();

	//void ReplaceHost(CPopHost *pServerHost);
	
	
	void ConditionMessagesPriorTo(int nFirstUnread, bool bDeleteFromServer);
	void PadMsgList(int nMsgCount);
	void RemoveDeletedMsgRecords();
	BOOL ReconcileLists_(CPopHost* pDiskHost, CPopHost* pServerHost);

	//Only used for Non-UIDL proceesing
	CMsgRecord *FindHighLowIndices(int& HighIndex, int& LowIndex, int nLast);
	void ReplaceDeleteSentWithDelete();

private:
	//
	// The 'm_sServerType' value is read and written to the LMOS file
	// for informational purposes only.  It is not used at runtime since
	// we determine the UIDL capabilities of the server on-the-fly and
	// set the 'm_bCanUIDL' flag accordingly.
	//
	short			m_sServerType;			// kUIDL or kNonUIDL
	char			m_szLastPurge[256];		// When last purged
	
	//CMsgNumList*	m_pMsgListByMsgNum;		// List of message ID's, sorted by message number
	
	char			m_szAccountName[128+1];	// account name
	CString			m_strPersona;
	//POSITION		m_posLastMsg;
	BOOL			m_bCanUIDL;				// TRUE if server supports optional UIDL command

	CString m_strLMOSFileName;
	CCriticalSection m_LMOSGuard;

	void Lock(){ m_LMOSGuard.Lock(); }
	void Unlock(){ m_LMOSGuard.Unlock(); }

		
	//void InsertMsgRecord(CMsgRecord *);
	//bool WriteLMOSData(CFile* pLMOSFile);
	
	
private:
	//std::set<CMsgRecord *, CompareMsgRecord> m_MsgRecords;
	
	std::map<unsigned long, CMsgRecord *> m_MsgRecords;
	typedef std::map<unsigned long, CMsgRecord *>::iterator SI;
	typedef std::map<unsigned long, CMsgRecord *>::const_iterator CSI;
	typedef std::map<unsigned long, CMsgRecord *>::value_type MapValueType;

	SI m_LastIterator;
    
}; 




#endif