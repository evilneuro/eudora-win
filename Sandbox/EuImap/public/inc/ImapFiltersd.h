// IMAPFILTERSD.H
//

#ifndef __IMAPFILTERSD_H
#define __IMAPFILTERSD_H


class CTocList;
class CSumList;
class CPtrUidList;

class CImapFilterActions : public CFilterActions
{
public:
	CImapFilterActions  ();
	~CImapFilterActions ();

	// Initialization.
	BOOL ImapStartFiltering ();
	BOOL ImapEndFiltering ();
	void ClearAllImapLists ();

	// Copy info from background pre filtering.
	//
	void InitializeFromBackgroundFilter (CObList& strDstList, CObList& strReportList);

// Public Interface:
	int		DoXferPreFiltering (CTocDoc *pTocDoc, CPtrUidList& NewUidList);

	int		DoPostFiltering (CTocDoc *pTocDoc, CSumList& SumList, BOOL bDoSameServerXfers,
							BOOL bDoIncoming, BOOL bDoManual);

	void	DoManualFiltering (CTocDoc *pTocDoc, CSummary *SingleSum = NULL);

	void	SetMBFilename (LPCSTR pStr)
				{m_szMBFilename = pStr; }
	LPCSTR GetMBFilename ()
				{ return (LPCSTR)m_szMBFilename; }

	unsigned long GetHighestUidSeen ()
				{ return m_HighestUidXferred; }

	// Interface between the filter action and the way we implement a "Make Status"
	// filter.
	//
	void	SetImapMsgStatus(CSummary *pSum, unsigned char ucStatus);

	void	ImapSetServerOpt (CSummary *pSum, unsigned int uiServerOpt);

// Public data.
public:
	CStringList    m_FilterMboxList;

	// Set to TRUE if escape was pressed.
	BOOL		   m_EscapePressed;

	// Set to TRUE if we did moves during the prefilter stage.
	BOOL		   m_bPreFilterXferred;

	// Set to TRUE if we did a move during post filtering.
	BOOL			m_bPostFilterXferred;


// Internal methods.
private:
	int		ImapPersAndXferMatched (CTocDoc *pTocDoc, CFilter* pFilt);
	void	FormulateImapSearchCriteria (LPCSTR pHeader, int Verb, 
					LPCSTR pValue, CString& HeaderList, BOOL *pbBody, BOOL *pbNot);

	BOOL	DoRemoteTransfers (CTocDoc *pTocDoc, CFilter *pFilt, CPtrUidList& NewUidList,
				 LPCSTR pUids, CSumList *pSumList = NULL);

	BOOL	RemoveUidsFromList (CPtrUidList& NewUidList, LPCSTR pUids);

	void	ImapMatch (CTocDoc *pTocDoc, CFilter *filt, CPtrUidList& TotalUidList, CPtrUidList& SubUidList);

	int		PerformAction (CTocDoc *pTocDoc, CFilter *filt, CSumList& SumList,
							CPtrUidList& TotalUidList, CPtrUidList& SubUidList);

	void	DoServerStatusChanges (CTocDoc *pTocDoc, CSumList& SumList);

	BOOL	IsXferFilter (CFilter* pFilt);
	BOOL	IsOurPersonality (CTocDoc *pTocDoc, CFilter *pFilt, int HeaderNum);
	
	// Add a UID to the given CString in proper comma-separated form:
	void	AddUidToStatusString (CSummary *pSum, CString& szStr);

	void	DoFetchRequestedMessages (CTocDoc *pTocDoc, CSumList& SumList);

	void	UpdateTocAfterFiltering (CTocDoc *pToc);

// Private data:
private:
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

	// Keep track of new msgs that are left in inbox after xferring!
	long m_ulNewMsgsLeftInInbox;


	// Some INI options needed during filtering (especially during background).
	//
	BOOL m_bOpenInMailbox;
	BOOL m_bDoFilterReport;



};



#endif // __IMAPFILTERSD_H

