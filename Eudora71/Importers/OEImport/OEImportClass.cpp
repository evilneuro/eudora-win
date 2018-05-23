#include "stdafx.h"
#include <string>

#include "DebugNewHelpers.h"


using std::string;


/////////////////////////////////////////////////////////////////////////////
// OEImportClass construction

OEImportClass::OEImportClass()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_fnpFolders = NULL;
	m_lOE5MsgTable = NULL;
	m_fpoDigestFile = NULL;
	m_fpiOEFile  = NULL;
	m_lpszOEFileName = NULL;
	m_tempfile = NULL;
	m_szMainBoundary[0] = 0;
	m_szMessageBoundary[0] = 0;
	m_TempFilePathname = NULL;
	m_lpszDigestPath = NULL;
	m_szTempDirName[0] = 0;
	m_pABook = NULL;
	m_pABookTree = NULL;
	m_bInit = false;
	m_hWABDll = NULL;
	m_lpAdrBook = NULL;
	m_lpfnWabOpen = NULL;
	m_lpPropArray = NULL;
	m_lpWABObject = NULL;

}
/////////////////////////////////////////////////////////////////////////////
// Destructor - Clean up Everything
// Called by Windows
// Calls DeleteChildNodeData and DeleteFolderNodeData to kill linked lists
OEImportClass::~OEImportClass()
{
	CImportChild *pChild, *pChildtmp;
	CMbox_FnamePair *pFolder, *pFoldertmp;	

	if (m_lOE5MsgTable)		delete [] m_lOE5MsgTable;

	if(m_OEImportProvider.lpszDisplayName)
		delete [] m_OEImportProvider.lpszDisplayName;

	if(m_OEImportProvider.pFirstChild)
	{
		pChild = m_OEImportProvider.pFirstChild->pNext;
		DeleteChildNodeData(m_OEImportProvider.pFirstChild);
		
		delete m_OEImportProvider.pFirstChild;
		while(pChild)
		{
			pChildtmp = pChild->pNext;
			DeleteChildNodeData(pChild);
			delete pChild;
			pChild = pChildtmp;
		}
	}

	// we know we have just one node here
	//why worry like OL to recurse the tree
	if(m_pMessageStores) {

		delete [] m_pMessageStores->lpszDisplayName;
		m_pMessageStores->pDB = NULL;
		m_pMessageStores->pNext = NULL;
		m_fnpFolders = m_pMessageStores->lpMBoxID;

		if(m_fnpFolders) {
			pFolder = m_fnpFolders;

			while(pFolder)
			{
				pFoldertmp = pFolder->pNext;
				DeleteFolderNodeData(pFolder);
				delete pFolder;
				pFolder = pFoldertmp;

			}
		}
	}


	if(m_bInit)
	{
		if(m_lpPropArray)
			m_lpWABObject->FreeBuffer(m_lpPropArray);

		if(m_lpWABObject)
			m_lpWABObject->Release();

		if(m_lpAdrBook)
			m_lpAdrBook->Release();
		
		if(m_hWABDll)
			FreeLibrary(m_hWABDll);
	}

	DWORD dwType;
	ULONG cbData = sizeof(dwType);
	DWORD dwRefCount = 0;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs",
					0, KEY_ALL_ACCESS, &m_hSharedDllKey) == ERROR_SUCCESS)
	{

		RegQueryValueEx(m_hSharedDllKey, m_szWABDllPath, 0, &dwType,  (LPBYTE) &dwRefCount, &cbData);

		if(dwRefCount == 1)
		{
			RegDeleteValue(m_hSharedDllKey, m_szWABDllPath);


		}
		else
		{
			dwRefCount--;

			RegSetValueEx(m_hSharedDllKey, m_szWABDllPath, 0, REG_DWORD, (const PBYTE) &dwRefCount, sizeof(DWORD));
		}

		RegCloseKey(m_hSharedDllKey);
	}
	free(m_TempFilePathname);
	free(m_lpszDigestPath);

	//temporary fix bcos of OL where we create a tree of address books
	//OE has just one address book. Hence the linked list has
	//just one address book at the root. we free the root node of the tree

	if(m_pABookTree) {
		if(m_pABookTree->lpszDisplayName)
			delete [] m_pABookTree->lpszDisplayName;

		m_pABookTree->pFirstChild = NULL;
	}

	DeleteAddrTree(&m_pABook);
}

/////////////////////////////////////////////////////////////////////////////
// Clean up Provider List
// Called by Destructor
void OEImportClass::DeleteChildNodeData(CImportChild *pChild)
{
	if(pChild->lpszAccountDisplayName)
		delete [] pChild->lpszAccountDisplayName;

	if(pChild->lpszEmailAddress)
		delete [] pChild->lpszEmailAddress;

	if(pChild->lpszIdentityName)
		delete [] pChild->lpszIdentityName;

	if(pChild->lpszIncomingServer)
		delete [] pChild->lpszIncomingServer;

	if(pChild->lpszIncomingUserName)
		delete [] pChild->lpszIncomingUserName;

	if(pChild->lpszPathToMailData)
		delete [] pChild->lpszPathToMailData;

	if(pChild->lpszPathToAdrData)
		delete [] pChild->lpszPathToAdrData;

	if(pChild->lpszPersonalityName)
		delete [] pChild->lpszPersonalityName;

	if(pChild->lpszRealName)
		delete [] pChild->lpszRealName;

	if(pChild->lpszSMTPServer)
		delete [] pChild->lpszSMTPServer;

	return;

}
/////////////////////////////////////////////////////////////////////////////
// Clean up Folder List
// Called by Destructor
void OEImportClass::DeleteFolderNodeData(CMbox_FnamePair *pFolder)
{
	if(pFolder->lpszDisplayName)
		delete [] pFolder->lpszDisplayName;
	
	if(pFolder->lpszFilename)
		delete [] pFolder->lpszFilename;

	if(pFolder->lpszMailbox)
		delete [] pFolder->lpszMailbox;

	return;
}
// **************************************************************************
// ******* End cleanup

/////////////////////////////////////////////////////////////////////////////
// Called from external DllMain to provide a copy of the handle
void OEImportClass::LoadModuleHandle(HANDLE hModule)
{
	m_hModule = hModule;
}

/////////////////////////////////////////////////////////////////////////////
// Called from external DllMain to get the providers filled in
// Calls FindMainOEKey to find primary OE keys int he registry
// if they are found it finds identity accounts if they exist
// Fills in the provider and Child linked list
void OEImportClass::LoadOEProvider()
{
	if(FindMainOEKey())
	{
		FindIdentityOEKeys();
	}
	return;
}


