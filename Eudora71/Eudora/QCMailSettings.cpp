// QCMailSettings.cpp
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

#include "rs.h"
#include "resource.h"
#include "persona.h"

#include "QCNetSettings.h"
#include "QCSMTPSettings.h"
#include "QCPOPSettings.h"
#include "QCRas.h"

#include "QCUtils.h"   //for IsMainThreadMT
#include "pop.h"

#include "address.h"

#include "DebugNewHelpers.h"


QCNetworkSettings::~QCNetworkSettings()
{
	if(m_pRasDialParams)
		delete m_pRasDialParams;
}

int QCNetworkSettings::GrabSettings()
{

	char buf[QCMS_BUF_SIZE];
	//
	// Must set persona to get the proper INI strings
	// for this personality.  Set it back when done.
	//
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_strPersona );

	m_bDoCaching				= GetIniShort(IDS_INI_NETWORK_CACHING)	?true:false;
	m_bConnectOffline			= GetIniShort(IDS_INI_CONNECT_OFFLINE)	?true:false;
	m_bAuthenticateKerberos		= GetIniShort(IDS_INI_AUTH_KERB)		?true:false;
	m_bAuthenticateKerberos4	= GetIniShort(IDS_INI_KERB_4_ON)		?true:false;
	m_bDialupConnection			= GetIniShort(IDS_INI_AUTO_CONNECTION)	?true:false;

	m_nNetworkBufferSize	= GetIniLong(IDS_INI_NETWORK_BUFFER_SIZE);

	ASSERT(m_nNetworkBufferSize > 1024);

	m_nNetworkOpenTimeout	= GetIniLong(IDS_INI_NETWORK_OPEN_TIMEOUT);
	m_nNetworkTimeout		= GetIniLong(IDS_INI_NETWORK_TIMEOUT);
	m_bNetImmediateClose = GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE)?true:false;

	if(m_bDialupConnection)
	{
		m_strRASEntryName	= GetIniString(IDS_INI_AUTO_CONNECTION_NAME, buf, QCMS_BUF_SIZE);
		m_bRASSavePassword	= false;  //Get it from RAS auth dialog

		m_pRasDialParams = DEBUG_NEW RASDIALPARAMS;
		QCRasLibrary::ResetRasDialParams(m_pRasDialParams, m_strRASEntryName);
			
		//m_strRASUserName	= GetIniString(IDS_INI_AUTO_CONNECTION_USERNAME, buf, QCMS_BUF_SIZE);
		//m_strRASPassword	= GetIniString(IDS_INI_AUTO_CONNECTION_PASSWORD, buf, QCMS_BUF_SIZE);
		//m_strRASDomain		= GetIniString(IDS_INI_AUTO_CONNECTION_DOMAIN, buf, QCMS_BUF_SIZE);
		//m_bRASSavePassword	= GetIniShort(IDS_INI_AUTO_CONNECTION_SAVE_PASSWORD)	?true:false;
	}

	//POP
	//m_szPOPAccount			= GetIniString(IDS_INI_POP_ACCOUNT, buf, QCMS_BUF_SIZE);



	//m_szSMTPServer = GetIniString(IDS_INI_SMTP_SERVER,buf, sizeof(buf));
	//m_szPOPAccount = GetIniString(IDS_INI_POP_ACCOUNT,buf, sizeof(buf));
	
	//
	// Back to where it was.
	//
	g_Personalities.SetCurrent( CachePersonality );
	
	return 1;
}


bool QCNetworkSettings::GetRasPassword()
{
	if(IsDialupConnection())
	{
		if(!QCRasLibrary::IsReady())
			return false;
				
		if( ! QCRasLibrary::GetRasPassword(GetRASEntryName(), GetRasDialParams(), &m_bRASSavePassword) )	
			return false;
	}
	
	return true;
}





QCPOPSettings::QCPOPSettings(const char* persona, unsigned long ulBitFlags) 
		: m_szPersona(persona), m_ulBitFlags(ulBitFlags)
{	
	m_pNetSettings = DEBUG_NEW QCNetworkSettings(m_szPersona); 
	GrabSettings(); 
}


QCPOPSettings::~QCPOPSettings()
{ 
	delete m_pNetSettings; 
}


