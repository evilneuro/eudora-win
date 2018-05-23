#include "stdafx.h"
#include "resource.h"


#include "rs.h"			//for GetIni
#include "fileutil.h"	//for JJFile

#include "pop.h"		//for CSortedList
#include "pophost.h"
#include "MsgRecord.h"  //for CMsgRecord


//#include "Progress.h"   //for Progress Yuk
#include "Persona.h"    //for g_Personalities
#include "SpoolMgr.h"
#include "Summary.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif





//Thread-Safe constructor

////////////////////////////////////////////////////////////////////////
// CPopHost [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CPopHost::CPopHost(CString Persona, CString strCurrentAccount) :
	m_sServerType(kNonUIDL),
	m_bCanUIDL(FALSE),
	//m_posLastMsg(NULL),
	m_strPersona(Persona)
{
	//
	// All new hosts are defaulted to :
	//	1. the current account name...i.e. user@machine.domain
	//	2. last purge date of 0000
	//	3. server type of non-UIDL.
	//
	//m_pMsgListByMsgNum	= new CMsgNumList();

	if(strCurrentAccount != "")
	{
		if ( !m_strPersona.IsEmpty() )
		{
			strCurrentAccount += "-";
			strCurrentAccount += m_strPersona;
		}
	}
	else
		strCurrentAccount = "noname@nomachine.nodomain";

	SetAccountName(strCurrentAccount);
	SetLastPurge("0000");
	m_strLMOSFileName = QCSpoolMgrMT::GetLMOSFileName(m_strPersona);
}



//NON-Thread-Safe constructor	
////////////////////////////////////////////////////////////////////////
// CPopHost [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CPopHost::CPopHost() :
	m_sServerType(kNonUIDL),
	m_bCanUIDL(FALSE)
	//m_posLastMsg(NULL)
{
	//
	// All new hosts are defaulted to :
	//	1. the current account name...i.e. user@machine.domain
	//	2. last purge date of 0000
	//	3. server type of non-UIDL.
	//
	//m_pMsgListByMsgNum	= new CMsgNumList();

	char szCurrentAccount[128];
	GetIniString(IDS_INI_POP_ACCOUNT, szCurrentAccount, sizeof(szCurrentAccount));

	CString Persona;
	if (!*szCurrentAccount)
		strcpy(szCurrentAccount, "noname@nomachine.nodomain");
	else
	{
		Persona = g_Personalities.GetCurrent();
		if ( ! Persona.IsEmpty() )
		{
			strcat( szCurrentAccount, "-" );
			strcat( szCurrentAccount, Persona );
		}
	}

	SetAccountName(szCurrentAccount);
	SetLastPurge("0000");

	m_strPersona = Persona;
	//m_strLMOSFileName = QCSpoolMgrMT::GetLMOSFileName(m_strPersona);
	//CString QCSpoolMgrMT::GetLMOSFileName(CString strPersona)
	extern CString EudoraDir;
	ASSERT(!EudoraDir.IsEmpty());
	CString strSpoolRootDir = EudoraDir + "spool\\";

	CString m_strSpoolRootDir = EudoraDir + "spool\\";
	
	if(m_strPersona == "" || m_strPersona == "<Dominant>")
		m_strLMOSFileName = strSpoolRootDir + CRString(IDS_LMOS_FILENAME);
	else
		m_strLMOSFileName = strSpoolRootDir + m_strPersona + "\\" + CRString(IDS_LMOS_FILENAME);

	
}
	
	
	
	
	
	
	
////////////////////////////////////////////////////////////////////////
// ~CPopHost [public, destructor, virtual]
//
////////////////////////////////////////////////////////////////////////
CPopHost::~CPopHost()
{
	//delete m_pMsgListByMsgNum;
	//Delete all the records in the server
	int count = m_MsgRecords.size();
	//TRACE("Deleted %d Msg Records\n", count);
	for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		CMsgRecord* pMsgRecord = (*si).second;
		delete pMsgRecord;
	}

}


