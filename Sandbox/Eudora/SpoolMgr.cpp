#include "stdafx.h"
#include "afxmt.h"
#include "resource.h"		//IDS_LMOS_FILE_NAME

#include "QCUtils.h"

#include "EudError.h"
#include "summary.h"
#include "persona.h"
#include "SpoolMgr.h"
#include "QCTaskManager.h"  //QcTaskManager::m_bProcessSpool QueueWorkerThread
#include "tocdoc.h"			//for tocdoc outtoc
#include "fileutil.h"
#include "MsgRecord.h"		//for CMsgRecord
#include "rs.h"				//CRString

#include "HostList.h"
#include "PopSession.h"  //for CPOPRecoverer

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern CString EudoraDir;
CList<QCSpoolMgrMT *, QCSpoolMgrMT *>  QCSpoolMgrMT::m_SpoolMgrs;
CString QCSpoolMgrMT::m_strSpoolRootDir;
CCriticalSection QCSpoolMgrMT::m_Guard;


 /* ====================== CSpoolMgrMT =============================== */
QCSpoolMgrMT::QCSpoolMgrMT(const char* szPersona)
{
	
	//
	// Create Spool Directory
	//
	m_strSpoolDir = m_strSpoolRootDir + szPersona + "\\";
	ASSERT(m_strSpoolDir != ""); //got to have some path

	CreateDirectory( m_strSpoolDir, NULL );		

	m_strPersona = CString(szPersona);

	m_Out	= new QCOutSpool(m_strPersona, SPOOL_TYPE_SEND,	"temp.out", m_strSpoolRootDir + szPersona);
	//reclaim any left over files
	m_Out->BuildSpoolFromFiles(); 

	m_In	= new QCInSpool(m_strPersona, SPOOL_TYPE_RECEIVED,	"temp.in", m_strSpoolRootDir + szPersona);
	//reclaim any left over files
	m_In->BuildSpoolFromFiles(); 
	
}


QCSpoolMgrMT::~QCSpoolMgrMT()
{
	delete m_Out;
	delete m_In;
}


QCSpoolMgrMT* QCSpoolMgrMT::CreateSpoolMgr(const char* szPersona)
{
	QCSpoolMgrMT* pMgr;

	CSingleLock lock(&m_Guard, TRUE);
	
	//
	// Has this persona already created a spool Manager?
	//
	POSITION pos = m_SpoolMgrs.GetHeadPosition();
	while( pos != NULL )
	{
		pMgr = (QCSpoolMgrMT*) m_SpoolMgrs.GetNext( pos );
		if ( pMgr->GetPersona() == szPersona )
			goto done;
	}
	//
	// Guess not.  Create Spool Manager for this persona.
	//
	pMgr = new QCSpoolMgrMT(szPersona);
	m_SpoolMgrs.AddTail(pMgr);

	//
	// Verify that Spool Directory exists
	//
done:
	if ( !::FileExistsMT( m_strSpoolRootDir + szPersona ) )
		CreateDirectory( m_strSpoolRootDir + szPersona, NULL );		
	

	return pMgr;
}

HRESULT QCSpoolMgrMT::Shutdown()
{
	CSingleLock lock(&m_Guard, TRUE);
	
	while ( !m_SpoolMgrs.IsEmpty() )
	{
		QCSpoolMgrMT* mgr = (QCSpoolMgrMT*) m_SpoolMgrs.RemoveHead();
		delete mgr;
	}
	
	return S_OK;
}

//static
HRESULT QCSpoolMgrMT::Init(CPersonality& p)
{
	ASSERT(!EudoraDir.IsEmpty());
	m_strSpoolRootDir = EudoraDir + "spool\\";

	CString	homie =	p.GetCurrent();

	LPSTR lpPersonalities = p.List();
	lpPersonalities += strlen( lpPersonalities ) + 1;	
	CString Persona = "";								
	do
	{
		p.SetCurrent( Persona );
		QCSpoolMgrMT* SpoolMgr  = CreateSpoolMgr(Persona);

		// Advance to the next personality
		Persona = lpPersonalities;
		lpPersonalities += strlen( lpPersonalities ) + 1;
	} while ( !Persona.IsEmpty() );

	p.SetCurrent( homie );


	//For example, Sent some messages and exit after they are sent but not yet updated with the sent status.
	//If the spool files gets deleted, there is no way to change the status from SPooled to sent
	//The following code does it each time Eudora starts up
	CTocDoc *OutToc = GetOutToc();
	//Failure recovery if any message is left in SPOOLED state
	if (OutToc)
	{
		CSummary *Sum;
		// Count how many messages to send
		for (POSITION pos = OutToc->m_Sums.GetHeadPosition(); pos; )
		{
			Sum = OutToc->m_Sums.GetNext(pos);
			
			if (Sum->m_State == MS_SPOOLED)
			{

				ASSERT(0);
				TRACE( "SPool::BuildSpoolFromFiles: Summary recoverd after processing left over files: \n\t%s\n", 
							Sum->GetTitle()); 
				Sum->SetState(MS_QUEUED);
			}
		}
	}
	
	return S_OK;
}


