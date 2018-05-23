// HostList.cpp
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


#include "fileutil.h"   //for JJFile
#include "hostlist.h"
#include "pophost.h"
#include "rs.h"
#include "Persona.h"   //for g_Personalities

#include "SpoolMgr.h"

#include "utils.h"  //for QCLoadString


#include "DebugNewHelpers.h"


//	Globals	=================================================
CHostList* gPopHostList = NULL;


//Public Static

CPopHost *CHostList::GetHostWithMsgHash(long Hash)
{
	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);

	if (gPopHostList == NULL)
		return NULL;

	POSITION pos = gPopHostList->GetHeadPosition();
	while (pos)
	{
		CPopHost* pCurrentPopHost = (CPopHost *) gPopHostList->GetNext(pos);
		CMsgRecord* pMsgRecord = pCurrentPopHost->GetMsgByHash(Hash);
		if(pMsgRecord)
			return pCurrentPopHost;
	}

	return NULL;
}


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

	CPopHost* pCurrentPopHost = GetHost( GetIniLong(IDS_INI_LMOS_BACKUPS), g_Personalities.GetCurrent() );
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
CPopHost* CHostList::GetHost(long nLMOSBackupCount, const char* strPersona, const char* strPOPAccount /*= NULL*/)
{
	//Acquire lock to access gPopHostList
	CSingleLock lock(&m_HostListGuard, TRUE);
	char Account[256] = {0};

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

		if (!strPOPAccount || !*strPOPAccount)
		{
			//get the POP account for this personality
			CString section("Persona-");
			section += strPersona;

			char Entry[256] = {0};
			if(!QCLoadString( IDS_INI_POP_ACCOUNT, Entry, sizeof(Entry)) )
			{
				ASSERT(0);   //got to find this
				return NULL;
			}

			GetPrivateProfileString(section, Entry, "junk", Account, sizeof(Account), INIPath);
			strPOPAccount = Account;
		}		

		pCurrentPopHost = DEBUG_NEW_NOTHROW CPopHost(strPersona, strPOPAccount, nLMOSBackupCount);
		if (pCurrentPopHost && pCurrentPopHost->LoadLMOSData())
			gPopHostList->AddTail(pCurrentPopHost);
		else
		{
			delete pCurrentPopHost;
			pCurrentPopHost = NULL;
		}
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
	gPopHostList = DEBUG_NEW_MFCOBJ_NOTHROW CHostList();
	if (NULL == gPopHostList)
		return NULL;
    
	

	//Create a pophost for each persona and load their LMOS data
	CString CachePersona = g_Personalities.GetCurrent();
	pszList = pszPersonaList; 

	for ( ; pszList && *pszList; pszList += strlen(pszList) + 1)
	{
		//IMAP personas won't be using LMOS
		if(g_Personalities.IsImapPersona(pszList))
			continue;
	
		g_Personalities.SetCurrent(pszList);

		CPopHost* pCurrentHost = DEBUG_NEW_NOTHROW CPopHost();
		
		if (pCurrentHost && pCurrentHost->LoadLMOSData())
			gPopHostList->AddTail(pCurrentHost);
		else
			delete pCurrentHost;
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
		CPopHost* pNewHost = DEBUG_NEW CPopHost();
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
	CHostList* pHostList = DEBUG_NEW_MFCOBJ_NOTHROW CHostList();
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
		CPopHost* pNewHost = DEBUG_NEW CPopHost();
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
		CPopHost* pPopHost = DEBUG_NEW_NOTHROW CPopHost();
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


////////////////////////////////////////////////////////////////////////
// Cleanup [public, static]
//
// Delete the global gPopHostList host list.
////////////////////////////////////////////////////////////////////////
/*static*/ void CHostList::Cleanup()
{
	delete gPopHostList;
	gPopHostList = NULL;
}