////////////////////////////////////////////////////////////////////////
// SetAccountNameAndLastPurge [public]
//
////////////////////////////////////////////////////////////////////////
bool CPopHost::SetAccountNameAndLastPurge(char* pszBuffer)
{
	
	ASSERT(::IsMainThreadMT());

	//
	// We assume the buffer contains a string of the form:
	//
	//   #POPSTART <c_wsakai@adept.qualcomm.com>\r\nXXXXXX\r\nYYYYYY
	//
	// where XXXXXX is a decimal value for the last purge datestamp and
	// YYYYYY is a decimal value for the server UIDL type.
	//
	char* pszTemp1 = strchr(pszBuffer, '<');	// find opening '<'
	char* pszTemp2 = strchr(pszBuffer, '>');	// find closing '>'
	
	if (pszTemp1 != NULL && pszTemp2 != NULL)
	{
		//
		// Copy account name to 'm_szAccountName'
		//
		pszTemp1++;								// skip leading '<'
		*pszTemp2 = 0;							// temporarily null terminate
		strcpy(m_szAccountName, pszTemp1);		// snag the data
		*pszTemp2 = ' ';						// undo temporary null termination
		
		//
		// Copy last purge timestamp to 'm_szLastPurge'
		//
		pszTemp1 = strstr(pszBuffer,"\r\n");	// find end of 1st line
		//RETURN_IF_NULL(pszTemp1);
		if(pszTemp1 == NULL) return false;

		pszTemp1 += 2;							// skip over end of line
		pszTemp2 = strstr(pszTemp1,"\r\n");		// find end of 2nd line
		//RETURN_IF_NULL(pszTemp2);
		if(pszTemp2 == NULL) return false;

		*pszTemp2 = 0;							// temporarily null terminate
		strcpy(m_szLastPurge,	pszTemp1);		// snag the data
		*pszTemp2 = '\r';						// undo temporary null termination

		//
		// Ignore the Server UIDL type.
		//
	}


	//Find out the persona that owns this account and set m_strPersona
	LPSTR lpPersonas = g_Personalities.List();
	CString cache = g_Personalities.GetCurrent();
	CString POPAcct;
	bool bExistsPersona = false;
	for( ; lpPersonas && *lpPersonas; lpPersonas += strlen(lpPersonas) +1)
	{
		g_Personalities.SetCurrent(lpPersonas);
		POPAcct = GetIniString(IDS_INI_POP_ACCOUNT);
		
		//Create the POP entry represented in LMOS file
		if(strcmp(lpPersonas, "<Dominant>") != 0)  //if dominant, take POP acct as it is..
		{
			POPAcct += "-";
			POPAcct += lpPersonas;
		}

		if(POPAcct.CompareNoCase(m_szAccountName) == 0){
			bExistsPersona = true;
			if(strcmp(lpPersonas, "<Dominant>") == 0)
				this->m_strPersona = "";
			else
				this->m_strPersona = lpPersonas;
			break;
		}
	}
	
	g_Personalities.SetCurrent(cache);
	
	//Set the LMOS file name
	m_strLMOSFileName = QCSpoolMgrMT::GetLMOSFileName(m_strPersona);

	return bExistsPersona;
}



//LOCKED ENTRY
//void CPopHost::ReplaceHost(CPopHost *pServerHost)
//{
	//CSingleLock lock(&m_LMOSGuard, TRUE);
	
	//remove all old message records
	/*
	POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
	while (pos)
	{
		CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(pos);
		delete pMsgRecord;
	}
	m_pMsgListByMsgNum->RemoveAll();
	*/
	/*

	//swap the lists
	std::swap(m_MsgRecords, pServerHost->m_MsgRecords);

	//Delete all the records in the server
	for(SI si = pServerHost->m_MsgRecords.begin(); si != pServerHost->m_MsgRecords.end(); ++si)
	{
		CMsgRecord* pMsgRecord = *si;
		pServerHost->m_MsgRecords.erase(si++);
		delete pMsgRecord;
	}

	//m_pMsgListByMsgNum->AddHead(pServerHost->m_pMsgListByMsgNum);
	
	//Copy UIDL caps and other fields
	m_bCanUIDL    = pServerHost->m_bCanUIDL;
	m_sServerType = pServerHost->m_sServerType;
	//m_posLastMsg  = pServerHost->m_posLastMsg;
	m_LastIterator  = pServerHost->m_LastIterator;

	delete pServerHost;
	*/
//}



//LOCKED ENTRY
int CPopHost::GetCount()
{
	CSingleLock lock(&m_LMOSGuard, TRUE);
	
	//return m_pMsgListByMsgNum->GetCount();
	return m_MsgRecords.size();
}







////////////////////////////////////////////////////////////////////////
// SetCanUIDL [public]
//
////////////////////////////////////////////////////////////////////////
void CPopHost::SetCanUIDL(BOOL bCanUIDL)
{
	m_bCanUIDL = bCanUIDL;

	//
	// Set the internal server type so that we can write out the
	// value to the LMOS file for informational purposes only.
	// The 'm_sServerType' is not used at runtime.  Use 'm_bCanUIDL'
	// instead.
	//
	if (bCanUIDL)
		m_sServerType = kUIDL;
	else
		m_sServerType = kNonUIDL;
}







////////////////////////////////////////////////////////////////////////
// GetLastPurge [public]
//
////////////////////////////////////////////////////////////////////////
long CPopHost::GetLastPurge() const
{
	return atol(m_szLastPurge);
}


////////////////////////////////////////////////////////////////////////
// SetLastPurge [public]
//
////////////////////////////////////////////////////////////////////////
void CPopHost::SetLastPurge(const char* pszLastPurge)
{
	strcpy(m_szLastPurge, pszLastPurge);
}


////////////////////////////////////////////////////////////////////////
// GetLastPurgeString [public]
//
////////////////////////////////////////////////////////////////////////
void CPopHost::GetLastPurgeString(char* pszLastPurge) const
{
	sprintf(pszLastPurge, "%ld", m_szLastPurge);
}


bool CPopHost::WriteLMOSHeader(CFile* pLMOSFile)
{
	char szBuffer[LMOS_HEADER_SIZE] = {0};
	long version = 1;
	memcpy(szBuffer, &version, LMOS_VERSION_SIZE);
	
	//write the account name
	memcpy(szBuffer+LMOS_POPACCT_OFFSET, m_szAccountName, LMOS_POPACCT_SIZE);

	//write the purge date
	long nPurgeDate = GetLastPurge();
	memcpy(szBuffer+LMOS_PURGEDATE_OFFSET, &nPurgeDate, LMOS_PURGEDATE_SIZE);

	memcpy(szBuffer+LMOS_SERVTYPE_OFFSET, &m_sServerType, LMOS_SERVTYPE_SIZE);

	pLMOSFile->Write(szBuffer, LMOS_HEADER_SIZE);
	
	return true;
}




