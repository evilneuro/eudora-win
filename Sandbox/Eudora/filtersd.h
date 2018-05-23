// FILTERSD.H
//

#ifndef _EUD_FILTERSD_H_
#define _EUD_FILTERSD_H_


// Some constants to make life easier
const int VERB_FIRST = IDS_CONTAINS;
//const int VERB_LAST = IDS_INTERSECTS;
const int VERB_LAST = IDS_MATCHES_REGEX;

const int NumVerbs = VERB_LAST - VERB_FIRST + 1;

const int CONJUNCTION_FIRST = IDS_FIO_IGNORE;
const int CONJUNCTION_LAST = IDS_FIO_UNLESS;
const int NumConjunctions = CONJUNCTION_LAST - CONJUNCTION_FIRST + 1;


// Change priorites
const int PC_RAISE = 1;
const int PC_LOWER = 2;

// Transfer and copy
const int TT_TRANSFER_TO	= 0;
const int TT_COPY_TO		= 1;
const int TT_ON				= 2;

// Open Message/Mailbox
const int OM_MAILBOX	= 1;
const int OM_MESSAGE	= 2;

//Notify User
const int NO_NORMAL		= 1;
const int NO_REPORT		= 2;

// Server Opts
const int SO_FETCH		= 1;
const int SO_DELETE		= 2;

// When to apply a filter
const int WTA_INCOMING	= 1;
const int WTA_OUTGOING	= 2;
const int WTA_MANUAL	= 4;

// Type of Actions that need to be returned
const int FA_TRANSFER   = 1;
const int FA_SKIPREST   = 2;
const int FA_NORMAL		= 4;

class CFilterActions;

class CFilter : public CObject
{
public:
	CFilter();

	const char* SetName();
    
	BOOL MatchValue(int NumCondition, const char* Contents) const;
	BOOL MatchCondition(int NumCondition, const char* text) const;
	BOOL Match(const char* text) const;
	int Action(CSummary*& Sum, CFilterActions* fltAct, const char *text);

	static bool IsRestrictedFilterAction(UINT nAction);

#ifdef IMAP4 // IMAP4
	int ImapLocalAction(CSummary*& Sum, CFilterActions *fltAct);
#endif // END IMAP4		

	CString	m_Name;
	int		m_WhenToApply;
	CString	m_Header[2];
	int		m_Verb[2];
	CString	m_Value[2];
	int		m_Conjunction;

	UINT	m_Actions[5];

	int		m_Label;
	int		m_PriorityChange;
	int		m_Transfer;
	CString	m_Mailbox;
	int		m_MakeStatus;
	int		m_MakePriority;
	int		m_OpenMM;
	BOOL	m_Print;
	int		m_NotfyUserNR;
	int		m_ServerOpt;
	BOOL	m_SkipRest;
	BOOL	m_AddToHistory;
	
	//Text to speech 
	int m_SpeakOptions;
	CString m_strVoiceGUID;

	BOOL	m_ChangeSubject;
	CString	m_MakeSubject;
	
	BOOL	m_PlaySound[5];
	CString	m_Sound[5];
	
	BOOL	m_DoNotifyApp[5];
	CString	m_NotifyApp[5];
	
	BOOL	m_DoForward[5];
	CString m_Forward[5];
	
	BOOL	m_DoRedirect[5];
	CString m_Redirect[5];
	
	BOOL	m_DoReply[5];
	CString m_Reply[5];
	
	BOOL	m_DoCopyTo[5];
	CString	m_CopyTo[5];
 	CString m_Desc[5];

	BOOL	m_DoPersonality;
	CString m_Personality;

	
	BOOL IsIncoming()	{ return ((m_WhenToApply & WTA_INCOMING)? TRUE : FALSE); }
	BOOL IsOutgoing()	{ return ((m_WhenToApply & WTA_OUTGOING)? TRUE : FALSE); }
	BOOL IsManual()		{ return ((m_WhenToApply & WTA_MANUAL)? TRUE : FALSE); }
	void SetIncoming(BOOL OnOff = TRUE)
		{ if (OnOff) m_WhenToApply |= WTA_INCOMING; else m_WhenToApply &= ~WTA_INCOMING; }
	void SetOutgoing(BOOL OnOff = TRUE)
		{ if (OnOff) m_WhenToApply |= WTA_OUTGOING; else m_WhenToApply &= ~WTA_OUTGOING; }
	void SetManual(BOOL OnOff = TRUE)
		{ if (OnOff) m_WhenToApply |= WTA_MANUAL; else m_WhenToApply &= ~WTA_MANUAL; }
		
	BOOL IsRaise()	{ return ((m_PriorityChange & PC_RAISE)? TRUE : FALSE); }
	BOOL IsLower()	{ return ((m_PriorityChange & PC_LOWER)? TRUE : FALSE); }
	void SetRaise()	{ m_PriorityChange = PC_RAISE; }
	void SetLower()	{ m_PriorityChange = PC_LOWER; }
	void SetNoPriority()	{ m_PriorityChange = 0; }
	
	BOOL IsTransfer()	{ return ((m_Transfer & TT_ON)? TRUE : FALSE); }
	BOOL IsCopyTo()		{ return ((m_Transfer & TT_COPY_TO)? TRUE : FALSE); }
	BOOL IsTransferTo()	{ return (!IsCopyTo()); }
	void SetTransfer(BOOL OnOff = TRUE)
		{ if (OnOff) m_Transfer |= TT_ON; else m_Transfer &= ~TT_ON; }
	void SetTransferTo()
		{ m_Transfer |= TT_TRANSFER_TO; m_Transfer &= ~TT_COPY_TO; }
	void SetCopyTo()
		{ m_Transfer &= ~TT_TRANSFER_TO; m_Transfer |= TT_COPY_TO; }

