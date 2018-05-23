// POPSession.cpp
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#include "afxmt.h"

#include <ctype.h>
#include <memory>		//	For auto_ptr

#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#endif

#include <dos.h>
#include <mmsystem.h>

#include <QCUtils.h>

#include "resource.h"

#include "eudora.h"
#include "guiutils.h"
#include "fileutil.h"
#include "address.h"

#include "rs.h"   //for CRString
#include "summary.h"
#include "password.h"
#include "lex822.h"
#include "QCWorkerSocket.h"
#include "compmsgd.h"  //for SendQueuedMessages
#include "persona.h"	//for g_Personalities used by PostProcessing
#include "MsgRecord.h"  //for CMsgRecord
#include "PopHost.h"
#include "HostList.h"
#include "QCWorkerThreadMT.h"
#include "QCNetSettings.h"
#include "QCPOPSettings.h"
#include "POPSession.h"

#include "md5.h"
#include "debug.h"
#include "utils.h"

#include "AuthRPA.h"
#include "SpoolMgr.h"
#include "kclient.h"  // include the Kerberos stuff
#include "Automation.h"  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include "QCRas.h"
#include "Header.h"
#include "FileLineReader.h"
#include "trnslate.h"

#include "sasl.h"
#include "POPGssapi.h"
#include "Base64.h"


//	Defines and other Constants	=============================
#include "DebugNewHelpers.h"

extern bool g_bDisableUIDL;

//	General errors returned by these routines
const	int	kNoErr					= 0;	// No error

void RecoverReceivedMail(void *pv, bool)
{
	char *szPersona = static_cast<char *>(pv);
	
	//Get the window with focus and save it
	HWND hFocus = ::GetFocus();
	

	CPOP MailMan(szPersona);
	MailMan.GetMailFromSpool();

	//If window is still around give the focus back
	if(hFocus)
		::SetFocus(hFocus);
}


void ProcessIncomingMail(void *pv, bool bEnteringLastTime)
{
	CPOPSession *popThread = (CPOPSession *)pv;
	
	CPOP MailMan(popThread->GetSettings(), popThread->GetNotifier());
	bool bDone = MailMan.GetMailFromSpool();

	if(!bDone) //stopped by user
	{
		CTaskInfoMT *pTaskInfo = popThread->GetTaskInfo();
		pTaskInfo->SetPostProcessing(true);
		pTaskInfo->IgnoreIdle(false);
	}
}


//
//	POPB64DecodeString
//		Decodes the specified string via the Base64 decoder.
//
//	Return:
//		Length of decoded string, 0 if the decode failed.
//
//	Note: See my comment below on POPB64EncodeString(). (dwiggins)
//
int POPB64DecodeString(char* s)
{
	Base64Decoder	d64;
	LONG			len;
	int				result;
	
	d64.Init();
	result = d64.Decode(s, strlen(s), s, len);

	if (result == 0)
		return len;
	else
		return 0;
}

//
//	POPB64EncodeString
//		Encodes the string "s", of length "sLen", into the given buffer "out".
//
//	Return:
//		The length of the encoded string as the value of the function.
//
//	Note: There are currently a number of functions floating around out there
//		that do essentially (or in some cases exactly) the same thing as this
//		function.  There are also two identical functions that do the same thing
//		as POPB64DecodeString().  At some point it might be nice to optimize
//		this into a single function, but for now I'll compound the problem by
//		creating yet another separate function. (dwiggins)
//
int POPB64EncodeString(char* s, int sLen, char *out, int outsize)
{
	Base64Encoder	TheEncoder;
	LONG			OutLen = 0;
	char*			OutSpot = out;

	memset(out, 0, outsize);

	TheEncoder.Init(out, OutLen, 0);
	OutSpot += OutLen;
	TheEncoder.Encode(s, sLen, OutSpot, OutLen);
	OutSpot += OutLen;
	TheEncoder.Done(OutSpot, OutLen);
	OutLen += OutSpot - out;

	out[OutLen] = 0;

	return OutLen;
}


// Callback stubs for SASL. NOTE: This should use the template-based callback
// mechanism in callbacks.h.

//
//	pop_challenge
//		Gets the next challenge from the POP server.
//
//	Parameters:
//		s[in]: Stream object, in this case the CPOPSession.
//		szOutbuf[out]: Buffer to hold challenge data.
//		iBufSize[in]: Size of outbuf.
//
//	Return:
//		Length of the challenge data.
//
int pop_challenge(void *s, char *szOutbuf, int iBufSize)
{
	if (!s)
	{
		return NULL;
	}

	CPOPSession		*pPOP = (CPOPSession*)s;
	return pPOP->SaslChallenge(szOutbuf, iBufSize);
}

//
//	pop_response
//		Sends the response to the POP server.
//
//	Parameters:
//		s[in]: Stream object, in this case the CPOPSession.
//		szResp[in]: Buffer containing response to send.
//		lSize[in]: Length of response.
//
//	Return:
//		TRUE if response was sent, FALSE otherwise.
//
BOOL pop_response(void *s, char *szResp, unsigned long lSize)
{
	if (!s)
	{
		return FALSE;
	}

	CPOPSession		*pPOP = (CPOPSession*)s;
	return pPOP->SaslResponse(szResp, lSize);
}

void QCMailNotifier::Increment()
{
	InterlockedIncrement(&m_nRefCount);
}


void QCMailNotifier::Decrement(int NoteFlags /*=0*/)
{
	//update the Notification flags OR'em	
	if(NoteFlags)
		AddNotifications(NoteFlags);

	//decrement and we are the last one, notify the results
	if(InterlockedDecrement(&m_nRefCount) == 0)
	{
		//if flags is 0, our tasks never got a chance to dowork
		if(m_Notifications)
			NotifyMailCheckResults(m_Notifications);
		//we are done, cleanup
		delete this;
	}
}
	

		

CPOPRecoverer::CPOPRecoverer(const char* strPersona)
{
	m_Persona = strPersona;
	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	m_pTaskInfo->SetPersona(m_Persona);
	m_pTaskInfo->SetTitle(CRString(IDS_POP_RECOVER_TITLE));
	m_pTaskInfo->IgnoreIdle();
}


HRESULT CPOPRecoverer::DoWork()
{
	//Set the post proceesing fn, this will be called in OnIdle loop or OnDemand from TaskManager
	m_pTaskInfo->SetPostProcFn(CRString(IDS_POP_POST_FILTER_RECOVERED_MAIL), RecoverReceivedMail,
	                           (void*)(LPCTSTR)m_Persona);
	m_pTaskInfo->SetPostProcessing(true);  //do the post processing after here
	
	return S_OK;
}




CCriticalSection CPOPSession::m_RPAAuthGuard;
CCriticalSection CPOPSession::m_KerberosAuthGuard;

//////////////////////////////////////////////////////////////////////////////
// CPOPSession

CPOPSession::CPOPSession(QCPOPSettings *settings, QCPOPNotifier *notifier) : 
	m_Settings(settings),
	m_Notifier(notifier),
	m_bCreatedNetConnection(FALSE),
	m_bPOPConnected(FALSE),
	m_nMessageCount(0),
	m_bGotNewMail(-1),// -1 means never started to collect mail
	m_bCleanup(false),
	m_bQuitIssuedSuccessfully(false),
	//m_Notifications(0),
	m_pNetConnection(NULL),
	m_pPopHost(NULL),
	m_bTaskStarted(false),
	m_bCanMangle(false),
	m_bSupportsAuthRespCode(false),
	m_bSupportsSSL(false),
	m_pSasl(NULL)
{
	m_ulBitFlags = m_Settings->GetBitFlags();
	m_strPersona = m_Settings->GetPersona();
	QCNetworkSettings *pNetSettings = m_Settings->GetNetSettings();

	//Set the error callback function for Network to report error back to us
	pNetSettings->SetReportErrorCallback(  
		makeCallback( (QCNetworkSettings::ErrorCallback *)0, *m_pTaskInfo, &CTaskInfoMT::CreateError) );

	pNetSettings->SetProgressCallback(  
		makeCallback( (Callback1<const char *> *)0, *m_pTaskInfo, &CTaskInfoMT::SetMainText) );

	pNetSettings->SetTaskInfo(m_pTaskInfo);

	m_pTaskInfo->SetTitle(CRString(IDS_POP_LOOKING_FOR_MAIL));
	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	m_pTaskInfo->SetPersona(m_Settings->GetPersona());
	
	if(pNetSettings->IsDialupConnection())
	{
		m_pTaskInfo->SetDialupEntryName( pNetSettings->GetRASEntryName() );
		m_pTaskInfo->SetScheduleTypes(TT_USES_DIALUP);
	}

	if (m_Notifier)
	{
		m_Notifier->Increment();
	}

	m_pTaskInfo->SetScheduleTypes(TT_USES_POP);
	m_pTaskInfo->SetPostState(TSS_POST_NONE);

	m_strGSSLibName = GetIniString(IDS_INI_GSS_DLLNAME);

	// Get personality specific settings from correct personality
	CString strHomie = g_Personalities.GetCurrent();
	g_Personalities.SetCurrent(m_strPersona);

	m_lastAuthTime = GetIniLong(IDS_INI_POP_LAST_AUTH);

	g_Personalities.SetCurrent(strHomie);

	m_GMTOffset = GetGMTOffset();

	m_nLMOSBackupCount = GetIniLong(IDS_INI_LMOS_BACKUPS);
}

CPOPSession::~CPOPSession()
{
	// Make sure settings changes affect correct personality
	CString strHomie = g_Personalities.GetCurrent();
	g_Personalities.SetCurrent(m_strPersona);
	
	// Clear the password if invalid password
	if (NeedsCleanup())
		::ClearPassword();

	// Save our last auth time
	SetIniLong(IDS_INI_POP_LAST_AUTH,LastAuthTime());

	g_Personalities.SetCurrent(strHomie);
	
	//if pophost is NULL, ie we never really tried to get the mail..
	if(m_bTaskStarted)
	{
		//we successfully issued QUIT means safely remove the messages marked as DELETE_SENT
		if(IsQuitSuccessful())
			RemoveDeletedMsgRecords();

		if(m_Notifier)
		{
			CPOP MailMan(GetSettings(), GetNotifier());
			MailMan.DoNotifyNewMail();
		}
	}

	//The notifier is decremented; if we are the last one, play a sound etc if new mail
	Notify();

	//We didnot create settings object, but we are responsible for deleteing it
	if(m_Settings)
		delete m_Settings;

	if (m_pSasl)
	{
		delete m_pSasl;
		m_pSasl = NULL;
	}
}


void CPOPSession::Notify()
{
	if(m_Notifier)
		m_Notifier->Decrement();
}


void CPOPSession::AuthError(const char * err)
{
	m_pTaskInfo->CreateError(err, TERR_POPAUTH);
}


void CPOPSession::ErrorDialog(UINT StringID, ...)
{
	char buf[1024] = {0};
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	m_pTaskInfo->CreateError(buf, TERR_POP);
}


void CPOPSession::ErrorDialog(UINT StringID, va_list argList)
{
	char buf[1024] = {0};
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list argSave = argList;
	
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), argList);
	
	va_end(argSave);

	m_pTaskInfo->CreateError(buf, TERR_POP);
}


	
void CPOPSession::RequestThreadStop()
{  
	//if Already issued STOP
	if(m_bStopThread)
		return; 

	CSingleLock lock(&m_Guard, TRUE);

	if(m_pNetConnection)
		m_pNetConnection->RequestStop(); 	
	
	m_bStopThread = true; 
}


POPVALUES *CPOPSession::GetSettings()
{
	POPVALUES *pv = DEBUG_NEW POPVALUES;
	pv->bGotNewMail = m_bGotNewMail;
	//pv->noteFlags = m_Notifications;
	pv->settings = m_Settings;
	//pv->bOpenInMailBox = m_Settings->OpenInMailbox();
	//pv->persona = m_strPersona;
	//pv->ulBitFlags = m_ulBitFlags;
	return pv;
}


CString CPOPSession::GetGSSLibraryName()
{ 
	if (!m_strGSSLibName.IsEmpty())
	{
		return m_strGSSLibName;
	}
	return CRString(IDS_GSSAPI_LIBNAME);
}


