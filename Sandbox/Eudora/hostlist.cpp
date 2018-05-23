#include "stdafx.h"
#include "resource.h"


#include "fileutil.h"   //for JJFile
#include "hostlist.h"
#include "pophost.h"
#include "rs.h"
#include "Persona.h"   //for g_Personalities

#include "pop.h"  //for gPopHostList
#include "Summary.h"  //CLMOSRecord
#include "SpoolMgr.h"

#include "utils.h"  //for QCLoadString


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Sum is ignored for now because we don't write Persona stuff to the
// TOC, hence when Eudora is first started the Summary object have no
// Persona info in them.  Eventually this my be remedied, in which case
// the Sum can be used to narrow the search
CMsgRecord* GetMsgByHash( CSummary * Sum )
{
	// Lets get the host...
	CPopHost* theHost = (CPopHost *)CHostList::GetHost(Sum->GetPersona());
	
	if(theHost)
		return( theHost->GetMsgByHash(Sum->GetHash()) );
	else
	{
		ASSERT(0);  //Host must be there
		return NULL;
	}
}



//Public Static

//If retuned value is a valid msg record, then that pophost is locked.  
//Its the user responsibility to unlock it!!  
CPopHost *CHostList::GetHostWithMsgHash(long Hash, CMsgRecord *pMsgRecord)
{
	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);

	pMsgRecord = NULL;
	
	CPopHost* pCurrentPopHost = NULL;
	if (gPopHostList == NULL)
		return NULL;

	if ((gPopHostList != NULL) && (! gPopHostList->IsEmpty()))
	{
		// We have a valid host list, so search for the host object
		// for the current personality.
		
		POSITION pos = gPopHostList->GetHeadPosition();
		while (pos)
		{
			pCurrentPopHost = (CPopHost *) gPopHostList->GetNext(pos);
			
			//pCurrentPopHost->Lock();
			pMsgRecord = pCurrentPopHost->GetMsgByHash(Hash);
			if(!pMsgRecord)
			{
				//pCurrentPopHost->Unlock();
			}
			else
			{
				ASSERT(pMsgRecord);
				ASSERT(pCurrentPopHost);
				return pCurrentPopHost;
			}
		}
	}

	return NULL;
}


CLMOSRecord::CLMOSRecord(CSummary *Sum) : m_pMsgRecord(NULL), m_pPopHost(NULL)
{
	m_pPopHost = CHostList::GetHostWithMsgHash(Sum->GetHash(), m_pMsgRecord);

	if(m_pPopHost)
	{
		m_pPopHost->Lock();
		m_pMsgRecord = m_pPopHost->GetMsgByHash(Sum->GetHash());
		if(!m_pMsgRecord)
			m_pPopHost->Unlock();
	}

	/*m_pPopHost = CHostList::GetHost(Sum->GetPersona());
	if(m_pPopHost)
	{
		m_pPopHost->Lock();
		m_pMsgRecord = m_pPopHost->GetMsgByHash(Sum->GetHash());
		if(!m_pMsgRecord)
			m_pPopHost->Unlock();
	}*/
}




CLMOSRecord::CLMOSRecord(CPopHost *pHost, unsigned long ulHash) : m_pPopHost(pHost)
{
	if(m_pPopHost)
	{
		m_pPopHost->Lock();
		m_pMsgRecord = m_pPopHost->GetMsgByHash(ulHash);
		if(!m_pMsgRecord)
			m_pPopHost->Unlock();
	}

}

	

CLMOSRecord::~CLMOSRecord()
{
	if(m_pMsgRecord)
		m_pPopHost->Unlock(); 
}

void CLMOSRecord::WriteLMOS()
{
	if(m_pPopHost)
		m_pPopHost->WriteLMOSData();
}



//INI settings
/*
IDS_INI_POP_ACCOUNT
IDS_LMOS_TMP_FILENAME
IDS_LMOS_FILENAME
*/



CCriticalSection CHostList::m_HostListGuard;