	BOOL SetTransferMBOX(const char * name, const char * pathName, BOOL skipRest = TRUE);

	CString SetupCommandLine(CSummary *sum, CString rawCmd);

protected:
	BOOL PreventAutoRepsonse(CSummary* Sum);
};

typedef enum
{
	FF_PRE_FILTER = 0,
	FF_MAIN_FILTER,
	FF_POST_FILTER
} FILTER_FILE_TYPE;


class CFilterList : public CObList
{
public:
	CFilterList() {}
	~CFilterList();

	void NukeAllData();

	CFilter* GetAt(POSITION position) const
		{ return ((CFilter*)CObList::GetAt(position)); }
	CFilter*& GetAt(POSITION position)
		{ return ((CFilter*&)CObList::GetAt(position)); }
	CFilter*& GetHead()
		{ return ((CFilter*&)CObList::GetHead()); }
	CFilter* GetHead() const
		{ return ((CFilter*)CObList::GetHead()); }
	CFilter*& GetTail()
		{ return ((CFilter*&)CObList::GetTail()); }
	CFilter* GetTail() const
		{ return ((CFilter*)CObList::GetTail()); }
	CFilter*& GetNext(POSITION& rPosition)
		{ return ((CFilter*&)CObList::GetNext(rPosition)); }
	CFilter* GetNext(POSITION& rPosition) const
		{ return ((CFilter*)CObList::GetNext(rPosition)); }
	CFilter*& GetPrev(POSITION& rPosition)
		{ return ((CFilter*&)CObList::GetPrev(rPosition)); }
	CFilter* GetPrev(POSITION& rPosition) const
		{ return ((CFilter*)CObList::GetPrev(rPosition)); }
	CFilter* RemoveHead()
		{ return ((CFilter*)CObList::RemoveHead()); }
	CFilter* RemoveTail()
		{ return ((CFilter*)CObList::RemoveTail()); }
		
	CFilter* IndexData(int nIndex) const;
};


/////////////////////////////////////////////////////////////////////////////
// CFiltersDoc document

class CFiltersDoc : public CDoc
{
	DECLARE_DYNCREATE(CFiltersDoc)
protected:
	CFiltersDoc();

// Attributes
public:
	CFilterList m_Filters;
	CFilterList m_PreFilters;
	CFilterList m_PostFilters;

private:
	bool m_bOneTimeInit;

// Operations
public:
	bool DoOneTimeInitIfNotDoneYet();
	bool HasDoneInit() { return m_bOneTimeInit; }

	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(const char* pszPathName);

	BOOL	CreateFilterItem(CFilter *filter);
	
	BOOL Read(BOOL bDoChecks = false);
	BOOL Read(const char *filtFileName, BOOL bDoChecks = false);
	BOOL Write();

	int FilterMsg(CSummary* Sum, int WhenToApply, CFilterActions *pFiltAct, FILTER_FILE_TYPE ffType = FF_MAIN_FILTER);
	void LoadExtraFilters();

// Implementation
protected:
	virtual ~CFiltersDoc();
	virtual	BOOL OnNewDocument();
	enum { FiltVersion = 3 };
	BOOL UpdateFilterVersion();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFiltersDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BOOL CreateFiltersDoc();
BOOL CanCloseFiltersFrame();
CFiltersDoc* GetFiltersDoc();

class CTocList;
class CSumList;

class CStringCount : public CObject
{
public:
	CStringCount(const char *Name) : m_Name(Name), m_Count(1) {}

	CString	m_Name;
	int		m_Count;
};

class CStringCountList : public CObList
{
public:
	CStringCountList() {}
	~CStringCountList();
    
    CStringCount*& GetAt(POSITION position)
		{ return ((CStringCount*&)CObList::GetAt(position)); }
	CStringCount* GetAt(POSITION position) const
		{ return ((CStringCount*)CObList::GetAt(position)); }
	CStringCount*& GetNext(POSITION& rPosition)
		{ return ((CStringCount*&)CObList::GetNext(rPosition)); }
	CStringCount* GetNext(POSITION& rPosition) const
		{ return ((CStringCount*)CObList::GetNext(rPosition)); }
	CStringCount* RemoveHead()
		{ return ((CStringCount*)CObList::RemoveHead()); }
		
	void RemoveAt(POSITION position)
		{ delete GetAt(position); CObList::RemoveAt(position); }
	void RemoveAll();
	CStringCount* Add(const char* Name);
};

class CFilterActions : public CObject
{
public:
	CTocList m_FilterTocList;
	CTocList m_FilterOpenMBoxTocList;
	CSumList m_FilterOpenSumList;
	CSumList m_FilterPrintSum;
	CStringCountList m_NotifyReportList;
	CStringList m_SoundList;
	CStringList m_NotifyApp;

	void ClearAllLists();

public:
	CFilterActions();
	~CFilterActions();

	BOOL StartFiltering(int WhenToApply = 0);
	int FilterOne(CSummary* Sum, const int WhenToApply);
	BOOL EndFiltering();
	BOOL ContainsProActions();

protected:
	BOOL HasFilterContext(FILTER_FILE_TYPE ffType, const int WhenToApply);
};


#endif