////////////////////////////////////////////////////////////////////////
// File: POP.CPP
//
//
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <mmsystem.h>


#include "pop.h"
#include "POPSession.h"


#include "eudora.h"  // isVersion4
#include "guiutils.h"
#include "rs.h"
#include "progress.h"

#include "tocdoc.h"
#include "tocview.h"

#include "lex822.h"
#include "header.h"
#include "FileLineReader.h"
#include "mime.h"

#include "HostList.h"



#include "mainfrm.h"
#include "filtersd.h"
#include "debug.h"
#include "persona.h"

#include "SpoolMgr.h"
#include "fileutil.h"



//	Defines and other Constants	=============================
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//	Externals	=============================================



//	Globals	=================================================
JJFile*			g_pPOPInboxFile 		= NULL;
CPOPSession*	gPOP			= NULL;  //FORNOW
CHostList*		gPopHostList	= NULL;


BOOL 	g_bHasAttachment = FALSE;
long 	g_lEstimatedMsgSize;
BOOL	g_bAttachmentDecodeEnabled = TRUE;
long	g_lBadCoding;



/*
BOOL g_bShowProgress;	// shared with Progress.cpp (yuk)
static void PlaySound( void );
*/



//////////////@@@@@@@@@@@@@@@@@ CPOP @@@@@@@@@@@@@@@@@@@@@@@///////////////////////

CPOP::CPOP(POPVALUES *pv, QCPOPNotifier *notes) : m_Notifier(notes), m_bEscapePressed(false)
{
	m_bGotNewMail = pv->bGotNewMail;
	
	m_Settings = pv->settings;
	

	//m_ulBitFlags = pv->ulBitFlags;
	m_ulBitFlags = m_Settings->GetBitFlags();

	//m_bOpenInMailbox = pv->bOpenInMailBox;
	m_bOpenInMailbox = m_Settings->OpenInMailbox();

	//m_strPersona = pv->persona;
	m_strPersona = m_Settings->GetPersona();
	
	delete pv;

}

CPOP::CPOP(CString strPersona) 
	: m_strPersona(strPersona), m_bGotNewMail(true),
		m_ulBitFlags(kRetrieveNewBit), m_Settings(NULL), m_bEscapePressed(false)
{ 
	m_bOpenInMailbox = GetIniShort(IDS_INI_OPEN_IN_MAILBOX)?true:false;
	m_Notifier = new QCPOPNotifier(1);
}


bool CPOP::GetMailFromSpool(bool bCrashRecovery)
{
	//Set to the persona to current
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_strPersona );
	

	bool bDone = true;
	BOOL bFetching = BOOL(m_ulBitFlags & (kRetrieveNewBit|kRetrieveMarkedBit|kFetchHeadersBit));
	
	CTocDoc* pInTocDoc = ::GetInToc();
	CTocView* pInTocView = (pInTocDoc? pInTocDoc->GetView() : NULL);

	BOOL bResult = FALSE;
	if(m_bGotNewMail)
	{
		bDone = DoGetPOPMailFromSpool(bCrashRecovery);
		if (bFetching && !(m_ulBitFlags & kFetchHeadersBit))
			bResult = DoFilterMail_();
	}
	
	/*if (bFetching)
	{
#ifndef COMMERICAL
		// Freeware bug fix: call CloseProgress before
		// DoNotifyNewMail().  Causes CloseProgress to be
		// called again below, but oh well.
		::CloseProgress();
#endif
		bResult = DoNotifyNewMail_(bResult);
	}*/
	
	::CloseProgress();

	// Turn redraw back on the In mailbox as it will have been turned off
	// if new messages have come in
	if (pInTocView)
		pInTocView->m_SumListBox.SetRedraw(TRUE);

	g_Personalities.SetCurrent( CachePersonality );

	return bDone;
}