//NOT THREAD_SAFE
////////////////////////////////////////////////////////////////////////
// GetHostForCurrentPersona [public, static]
//
// Grab the LMOS database info associated with the current persona,
// reading the LMOS database from disk into into 'gPopHostList' as
// needed.  If we can't find one in the current LMOS database, then
// create a new host object initialized for the current personality.
////////////////////////////////////////////////////////////////////////
CPopHost* CHostList::GetHostForCurrentPersona()
{
	ASSERT(::IsMainThreadMT());

	CPopHost* pCurrentPopHost = GetHost(g_Personalities.GetCurrent());
	return pCurrentPopHost;
}


////////////////////////////////////////////////////////////////////////
// GetHost [public, static]
//
// Grab the LMOS database info associated with the current persona,
// reading the LMOS database from disk into into 'gPopHostList' as
// needed.  If we can't find one in the current LMOS database, then
// create a new host object initialized for the current personality.
////////////////////////////////////////////////////////////////////////
CPopHost* CHostList::GetHost(CString strPersona, CString strPOPAccount)
{
	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);

	CPopHost* pCurrentPopHost = NULL;
	if (gPopHostList == NULL)
		return NULL;

	if ((gPopHostList != NULL) && (! gPopHostList->IsEmpty()))
	{
		//
		// We have a valid host list, so search for the host object
		// for the current personality.
		//
		POSITION pos = gPopHostList->GetHeadPosition();
		while (pos)
		{
			pCurrentPopHost = (CPopHost *) gPopHostList->GetNext(pos);
			if (pCurrentPopHost->IsMe(strPersona) )
			{
				pos = NULL;		// found a match!
				break;
			}
			else
				pCurrentPopHost = NULL;
		}
	}

	
	if (gPopHostList && (NULL == pCurrentPopHost))
	{
		//
		// Uh, oh.  NO HOST!!  So, create one for the current personality.
		//

		if(strPOPAccount == "")
		{
			//get the POP account for this personality
			CString section = "Persona-" + strPersona;
			char Entry[256] = {0};
			if(!QCLoadString( IDS_INI_POP_ACCOUNT, Entry, sizeof(Entry)) )
			{
				ASSERT(0);   //got to find this
				return NULL;
			}

			char Account[256] = {0};
			GetPrivateProfileString(section, Entry, "junk", Account, sizeof(Account), INIPath);
			strPOPAccount = Account;
		}		

		pCurrentPopHost = new CPopHost(strPersona, strPOPAccount);
		if (pCurrentPopHost)
			gPopHostList->AddTail( pCurrentPopHost);
	} 

	return pCurrentPopHost;
}





CHostList::~CHostList()
{
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		CPopHost *pHost = (CPopHost *) GetNext(pos); 
		delete pHost;
	}

	RemoveAll();

}