//When the POP thread is queued, Task manager calls this function and asks it to do its stuff
HRESULT CPOPSession::DoWork()
{ 
	m_bTaskStarted = true;

	//Set the post proceesing fn, this will be called in OnIdle loop or OnDemand from TaskManager
	m_pTaskInfo->SetPostProcFn(CRString(IDS_POP_POST_FILTER_NEW_MESSAGES), ProcessIncomingMail,
	                           this);
	

	if ( IsStoppingThread())
	{
		m_pTaskInfo->SetMainText(CRString(IDS_TASK_STOPPED));
		m_pTaskInfo->SetPostProcessing(true);  //do the post processing after here
		return S_OK;
	}
	
	
	GetMail();

	
	if( IsStoppingThread())
	{
		//We want to write LMOS to save the work thats done until now
		if( m_pPopHost )
			m_pPopHost->WriteLMOSData();
	}

	//If no new mail, ignore idle to make it a painless exit :-)
	if(m_bGotNewMail != TRUE)
		m_pTaskInfo->IgnoreIdle();

	m_pTaskInfo->SetPostProcessing(true);  //do the post processing after here
	
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// GetMail [public]
//
// Top level routine that actually gets the mail by connecting to the
// server and doing all of the dirty work.
////////////////////////////////////////////////////////////////////////
void CPOPSession::GetMail()
{
	m_pPopHost = CHostList::GetHost(m_nLMOSBackupCount, m_Settings->GetPersona(), m_Settings->GetPOPAccount());
	if (!m_pPopHost)
	{
		ErrorDialog(IDS_ERR_LMOS_LOAD);
		return;
	}
	
//	BOOL bSendMailAfter = m_strPassword.IsEmpty();

	//CTocDoc* pInTocDoc = ::GetInToc();
	//CTocView* pInTocView = (pInTocDoc? pInTocDoc->GetView() : NULL);


	BOOL bResult = TRUE;
	
	//
	// At this point, we don't have a network connection yet,
	// so based on the current authentication type, get the user's
	// POP account password.  For Kerberos, we have to wait until
	// we have a network connection.  Otherwise, we either
	// fetch the saved password from the INI file or prompt the
	// user for a password with a dialog box.
	//
	//stateNext = STATE_SETUP_NET;
	
	/*
	if (bNeedPOP)
		bResult = DoGetPassword_();

	if(!bResult) goto DONE;
	*/


	//
	// Make sure the global 'NetConnection' object is ready
	// to rock and roll, creating a new connection if necessary.
	//
	//stateNext = STATE_SENDMAIL_BEFORE;
	bResult = SetupNetConnection_();
	if(!bResult) goto DONE;


	//stateNext = STATE_OPEN_POP_CONNECTION;
	//stateFailed = STATE_TEARDOWN_NET;
	
	//@@@@@@@@@@@@@@@@@@@@@@
	//if (m_Settings->Sending() && !bSendMailAfter && !m_Settings->UsePOPSend())
	//	bResult = (SendQueuedMessages(QS_READY_TO_BE_SENT, FALSE) != SQR_MAJOR_ERROR);

	//if(!bResult) goto TEARDOWN;



	//case STATE_OPEN_POP_CONNECTION:
	//
	// If necessary, open a connection to the POP server,
	// using the user's POP server name, service name, and
	// port ID (the service name and port ID settings are
	// different if we're using Kerberos).  If there is an
	// existing open connection (as indicated by
	// 'm_bPOPConnected'), just use it without creating a new
	// connection.  This is where we put up the progress
	// dialog.
	//
	if (m_Settings->NeedPOP())
		bResult = OpenPOPConnection_();

	if(!bResult) goto TEARDOWN;

	
	//case STATE_LOGON:
	//
	// DoLogon_() calls POPAuthenticate() to do the following:
	// 1) authenticate to the POP server (could be vanilla
	// POP, APOP, or Kerberos), and 2) determine the number of
	// available messages (stored in 'm_nMessageCount').
	//

		// What can this POP server do?

	if(m_Settings->m_SSLSettings.GetSSLReceiveUsage() == QCSSLSettings::SSLUseAlternatePort)
	{
		m_pNetConnection->SetSSLMode(true,m_strPersona, &m_Settings->m_SSLSettings , "POP");
	}

	if (m_Settings->NeedPOP())
		bResult = DoLogon_();
	else //if DoLogon is called then GetCapabilities will also be called
		{
		GetCapabilities();
		if(m_bSupportsSSL && m_Settings->m_SSLSettings.m_SSLReceiveAvailableOrAlways)
			{
			SSLStart();
			}
		}

	if(!bResult) goto CLOSE_POP;

	





	//case STATE_SENDPOPMAIL:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ need to come back later
	//Dont use POP send for now
	//if (bSending && m_Settings->UsePOPSend() )
	//	bResult = (::SendQueuedMessages(QS_READY_TO_BE_SENT, FALSE) != SQR_MAJOR_ERROR);
	
	//if(!bResult) goto CLOSE_POP;
	

	//case STATE_RECONCILE_UIDL_INFO:
	//
	// Grab the LMOS database from disk, fetch the UIDL data
	// from the server, then reconcile the lists into a new
	// LMOS database.  Write the new LMOS info to disk.
	//
	if (m_Settings->Fetching() || m_Settings->Deleting())
	{
		bResult = DoReconcileUIDLInfo_();
		if(!bResult) goto CLOSE_POP;

		bResult = (m_pPopHost)?TRUE:FALSE;
	}
	
	if(!bResult) goto CLOSE_POP;
	
	
	//case STATE_GETPOPMAIL:
	//
	// Download the messages, writing directly to the global
	// In box, creating new message summaries, and bursting
	// attachments.
	//
	//stateNext = STATE_DELETEPOPMAIL;
	//stateFailed = STATE_CLOSE_POP_CONNECTION;
	
	if (m_Settings->Fetching()){
		//bResult = DoGetPOPMail_(pPOPHost->CanUIDL()); //This is now handled in DoReconcileUIDLInfo_()
		//bResult = DoGetPOPMail_();
		
		bResult = DoGetPOPMailToSpool();
		//if(m_bGotNewMail)
		//	bResult = DoGetPOPMailFromSpool();
	}
	
	if(!bResult) goto CLOSE_POP;
	
	
	
	//case STATE_DELETEPOPMAIL:
	//
	// If we're leaving messages on the server, then do the
	// message purging stuff.  This includes auto-deletion of
	// expired/trashed messages, forcible removal of ALL
	// messages, and removal of messages explicitly marked for
	// deletion.  Saves the updated LMOS state to disk.
	//
	if (m_Settings->Deleting())
		bResult = PurgeMail_();
	
	
	
	
	//case STATE_CLOSE_POP_CONNECTION:
	//
	// Send the POP server the QUIT command and close the
	// network connection.  If the QUIT command was
	// successful, we assume the messages were really deleted
	// from the server, so we remove the deleted messages from
	// our LMOS database, too.
	//
	
CLOSE_POP:
	
	if (m_Settings->NeedPOP())
		bResult = ClosePOPConnection_();
	

	//case STATE_SENDMAIL_AFTER:
	//@@@@@@@@@@@@@@@@@@@@@@
	//if (m_Settings->Sending() && bSendMailAfter)
	//	bResult = (::SendQueuedMessages(QS_READY_TO_BE_SENT, FALSE) != SQR_MAJOR_ERROR);
	
TEARDOWN:

	//
	// If we created the global 'NetConnection' object, then
	// we are responsible for destroying it.  Otherwise, just
	// close down the connection that somebody else created.
	//
	bResult = TeardownNetConnection_();
			
DONE:
	//if we are here, the thread must have completed successfully
	if(m_bGotNewMail)
		//m_pTaskInfo->SetMainText("Completed. Waiting for new messages to display.");
		m_pTaskInfo->SetMainText(CRString(IDS_POP_POST_PROCESS_NEW_MSGS));
	else
		//m_pTaskInfo->SetMainText("Completed. No new messages.");
		m_pTaskInfo->SetMainText(CRString(IDS_POP_POST_PROCESS_NO_MSGS));
}


////////////////////////////////////////////////////////////////////////
// GetCapabilities
//
// Send the "CAPA" command to the POP server to see what kind of extra
// things this POP server can do.
////////////////////////////////////////////////////////////////////////
void CPOPSession::GetCapabilities()
{
//	if (m_Settings->UsePOPCAPA() == false)
	//	return;

	// Right now the only capability we use is Mangle, so if we're not
	// doing Mangle then we shouldn't send the CAPA command at all.

//	if (m_Settings->UsePOPMangle() == false)
//		return;

	//Now we use this for SSL stuff also
	//eventually we will add an if statement here to call the CAPA command 
	//only if mangle is required or SSL mode is desired.

	char ResponseBuffer[QC_NETWORK_BUF_SIZE];
	if (m_Settings->UsePOPCAPA() || m_Settings->UsePOPMangle() ||(m_Settings->m_SSLSettings.m_SSLReceiveAvailableOrAlways ))
		{
		if (POPCmdGetReply(IDS_POP_CAPA, NULL, ResponseBuffer, sizeof(ResponseBuffer), FALSE) < 0)
			return;
		else
			{
			char szBuffer[QC_NETWORK_BUF_SIZE];
			while (ReadPOPLine(szBuffer, sizeof(szBuffer)) > 0)
				{
				if (strnicmp(szBuffer, "MANGLE", 5) == 0 ||
					strnicmp(szBuffer, "X-MANGLE", 7) == 0)
					{
					m_bCanMangle = true;
					}
				else if (strnicmp(szBuffer,"STLS", 4)== 0)
					{
					m_bSupportsSSL = true;
					}
				else if (strnicmp(szBuffer,"AUTH-RESP-CODE", 14)== 0)
					{
					m_bSupportsAuthRespCode = true;
					}
				else if (strnicmp(szBuffer,"SASL", 4)== 0)
					{
						if (!m_pSasl)
						{
							m_pSasl = DEBUG_NEW_NOTHROW CSasl();
						}
						if (m_pSasl)
						{
							m_pSasl->ParseAuthenticators(szBuffer + 4 + 1);
						}
					}
				}
			}
		}
}

//
//	CPOPSession::ForgetCapabilities()
//
//	Resets the capability list previously obtained from the POP server.
//	It is important to forget and rebuild the capability list after successful
//	SSL negotiation to protect against man-in-the-middle attacks which alter
//	the capabilities list prior to SSL negotiation.
//	
void CPOPSession::ForgetCapabilities()
{
	m_bCanMangle = false;
	m_bSupportsAuthRespCode = false;
	// Don't forget SSL info: once STLS has succeeded the server may no longer
	// list STLS as a capability and we need to remember that if we get this
	// far not only is it offered but it has succeeded. -dwiggins
	// m_bSupportsSSL = false;
	if (m_pSasl)
	{
		delete m_pSasl;
		m_pSasl = NULL;
	}
}

////////////////////////////////////////////////////////////////////////
// DoReconcileUIDLInfo_ [private]
//
// GetHostForCurrentPersona() grabs the LMOS info for the current
// persona and GetHostFromUIDLServer_() grabs the UIDL info, if
// supported, from the server.  The reconciliation process compares
// the previous LMOS data with the messages currently on the server --
// deleted messages are pruned away and new messages are added, then
// we preserve the LMOS info (e.g. original download date) for messages 
// that are still on the server.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::DoReconcileUIDLInfo_()
{
	m_pTaskInfo->SetMainText(CRString(IDS_POP_LOOKING_FOR_MAIL));
	
	// Possibly handle UIDL capable server.
	// Non-UIDL servers will set pServerHost to NULL.
	CPopHost* pServerHost = NULL;
	if (!GetHostFromUIDLServer_(pServerHost))
		return FALSE;
	ReconcileLists_(m_pPopHost, pServerHost);
	
	CPopHost* pDiskHost = m_pPopHost;
	if(!pDiskHost) 
		return FALSE;
	
	if(!m_Settings->Fetching())
		return TRUE;
	
	//
	// If the host can UIDL then we need to find the first unread
	// message, and then condition the host list to reflect this
	// message as the first to be collected (unless a skipped message
	// is marked). In a NON-UIDL case, we will use the first unread to
	// begin reading from, creating message records along the way.
	//
	int nFirstUnread = FindFirstUnread_();
	if (nFirstUnread < 0)
		return FALSE;
	

	//if user hits cancel, we are out of here..
	if(IsStoppingThread())
		return FALSE;
	
	//Handle Non-UIDL server
	//if (pPopHost != NULL && !bCanUIDL)
	if (!pDiskHost->CanUIDL())
	{
		//
		// Uh, oh.  This host is NOT capable of doing UIDL...
		//
		// Check our last message against the server's.  If they
		// match, continue topping only those needed.  If they don't
		// match, then force a whole list build with TOP commands.  We
		// may not care about matching up the messages, such as when
		// we're skipping all the messages or we're not leaving mail
		// on the server and we're asking the server to tell us where
		// to start.
		//
		//if ((nFirstUnread > m_nMessageCount) || 
		//	(!::GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) && GetFetchType_() != FETCH_NORMAL) ||
		//	MatchLastMessage_(pPopHost->GetCount()))
		
		if ((nFirstUnread > m_nMessageCount) || 
			(!m_Settings->DoLMOS() && m_Settings->GetFetchType() != FETCH_NORMAL) ||
			MatchLastMessage_(pDiskHost->GetCount()))
		{
			// 
			// Add empty objects for all new items.
			//
			pDiskHost->PadMsgList(m_nMessageCount);
		}
		else
		{
			//
			// Ouch.  TOP all the messages on the server and replace the host.
			//
			pServerHost = BuildNonUIDLMsgs_(1, m_nMessageCount);
			ReconcileLists_(pDiskHost, pServerHost);
			//pDiskHost = pServerHost; 
		}
	}
	
	
	if (pDiskHost != NULL)
	{
		//
		// Condition the message records for collection
		//
		pDiskHost->ConditionMessagesPriorTo(nFirstUnread, !m_Settings->DoLMOS());
	}
	
	//Moved from top; write the reconciled list whether its from UIDL server or from Non-UIDL server
	pDiskHost->WriteLMOSData();

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// DoGetPOPMail_ [private]
//
// Overloaded method to do the actual message fetches.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::DoGetPOPMailToSpool()
{
		
    // This needs to be conditioned...just in case we exit next...!!!
	m_bGotNewMail = FALSE;
	
	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
	{
		char szBuf[256];
		sprintf(szBuf, "Begin fetching messages for %s: %s", (LPCTSTR)m_strPersona, m_Settings->GetPOPAccount());
		PutDebugLog(DEBUG_MASK_LMOS, szBuf);
	}

	//Build a map of fetchable message numbers and their hash values
	//We would like to walkthrough our LMOS by ascending order of the message numbers

	std::map<int, CMsgRecord *> tMap;
	typedef std::map<int, CMsgRecord *>::iterator MI;
	CMsgRecord *pMsgRecord = m_pPopHost->GetNextFetchableMsgNum(m_ulBitFlags, TRUE);
	
	int nTotalMessages = 0;
	int nMsgNum = 0;
	while(pMsgRecord)
	{
		nMsgNum = pMsgRecord->GetMsgNum();
		//TRACE("Msg %d hash %ld pMsgRecord %p\n", nMsgNum, pMsgRecord->GetHashValue(), pMsgRecord);
		tMap[nMsgNum] = pMsgRecord;
		pMsgRecord = m_pPopHost->GetNextFetchableMsgNum(m_ulBitFlags);
	}
	if ( (nTotalMessages=tMap.size()) == 0)
		return TRUE;

	m_bGotNewMail = FALSE;
	CString strMainText;
	
	long lEstimatesMsgSize = 0;

	strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nTotalMessages);
	m_pTaskInfo->SetMainText(strMainText);

	m_pTaskInfo->SetTotal(nTotalMessages*100);

	// Now while the nMsgNumber is not zero we get messages...
    BOOL bReturn = TRUE;
	
	unsigned int nMsgsDownloaded = 0;
	for(MI mi = tMap.begin(); mi != tMap.end(); ++mi)
	{
		lEstimatesMsgSize = 0;
		pMsgRecord = (*mi).second;
		nMsgNum = (*mi).first;

		ASSERT( nMsgNum == pMsgRecord->GetMsgNum());
		//TRACE("Fetching Msg # %d Hash %ld\n", nMsgNum, pMsgRecord->GetHashValue());

		int nStatus = POPGetMessageToSpool(pMsgRecord);
		if (nStatus >= 0)
		{
			// Set the Retrieval flag to don't retrieve (Nget)
			pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE/*1*/);
		
			// That's it if we didn't download anything
			if (nStatus == 0)
			{
				ASSERT(0);
				continue;
			}
			
			// Set the has been read flag...
			pMsgRecord->SetReadFlag(LMOS_HASBEEN_READ/*0*/);

			// We need to condition the mail flag for later saves....
			m_bGotNewMail = TRUE;

			int nFreq = m_Settings->GetUpdateFrequency();
			if( nFreq == 25 && (m_Settings->GetNetSettings()->IsDialupConnection()))
				nFreq = 5;
				
			nMsgsDownloaded++;
			if(nMsgsDownloaded % nFreq == 0)
				m_pTaskInfo->SetPostProcessing(true);  //Post process this message
		}
		else
		{
			//
			// If we got here the call to get the message from the pop
			// failed, so bust out of here.
			//
			bReturn = FALSE;
			break;
		}

		strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nTotalMessages--);
		m_pTaskInfo->SetMainText(strMainText);
	}

	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
	{
		char szBuf[256];
		sprintf(szBuf, "Done fetching messages for %s: %s", (LPCTSTR)m_strPersona, m_Settings->GetPOPAccount());
		PutDebugLog(DEBUG_MASK_LMOS, szBuf);
	}

	//write out any LMOS flags that got changed.
	if(m_bGotNewMail)
		m_pPopHost->WriteLMOSData();

	return bReturn;
}


