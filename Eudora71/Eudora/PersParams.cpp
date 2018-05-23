// PersParams.cpp: implementation of the CPersParams class.
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "resource.h"
#include "rs.h"

#include "PersParams.h"

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//	if (GetIniShort(IDS_INI_USE_SIGNATURE)

CPersParams::CPersParams()
{
	PersName.Empty();
	POPAccount.Empty();			// IDS_INI_POP_ACCOUNT
	LoginName.Empty();			// IDS_INI_LOGIN_NAME
	InServer.Empty();			// IDS_INI_POP_SERVER
	RealName.Empty();			// IDS_INI_REAL_NAME
	ReturnAddress.Empty();		// IDS_INI_RETURN_ADDRESS
	DefaultDomain.Empty();		// IDS_INI_DOMAIN_QUALIFIER
	OutServer.Empty();			// IDS_INI_SMTP_SERVER
	Stationery.Empty();			// IDS_INI_STATIONERY
	Signature.Empty();			// IDS_INI_SIGNATURE_NAME
	IMAPPrefix.Empty();			// IDS_INI_IMAP_PREFIX
	LeaveOnServDays.Empty();	// IDS_INI_LEAVE_ON_SERVER_DAYS
	BigMsgThreshold.Empty();	// IDS_INI_BIG_MESSAGE_THRESHOLD
	IMAPMaxSize.Empty();		// IDS_INI_IMAP_MAXSIZE
	IMAPTrashMailbox.Empty();	// IDS_INI_IMAP_TRASH_MBOXNAME
	strIMAPAutoExpPct.Empty();	// IDS_INI_IMAP_AUTO_EXPUNGE_PCT
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
	bIMAPAutoExpNever = FALSE;	// IDS_INI_IMAP_AUTO_EXPUNGE_NEVER
	bIMAPAutoExpAlways = FALSE;	// IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS
	bIMAPAutoExpOnPct = FALSE;	// IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT
	bSMTPAuthAllowed = TRUE;	// IDS_INI_SMTP_AUTH_ALLOWED
	m_SSLReceiveUsage = 0;		// IDS_INI_SSL_RECEIVE_USE
	m_SSLSendUsage = 0;			// IDS_INI_SSL_SEND_USE
	bUseSubmissionPort = FALSE;	// IDS_INI_USE_SUBMISSION_PORT
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
	strIMAPAutoExpPct = copy.strIMAPAutoExpPct;
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
	bIMAPAutoExpNever = copy.bIMAPAutoExpNever;
	bIMAPAutoExpAlways = copy.bIMAPAutoExpAlways;
	bIMAPAutoExpOnPct = copy.bIMAPAutoExpOnPct;
	bSMTPAuthAllowed = copy.bSMTPAuthAllowed;
	bUseSMTPRelay = copy.bUseSMTPRelay;
	m_SSLReceiveUsage = copy.m_SSLReceiveUsage;
	m_SSLSendUsage = copy.m_SSLSendUsage;
	bUseSubmissionPort = copy.bUseSubmissionPort;
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
	CString str;

	GetDefaultIniSetting(IDS_INI_POP_ACCOUNT, POPAccount);
	GetDefaultIniSetting(IDS_INI_LOGIN_NAME, LoginName);
	GetDefaultIniSetting(IDS_INI_POP_SERVER, InServer);
	GetDefaultIniSetting(IDS_INI_REAL_NAME, RealName);
	GetDefaultIniSetting(IDS_INI_RETURN_ADDRESS, ReturnAddress);
	GetDefaultIniSetting(IDS_INI_DOMAIN_QUALIFIER, DefaultDomain);
	GetDefaultIniSetting(IDS_INI_SMTP_SERVER, OutServer);
	GetDefaultIniSetting(IDS_INI_PERSONA_STATIONERY, Stationery);
	GetDefaultIniSetting(IDS_INI_SIGNATURE_NAME, Signature);
	GetDefaultIniSetting(IDS_INI_IMAP_PREFIX, IMAPPrefix);
	GetDefaultIniSetting(IDS_INI_LEAVE_ON_SERVER_DAYS, LeaveOnServDays);
	GetDefaultIniSetting(IDS_INI_BIG_MESSAGE_THRESHOLD, BigMsgThreshold);
	GetDefaultIniSetting(IDS_INI_IMAP_MAXSIZE, IMAPMaxSize);
	GetDefaultIniSetting(IDS_INI_IMAP_TRASH_MBOXNAME, IMAPTrashMailbox);
	GetDefaultIniSetting(IDS_INI_IMAP_AUTO_EXPUNGE_PCT, strIMAPAutoExpPct);

	GetDefaultIniSetting(IDS_INI_PERSONA_CHECK_MAIL, str);		bCheckMail			= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_PERSONA_LMOS, str);			bLMOS				= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_USES_POP, str);				bPOP				= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_USES_IMAP, str);				bIMAP				= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_AUTH_PASS, str);				bPassword			= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_AUTH_KERB, str);				bKerberos			= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_AUTH_APOP, str);				bAPop				= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_AUTH_RPA, str);				bRPA				= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_CONNECT_WINSOCK, str);			bWinSock			= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_DELETE_MAIL_FROM_SERVER, str);	bDelServerAfter		= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_SERVER_DELETE, str);			bDelWhenTrashed		= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_SKIP_BIG_MESSAGES, str);		bSkipBigMsgs		= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_MINDNLOAD, str);			bIMAPMinDwnld		= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_OMITATTACH, str);			bIMAPFullDwnld		= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_XFERTOTRASH, str);		bIMAPXferToTrash	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_MARK_DELETED, str);		bIMAPMarkDeleted	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER, str);	bIMAPAutoExpNever	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS, str);	bIMAPAutoExpAlways	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT, str);	bIMAPAutoExpOnPct	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_SMTP_AUTH_ALLOWED, str);		bSMTPAuthAllowed	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_PERSONA_USE_RELAY, str);		bUseSMTPRelay	= (atoi(str) != 0);
	GetDefaultIniSetting(IDS_INI_SSL_RECEIVE_USE, str);			m_SSLReceiveUsage	= atoi(str);
	GetDefaultIniSetting(IDS_INI_SSL_SEND_USE, str);			m_SSLSendUsage		= atoi(str);
	GetDefaultIniSetting(IDS_INI_USE_SUBMISSION_PORT, str);		bUseSubmissionPort	= (atoi(str) != 0);

	return (true);
}