////////////////////////////////////////////////////////////////////////
// MakeNewHostList [public, static]
//
// Create a new host list, containing the data from the LMOS.DAT file.
////////////////////////////////////////////////////////////////////////
CHostList* CHostList::CreateNewHostList()   
{
	
	ASSERT( ::IsMainThreadMT());  //must be in main thread as we use g_Pesonalities

	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);

	
	bool bReadOldStyleLMOS = true;
	CString strLMOSNewFormatFile; 

	// add alternate personalities
	LPSTR pszPersonaList = g_Personalities.List();
	LPSTR pszList = pszPersonaList; 
	CString strLMOS = CRString(IDS_LMOS_FILENAME);
	
	for ( ; pszList && *pszList; pszList += strlen(pszList) + 1)
	{
		//IMAP personas won't be using LMOS
		if(g_Personalities.IsImapPersona(pszList))
			continue;

		strLMOSNewFormatFile = QCSpoolMgrMT::GetLMOSFileName(pszList);

		if(::FileExistsMT(strLMOSNewFormatFile))
		{
			bReadOldStyleLMOS = false;
			break;
		}
		
	}

	if(bReadOldStyleLMOS)
	{
		gPopHostList = MakeNewHostList();
		//write it back in new format
		//WriteLMOSFile();
		return gPopHostList;
	}

	//
	// Make a new host list to be returned to the caller.
	//
	gPopHostList = new CHostList();
	if (NULL == gPopHostList)
		return NULL;
    
	

	//Create a pophost for each persona and load their LMOS data
	CString CachePersona = g_Personalities.GetCurrent();
	pszList = pszPersonaList; 
	CPopHost *pCurrentHost = NULL;

	for ( ; pszList && *pszList; pszList += strlen(pszList) + 1)
	{
		//IMAP personas won't be using LMOS
		if(g_Personalities.IsImapPersona(pszList))
			continue;
	
		g_Personalities.SetCurrent(pszList);

		pCurrentHost = new CPopHost();
		
		//ignore return value because the lmos mightnot existnat yet
		pCurrentHost->LoadLMOSData();
		//if(!pCurrentHost->LoadLMOSData())
			//ASSERT(0);  //waht we do with currupt lmos ?????? add an empty one anyway?
		
		//
		// Purge out the deleted records, renumber remaining
		// records, then add the host to the list.
		//

		//pCurrentHost->RemoveDeletedMsgRecords();
		gPopHostList->AddTail(pCurrentHost);
	}

	g_Personalities.SetCurrent(CachePersona);



	//
	// Make sure we don't return with an empty host list, even if
	// that means creating a single, empty host.
	//
	if (gPopHostList->IsEmpty())
	{
		//
		// Create a new empty host for the current info settings.
		// NOTE: this host will default to the current settings info.
		//
		CPopHost* pNewHost = new CPopHost();
		gPopHostList->AddHead(pNewHost);
	}
        
	return gPopHostList;
}









////////////////////////////////////////////////////////////////////////
// MakeNewHostList [public, static]
//
// Create a new host list, containing the data from the LMOS.DAT file.
////////////////////////////////////////////////////////////////////////
CHostList* CHostList::MakeNewHostList()
{
	//
	// Make a new host list to be returned to the caller.
	//
	CHostList* pHostList = new CHostList();
	if (NULL == pHostList)
		return NULL;
    
	
	//
	// Form full pathname to LMOS file, then read all existing
	// LMOS data, if any, into memory.
	//
	{
		CString strPathname(::EudoraDir);
		strPathname += CRString(IDS_LMOS_FILENAME);

		JJFile theLMOSFile;
		if (SUCCEEDED(theLMOSFile.Open(strPathname, _O_RDWR | O_CREAT)))
		{
			//
			// Read each of the host entries, one at a time.
			//
			CPopHost* pCurrentHost = NULL;
			bool bPersonaExists = false;
			while ((pCurrentHost = MakeNewHost_(&theLMOSFile, &bPersonaExists)) != NULL)
			{
				//
				// Instruct each host to build up a list of message
				// records by reading lines from the LMOS file.
				//
				if (! pCurrentHost->ReadLMOSData(&theLMOSFile))
				{
					pCurrentHost = NULL;		// something bad happened
					break;
				}

				//
				// Purge out the deleted records, renumber remaining
				// records, then add the host to the list.
				//
				//what if we crashed.. we need these DeleteSent records
				//pCurrentHost->RemoveDeletedMsgRecords();
				
				//Insert in global list only if the persona exists
				if(bPersonaExists)
					pHostList->AddTail(pCurrentHost);
			}
		}
	}

	//
	// Make sure we don't return with an empty host list, even if
	// that means creating a single, empty host.
	//
	if (pHostList->IsEmpty())
	{
		//
		// Create a new empty host for the current info settings.
		// NOTE: this host will default to the current settings info.
		//
		CPopHost* pNewHost = new CPopHost();
		pHostList->AddHead(pNewHost);
	}
        
	return pHostList;
}