//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// WriteLMOSData [public]
//
// Driver for writing LMOS data to file.
////////////////////////////////////////////////////////////////////////
bool CPopHost::WriteLMOSData()
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	//if lmos.dat file is not yet created we write directly into it
	//otherwise write into a temp file and rename it to lmos.dat for narrowing down the failure time window
	bool bFileExists = ::FileExistsMT(m_strLMOSFileName)?true:false;
	CString strTempLMOSFileName;
	if(bFileExists)
		strTempLMOSFileName = m_strLMOSFileName + ".tmp";
	else
		strTempLMOSFileName = m_strLMOSFileName;

	try{
	
		{
			CFile theLMOSFile;
			CFileException openexception;
			if(!theLMOSFile.Open(strTempLMOSFileName, CFile::modeCreate | CFile::modeReadWrite, &openexception) )
			{
				//Occationally, we are failing to open the .tmp file.  To remedy this problem, try to use another temp file
				strTempLMOSFileName = m_strLMOSFileName + ".tmp1";
				if(!theLMOSFile.Open(strTempLMOSFileName, CFile::modeCreate | CFile::modeReadWrite, &openexception) )
				{
					//still failed.  This shouldn't be happening :(
					ASSERT(0);
					return false;
				}
			}
			
			//write out the header first
			if( !WriteLMOSHeader(&theLMOSFile))
				return false;
			
			//
			// Write out the message record data.
			//
			//if (! m_pMsgListByMsgNum->WriteLMOSData(&theLMOSFile))
			//	return false;

			for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
			{
				CMsgRecord* pMsgRecord = (*si).second;
				if (!pMsgRecord->WriteLMOSData(&theLMOSFile))
					return false;
			}
		
			theLMOSFile.Flush();
			theLMOSFile.Close();

		} //delete CFile and CfileException stack objects

		
		//If we used the temp file, rename it back to lmos.dat
		if(bFileExists){ 
			
			//delete the lmos.dat
			if (!::DeleteFile(m_strLMOSFileName))
			{
				ASSERT(0);
				TRACE("File Removal error: %d\n", ::GetLastError());
			}
			
			//rename the temp file to lmos.dat
			if (!::MoveFile(strTempLMOSFileName, m_strLMOSFileName))
			{
				ASSERT(0);
				TRACE("File Rename error: %d\n", ::GetLastError());
			}

		}
		
	}
	catch(CFileException* fe)
	{
		TRACE("PopHost::Writing LMOS file \"%s\",  Cause: \"%d\"\n", fe->m_strFileName, fe->m_lOsError);
		ASSERT(FALSE);
		fe->Delete();
	    return false;
	}
	

	return true;
} 


//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// PurgeMessages [public]
//
// There are two main modes here.  Either 1) delete everything off the
// server, or 2) do auto-deletion of "expired" messages and deletion
// of messages that are explicitly marked for deletion.
////////////////////////////////////////////////////////////////////////
//void CPopHost::PurgeMessages(unsigned long ulBitFlags)
BOOL CPopHost::PurgeMessages(CPOPSession *popSession, unsigned long ulBitFlags, BOOL bLeaveMailOnServer, 
							BOOL bDeleteMailFromServer, unsigned long ulPurgePeriod)
{
	//BOOL bLeaveMailOnServer = BOOL(GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER));
	//BOOL bDeleteMailFromServer = BOOL(GetIniShort(IDS_INI_DELETE_MAIL_FROM_SERVER));
	//unsigned long ulPurgePeriod = GetIniLong(IDS_INI_LEAVE_ON_SERVER_DAYS);
	
	
	CSingleLock lock(&m_LMOSGuard, TRUE);
	
	time_t currentTime = 0;
	BOOL bDeleteAll = ulBitFlags & kDeleteAllBit;

	BOOL bMessagesPurged = FALSE;
	
	if (bLeaveMailOnServer && !bDeleteAll)
	{
		//
		// Delete from server after 0 days should never happen
		// (Options dialog won't allow it), but let's not expire
		// anything if the situation does come up.
		//
		if (!(ulBitFlags & kLMOSBit) || !bDeleteMailFromServer || (0 == ulPurgePeriod))
		{
			// If not deleting mail from server, then make sure expire time never gets reached.
			currentTime = 0;
			ulPurgePeriod = 0;
		}
		else
		{
			time(&currentTime);
			currentTime -= _timezone;			// apply GMT offset
			ulPurgePeriod *= 60L * 60L * 24L;
			ulPurgePeriod -= _timezone;			// apply GMT offset
		}
	}
	
	//::MainProgress(CRString(IDS_POP_REMOVING_MESSAGES));
	//::Progress(0, NULL, m_pMsgListByMsgNum->GetCount());
		
	//if (! m_pMsgListByMsgNum->IsEmpty())
	//{
		//POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
			
		
		for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
		{
			CMsgRecord* pMsgRecord = (*si).second;
			if (pMsgRecord)
			{
				//
				// Only delete read messages if told to do so, and
				// they are fully read, or we're nuking stuff off the
				// server
				//
				if ((  (ulBitFlags & kDeleteRetrievedBit) &&
						pMsgRecord->GetReadFlag() == LMOS_HASBEEN_READ /*0*/ && 
						pMsgRecord->GetSkippedFlag() == LMOS_DONOT_SKIP/*1*/
						)
					|| bDeleteAll
					)
				{
					if (pMsgRecord->GetDeleteFlag() != LMOS_DELETE_SENT /*-1*/)
					{
						//
						// Only mark message for deletion if the DELE
						// command was not previously sent for this
						// message.
						//
						pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE /*0*/);
					}
				}
				if( pMsgRecord->PurgeThisMessage(popSession, currentTime - ulPurgePeriod))
					bMessagesPurged = TRUE;
			}
			//::ProgressAdd(1);
		}
	//}

	//
	// Save off the time as when we last purged this host...
	//
	char szLastPurgeTime[16];
	sprintf(szLastPurgeTime, "%ld", currentTime);
	SetLastPurge(szLastPurgeTime);

	return bMessagesPurged;
}