/////////////////////////////////////////////////////////////////////////////
// Find the main Key for OE importing, if not found
// Then no identity Keys, caller will quit
// If found, then add to link list
// Called by Exported LoadProviders
// Calls AddProvider to Add if necessary
// Calls AddChildOLExpress to add various accounts(OE4 style)
bool OEImportClass::FindMainOEKey()
{
	HKEY hKey;
	HKEY hAccountKey;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = _MAX_PATH;
	HKEY hChildKey;
	int ChildUID = 0;
	FILETIME fileTime;

	// If the key for Internet Account Manager is present we have
	// a default install of OE and default identity is used
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Internet Account Manager",
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		// Should we enumerate through the list accounts? Arghhhh
		AddProvider("Outlook Express");
		
		tempbuffer = DEBUG_NEW char[tempbufsize];

		if (RegOpenKeyEx(hKey, "Accounts", 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(  hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					AddChildOLExpress(hChildKey);
					RegCloseKey(hChildKey);
				}
			tempbufsize = _MAX_PATH;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		return true;
	}
	else return false;

}
/////////////////////////////////////////////////////////////////////////////
// Find the identity Keys for OE importing, if not found no identities
// If found, then add to link list
// Called by Exported LoadProviders
// Calls AddChildOLExpress to add various identity accounts(OE5 style)
bool OEImportClass::FindIdentityOEKeys()
{
	HKEY hKey;
	HKEY hAccountKey;
	DWORD dwIndex=0;
	DWORD dwsubIndex=0;
	char tempbuffer[_MAX_PATH]; // Full Buffer
	char tempkeybuf[_MAX_PATH]; // Key buffer for GUID name
	DWORD tempbufsize = _MAX_PATH;
	DWORD tempkeybufsize = _MAX_PATH;
	HKEY hChildKey;
	FILETIME fileTime;

	// Open identity key
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		"Identities",
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		 // Get GUID name and enumerate all of them
		while (RegEnumKeyEx( hKey, dwIndex++, tempkeybuf, &tempkeybufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
		{
			// Build path to mail settings
			strcpy(tempbuffer, "Identities\\");
			strcat(tempbuffer, tempkeybuf);
			strcat(tempbuffer, "\\Software\\Microsoft\\Internet Account Manager\\Accounts");

			// Open and enumerate accounts, this part is mostly copy pasted from LoadOLExpressAccounts
			if(RegOpenKeyEx(HKEY_CURRENT_USER, tempbuffer, 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
			{
				while (RegEnumKeyEx(  hAccountKey, dwsubIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
				{
					if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
					{
						AddChildOLExpress(hChildKey, tempkeybuf);
						RegCloseKey(hChildKey);
					}
					tempbufsize = _MAX_PATH;
				}

			}
			dwsubIndex = 0;
			memset(tempbuffer, 0, _MAX_PATH);
			if(hAccountKey)
				RegCloseKey(hAccountKey);
			tempkeybufsize = _MAX_PATH;
		}
		memset(tempkeybuf, 0, _MAX_PATH);
	}else return false;
	
	if(hKey)
		RegCloseKey(hKey);

	return true;

}

/////////////////////////////////////////////////////////////////////////////
// We Have our Provider to export from FindProvider to Main Eudora App
// Fill In values appropriately.
// Called From FindMainOEKey to initialize Provider for use
bool OEImportClass::AddProvider(char * lpszProviderName)
{
	if (!lpszProviderName)
		return false;
	
	m_OEImportProvider.lpszDisplayName = DEBUG_NEW char[strlen(lpszProviderName)+1];
	strcpy(m_OEImportProvider.lpszDisplayName, lpszProviderName);
	
	
	m_OEImportProvider.pFirstChild = NULL;
	m_OEImportProvider.iNumChildren = 0;
	m_OEImportProvider.bHasMail = 1;
	m_OEImportProvider.bHasAddresses = 1;

	return true;
}
/////////////////////////////////////////////////////////////////////////////
// Adds a Child Node to Our OE Provider
// Called By FindMainOEKey and FindIdentityOEKeys
// Calls CreateChildNode, FindMailPath, and OL98ExtractValue
int OEImportClass::AddChildOLExpress(HKEY hChildKey, char *lpszIdentityKey)
{
	CImportChild	*pCurrentChild;
	DWORD			dwKeyDataType;
	int				ChildID = 0;
	char			*szData;
	char			*trash;
	DWORD			trashsize = 128;
	DWORD			dwDataBufSize=128;
	HKEY			hkIdentityName;
	HKEY			hkIdentityUser;
	char			*szIdentityName = NULL;

	trash = DEBUG_NEW char[trashsize];
	// Let's just check that it's a valid EMAIL
	// Account as opposed to an LDAP account
	// if SMTP Display Name is there, then it is.
	if (RegQueryValueEx(hChildKey, "SMTP Display Name", NULL, &dwKeyDataType,
		(LPBYTE) trash, &trashsize) == ERROR_SUCCESS)
	{
		delete [] trash;

		pCurrentChild = CreateChildNode();

		if(!OLExtractValue(&(pCurrentChild->lpszAccountDisplayName), "Account Name", hChildKey))
		{
			return -1;
		}

		if(!OLExtractValue(&(pCurrentChild->lpszPersonalityName), "Account Name", hChildKey))
		{
			return -1;
		}

		if(!OLExtractValue(&(pCurrentChild->lpszRealName), "SMTP Display Name", hChildKey))
		{
			return -1;
		}

		if(!OLExtractValue(&(pCurrentChild->lpszEmailAddress), "SMTP Email Address", hChildKey))
		{
			return -1;
		}

		if(!OLExtractValue(&(pCurrentChild->lpszSMTPServer), "SMTP Server", hChildKey))
		{
			return -1;
		}

		if(OLExtractValue(&(pCurrentChild->lpszIncomingUserName), "IMAP User name", hChildKey))
		{
			pCurrentChild->bIsIMAP = true;
		}

		if(OLExtractValue(&(pCurrentChild->lpszIncomingServer), "IMAP Server", hChildKey))
		{
			pCurrentChild->bIsIMAP = true;
		}

		if(OLExtractValue(&(pCurrentChild->lpszIncomingUserName), "POP3 User name", hChildKey))
		{
			pCurrentChild->bIsIMAP = false;
		}

		if(OLExtractValue(&(pCurrentChild->lpszIncomingServer), "POP3 Server", hChildKey))
		{
			pCurrentChild->bIsIMAP = false;
		}

		//what are the things we are importing - for now mail and address books
		pCurrentChild->kImportType = MBOX_ABOOK;

		if(lpszIdentityKey)
		{
			if(strlen(lpszIdentityKey) != 0)
				strcpy(pCurrentChild->m_szIdentityUID, lpszIdentityKey);
			else
				memset(pCurrentChild->m_szIdentityUID, 0, _MAX_PATH);

			if (RegOpenKeyEx(HKEY_CURRENT_USER,
				"Identities",
				0, KEY_READ, &hkIdentityName) == ERROR_SUCCESS)
			{
					if(RegOpenKeyEx(hkIdentityName, lpszIdentityKey, 0, KEY_READ, &hkIdentityUser) == ERROR_SUCCESS)
					{
						OLExtractValue(&(pCurrentChild->lpszIdentityName), "Username", hkIdentityUser);
						RegCloseKey(hkIdentityUser);
						RegCloseKey(hkIdentityName);
					}

			}
		}
		else
		{
			if (RegOpenKeyEx(HKEY_CURRENT_USER,
				"Identities",
				0, KEY_READ, &hkIdentityName) == ERROR_SUCCESS)
			{
				if(OLExtractValue(&szIdentityName, "Default User ID", hkIdentityName))
				{
					if(RegOpenKeyEx(hkIdentityName, szIdentityName, 0, KEY_READ, &hkIdentityUser) == ERROR_SUCCESS)
					{
						OLExtractValue(&(pCurrentChild->lpszIdentityName), "Username", hkIdentityUser);
						RegCloseKey(hkIdentityUser);
						RegCloseKey(hkIdentityName);
					}
					strcpy(pCurrentChild->m_szIdentityUID, szIdentityName);
					delete [] szIdentityName;

				}

				
			}

		}
		FindMailPath(pCurrentChild);
		FindWABPath(&(pCurrentChild->lpszPathToAdrData));

		szData = DEBUG_NEW char[dwDataBufSize];

		if (RegQueryValueEx(hChildKey, "Leave Mail On Server", NULL, &dwKeyDataType,
			(LPBYTE) &(pCurrentChild->bLMOS), &dwDataBufSize) == ERROR_SUCCESS)
		{
			dwDataBufSize = 128;
		}
		else
		{
			pCurrentChild->bLMOS = false;
		}

		delete [] szData;
		return (pCurrentChild->iAccountID);
	}
	else
	{
		delete [] trash;
		return -1;
	}
}
/////////////////////////////////////////////////////////////////////////////
// Creates and initializes a Child Node gives it to caller
// Called from AddChildOLExpress
CImportChild* OEImportClass::CreateChildNode()
{

	CImportChild	*pCurrentChild;
	int iChildID = 0;

	m_OEImportProvider.iNumChildren++;
	
	pCurrentChild = m_OEImportProvider.pFirstChild;

	if (pCurrentChild == NULL)
	{
		m_OEImportProvider.pFirstChild = DEBUG_NEW CImportChild;
		pCurrentChild = m_OEImportProvider.pFirstChild;
	}
	else
	{
		iChildID++;			// Get a bonus increment just for being here, we then add one for each level after this.
							// By being here we're naturally at the second level.
		while (pCurrentChild->pNext != NULL)
		{
			iChildID++;
			pCurrentChild = pCurrentChild->pNext;
		}

		pCurrentChild->pNext = DEBUG_NEW CImportChild;
		pCurrentChild =	pCurrentChild->pNext;
	}
	pCurrentChild->pParent = &m_OEImportProvider;
	pCurrentChild->iAccountID = iChildID;

	pCurrentChild->bHasMail = false;
	pCurrentChild->bHasAddresses = false;

	// Populate with empty String so we don't crash when Scott tries to display it :)
	pCurrentChild->lpszAccountDisplayName = DEBUG_NEW char[1];
	pCurrentChild->lpszAccountDisplayName[0] = 0;

	pCurrentChild->lpszPathToMailData = DEBUG_NEW char[1];
	pCurrentChild->lpszPathToMailData[0] = 0;

	pCurrentChild->lpszPathToAdrData = DEBUG_NEW char[1];
	pCurrentChild->lpszPathToAdrData[0] = 0;

	pCurrentChild->lpszPersonalityName = DEBUG_NEW char[1];
	pCurrentChild->lpszPersonalityName[0] = 0;

	pCurrentChild->lpszIncomingUserName = DEBUG_NEW char[1];
	pCurrentChild->lpszIncomingUserName[0] = 0;

	pCurrentChild->lpszIncomingServer = DEBUG_NEW char[1];
	pCurrentChild->lpszIncomingServer[0] = 0;

	pCurrentChild->lpszRealName = DEBUG_NEW char[1];
	pCurrentChild->lpszRealName[0] = 0;

	pCurrentChild->lpszEmailAddress = DEBUG_NEW char[1];
	pCurrentChild->lpszEmailAddress[0] = 0;

	pCurrentChild->lpszSMTPServer = DEBUG_NEW char[1];
	pCurrentChild->lpszSMTPServer[0] = 0;

	pCurrentChild->lpszIdentityName = DEBUG_NEW char[1];
	pCurrentChild->lpszIdentityName[0] = 0;

	pCurrentChild->pNext = NULL;
	pCurrentChild->bLMOS = false;
	pCurrentChild->bIsIMAP = false;
	
	return pCurrentChild;
}
/////////////////////////////////////////////////////////////////////////////
// Extracts registry value from given key name under child key
// OLExtractValue(char ** Dest[OUT], char *KeyName[IN], HKEY hChildKey[IN])
// Called from AddChildOLExpress
bool OEImportClass::OLExtractValue(char ** ppDest, char *lpszKeyName, HKEY hChildKey)
{
	DWORD dwDataBufSize=128;
	DWORD dwKeyDataType;
	char * szData;

	szData = DEBUG_NEW_NOTHROW char[dwDataBufSize];
	if (!szData)
	{
		return false;
	}

	if (RegQueryValueEx(hChildKey, lpszKeyName, NULL, &dwKeyDataType,
		(LPBYTE) szData, &dwDataBufSize) == ERROR_SUCCESS)
	{
		// Some other value has been filled in previously and is now going to be overridden
		// Most likely an IMAP server was first and it's going to be overridden with a POP server
		if (*ppDest != NULL)	
		{					
			delete [] *ppDest;
			*ppDest = NULL;
		}

		*ppDest = DEBUG_NEW_NOTHROW char[dwDataBufSize+1];
		if (!*ppDest)
		{
			delete [] szData;
			return false;
		}

		strcpy(*ppDest, szData);
		delete [] szData;
		return true;
		
	}
	else
	{
		delete [] szData;
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Finds path to mail from the current provider
// Fill In values appropriately.
// Called From AddChildOLExpress to fill in data member
bool OEImportClass::FindMailPath(CImportChild *pChild)
{
	HKEY	hKey;
	char szData[512];
	DWORD dwKeyDataType;
	DWORD dwDataBufSize;
	bool bFoundOE5;

 	delete [] pChild->lpszPathToMailData;
	pChild->lpszPathToMailData = NULL;
	
	// we working up to, but first gotta toss the UID value in
	// HCU\Identities\{UID}\Software\Microsoft\Outlook Express\...

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Identities", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		dwDataBufSize = sizeof(szData)/sizeof(szData[0]);

		HKEY hKeyOE;
		// If we did not get Identity key before, in FindIdentityOEKeys
		// or in FindMainOEKey, this is our very last ditch shot.
		if(!(strlen(pChild->m_szIdentityUID) > 0))
		{
			if (RegQueryValueEx(hKey, "Last User ID", NULL, &dwKeyDataType,
			(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
			{
				//put it in the child, since this is the one we will be using
				strcpy(pChild->m_szIdentityUID, szData);
			}
		}

		strcpy(szData, pChild->m_szIdentityUID);
		strcat(szData, "\\Software\\Microsoft\\Outlook Express");
		
		if (RegOpenKeyEx(hKey, szData, 0, KEY_READ, &hKeyOE) == ERROR_SUCCESS)
		{
			TCHAR Version[64];
			DWORD dwVersionSize = sizeof(Version);
			double HighestVersion = 0;
			for (DWORD dwIndex = 0;
				RegEnumKeyEx(hKeyOE, dwIndex, Version, &dwVersionSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
				dwIndex++)
			{
				double ThisVersion = atof(Version);
				if (ThisVersion > HighestVersion)
				{
					HKEY hKeyOEVersion;
					if (RegOpenKeyEx(hKeyOE, Version, 0, KEY_READ, &hKeyOEVersion) == ERROR_SUCCESS)
					{
						dwDataBufSize = sizeof(szData)/sizeof(szData[0]);
						if (RegQueryValueEx(hKeyOEVersion, "Store Root", NULL, &dwKeyDataType,
							(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
						{
							if (ThisVersion >= 5.0)
							{
								bFoundOE5 = TRUE;
								// DRW 9/14/99 - We've gotten this far, remember what we just found.
								pChild->lpszPathToMailData = DEBUG_NEW_NOTHROW char[_MAX_PATH + 1];
								if (pChild->lpszPathToMailData)
								{
									// DRW 9/14/99 - Expand the %UserProfile% portion.
									if (ExpandEnvironmentStrings(szData, pChild->lpszPathToMailData, _MAX_PATH + 1) == 0)
									{
										delete [] pChild->lpszPathToMailData;
										pChild->lpszPathToMailData = NULL;
									}
									else
									{
										char	szOE5FolderFile[_MAX_PATH + 1];
										strcpy(szOE5FolderFile, pChild->lpszPathToMailData);
										if (szOE5FolderFile[strlen(szOE5FolderFile) - 1] != '\\')
										{
											strcat(szOE5FolderFile, "\\");
										}
										strcat(szOE5FolderFile, "folders.dbx");
										struct _stat		st;									// File info (for file size)
										if (_stat(szOE5FolderFile, &st) == -1)
										{
											bFoundOE5 = false;
										}
									}
								}
							}
							HighestVersion = ThisVersion;
						}
						RegCloseKey(hKeyOEVersion);
					}
				}
			}
			RegCloseKey(hKeyOE);
		}
	}	

		RegCloseKey(hKey);
		


	// DRW 9/13/99 - Added OE5 handling.
	if (!bFoundOE5)
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER,  "Software\\Microsoft\\Outlook Express",
			0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			dwDataBufSize = sizeof(szData)/sizeof(szData[0]);
			if (RegQueryValueEx(hKey, "Store Root", NULL, &dwKeyDataType,
				(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
			{
				TCHAR szBuf[2048];

				HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
				
				if(!LoadString((HINSTANCE)m_hModule, IDS_IMPORT_MAIL_OE4_AND_OE5_FOUND, szBuf, 2048))
					strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
				
				

				if (!bFoundOE5 || MessageBox(hEudoraMain,szBuf,"Eudora",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					pChild->lpszPathToMailData = DEBUG_NEW char[dwDataBufSize+strlen("\\Mail")+1];
					strcpy(pChild->lpszPathToMailData,(const char *)&szData);
					strcat(pChild->lpszPathToMailData, "\\Mail");
				}
				else
					return (false);
			}
			RegCloseKey(hKey);
		}

		if (pChild->lpszPathToMailData)
			return (true);
	}
	else
	{
		// DRW 9/14/99 - If we found OE5, we already know the mail path.
		if (pChild->lpszPathToMailData)
		{
			return true;
		}
	}


	return (false);

}
/////////////////////////////////////////////////////////////////////////////
// Finds path to address book from the current provider
// Fill In values appropriately.
// Called From AddChildOLExpress to fill in data member
bool OEImportClass::FindWABPath(char ** path) //Good
{
	HKEY	hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,  "Software\\Microsoft\\WAB\\WAB4\\Wab File Name",
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szData[512];
		DWORD dwKeyDataType;
		DWORD dwDataBufSize = sizeof(szData)/sizeof(szData[0]);

		if (RegQueryValueEx(hKey, NULL, NULL, &dwKeyDataType,
			(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
		{
			if (dwDataBufSize < 1)
			{
				RegCloseKey(hKey);
				return false;
			}

			*path = DEBUG_NEW char[dwDataBufSize+1];
			strcpy(*path,(const char *)&szData);
		}
		else 
		{
			RegCloseKey(hKey);
			return false;
		}
		RegCloseKey(hKey);
	}
	return (true);
}
// **************************************************************************
// *************    END  LOAD OE PROVIDER FUNCTIONS

/////////////////////////////////////////////////////////////////////////////
// Called from external DllMain to get the mail folders tree filled in
// Calls GetOEVersion to fill in our version member: m_iOEVersion
// Calls findEntryByToken to find folder entries
CMessageStore* OEImportClass::LoadFolders(char *lpszFolderPath)
{

	//m_fnpFolders
	char * lpszFolderFilePath;
	FILE * fFolders;
	char * lpszFloater;

	CMbox_FnamePair  **realnames = &m_fnpFolders;
	
	if (!lpszFolderPath)
		return NULL;

	//CODE REVIEW
	//shifted the code to after checking if the char* is null

	char *floater = strrchr(lpszFolderPath, '\\');
	if(floater)
		floater[1] = 0;

	m_iOEVersion = GetOEVersion(lpszFolderPath);

	// DRW 9/13/99 - Added OE5 handling.
	switch(m_iOEVersion)
	{
	case 0:

		lpszFolderFilePath = DEBUG_NEW char[strlen(lpszFolderPath)+strlen("\\folders.nch")+1];
		strcpy(lpszFolderFilePath, lpszFolderPath);
		if (lpszFolderFilePath[strlen(lpszFolderFilePath) - 1] != '\\')
		{
			strcat(lpszFolderFilePath, "\\");
		}
		strcat(lpszFolderFilePath, "folders.nch");

		break;
	
	case 1:

		lpszFolderFilePath = DEBUG_NEW char[strlen(lpszFolderPath)+strlen("\\folders.dbx")+1];
		strcpy(lpszFolderFilePath, lpszFolderPath);
		if (lpszFolderFilePath[strlen(lpszFolderFilePath) - 1] != '\\')
		{
			strcat(lpszFolderFilePath, "\\");
		}
		strcat(lpszFolderFilePath, "folders.dbx");
		break;
	case -1:
	default:
		return NULL;	
	}

	fFolders = fopen(lpszFolderFilePath, "rb");

	if(!fFolders)
		return NULL;
	
	while (findEntryByToken(fFolders))	// Found a token
	{
		if (*realnames)
		{
			while ((*realnames)->pNext)
				realnames = &((*realnames)->pNext);

			realnames = &((*realnames)->pNext);
		}

		char * Buffer = NULL;
		Buffer = DEBUG_NEW char[601];
		// DRW 9/13/99 - Added OE5 handling.
		if (m_iOEVersion == OE_4_VERSION)
		{
			// DRW 9/13/99 - If OE4 read 600 character buffer.
			fread(Buffer, sizeof(char), 600, fFolders);

		}
		else
		if(m_iOEVersion == OE_5_VERSION)
		{
			// DRW 9/13/99 - If OE5 read until next '\0'.
			char	c = '\0';
			// DRW 1/10/00 - Some mailbox entries have extra characters before
			// the actual mailbox name.  Skip characters until we find a
			// printable character.
			do
			{
				fread(&c, sizeof(char), 1, fFolders);
			} while ((c < ' ') || (c > '~'));
			// DRW 1/10/00 - Now read until the next '\0'.
			char *	szTempBufPtr = Buffer;
			while (c != '\0')
			{
				*szTempBufPtr++ = c;
				fread(&c, sizeof(char), 1, fFolders);
			}
			*szTempBufPtr = '\0';
			// DRW 2/5/00 - Anything with a mailbox ID of 6 or less is a standard OE5 mailbox.
			// I have seen cases where the mailbox token is immediately followed by a couple
			// bytes of information (I'm not sure what) instead of the folder name.  If the
			// mailbox name is less than 3 characters long, we obviously don't really have
			// the actual folder name.  Ignore what we read and read the actual folder name.
			if ((m_OE5Info.lOE5MBoxID <= 6) && (strlen(Buffer) < 3))
			{
				do
				{
					fread(&c, sizeof(char), 1, fFolders);
				} while ((c < ' ') || (c > '~'));
				szTempBufPtr = Buffer;
				while (c != '\0')
				{
					*szTempBufPtr++ = c;
					fread(&c, sizeof(char), 1, fFolders);
				}
				*szTempBufPtr = '\0';
			}
		}

		if (Buffer[0] != 0)
		{
			// DRW 9/13/99 - Added OE5 handling.
			if (m_iOEVersion == OE_4_VERSION)
			{
				// DRW 9/13/99 - If OE4 skip next 259 characters.
				lpszFloater = Buffer+259;	// Distance from existence of Realname of folder and it's Filename
			}
			else
			if (m_iOEVersion == OE_5_VERSION)
			{
				// DRW 9/13/99 - If OE5 read to next '\0'.
				lpszFloater = DEBUG_NEW_NOTHROW char[256];
				if (lpszFloater)
				{
					char	c = '\0';
					char *	szTempBufPtr = lpszFloater;
					fread(&c, sizeof(char), 1, fFolders);
					while (c != '\0')
					{
						*szTempBufPtr++ = c;
						fread(&c, sizeof(char), 1, fFolders);
					}
					*szTempBufPtr = '\0';
				}
			}

			bool	bValidMailbox = true;
			// If we are importing OE5 see if Buffer and floater appear to represent a valid mailbox.
			if (m_iOEVersion == OE_5_VERSION)
			{
				if ((Buffer == NULL) || (lpszFloater == NULL) || (strlen(lpszFloater) == 0))
				{
					bValidMailbox = false;
				}
				else if (stricmp(lpszFloater + strlen(lpszFloater) - 4, ".dbx") != 0)
				{
					// floater does not end in .dbx so there is no mailbox file to process.
					bValidMailbox = false;
				}
				else
				{
					// See if we already inserted an entry with the same info.
					CMbox_FnamePair *	mbpTemp = m_fnpFolders;

					if (mbpTemp)
					{
						do
						{
							if ((stricmp(mbpTemp->lpszFilename, lpszFloater) == 0))
							{
								bValidMailbox = false;
							}
							mbpTemp = mbpTemp->pNext;
						}
						while (bValidMailbox && mbpTemp);
					}
				}
			}

			if (bValidMailbox && m_iOEVersion == OE_5_VERSION)
			{
				// DRW - I have only seen this once, but there is a case under OE5 where
				// a folder lists itself as its parent.  In this case, our best chance of
				// salvage is to assign a nonsense ID so no one incorrectly tries to parent
				// itself to this mailbox.  But what if this mailbox really is someone's
				// parent?  You can't have everything.
				if ((m_iOEVersion == OE_5_VERSION) && (m_OE5Info.lOE5MBoxID == m_OE5Info.lOE5ParentMBoxID))
				{
					m_OE5Info.lOE5MBoxID = 1234;
				}
				char * lpszTemp = NULL;

				*realnames = DEBUG_NEW CMbox_FnamePair;

				(*realnames)->lpszFilename = DEBUG_NEW char[strlen(lpszFloater)+1];
				strcpy((*realnames)->lpszFilename, lpszFloater);
				
				lpszTemp = MakeOEDisplayName((*realnames)->lpszFilename);
				if(lpszTemp)
				{
					(*realnames)->lpszDisplayName = DEBUG_NEW char[strlen(lpszTemp)+1];
					strcpy((*realnames)->lpszDisplayName, lpszTemp);
				}
				else // how we used to do it, not pretty, but it shows something.
				{
					(*realnames)->lpszDisplayName = DEBUG_NEW char[strlen(Buffer)+1];
					strcpy((*realnames)->lpszDisplayName, Buffer);
				}
				(*realnames)->lMBoxID = m_OE5Info.lOE5MBoxID;
				(*realnames)->bHasSubfolders = m_OE5Info.bOE5HasSubfolders;
				(*realnames)->lParentMBoxID = m_OE5Info.lOE5ParentMBoxID;
				(*realnames)->lpszMailbox = NULL;
				(*realnames)->bCreated = false;
				(*realnames)->folderType = REGULAR_MBOX;
				(*realnames)->pNext = NULL;

				delete [] lpszTemp;
				lpszTemp  = NULL;
			}
			// DRW 9/13/99 - If OE5, free floater.
			if ((m_iOEVersion == OE_5_VERSION) && lpszFloater)
			{
				delete [] lpszFloater;
			}
		}
		delete [] Buffer;
	}
	if(m_iOEVersion == OE_4_VERSION)
	{
		char *lpszFinalFileName = DEBUG_NEW char [_MAX_PATH];
		char *lpszSearchToken = DEBUG_NEW char[_MAX_PATH];
		char *lpszTemp;
		// lpszFolderPath;
		lpszTemp = GetOE4SearchToken(lpszFolderPath);
		if(lpszTemp)
		{
			strcpy(lpszSearchToken, lpszTemp);
			delete [] lpszTemp;
			lpszTemp = NULL;
		}
		else
			return NULL;
		
		lpszTemp = GetFirstOE4Mbox(lpszSearchToken);
		if(lpszTemp)
		{
			strcpy(lpszFinalFileName, lpszTemp);
			delete [] lpszTemp;
			lpszTemp = NULL;
		}
		else
			return NULL;
		
		while(lpszFinalFileName != NULL)
		{
			char *lpszDisplayName = DEBUG_NEW char [_MAX_PATH];

			lpszTemp = MakeOEDisplayName(lpszFinalFileName);
			if(lpszTemp)
			{
				strcpy(lpszDisplayName, lpszTemp);
				delete [] lpszTemp;
				lpszTemp = NULL;
			}
			
			if (*realnames)
			{
				while ((*realnames)->pNext)
					realnames = &((*realnames)->pNext);
			
				realnames = &((*realnames)->pNext);
			}

			*realnames = DEBUG_NEW CMbox_FnamePair;
			(*realnames)->lpszDisplayName = DEBUG_NEW char[strlen(lpszDisplayName)+1];
			strcpy((*realnames)->lpszDisplayName, lpszDisplayName);

			if(lpszDisplayName)
				delete [] lpszDisplayName;

			(*realnames)->lpszFilename = DEBUG_NEW char[strlen(lpszFinalFileName)+1];
			strcpy((*realnames)->lpszFilename, lpszFinalFileName);

			(*realnames)->lMBoxID = 0;
			(*realnames)->bHasSubfolders = 0;
			(*realnames)->lParentMBoxID = 0;
			(*realnames)->lpszMailbox = NULL;
			(*realnames)->bCreated = false;
			(*realnames)->folderType = REGULAR_MBOX;
			(*realnames)->pNext = NULL;

			lpszTemp = GetNextOE4Mbox();
			
			if(lpszTemp)
			{
				strcpy(lpszFinalFileName, lpszTemp);
			}
			else
				lpszFinalFileName = NULL;
		}

		if(lpszSearchToken)
			delete [] lpszSearchToken;

		if(m_hFindHandle)
			FindClose(m_hFindHandle);

	}

	delete [] lpszFolderFilePath;

	m_pMessageStores = DEBUG_NEW CMessageStore;
	InitMsgStoreTree(m_pMessageStores);
	m_pMessageStores->lpMBoxID = m_fnpFolders;
	m_pMessageStores->pDB = NULL;
	m_pMessageStores->bIsDefault = false;
	m_pMessageStores->pNext = NULL;

	return m_pMessageStores;

}
/////////////////////////////////////////////////////////////////////////////
// Called from LoadFolders to get the version and fill version member
int OEImportClass::GetOEVersion(char *lpszFolderPath)
{
	char *				lpszFolderFilePath;	
	struct _stat		st;	


	lpszFolderFilePath = DEBUG_NEW char[strlen(lpszFolderPath)+strlen("\\folders.nch")+1];
	strcpy(lpszFolderFilePath, lpszFolderPath);
	if (lpszFolderFilePath[strlen(lpszFolderFilePath) - 1] != '\\')
	{
		strcat(lpszFolderFilePath, "\\");
	}
	strcat(lpszFolderFilePath, "folders.nch");

	if (_stat(lpszFolderFilePath, &st) == 0)
	{
		return OE_4_VERSION;
	}
	
	delete [] lpszFolderFilePath;
	lpszFolderFilePath = DEBUG_NEW char[strlen(lpszFolderPath)+strlen("\\folders.dbx")+1];
	strcpy(lpszFolderFilePath, lpszFolderPath);
	if (lpszFolderFilePath[strlen(lpszFolderFilePath) - 1] != '\\')
	{
		strcat(lpszFolderFilePath, "\\");
	}
	strcat(lpszFolderFilePath, "folders.dbx");
	
	if (_stat(lpszFolderFilePath, &st) == 0)
	{
		delete [] lpszFolderFilePath;
		return OE_5_VERSION;
	}
	delete [] lpszFolderFilePath;

	return -1; //No Folders file, can't tell version
}

bool OEImportClass::InitMsgStoreTree(CMessageStore *pEntry)
{
	pEntry->lpszDisplayName     = NULL;
	pEntry->lpMBoxID			= NULL;
	pEntry->pDB					= NULL;
	pEntry->bIsDefault			= false;
	pEntry->pNext				= NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Called from LoadFolders to Parse folder entries in the nch or dbx yy

bool OEImportClass::findEntryByToken(FILE *pchfile) // testtoken should default to 0x00000000
{
	char			buffer[2] = {0x00, 0x00};
	unsigned char	testtoken[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char	magictoken[4] = {0x00, 0x00, 0x70, 0x02};
	unsigned char	mboxidtoken0[1] = {0x80};				// DRW 9/13/99 - OE5 token for mailbox id.
	unsigned char	parentmboxidtoken[2] = {0x00, 0x81};		// DRW 9/13/99 - OE5 token for parent mailbox id.
	unsigned char	hasnosubfolderstoken[2] = {0x86, 0x21};		// DRW 9/22/99 - OE5 token for folder with no subfolders.
	int				i;

	// DRW 9/13/99 - OE5 has different magictoken.
	if (m_iOEVersion == OE_5_VERSION)
	{
		magictoken[0] = 0x8A;
		magictoken[1] = 0x03;
		magictoken[2] = 0x00;
		magictoken[3] = 0x00;
	}

	m_OE5Info.lOE5MBoxID = 0;
	m_OE5Info.lOE5ParentMBoxID = 0;
	m_OE5Info.bOE5HasSubfolders = true;

	
	while (fread(buffer, sizeof(char), 1, pchfile))
	{
		// Test for mailbox entry start token.
		if ((memcmp (magictoken, testtoken,4) == 0))
		{
			if (m_iOEVersion != OE_5_VERSION)
			{
				break;
			}
			else if ((m_OE5Info.lOE5MBoxID == 0) || (m_OE5Info.lOE5ParentMBoxID != 0))
			{
				break;
			}
		}
		if (m_iOEVersion == OE_5_VERSION)
		{
			// Test for other information, remembering it if we find it,
			// but don't break out.

			// Test for mailbox id token.
			if (memcmp(mboxidtoken0, testtoken, 1) == 0)
			{
				m_OE5Info.lOE5MBoxID = (long)testtoken[1];
				// DRW 1/10/00 - Sigh.  Sometimes the inbox (which is always mailbox 4)
				// specifies its parent as 0, not 1, which causes this mailbox to be
				// rejected later.  I hate to do this, but...
				if (m_OE5Info.lOE5MBoxID == 4)
				{
					m_OE5Info.lOE5ParentMBoxID = 1;
				}
				m_OE5Info.bOE5HasSubfolders = true;
			}
			// Test for parent id token.
			else if (memcmp(parentmboxidtoken, testtoken, 2) == 0)
			{
				m_OE5Info.lOE5ParentMBoxID = (long)testtoken[2];
				m_OE5Info.bOE5HasSubfolders = true;
			}
			// Test for has subfolder(s) token.
			else if (memcmp(hasnosubfolderstoken, testtoken, 2) == 0)
			{
				m_OE5Info.bOE5HasSubfolders = false;
			}
		}
		testtoken[0] = testtoken[1];
		testtoken[1] = testtoken[2];
		testtoken[2] = testtoken[3];
		testtoken[3] = buffer[0];
	}
	
	if (memcmp (magictoken, testtoken,4) != 0)
		return false;

	// DRW 9/13/99 - We don't need to do this for OE5.
	if (m_iOEVersion != OE_5_VERSION)
	{
		for (i = 1; i < 15; i++)
		{
			if ( (fread(buffer, sizeof(char), 1, pchfile)) == 0)
				return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Called from external LoadFolders to get the first matching mbox
// Will fill in Folders List and save search handle in m_hFindHandle
char * OEImportClass::GetFirstOE4Mbox(char *lpszStorePathName)
{
	WIN32_FIND_DATA wfd;
	char *lpszFirstBox = DEBUG_NEW char[_MAX_PATH];

	m_hFindHandle = FindFirstFile(lpszStorePathName, &wfd);
	
	if(m_hFindHandle != INVALID_HANDLE_VALUE)
	{
		strcpy(lpszFirstBox, wfd.cFileName);
		return lpszFirstBox;
	}
	else
		return NULL;
	
}
/////////////////////////////////////////////////////////////////////////////
// Called from external LoadFolders to get the next and subsequent 
// matching mboxes
char * OEImportClass::GetNextOE4Mbox()
{
	static bIteratingFiles = true;
	WIN32_FIND_DATA wfd;
	char *lpszSearchKey = DEBUG_NEW char [_MAX_PATH];

	bIteratingFiles = FindNextFile(m_hFindHandle, &wfd);
		
	if(bIteratingFiles)
	{
		lpszSearchKey = wfd.cFileName;
		return lpszSearchKey;
	}
	else
		return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// Called from external LoadFolders to fill in display name part of list
// kinda a hack since we do get nearly the display name in OE4, but this is
// the condition it was in and OE4 is much less widespread than OE5 by now
//  We make display name from 
// folder file name (Folder1.mbx = Folder1)
// OE5 file name is folder name
char* OEImportClass::MakeOEDisplayName(char *lpszFileName)
{
	char *lpszDisplayName = DEBUG_NEW char[_MAX_PATH];
	char *lpszTemp;

	strcpy(lpszDisplayName, lpszFileName);
	lpszTemp = strrchr(lpszDisplayName, '.');
	if(lpszTemp)
		*lpszTemp = 0;

	return lpszDisplayName;


}
/////////////////////////////////////////////////////////////////////////////
// Called from external LoadFolders to get the search token from
// Mail path name
char * OEImportClass::GetOE4SearchToken(char *lpszStorePathName)
{
	char *lpszLocalPathName = DEBUG_NEW char[_MAX_PATH];

	strcpy(lpszLocalPathName, lpszStorePathName);

	if(lpszLocalPathName[strlen(lpszLocalPathName) - 1] != '\\')
		strcat(lpszLocalPathName, "\\");

	strcat(lpszLocalPathName, "*.mbx");

	return lpszLocalPathName;

}
// **************************************************************************
// ******* End LOAD FOLDERS ROUTINES


// Called from external MakeMimeDigest when we have a specific file to process
// Compares file ending type to see which version(in case this was our entry
// point ina  user selected mailbox from the wizard
// Calls GetNumMsgs to retrieve number of messages
// Calls CreateDigestFile to make the Mime Digest File
// Calls InitDigestFile to write header info and get boundary names
// Calls OpenTempFile to write a message at a time to temp before 
// transferring to digest file
// Calls GrabNextMsg in a loop to get all the messages
// Calls EscapePressed to see if user is cancelling
// Calls DBXwithLFOnly to see if it is international OE box
// Calls OELFtoCRLF to make it proper if it was intl'
// Calls WriteToDigestFile writes the temp file into the Digest
// calls DeleteTempFile to delete our temp file
// never delete digest file, we are passing the location of it
// Caller can delete.
char * OEImportClass::MakeMimeWithFile(char *lpszFilename, void * pID, unsigned int iSize)
{
	char *floater;
	
	// Open OE File
	floater = strrchr(lpszFilename, '.');
	if(floater)
	{
		floater++;
	}
	else
		return NULL;

	if(stricmp(floater, "dbx") == 0)
	{
		m_iOEVersion = OE_5_VERSION;
	}
	else
	if(stricmp(floater, "mbx") == 0)
	{
		m_iOEVersion = OE_4_VERSION;
			
	}
	else
		return NULL;
	m_fpiOEFile = fopen(lpszFilename, "rb");
	if(!m_fpiOEFile)
		return NULL;

	// Open output mime file
	LONG numMsg = GetNumMsgs(lpszFilename);

	if(numMsg < 1)
		return NULL;

	// Open Fabulous Files
	m_fpoDigestFile = CreateDigestFile();
	if(!m_fpoDigestFile)
		return NULL;
	InitDigestFile();

	OpenTempFile();
	while(GrabNextMsg())
	{
		if (EscapePressed(1))
			break;

		// if LF put CRLF,
		// For VI 5784 Trouble importing intl mailboxes from oe5
		// CRLF takes care of this, we already do QP stuff
		// jm - 06/00
		if(DBXwithLFOnly())
		{
			// Makes a new temp file and deletes old one
			// Sets temp path to new temp file
			// new temp file contains the CRLF pair
			OELFtoCRLF();
		}
		
		CloseTempFile();
		WriteToDigestFile();
		OpenTempFile();

	}
	CloseTempFile();
	WriteDigestEnd();

	fclose(m_fpiOEFile);
	DeleteTempFile();
	return m_lpszDigestPath;
}


// Gets the amount of messages
// Called from MakeMimeWithFile
LONG OEImportClass::GetNumMsgs(char *pathname)
{
	ULONG	NumMsgs = 0;

	// DRW 9/13/99 - Added OE5 handling.
	if (m_iOEVersion == OE_4_VERSION)
	{
		FILE * stream;
		char tempbuf[4];
		char * indexfile;
		char * floater;
		int iRes;

		if (!pathname)
			return -1;

		indexfile = DEBUG_NEW char [strlen(pathname)+1];
		strcpy(indexfile, pathname);
		floater = strrchr(indexfile, '.');
		strcpy(floater, ".idx");

	   stream = fopen(indexfile, "rb" );
	   if(!stream)
	   {
		   delete [] indexfile;
		   return -1;
	   }
	   else
	   {

		   iRes = fseek(stream, 8, SEEK_SET);
			if (iRes != 0)
			{
				fclose(stream);
				delete [] indexfile;
				return -1;  
			}

			iRes = fread(tempbuf, sizeof(char), 4, stream);
			if (iRes < 4)
			{
				fclose(stream);
				delete [] indexfile;
				return -1;  
			}

			fclose(stream);
				
			((char *)&NumMsgs)[3] = tempbuf[3];
			((char *)&NumMsgs)[2] = tempbuf[2];
			((char *)&NumMsgs)[1] = tempbuf[1];
			((char *)&NumMsgs)[0] = tempbuf[0];

			delete [] indexfile;
	   }
	}
	else
	if (m_iOEVersion == OE_5_VERSION)
	{
		// What follows is Jason Miller's OE5 importing code, taken directly from the DBXwindow project.

		// Beginning of the file Header
		typedef struct
		{
			char oe5Header[32];			// Looks OE5 specific havent decoded
			char Unknown16[16];			// stuff
			long lAttributeLookupAddress;	// One Address of Attribute Addresses
			char Unknown144[144];		// stuff
			long lMessageCount;			// Total amount of messages
			char Unknown28[28];			// More Stuff
			long lMasterTable;			// If different attr add. Then this is a master table
		} oe5_file_header;

		typedef struct
		{
			long lcurrentaddress;		// Where we are in the file
			char Unknown4[4];			// Stuff
			long lOneTable;				// A Table that is 'stuck' in there
			char Unknown5[5];			// Stuff
			#pragma pack(push,1)				// To push the pack to get to Table count
			BYTE lTableCount;			// How many tables here so we can make memory for an array
			char Unknown10[10];			// Stuff
			#pragma pack(pop)
		} master_table_header;		// sometimes there is no master table for smaller 
										// files it will be done in the implementation
		typedef struct
		{
			long lcurrentaddress;		// Location in fiel we are
			char Unknown4[4];			// Stuff
			long lFirstTable;			// First Table 'stuck' there
			long lAddressofReference;	// Address of Who pointed to this super index header
			BYTE Unknown1;				//Stuff
			#pragma pack(push,1)				// PUSH
			BYTE lTableCount;			// How many to allocate array space
			char Unknown10[10];			// Stuff
			#pragma pack(pop)
		} super_index_header;			// Master table points to some of these structures. 
										//sometimes there are none esp. if there is no master table
		typedef struct
		{
			long lcurrentaddress;		// Where we are
			char Unknown8[8];			// Stuff
			long lAddressofReference;	// Who pointed to us
			BYTE Unknown1;				// Stuff
			#pragma pack(push,1)				// PUSH
			BYTE lTableCount;			// How many elements for the array
			char Unknown7[6];			// Stuff
			#pragma pack(pop)
		} index_header;

		int					 mnt=0, it=0, count = 0;	// iterators
		oe5_file_header		 FileHeader;				// Beginning of file info
		master_table_header	 Master;					// Master index Sometimes is a Super Index in smaller files
		super_index_header	 SuperIndex;				// Super index
		index_header		 Index;						// Index of attributes and addresses of messages
		long				 tmpbuf;					// Temporary storage buffer to read
		long				 lIndexMemberCount = 0;
		long				*lSuperIndexArr = NULL;		//my arrays aforementioned
		long				*lIndexAddressArr = NULL;	//my arrays aforementioned
		long				 lSeekPos = 0;
		int					 iFileRes;
		// BYTE				 lBlockCount;				//not used i think
		// long				 MiniBlockCount;			//not used i think
		// BOOL				 cbIsLarge=FALSE;			// Not used I think

		m_lOE5MsgTable = NULL;
		m_lNumOE5MsgTableEntries = 0;
		m_lCurrOE5MsgTableEntry = 0;

/*		#ifdef INDBXWINDOW
		if (m_infile.Open(pathname, CFile::modeRead) == 0)
		#else
*/
/*		m_fpiOEFile = fopen(pathname, "r");
		if (!m_fpiOEFile)
//		#endif
		{
			return -1;  
		}
*/
		//Get the header

		iFileRes = fseek(m_fpiOEFile, 0, SEEK_SET);
		if(iFileRes != 0)
			return -1;

		iFileRes = fread((char*)&FileHeader, sizeof(oe5_file_header), 1, m_fpiOEFile);
		if(iFileRes < 1)
		{
			return -1;
		}

		// Obsolete but may be needed later
		// if(FileHeader.lAttributeLookupAddress > 123476)	cbIsLarge=TRUE;
		
		NumMsgs = FileHeader.lMessageCount;

		// DRW - Don't do anything if we don't have any messages.  new()ing something
		// of 0 length causes problems in Eudora.
		if (NumMsgs > 0)
		{
			if (m_lOE5MsgTable)
			{
				delete m_lOE5MsgTable;
			}

			// Allocate space for all the attributes for each message
			m_lOE5MsgTable = DEBUG_NEW long [FileHeader.lMessageCount];
			m_lNumOE5MsgTableEntries = FileHeader.lMessageCount;

			// More than ~30 messages need tables.
			if(FileHeader.lAttributeLookupAddress != FileHeader.lMasterTable)
			{
				//Retrieve master table
				iFileRes = fseek(m_fpiOEFile, FileHeader.lMasterTable, SEEK_SET);
				if(iFileRes !=0)
					return -1;

				iFileRes  = fread((char*)&Master, sizeof(master_table_header), 1, m_fpiOEFile);
				if(iFileRes < 1)
					return -1;

				//Set first item to Super table holder
				lSuperIndexArr = DEBUG_NEW long[(Master.lTableCount * 2) + 1];
				lSuperIndexArr[0] = Master.lOneTable;

				//Get rest
				lSeekPos = FileHeader.lMasterTable + sizeof(master_table_header);
				for(mnt = 1; mnt < Master.lTableCount + 1; mnt++)
				{
					iFileRes = fread((char*)&tmpbuf, sizeof(long), 1, m_fpiOEFile);
					if(iFileRes < 1)
						return -1;

					lSeekPos += sizeof(long);
					lSuperIndexArr[mnt] = tmpbuf;
					lSeekPos += 8L;
					iFileRes = fseek(m_fpiOEFile, lSeekPos, SEEK_SET);
					if(iFileRes != 0)
						return -1;
				}
				// hmm i forget why i did this willl take a closer look later
				// oh yeah to see if this was in fact a master table/ super index
				// situation
				iFileRes = fseek(m_fpiOEFile, lSuperIndexArr[1], SEEK_SET);
				if(iFileRes != 0)
					return -1;

				iFileRes = fread((char*)&SuperIndex, sizeof(super_index_header), 1, m_fpiOEFile);
				if(iFileRes < 1)
					return -1;
				// if it is 0 then we are in a smaller file and
				//no super index is needed cuz master table acts same
				if(SuperIndex.lFirstTable != 0)
				{
					// Get amount of Index Tables
					for(mnt = 0; mnt < Master.lTableCount + 1; mnt++)
					{
						iFileRes = fseek(m_fpiOEFile, lSuperIndexArr[mnt], SEEK_SET);
						if(iFileRes != 0)
							return -1;

						iFileRes = fread((char*)&SuperIndex, sizeof(super_index_header), 1, m_fpiOEFile);
						if(iFileRes < 1)
							return -1;

						lIndexMemberCount += SuperIndex.lTableCount + 1;
					}
					// Allocate space for index tables
					lIndexAddressArr = DEBUG_NEW long[lIndexMemberCount * 2];
					
					mnt = 0;
					lIndexAddressArr[mnt++] = SuperIndex.lAddressofReference;
					
					for(it = 0; it < Master.lTableCount + 1; it++)
					{
						iFileRes = fseek(m_fpiOEFile, lSuperIndexArr[it], SEEK_SET);
						if(iFileRes != 0)
							return -1;

						iFileRes = fread((char*)&SuperIndex, sizeof(super_index_header), 1, m_fpiOEFile);
						if(iFileRes < 1)
							return -1;

						lSeekPos = lSuperIndexArr[it] + sizeof(super_index_header);
							
						lIndexAddressArr[mnt++] = SuperIndex.lFirstTable;		
						lIndexAddressArr[mnt++] = SuperIndex.lcurrentaddress;

						for(count = 0; count < SuperIndex.lTableCount; count++)
						{
							iFileRes = fread((char*)&tmpbuf, sizeof(long),1, m_fpiOEFile);
							if(iFileRes < 1)
								return -1;

							lSeekPos += (sizeof(long) + 8L);
							iFileRes = fseek(m_fpiOEFile, lSeekPos, SEEK_SET);
							if(iFileRes != 0)
								return -1;

							if (mnt < lIndexMemberCount * 2)
							{
								lIndexAddressArr[mnt++] = tmpbuf;
							}
							else
							{
//								TRACE("Overflowing allocated space\n");
							}
						}	
					}
					
					int tempMax;
					tempMax = mnt;
					it = 0;

					for(mnt= 0; mnt < tempMax; mnt++)
					{
						iFileRes = fseek(m_fpiOEFile, lIndexAddressArr[mnt], SEEK_SET);
						if(iFileRes != 0)
							return -1;

						iFileRes = fread((char*)&Index, sizeof(index_header), 1, m_fpiOEFile);
						if(iFileRes < 1)
							return -1;

						lSeekPos = lIndexAddressArr[mnt] + sizeof(index_header);

						for(count= 0; count < Index.lTableCount; count++)
						{
							iFileRes = fread((char*)&tmpbuf, sizeof(long), 1, m_fpiOEFile);
							if(iFileRes < 1)
								return -1;

							lSeekPos += (sizeof(long) + 8L);
							iFileRes = fseek(m_fpiOEFile, lSeekPos, SEEK_SET);
							if(iFileRes != 0)
								return -1;

							m_lOE5MsgTable[it++] = tmpbuf;
						}
					}
				}
				else
				{
					lSuperIndexArr[mnt] = Master.lcurrentaddress;

					it = 0;

					for(mnt= 0; mnt < Master.lTableCount+2; mnt++)
					{
						iFileRes = fseek(m_fpiOEFile, lSuperIndexArr[mnt], SEEK_SET);
						if(iFileRes != 0)
							return -1;

						iFileRes = fread((char*)&Index, sizeof(index_header), 1, m_fpiOEFile);
						if(iFileRes < 1)
							return -1;

						lSeekPos = lSuperIndexArr[mnt] + sizeof(index_header);

						for(count= 0; count < Index.lTableCount; count++)
						{
							iFileRes = fread((char*)&tmpbuf, sizeof(long), 1, m_fpiOEFile);
							if(iFileRes < 1)
								return -1;

							lSeekPos += (sizeof(long) + 8L);
							iFileRes = fseek(m_fpiOEFile, lSeekPos, SEEK_SET);
							if(iFileRes != 0)
								return -1;

							m_lOE5MsgTable[it++] = tmpbuf;
						}
					}
				}
			}
			else
			{
				iFileRes = fseek(m_fpiOEFile, FileHeader.lMasterTable, SEEK_SET);
				if(iFileRes != 0)
					return -1;

				iFileRes = fread((char*)&Index, sizeof(index_header), 1, m_fpiOEFile);
				if(iFileRes < 1)
					return -1;
				lSeekPos = FileHeader.lMasterTable + sizeof(index_header);
				
				for(count= 0; count < Index.lTableCount; count++)
				{
					iFileRes = fread((char*)&tmpbuf, sizeof(long), 1, m_fpiOEFile);
					if(iFileRes < 1)
						return -1;

					lSeekPos += (sizeof(long) + 8L);
					iFileRes = fseek(m_fpiOEFile, lSeekPos, SEEK_SET);
					if(iFileRes != 0)
						return -1;

					m_lOE5MsgTable[it++] = tmpbuf;
				}
			}
		}

		if (lSuperIndexArr)		delete [] lSuperIndexArr;
		if (lIndexAddressArr)	delete [] lIndexAddressArr;
		
	}
	return NumMsgs;
}
// END GetNumMsgs

// Called from MakeMimeWithFile to create Digest File
FILE * OEImportClass::CreateDigestFile()
{
	char *lpszFloater;
	FILE *fp;

	if(m_lpszDigestPath)
	{
		free (m_lpszDigestPath);
		m_lpszDigestPath = NULL;
	}

	m_lpszDigestPath = _tempnam("","");
	lpszFloater = strrchr(m_lpszDigestPath, '\\');
	if(*lpszFloater)
		*lpszFloater = 0;
	strcpy(m_szTempDirName, m_lpszDigestPath);

	if(m_lpszDigestPath)
	{
		free (m_lpszDigestPath);
		m_lpszDigestPath = NULL;
	}

	if(m_szTempDirName[strlen(m_szTempDirName) - 1] != '\\')
		strcat(m_szTempDirName, "\\");
	
	m_lpszDigestPath = _tempnam(m_szTempDirName, "EuImport");

	fp = fopen(m_lpszDigestPath, "w+b");

	return fp;

}
// Called from MakeMimeWithFile to Initiailize the member
// boundary variables and write out header info ont he digest
bool OEImportClass::InitDigestFile()
{

	int chsize = sizeof(char);

	sprintf(m_szMainBoundary, "=====================main_%ld==_", GetTickCount());

	fwrite("MIME-Version: 1.0\r\n", chsize, 19, m_fpoDigestFile); 
	fwrite("Content-Type: multipart/digest;\r\n", chsize, 33, m_fpoDigestFile);
	fwrite("\tboundary=\"", chsize, 11, m_fpoDigestFile);
	fwrite(m_szMainBoundary, chsize, strlen(m_szMainBoundary), m_fpoDigestFile);
	fwrite("\"\r\n\r\n", chsize, 5, m_fpoDigestFile);

	return true;
}

// Called to write messages into the Digest file
bool OEImportClass::WriteToDigestFile()
{
	FILE * fpiTmpFile;
	int chsize = sizeof(char);
	int ibufsize = 600;
	char szBuffer[600];
	int ibufread;
	int iprevBuf;

	fpiTmpFile = fopen(m_TempFilePathname, "rb");

	if(!fpiTmpFile)
		return false;

	fwrite("--", chsize, 2, m_fpoDigestFile);
	fwrite(m_szMainBoundary, chsize, strlen(m_szMainBoundary), m_fpoDigestFile);
	fwrite("\r\n", chsize, 2, m_fpoDigestFile);
	fwrite("Content-Type: message/rfc822\r\n", chsize, 30, m_fpoDigestFile);
	fwrite("\r\n", chsize, 2, m_fpoDigestFile);


	do
	{
		ibufread = fread(szBuffer, chsize, ibufsize, fpiTmpFile);
		if(ibufread != 0)
		{
			iprevBuf = fwrite(szBuffer, chsize, ibufread, m_fpoDigestFile);
			if(iprevBuf != ibufread)
			{
				iprevBuf += 0;

			}
		}
	}while(ibufread);

	fwrite("\r\n", chsize, 2, m_fpoDigestFile);

	fclose(fpiTmpFile);
	return true;
}

// Called from MakeMimeWithFile to Write the end of the digest file
bool OEImportClass::WriteDigestEnd()
{
	int chsize = sizeof(char);

	fwrite("--", chsize, 2, m_fpoDigestFile);
	fwrite(m_szMainBoundary, chsize, strlen(m_szMainBoundary), m_fpoDigestFile);
	fwrite("--", chsize, 2, m_fpoDigestFile);
	fwrite("\r\n", chsize, 2, m_fpoDigestFile);
	fclose(m_fpoDigestFile);

	return true;
}

// Called from MakeMimeWithFile primarily to open
// create/overwrite a temp file and set the member variable
bool OEImportClass::OpenTempFile()
{
	if (!m_TempFilePathname)
	{
		m_TempFilePathname = _tempnam(".","EUD");
	}

	m_tempfile = fopen(m_TempFilePathname, "w+b");
	if(!m_tempfile)
	{
		return (false);
	}

	return (true);
}

// Closes our temp file member variable
// Called from lotsa places as well
// mainly MakeMimeWithFile to close it
// so we can overwrite it also called in
// such fns such as OELFtoCRLF to get stats on it
bool OEImportClass::CloseTempFile()
{
	int err = fflush(m_tempfile);
	err = fclose(m_tempfile);
	return true;
}

// Deletes our Temp file member
// Called from a couple places like MakeMimeWithFile for cleanup
// and OELFtoCRLF to switch temp files
bool OEImportClass::DeleteTempFile()
{

	if (m_TempFilePathname)
	{
		DeleteFile(m_TempFilePathname);

	}return true;
}



// Called from MakeMimeWithFile in a loop to get all msgs
// Calls FindToken to find OE4 Tokens
// Calls ReadTilDone to actually read the message out
bool OEImportClass::GrabNextMsg()		// Large chunk size
{
	// DRW 9/13/99 - Added OE5 handling.
	if (m_iOEVersion == OE_4_VERSION)
	{
		char tempbuffer[2] = {0x00, 0x00};
		int readbufsize = 4096;

		HRESULT badRead = 0;

		if (!FindToken())
		{
			return false;
		}

		return ReadTilDone();
	}
	else
	if(m_iOEVersion == OE_5_VERSION)
	{
		if (m_lOE5MsgTable && (m_lCurrOE5MsgTableEntry < m_lNumOE5MsgTableEntries))
		{
			// DRW 9/13/99 - Look in file specified by pathname for number of messages.
			bool	bReturn = ReadTilDone();

			++m_lCurrOE5MsgTableEntry;

			return bReturn;
		}
	}
	return false;
}
// Called from GrabNextMsg to actually parse out all mail
bool OEImportClass::ReadTilDone()
{
	// DRW 9/13/99 - Added OE5 handling.
	if (m_iOEVersion == OE_4_VERSION)
	{
		char *readbuffer;
		int readbufsize = 4096;
		readbuffer = DEBUG_NEW char[readbufsize+1]; //4097
		readbuffer[readbufsize] = 0;	// Let's never get this far...
		int iFileRes;
		char * floater;
		int posn;
		long bytesread;
		bool done = false;

		while (!done)
		{
			posn = 1;

			long blah;
			blah = ftell(m_fpiOEFile);

			iFileRes = fseek(m_fpiOEFile, blah, SEEK_SET);
			if(iFileRes != 0)
				return false;

			bytesread = fread(readbuffer, sizeof(int), readbufsize, m_fpiOEFile);
			if (bytesread < 1)
			{
				delete [] readbuffer;
				return false;
			}

			floater = readbuffer;

			while (*floater != 0 && posn < bytesread)
			{
				floater++;
				posn++;
			}

			if (*floater == 0)
			{
				fwrite(readbuffer, sizeof(char), posn-1, m_tempfile);


				fwrite("\r\n", sizeof(char), 2, m_tempfile);
				if (posn < bytesread)
				{	// Has to be negative, we're trying to go backward. shouldn't be able to fail.
					iFileRes = fseek(m_fpiOEFile, posn-bytesread,SEEK_CUR);
				}

				if (readbufsize != 0)
				{	// Now to make sure the tempfile gets only the data it needs and extraneous nulls are removed.
					

				}
				done = true;
			}
			else if (bytesread < readbufsize)	// End of file may not have a null at the end...
			{									// No need to rewind the file any.
				fwrite(readbuffer, sizeof(char), bytesread, m_tempfile);
				fwrite("\r\n", sizeof(char), 2, m_tempfile);
				done = true;
			}
			else
			{
				fwrite(readbuffer, sizeof(char), posn, m_tempfile);
			}
		}
		delete [] readbuffer;
	}
	else
	if (m_iOEVersion == OE_5_VERSION)
	{
		// What follows is Jason Miller's OE5 importing code, taken directly from the DBXwindow project.

		typedef struct
		{
			long lcurrentposition;		// Where we are
			long lFlags;				// Flags on the message
			long lSectionSize;			// How many bytes in this chunk
			long lNextHeader;			// Where next header is
		} message_header;				// This is for each chunk of each message

		typedef struct
		{
			long lcurrentposition;		// Where we are
			long lLength;				// How big this all is
			short int lLengthtoNextHeader;	// Where the next is
			char Unknown7[7];				// Stuff
			BYTE cbRead;				// Is Message Read
			BYTE cbAttach;				// Any attachments
			BYTE cbStatus;				// Forwarded replied?
			char Unknown5[5];			// Stuff
			#pragma pack(push,1)				// PUSH
			long lMessagePosition;		// Maybe where to address of the message is
			char Unknown24[24];			// Stuff
			long lMessageSizeTest1;		// Not used
			BYTE cbImportant;			// Low, reg, hi priority flag
			char Unknown3[3];			// Stuff
			long lMessageSizeTest2;		// Not used
			char Unknown7_2[7];			// STUFF
			long lMessagePosition0;		// All of the following are figuring out where the address
			long lMessagePosition1;		// of the message is. This is not necessary as we will see in
			long lMessagePosition2;		// The implementation
			long lMessagePosition3;
			long lMessagePosition4;
			long lMessagePosition5;
			long lMessagePosition6;
			long lMessagePosition7;
			#pragma pack(pop)
		} oeTable;	// Attributes header so sucky

		typedef struct
		{
			long lPosition;
			long lIndexSize;
			unsigned short	lLengthOfObject;
			BYTE			cbEntries;
			BYTE			cbChanges;
		} OEMessageHeader;

		typedef struct
		{
			BYTE	cbIndex;
#pragma pack(push, 1)
			 long lValue;
#pragma pack(pop)
		} OEMessageHeaderInfo;

		typedef struct
		{
			OEMessageHeaderInfo oeInfo;
			void * pOEHeaderList;
		} OEMessageHeaderInfoList;

		typedef struct
		{
			long		lIndex;
			long		lMessageFlags;
			FILETIME	ftCreateSend;
			long		lMessageLines;
			long		lMessageOffset;
			char		*lpszOrigSubject;
			FILETIME	ftSaved;
			char		*lpszMsgID;
			char		*lpszSubject;
			char		*lpszSenderMailName;
			char		*lpszAnswerToID;
			char		*lpszServerNumber;
			char		*lpszServer;
			char		*lpszSenderName;
			char		*lpszSenderMail;
			char		*lpszUnknown;
			long		lPriority;
			long		lMessageTextLength; // dont use
			FILETIME	ftCreateReceived;
			char		*lpszReceiverName;
			char		*lpszReceiverAddr;
			char		*lpszUnknown2;
			long		lUnknown3;
			char		*lpszUnknown4;
			long		lUnknown5;
			long		lUnknown6;
			char		*lpszAcctName;
			char		*lpszRegKey;
		} OEMessageAttributes;

		message_header		*myHeader;				// Local Message Header structure
		oeTable				*myTable;				// Local Message Table structure
		
		OEMessageHeader		oeHeader;
		OEMessageHeaderInfoList	*oeInfoList;
	
		
		long				 lMessageLocation=0;	// Message location for unsigned short overflow
		int					 breakme=0;
		BOOL				 xxxFlag = TRUE;
		LPSTR				 szMessageText = NULL;
		LPSTR				 szPtr = NULL;
		long				 lTextSize = 0;
		long				 lFileLength = 0;
		int					 iFileRes;

		// Determine file length.
/*		#ifdef INDBXWINDOW
		CFileStatus			 filestat;				// To get file size
		m_infile.GetStatus(filestat);				// Fill the file status box
		lFileLength = filestat.m_size;
		#else
*/
		
		iFileRes = fseek(m_fpiOEFile, 0, SEEK_END);
		lFileLength = ftell(m_fpiOEFile);
		fseek(m_fpiOEFile, 0, SEEK_SET);

//		#endif

		if (m_lOE5MsgTable == NULL)
		{
			return false;
		}
		else
		{
			myTable = DEBUG_NEW oeTable;	// Initialize memory(I hope)
			myHeader = DEBUG_NEW message_header;	// Initialize memory(I hope)

			fseek(m_fpiOEFile, m_lOE5MsgTable[m_lCurrOE5MsgTableEntry], SEEK_SET);			
			fread((char*)myTable, sizeof(oeTable), 1, m_fpiOEFile);	// Read the table

			fseek(m_fpiOEFile, m_lOE5MsgTable[m_lCurrOE5MsgTableEntry], SEEK_SET);			
			fread((char*)&oeHeader, sizeof(OEMessageHeader), 1, m_fpiOEFile);	// Read the table

			oeInfoList = DEBUG_NEW OEMessageHeaderInfoList[oeHeader.cbEntries];
			


			for( int oemh = 0; oemh < oeHeader.cbEntries; oemh++)
			{
				fread((char*)&(oeInfoList[oemh].oeInfo), sizeof(OEMessageHeaderInfo), 1, m_fpiOEFile);	// Read the table
				fseek(m_fpiOEFile, -1, SEEK_CUR);
				oeInfoList[oemh].oeInfo.lValue &= 0x00FFFFFF;
			}
			oemh = 0;
			bool bFoundOffset = false;

			for(oemh; oemh < oeHeader.cbEntries; oemh++)
			{
				if( oeInfoList[oemh].oeInfo.cbIndex & HEADER_MESSAGE_OFFSET)
				{
					bFoundOffset = true;

					if(oeInfoList[oemh].oeInfo.cbIndex & INLINE_VALUE_TOKEN)
					{
						lMessageLocation = oeInfoList[oemh].oeInfo.lValue;
						break;
					}
					else
					{
						fseek(m_fpiOEFile, oeInfoList[oemh].oeInfo.lValue, SEEK_CUR);

						fread((char*) &lMessageLocation, sizeof(long),  1, m_fpiOEFile);
						break;

					}
				}
					
			}

			if(!bFoundOffset)
			{
				delete myTable;
				delete myHeader;
				delete [] oeInfoList;
				return false; // Skipped Message
			}

			if( lMessageLocation > lFileLength)
			{
				delete myTable;
				delete myHeader;
				delete [] oeInfoList;
				return false; // Skipped Message
			}

			fseek(m_fpiOEFile, lMessageLocation, SEEK_SET); // just get the value

			fread((char*)myHeader, sizeof(message_header), 1, m_fpiOEFile);	//Read the header now that we there

			if (myHeader->lNextHeader < 0)
			{
				delete myTable;
				delete myHeader;
				delete [] oeInfoList;
				return false;
			}

			if(myHeader->lFlags == 0x01fc)
			{
				delete myTable;
				delete myHeader;
				delete [] oeInfoList;
				return false; // Flag for deleted message(I havent seen???)
			}
			if(myHeader->lSectionSize > /*!=*/ 512 && myHeader->lSectionSize != 0) xxxFlag = FALSE;	//Not a valid message
			
			lTextSize = 0;					// Initialize to zero
			long tmp;							// temporary long for use

			do
			{
				// Add section size (for this chunk) to text size so we can allocate
				lTextSize += myHeader->lSectionSize;	

				if(myHeader->lNextHeader == 0) break;	// No more!
				
				if(myHeader->lNextHeader > lFileLength)	//If it goes past the file dummy MS
				{
					lTextSize -= myHeader->lSectionSize;	// Find the section size and ditch it

					lTextSize +=	(lFileLength - tmp);// Add the real size to read
					break; 
				}
				
				fseek(m_fpiOEFile, myHeader->lNextHeader, SEEK_SET);	// To next header
				fread((char*) myHeader, sizeof(message_header), 1, m_fpiOEFile);		// Get it
				
/*				#ifdef INDBXWINDOW
				tmp = m_fpiOEFile.GetPosition();					//Current position(start of data chunk)
				#else
*/
				tmp = ftell(m_fpiOEFile);							// Current position(start of data chunk)
//				#endif
			}while(TRUE);								// Til thereis no more
			
			szMessageText = (LPSTR) DEBUG_NEW char[lTextSize];	// Allocate space for the message
															// Later tmp file maybe(doubt it)

			//if smaller than first message position we overflowed our unsigned short
			fseek(m_fpiOEFile, lMessageLocation, SEEK_SET);	// Goto Message

			
			fread((char*)myHeader, sizeof(message_header), 1, m_fpiOEFile);	// Get the header again

			szPtr = szMessageText;		// Point to beginning of buffer

			do
			{
				fread((char*)szPtr,myHeader->lSectionSize, 1, m_fpiOEFile);	// Read section
				szPtr += myHeader->lSectionSize;				// Goto end of section

				if(myHeader->lNextHeader == 0) break;			// No more, exit

				if(myHeader->lNextHeader > lFileLength)		// if bigger than file end deal withit
				{
					szPtr -= 512;	// oop should be sectionsize.but this is value of sectionsize(inconsistent)
					szPtr +=	(lFileLength - tmp);	// Add the rest of file
					break;
				}

				fseek(m_fpiOEFile, myHeader->lNextHeader,SEEK_SET);	// To next chunk
				fread((char*)myHeader,sizeof(message_header), 1, m_fpiOEFile);			// Read the header

/*				#ifdef INDBXWINDOW
				tmp = m_fpiOEFile.GetPosition();						// Keep track of beginning of data
				#else
*/
				tmp = ftell(m_fpiOEFile);								// Keep track of beginning of data
//				#endif
			
			}while(TRUE);			// Til no more
			szPtr = szMessageText;

			if ((myTable->cbRead == 1) || (myTable->cbRead == 33))
			{
				fwrite("Status: U\r\n", sizeof(char), 11, m_tempfile);
			}
			fwrite(szMessageText, sizeof(char), lTextSize, m_tempfile);

			// Some imported messages don't end in carriage return/linefeed which
			// causes MoveMsgToEudora() to import them incorrectly.  Add "\r\n"
			// if it is needed.
			if ((szMessageText[lTextSize - 2] != '\r') && (szMessageText[lTextSize - 1] != '\n'))
			{
				fwrite("\r\n", sizeof(char), 2, m_tempfile);
			}

	/*		#ifdef DO_JASONS_DEBUGGING
			int q,z=0, spacer;
			LPSTR Subject='\0';
			CString Holdme, Writeme2;

			for(q=0;q<lTextSize;q++)// Lazy loop to find subject..hope no XHeaders start with X-Subj
			{
				if(szPtr[q] == 'S')
				{
					if(szPtr[q+1] == 'u')
					{
						if(szPtr[q+2] == 'b')
						{
							if(szPtr[q+3] == 'j')
							{
							Subject = &szPtr[q];
							break;
							}
						}
					}
				}
			}

			if(Subject != NULL)
			{
				while(Subject[z]!='\r')// finding where subject ends
				{
				z++;
				};
			Subject[/*++*//*z] = '\0';
			spacer = z;//Make it columnar
			
			Holdme = Subject;
			}else
			{
			Holdme = "<No Subject>"; 
			spacer=12;
			}

			if(m_lCurrOE5MsgTableEntry > 8)//Make it columnar
			{
				spacer++;
			}
			if(m_lCurrOE5MsgTableEntry > 98)//Make it columnar
			{
				spacer++;
			}
				
			char TitleNumber[6];	// To write Message # inf ront of subject
			sprintf(TitleNumber,"%ld",m_SubjectNumber++);
			Writeme2 += TitleNumber;
			Writeme2 += ". " + Holdme;
			for(spacer;spacer < 120;spacer++)//Make it columnar
			{
				Writeme2 += " ";
			}

			switch(myTable->cbRead)// Flagged message and read/unread byte
			{
			case 1:   {Writeme2 += "Unread      ";break;}
			case 33:   {Writeme2 += "UFlagged    ";break;}
			case 129:  {Writeme2 += "Read        ";break;}
			case 161: {Writeme2 += "RFlagged    ";break;}
			default: {Writeme2 += "Unknown     ";break;}
			}

			switch(myTable->cbAttach)//if attachment or not byte
			{
			case 0: {Writeme2 += "No Attachment    "; break;}
			case 64: {Writeme2 += "Attachment       "; break;}
			default: {Writeme2 += "Unknown          "; break;}
			}

			switch(myTable->cbStatus)// Whether replied forwarded or none byte
			{
			case 0: {Writeme2 += "Unread      ";break;}
			case 2: {Writeme2 += "Read        ";break;}
			case 10: {Writeme2 += "Replied     "; break;}
			case 18: {Writeme2 += "Forwarded   "; break;}
			default: {Writeme2 += "Unknown     "; break;}
			}
			
			switch(myTable->cbImportant)//Important byte
			{
			case 1: {Writeme2 += "High Priority"; break;}
			case 3: {Writeme2 += "Regular Priority"; break;}
			case 5: {Writeme2 += "Low Priority"; break;}
			case 123: {Writeme2 +="Unread Regular"; break;}
			default: {Writeme2 += "Regular Priority"; break;}
			}

			Writeme2 += "\r\n";// next line

			CFile SubjectFile;
			
			SubjectFile.Open("Subjects.txt", CFile::modeWrite, NULL);
			SubjectFile.SeekToEnd();
			int strlen;
			strlen = Writeme2.GetLength();
			SubjectFile.Write(Writeme2, strlen);
			SubjectFile.Close();
			#endif // DO_JASONS_DEBUGGING
*/
			if (szMessageText)	delete [] szMessageText;
			if (myHeader)		delete myHeader; 
			if (myTable)		delete myTable;
			if (oeInfoList)		delete [] oeInfoList;
		}
	}
	return true;
}// END ReadTilDone

// To let user abort
// Called from MakeMimeWithFile
int OEImportClass::EscapePressed(int Repost /*= FALSE*/)
{

	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		// If Esc key was hit, restore mailbox and quit out
		if (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE)
		{
			if (Repost)
				PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			return (TRUE);
		}
		DispatchMessage(&msg);

	}

	return (FALSE);
}
// Checks to see if there are \n only
// returns true if so so we can make the file \r\n
// Called from MakeMimeWithFile
// Calls TempFile operations
bool OEImportClass::DBXwithLFOnly()
{
	int lBufReadSize;
	char szheaderbuf[1024];
	struct _stat  st;

	CloseTempFile();
	if(_stat(m_TempFilePathname, &st) != 0)
		return false;

	lBufReadSize = st.st_size;
	m_tempfile = fopen(m_TempFilePathname, "rb");
	fseek(m_tempfile, 0, SEEK_SET); //to the beginning

	if(lBufReadSize > 1024)
		lBufReadSize = 1024;

	fread(szheaderbuf, sizeof(char), lBufReadSize, m_tempfile);

	for(int i=0; i < lBufReadSize; i++)
	{
		if(szheaderbuf[i] == '\n')
		{
			if(szheaderbuf[i-1] == '\r')
				return false;
			else
				return true;

		}
	}

	return false;

}
// Makes a new temp file with appropriate \r\n from
// bad temp file with only \n
// deletes old temp file and points tempfile to itself
// Called from MakeMimeWithFile
void OEImportClass::OELFtoCRLF()
{
	// OE temp file, Path, buffer for text, current char of buffer,
	// and size for buffer
	FILE *	fOEtmp;
	char*	szOEtmpPath;
	char*	FileBuf = NULL;
	char	currchar;
	long	lBufSize;
	struct _stat st;

	// Get a brand new temp file name
	szOEtmpPath = _tempnam(".","EUD");

	fOEtmp = fopen(szOEtmpPath, "w+b");
	
	if(!fOEtmp)
	{
		return;
	}

	CloseTempFile();
	_stat(m_TempFilePathname, &st);

	m_tempfile = fopen(m_TempFilePathname, "rb");
	// Get the contents
	//	fseek(m_tempfile, 0, SEEK_END);
	//	lBufSize = ftell(m_tempfile);
	lBufSize = st.st_size;

	FileBuf = DEBUG_NEW char[lBufSize];
	fseek(m_tempfile, 0, SEEK_SET);
	fread(FileBuf, sizeof(char), lBufSize, m_tempfile);

	// Now one at a time look for LF and prepend a CR to new file
	for(int i = 0; i < lBufSize; i++)
	{
		currchar = FileBuf[i];
		
		if(currchar == '\n')
			fputc('\r', fOEtmp);

		fputc(currchar, fOEtmp);
	}

	// Switch temp file
	CloseTempFile();
	DeleteFile(m_TempFilePathname);
	m_TempFilePathname = szOEtmpPath;
	fclose(fOEtmp);

}

//////////////////////////////////////
// FindToken
//////////////////////
// Read one byte at a time, this is slow, but it's only for a few hundred bytes,
// the mass email moving stuff is read in chunks.
// Does it's reading/seeking from m_fpiOEFile.
// Called from GrabNextMsg
bool OEImportClass::FindToken() // testtoken should default to 0x00000000
{
	char buffer[2] = {0x00, 0x00};
	char testtoken[4] = {0x00, 0x00, 0x00, 0x00};
	char magictoken[4] = {0x00, 0x7F, 0x00, 0x7F};
	int i;
	
//	i = m_infile.Read(buffer,1);
	
	while ((memcmp (magictoken, testtoken,4) != 0) && (fread(buffer,sizeof(char), 1, m_fpiOEFile) > 0))
	{
		testtoken[0] = testtoken[1];
		testtoken[1] = testtoken[2];
		testtoken[2] = testtoken[3];
		testtoken[3] = buffer[0];
	}
	
	if (memcmp (magictoken, testtoken,4) != 0)
		return false;

	for (i = 1; i < 13; i++)
	{
		if (fread(buffer, sizeof(char), 1, m_fpiOEFile) < 1)
			return false;
	}
	return true;
}
// **************************************************************************
// ******* End MAKE DIGEST ROUTINES

// Address Book stuff
CAddressBookTree * OEImportClass::GetAddressTree(char *lpszPath)
{

	if(Convert(lpszPath))
		return m_pABookTree;
	else
		return NULL;

	return NULL;

}
bool OEImportClass::Convert(char* lpszWabPath)
{
	if( ! InitWAB(lpszWabPath))
		return false;
		

	if (!ProcessABook())
	{
		return false;
	}

	//create the address book tree. Here we have just one address book
	//hence the address book linked list had just a single node.
	m_pABookTree = DEBUG_NEW CAddressBookTree;
	InitAddrTree(m_pABookTree);
	m_pABookTree->lpszDisplayName = DEBUG_NEW char[strlen("Outlook Express") + 1];
	strcpy(m_pABookTree->lpszDisplayName, "Outlook Express");
	m_pABookTree->pFirstChild = m_pABook;
	m_pABookTree->pNext = NULL;

	return true;


}

bool OEImportClass::InitAddrTree(CAddressBookTree *pEntry)
{
	pEntry->lpszDisplayName = NULL;
	pEntry->pFirstDLChild = NULL;
	pEntry->pFirstChild = NULL;
	pEntry->pNext = NULL;
	return true;
}

bool OEImportClass::InitWAB(char *lpszFilePath)
{
	DWORD	dwType = 0;
	ULONG	cbData = sizeof(m_szWABDllPath);
	HKEY	wabkey = NULL;
	bool	bReturn = false;
	DWORD	dwRefCount = 0;

	*m_szWABDllPath = NULL;

	// Find wab32.dll path from registry using wabapi.h's WAB_DLL_PATH_KEY
	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &wabkey) == ERROR_SUCCESS)
		RegQueryValueEx(wabkey, "", 0, &dwType, (LPBYTE) m_szWABDllPath, &cbData);

	if(wabkey)
		RegCloseKey(wabkey);
	else
	{
		// Do Some stuff to find WAB.dll
		return false;

	}

	// Load it
	m_hWABDll = LoadLibrary( (lstrlen(m_szWABDllPath)) ? m_szWABDllPath : WAB_DLL_NAME);

	if(m_hWABDll)
	{
		// Get WABOpen function to member function pointer
		m_lpfnWabOpen = (LPWABOPEN) GetProcAddress(m_hWABDll, "WABOpen");
		
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs",
					0, KEY_ALL_ACCESS, &m_hSharedDllKey) == ERROR_SUCCESS)
		{
			cbData = sizeof(dwRefCount);
			RegQueryValueEx(m_hSharedDllKey, m_szWABDllPath, 0, &dwType,  (LPBYTE) &dwRefCount, &cbData);

			dwRefCount++;
			RegSetValueEx(m_hSharedDllKey, m_szWABDllPath, 0, REG_DWORD, (const PBYTE) &dwRefCount, sizeof(DWORD));
			
			RegCloseKey(m_hSharedDllKey);

		}

		if(m_lpfnWabOpen)
		{
			HRESULT		hr;
			WAB_PARAM	wp = {0};
			
			// Use Filename if you want a non-default WAB
			wp.cbSize = sizeof(WAB_PARAM);
			wp.szFileName = lpszFilePath;

			// perform the open
			hr = m_lpfnWabOpen(&m_lpAdrBook, &m_lpWABObject, &wp, 0);
			m_lpAdrBook->AddRef();
			m_lpWABObject->AddRef();

			if(hr == ERROR_SUCCESS)
			{
				bReturn = m_bInit = true;
			}
		}
		else
			bReturn = false;

	}
	else
	{
		// ByBye
		bReturn  = false;
	}
	
	if(bReturn == false)
	{
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs",
						0, KEY_ALL_ACCESS, &m_hSharedDllKey) == ERROR_SUCCESS)
			{
			RegQueryValueEx(m_hSharedDllKey, m_szWABDllPath, 0, &dwType,  (LPBYTE) &dwRefCount, &cbData);

			if(dwRefCount == 1)
			{
				RegDeleteValue(m_hSharedDllKey, m_szWABDllPath);


			}
			else
			{
				dwRefCount--;

				RegSetValueEx(m_hSharedDllKey, m_szWABDllPath, 0, REG_DWORD, (const PBYTE) &dwRefCount, sizeof(DWORD));
			}

			RegCloseKey(m_hSharedDllKey);
		}
	}

	return bReturn;
}

bool OEImportClass::ProcessABook()
{
	HRESULT			hresult;
	LPMAPITABLE		lpContTable;
	LPSPropTagArray	lpPropTagArray;
	LPSRowSet		lpRowSet;
	LPABCONT		lpABC = NULL;
	SizedSPropTagArray	(2, AbookProps) =
	{
		2,
		{
			PR_DISPLAY_TYPE,
			PR_ENTRYID
		}
	};

	LPENTRYID	EntryIDValue = NULL;
	ULONG		EntryIDSize = 0;
	ULONG		EntryObjectType = 0;
	ULONG		i;

	ULONG		ABType;
	LPMAPIPROP	ABEntry;

	hresult = m_lpAdrBook->GetPAB( &EntryIDSize, &EntryIDValue);


	//testing sangita
	
	/*SRowSet *rows;
	hresult = m_lpAdrBook->GetSearchPath(NULL, &rows);

	for (unsigned int j=0; j<rows->cRows; j++)
		string name(rows->aRow[j].lpProps[1].Value.lpszA);*/
	//end testing sangita
	
	if(hresult != ERROR_SUCCESS)
		return false;
	
	hresult = m_lpAdrBook->OpenEntry(EntryIDSize, (LPENTRYID) EntryIDValue, NULL, 0, &EntryObjectType, (LPUNKNOWN *) &lpABC);

	m_lpWABObject->FreeBuffer(EntryIDValue);

	if(hresult != ERROR_SUCCESS)
		return false;

	
	
	hresult = lpABC->GetContentsTable(NULL, &lpContTable);
	
	if(lpABC)
		lpABC->Release();

	if (HR_FAILED(hresult))
	{
		return false;
	}


	hresult = lpContTable->QueryColumns(NULL, &lpPropTagArray);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		return false;
	}

	hresult = lpContTable->SetColumns((LPSPropTagArray)&AbookProps, NULL);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_lpWABObject->FreeBuffer(lpPropTagArray);
		return false;
	}
	while (!HR_FAILED(hresult = lpContTable->QueryRows(1,0, &lpRowSet)))
	{
		if (lpRowSet->cRows != 0)
		{
			for (i = 0; i < lpRowSet->aRow[0].cValues; i++)
			{
				if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ENTRYID & 0xFFFF0000))
				{
					EntryIDValue = (LPENTRYID)(lpRowSet->aRow[0].lpProps[i].Value.bin.lpb);
					EntryIDSize	= (ULONG)(lpRowSet->aRow[0].lpProps[i].Value.bin.cb);
				}
				else if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_TYPE & 0xFFFF0000))
				{
					EntryObjectType = (ULONG)lpRowSet->aRow[0].lpProps[i].Value.ul;
				}
			}
			
			if ((EntryIDValue) && ( (EntryObjectType == DT_MAILUSER) || (EntryObjectType == MAPI_E_NOT_FOUND) ) )
			{
				hresult = m_lpAdrBook->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &ABType, (LPUNKNOWN*)&ABEntry);
				m_lpWABObject->FreeBuffer(EntryIDValue);
				if (HR_FAILED(hresult))
				{
					if (lpContTable)
					{
						lpContTable->Release();
						m_lpWABObject->FreeBuffer(lpContTable);
						lpContTable = NULL;
					}
					if (lpPropTagArray)
						m_lpWABObject->FreeBuffer(lpPropTagArray);

					FreeRowSet(m_lpWABObject, lpRowSet);
					return (false);
				}
				hresult = ProcessABEntry(ABEntry);
				ABEntry->Release();
				m_lpWABObject->FreeBuffer(&ABEntry);
				ABEntry = NULL;
			}
			
			FreeRowSet(m_lpWABObject, lpRowSet);
			if (lpPropTagArray)
				m_lpWABObject->FreeBuffer(lpPropTagArray);
			lpRowSet = NULL;

		}
		else 
		{
			FreeRowSet(m_lpWABObject, lpRowSet);
			lpRowSet = NULL;
			m_lpWABObject->FreeBuffer(lpPropTagArray);
			
			break;
		}
	}
	
	if (lpContTable)
	{
		lpContTable->Release();
	}

	return true;


}
bool OEImportClass::FreeRowSet(LPWABOBJECT lpWABObject, LPSRowSet lpRows)
{
    ULONG   n;

    if(lpRows)
    {
        for( n = 0 ; n < lpRows->cRows ; n++)
        {
            if( lpWABObject->FreeBuffer(lpRows->aRow[n].lpProps ) )
            {
                return false;
            }
        }
        if( lpWABObject->FreeBuffer(lpRows) )
        {
            return false;
        }
    }
    return true;
}

bool OEImportClass::ProcessABEntry(LPMAPIPROP ABEntry)
{
	HRESULT hresult;
	LPSPropTagArray	AllPropValues;

	CAbookEntry *Entry;
	Entry = DEBUG_NEW CAbookEntry;

	InitAddrEntry(Entry);

	ULONG			i;
	ULONG			NumProps;
	LPSPropValue	PropValues;


	hresult = ABEntry->GetPropList(NULL, &AllPropValues);

	hresult = ABEntry->GetProps(AllPropValues, NULL, &NumProps, &PropValues);

	m_lpWABObject->FreeBuffer(AllPropValues);

	// jm - 06/00 Added support for unicode address book.
	// This fixes bug #5821
	for (i = 0; i <= NumProps; i++)
	{
		switch(PropValues[i].ulPropTag)
		{

		case PR_COMMENT:
			{
				Entry->notes = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->notes, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COMMENT_W:
			{
				Entry->notes = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->notes, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}

		case PR_GIVEN_NAME:
			{
				Entry->first_name = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->first_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_GIVEN_NAME_W:
			{
				Entry->first_name = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->first_name, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );

				break;
			}

		case PR_BUSINESS_TELEPHONE_NUMBER:
			{
				Entry->wrk_phone = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_TELEPHONE_NUMBER_W:
			{
				Entry->wrk_phone = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_phone, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_TELEPHONE_NUMBER:
			{
				Entry->phone = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_TELEPHONE_NUMBER_W:
			{
				Entry->phone = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->phone, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_SURNAME:
			{
				Entry->last_name = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->last_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_SURNAME_W:
			{
				Entry->last_name = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->last_name, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_COMPANY_NAME:
			{
				Entry->company = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->company, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COMPANY_NAME_W:
			{
				Entry->company = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->company, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_TITLE:
			{
				Entry->wrk_title = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_title, PropValues[i].Value.lpszA);
				break;
			}
		case PR_TITLE_W:
			{
				Entry->wrk_title = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_title, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_DEPARTMENT_NAME:
			{
				Entry->wrk_dept = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_dept, PropValues[i].Value.lpszA);
				break;
			}
		case PR_DEPARTMENT_NAME_W:
			{
				Entry->wrk_dept = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_dept, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_OFFICE_LOCATION:
			{
				Entry->wrk_office = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_office, PropValues[i].Value.lpszA);
				break;
			}
		case PR_OFFICE_LOCATION_W:
			{
				Entry->wrk_office = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_office, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_MOBILE_TELEPHONE_NUMBER:
			{
				Entry->cell_phone = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->cell_phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_MOBILE_TELEPHONE_NUMBER_W:
			{
				Entry->cell_phone = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->cell_phone, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_PAGER_TELEPHONE_NUMBER:
			{
				Entry->wrk_pgr = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_pgr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_PAGER_TELEPHONE_NUMBER_W:
			{
				Entry->wrk_pgr = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_pgr, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_BUSINESS_FAX_NUMBER:
			{
				Entry->wrk_fax = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_fax, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_FAX_NUMBER_W:
			{
				Entry->wrk_fax = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_fax, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_FAX_NUMBER:
			{
				Entry->fax = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->fax, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_FAX_NUMBER_W:
			{
				Entry->fax = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->fax, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_COUNTRY:
			{
				Entry->wrk_country = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_country, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COUNTRY_W:
			{
				Entry->wrk_country = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_country, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_LOCALITY:
			{
				Entry->wrk_city = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_city, PropValues[i].Value.lpszA);
				break;
			}
		case PR_LOCALITY_W:
			{
				Entry->wrk_city = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_city, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_STATE_OR_PROVINCE:
			{
				Entry->wrk_state = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_state, PropValues[i].Value.lpszA);
				break;
			}
		case PR_STATE_OR_PROVINCE_W:
			{
				Entry->wrk_state = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_state, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_STREET_ADDRESS:
			{
				Entry->wrk_street_addr = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_street_addr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_STREET_ADDRESS_W:
			{
				Entry->wrk_street_addr = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_street_addr, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_POSTAL_CODE:
			{
				Entry->wrk_zip = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_zip, PropValues[i].Value.lpszA);
				break;
			}
		case PR_POSTAL_CODE_W:
			{
				Entry->wrk_zip = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_zip, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_MIDDLE_NAME:
			{
				Entry->middle_name = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->middle_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_MIDDLE_NAME_W:
			{
				Entry->middle_name = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->middle_name, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_NICKNAME:
			{
				Entry->nickname = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->nickname, PropValues[i].Value.lpszA);
				break;
			}
		case PR_NICKNAME_W:
			{
				Entry->nickname = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->nickname, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_PERSONAL_HOME_PAGE:
			{
				Entry->web_page = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->web_page, PropValues[i].Value.lpszA);
				break;
			}
		case PR_PERSONAL_HOME_PAGE_W:
			{
				Entry->web_page = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->web_page, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_BUSINESS_HOME_PAGE:
			{
				Entry->wrk_web_page = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_web_page, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_HOME_PAGE_W:
			{
				Entry->wrk_web_page = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->wrk_web_page, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_EMAIL_ADDRESS:
			{
				Entry->default_add = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->default_add, PropValues[i].Value.lpszA);

				break;
			}
		case PR_EMAIL_ADDRESS_W:
			{
				Entry->default_add = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->default_add, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_CONTACT_EMAIL_ADDRESSES:
			{
				ULONG	count;
				ULONG	totalSize = 0;
				char *curAddr;
				char * floater;
				ULONG	j;

				// First we're going to go thru and figure out how much space we need to allocate.
				count = PropValues[i].Value.MVszA.cValues;
				for (j = 0; j < count; j++)
				{
					totalSize += strlen(PropValues[i].Value.MVszA.lppszA[j]);
					totalSize += 1;	// for the comma
				}
				totalSize ++;		// For the Null character

				Entry->addresses = DEBUG_NEW_NOTHROW char[totalSize];
				if (Entry->addresses == NULL)	// If it's too big, just screw it.
					break;

				Entry->addresses[0] = 0;

				for (j = 0; j < count; j++)
				{
					curAddr = DEBUG_NEW char[strlen(PropValues[i].Value.MVszA.lppszA[j])+1];
					strcpy(curAddr, (PropValues[i].Value.MVszA.lppszA[j]));
					strcat(Entry->addresses, curAddr);
					strcat(Entry->addresses, (","));
					delete [] curAddr;
				}
 
				floater = strrchr(Entry->addresses, ',');

				*floater = 0;
				break;
			}
		case PR_CONTACT_EMAIL_ADDRESSES_W:
			{
				ULONG	count;
				ULONG	totalSize = 0;
				wchar_t *curAddr;
				char * floater;
				ULONG	j;
				char *tmpbuf;

				// First we're going to go thru and figure out how much space we need to allocate.
				count = PropValues[i].Value.MVszW.cValues;
				for (j = 0; j < count; j++)
				{
					totalSize += wcslen(PropValues[i].Value.MVszW.lppszW[j]);
					totalSize += 1;	// for the comma
				}
				totalSize ++;		// For the Null character

				Entry->addresses = DEBUG_NEW_NOTHROW char[totalSize];
				if (Entry->addresses == NULL)	// If it's too big, just screw it.
					break;

				Entry->addresses[0] = 0;

				for (j = 0; j < count; j++)
				{
					int tmplen = wcslen(PropValues[i].Value.MVszW.lppszW[j])+1;
					curAddr = DEBUG_NEW wchar_t[tmplen];
					tmpbuf = DEBUG_NEW char[tmplen];
					wcscpy(curAddr, (PropValues[i].Value.MVszW.lppszW[j]));
					
					WideCharToMultiByte( CP_ACP, 0, curAddr, -1,
									tmpbuf, 
									tmplen,NULL, NULL );
					
					strcat(Entry->addresses, tmpbuf);
					strcat(Entry->addresses, (","));
					delete [] curAddr;
					delete [] tmpbuf;
				}
 
				floater = strrchr(Entry->addresses, ',');

				*floater = 0;
				break;
			}
		case PR_HOME_ADDRESS_CITY:
			{
				Entry->city = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->city, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_CITY_W:
			{
				Entry->city = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->city, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_COUNTRY:
			{
				Entry->country = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->country, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_COUNTRY_W:
			{
				Entry->country = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->country, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_POSTAL_CODE:
			{
				Entry->zip = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->zip, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_POSTAL_CODE_W:
			{
				Entry->zip = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->zip, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
			{
				Entry->state = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->state, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_STATE_OR_PROVINCE_W:
			{
				Entry->state = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->state, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_STREET:
			{
				Entry->street_addr = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->street_addr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_STREET_W:
			{
				Entry->street_addr = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)Entry->street_addr, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		default:
			{
				break;
			}
		}
	}

	AddEntryToTree(Entry);
	delete Entry;

	return true;
}
bool OEImportClass::InitAddrEntry(CAbookEntry *pEntry)
{
	pEntry->pID					= NULL;
	pEntry->IDSize				= NULL;

	pEntry->first_name			= NULL;
	pEntry->middle_name			= NULL;
	pEntry->last_name			= NULL;
	pEntry->title				= NULL;
	pEntry->nickname			= NULL;
	pEntry->default_add			= NULL;
	pEntry->addresses			= NULL;
	pEntry->street_addr			= NULL;
	pEntry->city				= NULL;
	pEntry->state				= NULL;
	pEntry->zip					= NULL;
	pEntry->country				= NULL;
	pEntry->phone				= NULL;
	pEntry->fax					= NULL;
	pEntry->cell_phone			= NULL;
	pEntry->web_page			= NULL;
	pEntry->company				= NULL;
	pEntry->wrk_street_addr		= NULL;
	pEntry->wrk_city			= NULL;
	pEntry->wrk_state			= NULL;
	pEntry->wrk_zip				= NULL;
	pEntry->wrk_country			= NULL;
	pEntry->wrk_title			= NULL;
	pEntry->wrk_dept			= NULL;
	pEntry->wrk_office			= NULL;
	pEntry->wrk_phone			= NULL;
	pEntry->wrk_fax				= NULL;
	pEntry->wrk_pgr				= NULL;
	pEntry->wrk_ip_phone		= NULL;
	pEntry->wrk_web_page		= NULL;
	pEntry->spouse				= NULL;
	pEntry->children			= NULL;
	pEntry->gender				= NULL;
	pEntry->birthday			= NULL;
	pEntry->anniversary			= NULL;
	pEntry->notes				= NULL;
	pEntry->conf_server			= NULL;
	pEntry->def_conf_add		= NULL;
	pEntry->conf_addresses		= NULL;

	pEntry->type				= 0;
	pEntry->m_lImportID			= 0;
	pEntry->pNext				= NULL;
	return true;
}
bool OEImportClass::DeleteAddrEntry(CAbookEntry *pEntry)
{
	delete [] pEntry->first_name;
	delete [] pEntry->middle_name;
	delete [] pEntry->last_name;
	delete [] pEntry->title;
	delete [] pEntry->nickname;
	delete [] pEntry->default_add;
	delete [] pEntry->addresses;
	delete [] pEntry->street_addr;
	delete [] pEntry->city;
	delete [] pEntry->state;
	delete [] pEntry->zip;
	delete [] pEntry->country;
	delete [] pEntry->phone;
	delete [] pEntry->fax;
	delete [] pEntry->cell_phone;
	delete [] pEntry->web_page;
	delete [] pEntry->company;
	delete [] pEntry->wrk_street_addr;
	delete [] pEntry->wrk_city;
	delete [] pEntry->wrk_state;
	delete [] pEntry->wrk_zip;
	delete [] pEntry->wrk_country;
	delete [] pEntry->wrk_title;
	delete [] pEntry->wrk_dept;
	delete [] pEntry->wrk_office;
	delete [] pEntry->wrk_phone;
	delete [] pEntry->wrk_fax;
	delete [] pEntry->wrk_pgr;
	delete [] pEntry->wrk_ip_phone;
	delete [] pEntry->wrk_web_page;
	delete [] pEntry->spouse;
	delete [] pEntry->children;
	delete [] pEntry->gender;
	delete [] pEntry->birthday;
	delete [] pEntry->anniversary;
	delete [] pEntry->notes;
	delete [] pEntry->conf_server;
	delete [] pEntry->def_conf_add;
	delete [] pEntry->conf_addresses;

	delete [] pEntry->pNext;
	return true;

}



bool OEImportClass::AddEntryToTree(CAbookEntry *pEntry)
{
	CAbookEntry *pWalker;

	pWalker = m_pABook;

	if(pWalker == NULL)
	{
		m_pABook = DEBUG_NEW CAbookEntry;
		pWalker = m_pABook;
		InitAddrEntry(pWalker);
		pWalker->pNext = NULL;
	}
	else
	{
		while (pWalker->pNext != NULL)
		{
			pWalker = pWalker->pNext;
		}

		pWalker->pNext = DEBUG_NEW CAbookEntry;
		pWalker = pWalker->pNext;
		InitAddrEntry(pWalker);
		pWalker->pNext = NULL;

	}
	*pWalker = *pEntry;

	return true;
}
bool OEImportClass::DeleteAddrTree(CAbookEntry **Tree)
{

	if (Tree == NULL)
		Tree = &m_pABook;

	if (*Tree == NULL)
		return true;
	
	
	CAbookEntry *trash = *Tree;
	if(trash->pNext)
		DeleteAddrTree(&(trash->pNext));


	DeleteAddrEntry(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

// Personality stuff
//implement later
CPersonalityTree* OEImportClass::GetPersonalityTree(char * lpszProfileName)
{

	return NULL;

}