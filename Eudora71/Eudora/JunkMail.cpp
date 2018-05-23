// JunkMail.cpp
//
// Copyright (c) 2003 by QUALCOMM, Incorporated
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

#include "JunkMail.h"

#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"

#include "tocdoc.h"
#include "summary.h"
#include "msgdoc.h"
#include "eudora.h"
#include "Trnslate.h"
#include "filtersd.h"
#include "address.h"
#include "nickdoc.h"
#include "helpxdlg.h"
#include "namenick.h"
#include "progress.h"
#include "ImapSum.h"
#include "StatMng.h"

#include "DebugNewHelpers.h"


extern QCMailboxDirector		g_theMailboxDirector;


//
//	ExtractFromFieldFromSummary()
//
//	Take a given summary and extract the full From: field (not the trimmed
//	down version in the summary's m_From field.
//
//	Parameters
//		pSum[in] - Summary to extract the field from.
//		strFrom[out] - The extracted From: field.
//
void ExtractFromFieldFromSummary(CSummary *pSum, CString &strFrom)
{
	if (!pSum || !pSum->m_TheToc)
	{
		return;
	}

	if (pSum->m_TheToc->IsImapToc() && pSum->IsNotIMAPDownloadedAtAll())
	{
		// IMAP is a special case if we haven't already downloaded the body
		// (since the summary doesn't keep the From: header).
		pSum->IMAPGetFromHeader(strFrom);
		int			 iPos = strFrom.Find("From: ");
		if (iPos >= 0)
		{
			strFrom = strFrom.Right(strFrom.GetLength() - (iPos + strlen("From: ")));
		}
	}
	else
	{
		CMessageDoc		*pDoc = NULL;
		char			*szText = NULL;
		bool			 bCreatedDoc = false;
		bool			 bTextWasAllocated = false;
		int				 iPos = -1;

		// Get the document associated with the summary.
		pDoc = pSum->GetMessageDoc(&bCreatedDoc);
		if (!pDoc)
		{
			return;
		}

		// Get the document's text.
		szText = pDoc->GetText();
		if (!szText)
		{
			szText = pDoc->GetFullMessage();
			bTextWasAllocated = true;
		}

		strFrom = szText;

		// Work on the headers only.
		iPos = strFrom.Find("\r\n\r\n");
		if (iPos >= 0)
		{
			strFrom = strFrom.Left(iPos + 1);
		}

		// Find the From: header and extract the address.
		iPos = strFrom.Find("\r\nFrom: ");
		if (iPos >= 0)
		{
			strFrom = strFrom.Right(strFrom.GetLength() - (iPos + strlen("\r\nFrom: ")));
			iPos = strFrom.Find("\r\n");
			if (iPos >= 0)
			{
				strFrom = strFrom.Left(iPos);
			}
		}

		// Clean up anything we created.
		if (bCreatedDoc)
		{
			pSum->NukeMessageDocIfUnused(pDoc);
		}
		if (bTextWasAllocated)
		{
			delete [] szText;
		}
	}
}

//
//	CJunkMail::CJunkMail()
//
CJunkMail::CJunkMail()
{
}

//
//	CJunkMail::~CJunkMail()
//
CJunkMail::~CJunkMail()
{
}


//
//	CJunkMail::IsWhitelisted()
//
//	Indicates whether or not the address in strFrom is in the specified
//	address book hash.
//
//	Parameters:
//		strFrom[in] - String containing the full From: field containing the
//			address to check for.
//		poaABHashes[in] - Array of hashes of addresses in the address book.
//
//	Return: true if strFrom matches an address book entry, false otherwise.
//
bool CJunkMail::IsWhitelisted(CString &strFrom, CObArray *poaABHashes)
{
	if (strFrom.IsEmpty() || !poaABHashes)
	{
		return false;
	}

	bool			 bMatches = false;

	// Extract the address (in lower case).
	strFrom.MakeLower();
	char		*szAddress = strFrom.GetBuffer(0);
	StripAddress(szAddress);

	// See if the From: address matches an address book entry.
	if (CFilter::MatchesHash(poaABHashes, szAddress))
	{
		bMatches = true;
	}

	return bMatches;
}

