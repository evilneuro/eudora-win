#include "stdafx.h"
#include "shlobj.h" //for Shell functions

#include <fstream>

#include "DebugNewHelpers.h"


//typedef basic_ifstream<char, char_traits<char> > ifstream;

/////////////////////////////////////////////////////////////////////////////
// NSImportClass construction

NSImportClass::NSImportClass()
{
	m_hModule = NULL;
	m_head = NULL;

	m_csBaseDir = NULL;
	m_szMainBoundary[0] = 0;
	m_szMessageBoundary[0] = 0;
	m_szTempDirName[0] = 0;
	
	m_fnpFolders = NULL;

	m_fpoDigestFile = NULL;
	m_fpiNSFile = NULL;
	m_tempfile = NULL;
	m_TempFilePathname = NULL;
	m_lpszDigestPath = NULL;
	
	m_NSImportProvider.bHasAddresses = 0;
	m_NSImportProvider.bHasLdif = 0;
	m_NSImportProvider.bHasMail = 0;
	m_NSImportProvider.iNumChildren = 0;
	m_NSImportProvider.lpszDisplayName = NULL;
	m_NSImportProvider.pFirstChild = NULL;
	m_NSImportProvider.pNext = NULL;

	m_pABook = NULL;
	m_pABookTree = NULL;
}