//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// InsertMsgRecord [public]
//
////////////////////////////////////////////////////////////////////////
void CPopHost::InsertMsgRecord(CMsgRecord* pMsgRecord)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	ASSERT(pMsgRecord->GetDateLong());
	//m_pMsgListByMsgNum->InsertMsgRecord(theObject);
	//m_MsgRecords.insert(pMsgRecord);
	unsigned long ulHash = pMsgRecord->GetHashValue();
	ASSERT(ulHash > 0);
	MapValueType val(ulHash,pMsgRecord);
	std::pair <SI, bool> P = m_MsgRecords.insert(val);
	
	if( P.second != true)
	{
		//we come here if there is a duplicate UIDL thats in the map already
		//There is nothing we can do here, but to avoid memory leak delete that message record
		ASSERT(0);
		char buf[1024] = {0};
		pMsgRecord->DebugDump(buf, 1024);
		::PutDebugLog(DEBUG_MASK_RCVD, buf, strlen(buf));
		delete pMsgRecord;
	}

}


//LOCKED ENTRY

//For Non-UIDL servers, we have to update the Hash value after we insert in the PopHost
//This makes the hash index work correctly
void CPopHost::UpdateHashValue(CMsgRecord *pMsgRecord, unsigned long ulHash)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	unsigned long ulOldHash = pMsgRecord->GetHashValue();
	SI si = m_MsgRecords.find(ulOldHash);
	if( si != m_MsgRecords.end())
	{
		m_MsgRecords.erase(si);
	}

	pMsgRecord->SetHashValue(ulHash);
	InsertMsgRecord(pMsgRecord);
}



//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// GetMsgByHash [public]
//
// Returns the element in the message list that has the matching
// hashID value.  Returns NULL if not found.
////////////////////////////////////////////////////////////////////////
CMsgRecord* CPopHost::GetMsgByHash(unsigned long ulHash)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	/*POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
	while (pos)
	{
		CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(pos);
		if (pMsgRecord->GetHashValue() == ulHash)
			return pMsgRecord;
	}*/

	/*for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		CMsgRecord* pMsgRecord = (*si).second;
		unsigned long MsgHash = pMsgRecord->GetHashValue();
		if (MsgHash == ulHash)
			return pMsgRecord;
	}*/

	CSI si = m_MsgRecords.find(ulHash);
	if( si != m_MsgRecords.end())
		return (*si).second;


	return NULL;
}



//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// GetMsgByMsgNum [public]
//
// Returns the element in the message list that has the matching
// message number.  Returns NULL if not found.
////////////////////////////////////////////////////////////////////////
CMsgRecord* CPopHost::GetMsgByMsgNum(int nMsgNum)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	/*POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
	while (pos)
	{
		CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(pos);
		if (pMsgRecord->GetMsgNum() == nMsgNum)
			return pMsgRecord;
	}*/
	for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		CMsgRecord* pMsgRecord = (*si).second;
		if ( pMsgRecord->GetMsgNum() == nMsgNum)
			return pMsgRecord;
	}

	
	return NULL;
}


//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// GetNextFetchableMsgNum [public]
//
// Seek to the next message record in the MsgListByMsgNum list and return
// its message number if it is "fetchable".  Otherwise, return -1 if there
// are no more messages.
////////////////////////////////////////////////////////////////////////
CMsgRecord* CPopHost::GetNextFetchableMsgNum(unsigned long ulBitFlags, BOOL bFirstTime /*= FALSE*/)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	if(m_MsgRecords.size() == 0)
	{
		return NULL;
	}

	if (bFirstTime)
		m_LastIterator = m_MsgRecords.begin();
	
	
	while (m_LastIterator != m_MsgRecords.end())
	{
		//CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(m_posLastMsg);
		CMsgRecord* pMsgRecord = (*m_LastIterator).second;

		if (NULL == pMsgRecord)
		{
			ASSERT(0);
			++m_LastIterator;
			continue;
		}
		
		//
		// Get the message if:
		//   1) Doing a fetch the headers of all messages on the server
		//   2) Doing a fetch of marked messages, and the message is marked
		//   3) Doing a fetch of new messages, and the message is new
		//
		// FORNOW, does this properly handle the case where we want
		// only explicitly marked messages while ignoring new
		// messages?  Yes, it looks like this case is taken care of in
		// CMsgRecord::GetMsgFetchType().
		//
		if ((ulBitFlags & kFetchHeadersBit) ||
			((ulBitFlags & kRetrieveMarkedBit) && pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE/*0*/) ||
			((ulBitFlags & kRetrieveNewBit) && pMsgRecord->GetReadFlag() == LMOS_NOT_READ/*1*/))
		{
			++m_LastIterator;
			//return pMsgRecord->GetMsgNum();
			return pMsgRecord;
		}

		++m_LastIterator;
	}
	
	return NULL;		// no more messages
}






