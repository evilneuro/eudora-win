// SpoolMgr.h
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

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

#include "DebugNewHelpers.h"


extern CString EudoraDir;
CList<QCSpoolMgrMT *, QCSpoolMgrMT *>	QCSpoolMgrMT::s_SpoolMgrs;
CString									QCSpoolMgrMT::s_strSpoolRootDir;
CCriticalSection						QCSpoolMgrMT::s_Guard;
int										QCSpoolMgrMT::s_nSpoolRenameRetryCount = 0;
int										QCSpoolMgrMT::s_nSpoolRenameRetryInterval = 0;

void FixPersonaName(CString &personaName);


 /* ====================== CSpoolMgrMT =============================== */
QCSpoolMgrMT::QCSpoolMgrMT(const char* szPersonaUnsafe)
{
	if (s_strSpoolRootDir.IsEmpty())
	{
		if (GetIniShort(IDS_INI_SPOOL_DIR_TEMP))
			s_strSpoolRootDir = TempDir + "EuSpool\\";
		else
			s_strSpoolRootDir = EudoraDir + "spool\\";
		CreateDirectoryMT(s_strSpoolRootDir);		
	}

	m_strPersona = szPersonaUnsafe;

	// Create Spool Directory, checking for <Dominant> personality, which
	// is represented by an empty string any time we're called.
	m_strSpoolDir = s_strSpoolRootDir;
	if (szPersonaUnsafe && *szPersonaUnsafe)
	{
		CString personaName(szPersonaUnsafe);
		FixPersonaName(personaName);
		m_strSpoolDir += personaName + "\\";
	}
	CreateDirectoryMT(m_strSpoolDir);		

	m_Out	= DEBUG_NEW QCOutSpool(m_strPersona, SPOOL_TYPE_SEND, m_strSpoolDir);

	// reclaim any leftover files
	m_Out->BuildSpoolFromFiles();

	//	Delete any leftover temp files
	m_Out->DeleteLeftoverTempFiles();

	m_In	= DEBUG_NEW QCInSpool(m_strPersona, SPOOL_TYPE_RECEIVED, m_strSpoolDir);

	// reclaim any leftover files
	m_In->BuildSpoolFromFiles();

	//	Delete any leftover temp files
	m_In->DeleteLeftoverTempFiles();
}


QCSpoolMgrMT::~QCSpoolMgrMT()
{
	delete m_Out;
	delete m_In;
}


QCSpoolMgrMT* QCSpoolMgrMT::CreateSpoolMgr(const char* szPersonaUnsafe)
{
	CSingleLock lock(&s_Guard, TRUE);
	
	QCSpoolMgrMT* pSpoolMgr = NULL;

	//
	// Has this persona already created a spool Manager?
	//
	POSITION pos = s_SpoolMgrs.GetHeadPosition();
	while( pos != NULL )
	{
		QCSpoolMgrMT* pMgr = (QCSpoolMgrMT*) s_SpoolMgrs.GetNext( pos );
		if (pMgr->GetPersona().CompareNoCase(szPersonaUnsafe) == 0)
		{
			pSpoolMgr = pMgr;
			break;
		}
	}

	if (!pSpoolMgr)
	{
		//
		// Guess not.  Create Spool Manager for this persona.
		//
		pSpoolMgr = DEBUG_NEW QCSpoolMgrMT(szPersonaUnsafe);
		s_SpoolMgrs.AddTail(pSpoolMgr);
	}

	return pSpoolMgr;
}

HRESULT QCSpoolMgrMT::Shutdown()
{
	CSingleLock lock(&s_Guard, TRUE);
	
	while ( !s_SpoolMgrs.IsEmpty() )
	{
		QCSpoolMgrMT* mgr = (QCSpoolMgrMT*) s_SpoolMgrs.RemoveHead();
		delete mgr;
	}
	
	return S_OK;
}

