// ImapPreFilterActs.H
//

#ifndef __IMAPPREFILTERACT_H
#define __IMAPPREFILTERACT_H


class CImapSumList;
class CPtrUidList;
class CTocDoc;
class CIMAP;
class CTaskInfoMT;

class CPreFilterActions : public CObject
{
public:
	CPreFilterActions  ();
	~CPreFilterActions ();

	// Initialization.
	BOOL StartFiltering(CTocDoc *pSrcToc);

	BOOL ImapEndFiltering ();
	void ClearAllLists ();

// Public Interface:
	void CreatePreFilterList(CTocDoc* pSrcToc);

// THREAD-SAFE methods:
//
	int DoXferPreFilteringMT (CPtrUidList& NewUidList);

	// STL Map version:
	//
	int DoXferPreFilteringMT (CUidMap& NewUidMap);

	void GetResultsLists (CObList& strDstList, CObList& strReportList);

	void	SetMBFilename (LPCSTR pStr)
				{m_szMBFilename = pStr; }
	LPCSTR GetMBFilename ()
				{ return (LPCSTR)m_szMBFilename; }

	unsigned long GetHighestUidSeen ()
				{ return m_HighestUidXferred; }

	// Caller would set this and clear it if it's no longer valid.
	//
	void SetTaskInfoObject (CTaskInfoMT* pObj)
		{ m_pTaskInfo = pObj; }

	BOOL MessagesWereMoved ()
		{ return m_bPreFilterMoved; }


#if 0 // JOK

	// Interface between the filter action and the way we implement a "Make Status"
	// filter.
	//
	void	SetImapMsgStatus(CSummary *pSum, unsigned char ucStatus);

	void	ImapSetServerOpt (CSummary *pSum, unsigned int uiServerOpt);

#endif // JOK

// Public data.
public:
	// Set to TRUE if escape was pressed.
	BOOL		   m_bStopRequested;

	// Set to TRUE if we did moves during the prefilter stage.
	BOOL		   m_bPreFilterMoved;


// Internal methods.
private:
	// THREAD-SAFE METHODS:
	//
	BOOL	IsOurPersonalityMT (CPreFilter *pFilt, int HeaderNum);
	void	ImapMatchMT (CPreFilter *filt, CPtrUidList& TotalUidList, CPtrUidList& SubUidList);

	void	FormulateImapSearchCriteriaMT (LPCSTR pHeader, int Verb, 
							LPCSTR pValue, CString& HeaderList, BOOL *pbBody, BOOL *pbNot);

	BOOL	DoRemoteTransfersMT (CPreFilter *pFilt, CPtrUidList& NewUidList, LPCSTR pUids,
							 CImapSumList *pSumList = NULL);
	
	BOOL	RemoveUidsFromListMT (CPtrUidList& NewUidList, LPCSTR pUids);

	void	UpdateAfterCopy (CPreFilter* pFilt, LPCSTR pDstMboxPath, UINT nMsgsCopied);

	// NON_THREAD_SAFE private methods.
	//
	HRESULT	AcquireNetworkConnection ();

	void	ClearAllImapLists ();


#if 0 // JOK

	int		PerformAction (CTocDoc *pTocDoc, CFilter *filt, CSumList& SumList,
							CPtrUidList& TotalUidList, CPtrUidList& SubUidList);

	void	DoServerStatusChanges (CTocDoc *pTocDoc, CSumList& SumList);

	BOOL	IsXferFilter (CFilter* pFilt);
	
	// Add a UID to the given CString in proper comma-separated form:
	void	AddUidToStatusString (CImapSum *pSum, CString& szStr);

	void	UpdateTocAfterFiltering (CTocDoc *pToc);

#endif // JOK

//
private:
	// This is our list of XFER/COPY to same server filters (CPreFilter objects).
	// This is created in the main tread during the "StartFilterActions()" method.
	// THe CPreFilter objects are created by copying info from the global g_Filter.
	//
	CPreFilterList m_PreFilterList;

// Private data:
private:
	// Map of destination mailboxes.
	//
	CStringMap    m_FilterMboxMap;
	CStringMap    m_FilterReportMap;

	// During filtering, accumulate uid's that need specific flags
	// changed on the server:
	CString m_szAnsweredUids;
	CString m_szReadUids;
	CString m_szUnreadUids;
	CString m_szDeletedUids;
	CString m_szFetchUids;

	// Store the filename name of the mailbox we're filtering FROM!!
	// Used for identifying the mailbox.
	//
	CString m_szMBFilename;

	// Keep track of the largest UID we've seen during an incoming filter.
	//
	unsigned long m_HighestUidXferred;


	// Some INI options needed during filtering (especially during background).
	//
	BOOL m_bOpenInMailbox;
	BOOL m_bDoFilterReport;

	// Needed to allow thread-safe methods:
	// Set this before any background operations.
	//
	CString			m_szPersona;
	unsigned long	m_AccountID;
	CString			m_szServerName;

	// Full IMAP name of source. "INBOX" in most cases.
	//
	CString			m_szSrcImapName;

	CIMAP*			m_pImap;

	// Be careful how ths is used!!!
	//
	CTaskInfoMT*		m_pTaskInfo;

};



#endif // __IMAPPREFILTERACT_H

