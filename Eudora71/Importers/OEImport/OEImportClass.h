#ifndef _OE_IMPORT_CLASS_H_
#define _OE_IMPORT_CLASS_H_


// OEImport.eif File
// Imports for Outlook Express 4 and 5
// Can find it itself most of the time
// If not, then you can tell it where to look
// If we can't process the folders mailbox we will
// give a flat folder structure with all the files in
// that directory processed as mailboxes(some will fail, 
// like UIDL and folders dbx files)[OE5]
// You can also give it a specific file to import
// and we will put it under the Provider Name folder
// "Outlook Express"
// Addresses not done yet.
// Should Find WAB
// Take WAB path
// Take Microsoft CSV
#include "stdafx.h"
#include "OEImport.h"
#include "wab.h"
#include <stdio.h>

// Defines for various OE versions
#define OE_4_VERSION 0
#define OE_5_VERSION 1


	enum OE5HeaderInfoType
	{
		HEADER_MESSAGE_INDEX = 0,
		HEADER_MESSAGE_FLAGS,
		HEADER_T_MESSAGE_CREATE_R,
		HEADER_MESSAGE_LINES,
		HEADER_MESSAGE_OFFSET,
		HEADER_MESSAGE_SUBJECT_ORIG,
		HEADER_T_MESSAGE_SAVE,
		HEADER_MESSAGE_LOCAL_ID,
		HEADER_MESSAGE_SUBJECT,
		HEADER_MESSAGE_SENDER_INFO,
		HEADER_MESSAGE_ANSWERED_TO,
		HEADER_MESSAGE_RFC822_ID,
		HEADER_MESSAGE_SERVER,
		HEADER_MESSAGE_SENDER_NAME,
		HEADER_MESSAGE_SENDER_ADDRESS,
		HEADER_MESSAGE_UNKNOWN_1,
		HEADER_MESSAGE_PRIORITY,
		HEADER_MESSAGE_LENGTH, // Don't use for length
		HEADER_T_MESSAGE_CREATE_S,
		HEADER_MESSAGE_RECEIVER_NAME,
		HEADER_MESSAGE_RECEIVER_ADDR,
		HEADER_MESSAGE_UNKNOWN_2,
		HEADER_MESSAGE_UNKNOWN_3,
		HEADER_MESSAGE_UNKNOWN_4,
		HEADER_MESSAGE_UNKNOWN_5,
		HEADER_MESSAGE_UNKNOWN_6,
		HEADER_MESSAGE_ACCT_NAME,
		HEADER_MESSAGE_REG_ENTRY,
		HEADER_MESSAGE_STRUCT
	};

#define INLINE_VALUE_TOKEN 0x80

class OEImportClass
{
	// Constructor/Destructor
public:
	OEImportClass();
	~OEImportClass();
	// Destruction helpers
private:
	void				DeleteFolderNodeData(CMbox_FnamePair *pFolder);
	void				DeleteChildNodeData(CImportChild *pChild);

	// Exported Functions call these happy people.
public:
	void				LoadModuleHandle(HANDLE hModule);
	void				LoadOEProvider();
	int					GetNumProviderChildren() {	return m_OEImportProvider.iNumChildren;	}
	CImportProvider *	GetProvider() { return &(m_OEImportProvider);	}

	// Those happy people call these happy people
	// AKA Provider Operations
private:
	bool				FindIdentityOEKeys();
	bool				FindMainOEKey();
	bool				AddProvider(char * lpszProviderName);
	int					AddChildOLExpress(HKEY hChildKey, char *lpszIdentityKey = NULL);
	CImportChild *		CreateChildNode();
	bool				OLExtractValue(char ** ppDest, char *lpszKeyName, HKEY hChildKey);
	bool				FindMailPath(CImportChild *pChild);
	bool				FindWABPath(char ** path);
	bool				InitAddrTree(CAddressBookTree *pEntry);
	
	// These happy Members are used by Provider Operations
private:
	CImportProvider		m_OEImportProvider;
	HANDLE				m_hModule;


	// Exported Functions call these happy people.
public:
	CMessageStore *	LoadFolders(char *lpszFolderPath);