////////////////////////////////////////////////////////////////////////
// MakeNewHost_ [private, static]
//
// Given an open LMOS file, read up the three lines starting with
// the line containing the #POPSTART tag into a buffer.  If successful,
// create a new CPopHost, passing it the three lines of data in the
// buffer.
////////////////////////////////////////////////////////////////////////
CPopHost* CHostList::MakeNewHost_(JJFile* pLMOSFile, bool *pbExists)
{
	CRString strStartTag(IDS_LMOS_HOST_START);		// #POPSTART keyword
	char szBuffer[kLMOSBuffSize];
	long lNumBytesRead = 0;
	long lError = 0;

	//
	// Read lines from the LMOS file until we hit a line with a
	// #POPSTART tag.
	//
	memset(szBuffer, 0, sizeof(szBuffer));
	for (;;)
	{
		if (SUCCEEDED(pLMOSFile->GetLine(szBuffer, sizeof(szBuffer), &lNumBytesRead)) && (lNumBytesRead > 0))
		{
			if (strstr(szBuffer, strStartTag) != NULL)
			{
				//
				// Found the #POPSTART tag!
				//
				strcat(szBuffer, "\r\n");	// add back the stripped off line terminator
				lError = 0;
				break;
			}
		}
		else
		{
			// We need to blow this popsicle stand...
			lError = -1;
			break;
		}
	}

	if (0 == lError)
	{
		//
		// If we get here, the buffer contains the line with the
		// #POPSTART tag and the account info.  Now, append the next 
		// two lines to the buffer.
		//
		long lBufferSize = sizeof(szBuffer);
		char* pszBuffer = szBuffer;

		pszBuffer += lNumBytesRead;
		lBufferSize -= lNumBytesRead;

		//
		// Read second line.
		//
		if (SUCCEEDED(pLMOSFile->GetLine(pszBuffer, lBufferSize, &lNumBytesRead)) && (lNumBytesRead > 0))
		{
			strcat(szBuffer,"\r\n");		// add back stripped off line terminator
			pszBuffer += lNumBytesRead;
			lBufferSize -= lNumBytesRead;

			//
			// Read third line.
			//
			if (SUCCEEDED(pLMOSFile->GetLine(pszBuffer, lBufferSize, &lNumBytesRead)) && (lNumBytesRead > 0))
				strcat(szBuffer, "\r\n");	// add back stripped off line terminator
			else
			{
				lError = -1;
				ASSERT(0);
			}
		}
		else
		{
			lError = -1;
			ASSERT(0);
		}
	}

	if (0 == lError)
	{
		// 
		// If we get here, the buffer contains the first three lines
		// of our host info (containing the account name, last purge
		// datestamp, and server UIDL type).  Create a new host object
		// and set the account name and last purge date -- the server
		// UIDL type is ignored.
		//
		CPopHost* pPopHost = new CPopHost();
		if (pPopHost)
			*pbExists = pPopHost->SetAccountNameAndLastPurge(szBuffer);

		return pPopHost;
	}

	return NULL;
}



////////////////////////////////////////////////////////////////////////
// Compare [protected]
//
// FORNOW, very bad style.  This is a non-virtual override of the
// virtual CSortedList::Compare(CMsgRecord*, CMsgRecord*) method from
// the base class.  It is non-virtual since it doesn't match the
// prototype of the base class method.  Is this really what we
// intended?  WKS 97.07.02.
////////////////////////////////////////////////////////////////////////
short CHostList::Compare(CPopHost* pPopHost1, CPopHost* pPopHost2)
{
	unsigned long ulLastPurge1 = pPopHost1->GetLastPurge();
	unsigned long ulLastPurge2 = pPopHost2->GetLastPurge();
   
	if (ulLastPurge1 < ulLastPurge2)
		return -1;
	else if (ulLastPurge1 > ulLastPurge2)
		return 1;
	else
		return 0;
}