//static
CString QCSpoolMgrMT::GetLMOSFileName(CString strPersona)
{
	CString strDir;

	if( m_strSpoolRootDir == "")
		m_strSpoolRootDir = EudoraDir + "spool\\";

	if(strPersona == "" || strPersona == "<Dominant>")
		return m_strSpoolRootDir + CRString(IDS_LMOS_FILENAME);
	else
		return (m_strSpoolRootDir + strPersona + "\\" + CRString(IDS_LMOS_FILENAME));
}







//******* CSpoolInfo implementation **************************//

//GET
SPOOLSTATUS CSpoolInfo::GetSpoolStatus()
{
	CSingleLock lock(&m_Guard, TRUE);
	return m_nSpoolStatus; 
}

SPOOLTYPE CSpoolInfo::GetSpoolType()
{ 
	CSingleLock lock(&m_Guard, TRUE);
	return m_nSpoolType; 
}

//SET
void CSpoolInfo::SetSpoolStatus(SPOOLSTATUS status)
{ 
	CSingleLock lock(&m_Guard, TRUE);

	//IF trying to set to the same status, we are done
	if(m_nSpoolStatus == status)
		return;

	m_nSpoolStatus = status;
	if( (status == SS_SENDING_INPROGRESS) || 
		(status == SS_RECEIVING_INPROGRESS) ||
		(status == SS_REMOVAL_INPROGRESS) )
		return;

	//if file types changed, we should rename the file also
	SetSpoolTypeByStatus(status);
	
}


//returns true if the file tpe got changed
bool CSpoolInfo::SetSpoolTypeByStatus(SPOOLSTATUS spStatus)
{
	SPOOLTYPE oldType = m_nSpoolType;
	CString szOld = GetFileName();
	switch(spStatus)
	{
		case SS_SENT:			m_nSpoolType = SPOOL_TYPE_SENT; break;
		case SS_UNSENDABLE:		m_nSpoolType = SPOOL_TYPE_UNSENDABLE; break;
		case SS_UNSENT:			m_nSpoolType = SPOOL_TYPE_UNSENT; break;
		
		case SS_REMOVED:		m_nSpoolType = SPOOL_TYPE_REMOVED; break;
		
		case SS_SEND:			ASSERT(0); m_nSpoolType = SPOOL_TYPE_SEND; break;
		case SS_RECEIVED:		ASSERT(0); m_nSpoolType = SPOOL_TYPE_RECEIVED; break;
		case SS_UNKNOWN:	ASSERT(0); break;
		case SS_DONE_SEND:	ASSERT(0); break;
		default:
			m_nSpoolType = oldType;
			break;
	}

	//if types changed rename the file or remove it
	if(m_nSpoolType != oldType)
	{
		HRESULT hr;
		if (m_nSpoolType == SPOOL_TYPE_REMOVED)
			hr = ::FileRemoveMT(szOld);
		else 
		{
			CString szNew = this->GetFileName();
			hr = ::FileRenameMT(szOld,szNew);
		}
		if(FAILED(hr)) ASSERT(0);
	}
	return true;
}


void CSpoolInfo::SetSpoolType(SPOOLTYPE type)
{
	CSingleLock lock(&m_Guard, TRUE);
	m_nSpoolType = type; 
}