int CPOPSession::POPGetMessageToSpool(CMsgRecord *pMsgRecord)
{
	int nMessageNum = pMsgRecord->GetMsgNum();
	long lEstimatedMsgSize = 0, lsaveEstimated = 0;
	char szMessageNum[10];		// Message number
	wsprintf(szMessageNum, "%d", nMessageNum);
	bool bEntireMessage = FALSE;// are we going to read the entire message
	int reDownloadStatus = kNoReDownload;		// Non-zero if we need to redownload - bit flags will indicate why

	//
	// Send the POP server a LIST command, then expect the following reply:
	//
	//     +OK <mess #> <mess size>
	//
	// All we want is the message size, which is stuffed into the global 
	// 'g_lEstimatedMsgSize'
	//
	char szBuffer[QC_NETWORK_BUF_SIZE];
	if (this->POPCmdGetReply(IDS_POP_LIST, szMessageNum, szBuffer, sizeof(szBuffer)) < 0)
		return -1;

	// Grab the message size
	{
		char szDummy[40];		// unused string
		int nDummy = 0;			// unused int
		sscanf(szBuffer, "%s %d %ld", szDummy, &nDummy, &lEstimatedMsgSize);
	}

	lsaveEstimated = lEstimatedMsgSize;



msgdownload:


	lEstimatedMsgSize = lsaveEstimated;


	//Set the estimated size for this message
	pMsgRecord->SetEstimatedSize(lEstimatedMsgSize);
    
	//
	// At this point we either read all, or top or nothing.  Have to
	// call GetMsgFetchType() here since it depends on 'g_lEstimatedMsgSize' being set.
	//



	int nLinesToRead = 20;
	int nStatus = 0;
	MsgFetchType fetchType = MSGFETCH_ENTIRE;
    if ( !(reDownloadStatus & kReDownloadForPluginFlag) )
		fetchType = GetMsgFetchType(pMsgRecord);
	bEntireMessage = FALSE;
	
	switch (fetchType)
	{
	case MSGFETCH_ENTIRE:
		nStatus = this->POPCmdGetReply(IDS_POP_RETRIEVE, szMessageNum, szBuffer, sizeof(szBuffer));
		if(nStatus >= 0)
		{
			//setting this flag make sures that the message is not on the server anymore
			//if previously its in skipped state
			pMsgRecord->SetSkippedFlag(LMOS_DONOT_SKIP);
			bEntireMessage = TRUE;
		}
		break;

	case MSGFETCH_HEADER:
		nLinesToRead = 1;
		// Fall through...

	case MSGFETCH_STUB:
		{
			//
			// Send the POP server a TOP...
			//
			lEstimatedMsgSize = nLinesToRead * 80; // Estimate of size
			
			char ArgBuffer[128];
			if (m_Settings->UsePOPMangle() && m_bCanMangle)
				sprintf(ArgBuffer, "%s %d MANGLE", szMessageNum, (nLinesToRead == 1)? 1: 100);
			else
				sprintf(ArgBuffer, "%s %d", szMessageNum, nLinesToRead);
			nStatus = this->POPCmdGetReply(IDS_POP_TOP, ArgBuffer, szBuffer, sizeof(szBuffer));
			if (nStatus >= 0)
			{
				//
				// Reset the estimated size and make sure we don't try to 
				// process any message data ...
				//
				
				pMsgRecord->SetSkippedFlag(LMOS_SKIP_MESSAGE);
			}
		}
		break;

	case MSGFETCH_NOTHING:
		return 0;

	default:
		ASSERT(0);
		return 0;
	}

	if (nStatus >= 0)
	{
		//
		// If we get this far, we have a message pending at the server.
		// FetchMessageText() handles the details of downloading the
		// header, downloading the message body, decoding attachments, and
		// creating a new Summary record for the Inbox.
		//
		g_lBadCoding = 0;
		BOOL bDeleteMessage = FALSE;
		//if ((nStatus = FetchMessageText_(pInToc, nMessageNum)) < 0)
		//	return -1;

		bool		bKeepSpoolFileOpen = ( (reDownloadStatus & kReDownloadKeepingSpoolFileOpenFlag) != 0 );
		reDownloadStatus = kNoReDownload;
		if ((nStatus = FetchMessageToSpool(pMsgRecord, lEstimatedMsgSize, bEntireMessage, bKeepSpoolFileOpen, reDownloadStatus)) < 0)
		{
			if(reDownloadStatus != kNoReDownload)
				goto msgdownload;
			return -1;
		}


		if (m_ulBitFlags & kDeleteAllBit)
			bDeleteMessage = TRUE;
		else if (m_ulBitFlags & kLMOSBit)
			bDeleteMessage = !m_Settings->DoLMOS(); //!GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER);
		else
			bDeleteMessage = FALSE;
		
		// 
		// If we're not doing LMOS, we have a message to be deleted
		// from the server with the DELE command.
		//
		if (bDeleteMessage && pMsgRecord->GetSkippedFlag() == LMOS_DONOT_SKIP)
		{
			//
			// Send the DELE command for the message we just finished
			// downloading and mark the LMOS record with the "delete sent"
			// flag (delS).
			//
			wsprintf(szMessageNum, "%d", nMessageNum);
			if( POPCmdGetReply(IDS_POP_DELETE, szMessageNum, szBuffer, sizeof(szBuffer)) >= 0)
			{
				//TRACE("Delete sent for msg %d\n", nMessageNum);
				pMsgRecord->SetDeleteFlag(LMOS_DELETE_SENT/*-1*/);
			}
			else
			{
				TRACE("Failed to send DELETE for msg %d\n", nMessageNum);
				ASSERT(0);
			}
		}
	}

	return nStatus;
}




