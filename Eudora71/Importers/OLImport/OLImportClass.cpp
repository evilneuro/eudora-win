#include "stdafx.h"
#include "base64.h"
#include <mapi.h>
#include <mspst.h>
#include <assert.h>
#include "OLImportClass.h"

#include "DebugNewHelpers.h"

//CODE REVIEW
//switched the using namespace line to the class file
//instead of the header file
using namespace std;

OLImportClass::OLImportClass():
							EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE("Eudora to Outlook Importer Temp Profile"), 
							EUDORA_TO_OUTLOOK_IMPORTER_TEMP_MESSAGE_STORE("Eudora to Outlook Importer Temp Message Store")
{
	m_hModule= false;
	m_pMessageStores = false;
	m_fnpFolders = NULL;
	m_bMapiLoggedOn  = false;
	m_bRootCreated = false;
	m_hMapiDLL = NULL;
	m_fpoDigestFile = NULL;
	m_szMainBoundary[0] = 0;
	m_szMessageBoundary[0] = 0;
	m_tempfile = NULL;
	m_TempFilePathname = NULL;
	m_lpszDigestPath = NULL;
	m_szTempDirName[0] = 0;
	m_pABook = NULL;
	m_pABookTree = NULL;
	m_bGotEids = false;
	m_bProfileKey = false;
	m_bIdentityKey = false;
	m_InboxEID = NULL;
	m_OutboxEID = NULL;
	m_SentEID = NULL;
	m_DeletedEID = NULL;
	m_CalendarEID = NULL;
	m_ContactsEID = NULL;
	m_JournalEID = NULL;
	m_NotesEID = NULL;
	m_TasksEID = NULL;
	
}

OLImportClass::~OLImportClass()
{

	//delete the tmp personality that was created
	//LPPROFADMIN lpProf = 0;
	//HRESULT hResult = m_MAPIAdminProfiles(0, &lpProf);
	//if(hResult == S_OK) 
	//	MAPIEnsureEudoraTmpProfileDeleted(lpProf);

	//deleted the list of providers that we created
	CImportChild *pChild, *pChildtmp;

	if(m_OLImportProvider.lpszDisplayName)
		delete [] m_OLImportProvider.lpszDisplayName;

	if(m_OLImportProvider.pFirstChild)
	{
		pChild = m_OLImportProvider.pFirstChild->pNext;
		DeleteChildNodeData(m_OLImportProvider.pFirstChild);
		
		delete m_OLImportProvider.pFirstChild;
		while(pChild)
		{
			pChildtmp = pChild->pNext;
			DeleteChildNodeData(pChild);
			delete pChild;
			pChild = pChildtmp;
		}
	}

	free(m_TempFilePathname);
	free(m_lpszDigestPath);

	//cleans the message stores, address book and personalities structures
	FreeFolderABPersonalityTree();

	//closes all MAPI message stores and MAPI data bases that were open
	MAPICloseDefaultStore();

	//restore the default mail client back to Eudora when the importing is done... :)
	AddToRegistry(HKEY_LOCAL_MACHINE, "Software\\Clients\\Mail", NULL, "Eudora");	// HLM\Software\Clients\Mail\Eudora



}


/////////////////////////////////////////////////////////////////////////////
// frees up the message store, address book and personality hierarchy
// called from the destructor
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::FreeFolderABPersonalityTree() 
{

	//deletes the message store structure
	DeleteMsgStoreTree(&m_pMessageStores);

	//deletes the address book structure
	DeleteAddrTree(&m_pABookTree);

	//deletes the personality structure
	DeletePersonalityTree(&m_pPersonalityTree);


}