// CLASS CSpoolInfo
CString CSpoolInfo::GetFileName()
{
	CSingleLock lock(&m_Guard, TRUE);

	//ASSERT(this->m_nSpoolStatus != SPOOL_FILE_DELETE);
	ASSERT(this->m_nSpoolType != SPOOL_TYPE_REMOVED);
	
	CString szNew = this->m_strFileName;
	
	switch(this->m_nSpoolType)
	{
		case SPOOL_TYPE_SEND:
			szNew += ".SND";
			break;
		case SPOOL_TYPE_SENT:
			szNew += ".SNT";
			break;
		case SPOOL_TYPE_UNSENDABLE:
			szNew += ".UNL";
			break;
		case SPOOL_TYPE_UNSENT:
			szNew += ".UNS";
			break;
		case SPOOL_TYPE_RECEIVED:
			szNew += ".RCV";
			break;
		default:
			//no other status
			ASSERT(0);
		break;
	}
	
	return szNew;
}


//CLASS COutSpoolInfo

COutSpoolInfo::COutSpoolInfo(CString name, SPOOLTYPE st, SPOOLSTATUS ss, CSummary *sum)
		: CSpoolInfo(name, st, ss), m_pSum(sum), m_nTaskID(0)
{	
		if(m_pSum != NULL)
		{
			m_UniqueID = m_pSum->GetUniqueMessageId(); 
			m_OutToc = m_pSum->m_TheToc;
			m_strTitle = m_pSum->GetTitle();
		}
}

CString COutSpoolInfo::GetTitle()
{
	CSingleLock lock(&m_Guard, TRUE);
	return m_strTitle;
}

int COutSpoolInfo::GetTaskID()
{
	CSingleLock lock(&m_Guard, TRUE);
	ASSERT(m_nTaskID >= 0);
	return m_nTaskID;
}

void COutSpoolInfo::SetTaskID(int nTaskID)
{
	CSingleLock lock(&m_Guard, TRUE);
	ASSERT(nTaskID > 0);
	m_nTaskID = nTaskID;
}


int COutSpoolInfo::GetUniqueID()
{ 
	CSingleLock lock(&m_Guard, TRUE);

	ASSERT(m_UniqueID); 
	return m_UniqueID; 
}
	

CSummary *COutSpoolInfo::GetSummary()
{
	
	CTocDoc *Toc = GetOutToc();
	if (!Toc)
		return (CSummary *)NULL;

	//Here we compare the OutToc we saved with the present one if they are same then 
	//our summary pointer would be valid, otherwise go and search for the summary by unique ID

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//what if the summary is moved out of here before..
	//if(Toc == m_OutToc)
	//	return m_pSum;


	CSummary *Sum = NULL;
	
	//Start search from the end, because thats where new messages would be!!
	for (POSITION pos = Toc->m_Sums.GetTailPosition(); pos; )
	{
		Sum = Toc->m_Sums.GetPrev(pos);
		if(Sum->GetUniqueMessageId() == m_UniqueID)
			return Sum;
	}

	return (CSummary *)NULL;
}



CInSpoolInfo::CInSpoolInfo(CString name, SPOOLTYPE st, SPOOLSTATUS ss,  CMsgRecord *msgRec)
		: CSpoolInfo(name, st, ss), m_pMsgRecord(msgRec)
{	
	if(m_pMsgRecord != NULL)
		m_Hash = m_pMsgRecord->GetHashValue(); 
		
}
 

int CInSpoolInfo::GetUniqueID()
{
	CSingleLock lock(&m_Guard, TRUE);

	ASSERT(m_Hash); 
	return m_Hash; 
}

CMsgRecord *CInSpoolInfo::GetMsgRecord()
{
	CSingleLock lock(&m_Guard, TRUE);

	return m_pMsgRecord; 
}




// ===================== QCSpool ====================== 
QCSpool::QCSpool(const CString& strPersona, SPOOLTYPE type, CString szTempFilename, CString szBasePath)
{

	m_strPersona = strPersona;
	//m_strBasePath	= szBasePath + "\\";

	if(szBasePath[szBasePath.GetLength()-1] != '\\')
		szBasePath	= szBasePath+"\\";

	m_strBasePath	= szBasePath;
	m_strTempFile	= m_strBasePath+szTempFilename;
	m_bWrite		= FALSE;

	m_threadID		= AfxGetThread()->m_nThreadID;
	m_type			= type;

	if (m_type == SPOOL_TYPE_SEND )
		m_strExtension = "SND";
	else if (m_type == SPOOL_TYPE_RECEIVED )
		m_strExtension = "RCV";

	//Build the spool from the files left over in previous crashes or closes before we got a
	//chance to send them over-wire
	//Also make sure to remove  the temp file if left(temp.out)
	
	::FileRemoveMT(m_strTempFile);
	
}