////////////////////////////////////////////////////////////////////////
// FetchMessageText_ [private]
//
// Processes the message text as a result of issuing the TOP or RETR
// command.  Calls WriteMessageToMBX() to do most of the work of
// downloading the header and message body to IN.MBX (bursting
// attachments as necessary).  Once the download is complete, sets
// the MesgID for the LMOS record, then creates and writes a new Summary
// object to IN.TOC.
//
//   1 on SUCCESS  -1 Failure
//
//
////////////////////////////////////////////////////////////////////////
int CPOPSession::FetchMessageToSpool(CMsgRecord *pMsgRecord, long lEstimatedSize, bool bMessageEntire, bool bKeepSpoolFileOpen, int& reDownloadStatus)
{
	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(m_strPersona);
	CString strFileName;
	
	if (bKeepSpoolFileOpen)
	{
		//	We're keeping the spool file open, so tell BeginWrite to use the final file
		//	name immediately and pass in the hash value to format the final file name
		//	(retrieved before this call even in non-UIDL case because we've already
		//	been through FetchMessageToSpool once before for this message).
		SpoolMgr->m_In->BeginWrite( strFileName, true, pMsgRecord->GetHashValue() );
	}
	else
	{
		SpoolMgr->m_In->BeginWrite(strFileName);
	}

	HRESULT					hrFile;
	char					szBuffer[QC_NETWORK_BUF_SIZE];
	auto_ptr<JJFileMT>		pSpoolFile(new JJFileMT);

	//	Check for memory allocation failure
	if (pSpoolFile.get() == NULL)
	{
		return -1;
	}
	
	//	Open the temp file, checking for any error
	hrFile = pSpoolFile->Open(strFileName, O_RDWR | O_CREAT | O_TRUNC);
	if ( FAILED(hrFile) )
	{
		//	Open failed - end the spool mananger write (in every other failure case below
		SpoolMgr->m_In->EndWrite();
		
		//	Notify the user in the task error that we had a problem with a temp file
		ErrorDialog(IDS_ERR_POP_TEMP_FILE);
		
		//	Write entry into log describing the details of what went wrong
		wsprintf( szBuffer, CRString(IDS_DEBUG_POP_OPEN_TEMP_FAILED), strFileName, pSpoolFile->GetLastError_() );
		::PutDebugLog(DEBUG_MASK_RCVD, szBuffer);

		return -1;
	}
	
	int nStatus = 1;

	long nSoFar = m_pTaskInfo->GetSoFar();
	long nDoneSoFar = 0;

	bool bEndOfHeaders = false;
	char From[32];
	char Date[32];
	char Subject[64];
	CRString RS_PluginInfo(IDS_PLUGIN_INFO);
	bool bCallTranslators = FALSE;

	*From = *Date = *Subject = 0;
	
	while ( (nStatus = this->ReadPOPLine(szBuffer, sizeof(szBuffer))) > 0 )
	{
		// keep looping...
		int Len = strlen(szBuffer);

		//	Write out line into temp file checking for an error. If we get an error - bail.
		hrFile = pSpoolFile->Put(szBuffer, Len);
		if ( FAILED(hrFile) )
			break;

		if (!bEndOfHeaders)
		{
			if (*szBuffer == '\r')
			{
				bEndOfHeaders = true;
				if (*From || *Date || *Subject)
				{
					char Summary[sizeof(From) + sizeof(Date) + sizeof(Subject) + 3];
					if (*From)
						strcpy(Summary, From);
					else
						*Summary = 0;
					if (*Date)
					{
						if (*Summary)
							strcat(Summary, ",");
						strcat(Summary, Date);
					}
					if (*Subject)
					{
						if (*Summary)
							strcat(Summary, ",");
						strcat(Summary, Subject);
					}
					m_pTaskInfo->Progress(Summary);
				}
			}
			else
			{
				while (Len >= 0 && (szBuffer[Len - 1] == '\r' || szBuffer[Len - 1] == '\n'))
					szBuffer[--Len] = 0;

				if (strnicmp(szBuffer, "From:", 5) == 0)
				{
					strncpy(From, GetRealName(szBuffer + 5), sizeof(From) - 1);
					From[sizeof(From) - 1] = 0;
					TrimWhitespaceMT(From);
				}
				else if (strnicmp(szBuffer, "Date:", 5) == 0)
				{
					strncpy(Date, szBuffer + 5, sizeof(Date) - 1);
					Date[sizeof(Date) - 1] = 0;
				}
				else if (strnicmp(szBuffer, "Subject:", 8) == 0)
				{
					strncpy(Subject, szBuffer + 8, sizeof(Subject) - 1);
					Subject[sizeof(Subject) - 1] = 0;
				}
				else if (!bMessageEntire && strstr(szBuffer, RS_PluginInfo) !=NULL)
					//dont bother making the comparison if we are going to download full message anyways
				{
					bCallTranslators = TRUE;//we need to call translators.
				}
			}
		}
		
		nDoneSoFar += nStatus;
		if( nDoneSoFar <= lEstimatedSize)
			m_pTaskInfo->SetSoFar( (int)(nSoFar + ((double)nDoneSoFar/lEstimatedSize)*100));
	}

	//	If above calls to pSpoolFile->Put all succeeded, then flush the file buffer
	//	to disk to make sure that all writes worked (the file would have been flushed
	//	anyway below in either Seek or Close, but explicitly calling Flush here allows
	//	for one central error check).
	if ( SUCCEEDED(hrFile) )
		hrFile = pSpoolFile->Flush();

	if ( FAILED(hrFile) )
	{
		nStatus = -1;

		//	Notify the user in the task error that we had a problem with a temp file
		ErrorDialog(IDS_ERR_POP_TEMP_FILE);

		//	Write entry into log describing the details of what went wrong
		wsprintf( szBuffer, CRString(IDS_DEBUG_POP_WRITE_TEMP_FAILED), strFileName, pSpoolFile->GetLastError_() );
		::PutDebugLog(DEBUG_MASK_RCVD, szBuffer);
	}

	//	Call the translator only if:
	//	* There was no error
	//	* We are not downloading the full message
	//	* "X-Eudora-Plugin-Info" is present
	if ( (nStatus >= 0) && bCallTranslators && !bMessageEntire )
	{
		pSpoolFile->Seek(0);

		HeaderDesc			hd(1000, TRUE);
		FileLineReader		lineReader;

		lineReader.SetFile( pSpoolFile.get() );
		hd.Read(&lineReader);					

		CTranslatorManager *pManager = ((CEudoraApp*)AfxGetApp())->GetTranslators();
		if ( pManager->WantsFullMessage(hd.m_TLMime, EMS_POPAccount) )
			reDownloadStatus |= kReDownloadForPluginFlag;
	}

	if (reDownloadStatus != kNoReDownload) // we need to redownload so throw away the spool file
	{
		pSpoolFile->Close();
		FileRemoveMT(strFileName);
		nStatus = -1;
	}

	//
	// Form our own message id by concatenating the message id
	// from the header and the received date, converting any
	// embedded spaces to dashes.
	//
	if(nStatus >= 0)
	{
		if (m_pPopHost)
		{
			//char szMessageId[2000];
			char szMessageId[LMOS_MSGID_SIZE+1] = {0};
			
			if ( !bKeepSpoolFileOpen && !m_pPopHost->CanUIDL() )
			{
				//	The computed hash is only used for Non-UIDL servers and is only calculated when
				//	bKeepSpoolFileOpen is false (i.e. when we haven't already been called once).
				unsigned long hash = ComputeMessageIdAndHash(strFileName, szMessageId);

				//
				// Set the message ID in the corresponding LMOS record.
				//
				pMsgRecord->SetMesgID(szMessageId);

				//Update the hash and map index also, as it is index by the hash value
				m_pPopHost->UpdateHashValue(pMsgRecord, hash);
			}
		}
		else
		{
			//
			// This should never happen, right?
			//
			ASSERT(0);
		}

		//	Call EndWrite (which in turn calls FileRenameMT), checking for any error when renaming
		//	the temp file to the .rcv file.
		if (bKeepSpoolFileOpen)
		{
			//	We're keeping the spool file open
			hrFile = SpoolMgr->m_In->EndWrite( pMsgRecord->GetHashValue(), pMsgRecord, pSpoolFile.get() );
			
			if ( FAILED(hrFile) )
			{
				//	Notify the user in the task error that we had a problem with a temp file
				ErrorDialog(IDS_ERR_POP_TEMP_FILE);
			}
			else
			{
				//	EndWrite suceeded so make sure the auto_ptr doesn't delete the spool file
				pSpoolFile.release();
			}
		}
		else
		{
			//	We're not keeping the spool file open
			pSpoolFile->Close();
			hrFile = SpoolMgr->m_In->EndWrite(pMsgRecord->GetHashValue(), pMsgRecord);

			if ( FAILED(hrFile) )
			{
				//	Check setting to see if we should:
				//  * attempt again keeping the file open (default value of setting)
				//	* just fail (old behavior)
				//	* just ignore the error
				switch ( m_Settings->GetSpoolErrorAction() )
				{
					case kReDownloadKeepingSpoolFileOpen:
						//	Pass back that we want to redownload keeping the spool file open
						reDownloadStatus |= kReDownloadKeepingSpoolFileOpenFlag;
						break;

					case kFail:
						//	Notify the user in the task error that we had a problem with a temp file
						ErrorDialog(IDS_ERR_POP_TEMP_FILE);
						break;
					
					case kIgnoreFailureAndDeleteMessage:
						//	Forget the error and the fact that the message wasn't downloaded
						//	successfully and just delete the message anyway
						hrFile = NOERROR;
						break;
				}
			}
		}

		//	Remember whether or not we had an error
		nStatus = FAILED(hrFile) ? -1 : 1;
	}
	else
	{
		nStatus = -1;
		SpoolMgr->m_In->EndWrite();
	}


	m_pTaskInfo->SetSoFar(nSoFar+100);
	return nStatus;
}




////////////////////////////////////////////////////////////////////////
// GetMsgFetchType [public]
//
// Decide whether to skip the message, fetch a message stub, or fetch
// the whole message.
////////////////////////////////////////////////////////////////////////
MsgFetchType CPOPSession::GetMsgFetchType(CMsgRecord *pMsgRecord)
{
	MsgFetchType msgFetchType = MSGFETCH_NOTHING;

	if (m_ulBitFlags & kFetchHeadersBit)
		msgFetchType = MSGFETCH_HEADER;

	// see if we have enough disk space
	DWORD dwTotalBytes = ::GetAvailableSpaceMT(::EudoraDir);

	// Only consider attachment directory if it's on a different drive from EudoraDir, and it actually exists.
	// If it doesn't exist, then the attachment code will wind up putting in a directory off of EudoraDir.
	CString strAttachDir = m_Settings->GetAttachDir();
	if (!strAttachDir.IsEmpty() && strnicmp(EudoraDir, strAttachDir, 1) && ::FileExistsMT(strAttachDir, TRUE))
	{
		DWORD dwBytes = ::GetAvailableSpaceMT(strAttachDir);
		dwTotalBytes = min(dwTotalBytes, dwBytes);
	}

	if (dwTotalBytes < 65536)		// too little disk to even try
	{
		ErrorDialog(IDS_DISK_SPACE_OUT);
		return MSGFETCH_NOTHING;
	}

	if (dwTotalBytes < DWORD(pMsgRecord->GetEstimatedSize()))
	{
		ErrorDialog(IDS_DISK_SPACE_LOW);
		return MSGFETCH_STUB;		// Treat it like a Skip Big Message with a big message
	}

	if ((	m_ulBitFlags & kRetrieveMarkedBit) && 
			pMsgRecord->GetReadFlag() == LMOS_HASBEEN_READ && 
			pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE 
		)
	{
		//
		// You have a stub that you've read and marked for fetch.
		//
		return MSGFETCH_ENTIRE;
	}

	if(	(m_ulBitFlags & kRetrieveNewBit) && 
		pMsgRecord->GetReadFlag() == LMOS_NOT_READ 
		)
	{
		if (m_Settings->SkipBigMessages() &&									//GetIniShort(IDS_INI_SKIP_BIG_MESSAGES) && 
			pMsgRecord->GetEstimatedSize() > m_Settings->GetBigMsgThreshold() && //GetIniLong(IDS_INI_BIG_MESSAGE_THRESHOLD)
			m_Settings->GetBigMsgThreshold() > 0
			)
		{
			return MSGFETCH_STUB;
		}
		else
			return MSGFETCH_ENTIRE;
	}
	
	return msgFetchType;
}


//
//	CPOPSession::SaslChallenge
//		Obtains a challenge from the POP server, decodes it and places it in outbuf.
//
//	Parameters:
//		outbuf[out]: Buffer for decoded challenge.
//		outsize[in]: Size of szChal buffer.
//
//	Return:
//		Length of the challenge contained in outbuf.
//
int CPOPSession::SaslChallenge(char *outbuf, int outsize)
{
	int			 len = 0;

	if (!outbuf)
	{
		return 0;
	}

	char buf[QC_NETWORK_BUF_SIZE];
	*buf = 0;

	// Read a line from the POP server.
	if (m_pNetConnection->GetLine(buf, sizeof(buf)) < 0)
	{
		return 0;
	}

	if (buf[0] == '-')
	{
		// Error message doesn't need to be decoded.
		len = outsize - 1;		// - 1 because we copy len + 1 below.
	}
	else
	{
		// Skip past the "+ ".
		strcpy(buf, buf + 2);

		// Decode the string.
		len = POPB64DecodeString(buf);
		buf[len] = 0;
	}

	// Make sure the output buffer can hold the data.
	if (len > outsize)
	{
		return 0;
	}

	// Now send the output back.
	if (len > 0)
	{
		buf[len] = 0;
		// Need to use memcpy() not strcpy() because data can contain NULLs.
		memcpy(outbuf, buf, len + 1);
	}

	return len;
}

