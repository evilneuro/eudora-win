// PersParams.cpp: implementation of the CPersParams class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "rs.h"

#include "PersParams.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//	if (GetIniShort(IDS_INI_USE_SIGNATURE)

CPersParams::CPersParams()
{
	PersName = "";
	POPAccount = "";			// IDS_INI_POP_ACCOUNT
	LoginName = "";				// IDS_INI_LOGIN_NAME
	InServer = "";				// IDS_INI_POP_SERVER
	RealName = "";				// IDS_INI_REAL_NAME
	ReturnAddress = "";			// IDS_INI_RETURN_ADDRESS
	DefaultDomain = "";			// IDS_INI_DOMAIN_QUALIFIER
	OutServer = "";			// IDS_INI_SMTP_SERVER
	Stationery = "";			// IDS_INI_STATIONERY
	Signature = "";				// IDS_INI_SIGNATURE_NAME
	IMAPPrefix = "";			// IDS_INI_IMAP_PREFIX
	LeaveOnServDays = "";		// IDS_INI_LEAVE_ON_SERVER_DAYS
	BigMsgThreshold = "";		// IDS_INI_BIG_MESSAGE_THRESHOLD
	IMAPMaxSize = "";			// IDS_INI_IMAP_MAXSIZE
	IMAPTrashMailbox = "";		// IDS_INI_IMAP_TRASH_MBOXNAME
	bCheckMail = FALSE;			// IDS_INI_PERSONA_CHECK_MAIL
	bLMOS = FALSE;				// IDS_INI_PERSONA_LMOS
	bPOP = FALSE;				// IDS_INI_USES_POP
	bIMAP = FALSE;				// IDS_INI_USES_IMAP
	bPassword = FALSE;			// IDS_INI_AUTH_PASS
	bKerberos = FALSE;			// IDS_INI_AUTH_KERB
	bAPop = FALSE;				// IDS_INI_AUTH_APOP
	bRPA = FALSE;				// IDS_INI_AUTH_RPA
	bWinSock = FALSE;			// IDS_INI_CONNECT_WINSOCK
	bDelServerAfter = FALSE;	// IDS_INI_DELETE_MAIL_FROM_SERVER
	bDelWhenTrashed = FALSE;	// IDS_INI_SERVER_DELETE
	bSkipBigMsgs = FALSE;		// IDS_INI_SKIP_BIG_MESSAGES
	bIMAPMinDwnld = FALSE;		// IDS_INI_IMAP_MINDNLOAD
	bIMAPFullDwnld = FALSE;		// IDS_INI_IMAP_OMITATTACH
	bIMAPXferToTrash = FALSE;   // IDS_INI_IMAP_XFERTOTRASH
	bIMAPMarkDeleted = FALSE;	// IDS_INI_IMAP_MARK_DELETED
}

CPersParams::CPersParams(const CPersParams &copy)
{
	PersName = copy.PersName;
	POPAccount = copy.POPAccount;
	LoginName = copy.LoginName;
	InServer = copy.InServer;
	RealName = copy.RealName;
	ReturnAddress = copy.ReturnAddress;
	DefaultDomain = copy.DefaultDomain;
	OutServer = copy.OutServer;
	Stationery = copy.Stationery;
	Signature = copy.Signature;
	IMAPPrefix = copy.IMAPPrefix;
	LeaveOnServDays = copy.LeaveOnServDays;
	BigMsgThreshold = copy.BigMsgThreshold;
	IMAPMaxSize = copy.IMAPMaxSize;
	bCheckMail = copy.bCheckMail;
	bLMOS = copy.bLMOS;
	bPOP = copy.bPOP;
	bIMAP = copy.bIMAP;
	bPassword = copy.bPassword;
	bKerberos = copy.bKerberos;
	bAPop = copy.bAPop;
	bRPA = copy.bRPA;
	bWinSock = copy.bWinSock;
	bDelServerAfter = copy.bDelServerAfter;
	bDelWhenTrashed = copy.bDelWhenTrashed;
	bSkipBigMsgs = copy.bSkipBigMsgs;
	bIMAPMinDwnld = copy.bIMAPMinDwnld;
	bIMAPFullDwnld = copy.bIMAPFullDwnld;
	bIMAPMarkDeleted = copy.bIMAPMarkDeleted;
	bIMAPXferToTrash = copy.bIMAPXferToTrash;
	IMAPTrashMailbox = copy.IMAPTrashMailbox;
}