int QCPOPSettings::GrabSettings()
{
	ASSERT( ::IsMainThreadMT());

	//
	// Must set persona to get the proper INI strings
	// for this personality.  Set it back when done.
	//
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_szPersona );

	m_strPOPAccount = GetIniString(IDS_INI_POP_ACCOUNT);

	if( m_szPersona.IsEmpty())
		m_strLMOSEntryName = m_strPOPAccount;
	else{
		m_strLMOSEntryName = m_strPOPAccount;
		m_strLMOSEntryName += "-";
		m_strLMOSEntryName += m_szPersona;
	}

	m_bUsePOPCAPA = (GetIniShort(IDS_INI_USE_POP_CAPA))?true:false;
	m_bUsePOPMangle = (GetIniShort(IDS_INI_USE_POP_MANGLE))?true:false;

	m_nPopPort = (int) GetIniShort(IDS_INI_POP_PORT);

	m_bFirstUnreadNormal = (GetIniShort(IDS_INI_FIRST_UNREAD_NORMAL))?true:false;
	m_bUsePOPLast = (GetIniShort(IDS_INI_USE_POP_LAST))?true:false;

	if (m_bFirstUnreadNormal)
		m_FetchType = FETCH_NORMAL;
	else if (m_bUsePOPLast)
		m_FetchType = FETCH_LAST;
	else
		m_FetchType = FETCH_MSG_STATUS;
	

	//LMOS
	m_bSkipBigMessages = GetIniShort(IDS_INI_SKIP_BIG_MESSAGES)?true:false;
	m_BigMsgThreshold = GetIniLong(IDS_INI_BIG_MESSAGE_THRESHOLD);
	m_strAttachDir = GetIniString(IDS_INI_AUTO_RECEIVE_DIR);
	
	m_bLeaveMailOnServer = GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER)?true:false;
	m_bDeleteMailAfterSomeDays = GetIniShort(IDS_INI_DELETE_MAIL_FROM_SERVER)?true:false;
	m_nDaysToLeaveOnServer = GetIniLong(IDS_INI_LEAVE_ON_SERVER_DAYS);

	m_bSendOnCheck = GetIniShort(IDS_INI_SEND_ON_CHECK)?true:false;
	m_bUsePOPSend =  (GetIniShort(IDS_INI_USE_POP_SEND))?true:false;
	m_strBadPass =	GetIniString(IDS_INI_BAD_PASSWORD_STRING);
	m_strPasswordOKWordList = GetIniString(IDS_INI_PASSWORD_OK_WORDLIST);
	m_bSavePassword = GetIniShort(IDS_INI_SAVE_PASSWORD)?true:false;
	m_strSavePassword = GetIniString(IDS_INI_SAVE_PASSWORD_TEXT);
	m_bConnectOffline = GetIniShort(IDS_INI_CONNECT_OFFLINE)?true:false;

	if (GetIniShort(IDS_INI_AUTH_KERB))
	{
		m_bAuthKerb = GetIniShort(IDS_INI_KERB_4_ON)?true:false;
		m_bAuthKerb5 = GetIniShort(IDS_INI_KERB_4_OFF)?true:false;
	}
	else
	{
		m_bAuthKerb = false;
		m_bAuthKerb5 = false;
	}
	m_bAuthAPOP = GetIniShort(IDS_INI_AUTH_APOP)?true:false;
	m_bAuthRPA = GetIniShort(IDS_INI_AUTH_RPA)?true:false;
	
	if (m_bAuthKerb)
		m_AuthType = AUTH_KERBEROS;					// Kerberos 4
	else if (m_bAuthAPOP)
		m_AuthType = AUTH_APOP;						// APOP
	else if (m_bAuthRPA)
		m_AuthType =  AUTH_RPA;						// RPA
	else if (m_bAuthKerb5)
		m_AuthType =  AUTH_KERBEROS_5;				// Kerberos 5
	else
		m_AuthType =  AUTH_POP;						// regular POP

	
	m_nSpoolErrorAction = GetIniShort(IDS_INI_SPOOL_ERROR_ACTION);
	m_bAlert = GetIniShort(IDS_INI_ALERT)?true:false;
	m_bSound = GetIniShort(IDS_INI_SOUND)?true:false;
	m_bOpenInMailbox = GetIniShort(IDS_INI_OPEN_IN_MAILBOX)?true:false;
	
	if(m_AuthType == AUTH_KERBEROS)
	{
		m_strKerberosFakePassword = GetIniString(IDS_KERB_FAKE_PASSWORD);
		m_bKerbersoSetUserName = GetIniShort(IDS_INI_KERBEROS_SET_USERNAME)?true:false;
		m_strKerberosService = GetIniString(IDS_INI_KERB_SERVICE);
		m_strKerberosRealm	 = GetIniString(IDS_INI_KERB_REALM);
		m_strKerberosFormat = GetIniString(IDS_INI_KERB_FORMAT);
	}


	//Compute various flags used by POPSession::GetMail
	m_bFetching = (m_ulBitFlags & (kRetrieveNewBit|kRetrieveMarkedBit|kFetchHeadersBit))?true:false;
	m_bSending = ((m_ulBitFlags & kSendMailBit) || 
			((m_ulBitFlags & kSendIfSendOnCheckBit) && m_bSendOnCheck))?true:false;

	m_bDeleting = (m_ulBitFlags & (kLMOSBit|kDeleteMarkedBit|kDeleteRetrievedBit|kDeleteAllBit))?true:false;
	m_bNeedPOP = (m_bFetching || (m_bSending && m_bUsePOPSend) || m_bDeleting || (m_ulBitFlags & kValidateLoginOnly))?true:false;
    m_bFilterMail = GetIniShort(IDS_INI_FILTER_INCOMING_MAIL) != 0;
	m_nUpdateFreq = GetIniLong(IDS_INI_NEWMAIL_UPDATE_FREQUENCY);
	m_SSLSettings.GrabSettings();

	g_Personalities.SetCurrent( CachePersonality );
	
	return 1;
}