//static
HRESULT QCSpoolMgrMT::Init(CPersonality& p)
{
	//	Load the rename retry count, keeping the value between 1 and 30
	if (QCSpoolMgrMT::s_nSpoolRenameRetryCount == 0)
		QCSpoolMgrMT::s_nSpoolRenameRetryCount = min( 30L, max( 1L, GetIniLong(IDS_INI_SPOOL_RENAME_RETRY_COUNT) ) );
	
	//	Load the rename retry interval, keeping the value between 100 and 1000
	if (QCSpoolMgrMT::s_nSpoolRenameRetryInterval == 0)
		QCSpoolMgrMT::s_nSpoolRenameRetryInterval = min( 1000L, max( 100L, GetIniLong(IDS_INI_SPOOL_RENAME_RETRY_INTERVAL) ) );

	CString	homie =	p.GetCurrent();

	LPSTR lpPersonalities = p.List();
	lpPersonalities += strlen( lpPersonalities ) + 1;	
	CString Persona = "";								
	do
	{
		p.SetCurrent( Persona );
		CreateSpoolMgr(Persona);

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
		CSumList &		listSums = OutToc->GetSumList();
		
		CSummary *Sum;
		// Count how many messages to send
		for (POSITION pos = listSums.GetHeadPosition(); pos; )
		{
			Sum = listSums.GetNext(pos);
			
			if (Sum->m_State == MS_SPOOLED)
			{

				TRACE( "SPool::BuildSpoolFromFiles: Summary recoverd after processing left over files: \n\t%s\n", 
							Sum->GetTitle()); 
				Sum->SetState(MS_QUEUED);
			}
		}
	}
	
	return S_OK;
}


//static
CString QCSpoolMgrMT::GetLMOSFileName(const char* strPersonaUnsafe)
{
	CString strLMOS;

	strLMOS = EudoraDir + "spool\\";
	
	if (strPersonaUnsafe && *strPersonaUnsafe && stricmp(strPersonaUnsafe, CRString(IDS_DOMINANT)))
	{
		// another entry point for unsafe names.  :-(
		CString strPersona(strPersonaUnsafe);
		FixPersonaName(strPersona);

		strLMOS += strPersona + "\\";
	}
	
	strLMOS += CRString(IDS_LMOS_FILENAME);
		
	return strLMOS;
}



//******* CSpoolInfo implementation **************************//

CSpoolInfo::CSpoolInfo()
	:	m_strFileName(), m_pSpoolFile(NULL), m_nSpoolStatus(SS_UNKNOWN), m_nSpoolType(SPOOL_TYPE_SEND)
{
}


CSpoolInfo::CSpoolInfo(const char* name, SPOOLTYPE st, SPOOLSTATUS ss, JJFileMT * pSpoolFile)
	:	m_strFileName(name), m_pSpoolFile(pSpoolFile), m_nSpoolStatus(ss), m_nSpoolType(st)
{
}


CSpoolInfo::~CSpoolInfo()
{
	delete m_pSpoolFile;
}


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

COutSpoolInfo::COutSpoolInfo(const char* name, SPOOLTYPE st, SPOOLSTATUS ss, CSummary* sum)
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
	

CSummary* COutSpoolInfo::GetSummary()
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

	CSumList &		listSums = Toc->GetSumList();

	CSummary *Sum = NULL;
	
	//Start search from the end, because thats where new messages would be!!
	for (POSITION pos = listSums.GetTailPosition(); pos; )
	{
		Sum = listSums.GetPrev(pos);
		if(Sum->GetUniqueMessageId() == m_UniqueID)
			return Sum;
	}

	return (CSummary *)NULL;
}



CInSpoolInfo::CInSpoolInfo(const char* name, SPOOLTYPE st, SPOOLSTATUS ss,  CMsgRecord *msgRec, JJFileMT * pSpoolFile)
		: CSpoolInfo(name, st, ss, pSpoolFile), m_pMsgRecord(msgRec)
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
QCSpool::QCSpool(const char* strPersona, SPOOLTYPE type, const char* szBasePath)
{
	m_strPersona = strPersona;

	m_strBasePath = szBasePath;
	if (m_strBasePath[m_strBasePath.GetLength() - 1] != '\\')
		m_strBasePath += '\\';

	m_bWrite		= FALSE;

	m_threadID		= AfxGetThread()->m_nThreadID;
	m_type			= type;

	if (m_type == SPOOL_TYPE_SEND )
		m_strExtension = "SND";
	else if (m_type == SPOOL_TYPE_RECEIVED )
		m_strExtension = "RCV";
	
	//	Default to not using the final name while downloading. We don't normally want
	//	to use the final name the whole time for two reasons:
	//	* For non-UIDL incoming cases we don't have the unique ID until after we've
	//	  downloaded
	//	* If a crash occurs before the message is fully downloaded it could cause the
	//	  patially downloaded message to be recoved, even though it is still on the server
	//	  and will be re-downloaded
	m_bUseFinalName = false;
}


