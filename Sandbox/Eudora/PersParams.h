// PersParams.h: interface for the CPersParams class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSPARAMS_H__E76A09A6_0572_11D2_94D2_00805F9BF4D7__INCLUDED_)
#define AFX_PERSPARAMS_H__E76A09A6_0572_11D2_94D2_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CPersParams  
{
public:
	typedef enum { IST_UNKNOWN, IST_POP, IST_IMAP } ServType;

	CPersParams();
	CPersParams(const CPersParams &copy);
	virtual ~CPersParams();

	bool SetInServType(const ServType &type);
	bool GetInServType(ServType &type) const;

	bool UpdatePopAccount();
	bool GetDefaultParams();

	CString	PersName;
	CString RealName;			// IDS_INI_REAL_NAME
	CString	POPAccount;			// IDS_INI_POP_ACCOUNT
	CString	LoginName;			// IDS_INI_LOGIN_NAME
	CString InServer;			// IDS_INI_POP_SERVER
	CString OutServer;			// IDS_INI_SMTP_SERVER
	CString ReturnAddress;		// IDS_INI_RETURN_ADDRESS
	CString DefaultDomain;		// IDS_INI_DOMAIN_QUALIFIER
	CString	Stationery;			// IDS_INI_STATIONERY
	CString Signature;			// IDS_INI_SIGNATURE_NAME
	CString	IMAPPrefix;			// IDS_INI_IMAP_PREFIX

	CString	LeaveOnServDays;	// IDS_INI_LEAVE_ON_SERVER_DAYS
	CString	BigMsgThreshold;	// IDS_INI_BIG_MESSAGE_THRESHOLD
	CString	IMAPMaxSize;		// IDS_INI_IMAP_MAXSIZE
	CString IMAPTrashMailbox;	// IDS_INI_IMAP_TRASH_MBOXNAME.

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
};

#endif // !defined(AFX_PERSPARAMS_H__E76A09A6_0572_11D2_94D2_00805F9BF4D7__INCLUDED_)