//
//	CJunkMail::IsWhitelisted()
//
//	Extracts the From: field from the CSummary and indicates whether or
//	not the sender is in the specified address book hash.
//
//	Parameters:
//		pSum[in] - Pointer to summary of message whose From: field we are checking.
//		poaABHashes[in] - Array of hashes of addresses in the address book.
//
//	Return: true if From: field matches an address book entry, false otherwise.
//
bool CJunkMail::IsWhitelisted(CSummary *pSum, CObArray *poaABHashes)
{
	if (!pSum || !poaABHashes)
	{
		return false;
	}

	CString			 strFrom;

	ExtractFromFieldFromSummary(pSum, strFrom);

	return IsWhitelisted(strFrom, poaABHashes);
}

//
//	CJunkMail::IsWhitelisted()
//
//	Extracts the From: field from the CImapSum and indicates whether or
//	not the sender is in the specified address book hash.
//
//	Parameters:
//		pSum[in] - Pointer to summary of message whose From: field we are checking.
//		poaABHashes[in] - Array of hashes of addresses in the address book.
//
//	Return: true if From: field matches an address book entry, false otherwise.
//
bool CJunkMail::IsWhitelisted(CImapSum *pSum, CObArray *poaABHashes)
{
	if (!pSum || !poaABHashes)
	{
		return false;
	}

	CString			 strFrom;
	pSum->GetRawFrom(strFrom);
	return IsWhitelisted(strFrom, poaABHashes);
}

//
//	CJunkMail::AddMsgSenderToAddressBook()
//
//	Take the given CSummary and add an address book entry for the sender
//	of that message.
//
//	Parameters
//		pSum[in] - CSummary to extract the field from.
//
void CJunkMail::AddMsgSenderToAddressBook(CSummary *pSum)
{
	if (!pSum)
	{
		return;
	}

	// Get From: field.
	CString			 strAddress;
	ExtractFromFieldFromSummary(pSum, strAddress);

	if (!strAddress.IsEmpty())
	{
		AddAddressToAddressBook(strAddress);
	}
}

//
//	CJunkMail::AddMsgSenderToAddressBook()
//
//	Take a given CImapSum and add an address book entry for the sender
//	of that message.
//
//	Parameters
//		pSum[in] - CImapSum to extract the field from.
//
void CJunkMail::AddMsgSenderToAddressBook(CImapSum *pSum)
{
	if (!pSum)
	{
		return;
	}

	CString			 strFrom;
	pSum->GetRawFrom(strFrom);
	AddAddressToAddressBook(strFrom);
}

//
//	CJunkMail::AddAddressToAddressBook()
//
//	Take the given address, create a nickname for it and add it to
//	the address book.
//
//	Parameters
//		strAddress[in] - Address to add to the address book.
//
void CJunkMail::AddAddressToAddressBook(CString &strAddress)
{
	if (strAddress.IsEmpty())
	{
		return;
	}

	// Find the specified nickname file.
	CString			 strNicknameFile;
	GetIniString(IDS_INI_NONJUNK_ADDBOOK, strNicknameFile);
	CNicknameFile	*pNicknameFile = g_Nicknames->GetFirstNicknameFile();
	CNicknameFile	*pNicknameFileFirst = NULL;
	while (pNicknameFile)
	{
		if (pNicknameFile->m_ReadOnly || pNicknameFile->m_Invisible)
		{
			// Can't do anything with readonly or invisible.
			pNicknameFile = g_Nicknames->GetNextNicknameFile();
		}
		else if (strNicknameFile.Compare(pNicknameFile->GetName()) == 0)
		{
			// We have found our match.
			break;
		}
		else
		{
			// Remember the first nickname file in case we don't find a match.
			if (!pNicknameFileFirst)
			{
				pNicknameFileFirst = pNicknameFile;
			}
			// No match: find the next nickname file.
			pNicknameFile = g_Nicknames->GetNextNicknameFile();
		}
	}

	// If we never found a match go ahead and add the address to the first
	// nickname file.
	if (!pNicknameFile)
	{
		pNicknameFile = pNicknameFileFirst;
	}

	if (pNicknameFile)
	{
		CString		 strNickname;
		CString		 strFullName;
		CString		 strFirstName;
		CString		 strLastName;

		// We need not check for duplicate entries.  If we get this far
		// we assume the calling function has already checked 
		// Hmmm... At this point we should probably check and see if any
		// entries in this nickname file match the sender's address,
		// otherwise unjunking multiple messages from the sender will
		// generate multiple address book entries. -dwiggins
		
		strNickname = strAddress;

		// Trim off the email address.
		strNickname = strNickname.SpanExcluding("<");

		// Strip (illegal) leading/trailing whitespace, if any
		strNickname.TrimLeft();
		strNickname.TrimRight();

		// Strip off leading and trailing quotes.
		int			 iPos = strNickname.Find("\"");
		if (iPos >= 0)
		{
			strNickname = strNickname.Right(strNickname.GetLength() - (iPos + 1));
			iPos = strNickname.Find("\"");
			if (iPos >= 0)
			{
				strNickname = strNickname.Left(iPos);
			}
		}

		// Create a nickname.
		MakeAutoNickname(strNickname,
						  strFullName,
						  strFirstName,
						  strLastName);

		// At the least we need a nickname.  If no name was provided use the address.
		if (strNickname.IsEmpty())
		{
			strNickname = strAddress;
		}

		// Make nickname unique.  I am being lazy for the moment.
		// This is an unusual case: the user is whitelisting their address
		// book and adding senders of unjunked messages to their address
		// book but they already have an address book entry with the name
		// of the sender but not their email address.  Maybe in this case
		// we should add the address to the entry?  Dunno.  Don't care that
		// much right now.  Fix it before we go beta. -dwiggins
		while (pNicknameFile->FindNickname(strNickname))
		{
			strNickname += "_";
		}

		// Extract address.
		strAddress = StripAddress(strAddress.GetBuffer(0));

		// Add the nickname.
		AddNickname(pNicknameFile,
					strNickname,
					strFullName,
					strFirstName,
					strLastName,
					strAddress);
	}
}