QCOutSpool::QCOutSpool(const CString& strPersona, SPOOLTYPE m_type, 
					   CString szTempFilename, CString szPersona)
		: QCSpool(strPersona, m_type, szTempFilename, szPersona) 
{
	
}
	


void QCOutSpool::AddToSpool(COutSpoolInfo *si)
{ 
	//m_SpoolInfoList.enqueue(si); 
	CSingleLock lock(&m_ListGuard, TRUE);
	m_SpoolInfoList.push_back(si);

}


bool QCOutSpool::RemoveFromSpool(COutSpoolInfo *si)
{ 
	//return m_SpoolInfoList.dequeue(si); 
	CSingleLock lock(&m_ListGuard, TRUE);
	LI li = find(m_SpoolInfoList.begin(), m_SpoolInfoList.end(), si);
	if( li == m_SpoolInfoList.end())
	{
		ASSERT(0);
		return false;
	}
	
	m_SpoolInfoList.erase(li);
	delete si;
	return true;
}

COutSpoolInfo* QCOutSpool::Get(SPOOLSTATUS spOld, SPOOLSTATUS spNew, int nTaskID)
{ 
	//return m_SpoolInfoList.Get(spOld, spNew, UID); 
	
	COutSpoolInfo* si = NULL;
	CSingleLock lock(&m_ListGuard, TRUE); 
	
	for( LI li = m_SpoolInfoList.begin(); li != m_SpoolInfoList.end(); ++li)
	{
		si = *li;
	
		if( nTaskID && si->GetTaskID() != nTaskID)
			continue;
		
		//SS_DONE_SEND is a combination of all spools that resulted fron a SEND
		if(spOld == SS_DONE_SEND)
		{
			SPOOLSTATUS st = si->GetSpoolStatus();
			if( (st == SS_SENT) || (st == SS_UNSENT) || (st == SS_UNSENDABLE) || (st == SS_SEND))
			{
				si->SetSpoolStatus(spNew);
				return si;
			}
		}
		else
		{
			if(si->GetSpoolStatus() == spOld)
			{
				si->SetSpoolStatus(spNew);
				return si;
			}
		}
	}
	return NULL;
}


int QCOutSpool::GetCountAndOwnership(SPOOLSTATUS status, int nTaskID, long* nTotalSize)
{
	//return m_SpoolInfoList.GetCount(st, UID); 

	CSingleLock lock(&m_ListGuard, TRUE); 

	*nTotalSize = 0;
	int count = 0;
	
	COutSpoolInfo * si = NULL;
	for( LI li = m_SpoolInfoList.begin(); li != m_SpoolInfoList.end(); ++li)
	{
		si = *li;
		
		if(si->GetTaskID())
			continue;

		if(si->GetSpoolStatus() == status)
		{
			count++;
			if(nTaskID)
				si->SetTaskID(nTaskID);
			
			CFileStatus Status;	// DITCH
			if( CFile::GetStatus(si->GetFileName(), Status))
				*nTotalSize += Status.m_size;
		}
	}
	return count;
}