HRESULT QCSpool::BeginWrite(CString& szFileName, bool bUseFinalName, unsigned int uniqueID)
{
	ASSERT(m_bWrite==FALSE);
	//ASSERT(AfxGetThread()->m_nThreadID==m_threadID);

	m_bUseFinalName = bUseFinalName;

	if (bUseFinalName)
	{
		//	We're skipping straight to using the final file name
		FormatFinalFileName(uniqueID, m_strTempFile);
	}
	else
	{
		//	Get a temp file name using our extension as the prefix
		//	This will look something like:
		//	"SND25C.tmp" or "RCV25C.tmp" if GetTempFileName works
		//	or
		//	"SND-temp.tmp" or "RCV-temp.tmp" if GetTempFileName fails
		//
		//	Previously the spools reused the file names temp.out for sending email and
		//	temp.in for receiving email.
		//	Issues with old names (particularly temp.in) that the new naming scheme attempts to fix:
		//	* In one case McAfee was reported to have deleted temp.in because of a virus
		//	  in a previously downloaded message (i.e. the temp.in that was deleted was
		//	  a different virus-free message, but McAfee didn't recognize this). Using
		//	  different file names should help avoid this problem.
		//	* McAfee's default suspect file extension list has an entry .in? (where ? can be any
		//	  character - including no character), which made temp.in be scanned even when
		//	  the user only had scanning for suspect extensions. Files named *.tmp are not
		//	  in McAfee's default suspect file extension list at the time of this writing.
		HRESULT hr = GetTempFileName( m_strBasePath, m_strExtension, 0, m_strTempFile.GetBuffer(MAX_PATH + 1) );
		if ( SUCCEEDED(hr) )
			m_strTempFile.ReleaseBuffer();
		else
			m_strTempFile = m_strExtension + "-temp.tmp";
	}

	//	Pass back the temp file path
	szFileName = m_strTempFile;

	m_bWrite = TRUE;
	
	return S_OK;
}


void QCSpool::FormatFinalFileName(unsigned int uniqueID, CString& szFinalFileName)
{
	szFinalFileName.Format("%s%X.%s", (LPCTSTR)m_strBasePath, uniqueID, (LPCTSTR)m_strExtension);
}


