#ifndef _OL_IMPORT_H_
#define _OL_IMPORT_H_

#include "stdafx.h"


#define _MAX_PATH 260

enum AddressType {MESSAGING_USER, DIST_LIST}; // type of the entry in the address book
enum FolderType {IN_MBOX, OUT_MBOX, TRASH_MBOX, REGULAR_MBOX}; // type of mailbox... whether it is the
																 // special IN, OUT, TRASH or just a regular one
enum ImportType {MBOX_ABOOK_PERSONA, MBOX_ABOOK, PERSONA, MBOX};// import either i) mail, address book and personality
																// ii) mail and address book iii) personality only
																// iv) mailboxes only
typedef struct OL5FolderInfo_tag
{

	long		lOLMBoxID;
	long		lOLParentMBoxID;
	bool		bOLHasSubfolders;

} OE5FolderInfo;

typedef struct	CMbox_FnamePair_tag
{
	char *					lpszDisplayName;	// name of the mailbox found
	char *					lpszFilename;		// file that is temp created for the mailbox
	long					lMBoxID;			// ID for this mailbox.
	long					lParentMBoxID;		// ID for the parent mailbox.
	bool					bHasSubfolders;		// record whether or not folder has subfolders.
	char *					lpszMailbox;		// record the Eudora mailbox created for this mailbox.
	bool					bCreated;			// Record whether the mailbox for this been created yet.
	void *					pID;				// stores the Entry ID in MAPI that is used as an identifier to 
												// fetch the mailbox from the message store
	unsigned int			iSize;				// size of the entry identifier pID
	FolderType				folderType;			// enum defining what type of mailbox it is i.e. IN, OUT, TRASH etc
	struct CMbox_FnamePair_tag*	pNext;
} CMbox_FnamePair;

typedef struct CMessageStore_tag
{

	char	*					lpszDisplayName; // name of the message store
	CMbox_FnamePair *			lpMBoxID;		 // pointer to the list of folders under the message store
	void    *					pDB;			 // store the pointer to the message store in MAPI to allow us to reopen it later and fetch the messages
	bool						bIsDefault;		 // is it the default message store	
	struct CMessageStore_tag*	pNext;	
} CMessageStore;

typedef struct CImportChild_tag
{
	int						iAccountID;
	char					*lpszAccountDisplayName;
	char					*lpszIdentityName;
	bool					bLMOS;
	bool					bIsIMAP;
	char					*lpszPersonalityName;
	char					*lpszIncomingUserName;	//POP or IMAP based on IsIMAP flag;
	char					*lpszIncomingServer;
	char					*lpszRealName;
	char					*lpszEmailAddress;
	char					*lpszSMTPServer;
struct CImportChild_tag			*pNext;
struct CImportProvider_tag		*pParent;
	bool					bHasMail;
	bool					bHasAddresses;
	bool					bHasLdif;
	ImportType				kImportType;
	char					*lpszPathToMailData;
	char					*lpszPathToAdrData;
	char					*lpszPathtoLdif;
	char					m_szIdentityUID[_MAX_PATH];
} CImportChild;

typedef struct CImportProvider_tag
{
	char					*lpszDisplayName;	// name of the profile
	CImportChild			*pFirstChild;		
	int						iNumChildren;
	bool					bHasMail;
	bool					bHasAddresses;
	bool					bHasLdif;
	CImportProvider_tag *	pNext;
} CImportProvider;

typedef struct CAbookEntry_tag
{
	
	void		*pID;				// entry indentifier for a messaging user
	ULONG		IDSize;				// size of the ID

	char		*first_name;
	char		*middle_name;		
	char		*last_name;
	char		*title;				
	char		*nickname;
	char		*default_add;
	char		*addresses;
	char		*street_addr;
	char		*city;
	char		*state;
	char		*zip;
	char		*country;
	char		*phone;
	char		*fax;
	char		*cell_phone;
	char		*web_page;
	char		*company;
	char		*wrk_street_addr;	
	char		*wrk_city;			
	char		*wrk_state;			
	char		*wrk_zip;			
	char		*wrk_country;		
	char		*wrk_title;
	char		*wrk_dept;			
	char		*wrk_office;
	char		*wrk_phone;
	char		*wrk_fax;			
	char		*wrk_pgr;			
	char		*wrk_ip_phone;		
	char		*wrk_web_page;		
	char		*spouse;			
	char		*children;			
	char		*gender;			
	char		*birthday;			
	char		*anniversary;		
	char		*notes;
	char		*conf_server;		
	char		*def_conf_add;		
	char		*conf_addresses;	

	long		type;

	long		m_lImportID;
	
	CAbookEntry_tag	*pNext;

}CAbookEntry;