//
//	CPOPSession::SaslResponse
//		Encodes the response and sends it to the POP server.
//
//	Parameters:
//		response[in]: Response to encode and send.
//		size[in]: Length of response.
//
//	Return:
//		TRUE if response was sent, FALSE otherwise.
//
BOOL  CPOPSession::SaslResponse (char *response, unsigned long size)
{
	char *out = NULL;

	if (size)
	{
		// Encode response.
		//
		int outsize = (size * 4)/3 + 20;
		out = DEBUG_NEW char [outsize];

  		/* make CRLF-less BASE64 string */
		int len = POPB64EncodeString(response, size, out, outsize);

		int		j;
		char	*t, *u;

		for (t = out, u = t, j = 0; j < len; j++)
		{
			if (t[j] > ' ')
				 *u++ = t[j];
		}

		*u = '\0';

		/* append CRLF */
		*u++ = '\015'; *u++ = '\012';

		*u = '\0';
	}
	else if (response)
	{
		// Data string is empty but not NULL: send no data.
		out = DEBUG_NEW char [8];
		wsprintf (out, "\r\n");
	}
	else
	{
		// Data string is NULL: send SASL cancel.
		out = DEBUG_NEW char [8];
		wsprintf (out, "*\r\n");
	}

	// the command buffer is now ready, send the command to the server...
	int status = m_pNetConnection->PutDirect(out);

	// Cleanup:
	delete[] out;

	if (status < 0)
	{
		m_pNetConnection->Close();
		return (FALSE);
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OpenPOPConnection_ [private]
//
// Establishes the connection to the POP server as necessary, using
// the current POP account, service name, and port id.  The service name
// and port id are different for the Kerberos case.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::OpenPOPConnection_()
{
	if (m_bPOPConnected)
		return TRUE;

	m_pTaskInfo->SetMainText(CRString(IDS_SERVER_CONNECT));
	//
	// Grab POP account setting
	//
	const char *szPOPAccount = m_Settings->GetPOPAccount();
	
	if (*szPOPAccount == 0) 
	{
		if (!gbAutomationCheckMail)
			ErrorDialog(IDS_ERR_NO_POP_ACCT);
		return FALSE;
	}

	//
	// Extract the POP server from the POP account string.
	//
	char* pszPOPServer = strrchr(szPOPAccount, '@');
	if (!pszPOPServer || !*++pszPOPServer)
	{
		if (!gbAutomationCheckMail)
			ErrorDialog(IDS_ERR_BAD_POP_ACCT, szPOPAccount);
		return FALSE;
	} 

	//
	// Decide which service and port id to use...
	//
	UINT idPopService	= IDS_POP_SERVICE;
	UINT idPopPort		= m_Settings->GetPort();
	
	UINT DefaultPort	= 110;
	if (m_Settings->GetAuthenticationType() == AUTH_KERBEROS)
	{
		idPopService	= IDS_KERB_POP_SERVICE;	// kpop
		idPopPort		= (int) GetIniShort(IDS_INI_KERB_PORT);
		DefaultPort		= 1110;
    }
	else if(m_Settings->m_SSLSettings.GetSSLReceiveUsage() == QCSSLSettings::SSLUseAlternatePort)
	{
		idPopPort		= m_Settings->m_SSLSettings.m_POPAlternatePort;
    	DefaultPort = 995;
	}

	char Server[128];
	strncpy(Server, pszPOPServer, sizeof(Server));

	// Hesiod Kludge code.  There should really be a userinterface for this
	if (!stricmp(Server, "hesiod") && SUCCEEDED(QCHesiodLibMT::LoadHesiodLibrary()))
	{
		CString LoginName(szPOPAccount);
		int At = LoginName.ReverseFind('@');
		if (At > 0)
			LoginName.ReleaseBuffer(At);
		int nHesiodError = -1;
		if (FAILED(QCHesiodLibMT::GetHesiodServer(CRString(IDS_POP_SERVICE), LoginName, Server, sizeof(Server), &nHesiodError)))
			*Server = 0;
		QCHesiodLibMT::FreeHesiodLibrary();
		if (!*Server)
		{
			if (!gbAutomationCheckMail)
				ErrorDialog(IDS_ERR_HESIOD_HOST_LOOKUP);
			return FALSE;
		}
	}
	// End Hesiod kludge

	//
	// Establish the network connection to the POP server...
	//
	if (!m_pNetConnection || m_pNetConnection->Open(Server, idPopService, idPopPort, DefaultPort, TRUE) < 0)
		return FALSE;

	m_bPOPConnected = TRUE;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// POPCmdGetReply_ [public]
//
// Takes an argument request, and a place to store the
// result and does the footwork
////////////////////////////////////////////////////////////////////////
int CPOPSession::POPCmdGetReply(UINT uCmdID, const char* pszArgs, char* pszBuffer, int nBufferSize, 
								BOOL bReportError /*= TRUE*/ )
{
	//
	// Format the command string, with optional arguments.  Update
	// the progress display with the outgoing command string.
	//
	char szCommand[QC_NETWORK_BUF_SIZE];
	strcpy(szCommand, CRString(uCmdID));

	if (uCmdID == IDS_POP_PASS || uCmdID == IDS_POP_APOP)
		m_pTaskInfo->Progress( szCommand );			// Don't show arguments to some commands
	if (pszArgs)
		strcat(szCommand, pszArgs);		// Add arguments
		
	if (uCmdID != IDS_POP_PASS && uCmdID != IDS_POP_APOP)
		m_pTaskInfo->Progress( szCommand );
	strcat(szCommand, "\r\n");

	//
	// Send the command to the server.
	//
	int nStatus = m_pNetConnection->PutDirect(szCommand);
	if (nStatus < 0)
		return nStatus;

	//
	// Read back the results, until we see either +OK or -ERR.
	//
	do
	{
		nStatus = m_pNetConnection->GetLine(pszBuffer, nBufferSize);
		if (nStatus == 0)
			return -1;	// 0 means the connection closed cleanly.
						// however, the connection didn't ought to have closed yet
						// so return an error to the caller
		if (nStatus < 0)
			return nStatus;
		if (nStatus > 1)
			pszBuffer[nStatus - 2] = 0;
	} while (strncmp(pszBuffer, "+OK", 3) != 0 && strncmp(pszBuffer, "-ERR", 4) != 0);

	if (*pszBuffer == '-')
	{
		
		//
		// POP command resulted in error.
		//
		if (bReportError)
		{
			if ((IDS_POP_PASS == uCmdID) || (IDS_POP_APOP == uCmdID))
			{
				// Default to cleaning up password if we can't be sure the
				// password is the same (because it was newly typed - not saved) OR
				// the password has never worked before.
				m_bCleanup = m_Settings->GetSavePass().IsEmpty() || (m_lastAuthTime == 0);

				char* ExtendedResponse = pszBuffer + 4;

				while (isspace((unsigned char)*ExtendedResponse))
					ExtendedResponse++;
				char *szResponse = _strlwr( ::SafeStrdupMT(ExtendedResponse));

				// If the AUTH-RESP-CODE capability was seen, only erase the password
				// if the extended response code is AUTH; see RFC 2449
				if( m_bSupportsAuthRespCode )
					m_bCleanup = ( strnicmp(szResponse, "[AUTH", 5) == 0 );
				else
				{
					char *pszBadPass = _strlwr(::SafeStrdupMT(m_Settings->GetBadPass()));  //GetIniString(IDS_INI_BAD_PASSWORD_STRING);
					if (!*pszBadPass || strstr(szResponse, pszBadPass) == NULL)
					{
						// Check for POP3 Extended Response [IN-USE] to see if maildrop is locked.
						// If so, then don't erase the password because it's not bad.
						// Also don't erase password if we get [LOGIN-DELAY].
						if( strnicmp(szResponse, "[IN-USE", 7) == 0 || strnicmp(szResponse, "[LOGIN-DELAY", 12) == 0 )
							m_bCleanup = false;
						else
						{
							char *szWordList = _strlwr( ::SafeStrdupMT( m_Settings->GetPasswordOKWordList()));
							char *token = strtok(szWordList, ",");
							while (token)
							{
								if( strstr(szResponse, token))
								{
									m_bCleanup = false;
									break;
								}
								token = strtok(NULL, ",\0");
							}
							delete [] szWordList;
						}
					}
					delete [] pszBadPass;
				}
				delete [] szResponse;
				
				wsprintf(szCommand, "%s%s", (const char *) CRString(uCmdID),
						(const char *) CRString(IDS_ERR_POP_PASSWORD_HIDE));
			}

			//
			// Display error message for this POP session.
			//
			ErrorDialog(IDS_ERR_TRANSFER_MAIL, szCommand, "POP", m_Settings->GetPOPAccount(), pszBuffer + 1);
		}

		return -1;	// failure
	}

	return 1;		// success
}



#pragma optimize ("y", on)

//	================================================================================
//	RPAAuthenticate_ -- provides the mechanics for setting up and using the RPA via SSPI
//				  authentication protocols...

int	CPOPSession::RPAAuthenticate_()
{
	char buf[128];
	char cmdbuf[128];
	
	//Only one thread at a time can do the RPA authentication
	CSingleLock lock(&m_RPAAuthGuard, TRUE);

	QCAuthRPA Rpa(m_pNetConnection, makeCallback( (Callback1<const char *>*)0, *this, &CPOPSession::AuthError));

	int rc = Rpa.LoadRPALibrary();
	if ( rc == 0)
	{
		if ((rc = Rpa.GetCredentialHandle()) == 0)
		{
			// Send the POP server a AUTH command for RPA
			strcpy(cmdbuf, CRString(IDS_POP_AUTH));
			strcat(cmdbuf, CRString(IDS_POP_RPA));
			strcat(cmdbuf, "\r\n");
			// Send the command to the server
			rc = m_pNetConnection->PutDirect(cmdbuf);
			if (rc >= 0)
			{
				rc = m_pNetConnection->GetLine(buf, sizeof(buf));
				if (rc > 0)
				{
					if (rc > 1)
						buf[rc - 2] = 0;
					// for AUTH RPA it only returns a +.
					if (buf[0] == '+')
					// Now we have a message from the POP server of the form waiting to be read...
						rc = Rpa.Authenticate(m_Settings->GetPOPAccount());
					else if(buf[0] == '-') //we had an error
					{
						ErrorDialog(IDS_ERR_TRANSFER_MAIL, cmdbuf, "POP", m_Settings->GetPOPAccount(), buf + 1);
						return -1;
					}
				}
			}
		}
	}
	Rpa.Cleanup();
	return (rc);
}


////////////////////////////////////////////////////////////////////////
// FindFirstUnread_ [private]
//
// Queries the server for the first unread message.  Returns unread
// message number.  If necessary, does brute force binary search of
// messages on the server.
////////////////////////////////////////////////////////////////////////
int CPOPSession::FindFirstUnread_()
{
	switch (GetFetchType_())
	{
	case FETCH_NORMAL:
		return 0;

	case FETCH_LAST:
		{
			//
			// Send the LAST command and return the integer result.
			//
			char szBuffer[64];
			if (POPCmdGetReply(IDS_POP_LAST, NULL, szBuffer, sizeof(szBuffer)) < 0)
				return -1;
			return (atoi(szBuffer + 4) + 1);
		}
		break;

	case FETCH_MSG_STATUS:
		break;			// fall through to stuff below...

	default:
		ASSERT(0);
		return -1;
	}
    
	//
	// Check to see if the last message has already been read.
	//
	int nHasBeenRead = POPMsgHasBeenRead_(m_nMessageCount);
	if (nHasBeenRead < 0)
		return -1;		// network read error
	else if (nHasBeenRead)
	{
		//
		// Last message already read, so return next message number.
		//
		return (m_nMessageCount + 1);
	}

	//
	// If we get this far, we have to use the brute force method to
	// check, so put up the progress dialog.  Start with the number
	// of messages in the queue, then do a binary search through
	// the existing messages on the server.
	//
	//Progress(0, CRString(IDS_POP_IGNORING_MESSAGES), m_nMessageCount);
	m_pTaskInfo->Progress(CRString(IDS_POP_IGNORING_MESSAGES));

	int nFirst = 1;					// Starting point for search
	int nLast = m_nMessageCount;	// Last message for this host
	int nCurrent = nLast;			// Our current search point

	while (nFirst <= nLast)
	{
		nCurrent = (nFirst + nLast) / 2;
		nHasBeenRead = POPMsgHasBeenRead_(nCurrent);
		if (nHasBeenRead < 0)
			return -1;				// network read error
		else if (nHasBeenRead)
			nFirst = nCurrent + 1;	// not in top half, so try bottom half
		else
			nLast = nCurrent - 1;	// somewhere in top half, narrow the search

		//Progress(m_nMessageCount - (nLast - nFirst));
	}

	//
	// Close the progress dialog.
	//
	//Progress(1, NULL, 1);

	if (! nHasBeenRead)
		return nCurrent;
	else
		return nCurrent + 1;
}




////////////////////////////////////////////////////////////////////////
// POPAuthenticate_ [private]
//
// Get the POP connection up and running by authenticating and
// fetching the number of available messages via the STAT command.  The
// authentication method varies based on whether we're using POP,
// APOP, or Kerberos.
//
// A positive return value indicates the count of letters waiting.  A
// zero return value indicates no mail waiting, but no failure.  A
// negative return value indicates something bad happened like
// authentication failed or there was a network error.
////////////////////////////////////////////////////////////////////////
int CPOPSession::POPAuthenticate_()
{
	ASSERT(m_bPOPConnected);
	ASSERT(m_pNetConnection);

	if (GetAuthenticationType_() == AUTH_KERBEROS)
	{
		//
		// Doing Kerberos, so we'd better try to get a ticket....
		//
		KerberosErrType errKerb = KerberosAuthenticate_();
		m_pTaskInfo->Progress(CRString(IDS_KERB_LOGIN));

		//
		// At this point we have either checked the Kerberos stuff or we
		// haven't.  If we haven't, the status is defaulted to '1' so
		// we continue as normal.  If we have checked Kerberos, then we
		// either continue or exit based on the return code from the
		// Kerberos connection things.
		//
		if (errKerb != KERBERR_OK)
		{
			if ((errKerb != KERBERR_LOAD_LIB_FAILED) &&
				(errKerb != KERBERR_CANCELED))
			{
				ErrorDialog(IDS_KERB_ERROR, int(errKerb));
			}

			// Set up for a clean exit....	
			m_bPOPConnected = FALSE;
			ASSERT(int(errKerb) < 0);
			return int(errKerb);
		}
	}

	//
	// If we get this far, we have a valid Kerberos or POP connection.
	//
	int nStatus = 1;			// Init the status to OK
	char szBuffer[QC_NETWORK_BUF_SIZE];
	memset(szBuffer, 0, sizeof(szBuffer));

	do
	{
		//
		// Start getting stuff from the server, looking for a '+' or '-' 
		// which starts a line
		//
		nStatus = m_pNetConnection->GetLine(szBuffer, sizeof(szBuffer));
		if (nStatus < 0)
		{
			//
			// Never got the banner.  Abort!
			//
			m_bPOPConnected = FALSE;
			return nStatus;
		}
	} while (*szBuffer != '+' && 
			 *szBuffer != '-' && 
			 strncmp(szBuffer, "+OK", 3) != 0 &&
			 strncmp(szBuffer, "-ERR", 4) != 0);
	

	if(GetAuthenticationType_() == AUTH_RPA)
	{
		// Yes -- Better try to get a ticket....
		nStatus = RPAAuthenticate_();
		if (nStatus < 0)
		{
			// Never got the Authentication.  Abort.
			m_bPOPConnected = FALSE;
			return nStatus;
		}
	}

	if (*szBuffer != '+')
	{
		//
		// Did not get a '+' like we wanted.  Abort!
		//
		ErrorDialog(IDS_ERR_POP_CMD, m_Settings->GetPOPAccount(), szBuffer + 1);
		return -1;
	}
	
	GetCapabilities();
	if(m_bSupportsSSL && (m_Settings->m_SSLSettings.m_SSLReceiveAvailableOrAlways))
	{
		if (SSLStart())
		{
			// When doing SSL, the first thing you should do after a successful negotiation
			// is redownload the server's capabilities (see ForgetCapabilities() for explanation).
			// The negotiation won't actually happen until we send something, so send the
			// request for the capabilities list.  This will cause the negotiation to happen
			// and will generate a new capabilities list.
			ForgetCapabilities();
			GetCapabilities();
		}
	}
	if(!m_bSupportsSSL && m_Settings->m_SSLSettings.m_nSSLReceiveUsage == SSLSettings::SSLUseAlways)
		{
		ErrorDialog(IDS_SSL_NOSUPPORT, m_Settings->GetPOPAccount());
		return -1;
		}
	//
	// Create the USER string by stripping the '@domain.com'
	// from the POP account string.
	//
	char szPOPUserName[128];
	strcpy(szPOPUserName, m_Settings->GetPOPAccount());
	char *pAtSign = strrchr(szPOPUserName, '@');
	if (NULL == pAtSign)
	{
		ASSERT(0);
		return (-1);
	}
	*pAtSign = 0;		// truncate the POP account at the '@' sign

	//
	// Authenticate the POP account.
	//
	switch (m_Settings->GetAuthenticationType())
	{
	case AUTH_POP:
	case AUTH_KERBEROS:
		//
		// In Kerberos we have already gotten the ticket for service,
		// and stuck the fake password into the global 'POPPassword'.
		//
		if (this->POPCmdGetReply(IDS_POP_USER, szPOPUserName, szBuffer, sizeof(szBuffer)) < 0 ||
			this->POPCmdGetReply(IDS_POP_PASS, m_strPassword, szBuffer, sizeof(szBuffer)) < 0)
		{
			return -1;
		}
		break;  
	case AUTH_RPA:
		break;
	case AUTH_APOP:
		{
			//
			// Doing APOP, so gen the digest using the opening banner
			// from the POP server, combine the digest with the
			// account user name, and send it with the APOP command.
			//
			char szPassword[255];
			ASSERT(strlen(m_strPassword) < sizeof(szPassword));
			strcpy(szPassword, m_strPassword);
			
			char szDigest[255];
			memset(szDigest, 0, sizeof(szDigest));
			if (! ::GenerateMD5Digest(szBuffer, szPassword, szDigest))
				return -1;

			char szAPOPUserName[500];
			sprintf(szAPOPUserName, "%s%s", szPOPUserName, szDigest);
			
			if (this->POPCmdGetReply(IDS_POP_APOP, szAPOPUserName, szBuffer, sizeof(szBuffer)) < 0)
				return -1;
		}
		break;

	case AUTH_KERBEROS_5:
		{
			// Tell POP server we are doing GSSAPI.
			char		 szCmdbuf[128];
			strcpy(szCmdbuf, CRString(IDS_POP_AUTH));
			strcat(szCmdbuf, CRString(IDS_POP_GSSAPI));
			strcat(szCmdbuf, "\r\n");
			int nStatus = m_pNetConnection->PutDirect(szCmdbuf);
			if (nStatus < 0)
			{
				return nStatus;
			}

			// Grab POP account setting.
			const char *szPOPAccount = m_Settings->GetPOPAccount();
			if (*szPOPAccount == 0) 
			{
				 if (!gbAutomationCheckMail)
				 {
					ErrorDialog(IDS_ERR_NO_POP_ACCT);
				 }
				ASSERT(0);
				return -1;
			}
			// Extract the POP server from the POP account string.
			char* pszPOPServer = strrchr(szPOPAccount, '@');
			if (!pszPOPServer || !*++pszPOPServer)
			{
				ASSERT(0);
				return -1;
			}

			// Perform the authentication.
			if (!POPGssapiAuthenticator(pop_challenge, pop_response,
										this,
										szPOPUserName, ((QCWorkerSocket*)m_pNetConnection)->GetMachineName()))
			{
				// Authentication failed.  We don't need to show a message
				// because the authenticator handled that.
				return -1;
			}

			// Read the server response to the authentication process.
			if (ReadPOPLine(szBuffer, sizeof(szBuffer)) < 0 || strncmp(szBuffer, "+OK", 3) != 0)
			{
				// Server didn't give us the OK response: authentication failed.
				AuthError(szBuffer);
				return -1;
			}
			break;
		}
	default:
		ASSERT(0);
		return -1;
	}

	// Auth successful!  Remember time
	m_lastAuthTime = time(NULL) + m_GMTOffset;

	// Get the message count from the server
	if (this->POPCmdGetReply(IDS_POP_STAT, NULL, (char *) szBuffer, sizeof(szBuffer)) < 0)
		return (-1);

	// Successful response to STAT is of the form: +OK num_of_messages num_bytes_of_messages
	// Check if the number of messages is there (as that's what we're returning in this
	// function) as returning zero makes it appear to Eudora as though there are no messages
	// on the server, and the existing LMOS info will get purged.
	if (strlen(szBuffer) < 4)
		return (-1);
	char* NumMessagesStart = szBuffer + 3;
	while (*NumMessagesStart == ' ')
		NumMessagesStart++;
	if (!isdigit((unsigned char)*NumMessagesStart))
		return (-1);
	nStatus = atoi(NumMessagesStart);

	return nStatus;
}

 


////////////////////////////////////////////////////////////////////////
// MatchLastMessage_ [private]
//
// Called only for non-UIDL servers.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::MatchLastMessage_(int nLast)
{
	
	// If trying to match message #0 (i.e. no messages in current LMOS.DAT), then go
	// ahead and return TRUE since it's just going to get every message
	if (nLast == 0)
		return TRUE;
	
	//the global static index is removed with this variable
	unsigned long ulMessageIndex = 0; 

	BOOL	theReturn = FALSE;  

	
	// The message could be one of two places.  If the messages marked as having the
	// DELE command sent to them didn't actually get deleted, then the last message
	// will be correctly placed at the end of the list.  However, if messages did
	// get deleted, then we need to subtract off all the messages that are marked as
	// having the DELE command sent to them.
	
	int HighIndex, LowIndex;
	CMsgRecord * theLastMsg = m_pPopHost->FindHighLowIndices(HighIndex, LowIndex, nLast);

	if (!theLastMsg)
		return (FALSE);
	
	// Check the low index first, since it's where the message should be
	char szMessageId[2000];
	unsigned long theHash = MakeFakeMessageIdAndHash_(LowIndex, szMessageId);

	if (theLastMsg->IsEqualMesgID(szMessageId))
	{
		//s_ulMessageIndex = LowIndex;
		ulMessageIndex = LowIndex;
		theReturn = TRUE;
		
		// The low index matched, so we need to get rid of the entries that were
		// marked as having the DELE command sent because they really were deleted
		m_pPopHost->RemoveDeletedMsgRecords();
	}
	else
	{
		// Low index failed, try high one, but only if different
		if (LowIndex != HighIndex)
		{
			theHash = MakeFakeMessageIdAndHash_(HighIndex, szMessageId);
			
			if (theLastMsg->IsEqualMesgID(szMessageId))
			{
				//s_ulMessageIndex = HighIndex;
				ulMessageIndex = HighIndex;
				theReturn = TRUE;
				
				m_pPopHost->ReplaceDeleteSentWithDelete();
			}
		}
	}

	return theReturn;

}


////////////////////////////////////////////////////////////////////////
// ReadPOPLine [public, static]
//
// Reads one line from the POP server.  Strips extra leading periods and
// quotes envelopes.  Returns the number of characters read, 0 if this was
// the end of transmission (<period><newline>).
////////////////////////////////////////////////////////////////////////
int CPOPSession::ReadPOPLine(char* pszBuffer, int nBufferSize)
{
	if (NULL == m_pNetConnection)
	{
		ASSERT(0);
		return -1;
	}

	int nBytesRead = m_pNetConnection->GetLine(pszBuffer, nBufferSize);
	if (nBytesRead == 0)
		return -1;		// We're expecting something, so getting nothing is an error
	if (nBytesRead < 0) 
		return nBytesRead;		// network read error

	if ('.' == pszBuffer[0])
	{
		if ('\r' == pszBuffer[1] || '\n' == pszBuffer[1])
		{
			//
			// Hit end of transmission.
			//
			pszBuffer[0] = 0;
			nBytesRead = 0;
		}
		else if ('.' == pszBuffer[1])
		{
			//
			// Found two periods in a row at beginning of line,
			// so strip extra leading period.
			//
			strcpy(pszBuffer, pszBuffer + 1);
			nBytesRead--;
		}
	}
	else if (::IsFromLine(pszBuffer))
	{
		//
		// Quote Unix-style from lines of the form 
		//
		//    "From beckley@qualcomm.com Thu Oct 15 16:15:08 1992"
		//
		// with a leading '>' character.
		//
		memmove(pszBuffer + 1, pszBuffer, ++nBytesRead);
		pszBuffer[0] = '>';
	}

	return nBytesRead;
}


////////////////////////////////////////////////////////////////////////
// ExtractMessageID_ [private]
//
// Given a read-only buffer (pszBuffer), copy the message ID from that
// buffer into the given target buffer (pszMsgId).  Returns an empty
// buffer if not successful.
////////////////////////////////////////////////////////////////////////
void CPOPSession::ExtractMessageID_(const char* pszBuffer, char *pszMsgId)
{
	//
	// pszBuffer contains something like...
	//
	// "Message-ID: <some junk>\r\n"
	//
	char* pszBuf = strchr(pszBuffer,'<');
	if (pszBuf != NULL)
	{
		//
		// Found the leading angle bracket, so copy stuff from the
		// source buffer to the target buffer.
		//
		strncpy(pszMsgId, pszBuf, LMOS_MSGID_SIZE - 1);
		pszMsgId[LMOS_MSGID_SIZE - 1] = 0;

		char* pszBuf = strchr(pszMsgId,'>');
		if (pszBuf != NULL)
			pszBuf[1] = 0;

		// 
		// Convert any spaces to dashes.
		//
		for (pszBuf = pszMsgId; *pszBuf; pszBuf++)
		{
			if(*pszBuf == ' ')
				*pszBuf = '-';
		}
	}
	else
		*pszMsgId = 0;
}


////////////////////////////////////////////////////////////////////////
// ExtractDateTime_ [private]
//
// Given a read-only buffer (pszBuffer), copy the date from that
// buffer into the given target buffer (pszDate).  Returns an empty
// buffer if not successful.
////////////////////////////////////////////////////////////////////////
void CPOPSession::ExtractDateTime_(const char* pszBuffer, char* pszDate)
{
	//
	// pszBuffer contains something like...
	//
	// "Received: from (whole bunch of address info); Day, 3 Feb 1995 07:47:50 -0800\r\n"
	//                                                     ^ ^^^ ^^^^ ^^ ^^ ^^ ^^^^^
	// From that we need to extract and return that to the caller
	//
	char* pszBuf = strchr(pszBuffer, ',');
	if (pszBuf != NULL)
	{
		//
		// Found comma, so step over it and copy stuff from the source
		// into the target buffer.
		//
		pszBuf++;
		strncpy(pszDate, pszBuf, LMOS_MSGID_SIZE - 1);
		pszDate[LMOS_MSGID_SIZE - 1] = 0;
		
		//
		// Find the CRLF terminator and the end of the string
		// and convert it to a null terminator.
		//
		pszBuf = strchr(pszDate, '\r');
		if (pszBuf != NULL)
			*pszBuf = 0;
			
		// 
		// Get rid of leading and trailing whitespace, then convert
		// any "internal" spaces to dashes.
		//
		::TrimWhitespaceMT(pszDate);
		for (pszBuf = pszDate; *pszBuf; pszBuf++)
		{
			if (*pszBuf == ' ')
				*pszBuf = '-';
		}
	}
	else
		*pszDate = 0;
}


////////////////////////////////////////////////////////////////////////
// MakeFakeMessageIdAndHash_ [private]
//
// Given a message number, retrieve the header of the message using the
// TOP command.  Search the header to 1) extract the date from the 
// Received: header and 2) extract the message ID from the Message-Id:
// header.  If both are found, then concatenate the values into a "fake"
// message ID (returned in 'pszFakeMessageID' and then return the hash 
// of the fake message ID.
//
// This routine is only used when the server doesn't support UIDL.
////////////////////////////////////////////////////////////////////////
unsigned long CPOPSession::MakeFakeMessageIdAndHash_(int nMsgNum, char* pszFakeMessageID) 
{
	// Put some garbage into the string
	strcpy(pszFakeMessageID, "I'll gladly pay you Tuesday for a hamburger today.");

	// Range check
	if (nMsgNum < 1 || nMsgNum > m_nMessageCount)
		return 0;

    
	//
	// Send the POP server a TOP command.  Request one line of the
	// body because some POP servers choke on a TOP command with zero
	// for the number of body lines to return.
	//
	char szBuffer[QC_NETWORK_BUF_SIZE] = {0};
	{
		char szCommand[32];
		wsprintf(szCommand, "%d 1", nMsgNum);
		if (this->POPCmdGetReply(IDS_POP_TOP, szCommand, szBuffer, sizeof(szBuffer)) < 0)
			return 0;
	}

	//
	// Setup search variables...
	//
	BOOL bFoundDate = FALSE;			// TRUE if we extracted a date string
	BOOL bFoundMsgId = FALSE;			// TRUE if we extracted a Message-Id string
	BOOL bInReceivedHdr = FALSE;
	BOOL bInMsgIdHdr = FALSE;
	char szDate[500];
	char szMsgId[500];
	unsigned long ulHash = 0; 		// returned
	CRString strReceivedHeader(IDS_POP_RECEIVED_HEADER);
	CRString strMsgIdHeader(IDS_POP_MESSAGE_ID_HEADER);

	szDate[0] = 0;
	szMsgId[0] = 0;

	//
	// Process each line of TOP data, one at a time.  Stop when we hit 
	// an empty line.
	//
	while ((ReadPOPLine(szBuffer, sizeof(szBuffer)) > 0) && (*szBuffer != '\r'))
	{
		if (*szBuffer != ' ' && *szBuffer != '\t')
		{
			bInReceivedHdr = FALSE;
			bInMsgIdHdr = FALSE;
		}

		// 
		// If we are in a folded Received header and the first char is
		// a space or a tab, then we need to check for the date.  OR,
		// if this is a beginning of a Received header then we need to
		// check for the date also.
		//
		if (!bFoundDate &&
			((bInReceivedHdr && (*szBuffer == ' ' || *szBuffer == '\t')) ||
			 (!strnicmp(szBuffer, strReceivedHeader, strReceivedHeader.GetLength()))))
		{
			//
			// Check to see if we have Date info in this "Received:"
			// header line.  If so, store it in szDate[].
			//
			szDate[0] = 0;
			ExtractDateTime_(szBuffer, szDate);
			if (szDate[0] != 0)
				bFoundDate = TRUE;
			else
				bInReceivedHdr = TRUE;
		}
		else if (!bFoundMsgId && 
				 ((bInMsgIdHdr && (*szBuffer == ' ' || *szBuffer == '\t')) ||
				  (!strnicmp(szBuffer, strMsgIdHeader, strMsgIdHeader.GetLength()))))
		{
			//
			// Check to see if we have message ID info in this
			// "Message-Id:" header line.  If so, store it in
			// szMsgId[].
			//
			szMsgId[0] = 0;
			ExtractMessageID_(szBuffer, szMsgId);
			if (szMsgId[0] != 0)
				bFoundMsgId = TRUE;
			else
				bInMsgIdHdr = TRUE;
		}

		if (bFoundDate && bFoundMsgId)
		{
			break;
		}
	}

	//
	// Create our fake message ID string.
	//

	//Header class is creating a date field of max 31 chars, so we do same until we use a CONSTANT value properly named
	szDate[31] = 0; //rAAAACCCCCCCCKKKKKKKKKKKKKKK Alert
	CString strFakeMessageID;
	strFakeMessageID.Format("%s-%s", szMsgId, szDate);

	// 
	// Hash it and copy it to the caller's buffer.
	//
	ulHash = ::HashMT(strFakeMessageID + m_strPersona);
	strncpy(pszFakeMessageID, strFakeMessageID, LMOS_MSGID_SIZE - 1);
	pszFakeMessageID[LMOS_MSGID_SIZE - 1] = 0;

	//
	// Flush the rest of the TOP data...
	//
	while (*szBuffer && (this->ReadPOPLine(szBuffer, sizeof(szBuffer)) > 0))
	{
		// keep looping...
	}

	return ulHash;
}




////////////////////////////////////////////////////////////////////////
// ComputeMessageIdAndHash [private]
//
// Given a message number, retrieve the header of the message using the
// TOP command.  Search the header to 1) extract the date from the 
// Received: header and 2) extract the message ID from the Message-Id:
// header.  If both are found, then concatenate the values into a "fake"
// message ID (returned in 'pszFakeMessageID' and then return the hash 
// of the fake message ID.
//
// This routine is only used when the server doesn't support UIDL.
////////////////////////////////////////////////////////////////////////
unsigned long CPOPSession::ComputeMessageIdAndHash(const char* strFileName, char *pszFakeMessageID)
{
	// Put some garbage into the string
	strcpy(pszFakeMessageID, "I'll gladly pay you Tuesday for a hamburger today.");

	char szBuffer[512];
	JJFileMT InSpoolFile;
	InSpoolFile.Open(strFileName, _O_RDWR);
	if(InSpoolFile.IsOpen() != S_OK)
		return 0;

	//
	// Setup search variables...
	//
	BOOL bFoundDate = FALSE;			// TRUE if we extracted a date string
	BOOL bFoundMsgId = FALSE;			// TRUE if we extracted a Message-Id string
	BOOL bInReceivedHdr = FALSE;
	BOOL bInMsgIdHdr = FALSE;
	char szDate[500];
	char szMsgId[500];
	unsigned long ulHash = 0; 		// returned
	CRString strReceivedHeader(IDS_POP_RECEIVED_HEADER);
	CRString strMsgIdHeader(IDS_POP_MESSAGE_ID_HEADER);

	szDate[0] = 0;
	szMsgId[0] = 0;

	//
	// Process each line of TOP data, one at a time.  Stop when we hit 
	// an empty line.
	//
	while ((InSpoolFile.GetLine(szBuffer, sizeof(szBuffer)) == S_OK) && (*szBuffer != '\r'))
	{
		if (*szBuffer != ' ' && *szBuffer != '\t')
		{
			bInReceivedHdr = FALSE;
			bInMsgIdHdr = FALSE;
		}

		// 
		// If we are in a folded Received header and the first char is
		// a space or a tab, then we need to check for the date.  OR,
		// if this is a beginning of a Received header then we need to
		// check for the date also.
		//
		if (!bFoundDate &&
			((bInReceivedHdr && (*szBuffer == ' ' || *szBuffer == '\t')) ||
			 (!strnicmp(szBuffer, strReceivedHeader, strReceivedHeader.GetLength()))))
		{
			//
			// Check to see if we have Date info in this "Received:"
			// header line.  If so, store it in szDate[].
			//
			szDate[0] = 0;
			ExtractDateTime_(szBuffer, szDate);
			if (szDate[0] != 0)
				bFoundDate = TRUE;
			else
				bInReceivedHdr = TRUE;
		}
		else if (!bFoundMsgId && 
				 ((bInMsgIdHdr && (*szBuffer == ' ' || *szBuffer == '\t')) ||
				  (!strnicmp(szBuffer, strMsgIdHeader, strMsgIdHeader.GetLength()))))
		{
			//
			// Check to see if we have message ID info in this
			// "Message-Id:" header line.  If so, store it in
			// szMsgId[].
			//
			szMsgId[0] = 0;
			ExtractMessageID_(szBuffer, szMsgId);
			if (szMsgId[0] != 0)
				bFoundMsgId = TRUE;
			else
				bInMsgIdHdr = TRUE;
		}

		if (bFoundDate && bFoundMsgId)
		{
			break;
		}
	}

	//
	// Create our fake message ID string.
	//

	//Header class is creating a date field of max 31 chars, so we do same until we use a CONSTANT value properly named
	szDate[31] = 0; //HAAAACCCCCCCCKKKKKKKKKKKKKKK Alert
	CString strFakeMessageID;
	strFakeMessageID.Format("%s-%s", szMsgId, szDate);

	// 
	// Hash it and copy it to the caller's buffer.
	//
	ulHash = ::HashMT(strFakeMessageID + m_strPersona);
	strncpy(pszFakeMessageID, strFakeMessageID, LMOS_MSGID_SIZE - 1);
	pszFakeMessageID[LMOS_MSGID_SIZE - 1] = 0;

	return ulHash;
}


////////////////////////////////////////////////////////////////////////
// POPMsgHasBeenRead_ [private]
//
// Do a brute force check to see if the server-resident message has
// been read yet.  This means checking the status of the "Status:"
// header.  Really only need to do this on POP servers which don't
// support the LAST command.
//
// Returns TRUE if the message has already been fully read.  Returns
// FALSE if it hasn't seen the message.  Otherwise, returns -1 on
// error.
////////////////////////////////////////////////////////////////////////
int CPOPSession::POPMsgHasBeenRead_(int nMsgNum) 
{
	// An obvious check first
	if (nMsgNum < 1 || nMsgNum > m_nMessageCount)
	{
		ASSERT(0);
		return FALSE;
	}

	// 
	// Begin by sending the POP server a TOP command.
	// Ask for one line of the body because some POP servers choke on a
	// TOP command with zero for the number of body lines to return.
	//
	char szBuffer[128];
	{
		char szArgs[16];
		wsprintf(szArgs, "%d 1", nMsgNum);
		if (POPCmdGetReply(IDS_POP_TOP, szArgs, szBuffer, sizeof(szBuffer)) < 0)
			return -1;			// network read error
	}
	
	//
	// Read response to TOP command until we see an empty line or a
	// line containing a "Status:" header.
	// 
    int nAlreadyRead = FALSE;
	int nStatus = 0;
	CRString strStatusHeader(IDS_POP_STATUS_HEADER);
	while ((nStatus = ReadPOPLine(szBuffer, sizeof(szBuffer))) > 0 && *szBuffer != '\r')
	{ 
		if (!strnicmp(szBuffer, strStatusHeader, strStatusHeader.GetLength()))
		{
			//
			// Found the line with the "Status:" header.  Check to see
			// if we've already seen it.
			//
			if (strchr(szBuffer + strStatusHeader.GetLength(), 'R'))
			{
				nAlreadyRead = TRUE;
				break;
			}
		}
	}

	if (nStatus <= 0)
		nAlreadyRead = -1;		// network read error

	//
	// Flush the rest of the TOP data...
	//
	while (*szBuffer && (nStatus = this->ReadPOPLine(szBuffer, sizeof(szBuffer))) > 0) 
	{
		// keep looping...
	}

	if (nStatus < 0)
		return -1;				// network read error

	return nAlreadyRead;
}


////////////////////////////////////////////////////////////////////////
// GetPOPPassword_ [private]
//
// Grab saved password from INI file, if available.  Otherwise, prompt
// user for password.  If Eudora is configured to save passwords, go
// ahead and save to the INI file.
////////////////////////////////////////////////////////////////////////
int CPOPSession::GetPOPPassword_()
{
	ASSERT( ::IsMainThreadMT());

	bool SavePass = m_Settings->SavePass();
	if (::POPPassword.IsEmpty())
	{
		if(SavePass)
			::POPPassword = ::DecodePassword(m_Settings->GetSavePass());
			
		if(::POPPassword.IsEmpty())
		{
			if (! ::GetPassword(::POPPassword))
				return (-1);
			if (SavePass)
				SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, ::EncodePassword(::POPPassword));
		}
	}

	// Save or erase password
	const char* pszPasswordText = GetIniString(IDS_INI_SAVE_PASSWORD_TEXT);
	if (SavePass && !pszPasswordText)
		SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, ::EncodePassword(::POPPassword));
	else if (!SavePass && pszPasswordText)
		SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, NULL);
	
	//copy the global password into POPSession for thread-safe access
	m_strPassword = ::POPPassword;
	
	return (1);
}




