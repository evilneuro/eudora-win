#ifndef _MSG_RECORD_
#define _MSG_RECORD_

#include "afxmt.h"  //for CCriticalSection
//forwward declarations
class JJFile;  
class CPOPSession;


// Constants
//const	short	kMaxRecordString = 100;
//const	short	kUIDL				= 1;		// Using UIDL type message ID's
//const	short	kNonUIDL			= 0;		// Using some other messag ID... 
//const	short	kFromFile			= 2;		// Using data from a file....


enum { kMaxRecordString = 100 };
enum { kNonUIDL =0,  kUIDL=1, kFromFile = 2};

enum {	LMOS_RECORD_SIZE	= 256,
		LMOS_UIDL_SIZE		= 70,
		LMOS_MSGID_SIZE		= 128,
		LMOS_DATE_SIZE		= 4,     //16,
		LMOS_HASH_SIZE		= 4,
		LMOS_FLAGS_SIZE     = 4,
		LMOS_ESTSIZE_SIZE   = 4,
		LMOS_4_BYTES		= 4,

		LMOS_HEADER_SIZE	= 256,
		LMOS_POPACCT_SIZE	= 128,
		LMOS_PURGEDATE_SIZE = 4,
		LMOS_VERSION_SIZE	= 4,
		LMOS_SERVTYPE_SIZE	= 2
};


enum {	LMOS_POPACCT_OFFSET   = 4,
		LMOS_PURGEDATE_OFFSET = 132,	//LMOS_POPACCT_OFFSET + LMOS_POPACCT_SIZE,
		LMOS_SERVTYPE_OFFSET  = 144		//LMOS_PURGEDATE_OFFSET +  LMOS_PURGEDATE_SIZE 
};


enum LMOS_OFFSET {	LMOS_UIDL_OFFSET   = 0,
					LMOS_MSGID_OFFSET  = 70,		//LMOS_UIDL_OFFSET  + LMOS_UIDL_SIZE,  
					LMOS_DATE_OFFSET   = 198,		//LMOS_MSGID_OFFSET + LMOS_MSGID_SIZE,
					LMOS_FLAGS_OFFSET  = 202,   //214,	//LMOS_DATE_OFFSET  + LMOS_DATE_SIZE,
					LMOS_HASH_OFFSET   = 206,	//218,	//LMOS_FLAGS_OFFSET + LMOS_4_BYTES,
					LMOS_ESTSIZE_OFFSET= 210,	//222	//LMOS_HASH_OFFSET  + LMOS_4_BYTES
};


enum LMOS_DELETE_FLAG	{	LMOS_DELETE_MESSAGE		=0x00000001,	// 0 del
							LMOS_DONOT_DELETE		=0x00000002,	// 1 Ndel		
							LMOS_DELETE_SENT		=0x00000004	};	//-1 delS
							
enum LMOS_READ_FLAG		{	LMOS_HASBEEN_READ		=0x00000008,	// 0 read
							LMOS_NOT_READ			=0x00000010,	// 1 NRead		
						LMOS_READ_PROGRAMMATICALLY	=0x00000020	};  //-1 readP

enum LMOS_RETRIEVE_FLAG	{	LMOS_RETRIEVE_MESSAGE	=0x00000040,	// 0 get							
							LMOS_DONOT_RETRIEVE		=0x00000080	};	// 1 Nget
							
enum LMOS_SKIP_FLAG		{	LMOS_SKIP_MESSAGE		=0x00000100,	// 0 skip
							LMOS_DONOT_SKIP			=0x00000200,	// 1 Nskip
							LMOS_SKIP_WHAT			=0x00000400}; 	//-1 skipP never used
							
enum LMOS_SAVE_FLAG		{	LMOS_SAVE_MESSAGE		=0x00000800,	// 0 save
							LMOS_DONOT_SAVE			=0x00001000	}; 	// 1 Nsave


enum MsgFetchType
{
		MSGFETCH_NOTHING	= 0,
		MSGFETCH_HEADER		= 1,
		MSGFETCH_STUB		= 2,
		MSGFETCH_ENTIRE		= 3
};



class CMsgRecord //: public CObject
{

public:
	CMsgRecord();
	CMsgRecord(unsigned long msgnum, const char *uidl, const char *msgid, 
		unsigned long ulDate, unsigned long flags, unsigned long hash, unsigned long est_size);
	
	CMsgRecord(const char *, const char *, const char *, const char *, const char *, const char *, 
		const char *, const char *, const char *,unsigned long);
	virtual ~CMsgRecord();
	