void QCOutSpool::BuildSpoolFromFiles()
{
	
	ASSERT(::IsMainThreadMT());

	CSummary *Sum;
	QCFileFinder finder;
	CString szFileName;  //relative file name
	CString szBaseFileName; //File name with base path
	unsigned long uniqueID = 0;
	COutSpoolInfo *si=NULL;
	
	enum { FILE_SEND=0, FILE_SENT=1, FILE_UNSENT=2, FILE_UNSENDABLE=3 };
	CString SpoolFileTypes[4] = { "*.SND", "*.SNT", "*.UNS", "*.UNL" };
	
	CTocDoc *OutToc = GetOutToc();
	
	for(int i=0; i < 4; i++)
	{
		finder.FindFile(m_strBasePath + SpoolFileTypes[i]);
		
		while(finder.FindNextFile())
		{
			szFileName = finder.GetFileName();
			szFileName = szFileName.Left(szFileName.GetLength()-4);
			uniqueID = 0;
			sscanf(szFileName, "%x", &uniqueID);
			if(!uniqueID)
			{
				ASSERT(0); //must be a digit otherwise we are in big trouble
				TRACE("Parsing bad Outgoing file: \"%s\"\n", szFileName);
				continue;  //process the next file
			}

			
			szBaseFileName = m_strBasePath + szFileName;
			
			Sum = GetSummaryByUniqueID(uniqueID, OutToc);
			
			if(uniqueID && Sum)
			{
				switch(i)
				{
				case FILE_SEND:
					Sum->SetState(MS_QUEUED); 
					break;
				case FILE_UNSENT:
					//Yes, Queue it again..the user just doesn't wanna send right now..  
					Sum->SetState(MS_QUEUED); 
					break;
				case FILE_UNSENDABLE:

					Sum->SetState(MS_UNSENDABLE);
					Sum->Display();
					//DisplayError(szBaseFileName);
					break;
				
				case FILE_SENT:
					//Need to do filtering
					si = new COutSpoolInfo(szBaseFileName, SPOOL_TYPE_SENT, SS_SENT, Sum);
				
					this->AddToSpool(si);
					Sum->SetState(MS_SENT);

					break;

				default:
					ASSERT(0);
					break;

				}
									
			}
			else
			{
				ASSERT(0);
			}
			//No matter what, remove the file
			//FileRemoveMT(szBaseFileName);
			FileRemoveMT(m_strBasePath  + finder.GetFileName());
		}//while
	} //for 4 different SENT files

/*
	//Failure recovery if any message is left in SPOOLED state
	if (OutToc)
	{
		// Count how many messages to send
		for (POSITION pos = OutToc->m_Sums.GetHeadPosition(); pos; )
		{
			Sum = OutToc->m_Sums.GetNext(pos);
			
			if (Sum->m_State == MS_SPOOLED)
			{

				ASSERT(0);
				TRACE( "SPool::BuildSpoolFromFiles: Summary recoverd after processing left over files: \n\t%s\n", 
							Sum->GetTitle()); 
				Sum->SetState(MS_QUEUED);
			}
		}
	}
*/

}



HRESULT QCSpool::BeginWrite(CString& szFileName)
{
	ASSERT(m_bWrite==FALSE);
	//ASSERT(AfxGetThread()->m_nThreadID==m_threadID);

	// Validate temp file path...
	szFileName = m_strTempFile;

	m_bWrite=TRUE;
	
	return (S_OK);
}




void QCOutSpool::EndWrite()
{
	ASSERT(m_bWrite==TRUE);
	//ASSERT(AfxGetThread()->m_nThreadID==m_threadID);
	
	m_bWrite=FALSE;
	
}


HRESULT QCOutSpool::EndWrite(CString& szFile, unsigned int uniqueID, CSummary *sum)
{
	ASSERT(m_bWrite==TRUE);
	//ASSERT(AfxGetThread()->m_nThreadID==m_threadID);
	HRESULT hr;

	szFile="";
	//File calls must be MT

	CString szFilename="";
	szFilename.Format("%s%X.%s", (LPCTSTR)m_strBasePath, uniqueID, (LPCTSTR)m_strExtension);
	
	// Make sure file is closed?
	hr = FileRenameMT(m_strTempFile, szFilename);
	if ( FAILED(hr) )
		return hr;
	szFile.Format("%s%X", (LPCTSTR)m_strBasePath, uniqueID);

	COutSpoolInfo *si = new COutSpoolInfo(szFile, SPOOL_TYPE_SEND, SS_SEND, sum);
	
	this->AddToSpool(si);

	m_bWrite=FALSE;

	return S_OK;
}



QCInSpool::QCInSpool(const CString& strPersona, SPOOLTYPE m_type, 
					 const CString szTempFilename, CString szBasePath)
		: QCSpool(strPersona, m_type, szTempFilename, szBasePath) 
{
	
}


void QCInSpool::AddToSpool(CInSpoolInfo *si)
{ 
	//m_SpoolInfoList.enqueue(si); 
	CSingleLock lock(&m_ListGuard, TRUE);
	m_SpoolInfoList.push_back(si);

}


bool QCInSpool::RemoveFromSpool(CInSpoolInfo *si)
{ 
	//return m_SpoolInfoList.dequeue(si); 
	CSingleLock lock(&m_ListGuard, TRUE);
	LI li = find(m_SpoolInfoList.begin(), m_SpoolInfoList.end(), si);
	if( li == m_SpoolInfoList.end())
	{
		ASSERT(0);
		return false;
	}
	
	m_SpoolInfoList.erase(li);
	delete si;
	return true;
}