HRESULT QCSpool::RenameTempFileToFinalName(DebugMaskType debugMask, unsigned int uniqueID)
{
	char	szBuffer[MAX_PATH*2+256];
		
	CString		szFilename;
	FormatFinalFileName(uniqueID, szFilename);

	ASSERT( !::FileExistsMT(szFilename) );

	//	Make sure that our temp file still exists before trying to rename it
	if ( !::FileExistsMT(m_strTempFile) )
	{
		wsprintf(szBuffer, CRString(IDS_DEBUG_TEMP_MISSING), m_strTempFile, szFilename);
		::PutDebugLog(debugMask, szBuffer);

		return MAKE_HRESULT(1, FACILITY_WINDOWS, ERROR_FILE_NOT_FOUND);
	}
	
	//	Attempt to rename the file to the final name up to 10 times
	//	over a period of 3 seconds. This should help us recover
	//	from cases where a virus scanner like McAfee has locked
	//	access to our *.tmp file.
	HRESULT		hr;
	long		nTries = QCSpoolMgrMT::GetSpoolRenameRetryCount();
	bool		bKeepTrying;
	DWORD		dwError = ERROR_SUCCESS;

	do
	{
		bKeepTrying = false;
		hr = ::FileRenameMT(m_strTempFile, szFilename);

		if ( FAILED(hr) && (--nTries > 0) )
		{
			//	Retry if the error is the right kind of error - i.e.
			//	that some other program (like McAfee) is using our file at the moment.
			//	In testing by opening the temp file with Word the error
			//	returned was ERROR_SHARING_VIOLATION, but also check for
			//	2 other errors that seem like they could indicate the same thing.
			dwError = GetLastError();
			switch (dwError)
			{
				case ERROR_ACCESS_DENIED:
				case ERROR_SHARING_VIOLATION:
				case ERROR_LOCK_VIOLATION:
					Sleep( QCSpoolMgrMT::GetSpoolRenameRetryInterval() );
					bKeepTrying = true;
			}
		}
	} while (bKeepTrying);

	if ( FAILED(hr) )
	{
		//	Write entry into log describing the details of what went wrong
		wsprintf(szBuffer, CRString(IDS_DEBUG_RENAME_TEMP_FAILED), m_strTempFile, szFilename, dwError);
		::PutDebugLog(debugMask, szBuffer);

		//	Renaming the file failed, but perhaps we can copy it.
		//
		//	This is not ideal, because copying is slower and uses up more disk space, but it's
		//	better than stopping the mail receiving or sending. In particular if we're in the
		//	midst of an email download and McAfee is interfering with a particular message
		//	we're likely to run into the same problem again the next time we attempt to download
		//	it. If so then the user could get an error during each subsequent mail check
		//	until McAfee is disabled or the message is deleted outside Eudora.
		//
		//	Since renaming has already failed, whether or not copy succeeds we'll have a
		//	leftover .tmp file that will be deleted the next time we're launched.
		if ( ::CopyFile(m_strTempFile, szFilename, FALSE) )
		{
			//	Copying succeeded
			hr = ERROR_SUCCESS;
		}
		else
		{
			//	Copying failed - log this error number as well
			wsprintf( szBuffer, CRString(IDS_DEBUG_COPY_TEMP_FAILED), m_strTempFile, szFilename, GetLastError() );
			::PutDebugLog(debugMask, szBuffer);
		}
	}

	return hr;
}


void QCSpool::DeleteLeftoverTempFiles()
{
	//	Remove temp files that are named appropriately
	//	(i.e. "SND*.tmp" or "RCV*.tmp" where * is either a hex number like 25C or -temp)
	QCFileFinder	finder;
	finder.FindFile(m_strBasePath + m_strExtension + "*.tmp");

	while( finder.FindNextFile() )
	{
		//	Warn the developer that there was a leftover file.
		//	There shouldn't be a lot of these because for a given
		//	sending or receiving session there could only be one leftover file
		//	(rename failure halts sending or receiving).
		ASSERT(0);

		//	Remove the leftover temp file
		FileRemoveMT(m_strBasePath  + finder.GetFileName());
	}
}


QCOutSpool::QCOutSpool(const char* strPersona, SPOOLTYPE m_type, const char* szBasePath)
		: QCSpool(strPersona, m_type, szBasePath)
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
				*nTotalSize += static_cast<long>(Status.m_size);
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
					si = DEBUG_NEW COutSpoolInfo(szBaseFileName, SPOOL_TYPE_SENT, SS_SENT, Sum);
				
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
}


void QCOutSpool::EndWrite()
{
	ASSERT(m_bWrite == TRUE);
	
	m_bWrite = FALSE;
}


HRESULT QCOutSpool::EndWrite(unsigned int uniqueID, CSummary *sum)
{
	ASSERT(m_bWrite == TRUE);

	if (!m_bUseFinalName)
	{
		HRESULT		hr = RenameTempFileToFinalName(DEBUG_MASK_SEND, uniqueID);
		if ( FAILED(hr) )
			return hr;
	}

	CString		szFilename;
	szFilename.Format("%s%X", (LPCTSTR)m_strBasePath, uniqueID);

	COutSpoolInfo *si = DEBUG_NEW COutSpoolInfo(szFilename, SPOOL_TYPE_SEND, SS_SEND, sum);
	
	AddToSpool(si);

	m_bWrite = FALSE;

	return S_OK;
}



QCInSpool::QCInSpool(const char* strPersona, SPOOLTYPE m_type, const char* szBasePath)
		: QCSpool(strPersona, m_type, szBasePath) 
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
	ASSERT(m_bWrite == TRUE);
	
	m_bWrite = FALSE;
}