/*
////////////////////////////////////////////////////////////////////////
// WriteLMOSFile [public, static]
//
////////////////////////////////////////////////////////////////////////
void CHostList::WriteLMOSFile()
{
	//
	// Setup temp LMOS filename
	//
	char szLMOSFilename[_MAX_PATH + 1];
	::GetIniString(IDS_LMOS_TMP_FILENAME, szLMOSFilename, sizeof(szLMOSFilename));

	char szLMOSPathname[_MAX_PATH + 1];
	strcpy(szLMOSPathname, EudoraDir);
	strcat(szLMOSPathname, szLMOSFilename);
	
	//
	// Open the temp file and write out the data, one section for each
	// POP server.
	//
	JJFile theFile;
	if (SUCCEEDED(theFile.Open(szLMOSPathname, _O_RDWR | O_CREAT | O_TRUNC)))
	{
		CRString strVersion(IDS_VERSION);
		long lError = 0;
		if (SUCCEEDED(theFile.PutLine(strVersion, strVersion.GetLength())))
		{
			POSITION pos = gPopHostList->GetHeadPosition();
			while (pos)
			{
				CPopHost* pPopHost = (CPopHost *) gPopHostList->GetNext(pos);
				if (pPopHost)
				{
					if (! pPopHost->WriteLMOSData(&theFile))
					{
						lError = -1;
						pos = NULL;
					}
				}
			}

			//
			// Finish off with a blank line.
			//
			if (lError >= 0)
			{
				if (FAILED(theFile.PutLine()))
					lError = -1;
			}

			if (lError >= 0)
			{
				//
				// Rename the temp file to the real file name.
				//
				GetIniString(IDS_LMOS_FILENAME, szLMOSFilename, sizeof(szLMOSFilename));
				strcpy(szLMOSPathname, EudoraDir);
				strcat(szLMOSPathname, szLMOSFilename);

				lError = theFile.Rename(szLMOSPathname); 
			}
		}
		else
			lError = -1;

		ASSERT(lError >= 0);
	}
}

*/


////////////////////////////////////////////////////////////////////////
// WriteLMOSFile [public, static]
//
////////////////////////////////////////////////////////////////////////
void CHostList::WriteLMOSFile()
{
	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);

	POSITION pos = gPopHostList->GetHeadPosition();
	while (pos)
	{
		CPopHost* pPopHost = (CPopHost *) gPopHostList->GetNext(pos);
		if (pPopHost)
		{
			if (! pPopHost->WriteLMOSData())
			{
				ASSERT(0);
			}
		}
	}
	
	
}

////////////////////////////////////////////////////////////////////////
// ReplaceHost [public]
//
////////////////////////////////////////////////////////////////////////
void CHostList::ReplaceHost(CPopHost* pOldPopHost, CPopHost* pNewPopHost)
{
	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);

	POSITION pos = Find(pOldPopHost);
	if (pos)
	{
		RemoveAt(pos);
		delete pOldPopHost;
		pOldPopHost = NULL;
	}

	InsertPopHost(pNewPopHost);
}


////////////////////////////////////////////////////////////////////////
// InsertPopHost [public]
//
// Insert the given pop host into the list in sorted order by the last
// purge timestamp.
////////////////////////////////////////////////////////////////////////
void CHostList::InsertPopHost(CPopHost* pPopHost)
{
	POSITION pos = NULL;
	if (! IsEmpty())
		pos = GetHeadPosition();
  
	if (! pos)
	{
		//
		// Easy case.  List is empty, so just add the first entry.
		//
		AddHead( pPopHost);
	}
	else
	{
		//
		// Walk the list comparing entries until we find where the new
		// entry has an equal or greater purge date.
		//
		BOOL bInserted = FALSE;
		while (pos)
		{
			//
			// Basically we want to look for the point where the new
			// object is greater than the object we are currently
			// checking...assuming all is in order..the new object
			// should be inserted after that object and before the
			// next...
			//
			POSITION posInsert = pos;
			if (Compare((CPopHost*) GetNext(pos), pPopHost) >= 0)
			{
				InsertBefore(posInsert,  pPopHost);
				bInserted = TRUE;
				break;
			}
		}

		if (! bInserted)
			AddTail( pPopHost);
	}
}  


