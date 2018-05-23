// pophost.cpp
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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

#include "DebugNewHelpers.h"





//Thread-Safe constructor

////////////////////////////////////////////////////////////////////////
// CPopHost [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CPopHost::CPopHost(const char* Persona, const char* strCurrentAccount, long nLMOSBackupCount)
	:	m_sServerType(kNonUIDL), m_bCanUIDL(FALSE), m_strPersona(Persona),
		m_nLMOSBackupCount(nLMOSBackupCount), m_timeNextLMOSBackup(0)
{
	//
	// All new hosts are defaulted to :
	//	1. the current account name...i.e. user@machine.domain
	//	2. last purge date of 0000
	//	3. server type of non-UIDL.
	//
	//m_pMsgListByMsgNum	= new CMsgNumList();

	if (strCurrentAccount && *strCurrentAccount)
	{
		CString AccountPersonaCombo(strCurrentAccount);
		if ( !m_strPersona.IsEmpty() )
		{
			AccountPersonaCombo += "-";
			AccountPersonaCombo += m_strPersona;
		}
		SetAccountName(AccountPersonaCombo);
	}
	else
		SetAccountName("noname@nomachine.nodomain");

	SetLastPurge("0000");
	m_strLMOSFileName = QCSpoolMgrMT::GetLMOSFileName(m_strPersona);
}



//NON-Thread-Safe constructor	
////////////////////////////////////////////////////////////////////////
// CPopHost [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CPopHost::CPopHost()
	:	m_sServerType(kNonUIDL), m_bCanUIDL(FALSE), m_timeNextLMOSBackup(0)
{
	//
	// All new hosts are defaulted to :
	//	1. the current account name...i.e. user@machine.domain
	//	2. last purge date of 0000
	//	3. server type of non-UIDL.
	//
	//m_pMsgListByMsgNum	= new CMsgNumList();

	CString			szCurrentAccount;
	GetIniString(IDS_INI_POP_ACCOUNT, szCurrentAccount);

	CString Persona;
	if ( szCurrentAccount.IsEmpty() )
		szCurrentAccount = "noname@nomachine.nodomain";
	Persona = g_Personalities.GetCurrent();
	if ( ! Persona.IsEmpty() )
	{
		szCurrentAccount += "-";
		szCurrentAccount += Persona;
	}

	SetAccountName(szCurrentAccount);
	SetLastPurge("0000");

	m_strPersona = Persona;
	//m_strLMOSFileName = QCSpoolMgrMT::GetLMOSFileName(m_strPersona);
	
	// As far as I can tell, this was the right thing to have done
	// nevermind it being commented out above.  SD 1/2/2000
	m_strLMOSFileName = QCSpoolMgrMT::GetLMOSFileName(m_strPersona);

	m_nLMOSBackupCount = GetIniLong(IDS_INI_LMOS_BACKUPS);
}
	
	
	
	
	
	
	
