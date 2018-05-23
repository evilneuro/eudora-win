// ImapPreFiltersd.h
//
// This is a copy of some characteristics of a CFilter.
// 1.	These contain ONLY XFER or COPY actions to mailboxes on the same server as the 
//		source mailbox.
// 2.	Actions that can be performed in a background thread are thread safe.
// 3.	Contains additional methods do facilitate the things we want to do
//		for IMAP malboxes.
//

// NOTE: mOST Constants defined in filtersd.h, AND SINCE WE HAVE TO INCLUDE IT ANYWAY...
//

// Defined in filtersv.h but we don't want to include that here.
//

// Forward declarations:
//
class CFilter;

class CPreFilter : public CObject
{
public:
	CPreFilter();
	~CPreFilter();

	BOOL MatchValueMT (int NumCondition, const char* Contents) const;

	void CopyFromCFilter (CFilter* pFilt);

	BOOL ContainsCopyOrXfer ();
	BOOL HasSkipRest ();

	// ATTRIBUTES.
	//
	CString	m_Name;
	int		m_WhenToApply;
	CString	m_Header[2];
	int		m_Verb[2];
	CString	m_Value[2];
	int		m_Conjunction;

	UINT	m_Actions[5];

	int		m_Transfer;
	CString	m_Mailbox;

	int		m_ServerOpt;
	BOOL	m_SkipRest;
	
	BOOL	m_DoCopyTo[5];
	CString	m_CopyTo[5];

	// For COPY/XFER's, we store th full destination IMAP name here.
	//
	CString	m_szDstImapName [5];

	// Need this to determine what to put in report list.
	//
	int		m_NotfyUserNR;

	// Inline functions.
	//	
	BOOL IsIncoming()	{ return ((m_WhenToApply & WTA_INCOMING)? TRUE : FALSE); }
	BOOL IsOutgoing()	{ return ((m_WhenToApply & WTA_OUTGOING)? TRUE : FALSE); }
	BOOL IsManual()		{ return ((m_WhenToApply & WTA_MANUAL)? TRUE : FALSE); }

	void SetIncoming(BOOL OnOff = TRUE)
		{ if (OnOff) m_WhenToApply |= WTA_INCOMING; else m_WhenToApply &= ~WTA_INCOMING; }
	void SetOutgoing(BOOL OnOff = TRUE)
		{ if (OnOff) m_WhenToApply |= WTA_OUTGOING; else m_WhenToApply &= ~WTA_OUTGOING; }
	void SetManual(BOOL OnOff = TRUE)
		{ if (OnOff) m_WhenToApply |= WTA_MANUAL; else m_WhenToApply &= ~WTA_MANUAL; }
	
	BOOL IsTransfer()	{ return ((m_Transfer & TT_ON)? TRUE : FALSE); }
	BOOL IsCopyTo()		{ return ((m_Transfer & TT_COPY_TO)? TRUE : FALSE); }
	BOOL IsTransferTo()	{ return (!IsCopyTo()); }
	void SetTransfer(BOOL OnOff = TRUE)
		{ if (OnOff) m_Transfer |= TT_ON; else m_Transfer &= ~TT_ON; }
	void SetTransferTo()
		{ m_Transfer |= TT_TRANSFER_TO; m_Transfer &= ~TT_COPY_TO; }
	void SetCopyTo()
		{ m_Transfer &= ~TT_TRANSFER_TO; m_Transfer |= TT_COPY_TO; }

	void SetDstImapName (int index, LPCSTR pDstImapName)
		{ m_szDstImapName [index] = pDstImapName; }

	LPCSTR GetDstImapName (int index)
		{ return m_szDstImapName [index]; }

};


class CPreFilterList : public CObList
{
public:
	CPreFilterList() {}
	~CPreFilterList();

	void NukeAllData();

	// Create a new CPreFilter object, copy relevant data from the given CFilter object,
	// and add to te end of the list.
	//
	void AddFromCFilter (CFilter* pFilt, CTocDoc *pSrcToc);


	CPreFilter* GetAt(POSITION position) const
		{ return ((CPreFilter*)CObList::GetAt(position)); }
	CPreFilter*& GetAt(POSITION position)
		{ return ((CPreFilter*&)CObList::GetAt(position)); }
	CPreFilter*& GetHead()
		{ return ((CPreFilter*&)CObList::GetHead()); }
	CPreFilter* GetHead() const
		{ return ((CPreFilter*)CObList::GetHead()); }
	CPreFilter*& GetTail()
		{ return ((CPreFilter*&)CObList::GetTail()); }
	CPreFilter* GetTail() const
		{ return ((CPreFilter*)CObList::GetTail()); }
	CPreFilter*& GetNext(POSITION& rPosition)
		{ return ((CPreFilter*&)CObList::GetNext(rPosition)); }
	CPreFilter* GetNext(POSITION& rPosition) const
		{ return ((CPreFilter*)CObList::GetNext(rPosition)); }
	CPreFilter*& GetPrev(POSITION& rPosition)
		{ return ((CPreFilter*&)CObList::GetPrev(rPosition)); }
	CPreFilter* GetPrev(POSITION& rPosition) const
		{ return ((CPreFilter*)CObList::GetPrev(rPosition)); }
	CPreFilter* RemoveHead()
		{ return ((CPreFilter*)CObList::RemoveHead()); }
	CPreFilter* RemoveTail()
		{ return ((CPreFilter*)CObList::RemoveTail()); }
		
	CPreFilter* IndexData(int nIndex) const;

// 
private:
	CPreFilter* CreateFromFilter (CFilter* pFilt);
};



// EXPORTED FUNCTIONS
//
BOOL IsOnSameServer (CTocDoc *pSrcToc, QCImapMailboxCommand *pImapCommand);





