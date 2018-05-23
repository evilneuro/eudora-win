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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


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

	m_bDoCaching			= GetIniShort(IDS_INI_NETWORK_CACHING)	?true:false;
	m_bConnectOffline		= GetIniShort(IDS_INI_CONNECT_OFFLINE)	?true:false;
	m_bAuthenticateKerberos = GetIniShort(IDS_INI_AUTH_KERB)		?true:false;
	m_bDialupConnection		= GetIniShort(IDS_INI_AUTO_CONNECTION)	?true:false;

	m_nNetworkBufferSize	= GetIniLong(IDS_INI_NETWORK_BUFFER_SIZE);

	ASSERT(m_nNetworkBufferSize > 1024);

	m_nNetworkOpenTimeout	= GetIniLong(IDS_INI_NETWORK_OPEN_TIMEOUT);
	m_nNetworkTimeout		= GetIniLong(IDS_INI_NETWORK_TIMEOUT);
	m_bNetImmediateClose = GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE)?true:false;

	if(m_bDialupConnection)
	{
		m_strRASEntryName	= GetIniString(IDS_INI_AUTO_CONNECTION_NAME, buf, QCMS_BUF_SIZE);
		m_bRASSavePassword	= false;  //Get it from RAS auth dialog

		m_pRasDialParams = new RASDIALPARAMS;
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





QCPOPSettings::QCPOPSettings(const CString& persona, unsigned long ulBitFlags) 
		: m_szPersona(persona), m_ulBitFlags(ulBitFlags)
{	
	m_pNetSettings = new QCNetworkSettings(m_szPersona); 
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
	
	m_bAuthKerb = GetIniShort(IDS_INI_AUTH_KERB)?true:false;
	m_bAuthAPOP = GetIniShort(IDS_INI_AUTH_APOP)?true:false;
	m_bAuthRPA = GetIniShort(IDS_INI_AUTH_RPA)?true:false;
	
	
	if (m_bAuthKerb)
		m_AuthType = AUTH_KERBEROS;					// Kerberos
	else if (m_bAuthAPOP)
		m_AuthType = AUTH_APOP;						// APOP
	else if (m_bAuthRPA)	// RPA
		m_AuthType =  AUTH_RPA;
	else
		m_AuthType =  AUTH_POP;						// regular POP

	
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
	m_bNeedPOP = (m_bFetching || (m_bSending && m_bUsePOPSend) || m_bDeleting)?true:false;

	m_nUpdateFreq = GetIniLong(IDS_INI_NEWMAIL_UPDATE_FREQUENCY);
	
	g_Personalities.SetCurrent( CachePersonality );
	
	return 1;
}



QCSMTPSettings::QCSMTPSettings(const CString& persona) 
		: m_szPersona(persona)
{	
	m_pNetSettings = new QCNetworkSettings(m_szPersona); 
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

	m_szSMTPServer = GetIniString(IDS_INI_SMTP_SERVER,buf, sizeof(buf));
	m_szPOPAccount = GetIniString(IDS_INI_POP_ACCOUNT,buf, sizeof(buf));

	m_bUsePOPSend =  (GetIniShort(IDS_INI_USE_POP_SEND))?true:false;
	
	m_nSmtpPort = (int) GetIniShort(IDS_INI_SMTP_PORT);

	// ESMTP AUTH settings: (Added 12/21/98, JOK)
	//
	m_bAuthAllowed	= (GetIniShort(IDS_INI_SMTP_AUTH_ALLOWED))?true:false;
	m_bAuthRequired = (GetIniShort(IDS_INI_SMTP_AUTH_REQUIRED))?true:false;
		
	//
	// Back to where it was.
	//
	g_Personalities.SetCurrent( CachePersonality );
	
	return 1;
}