//
//	CJunkMail::ProcessOne()
//
//	Process the message specified by pSum.
//
//	Parameters:
//		pSum[in] - Pointer to summary of message to test for junk.
//		poaABHashes[in] - Array of hashes of addresses in the address book.
//			If this is NULL don't do any whitelisting.
//
//	Return: true if message is considered junk, false otherwise.
//
bool CJunkMail::ProcessOne(CSummary *pSum, CObArray *poaABHashes)
{
	if (!pSum)
	{
		return false;
	}

	Progress(-1, pSum->MakeTitle());

	if (poaABHashes && IsWhitelisted(pSum, poaABHashes))
	{
		// Message sender matches address book entry: not junk.
		pSum->SetJunkScore(0);
		pSum->SetJunkPluginID(specialJunkIDWhitelist);
		
		// Let stats know
		UpdateNumStat(US_STATWHITELIST,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));

		return false;
	}

	// See if this message qualifies as junk.
	if (pSum->GetJunkScore() >= (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK))
	{
		CTocDoc		*pInTOC = ::GetInToc();
		CTocDoc		*pJunkTOC = ::GetJunkToc();
		if (!pInTOC || !pJunkTOC)
		{
			ASSERT(0);
			return false;
		}

		// Let stats know
		UpdateNumStat(US_STATSCOREDJUNK,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));

		// If user is deleting junk from the server, mark this message for deletion.
		if (GetIniShort(IDS_INI_DELETE_FETCHED_JUNK))
		{
			pSum->SetServerStatus(ID_MESSAGE_SERVER_DELETE);
		}

		if (GetIniShort(IDS_INI_JUNK_IGNORE_DATE))
		{
			// User is ignoring date on incoming junk (accounting for time zone
			// since CSummary::SetDate() factors it in).
			pSum->SetDate(pSum->m_lArrivalSeconds - (pSum->m_TimeZoneMinutes * 60), TRUE/*bJunk*/);
		}

		// Transfer the message from the In mailbox to the Junk mailbox.
		pInTOC->Xfer(pJunkTOC, pSum);
		return true;
	}
	else
	{
		// Let stats know
		UpdateNumStat(US_STATSCOREDNOTJUNK,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));
	}


	return false;
}