QCSMTPSettings::QCSMTPSettings(const char* persona)
		: m_szPersona(persona)
{	
	m_pNetSettings = DEBUG_NEW QCNetworkSettings(m_szPersona); 
}

QCSMTPSettings::~QCSMTPSettings()
{
	delete m_pNetSettings;
}
	
int QCSMTPSettings::GrabSettings()
{

	char buf[QCMS_BUF_SIZE];
	//
	// Must set persona to get the proper INI strings
	// for this personality.  Set it back when done.
	//
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_szPersona );

	m_szSMTPServer = GetIniString(IDS_INI_SMTP_SERVER, buf, sizeof(buf));
	m_szPOPAccount = GetIniString(IDS_INI_POP_ACCOUNT, buf, sizeof(buf));

	m_bUsePOPSend =  (GetIniShort(IDS_INI_USE_POP_SEND))? true : false;

	// Do not inherit the UseSubmissionPort setting from the Dominant personality
	g_Personalities.GetProfileString(m_szPersona, CPersonality::GetIniKeyName(IDS_INI_USE_SUBMISSION_PORT), 
										CPersonality::GetIniDefaultValue(IDS_INI_USE_SUBMISSION_PORT),
										buf, sizeof(buf));
	m_bUseSubmissionPort = atoi(buf)? true : false;
	m_bIgnoreQUITErrors	= (GetIniShort(IDS_INI_IGNORE_SMTP_QUIT_ERRORS))? true : false;
	
	m_nSmtpPort = (int) GetIniShort(IDS_INI_SMTP_PORT);
	m_nSmtpSubmissionPort = (int) GetIniShort(IDS_INI_SUBMISSION_PORT);

	// ESMTP AUTH settings
	m_bAuthAllowed	= (GetIniShort(IDS_INI_SMTP_AUTH_ALLOWED))? !m_bUsePOPSend : false;
	m_bAuthRequired = (GetIniShort(IDS_INI_SMTP_AUTH_REQUIRED))? !m_bUsePOPSend : false;
	m_bAuthFollowsK5INI = GetIniShort(IDS_INI_SMTP_AUTH_K5_INI) ? true : false;
	m_szAuthBanished = GetIniString(IDS_INI_SMTP_AUTH_BANISHED, buf, sizeof(buf));
	m_SSLSettings.GrabSettings();

	// Make a guess as to what the user's domain is
	
	// clear it
	m_szBestGuessAtDomain.Empty();
	
	// try the domain qualifier
	GetIniString( IDS_INI_DOMAIN_QUALIFIER, buf, sizeof( buf ) );
	const char *notAnAt = buf;
	while ( *notAnAt == '@' ) notAnAt++;
	
	// If we found a non-at character, use the domain qualifier
	if ( *notAnAt )
		m_szBestGuessAtDomain = notAnAt;
	else
	{
		// if not, try the return address
		const char* ra = GetReturnAddress();
		if (ra)
		{
			strcpy(buf, ra);
			StripAddress(buf);
			char *spot;
			
			// just use the hostname
			if ( spot = strrchr( buf, '@' ) )
				m_szBestGuessAtDomain = spot+1;
		}
	}
		
	//
	// Back to where it was.
	//
	g_Personalities.SetCurrent( CachePersonality );
	
	return 1;
}