bool CPopHost::LoadLMOSHeader(CFile *pLMOSFile)
{
	
	char szBuffer[LMOS_HEADER_SIZE] = {0};
	if(pLMOSFile->Read(&szBuffer, LMOS_HEADER_SIZE) != LMOS_HEADER_SIZE)
			return false;
	
	int num_version = 0;
	memcpy(&num_version, szBuffer, 4);
	ASSERT(num_version == 1);

	strncpy(m_szAccountName, szBuffer + LMOS_POPACCT_OFFSET, LMOS_POPACCT_SIZE);
	m_szAccountName[LMOS_POPACCT_SIZE] = 0;

	long nPurgeDate = 0;
	memcpy(&nPurgeDate, szBuffer+LMOS_PURGEDATE_OFFSET, LMOS_PURGEDATE_SIZE);
	sprintf(m_szLastPurge, "%ld", nPurgeDate);

	//Ignore Server UIDL type FORNOW
	//memcpy(&m_sServerType, szBuffer+LMOS_SERVTYPE_OFFSET, LMOS_SERVTYPE_SIZE);

	return true;
}

////////////////////////////////////////////////////////////////////////
// ReadLMOSData [public]
//
// Read all LMOS data records from the given LMOS file, until we hit a
// #POPEND token.
////////////////////////////////////////////////////////////////////////
bool CPopHost::LoadLMOSData()
{
	int ulMessageIndex = 1;
	bool bBadData = false;
	
	CFile theLMOSFile;
	CFileException fexcept;
	try{
	
		if( !theLMOSFile.Open(m_strLMOSFileName, CFile::modeRead, &fexcept))
		{
			//lmos.dat might not have been created by now..this is ok for new personas that just got
			//created
			//these error are expected ENOENT or directory non-existant ESRCH
			if(!(fexcept.m_lOsError == ENOENT || fexcept.m_lOsError == ESRCH))
			{
				ASSERT(0);  //if none of above errors the what is it??
			}
			return false;
		}
		
		if(!LoadLMOSHeader(&theLMOSFile))
			return false;		//could not getter header 

		//The file pointer is ready for getting data right after getting the header
		char szBuffer[LMOS_RECORD_SIZE] = {0};
	
		while ( (theLMOSFile.Read(szBuffer, LMOS_RECORD_SIZE) == LMOS_RECORD_SIZE) &&  bBadData == false)
		{
			
			//
			// If we get this far, we should have a valid message record.
			// Pass the line of data to CreateMsgRecord() to create a corresponding
			// message record object.
			//
			//strcat(szBuffer, "\r\n");		// add back stripped off CRLF
			
			CMsgRecord* pMsgRecord = CMsgRecord::CreateMsgRecordFromFile(m_strPersona, szBuffer, LMOS_RECORD_SIZE, &ulMessageIndex);
			if (pMsgRecord != NULL)
			{
				//
				// Successfully created message record object, so store it
				// in the list.
				//
				fBaptizeBlockMT(pMsgRecord, "MsgRec From LMOS file");
				InsertMsgRecord(pMsgRecord);
				memset(szBuffer, 0, sizeof(szBuffer));
			}
			else
			{
				bBadData = true;		// bad data, or out of memory
				break;
			} 
		}
		
	}
	catch(CFileException* fe)
	{	
		TRACE("PopHost::Loading LMOS file \"%s\",  Cause: \"%d\"\n", fe->m_strFileName, fe->m_lOsError);
	    ASSERT(FALSE);
		fe->Delete();
	    return false; 
	}

	if(bBadData) 
		return false;

	theLMOSFile.Close();
	return true;
}


////////////////////////////////////////////////////////////////////////
// ReadLMOSData [public]
//
// Read all LMOS data records from the given LMOS file, until we hit a
// #POPEND token.
////////////////////////////////////////////////////////////////////////
BOOL CPopHost::ReadLMOSData(JJFile* pLMOSFile)
{
	//s_ulMessageIndex = 1;
	int ulMessageIndex = 1;

	//
	// Save the '#POPEND' keyword.
	//
	char szEndKeyword[64];
	strcpy(szEndKeyword, "#POPEND");
	//GetIniString(IDS_LMOS_HOST_END, szEndKeyword, sizeof(szEndKeyword) - 1);

	//
	// First lets get the next line of data from the file
	//
	char szBuffer[kLMOSBuffSize];
	memset(szBuffer, 0, sizeof(szBuffer));
	BOOL bDone = FALSE;
	long lNumBytesRead = 0;
	while (SUCCEEDED(pLMOSFile->GetLine(szBuffer, sizeof(szBuffer), &lNumBytesRead)) &&
		   (lNumBytesRead > 0) &&
		   !bDone)
	{
		if (strstr(szBuffer, szEndKeyword) != NULL)
		{
			//
			// Hit '#POPEND' token, so we're outta here.
			//
			bDone = TRUE;
			break;
		}

		//
		// If we get this far, we should have a valid message record.
		// Pass the line of data to CreateMsgRecord() to create a corresponding
		// message record object.
		//
		strcat(szBuffer, "\r\n");		// add back stripped off CRLF
		CMsgRecord* pMsgRecord = CMsgRecord::CreateMsgRecord(m_strPersona, kFromFile, szBuffer, strlen(szBuffer), &ulMessageIndex);
		if (pMsgRecord != NULL)
		{
			//
			// Successfully created message record object, so store it
			// in the list.
			//
			InsertMsgRecord(pMsgRecord);
			memset(szBuffer, 0, sizeof(szBuffer));
		}
		else
		{
			bDone = TRUE;		// bad data, or out of memory
			break;
		} 
	}

	//s_ulMessageIndex = 0;
	
	return bDone;
}