//
//	CJunkMail::ReprocessOne()
//
//	Process the message specified by pSum.
//
//	Parameters:
//		pSum[in] - Pointer to summary of message to test for junk.
//		pFilt[in] - Pointer to filter object for messages that will be
//			filtered during reprocessing.
//		poaABHashes[in] - Address Book hashes
//
//	Return: true if message is considered junk, false otherwise.
//
bool CJunkMail::ReprocessOne(CSummary *pSum, CFilterActions *pFilt, CObArray *poaABHashes)
{
	if (!pSum || !pFilt)
	{
		return false;
	}

	// See if this message qualifies as junk.
	GetTransMan()->CallJunkTranslators(pSum, EMSFJUNK_RESCORE | EMSFJUNK_USER_INITIATED, poaABHashes);

	CTocDoc		*pSrcTOC = pSum->m_TheToc;
	CTocDoc		*pDestTOC = NULL;

	if (pSum->GetJunkScore() >= (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK))
	{
		// This message is junk.  If we are holding junk in the Junk mailbox
		// set this as the destination.
		if (GetIniShort(IDS_INI_USE_JUNK_MAILBOX))
		{
			pDestTOC = GetJunkToc();
		}

		// If user is deleting junk from the server, mark this message for deletion.
		if (GetIniShort(IDS_INI_DELETE_FETCHED_JUNK))
		{
			pSum->SetServerStatus(ID_MESSAGE_SERVER_DELETE);
		}
	}
	else
	{
		// If user is deleting junk from the server, mark this message as leave on server.
		if (GetIniShort(IDS_INI_DELETE_FETCHED_JUNK))
		{
			pSum->SetServerStatus(ID_MESSAGE_SERVER_LEAVE);
		}

		// This message is not junk.  If the message is in the Junk mailbox
		// run it through the filters.
		if (pSum->m_TheToc == GetJunkToc())
		{
			// Run message through filters.
			int nFilterAction = pFilt->FilterOne(pSum, WTA_INCOMING, poaABHashes);
			// WARNING: If the filter action was a transfer don't trust pSum beyond this point.
			// If the transfer was to an IMAP mailbox this object will be deleted and we have
			// no way of knowing that.
			if (!(nFilterAction & FA_TRANSFER))
			{
				// Filters did not move the message: destination is the In mailbox.
				pDestTOC = GetInToc();
			}
		}
	}

	if (pSrcTOC && pDestTOC && (pSrcTOC != pDestTOC))
	{
		// This message is being relocated.

		// Transfer the message.
		pSrcTOC->Xfer(pDestTOC, pSum);
		return true;
	}

	return false;
}