int QCInSpool::GetCount(SPOOLSTATUS status)
{
	//return m_SpoolInfoList.GetCount(st); 
	CSingleLock lock(&m_ListGuard, TRUE); 
	
	int count = 0;
	CInSpoolInfo * si = NULL;
	for( LI li = m_SpoolInfoList.begin(); li != m_SpoolInfoList.end(); ++li)
	{
		si = *li;
	
		if(si->GetSpoolStatus() == status)
			count++;
	}
	return count;
}

CInSpoolInfo* QCInSpool::Get(SPOOLSTATUS spOld, SPOOLSTATUS spNew)
{ 
	//return m_SpoolInfoList.Get(spOld, spNew); 
	
	CInSpoolInfo* si = NULL;
	CSingleLock lock(&m_ListGuard, TRUE); 
	
	for( LI li = m_SpoolInfoList.begin(); li != m_SpoolInfoList.end(); ++li)
	{
		si = *li;
	
		if(si->GetSpoolStatus() == spOld)
		{
			si->SetSpoolStatus(spNew);
			return si;
		}
	}
	return NULL;
}

	

void QCInSpool::EndWrite()
{
	ASSERT(m_bWrite==TRUE);
	//ASSERT(AfxGetThread()->m_nThreadID==m_threadID);
	
	m_bWrite=FALSE;
	
}



HRESULT QCInSpool::EndWrite(CString& szFile, unsigned int uniqueID, CMsgRecord *msgRec)
{
	ASSERT(m_bWrite==TRUE);
	//ASSERT(AfxGetThread()->m_nThreadID==m_threadID);
	HRESULT hr;

	m_bWrite=FALSE; //done with writing

	szFile.Empty();
	//File calls must be MT

	CString szFilename="";
	szFilename.Format("%s%X.%s", (LPCTSTR)m_strBasePath, uniqueID, (LPCTSTR)m_strExtension);

	if (::FileExistsMT(szFilename))
		ASSERT(0);

	// Make sure file is closed?
	hr = FileRenameMT(m_strTempFile, szFilename);
	if ( FAILED(hr) )
		return hr;
	szFile.Format("%s%X", (LPCTSTR)m_strBasePath, uniqueID);

	CInSpoolInfo *si = new CInSpoolInfo(szFile, SPOOL_TYPE_RECEIVED, SS_RECEIVED, msgRec);

	this->AddToSpool(si);

	return S_OK;
}



//Implements failure recovery; 
extern void RecoverReceivedMail(CString);
void QCInSpool::BuildSpoolFromFiles()
{
	
	QCFileFinder finder;
	finder.FindFile(m_strBasePath + "*.RCV");

	CPopHost *pPopHost = CHostList::GetHost(m_strPersona);
	if(!pPopHost){
		ASSERT(0);
		return;
	}

	unsigned long uniqueID = 0;
	int nNumFilesRecovered = 0;

	while(finder.FindNextFile())
	{
		//Get the file UID or Hash
		CString strFileName = finder.GetFileName();
		//Get rid of the extension .RCV
		strFileName = strFileName.Left(strFileName.GetLength()-4);
		uniqueID = 0;
		sscanf(strFileName, "%x", &uniqueID);
		if(!uniqueID){
			ASSERT(0); //must be a digit otherwise we are in big trouble
			TRACE("BuildSpool::Parsing bad RCV file: \"%s\"\n", strFileName);
			continue;  //process the next file
		}
		
		//Get the message record for this hash
		CLMOSRecord LMOSEntry(pPopHost, uniqueID);
		CMsgRecord *pMsgRecord = LMOSEntry.GetMsgRecord();
		if(!pMsgRecord)
		{
			ASSERT(0);
			TRACE("BuildSpool::RCV file didn't have an LMOS entry: \"%s\"\n", strFileName);
			continue;
		}

		//build a OutSpoolInfo entry based on this information
											// ../spool/ + "123445"
		CInSpoolInfo *si = new CInSpoolInfo(m_strBasePath + strFileName, SPOOL_TYPE_RECEIVED, SS_RECEIVED, pMsgRecord);

		//Insert in out Spool Queue to be de-mime-d and filtered
		AddToSpool(si);
		nNumFilesRecovered++;

				//::FileRemoveMT(m_strBasePath  + finder.GetFileName());
	}//while

	if(nNumFilesRecovered)
	{
		//Create a dummy receiving task to initiate the PostIncomingMail process
		CPOPRecoverer *recover = new CPOPRecoverer(m_strPersona);
		QCGetTaskManager()->QueueWorkerThread(recover);
	}
}