//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// ConditionMessagesPriorTo [public]
//
// For all messages up to 'nFirstUnread', mark them as
// "programmatically" read if it was marked for fetch OR if not marked
// for fetch and it was skipped.  Also, if we're not leaving messages
// on the server, then mark all messages up to 'nFirstUnread' for
// deletion.
////////////////////////////////////////////////////////////////////////
void CPopHost::ConditionMessagesPriorTo(int nFirstUnread, bool bDeleteFromServer)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	//BOOL bDeleteFromServer = (GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) == 0);
	//POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
	SI si = m_MsgRecords.begin();

	
	for (int i = 1; (si != m_MsgRecords.end()) && i < nFirstUnread; i++, ++si)
	{
		//CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(pos);
		CMsgRecord* pMsgRecord = (*si).second;
		if (pMsgRecord != NULL)
		{
			//ASSERT(pMsgRecord->GetRetrieveFlag() != -1);
			//if (pMsgRecord->GetRetrieveFlag())
			if (pMsgRecord->GetRetrieveFlag() != LMOS_RETRIEVE_MESSAGE)
			{
				if (pMsgRecord->GetSkippedFlag() == LMOS_SKIP_MESSAGE /*0*/)
				{
					//
					// If we get here, this old message was not marked 
					// for retrieval and was skipped, so just leave it
					// on the server without downloading it again.
					//
					pMsgRecord->SetReadFlag(LMOS_READ_PROGRAMMATICALLY /*-1*/);	// mark as being read "programmatically"

//FORNOW					//
//FORNOW					// FORNOW, this is the only place where this is set
//FORNOW					// to -1.  Normally, 0 means "get" and 1 means "Nget".
//FORNOW					//
//FORNOW					pMsgRecord->SetRetrieveFlag(-1);	// don't retrieve it
				}
			}
			else
			{
				//
				// Message marked as "get", so mark it as read "programmatically".
				//
				pMsgRecord->SetReadFlag(LMOS_READ_PROGRAMMATICALLY /*-1*/);
			}
				
			//
			// If not leaving mail on server, then mark it to be deleted
			//
			if (bDeleteFromServer)
				pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE /*0*/);
		}
	}
}

//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// PadMsgList [public]
//
// Given an message list with GetCount() objects, extend the list to 
// include 'nMsgCount' objects, with dummy message number values.
////////////////////////////////////////////////////////////////////////
void CPopHost::PadMsgList(int nMsgCount)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);
	
	const int COUNT =  m_MsgRecords.size() + 1;
	
	//
	// More efficient to add in reverse order since insertion starts at the
	// beginning of the list looking for the spot to add, so the closer to the
	// front the insertion the quicker it is
	//
	for (int i = nMsgCount; i >= COUNT; i--)
	{
		CMsgRecord* pMsgRecord = new CMsgRecord();
		char szMsgNum[32];
		sprintf(szMsgNum, "%d", i);
		pMsgRecord->SetMsgNum(szMsgNum);
		pMsgRecord->SetHashString(szMsgNum, m_strPersona);
		InsertMsgRecord(pMsgRecord);
	}
}



//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// RemoveDeletedMsgRecords [public]
//
// Remove the deleted message records from the LMOS database, then
// renumber any remaining message records (corresponding to the messages
// left on the server).
////////////////////////////////////////////////////////////////////////
void CPopHost::RemoveDeletedMsgRecords()
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	//
	// Walk the entire list in search of messages marked as deleted,
	// and delete them as found.
	//
	/*
	POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
	while (pos)
	{
		POSITION posDelete = pos;		// Save the current position.
		CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(pos);
		if (pMsgRecord && pMsgRecord->GetDeleteFlag() == LMOS_DELETE_SENT -1)
		{
			//
			// Found a message marked as "delS" (delete sent),
			// so remove this entry from the LMOS database.
			//
			m_pMsgListByMsgNum->RemoveAt(posDelete);
			delete pMsgRecord;
		}
	}*/

	//TRACE("Deleting messages with LMOS_DELETE_SENT flag on \n[ ");

	SI si = m_MsgRecords.begin();
	while( si != m_MsgRecords.end())
	{
		CMsgRecord* pMsgRecord = (*si).second;
		if (pMsgRecord && pMsgRecord->GetDeleteFlag() == LMOS_DELETE_SENT /*-1*/)
		{
			//remove it from the LMOS file
			m_MsgRecords.erase(si++);
			delete pMsgRecord;
		}
		else
		{
			++si; //point to the next record
		}
	}
	//TRACE(" ]\n");
	
		
	//
	// Renumber any messages that are left on the server.
	//
	/*
	int nMsgNum = 1;
	pos = m_pMsgListByMsgNum->GetHeadPosition(); 
	while (pos)
	{
		CMsgRecord* pMsgRecord = m_pMsgListByMsgNum->GetNext(pos);
		if (pMsgRecord)
			pMsgRecord->SetMsgNum(nMsgNum++);
		else
		{
			ASSERT(0);
		}
	}*/


	int nMsgNum = 1;
	
	for(si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		CMsgRecord* pMsgRecord = (*si).second;
		if (pMsgRecord)
			pMsgRecord->SetMsgNum(nMsgNum++);
		else
		{
			ASSERT(0);
		}
	}

}


