// ImapFiltersd.h
//
// Contains CImapFilterActions, a subclass of CFilterActions that provides extra functions for
// performing server queries for filter matches and functions for performing IMAP specific
// filter action operations.
//
// When using this subclass call DoFiltering() or DoManualFiltering() rather than FilterOne().
//
// Copyright (c) 1997-2004 by QUALCOMM, Incorporated
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

#ifndef __IMAPFILTERSD_H
#define __IMAPFILTERSD_H


class CTocList;
class CSumList;
class CPtrUidList;

class CImapFilterActions : public CFilterActions
{
public:
	CImapFilterActions();
	~CImapFilterActions();

	// Initialization.
	BOOL			 StartFiltering();
	BOOL			 EndFiltering();

	// Public Interface:
	int				 DoFiltering(CTocDoc *pTocDoc,
								 CSumList &sumlist,
								 CString *pstrTransUIDs,
								 BOOL bDoSameServerXfers,
								 BOOL bDoIncoming,
								 BOOL bDoManual);
	void			 DoManualFiltering(CTocDoc *pTocDoc, CSummary *pSum = NULL);

	void			 SetImapMsgStatus(CSummary *pSum, unsigned char ucStatus);
	void			 ImapSetServerOpt(CSummary *pSum, unsigned int uiServerOpt);

// Internal methods.
private:
	void			 ImapMatch(CTocDoc *pTocDoc,
							   CFilter *pFilt,
							   CPtrUidList &ulstTotalUidList,
							   CSumList *pSumList,
							   CPtrUidList &ulstSubUidList);

	void			 FormulateImapSearchCriteria(LPCSTR szHeader,
												 int iVerb, 
												 LPCSTR szValue,
												 CString &strHeaderList,
												 BOOL *pbBody,
												 BOOL *pbNot);

	BOOL			 DoRemoteTransfers(CTocDoc *pTocDoc,
									   CFilter *pFilt,
									   CPtrUidList &ulstNewUidList,
									   LPCSTR szUids,
									   CSumList *pSumList = NULL);

	BOOL			 RemoveUidsFromList(CPtrUidList& ulstNewUidList, LPCSTR szUids);

	int				 PerformAction(CTocDoc *pTocDoc,
								   CFilter *pFilt,
								   CSumList& sumlist,
								   CPtrUidList &ulstTotalUidList,
								   CPtrUidList &ulstSubUidList,
								   CString *pstrTransUIDs,
								   CObArray *poaABHashes);

	void			 DoServerStatusChanges(CTocDoc *pTocDoc, CSumList &sumlist);
	void			 DoFetchRequestedMessages(CTocDoc *pTocDoc, CSumList &sumlist);

	BOOL			 IsXferFilter(CFilter* pFilt);
	BOOL			 IsOurPersonality(CTocDoc *pTocDoc, CFilter *pFilt, int iHeaderNum);
	
	void			 AddUidToStatusString(CSummary *pSum, CString& szStr);

	void			 UpdateTocAfterFiltering(CTocDoc *pTocDoc);

// Public data.
public:
	CStringList		 m_strlstFilterMboxList;	// List of remote mailboxes we transfered messages to.

	BOOL			 m_bPreFilterXferred;	// Set to TRUE if we did moves during the prefilter stage.
	BOOL			 m_bPostFilterXferred;	// Set to TRUE if we did a move during post filtering.

	// Data access.
	void			 SetMBFilename(LPCSTR pStr) {m_szMBFilename = pStr;}
	LPCSTR			 GetMBFilename() {return(LPCSTR)m_szMBFilename;}
	unsigned long	 GetHighestUidSeen() {return m_HighestUidXferred;}


// Private data:
private:
	// During filtering, accumulate uid's that need specific flags
	// changed on the server:
	CString			 m_szAnsweredUids;
	CString			 m_szReadUids;
	CString			 m_szUnreadUids;
	CString			 m_szDeletedUids;
	CString			 m_szFetchUids;

	// Store the filename name of the mailbox we're filtering FROM!!
	// Used for identifying the mailbox.
	CString			 m_szMBFilename;

	// Keep track of the largest UID we've seen during an incoming filter.
	unsigned long	 m_HighestUidXferred;

	// Keep track of new msgs that are left in inbox after xferring!
	long			 m_ulNewMsgsLeftInInbox;

	// Some INI options needed during filtering (especially during background).
	BOOL			 m_bOpenInMailbox;
	BOOL			 m_bDoFilterReport;
};

#endif // __IMAPFILTERSD_H