////////////////////////////////////////////////////////////////////////
// ~CPopHost [public, destructor, virtual]
//
////////////////////////////////////////////////////////////////////////
CPopHost::~CPopHost()
{
	//delete m_pMsgListByMsgNum;
	//Delete all the records in the server
	//int count = m_MsgRecords.size();
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
		m_szAccountName = pszTemp1;				// snag the data
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
	CRString Dominant(IDS_DOMINANT);
	CString cache = g_Personalities.GetCurrent();
	CString POPAcct;
	bool bExistsPersona = false;
	for( ; lpPersonas && *lpPersonas; lpPersonas += strlen(lpPersonas) +1)
	{
		g_Personalities.SetCurrent(lpPersonas);
		POPAcct = GetIniString(IDS_INI_POP_ACCOUNT);
		
		//Create the POP entry represented in LMOS file
		if(strcmp(lpPersonas, Dominant) != 0)  //if dominant, take POP acct as it is..
		{
			POPAcct += "-";
			POPAcct += lpPersonas;
		}

		if(POPAcct.CompareNoCase(m_szAccountName) == 0){
			bExistsPersona = true;
			if(strcmp(lpPersonas, Dominant) == 0)
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
	strncpy(szBuffer+LMOS_POPACCT_OFFSET, m_szAccountName, LMOS_POPACCT_SIZE);

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
	if (bFileExists)
	{
		time_t	timeCurrent = time(NULL);

		if (timeCurrent > m_timeNextLMOSBackup)
		{
			if ( QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS) )
			{
				CString szLogEntry;
				szLogEntry.Format("Backing up LMOS file for %s", static_cast<LPCTSTR>(m_szAccountName));
				PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
			}
			
			// Backup LMOS file specified number of times
			CascadeBackupFile( m_strLMOSFileName, m_nLMOSBackupCount );

			// Backup once a minute - avoids losing all useful backups because
			// of one active session where the LMOS gets written a bunch of times.
			// Normally I'd make this a setting, but I'd rather conserve settings
			// at the moment (running low). Also passing along the setting value
			// was such a PITA that I don't feel like doing it again.
			m_timeNextLMOSBackup = timeCurrent + 60;
		}

		strTempLMOSFileName = GetTmpFileNameMT("lmo");
	}
	else
	{
		strTempLMOSFileName = m_strLMOSFileName;
	}

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

			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
			{
				CString szLogEntry;
				szLogEntry.Format("Writing LMOS file for %s", (LPCTSTR)m_szAccountName);
				PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
			}
			
			//write out the header first
			if( !WriteLMOSHeader(&theLMOSFile))
				return false;
			
			for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si)
			{
				CMsgRecord* pMsgRecord = (*si).second;
				// Log the LMOS data.
				if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
				{
					char		 szBuf[256];
					char		 szHash[16];
					pMsgRecord->GetHashString(szHash);
					sprintf(szBuf, "Msg %d hash %s gmt %X flags 0x%04X %c %c %c %c %c",
							pMsgRecord->GetMsgNum(), szHash, pMsgRecord->GetDateLong(), pMsgRecord->GetFlags(),
							(pMsgRecord->GetRetrieveFlag() & LMOS_RETRIEVE_MESSAGE) ? 'F' : 'f',
							(pMsgRecord->GetReadFlag() & LMOS_HASBEEN_READ) ? 'R' : 
								((pMsgRecord->GetReadFlag() & LMOS_NOT_READ) ? 'r' : 'P'),
							(pMsgRecord->GetSkippedFlag() & LMOS_SKIP_MESSAGE) ? 'K' :
								((pMsgRecord->GetSkippedFlag() & LMOS_DONOT_SKIP) ? 'k' : 'w'),
							(pMsgRecord->GetSaveFlag() & LMOS_SAVE_MESSAGE) ? 'S' : 's',
							(pMsgRecord->GetDeleteFlag() & LMOS_DELETE_MESSAGE) ? 'D' :
								((pMsgRecord->GetDeleteFlag() & LMOS_DONOT_DELETE) ? 'd' : 't'));
					PutDebugLog(DEBUG_MASK_LMOS, szBuf);
				}
				if (!pMsgRecord->WriteLMOSData(&theLMOSFile))
					return false;
			}
		
			theLMOSFile.Flush();
			theLMOSFile.Close();

		} //delete CFile and CfileException stack objects

		
		//If we used the temp file, rename it back to lmos.dat
		if(bFileExists){ 
			
			HRESULT		hr = FileRenameReplaceMT(strTempLMOSFileName, m_strLMOSFileName);

			if ( FAILED(hr) )
			{
				ASSERT(!"FileRenameReplaceMT failed with LMOS file");

				PutDebugLog(DEBUG_MASK_LMOS, "Renaming LMOS file failed");
			}
		}
		
	}
	catch(CFileException* fe)
	{
		ASSERT(FALSE);

		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
		{
			CString szLogEntry;
			szLogEntry.Format("Writing LMOS file \"%s\" failed. Cause: %d", fe->m_strFileName, fe->m_lOsError);
			PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
		}

		fe->Delete();
	    return false;
	}
	

	return true;
} 