////////////////////////////////////////////////////////////////////////
// DoGetPOPMail_ [private]
//
// Overloaded method to do the actual message fetches.
////////////////////////////////////////////////////////////////////////
bool CPOP::DoGetPOPMailFromSpool(bool bCrashRecovery)
{
	
	//
	// All we need do is get the current host...this call will
	// make the new host list if needed...
	//
	CPopHost* pPopHost = CHostList::GetHostForCurrentPersona();
	if (NULL == pPopHost)
		return FALSE;
	
	/*
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
	*/
	

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
	
	        	
	// Lets get the first message to grab
	//nMsgNumber = pPopHost->GetNextFetchableMsgNum(m_ulBitFlags, TRUE);

	
	//already set in DoGetPopMailToSpool function
	//m_bGotNewMail = FALSE;

	// Now while the nMsgNumber is not zero we get messages...
    bool bReturn = true;
	//for (; nMsgNumber > 0; nMsgNumber = pPopHost->GetNextFetchableMsgNum(m_ulBitFlags), ::DecrementCountdownProgress())


	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(m_strPersona);
	CInSpoolInfo *pSpoolInfo = NULL;

	int nTotalMsgs = SpoolMgr->m_In->GetCount(SS_RECEIVED);
	// Let's set up the progess window string, but only if there were spool files to process
	if (nTotalMsgs)
		::CountdownProgress(CRString(IDS_PREPARING_TO_DISPLAY), nTotalMsgs);
	

	int nMsgNumber = 0;
	while(nTotalMsgs--)
	{
		if (EscapePressed())
		{
			bReturn = false;
			break;
		}
			
		pSpoolInfo = SpoolMgr->m_In->Get(SS_RECEIVED, SS_REMOVAL_INPROGRESS); 
		if(!pSpoolInfo)
		{
			ASSERT(0);
			break;
		}

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

		CMsgRecord* pMsgRecord = pSpoolInfo->GetMsgRecord();
		
		//Don't decode attachments if we skipped a message and only showing either headers or a stub
		g_bAttachmentDecodeEnabled = (pMsgRecord->GetSkippedFlag() == LMOS_SKIP_MESSAGE)?FALSE:TRUE;
		
		if(g_bAttachmentDecodeEnabled)
			g_lEstimatedMsgSize = pMsgRecord->GetEstimatedSize();
		else
			g_lEstimatedMsgSize = -(pMsgRecord->GetEstimatedSize());
		
		//ASSERT(g_lEstimatedMsgSize < 111000);
		//int nStatus = POPGetMessage_(pInTOC, nMsgNumber);
		int nStatus = FetchMessageTextFromSpool(pInTOC, pSpoolInfo);
		
		
		if (nStatus >= 0)
		{
			//Remove  the file as we are done with it
			//SpoolMgr->MoveFile(pSpoolInfo, SPOOL_FILE_DELETE);
			pSpoolInfo->SetSpoolStatus(SS_REMOVED);
			SpoolMgr->m_In->RemoveFromSpool(pSpoolInfo);

			if (pMsgRecord != NULL)
			{
				if(bCrashRecovery)
				{
					// Set the Retrieval flag to don't retrieve (Nget)
					pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE);
	
					// Set the has been read flag...
					pMsgRecord->SetReadFlag(LMOS_HASBEEN_READ);

				}
				
				/*
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

				*/
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
			TRACE("Fatal error: Could not fetch msg from spool\n");
			
			//If stopped due to an escape, reset its state to be processed later
			if(m_bEscapePressed)
				pSpoolInfo->SetSpoolStatus(SS_RECEIVED);
		
			bReturn = false;
			break;
		}
		::DecrementCountdownProgress();
	}

	
	if(bCrashRecovery)
	{
		pPopHost->WriteLMOSData();
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
int CPOP::FetchMessageTextFromSpool(CTocDoc* pInTOC, CSpoolInfo *pSpoolInfo)
{
	//
	// Initialize globals...
	//
	g_bHasAttachment = FALSE;

	
	CString strSpoolFileName = pSpoolInfo->GetFileName();
	ASSERT( ::FileExistsMT(strSpoolFileName));
	
	
	JJFile spoolFile;
	if( spoolFile.Open(strSpoolFileName, O_RDWR) != S_OK)
	{
		ASSERT(0);
		TRACE("Fatal error: Could not open Spool file %s\n", strSpoolFileName);
		return -1;
	}
	
	FileLineReader *pFileLineReader = new FileLineReader;
	pFileLineReader->SetFile(&spoolFile);

	//
	// Call WriteMessageToMBX() to download header to IN.MBX,
	// instantiate the proper MIME reader, call the MIME reader to
	// download the message body, decode and burst attachments as
	// needed.
	//
	HeaderDesc hd(g_lEstimatedMsgSize);
	int nStatus = WriteMessageToMBX_(hd, pFileLineReader);			// 1 on success, -1 on error
	if(nStatus < 0)
	{
		//See whether sptopped by an Escape or hitting the STOP button
		m_bEscapePressed = pFileLineReader->IsEscapePressed();
	}

	delete pFileLineReader;
	ASSERT(1 == nStatus || -1 == nStatus);
	
	/*
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
	*/

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
	if(hd.hasRich)  //if (g_AnyRich)
		pSummary->SetFlag(MSF_XRICH);

	if(hd.isMHTML)  //if (g_AnyHTML)
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



////////////////////////////////////////////////////////////////////////
// WriteMessageToMBX_ [private]
//
// Uses the global g_pPOPInboxFile to write the funny Eudora "From" envelope
// timestamp to the MBX file.  Writes an optional X-Persona line.
// Then, read the message header and read the message body using the
// MIMEState object.  All the data is written to the MBX file
// (g_pPOPInboxFile).  Returns 1 if successful, else -1 on error.
////////////////////////////////////////////////////////////////////////
int CPOP::WriteMessageToMBX_(HeaderDesc& hd, LineReader* pLineReader)
{
	CObArray objArrayMimeStates;
	//POPLineReader lineReader;		// really just a polymorphic wrapper for CPOPSession::ReadPOPLine()
	//MIMEState ms(&lineReader);
	MIMEState ms(pLineReader);

	char szBuffer[2048];

	//
	// Initialize the MIME reader state.
	//
	::mimeInit();

	//
	// This looks like a loop, but it's not.  It's just a hack to
	// get the 'break' statement to behave like a goto.  :-)
	//
	while (1)
	{
		long lStartOffset = 0;
		g_pPOPInboxFile->Tell(&lStartOffset);
		if (lStartOffset < 0)
			break;

		hd.lDiskStart = lStartOffset;

		//
		// Write time-stamped envelope marker line.  You know, the
		// one that looks like:
		//
		//           From ???@??? Fri Mar 10 09:04:22 1995
		//
		{
			time_t currentTime;
			time(&currentTime);
			if (currentTime < 0)
				currentTime = 1;
			struct tm* pTime = localtime(&currentTime);

			CRString Weekdays(IDS_WEEKDAYS);
			CRString Months(IDS_MONTHS);
			wsprintf(szBuffer, CRString(IDS_FROM_FORMAT), 
								((const char *) Weekdays) + pTime->tm_wday * 3,
								((const char *) Months) + pTime->tm_mon * 3, 
								pTime->tm_mday, 
								pTime->tm_hour,
								pTime->tm_min, 
								pTime->tm_sec, 
								pTime->tm_year + 1900);
			if (FAILED(g_pPOPInboxFile->PutLine(szBuffer)))
				break;
		}

		//
		// If we have a non-default personality, write the X-Persona header
		//
		CString strCurrentPersona = g_Personalities.GetCurrent();
		if ( ! strCurrentPersona.IsEmpty() )
		{
			CString XPersonaString;
			XPersonaString.Format(CRString(IDS_PERSONA_XHEADER_FORMAT), (const char *) strCurrentPersona);
			if (FAILED(g_pPOPInboxFile->PutLine( XPersonaString )))
				break;
		}
	
		//
		// Download the header and write it to the MBX file.
		//
		//int nReadStatus = hd.Read(&lineReader);
		int nReadStatus = hd.Read(pLineReader);
		hd.lDiskStart = lStartOffset;		// Envelope is part of the header

		//
		// Did we get the end of the message already?  We weren't
		// supposed to, so the message must be truncated (i.e.  no
		// body).  Oh well, let's just write what we got to file, and
		// consider it a successful download because there's really
		// not much else we can do.
		//
		if (nReadStatus != EndOfMessage)
		{
			if (nReadStatus != EndOfHeader) 
				break;
		
			wsprintf(szBuffer, "%s, %s", hd.who, hd.subj);
			::Progress(szBuffer);

			//
			// Instantiate the appropriate MIME reader object
			// using info gleaned from the Content-Type: header.
			//
			if (! ms.Init(&hd) || objArrayMimeStates.Add(&ms) < 0)
				return -1;

			//
			// Okay, call the polymorphic MIME reader to really 
			// download the message body...
			//
			BoundaryType endType = ms.m_Reader->ReadIt(objArrayMimeStates, szBuffer, sizeof(szBuffer));
			objArrayMimeStates.RemoveAt(objArrayMimeStates.GetUpperBound());

			//
			// If the top-level type was message/[rfc822|news] then
			// hd.diskStart got munged so set it back to the very
			// beginning of the message
			//
			hd.lDiskStart = lStartOffset;

			if (endType == btError)
				break;
		}
	
		if (g_lEstimatedMsgSize < 0)
		{
			//
			// Write out warning about skipped message body...
			//
			g_pPOPInboxFile->PutLine(CRString(IDS_WARN_SKIP_BIG_MESSAGE));
			wsprintf(szBuffer, CRString(IDS_BIG_MESSAGE_SIZE), -g_lEstimatedMsgSize);
			g_pPOPInboxFile->PutLine(szBuffer);
			g_pPOPInboxFile->PutLine(CRString(IDS_HOW_TO_GET_BIG_MESSAGE));
		}
	
		::Progress(1, NULL, 1);
	
		long lEndOffset = 0;
		g_pPOPInboxFile->Tell(&lEndOffset);
		if (lEndOffset < 0)
			break;

		wsprintf(szBuffer, CRString(IDS_DEBUG_GOOD_RETRIEVE), hd.who, hd.subj);
		::PutDebugLog(DEBUG_MASK_RCV, szBuffer);

		//
		// We don't use the Status: header to determine if a big
		// message has already been skipped.  That was the old way.
		// The new way is determined elsewhere with LMOS record flags.
		//
		return 1;
	}

	wsprintf(szBuffer, CRString(IDS_DEBUG_BAD_RETRIEVE), hd.who, hd.subj);
	::PutDebugLog(DEBUG_MASK_RCV, szBuffer);
	return -1;
}



////////////////////////////////////////////////////////////////////////
// DoFilterMail_ [private]
//
// The main filter action driver.  FORNOW, this probably doesn't belong
// in the POP module since it operates directly on the TOCs.
////////////////////////////////////////////////////////////////////////
BOOL CPOP::DoFilterMail_()
{
	//
	// Remember that m_bGotNewMail is really a tri-state variable with values
	// of -1, FALSE, and TRUE.
	//
	if (m_bGotNewMail != TRUE)
		return FALSE;			// FALSE indicates that we should not open the Inbox

	//
	// Get the In and Trash mailboxes.
	//
	CTocDoc* pInTOC = ::GetInToc();
	CTocDoc* pTrashTOC = ::GetTrashToc();
	if (NULL == pInTOC || NULL == pTrashTOC)
	{
		ASSERT(0);
		return FALSE;			// FALSE indicates that we should not open the Inbox
	}

	//
	// Save the number of messages in the Trash.
	//
	int nOriginalTrashCount = pTrashTOC->NumSums();
	m_nTrashedMessageCount = 0;

	//
	// Filter new messages
	//
	CFilterActions filt;
	if (! filt.StartFiltering(WTA_INCOMING))
	{
		m_Notifier->m_bOpenInbox = true;
		m_Notifier->SetNotifyNewMail(true);
		m_Notifier->AddNotifications( CMF_RECEIVED_MAIL );
		return TRUE;
	}

	//
	// Turn off redraw.  Otherwise, transfers caused by filters will 
	// cause screen flicker.
	//
	if (pInTOC->GetView())
		pInTOC->GetView()->m_SumListBox.SetRedraw(FALSE);

	//
	// Determine the position of the first newly downloaded message
	// in the Inbox.
	//
	POSITION pos = NULL;
	if (! m_LastMessagePos)
		pos = pInTOC->m_Sums.GetHeadPosition();
	else
		pInTOC->m_Sums.GetNext(pos = m_LastMessagePos);
		
	// 
	// Count how many messages there are to filter so that we can do
	// a proper progress display
	//
	int nNumMessagesToFilter = 0;
	{
		for (POSITION posCount = pos; posCount; nNumMessagesToFilter++)
			pInTOC->m_Sums.GetNext(posCount);
	}
	
	m_bNotifyNewMail = FALSE;
	BOOL bOpenInbox = FALSE;		// returned
	if (nNumMessagesToFilter)
	{
		//
		// If we get this far, we have some messages to filter, so
		// get to it...
		//
		m_nDownloadCount = nNumMessagesToFilter;
		::CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), nNumMessagesToFilter);
		::Progress(-1, NULL, -1);
		
		while (pos)
		{
			if (EscapePressed())
				break;
			int nFilterAction = filt.FilterOne(pInTOC->m_Sums.GetNext(pos), WTA_INCOMING);
			if (! (nFilterAction & FA_TRANSFER))
			{
				//bOpenInbox = TRUE;
				m_Notifier->m_bOpenInbox = true;
			}
			
			if (! m_bNotifyNewMail)
			{
				//m_Notifications |= CMF_RECEIVED_MAIL;
				m_bNotifyNewMail = ((nFilterAction & FA_NORMAL) != 0);
				m_Notifier->SetNotifyNewMail((nFilterAction & FA_NORMAL) != 0);
				m_Notifier->AddNotifications( CMF_RECEIVED_MAIL );
			}

			::DecrementCountdownProgress();
		}
	}

	filt.EndFiltering();

	if (pInTOC->IsModified())
		pInTOC->Write();

	//
	// Turn redraw back on.
	//
	if (pInTOC->GetView())
		pInTOC->GetView()->m_SumListBox.SetRedraw(TRUE);

	//
	// Save the number of messages trashed by this filtering pass.
	//
	m_nTrashedMessageCount = pTrashTOC->NumSums() - nOriginalTrashCount;

	//Update the notifier
	m_Notifier->AddTrashCount(m_nTrashedMessageCount);
	m_Notifier->AddDownloadCount(m_nDownloadCount);
	
	return bOpenInbox;
}


////////////////////////////////////////////////////////////////////////
// DoNotifyNewMail_ [private]
//
// Opens the In mailbox, depending on availability of newly downloaded
// mail and user setting.  Also, decides whether or not to display New
// Mail alert, No New Mail alert, and play new mail sound depending on
// user settings.  The actual display of alerts and playing of sounds
// is deferred until later, since we could be checking mail for
// multiple personalities.
////////////////////////////////////////////////////////////////////////
//BOOL CPOP::DoNotifyNewMail_(BOOL bOpenInbox)
BOOL CPOP::DoNotifyNewMail()
{
	m_bNotifyNewMail = m_Notifier->m_bNotifyNewMail;
	m_nDownloadCount = m_Notifier->m_nDownloadCount;
	m_nTrashedMessageCount = m_Notifier->m_nTrashedMessageCount;
	bool bOpenInbox = m_Notifier->m_bOpenInbox;

	if(!m_bNotifyNewMail)
	{
		m_bGotNewMail = FALSE;
		m_bNotifyNewMail = true;
	}
	

	if (m_bNotifyNewMail)
	{
		//
		// Do the notify stuff.  Note that 'm_bGotNewMail' is really a
		// tri-valued variable:
		//     -1: never started to collect mail
		//  FALSE: no new mail
		//   TRUE: new mail
		//
		switch (m_bGotNewMail)
		{
		case FALSE:
			if (! (m_ulBitFlags & kSilentCheckBit))
			{
				if(m_Settings && m_Settings->Alert())
				{
					//::CloseProgress();
					//m_Notifications |= CMF_NOTIFY_NOMAIL;
					m_Notifier->AddNotifications( CMF_NOTIFY_NOMAIL );
				}
			}
			break;

		case TRUE:
			//
			// Check to see if all mail was filtered directly to the
			// trash.  If so, don't do any alerting...
			//

			//the second condition happens if we dont have any filters
			if ( (m_nDownloadCount != m_nTrashedMessageCount) || (m_nDownloadCount == 0) )
			{
				//
				// Set the main window icon to new mail icon if it's 
				// minimized or Version 4 with icon on the task bar
				//
				if (AfxGetMainWnd()->IsIconic() || IsVersion4())
				{
					((CMainFrame*)AfxGetMainWnd())->SetIcon(TRUE, m_nDownloadCount - m_nTrashedMessageCount);
				}

				if (bOpenInbox)
				{
					//
					// Auto-open the Inbox window
					//
					CTocDoc* pInTOC = ::GetInToc();
					if (pInTOC)
					{
						if( m_bOpenInMailbox) //if (GetIniShort(IDS_INI_OPEN_IN_MAILBOX))
							pInTOC->DisplayBelowTopMostMDIChild(); //pInTOC->Display();
							
						//
						// If the mailbox was just opened or was already opened, then
						// be sure to scroll to the "First Unread Message of the Last
						// Unread Block" ... you know, Fumlub.
						// 
						CTocView* pTocView = pInTOC->GetView();
						if (pTocView)
							pTocView->Fumlub();
					}
				}

				if(m_Settings && m_Settings->Sound())
				{
					//m_Notifications |= CMF_PLAY_SOUND;  //s_bPlaySound = TRUE;
					m_Notifier->AddNotifications( CMF_PLAY_SOUND );
				}

				if(m_Settings && m_Settings->Alert())
				{
					//m_Notifications |= CMF_ALERT_NEWMAIL; //s_bAlertNewMail = TRUE;
					m_Notifier->AddNotifications( CMF_ALERT_NEWMAIL );
				}
					
			}
			break;

		case -1:
			//
			// No alerting necessary since we haven't yet downloaded
			// any mail.
			//
			break;

		default:
			ASSERT(0);
			break;
		}

		m_nTrashedMessageCount = 0;
		m_nDownloadCount = 0;
	}
	
	
	return TRUE;
}
    


//INI settings
/*
IDS_INI_FIRST_UNREAD_NORMAL
IDS_INI_USE_POP_LAST
IDS_INI_LEAVE_MAIL_ON_SERVER
IDS_INI_SEND_ON_CHECK
IDS_INI_USE_POP_SEND
IDS_INI_POP_ACCOUNT
IDS_INI_BAD_PASSWORD_STRING
IDS_ERR_TRANSFER_MAIL
IDS_INI_LEAVE_MAIL_ON_SERVER
IDS_INI_SAVE_PASSWORD
IDS_INI_SAVE_PASSWORD_TEXT
IDS_INI_NET_IMMEDIATE_CLOSE
IDS_INI_CONNECT_OFFLINE
IDS_INI_AUTH_KERB
IDS_INI_AUTH_APOP
IDS_INI_AUTH_RPA
IDS_INI_ALERT
IDS_INI_OPEN_IN_MAILBOX
IDS_INI_SOUND
IDS_INI_ALERT
IDS_KERB_FAKE_PASSWORD
IDS_INI_KERBEROS_SET_USERNAME
IDS_INI_KERB_SERVICE
IDS_INI_KERB_REALM
IDS_INI_KERB_FORMAT
IDS_INI_NEW_MAIL_SOUND
*/
 




