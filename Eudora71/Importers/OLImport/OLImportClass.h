#ifndef _OL_IMPORT_CLASS_H_
#define _OL_IMPORT_CLASS_H_

#include "stdafx.h"
#include "OLImport.h"
#include <mapi.h>
#include <mapiutil.h>
#include <mapix.h>
#include <string>

//using namespace std;

#define OL_98_VERSION 0
#define OL_00_VERSION 1

#define TEXT_PLAIN	0
#define TEXT_RICH	1
#define TEXT_HTML	2
#define MULTI_MIXED 3

class OLImportClass
{

	
// Constructor/Destructor
public:
	OLImportClass();
	~OLImportClass();


// Destruction helpers
private:
	void				FreeFolderABPersonalityTree();


//Mapi Function pointers
private:
	typedef ULONG			(CALLBACK *CMIMAPIFreeBuffer)(LPVOID);
	typedef HRESULT			(CALLBACK *CMIMAPILogoff)(ULONG, ULONG, ULONG);
	typedef HRESULT			(CALLBACK *CMIMAPIInitialize)(LPVOID);
	typedef HRESULT			(CALLBACK *CMIMAPILogonEx)(ULONG, LPTSTR, LPTSTR, FLAGS, LPMAPISESSION FAR *);
	typedef HRESULT			(CALLBACK *CMIMAPIUnInitialize)(void);
	typedef	HRESULT			(CALLBACK *CMIHrQueryAllRows)(LPMAPITABLE, LPSPropTagArray, LPSRestriction, LPSSortOrderSet, LONG, LPSRowSet FAR *);
	typedef HRESULT			(CALLBACK *CMIMAPIAdminProfiles)(ULONG, LPPROFADMIN FAR *);

	
// Exported Functions for querying profiles 
public:
	void					LoadModuleHandle(HANDLE hModule);
	void					LoadOLProvider(bool bDisplayErrDialog);
	int						GetNumProviderChildren() {	return m_OLImportProvider.iNumChildren;	}
	CImportProvider *		GetProvider() { return &(m_OLImportProvider);	}


// Provider Operations
private:
	bool					FindMainOLKey(bool bDisplayErrDialog);
	bool					FindOLRegKeys();
	bool					AddProvider(char * lpszProviderName);
	void					AddProfiles(LPSTR m_profileName, ImportType kImpType);
	CImportChild *			CreateChildNode(ImportType kImpType);
	bool					AddToRegistry(HKEY hkey, const char *regPath, const char *value, const char *valueData, bool bShowError = false );

// provider stuff
private:
	HANDLE					m_hModule;
	CImportProvider			m_OLImportProvider;
	CMIMAPIAdminProfiles	m_MAPIAdminProfiles;
	bool					m_bProfileKey;
	bool					m_bIdentityKey;


// Exported Functions for Message Store & Mailboxes
public:
	CMessageStore *		LoadFolders(char *lpszFolderPath, char * lpszProfileName);
	char *				MakeMimeWithFile(char *lpszFilename, void * pID, unsigned int iSize, void *pDB);


// message store & mailbox stuff
private:
	
	//the name of the tmp profile and tmp message store to be created when importing pst files
	const char * const		EUDORA_TO_OUTLOOK_IMPORTER_TEMP_PROFILE;
	const char * const		EUDORA_TO_OUTLOOK_IMPORTER_TEMP_MESSAGE_STORE;
	enum m_MapiFolderType {INBOX,OUTBOX,SENT_MBOX,DELETED_MBOX,CALENDAR,CONTACTS,JOURNAL,NOTES,TASKS,OL_SPECIAL,MAIL_MBOX,OTHER_OL_STUFF};
	
	// These are used primarily by Folder Operations
	CMessageStore	*		m_pMessageStores;  // pointer to the message store list
	CMbox_FnamePair *		m_fnpFolders;	   // pointer to the current list of folders for the current message store
	char *					m_newMsgStoreName; // the new message stote name when we import from .pst files
	bool					m_bMapiLoggedOn;   // ? loggen on to MAPI
	bool					m_bRootCreated;    
	CMIMAPIInitialize		m_MAPIInitialize;	
	CMIMAPILogonEx			m_MAPILogonEx;
	LPMAPITABLE				m_lpTblStores;
	LPMDB					m_lpMDB;			// pointer to open message store
	LPMAPIPROP				m_lpRootFolder;
	HINSTANCE				m_hMapiDLL;		    //handle to the MAPI32 DLL
	CMIMAPIFreeBuffer		m_MAPIFreeBuffer;	
	LPMAPISESSION			m_lpSession;		// Pointer to the Session
	CMIHrQueryAllRows		m_HrQueryAllRows;
	bool					m_bGotEids;           // a shortcut for whether or not all the following have been set
	SPropValue * m_InboxEID, * m_OutboxEID, * m_SentEID, * m_DeletedEID;
	SPropValue * m_CalendarEID, * m_ContactsEID, * m_JournalEID, * m_NotesEID, * m_TasksEID;