//LOCKED ENTRY
////////////////////////////////////////////////////////////////////////
// WriteSingleLMOSRecord [public]
//
// Write a single LMOS record to disk.  Fast because it only updates
// the part of the file for the passed in record.
////////////////////////////////////////////////////////////////////////
bool CPopHost::WriteSingleLMOSRecord(CMsgRecord* pMsgRecord)
{
	CSingleLock lock(&m_LMOSGuard, TRUE);

	try
	{
		CFile theLMOSFile;
		if (theLMOSFile.Open(m_strLMOSFileName, CFile::modeWrite))
		{
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
			{
				CString szLogEntry;
				szLogEntry.Format("Writing single record of LMOS file for %s", (LPCTSTR)m_szAccountName);
				PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
			}

			int i = 0;
			for(SI si = m_MsgRecords.begin(); si != m_MsgRecords.end(); ++si, i++)
			{
				if (pMsgRecord == (*si).second)
				{
					// Log the LMOS data.
					if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
					{
						char		 szBuf[256];
						char		 szHash[16];
						pMsgRecord->GetHashString(szHash);
						sprintf(szBuf, "Msg %d hash %s gmt %X flags 0x%04X %c %c %c %c %c",
								pMsgRecord->GetMsgNum(), szHash, pMsgRecord->GetDateLong(), pMsgRecord->GetFlags(),
								(pMsgRecord->GetRetrieveFlag() & LMOS_RETRIEVE_MESSAGE) ? 'F' : 'f',
								(pMsgRecord->GetReadFlag() & LMOS_HASBEEN_READ) ? 'R' : 
									((pMsgRecord->GetReadFlag() & LMOS_NOT_READ) ? 'r' : 'P'),
								(pMsgRecord->GetSkippedFlag() & LMOS_SKIP_MESSAGE) ? 'K' :
									((pMsgRecord->GetSkippedFlag() & LMOS_DONOT_SKIP) ? 'k' : 'w'),
								(pMsgRecord->GetSaveFlag() & LMOS_SAVE_MESSAGE) ? 'S' : 's',
								(pMsgRecord->GetDeleteFlag() & LMOS_DELETE_MESSAGE) ? 'D' :
									((pMsgRecord->GetDeleteFlag() & LMOS_DONOT_DELETE) ? 'd' : 't'));
						PutDebugLog(DEBUG_MASK_LMOS, szBuf);
					}

					// Seek to the spot in the file where the record resides
					theLMOSFile.Seek(LMOS_HEADER_SIZE + (i * LMOS_RECORD_SIZE), CFile::begin);

					if (!pMsgRecord->WriteLMOSData(&theLMOSFile))
						return false;
					break;
				}
			}

			theLMOSFile.Flush();
			theLMOSFile.Close();
		}
	}
	catch(CFileException* fe)
	{
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
		{
			CString szLogEntry;
			szLogEntry.Format("Writing single record of LMOS file \"%s\" failed. Cause: %d", fe->m_strFileName, fe->m_lOsError);
			PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
		}

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
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
		{
			const char DupIDStr[] = "Duplicate UID:";
			char buf[1024];

			strcpy(buf, DupIDStr);
			pMsgRecord->DebugDump(buf + sizeof(DupIDStr) - 1, sizeof(buf) - sizeof(DupIDStr));
			::PutDebugLog(DEBUG_MASK_LMOS, buf);
		}
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
	UINT ReadLen;

	ReadLen = pLMOSFile->Read(&szBuffer, LMOS_HEADER_SIZE);
	if (ReadLen != LMOS_HEADER_SIZE)
	{
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
		{
			CString szLogEntry;
			szLogEntry.Format("Error reading LMOS header of %s: bytes read=%u", (LPCTSTR)pLMOSFile->GetFilePath(), ReadLen);
			PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
		}

		return false;
	}
	
	int num_version = 0;
	memcpy(&num_version, szBuffer, 4);
	ASSERT(num_version == 1);

	char *		szAccountNameBuffer = m_szAccountName.GetBufferSetLength(LMOS_POPACCT_SIZE+1);
	ASSERT(szAccountNameBuffer);
	if (szAccountNameBuffer)
	{
		strncpy(szAccountNameBuffer, szBuffer + LMOS_POPACCT_OFFSET, LMOS_POPACCT_SIZE);
		szAccountNameBuffer[LMOS_POPACCT_SIZE] = 0;
		
		m_szAccountName.ReleaseBuffer();
	}

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
	try
	{
		if( !theLMOSFile.Open(m_strLMOSFileName, CFile::modeRead, &fexcept))
		{
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
			{
				CString szLogEntry;
				szLogEntry.Format("Error opening LMOS file %s: OsError=%ld, cause=%d", (LPCTSTR)m_strLMOSFileName, fexcept.m_lOsError, fexcept.m_cause);
				PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
			}

			//lmos.dat might not have been created by now..this is ok for new personas that just got
			//created
			//these error are expected ENOENT or directory non-existant ESRCH
			if(!(fexcept.m_lOsError == ENOENT || fexcept.m_lOsError == ESRCH))
			{
				ASSERT(0);  //if none of above errors the what is it??
				return false;
			}
			return true;
		}
		
		if(!LoadLMOSHeader(&theLMOSFile))
			return false;		//could not getter header 

		//The file pointer is ready for getting data right after getting the header
		char szBuffer[LMOS_RECORD_SIZE] = {0};
	
		while (theLMOSFile.Read(szBuffer, LMOS_RECORD_SIZE) == LMOS_RECORD_SIZE)
		{
			
			//
			// If we get this far, we should have a valid message record.
			// Pass the line of data to CreateMsgRecord() to create a corresponding
			// message record object.
			//
			
			CMsgRecord* pMsgRecord = CMsgRecord::CreateMsgRecordFromFile(m_strPersona, szBuffer, LMOS_RECORD_SIZE, &ulMessageIndex);
			if (pMsgRecord != NULL)
			{
				//
				// Successfully created message record object, so store it
				// in the list.
				//
				InsertMsgRecord(pMsgRecord);
			}
			else
			{
				if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
				{
					CString szLogEntry;
					szLogEntry.Format("Error reading LMOS file %s: record#=%lu", (LPCTSTR)m_strLMOSFileName, ulMessageIndex);
					PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
				}
				bBadData = true;		// bad data, or out of memory
			} 
			memset(szBuffer, 0, sizeof(szBuffer));
		}
		
	}
	catch(CFileException* fe)
	{	
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
		{
			CString szLogEntry;
			szLogEntry.Format("Error loading LMOS file %s: OsError=%ld, cause=%d", (LPCTSTR)m_strLMOSFileName, fe->m_lOsError, fe->m_cause);
			PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
		}
		fe->Delete();
		return false;
	}

	if(bBadData) 
		return false;

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
		CMsgRecord* pMsgRecord = DEBUG_NEW CMsgRecord();
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

	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
	{
		CString szLogEntry;
		szLogEntry.Format("Begin reconciling LMOS lists for %s", (LPCTSTR)m_szAccountName);
		PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
	}
	
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
			// Log the LMOS data.
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
			{
				char		 szBuf[256];
				char		 szHash[16];
				pDiskMsg->GetHashString(szHash);
				sprintf(szBuf, "Old %d hash %s gmt %X flags 0x%04X 0x%04X %c%c %c%c %c%c %c%c %c%c",
						pServerMsg->GetMsgNum(), szHash, pServerMsg->GetDateLong(),
						pServerMsg->GetFlags(), pDiskMsg->GetFlags(),

						(pServerMsg->GetRetrieveFlag() & LMOS_RETRIEVE_MESSAGE) ? 'F' : 'f',
						(pDiskMsg->GetRetrieveFlag() & LMOS_RETRIEVE_MESSAGE) ? 'F' : 'f',

						(pServerMsg->GetReadFlag() & LMOS_HASBEEN_READ) ? 'R' : 
							((pServerMsg->GetReadFlag() & LMOS_NOT_READ) ? 'r' : 'P'),
						(pDiskMsg->GetReadFlag() & LMOS_HASBEEN_READ) ? 'R' : 
							((pDiskMsg->GetReadFlag() & LMOS_NOT_READ) ? 'r' : 'P'),
						
						(pServerMsg->GetSkippedFlag() & LMOS_SKIP_MESSAGE) ? 'K' :
							((pServerMsg->GetSkippedFlag() & LMOS_DONOT_SKIP) ? 'k' : 'w'),
						(pDiskMsg->GetSkippedFlag() & LMOS_SKIP_MESSAGE) ? 'K' :
							((pDiskMsg->GetSkippedFlag() & LMOS_DONOT_SKIP) ? 'k' : 'w'),

						(pServerMsg->GetSaveFlag() & LMOS_SAVE_MESSAGE) ? 'S' : 's',
						(pDiskMsg->GetSaveFlag() & LMOS_SAVE_MESSAGE) ? 'S' : 's',

						(pServerMsg->GetDeleteFlag() & LMOS_DELETE_MESSAGE) ? 'D' :
							((pServerMsg->GetDeleteFlag() & LMOS_DONOT_DELETE) ? 'd' : 't'),
						(pDiskMsg->GetDeleteFlag() & LMOS_DELETE_MESSAGE) ? 'D' :
							((pDiskMsg->GetDeleteFlag() & LMOS_DONOT_DELETE) ? 'd' : 't'));
				PutDebugLog(DEBUG_MASK_LMOS, szBuf);
			}

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

			// Log the LMOS data.
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
			{
				char		 szBuf[256];
				char		 szHash[16];
				pServerMsg->GetHashString(szHash);
				sprintf(szBuf, "New %d hash %s gmt %X flags 0x%04X %c %c %c %c %c",
						pServerMsg->GetMsgNum(), szHash, pServerMsg->GetDateLong(), pServerMsg->GetFlags(),
						(pServerMsg->GetRetrieveFlag() & LMOS_RETRIEVE_MESSAGE) ? 'F' : 'f',
						(pServerMsg->GetReadFlag() & LMOS_HASBEEN_READ) ? 'R' : 
							((pServerMsg->GetReadFlag() & LMOS_NOT_READ) ? 'r' : 'P'),
						(pServerMsg->GetSkippedFlag() & LMOS_SKIP_MESSAGE) ? 'K' :
							((pServerMsg->GetSkippedFlag() & LMOS_DONOT_SKIP) ? 'k' : 'w'),
						(pServerMsg->GetSaveFlag() & LMOS_SAVE_MESSAGE) ? 'S' : 's',
						(pServerMsg->GetDeleteFlag() & LMOS_DELETE_MESSAGE) ? 'D' :
							((pServerMsg->GetDeleteFlag() & LMOS_DONOT_DELETE) ? 'd' : 't'));
				PutDebugLog(DEBUG_MASK_LMOS, szBuf);
			}

			//erase from server so that the pointer(MsgRecord) won't be deleted later.
			pServerHost->m_MsgRecords.erase(si++); //erase & point to the next record
		}
		else{
			++si;  //point to the next record
		}
	}

	m_bCanUIDL    = pServerHost->m_bCanUIDL;
	m_sServerType = pServerHost->m_sServerType;

	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
	{
		CString szLogEntry;
		szLogEntry.Format("Done reconciling LMOS lists for %s", (LPCTSTR)m_szAccountName);
		PutDebugLog(DEBUG_MASK_LMOS, szLogEntry);
	}

	delete pServerHost;
	return TRUE;
}
