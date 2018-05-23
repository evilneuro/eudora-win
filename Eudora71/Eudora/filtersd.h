// FILTERSD.H
//
// Copyright (c) 1993-2000 by QUALCOMM, Incorporated
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

#ifndef _EUD_FILTERSD_H_
#define _EUD_FILTERSD_H_


// Some constants to make life easier
const int VERB_FIRST = IDS_CONTAINS;
const int VERB_LAST_NON_JUNK = IDS_MATCHES_REGEX;
const int VERB_LAST = IDS_MORE_THAN;

const int NumVerbs = VERB_LAST - VERB_FIRST + 1;
const int NumVerbsNonJunk = VERB_LAST_NON_JUNK - VERB_FIRST + 1;

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

class CABHash : public CObject
{
public:
	CABHash(CString &strABName);
	~CABHash();

	void			 GenerateHashArray(const char* szABookName =NULL);

	void			 AddAddress(char *szAddress);

	bool			 MatchesName(CString &strABName);
	bool			 ContainsHash(ULONG ulHash);

private:
	CString			 m_strABName;
	CDWordArray		 m_dwaHashArray;
};

class CFilter : public CObject
{
public:
	CFilter();
	~CFilter();

	const char* SetName();
    
	// There isn't any particularly good reason for the following to be static
	// methods of CFilter since they are no longer unique to CFilter and don't
	// assume anything about the CFilter class.  We could move them to a better
	// location (for example, under CFilterDoc which actually maintains the master
	// hash array).  Maybe someday. -dwiggins
	static void GenerateHashes(CObArray *poaABHashes, BOOL bAll = TRUE);
	static BOOL MatchesHash(CObArray *poaABHashes, const char *szContents);
	static void AddToHash(const char *szAddresses, CString strABName);

	BOOL MatchesAddress(const char *szAddress, const char *szContents);
	BOOL IntersectsAddresses(char *szAddresses, const char *szContents);
	BOOL MatchValue(int NumCondition, const char* Contents, CSummary *pSum = NULL);
	BOOL MatchCondition(int NumCondition, const char* text, CSummary *pSum);
	BOOL Match(const char* text, CSummary *pSum);
	int Action(const char* text, CSummary*& Sum, CFilterActions *fltAct, CObArray * poaABHashes, bool bNoJunkAction = false);

	static bool IsRestrictedFilterAction(UINT nAction);

#ifdef IMAP4 // IMAP4
	int ImapAction(CSummary *&pSum, CFilterActions *pFltAct, int iActionNum, CObArray *poaABHashes, CTocDoc **ppDestToc = NULL);
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
	BOOL	m_Junk;
	int		m_NotfyUserNR;
	int		m_ServerOpt;
	BOOL	m_SkipRest;
	
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

	CObArray	*m_poaABHashes;
	
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

	CString SetupCommandLine(const char* text, CSummary *sum, const char* rawCmd);

protected:
	BOOL PreventAutoRepsonse(const char* text);
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

	CObArray	*m_poaABHashes;
	
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

	int FilterMsg(CSummary*& Sum,
				  int WhenToApply,
				  CFilterActions *pFiltAct,
				  CObArray * poaABHashes,
				  bool bNoJunkAction = false,
				  FILTER_FILE_TYPE ffType = FF_MAIN_FILTER);
	void LoadExtraFilters();

	CABHash *GetHash(CString &strABName);
	CABHash *AddHash(CString &strABName);
	void ForgetHashes(CString &strABName);

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
	CTempSumList m_FilterOpenSumList;
	CTempSumList m_FilterPrintSum;
	CStringCountList m_NotifyReportList;
	CStringList m_SoundList;
	CStringList m_NotifyApp;

	void ClearAllLists();

public:
	CFilterActions();
	~CFilterActions();

	BOOL StartFiltering(int WhenToApply = 0);
	int FilterOne(CSummary* Sum,
				  const int WhenToApply,
				  CObArray *poaABHashes,
				  bool bNoJunkAction = false,
				  CSummary **ppSumNew = NULL);
	BOOL EndFiltering();
	BOOL ContainsProActions();
	static void ForgetHashes(CString &strABName);
	static bool AnyManualFilters();

protected:
	BOOL HasFilterContext(FILTER_FILE_TYPE ffType, const int WhenToApply);
};


#endif