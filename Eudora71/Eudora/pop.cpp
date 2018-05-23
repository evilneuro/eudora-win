// pop.cpp
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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

#include <mmsystem.h>

#include "resource.h"

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

#include "trnslate.h"
#include "JunkMail.h"

//AKN:UsgStats
// Usage Statistics
#include "StatMng.h"
//AKN:UsgStats

//	Defines and other Constants	=============================
#include "DebugNewHelpers.h"

//	Externals	=============================================



//	Globals	=================================================
JJFile*			g_pPOPInboxFile 		= NULL;
CPOPSession*	gPOP			= NULL;  //FORNOW


BOOL 	g_bHasAttachment = FALSE;
long 	g_lEstimatedMsgSize;
BOOL	g_bAttachmentDecodeEnabled = TRUE;
long	g_lBadCoding;

int		g_nAttachmentCount = 0;
/*
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

CPOP::CPOP(const char* strPersona)
	: m_strPersona(strPersona), m_bGotNewMail(true),
		m_ulBitFlags(kRetrieveNewBit), m_Settings(NULL), m_bEscapePressed(false)
{ 
	m_bOpenInMailbox = GetIniShort(IDS_INI_OPEN_IN_MAILBOX)?true:false;
	m_Notifier = DEBUG_NEW QCPOPNotifier(1);
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

	//
	// Remember that m_bGotNewMail is really a tri-state variable with values
	// of -1, FALSE, and TRUE.
	//
	BOOL bResult = FALSE;
	if(m_bGotNewMail == TRUE)
	{
		bDone = DoGetPOPMailFromSpool(bCrashRecovery);

		// Turn off redraw.  Otherwise, transfers caused by junking or filters will 
		// cause screen flicker.
		if (pInTocView)
		{
			pInTocView->m_SumListBox.SetRedraw(FALSE);
		}

		if (bFetching && !(m_ulBitFlags & kFetchHeadersBit) &&
			UsingFullFeatureSet() && GetIniShort(IDS_INI_USE_JUNK_MAILBOX))
		{
			DoProcessJunkMail_();
		}
		if (bFetching && !(m_ulBitFlags & kFetchHeadersBit) && GetIniShort(IDS_INI_FILTER_INCOMING_MAIL))
			bResult = DoFilterMail_();

		// Turn redraw back on.
		if (pInTocView)
		{
			pInTocView->m_SumListBox.SetRedraw(TRUE);
		}
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

	CSumList &		listInSums = pInTOC->GetSumList();

	//
	// Keep track of the last message in the In mailbox so that we can
	// count how many new messages have arrived during this mail check.
	//
	m_LastMessagePos = listInSums.GetTailPosition();
	
	
	//
	// FORNOW, this is really ugly.  We're setting a global pointer to
	// point to a stack-based object!!!!!  UGLY, UGLY, UGLY, UGLY, UGLY,
	// UGLY....  WKS 97.06.27.
	//
	JJFile MBox;
	g_pPOPInboxFile = &MBox;
	if (FAILED(g_pPOPInboxFile->Open(pInTOC->GetMBFileName(), O_RDWR | O_APPEND | O_CREAT)))
	{
		g_pPOPInboxFile = NULL;
		return FALSE;
	}
	        	
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
	CObArray	oaABHashes;

	if (nTotalMsgs)
	{
		// There are spool files to process - setup the progress window string
		::CountdownProgress(CRString(IDS_PREPARING_TO_DISPLAY), nTotalMsgs);

		// Generate the hashes that will be used by junk processing
		CFilter::GenerateHashes(&oaABHashes);
	}

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
		POSITION posLast = listInSums.GetTailPosition();

		// 
		// Lets get the message...  A status greater than zero indicates
		// a successful download, a status of zero indicates no message
		// was downloaded, and a status less than zero indicates an error.
		//

		CMsgRecord* pMsgRecord = pSpoolInfo->GetMsgRecord();

		if (!pMsgRecord)
		{
			// This is the case where an incoming spool message was found on startup, but it's
			// not in the LMOS database (maybe it already got deleted from the server).
			// Mark it to decode attachments and give a positive message size so it doesn't
			// look like a partial message (although it's possible that it is just a stub).
			g_bAttachmentDecodeEnabled = TRUE;
			g_lEstimatedMsgSize = 1024;
		}
		else
		{
			//Don't decode attachments if we skipped a message and only showing either headers or a stub
			g_bAttachmentDecodeEnabled = (pMsgRecord->GetSkippedFlag() == LMOS_SKIP_MESSAGE)?FALSE:TRUE;
			
			if(g_bAttachmentDecodeEnabled)
				g_lEstimatedMsgSize = pMsgRecord->GetEstimatedSize();
			else
				g_lEstimatedMsgSize = -(pMsgRecord->GetEstimatedSize());
		}
		
		int nStatus = FetchMessageTextFromSpool(pInTOC, pSpoolInfo);

		if (nStatus >= 0)
		{
			if (bCrashRecovery && pMsgRecord)
			{
				// Set the Retrieval flag to don't retrieve (Nget)
				pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE);

				// Set the has been read flag...
				pMsgRecord->SetReadFlag(LMOS_HASBEEN_READ);
			}
			
			//
			// Lets get the last summary prior to our get message...
			// this is the last position BEFORE the series of gets..
			//
			CSummary* pLastSummary = NULL;
			if (posLast == NULL)
				posLast = listInSums.GetHeadPosition();
			else
				pLastSummary = listInSums.GetNext(posLast);

			// CHANGE MAY HAVE TO BE MADE BELOW - SAGAR  - FOR EMSR_DELETE



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
				pLastSummary = listInSums.GetNext(posLast);
				if (pLastSummary)
				{
					if (pMsgRecord)
						pLastSummary->SetHash(pMsgRecord->GetHashValue());
					else
					{
						// Glean hash from spool base name
						CString FullSpoolName(pSpoolInfo->GetFileName());
						const char* Base = strrchr(FullSpoolName, '\\');
						if (Base)
							Base++;
						else
							Base = FullSpoolName;

						char BaseName[32];
						strncpy(BaseName, Base, sizeof(BaseName));
						BaseName[sizeof(BaseName) - 1] = 0;
						char* Dot = strrchr(BaseName, '.');
						if (Dot)
							*Dot = 0;

						pLastSummary->SetHash(strtoul(BaseName, NULL, 16));
					}

					GetTransMan()->CallJunkTranslators(pLastSummary, EMSFJUNK_SCORE_ON_ARRIVAL, &oaABHashes, g_pPOPInboxFile);
				}
			}

			//Remove the file as we are done with it
			pSpoolInfo->SetSpoolStatus(SS_REMOVED);
			SpoolMgr->m_In->RemoveFromSpool(pSpoolInfo);
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
		// We previously only wrote the TOC if it was modified.
		// We now always write the TOC, because if we got this far some mail was
		// being processed. If we don't write the TOC now, then the TOC could
		// look out of date in comparison to the MBX the next time we read them.
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
	extern bool g_bMessageToBeDeleted ;
	g_bMessageToBeDeleted = FALSE; //reset this for every message.
	
	CString strSpoolFileName = pSpoolInfo->GetFileName();
	ASSERT( ::FileExistsMT(strSpoolFileName));
	
	JJFileMT *	pSpoolFile = pSpoolInfo->GetFile();
	JJFile		spoolFile;

	if (pSpoolFile)
	{
		// Reset our file pointer and buffer so that we can read from the beginning
		pSpoolFile->Reset();
	}
	else
	{
		if (spoolFile.Open(strSpoolFileName, O_RDONLY) != S_OK)
		{
			ASSERT(0);
			TRACE("Fatal error: Could not open Spool file %s\n", strSpoolFileName);
			return -1;
		}

		pSpoolFile = &spoolFile;
	}
	
	FileLineReader *pFileLineReader = DEBUG_NEW FileLineReader;
	pFileLineReader->SetFile(pSpoolFile);

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

	// We're done reading from our spool file so close it. This would be done for us anyway
	// in the JJFileMT destructor when spoolFile is used, but is absolutely necessary
	// when pSpoolFile is used from the pSpoolInfo (so that we can later delete the file).
	pSpoolFile->Close();
	
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

    if( g_bMessageToBeDeleted )
	{
		//plugin wants this message to be nuked.
		//so  just dont write anything to the TOC file.
		//Also truncate what we have written to MBX as if 
		//we never got this message. To make Eudora happy ,
		//make sure that the TOC file date is later 
		//than the MBX file date. So we set the modified 
		//flag for TOC

		g_pPOPInboxFile->Truncate(hd.lDiskStart);
		pInTOC->SetModifiedFlag(TRUE);
        return 1; //special case of status where its succeeded
	}
		// Resize the mailbox to the end of the successfully read 
	// message.
	//
	{
		long lCurrent = 0;
		g_pPOPInboxFile->Tell(&lCurrent);
		ASSERT(lCurrent >= 0);
		g_pPOPInboxFile->Truncate(lCurrent);
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
	{
		pSummary->SetFlag(MSF_HAS_ATTACHMENT);

		//AKN:UsgStats
		// Update the Usage Statistics for Attachment received
		UpdateNumStat(US_STATRECEIVEDATTACH,(short)g_nAttachmentCount,pSummary->m_Seconds + (pSummary->m_TimeZoneMinutes * 60));
		//AKN:UsgStats
	}

	if (hd.isDispNotifyTo)
		pSummary->SetFlag(MSF_READ_RECEIPT);

	//AKN:UsgStats
	// Update the Usage Statistics for Received Mail
	UpdateNumStat(US_STATRECEIVEDMAIL,1,pSummary->m_Seconds + (pSummary->m_TimeZoneMinutes * 60));
	//AKN:UsgStats

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
	MIMEState ms(pLineReader,0);

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
			if (! ms.Init(&hd,objArrayMimeStates.GetSize()) || objArrayMimeStates.Add(&ms) < 0)
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
		::PutDebugLog(DEBUG_MASK_RCVD, szBuffer);

		//
		// We don't use the Status: header to determine if a big
		// message has already been skipped.  That was the old way.
		// The new way is determined elsewhere with LMOS record flags.
		//
		return 1;
	}

	wsprintf(szBuffer, CRString(IDS_DEBUG_BAD_RETRIEVE), hd.who, hd.subj);
	::PutDebugLog(DEBUG_MASK_RCVD, szBuffer);
	return -1;
}


////////////////////////////////////////////////////////////////////////
// DoProcessJunkMail_ [private]
//
// The main junk mail action driver.  FORNOW, this probably doesn't belong
// in the POP module since it operates directly on the TOCs.
////////////////////////////////////////////////////////////////////////
void CPOP::DoProcessJunkMail_()
{
	//
	// Remember that m_bGotNewMail is really a tri-state variable with values
	// of -1, FALSE, and TRUE.
	//
	if (m_bGotNewMail != TRUE)
		return;

	//
	// Get the In and Junk mailboxes.
	//
	CTocDoc* pInTOC = ::GetInToc();
	CTocDoc* pJunkTOC = ::GetJunkToc();
	if (NULL == pInTOC || NULL == pJunkTOC)
	{
		ASSERT(0);
		return;
	}

	CSumList &		listInSums = pInTOC->GetSumList();

	//
	// Save the number of messages in Junk.
	//
	int nOriginalJunkCount = pJunkTOC->NumSums();

	//
	// Determine the position of the first newly downloaded message
	// in the Inbox.
	//
	POSITION pos = NULL;
	if (! m_LastMessagePos)
		pos = listInSums.GetHeadPosition();
	else
		listInSums.GetNext(pos = m_LastMessagePos);
		
	// 
	// Count how many messages there are to process so that we can do
	// a proper progress display
	//
	int nNumMessagesToProcess = 0;
	{
		for (POSITION posCount = pos; posCount; nNumMessagesToProcess++)
			listInSums.GetNext(posCount);
	}
	
	m_bNotifyNewMail = FALSE;
	if (nNumMessagesToProcess)
	{
		//
		// If we get this far, we have some messages to process, so
		// get to it...
		//
		m_nDownloadCount = nNumMessagesToProcess;
		::CountdownProgress(CRString(IDS_JUNK_MESSAGES_LEFT), nNumMessagesToProcess);
		::Progress(-1, NULL, -1);

		CObArray	*poaABHashes = NULL;
		// If the address book is to be used as a whitelist, hash the address book.
		if (GetIniShort(IDS_INI_ADDBOOK_IS_WHITELIST))
		{
			poaABHashes = DEBUG_NEW CObArray;
			CFilter::GenerateHashes(poaABHashes);
		}
		
		while (pos)
		{
			if (EscapePressed())
				break;
			
			CJunkMail::ProcessOne(listInSums.GetNext(pos), poaABHashes);

			::DecrementCountdownProgress();
		}

		delete poaABHashes;
	}

	if (pInTOC->IsModified())
		pInTOC->Write();

	//
	// Save the number of messages junked by this pass.
	//
	m_nJunkedMessageCount = pJunkTOC->NumSums() - nOriginalJunkCount;

	//Update the notifier
	m_Notifier->AddJunkCount(m_nJunkedMessageCount);

	// Open the junk mailbox if there was junk and the user wants mailboxes
	// to open and junk mail is considered unread.
	if ((m_nJunkedMessageCount > 0) &&
		GetIniShort(IDS_INI_OPEN_IN_MAILBOX) &&
		!GetIniShort(IDS_INI_JUNK_NEVER_UNREAD))
	{
		pJunkTOC->DisplayBelowTopMostMDIChild();
	}
	
	return;
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

	CSumList &		listInSums = pInTOC->GetSumList();

	//
	// Save the number of messages in the Trash.
	//
	int nOriginalTrashCount = pTrashTOC->NumSums();

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
	// Determine the position of the first newly downloaded message
	// in the Inbox.
	//
	POSITION pos = NULL;
	if (! m_LastMessagePos)
		pos = listInSums.GetHeadPosition();
	else
		listInSums.GetNext(pos = m_LastMessagePos);
		
	// 
	// Count how many messages there are to filter so that we can do
	// a proper progress display
	//
	int nNumMessagesToFilter = 0;
	{
		for (POSITION posCount = pos; posCount; nNumMessagesToFilter++)
			listInSums.GetNext(posCount);
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

		CObArray	oaABHashes;

		// Generate the address book hashes once for the entire filtering loop
		CFilter::GenerateHashes(&oaABHashes);
		
		while (pos)
		{
			if (EscapePressed())
				break;
			int nFilterAction = filt.FilterOne(listInSums.GetNext(pos), WTA_INCOMING, &oaABHashes);
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

	// Note that as with m_bGotNewMail below m_bNotifyNewMail is
	// now a tri-valued variable.  This is necessary so that if a
	// connection fails we do no notification at all. -dwiggins
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
			// trash or was junked.  If so, don't do any alerting...
			//

			//the second condition happens if we dont have any filters
			if ((m_Notifier->m_nDownloadCount != (m_Notifier->m_nTrashedMessageCount + 
												  m_Notifier->m_nJunkedMessageCount)) ||
				(m_Notifier->m_nDownloadCount == 0))
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
						pInTOC->SetNeedsFumlub();
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
		m_nJunkedMessageCount = 0;
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
 