// --------------------------------------------------------------------------

CPersParams::~CPersParams()
{

}

// --------------------------------------------------------------------------

bool CPersParams::SetInServType(const ServType &type)
{
	switch (type)
	{
		case IST_POP:
		{
			bPOP = TRUE;
			bIMAP = FALSE;
		}
		break;

		case IST_IMAP:
		{
			bIMAP = TRUE;
			bPOP = FALSE;
		}
		break;

		default:
		{
			return (false);
		}
		break;
	}

	return (true);
}

// --------------------------------------------------------------------------

bool CPersParams::GetInServType(ServType &type) const
{
	if (bPOP)
		type = IST_POP;
	else if (bIMAP)
		type = IST_IMAP;
	else
		return (false);

	return (true);
}

// --------------------------------------------------------------------------

bool CPersParams::UpdatePopAccount()
{
	if ((!LoginName.IsEmpty()) && (!InServer.IsEmpty()))
		POPAccount = LoginName + '@' + InServer;
	else
		POPAccount.Empty();
	
	return (true);
}

// --------------------------------------------------------------------------

// [PUBLIC] GetDefaultParams
//
// Overwrites the given params with the defaults.
//
bool CPersParams::GetDefaultParams()
{
	VERIFY(GetDefaultIniSetting(IDS_INI_POP_ACCOUNT, POPAccount));
	VERIFY(GetDefaultIniSetting(IDS_INI_LOGIN_NAME, LoginName));
	VERIFY(GetDefaultIniSetting(IDS_INI_POP_SERVER, InServer));
	VERIFY(GetDefaultIniSetting(IDS_INI_REAL_NAME, RealName));
	VERIFY(GetDefaultIniSetting(IDS_INI_RETURN_ADDRESS, ReturnAddress));
	VERIFY(GetDefaultIniSetting(IDS_INI_DOMAIN_QUALIFIER, DefaultDomain));
	VERIFY(GetDefaultIniSetting(IDS_INI_SMTP_SERVER, OutServer));
	VERIFY(GetDefaultIniSetting(IDS_INI_PERSONA_STATIONERY, Stationery));
	VERIFY(GetDefaultIniSetting(IDS_INI_SIGNATURE_NAME, Signature));
	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_PREFIX, IMAPPrefix));
	VERIFY(GetDefaultIniSetting(IDS_INI_LEAVE_ON_SERVER_DAYS, LeaveOnServDays));
	VERIFY(GetDefaultIniSetting(IDS_INI_BIG_MESSAGE_THRESHOLD, BigMsgThreshold));
	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_MAXSIZE, IMAPMaxSize));
	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_TRASH_MBOXNAME, IMAPTrashMailbox));

	CString str;
	VERIFY(GetDefaultIniSetting(IDS_INI_PERSONA_CHECK_MAIL, str));
	bCheckMail = (atoi(str) != 0);

	VERIFY(GetDefaultIniSetting(IDS_INI_PERSONA_LMOS, str));
	bLMOS = (atoi(str) != 0);

	VERIFY(GetDefaultIniSetting(IDS_INI_USES_POP, str));
	bPOP = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_USES_IMAP, str));
	bIMAP = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_AUTH_PASS, str));
	bPassword = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_AUTH_KERB, str));
	bKerberos = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_AUTH_APOP, str));
	bAPop = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_AUTH_RPA, str));
	bRPA = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_CONNECT_WINSOCK, str));
	bWinSock = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_DELETE_MAIL_FROM_SERVER, str));
	bDelServerAfter = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_SERVER_DELETE, str));
	bDelWhenTrashed = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_SKIP_BIG_MESSAGES, str));
	bSkipBigMsgs = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_MINDNLOAD, str));
	bIMAPMinDwnld = (atoi(str) != 0);
	
	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_OMITATTACH, str));
	bIMAPFullDwnld = (atoi(str) != 0);

	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_XFERTOTRASH, str));
	bIMAPXferToTrash = (atoi(str) != 0);

	VERIFY(GetDefaultIniSetting(IDS_INI_IMAP_MARK_DELETED, str));
	bIMAPMarkDeleted = (atoi(str) != 0);


	return (true);
}
