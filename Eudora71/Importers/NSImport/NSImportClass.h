#ifndef _NS_IMPORT_CLASS_H_
#define _NS_IMPORT_CLASS_H_

#include "stdafx.h"
#include "NSImport.h"
#include <stdio.h>


class NSImportClass
{
private:
	// Structures used and needed 
	// Mine
	typedef struct CCharArrList_tag
	{
		char *				lpszItem;
		CCharArrList_tag *	pNext;

	} CCharArrList;

	// From NS import code, guess i coulda switched to my struct.
	struct	Pathnames
	{
		char *					pathname;
		struct Pathnames*		next;
	};

	// structure functions. YEah, shoulda been a class, but oh well.
private:
	bool				DeleteCharListTree(CCharArrList **Tree);
	bool				AddCharListTail(CCharArrList *pTree, CCharArrList *pItem);
	bool				AddCharListTail(CCharArrList *pTree, char *pItem);

	// Constructor/Destructor
public:
	NSImportClass();
	~NSImportClass();
	// Destruction helpers
private:
	void				DeleteChildNodeData(CImportChild *pChild);
	void				DeleteFolderNodeData(CMbox_FnamePair *pFolder);

	
	// Exported Functions call these happy people.
public:
	void				LoadModuleHandle(HANDLE hModule);
	void				LoadNSProvider();
	int					GetNumProviderChildren() {	return m_NSImportProvider.iNumChildren;	}
	CImportProvider *	GetProvider() { return &(m_NSImportProvider);	}

	// Those happy people call these happy people
	// AKA Provider Operations
private:
	bool				LoadNSAccounts();
	int					AddChildNS(char* szPrefsFilePath, char *PersName);
	CImportChild *		CreateChildNode();
	bool				NSExtractValue(char ** Dest, char * Line);
	BOOL				LocateNetscapePrefsFile(CCharArrList * FileList);
	void *				GetFilesInDirectory(const char* SearchString, const BOOL bRecurseSubDirs /* = FALSE */, CCharArrList *pStringList /* = NULL */, int iBaseDirElement = 0);
	bool				AddProvider(char * lpszProviderName);

	// These happy Members are used by Provider Operations
private:
	CImportProvider		m_NSImportProvider;
	HANDLE				m_hModule;
	CCharArrList *		m_csBaseDir;
	char				szBaseDir[MAX_PATH];
	struct Pathnames *	m_head;

	// Exported Functions call these happy people.
public:
	CMessageStore *	LoadFolders(char *lpszFolderPath);

	// Those happy people call these happy people
	// AKA Load Folder Operations
private:
	bool				ProcessMailboxes(char *pathname, int iParentID);
	bool				BuildFileList(struct Pathnames **files, struct Pathnames *folders);
	bool				BuildFolderList(struct Pathnames **files);
	bool				existsInTree(char * name, struct Pathnames *folders);
	bool				AddNode(struct Pathnames **currentfile, 	struct _finddata_t *file);
	bool				InitMsgStoreTree(CMessageStore *pEntry);

	// These happy Members are used primarily by Folder Operations
private:
	CMessageStore *		m_pMessageStores;
	CMbox_FnamePair *	m_fnpFolders;

	// Exported Functions call these happy people.
public:
	char *				MakeMimeWithFile(char *lpszFileName, void * pID, unsigned int iSize);

	// Those happy people call these happy people
	// AKA Making our Mime Digest Operations
private:
	int					EscapePressed(int Repost = FALSE);
	FILE *				CreateDigestFile();
	bool				InitDigestFile();
	bool				WriteToDigestFile();
	bool				WriteDigestEnd();
	bool				OpenTempFile();
	bool				CloseTempFile();
	bool				DeleteTempFile();
	bool				OpenNSFile(char * pathname);
	bool				CloseNSFile();
	char *				GetLine(FILE *pFile);
	char *				GetLineNoCRLF(FILE *pFile);
	bool				GrabMessages();
	bool				DeleteDigestFile();
	void				DeleteTree(Pathnames * node);

	// These happy Members are used primarily by Mime Digest Operations
private:
	FILE *				m_fpoDigestFile;
	FILE *				m_fpiNSFile;
	char				m_szMainBoundary[128];
	char				m_szMessageBoundary[128];
	FILE *				m_tempfile;
	char *				m_TempFilePathname;
	char *				m_lpszDigestPath;
	char				m_szTempDirName[_MAX_PATH];

	//Address Book stuff
public:
	CAddressBookTree *		GetAddressTree(char *lpszPath);

private:
	CAbookEntry *		GetNextEntry(FILE *theFile);
	bool				Convert(LPCSTR File);
	bool				AddEntryToTree(CAbookEntry *pEntry);
	bool				InitAddrEntry(CAbookEntry *pEntry);
	bool				InitAddrTree(CAddressBookTree *pEntry);
	bool				DeleteAddrEntry(CAbookEntry *pEntry);
	bool				DeleteAddrTree(CAbookEntry **Tree);

private:
	CAbookEntry *			m_pABook;
	CAddressBookTree *		m_pABookTree;

	// Advanced dialog stuff
public:
	char *				GetAddrExtension() { return "LDIF"; }
	char *				GetAddrExtensionFilter() { return "LDIF (*.LDIF)|*.ldif|"; }
	char *				GetMailExtension() { return "prefs.js"; }
	char *				GetMailExtensionFilter() { return "Netscape Prefs (prefs.js)|prefs.js|"; }


//personality stuff
public:
	CPersonalityTree*	GetPersonalityTree(char * lpszProfileName);


};

#endif //_NS_IMPORT_CLASS_H_