	// these are used primarily by Mime Digest Operations
	FILE *					m_fpoDigestFile;
	char					m_szMainBoundary[128];
	char					m_szMessageBoundary[128];
	FILE *					m_tempfile;
	char *					m_TempFilePathname;
	char *					m_lpszDigestPath;
	char					m_szTempDirName[_MAX_PATH];
	
	// MAPI Message Store & Mailbox functions, MIME making functions for the mailboxes
private:
	bool					MAPILogon(bool bDisplayErrDialog = false);
	bool					MAPIInitialize(char *lpszProfileName);
	bool					MAPIOpenMessageStore(const char* const pMessageStoreName);
	bool					MAPIOpenDefaultStore();
	bool					MAPIProcessFolder(ULONG cbEntryID, LPENTRYID lpEntryID, UINT iParentID = 0);
	bool					FreeRowSet(CMIMAPIFreeBuffer MAPIFreeBufferfunc, LPSRowSet lpRows);
	void					MAPICloseDefaultStore();
	bool					MAPILogoff();
	bool					MakeFolder(LPMAPIPROP Folder, bool bHasChildren, UINT iID, UINT iParentID);
	bool					IsValidMailFolder(LPMAPIPROP ParentFolder);
	bool					CreateTmpProfileFromPST(char* lpszFolderPath, char **lppszProfileName, char **lpNewMsgStoreName);
	void					MAPIEnsureEudoraTmpProfileDeleted(IProfAdmin *pProfAdmin);
	bool					OpenAllStoresForProfile();
	bool					InitMsgStoreTree(CMessageStore *pEntry);
	bool					AddMsgStoreEntryToTree(CMessageStore *pEntry);
	bool					MAPIProcessMailbox(ULONG cbEntryID, LPENTRYID lpEntryID, char *lpszMboxName, void *pDB);
	bool					ContainsMessages(LPMAPIPROP ParentFolder, ULONG * NumReturned);
	bool					GetMessages(IMAPIFolder	* CurrentFolder);
	char *					GetLine(IStream * Stream, ULONG * bytesRead, ULONG *posn);
	bool					CommitRowToMbx(LPSRow lpRow);
	bool					GenerateHeaders(char *SenderName, char *SenderEmail, char *Date, char *RcptEmail, char *MessageSubject);
	bool					AddContentHeader(IStream *Stream, ULONG *bodytype, bool bHasAttach = 0);
	bool					GrabMsgBody(IStream * BodyStream);
	bool					WriteAttachToMbx(IStream * stream, char * name, char *lpszMimeHeader);
	FILE *					CreateDigestFile();
	bool					InitDigestFile();
	bool					WriteToDigestFile();
	bool					WriteDigestEnd();
	bool					OpenTempFile();
	bool					CloseTempFile();
	bool					DeleteTempFile();
	int						EscapePressed(int Repost = FALSE);
	void					MakeFolders(IMAPIFolder * parent, std::string prefix, UINT iParentID = 0);
	m_MapiFolderType		MapiGetFolderType(SPropValue *eid, IMAPIFolder *f);
	void					MapiEnsureCommonEids();
	bool					AreEIDsEqual(const SPropValue * const eid, IMAPISession * pSession, const SPropValue * const e);
	void					ClearEID(SPropValue * psPropVal);
	void					SetEID(SPropValue * psPropVal, ULONG size, ENTRYID * pEID);
	bool					IsEIDEmpty(const SPropValue * psPropVal);
	bool					AddFolder(char* szFolderName, char* szFolderDisplayName, bool bHasChildren, UINT iID, UINT iParentID, void * pEntryID, unsigned int iSize, FolderType folderType);
	bool					DeleteMsgStoreTree(CMessageStore **Tree);
	bool					DeleteMsgStoreEntry(CMessageStore *pEntry);
	bool					DeleteMBoxFoldersTree(CMbox_FnamePair **Tree);
	void					DeleteFolderNodeData(CMbox_FnamePair *pFolder);
	void					DeleteChildNodeData(CImportChild *pChild);


//Address Book stuff

public:
	CAddressBookTree *		GetAddressTree(char *lpszPath, char * lpszProfileName);

private:
	bool					Convert(char * lpszProfileName);
	CAbookEntry *			ReadInEntry();
	bool					OpenAddressBook();
	bool					ProcessABook();
	bool					ProcessABEntry(LPMAPIPROP ABEntry, void * pEntryIDValue, ULONG lEntryIDSize);
	bool					ProcessABookContainer(LPABCONT lpABookCont, std::string lABookName);
	void					CloseAddressBook();
	bool					AddEntryToDistListTree(CAbookDistListTree *pEntry);
	bool					AddEntryToDistList(CAbookDistListEntry *pEntry);
	bool					AddEntryToBook(CAbookEntry *pEntry);
	bool					InitAddrEntry(CAbookEntry *pEntry);
	bool					InitAddrTree(CAddressBookTree *pEntry);
	bool					InitAddrEntry(CAbookDistListEntry *pEntry);
	bool					InitAddrTree(CAbookDistListTree *pEntry);
	bool					AddABookToTree(CAddressBookTree *pEntry);
	bool					DeleteAddrEntry(CAbookEntry *pEntry);
	bool					DeleteAddrEntry(CAddressBookTree *pEntry);
	bool					DeleteAddrEntry(CAbookDistListEntry *pEntry);
	bool					DeleteAddrEntry(CAbookDistListTree *pEntry);
	bool					DeleteAddrTree(CAbookEntry **Tree);
	bool					DeleteAddrTree(CAddressBookTree **Tree);
	bool					DeleteAddrTree(CAbookDistListEntry **Tree);
	bool					DeleteAddrTree(CAbookDistListTree **Tree);
	bool					RecurseABookHierarchy(LPABCONT lpABookCont, std::string lABookName);
	bool					ProcessABDistListEntry(LPDISTLIST lpABDistList, void *pID, ULONG IDSize);

private:
	LPABCONT				m_lpABC;		
	LPADRBOOK				m_lpAdrBook;	 
	CAbookEntry *			m_pABook;				// list of messaging users
	CAbookDistListTree *	m_pABookDistListTree;	// list of distribution lists
	CAbookDistListEntry *	m_pABookDistList;		// dist list entry... list of messaging users/dist list it contains
	CAddressBookTree *		m_pABookTree;			// list of address books
	
// Advanced dialog stuff
public:
	char *				GetAddrExtension() { return "PST"; }
	char *				GetAddrExtensionFilter() { return "Outlook PST (*.pst)|*.pst|"; }
	char *				GetMailExtension() { return "PST"; }
	char *				GetMailExtensionFilter() { return "Outlook PST (*.pst)|*.pst|"; }


//personality stuff
public:
	CPersonalityTree*	GetPersonalityTree(char * lpszProfileName, bool bIsProfile);

private:
	bool				GetPersonalityInfoFromRegistryForProfile(char * lpszProfileName);
	bool				GetPersonalityInfoFromRegistryForIdentity(char * lpszProfileName);
	bool				IsValidAccount(HKEY hPersonKey);
	bool				ProcessPersonalityInfo(HKEY hPersonKey);
	bool				ProcessPersonalityInfoForIdentity(HKEY hPersonKey);
	bool				AddPersonalityToTree(CPersonalityTree *pEntry);
	bool				InitPersonalityTree(CPersonalityTree *pEntry);
	bool				DeletePersonalityTree(CPersonalityTree **Tree);
	bool				DeletePersonalityEntry(CPersonalityTree *pEntry);
	bool				SetLMOSValue(CPersonalityTree **ppPersonTree, bool lmos, bool delServerAfter, char* LMOSDays, bool delWhenTrashed);
	bool				ExtractWValue(char ** ppDest, wchar_t *lpszKeyName, HKEY hChildKey);
	bool				ExtractValue(char ** ppDest, char *lpszKeyName, HKEY hChildKey);
	bool				ExtractWValue(wchar_t *lpszKeyName, HKEY hChildKey);
	void				LoadResourceString(UINT nID, char* pDest);
	void				LoadResourceWideString(UINT nID, wchar_t* pDest);

private:
	CPersonalityTree*	m_pPersonalityTree;		// list of personalities

};


#endif // _OL_IMPORT_CLASS_H_