typedef struct CAbookDistListEntry_tag
{
	void						*pID;		// id of the messaging user/dist list
	ULONG						IDSize;		// size of the ID
	AddressType					entryType;	// type - messaging user/dist list
	void						*pABNode;	// pointer to a node in the address book - messaging user/dist list node
	CAbookDistListEntry_tag		*pNext;		// ptr to next node
} CAbookDistListEntry;

typedef struct CAbookDistListTree_tag
{
	void					*pID;			// ID of the distribution list
	ULONG					IDSize;			// size of the ID
	char					*pNickname;		// nick name for the distribution list
	CAbookDistListEntry		*pFirstDLChild;	// pointer to a linked list of distribution list members (messaging users/dist lists)
	CAbookDistListTree_tag	*pNext;			// ptr to next node
} CAbookDistListTree;

typedef struct CAddressBookTree_tag
{
	char					*lpszDisplayName;	// name of the address book
	CAbookEntry				*pFirstChild;		// linked list of messaging users in the address book
	CAbookDistListTree		*pFirstDLChild;		// linked list of dist lists in the address book
	CAddressBookTree_tag	*pNext;				// ptr to next node
} CAddressBookTree;

typedef struct CPersonalityTree_tag
{
	char	*pPersName;
	char	*pRealName;			// IDS_INI_REAL_NAME
	char	*pPOPAccount;		// IDS_INI_POP_ACCOUNT
	char	*pLoginName;		// IDS_INI_LOGIN_NAME
	char	*pInServer;			// IDS_INI_POP_SERVER
	char	*pOutServer;		// IDS_INI_SMTP_SERVER
	char	*pReturnAddress;	// IDS_INI_RETURN_ADDRESS
	char	*pDefaultDomain;	// IDS_INI_DOMAIN_QUALIFIER
	char	*pStationery;		// IDS_INI_STATIONERY
	char	*pSignature;		// IDS_INI_SIGNATURE_NAME
	char	*pIMAPPrefix;		// IDS_INI_IMAP_PREFIX

	char	*pLeaveOnServDays;	// IDS_INI_LEAVE_ON_SERVER_DAYS
	char	*pBigMsgThreshold;	// IDS_INI_BIG_MESSAGE_THRESHOLD
	char	*pIMAPMaxSize;		// IDS_INI_IMAP_MAXSIZE
	char	*pIMAPTrashMailbox;	// IDS_INI_IMAP_TRASH_MBOXNAME.

	BOOL	bCheckMail;			// IDS_INI_PERSONA_CHECK_MAIL
	BOOL	bLMOS;				// IDS_INI_PERSONA_LMOS
	BOOL	bPOP;				// IDS_INI_USES_POP
	BOOL	bIMAP;				// IDS_INI_USES_IMAP
	BOOL	bPassword;			// IDS_INI_AUTH_PASS
	BOOL	bKerberos;			// IDS_INI_AUTH_KERB
	BOOL	bAPop;				// IDS_INI_AUTH_APOP
	BOOL	bRPA;				// IDS_INI_AUTH_RPA
	BOOL	bWinSock;			// IDS_INI_CONNECT_WINSOCK
	BOOL	bDelServerAfter;	// IDS_INI_DELETE_MAIL_FROM_SERVER
	BOOL	bDelWhenTrashed;	// IDS_INI_SERVER_DELETE
	BOOL	bSkipBigMsgs;		// IDS_INI_SKIP_BIG_MESSAGES
	BOOL	bIMAPMinDwnld;		// IDS_INI_IMAP_MINDNLOAD
	BOOL	bIMAPFullDwnld;		// IDS_INI_IMAP_OMITATTACH
	BOOL	bIMAPXferToTrash;	// IDS_INI_IMAP_XFERTOTRASH.
	BOOL	bIMAPMarkDeleted;	// IDS_INI_IMAP_MARK_DELETED.
	BOOL	bSMTPAuthAllowed;	// IDS_INI_SMTP_AUTH_ALLOWED
	BOOL	bUseSMTPRelay;		// IDS_INI_PERSONA_USE_RELAY
	int		iSSLReceiveUsage;	// IDS_INI_SSL_RECEIVE_USE
	int		iSSLSendUsage;		// IDS_INI_SSL_SEND_USE	
	CPersonalityTree_tag	*pNext;				// ptr to next node
} CPersonalityTree;

#endif //_OL_IMPORT_H_