bool QCFileFinder::FindNextFile(){
	if(m_bFirstTime){
		m_bFirstTime = false;
		m_hContext = ::FindFirstFile(m_SearchString, &data);
		if(m_hContext != INVALID_HANDLE_VALUE)
			return true;
	}
	
	if(m_hContext == INVALID_HANDLE_VALUE)
		return false;
	
	if( ::FindNextFile(m_hContext, &data))
		return true;

	m_hContext = NULL;
		return false;
}


CString QCFileFinder::GetFileName(){
	ASSERT(m_hContext);
	return CString(data.cFileName);
}



//Global Function to find a summary object with the given Unique ID
//In:  uniqueID
//In:  Toc if not given OutToc is created and taken as the TOC

CSummary *GetSummaryByUniqueID(int uniqueID, CTocDoc *Toc) //Toc=NULL
{
	
	if(Toc == NULL) Toc = GetOutToc();
	if (!Toc)
		return (CSummary *)NULL;

	CSummary *Sum = NULL;
	// Count how many messages to send
	for (POSITION pos = Toc->m_Sums.GetHeadPosition(); pos; )
	{
		Sum = Toc->m_Sums.GetNext(pos);
		if(Sum->GetUniqueMessageId() == uniqueID)
			return Sum;
	}

	return (CSummary *)NULL;
}


/*


//Thread-safe List in which SpoolInfo objects get stored
//This function gets a FIRST_FOUND SpoolInfo object based upon the SPOOLTYPE
//Also it changes the SPOOLTYPE to the newType given; Helps us thread-safely changing the SPOOLTYPE to
//	someother type

template <class T>
T QCSpoolListMT<T>::Get(SPOOLSTATUS spFind, SPOOLSTATUS spNew, int nTaskID =0)
{
	T si = NULL;
	
	CSingleLock lock(&m_ListGuard, TRUE); 
	POSITION pos = m_List.GetHeadPosition();
	
	//SS_DONE_SEND is a combination of all spools that resulted fron a SEND
	if(spFind == SS_DONE_SEND)
	{
		SPOOLSTATUS st;
		while( pos != NULL )
		{
			si = m_List.GetNext( pos );
			st = si->GetSpoolStatus();
			if( nTaskID && si->GetTaskID() != nTaskID)
				continue;
			
			if( (st == SS_SENT) || (st == SS_UNSENT) || (st == SS_UNSENDABLE) || (st == SS_SEND))
			{
				si->SetSpoolStatus(spNew);
				return si;
			}
		}
		return NULL;
	}		

	while( pos != NULL )
	{
		si = m_List.GetNext( pos );

		if( nTaskID && si->GetTaskID() != nTaskID)
			continue;

		if(si->GetSpoolStatus() == spFind)
		{
			si->SetSpoolStatus(spNew);
			return si;
		}
	}
	
	return NULL;
}

template<class T>
int QCSpoolListMT<T>::GetCount(SPOOLSTATUS status, int UID=0)
{
	CSingleLock lock(&m_ListGuard, TRUE); 
	int count = 0;
	T si = NULL;

	POSITION pos = m_List.GetHeadPosition();
	while( pos != NULL )
	{
		si = m_List.GetNext( pos );
		
		if(si->GetTaskID())
			continue;

		if(si->GetSpoolStatus() == status)
		{
			count++;
			if(UID)
				si->SetTaskID(UID);
		}
	}
	return count;
}
	



template <class T>
void QCSpoolListMT<T>::enqueue(T msg)
{
	CSingleLock lock(&m_ListGuard, TRUE); 
	m_List.AddTail(msg); 
}

template <class T>	
bool QCSpoolListMT<T>::dequeue(T msg)
{
	CSingleLock lock(&m_ListGuard, TRUE); 
	if(m_List.IsEmpty()) return false;
	msg = m_List.RemoveHead();
	delete msg;
	return true;
}
*/



