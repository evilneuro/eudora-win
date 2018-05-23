#ifndef _SPOOL_MGR_H_
#define _SPOOL_MGR_H_

#include "afxmt.h"
#include "persona.h"
#include <list>
using namespace std;

typedef enum {

		//Sending
		SPOOL_TYPE_SEND,
		SPOOL_TYPE_SENT,
		SPOOL_TYPE_UNSENT,
		SPOOL_TYPE_UNSENDABLE,
		
		//Receiving
		SPOOL_TYPE_RECEIVED,
		
		//Removing
		SPOOL_TYPE_REMOVED
				
} SPOOLTYPE;



typedef enum {
		SS_UNKNOWN=0,			//Never come here

		SS_UNSENDABLE,		//SMTP_E_FILE_UNSENDABLE,			// could not 
		SS_SENT,			//SPOOL_S_FILE_SENT,			// Spool file was sent successfully
		SS_UNSENT,			//SPOOL_S_FILE_UNSENT,				// Spool file was not sent because of unknown error.
		SS_SEND,			//SPOOL_S_FILE_SEND,				// Spool file was spooled successfully

		SS_DONE_SEND,		//All of SS_SENT, SSUNSENT, SS_UNSENDABLE


							//SPOOL_S_FILE_UNSENDABLE,			// Spool file was unsendable because of SMTP error.
		SS_RECEIVED,		//SPOOL_S_FILE_RECEIVED,			// Spool file was successfully downloaded from server.
		
		SS_REMOVED,			//SPOOL_FILE_DELETE
		
		//While in transition from state to state
		SS_SENDING_INPROGRESS,
		SS_RECEIVING_INPROGRESS,
		SS_REMOVAL_INPROGRESS

} SPOOLSTATUS;



class CSpoolInfo {
public:
	
	CSpoolInfo(){}
	CSpoolInfo(CString name, SPOOLTYPE st, SPOOLSTATUS ss)
		: m_strFileName(name), m_nSpoolStatus(ss), m_nSpoolType(st){}
		
	//GET
	CString GetFileName();
	SPOOLSTATUS GetSpoolStatus();
	SPOOLTYPE GetSpoolType();
	
	//SET
	void SetSpoolStatus(SPOOLSTATUS status);
	
	virtual int GetUniqueID() = 0;

protected:
	//Protected as derived classes need access to lock'em
	CCriticalSection m_Guard;
	bool SetSpoolTypeByStatus(SPOOLSTATUS spStatus);
	void SetSpoolType(SPOOLTYPE type);

private:
	CString			m_strFileName;
	SPOOLSTATUS		m_nSpoolStatus;
	SPOOLTYPE		m_nSpoolType;
	
};


class CSummary;
class CMsgRecord;
class CTocDoc;

class COutSpoolInfo : public CSpoolInfo {
public:
	
	COutSpoolInfo(CString name, SPOOLTYPE st, SPOOLSTATUS ss,  CSummary *sum);
	CSummary *GetSummary();
	virtual int GetUniqueID();
	CString GetTitle();
	int GetTaskID();
	void SetTaskID(int nTaskID);
	
private:
	int m_UniqueID;
	CSummary *m_pSum;
	CTocDoc  *m_OutToc;
	CString m_strTitle;
	int m_nTaskID;
};



class CInSpoolInfo : public CSpoolInfo {
public:
	
	CInSpoolInfo(CString name, SPOOLTYPE st, SPOOLSTATUS ss,  CMsgRecord *msgRec);
	CMsgRecord *GetMsgRecord();
	virtual int GetUniqueID();
	
private:
	CMsgRecord *m_pMsgRecord;
	int m_Hash;
	
};









/*
#include "afxmt.h"


//BASE Template for a Thread-safe list

template <class T>
class QCSpoolListMT {

public:
	QCSpoolListMT(){}
	void enqueue(T msg);
	bool dequeue(T msg);
	T Get(SPOOLSTATUS spFind, SPOOLSTATUS spNew, int UID=0);
	int GetCount(SPOOLSTATUS status, int UID=0);
					
protected:
	CCriticalSection m_ListGuard;
	CList<T, T> m_List;
};

*/


//A Generic File Finder 
//FindFile Starts the search; 

class QCFileFinder {
	CString m_SearchString;
	bool m_bFirstTime;
	HANDLE m_hContext;
	WIN32_FIND_DATA data;

public:

	QCFileFinder(CString str=""):m_SearchString(str){ m_bFirstTime = true; m_hContext = NULL;}
	bool FindFile(CString str) { m_SearchString = str;  m_bFirstTime = true; return true; }
	
