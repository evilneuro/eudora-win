#ifndef _MAPIIMPORT_H_
#define _MAPIIMPORT_H_

#include "mapiinst.h"
#include "mapix.h"
#include "QCMailboxCommand.h"
#include "fileutil.h"
#include "mboxconverters.h"

#define		OUTLOOK_EXPRESS	0
#define		OUTLOOOK98		1
#define		NETSCAPE		2

#define		OE_DISPLAY_NAME	_T("Outlook Express")
#define		OL98_DISPLAY_NAME	_T("Outlook")		// Might just be Outlook, or Outlook 99, who knows.
#define		NS_DISPLAY_NAME	_T("Netscape Navigator")

typedef enum {MAIL, PERSONALITY, MAIL_AND_PERS, PROVIDER, ADDRESS_BOOK} ChildType;


//Structures
//
class ImportChild
{
public:

	int						AccountID;
	char					*AccountDisplayName;
	bool					LMOS;
	bool					IsIMAP;
	char					*PersonalityName;
	char					*IncomingUserName;	//POP or IMAP based on IsIMAP flag;
	char					*IncomingServer;
	char					*RealName;
	char					*EmailAddress;
	char					*SMTPServer;
	class ImportChild		*next;
	class ImportProvider		*parent;
	bool					hasMail;
	bool					hasAddresses;
	bool					hasLdif;
	char					*PathToData;
	char					*PathtoLdif;
public:

	ImportChild();


};

class ImportProvider
{
public:

	int						UniqueID;
	char					*DisplayName;
	ImportChild				*FirstChild;
	int						NumChildren;
	bool					hasMail;
	bool					hasAddresses;
	bool					hasLdif;
	HICON					icon;
	ImportProvider			*next;

public:

	ImportProvider();

};


bool MAPIimport();


//////////////////////////////////////////////////////////////
// CImportMail is a class that when created probes your system by looking i n the registry and Netscape files 
// To find all your personalities, we put this in a tree under the m_head variable.
//
// When the caller decides which personality they want the tell this class and it gets it and adds it to the
// Personalities window in Eudora.
//
// If the caller decided to get mail for a provider they call Importmail with one of the children and we grab all
// The mail with the appropriate method.
//////////////////////////////////////////////////////////////
class CImportMail
{
		typedef ULONG			(CALLBACK *CMIMAPIFreeBuffer)(LPVOID);
		typedef HRESULT			(CALLBACK *CMIMAPILogoff)(ULONG, ULONG, ULONG);
		typedef HRESULT			(CALLBACK *CMIMAPIInitialize)(LPVOID);
		typedef HRESULT			(CALLBACK *CMIMAPILogonEx)(ULONG, LPTSTR, LPTSTR, FLAGS, LPMAPISESSION FAR *);
		typedef HRESULT			(CALLBACK *CMIMAPIUnInitialize)(void);
public:
	CImportMail();
	~CImportMail();

public:		
	class ImportProvider	*m_head;
	HINSTANCE				m_hDLL;               // Handle to DLL
	LPMAPISESSION			m_lpSession;			// Pointer to the Session
	bool					m_LoggedOn;
	CMapiInstaller::Status	m_InstalledStatus;
	CMIMAPIFreeBuffer		m_MAPIFreeBuffer;
//	CMIMAPILogonEx			m_MAPILogonEx;
//	CMIMAPILogoff			m_MAPILogoff;

private:
	// Personality Functions
	int						AddChildOL98(int parent, HKEY hChildKey);
	int						AddChildOLExpress(int parent, HKEY hChildKey);
	int						AddChildNS(int parent, HKEY hChildKey, char *PersName);
	int						AddChildEudora(int parent);

	ImportChild *			CreateChildNode(ImportProvider * head, int parent);
	bool					LoadOL98Accounts();
	bool					LoadOLExpressAccounts();
	bool					LoadNSAccounts();
	bool					LoadEudoraAccounts();
	int						AddProvider(char * ProviderName);

	// Generic functions
	void					FindProviders();
	bool					NSExtractValue(char ** Dest, char * Line);
	bool					OL98ExtractValue(char ** Dest, char *KeyName, HKEY hChildKey);
	void					RemoveChildren(ImportChild *trash);
	bool					ValidAccount(HKEY	account);

	void					UpdatePersonalityWazoo();
	void					EmptyTree(ImportProvider **Tree);
	// MAPI Specific Functions

	bool					MAPILogon();
	bool					MAPILogout(bool InvokeHack = FALSE);

public:
	bool					ImportPersonality(ImportChild *Personality);

	bool					ImportMail(ImportChild *child, CWnd *pParentWnd = NULL);
	bool					ImportAddresses(ImportChild *child,  CWnd *pParentWnd = NULL);
	bool					ImportLdif(ImportChild *Child, LPCSTR File, CWnd *pParentWnd = NULL);

//	bool					ImportLdif(ImportChild *Child, char* File);