	// Those happy people call these happy people
	// AKA Load Folder Operations
private:
	int					GetOEVersion(char *lpszFolderPath);
	bool				findEntryByToken(FILE *pchfile);
	char *				MakeOEDisplayName(char *lpszFileName);
	char *				GetFirstOE4Mbox(char *lpszStorePathName);
	char *				GetNextOE4Mbox();	
	char *				GetOE4SearchToken(char *lpszStorePathName);
	bool				InitMsgStoreTree(CMessageStore *pEntry);

	// These happy Members are used primarily by Folder Operations
private:
	CMessageStore *		m_pMessageStores;
	CMbox_FnamePair *	m_fnpFolders;
	OE5FolderInfo		m_OE5Info;
	HANDLE				m_hFindHandle;
	int					m_iOEVersion;

	// Exported Functions call these happy people.
public:
	char *				MakeMimeWithFile(char *lpszFilename, void * pID, unsigned int iSize);

	// Those happy people call these happy people
	// AKA Making our Mime Digest Operations
private:
	LONG				GetNumMsgs(char *pathname);
	FILE *				CreateDigestFile();
	bool				InitDigestFile();
	bool				WriteToDigestFile();
	bool				WriteDigestEnd();
	bool				OpenTempFile();
	bool				CloseTempFile();
	bool				DeleteTempFile();
	bool				GrabNextMsg();
	bool				ReadTilDone();
	int					EscapePressed(int Repost = FALSE);
	bool				DBXwithLFOnly();
	void				OELFtoCRLF();
	bool				FindToken();

	// These happy Members are used primarily by Mime Digest Operations
private:
	long *				m_lOE5MsgTable;				// DRW 11/29/99 - Location of message entries in dbx file
	long				m_lNumOE5MsgTableEntries;	// DRW 9/13/99 - Number of entries in message table
	long				m_lCurrOE5MsgTableEntry;	// DRW 9/13/99 - ID of current entry in message table
	char				m_szTempDirName[_MAX_PATH];
	FILE *				m_fpoDigestFile;
	FILE *				m_fpiOEFile;
	char *				m_lpszOEFileName;
	char				m_szMainBoundary[128];
	char				m_szMessageBoundary[128];
	FILE *				m_tempfile;
	char *				m_TempFilePathname;
	char *				m_lpszDigestPath;

	//Address Book stuff
public:
	CAddressBookTree *		GetAddressTree(char *lpszPath);

private:
	bool				Convert(char* lpszWabPath);
	bool				InitWAB(char *lpszFilePath = NULL);
	bool				ProcessABook();
	bool				ProcessABEntry(LPMAPIPROP ABEntry);
	bool				FreeRowSet(LPWABOBJECT lpWABObject, LPSRowSet lpRows);
	bool				AddEntryToTree(CAbookEntry *pEntry);
	bool				InitAddrEntry(CAbookEntry *pEntry);
	bool				DeleteAddrEntry(CAbookEntry *pEntry);
	bool				DeleteAddrTree(CAbookEntry **Tree);

private:
	bool				m_bInit;
	HINSTANCE			m_hWABDll;
	LPWABOPEN			m_lpfnWabOpen;
	LPADRBOOK			m_lpAdrBook;
	LPWABOBJECT			m_lpWABObject;
	LPSPropValue		m_lpPropArray;
	char				m_szWABDllPath[MAX_PATH];
	HKEY				m_hSharedDllKey;
	CAbookEntry *		m_pABook;
	CAddressBookTree *	m_pABookTree;

	// Advanced dialog stuff
public:
	char *				GetAddrExtension() { return "WAB"; }
	char *				GetAddrExtensionFilter() { return "Windows Address Book (*.wab)|*.wab|"; }
	char *				GetMailExtension() { return "Folders.dbx or Folders.nch"; }
	char *				GetMailExtensionFilter() { return "OE Folder Files (Folders.nch;Folders.dbx)|Folders.nch;Folders.dbx|"; }


//personality stuff
public:
	CPersonalityTree*	GetPersonalityTree(char * lpszProfileName);
		
};

#endif //_OE_IMPORT_CLASS_H_
