// PersParams.h: interface for the CPersParams class.
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
	CString	strIMAPAutoExpPct;	// IDS_INI_IMAP_AUTO_EXPUNGE_PCT

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
	BOOL	bIMAPAutoExpNever;	// IDS_INI_IMAP_AUTO_EXPUNGE_NEVER
	BOOL	bIMAPAutoExpAlways;	// IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS
	BOOL	bIMAPAutoExpOnPct;	// IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT
	BOOL	bSMTPAuthAllowed;	// IDS_INI_SMTP_AUTH_ALLOWED
	BOOL	bUseSMTPRelay;	// IDS_INI_PERSONA_USE_RELAY
	int		m_SSLReceiveUsage;	// IDS_INI_SSL_RECEIVE_USE
	int		m_SSLSendUsage;		// IDS_INI_SSL_SEND_USE
	BOOL    bUseSubmissionPort; // IDS_INI_USE_SUBMISSION_PORT
};

#endif // !defined(AFX_PERSPARAMS_H__E76A09A6_0572_11D2_94D2_00805F9BF4D7__INCLUDED_)