	bool					ImportMail(ImportProvider *Provider, CWnd *pParentWnd = NULL);
	bool					ImportAddresses(ImportProvider *Provider,  CWnd *pParentWnd = NULL);	
	long					NumOfPersonalities();			

};

void						*AvailableImportMethods();
bool						Import(struct ImportMethod *chosenMethod);



class MAPIImport
{
	typedef ULONG			(CALLBACK *CMIMAPIFreeBuffer)(LPVOID);
	typedef	HRESULT			(CALLBACK *CMIHRQueryAllRows)(LPMAPITABLE, LPSPropTagArray, LPSRestriction, LPSSortOrderSet, LONG, LPSRowSet FAR *);

private:
	LPMAPISESSION			m_lpSession;
	LPMAPITABLE				m_lpTblStores;
	LPMDB					m_lpMDB;     // global pointer to open message store
	LPMAPIPROP				m_lpRootFolder;
	CMIMAPIFreeBuffer		m_MAPIFreeBuffer;
	HINSTANCE				m_hDLL;               // Handle to DLL

public:

	bool					OpenRootFolder();
	bool					ProcessFolder(LPMAPIPROP	ParentFolder,  QCMailboxCommand *ParentFolderCmd);
							~MAPIImport();
							MAPIImport();

	protected:	

	bool					Initialize();
	void					CleanUp();
	bool					OpenDefaultStore();
	void					CloseDefaultStore();
	bool					ProcessFolder(ULONG cbEntryID, LPENTRYID lpEntryID, QCMailboxCommand *ParentFolderCmd);
	bool					IsValidMailFolder(LPMAPIPROP ParentFolder);
	bool					ContainsMessages(LPMAPIPROP	ParentFolder, ULONG * ReturnedNumOfMsgs);
	bool					ContainsSubfolders(LPMAPIPROP CurrentFolder);
	bool					GetMessages(LPMAPIPROP	ParentFolder,  QCMailboxCommand *ParentFolderCmd);
	bool					MakeNewMailbox(LPMAPIPROP	CurrentFolder,  QCMailboxCommand *ParentFolderCmd, QCMailboxCommand **NewFolderCmd);
	bool					KillTheTOCFile(char *pathToMBX);
	QCMailboxCommand *		CreateEudoraFolder(LPMAPIPROP	ParentFolder,  QCMailboxCommand *ParentFolderCmd);
	bool					CommitRowToMbx(LPSRowSet	lpRowSet, JJFile *Mailbox);
	char					* GetLine(IStream * Stream, ULONG * bytesRead, ULONG *posn);
	bool					AddContentHeader(IStream *Stream, JJFile *Mailbox, ULONG *bodytype);
	bool					GrabMsgBody(IStream * BodyStream, JJFile *Mailbox);
	bool					GenerateHeaders(JJFile *Mailbox, char *SenderName, char *SenderEmail, char *Date, char *RcptEmail, char *MessageSubject);
	bool					WriteAttachToMbx(IStream * stream, JJFile * Mailbox, char * name);

public:

	bool					Convert(HINSTANCE hDLL, LPMAPISESSION lpSession);
	static bool				FreeRowSet(CMIMAPIFreeBuffer MAPIFreeBufferfunc, LPSRowSet lpRows);
};




class MAPIAddrImport
{
	typedef HRESULT			(CALLBACK *CMIMAPIInitialize)(LPVOID);
	typedef HRESULT			(CALLBACK *CMIMAPILogonEx)(ULONG, LPTSTR, LPTSTR, FLAGS, LPMAPISESSION FAR *);
	typedef ULONG			(CALLBACK *CMIMAPIFreeBuffer)(LPVOID);
	typedef	HRESULT			(CALLBACK *CMIHRQueryAllRows)(LPMAPITABLE, LPSPropTagArray, LPSRestriction, LPSSortOrderSet, LONG, LPSRowSet FAR *);

private:
	LPMAPISESSION			m_lpSession;
	LPABCONT				m_lpABC;
	LPADRBOOK				m_lpAdrBook;
	CMIMAPIInitialize		m_MAPIInitialize;
	CMIHRQueryAllRows		m_HrQueryAllRows;
	CMIMAPILogonEx			m_MAPILogonEx;
	CMIMAPIFreeBuffer		m_MAPIFreeBuffer;
	HINSTANCE				m_hDLL;               // Handle to DLL
	CMapiInstaller::Status	m_InstalledStatus;

public:
							MAPIAddrImport();
							~MAPIAddrImport();
	bool					Convert(HINSTANCE hDLL, LPMAPISESSION lpSession);
private:

	bool					Initialize();
	AbookEntry *			ReadInEntry();
	bool					OpenAddressBook();
	bool					ProcessABook(AbookCreator *abookcreator);
	bool					ProcessABEntry(LPMAPIPROP ABEntry, AbookCreator *abookcreator);
	void					CloseAddressBook();

};




void						ImportMe();
bool						FreeRowSet(LPSRowSet lpRows);

#endif // _MAPIIMPORT_H_