	//Used to traverse to next file and get the results by calling GET functions such as GetFileName
	bool FindNextFile();
	CString GetFileName();

	//Close the searching context and ready for a new one which can be specified by FindFile
	void Close() { m_hContext = NULL; }
};






class QCSpool;
class QCOutSpool;
class QCInSpool;

class QCSpoolMgrMT
{
protected:
	QCSpoolMgrMT(const char* szPersona);
	~QCSpoolMgrMT();
public:
	
	//Factory method for creating SpoolMgr objects
	static QCSpoolMgrMT* CreateSpoolMgr(const char* szPersona);
	
	static HRESULT Init(CPersonality&);
	static HRESULT Shutdown();
	static CString GetLMOSFileName(CString strPersona);

	static CList<QCSpoolMgrMT *, QCSpoolMgrMT *>  m_SpoolMgrs;
	static CString m_strSpoolRootDir;
	static CCriticalSection m_Guard; //m_pLockable;

	
public:
	
	QCOutSpool* m_Out;
	QCInSpool * m_In;
	
	// DeleteSpool();
	CString GetPersona() { return m_strPersona; }
	CString GetSpoolDirectory(){ return m_strSpoolDir; }

private:
 	CString m_strPersona;
	CString m_strSpoolDir;
};







//Thread-safe Spool Class

class QCSpool
{

	friend QCSpoolMgrMT; //static QCSpoolMgrMT* QCSpoolMgrMT::CreateSpoolMgr();
	//typedef  QCSpoolListMT  SpoolInfoListMT;
	
protected:
	QCSpool(const CString& strPersona, SPOOLTYPE m_type, CString szTempFilename, CString szPath);
	~QCSpool(){}
	
public:
	
	virtual void BuildSpoolFromFiles() = 0;

	HRESULT BeginWrite(CString& szFilename);
	//void	EndWrite();
	//virtual HRESULT EndWrite(CString& szFile, unsigned int uniqueID, CSummary *sum) = 0;

	CString GetBasePath() { return m_strBasePath; }
	CString GetExtension() { return m_strExtension; }
	SPOOLTYPE GetSpoolType() { return m_type; }

protected:
	SPOOLTYPE m_type;
	CString m_strTempFile;
	CString m_strExtension;
	CString m_strBasePath;

	//CString m_strMailDir;
	CString m_strPersona;
	
	
	DWORD	m_threadID;
	bool	m_bWrite;
};





class QCOutSpool : public QCSpool
{

	friend QCSpoolMgrMT; 
	
public:
	virtual void BuildSpoolFromFiles();
	void	EndWrite();
	HRESULT EndWrite(CString& szFile, unsigned int uniqueID, CSummary *sum);

	COutSpoolInfo* Get(SPOOLSTATUS spOld, SPOOLSTATUS spNew, int UID=0);
	void AddToSpool(COutSpoolInfo *si);
	bool RemoveFromSpool(COutSpoolInfo *si);
	int GetCountAndOwnership(SPOOLSTATUS st, int nTaskID, long* nTotalSize);
	
private:
	QCOutSpool(const CString& strPersona, SPOOLTYPE m_type, CString szTempFilename, CString szPersona);
	~QCOutSpool(){}

	CCriticalSection m_ListGuard;
	//QCSpoolListMT<COutSpoolInfo *> m_SpoolInfoList;
	typedef list<COutSpoolInfo *>::iterator LI;
	list<COutSpoolInfo *> m_SpoolInfoList;
};



class QCInSpool : public QCSpool
{
	friend QCSpoolMgrMT;
	
public:
	virtual void BuildSpoolFromFiles();
	void	EndWrite();
	HRESULT EndWrite(CString& szFile, unsigned int uniqueID, CMsgRecord *msgRec);

	CInSpoolInfo* Get(SPOOLSTATUS spOld, SPOOLSTATUS spNew);
	int GetCount(SPOOLSTATUS spOld);

	void AddToSpool(CInSpoolInfo *si);
	bool RemoveFromSpool(CInSpoolInfo *si);
	
private:
	QCInSpool(const CString& strPersona, SPOOLTYPE m_type, CString szTempFilename, CString szPersona);
	~QCInSpool(){}

	//QCSpoolListMT<CInSpoolInfo *> m_SpoolInfoList;
	CCriticalSection m_ListGuard;
	typedef list<CInSpoolInfo *>::iterator LI;
	list<CInSpoolInfo *> m_SpoolInfoList;
	
	
};


CSummary *GetSummaryByUniqueID(int uniqueID, CTocDoc *Toc = NULL);



#endif