//
//	CJunkMail::DeclareJunk()
//
//	Declare the message specified by pSum as either junk or not junk.
//	Move the message to the appropriate mailbox and inform the plugins.
//
//	Parameters:
//		pSum[in] - Pointer to summary of message to test for junk.
//		bJunk[in] - Flag to indicate new junk status of this message.
//			If true message will now be considered junk, if false not junk.
//		bAddNotJunkedSendersToAB[in] - Flag that indicates user is
//			Not Junk'ing and wants to have Not Junk'ed senders added to the
//			address book.
//		poaABHashes[in] - Pointer to object array of hashes of address book
//			contents.  It is important that the caller create this array
//			because hashing the entire address book is potentially time
//			consuming and should only be done once and used for all messages
//			being Declared Junk or Not Junk.
//		pFilt[in] - Pointer to filter object for messages that will be
//			filtered during reprocessing.  If this is NULL then assume this
//			was called from a filter action.
//
//	Return:
//		The new summary object.  Note that this value is not yet generated
//		when transferred via a filter.  We really should generate it there so
//		we can fully support Not Junk in the Find Messages window which needs
//		to be updated after a transfer.  We need to change FilterOne() and
//		FilterMsg() to give us back the new CSummary (if there is one).
//
CSummary *CJunkMail::DeclareJunk( CSummary *pSum, bool bJunk, bool bAddNotJunkedSendersToAB,
								  CObArray *poaABHashes, CFilterActions *pFilt )
{
	CSummary		*pNewSum = pSum;

	if (!pSum || !pSum->m_TheToc)
	{
		return pNewSum;
	}

	CTocDoc			*pTocSrc = pSum->m_TheToc;
	
	// Worry about stats first
	bool bWasJunk = pSum->GetJunkScore() >= (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK);
	if (bWasJunk != bJunk)
	{
		switch (pSum->GetJunkPluginID())
		{
			case specialJunkIDManual:	// not automatically scored
				// since no plugins messed with it, we don't add it to any
				// of the "false" lists
				break;
				
			case specialJunkIDWhitelist: // whitelisted
				if (bJunk)
				{
					// whitelisted message was really junk.  Count this.
					UpdateNumStat(US_STATFALSEWHITELIST,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));
				}
				break;
				
			case specialJunkIDScoredZero:	// plugins all said zero
			default:	// some plugin gave a nonzero score
				if (bJunk)
				{
					// False negative; we scored it below the threshhold, but it
					// really was above
					UpdateNumStat(US_STATFALSENEGATIVES,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));
				}
				else
				{
					// False positive; we scored it junk, but the user says not
					UpdateNumStat(US_STATFALSEPOSITIVES,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));
				}
				break;
		}
	}

	// Get the destination TOC.  If junking this will be Junk mailbox,
	// if unjunking this will be the In mailbox.
	CTocDoc		*pTocDest = NULL;
	if (bJunk)
	{
		pTocDest = ::GetJunkToc();
	}
	else
	{
		pTocDest = ::GetInToc();
	}
	if (!pTocDest)
	{
		return pNewSum;
	}
	
	// Get the mailbox command for the destination TOC.
	QCMailboxCommand	*pCommand = g_theMailboxDirector.FindByPathname((const char*)pTocDest->GetMBFileName());
	if (!pCommand)
	{
		return pNewSum;
	}

	// Save the current junk values (score, plugin, manual) in case the user undoes the Junk/Not Junk.
	pSum->SaveJunkValues();

	// Inform the plugins that the user considers this message either junk or not junk.
	// Call the plugin's "declare junk"/"declare not junk" method.
	long			lContext = bJunk ? EMSFJUNK_MARK_IS_JUNK : EMSFJUNK_MARK_NOT_JUNK;
	if (pFilt)
	{
		// If filters specified this was user initiated.
		lContext |= EMSFJUNK_USER_INITIATED;
	}
	GetTransMan()->CallJunkTranslators(pSum, lContext, poaABHashes);

	// Reset the ID of the plugin generating the score to indicate this
	// message was categorized by the user.
	pSum->SetJunkPluginID(specialJunkIDManual);

	if (bJunk)
	{
		// If the user specified junk score is greater than 0, set the score
		// to the user specified junk score.  (A score of 0 indicates score
		// should be left alone.)
		short		sJunkScore = GetIniShort(IDS_INI_MANUAL_JUNK_SCORE);
		if (sJunkScore > 0)
		{
			pSum->SetJunkScore((unsigned char)sJunkScore);
		}

		// If user is deleting junk from the server, mark this message for deletion.
		if (GetIniShort(IDS_INI_DELETE_FETCHED_JUNK))
		{
			pSum->SetServerStatus(ID_MESSAGE_SERVER_DELETE);
		}

		// If this message is not currently in the Junk mailbox move it there.
		if (pSum->m_TheToc != GetJunkToc())
		{
			if (pFilt)
			{
				// If we have a filter object then this was a manual operation.
				pNewSum = pSum->m_TheToc->Xfer(GetJunkToc(), pSum);
				pTocSrc->MailboxChangedByHuman();
			}
			else
			{
				// No filter object means this was a filter operation.  Do the transfer
				// directly (as is done with manual transfers) and keep the new summary
				// object (which is needed for filter reporting).
				pNewSum = pSum->m_TheToc->Xfer(GetJunkToc(), pSum);
			}
			// Note that the above transfer was a junk-related action.
			pSum->SetLastXferWasJunkAction(true);
		}
	}
	else
	{
		// Set the score to the user specified junk score.
		pSum->SetJunkScore((unsigned char)GetIniShort(IDS_INI_MANUAL_NOT_JUNK_SCORE));

		// If user is deleting junk from the server, mark this message as leave on server.
		if (GetIniShort(IDS_INI_DELETE_FETCHED_JUNK))
		{
			pSum->SetServerStatus(ID_MESSAGE_SERVER_LEAVE);
		}

		// If there are hashes then we need to add the sender to the address
		// book (assuming they are not already in the address book).
		// We need to do this before filtering because filtering into an IMAP
		// mailbox will delete the pSum object and we will have no way of
		// know that this has happened.
		if (bAddNotJunkedSendersToAB && !IsWhitelisted(pSum, poaABHashes))
		{
			AddMsgSenderToAddressBook(pSum);
		}

		// If the message is in the Junk folder move it out (either by
		// filtering or by direct transfer).
		if (pSum->m_TheToc == ::GetJunkToc())
		{
			int nFilterAction = 0;
			if (pFilt)
			{
				// Run this message through the filters.
				nFilterAction = pFilt->FilterOne(pSum, WTA_INCOMING, poaABHashes, true/*bNoJunkAction*/);
				// WARNING: If the filter action was a transfer don't trust pSum beyond this point.
				// If the transfer was to an IMAP mailbox this object will be deleted and we have
				// no way of knowing that.
			}
			if (!(nFilterAction & FA_TRANSFER))
			{
				// Filters did not move the message: transfer it to the In mailbox.
				pNewSum = pTocSrc->Xfer(pTocDest, pSum);

				// Note that the above transfer was a junk-related action.
				pSum->SetLastXferWasJunkAction(true);

				pTocSrc->MailboxChangedByHuman();
			}
		}
	}

	return pNewSum;
}