/////////////////////////////////////////////////////////////////////////////
// deletes the message store structure
// linked list... hence this function just recurses to the last node
// and calls DeleteMsgStoreEntry to actually delete each of the nodes
// in the message store linked list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteMsgStoreTree(CMessageStore **Tree)
{

	if (Tree == NULL)
		Tree = &m_pMessageStores;

	if (*Tree == NULL)
		return true;
	
	
	CMessageStore *trash = *Tree;
	if(trash->pNext)
		DeleteMsgStoreTree(&(trash->pNext));


	DeleteMsgStoreEntry(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deleted each of the nodes in the message store linked list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteMsgStoreEntry(CMessageStore *pEntry)
{
	if(pEntry) {
		if(pEntry->lpszDisplayName)
			delete [] pEntry->lpszDisplayName;
		m_MAPIFreeBuffer(pEntry->pDB);
		pEntry->pNext = NULL;
		// since the message store contains a pointer to a 
		// list of folders... call the delete function of the mailbox folders 
		// to clean up the folder hierarchy again
		DeleteMBoxFoldersTree(&pEntry->lpMBoxID);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deletes the folder structure
// linked list... hence this function just recurses to the last node
// and calls DeleteFolderNodeData to actually delete each of the nodes
// in the folder linked list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteMBoxFoldersTree(CMbox_FnamePair **Tree)
{

	if (*Tree == NULL)
		return true;
		
	CMbox_FnamePair *trash = *Tree;
	if(trash->pNext)
		DeleteMBoxFoldersTree(&(trash->pNext));


	DeleteFolderNodeData(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deleted each of the nodes in the folder linked list
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::DeleteFolderNodeData(CMbox_FnamePair *pFolder)
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
// deleted each of the nodes in the profile linked list
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::DeleteChildNodeData(CImportChild *pChild)
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
// Called from external DllMain to provide a copy of the handle
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::LoadModuleHandle(HANDLE hModule)
{
	m_hModule = hModule;
}

/////////////////////////////////////////////////////////////////////////////
// called from main Eudora app to load the various profiles
// from Microsoft Outlook
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::LoadOLProvider(bool bDisplayErrDialog)
{
	FindMainOLKey(bDisplayErrDialog);

	return;
}

/////////////////////////////////////////////////////////////////////////////
// actually loads all the profiles
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::FindMainOLKey(bool bDisplayErrDialog)
{
	HKEY hKey;
	int MethodID;
	bool returnval = false;

	//to import personalities we need to know whether we got the registry entries
	//either based on profiles, identities or none at all

	//check if we got profile based personalities

	//CODE REVIEW
	//put all the strings in the .rc file

	char szProfileRegEntry[256];
	LoadResourceString(IDS_PROFILE_REGISTRY_ENTRY, szProfileRegEntry);

	char szProfileRegEntry1[256];
	LoadResourceString(IDS_IDENTITY_REGISTRY_ENTRY_1, szProfileRegEntry1);

	char szProfileRegEntry2[256];
	LoadResourceString(IDS_IDENTITY_REGISTRY_ENTRY_2, szProfileRegEntry2);

	if(RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		
		m_bProfileKey = true;
		m_bIdentityKey = false;

	} else if( (RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry1, 0, KEY_READ, &hKey) == ERROR_SUCCESS) ||
		(RegOpenKeyEx(HKEY_CURRENT_USER, szProfileRegEntry2,
		0, KEY_READ, &hKey) == ERROR_SUCCESS) ) {
		
		m_bProfileKey = false;
		m_bIdentityKey = true;

	} else {
		m_bProfileKey = false;
		m_bIdentityKey = false;
	}

	// here we use MAPI to get the differnt profiles
	HRESULT			hResult;	

	if(!MAPILogon(bDisplayErrDialog))
		return false;

	char szProvider[256];
	LoadResourceString(IDS_PROVIDER_NAME, szProvider);

	LPPROFADMIN lpProf = 0;
	hResult = m_MAPIAdminProfiles(0, &lpProf);
	if(hResult == S_OK) {

		// we usually create a tmp profile when importing from a .pst file.
		// hence before we display to the user the existing profiles, we make sure the
		// tmp profile that we might have created is deleted.
		// deleted later on... for now just make sure that the tmp profile 
		// is not displayed in the dialog box when showing the available profiles
		// added check for above later
		//MAPIEnsureEudoraTmpProfileDeleted(lpProf);

		LPMAPITABLE lpMapiTable = 0;
		LPSRowSet lpRowSet = 0;
		if(lpProf->GetProfileTable(0, &lpMapiTable) == S_OK) {
			lpMapiTable->SeekRow( BOOKMARK_BEGINNING,0 , NULL );
			lpMapiTable->QueryRows(10, NULL, &lpRowSet);
			ULONG nRows = lpRowSet->cRows;

			//this is the number of profiles... if the # of profiles is zero that means
			//we can't do any importing from Outlook, so we quit and return false.
			if(nRows <= 0) {
				return false;
			} else {
				MethodID = AddProvider(szProvider);
				returnval = true;
			}

			for(unsigned int iRow = 0; iRow < nRows ; iRow++) {
				for (unsigned int i = 0; i < lpRowSet->aRow[iRow].cValues; i++) {
					if ((lpRowSet->aRow[iRow].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_NAME & 0xFFFF0000)) {
						LPSTR lpszDisplayName = lpRowSet->aRow[iRow].lpProps[i].Value.lpszA;
						if((lpszDisplayName != NULL) && (strcmp(lpszDisplayName, EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE))) {
							if(m_bProfileKey)	// if registry entry exists for profile then personalities can be imported for each profile
								AddProfiles(lpszDisplayName, MBOX_ABOOK_PERSONA);					
							else 
								AddProfiles(lpszDisplayName, MBOX_ABOOK);
						}
					}
				}
			}
		}
	}

	//we land here, if we don't have registry keys based on profiles
	//we need to get the personality information based on identities
	//for these entries no mail/address book will be imported
	//only personality information will be imported
	if(!m_bProfileKey && m_bIdentityKey) {
		FindOLRegKeys();
		returnval = true;
	}

	return returnval;
}

/////////////////////////////////////////////////////////////////////////////
// search in the registry for personality information related to identities
// add a child node to the provider tree such that these nodes indicate only
// personality information and no mail and address books will be imported
// through these entries. Further the name of the chile node in the provider 
// tree will be the identity name in the registry
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::FindOLRegKeys() 
{

	HKEY hKey;
	HKEY hAccountKey;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = 128;
	HKEY hChildKey;
	FILETIME fileTime;
	bool returnval = false;

	char szProfileRegEntry1[256];
	LoadResourceString(IDS_IDENTITY_REGISTRY_ENTRY_1, szProfileRegEntry1);

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry1, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{

		tempbuffer = DEBUG_NEW char[tempbufsize];

		if (RegOpenKeyEx(hKey, "Accounts", 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (ExtractValue(&tempbuffer, "Account Name", hChildKey))
						AddProfiles(tempbuffer, PERSONA);//here registry entries are for only individual identities and only personality info can be imported

					RegCloseKey(hChildKey);
				}
			tempbufsize = 128;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		returnval = true;
	}

	char szProfileRegEntry2[256];
	LoadResourceString(IDS_IDENTITY_REGISTRY_ENTRY_2, szProfileRegEntry2);

	dwIndex = 0;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry2, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{

		tempbuffer = DEBUG_NEW char[tempbufsize];

		if (RegOpenKeyEx(hKey, "Accounts", 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (ExtractValue(&tempbuffer, "Account Name", hChildKey))
						AddProfiles(tempbuffer, PERSONA); //here registry entries are for only individual identities and only personality info can be imported

					RegCloseKey(hChildKey);
				}
			tempbufsize = 128;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		returnval = true;
	}

	return returnval;
}


/////////////////////////////////////////////////////////////////////////////
// We Have our Provider to export from FindProvider to Main Eudora App
// Fill In values appropriately.
// Called From FindMainOLKey to initialize Provider for use
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddProvider(char * lpszProviderName)
{
	if (!lpszProviderName)
		return false;
	
	m_OLImportProvider.lpszDisplayName = DEBUG_NEW char[strlen(lpszProviderName)+1];
	strcpy(m_OLImportProvider.lpszDisplayName, lpszProviderName);
	m_OLImportProvider.pFirstChild = NULL;
	m_OLImportProvider.iNumChildren = 0;
	m_OLImportProvider.bHasMail = 1;
	m_OLImportProvider.bHasAddresses = 1;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// We Have our profiles to export from FindProvider to Main Eudora App
// Fill In values appropriately.
// Called From FindMainOLKey to initialize Provider for use
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::AddProfiles(LPSTR lpszProfileName, ImportType kImpType)
{
	CImportChild *pCurrentChild;

	pCurrentChild = CreateChildNode(kImpType);

	if(pCurrentChild->lpszIdentityName)
		delete [] pCurrentChild->lpszIdentityName;

	pCurrentChild->lpszIdentityName = DEBUG_NEW_NOTHROW char[ strlen( lpszProfileName ) + 1 ];
    if( pCurrentChild->lpszIdentityName )
		strcpy(pCurrentChild->lpszIdentityName, lpszProfileName);

}

/////////////////////////////////////////////////////////////////////////////
//initialize values
/////////////////////////////////////////////////////////////////////////////
CImportChild* OLImportClass::CreateChildNode(ImportType kImpType)
{

	CImportChild	*pCurrentChild;
	int iChildID = 0;

	m_OLImportProvider.iNumChildren++;
	
	pCurrentChild = m_OLImportProvider.pFirstChild;

	if (pCurrentChild == NULL)
	{
		m_OLImportProvider.pFirstChild = DEBUG_NEW CImportChild;
		pCurrentChild = m_OLImportProvider.pFirstChild;
	}
	else
	{
		iChildID++;			
							
		while (pCurrentChild->pNext != NULL)
		{
			iChildID++;
			pCurrentChild = pCurrentChild->pNext;
		}

		pCurrentChild->pNext = DEBUG_NEW CImportChild;
		pCurrentChild =	pCurrentChild->pNext;
	}
	pCurrentChild->pParent = &m_OLImportProvider;
	pCurrentChild->iAccountID = iChildID;

	pCurrentChild->bHasMail = false;
	pCurrentChild->bHasAddresses = false;

	pCurrentChild->kImportType = kImpType;

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
// Called from the main Eudora app. 
// when the user selects a profile or selects a particular
// .pst file to import this function gets called and builds a list
// of message stores and the folder hierarchy under each of the message stores
// lpszProfileName - name of the profile chosen to import from
// lpszFolderPath - path to the chosen .pst file
// case 1 - if profile is chosen then lpszProfileName value passed and lpszFolderPath is null
// case 2 - if .pst option is chosen then lpszFolderPath value passed and lpszProfileName is null
/////////////////////////////////////////////////////////////////////////////
CMessageStore* OLImportClass::LoadFolders(char *lpszFolderPath, char *lpszProfileName)
{

	bool bPSTFileImport = false;

	//if we get the path to the pst file and the profile to import is null then we know we have to import from
	//a pst file
	if(lpszFolderPath && !lpszProfileName)
		bPSTFileImport = true;

	// Log onto MAPI if possible
	if(!m_bMapiLoggedOn)
	{
		if(MAPILogon())
			m_bMapiLoggedOn = true;
		else
			return NULL;

	}

	//if the user is importing for the second time then we already have the
	//data structures with old values. so clean up all the trees.
	//most of the structures are cleaned in the destructor and might end up
	//with values already... esp linked list we better clean up here before starting
	//any sort of importing again.
	FreeFolderABPersonalityTree();

	// here we handle the case of importing from a .pst file.
	char *lpNewMsgStoreName = 0;
	if(bPSTFileImport) {
		
		if(!CreateTmpProfileFromPST(lpszFolderPath, &lpszProfileName, &lpNewMsgStoreName))
			return false;
	}

	// initialize MAPI for the selected profile
	if (!MAPIInitialize(lpszProfileName))
		return false;

	// if the user is importing from a .pst file then we have one and only
	// one message store that we create temporarily for importing... we don't
	// need to worry bout more than one node in the linked list
	// hence we get the name of the message store that we create for the .pst file
	// open only that specific message store and build a fake linked list of just one
	// node to return to the main Eudora app
	if(bPSTFileImport) {

		//we open the message store by the same passed
		if (!MAPIOpenMessageStore(lpNewMsgStoreName)) 
			return false;
		if (!MAPIProcessFolder(0, NULL))	// Starts at the root (NULL) and works down.
			return false;

		m_pMessageStores = DEBUG_NEW CMessageStore;
		m_pMessageStores->lpszDisplayName = NULL;
		m_pMessageStores->lpMBoxID = m_fnpFolders;
		m_pMessageStores->pDB = m_lpMDB;
		m_fnpFolders = NULL; // the entire tree is deleted later... just set to NULL for now to ge new values
		m_pMessageStores->pNext = NULL;
	
	} else {

		// here we are importing for the entire profile... 
		// each profile will have several message stores not just for each identity under the profile
		// but users can actually define various message stores like archived message
		// stores to just save old messages and we don't want to leave them out
		// open all the message stores in the profile and iterate through the
		// message stores processing each of the message stores found

		if(!OpenAllStoresForProfile())
			return false;

		CMessageStore	*pTmpMsgStore = m_pMessageStores;
		while(m_pMessageStores) {

			if (MAPIOpenMessageStore(m_pMessageStores->lpszDisplayName)) {

				if(m_lpMDB) {

					if (!MAPIProcessFolder(0, NULL))	// Starts at the root (NULL) and works down.
						return false;

					m_pMessageStores->pDB = m_lpMDB;
					m_pMessageStores->lpMBoxID = m_fnpFolders;
					m_lpMDB = NULL;
					m_fnpFolders = NULL; // done so that the next message store fills in a whole new list of folders for the msg store
				}						 // the entire tree is deleted later... just set to NULL for now to ge new values
				m_pMessageStores = m_pMessageStores->pNext;				
			}
		}
		m_pMessageStores = pTmpMsgStore;
	
	}

	EscapePressed(); // Just to clear the key from the queue

	return m_pMessageStores;
}

/////////////////////////////////////////////////////////////////////////////
// Logs us onto MAPI [mapi32.dll]
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPILogon(bool bDisplayErrDialog /* = false */)
{
	HRESULT			hResult;	

	m_hMapiDLL = LoadLibrary("Mapi32");
	
	m_MAPIFreeBuffer					=	(CMIMAPIFreeBuffer)GetProcAddress(m_hMapiDLL, "MAPIFreeBuffer");
	m_MAPIAdminProfiles					=	(CMIMAPIAdminProfiles)GetProcAddress(m_hMapiDLL, "MAPIAdminProfiles");
	m_MAPIInitialize					=	(CMIMAPIInitialize)GetProcAddress(m_hMapiDLL, "MAPIInitialize");
	m_HrQueryAllRows					=	(CMIHrQueryAllRows)GetProcAddress(m_hMapiDLL, "HrQueryAllRows@24");
	m_MAPILogonEx						=	(CMIMAPILogonEx)GetProcAddress(m_hMapiDLL, "MAPILogonEx");

	if (m_hMapiDLL  != NULL)
	{
		if (!m_MAPIInitialize || !m_MAPILogonEx || !m_MAPIFreeBuffer)
		{

			// handle the error
			FreeLibrary(m_hMapiDLL); 

			if(bDisplayErrDialog && (m_bProfileKey || m_bIdentityKey) ) {

				TCHAR szBuf[2048];

				HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		
				if(!LoadString((HINSTANCE)m_hModule, IDS_REINSTALL_OUTLOOK, szBuf, 2048))
					strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
		
				MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);		
			}

			return false;
		}


		// make OL the default email client to try and import
		// if we don't do this earlier users were getting an error that
		// they have to set OL to the default client to proceed
		// we spare the user and set the default client to Microsoft Outlook
		// just for importing purposes and Eudora sets itself to the default client
		// once importing is done

		char szDefMailRegEntry[256];
		LoadResourceString(IDS_DEFAULT_MAIL_REG_ENTRY, szDefMailRegEntry);

		char szProviderName[256];
		LoadResourceString(IDS_PROVIDER_NAME, szProviderName);

		AddToRegistry(HKEY_LOCAL_MACHINE, szDefMailRegEntry, NULL, szProviderName);	// HLM\Software\Clients\Mail\Eudora

		hResult = m_MAPIInitialize(NULL);

		if( HR_FAILED(hResult))
		{
			FreeLibrary(m_hMapiDLL); 
			return false;
		}

	}
	else 
	{	
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// function is called LoadFolders to open all the message stores
// for a given profile... the profile contains multiple message stores
// hence build a list of message stores under the profile
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::OpenAllStoresForProfile()
{
	HRESULT hresult;
	IMAPITable *pMSTable;

    hresult = m_lpSession->GetMsgStoresTable(0, &pMSTable);

    if (hresult == S_OK) { 
		
        SizedSPropTagArray(2, mstablecols) = { 2, {PR_RESOURCE_FLAGS,PR_DISPLAY_NAME} };
        SRowSet *pRows;
        
		hresult = m_HrQueryAllRows(pMSTable,(SPropTagArray*)&mstablecols,NULL,NULL,0,&pRows);
        
		if (hresult==S_OK) { 
			for (unsigned int i=0; i<pRows->cRows; i++) { 
				char* name=""; 
				bool isdefault=false;
				
				if (pRows->aRow[i].lpProps[0].ulPropTag==PR_RESOURCE_FLAGS) 
					isdefault=(pRows->aRow[i].lpProps[0].Value.ul&STATUS_DEFAULT_STORE)!=0;
				if (pRows->aRow[i].lpProps[1].ulPropTag==PR_DISPLAY_NAME) {
					name = DEBUG_NEW char[strlen(pRows->aRow[i].lpProps[1].Value.lpszA)+1];
					strcpy(name, pRows->aRow[i].lpProps[1].Value.lpszA);
				}
              
				if (name!="") { 
					//add the message store name to the linked list of message store names
			
					CMessageStore *pEntry;
					pEntry = DEBUG_NEW CMessageStore;
					InitMsgStoreTree(pEntry);
					pEntry->lpszDisplayName = DEBUG_NEW char[strlen(name)+1];
					strcpy(pEntry->lpszDisplayName, name);
					// there is just one default message store in the profile
					// the default store is the store that has the IN, OUT and TRASH mailboxes
					// we use this flag later on in our import process...
					// if it is the default message store and user is importing for the first time
					// into Eudora we fetch the mails from OL folders and populate the main Eudora
					// In, Out and Trash mailboxes
					pEntry->bIsDefault = isdefault;
					AddMsgStoreEntryToTree(pEntry);
					delete [] name;

				}
            }
            FreeRowSet(m_MAPIFreeBuffer, pRows);
          }
          pMSTable->Release();
	} else {
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// initializes the message store structure members
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitMsgStoreTree(CMessageStore *pEntry)
{
	pEntry->lpszDisplayName     = NULL;
	pEntry->pDB					= NULL;
	pEntry->lpMBoxID			= NULL;
	pEntry->bIsDefault			= false;
	pEntry->pNext				= NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// adds the node passed to the member variable m_pMessageStores 
// that contains the linked list of message stores
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddMsgStoreEntryToTree(CMessageStore *pEntry)
{
	CMessageStore *pWalker;

	pWalker = m_pMessageStores;

	if(pWalker == NULL)
	{
		m_pMessageStores = DEBUG_NEW CMessageStore;
		pWalker = m_pMessageStores;
		InitMsgStoreTree(pWalker);
		pWalker->pNext = NULL;
	}
	else
	{
		while (pWalker->pNext != NULL)
		{
			pWalker = pWalker->pNext;
		}

		pWalker->pNext = DEBUG_NEW CMessageStore;
		pWalker = pWalker->pNext;
		InitMsgStoreTree(pWalker);
		pWalker->pNext = NULL;

	}
	*pWalker = *pEntry;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// if user is importing from a .pst file there is no direct way i.e. no MAPI API 
// to do the import... all importing using MAPI is done with respect to profiles
// so we create a temporary profile defined by the variable
// EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE. I've on purpose created this name
// so that it doesn't clash with any other existing profiles. for the life of me
// I can't imagine anyone with a profile name that I am using now.
// so we create a temporary profile, and add an MS-PST store to this profile, and configure
// the store to point to the specified PST file. Then, given our own profile name
// and the name of this store, we open the message store and create the list of 
// folders for the store
// Although we keep calling it a temporary profile, there's not anything intrinsically
// temporary about it. It is actually created and will exist if we do not delete the profile
// Its temporariness is in the fact that a call to MAPIEnsureEudoraTmpProfileDeleted deletes
// the profile
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::CreateTmpProfileFromPST(char* lpszFolderPath, char **lppszProfileName, char **lpNewMsgStoreName) 
{

	
	HRESULT hresult;
	
	//
	// First thing we do is clean out anything we've allocated before.
	m_bGotEids = false; 

	if (m_lpMDB != 0) 
		m_lpMDB->Release(); 
	m_lpMDB=0;

	if (m_lpSession != 0) {
		m_lpSession->Logoff(0,0,0); 
		m_lpSession->Release();
	}

  	if ((m_hMapiDLL == 0) || (m_MAPIAdminProfiles == 0)) 
		return false;
  
	// Plan: create a temporary profile, and add a PST service to it, configured for that filename.
	IProfAdmin *pProfAdmin;
	hresult = m_MAPIAdminProfiles(0,&pProfAdmin);
	
	if (hresult != S_OK) 
		return false;
	
	// ensure that the temporary profile that might have been created by us
	// when the user had previously imported from a .pst file is deleted.
	MAPIEnsureEudoraTmpProfileDeleted(pProfAdmin);
	HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);

	//make a nice long name for the temp profile, since it has to be unique from the profiles the user already has
	hresult = pProfAdmin->CreateProfile(const_cast<char*>(EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE), NULL, PtrToUlong(hEudoraMain), 0);
	
	if (hresult != S_OK) {
		pProfAdmin->Release(); 
		return false;
	}

	IMsgServiceAdmin *pMsgAdmin;
	hresult = pProfAdmin->AdminServices(const_cast<char*>(EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE),NULL,PtrToUlong(hEudoraMain),0,&pMsgAdmin);
	
	if (hresult != S_OK) {
		pMsgAdmin->Release(); 
		return false;
	}

	// Now we create the message-store-service.
	//make a nice long name for the temp message store
	hresult = pMsgAdmin->CreateMsgService("MSPST MS", const_cast<char*>(EUDORA_TO_OUTLOOK_IMPORTER_TEMP_MESSAGE_STORE) ,PtrToUlong(hEudoraMain),SERVICE_UI_ALLOWED);
	if (hresult == MAPI_E_UNKNOWN_FLAGS) // Outlook97 doesn't understand those two flags at the end...{ 
		hresult = pMsgAdmin->CreateMsgService("MSPST MS", const_cast<char*>(EUDORA_TO_OUTLOOK_IMPORTER_TEMP_MESSAGE_STORE), 0, 0);
	
	if (hresult != S_OK) {
		pMsgAdmin->Release(); 
		MAPIEnsureEudoraTmpProfileDeleted(pProfAdmin); 
		pProfAdmin->Release(); 
		return false;
	}

	// We need to get hold of the MAPIUID for this message-service. We do this
	// by enumerating the message-stores (there will be only one!) and picking it up.
	// Actually, we set up 'mscols' to retrieve the name as well as the MAPIUID
	IMAPITable *pMSTable;
	hresult = pMsgAdmin->GetMsgServiceTable(0,&pMSTable);
	
	if (hresult != S_OK) {
		pMsgAdmin->Release(); 
		MAPIEnsureEudoraTmpProfileDeleted(pProfAdmin); 
		pProfAdmin->Release(); 
		return false;
	}

	SizedSPropTagArray(2, mscols) = { 2, {PR_SERVICE_UID,PR_DISPLAY_NAME} };

	pMSTable->SetColumns((SPropTagArray*)&mscols,0);
	SRowSet *pRowSet;
	hresult = pMSTable->QueryRows(1,0,&pRowSet);
	pMSTable->Release();
	
	if (hresult != S_OK) {
		pMsgAdmin->Release(); 
		MAPIEnsureEudoraTmpProfileDeleted(pProfAdmin); 
		pProfAdmin->Release(); 
		return false;
	}

	MAPIUID msuid = *((MAPIUID*)pRowSet->aRow[0].lpProps[0].Value.bin.lpb);
	FreeRowSet(m_MAPIFreeBuffer, pRowSet);
	
	
	// Now configure our message-store to use the PST filename.
	SPropValue msprops[1];
	msprops[0].ulPropTag = PR_PST_PATH; 
	msprops[0].Value.lpszA = lpszFolderPath;
	pMsgAdmin->ConfigureMsgService(&msuid, PtrToUlong(hEudoraMain), SERVICE_UI_ALLOWED, 1, msprops);

	// That will have changed the message-store's display-name. Let's get it again.
	// The 'mscols' was already set up to retrieve names. 
	pMsgAdmin->GetMsgServiceTable(0, &pMSTable);
	pMSTable->SetColumns((SPropTagArray*)&mscols,0);
	pMSTable->QueryRows(1,0,&pRowSet);
	pMSTable->Release();
	
	if (pRowSet->aRow[0].lpProps[1].ulPropTag==PR_DISPLAY_NAME) {
		*lpNewMsgStoreName = DEBUG_NEW char[strlen(pRowSet->aRow[0].lpProps[1].Value.lpszA) + 1];
		strcpy(*lpNewMsgStoreName, pRowSet->aRow[0].lpProps[1].Value.lpszA);
	}
	
	*lppszProfileName = DEBUG_NEW char[strlen(EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE) + 1];
	strcpy(*lppszProfileName, EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE);

	FreeRowSet(m_MAPIFreeBuffer, pRowSet);
	// all done!
	pMsgAdmin->Release();
	pProfAdmin->Release();
  
	return true;

}


/////////////////////////////////////////////////////////////////////////////
// MAPI_ENSUREEUDORATMPPROFILEDELETED -- In case we had created a temporary profile,
// this function ensures that it is deleted.
// Note: it is a *SEVERE* error if you try to delete a profile that's not there.
// Under Outlook2000, attempting to do this will delete every message-store
// of the main profile.
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::MAPIEnsureEudoraTmpProfileDeleted(IProfAdmin *pProfAdmin)
{ 
	bool bGotProfile=false;
	IMAPITable *pProfTable;

	HRESULT hresult = pProfAdmin->GetProfileTable(0, &pProfTable);

	if (hresult==S_OK) {
		
		SizedSPropTagArray(2, ProfTableCols) = { 2, {PR_DISPLAY_NAME,PR_DEFAULT_PROFILE} };
		SRowSet *pProfRows;
		
		hresult = m_HrQueryAllRows(pProfTable,(SPropTagArray*)&ProfTableCols,NULL,NULL,0,&pProfRows);

		if (hresult == S_OK) { 
			for (unsigned int i=0; i<pProfRows->cRows; i++) { 
				string name="";
				
				if (pProfRows->aRow[i].lpProps[0].ulPropTag==PR_DISPLAY_NAME) 
					name=pProfRows->aRow[i].lpProps[0].Value.lpszA;

				if (name==EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE) 
					bGotProfile=true;
			}

			FreeRowSet(m_MAPIFreeBuffer, pProfRows);
		}
		
		pProfTable->Release();
	}

	if (bGotProfile) 
		pProfAdmin->DeleteProfile(const_cast<char*>(EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE),0);
}



/////////////////////////////////////////////////////////////////////////////
// try to log on to MAPI for the given profile... this will get us a session variable
// that is associated with the profile and we use this session variable
// in the rest of the code to access various other stuff
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPIInitialize(char *lpszProfileName)
{

	HRESULT			hResult;

	hResult = m_MAPILogonEx(NULL, lpszProfileName, NULL, MAPI_EXTENDED | MAPI_LOGON_UI | MAPI_NEW_SESSION,(LPMAPISESSION FAR *) &m_lpSession);
	if( HR_FAILED(hResult) || !m_lpSession)
	{

		TCHAR szBuf[2048];

		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
			
		if(!LoadString((HINSTANCE)m_hModule, IDS_IMPORT_MAPI_OPEN_ERROR, szBuf, 2048))
			strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
			
		MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);		


		// Logon Failed....
		FreeLibrary(m_hMapiDLL); 
		return false;
	}
	return true;

}

/////////////////////////////////////////////////////////////////////////////
// task is to get the store we know by name. We do this by enumerating
// all the message-stores in the table, and picking out the one with the right name.
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPIOpenMessageStore(const char* const pMessageStoreName)
{

	HRESULT hresult;

    IMAPITable *pMSTable=0;
    hresult = m_lpSession->GetMsgStoresTable(0, &pMSTable);

    if (hresult == S_OK) { 
		SizedSPropTagArray(2, mstablecols) = { 2, {PR_ENTRYID,PR_DISPLAY_NAME} };
		SRowSet *pRows;
		hresult = m_HrQueryAllRows(pMSTable,(SPropTagArray*)&mstablecols,NULL,NULL,0,&pRows);

		if (hresult == S_OK) {
			
			for (unsigned int i=0; i<pRows->cRows && m_lpMDB == 0; i++) { 

				HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
				char* name=""; 
				SPropValue *eid;
				if (pRows->aRow[i].lpProps[0].ulPropTag==PR_ENTRYID) 
					eid = &pRows->aRow[i].lpProps[0];
				if (pRows->aRow[i].lpProps[1].ulPropTag==PR_DISPLAY_NAME) 
					name = pRows->aRow[i].lpProps[1].Value.lpszA;
				if ((!strcmp(name, pMessageStoreName)) && (!IsEIDEmpty(eid))) 
					hresult = m_lpSession->OpenMsgStore(PtrToUlong(hEudoraMain),pRows->aRow[i].lpProps[0].Value.bin.cb,
									(LPENTRYID)pRows->aRow[i].lpProps[0].Value.bin.lpb, NULL, MDB_NO_MAIL, &m_lpMDB);
		        if (hresult != S_OK) 
					m_lpMDB = 0;
			}

		} else {

			return false;
		}
        
		FreeRowSet(m_MAPIFreeBuffer, pRows);

	} else {
		return false;
	}

	if(pMSTable)
		pMSTable->Release();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// previously was using the code before i knew about the multiple message stores
// being under one profile and was importing only from the default message store
// Currently not being used... however has some useful code that i might
// pick up when wrapping stuff up... will delete once it is not required
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPIOpenDefaultStore()
{

   LPSRowSet	pRow = NULL;
   static		SRestriction sres;
   SPropValue	spv;
   HRESULT		hResult;

   enum {EID, NAME, NUM_COLS};
   static SizedSPropTagArray(NUM_COLS,sptCols) = {NUM_COLS, PR_ENTRYID, PR_DISPLAY_NAME};

  
   //Get the table of all the message stores available
   hResult = m_lpSession->GetMsgStoresTable(0, &m_lpTblStores);

   
   if (HR_FAILED(hResult) || !m_lpTblStores) 
	   return false;

   //Set up restriction for the default store
   sres.rt = RES_PROPERTY; //Comparing a property
   sres.res.resProperty.relop = RELOP_EQ; //Testing equality
   sres.res.resProperty.ulPropTag = PR_DEFAULT_STORE; //Tag to compare
   sres.res.resProperty.lpProp = &spv; //Prop tag and value to compare against

   spv.ulPropTag = PR_DEFAULT_STORE; //Tag type
   spv.Value.b   = TRUE; //Tag value

   //Convert the table to an array which can be stepped through
   //Only one message store should have PR_DEFAULT_STORE set to true, so only one will be returned
   hResult = m_HrQueryAllRows(
						m_lpTblStores, 
						(LPSPropTagArray) &sptCols, 
						&sres, 
						NULL, 
						0, //Max number of rows (0 means no limit)
						&pRow); 


   if (HR_FAILED(hResult) || !pRow) {
		FreeRowSet(m_MAPIFreeBuffer, pRow);
		if (m_lpTblStores)
			m_MAPIFreeBuffer(m_lpTblStores);
		m_lpTblStores = NULL;
		return false;
   }

	//Open the first returned (default) message store
	hResult = m_lpSession->OpenMsgStore(
										NULL,
										pRow->aRow[0].lpProps[EID].Value.bin.cb,
										(LPENTRYID)pRow->aRow[0].lpProps[EID].Value.bin.lpb,
										NULL,
										MAPI_BEST_ACCESS,
										&m_lpMDB);

   
    if( HR_FAILED( hResult ) || !m_lpMDB)
    {

   		if (pRow)
			FreeRowSet(m_MAPIFreeBuffer, pRow);
		if (m_lpTblStores)
			m_MAPIFreeBuffer(m_lpTblStores);
		m_lpTblStores = NULL;

		TCHAR szBuf[2048];

		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		
		if(!LoadString((HINSTANCE)m_hModule, IDS_IMPORT_MAIL_READING_ERROR3, szBuf, 2048))
			strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
		
		MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);		
		return false;
    }


	return true;

}

/////////////////////////////////////////////////////////////////////////////
// Processes out the folder essentially filling the structure
// Recursive if a folder has subfolders
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPIProcessFolder(ULONG cbEntryID, LPENTRYID lpEntryID, UINT iParentID)
{

	HRESULT	hresult;

	// Now we have the msgstore for the specified profile. We get the intepersonal-IPM subtree.
	// All email folders are children of the IMP subtree.
	IMAPIFolder *pIPMRoot=0; 
	SPropValue *pIPM_EID;

	SizedSPropTagArray(1, cols) = { 1, {PR_IPM_SUBTREE_ENTRYID}};
	ULONG pcount;
	hresult = m_lpMDB->GetProps((SPropTagArray*)&cols,0,&pcount,&pIPM_EID);

	if (hresult == MAPI_W_ERRORS_RETURNED) 
		return false;
  

	if (pIPM_EID) { 
		ULONG ipmroottype;
		hresult = m_lpMDB->OpenEntry(pIPM_EID->Value.bin.cb,(ENTRYID*)pIPM_EID->Value.bin.lpb,NULL,0,&ipmroottype,(IUnknown**)&pIPMRoot);
		if (hresult == S_OK) { 
			if (ipmroottype != MAPI_FOLDER) {
				pIPMRoot->Release(); 
				pIPMRoot=0;
			}
		}
	}

	if (pIPMRoot==0)
		return false;

	// the following recursive call does the work! puts the tree under "ipmroot" into mapi_Folders.
	MakeFolders(pIPMRoot, "");
	pIPMRoot->Release();
	return true;

}

/////////////////////////////////////////////////////////////////////////////
// Free all the rows in a rowset then the rowset itself
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::FreeRowSet(CMIMAPIFreeBuffer MAPIFreeBufferfunc, LPSRowSet lpRows)
{
    ULONG   n;

    if(lpRows)
    {
        for( n = 0 ; n < lpRows->cRows ; n++)
        {
            if( MAPIFreeBufferfunc(lpRows->aRow[n].lpProps ) )
            {
                return false;
            }
        }
        if( MAPIFreeBufferfunc(lpRows) )
        {
            return false;
        }
    }
    
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// Given an MAPIFolder, we recursively retrieve
// all the folders it contains, and stick them into the 'folders' list.
/////////////////////////////////////////////////////////////////////////////

void OLImportClass::MakeFolders(IMAPIFolder *pParent, std::string prefix, UINT iParentID)
{ 

	static UINT iID = 1;
	
	if (pParent==0) 
		return;

	// NB. We cannot call parent->GetHierarchyTable. That's because GetHierarchyTable
	// tries to open it will full (read/write) access, but Outlook2000 atleast only supports
	// readonly access, hence giving an MAPI_E_NO_ACCESS error. Therefore, we get
	// the hierarchy in this roundabout way, in readonly mode.
	IMAPITable *hierarchy; 
	HRESULT hresult;

	const GUID local_IID_IMAPITable = {0x00020301,0,0, {0xC0,0,0,0,0,0,0,0x46}};
  
	hresult = pParent->OpenProperty(PR_CONTAINER_HIERARCHY,&local_IID_IMAPITable,0,0,(IUnknown**)&hierarchy);
	if (hresult!=S_OK) 
		return;

	// and query for all the rows
	SizedSPropTagArray(3, cols) = {3, {PR_ENTRYID,PR_DISPLAY_NAME,PR_SUBFOLDERS} };
	SRowSet *rows = 0;
	hresult = m_HrQueryAllRows(hierarchy,(SPropTagArray*)&cols, NULL, NULL, 0, &rows);
	hierarchy->Release();
	if (hresult!=S_OK) {
		for (unsigned int i=0; i<rows->cRows; i++)
			m_MAPIFreeBuffer(rows->aRow[i].lpProps);
		m_MAPIFreeBuffer(rows);
		return;
	}
  
	// Note: the entry-ids returned by the list are just short-term list-specific
	// entry-ids. But we want to put long-term entry-ids in our 'folder' list.
	// That's why it's necessary to open the folder...

	// Go through all the rows. For each entry, if it is a message-folder add it, and potentially recurse
	for (unsigned int i=0; i<rows->cRows; i++)  { 
		bool bHasSubfolders = (rows->aRow[i].lpProps[2].Value.b > 0) ? true : false;
		string name(rows->aRow[i].lpProps[1].Value.lpszA);
		IMAPIFolder *subf; 
		ULONG subftype;
		
		hresult = pParent->OpenEntry(rows->aRow[i].lpProps[0].Value.bin.cb,
							(LPENTRYID)rows->aRow[i].lpProps[0].Value.bin.lpb, NULL,
							0, &subftype, (IUnknown**)&subf);
		
		if (hresult == S_OK) { 
		
			if (subftype == MAPI_FOLDER) { 

				SPropValue *veid=0;
	
				SizedSPropTagArray(1, cols) = { 1, {PR_ENTRYID}};
				ULONG pcount;
			
				hresult = subf->GetProps((SPropTagArray*)&cols,0,&pcount,&veid);

				if (hresult == MAPI_W_ERRORS_RETURNED) 
					return;

				if (hresult == S_OK) { 
				
					m_MapiFolderType folderType = MapiGetFolderType(veid, subf);
					
					bool bUsefolder = (folderType==INBOX||folderType==SENT_MBOX||folderType==MAIL_MBOX||folderType==OTHER_OL_STUFF||folderType==DELETED_MBOX);

					// enum that we use later on in the main Eudora app
					// if user is importing for first time and is default message store
					// we set the type of the mailboxes here so that during the
					// import process we import the In, Out and Trash directly into
					// main Eudora folder. This value is ignored for subsequent imports
					FolderType folder_type = REGULAR_MBOX;

					if(folderType == INBOX) 
						folder_type = IN_MBOX;
					if(folderType == SENT_MBOX)
						folder_type = OUT_MBOX;
					if(folderType == DELETED_MBOX)
						folder_type = TRASH_MBOX;

					if (bUsefolder)
						AddFolder(const_cast<char *>((prefix+name).c_str()), const_cast<char *>(name.c_str()), bHasSubfolders, iID++, iParentID, static_cast<void *>(veid->Value.bin.lpb), veid->Value.bin.cb, folder_type);
												
					if (bUsefolder && bHasSubfolders) 
						MakeFolders(subf, prefix+name+"\\", iID-1);
				}
			}
			
			subf->Release();
		}
	}

	for (unsigned int j=0; j<rows->cRows; j++)
		m_MAPIFreeBuffer(rows->aRow[j].lpProps);
	m_MAPIFreeBuffer(rows);

//#endif
}


/////////////////////////////////////////////////////////////////////////////
// GET-FOLDER-TYPE -- given a folder and its long-term entry-id,
// returns its type (inbox/outbox/calendar/...). There are three
// techniques for doing this; we do them all, in order of preference.
/////////////////////////////////////////////////////////////////////////////
OLImportClass::m_MapiFolderType OLImportClass::MapiGetFolderType(SPropValue *eid, IMAPIFolder *f)
{
  // 1. Most assured way to get the type of a folder is to check
  // whether it's long-term ENTRYID is the same as one of the
  // standard ones. EnsureCommonEids routine retrieve the standard ones for us.
  MapiEnsureCommonEids();
  
  if (AreEIDsEqual(eid, m_lpSession, m_InboxEID)) 
	  return INBOX;
  if (AreEIDsEqual(eid, m_lpSession, m_OutboxEID)) 
	  return OUTBOX;
  if (AreEIDsEqual(eid, m_lpSession, m_SentEID)) 
	  return SENT_MBOX;
  if (AreEIDsEqual(eid, m_lpSession, m_DeletedEID)) 
	  return DELETED_MBOX;
  
  // 2. Second best way, specific to Outlook, is to see if
  // it's equal to one of the Outlook specific ones.
  if (AreEIDsEqual(eid, m_lpSession, m_CalendarEID)) 
	  return CALENDAR;
  if (AreEIDsEqual(eid, m_lpSession, m_ContactsEID)) 
	  return CONTACTS;
  if (AreEIDsEqual(eid, m_lpSession, m_JournalEID)) 
	  return JOURNAL;
  if (AreEIDsEqual(eid, m_lpSession, m_NotesEID)) 
	  return NOTES;
  if (AreEIDsEqual(eid, m_lpSession, m_TasksEID)) 
	  return TASKS;

  // 3. Third best way is to check it's PR_CONTAINER_CLASS property. 
  // Everything begins with IPM or IPF.
  SPropValue *sp;
  SizedSPropTagArray(1, cols) = { 1, {PR_CONTAINER_CLASS}};
  ULONG pcount; 
  HRESULT hresult = f->GetProps((SPropTagArray*)&cols,0, &pcount, &sp);

  if (hresult == MAPI_W_ERRORS_RETURNED)
	  m_MAPIFreeBuffer(sp); 
   
  if (hresult != S_OK) 
	  return OTHER_OL_STUFF;

  string s(sp->Value.lpszA);
  
  m_MAPIFreeBuffer(sp);
  
  if (s.length()>3 && s[0]=='I' && s[1]=='P') s[2]='.'; // not sure IPM or IPF.
  const char *c=s.c_str();
  if (strncmp(c,"IP..Note",8)==0) 
	  return MAIL_MBOX;
  else if (strncmp(c,"IP..Appointment",15)==0) 
	  return CALENDAR;
  else if (strncmp(c,"IP..Contact",11)==0) 
	  return CONTACTS;
  else if (strncmp(c,"IP..Journal",11)==0) 
	  return JOURNAL;
  else if (strncmp(c,"IP..StickyNote",14)==0) 
	  return NOTES;
  else if (strncmp(c,"IP..Task",8)==0) 
	  return TASKS;
  else if (strncmp(c,"IP..",4)==0) 
	  return OL_SPECIAL;
  else 
	  return OTHER_OL_STUFF;
}

/////////////////////////////////////////////////////////////////////////////
// There are some standard ENTRYIDs for some standard
// folders. This function sets up these in member variables. Note that they
// are specific to the current profile and message-store. 
// and will change for a diff profile and diff message store
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::MapiEnsureCommonEids()
{ 
	if (m_bGotEids) 
		return;

	if(!m_InboxEID)
		m_InboxEID = DEBUG_NEW SPropValue;
		
	if(!m_OutboxEID)
		m_OutboxEID = DEBUG_NEW SPropValue;

	if(!m_SentEID)
		m_SentEID = DEBUG_NEW SPropValue;

	if(!m_DeletedEID)
		m_DeletedEID = DEBUG_NEW SPropValue;

	if(!m_CalendarEID)
		m_CalendarEID = DEBUG_NEW SPropValue;

	if(!m_ContactsEID)
		m_ContactsEID = DEBUG_NEW SPropValue;

	if(!m_JournalEID)
		m_JournalEID = DEBUG_NEW SPropValue;

	if(!m_NotesEID)
		m_NotesEID = DEBUG_NEW SPropValue;

	if(!m_TasksEID)
		m_TasksEID = DEBUG_NEW SPropValue;


	ClearEID(m_InboxEID);
	ClearEID(m_OutboxEID);
	ClearEID(m_SentEID); 
	ClearEID(m_DeletedEID);
	ClearEID(m_CalendarEID);
	ClearEID(m_ContactsEID); 
	ClearEID(m_JournalEID); 
	ClearEID(m_NotesEID); 
	ClearEID(m_TasksEID);

	m_bGotEids = true;
	
	if (m_lpMDB==0) 
		return;
	
	DWORD size; 
	ENTRYID *eid; 
	HRESULT hresult;

	// 1. INBOX special folder -- in fact, the user can designate any folder as
	// an inbox. All we can do is check where incoming IPM.Note messages (i.e. emails)
	// are placed.
	hresult = m_lpMDB->GetReceiveFolder("IPM.Note",0,&size,&eid,NULL);
	if (hresult == S_OK) 
		SetEID(m_InboxEID, size, eid);

	// 2. Other special folders. The message-store has properties for these.
	SizedSPropTagArray(4, cols) = { 4, {PR_VALID_FOLDER_MASK, PR_IPM_OUTBOX_ENTRYID, PR_IPM_SENTMAIL_ENTRYID, PR_IPM_WASTEBASKET_ENTRYID} };
	ULONG pcount; 
	SPropValue *props;
	hresult = m_lpMDB->GetProps((SPropTagArray*)&cols, 0, &pcount, &props);
	
	if ( (hresult == S_OK) || (hresult == MAPI_W_ERRORS_RETURNED) ) { 
		
		LONG mask; 
	
		if (props[0].ulPropTag != PT_ERROR) 
			mask=props[0].Value.ul; 
		else 
			mask=0;
    
		if ( (mask&FOLDER_IPM_OUTBOX_VALID) && (props[1].ulPropTag != PT_ERROR) )
			SetEID(m_OutboxEID, props[1].Value.bin.cb, (ENTRYID*)props[1].Value.bin.lpb);

		if ( (mask&FOLDER_IPM_SENTMAIL_VALID) && (props[2].ulPropTag != PT_ERROR) )
			SetEID(m_SentEID, props[2].Value.bin.cb, (ENTRYID*)props[2].Value.bin.lpb);

		if ((mask&FOLDER_IPM_WASTEBASKET_VALID) && (props[3].ulPropTag != PT_ERROR) )
			SetEID(m_DeletedEID, props[3].Value.bin.cb, (ENTRYID*)props[3].Value.bin.lpb);
		
		m_MAPIFreeBuffer(props);
	}

	// 3. The outlook specials. The inbox has properties for these.
	if (IsEIDEmpty(m_InboxEID)) 
		return;
  
	ULONG intype; 
	IMAPIFolder *infolder;
	hresult = m_lpMDB->OpenEntry(m_InboxEID->Value.bin.cb, (LPENTRYID)m_InboxEID->Value.bin.lpb, NULL, 0, &intype, (IUnknown**)&infolder);
	
	if (hresult != S_OK) 
		return;
	
	if (intype != MAPI_FOLDER) {
		infolder->Release();
		return;
	}

	SizedSPropTagArray(5, spec) = {5, {0x36D00102, 0x36D10102, 0x36D20102, 0x36D30102, 0x36D40102}};
	hresult = infolder->GetProps((SPropTagArray*)&spec,0,&pcount,&props);
	
	if ( (hresult==S_OK) || (hresult==MAPI_W_ERRORS_RETURNED) ) { 
		
		if (props[0].ulPropTag != PT_ERROR) 
			SetEID(m_CalendarEID, props[0].Value.bin.cb, (ENTRYID*)props[0].Value.bin.lpb);
		if (props[1].ulPropTag != PT_ERROR) 
			SetEID(m_ContactsEID, props[1].Value.bin.cb, (ENTRYID*)props[1].Value.bin.lpb);
		if (props[2].ulPropTag != PT_ERROR) 
			SetEID(m_JournalEID, props[2].Value.bin.cb, (ENTRYID*)props[2].Value.bin.lpb);
		if (props[3].ulPropTag != PT_ERROR) 
			SetEID(m_NotesEID, props[3].Value.bin.cb, (ENTRYID*)props[3].Value.bin.lpb);
		if (props[4].ulPropTag != PT_ERROR) 
			SetEID(m_TasksEID, props[4].Value.bin.cb, (ENTRYID*)props[4].Value.bin.lpb);

		m_MAPIFreeBuffer(props);
	}
}

/////////////////////////////////////////////////////////////////////////////
// compares if the 2 Entry IDs are the same
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AreEIDsEqual(const SPropValue * const eid, IMAPISession * pSession, const SPropValue * const e) 
{
	if ( IsEIDEmpty(eid) || IsEIDEmpty(e) ) 
		return false;

    ULONG res; 
	HRESULT hresult = pSession->CompareEntryIDs(eid->Value.bin.cb, (LPENTRYID)eid->Value.bin.lpb, 
												e->Value.bin.cb, (LPENTRYID)e->Value.bin.lpb, 0, &res);
    
	if (hresult != S_OK) 
		return false;
    
	return (res != 0);

}

/////////////////////////////////////////////////////////////////////////////
// clears the Entry ID
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::ClearEID(SPropValue * psPropVal)
{

	psPropVal->Value.bin.lpb = 0;
	psPropVal->Value.bin.cb=0; 

}

/////////////////////////////////////////////////////////////////////////////
// sets the value for an Entry ID
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::SetEID(SPropValue * psPropVal, ULONG size, ENTRYID * pEID) 
{
	if (psPropVal->Value.bin.lpb != 0) 
		delete[] ((char*)psPropVal->Value.bin.lpb); 
	
	psPropVal->Value.bin.cb = size; 
	
	if (pEID==0) 
		psPropVal->Value.bin.lpb=0; 
	else {
		psPropVal->Value.bin.lpb = reinterpret_cast<LPBYTE>(DEBUG_NEW char[size]);
		memcpy(psPropVal->Value.bin.lpb, (LPBYTE)pEID, size);
	}
}

/////////////////////////////////////////////////////////////////////////////
// checks if it is an empty Entry ID
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::IsEIDEmpty(const SPropValue * const psPropVal) 
{
	return ( (psPropVal->Value.bin.cb==0) || (psPropVal->Value.bin.lpb==0) );
}

/////////////////////////////////////////////////////////////////////////////
// Done with Store, so close it and free up and all MAPI specific variables
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::MAPICloseDefaultStore()
{
	unsigned long	ulFlags = LOGOFF_NO_WAIT;
	
	if(m_lpMDB) {
		m_lpMDB->StoreLogoff(&ulFlags);
		m_lpMDB->Release();
		m_MAPIFreeBuffer(m_lpMDB);
		m_lpMDB = NULL;
	}

	if(m_lpSession)
		m_lpSession->Release();

	if(m_lpTblStores) {
		m_lpTblStores->Release();
		m_MAPIFreeBuffer(m_lpTblStores);
		m_lpTblStores = NULL;
	}

	if(m_InboxEID)
		m_MAPIFreeBuffer(m_InboxEID);

	if(m_OutboxEID)
		m_MAPIFreeBuffer(m_OutboxEID);

	if(m_SentEID)
		m_MAPIFreeBuffer(m_SentEID);

	if(m_DeletedEID)
		m_MAPIFreeBuffer(m_DeletedEID);
	
	if(m_CalendarEID)
		m_MAPIFreeBuffer(m_CalendarEID);

	if(m_ContactsEID)
		m_MAPIFreeBuffer(m_ContactsEID);

	if(m_JournalEID)
		m_MAPIFreeBuffer(m_JournalEID);

	if(m_NotesEID)
		m_MAPIFreeBuffer(m_NotesEID);

	if(m_TasksEID)
		m_MAPIFreeBuffer(m_TasksEID);

}

/////////////////////////////////////////////////////////////////////////////
// Done with MAPI, so Logoff
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPILogoff()
{	
	CMIMAPIUnInitialize MAPIUnInitialize	=	(CMIMAPIUnInitialize)GetProcAddress(m_hMapiDLL, "MAPIUninitialize");

	if (m_lpSession)
	{
		m_lpSession->Logoff(NULL, NULL, 0);
		m_lpSession->Release();

		m_MAPIFreeBuffer(m_lpSession);
		m_lpSession = NULL;
	}

	MAPIUnInitialize();

	FreeLibrary(m_hMapiDLL);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Make a folder and add to our linked list of folders for the message store
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddFolder(char* szFolderName, char* szFolderDisplayName, bool bHasChildren, UINT iID, UINT iParentID, void * pEntryID, unsigned int iSize, FolderType folderType)
{

	CMbox_FnamePair  **realnames = &m_fnpFolders;

	if (*realnames)
	{
		while ((*realnames)->pNext)
			realnames = &((*realnames)->pNext);

		realnames = &((*realnames)->pNext);
	}


	*realnames = DEBUG_NEW CMbox_FnamePair;

	(*realnames)->lpszFilename = DEBUG_NEW char[strlen(szFolderName) + 1];
	strcpy((*realnames)->lpszFilename , szFolderName);

	(*realnames)->lpszDisplayName = DEBUG_NEW char[strlen(szFolderDisplayName)+1];
	strcpy((*realnames)->lpszDisplayName, szFolderDisplayName);

	(*realnames)->pID = pEntryID;
	(*realnames)->iSize = iSize;
	(*realnames)->lMBoxID = iID;
	(*realnames)->bHasSubfolders = bHasChildren;
	(*realnames)->lParentMBoxID = iParentID;
	(*realnames)->lpszMailbox = NULL;
	(*realnames)->bCreated = false;
	(*realnames)->folderType = folderType;
	(*realnames)->pNext = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Sees if it is a valid folder that can contain mail
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::IsValidMailFolder(LPMAPIPROP ParentFolder)
{
	LPSPropValue		pReturnedValueArray;
	unsigned long		numEntries;
	char *				lpszContainerClass = NULL;
	char *				lpszFolderName = NULL;
	ULONG i;

	SizedSPropTagArray	(2, DesiredProps) =
    {
        2,
        {
            PR_CONTAINER_CLASS,
			PR_DISPLAY_NAME
        }
    };

	ParentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL,  &numEntries, &pReturnedValueArray);
	
	for (i = 0; i < numEntries ; i++)
	{
		if ((pReturnedValueArray[i].ulPropTag & 0xFFFF0000) == (PR_CONTAINER_CLASS & 0xFFFF0000))
		{
			lpszContainerClass = pReturnedValueArray[i].Value.lpszA;
			if (pReturnedValueArray[i].ulPropTag	== 0x3613000a)
				lpszContainerClass = NULL;
		}
		if ((pReturnedValueArray[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_NAME & 0xFFFF0000))
		{
			lpszFolderName = pReturnedValueArray[i].Value.lpszA;
		}

	}

	if (lpszContainerClass == NULL)	// No container Class. Must be a mailbox.
	{
		m_MAPIFreeBuffer(pReturnedValueArray);
		return true;
	}
	else
	{
		if (!strcmp(lpszContainerClass, "IPF.Note"))	// But it's note.
		{
			m_MAPIFreeBuffer(pReturnedValueArray);
			return true;
		}
		m_MAPIFreeBuffer(pReturnedValueArray);
		return false;
	}
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// [PUBLIC] Called from the exported dll function to make the mime digest
/////////////////////////////////////////////////////////////////////////////
char * OLImportClass::MakeMimeWithFile(char *lpszFilename, void * pID, unsigned int iSize, void *pDB)
{

	m_fpoDigestFile = CreateDigestFile();
	if(!m_fpoDigestFile)
		return NULL;
	InitDigestFile();

	if(!MAPIProcessMailbox(iSize, (LPENTRYID)pID, lpszFilename, pDB))
	{
		return false;
	}

	return m_lpszDigestPath;

}

/////////////////////////////////////////////////////////////////////////////
// We process the messages in the given mailbox
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::MAPIProcessMailbox(ULONG cbEntryID, LPENTRYID lpEntryID, char *lpszMboxName, void *pDB)
{

	IMAPIFolder *lpFolder; 
	ULONG ftype;
	ULONG			ReturnedNumOfMsgs = 0;
	m_lpMDB = (LPMDB)pDB;

	HRESULT hresult = m_lpMDB->OpenEntry(cbEntryID, lpEntryID, NULL, 0, &ftype,(IUnknown**)&lpFolder);

	if (hresult==S_OK) { 

		if (ftype==MAPI_FOLDER) { 
			if(ContainsMessages(lpFolder, &ReturnedNumOfMsgs)) {
				GetMessages(lpFolder);
			}
		}
    
		lpFolder->Release();
	
	} 
	else
		return false;

	m_MAPIFreeBuffer(lpEntryID);
	return true;

}

/////////////////////////////////////////////////////////////////////////////
// Verifys if the folder/mailbox has messages in it
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ContainsMessages(LPMAPIPROP ParentFolder, ULONG * NumReturned)
{
	HRESULT				hresult = 0;
	LPSPropValue		pReturnedValue;
	ULONG				numEntries = 1;

	*NumReturned = 0;

	SizedSPropTagArray	(1, DesiredProps) =
    {
        1,
        {
            PR_CONTENT_COUNT
        }
    };

	hresult = ParentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL, &numEntries, &pReturnedValue);
	if (HR_FAILED(hresult))
		return false;

	if (pReturnedValue)
	{
		*NumReturned = pReturnedValue->Value.ul;
		m_MAPIFreeBuffer(pReturnedValue);
		return true;
	}
	else return false;
}

/////////////////////////////////////////////////////////////////////////////
// Gets messages and writes to a temp file prior to appending to digest
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::GetMessages(IMAPIFolder * CurrentFolder)
{
	HRESULT hresult;
	LPMAPITABLE lpTable;
	LPSRowSet		lpRowSet;
	SizedSPropTagArray	(10, MsgProps) =
	{
		10,
		{
			PR_ENTRYID,
			PR_MESSAGE_SIZE,
			PR_SUBJECT,
			PR_SENDER_NAME,
			PR_SENDER_EMAIL_ADDRESS,
			PR_HASATTACH,
			PR_CREATION_TIME,
			PR_LAST_MODIFICATION_TIME,
			PR_CLIENT_SUBMIT_TIME,
			PR_DISPLAY_TO
		}
	};
	LPIID           lpInterface;
	lpInterface = (LPIID)(&IID_IStream);

	hresult = CurrentFolder->GetContentsTable(0,&lpTable);
	if (hresult==S_OK) { 
		
		hresult = m_HrQueryAllRows(lpTable,(SPropTagArray*)&MsgProps,NULL,NULL,0,&lpRowSet);
		if (hresult==S_OK) {		

			for (unsigned int ix=0; ix<lpRowSet->cRows; ix++) {
				
				OpenTempFile();
				CommitRowToMbx(&(lpRowSet->aRow[ix]));
				CloseTempFile();
				WriteToDigestFile();
				m_MAPIFreeBuffer(&(lpRowSet->aRow[ix]));

				if (EscapePressed(TRUE))
					break;
			}	
		
		}

		m_MAPIFreeBuffer(lpRowSet);
		lpRowSet = NULL;	
	}
	
	lpTable->Release();
	m_MAPIFreeBuffer(lpTable);
	lpTable = NULL;
	WriteDigestEnd();
	DeleteTempFile();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Used to get only a line
/////////////////////////////////////////////////////////////////////////////
char * OLImportClass::GetLine(IStream * Stream, ULONG * bytesRead, ULONG *posn)
{
	char * pReturned;
	char * pTemp;
	char * pSrcFloater;
	HRESULT hresult;
	ULONG	NumBytesRead;
	int count = 0;
	LARGE_INTEGER SeekDisp;

	pTemp = DEBUG_NEW char[1025];
	pSrcFloater =pTemp;
	hresult = ((IStream *)Stream)->Read((void *)pTemp, 1024, &NumBytesRead); 
	if (HR_FAILED(hresult))
	{
		delete [] pTemp;
		return 0;
	}

	if (NumBytesRead == 0)
	{
		delete []pTemp;
		return 0;
	}

	while ((*pSrcFloater != '\r') && (count++ < 1022))
	{
		pSrcFloater++;
	}

	while ((*pSrcFloater == '\r') || (*pSrcFloater == '\n'))
	{
		pSrcFloater++;
		count++;
	}

	pReturned = DEBUG_NEW char[count+3];
	*posn += count;
	SeekDisp.LowPart = *posn;
	SeekDisp.HighPart = 0;

	hresult = ((IStream *)Stream)->Seek(SeekDisp, STREAM_SEEK_SET, NULL);
	if (HR_FAILED(hresult))
	{
		delete []pTemp;
		delete []pReturned;
		return 0;
	}

	pSrcFloater--;
	while ((*pSrcFloater == '\r') || (*pSrcFloater == '\n'))
	{
		pSrcFloater--;
		count--;
	}
	 pSrcFloater++;
	*pSrcFloater++ = '\r';
	*pSrcFloater++ = '\n';
	*pSrcFloater = 0;
	count += 2;

	*bytesRead = count;
	strcpy(pReturned, pTemp);
	delete [] pTemp;
	return (pReturned);
}

/////////////////////////////////////////////////////////////////////////////
// Adds message to temp file.
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::CommitRowToMbx(LPSRow lpRow)
{
	ULONG	msgType;
	LPMESSAGE openedMsg;
	LPENTRYID	EntryIDValue = NULL;
	ULONG	EntryIDSize = 0;
	ULONG	MessageBodySize = 0;
	char *	MessageSubject = NULL;
	char *	SenderName = NULL;
	char *	SenderEmail = NULL;
	char *	RcptEmail = NULL;
	char * Date = NULL;
	char *	lpszBoundary = NULL;
	bool	HasAttach;
	ULONG	bodyType = TEXT_PLAIN;		// 0 = plain/unknown, 1 = enriched, 2 = html, 3 = multipart
	char szBoundaryStuff[2048];

	ULONG i;
	for (i = 0; i < lpRow->cValues; i++)
	{
		if ((lpRow->lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ENTRYID & 0xFFFF0000))
		{
			EntryIDValue = (LPENTRYID)(lpRow->lpProps[i].Value.bin.lpb);
			EntryIDSize	= (ULONG)(lpRow->lpProps[i].Value.bin.cb);
		}
		else if((lpRow->lpProps[i].ulPropTag & 0xFFFF0000) == (PR_MESSAGE_SIZE & 0xFFFF0000))
		{
			MessageBodySize = (ULONG)(lpRow->lpProps[i].Value.ul);
		}
		else if(lpRow->lpProps[i].ulPropTag == PR_SUBJECT)
		{
			char * TempMessageSubject = DEBUG_NEW char[1024];
			strncpy(TempMessageSubject, (char *)(lpRow->lpProps[i].Value.lpszA), 1024);
			MessageSubject = DEBUG_NEW char[strlen(TempMessageSubject)+1];
			strcpy(MessageSubject, TempMessageSubject);
			delete [] TempMessageSubject;
		}
		else if(lpRow->lpProps[i].ulPropTag == PR_SENDER_NAME)
		{
			char * TempSenderName = DEBUG_NEW char[1024];
			strncpy(TempSenderName, (char *)(lpRow->lpProps[i].Value.lpszA), 1024);
			SenderName = DEBUG_NEW char[strlen(TempSenderName)+1];
			strcpy(SenderName, TempSenderName);
			delete [] TempSenderName;
		}
		else if(lpRow->lpProps[i].ulPropTag == PR_SENDER_EMAIL_ADDRESS)
		{
			char * TempSenderEmail = DEBUG_NEW char[1024];
			strncpy(TempSenderEmail, (char *)(lpRow->lpProps[i].Value.lpszA), 1024);
			SenderEmail = DEBUG_NEW char[strlen(TempSenderEmail)+1];
			strcpy(SenderEmail, TempSenderEmail);
			delete [] TempSenderEmail;
		}

		else if(lpRow->lpProps[i].ulPropTag == PR_HASATTACH)
		{
			HasAttach = lpRow->lpProps[i].Value.b != 0; // which is bool type?
		}
		else if (lpRow->lpProps[i].ulPropTag == PR_DISPLAY_TO)
		{
			char * TempRcptEmail = DEBUG_NEW char[1024];
			strncpy(TempRcptEmail, (char *)(lpRow->lpProps[i].Value.lpszA), 1024);
			RcptEmail = DEBUG_NEW char[strlen(TempRcptEmail)+1];
			strcpy(RcptEmail, TempRcptEmail);
			delete [] TempRcptEmail;
		}
		else if(lpRow->lpProps[i].ulPropTag == PR_CREATION_TIME)
		{

		}
		else if(lpRow->lpProps[i].ulPropTag == PR_LAST_MODIFICATION_TIME)
		{

		}
		else if(lpRow->lpProps[i].ulPropTag == PR_CLIENT_SUBMIT_TIME)
		{

		}
	}
	char * buf;
	if (SenderName && MessageSubject)
	{
		buf = DEBUG_NEW char[strlen(SenderName) +strlen(MessageSubject)+3];
		wsprintf(buf, "%s, %s",SenderName, MessageSubject);
	}
	else if (!SenderName && MessageSubject)
	{
		buf = DEBUG_NEW char[strlen(MessageSubject)+1];
		strcpy(buf, MessageSubject);
	}
	else if (!MessageSubject && SenderName)
	{
		buf = DEBUG_NEW char[strlen(SenderName)+1];
		strcpy(buf, SenderName);
	}	
	else
	{
		buf = DEBUG_NEW char[4];
		strcpy(buf, " , ");
	}

	if (strlen(buf) > 64)	
	{
		buf[61] = '.';
		buf[62] = '.';
		buf[63] = '.';
		buf[64] = 0;
	}

	delete [] buf;

	if (EntryIDValue)
	{	
		ULONG	NumBytesRead;
		LPUNKNOWN		TempIface;
		char * HeaderText;

		HRESULT	hresult;
		LPIID           lpInterface     = 0;
		lpInterface = (LPIID)(&IID_IStream);


		hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &msgType, (LPUNKNOWN*)&openedMsg);
		if (HR_FAILED(hresult))
		{
			return false;
		}

		hresult = ((IMAPIProp *) openedMsg)->OpenProperty(PR_TRANSPORT_MESSAGE_HEADERS, lpInterface, 0, NULL, &TempIface);
		if ((!(HR_FAILED(hresult))) && TempIface)
		{
			char MIMEheader[] = "Mime-Version:";
			char Contentheader[] = "Content-Type:";

			// Take off multipart mixed header.
			ULONG totalbytes=0;
			bool gotMimeContent = false;

			fwrite("From ???@??? Fri Apr 10 11:02:36 1998\r\n", sizeof(char), 39, m_tempfile); 

			while((HeaderText) = (GetLine((IStream *)TempIface, &NumBytesRead, &totalbytes)))
			{
				// IF we have attachment lets reuse their mime headers
			
				if(!HasAttach)
				{
					// otherwise we dont want them as they get in the way
					if ((_strnicmp(HeaderText, MIMEheader, sizeof(MIMEheader)-1) != 0) && (_strnicmp(HeaderText, Contentheader, sizeof(Contentheader)-1) != 0))
					{
						fwrite(HeaderText, sizeof(char), NumBytesRead, m_tempfile);
					}
				}
				else
				{
					char *floater= NULL;

					if ((HeaderText[0] != '\r') && (HeaderText[0] != '\n'))
					{
						if ((_strnicmp(HeaderText, Contentheader, sizeof(Contentheader)-1) == 0))
						{
							gotMimeContent = true;
						}
						if(gotMimeContent)
						{
							floater = strstr(HeaderText, "boundary=\"");
							if(floater)
							{
								floater += strlen("boundary=\"");
								lpszBoundary = DEBUG_NEW char[1024];
								strncpy(lpszBoundary, floater, (strlen(floater) - 3));
								lpszBoundary[strlen(floater) - 3] = 0;
								floater = &lpszBoundary[strlen(lpszBoundary) - 1];
								
								// sometimes we have boundary="x";
								// We really want an accurate boundary to find the pieces and not mem overflow
								while(true)
								{
									if(floater[0] == '\r' || floater[0] == '\n' || floater[0] == ';' || floater[0] == '\"')
										floater--;
									else
									{
										floater[1] = 0;
										break;
									}
								}
							}

						}
					}
					fwrite(HeaderText, sizeof(char), NumBytesRead, m_tempfile);
				}
				delete [] HeaderText;
			}
			TempIface->Release();
			m_MAPIFreeBuffer(TempIface);
			TempIface = NULL;
		}
		else
		{// We need to build our own headers.
			GenerateHeaders(SenderName, SenderEmail, Date, RcptEmail, MessageSubject);
		}
		delete [] SenderName;
		SenderName = NULL;
		delete [] SenderEmail;
		SenderEmail = NULL;
		delete [] Date;
		Date = NULL;
		delete [] RcptEmail;
		RcptEmail = NULL;
		delete [] MessageSubject;
		MessageSubject = NULL;
		fwrite("\r\n", sizeof(char), 2, m_tempfile);
	

		hresult = ((IMAPIProp *) openedMsg)->OpenProperty(PROP_TAG( PT_TSTRING,   0x1013), lpInterface, 0, NULL, &TempIface);
		if ((!(HR_FAILED(hresult))) && TempIface)
		{
			bodyType = TEXT_RICH;

			if(HasAttach)
			{
				sprintf(szBoundaryStuff, "\r\n--%s\r\n", lpszBoundary); 
				fwrite(szBoundaryStuff, sizeof(char), strlen(szBoundaryStuff), m_tempfile);
				AddContentHeader((IStream *)TempIface, &bodyType);
			}

			GrabMsgBody((IStream *)TempIface);



			TempIface->Release();
			m_MAPIFreeBuffer(TempIface);
			TempIface = NULL;
		}
		else
		{
			TempIface = NULL;
			hresult = ((IMAPIProp *) openedMsg)->OpenProperty(PR_BODY, lpInterface, 0, NULL, &TempIface);
			if ((!(HR_FAILED(hresult))) && TempIface)
			{
				bodyType = TEXT_PLAIN;
				if(HasAttach)
				{
					sprintf(szBoundaryStuff, "\r\n--%s\r\n", lpszBoundary); 
					fwrite(szBoundaryStuff, sizeof(char), strlen(szBoundaryStuff), m_tempfile);
					AddContentHeader((IStream *)TempIface, &bodyType);
				}

				GrabMsgBody((IStream *)TempIface);


				TempIface->Release();
				m_MAPIFreeBuffer(TempIface);
				TempIface = NULL;
			}
			else
			{	// There's apparently no body. Huh..
				bodyType = TEXT_PLAIN;

				fwrite(" \r\n", sizeof(char), 3, m_tempfile);	// just a space and a newline to give it a bit of a body...
				if (TempIface)
				{
					TempIface->Release();
					m_MAPIFreeBuffer(TempIface);
					TempIface = NULL;
				}
			}
		}

		// Attachment Stuff
		// Reading, encoding, writing
		LPMAPITABLE lpAttachTable;
		LPSRowSet	lpAttachRowSet;
		SizedSPropTagArray	(5, AttachProps) =
		{
			5,
			{
				PR_ATTACH_LONG_FILENAME,
				PR_ATTACH_NUM,
				PR_ATTACH_MIME_TAG,
				PR_STORE_ENTRYID,
				PR_ATTACH_DATA_BIN
			}
		};
		char * Filename = NULL;

		hresult = ((IMessage*) openedMsg)->GetAttachmentTable( NULL,	&lpAttachTable);
		if (!HR_FAILED(hresult))
		{
			hresult = lpAttachTable->SetColumns( (LPSPropTagArray) &AttachProps, 0);

			while (!(hresult = lpAttachTable->QueryRows(1,0, &lpAttachRowSet)))
			{
				if (lpAttachRowSet->cRows != 0)
				{
					LONG AttachNum = 0;
					char *lpszMimeHeader = NULL;

					for (i = 0; i < lpAttachRowSet->aRow[0].cValues; i++)
					{

						if ((lpAttachRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ATTACH_NUM & 0xFFFF0000))
						{
							AttachNum = (LONG)(lpAttachRowSet->aRow[0].lpProps[i].Value.l);
						}
						else if ((lpAttachRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ATTACH_LONG_FILENAME & 0xFFFF0000))
						{	
							if ((((ULONG)(lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA)) & 0xFFFF0000) != (0x8004010f & 0xFFFF0000))
							{
								Filename = DEBUG_NEW char[strlen(lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA)+1];
								strcpy(Filename, lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA);
							}
						}
						else 	
						if ((lpAttachRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ATTACH_MIME_TAG & 0xFFFF0000))
						{	
							if ((((ULONG)(lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA)) & 0xFFFF0000) != (0x8004010f & 0xFFFF0000))
							{
								lpszMimeHeader  = DEBUG_NEW char[strlen(lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA)+1];
								strcpy(lpszMimeHeader , lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA);
							}
						}

					}
					if (AttachNum && Filename && strlen(Filename) != 0)
					{
						LPATTACH	lpAttach = NULL;
						hresult = ((IMessage *)openedMsg)->OpenAttach(AttachNum, NULL, NULL, &lpAttach);
						if (!HR_FAILED(hresult))	
						{
							hresult = ((IMAPIProp *) lpAttach)->OpenProperty(PR_ATTACH_DATA_BIN, lpInterface, 0, NULL, &TempIface);
							if ((!(HR_FAILED(hresult))) && TempIface)
							{
								// Writing our MIME RFC  header
								char szContent[1024];
								sprintf(szBoundaryStuff, "\r\n--%s\r\n", lpszBoundary); 
								fwrite(szBoundaryStuff, sizeof(char), strlen(szBoundaryStuff), m_tempfile);
								sprintf(szContent, "Content-Type: %s; name=\"%s\";\r\n", lpszMimeHeader, Filename);
								fwrite(szContent, sizeof(char), strlen(szContent), m_tempfile);
								sprintf(szContent, "Content-Transfer-Encoding: base64\r\n");
								fwrite(szContent, sizeof(char), strlen(szContent), m_tempfile);
								sprintf(szContent, "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", Filename);
								fwrite(szContent, sizeof(char), strlen(szContent), m_tempfile);
							
								WriteAttachToMbx((IStream *)TempIface, Filename, lpszMimeHeader);

								TempIface->Release();
								m_MAPIFreeBuffer(TempIface);
								TempIface = NULL;
							}
							lpAttach->Release();
							m_MAPIFreeBuffer(lpAttach);
							lpAttach = NULL;
						}
					}

					if (Filename)
						delete [] Filename;
					if(lpszMimeHeader)
						delete [] lpszMimeHeader;
					lpszMimeHeader = NULL;
					Filename = NULL;
					FreeRowSet(m_MAPIFreeBuffer, lpAttachRowSet);
					lpRow = NULL;
				}
				else break;

				if (EscapePressed(TRUE))
					return false;
			}
			// Close the boundary is it has an attachment
			if(HasAttach)
			{
				sprintf(szBoundaryStuff, "\r\n--%s--\r\n", lpszBoundary); 
				fwrite(szBoundaryStuff, sizeof(char), strlen(szBoundaryStuff), m_tempfile);
			}
			lpAttachTable->Release();
			m_MAPIFreeBuffer(lpAttachTable);
			lpAttachTable = NULL;

			openedMsg->Release();
			m_MAPIFreeBuffer(openedMsg);
			openedMsg = NULL;
		}
	}
	else
	{// No entry ID? No message. Delete our allocated data...
		delete [] SenderName;
		delete [] SenderEmail;
		delete [] Date;
		delete [] RcptEmail;
		delete [] MessageSubject;
	}
	delete [] lpszBoundary;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Generates headers, if we were headerless
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::GenerateHeaders(char *SenderName, char *SenderEmail, char *Date, char *RcptEmail, char *MessageSubject)
{
	int chsize = sizeof(char);

	fwrite("From ???@??? Fri Apr 10 11:02:36 1998\r\n", chsize, 39, m_tempfile);
	fwrite("From: ", chsize, 6, m_tempfile);
	if (SenderName)
	{
		fwrite(SenderName, chsize, strlen(SenderName), m_tempfile);
	}
	if (SenderEmail)
	{
		fwrite(" <", chsize, 2, m_tempfile);
		fwrite(SenderEmail, chsize, strlen(SenderEmail), m_tempfile);
		fwrite(">\r\n", chsize, 3, m_tempfile);
	}
	else
	{
		fwrite("\r\n", chsize, 2, m_tempfile);
	}
	if (Date)
	{
		fwrite("Date: ", chsize, 6, m_tempfile);
		fwrite("\r\n", chsize, 2, m_tempfile);	
	}
	if (RcptEmail)
	{
		fwrite("To: ", chsize, 4, m_tempfile);
		fwrite(RcptEmail, chsize, strlen(RcptEmail), m_tempfile);
		fwrite("\r\n", chsize, 2, m_tempfile);	
	}
	if (MessageSubject)
	{
		fwrite("Subject: ", chsize, 9, m_tempfile);
		fwrite(MessageSubject, chsize, strlen(MessageSubject), m_tempfile);
		fwrite("\r\n",chsize, 2, m_tempfile);	
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Adds content type headers if necessary
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddContentHeader(IStream *Stream, ULONG *bodytype, bool bHasAttach)
{
	int chsize = sizeof(char);
//	fwrite("Mime-Version: 1.0\r\n", chsize, 19, m_tempfile);
	HRESULT hresult;
	ULONG numBytes=0;
	char * readBuffer = NULL;
	char * strReturns = NULL;
	LARGE_INTEGER SeekDisp;
	
	SeekDisp.LowPart = 0;
	SeekDisp.HighPart = 0;

	readBuffer = DEBUG_NEW char[1026];
	if (readBuffer == NULL)
		*bodytype = TEXT_PLAIN;
	else if (*bodytype != TEXT_PLAIN)
	{
		hresult = Stream->Read(readBuffer, 1024, &numBytes);
		if (HR_FAILED(hresult))
		{
			*bodytype = TEXT_PLAIN;
		}

		Stream->Seek(SeekDisp, STREAM_SEEK_SET, NULL);
		if (*bodytype != TEXT_PLAIN)
		{
			strReturns = strstr(readBuffer, "html>");
			if (strReturns == 0)
				*bodytype = TEXT_RICH;
			else 
				*bodytype = TEXT_HTML;
		}
	}

	if (readBuffer)
		delete [] readBuffer;

	if (*bodytype == TEXT_PLAIN)
	{
		fwrite("Content-Type: text/plain; charset=\"us-ascii\"\r\n\r\n", chsize, 48, m_tempfile);
	}
	else if (*bodytype == TEXT_RICH)
	{
		fwrite("Content-Type: text/enriched; charset=\"us-ascii\"\r\n\r\n<x-rich>\r\n", chsize, 61, m_tempfile);
	}
	else if (*bodytype == TEXT_HTML)
	{
		fwrite("Content-Type: text/html; charset=\"us-ascii\"\r\n\r\n<x-html>\r\n", chsize, 57, m_tempfile);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Gets just the body text
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::GrabMsgBody(IStream * BodyStream)
{
	ULONG numBytes;
	char *bodyChunk;
	HRESULT hresult;

	bodyChunk = DEBUG_NEW char[1025];
	hresult = BodyStream->Read(bodyChunk, 1024, &numBytes);
	if (!HR_FAILED(hresult) && numBytes != 0)
	{
		fwrite(bodyChunk, sizeof(char), numBytes, m_tempfile);
		GrabMsgBody(BodyStream);
	}
	
	delete [] bodyChunk;

	fwrite("\r\n", sizeof(char), 2, m_tempfile);	// Just in case someone forgot to end a msg with a CR.

	return (true);
}

/////////////////////////////////////////////////////////////////////////////
// Retrieves, encodes, and writes the attachment
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::WriteAttachToMbx(IStream * stream, char * name, char *lpszMimeHeader)
{
	ULONG numBytes;
	char *bodyChunk = NULL;
	char * floater = strrchr(name, '\\');
	floater++;



	bodyChunk = DEBUG_NEW_NOTHROW char[10000];
	if (bodyChunk == NULL)
	{
		return false;
	}
	Base64Encoder	TheEncoder;
	char tmp_Password[24000];
	char*			OutSpot = tmp_Password;
	LONG			OutLen = 0;

	TheEncoder.Init(tmp_Password, OutLen,0);
	OutSpot += OutLen;

	while(!HR_FAILED(stream->Read(bodyChunk, 1024, &numBytes)) && numBytes != 0)
	{
		
		TheEncoder.Encode(bodyChunk, numBytes, OutSpot, OutLen);
		fwrite(OutSpot, sizeof(char), OutLen, m_tempfile);

		
		if (EscapePressed(TRUE))
		{
			delete [] bodyChunk;
			return false;
		}
	}
	TheEncoder.Done(OutSpot, OutLen);
	fwrite(OutSpot, sizeof(char), OutLen, m_tempfile);

	delete [] bodyChunk;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Called from MakeMimeWithFile to create Digest File
/////////////////////////////////////////////////////////////////////////////
FILE * OLImportClass::CreateDigestFile()
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

/////////////////////////////////////////////////////////////////////////////
// Called from MakeMimeWithFile to Initiailize the member
// boundary variables and write out header info on the digest
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitDigestFile()
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

/////////////////////////////////////////////////////////////////////////////
// Called to write messages into the Digest file
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::WriteToDigestFile()
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

/////////////////////////////////////////////////////////////////////////////
// Called from MakeMimeWithFile to Write the end of the digest file
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::WriteDigestEnd()
{
	int chsize = sizeof(char);

	fwrite("--", chsize, 2, m_fpoDigestFile);
	fwrite(m_szMainBoundary, chsize, strlen(m_szMainBoundary), m_fpoDigestFile);
	fwrite("--", chsize, 2, m_fpoDigestFile);
	fwrite("\r\n", chsize, 2, m_fpoDigestFile);
	fclose(m_fpoDigestFile);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Called from MakeMimeWithFile primarily to open
// create/overwrite a temp file and set the member variable
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::OpenTempFile()
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

/////////////////////////////////////////////////////////////////////////////
// Closes our temp file member variable
// Called from lotsa places as well
// mainly MakeMimeWithFile to close it
// so we can overwrite it also called in
// such fns such as OELFtoCRLF to get stats on it
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::CloseTempFile()
{
	int err = fflush(m_tempfile);
	err = fclose(m_tempfile);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Deletes our Temp file member
// Called from a couple places like MakeMimeWithFile for cleanup
// and OELFtoCRLF to switch temp files
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteTempFile()
{

	if (m_TempFilePathname)
	{
		DeleteFile(m_TempFilePathname);

	}return true;
}

/////////////////////////////////////////////////////////////////////////////
// Was Escape just pressed?
/////////////////////////////////////////////////////////////////////////////
int	OLImportClass::EscapePressed(int Repost/* = FALSE*/)
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


/////////////////////////////////////////////////////////////////////////////
// Address Book stuff
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// called from the exported function to return the list of address books
// and the details of the messaging users and dist lists under each of the address books
// we even fetch address books only for the selected profile
/////////////////////////////////////////////////////////////////////////////
CAddressBookTree * OLImportClass::GetAddressTree(char *lpszPath, char * lpszProfileName)
{

	if(Convert(lpszProfileName))
		return m_pABookTree;
	else
		return NULL;

	return NULL;

}

/////////////////////////////////////////////////////////////////////////////
// called from GetAddressTree to actually fetch the address book tree for
// the given profile
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::Convert(char * lpszProfileName)
{

	// Log onto MAPI if possible
	if(!m_bMapiLoggedOn)
	{
		if(MAPILogon())
		{
			m_bMapiLoggedOn = true;
		}
		else
			return NULL;

	}

	// free up all used data structures if the user is importing consecutively
	// otherwise we will land up with old values
	FreeFolderABPersonalityTree();

	if (!MAPIInitialize(lpszProfileName))
		return false;

	if (!OpenAddressBook())
		return false;


	if (!ProcessABook())
	{
		return false;
	}

	CloseAddressBook();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// closes the address book after we are done
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::CloseAddressBook()
{
	m_lpABC->Release();
	m_MAPIFreeBuffer(m_lpABC);
	m_lpABC = NULL;
	m_lpAdrBook->Release();
	m_lpSession->Release();

}

/////////////////////////////////////////////////////////////////////////////
// open the address book hierarchy for the session... and recurse through
// the address book containers
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::OpenAddressBook()
{

	HRESULT			hresult;
	ULONG			ulcbEntryID = 0;
	LPENTRYID		lpEntryID = NULL;
	ULONG			ulObjType;

	if (!m_lpSession)
	{
		TCHAR szBuf[2048];

		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		
		if(!LoadString((HINSTANCE)m_hModule, IDS_IMPORT_ADDR_OPENING_ERROR, szBuf, 2048))
			strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
		
		MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);		

		if (m_lpAdrBook)

			return false;
	}
	hresult = m_lpSession->OpenAddressBook( NULL, NULL, 0, &m_lpAdrBook);
	if (HR_FAILED(hresult) || !m_lpAdrBook)
	{	
		TCHAR szBuf[2048];

		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		
		if(!LoadString((HINSTANCE)m_hModule, IDS_IMPORT_ADDR_OPENING_ERROR2, szBuf, 2048))
			strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
		
		MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);		
		return false;
	}

	hresult = m_lpAdrBook->OpenEntry(ulcbEntryID, lpEntryID, NULL, NULL, &ulObjType, (LPUNKNOWN*)&m_lpABC);
	if (HR_FAILED(hresult) || !m_lpABC)
	{		
		TCHAR szBuf[2048];

		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		
		if(!LoadString((HINSTANCE)m_hModule, IDS_IMPORT_ADDR_OPENING_ERROR4, szBuf, 2048))
			strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
		
		MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);		

		if (m_lpAdrBook)
			m_MAPIFreeBuffer(m_lpAdrBook);
		return false;
	}

	if(!RecurseABookHierarchy(m_lpABC, "Root"))
		return false;

	return true;

}

/////////////////////////////////////////////////////////////////////////////
// recurse through the address book hierarchy... actually there is no
// real hierarchy... OL creates view with just a root called "Outlook Address Book"
// and dumps all the address books under it... we get access to only this view
// and hence no info bout the hierarchy as such. the only reason i have this
// set up as a recursive fn is bcos we get some fake root earlier on that doesn't contain
// any addresses books. so we recurse till we get the actual root under which we have the 
// address books flattened out after which calls to ProcessABookContainer process the address book
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::RecurseABookHierarchy(LPABCONT lpABookCont, std::string lABookName) {

	LPMAPITABLE		lpTable = NULL;
	HRESULT			hresult = NULL;

	hresult =((IMAPIContainer *)lpABookCont)->GetHierarchyTable(NULL, &lpTable);

	if (hresult != S_OK) {
		// no hierachy in the container. maybe just an addressbook
		if(lpABookCont)
			ProcessABookContainer(lpABookCont, lABookName);
		return true;
	}
	
	// and query for all the rows
	SizedSPropTagArray(2, cols) = {2, {PR_ENTRYID, PR_DISPLAY_NAME} };
	SRowSet *rows;
	hresult = m_HrQueryAllRows(lpTable,(SPropTagArray*)&cols, NULL, NULL, 0, &rows);
	lpTable->Release();
	
	if (hresult!=S_OK) {
		m_MAPIFreeBuffer(rows); 
		return false;
	}

	for (unsigned int i=0; i<rows->cRows; i++) {

		IUnknown *ab; 
		ULONG abtype;

		string name(rows->aRow[i].lpProps[1].Value.lpszA);

		hresult = lpABookCont->OpenEntry(rows->aRow[i].lpProps[0].Value.bin.cb,
								(LPENTRYID)rows->aRow[i].lpProps[0].Value.bin.lpb, NULL,
								0, &abtype, &ab);
		if (hresult==S_OK) { 
			if (abtype == MAPI_ABCONT) {
				//recurse throught the address book container
				RecurseABookHierarchy((LPABCONT)ab, name);
			}
		}
	}

	return true;

}

/////////////////////////////////////////////////////////////////////////////
// function processes the address book container and all the messaging users
// and distribution lists under the address book
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ProcessABookContainer(LPABCONT lpABookCont, std::string lABookName)
{
	HRESULT			hresult;
	LPMAPITABLE		lpContTable;
	LPSPropTagArray	lpPropTagArray;
	LPSRowSet		lpRowSet;
	SizedSPropTagArray	(2, AbookProps) =
	{
		2,
		{
			PR_OBJECT_TYPE,
			PR_ENTRYID
		}
	};

	LPENTRYID	EntryIDValue = NULL;
	ULONG		EntryIDSize = 0;
	ULONG		EntryObjectType = 0;
	ULONG		i;

	ULONG		ABType;
	LPMAPIPROP	ABEntry;
	LPDISTLIST	ABDistList;

	hresult = lpABookCont->GetContentsTable(NULL, &lpContTable);
	if (HR_FAILED(hresult))
	{
		return false;
	}

	hresult = lpContTable->QueryColumns(NULL, &lpPropTagArray);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_MAPIFreeBuffer(lpContTable);
		return false;
	}

	hresult = lpContTable->SetColumns((LPSPropTagArray)&AbookProps, NULL);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_MAPIFreeBuffer(lpContTable);
		m_MAPIFreeBuffer(lpPropTagArray);
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
				else if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_OBJECT_TYPE & 0xFFFF0000))
				{
					EntryObjectType = (ULONG)lpRowSet->aRow[0].lpProps[i].Value.ul;
				}
			}
			
			// processing to do if it is just a single messaging user
			if ((EntryIDValue) && (EntryObjectType == MAPI_MAILUSER))
			{
				hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &ABType, (LPUNKNOWN*)&ABEntry);
				if (HR_FAILED(hresult))
				{
					if (lpContTable)
					{
						lpContTable->Release();
						m_MAPIFreeBuffer(lpContTable);
					}
					if (lpPropTagArray)
						m_MAPIFreeBuffer(lpPropTagArray);
						FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						return (false);
				}

				//getting the long term identifier
				SPropValue *eid=0;
	
				SizedSPropTagArray(1, cols) = { 1, {PR_ENTRYID}};
				ULONG pcount;
			
				hresult = ABEntry->GetProps((SPropTagArray*)&cols,0,&pcount,&eid);

				hresult = ProcessABEntry(ABEntry, eid->Value.bin.lpb, eid->Value.bin.cb);
				ABEntry->Release();
				m_MAPIFreeBuffer(&ABEntry);
				ABEntry = NULL;
			}

			// processing to do if it is just a distribution list
			if ((EntryIDValue) && (EntryObjectType == MAPI_DISTLIST))
			{
				hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &ABType, (LPUNKNOWN*)&ABDistList);
				if (HR_FAILED(hresult))
				{
					if (lpContTable)
					{
						lpContTable->Release();
						m_MAPIFreeBuffer(lpContTable);
					}
					if (lpPropTagArray)
						m_MAPIFreeBuffer(lpPropTagArray);
						FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						return (false);
				}

				//getting the long term identifier
				SPropValue *eid=0;
	
				SizedSPropTagArray(1, cols) = { 1, {PR_ENTRYID}};
				ULONG pcount;
			
				hresult = ABDistList->GetProps((SPropTagArray*)&cols,0,&pcount,&eid);

				hresult = ProcessABDistListEntry(ABDistList, eid->Value.bin.lpb, eid->Value.bin.cb);
				ABDistList->Release();
				m_MAPIFreeBuffer(&ABDistList);
				ABDistList = NULL;
			}
			
			FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
			if (lpPropTagArray)
				m_MAPIFreeBuffer(lpPropTagArray);
			lpRowSet = NULL;
		}
		else break;
	}  

	//add the address book to the linked list of address books
	CAddressBookTree * pEntry;
	pEntry = DEBUG_NEW CAddressBookTree;
	InitAddrTree(pEntry);
	pEntry->lpszDisplayName = DEBUG_NEW char[strlen(lABookName.c_str()) + 1];
	strcpy(pEntry->lpszDisplayName, lABookName.c_str());
	pEntry->pFirstChild = m_pABook;
	pEntry->pFirstDLChild = m_pABookDistListTree;
	AddABookToTree(pEntry);
	m_pABook = NULL;				// the entire tree is deleted later... just set to NULL for now to get new values
	m_pABookDistListTree = NULL;	// the entire tree is deleted later... just set to NULL for now to get new values
	delete pEntry;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// now we have some problems with distribution lists. 
// problems - a distribution list can contain messaging users not just from its own 
// address book but from other address books under the profle. 
// further Microsoft Outlook allows for having multiple distribution lists
// and messaging users with the same nicknames... Eudora prevents using dupliacate
// nicknames so we make each of the nicknames unique by appending a number to the end
// in the main Eudora app. How are we to know the nickname that is going to be created
// further down the road? So initially when the distribution list is getting
// created will contain a list of Entry IDs of the messaging users and distribution lists 
// within it. BTW Entry IDs are the unique identifier in MAPI to access any kid of object.
// In this function we do passes into the address book tree and convert all
// the Entry IDs in the distribution list we have into pointers to the respective node
// in the tree. we cannot have stored the pointer to the node directly in the distribution list
// earlier on bcos we cannot guarantee that we would have come across the messaging user/dist list
// it contains and that their nodes have already been created. Hence when we get a distribution list entry we
// open all the entries it contains and store a list of the Entry IDs. Once the entire
// list is built we make passes through the address book tree and actually convert the
// Entry IDs to pointers to that specific messaging user/dist list node. This has the advantage that
// when we do create the unique nicknames within the main Eudora app, the dist list 
// nicknames will get automatically updated
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ProcessABook()
{

	HRESULT	hresult = NULL;

	// this function processes the address book structure and converts the
	// the entry ids in each node to pointers to the respective entry id nodes 
	// of the address book

	CAddressBookTree *pABookIterator =	m_pABookTree;

	while (pABookIterator) {

		CAbookDistListTree *pDistListTreeIterator = pABookIterator->pFirstDLChild;

		while(pDistListTreeIterator) {
			
			CAbookDistListEntry *pTmpDistList = pDistListTreeIterator->pFirstDLChild;
			while (pDistListTreeIterator->pFirstDLChild) {

					//getting the ID, size and type of the entry id to find in the AB structure
					ENTRYID *nodeID = (ENTRYID *)pDistListTreeIterator->pFirstDLChild->pID;
					ULONG nodeSize = pDistListTreeIterator->pFirstDLChild->IDSize;
					AddressType nodeType = pDistListTreeIterator->pFirstDLChild->entryType;

					// get another pointer to the AB structure to loop through it and get the 
					// node whose ID matched the nodeID we are looking for
					//store the pointer to the found node
					CAddressBookTree *pABTreeIterator =	m_pABookTree;

					while(pABTreeIterator) {

						bool flag = false;

						if(nodeType == MESSAGING_USER) {

							CAbookEntry *pABEntryIterator = pABTreeIterator->pFirstChild;
							while (pABEntryIterator) {

								ENTRYID *ID = (ENTRYID *)pABEntryIterator->pID;
								ULONG size = pABEntryIterator->IDSize;

								ULONG result;

								m_lpSession->CompareEntryIDs(nodeSize, nodeID, size, ID, 0, &result);
								
								if (hresult != S_OK) 
									return false;
							    
								if (result !=0 ) {
									//the entry IDs are the same
									pDistListTreeIterator->pFirstDLChild->pABNode = pABEntryIterator;
									flag = true;
									break;
								}

								pABEntryIterator = pABEntryIterator->pNext;
							} // loop through the mail user linked list

						} else if(nodeType == DIST_LIST) {

							CAbookDistListTree *pDLTreeIterator = pABTreeIterator->pFirstDLChild;
							while (pDLTreeIterator) {

								ENTRYID *ID = (ENTRYID *)pDLTreeIterator->pID;
								ULONG size = pDLTreeIterator->IDSize;

								ULONG result;

								m_lpSession->CompareEntryIDs(nodeSize, nodeID, size, ID, 0, &result);
								
								if (hresult != S_OK) 
									return false;
							    
								if (result !=0 ) {
									//the entry IDs are the same
									pDistListTreeIterator->pFirstDLChild->pABNode = pDLTreeIterator;
									flag = true;
									break;
								}

								pDLTreeIterator = pDLTreeIterator->pNext;
							} // loop through the dist list linked list

						}

						if(flag)
							break;

						pABTreeIterator = pABTreeIterator->pNext;
					} // loop through the entire AB tree

				pDistListTreeIterator->pFirstDLChild = pDistListTreeIterator->pFirstDLChild->pNext;
			} // loop through each dist list entry
			pDistListTreeIterator->pFirstDLChild = pTmpDistList;

			pDistListTreeIterator = pDistListTreeIterator->pNext;
		} // loop through the dist list tree

		pABookIterator = pABookIterator->pNext;
	} // loop through the address book tree

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// this function processes a distribution list entry
// it opens all the entries within a distribution list
// and stores the Entry IDs of each of the messaging user/dist list it contains
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ProcessABDistListEntry(LPDISTLIST lpABDistList, void *pID, ULONG IDSize)
{
	HRESULT			hresult;
	LPMAPITABLE		lpContTable;
	LPSPropTagArray	lpPropTagArray;
	LPSRowSet		lpRowSet;
	SizedSPropTagArray	(2, AbookProps) =
	{
		2,
		{
			PR_OBJECT_TYPE,
			PR_ENTRYID
		}
	};

	LPENTRYID	EntryIDValue = NULL;
	ULONG		EntryIDSize = 0;
	ULONG		EntryObjectType = 0;
	ULONG		i;

	ULONG		ABType;
	LPMAPIPROP	ABEntry;
	LPDISTLIST	ABDistList;

	hresult = lpABDistList->GetContentsTable(NULL, &lpContTable);
	if (HR_FAILED(hresult))
	{
		return false;
	}

	hresult = lpContTable->QueryColumns(NULL, &lpPropTagArray);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_MAPIFreeBuffer(lpContTable);
		return false;
	}

	hresult = lpContTable->SetColumns((LPSPropTagArray)&AbookProps, NULL);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_MAPIFreeBuffer(lpContTable);
		m_MAPIFreeBuffer(lpPropTagArray);
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
				else if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_OBJECT_TYPE & 0xFFFF0000))
				{
					EntryObjectType = (ULONG)lpRowSet->aRow[0].lpProps[i].Value.ul;
				}
			}
			
			if ((EntryIDValue) && (EntryObjectType == MAPI_MAILUSER))
			{
				// Note: the entry-ids returned by the list are just short-term list-specific
				// entry-ids. But we want to put long-term entry-ids in our linked list.
				// That's why it's necessary to open the address book entry...

				hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &ABType, (LPUNKNOWN*)&ABEntry);
				if (HR_FAILED(hresult))
				{
					if (lpContTable)
					{
						lpContTable->Release();
						m_MAPIFreeBuffer(lpContTable);
					}
					if (lpPropTagArray)
						m_MAPIFreeBuffer(lpPropTagArray);
						FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						return (false);
				}

				SPropValue *eid=0;
	
				SizedSPropTagArray(1, cols) = { 1, {PR_ENTRYID}};
				ULONG pcount;
			
				hresult = ABEntry->GetProps((SPropTagArray*)&cols,0,&pcount,&eid);

				if(hresult != S_OK) {
					m_MAPIFreeBuffer(eid);
					return false;
				}
				
				//process the entry to create the distribution list
				//add the mailing user to the linked list of objects in a dist list

				CAbookDistListEntry * pEntry;
				pEntry = DEBUG_NEW CAbookDistListEntry;
				InitAddrEntry(pEntry);
				pEntry->pID = eid->Value.bin.lpb;
				pEntry->IDSize = eid->Value.bin.cb;
				pEntry->entryType = MESSAGING_USER;
				pEntry->pABNode = NULL;
				AddEntryToDistList(pEntry);
				delete pEntry;
								
				ABEntry->Release();
				m_MAPIFreeBuffer(&ABEntry);
				ABEntry = NULL;
			}

			if ((EntryIDValue) && (EntryObjectType == MAPI_DISTLIST))
			{
				hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &ABType, (LPUNKNOWN*)&ABDistList);
				if (HR_FAILED(hresult))
				{
					if (lpContTable)
					{
						lpContTable->Release();
						m_MAPIFreeBuffer(lpContTable);
					}
					if (lpPropTagArray)
						m_MAPIFreeBuffer(lpPropTagArray);
						FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						return (false);
				}

				SPropValue *eid=0;
	
				SizedSPropTagArray(1, cols) = { 1, {PR_ENTRYID}};
				ULONG pcount;
			
				hresult = ABDistList->GetProps((SPropTagArray*)&cols,0,&pcount,&eid);
				
				//process the entry to create the distribution list
				//add the dist list to the linked list of objects in a dist list
				CAbookDistListEntry * pEntry;
				pEntry = DEBUG_NEW CAbookDistListEntry;
				InitAddrEntry(pEntry);
				pEntry->pID = eid->Value.bin.lpb;
				pEntry->IDSize = eid->Value.bin.cb;
				pEntry->entryType = DIST_LIST;
				pEntry->pABNode = NULL;
				AddEntryToDistList(pEntry);
				delete pEntry;				


				ABDistList->Release();
				m_MAPIFreeBuffer(&ABDistList);
				ABDistList = NULL;
			}

			
			FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
			if (lpPropTagArray)
				m_MAPIFreeBuffer(lpPropTagArray);
			lpRowSet = NULL;
		}
		else break;
	}  

	//add the entry to the address book m_pABook
	//add the address book to the linked list of address books
	CAbookDistListTree * pEntry;
	pEntry = DEBUG_NEW CAbookDistListTree;
	InitAddrTree(pEntry);
	pEntry->pID = pID;
	pEntry->IDSize = IDSize;

	SPropValue *nick_eid=0;
	
	SizedSPropTagArray(1, col) = { 1, {PR_DISPLAY_NAME}};
	ULONG count;
			
	hresult = lpABDistList->GetProps((SPropTagArray*)&col,0,&count,&nick_eid);
	if(hresult != S_OK) {
		return false;
		m_MAPIFreeBuffer(nick_eid);
	}
	pEntry->pNickname = DEBUG_NEW char[strlen(nick_eid->Value.lpszA) + 1];
	strcpy(pEntry->pNickname, nick_eid->Value.lpszA);
	m_MAPIFreeBuffer(nick_eid);
	
	pEntry->pFirstDLChild = m_pABookDistList;
	AddEntryToDistListTree(pEntry);
	m_pABookDistList = NULL;		// the entire tree is deleted later... just set to NULL for now to get new values
	delete pEntry;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// process an individual messaging user entry. get all the info of the entry 
// and add entry to the address book
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ProcessABEntry(LPMAPIPROP ABEntry, void * pEntryIDValue, ULONG lEntryIDSize)
{
	HRESULT hresult;
	LPSPropTagArray	AllPropValues;

	CAbookEntry *pEntry;
	pEntry = DEBUG_NEW CAbookEntry;

	InitAddrEntry(pEntry);

	pEntry->pID  = pEntryIDValue;
	pEntry->IDSize = lEntryIDSize;
	
	ULONG			i;
	ULONG			NumProps;
	LPSPropValue	PropValues;


	hresult = ABEntry->GetPropList(NULL, &AllPropValues);

	hresult = ABEntry->GetProps(AllPropValues, NULL, &NumProps, &PropValues);

	for (i = 0; i <= NumProps; i++)
	{
		switch(PropValues[i].ulPropTag)
		{

		case PR_COMMENT:
			{
				pEntry->notes = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->notes, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COMMENT_W:
			{
				pEntry->notes = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->notes, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}

		case PR_GIVEN_NAME:
			{
				pEntry->first_name = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->first_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_GIVEN_NAME_W:
			{
				pEntry->first_name = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->first_name, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );

				break;
			}

		case PR_BUSINESS_TELEPHONE_NUMBER:
			{
				pEntry->wrk_phone = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_TELEPHONE_NUMBER_W:
			{
				pEntry->wrk_phone = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_phone, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_TELEPHONE_NUMBER:
			{
				pEntry->phone = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_TELEPHONE_NUMBER_W:
			{
				pEntry->phone = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->phone, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_SURNAME:
			{
				pEntry->last_name = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->last_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_SURNAME_W:
			{
				pEntry->last_name = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->last_name, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_COMPANY_NAME:
			{
				pEntry->company = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->company, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COMPANY_NAME_W:
			{
				pEntry->company = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->company, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_TITLE:
			{
				pEntry->wrk_title = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_title, PropValues[i].Value.lpszA);
				break;
			}
		case PR_TITLE_W:
			{
				pEntry->wrk_title = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_title, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_DEPARTMENT_NAME:
			{
				pEntry->wrk_dept = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_dept, PropValues[i].Value.lpszA);
				break;
			}
		case PR_DEPARTMENT_NAME_W:
			{
				pEntry->wrk_dept = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_dept, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_OFFICE_LOCATION:
			{
				pEntry->wrk_office = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_office, PropValues[i].Value.lpszA);
				break;
			}
		case PR_OFFICE_LOCATION_W:
			{
				pEntry->wrk_office = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_office, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_MOBILE_TELEPHONE_NUMBER:
			{
				pEntry->cell_phone = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->cell_phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_MOBILE_TELEPHONE_NUMBER_W:
			{
				pEntry->cell_phone = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->cell_phone, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_PAGER_TELEPHONE_NUMBER:
			{
				pEntry->wrk_pgr = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_pgr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_PAGER_TELEPHONE_NUMBER_W:
			{
				pEntry->wrk_pgr = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_pgr, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_BUSINESS_FAX_NUMBER:
			{
				pEntry->wrk_fax = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_fax, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_FAX_NUMBER_W:
			{
				pEntry->wrk_fax = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_fax, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_FAX_NUMBER:
			{
				pEntry->fax = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->fax, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_FAX_NUMBER_W:
			{
				pEntry->fax = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->fax, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_COUNTRY:
			{
				pEntry->wrk_country = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_country, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COUNTRY_W:
			{
				pEntry->wrk_country = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_country, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_LOCALITY:
			{
				pEntry->wrk_city = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_city, PropValues[i].Value.lpszA);
				break;
			}
		case PR_LOCALITY_W:
			{
				pEntry->wrk_city = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_city, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_STATE_OR_PROVINCE:
			{
				pEntry->wrk_state = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_state, PropValues[i].Value.lpszA);
				break;
			}
		case PR_STATE_OR_PROVINCE_W:
			{
				pEntry->wrk_state = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_state, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_STREET_ADDRESS:
			{
				pEntry->wrk_street_addr = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_street_addr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_STREET_ADDRESS_W:
			{
				pEntry->wrk_street_addr = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_street_addr, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_POSTAL_CODE:
			{
				pEntry->wrk_zip = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_zip, PropValues[i].Value.lpszA);
				break;
			}
		case PR_POSTAL_CODE_W:
			{
				pEntry->wrk_zip = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_zip, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_MIDDLE_NAME:
			{
				pEntry->middle_name = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->middle_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_MIDDLE_NAME_W:
			{
				pEntry->middle_name = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->middle_name, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_NICKNAME:
			{
				pEntry->nickname = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->nickname, PropValues[i].Value.lpszA);
				break;
			}
		case PR_NICKNAME_W:
			{
				pEntry->nickname = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->nickname, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_PERSONAL_HOME_PAGE:
			{
				pEntry->web_page = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->web_page, PropValues[i].Value.lpszA);
				break;
			}
		case PR_PERSONAL_HOME_PAGE_W:
			{
				pEntry->web_page = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->web_page, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_BUSINESS_HOME_PAGE:
			{
				pEntry->wrk_web_page = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->wrk_web_page, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_HOME_PAGE_W:
			{
				pEntry->wrk_web_page = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->wrk_web_page, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_EMAIL_ADDRESS:
			{
				pEntry->default_add = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->default_add, PropValues[i].Value.lpszA);

				break;
			}
		case PR_EMAIL_ADDRESS_W:
			{
				pEntry->default_add = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->default_add, 
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

				pEntry->addresses = DEBUG_NEW_NOTHROW char[totalSize];
				if (pEntry->addresses == NULL)	// If it's too big, just screw it.
					break;

				pEntry->addresses[0] = 0;

				for (j = 0; j < count; j++)
				{
					curAddr = DEBUG_NEW char[strlen(PropValues[i].Value.MVszA.lppszA[j])+1];
					strcpy(curAddr, (PropValues[i].Value.MVszA.lppszA[j]));
					strcat(pEntry->addresses, curAddr);
					strcat(pEntry->addresses, (","));
					delete [] curAddr;
				}
 
				floater = strrchr(pEntry->addresses, ',');

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

				pEntry->addresses = DEBUG_NEW_NOTHROW char[totalSize];
				if (pEntry->addresses == NULL)	// If it's too big, just screw it.
					break;

				pEntry->addresses[0] = 0;

				for (j = 0; j < count; j++)
				{
					int tmplen = wcslen(PropValues[i].Value.MVszW.lppszW[j])+1;
					curAddr = DEBUG_NEW wchar_t[tmplen];
					tmpbuf = DEBUG_NEW char[tmplen];
					wcscpy(curAddr, (PropValues[i].Value.MVszW.lppszW[j]));
					
					WideCharToMultiByte( CP_ACP, 0, curAddr, -1,
									tmpbuf, 
									tmplen,NULL, NULL );
					
					strcat(pEntry->addresses, tmpbuf);
					strcat(pEntry->addresses, (","));
					delete [] curAddr;
					delete [] tmpbuf;
				}
 
				floater = strrchr(pEntry->addresses, ',');

				*floater = 0;
				break;
			}
		case PR_HOME_ADDRESS_CITY:
			{
				pEntry->city = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->city, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_CITY_W:
			{
				pEntry->city = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->city, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_COUNTRY:
			{
				pEntry->country = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->country, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_COUNTRY_W:
			{
				pEntry->country = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->country, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_POSTAL_CODE:
			{
				pEntry->zip = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->zip, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_POSTAL_CODE_W:
			{
				pEntry->zip = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->zip, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
			{
				pEntry->state = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->state, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_STATE_OR_PROVINCE_W:
			{
				pEntry->state = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->state, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		case PR_HOME_ADDRESS_STREET:
			{
				pEntry->street_addr = DEBUG_NEW char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(pEntry->street_addr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_STREET_W:
			{
				pEntry->street_addr = DEBUG_NEW char[wcslen(PropValues[i].Value.lpszW)+1];
				WideCharToMultiByte( CP_ACP, 0, PropValues[i].Value.lpszW, -1,
									(LPSTR)pEntry->street_addr, 
									wcslen(PropValues[i].Value.lpszW)+1,NULL, NULL );
				break;
			}
		default:
			{
				break;
			}
		}
	}
	

	AddEntryToBook(pEntry);
	delete pEntry;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// initializes a messaging user entry
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitAddrEntry(CAbookEntry *pEntry)
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

/////////////////////////////////////////////////////////////////////////////
// deletes a messaging user entry
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrEntry(CAbookEntry *pEntry)
{
	m_MAPIFreeBuffer(pEntry->pID);

	if(pEntry->first_name)
		delete [] pEntry->first_name;
	if(pEntry->middle_name)
		delete [] pEntry->middle_name;
	if(pEntry->last_name)
		delete [] pEntry->last_name;
	if(pEntry->title)
		delete [] pEntry->title;
	if(pEntry->default_add)
		delete [] pEntry->default_add;
	if(pEntry->addresses)
		delete [] pEntry->addresses;
	if(pEntry->street_addr)
		delete [] pEntry->street_addr;
	if(pEntry->city)
		delete [] pEntry->city;
	if(pEntry->state)
		delete [] pEntry->state;
	if(pEntry->zip)
		delete [] pEntry->zip;
	if(pEntry->country)
		delete [] pEntry->country;
	if(pEntry->phone)
		delete [] pEntry->phone;
	if(pEntry->fax)
		delete [] pEntry->fax;
	if(pEntry->cell_phone)
		delete [] pEntry->cell_phone;
	if(pEntry->web_page)
		delete [] pEntry->web_page;
	if(pEntry->company)
		delete [] pEntry->company;
	if(pEntry->wrk_street_addr)
		delete [] pEntry->wrk_street_addr;
	if(pEntry->wrk_city)
		delete [] pEntry->wrk_city;
	if(pEntry->wrk_state)
		delete [] pEntry->wrk_state;
	if(pEntry->wrk_zip)
		delete [] pEntry->wrk_zip;
	if(pEntry->wrk_country)
		delete [] pEntry->wrk_country;
	if(pEntry->wrk_title)
		delete [] pEntry->wrk_title;
	if(pEntry->wrk_dept)
		delete [] pEntry->wrk_dept;
	if(pEntry->wrk_office)
		delete [] pEntry->wrk_office;
	if(pEntry->wrk_phone)
		delete [] pEntry->wrk_phone;
	if(pEntry->wrk_fax)
		delete [] pEntry->wrk_fax;
	if(pEntry->wrk_pgr)
		delete [] pEntry->wrk_pgr;
	if(pEntry->wrk_ip_phone)
		delete [] pEntry->wrk_ip_phone;
	if(pEntry->wrk_web_page)
		delete [] pEntry->wrk_web_page;
	if(pEntry->spouse)
		delete [] pEntry->spouse;
	if(pEntry->children)
		delete [] pEntry->children;
	if(pEntry->gender)
		delete [] pEntry->gender;
	if(pEntry->birthday)
		delete [] pEntry->birthday;
	if(pEntry->anniversary)
		delete [] pEntry->anniversary;
	if(pEntry->notes)
		delete [] pEntry->notes;
	if(pEntry->conf_server)
		delete [] pEntry->conf_server;
	if(pEntry->def_conf_add)
		delete [] pEntry->def_conf_add;
	if(pEntry->conf_addresses)
		delete [] pEntry->conf_addresses;
	if(pEntry->pNext)
		delete pEntry->pNext;
	return true;

}

/////////////////////////////////////////////////////////////////////////////
// deletes an address book entry
// address book contains lists of both messaging users and dist lists
// make calls to respective fns to handle freeing them up
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrEntry(CAddressBookTree *pEntry)
{
	if(pEntry->lpszDisplayName)
		delete [] pEntry->lpszDisplayName;
	DeleteAddrTree(&pEntry->pFirstChild);
	DeleteAddrTree(&pEntry->pFirstDLChild);
	if(pEntry->pNext)
		delete pEntry->pNext;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// initiates a delete to the entire dist list tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrEntry(CAbookDistListTree *pEntry)
{
	DeleteAddrTree(&pEntry->pFirstDLChild);
	m_MAPIFreeBuffer(pEntry->pID);
	if(pEntry->pNext)
		delete pEntry->pNext;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deletes an individual entry in the dist list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrEntry(CAbookDistListEntry *pEntry)
{
	m_MAPIFreeBuffer(pEntry->pID);
	if(pEntry->pNext)
		delete pEntry->pNext;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// adds an entry to the current messaging users list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddEntryToBook(CAbookEntry *pEntry)
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

/////////////////////////////////////////////////////////////////////////////
// adds an entry to an individual dist list entry...
// remember a dist list itself is a linked list of entries (other messaging users and dist lists)
// and then each of these dist list entries make up the dist list tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddEntryToDistList(CAbookDistListEntry *pEntry)
{
	CAbookDistListEntry *pWalker;

	pWalker = m_pABookDistList;

	if(pWalker == NULL)
	{
		m_pABookDistList = DEBUG_NEW CAbookDistListEntry;
		pWalker = m_pABookDistList;
		InitAddrEntry(pWalker);
		pWalker->pNext = NULL;
	}
	else
	{
		while (pWalker->pNext != NULL)
		{
			pWalker = pWalker->pNext;
		}

		pWalker->pNext = DEBUG_NEW CAbookDistListEntry;
		pWalker = pWalker->pNext;
		InitAddrEntry(pWalker);
		pWalker->pNext = NULL;

	}
	*pWalker = *pEntry;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// adds an entry to the dist list tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddEntryToDistListTree(CAbookDistListTree *pEntry)
{
	CAbookDistListTree *pWalker;

	pWalker = m_pABookDistListTree;

	if(pWalker == NULL)
	{
		m_pABookDistListTree = DEBUG_NEW CAbookDistListTree;
		pWalker = m_pABookDistListTree;
		InitAddrTree(pWalker);
		pWalker->pNext = NULL;
	}
	else
	{
		while (pWalker->pNext != NULL)
		{
			pWalker = pWalker->pNext;
		}

		pWalker->pNext = DEBUG_NEW CAbookDistListTree;
		pWalker = pWalker->pNext;
		InitAddrTree(pWalker);
		pWalker->pNext = NULL;

	}
	*pWalker = *pEntry;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// initializes the dist list tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitAddrTree(CAbookDistListTree *pEntry)
{
	pEntry->pID				= NULL;
	pEntry->IDSize			= NULL;
	pEntry->pNickname		= NULL;
	pEntry->pFirstDLChild	= NULL;
	pEntry->pNext			= NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// initializes an individual dist list entry
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitAddrEntry(CAbookDistListEntry *pEntry)
{
	pEntry->pID			= NULL;
	pEntry->IDSize		= NULL;
	pEntry->pABNode		= NULL;
	pEntry->pNext		= NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// initializes the entire address book tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitAddrTree(CAddressBookTree *pEntry)
{
	pEntry->lpszDisplayName		= NULL;
	pEntry->pFirstChild			= NULL;
	pEntry->pFirstDLChild		= NULL;
	pEntry->pNext				= NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// we have multiple address books and build a linked list out of them
// this one adds an address book to the list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddABookToTree(CAddressBookTree *pEntry) 
{

	CAddressBookTree * pWalker;

	pWalker = m_pABookTree;

	if(pWalker == NULL)
	{
		m_pABookTree = DEBUG_NEW CAddressBookTree;
		pWalker = m_pABookTree;
		InitAddrTree(pWalker);
		pWalker->pNext = NULL;
	}
	else
	{
		while (pWalker->pNext != NULL)
		{
			pWalker = pWalker->pNext;
		}

		pWalker->pNext = DEBUG_NEW CAddressBookTree;
		pWalker = pWalker->pNext;
		InitAddrTree(pWalker);
		pWalker->pNext = NULL;

	}
	*pWalker = *pEntry;

	return true;

}

/////////////////////////////////////////////////////////////////////////////
// deletes a dist list entry
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrTree(CAbookDistListEntry **Tree)
{

	if (Tree == NULL)
		Tree = &m_pABookDistList;

	if (*Tree == NULL)
		return true;
	
	CAbookDistListEntry *trash = *Tree;
	if(trash->pNext)
		DeleteAddrTree(&(trash->pNext));


	DeleteAddrEntry(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deletes the dist list tree, which is a list of individual dist list entries
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrTree(CAbookDistListTree **Tree)
{

	if (Tree == NULL)
		Tree = &m_pABookDistListTree;

	if (*Tree == NULL)
		return true;
	
	CAbookDistListTree *trash = *Tree;
	if(trash->pNext)
		DeleteAddrTree(&(trash->pNext));


	DeleteAddrEntry(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deletes the list of messaging users
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrTree(CAbookEntry **Tree)
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

/////////////////////////////////////////////////////////////////////////////
// deletes the list of address books
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeleteAddrTree(CAddressBookTree **Tree)
{

	if (Tree == NULL)
		Tree = &m_pABookTree;

	if (*Tree == NULL)
		return true;
	
	
	CAddressBookTree *trash = *Tree;
	if(trash->pNext)
		DeleteAddrTree(&(trash->pNext));


	DeleteAddrEntry(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// called when me make an entry to make Microsoft Outlook the default
// client to help importing using MAPI. 
// This function makes an entry in the registry for the same.
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddToRegistry(HKEY hkey, const char *regPath, const char *value, const char *valueData, bool bShowError /* = false */){
    HKEY  hkResult;
	LONG err = 0; 

	DWORD dwDisposition;

	err = RegCreateKeyEx(hkey, (LPCTSTR)regPath,0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	
	if (ERROR_SUCCESS == err)
	{
		err = RegSetValueEx(hkResult, value, 0, REG_SZ, (const unsigned char*)valueData, 20);
		RegCloseKey(hkResult);

	}

	return ((ERROR_SUCCESS == err) ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Personality Stuff
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// called from the main Eudora app and it builds a list of personalities and
// returns the list

// this function opens up the list of possible registry entries and returns 
// them to Eudora. there could be several places where Microsoft Outlook
// can have made the entries... differs for different versions and flavors
// we'll just try out best to get the entry, or importing personalities wud fail
// MAPI fails here to give us any information bout personalities... everything
// seems to associated only with a profile and there is no mention of individual
// personalties under a profile... anyways all hope is not lost, we'll get the
// values from the registry
// possible places 
// 1)HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Windows Messaging Subsystem\Profiles
// this is the ideal solution... here all identites are associated with profiles
// if this entry is found then in the main importer dialog we can import the mailboxes,
// address books and identites for the selected profiles, since we know the association
// between the identies and the profiles here
// 2)HKEY_CURRENT_USER\Software\Microsoft\Office\Outlook\OMI Account Manager\Accounts
// or HKEY_CURRENT_USER\Software\Microsoft\Office\8.0\Outlook\OMI Account Manager
// In this case the registry shows no association between the profiles and the identies
// we don't know which identity belong to which profile... so we just display the importing
// of personalities as separate entries from the profiles
// 3) we don't find the entries...
/////////////////////////////////////////////////////////////////////////////
CPersonalityTree* OLImportClass::GetPersonalityTree(char * lpszProfileName, bool bIsProfile)
{

	//free up old and existing values in the data structures before proceeding with new import
	FreeFolderABPersonalityTree();

	if(bIsProfile) {
		if(GetPersonalityInfoFromRegistryForProfile(lpszProfileName))
			return m_pPersonalityTree;
		else
			return NULL;
	} else {
		if(GetPersonalityInfoFromRegistryForIdentity(lpszProfileName))
			return m_pPersonalityTree;
		else
			return NULL;
	}

	return NULL;

}

//here we just loop and find an account of the same name as that passed and process the account information
//for the identity
bool OLImportClass::GetPersonalityInfoFromRegistryForIdentity(char * lpszProfileName)
{
	HKEY hKey;
	HKEY hAccountKey;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = 128;
	HKEY hChildKey;
	FILETIME fileTime;
	bool returnval = false;

	char szProfileRegEntry1[256];
	LoadResourceString(IDS_IDENTITY_REGISTRY_ENTRY_1, szProfileRegEntry1);

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry1, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		tempbuffer = DEBUG_NEW char[tempbufsize];

		if (RegOpenKeyEx(hKey, "Accounts", 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (ExtractValue(&tempbuffer, "Account Name", hChildKey)) {
						if(!strcmp(tempbuffer, lpszProfileName)) {
							ProcessPersonalityInfoForIdentity(hChildKey);
							break;
						}
					}
					RegCloseKey(hChildKey);
				}

			tempbufsize = 128;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		returnval = true;
	}
	dwIndex = 0;

	char szProfileRegEntry2[256];
	LoadResourceString(IDS_IDENTITY_REGISTRY_ENTRY_2, szProfileRegEntry2);

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry2, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		tempbuffer = DEBUG_NEW char[tempbufsize];

		if (RegOpenKeyEx(hKey, "Accounts", 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (ExtractValue(&tempbuffer, "Account Name", hChildKey)) {
						if(!strcmp(tempbuffer, lpszProfileName)) {
							ProcessPersonalityInfoForIdentity(hChildKey);
							break;
						}
					}

					RegCloseKey(hChildKey);
				}
			tempbufsize = 128;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		returnval = true;
	}

	return returnval;	

}

bool OLImportClass::GetPersonalityInfoFromRegistryForProfile(char * lpszProfileName)
{
	//search for profiles under the foll key and find the matching profile name to the one passed here.
	//HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Windows Messaging Subsystem\Profiles

	HKEY hKey;
	HKEY hProfileKey;
	HKEY hAccountKey;
	HKEY hPersonalityKey;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = 128;
	FILETIME fileTime;

	char szProfileRegEntry[256];
	LoadResourceString(IDS_PROFILE_REGISTRY_ENTRY, szProfileRegEntry);

	char szProfileKeyRegEntry[256];
	LoadResourceString(IDS_PROFILE_KEY_REGISTRY, szProfileKeyRegEntry);

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		szProfileRegEntry, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

		if (RegOpenKeyEx(hKey, lpszProfileName, 0, KEY_READ, &hProfileKey) == ERROR_SUCCESS) {
			tempbuffer = DEBUG_NEW char[tempbufsize];

			if (RegOpenKeyEx(hProfileKey, szProfileKeyRegEntry, 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS) {
				
				while (RegEnumKeyEx(hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS) {
					if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hPersonalityKey) == ERROR_SUCCESS) {
	
						if(IsValidAccount(hPersonalityKey)) 
							ProcessPersonalityInfo(hPersonalityKey);
							
						RegCloseKey(hPersonalityKey);
					}
					tempbufsize = 128;
				}
			} else 
				return false;
		} else
			return false;

		RegCloseKey(hAccountKey);
		RegCloseKey(hProfileKey);
		RegCloseKey(hKey);
		return true;

	} else 
		return false;

}

/////////////////////////////////////////////////////////////////////////////
// When we find the account info, we can't be sure if it is an entry
// for the address book or the personality. Hence if it there is a key
// called "SMTP Server" we can take it for granted that it is a personality entry
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::IsValidAccount(HKEY hPersonalityKey) {
		
	DWORD   dwKeyDataType;
	char	*trash = NULL;
	DWORD	trashsize = 128;

	trash = DEBUG_NEW char[trashsize];

	char szSMTPServer[256];
	LoadResourceString(IDS_SMTP_SERVER, szSMTPServer);

	// check to see if it is a valid EMail Account as opposed to an AB Container registry entry etc...
	// if SMTP Display Name is there, then it is a valid EMail Account.
	if (RegQueryValueEx(hPersonalityKey, szSMTPServer, NULL, &dwKeyDataType,
		(LPBYTE) trash, &trashsize) != ERROR_SUCCESS) {

		delete [] trash;
		return false;
	}	

	delete [] trash;
	return true;
}


bool OLImportClass::ProcessPersonalityInfoForIdentity(HKEY hPersonKey) 
{
	CPersonalityTree *pEntry;
	pEntry = DEBUG_NEW CPersonalityTree;
	InitPersonalityTree(pEntry);

	//set the default values for the personality entries that we cannot get
	//default values for
	pEntry->bCheckMail = true;
	pEntry->bPassword = true;
	pEntry->bAPop = false;
	pEntry->bKerberos = false;
	pEntry->bRPA = false;

	char szAcctName[256];
	LoadResourceString(IDS_ACCOUNT_NAME, szAcctName);
	if (!ExtractValue(&(pEntry->pPersName), szAcctName, hPersonKey))
			return false;

	//if we don't find a real name, lets just use the personality name as the real name
	char szSMTPDisplay[256];
	LoadResourceString(IDS_IDENTITY_SMTP_DISPLAY, szSMTPDisplay);
	if(!ExtractValue(&(pEntry->pRealName), szSMTPDisplay, hPersonKey))
			pEntry->pRealName = pEntry->pPersName;

	// SMTP server is essential. if it isn't present we quit
	char szSMTPServer[256];
	LoadResourceString(IDS_SMTP_SERVER, szSMTPServer);
	if (!ExtractValue(&(pEntry->pOutServer), szSMTPServer, hPersonKey)) 
		return false;	
	
	char szSMTPEMail[256];
	LoadResourceString(IDS_IDENTITY_SMTP_EMAIL, szSMTPEMail);
	if (!ExtractValue(&(pEntry->pPOPAccount), szSMTPEMail, hPersonKey)) 
			return false;

	//we don't set a diff reply email address in Eudora personality class
	//in fact the return address field stores the email address itself
	//hence we set our return address with the email
	//OL has another field - Reply Email. Lets see if we can set the return address
	//to be diff than the send email id mayb in some INI file settings
	if (!ExtractValue(&(pEntry->pReturnAddress), szSMTPEMail, hPersonKey))
			return false;

	char szPOPServer[256];
	LoadResourceString(IDS_POP_SERVER, szPOPServer);
	char szIMAPServer[256];
	LoadResourceString(IDS_IMAP_SERVER, szIMAPServer);

	if (ExtractValue(&(pEntry->pInServer), szPOPServer, hPersonKey)) {
			
			pEntry->bPOP = true;
			pEntry->bIMAP = false;

			char szPOPUser[256];
			LoadResourceString(IDS_IDENTITY_POP_USER, szPOPUser);
			if(!ExtractValue(&(pEntry->pLoginName), szPOPUser, hPersonKey))
				return false;

			wchar_t szPOPSSL[2048];
			LoadResourceWideString(IDS_IDENTITY_POP_SSL, szPOPSSL);
			if (!ExtractWValue(szPOPSSL, hPersonKey))
				pEntry->iSSLReceiveUsage = 0;
			else
				pEntry->iSSLReceiveUsage = 1;

	} else if (ExtractValue(&(pEntry->pInServer), szIMAPServer, hPersonKey)) {

			pEntry->bIMAP = true;
			pEntry->bPOP = false;

			char szIMAPUser[256];
			LoadResourceString(IDS_IDENTITY_IMAP_USER, szIMAPUser); 
			if(!ExtractValue(&(pEntry->pLoginName), szIMAPUser, hPersonKey))
				return false;

			wchar_t szIMAPSSL[2048];
			LoadResourceWideString( IDS_IDENTITY_IMAP_SSL, szIMAPSSL);
			if (!ExtractWValue(szIMAPSSL, hPersonKey))
				pEntry->iSSLReceiveUsage = 0;
			else
				pEntry->iSSLReceiveUsage = 1;

			char szIMAPRootFolder[256];
			LoadResourceString(IDS_IDENTITY_IMAP_ROOT_FOLDER, szIMAPRootFolder); 
			ExtractValue(&(pEntry->pIMAPPrefix), szIMAPRootFolder, hPersonKey);

	} else {
		return false;
	}

	wchar_t szSMTPSSL[2048];
	LoadResourceWideString(IDS_IDENTITY_SMTP_SSL, szSMTPSSL);	
	if (!ExtractWValue(szSMTPSSL, hPersonKey))
			pEntry->iSSLSendUsage = 0;
	else
			pEntry->iSSLSendUsage = 1;
	
	wchar_t szLMOS[2048];
	LoadResourceWideString(IDS_IDENTITY_LMOS, szLMOS);	
	if (ExtractWValue(szLMOS, hPersonKey)) {
			pEntry->bLMOS = true;

			wchar_t szRemWhenDel[2048];
			LoadResourceWideString(IDS_IDENTITY_REMOVE_WHEN_DEL, szRemWhenDel);
			wchar_t szExpDays[2048];
			LoadResourceWideString(IDS_IDENTITY_EXP_DAYS, szExpDays);
			char* pTmpValue = NULL;

			if (ExtractWValue(szRemWhenDel, hPersonKey)) {
				pEntry->bDelWhenTrashed = true;

			} else if (ExtractWValue(&pTmpValue , szExpDays, hPersonKey)) {
				pEntry->pLeaveOnServDays = DEBUG_NEW char[3];
				sprintf(pEntry->pLeaveOnServDays, "%d", pTmpValue[0]);
				pEntry->bDelServerAfter = true;
			}

			if(pTmpValue)
				delete [] pTmpValue;

	} else {
			pEntry->bLMOS = false;
	}
	

	wchar_t szUseAuth[2048];
	LoadResourceWideString(IDS_IDENTITY_USE_SICILY, szUseAuth);
	char* pTmpValue = NULL;
	if (!ExtractWValue(&pTmpValue, szUseAuth, hPersonKey)) {
		pEntry->bSMTPAuthAllowed = false;
		pEntry->bUseSMTPRelay = false;
		delete [] pTmpValue;
	} else {
		char authVal[1];
		sprintf(authVal, "%d", pTmpValue[0]);

		if(authVal[0] == '2') {
			pEntry->bSMTPAuthAllowed = true;
			pEntry->bUseSMTPRelay = false;		
		} else if(authVal[0] == '3') {
			pEntry->bSMTPAuthAllowed = true;
			pEntry->bUseSMTPRelay = true;		
		}
	}
	if(pTmpValue)
		delete [] pTmpValue;

	AddPersonalityToTree(pEntry);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// extracts all the value for the key found and adds the personality node
// to the list of personalities
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ProcessPersonalityInfo(HKEY hPersonKey) {

	CPersonalityTree *pEntry;
	pEntry = DEBUG_NEW CPersonalityTree;
	InitPersonalityTree(pEntry);

	//set the default values for the personality entries that we cannot get
	//default values for
	pEntry->bCheckMail = true;
	pEntry->bPassword = true;
	pEntry->bAPop = false;
	pEntry->bKerberos = false;
	pEntry->bRPA = false;

	wchar_t szAcctName[2048];
	LoadResourceWideString(IDS_ACCOUNT_NAME, szAcctName);
	if (!ExtractWValue(&(pEntry->pPersName), szAcctName, hPersonKey))
			return false;

	//if we don't find a real name, lets just use the personality name as the real name
	wchar_t szDisplayName[2048];
	LoadResourceWideString(IDS_PROFILE_DISPLAY_NAME, szDisplayName);
	if(!ExtractWValue(&(pEntry->pRealName), szDisplayName, hPersonKey))
			pEntry->pRealName = pEntry->pPersName;

	// SMTP server is essential. if it isn't present we quit
	wchar_t szSMTPServer[2048];
	LoadResourceWideString(IDS_SMTP_SERVER, szSMTPServer);
	if (!ExtractWValue(&(pEntry->pOutServer), szSMTPServer, hPersonKey))
			return false;	
	
	wchar_t szEMail[2048];
	LoadResourceWideString(IDS_PROFILE_EMAIL, szEMail);
	if (!ExtractWValue(&(pEntry->pPOPAccount), szEMail, hPersonKey))
			return false;

	//we don't set a diff reply email address in Eudora personality class
	//in fact the return address field stores the email address itself
	//hence we set our return address with the email
	//OL has another field - Reply Email. Lets see if we can set the return address
	//to be diff than the send email id mayb in some INI file settings
	if (!ExtractWValue(&(pEntry->pReturnAddress), szEMail, hPersonKey))
			return false;

	wchar_t szPOPServer[2048];
	LoadResourceWideString(IDS_POP_SERVER, szPOPServer);
	wchar_t szIMAPServer[2048];
	LoadResourceWideString(IDS_IMAP_SERVER, szIMAPServer);

	if (ExtractWValue(&(pEntry->pInServer), szPOPServer, hPersonKey)) {
			
			pEntry->bPOP = true;
			pEntry->bIMAP = false;

			wchar_t szPOPUser[2048];
			LoadResourceWideString(IDS_PROFILE_POP_USER, szPOPUser);
			if(!ExtractWValue(&(pEntry->pLoginName), szPOPUser, hPersonKey))
				return false;

			wchar_t szPOPSSL[2048];
			LoadResourceWideString(IDS_PROFILE_POP_SSL, szPOPSSL);
			if (!ExtractWValue(szPOPSSL, hPersonKey))
				pEntry->iSSLReceiveUsage = 0;
			else
				pEntry->iSSLReceiveUsage = 1;

	} else if (ExtractWValue(&(pEntry->pInServer), szIMAPServer, hPersonKey)) {

			pEntry->bIMAP = true;
			pEntry->bPOP = false;

			wchar_t szIMAPUser[2048];
			LoadResourceWideString(IDS_PROFILE_IMAP_USER, szIMAPUser);
			if(!ExtractWValue(&(pEntry->pLoginName), szIMAPUser, hPersonKey))
				return false;

			wchar_t szIMAPSSL[2048];
			LoadResourceWideString(IDS_PROFILE_IMAP_SSL, szIMAPSSL);
			if (!ExtractWValue(szIMAPSSL, hPersonKey))
				pEntry->iSSLReceiveUsage = 0;
			else
				pEntry->iSSLReceiveUsage = 1;

			wchar_t szIMAPFolder[2048];
			LoadResourceWideString(IDS_PROFILE_IMAP_FOLDER, szIMAPFolder);
			ExtractWValue(&(pEntry->pIMAPPrefix), szIMAPFolder, hPersonKey);

	} else {
		return false;
	}

	wchar_t szSMTPSSL[2048];
	LoadResourceWideString(IDS_PROFILE_SMTP_SSL, szSMTPSSL);	
	if (!ExtractWValue(szSMTPSSL, hPersonKey))
			pEntry->iSSLSendUsage = 0;
	else
			pEntry->iSSLSendUsage = 1;
	
	wchar_t szLMOS[2048];
	LoadResourceWideString(IDS_PROFILE_LMOS, szLMOS);	
	char* pTmpValue = NULL;
	if (!ExtractWValue(&pTmpValue, szLMOS, hPersonKey)) {
			delete [] pTmpValue;
	} else {
		//set the values for bLMOS + bDelServerAfter + LeaveOnServerDays + bDelWhenTrashed
		// entry for LMOS is diff in the registry. it has 2 parts the lower 16 bit integer
		// sets the type of LMOS and the higher 16 bit sets the number of days to leave on server
		char lVal[1];
		char hVal[3];
		sprintf(lVal, "%d", pTmpValue[0]);
		sprintf(hVal, "%d", pTmpValue[1]);

		if(lVal[0] == '0') 
			SetLMOSValue(&pEntry, false, false, NULL, false);
		else if(lVal[0] == '1')
			SetLMOSValue(&pEntry, true, true, NULL, false);
		else if(lVal[0] == '3')
			SetLMOSValue(&pEntry, true, true, hVal, false);
		else if(lVal[0] == '5')
			SetLMOSValue(&pEntry, true, false, NULL, true);
		else if(lVal[0] == '7')
			SetLMOSValue(&pEntry, true, true, hVal, true);

		delete [] pTmpValue;

	}

	wchar_t szUseAuth[2048];
	LoadResourceWideString(IDS_PROFILE_SMTP_USE_AUTH, szUseAuth);	
	if (ExtractWValue(szUseAuth, hPersonKey)) {

		pEntry->bSMTPAuthAllowed = true;

		wchar_t szAuthMethod[2048];
		LoadResourceWideString(IDS_PROFILE_AUTH_METHOD, szAuthMethod);	
		if (ExtractWValue(szAuthMethod, hPersonKey))
			pEntry->bUseSMTPRelay = true;
		else
			pEntry->bUseSMTPRelay = false;

	} else {
		pEntry->bSMTPAuthAllowed = false;
		pEntry->bUseSMTPRelay = false;
	}
	
	AddPersonalityToTree(pEntry);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// extracts the value from the registry for regular REG_SZ entries
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ExtractValue(char ** ppDest, char *lpszKeyName, HKEY hPersonKey)
{
	DWORD dwDataBufSize=128;
	DWORD dwKeyDataType;
	char * szData;

	szData = DEBUG_NEW_NOTHROW char[dwDataBufSize];
	if (!szData)
	{
		return false;
	}

	if (RegQueryValueEx(hPersonKey, lpszKeyName, NULL, &dwKeyDataType,
		(LPBYTE)szData, &dwDataBufSize) == ERROR_SUCCESS)
	{
		// Some other value has been filled in previously and is now going to be overridden
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
// extracts the value from the registry for regular REG_BINARY entries
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ExtractWValue(char ** ppDest, wchar_t *lpszKeyName, HKEY hPersonKey)
{
	DWORD dwDataBufSize=128;
	DWORD dwKeyDataType;
	wchar_t * szData;

	szData = DEBUG_NEW_NOTHROW wchar_t[dwDataBufSize];
	if (!szData)
	{
		return false;
	}

	if (RegQueryValueExW(hPersonKey, lpszKeyName, NULL, &dwKeyDataType,
		(LPBYTE)szData, &dwDataBufSize) == ERROR_SUCCESS)
	{
		// Some other value has been filled in previously and is now going to be overridden
		if (*ppDest != NULL)	
		{					
			delete [] *ppDest;
			*ppDest = NULL;
		}

		
		*ppDest = DEBUG_NEW_NOTHROW char[(dwDataBufSize/2)+1];
		if (!*ppDest)
		{
			delete [] szData;
			return false;
		}

		WideCharToMultiByte( CP_ACP, 0, szData, -1,
							*ppDest, 
							(dwDataBufSize/2)+1,NULL, NULL );

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
// extracts the value from the registry for regular REG_DWORD entries
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::ExtractWValue(wchar_t *lpszKeyName, HKEY hPersonKey)
{

	DWORD dwDataBufSize=128;
	DWORD dwKeyDataType;
	wchar_t * szData;

	szData = DEBUG_NEW_NOTHROW wchar_t[dwDataBufSize];
	if (!szData)
	{
		return false;
	}

	if (RegQueryValueExW(hPersonKey, lpszKeyName, NULL, &dwKeyDataType,
		(LPBYTE)szData, &dwDataBufSize) == ERROR_SUCCESS)
	{
	
		char *pVal = DEBUG_NEW_NOTHROW char[(dwDataBufSize/2)+1];
		if (!pVal)
		{
			delete [] szData;
			return false;
		}

		WideCharToMultiByte( CP_ACP, 0, szData, -1,
							pVal, 
							(dwDataBufSize/2)+1,NULL, NULL );

		bool bRetVal = false;
		
		char val[1];
		sprintf(val, "%d", pVal[0]);

		if(val[0] == '1')
			bRetVal = true;

		delete [] szData;
		delete [] pVal;
		return bRetVal;
	}
	else
	{
		delete [] szData;
		return false;
	}

}

/////////////////////////////////////////////////////////////////////////////
//set values for bLMOS + bDelServerAfter + LeaveOnServerDays + bDelWhenTrashed
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::SetLMOSValue(CPersonalityTree **ppPersonTree, bool lmos, bool delServerAfter, char* LMOSDays, bool delWhenTrashed)
{
	(*ppPersonTree)->bLMOS = lmos;
	(*ppPersonTree)->bDelServerAfter = delServerAfter;
	(*ppPersonTree)->pLeaveOnServDays = DEBUG_NEW char(strlen(LMOSDays) + 1);
	strcpy((*ppPersonTree)->pLeaveOnServDays, LMOSDays);
	(*ppPersonTree)->pLeaveOnServDays[strlen(LMOSDays)] = '\0';
	(*ppPersonTree)->bDelWhenTrashed = delWhenTrashed;
	return true;
	
}

/////////////////////////////////////////////////////////////////////////////
// adds the personality node to the list
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::AddPersonalityToTree(CPersonalityTree *pEntry) 
{

	CPersonalityTree * pWalker;

	pWalker = m_pPersonalityTree;

	if(pWalker == NULL)
	{
		m_pPersonalityTree = DEBUG_NEW CPersonalityTree;
		pWalker = m_pPersonalityTree;
		InitPersonalityTree(pWalker);
		pWalker->pNext = NULL;
	}
	else
	{
		while (pWalker->pNext != NULL)
		{
			pWalker = pWalker->pNext;
		}

		pWalker->pNext = DEBUG_NEW CPersonalityTree;
		pWalker = pWalker->pNext;
		InitPersonalityTree(pWalker);
		pWalker->pNext = NULL;

	}
	*pWalker = *pEntry;

	return true;

}

/////////////////////////////////////////////////////////////////////////////
// adds the personality tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::InitPersonalityTree(CPersonalityTree *pEntry)
{
	pEntry->iSSLReceiveUsage = 0;
	pEntry->iSSLSendUsage = 0;
	pEntry->pBigMsgThreshold = NULL;
	pEntry->pDefaultDomain = NULL;
	pEntry->pIMAPMaxSize = NULL;
	pEntry->pIMAPPrefix = NULL;
	pEntry->pIMAPTrashMailbox = NULL;
	pEntry->pInServer = NULL;
	pEntry->pLeaveOnServDays = NULL;
	pEntry->pLoginName = NULL;
	pEntry->pNext = NULL;
	pEntry->pOutServer = NULL;
	pEntry->pPersName = NULL;
	pEntry->pPOPAccount = NULL;
	pEntry->pRealName = NULL;
	pEntry->pReturnAddress = NULL;
	pEntry->pSignature = NULL;
	pEntry->pStationery = NULL;
	pEntry->bAPop = false;
	pEntry->bCheckMail = false;
	pEntry->bDelServerAfter = false;
	pEntry->bDelWhenTrashed = false;
	pEntry->bIMAP = false;
	pEntry->bIMAPFullDwnld = false;
	pEntry->bIMAPMarkDeleted = false;
	pEntry->bIMAPMinDwnld = true;
	pEntry->bIMAPXferToTrash = false;
	pEntry->bKerberos = false;
	pEntry->bLMOS = false;
	pEntry->bPassword = false;
	pEntry->bPOP = false;
	pEntry->bRPA = false;
	pEntry->bSkipBigMsgs = false;
	pEntry->bSMTPAuthAllowed = false;
	pEntry->bUseSMTPRelay = false;
	pEntry->bWinSock = false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deletes the personality tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeletePersonalityTree(CPersonalityTree **Tree)
{

	if (Tree == NULL)
		Tree = &m_pPersonalityTree;

	if (*Tree == NULL)
		return true;
	
	
	CPersonalityTree *trash = *Tree;
	if(trash->pNext)
		DeletePersonalityTree(&(trash->pNext));


	DeletePersonalityEntry(trash);
	delete *Tree;
	*Tree = NULL;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// deletes a node in the personality tree
/////////////////////////////////////////////////////////////////////////////
bool OLImportClass::DeletePersonalityEntry(CPersonalityTree *pEntry)
{

	if(pEntry->pBigMsgThreshold)
		delete [] pEntry->pBigMsgThreshold;
	if(pEntry->pDefaultDomain)
		delete [] pEntry->pDefaultDomain;
	if(pEntry->pIMAPMaxSize)
		delete [] pEntry->pIMAPMaxSize;
	if(pEntry->pIMAPPrefix)
		delete [] pEntry->pIMAPPrefix;
	if(pEntry->pIMAPTrashMailbox)
		delete [] pEntry->pIMAPTrashMailbox;
	if(pEntry->pInServer)
		delete [] pEntry->pInServer;
	if(pEntry->pLoginName)
		delete [] pEntry->pLoginName;
	if(pEntry->pNext)
		delete pEntry->pNext;
	if(pEntry->pOutServer)
		delete [] pEntry->pOutServer;
	if(pEntry->pPersName)
		delete [] pEntry->pPersName;
	if(pEntry->pPOPAccount)
		delete [] pEntry->pPOPAccount;
	if(pEntry->pRealName)
		delete [] pEntry->pRealName;
	if(pEntry->pReturnAddress)
		delete [] pEntry->pReturnAddress;
	if(pEntry->pSignature)
		delete [] pEntry->pSignature;
	if(pEntry->pStationery)
		delete [] pEntry->pStationery;

	return true;

}

/////////////////////////////////////////////////////////////////////////////
// loads a string from the resource file for the given ID
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::LoadResourceString(UINT nID, char* pDest)
{
	if (!pDest)
		assert(0);
	
	if(!LoadString((HINSTANCE)m_hModule, nID, pDest, 256)) {
		
		delete [] pDest;
		pDest = NULL;

		assert(0);
		TCHAR szBuf[2048];
		strcpy(szBuf, "Unspecified Error Importing\nWould you like to continue?");
		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		MessageBox(hEudoraMain,szBuf,"Eudora",MB_OK | MB_ICONSTOP);	
	}


}

/////////////////////////////////////////////////////////////////////////////
// loads a string from the resource file for the given ID and converts it
// into unicode format
/////////////////////////////////////////////////////////////////////////////
void OLImportClass::LoadResourceWideString(UINT nID, wchar_t* pDest)
{
	char szBuf[256];
	if(!LoadString((HINSTANCE)m_hModule, nID, szBuf, 256)) {
		assert(0);
		TCHAR szBuff[2048];
		strcpy(szBuff, "Unspecified Error Importing\nWould you like to continue?");
		HWND hEudoraMain = FindWindow("EudoraMainWindow", NULL);
		MessageBox(hEudoraMain,szBuff,"Eudora",MB_OK | MB_ICONSTOP);	
	}

	if (!pDest)
		assert(0);
	
	MultiByteToWideChar(CP_ACP, NULL, szBuf, -1, pDest, 2048);

}
