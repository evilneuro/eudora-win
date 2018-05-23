////////////////////////////////////////////////////////////////////////
// File: POP.CPP
//
//
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "afxmt.h"

#include <ctype.h>

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

//	Defines and other Constants	=============================
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern bool g_bDisableUIDL;

//	General errors returned by these routines
const	int	kNoErr					= 0;	// No error

void RecoverReceivedMail(void *pv, bool)
{

	char *szPersona = static_cast<char *>(pv);
	
	//Get the window with focus and save it
	HWND hFocus = ::GetFocus();
	

	CPOP MailMan(szPersona);
	bool bDone = MailMan.GetMailFromSpool();


	//If window is still around give the focus back
	if(hFocus)
		::SetFocus(hFocus);

};


void ProcessIncomingMail(void *pv, bool bEnteringLastTime)
{
	/*
	//Get the window with focus and save it
	HWND hFocus = ::GetFocus();

	CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	BOOL bIsMaximized = FALSE;
	CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);
	*/
	
	//Start here
	CPOPSession *popThread = (CPOPSession *)pv;
	
	CPOP MailMan(popThread->GetSettings(), popThread->GetNotifier());
	bool bDone = MailMan.GetMailFromSpool();

	if(!bDone) //stopped by user
	{
		CTaskInfoMT *pTaskInfo = popThread->GetTaskInfo();
		pTaskInfo->SetPostProcessing(true);
		pTaskInfo->IgnoreIdle(false);
	}

	/*
	if(bEnteringLastTime)
	{
		//Clear the password if invalid password
		if(popThread->NeedsCleanup())
			popThread->Cleanup();
	
		//we successfully issued QUIT means safely remove the messages marked as DELETE_SENT
		if(popThread->IsQuitSuccessful())
			popThread->RemoveDeletedMsgRecords();

		MailMan.DoNotifyNewMail();

		//The notifier is decremented; if we are the last one, play a sound etc if new mail
		popThread->Notify();
	}
	*/
	
	/*if(pActiveMDIChild) 
		pActiveMDIChild->MDIActivate();

	//If window is still around give the focus back
	if(hFocus)
		::SetFocus(hFocus);
		*/
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
	

		

CPOPRecoverer::CPOPRecoverer(CString strPersona)
{
	m_szPersona = new char[strPersona.GetLength()+1];
	strcpy(m_szPersona, strPersona);
	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	m_pTaskInfo->SetPersona(m_szPersona);
	m_pTaskInfo->SetTitle("Recovering mail received during previous session");
	m_pTaskInfo->IgnoreIdle();
	
}


HRESULT CPOPRecoverer::DoWork()
{

	//Set the post proceesing fn, this will be called in OnIdle loop or OnDemand from TaskManager
	m_pTaskInfo->SetPostProcFn("Filter recoverd mail", RecoverReceivedMail, m_szPersona);
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
	m_bCanMangle(false)
{
	m_ulBitFlags = m_Settings->GetBitFlags();
	m_strPersona = m_Settings->GetPersona();
	
	QCNetworkSettings *pNetSettings = m_Settings->GetNetSettings();

	//Set the error callback function for Network to report error back to us
	pNetSettings->SetReportErrorCallback(  
		makeCallback( (QCNetworkSettings::ErrorCallback *)0, *m_pTaskInfo, &CTaskInfoMT::CreateError) );

	pNetSettings->SetProgressCallback(  
		makeCallback( (Callback1<const char *> *)0, *m_pTaskInfo, &CTaskInfoMT::SetMainText) );

	m_pTaskInfo->SetTitle(CRString(IDS_POP_LOOKING_FOR_MAIL));
	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	m_pTaskInfo->SetPersona(m_Settings->GetPersona());
	
	if(pNetSettings->IsDialupConnection())
	{
		m_pTaskInfo->SetDialupEntryName( pNetSettings->GetRASEntryName() );
		m_pTaskInfo->SetScheduleTypes(TT_USES_DIALUP);
	}

	m_pTaskInfo->SetScheduleTypes(TT_USES_POP);
	m_pTaskInfo->SetPostState(TSS_POST_NONE);
}

CPOPSession::~CPOPSession()
{
	//Clear the password if invalid password
	if(NeedsCleanup())
		Cleanup();
	
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
}


void CPOPSession::Notify()
{
	if(m_Notifier)
		m_Notifier->Decrement();
}



void CPOPSession::Cleanup()
{
	CString strHomie = g_Personalities.GetCurrent();

	g_Personalities.SetCurrent(m_strPersona);
	::ClearPassword();
	g_Personalities.SetCurrent(strHomie);

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
	POPVALUES *pv = new POPVALUES;
	pv->bGotNewMail = m_bGotNewMail;
	//pv->noteFlags = m_Notifications;
	pv->settings = m_Settings;
	//pv->bOpenInMailBox = m_Settings->OpenInMailbox();
	//pv->persona = m_strPersona;
	//pv->ulBitFlags = m_ulBitFlags;
	return pv;
}


//When the POP thread is queued, Task manager calls this function and asks it to do its stuff
HRESULT CPOPSession::DoWork()
{ 
	m_bTaskStarted = true;

	//Set the post proceesing fn, this will be called in OnIdle loop or OnDemand from TaskManager
	m_pTaskInfo->SetPostProcFn("Filter the new messages", ProcessIncomingMail, this);
	

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
	m_pPopHost = CHostList::GetHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount());
	
	BOOL bSendMailAfter = m_strPassword.IsEmpty();

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
	if (m_Settings->NeedPOP())
		bResult = DoLogon_();

	if(!bResult) goto CLOSE_POP;
	
	// What can this POP server do?
	GetCapabilities();

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


	
	
	//FOLLOWING will be done in MAINTHREAD
/*
	//Moved down so that we have a clean break from the rest
	if(m_bGotNewMail)
		bResult = DoGetPOPMailFromSpool();
	
	// For each downloaded message (if any), run filtering on
	// it.  Side effects include updating the count of
	// downloaded messages (m_nDownloadCount), updating the
	// count of messages sent to the trash during filtering
	// (m_nTrashedMessageCount), and updating the flag which
	// tells us that new message has been downloaded
	// (m_bNotifyNewMail).
	//
	
	if (bFetching && !(ulBitFlags & kFetchHeadersBit))
		bResult = DoFilterMail_();
	

	//case STATE_NOTIFY:
	//
	// Open the In mailbox, depending on availability of
	// newly downloaded mail.  Also, decide whether or not to
	// display New Mail alert, No New Mail alter, and play new
	// mail sound.
	//
	if (bFetching)
	{
#ifndef COMMERICAL
		// Freeware bug fix: call CloseProgress before
		// DoNotifyNewMail().  Causes CloseProgress to be
		// called again below, but oh well.
		::CloseProgress();
#endif
		bResult = DoNotifyNewMail_(bResult, ulBitFlags);
	}
	

DONE:

	::CloseProgress();

	// Turn redraw back on the In mailbox as it will have been turned off
	// if new messages have come in
	if (pInTocView)
		pInTocView->m_SumListBox.SetRedraw(TRUE);
*/

}


////////////////////////////////////////////////////////////////////////
// GetCapabilities
//
// Send the "CAPA" command to the POP server to see what kind of extra
// things this POP server can do.
////////////////////////////////////////////////////////////////////////
void CPOPSession::GetCapabilities()
{
	if (m_Settings->UsePOPCAPA() == false)
		return;

	// Right now the only capability we use is Mangle, so if we're not
	// doing Mangle then we shouldn't send the CAPA command at all.
	if (m_Settings->UsePOPMangle() == false)
		return;

	char ResponseBuffer[QC_NETWORK_BUF_SIZE];
	if (POPCmdGetReply(IDS_POP_CAPA, NULL, ResponseBuffer, sizeof(ResponseBuffer), FALSE) < 0)
		return;

	char szBuffer[QC_NETWORK_BUF_SIZE];
	while (ReadPOPLine(szBuffer, sizeof(szBuffer)) > 0)
	{
		if (stricmp(szBuffer, "MANGLE") == 0 ||
			stricmp(szBuffer, "X-MANGLE") == 0)
		{
			m_bCanMangle = true;
		}
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
	//::MainProgress(CRString(IDS_POP_LOOKING_FOR_MAIL));
	m_pTaskInfo->SetMainText(CRString(IDS_POP_LOOKING_FOR_MAIL));
	
	//Handle UIDL capable server
	//ReconcileLists_(CHostList::GetHostForCurrentPersona(), GetHostFromUIDLServer_());
	ReconcileLists_(m_pPopHost, GetHostFromUIDLServer_());
	
	//m_pPopHost = CHostList::GetHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount());
	
	
	//Do we need to move it furthur down???????????????????
	//
	// Write the new LMOS data to disk.
	//
	//CHostList::WriteLMOSFile();

	//Get the reconciled host (w/ UIDL server)
	//CPopHost* pDiskHost = CHostList::GetHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount());

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
			CPopHost* pServerHost = BuildNonUIDLMsgs_(1, m_nMessageCount);
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


/*

////////////////////////////////////////////////////////////////////////
// DoGetPOPMail_ [private]
//
// Overloaded method to do the actual message fetches.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::DoGetPOPMail_()
{
	//
	// All we need do is get the current host...this call will
	// make the new host list if needed...
	//
	CPopHost* pPopHost = CHostList::GetHostForCurrentPersona();
	if (NULL == pPopHost)
		return FALSE;
		
    // This needs to be conditioned...just in case we exit next...!!!
	m_bGotNewMail = FALSE;
	
	// 
	// Count the total number of messages to download.
	//
	int nTotalMessages = 0;
	int nMsgNumber = pPopHost->GetNextFetchableMsgNum(m_ulBitFlags, TRUE);
	while (nMsgNumber > 0)
	{
		nTotalMessages++;
		nMsgNumber = pPopHost->GetNextFetchableMsgNum(m_ulBitFlags);
	}

	if (0 == nTotalMessages)
		return TRUE;
		
	CTocDoc* pInTOC = ::GetInToc();
	if (NULL == pInTOC)
		return FALSE;

	//
	// FORNOW, this is really ugly.  We're setting a global pointer to
	// point to a stack-based object!!!!!  UGLY, UGLY, UGLY, UGLY, UGLY,
	// UGLY....  WKS 97.06.27.
	//
	JJFile MBox;
	g_pPOPInboxFile = &MBox;
	if (FAILED(g_pPOPInboxFile->Open(pInTOC->MBFilename(), O_RDWR | O_APPEND | O_CREAT)))
	{
		g_pPOPInboxFile = NULL;
		return FALSE;
	}

	//
	// Keep track of the last message in the In mailbox so that we can
	// count how many new messages have arrived during this mail check.
	//
	m_LastMessagePos = pInTOC->m_Sums.GetTailPosition();
	
	// Let's set up the progess window string
	::CountdownProgress(CRString(IDS_POP_MESSAGES_LEFT), nTotalMessages);
	        	
	// Lets get the first message to grab
	nMsgNumber = pPopHost->GetNextFetchableMsgNum(m_ulBitFlags, TRUE);
	
	m_bGotNewMail = FALSE;

	// Now while the nMsgNumber is not zero we get messages...
    BOOL bReturn = TRUE;
	for (; nMsgNumber > 0; nMsgNumber = pPopHost->GetNextFetchableMsgNum(m_ulBitFlags), ::DecrementCountdownProgress())
	{
		//
		// Each time thru we need to get the last summary position so we
		// we can update the HASH values for all parts of this message later....
		//
		POSITION posLast = pInTOC->m_Sums.GetTailPosition();

		// 
		// Lets get the message...  A status greater than zero indicates
		// a successful download, a status of zero indicates no message
		// was downloaded, and a status less than zero indicates an error.
		//

		//CMsgRecord* pRec = pPopHost->GetMsgByMsgNum(nMsgNumber); //$$$$$$$$$$ DANGER REMOVE
			
		//int nStatus = POPGetMessage_(pInTOC, nMsgNumber);
		int nStatus = POPGetMessageToSpool(nMsgNumber);
		if (nStatus >= 0)
		{
			CMsgRecord* pMsgRecord = pPopHost->GetMsgByMsgNum(nMsgNumber);
			if (pMsgRecord != NULL)
			{
				// Set the Retrieval flag to don't retrieve (Nget)
				pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE);
							
				// That's it if we didn't download anything
				if (nStatus == 0)
					continue;
							
				// We need to condition the mail flag for later saves....
				m_bGotNewMail = TRUE;

				// Set the has been read flag...
				pMsgRecord->SetReadFlag(LMOS_HASBEEN_READ);

				// Was this message skipped due to size ?
				if (g_lEstimatedMsgSize < 0)
					pMsgRecord->SetSkippedFlag(LMOS_SKIP_MESSAGE);
                else
                	pMsgRecord->SetSkippedFlag(LMOS_DONOT_SKIP);

				//
				// Lets get the last summary prior to our get message...
				// this is the last position BEFORE the series of gets..
				//
				CSummary* pLastSummary = NULL;
				if (posLast == NULL)
					posLast = pInTOC->m_Sums.GetHeadPosition();
				else
					pLastSummary = pInTOC->m_Sums.GetNext(posLast);

				//
				// Now posLast points to the first summary object
				// added for this message....
				// For each new message added to the TOC...
				//
				while (posLast)
				{
					//
					// we need to get each message summary added to
					// the toc for this get message pass..
					// remember posLast will point to the NEXT object
					// position....
					//
					pLastSummary = pInTOC->m_Sums.GetNext(posLast);
					if (pLastSummary)
						pLastSummary->SetHash(pMsgRecord->GetHashValue());
				}
			}
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
	}

	//
	// Close the mailbox file... 
	//
	g_pPOPInboxFile->Close();

	if (m_bGotNewMail == TRUE)
	{
		if (pInTOC->IsModified())
			pInTOC->Write();
	}       

	g_pPOPInboxFile = NULL;
	return bReturn;
}
*/



////////////////////////////////////////////////////////////////////////
// DoGetPOPMail_ [private]
//
// Overloaded method to do the actual message fetches.
////////////////////////////////////////////////////////////////////////
BOOL CPOPSession::DoGetPOPMailToSpool()
{
		
    // This needs to be conditioned...just in case we exit next...!!!
	m_bGotNewMail = FALSE;
	
	// 
	// Count the total number of messages to download.
	//
	/*int nTotalMessages = 0;
	int nMsgNumber = m_pPopHost->GetNextFetchableMsgNum(m_ulBitFlags, TRUE);
	while (nMsgNumber > 0)
	{
		nTotalMessages++;
		nMsgNumber = m_pPopHost->GetNextFetchableMsgNum(m_ulBitFlags);
	}

	if (0 == nTotalMessages)
		return TRUE;*/

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


	//TRACE("CHECK__MAIL: Retrieving %d messages\n", nTotalMessages);
	/*CTocDoc* pInTOC = ::GetInToc();
	if (NULL == pInTOC)
		return FALSE;

	//
	// FORNOW, this is really ugly.  We're setting a global pointer to
	// point to a stack-based object!!!!!  UGLY, UGLY, UGLY, UGLY, UGLY,
	// UGLY....  WKS 97.06.27.
	//
	JJFile MBox;
	g_pPOPInboxFile = &MBox;
	if (FAILED(g_pPOPInboxFile->Open(pInTOC->MBFilename(), O_RDWR | O_APPEND | O_CREAT)))
	{
		g_pPOPInboxFile = NULL;
		return FALSE;
	}

	//
	// Keep track of the last message in the In mailbox so that we can
	// count how many new messages have arrived during this mail check.
	//
	m_LastMessagePos = pInTOC->m_Sums.GetTailPosition();
	
	*/

	// Let's set up the progess window string
	//::CountdownProgress(CRString(IDS_POP_MESSAGES_LEFT), nTotalMessages);
	        	
	// Lets get the first message to grab
	//nMsgNumber = m_pPopHost->GetNextFetchableMsgNum(m_ulBitFlags, TRUE);
	
	m_bGotNewMail = FALSE;
	CString strMainText;
	
	long lEstimatesMsgSize = 0;

	strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nTotalMessages);
	m_pTaskInfo->SetMainText(strMainText);

	m_pTaskInfo->SetTotal(nTotalMessages*100);

	// Now while the nMsgNumber is not zero we get messages...
    BOOL bReturn = TRUE;
	
	unsigned int nMsgsDownloaded = 0;
	//for (; nMsgNumber > 0; nMsgNumber = m_pPopHost->GetNextFetchableMsgNum(m_ulBitFlags) /*,::DecrementCountdownProgress()*/)
	for(MI mi = tMap.begin(); mi != tMap.end(); ++mi)
	{
		lEstimatesMsgSize = 0;
		//pMsgRecord = m_pPopHost->GetMsgByMsgNum(nMsgNumber);
		pMsgRecord = (*mi).second;
		nMsgNum = (*mi).first;

		ASSERT( nMsgNum == pMsgRecord->GetMsgNum());
		//TRACE("Fetching Msg # %d Hash %ld\n", nMsgNum, pMsgRecord->GetHashValue());
		
		
		//
		// Each time thru we need to get the last summary position so we
		// we can update the HASH values for all parts of this message later....
		//
		//POSITION posLast = pInTOC->m_Sums.GetTailPosition();

		// 
		// Lets get the message...  A status greater than zero indicates
		// a successful download, a status of zero indicates no message
		// was downloaded, and a status less than zero indicates an error.
				
		//int nStatus = POPGetMessage_(pInTOC, nMsgNumber);
		
		
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

						
			/*
			pMsgRecord->SetEstimatedSize(lEstimatesMsgSize);
			// Was this message skipped due to size ?
			if(lEstimatesMsgSize < 0) //if (g_lEstimatedMsgSize < 0)
				pMsgRecord->SetSkippedFlag(LMOS_SKIP_MESSAGE);
            else
                pMsgRecord->SetSkippedFlag(LMOS_DONOT_SKIP);

			*/
				//Insert message into a Thread-Safe List for furthur processing
				/*
				//
				// Lets get the last summary prior to our get message...
				// this is the last position BEFORE the series of gets..
				//
				CSummary* pLastSummary = NULL;
				if (posLast == NULL)
					posLast = pInTOC->m_Sums.GetHeadPosition();
				else
					pLastSummary = pInTOC->m_Sums.GetNext(posLast);

				//
				// Now posLast points to the first summary object
				// added for this message....
				// For each new message added to the TOC...
				//
				while (posLast)
				{
					//
					// we need to get each message summary added to
					// the toc for this get message pass..
					// remember posLast will point to the NEXT object
					// position....
					//
					pLastSummary = pInTOC->m_Sums.GetNext(posLast);
					if (pLastSummary)
						pLastSummary->SetHash(pMsgRecord->GetHashValue());
				}
				*/
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
		//m_pTaskInfo->ProgressAdd(100);  //one unit is 100 sub-units
	}

	/*
	//
	// Close the mailbox file... 
	//
	g_pPOPInboxFile->Close();

	if (m_bGotNewMail == TRUE)
	{
		if (pInTOC->IsModified())
			pInTOC->Write();
	}       

	g_pPOPInboxFile = NULL;
	*/

	//write out any LMOS flags that got changed.
	if(m_bGotNewMail)
		m_pPopHost->WriteLMOSData();


	return bReturn;
}


int CPOPSession::POPGetMessageToSpool(CMsgRecord *pMsgRecord)
{
	
	//g_bAttachmentDecodeEnabled = TRUE;

	int nMessageNum = pMsgRecord->GetMsgNum();
	long lEstimatedMsgSize = 0;
	char szMessageNum[10];		// Message number
	wsprintf(szMessageNum, "%d", nMessageNum);

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
		//sscanf(szBuffer, "%s %d %ld", szDummy, &nDummy, &g_lEstimatedMsgSize);
		sscanf(szBuffer, "%s %d %ld", szDummy, &nDummy, &lEstimatedMsgSize);
	}
    
	//Set the estimated size for this message
	pMsgRecord->SetEstimatedSize(lEstimatedMsgSize);

	//
	// At this point we either read all, or top or nothing.  Have to
	// call GetMsgFetchType() here since it depends on 'g_lEstimatedMsgSize' being set.
	//
	int nLinesToRead = 20;
	int nStatus = 0;
	//switch (pMsgRecord->GetMsgFetchType(m_ulBitFlags))
	switch (GetMsgFetchType(pMsgRecord))
	{
	case MSGFETCH_ENTIRE:
		//
		// Set up the progress window then send the POP server a 
		// RETR command...
		//
		//Progress(0, NULL, lEstimatedMsgSize);

		nStatus = this->POPCmdGetReply(IDS_POP_RETRIEVE, szMessageNum, szBuffer, sizeof(szBuffer));
		if(nStatus >= 0)
		{
			//setting this flag make sures that the message is not on the server anymore
			//if previously its in skipped state
			pMsgRecord->SetSkippedFlag(LMOS_DONOT_SKIP);
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
			//Progress(0, NULL, nLinesToRead * 80);	// Estimate of size
			lEstimatedMsgSize = nLinesToRead * 80; // Estimate of size
			
			char ArgBuffer[128];
			if (m_bCanMangle)
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

				//lEstimatedMsgSize = -lEstimatedMsgSize;
				//g_lEstimatedMsgSize = -g_lEstimatedMsgSize;
				//g_bAttachmentDecodeEnabled = FALSE;
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

		if ((nStatus = FetchMessageToSpool(nMessageNum, lEstimatedMsgSize)) < 0)
			return -1;


		/*
		if (g_lBadCoding)
		{
			switch (AlertDialog(IDD_BAD_DECODE))
			{
			case IDC_DOWNLOAD_AS_TEXT:
				g_bAttachmentDecodeEnabled = FALSE;
				goto refetch;
				break;
			case IDC_GET_RID_OF_IT:
				bDeleteMessage = TRUE;
				pInToc->SetModifiedFlag();
				break;
			case IDOK:
				bDeleteMessage = FALSE;
				pInToc->SetModifiedFlag();
				break;
			}
		}
		else
		*/
		{
			if (m_ulBitFlags & kDeleteAllBit)
				bDeleteMessage = TRUE;
			else if (m_ulBitFlags & kLMOSBit)
				bDeleteMessage = !m_Settings->DoLMOS(); //!GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER);
			else
				bDeleteMessage = FALSE;
		}
		
		// 
		// If we're not doing LMOS, we have a message to be deleted
		// from the server with the DELE command.
		//
		//if (bDeleteMessage && lEstimatedMsgSize > 0)
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
int CPOPSession::FetchMessageToSpool(int nMessageNum, long lEstimatedSize)
{
	
	//
	// Call WriteMessageToMBX() to download header to IN.MBX,
	// instantiate the proper MIME reader, call the MIME reader to
	// download the message body, decode and burst attachments as
	// needed.
	//
	//HeaderDesc hd(g_lEstimatedMsgSize);
	//int nStatus = WriteMessageToMBX_(hd);			// 1 on success, -1 on error

	CMsgRecord *pMsgRecord = m_pPopHost->GetMsgByMsgNum(nMessageNum);

	if(!pMsgRecord)
		return -1;

	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(m_strPersona);
	CString szFilename;
	
	
	char szHashString[120];
	CString strFileName;
	strFileName = SpoolMgr->m_In->GetBasePath();
	pMsgRecord->GetHashString(szHashString);
	strcat(szHashString, ".RCV");
	strFileName += szHashString;
	
	SpoolMgr->m_In->BeginWrite(strFileName);
	
	
	//JJFile spFile;
	JJFileMT spFile;
	//long NumRead;
	
	spFile.Open(strFileName, O_RDWR | O_CREAT );
	
	char szBuffer[QC_NETWORK_BUF_SIZE];
	int nStatus = 1;

	long nSoFar = m_pTaskInfo->GetSoFar();
	long nDoneSoFar = 0;

	
	while ( (nStatus = this->ReadPOPLine(szBuffer, sizeof(szBuffer))) > 0 )
	{
		// keep looping...
		spFile.Put(szBuffer, strlen(szBuffer));
		nDoneSoFar += nStatus;
		if( nDoneSoFar <= lEstimatedSize)
			m_pTaskInfo->SetSoFar( (int)(nSoFar + ((double)nDoneSoFar/lEstimatedSize)*100));
	}
	spFile.Close();

	
	//ASSERT(1 == nStatus || -1 == nStatus);
	//ASSERT( nStatus >= 0);
	
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
			
			/*
			wsprintf(szMessageId, "%s-%s", hd.messageID, hd.receivedDate);
			ASSERT(strlen(szMessageId) < sizeof(szMessageId));		// better late than never
			for (char* pszMessageId = szMessageId; *pszMessageId; pszMessageId++)
			{
				if (*pszMessageId == ' ')
					*pszMessageId = '-';
			}
			*/
			
			if (! m_pPopHost->CanUIDL())
			{
			
			//The computed hash is only used for Non-UIDL servers
			unsigned long hash = ComputeMessageIdAndHash(strFileName, szMessageId);

			//
			// Set the message ID in the corresponding LMOS record.
			//
			//CMsgRecord* pMsgRecord = pPopHost->GetMsgByMsgNum(nMessageNum);
			pMsgRecord->SetMesgID(szMessageId);
			//if (! m_pPopHost->CanUIDL())
			//{
				// 
				// Fill in the non-UIDL id with header data we've
				// sucked from the message.
				//

				//Update the hash and map index also, as it is index by the hash value
				m_pPopHost->UpdateHashValue(pMsgRecord, hash);
				
				
				//For Non-UIDL servers, compute the new MessageID and hash
				//If hash differs from old(ie we padded the Non-UIDL server list)
				//change the hash in LMOS and also change the Spool File name to 
				//reflect the right hash name

				/*
				CString newFileName;
				newFileName.Format("%s%ld.RCV", SpoolMgr->m_Out->GetBasePath(), hash) ;
				if(newFileName.CompareNoCase(strFileName) != 0)
				{
					::FileRenameMT(strFileName, newFileName);
				}
				*/
			}
		}
		else
		{
			//
			// This should never happen, right?
			//
			ASSERT(0);
		}
		nStatus = 1;
		SpoolMgr->m_In->EndWrite(strFileName, pMsgRecord->GetHashValue(), pMsgRecord);
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
	DWORD dwTotalBytes = ::GetAvailableSpaceMT((const char *) ::EudoraDir);
	
	//char szAttachDir[_MAX_PATH + 1];
	//::GetIniString(IDS_INI_AUTO_RECEIVE_DIR, szAttachDir, sizeof(szAttachDir));
	//if (*szAttachDir && strnicmp(EudoraDir, szAttachDir, 1 ) != 0)
	CString strAttachDir = m_Settings->GetAttachDir();
	if(strAttachDir != "" && strnicmp((const char *)EudoraDir, (const char *)strAttachDir, 1 ) != 0)
	{
		DWORD dwBytes = ::GetAvailableSpaceMT((const char *) strAttachDir);
		dwTotalBytes = min(dwTotalBytes, dwBytes);
	}

	if (dwTotalBytes < 8192)		// too little disk to even try
	{
		ErrorDialog(IDS_DISK_SPACE_OUT);
		return MSGFETCH_NOTHING;
	}

	//if (dwTotalBytes < DWORD(g_lEstimatedMsgSize))
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
			pMsgRecord->GetEstimatedSize() > m_Settings->GetBigMsgThreshold()  //GetIniLong(IDS_INI_BIG_MESSAGE_THRESHOLD)
			)
		{
			return MSGFETCH_STUB;
		}
		else
			return MSGFETCH_ENTIRE;
	}
	
	return msgFetchType;
}









/*
////////////////////////////////////////////////////////////////////////
// POPGetMessage [private]
//
// This gets the message from the server and calls all of the lower
// level routines to store it away in the mail box and update TOC.
////////////////////////////////////////////////////////////////////////
int CPOPSession::POPGetMessage_(CTocDoc* pInToc, int nMessageNum)
{
	if (NULL == pInToc)
	{
		ASSERT(0);
		return -1;
	}

	//
	// Get the LMOS record for this message.
	//
	CMsgRecord* pMsgRecord = NULL;
	{
		CPopHost* pPopHost = CHostList::GetHostForCurrentPersona();
		if (pPopHost)
			pMsgRecord = pPopHost->GetMsgByMsgNum(nMessageNum);
	}
	
	if (NULL == pMsgRecord)
	{
		ASSERT(0);
		return -1;
	}
	
	g_bAttachmentDecodeEnabled = TRUE;

refetch:
	char szMessageNum[10];		// Message number
	wsprintf(szMessageNum, "%d", nMessageNum);

	//
	// Send the POP server a LIST command, then expect the following reply:
	//
	//     +OK <mess #> <mess size>
	//
	// All we want is the message size, which is stuffed into the global 
	// 'g_lEstimatedMsgSize'
	//
	char szBuffer[64];
	if (this->POPCmdGetReply(IDS_POP_LIST, szMessageNum, szBuffer, sizeof(szBuffer)) < 0)
		return -1;

	// Grab the message size
	{
		char szDummy[40];		// unused string
		int nDummy = 0;			// unused int
		sscanf(szBuffer, "%s %d %ld", szDummy, &nDummy, &g_lEstimatedMsgSize);
	}
    
	//
	// At this point we either read all, or top or nothing.  Have to
	// call GetMsgFetchType() here since it depends on 'g_lEstimatedMsgSize' being set.
	//
	int nLinesToRead = 20;
	int nStatus = 0;
	switch (pMsgRecord->GetMsgFetchType(m_ulBitFlags))
	{
	case CMsgRecord::MSGFETCH_ENTIRE:
		//
		// Set up the progress window then send the POP server a 
		// RETR command...
		//
		Progress(0, NULL, g_lEstimatedMsgSize);
		nStatus = this->POPCmdGetReply(IDS_POP_RETRIEVE, szMessageNum, szBuffer, sizeof(szBuffer));
		break;

	case CMsgRecord::MSGFETCH_HEADER:
		nLinesToRead = 1;
		// Fall through...

	case CMsgRecord::MSGFETCH_STUB:
		//
		// Send the POP server a TOP...
		//
		Progress(0, NULL, nLinesToRead * 80);	// Estimate of size
		sprintf(szMessageNum + strlen(szMessageNum), " %d", nLinesToRead);
		nStatus = this->POPCmdGetReply(IDS_POP_TOP, szMessageNum, szBuffer, sizeof(szBuffer));
		if (nStatus >= 0)
		{
			//
			// Reset the estimated size and make sure we don't try to 
			// process any message data ...
			//
			g_lEstimatedMsgSize = -g_lEstimatedMsgSize;
			g_bAttachmentDecodeEnabled = FALSE;
		}
		break;

	case CMsgRecord::MSGFETCH_NOTHING:
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
		if ((nStatus = FetchMessageText_(pInToc, nMessageNum)) < 0)
			return -1;
		if (g_lBadCoding)
		{
			switch (AlertDialog(IDD_BAD_DECODE))
			{
			case IDC_DOWNLOAD_AS_TEXT:
				g_bAttachmentDecodeEnabled = FALSE;
				goto refetch;
				break;
			case IDC_GET_RID_OF_IT:
				bDeleteMessage = TRUE;
				pInToc->SetModifiedFlag();
				break;
			case IDOK:
				bDeleteMessage = FALSE;
				pInToc->SetModifiedFlag();
				break;
			}
		}
		else
		{
			if (m_ulBitFlags & kDeleteAllBit)
				bDeleteMessage = TRUE;
			else if (m_ulBitFlags & kLMOSBit)
				bDeleteMessage = !m_Settings->DoLMOS(); //!GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER);
			else
				bDeleteMessage = FALSE;
		}
		
		// 
		// If we're not doing LMOS, we have a message to be deleted
		// from the server with the DELE command.
		//
		if (bDeleteMessage && g_lEstimatedMsgSize > 0)
		{
			//
			// Send the DELE command for the message we just finished
			// downloading and mark the LMOS record with the "delete sent"
			// flag (delS).
			//
			wsprintf(szMessageNum, "%d", nMessageNum);
			if (this->POPCmdGetReply(IDS_POP_DELETE, szMessageNum, szBuffer, sizeof(szBuffer)) >= 0)
				pMsgRecord->SetDeleteFlag(LMOS_DELETE_SENT);
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
////////////////////////////////////////////////////////////////////////
int CPOPSession::FetchMessageText_(CTocDoc* pInTOC, int nMessageNum)
{
	//
	// Initialize globals...
	//
	g_AnyRich = FALSE;
	g_AnyHTML = FALSE;
	g_bHasAttachment = FALSE;

	//
	// Call WriteMessageToMBX() to download header to IN.MBX,
	// instantiate the proper MIME reader, call the MIME reader to
	// download the message body, decode and burst attachments as
	// needed.
	//
	HeaderDesc hd(g_lEstimatedMsgSize);
	POPLineReader lineReader;
	int nStatus = WriteMessageToMBX_(hd, &lineReader);			// 1 on success, -1 on error
	ASSERT(1 == nStatus || -1 == nStatus);
	
	//
	// Form our own message id by concatenating the message id
	// from the header and the received date, converting any
	// embedded spaces to dashes.
	//
	{
		CPopHost* pPopHost = CHostList::GetHostForCurrentPersona();
		if (pPopHost)
		{
			char szMessageId[2000];
			wsprintf(szMessageId, "%s-%s", hd.messageID, hd.receivedDate);
			ASSERT(strlen(szMessageId) < sizeof(szMessageId));		// better late than never
			for (char* pszMessageId = szMessageId; *pszMessageId; pszMessageId++)
			{
				if (*pszMessageId == ' ')
					*pszMessageId = '-';
			}

			//
			// Set the message ID in the corresponding LMOS record.
			//
			CMsgRecord* pMsgRecord = pPopHost->GetMsgByMsgNum(nMessageNum);
			pMsgRecord->SetMesgID(szMessageId);
			if (! pPopHost->CanUIDL())
			{
				// 
				// Fill in the non-UIDL id with header data we've
				// sucked from the message.
				//
				pMsgRecord->SetHashString(szMessageId);
			}
		}
		else
		{
			//
			// This should never happen, right?
			//
			ASSERT(0);
		}
	}

	//
	// Resize the mailbox to the end of the successfully read 
	// message.
	//
	{
		long lCurrent = 0;
		g_pPOPInboxFile->Tell(&lCurrent);
		ASSERT(lCurrent >= 0);
		g_pPOPInboxFile->ChangeSize(lCurrent);
	}
	if (nStatus < 0)
		return nStatus;

	//
	// Now, seek back to the start of the newly-written message header
	// and create a new summary object by re-reading the header info
	// out of the MBX file.  Seek() does a Flush() which can fault.
	//
	if (FAILED(g_pPOPInboxFile->Seek(hd.lDiskStart)))
		return -1;
	ASSERT(1 == nStatus);
	CSummary* pSummary = pInTOC->UpdateSum(g_pPOPInboxFile, TRUE);
	if (NULL == pSummary)
	{
		//
		// KLUDGE ALERT!
		// There may be more parts than we expect because there may be
		// an encoded From envelope in the message, so we need to keep
		// looking for messages until we don't get any more.
		//
		ASSERT(0);		// Tell Willie if this ASSERT ever fires...
		return nStatus;
	}

	//
	// Annotate various summary properties.
	//
	if (g_AnyRich)
		pSummary->SetFlag(MSF_XRICH);

	if (g_AnyHTML)
		pSummary->SetFlagEx(MSFEX_HTML);

	if (g_bHasAttachment)
		pSummary->SetFlag(MSF_HAS_ATTACHMENT);

	if (hd.isDispNotifyTo)
		pSummary->SetFlag(MSF_READ_RECEIPT);

	//
	// Write newly-built summary record to the IN.TOC file.
	//
	if (! pInTOC->WriteSum(pSummary))
		return -1;

	return nStatus;
}

*/





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

	//MainProgress(CRString(IDS_SERVER_CONNECT)); 
	m_pTaskInfo->SetMainText(CRString(IDS_SERVER_CONNECT));
	//
	// Grab POP account setting
	//
	//char szPOPAccount[128];
	const char *szPOPAccount = m_Settings->GetPOPAccount();
	
	//::GetIniString(IDS_INI_POP_ACCOUNT, szPOPAccount, sizeof(szPOPAccount));
	if (*szPOPAccount == '\0') 
	{
		if (!gbAutomationCheckMail)
		{
			ErrorDialog(IDS_ERR_NO_POP_ACCT);
		}
		return FALSE;
	}

	//
	// Extract the POP server from the POP account string.
	//
	char* pszPOPServer = strrchr(szPOPAccount, '@');
	if (NULL == pszPOPServer)
	{
		if (!gbAutomationCheckMail)
		{
			ErrorDialog(IDS_ERR_BAD_POP_ACCT, szPOPAccount);
		}
		return FALSE;
	} 

	pszPOPServer++;			// skip over '@'
	if (!pszPOPServer || *pszPOPServer == 0)
		return FALSE;

	//
	// Decide which service and port id to use...
	//
	UINT idPopService	= IDS_POP_SERVICE;
	//UINT idPopPort		= IDS_INI_POP_PORT;
	UINT idPopPort		= m_Settings->GetPort();
	
	UINT DefaultPort	= 110;
	if (m_Settings->GetAuthenticationType() == AUTH_KERBEROS)
	{
		idPopService	= IDS_KERB_POP_SERVICE;	// kpop
		//idPopPort		= IDS_INI_KERB_PORT;
		idPopPort		= (int) GetIniShort(IDS_INI_KERB_PORT);
		DefaultPort		= 1110;
    }

	//
	// Establish the network connection to the POP server...
	//
	if (!m_pNetConnection || m_pNetConnection->Open(pszPOPServer, idPopService, idPopPort, DefaultPort, TRUE) < 0)
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
		;//Progress(szCommand);			// Don't show arguments to some commands
	if (pszArgs)
		strcat(szCommand, pszArgs);		// Add arguments
		
	if (uCmdID != IDS_POP_PASS && uCmdID != IDS_POP_APOP)
		; //Progress(szCommand);
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
				m_bCleanup = true;

				char* ExtendedResponse = pszBuffer + 4;

				while (isspace((unsigned char)*ExtendedResponse))
					ExtendedResponse++;
				char *szResponse = _strlwr( ::SafeStrdupMT(ExtendedResponse));

				char *pszBadPass = _strlwr(::SafeStrdupMT(m_Settings->GetBadPass()));  //GetIniString(IDS_INI_BAD_PASSWORD_STRING);
				if (!*pszBadPass || strstr(szResponse, pszBadPass) == NULL)
				{
					// Check for POP3 Extended Response [IN-USE] to see if maildrop is locked.
					// If so, then don't erase the password because it's not bad.
					// Also don't erase password if we get [LOGIN-DELAY].
					// See <http://info.internet.isi.edu/in-drafts/files/draft-gellens-pop3ext-05.txt>
					//const char* ExtendedResponse = pszBuffer + 4;
					
					
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
				delete [] szResponse;
				delete [] pszBadPass;
				
				wsprintf(szCommand, "%s%s", (const char *) CRString(uCmdID),
						(const char *) CRString(IDS_ERR_POP_PASSWORD_HIDE));
			}

			//
			// Display error message for this POP session.
			//
			//char szPOPAccount[128];
			//GetIniString(IDS_INI_POP_ACCOUNT, szPOPAccount, sizeof(szPOPAccount));
			//ErrorDialog(IDS_ERR_TRANSFER_MAIL, szCommand, "POP", szPOPAccount, pszBuffer + 1);
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
				if (rc >= 0)
				{
					if (rc > 1)
						buf[rc - 2] = 0;
					// for AUTH RPA it only returns a +.
					if (buf[0] == '+')
					// Now we have a message from the POP server of the form waiting to be read...
						rc = Rpa.Authenticate(m_Settings->GetPOPAccount());
					else if(buf[0] == '-') //we had an error
					{
						//POPError(cmdbuf, buf + 1);
						//char szPOPAccount[128];
						//GetIniString(IDS_INI_POP_ACCOUNT, szPOPAccount, sizeof(szPOPAccount));
						//ErrorDialog(IDS_ERR_TRANSFER_MAIL, cmdbuf, "POP", szPOPAccount, buf + 1);
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
		//::Progress(-1, (const char *)CRString(IDS_KERB_LOGIN), -1);

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
	// Put up the progress window at this point...
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
		//CString strPOPAccount(::GetIniString(IDS_INI_POP_ACCOUNT));
		//ErrorDialog(IDS_ERR_POP_CMD, (const char *) strPOPAccount, szBuffer + 1);
		ErrorDialog(IDS_ERR_POP_CMD, m_Settings->GetPOPAccount(), szBuffer + 1);
		return -1;
	}
		
	//
	// Create the USER string by stripping the '@domain.com'
	// from the POP account string.
	//
	char szPOPUserName[128];
	strcpy(szPOPUserName, m_Settings->GetPOPAccount());
	//::GetIniString(IDS_INI_POP_ACCOUNT, szPOPUserName, sizeof(szPOPUserName));
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

	default:
		ASSERT(0);
		return -1;
	}

	// Get the message count from the server
	if (this->POPCmdGetReply(IDS_POP_STAT, NULL, (char *) szBuffer, sizeof(szBuffer)) < 0)
		return (-1);
	
	nStatus = atoi(szBuffer + 3);
	
	wsprintf(szBuffer, "%s (%d)", pAtSign + 1, nStatus);
	//::PutDebugLog(DEBUG_MASK_RCVD, szBuffer);

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
	// Lets get the current host object...
	//CPopHost		*theHost 		= CHostList::GetHostForCurrentPersona();

	
	// The message could be one of two places.  If the messages marked as having the
	// DELE command sent to them didn't actually get deleted, then the last message
	// will be correctly placed at the end of the list.  However, if messages did
	// get deleted, then we need to subtract off all the messages that are marked as
	// having the DELE command sent to them.
	
	/*int HighIndex = nLast;
	int LowIndex = HighIndex;
	int DeleteCount = 0;
	int LastDefined = 0;
		
	// and get the message list ordered by Number...
	CMsgNumList	*theMsgNumList = (CMsgNumList *)m_pPopHost->GetMsgNumList();
	
	POSITION pos = theMsgNumList->GetHeadPosition();
	CMsgRecord* theLastMsg = NULL;
	for (int i = 1; pos; i++)
	{
		CMsgRecord* theRecord = theMsgNumList->GetNext(pos);
		if (theRecord)
		{
			if (theRecord->IsEqualMesgID("udef") == FALSE)
			{
				theLastMsg = theRecord;
				LastDefined = i;
			}
			if (theRecord->GetDeleteFlag() == LMOS_DELETE_SENT)
				DeleteCount++;
		}
	}

	HighIndex = min(HighIndex, LastDefined);
	LowIndex = HighIndex - DeleteCount;*/

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
		//BOOL Equal = FALSE;
		if (LowIndex != HighIndex)
		{
			theHash = MakeFakeMessageIdAndHash_(HighIndex, szMessageId);
			
			if (theLastMsg->IsEqualMesgID(szMessageId))
			{
				//s_ulMessageIndex = HighIndex;
				ulMessageIndex = HighIndex;
				theReturn = TRUE;
				
				m_pPopHost->ReplaceDeleteSentWithDelete();
				/*
				// The high index matched, so we need to mark the entries that were
				// marked as having the DELE command sent back to just plain delete
				POSITION NextPos;
				pos = theMsgNumList->GetHeadPosition();
				for (NextPos = pos; pos; pos = NextPos)
				{
					CMsgRecord* rec = theMsgNumList->GetNext(NextPos);
					if (rec && rec->GetDeleteFlag() == LMOS_DELETE_SENT)
						rec->SetDeleteFlag(LMOS_DELETE_MESSAGE);
				}*/

			}
		}

		//Why we need following code? SK
		//else
		//	s_ulMessageIndex	= 0;
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
		// source buffer to the target buffer.  FORNOW, this is
		// dangerous since we have no idea how big the target buffer
		// is.
		//
		strcpy(pszMsgId, pszBuf);

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
		// into the target buffer.  FORNOW, this is dangerous since
		// we have no idea how big the target buffer is.
		//
		pszBuf++;
		strcpy(pszDate, pszBuf);
		
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
	// FORNOW, this is dangerous, since we have no idea
	// how big the caller's buffer is.
	//
	//ulHash = ::PersonaHash(strFakeMessageID);
	ulHash = ::HashMT(strFakeMessageID + m_strPersona);
	strcpy(pszFakeMessageID, strFakeMessageID);

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
unsigned long CPOPSession::ComputeMessageIdAndHash(CString& strFileName, char *pszFakeMessageID)
{
	// Put some garbage into the string
	strcpy(pszFakeMessageID, "I'll gladly pay you Tuesday for a hamburger today.");

	/*
	// Range check
	if (nMsgNum < 1 || nMsgNum > m_nMessageCount)
		return 0;
	
    
	//
	// Send the POP server a TOP command.  Request one line of the
	// body because some POP servers choke on a TOP command with zero
	// for the number of body lines to return.
	//
	char szBuffer[512];
	{
		char szCommand[32];
		wsprintf(szCommand, "%d 1", nMsgNum);
		if (this->POPCmdGetReply(IDS_POP_TOP, szCommand, szBuffer, sizeof(szBuffer)) < 0)
			return 0;
	}
	*/
	
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
	//while ((this->ReadPOPLine(szBuffer, sizeof(szBuffer)) > 0) && (*szBuffer != '\r'))
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
	// FORNOW, this is dangerous, since we have no idea
	// how big the caller's buffer is.
	//
	//ulHash = ::PersonaHash(strFakeMessageID);
	ulHash = ::HashMT(strFakeMessageID + m_strPersona);
	strcpy(pszFakeMessageID, strFakeMessageID);
/*
	//
	// Flush the rest of the TOP data...
	//
	while (*szBuffer && (this->ReadPOPLine(szBuffer, sizeof(szBuffer)) > 0))
	{
		// keep looping...
	}
*/
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
			//::CloseProgress();
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
	
	CPopHost* pServerHost = new CPopHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount());
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
////////////////////////////////////////////////////////////////////////
CPopHost* CPOPSession::GetHostFromUIDLServer_()
{
	// If no messages, then just return an empty list
	if (0 == m_nMessageCount)
		return new CPopHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount());


	//This is a hack to make Eudora act as if it is accessing NOn-UIDL servers, to make debugging easy
	//Set DebugDisableUIDL to 1 to make Eudora forget about UIDLs
	if( g_bDisableUIDL == true)
		return NULL;

	//
	// Ask the POP server for the UIDL...
	//
	char szBuffer[kLMOSBuffSize];
	
	if( POPCmdGetReply(IDS_POP_UIDL, NULL, szBuffer, sizeof(szBuffer), FALSE) < 0)
		return (CPopHost*) NULL;

	
 	CPopHost* pServerHost = NULL;			// returned
	
	if ('+' == szBuffer[0])
	{
		// 
		// Got OK to continue...
		//
		pServerHost = new CPopHost(m_Settings->GetPersona(), m_Settings->GetPOPAccount());
		
		// 
		// Show progress...
		//
		//::MainProgress(CRString(IDS_POP_CHECK_UIDL));
		//::Progress(0, NULL, m_nMessageCount);
		m_pTaskInfo->SetMainText(CRString(IDS_POP_CHECK_UIDL));

		//
		// Read until there are no more messages....
		//
		int nMessageNumber = 1;
		int nLength = 1;
		while (nLength > 0)
		{
			//
			// Read one line at a time
			//
			nLength = this->ReadPOPLine(szBuffer, sizeof(szBuffer));
			if (nLength > 0)
			{
				//::Progress(nMessageNumber++);

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

		//::Progress(1, NULL, 1);

		if(pServerHost)
			pServerHost->SetCanUIDL(TRUE);
	}
	else
	{
		//
		// We failed on the UIDL call.. so do something smart here...
		// something smart equates to get the list of messages from the server
		// and then create a new list for the host for each message after getting
		// each header from the server...this is gonna be slow...
		//
		//Progress(-1, CRString(IDS_POP_REQUESTING_LIST), -1);
		//pNewHost = this->GetNonUIDLHost();	   	 
	}

	return pServerHost;
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
	//::MainProgress(CRString(IDS_POP_CLEANING_UP));
	m_pTaskInfo->SetMainText(CRString(IDS_POP_CLEANING_UP));

	if (m_pNetConnection != NULL)
	{
		if (m_bPOPConnected)
		{
			if (GetAuthenticationType_() == AUTH_KERBEROS)
				;//::MainProgress(CRString(IDS_KERB_SHUTDOWN));
			else
				;//::MainProgress(CRString(IDS_POP_SHUTDOWN));

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
				// Purge the deleted messages from the LMOS database,
				// then renumber what's left.
				//
				//m_pPopHost->RemoveDeletedMsgRecords();
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
		//if (::CreateNetConnection())
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
			//if (::GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE) || 
			//	::GetIniShort(IDS_INI_CONNECT_OFFLINE))
			
			//if (m_Settings->GetNetImmediateClose()
			//		|| m_Settings->ConnectOffline())
			{
				delete m_pNetConnection;
				m_pNetConnection = NULL;
			}
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
	//MainProgress(CRString(IDS_POP_LOGGING_INTO_POP));
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
			
			/*
			if (s_bKerbAppLoaded)
				bSuccess = Kerb16SetName(szPOPUserName);	
			else
				bSuccess = (*fnSetUserName)(szPOPUserName);
			
			
			if(bSuccess)
				s_bKerbTicket = TRUE;
			else
				errKerb = KERBERR_REG_FAILED;
			*/	
			
			
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
			//Progress(-1, szDisplayString, -1);
			
		}

	    
		//
		// Don't have the foggiest...just following Steve Dorner's 
		// suggestion...
		//
		//strcpy(s_szKerbTicketBuffer,"AUTHV0.1JUSTJUNK");
		//BOOL bSentTicket = FALSE;
/*		
		int nSize = strlen(s_szKerbTicketBuffer);
		if (s_bKerbAppLoaded)
		{
			s_dwKerbTicketBufferLength = dwTickenLen;
			bSuccess = Kerb16GetTicket(szCommandString, (s_szKerbTicketBuffer+nSize), &s_dwKerbTicketBufferLength);
			dwTickenLen = s_dwKerbTicketBufferLength;
		}
		else
		{
			if (QCKerberos::m_pfnSendTicketForService)
			{
				//
				// According to chu@platinum.com (Howard Chu) we need
				// only to call this for both kerb4 & kerb5, we no
				// longer need GetTicketForService...
				//
				bSuccess = QCKerberos::SendTicketForService(szCommandString, buf, NetConnection->GetSocket());
				if (bSuccess) 
					bSentTicket = TRUE;
				
			}
			else if (QCKerberos::m_pfnGetTicketForService)
			{
				// Try the old original way...
				bSuccess = QCKerberos::GetTicketForService(szCommandString, (s_szKerbTicketBuffer+nSize), &dwTickenLen);
			}
			else
				bSuccess = FALSE;
			
		}
	*/

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
				//dwTickenLen += 16;
				
				dwTickenLen = QCKerberos::GetTicketLength() + 16;
				const char *szTicketBuf = QCKerberos::GetTicketBuffer();
				
				//int nStatus = NetConnection->PutDirect(s_szKerbTicketBuffer, int(dwTickenLen));
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
	
	/*
	if (s_hInstKerberosLib)
	{
		QCKerberos::FreeLibrary();
		//FreeLibrary(s_hInstKerberosLib);
		//s_hInstKerberosLib = NULL;
	}
	else if (s_bKerbAppLoaded)
	{
		Kerb16FreeLib();
		KerbQuitApp();
	}
	*/


	return errKerb;
}



      