//Used onlu in Non-UIDL servers	
void CPopHost::ReplaceDeleteSentWithDelete()
{
	CSingleLock lock(&m_LMOSGuard, TRUE);


	// The high index matched, so we need to mark the entries that were
	// marked as having the DELE command sent back to just plain delete
	
	for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		CMsgRecord* rec = (*si).second;
		if (rec && rec->GetDeleteFlag() == LMOS_DELETE_SENT)
			rec->SetDeleteFlag(LMOS_DELETE_MESSAGE, true);
	}
}


CMsgRecord *CPopHost::FindHighLowIndices(int& HighIndex, int& LowIndex, int nLast)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);


	// The message could be one of two places.  If the messages marked as having the
	// DELE command sent to them didn't actually get deleted, then the last message
	// will be correctly placed at the end of the list.  However, if messages did
	// get deleted, then we need to subtract off all the messages that are marked as
	// having the DELE command sent to them.
	HighIndex = LowIndex = nLast;
	int DeleteCount = 0;
	int LastDefined = 0;
		
	CMsgRecord* theLastMsg = NULL;
	//for (int i = 1; pos; i++)
	int i=0;
	for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		i++;
		CMsgRecord* theRecord = (*si).second;
		if (theRecord)
		{
			if (theRecord->IsEqualMesgID("udef") == FALSE)
			{
				theLastMsg = theRecord;
				LastDefined = i;
			}
			if (theRecord->GetDeleteFlag() == LMOS_DELETE_SENT)
				DeleteCount++;
		}
	}

	HighIndex = min(HighIndex, LastDefined);
	LowIndex = HighIndex - DeleteCount;
	
	return theLastMsg;
}



//Used by CMailTranferOptions Dialog for getting the following flags
void CPopHost::GetTransferOptions(BOOL& SendingFlag, BOOL& DeleteMarkedExist, BOOL& DeleteMarked, 
							 BOOL& RetrieveMarkedExist, BOOL& RetrieveMarked )
{

	CSingleLock lock(&m_LMOSGuard, TRUE);

	//POSITION pos = m_pMsgListByMsgNum->GetHeadPosition();
	//while (pos)
	for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
	{
		CMsgRecord* Msg = (*si).second;
		//CMsgRecord* Msg = m_pMsgListByMsgNum->GetNext(pos);
		if (Msg)
		{
			if (Msg->GetDeleteFlag() == LMOS_DELETE_MESSAGE /*0*/)
			{
				DeleteMarkedExist = TRUE;
				if (!SendingFlag)
					DeleteMarked = TRUE;
			}
			if (Msg->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE /*0*/)
			{
				RetrieveMarkedExist = TRUE;
				if (!SendingFlag)
					RetrieveMarked = TRUE;
			}
			if (DeleteMarked && RetrieveMarked)
				break;
		}
	}

}






BOOL CPopHost::ReconcileLists_(CPopHost* pDiskHost, CPopHost* pServerHost)
{
	if (!pDiskHost || !pServerHost)
		return FALSE;


	//
	// Walk the list of new UIDLs, looking to see if the message exists
	// in our old LMOS database.  If it does exist, then we need to preserve
	// the stored LMOS data info for that message.  If it doesn't exist,
	// that means it is a new message.
	//
	
	
	//Remove disk LMOS records that are non-existent on the server
	SI si = pDiskHost->m_MsgRecords.begin();
	while(si != pDiskHost->m_MsgRecords.end())
	{
		// Lookup the message to see if it exists in the old LMOS database.
		CMsgRecord* pDiskMsg = (*si).second;
		
		//Find corrsponding message on the disk if this message is already downloaded once
		CMsgRecord* pServerMsg = pServerHost->GetMsgByHash(pDiskMsg->GetHashValue());

		//if LMOS record is not found on the server, remove from the disk also
		if(!pServerMsg){
			pDiskHost->m_MsgRecords.erase(si++); //erase and point to the next record
			delete pDiskMsg;
		}
		else
		{
			//VERY IMPORTANT PART of RECONCILATION....
			++si;		//point to the next record

			//update with the new server assigned message number
			pDiskMsg->SetMsgNum( pServerMsg->GetMsgNum());
			
			//if disk has the delete sent flag, then obviously its not deleted
			//remark it to be deleted
			if( pDiskMsg->GetDeleteFlag() == LMOS_DELETE_SENT)
			{
				ASSERT(0);
				pDiskMsg->SetDeleteFlag(LMOS_DELETE_MESSAGE, true /*ignore delete_sent flag*/);
			}

			
		}
			
	}


	//Now insert new messages from the server
	//for(si = pServerHost->m_MsgRecords.begin(); si != pServerHost->m_MsgRecords.end(); ++si)
	si = pServerHost->m_MsgRecords.begin();
	while( si != pServerHost->m_MsgRecords.end())
	{
		// Lookup the message to see if it exists in the old LMOS database.
		CMsgRecord* pServerMsg = (*si).second;
		
		//Find corrsponding message on the disk if this message is already downloaded once
		CMsgRecord* pDiskMsg = pDiskHost->GetMsgByHash(pServerMsg->GetHashValue());

		//if LMOS record is not found on the server, remove from the disk also
		if(!pDiskMsg){
			pDiskHost->InsertMsgRecord(pServerMsg);
			//erase from server so that the pointer(MsgRecord) won't be deleted later.
			pServerHost->m_MsgRecords.erase(si++); //erase & point to the next record
		}
		else{
			++si;  //point to the next record
		}
	}

	//pDiskHost->ReplaceHost(pServerHost);
	m_bCanUIDL    = pServerHost->m_bCanUIDL;
	m_sServerType = pServerHost->m_sServerType;

	//TRACE("Disk Recs %d, server recs %d\n", pDiskHost->GetCount(), pServerHost->GetCount());

#ifdef _DEBUG
	//Just for debugging purposes
	for(si = pDiskHost->m_MsgRecords.begin(); si != pDiskHost->m_MsgRecords.end(); ++si)
	{
		// Lookup the message to see if it exists in the old LMOS database.
		CMsgRecord* pDiskMsg = (*si).second;
		ASSERT(pDiskMsg->GetMsgNum() < 10000);
		char buf[1024] = {0};
		pDiskMsg->DebugDump(buf, 1024);
		::PutDebugLog(DEBUG_MASK_RCVD, buf, strlen(buf));
	}
#endif

	
	delete pServerHost;
	return TRUE;
}