NSImportClass::~NSImportClass()
{
	CImportChild *pChild, *pChildtmp;
	CMbox_FnamePair *pFolder, *pFoldertmp;	

	if(m_csBaseDir)
		DeleteCharListTree(&m_csBaseDir);

	if(m_NSImportProvider.lpszDisplayName)
		delete [] m_NSImportProvider.lpszDisplayName;

	if(m_NSImportProvider.pFirstChild)
	{
		pChild = m_NSImportProvider.pFirstChild->pNext;
		DeleteChildNodeData(m_NSImportProvider.pFirstChild);
		
		delete m_NSImportProvider.pFirstChild;

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

		if(m_pMessageStores->lpszDisplayName)
			delete [] m_pMessageStores->lpszDisplayName;
		m_pMessageStores->pNext = NULL;
		m_pMessageStores->pDB = NULL;
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

	free (m_TempFilePathname);
	free (m_lpszDigestPath);

	//fix bcos of OL where we create a tree of address books
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
void NSImportClass::DeleteChildNodeData(CImportChild *pChild)
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
void NSImportClass::DeleteFolderNodeData(CMbox_FnamePair *pFolder)
{
	if(pFolder->lpszDisplayName)
		delete [] pFolder->lpszDisplayName;
	
	if(pFolder->lpszFilename)
		delete [] pFolder->lpszFilename;

	if(pFolder->lpszMailbox)
		delete [] pFolder->lpszMailbox;

	return;
}

/////////////////////////////////////////////////////////////////////////////
// Called from external DllMain to provide a copy of the handle
void NSImportClass::LoadModuleHandle(HANDLE hModule)
{
	m_hModule = hModule;
}

/////////////////////////////////////////////////////////////////////////////
// Called from external DllMain to get the providers filled in
// Calls LoadNSAccounts to find primary NS keys in the registry
// Fills in the provider and Child linked list
void NSImportClass::LoadNSProvider()
{
	LoadNSAccounts();
	return;
}


bool NSImportClass::LoadNSAccounts()
{
	

	char tempbuffer[128];

	// Changes were made to make sure we support NS versions 4.*. The reason why sometimes
	// the import failed before these changes was because the path to prefs.js file was not found,
	// which NS had changed it since 4.0*. Also see NSPrefs.* for some more information.


	TCHAR		szPrefsFileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
	CCharArrList * csPrefsFileList = DEBUG_NEW CCharArrList;	
	csPrefsFileList->lpszItem = NULL;
	csPrefsFileList->pNext = NULL;

	LocateNetscapePrefsFile(csPrefsFileList);
	CCharArrList *pWalker  = csPrefsFileList;
	
	AddProvider("Netscape Messenger");

	while (pWalker)
	{
		if(pWalker->lpszItem)
		{
			char strName[_MAX_PATH];
			strcpy(strName, pWalker->lpszItem);
			strcpy(szPrefsFileName,strName);
		
			strcpy(tempbuffer,szPrefsFileName);
			char *szUserString = strrchr(tempbuffer,'\\');
			if (szUserString)
			{
				*szUserString = '\0';
				szUserString = strrchr(tempbuffer,'\\');
				if (szUserString)
				{
					//NS 6 doesn't precede the prefs file dirname with actual persona
					// They have a blahbla.slt
					if(strstr(szUserString, ".slt"))
					{
						szUserString[0] = 0; // terminate this bogus name 23808.slt
						szUserString = strrchr(tempbuffer , '\\');
						if(szUserString)
							strcpy(tempbuffer, szUserString + 1);

					}
					else
						strcpy(tempbuffer,szUserString + 1);
				}
			}
		
			AddChildNS(szPrefsFileName, tempbuffer);
		}
		pWalker = pWalker->pNext;
	}

	DeleteCharListTree(&csPrefsFileList);
	return true;
	
}

int NSImportClass::AddChildNS(char* szPrefsFilePath, char *PersName)
{
	char * floater;
	char tempBuf[512];
	CImportChild *pCurrentChild;
	char szData[512];
	char prefsFileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT];

	strcpy(szData,szPrefsFilePath);
	char *szUserString = strrchr(szData,'\\');
	if (szUserString)
	{
		*szUserString = '\0';
	}

	strcpy(prefsFileName,szPrefsFilePath);

	std::ifstream prefsFile(prefsFileName, std::ios_base::in);
	if ( !prefsFile )
	// The file does not exist ...
	{
		return false;
	}

	pCurrentChild = CreateChildNode();

	if (pCurrentChild->lpszAccountDisplayName)
		delete [] pCurrentChild->lpszAccountDisplayName;

	pCurrentChild->lpszAccountDisplayName= DEBUG_NEW char[strlen(PersName)+1];
	strcpy(pCurrentChild->lpszAccountDisplayName,PersName);

	if (pCurrentChild->lpszIdentityName)
		delete [] pCurrentChild->lpszIdentityName;

	pCurrentChild->lpszIdentityName= DEBUG_NEW char[strlen(PersName)+1];
	strcpy(pCurrentChild->lpszIdentityName,PersName);

	if (pCurrentChild->lpszPersonalityName)
		delete [] pCurrentChild->lpszPersonalityName;

	pCurrentChild->lpszPersonalityName = DEBUG_NEW char[strlen(PersName)+1];
	strcpy(pCurrentChild->lpszPersonalityName,PersName);

	if (pCurrentChild->lpszPathToMailData)
		delete [] pCurrentChild->lpszPathToMailData;

	pCurrentChild->lpszPathToMailData = DEBUG_NEW char[strlen(szData)+1];
	strcpy(pCurrentChild->lpszPathToMailData, szData);

	while (prefsFile.getline(tempBuf, 510))
	{
		if (strstr(tempBuf, ("user_pref(\"mail.")))
		{
			if (strstr(tempBuf, ("user_pref(\"mail.identity.useremail")) 
				|| strstr(tempBuf, ("user_pref(\"mail.identity.id1.useremail")))
			{
				NSExtractValue(&(pCurrentChild->lpszEmailAddress), tempBuf);
			}
			else if (strstr(tempBuf, ("user_pref(\"mail.identity.username"))
				|| strstr(tempBuf, ("user_pref(\"mail.identity.id1.fullName")))
			{
				NSExtractValue(&(pCurrentChild->lpszRealName), tempBuf);
			}
			else if (strstr(tempBuf, ("user_pref(\"mail.leave_on_server"))
				|| strstr(tempBuf, ("user_pref(\"mail.server.server1.leave_on_server")))
			{
				floater = strrchr(tempBuf, 0x2c);  //2C is a comma
				floater += 2;			// was three, should be 2 to catch 't' in true -jdboyd
				if (*floater == 't')
				{
					pCurrentChild->bLMOS = true;
				}					
			}
			else if (strstr(tempBuf, ("user_pref(\"mail.pop_name"))
				|| strstr(tempBuf, ("user_pref(\"mail.server.server1.userName")))
			{
				NSExtractValue(&(pCurrentChild->lpszIncomingUserName), tempBuf);
			}
			else if (strstr(tempBuf, ("user_pref(\"mail.server_type"))
				|| strstr(tempBuf, ("user_pref(\"mail.server.server1.type")))	// server type -jdboyd
			{
				floater = strrchr(tempBuf, 0x2c);  //2C is a comma
				floater += 2;
				if (*floater == '1')
				{
					pCurrentChild->bIsIMAP = true;
				}					
			} // NS 6 below
			else if (strstr(tempBuf, ("user_pref(\"mail.server.server1.hostname")))
			{
				NSExtractValue(&(pCurrentChild->lpszIncomingServer), tempBuf);
			}
			else if (strstr(tempBuf, ("user_pref(\"mail.smtpserver.smtp1.hostname")))
			{
				NSExtractValue(&(pCurrentChild->lpszSMTPServer), tempBuf);	
			}// end NS 6
		}//NS below 6 below
		else if(strstr(tempBuf, ("user_pref(\"network.hosts.")))
		{
			if (strstr(tempBuf, ("user_pref(\"network.hosts.pop_server")))
			{
				NSExtractValue(&(pCurrentChild->lpszIncomingServer), tempBuf);
			}
			else if (strstr(tempBuf, ("user_pref(\"network.hosts.smtp_server")))
			{
				NSExtractValue(&(pCurrentChild->lpszSMTPServer), tempBuf);	
			}
		} // end NS below 6 
	}
	prefsFile.close();
	
	// For now Netscape cannot import addresses, Oh well, let's make sure it's never an option.
	// Instead it i,ports from an LDIF file.
	pCurrentChild->pParent->bHasAddresses = false;
	pCurrentChild->pParent->bHasLdif = true;

	//what are the things we are importing - for now mail
	pCurrentChild->kImportType = MBOX;

	// Here we figure out if you have any ldif files we can use.
	struct _finddata_t c_file;
	long m_hFile;

	if (_chdir(pCurrentChild->lpszPathToMailData))
	{
		pCurrentChild->lpszPathtoLdif = NULL;
	}
	else
	{
		if( (m_hFile = _findfirst( "*.ldif", &c_file )) == -1L )
		{
			pCurrentChild->lpszPathtoLdif = NULL;
		}
		else
		{
			pCurrentChild->lpszPathtoLdif = DEBUG_NEW char[strlen(pCurrentChild->lpszPathToAdrData) + strlen(c_file.name) +2];
			strcpy(pCurrentChild->lpszPathtoLdif, pCurrentChild->lpszPathToMailData);
			strcat(pCurrentChild->lpszPathtoLdif, "\\");
			strcat(pCurrentChild->lpszPathtoLdif, c_file.name);
		}
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////////
// Creates and initializes a Child Node gives it to caller
// Called from AddChildNS
CImportChild* NSImportClass::CreateChildNode()
{

	CImportChild	*pCurrentChild;
	int iChildID = 0;

	m_NSImportProvider.iNumChildren++;
	
	pCurrentChild = m_NSImportProvider.pFirstChild;

	if (pCurrentChild == NULL)
	{
		m_NSImportProvider.pFirstChild = DEBUG_NEW CImportChild;
		pCurrentChild = m_NSImportProvider.pFirstChild;
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
	pCurrentChild->pParent = &m_NSImportProvider;
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


bool NSImportClass::NSExtractValue(char ** Dest, char * Line)
{
	char * floater;

	if (*Dest)
	{
		delete [] *Dest;
		*Dest = NULL;
	}
	
	floater = strrchr(Line, 0x22);		// 22 is a quote character
	*floater = 0;
	floater = strrchr(Line, 0x2c);		// 2C is a comma
	floater += 3;
	*Dest = DEBUG_NEW char[strlen(floater)+1];
	strcpy(*Dest, floater);

	return true;
}


// Function to Update the Netscape's Prefs.js file
// BOOL bAddEudoraAsDefaultEmailer : TRUE - add the 2 lines in prefs.js to configure Eudora as the Default Mailto handler
//                                 : FALSE - remove the 2 lines in prefs.js which configured Eudora as the Default Mailto handler
//
// Function to locate the prefs.js file on a user's machine
// This uses 2 ways to locate the prefs.js file & the second logic needs to be revisted
// for better performance. 

BOOL NSImportClass::LocateNetscapePrefsFile(CCharArrList *FileList)
{
	HKEY		hKey;
	DWORD		dwIndex=0;
	
	char		szTempBuffer[512];
	DWORD		dwTempBufSize = 512;

	HKEY		hChildKey;
	FILETIME	fileTime;

	char		szData[1024];
	DWORD		dwDataBufSize=1024;
	char		szPrefsFileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT];

	DWORD		dwKeyDataType;
	static		BOOL	bDoItOnce = FALSE;

	static	BOOL	bRet = FALSE;
	BOOL		bFoundPrefsPathInUserRegKey = FALSE;
	static bool bFirst = true;

	static	CCharArrList * csPrefsFileList = DEBUG_NEW CCharArrList;
	if(bFirst)
	{
		csPrefsFileList->lpszItem = NULL;
		csPrefsFileList->pNext = NULL;
		bFirst = false;

	}
	CCharArrList * pList1 = csPrefsFileList;
	if (bDoItOnce == FALSE)
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			("Software\\NetScape\\Netscape Navigator\\Users"),
			0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{	
			
			while (RegEnumKeyEx(hKey, dwIndex++, szTempBuffer, &dwTempBufSize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hKey, szTempBuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (RegQueryValueEx(hChildKey, ("DirRoot"), NULL, &dwKeyDataType,
						(LPBYTE) szData, &dwDataBufSize) != ERROR_SUCCESS)
					{

						continue;
					}
					
					// Open File
					strcpy(szPrefsFileName, szData);
					strcat(szPrefsFileName, ("\\prefs.js"));

					pList1->lpszItem = DEBUG_NEW char[strlen(szPrefsFileName) + 1];
					strcpy(pList1->lpszItem, szPrefsFileName);
					pList1->pNext = DEBUG_NEW CCharArrList;
					pList1 = pList1->pNext;
					pList1->pNext = NULL;

					bFoundPrefsPathInUserRegKey = TRUE;

					bRet = TRUE;
					
				}
				dwTempBufSize = 512;				
				RegCloseKey(hChildKey);
			}
			RegCloseKey(hKey);
		}

		if (FALSE == bFoundPrefsPathInUserRegKey)
		{
			// The reason we have to search for the file is because of the registry entries made by 4.3 & above are differnt 
			// than that of 4.0* - 02/14/2000

			dwIndex = 0;

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				("Software\\NetScape\\Netscape Navigator"),
				0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{	
				while ( (bDoItOnce == FALSE) && (RegEnumKeyEx(hKey, dwIndex++, szTempBuffer, &dwTempBufSize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS) )
				{
					strcat(szTempBuffer,("\\Main"));
					if(RegOpenKeyEx(hKey, szTempBuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
					{
						if (RegQueryValueEx(hChildKey, ("Install Directory"), NULL, &dwKeyDataType,
							(LPBYTE) szData, &dwDataBufSize) != ERROR_SUCCESS)
						{
							continue;
						}

						char *szTemp = strstr(_strlwr(szData),"\\netscape");

						if (szTemp != NULL)
						{
							char *szSlash = NULL;
							szSlash = strchr(szTemp+1,'\\');
							if (szSlash)								
								szData[szSlash - szData] = '\0';							
							else
								szData[szTemp - szData + strlen(szTemp) + 1] = '\0';			
							strcat(szData,"\\Users");
										
						}
						else if (NULL != (szTemp = strstr(_strlwr(szData),"\\communicator")) )
						{
							szData[szTemp - szData] = '\0';
							strcat(szData,"\\Users");

						}

						// Save the current dir so that it can be restored later.
						if (szTemp != NULL && bDoItOnce == FALSE)
						{
							char oldDir[1024]; 							
							GetCurrentDirectory(1023, oldDir); 
						
							if (TRUE == SetCurrentDirectory(szData))
							{
								CCharArrList * pWalker = m_csBaseDir;
								if(pWalker)
								{
									while(pWalker->pNext)
									{
										pWalker = pWalker->pNext;

									}

									pWalker->pNext = DEBUG_NEW CCharArrList;
									pWalker = pWalker->pNext;
									pWalker->lpszItem = NULL;
									pWalker->pNext = NULL;

								}
								else
								{
									pWalker = DEBUG_NEW CCharArrList;
									m_csBaseDir = pWalker;
									pWalker->lpszItem = NULL;
									pWalker->pNext = NULL;
								}

								pWalker->lpszItem = DEBUG_NEW char[MAX_PATH];
								strcpy(pWalker->lpszItem, szData);
								strcat(pWalker->lpszItem, "\\");
								CCharArrList *	csFilesList = DEBUG_NEW CCharArrList;
								csFilesList->lpszItem = NULL;
								csFilesList->pNext = NULL;

								GetFilesInDirectory("prefs.js", TRUE, csFilesList);
								pWalker = csFilesList;
								while (pWalker)
								{
									char strName[_MAX_PATH];
									strcpy(strName,pWalker->lpszItem);
									AddCharListTail(csPrefsFileList, strName);
									pWalker = pWalker->pNext;

									bRet = TRUE;
								}
								DeleteCharListTree(&csFilesList);

								SetCurrentDirectory(oldDir);
							}
							else
							{
								szData[0] = 'c';
								if (TRUE == SetCurrentDirectory(szData))
								{
									CCharArrList * pWalker = m_csBaseDir;
									if(pWalker)
									{

										while(pWalker->pNext)
										{
											pWalker = pWalker->pNext;

										}

										pWalker->pNext = DEBUG_NEW CCharArrList;
										pWalker = pWalker->pNext;
										pWalker->lpszItem = NULL;
										pWalker->pNext = NULL;

									}
									else
									{
										pWalker = DEBUG_NEW CCharArrList;
										m_csBaseDir = pWalker;
										pWalker->lpszItem = NULL;
										pWalker->pNext = NULL;
									}

									pWalker->lpszItem = DEBUG_NEW char[MAX_PATH];
									strcpy(pWalker->lpszItem, szData);
									strcat(pWalker->lpszItem, "\\");
									CCharArrList *	csFilesList = DEBUG_NEW CCharArrList;
									csFilesList->lpszItem = NULL;
									csFilesList->pNext = NULL;
	
									GetFilesInDirectory("prefs.js", TRUE, csFilesList);
									*pWalker;
									pWalker = csFilesList;
									while (pWalker)
									{
										char strName[_MAX_PATH];
										strcpy(strName,pWalker->lpszItem);
										AddCharListTail(csPrefsFileList, strName);
										pWalker = pWalker->pNext;

										bRet = TRUE;
									}
									DeleteCharListTree(&csFilesList);

									SetCurrentDirectory(oldDir);
								}

							}
						}
						RegCloseKey(hChildKey);
					}
					dwTempBufSize = 512;
				}
				RegCloseKey(hKey);
			}


			// This loop is for Netscape 6 compatibility. The prior loop was for beta
			// When this is verified it should be removed.
			// Get our Application Data Directory
			// Some Wn95 Shells return NULL
			// NS6 likes to put it in the windows directory if that happens
			// Looks liek this kinda:
			// c:\CSIDL_APPDATA\Mozilla\Users50\{Profile}\????????.slt\Prefs.js
			IMalloc* pMalloc;
			HRESULT hr = SHGetMalloc(&pMalloc);
			if (FAILED(hr))
				return bRet;
   
			// The one and only Favorites Location
			LPITEMIDLIST pidlAppData = NULL;

			hr = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlAppData);
			if (FAILED(hr))
				return bRet;
			char szBuildNSPath[MAX_PATH];
			szBuildNSPath[0] = '\0';

			SHGetPathFromIDList(pidlAppData, szBuildNSPath);

			// Done with you all
			pMalloc->Free(pidlAppData);
			if(pMalloc)
				pMalloc->Release();

			if(strlen(szBuildNSPath) == 0)
				::GetWindowsDirectory(szBuildNSPath, MAX_PATH);

			// we have:  c:\CSIDL_APPDATA\
			// Build the rest
			if(szBuildNSPath[strlen(szBuildNSPath) - 1] != '\\')
				strcat(szBuildNSPath, "\\");

			strcat(szBuildNSPath, "Mozilla\\Users50");

			// we have:  c:\CSIDL_APPDATA\Mozilla\Users50
			char szSaveNSRootPath[MAX_PATH];

			strcpy(szSaveNSRootPath, szBuildNSPath);
	
			CCharArrList * pBaseWalker = m_csBaseDir;
			int iElement = 0;
			if(pBaseWalker )
			{
				iElement++;
				while(pBaseWalker->pNext)
				{
					pBaseWalker  = pBaseWalker->pNext;
					iElement++;

				}

				pBaseWalker->pNext = DEBUG_NEW CCharArrList;
				pBaseWalker = pBaseWalker->pNext;
				pBaseWalker->lpszItem = NULL;
				pBaseWalker->pNext = NULL;

			}
			else
			{
				pBaseWalker = DEBUG_NEW CCharArrList;
				m_csBaseDir = pBaseWalker;
				pBaseWalker->lpszItem = NULL;
				pBaseWalker->pNext = NULL;
			}

			pBaseWalker->lpszItem = DEBUG_NEW char[MAX_PATH];
			strcpy(pBaseWalker->lpszItem, szSaveNSRootPath);
			strcat(pBaseWalker->lpszItem, "\\");

			// Save the current dir so that it can be restored later.
			if (bDoItOnce == FALSE)
			{
				char oldDir[1024]; 							
				GetCurrentDirectory(1023, oldDir); 
			
				if (TRUE == SetCurrentDirectory(szSaveNSRootPath))
				{
					CCharArrList *	csFilesList = DEBUG_NEW CCharArrList;
					csFilesList->lpszItem = NULL;
					csFilesList->pNext = NULL;

					GetFilesInDirectory("prefs.js", TRUE, csFilesList, iElement);
					CCharArrList *pWalker;
					pWalker = csFilesList;
					while (pWalker)
					{
						char strName[_MAX_PATH];
						strcpy(strName,pWalker->lpszItem);
						AddCharListTail(csPrefsFileList, strName);

						bRet = TRUE;
						pWalker = pWalker->pNext;
						
					}
					DeleteCharListTree(&csFilesList);
					
					SetCurrentDirectory(oldDir);
				}
			}


		}

		bDoItOnce = TRUE;
	}


	char strName[_MAX_PATH];
	CCharArrList *pListWalker = csPrefsFileList;

	DeleteCharListTree(&FileList);
	FileList = NULL;
	FileList = DEBUG_NEW CCharArrList;

	FileList->lpszItem = NULL;
	FileList->pNext = NULL;

	while (pListWalker)
	{
		if(pListWalker->lpszItem)
		{
			strcpy(strName, pListWalker->lpszItem);
			AddCharListTail(FileList, strName);
		}
		pListWalker = pListWalker->pNext;
		
	}
	
	DeleteCharListTree(&csPrefsFileList);
	csPrefsFileList = NULL;

	return bRet;
}

// Function to search for a particular file in the current directory
// Altho' the main idea is from OT Toolkit's GetDirectory(), there function does not
// work completely correct (or as expected) & hence the modification.
void * NSImportClass::GetFilesInDirectory(const char * SearchString, const BOOL bRecurseSubDirs /* = FALSE */, CCharArrList *pStringList /* = NULL */, int iBaseDirElement )
{
	char tmpBaseDir[MAX_PATH];
	// Get the appropriate base directory based onw aht is passed in
	CCharArrList *pBaseWalk = m_csBaseDir;

	if(!pBaseWalk)
		return NULL;

	for(int i = 0; i < iBaseDirElement; i++)
	{
		pBaseWalk = pBaseWalk->pNext;

	}
	strcpy(tmpBaseDir, pBaseWalk->lpszItem);

	// If they don't pass in a list, create one.
	if (pStringList == NULL)
	{
		pStringList = DEBUG_NEW CCharArrList;
		pStringList->pNext = NULL;
		pStringList->lpszItem = NULL;
	} // if

	CCharArrList *pFileList = DEBUG_NEW CCharArrList;
	CCharArrList *pWalker = NULL;
	CCharArrList *pWalker2 = NULL;
	
	pFileList->pNext = NULL;
	pFileList->lpszItem = NULL;
	pWalker = pFileList;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	BOOL bWorking; 
	
	hFind = FindFirstFile(SearchString, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
		bWorking = FALSE;
	else
	{
		bWorking = TRUE;
		pWalker->lpszItem = DEBUG_NEW char [strlen(SearchString) + strlen(tmpBaseDir) + 1];
		strcpy(pWalker->lpszItem, tmpBaseDir);
		strcat(pWalker->lpszItem, SearchString);
	}

	while (bWorking)
	{
		bWorking = FindNextFile(hFind, &FindFileData);	
		if(bWorking)
		{
			pWalker->pNext = DEBUG_NEW CCharArrList;
			pWalker = pWalker->pNext;
			pWalker->pNext = NULL;

			pWalker->lpszItem = DEBUG_NEW char [strlen(FindFileData.cFileName) + 1];
			strcpy(pWalker->lpszItem, FindFileData.cFileName);
		}
	}

	FindClose(hFind);


	if(pFileList->lpszItem != NULL)
		AddCharListTail(pStringList, pFileList);
	// delete pFileList;

	DeleteCharListTree(&pFileList);
	pFileList = NULL;

	if (bRecurseSubDirs)
	{
		char FileName[_MAX_PATH];
		strcpy(FileName, SearchString); // Copy to modify;

		char CurDir[_MAX_PATH];

		strcpy(CurDir, FileName);


		char csDirName[_MAX_PATH];
		csDirName[0] = 0;

		// Find the last "\" in the string and return everything up to and including it.
		char *floater = strrchr(CurDir, '\\');
		if(floater)
			floater[1] = 0;
		else
			CurDir[0] = 0;

		CCharArrList *  pDirList = DEBUG_NEW CCharArrList;
		pDirList->lpszItem = NULL;
		pDirList->pNext = NULL;
		
		struct _finddata_t c_file;
		long hFile;
		char CurDirWildcard[_MAX_PATH];
		strcpy(CurDirWildcard, CurDir);
		strcat(CurDirWildcard, "*.*");

		if( (hFile = _findfirst( CurDirWildcard, &c_file )) != -1L )
		{
				if ( c_file.attrib & _A_SUBDIR ) 
				{
					OutputDebugString(c_file.name);
					OutputDebugString("\n");
					if ( (FALSE == strcmp(c_file.name,(".")) )  &&  
						 (FALSE == strcmp(c_file.name,("..")) ) )
					{
						strcpy(csDirName,CurDir);
						strcat(csDirName, c_file.name);
						AddCharListTail(pDirList, csDirName);
					}
					
				}
        
				while( _findnext( hFile, &c_file ) == 0 )
				{
					if ( c_file.attrib & _A_SUBDIR ) 
					{					
						OutputDebugString(c_file.name);
						OutputDebugString("\n");
						if ( ( !strcmp(c_file.name,(".")) ) ||
							 ( !strcmp(c_file.name,("..")) ) )
						{
							 continue;
						}
						else
						{
							strcpy(csDirName,CurDir);
							strcat(csDirName, c_file.name);
							AddCharListTail(pDirList, csDirName);
						}

					}
    
				}

		   _findclose( hFile );
		}

		pWalker = pDirList;

		// Go through the directories we just got and recurse through them too.
		while(pWalker != NULL && pWalker->lpszItem != NULL)
		{
			
		  char *String = DEBUG_NEW char [strlen(pWalker->lpszItem) + 1];
		  strcpy(String, pWalker->lpszItem);
		  // Get file name part of search path
		  strcpy(FileName, SearchString);
		  char *floater = strrchr(FileName, '\\');

		  if(floater)
		      floater++;
		 
		  char  SearchSpec[_MAX_PATH];
		  if(floater)
			  strcpy(SearchSpec, floater);
		  else
			  strcpy(SearchSpec, FileName);

		  char SendableSearchSpec[_MAX_PATH];
		  strcpy(SendableSearchSpec, String);
		  strcat(SendableSearchSpec, "\\");
		  strcat(SendableSearchSpec, SearchSpec);

		  // Do the recursion.
		  GetFilesInDirectory(SendableSearchSpec, bRecurseSubDirs, pStringList, iBaseDirElement);

		  pWalker = pWalker->pNext;
		  delete [] String;
		} // for

		DeleteCharListTree(&pDirList);
		pDirList = NULL;
	} // if

  return pStringList;
} // GetFilesInDirectory

bool NSImportClass::DeleteCharListTree(CCharArrList **Tree)
{

	if (Tree == NULL)
		return false; // Gimmee somethint o delete u bastard!

	if (*Tree == NULL)
		return true;
	
	
	CCharArrList *trash = *Tree;
	if(trash->pNext)
		DeleteCharListTree(&(trash->pNext));


	delete [] (*Tree)->lpszItem;
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

bool NSImportClass::AddCharListTail(CCharArrList *pTree, CCharArrList *pItem)
{
	CCharArrList *pWalker = pItem;
	CCharArrList *pWalker2 = NULL;

	if(pTree->pNext == NULL && pTree->lpszItem == NULL)
	{
		pTree->lpszItem = DEBUG_NEW char [strlen(pWalker->lpszItem) + 1];
		strcpy(pTree->lpszItem, pWalker->lpszItem);
		pWalker2 = pTree;
		pWalker = pWalker->pNext;
	}
	else
	{
		pWalker2 = pTree;
		while(pWalker2->pNext != NULL)
		{
			pWalker2 = pWalker2->pNext;
		}

	}
	while(pWalker != NULL)
	{
		pWalker2->pNext = DEBUG_NEW CCharArrList;
		pWalker2 = pWalker2->pNext;
		pWalker2->lpszItem = NULL;
		pWalker2->pNext = NULL;

		pWalker2->lpszItem = DEBUG_NEW char[strlen(pWalker->lpszItem) + 1];
		strcpy(pWalker2->lpszItem, pWalker->lpszItem);
		pWalker = pWalker->pNext;
	}
	return true;
}

bool NSImportClass::AddCharListTail(CCharArrList *pTree, char *pItem)
{
	CCharArrList *pWalker2 = NULL;

	if(pTree->pNext == NULL && pTree->lpszItem == NULL)
	{
		pTree->lpszItem = DEBUG_NEW char [strlen(pItem) + 1];
		strcpy(pTree->lpszItem, pItem);
		return true;

	}
	else
	{
		pWalker2 = pTree;
		while(pWalker2->pNext != NULL)
		{
			pWalker2 = pWalker2->pNext;
		}

	}
	pWalker2->pNext = DEBUG_NEW CCharArrList;
	pWalker2 = pWalker2->pNext;
	pWalker2->lpszItem = NULL;
	pWalker2->pNext = NULL;

	pWalker2->lpszItem = DEBUG_NEW char[strlen(pItem) + 1];
	strcpy(pWalker2->lpszItem, pItem);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// We Have our Provider to export from FindProvider to Main Eudora App
// Fill In values appropriately.

bool NSImportClass::AddProvider(char * lpszProviderName)
{
	if (!lpszProviderName)
		return false;
	
	m_NSImportProvider.lpszDisplayName = DEBUG_NEW char[strlen(lpszProviderName)+1];
	strcpy(m_NSImportProvider.lpszDisplayName, lpszProviderName);
	
	
	m_NSImportProvider.pFirstChild = NULL;
	m_NSImportProvider.iNumChildren = 0;
	m_NSImportProvider.bHasMail = 1;
	m_NSImportProvider.bHasAddresses = 1;

	return true;
}


CMessageStore * NSImportClass::LoadFolders(char *lpszFolderPath)
{

	int numofmsgs = 0;


	if (!lpszFolderPath)
	{
		return NULL;
	}

	//CODE REVIEW
	//shifted the code to after checking if the char* is null
	char *floater = strrchr(lpszFolderPath, '\\');
	if(floater)
		floater[1] = 0;

	char *MailDir;
	MailDir = DEBUG_NEW char[strlen(lpszFolderPath)+ _MAX_PATH];
	strcpy(MailDir, lpszFolderPath);
	strcpy(szBaseDir, MailDir);
	if(MailDir[strlen(MailDir) - 1] != '\\')
		strcat(MailDir, "\\");

	strcat(MailDir, ("Mail"));


	// <ick>
	ProcessMailboxes(MailDir, 0 );
	// </ick>

	delete [] MailDir;

	m_pMessageStores = DEBUG_NEW CMessageStore;
	InitMsgStoreTree(m_pMessageStores);
	m_pMessageStores->lpMBoxID = m_fnpFolders;
	m_pMessageStores->pDB = NULL;
	m_pMessageStores->bIsDefault = false;
	m_pMessageStores->pNext = NULL;

	return m_pMessageStores;

}

bool NSImportClass::InitMsgStoreTree(CMessageStore *pEntry)
{
	pEntry->lpszDisplayName     = NULL;
	pEntry->lpMBoxID			= NULL;
	pEntry->bIsDefault			= false;
	pEntry->pNext				= NULL;
	return true;
}

bool NSImportClass::ProcessMailboxes(char *pathname, int iParentID)
{
	// static ID for mailbox to keep track of heirarchy
	static int iID = 0;
	CMbox_FnamePair  **realnames = &m_fnpFolders;

	// Make it one.
	// zero and one are considered top level mailboxes as in...
	// Eudora\Netscape Messenger\MyIDOneBawx
	iID++;

	if ( _chdir(pathname))// OK, now go there.
	{
		return false;
	}


	struct Pathnames *files = NULL;
	struct Pathnames *folders = NULL;

	// Builds folder list this iteration and file list for this level ont he heirarchy
	// as in... Netscape\MyFolder etc. then Netscape\Inbox
	BuildFolderList(&folders);
	BuildFileList(&files, folders);



	struct Pathnames *currentfile = files;
	struct Pathnames *currentfolder = folders;


	// While we have files on this level in the heirarchy
	while(currentfile)
	{
		// Build or Fname pair accounting for parent ID
		if(*realnames)
		{
			while ((*realnames)->pNext)
				realnames = &((*realnames)->pNext);

			realnames = &((*realnames)->pNext);
		}
		
		*realnames = DEBUG_NEW CMbox_FnamePair;

		(*realnames)->folderType = REGULAR_MBOX;
		(*realnames)->bCreated = 0;
		(*realnames)->bHasSubfolders = false;
		(*realnames)->lMBoxID = iID;
		(*realnames)->lParentMBoxID = iParentID;
		(*realnames)->lpszDisplayName = DEBUG_NEW char[strlen(currentfile->pathname) + 1];
		strcpy((*realnames)->lpszDisplayName, currentfile->pathname);
		char *floater = strrchr((*realnames)->lpszDisplayName, '.');
		if(floater)
			floater[0] = 0;

		(*realnames)->lpszFilename = DEBUG_NEW char[strlen(currentfile->pathname)+strlen(pathname)+2];;
		strcpy((*realnames)->lpszFilename, pathname);
		strcat((*realnames)->lpszFilename, ("\\"));
		strcat((*realnames)->lpszFilename, currentfile->pathname);
		floater = strrchr((*realnames)->lpszFilename, '.');
		if(floater)
			floater[0] = 0;

		
		floater = (*realnames)->lpszFilename;
		floater += strlen(szBaseDir);

		strcpy((*realnames)->lpszFilename, floater);
		
		(*realnames)->lpszMailbox  = NULL;
		(*realnames)->pNext = NULL;

		if (EscapePressed(1))
			return false;
		char * filespath;

		filespath = DEBUG_NEW char[strlen(currentfile->pathname)+strlen(pathname)+2];
		strcpy(filespath,pathname);
		strcat(filespath, ("\\"));
		strcat(filespath, currentfile->pathname);
		floater = strrchr(filespath, '.');
		if (floater && 
			 ( (!strcmp(floater, (".snm"))) || 
			   (!strcmp(floater, (".msf"))) ) )

			*floater = 0;
			
		iID++;
		currentfile = currentfile->next;
		delete [] filespath;
	}
	// While we have folders under us at this heirarchical level
	while(currentfolder)
	{
		// If we didn't have files We are probably netscape six
		// Anyhow we don't want a tree incorrectly placed at the high level
		// Netscape\DontPutHere so we increment the ID
		if(files == NULL)
			iID++;

		if(*realnames)
		{
			while ((*realnames)->pNext)
				realnames = &((*realnames)->pNext);

			realnames = &((*realnames)->pNext);
		}
		// Build our FName Pair
		*realnames = DEBUG_NEW CMbox_FnamePair;

		(*realnames)->folderType = REGULAR_MBOX;
		(*realnames)->bCreated = 0;
		(*realnames)->bHasSubfolders = true;
		(*realnames)->lMBoxID = iID;
		(*realnames)->lParentMBoxID = iParentID;
		(*realnames)->lpszDisplayName = DEBUG_NEW char[strlen(currentfolder->pathname) + 1];
		strcpy((*realnames)->lpszDisplayName, currentfolder->pathname);
		char *floater = strrchr((*realnames)->lpszDisplayName, '.');
		if(floater)
			floater[0] = 0;

		(*realnames)->lpszFilename = DEBUG_NEW char[strlen(currentfolder->pathname)+strlen(pathname)+2];;
		strcpy((*realnames)->lpszFilename, pathname);
		strcat((*realnames)->lpszFilename, ("\\"));
		strcat((*realnames)->lpszFilename, currentfolder->pathname);
		floater = strrchr((*realnames)->lpszFilename, '.');
		if(floater)
			floater[0] = 0;

		floater = (*realnames)->lpszFilename;
		floater += strlen(szBaseDir);

		strcpy((*realnames)->lpszFilename, floater);
		
		(*realnames)->lpszMailbox  = NULL;
		(*realnames)->pNext = NULL;
		if (EscapePressed(1))
			return false;
		char *folderspath;
		folderspath = DEBUG_NEW char[strlen(currentfolder->pathname)+strlen(pathname)+2];
		strcpy(folderspath, pathname);
		strcat(folderspath, ("\\"));
		strcat(folderspath, currentfolder->pathname);

		// And recurse so we get the next level down etc.
		ProcessMailboxes(folderspath, iID);
		currentfolder = currentfolder->next;
		iID++;
		delete [] folderspath;
	}

	DeleteTree(folders);
	DeleteTree(files);
	return false;	// Why false? anyone know?
}


bool NSImportClass::BuildFolderList(struct Pathnames **files)
{
	struct Pathnames	**currentfile;
	struct _finddata_t	c_file;
	long hFile;

	if (*files != NULL)
		return false;		// Someone's already populated the list.
	
	currentfile = files;


	// sbd is static thru netscape versions
	if( (hFile = _findfirst( "*.sbd", &c_file )) == -1L )
	{
		_findclose( hFile);
		// However, if we don't find one that means two possibilities
		// A. We have a simple mailbox structure in Netscape // YEAY!
		// B. We have Netscape 6 // Boo
		// Netscape six does: PathToMail\Mail\account.name.com\Inbox
		// As opposed to PAthToMail\MAil\Inbox

		// Sooooo if we don't find Inbox.snm at this level we are netscape six and
		// will proceed as such. If we do we are prior to netscape six
		if( (hFile = _findfirst( "Inbox.snm", &c_file)) == -1L)
		{
			_findclose( hFile);

			hFile = _findfirst("*.*", &c_file);
			if(hFile == -1L)
				return false; // really really no mail folders here
			else
			{
				// Only want subdirs as they correspond with folders
				if(c_file.attrib & _A_SUBDIR)
				{
					// These directories are put there and don't have mail
					// So add anything but these
					if(strcmp(c_file.name, "Local Folders") != 0 
						&& strcmp(c_file.name, "server2") != 0 
						&& strcmp(c_file.name, ".") != 0 
						&& strcmp(c_file.name, "..") != 0 )
					{
						AddNode(currentfile, &c_file);
						currentfile = &((*currentfile)->next);
					}
					while( _findnext( hFile, &c_file ) == 0 )
					{
						if(c_file.attrib & _A_SUBDIR)
						{
							if(strcmp(c_file.name, "Local Folders") != 0 
								&& strcmp(c_file.name, "server2") != 0 
								&& strcmp(c_file.name, ".") != 0 
								&& strcmp(c_file.name, "..") != 0 )
							{
								AddNode(currentfile, &c_file);
								currentfile = &((*currentfile)->next);
							}
						}
					}
					_findclose(hFile);

				}

			}
		}
		else // before netscape six. Simple dir structure
			return false;	// No folders
	}
	else // some folders exist
	{            
		AddNode(currentfile, &c_file);
		currentfile = &((*currentfile)->next);
		while( _findnext( hFile, &c_file ) == 0 )
		{
			AddNode(currentfile, &c_file);
			currentfile = &((*currentfile)->next);
		}
		_findclose( hFile );
	}
	return true;
}
bool NSImportClass::AddNode(struct Pathnames **currentfile, 	struct _finddata_t *file)
{
	*currentfile = DEBUG_NEW struct Pathnames;
	if (*currentfile == NULL)
		return false;

	(*currentfile)->pathname = DEBUG_NEW char[strlen(file->name)+1];
	strcpy((*currentfile)->pathname, file->name);

	(*currentfile)->next = NULL;
	return true;
}
bool NSImportClass::BuildFileList(struct Pathnames **files, struct Pathnames *folders)
{
	struct Pathnames	**currentfile;
	struct _finddata_t	c_file;
	long hFile;

	if (*files != NULL)
		return false;		// Someone's already populated the list.
	
	currentfile = files;


	if( ( (hFile = _findfirst("*.snm", &c_file )) == -1L ) && 
		( (hFile = _findfirst("*.msf", &c_file )) == -1L ) )
		return false;	// No files

	else
	{            
		if (!existsInTree(c_file.name, folders))
		{
			AddNode(currentfile, &c_file);
			currentfile = &((*currentfile)->next);
		}
		while( _findnext( hFile, &c_file ) == 0 )
		{
			if (!existsInTree(c_file.name, folders))	// We only want to add a node if it doesn't have a folder associated with it.
			{
				AddNode(currentfile, &c_file);
				currentfile = &((*currentfile)->next);
			}
		}
		_findclose( hFile );
	}
	return true;
}
bool NSImportClass::existsInTree(char * name, struct Pathnames *folders)
{
	struct Pathnames *currentfolder = folders;
	while (currentfolder)
	{
		char * filename;
		char * foldername;
		char * floater= NULL;
		filename = DEBUG_NEW char[strlen(name)+1];
		foldername = DEBUG_NEW char[strlen(currentfolder->pathname)+1];
		strcpy(filename, name);
		
		floater = strrchr(filename, '.');
		if (floater)
			*floater = 0;

		strcpy(foldername, currentfolder->pathname);
		floater = strrchr(foldername, '.');
		if (floater)
			*floater = 0;

		if (!strcmp(foldername, filename))
		{
			delete [] filename;
			delete [] foldername;
			return true;
		}
		delete [] filename;
		delete [] foldername;
		currentfolder = currentfolder->next;
	}

	return false;
}

// To let user abort
// Called from MakeMimeWithFile
int NSImportClass::EscapePressed(int Repost /*= FALSE*/)
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

char * NSImportClass::MakeMimeWithFile(char *lpszFileName, void * pID, unsigned int iSize)
{
	// Open Fabulous Files
	m_fpoDigestFile = CreateDigestFile();
	if(!m_fpoDigestFile)
		return NULL;
	InitDigestFile();

	if (!OpenNSFile(lpszFileName))
		return NULL;
	

	OpenTempFile();

	if(GrabMessages())
	{
		CloseTempFile();
		WriteDigestEnd();

		CloseNSFile();

		DeleteTempFile();
	}
	else
	{
		WriteDigestEnd();
		CloseNSFile();

		DeleteTempFile();
		DeleteDigestFile();
		return NULL;
	}

	return m_lpszDigestPath;
}

// Called from MakeMimeWithFile to create Digest File
FILE * NSImportClass::CreateDigestFile()
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
bool NSImportClass::InitDigestFile()
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
bool NSImportClass::WriteToDigestFile()
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
bool NSImportClass::WriteDigestEnd()
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
bool NSImportClass::OpenTempFile()
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
bool NSImportClass::CloseTempFile()
{
	int err = fflush(m_tempfile);
	err = fclose(m_tempfile);
	return true;
}

// Deletes our Temp file member
// Called from a couple places like MakeMimeWithFile for cleanup
// and OELFtoCRLF to switch temp files
bool NSImportClass::DeleteTempFile()
{

	if (m_TempFilePathname)
	{
		DeleteFile(m_TempFilePathname);

	}
	return true;
}

bool NSImportClass::OpenNSFile(char * pathname)
{
	if (!pathname) 
		return false;
	
	m_fpiNSFile = fopen(pathname, "rb");


	if(m_fpiNSFile)
		return true;
	else
		return false;

	return false;
}

bool NSImportClass::CloseNSFile()
{

	fclose(m_fpiNSFile);
	return true;
}

bool NSImportClass::GrabMessages()
{
	int numlines =0;
	bool bFirst = true;
	char *lpszBuf;
	
	do
	{
		lpszBuf = GetLine(m_fpiNSFile);

		if (EscapePressed(1))
		{
			delete [] lpszBuf;
			return false;
		}
		if(strlen(lpszBuf) == 0 && bFirst)
		{
			delete [] lpszBuf;
			return false;
		}
		else
		if(strlen(lpszBuf) == 0)
		{
			delete [] lpszBuf;
			if(!bFirst)
			{
				fflush(m_tempfile);
				CloseTempFile();
				WriteToDigestFile();
				OpenTempFile();
			}
			return true;

		}

		if(strncmp(lpszBuf, ("From "), 5) != 0)
		{
			fwrite(lpszBuf, sizeof(char), strlen(lpszBuf), m_tempfile);
		}
		else
		{
			if(!bFirst)
			{
				fflush(m_tempfile);
				CloseTempFile();
				WriteToDigestFile();
				OpenTempFile();
			}
			else
				bFirst = false;
		}

		delete [] lpszBuf;
	}while(true);

	


	return false;
}
char * NSImportClass::GetLine(FILE* pFile)
{
	int			iCRLF		= 0;
	char *		readbuffer	= NULL;
	int			iNumRead	= 0;
	static int	iCurPos		= 0;
	bool		bFoundLine	= false;

	readbuffer = DEBUG_NEW char[4097];
	readbuffer[0] = 0;

	iCurPos = ftell(pFile);

	iNumRead = fread(readbuffer, sizeof(char), 4096, pFile);

	if(iNumRead == 0)
		return readbuffer;

	for(int i = 0; i < iNumRead; i++)
	{
		if(readbuffer[i] == '\n')
		{
			if(i + 1 > iNumRead)
				return readbuffer;
			else
			{
				bFoundLine = true;
				readbuffer[i+1] = 0;

				fseek(pFile, iCurPos + i + 1, SEEK_SET);
				iCurPos = ftell(pFile);
				return readbuffer;
			}
		}


	}

	if(!bFoundLine)
		return readbuffer;

	return NULL;

}
char * NSImportClass::GetLineNoCRLF(FILE* pFile)
{
	int			iCRLF		= 0;
	char *		readbuffer	= NULL;
	int			iNumRead	= 0;
	static int	iCurPos		= 0;
	bool		bFoundLine	= false;

	readbuffer = DEBUG_NEW char[4097];
	readbuffer[0] = 0;

	iCurPos = ftell(pFile);

	iNumRead = fread(readbuffer, sizeof(char), 4096, pFile);

	if(iNumRead == 0)
		return readbuffer;

	for(int i = 0; i < (iNumRead - 1); i++)
	{
		if(readbuffer[i] == '\n')
		{
			if(i + 1 > iNumRead)
				return readbuffer;
			else
			{
				bFoundLine = true;
				if(i == 0 || i == 1)
				{
					readbuffer[i+1] = 0;

					fseek(pFile, iCurPos + i + 1, SEEK_SET);
					iCurPos = ftell(pFile);
					return readbuffer;
				}
				else
				{
					if(readbuffer[i - 1] == '\r')
						readbuffer[i - 1] = 0;
					else
						readbuffer[i] = 0;

					fseek(pFile, iCurPos + i + 1, SEEK_SET);
					iCurPos = ftell(pFile);
					return readbuffer;

				}
			}
		}


	}

	if(!bFoundLine)
		return readbuffer;

	return NULL;

}

bool NSImportClass::DeleteDigestFile()
{
	if (m_lpszDigestPath)
	{
		DeleteFile(m_lpszDigestPath);

	}
	return true;
	
}
void NSImportClass::DeleteTree(Pathnames * node)
{
	if (!node)
		return;

	if (node->next)
		DeleteTree(node->next);

	delete [] node->pathname;
	delete node;
}

CAddressBookTree * NSImportClass::GetAddressTree(char *lpszPath)
{
	if(Convert(lpszPath))
		return m_pABookTree;
	else
		return NULL;

	return NULL;
}

bool NSImportClass::Convert(LPCSTR File)	
{
	FILE *pFile;
	CAbookEntry *Entry;
	bool result = false;

	pFile = fopen(File, "rb");
	if (pFile)
	{
		do
		{
			Entry = GetNextEntry(pFile);
			if (Entry)
			{
				AddEntryToTree(Entry);

				delete Entry;
			}
		}while (Entry);
		
		result = true;
	
		fclose(pFile);
	}

	//create the address book tree. Here we have just one address book
	//hence the address book linked list had just a single node.
	m_pABookTree = DEBUG_NEW CAddressBookTree;
	InitAddrTree(m_pABookTree);
	m_pABookTree->lpszDisplayName = DEBUG_NEW char[strlen("Netscape Messenger") + 1];
	strcpy(m_pABookTree->lpszDisplayName, "Netscape Messenger");
	m_pABookTree->pFirstChild = m_pABook;
	m_pABookTree->pNext = NULL;

	return result;
}

bool NSImportClass::InitAddrTree(CAddressBookTree *pEntry)
{
	pEntry->lpszDisplayName = NULL;
	pEntry->pFirstChild = NULL;
	pEntry->pFirstDLChild = NULL;
	pEntry->pNext = NULL;
	return true;
}


bool NSImportClass::InitAddrEntry(CAbookEntry *pEntry)
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
bool NSImportClass::DeleteAddrEntry(CAbookEntry *pEntry)
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



bool NSImportClass::AddEntryToTree(CAbookEntry *pEntry)
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
bool NSImportClass::DeleteAddrTree(CAbookEntry **Tree)
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
CAbookEntry *NSImportClass::GetNextEntry(FILE *theFile)
{
	char *line = NULL;
	bool stop = false;
	int i;
	CAbookEntry *Entry;
	Entry = DEBUG_NEW CAbookEntry;
	InitAddrEntry(Entry);
	bool done = false;
	char * objectclass = NULL;
	char *LastTag = NULL;
	char *Tag = NULL;

	// Get a line, if it's a blank line try again like 100 times, then return NULL
	// If it's a dn line then you're off to the races. 
	for(i=0; i < 100 && !stop; i++)
	{
		 line = GetLineNoCRLF(theFile);
		 if (line[0] != 0)
		 {
			LastTag = DEBUG_NEW char[strlen(("dn:"))+1];
			strcpy(LastTag, ("dn:"));
			stop = true;
		 }
		 delete [] line;
		 line = NULL;
	}
	if (i >= 100)
		return NULL;

	delete [] line;
	line = NULL;

	do
	{
		line = GetLineNoCRLF(theFile);
		char * floater;
		if (line[0] == ' ')	// Continuation from previous line
		{
			Tag = LastTag;
			floater = &line[1];
		}
		else
		{
			floater = strchr(line, ' ');
			if (!floater)
			{
				done = true;
				break;
			}
			else
				*floater = 0;

			Tag = DEBUG_NEW char[strlen(line)+1];
			strcpy(Tag, line);

			LastTag ? delete [] LastTag : NULL;
			LastTag = DEBUG_NEW char[strlen(line)+1];
			strcpy(LastTag, line);


			*floater = ' ';		// Set it back

			while (*floater == ' ')
				floater++;		// Set it to the first character of the string.
		}

		if(!strcmp(Tag, ("givenname:")))
		{
			if (Entry->first_name)
			{
				char *temp;
				temp = Entry->first_name;
				Entry->first_name = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->first_name, temp);
				strcat(Entry->first_name, floater);
				delete [] temp;
			}
			else
			{
				Entry->first_name = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->first_name, floater);
			}
		}
		else if (!strcmp(Tag, ("sn:")))
		{
			if (Entry->last_name)
			{
				char *temp;
				temp = Entry->last_name;
				Entry->last_name = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->last_name, temp);
				strcat(Entry->last_name, floater);
				delete [] temp;
			}
			else
			{
				Entry->last_name = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->last_name, floater);	
			}
		}
		else if (!strcmp(Tag, ("telephonenumber:")))
		{
			if (Entry->wrk_phone)
			{
				char *temp;
				temp = Entry->wrk_phone;
				Entry->wrk_phone = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_phone, temp);
				strcat(Entry->wrk_phone, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_phone = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_phone, floater);
			}
		}
		else if (!strcmp(Tag, ("locality:")))
		{
			if (Entry->wrk_city)
			{
				char *temp;
				temp = Entry->wrk_city;
				Entry->wrk_city = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_city, temp);
				strcat(Entry->wrk_city, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_city = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_city, floater);	
			}
		}
		else if (!strcmp(Tag, ("st:")))
		{
			if (Entry->wrk_state)
			{
				char *temp;
				temp = Entry->wrk_state;
				Entry->wrk_state = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_state, temp);
				strcat(Entry->wrk_state, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_state = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_state, floater);	
			}
		}
		else if (!strcmp(Tag, ("title:")))
		{
			if (Entry->wrk_title)
			{
				char *temp;
				temp = Entry->wrk_title;
				Entry->wrk_title = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_title, temp);
				strcat(Entry->wrk_title, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_title = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_title, floater);		
			}
		}
		else if (!strcmp(Tag, ("postOfficeBox:")))
		{
			if (Entry->wrk_street_addr)
			{
				char *temp = Entry->wrk_street_addr;
				Entry->wrk_street_addr = DEBUG_NEW char[strlen(temp)+strlen(floater)+3];
				strcpy(Entry->wrk_street_addr, floater);
				strcat(Entry->wrk_street_addr, ("\r\n"));
				strcat(Entry->wrk_street_addr, temp);
				delete [] temp;
			}
			else
			{
				Entry->wrk_street_addr = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_street_addr, floater);
			}
		}
		else if (!strcmp(Tag, ("streetaddress:")))
		{
			if (Entry->wrk_street_addr)
			{
				char *temp = Entry->wrk_street_addr;
				Entry->wrk_street_addr = DEBUG_NEW char[strlen(temp)+strlen(floater)+3];
				strcpy(Entry->wrk_street_addr, temp);
				strcat(Entry->wrk_street_addr, ("\r\n"));
				strcat(Entry->wrk_street_addr, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_street_addr = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_street_addr, floater);				
			}
		}

		else if (!strcmp(Tag, ("postalcode:")))
		{
			if (Entry->wrk_zip)
			{
				char *temp;
				temp = Entry->wrk_zip;
				Entry->wrk_zip = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_zip, temp);
				strcat(Entry->wrk_zip, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_zip = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_zip, floater);		
			}
		}
		else if (!strcmp(Tag, ("countryname:")))
		{
			if (Entry->wrk_country)
			{
				char *temp;
				temp = Entry->wrk_country;
				Entry->wrk_country = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_country, temp);
				strcat(Entry->wrk_country, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_country = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_country, floater);
			}
		}
		else if (!strcmp(Tag, ("facsimiletelephonenumber:")))
		{
			if (Entry->wrk_fax)
			{
				char *temp;
				temp = Entry->wrk_fax;
				Entry->wrk_fax = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_fax, temp);
				strcat(Entry->wrk_fax, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_fax = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->wrk_fax, floater);	
			}
		}
		else if (!strcmp(Tag, ("homephone:")))
		{
			if (Entry->phone)
			{
				char *temp;
				temp = Entry->phone;
				Entry->phone = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->phone, temp);
				strcat(Entry->phone, floater);
				delete [] temp;
			}
			else
			{
				Entry->phone = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->phone, floater);
			}
		}
		else if (!strcmp(Tag, ("o:")))
		{
			if (Entry->company)
			{
				char *temp;
				temp = Entry->company;
				Entry->company = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->company, temp);
				strcat(Entry->company, floater);
				delete [] temp;
			}
			else
			{
				Entry->company = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->company, floater);
			}
		}
		else if (!strcmp(Tag, ("xmozillanickname:")))
		{	
			if (Entry->nickname)
			{
				char *temp;
				temp = Entry->nickname;
				Entry->nickname = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->nickname, temp);
				strcat(Entry->nickname, floater);
				delete [] temp;
			}
			else
			{
				Entry->nickname = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->nickname, floater);	
			}
		}
		else if (!strcmp(Tag, ("objectclass:")))
		{
			if (objectclass)
			{
				char * temp = objectclass;
				objectclass = DEBUG_NEW char[strlen(temp)+strlen(floater)+2];
				strcpy(objectclass, temp);
				strcat(objectclass, (" "));
				strcat(objectclass, floater);
				delete [] temp;
			}
			else
			{
				objectclass = DEBUG_NEW char[strlen(floater)+1];
				strcpy(objectclass, floater);
			}
		}
		else if (!strcmp(Tag, ("mail:")))
		{
			Entry->default_add = DEBUG_NEW char[strlen(floater)+1];
			strcpy(Entry->default_add, floater);
		}
		else if (!strcmp(Tag, ("description:")))
		{
			if (Entry->notes)
			{
				char *temp;
				temp = Entry->notes;
				Entry->notes = DEBUG_NEW char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->notes, temp);
				strcat(Entry->notes, floater);
				delete [] temp;
			}
			else
			{
				Entry->notes = DEBUG_NEW char[strlen(floater)+1];
				strcpy(Entry->notes, floater);
			}
		}
		if (Tag != LastTag)		// If we've just reused LastTag then we shouldn't be deleting it.
			delete [] Tag;

		delete [] line;
	}while (!done);
	if (objectclass)
	{
		if (!strstr(objectclass, ("person")))
		{
			delete Entry;
			delete [] line;
			delete [] LastTag;
			delete [] objectclass;
			return GetNextEntry(theFile);	// try the next record then...
		}
	}

	// Now the entry is fully filled out.
	if (  ((Entry->first_name) ||(Entry->last_name)) && Entry->default_add)
	{
		delete [] line;
		delete [] LastTag;
		delete [] objectclass;
		return Entry;
	}
	else
	{
		delete Entry;
		delete [] line;
		delete [] LastTag;
		delete [] objectclass;
		return GetNextEntry(theFile);	// try the next record then...
	}
}

// Personality stuff
//implement later
CPersonalityTree* NSImportClass::GetPersonalityTree(char * lpszProfileName)
{
	return NULL;

}