HRESULT QCInSpool::EndWrite(unsigned int uniqueID, CMsgRecord *msgRec, JJFileMT * pSpoolFile)
{
	ASSERT(m_bWrite==TRUE);

	m_bWrite = FALSE; //done with writing

	//	Must be using no temp name when passed a spool file
	ASSERT( (pSpoolFile && m_bUseFinalName) || !pSpoolFile );

	if (!m_bUseFinalName)
	{
		HRESULT		hr = RenameTempFileToFinalName(DEBUG_MASK_RCVD, uniqueID);
		if ( FAILED(hr) )
			return hr;
	}

	CString		szFilename;
	szFilename.Format("%s%X", (LPCTSTR)m_strBasePath, uniqueID);

	CInSpoolInfo *si = DEBUG_NEW CInSpoolInfo(szFilename, SPOOL_TYPE_RECEIVED, SS_RECEIVED, msgRec, pSpoolFile);

	AddToSpool(si);

	return S_OK;
}



//Implements failure recovery; 
void QCInSpool::BuildSpoolFromFiles()
{
	
	QCFileFinder finder;
	finder.FindFile(m_strBasePath + "*.RCV");

	CPopHost *pPopHost = CHostList::GetHost(GetIniLong(IDS_INI_LMOS_BACKUPS), m_strPersona);
	if(!pPopHost){
		ASSERT(0);
		return;
	}

	unsigned long uniqueID = 0;
	int nNumFilesRecovered = 0;

	while(finder.FindNextFile())
	{
		//Get the file UID or Hash
		CString strFileName(finder.GetFileName());
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
			TRACE("BuildSpool::RCV file didn't have an LMOS entry: \"%s\"\n", strFileName);
			ASSERT(0);
			// We'll continue on, as it would be better to show this message than just to ignore it
		}

		//build a OutSpoolInfo entry based on this information
											// ../spool/ + "123445"
		CInSpoolInfo *si = DEBUG_NEW CInSpoolInfo(m_strBasePath + strFileName, SPOOL_TYPE_RECEIVED, SS_RECEIVED, pMsgRecord);

		//Insert in out Spool Queue to be de-mime-d and filtered
		AddToSpool(si);
		nNumFilesRecovered++;

				//::FileRemoveMT(m_strBasePath  + finder.GetFileName());
	}//while

	if(nNumFilesRecovered)
	{
		//Create a dummy receiving task to initiate the PostIncomingMail process
		CPOPRecoverer *recover = DEBUG_NEW CPOPRecoverer(m_strPersona);
		QCGetTaskManager()->QueueWorkerThread(recover);
	}
}



bool QCFileFinder::FindNextFile()
{
	if (m_bFirstTime)
	{
		m_bFirstTime = false;
		m_hContext = ::FindFirstFile(m_SearchString, &data);
		if (m_hContext != INVALID_HANDLE_VALUE)
			return true;
	}
	
	if (m_hContext == INVALID_HANDLE_VALUE)
		return false;
	
	if (::FindNextFile(m_hContext, &data))
		return true;

	m_hContext = NULL;

	return false;
}


CString QCFileFinder::GetFileName()
{
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

	CSumList &		listSums = Toc->GetSumList();

	CSummary *Sum = NULL;
	// Count how many messages to send
	for (POSITION pos = listSums.GetHeadPosition(); pos; )
	{
		Sum = listSums.GetNext(pos);
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


// Fix the persona name not to contain bad characters
// This avoids mail going silently into a black hole
// if the user has a personality with an inappropriate
// name.
void FixPersonaName(CString &personaName)
{
	// Grab the list of chars we don't like
	
	//char baddies[64];
	//GetIniString(IDS_PERSONA_EXCLUDE_CHARS,baddies,sizeof(baddies));
	//oops, can't use GetIniString in here :-(
	char *baddies = "\\/:*?\"<>|";
	
	// Get a pointer to the personality name
	char *goodSpot = personaName.GetBuffer(1);
	char *spot = goodSpot;
	char *end = spot+strlen(spot);
	
	// Walk down the string, copying acceptable
	// characters back into the string
	while (spot<end)
	{
		if (!strchr(baddies,*spot)) *goodSpot++ = *spot;
		spot++;
	}
	*goodSpot = 0;	// and null-terminate
	
	// Ok, we're done now
	personaName.ReleaseBuffer(-1);
}