////////////////////////////////////////////////////////////////////////
// ReconcileLists_ [private]
//
// Given the current LMOS data and new UIDL data from the server, walk
// through the LMOS data looking for things which are still on the
// server.  If we find that a message is still on the server, keep the
// LMOS entry.  Otherwise, purge the deleted message from the LMOS
// database.  When the reconciliation is complete, replace the old LMOS
// data with the newly reconciled LMOS data.
////////////////////////////////////////////////////////////////////////

BOOL CPOPSession::ReconcileLists_(CPopHost* pDiskHost, 	CPopHost* pServerHost)
{
	if (!pDiskHost || !pServerHost)
		return FALSE;

	return pDiskHost->ReconcileLists_(pDiskHost, pServerHost);

}




////////////////////////////////////////////////////////////////////////
// BuildNonUIDLMsgs_ [private]
//
////////////////////////////////////////////////////////////////////////
CPopHost* CPOPSession::BuildNonUIDLMsgs_(int nStart, int nFinish)
{
	if (nFinish <= 0)
		return NULL;
	
	CPopHost* pServerHost = DEBUG_NEW CPopHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount(), m_nLMOSBackupCount);
	pServerHost->SetCanUIDL(FALSE);

	//
	// Initialize the progress display.
	//
	m_pTaskInfo->SetMainText(CRString(IDS_POP_CHECK_NON_UIDL));

	//
	// Read & check until there are no more messages.
	//
	for (int j = nStart; j <= nFinish; j++)
	{
		//::Progress(j);

		char szBuffer[kLMOSBuffSize];
		memset(szBuffer, 0, sizeof(szBuffer));

		char szMessageID[kLMOSBuffSize];
		memset(szMessageID, 0, sizeof(szMessageID));

		unsigned long ulHash = MakeFakeMessageIdAndHash_(j, szMessageID);
		if (ulHash > 0)
		{
			//
			// Manufacture a string containing:
			//
			//    "MsgNum MessageID hashString\r\n"
			//
			sprintf(szBuffer,"%d %s %lu \r\n", j, szMessageID, ulHash);

			//
			// Create an LMOS message record using the manufactured string.
			//
			CMsgRecord* pMsgRecord = CMsgRecord::CreateMsgRecord(m_strPersona, kNonUIDL, szBuffer, strlen(szBuffer));
			if (pMsgRecord != NULL)
				pServerHost->InsertMsgRecord(pMsgRecord);
			else
				break;
		}
	}

	//::Progress(1, NULL, 1);
	return pServerHost;
}