/*
 
 
//	===================================================================================
//	===================================================================================
//	===================================================================================
//	===================================================================================
CSortedList::~CSortedList()
{
	// clean out the list items
	while (! IsEmpty())
		delete RemoveHead();
}



////////////////////////////////////////////////////////////////////////
// Compare [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
short CSortedList::Compare(CMsgRecord* pMsgRecord1, CMsgRecord* pMsgRecord2)
{
	long lDate1 = pMsgRecord1->GetDateLong();
	long lDate2 = pMsgRecord2->GetDateLong();
   
	if (lDate1 < lDate2)
		return -1;
	else if (lDate1 > lDate2)
		return 1;
	else
		return 0;
}
*/


//INI settings
/*
IDS_INI_POP_ACCOUNT
IDS_INI_LEAVE_MAIL_ON_SERVER
IDS_INI_DELETE_MAIL_FROM_SERVER
IDS_INI_LEAVE_ON_SERVER_DAYS

  ///IDS_LMOS_HOST_END       //now hardcoded with #POPEND for thread-safety
*/


/*
////////////////////////////////////////////////////////////////////////
// IsMe [public]
//
// Returns TRUE if the given name is identical to the internal account
// name.
////////////////////////////////////////////////////////////////////////
BOOL CPopHost::IsMe(const char* pszAccountName) const
{
	if (strcmp(m_szAccountName, pszAccountName) == 0)
		return TRUE;

	return FALSE;
}


BOOL CPopHost::\const CString& strPersona, CString strCurrentAccount) const
{
	if(strCurrentAccount != "")
	{
		if ( !strPersona.IsEmpty() )
		{
			strCurrentAccount += "-";
			strCurrentAccount += strPersona;
		}
	}
		
	if (strcmp(m_szAccountName, strCurrentAccount) == 0)
		return TRUE;

	return FALSE;
}
*/

/*
////////////////////////////////////////////////////////////////////////
// WriteLMOSData [public]
//
// Driver for looping through all message record objects and writing
// their LMOS data out to disk.
////////////////////////////////////////////////////////////////////////
BOOL CMsgNumList::WriteLMOSData(JJFile* pLMOSFile)
{
	if (IsEmpty())
	{
		if (SUCCEEDED(pLMOSFile->PutLine("NoMessages")))
			return TRUE;
		else
			return FALSE;
	}

	//
	// If we get this far, we have work to do...
	//
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		CMsgRecord* pMsgRecord = GetNext(pos);
		if (! pMsgRecord->WriteLMOSData(pLMOSFile))
			return FALSE;
	}

	return TRUE;
}


*/

/*
////////////////////////////////////////////////////////////////////////
// WriteLMOSData [public]
//
// Driver for writing LMOS data to file.
////////////////////////////////////////////////////////////////////////
BOOL CPopHost::WriteLMOSData(JJFile* pLMOSFile)
{
	char szBuffer[500];

	//
	// Write out first three lines containing account name, last
	// purge datestamp, and Server UIDL type.
	//
	sprintf(szBuffer,"#POPSTART <%s> \r\n%s\r\n%d", m_szAccountName, m_szLastPurge, m_sServerType);
	if (FAILED(pLMOSFile->PutLine(szBuffer, strlen(szBuffer))))
		return FALSE;

	//
	// Write out the message record data.
	//
	if (! m_pMsgListByMsgNum->WriteLMOSData(pLMOSFile))
		return FALSE;

	//
	// Write out the terminating line.
	//
	strcpy(szBuffer, "#POPEND");	
	if (FAILED(pLMOSFile->PutLine(szBuffer, strlen(szBuffer))))
		return FALSE;

	return TRUE;
} 
*/

                                                               