	//
	// Globally available CMsgRecord factory.
	//
	//global s_ulMessageIndex is removed and passed as a default argument
	static CMsgRecord* CreateMsgRecord(CString strPersona, int nType, char* pszBuffer, int nBufferLen, int *pnMessageNum=NULL);
	static CMsgRecord* CreateMsgRecordFromFile(CString strPersona, char* pszBuffer, int nBufferLen, int *pnMessageNum=NULL);

	

	void			GetUIDLID(char *theString);
	void			SetUIDLID(const char* pszUIDLID);
	BOOL			IsEqualUIDLID(const char *);
	
	void			GetMesgID(char *theString);
	void			SetMesgID(const char *theString);
	BOOL			IsEqualMesgID(const char *);
	
	unsigned long	GetDateLong();
	void			GetDate(char *theString) const;
	void			SetDate(const char *theString);
	
	void			SetHashString(const char *, CString strPersona);
	void			GetHashString(char *) const;

	//uses Intelocked functions
	unsigned long	GetHashValue() const				{return m_ulHashValue;}
	void			SetHashValue(unsigned long hashVal)	{ InterlockedExchange((long*)&m_ulHashValue, hashVal);}


	long			GetEstimatedSize(){ return m_ulEstimatedSize; }
	void			SetEstimatedSize(unsigned long size){ InterlockedExchange((long*)&m_ulEstimatedSize,size); }

	int				GetMsgNum() const					{ return (m_nMsgNum);}
	void			SetMsgNum(int num)					{ InterlockedExchange((long*)&m_nMsgNum, num);}
	void			SetMsgNum(const char *theString)	{ SetMsgNum(atoi(theString));}

	
	
	LMOS_DELETE_FLAG	GetDeleteFlag() const				{return m_sDeleteFlag;}
	void				SetDeleteFlag(LMOS_DELETE_FLAG deleteThis, bool bIgnoreDeleteSent =false);
	
	LMOS_SKIP_FLAG		GetSkippedFlag() const				{return m_sSkippedFlag;}
	void				SetSkippedFlag(LMOS_SKIP_FLAG skipThis) 	{InterlockedExchange((long*)&m_sSkippedFlag, skipThis);}
	
	LMOS_SAVE_FLAG		GetSaveFlag() const					{return m_sSaveFlag;} 
	void				SetSaveFlag(LMOS_SAVE_FLAG saveThis) 		{InterlockedExchange((long*)&m_sSaveFlag, saveThis);}
	
	LMOS_READ_FLAG		GetReadFlag() const					{return m_sReadFlag;} 
	void				SetReadFlag(LMOS_READ_FLAG readThis) 		{ InterlockedExchange((long*)&m_sReadFlag, readThis);}
	
	LMOS_RETRIEVE_FLAG	GetRetrieveFlag() const				{return m_sRetrieveFlag;} 
	void				SetRetrieveFlag(LMOS_RETRIEVE_FLAG getThis) { InterlockedExchange((long*)&m_sRetrieveFlag, getThis);}
	
	unsigned long GetFlags() const { return (m_sDeleteFlag|m_sSkippedFlag|m_sSaveFlag|m_sReadFlag|m_sRetrieveFlag); }


	BOOL			PurgeThisMessage(CPOPSession *, unsigned long);
	
	MsgFetchType	GetMsgFetchType(unsigned long ulBitFlags) const;
                                                
	bool			WriteLMOSData(CFile *);

	void DebugDump(char *buf, int len);
	//void UpdateWithDiskInfo(CMsgRecord *pDiskMsg);

private:
	char			m_szUIDLID[LMOS_UIDL_SIZE+1];	
	char			m_szMesgID[LMOS_MSGID_SIZE+1];	
	//char			m_szDate[LMOS_DATE_SIZE+1];
	unsigned long m_ulDate;
	int				m_nMsgNum;
	unsigned long	m_ulHashValue;
	unsigned long			m_ulEstimatedSize;
	LMOS_DELETE_FLAG		m_sDeleteFlag;
	LMOS_SKIP_FLAG			m_sSkippedFlag;
	LMOS_SAVE_FLAG			m_sSaveFlag;
	LMOS_READ_FLAG			m_sReadFlag;
	LMOS_RETRIEVE_FLAG		m_sRetrieveFlag;

	CCriticalSection m_Guard;
};






class CPopHost;
class CSummary;

class CLMOSRecord 
{
public:
	CLMOSRecord(CSummary *);
	CLMOSRecord(CPopHost *pHost, unsigned long ulHash);
	~CLMOSRecord();
	
	CMsgRecord *GetMsgRecord(){ return m_pMsgRecord; }
	void WriteLMOS();

private:
	
	CMsgRecord *m_pMsgRecord;
	CPopHost *m_pPopHost;	
};


#endif