////////////////////////////////////////////////////////////////////////
// GetHostFromUIDLServer_ [private]
//
// Talk to the POP server and build up a new CPopHost object with LMOS
// data from the UIDL list.
// Returns the newly built CPopHost object in the reference parameter,
// and the return value is whether or not a serious error occurred.
// It's not a serious error if the server doesn't support the UIDL
// command, but it is one if we get a network error, particularly if
// we get one in the middle of a UIDL list response.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::GetHostFromUIDLServer_(CPopHost*& pServerHost)
{
	pServerHost = NULL;

	// If no messages, then just return an empty list
	if (0 == m_nMessageCount)
	{
		pServerHost = DEBUG_NEW_NOTHROW CPopHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount(), m_nLMOSBackupCount);
		return TRUE;
	}

	//This is a hack to make Eudora act as if it is accessing Non-UIDL servers, to make debugging easy
	//Set DebugDisableUIDL to 1 to make Eudora forget about UIDLs
	if( g_bDisableUIDL == true)
		return TRUE;

	//
	// Ask the POP server for the UIDL...
	//
	char szBuffer[kLMOSBuffSize];
	
	if( POPCmdGetReply(IDS_POP_UIDL, NULL, szBuffer, sizeof(szBuffer), FALSE) < 0)
	{
		// If the server doesn't understand the UIDL command, that's fine and we can
		// continue on.  But if we get another error we need to stop.
		return (strnicmp(szBuffer, "-ERR", 4) == 0)? TRUE : FALSE;
	}

	if ('+' == szBuffer[0])
	{
		// 
		// Got OK to continue...
		//
		pServerHost = DEBUG_NEW CPopHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount(), m_nLMOSBackupCount);
		
		// 
		// Show progress...
		//
		m_pTaskInfo->SetMainText(CRString(IDS_POP_CHECK_UIDL));
		//::Progress(0, NULL, m_nMessageCount);

		//
		// Read until there are no more messages....
		//
		int nLength = 1;
		int ReceivedIDCount = 0;
		while (nLength > 0)
		{
			//
			// Read one line at a time
			//
			nLength = this->ReadPOPLine(szBuffer, sizeof(szBuffer));
			if (nLength > 0)
			{
				ReceivedIDCount++;

				//
				// Create an object from the supplied UIDL data and
				// add it to the host list.
				//
				CMsgRecord* pNewLMOSRecord = CMsgRecord::CreateMsgRecord(m_strPersona, kUIDL, szBuffer, sizeof(szBuffer));
				if (NULL == pNewLMOSRecord)
				{
					//
					// Allocation failed, so we're outta here...
					//
					delete pServerHost;
					pServerHost = NULL;
					break;
				}
				if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
				{
					char		 szBuf[256];
					char		 szUID[128];
					char		 szHash[16];
					pNewLMOSRecord->GetUIDLID(szUID, sizeof(szUID));
					pNewLMOSRecord->GetHashString(szHash);
					sprintf(szBuf, "UIDL %d %s %s", pNewLMOSRecord->GetMsgNum(), szUID, szHash);
					PutDebugLog(DEBUG_MASK_LMOS, szBuf);
				}
				pServerHost->InsertMsgRecord(pNewLMOSRecord);
			}
			else if (nLength < 0)
			{
				//
				// Read failed, so we're outta here...
				//
				delete pServerHost;
				pServerHost = NULL;
				break;
			}
		}

		if(pServerHost)
		{
			if (ReceivedIDCount == m_nMessageCount)
				pServerHost->SetCanUIDL(TRUE);
			else
			{
				// If the number of IDs returned from the UIDL command is not the same
				// as the number of messages returned from the STAT command, then
				// something went wrong and the list of IDs is most likely corrupt.
				ErrorDialog(IDS_ERR_POP_MISSING_IDS, ReceivedIDCount, m_nMessageCount);

				delete pServerHost;
				pServerHost = NULL;
			}
		}
	}

	return pServerHost? TRUE : FALSE;
}


