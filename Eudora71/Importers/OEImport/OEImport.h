#ifndef _OE_IMPORT_H_
#define _OE_IMPORT_H_

#include "stdafx.h"



#define _MAX_PATH 260

enum AddressType {MESSAGING_USER, DIST_LIST};
enum FolderType {IN_MBOX, OUT_MBOX, TRASH_MBOX, REGULAR_MBOX};
enum ImportType {MBOX_ABOOK_PERSONA, MBOX_ABOOK, PERSONA, MBOX};// import either i) mail, address book and personality
																// ii) mail and address book iii) personality only
																// iv) mailboxes only
typedef struct OE5FolderInfo_tag
{

	long		lOE5MBoxID;
	long		lOE5ParentMBoxID;
	bool		bOE5HasSubfolders;

} OE5FolderInfo;

typedef struct	CMbox_FnamePair_tag
{
	char *					lpszDisplayName;
	char *					lpszFilename;
	long					lMBoxID;			// DRW 9/20/99 - Record the OE5 ID for this mailbox.
	long					lParentMBoxID;		// DRW 9/20/99 - Record the OE5 ID for the parent mailbox.
	bool					bHasSubfolders;		// DRW 9/22/99 - Record whether or not folder has subfolders.
	char *					lpszMailbox;			// DRW 9/20/99 - Record the Eudora mailbox created for this mailbox.
	bool					bCreated;			// DRW 2/3/00 - Record whether the mailbox for this been created yet.
	void *					pID;				// stores the Entry ID in MAPI that is used as an identifier to 
												// fetch the mailbox from the message store
	unsigned int			iSize;				// size of the entry identifier pID
	FolderType				folderType;			// enum defining what type of mailbox it is i.e. IN, OUT, TRASH etc
	struct CMbox_FnamePair_tag*	pNext;
} CMbox_FnamePair;

typedef struct CMessageStore_tag
{

	char	*					lpszDisplayName;
	CMbox_FnamePair *			lpMBoxID;
	void    *					pDB;
	bool						bIsDefault;
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
	char					*lpszDisplayName;
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
	ULONG		IDSize;			// size of the ID

	char		*first_name;
	char		*middle_name;		// WAB only
	char		*last_name;
	char		*title;				// WAB only
	char		*nickname;
	char		*default_add;
	char		*addresses;
	// WAB only: Send E-Mail using plain text only???
	// NS only: Prefers rich text (HTML) mail
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
	char		*wrk_street_addr;	// WAB only
	char		*wrk_city;			// WAB only
	char		*wrk_state;			// WAB only
	char		*wrk_zip;			// WAB only
	char		*wrk_country;		// WAB only
	char		*wrk_title;
	char		*wrk_dept;			// WAB only
	char		*wrk_office;
	char		*wrk_phone;
	char		*wrk_fax;			// WAB only
	char		*wrk_pgr;			// WAB only
	char		*wrk_ip_phone;		// WAB only
	char		*wrk_web_page;		// WAB only
	char		*spouse;			// WAB only
	char		*children;			// WAB only
	char		*gender;			// WAB only
	char		*birthday;			// WAB only
	char		*anniversary;		// WAB only
	char		*notes;
	char		*conf_server;		// WAB only
	char		*def_conf_add;		// WAB only
	char		*conf_addresses;	// WAB only

	long		type;

	long		m_lImportID;
	
	CAbookEntry_tag	*pNext;

}CAbookEntry;

typedef struct CAbookDistListEntry_tag
{
	void						*pID;		// id of the messaging user/dist list
	ULONG						IDSize;	// size of the ID
	AddressType				entryType;	// type - messaging user/dist list
	void						*pABNode;	// pointer to a node in the address book - messaging user/dist list node
	CAbookDistListEntry_tag		*pNext;		// ptr to next node
} CAbookDistListEntry;

typedef struct CAbookDistListTree_tag
{
	void					*pID;			// ID of the distribution list
	ULONG					IDSize;		// size of the ID
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
	char	*pLoginName;			// IDS_INI_LOGIN_NAME
	char	*pInServer;			// IDS_INI_POP_SERVER
	char	*pOutServer;			// IDS_INI_SMTP_SERVER
	char	*pReturnAddress;		// IDS_INI_RETURN_ADDRESS
	char	*pDefaultDomain;		// IDS_INI_DOMAIN_QUALIFIER
	char	*pStationery;			// IDS_INI_STATIONERY
	char	*pSignature;			// IDS_INI_SIGNATURE_NAME
	char	*pIMAPPrefix;			// IDS_INI_IMAP_PREFIX

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
	BOOL	bUseSMTPRelay;	// IDS_INI_PERSONA_USE_RELAY
	int		iSSLReceiveUsage;	// IDS_INI_SSL_RECEIVE_USE
	int		iSSLSendUsage;		// IDS_INI_SSL_SEND_USE	
	CPersonalityTree_tag	*pNext;				// ptr to next node
} CPersonalityTree;

#endif //_OE_IMPORT_H_