////////////////////////////////////////////////////////////////////////
// PurgeMail_ [private]
//
// If we're leaving messages on the server, then do the message purging.
// This includes auto-deletion of expired messages, forcible removal
// of ALL messages, and removal of messages explicitly marked for deletion.
// Write the updated LMOS state to disk.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::PurgeMail_()
{
	//
	// Hide the progress bar
	//
	//Progress(-1, NULL, -1);

	char szBuf[256];
	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
	{
		sprintf(szBuf, "Begin purging messages for %s: %s", (LPCTSTR)m_strPersona, m_Settings->GetPOPAccount());
		PutDebugLog(DEBUG_MASK_LMOS, szBuf);
	}

	//
	// Perform auto-deletion of expired messages, purge all, and
	// deletion of explicitly marked messages.
	//
	if (m_pPopHost)
	{
		if( m_pPopHost->PurgeMessages(this, m_ulBitFlags, m_Settings->DoLMOS(), 
			m_Settings->DeleteMailAfterSomeDays(), m_Settings->GetDaysToLeaveOnServer())
		)
		{
				//m_Notifications |= CMF_DELETED_MAIL;  //m_bDeletedMail = TRUE;
				m_Notifier->AddNotifications( CMF_DELETED_MAIL );
		}
	}

	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_LMOS))
	{
		sprintf(szBuf, "Done purging messages for %s: %s", (LPCTSTR)m_strPersona, m_Settings->GetPOPAccount());
		PutDebugLog(DEBUG_MASK_LMOS, szBuf);
	}

	if (m_pPopHost)
		m_pPopHost->WriteLMOSData();

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// ClosePOPConnection_ [private]
//
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::ClosePOPConnection_()
{
	//
	// Put up the progress dialog
	//
	m_pTaskInfo->SetMainText(CRString(IDS_POP_CLEANING_UP));

	if (m_pNetConnection != NULL)
	{
		if (m_bPOPConnected)
		{
			if (GetAuthenticationType_() == AUTH_KERBEROS)
				m_pTaskInfo->SetMainText(CRString(IDS_KERB_SHUTDOWN));
			else
				m_pTaskInfo->SetMainText(CRString(IDS_POP_SHUTDOWN));

			//
			// Hide the progress bar
			//
			//::Progress(-1, NULL, -1);

			//
			// Send the POP server a QUIT command.
			//
			char szBuffer[QC_NETWORK_BUF_SIZE];
			if (this->POPCmdGetReply(IDS_POP_QUIT, NULL, szBuffer, sizeof(szBuffer)) >= 0)
			{
				//
				// If we get here, we succeeded with the QUIT and can 
				// assume that the delete's we requested are complete...
				//
				m_bQuitIssuedSuccessfully = true;
			}
		}

		m_pNetConnection->Close();
	}

	m_bPOPConnected = FALSE;

	return TRUE;
}

void CPOPSession::RemoveDeletedMsgRecords()
{
	m_pPopHost->RemoveDeletedMsgRecords();
}



////////////////////////////////////////////////////////////////////////
// SetupNetConnection_ [private]
//
// Creates a 'NetConnection' if necessary, returning TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::SetupNetConnection_()
{
	m_bCreatedNetConnection = FALSE;

	if (NULL == m_pNetConnection)
	{
		// Create a new connection...
		if (m_pNetConnection = CreateNetConnectionMT(m_Settings->GetNetSettings(), FALSE, FALSE))
		{
			ASSERT(m_pNetConnection);
			m_bCreatedNetConnection = TRUE;
			return TRUE;
		}
		else
		{
			ASSERT(NULL == m_pNetConnection);
			return FALSE;
		}
	}

	return TRUE;
}
 

////////////////////////////////////////////////////////////////////////
// TeardownNetConnection_ [private]
//
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::TeardownNetConnection_()
{
	CSingleLock lock(&m_Guard, TRUE);
	if (m_pNetConnection)
	{
		if (m_bCreatedNetConnection)
		{
			delete m_pNetConnection;
			m_pNetConnection = NULL;
		}
		if (m_pNetConnection)
			m_pNetConnection->Close();
	}

	return TRUE;
}





////////////////////////////////////////////////////////////////////////
// DoGetPassword_ [private]
//
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::DoGetPassword_()
{
	QCNetworkSettings *pNetSettings = m_Settings->GetNetSettings();
	if( !pNetSettings->GetRasPassword())
		return FALSE;

	
	// In general we want to do any processing that DOES NOT require
	// a network connection here...
	switch (GetAuthenticationType_())
	{
	case AUTH_KERBEROS:
	case AUTH_RPA:
	case AUTH_KERBEROS_5:
		break;
	case AUTH_APOP:
	case AUTH_POP:
		if (GetPOPPassword_() < 0)
			return FALSE;
		break;
	default:
		ASSERT(0);
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// DoLogon_ [private]
//
// Call POPAuthenticate() to do two things:  1) authenticate to the
// POP server, and 2) return the number of available messages.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::DoLogon_()
{
	m_pTaskInfo->SetMainText(CRString(IDS_POP_LOGGING_INTO_POP));

	// save the number of messages waiting
	m_nMessageCount = POPAuthenticate_();
	if (m_nMessageCount < 0)
	{
		//
		// We never got connected so return FALSE to abort the
		// state machine
		//
		return FALSE;
	}

	return TRUE;
}





////////////////////////////////////////////////////////////////////////
// KerberosAuthenticate_ [private]
//
// Provides the mechanics for setting up and using the Kerberos
// authentication protocols...
////////////////////////////////////////////////////////////////////////
// Stanfords Kerberos crashes without optimizations ... "Sylvan W. Clebsch" <sylvan@stanford.edu>
#pragma optimize ("y", off)
CPOPSession::KerberosErrType CPOPSession::KerberosAuthenticate_()
{
	CSingleLock lock( &m_KerberosAuthGuard, TRUE);


	KerberosErrType	errKerb				= KERBERR_OK;		// innocent until proven guilty
	BOOL			bSuccess			= FALSE;
	DWORD			dwTickenLen; 	//	= QCKerberos::kTicketBufferSize;
	char			szFullPOPHost[133];	// fully-qualified domain name (e.g., "adept.qualcomm.com")
	char			szPOPHost[133];		// just the machine name (e.g., "adept")
	CString			strService;
	char			szServiceName[133]; 
	char			szKerberosRealm[133]; 
	char			szDisplayString[256];
	char			buf[128];
	memset(buf,0, 128);

	if (NULL == m_pNetConnection)
	{
		//
		// We assume we have a net connection at this point.
		//
		ASSERT(0);
		return KERBERR_INTERNAL;
	}

	
	//
	// Get the POP server name here.
	// Might be a CNAME, we want the real host name here.
	// Kerberos requires lowercase host names.
	//
	{
		*szFullPOPHost = *szPOPHost = 0;
		strncpy(szFullPOPHost, m_pNetConnection->GetMachineName(), sizeof(szFullPOPHost));
		strlwr(szFullPOPHost);
		strncpy(szPOPHost, szFullPOPHost, sizeof(szPOPHost));

		// Kerberos IV code, FQDNs are not allowed.
		char* pszDot = strchr(szPOPHost, '.');
		if (pszDot)
		{
			*pszDot = '\0';
		}

		// These get set up in SetupKerberosPOPHost which gets called before the pop connection
		if (!*szFullPOPHost || !*szPOPHost)
		{
			return KERBERR_INTERNAL;
		}
	}

	//
	// Load up the KCLIENT.DLL
	//
	if (QCKerberos::LoadKerberosLibrary(makeCallback( (Callback1<const char *> *)0, *this, &CPOPSession::AuthError)) < 0)
	{
		return KERBERR_LOAD_LIB_FAILED;
	}
	
	// Lets turn on debugging
	//SetKrbdllMode(FALSE);
	
	//
	// Set global POPPassword CString to contain bogus Kerberos password.
	//
	m_strPassword = GetIniString(IDS_KERB_FAKE_PASSWORD);

	if (!QCKerberos::s_bKerbTicket && GetIniShort(IDS_INI_KERBEROS_SET_USERNAME))
	{
		//
		// Setup user name, based on POP account setting.  Convert POP
		// account string of the form "fred@pop.bogus.com" to just
		// "fred", then attempt to register ourselves as this user.
		//
		char szPOPUserName[133];
		GetIniString(IDS_INI_POP_ACCOUNT, szPOPUserName, sizeof(szPOPUserName));	// eg, "fred@pop3.bogus.com"

		char *pszAtSign = strrchr(szPOPUserName, '@');
		if (pszAtSign)
		{
			*pszAtSign = '\0';
			
			if (!QCKerberos::SetUserName(szPOPUserName))
				errKerb = KERBERR_REG_FAILED;
						
		}
		else
			errKerb = KERBERR_NO_NAME;
		
	}

	if (KERBERR_OK == errKerb)
	{
		char szCommandString[256]; 
		//
		// Format service string and update progress display.
		//
		{
			GetIniString(IDS_INI_KERB_SERVICE, szServiceName, sizeof(szServiceName));
			GetIniString(IDS_INI_KERB_REALM, szKerberosRealm, sizeof(szKerberosRealm));
			GetIniString(IDS_KERB_FILLER_STRING, QCKerberos::s_szKerbTicketBuffer, sizeof(QCKerberos::s_szKerbTicketBuffer));
			//QCKerberos::LoadTicket();
			
			LPCSTR pszArray[4];
			pszArray[0] = szServiceName;
			pszArray[1] = szFullPOPHost;
			pszArray[2] = szKerberosRealm;
			pszArray[3] = szPOPHost;
		
			const char* pszServiceFormat = GetIniString(IDS_INI_KERB_FORMAT);
			AfxFormatStrings(strService, pszServiceFormat, pszArray, 4);
			strcpy(szCommandString, strService);

			wsprintf(szDisplayString, (const char *)CRString(IDS_KERB_GETTING_TICKET), szCommandString);
			m_pTaskInfo->Progress(szDisplayString);
		}

		BOOL bSentTicket = FALSE;
		
		//Hack for making a socket blocking bcos kclient32.dll cant handle async sockets
		QCWorkerSocket *pWorkerSock = (QCWorkerSocket *)m_pNetConnection;
		
		//set it to blocking
		pWorkerSock->SwitchToBlockingMode();

		bSuccess = QCKerberos::SendTicket(szCommandString, m_pNetConnection->GetSocket(), &bSentTicket);

		//back to the old ways.. to async mode
		pWorkerSock->SwitchToAsyncMode();
		

		if (bSuccess)
		{
			if (bSentTicket)
			{
				// We've successfully sent the ticket off the Platinum way
				errKerb = KERBERR_OK;
			}
			else 
			{
				// We've got the ticket, now we got to send it to the server
				dwTickenLen = QCKerberos::GetTicketLength() + 16;
				const char *szTicketBuf = QCKerberos::GetTicketBuffer();
				
				int nStatus = m_pNetConnection->PutDirect(szTicketBuf, int(dwTickenLen));

				if (nStatus < kNoErr)
				{
					errKerb = KERBERR_PUT_FAILED;
				}
			}
		}
		else
		{
			// Something went wrong, we don't have a ticket
			errKerb = KERBERR_CANCELED;
		}
	}

	// Turn off debugging.
	//SetKrbdllMode(FALSE);
	
	QCKerberos::FreeLibrary();

	return errKerb;
}

bool CPOPSession::SSLStart()
{
	char ResponseBuffer[QC_NETWORK_BUF_SIZE];
	if (POPCmdGetReply(IDS_POP_STLS, NULL, ResponseBuffer, sizeof(ResponseBuffer), FALSE) < 0)
	{
		// If STLS fails (which can happen when having NAV scanning email) do not consider
		// SSL to be supported.
		m_bSupportsSSL = false;
		return false;
	}
	if (ResponseBuffer[0] == '+')
	{ 
		//Yes!! Server says go ahead with SSL.
		m_pNetConnection->SetSSLMode(true,m_strPersona, &m_Settings->m_SSLSettings , "POP");
		return true;
	}
	return false;

}

