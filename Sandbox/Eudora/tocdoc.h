// TOCDOC.H
//
// Routines for handling the data of mailbox TOCs
//

#ifndef _TOCDOC_H_
#define _TOCDOC_H_

/////////////////////////////////////////////////////////////////////////////
// CTocDoc document

#include "doc.h"
#include "summary.h"

typedef enum
{
	NOT_SORTED = 0,

	BY_STATUS = 1,
	BY_PRIORITY,
	BY_ATTACHMENT,
	BY_LABEL,
	BY_SENDER,
	BY_DATE,
	BY_SIZE,
	BY_SERVERSTATUS,
	BY_SUBJECT,

	BY_REVERSE_STATUS = 129,
	BY_REVERSE_PRIORITY,
	BY_REVERSE_ATTACHMENT,
	BY_REVERSE_LABEL,
	BY_REVERSE_SENDER,
	BY_REVERSE_DATE,
	BY_REVERSE_SIZE,
	BY_REVERSE_SERVERSTATUS,
	BY_REVERSE_SUBJECT
} SortType;

typedef enum
{
	FW_STATUS,
	FW_PRIORITY,
	FW_ATTACHMENT,
	FW_LABEL,
	FW_SENDER,
	FW_DATE,
	FW_SIZE,
	FW_SERVER,
	FW_UNUSED
} FieldWidthType;

enum  MailboxType
{
	MBT_IN,
	MBT_OUT,
	MBT_TRASH,
	MBT_REGULAR,
	MBT_FOLDER,
	//used for Imap
	MBT_IMAP_ACCOUNT,
	MBT_IMAP_NAMESPACE,
	MBT_IMAP_MAILBOX
};


extern enum UnreadStatusType;

// IMAP4
class CImapMailbox;
class CUidMap;
class MultSearchCriteria;
class SearchCriteria;

// END IMAP4

class CTocDoc : public CDoc
{
	friend class CTocView;
	friend class CCompMessageDoc;
	
	DECLARE_DYNCREATE(CTocDoc)
	
protected:
	CTocDoc();			// protected constructor used by dynamic creation

	CView* GetPreviewView();

// Attributes
public:
	CString& Name()
		{ return (m_strTitle); }
#ifdef OLDSTUFF	
	const char* StripName(const char* NewName);
#endif
	CString& Filename()
		{ return (m_strPathName); }
	CString& MBFilename()
		{ return (m_MBFilename); }
	int NumSums() const
		{ return (m_Sums.GetCount()); }
   	
	CSumList			m_Sums;			// Array of message summaries
	BOOL                m_ReadSums;     // Have the summaries been read from disk into mem?
	short				m_Type;			// Type of mailbox (In, Out, Trash, or regular)
	short				m_NeedsCompact;	// Whether or not we need to compact this mailbox
	CRect				m_SavedPos;		// Screen position when last saved
	UnreadStatusType	m_UnreadStatus;	// If this mailbox has unread messages
	BOOL				m_CanNuke;		// Can toc be deleted?

// IMAP4
	CImapMailbox*		m_pImapMailbox;		// NULL for non-IMAP mailboxes.


// Operations
public:
	BOOL Build();
	BOOL Read(BOOL HeaderOnly = FALSE);
    BOOL ReadHeader()
    	{ return (Read(TRUE)); }
	BOOL Write(BOOL bHeaderOnly = FALSE, BOOL bSkipUnreadNotification = FALSE);
	BOOL WriteHeader()
		{ return (Write(TRUE)); }
	BOOL WriteSum(CSummary* Sum);
	CSummary* UpdateSum(JJFile* MBox, BOOL First);
	BOOL FlushSum()
		{ m_TopSum = (short)NumSums(); return (Write()); }
	BOOL Display();
	BOOL DisplayBelowTopMostMDIChild();

	CTocView* GetView();
	BOOL IsOpen() const
		{ return (GetFirstViewPosition() != NULL); }
	void AddSum(CSummary* Sum, BOOL SelectIt = FALSE);
	void SubtractSum(POSITION SumPos, int Index = -1);
	void RemoveSum(POSITION SumPos, int Index = -1);
	void SubtractSum(CSummary* Sum, int Index = -1)
		{ SubtractSum(m_Sums.Find(Sum), Index); }
	void RemoveSum(CSummary* Sum, int Index = -1)
		{ RemoveSum(m_Sums.Find(Sum), Index); }
	void MoveSumBefore(POSITION SumPosToMove, POSITION SumPosToStay);
	void MoveSumAfter(POSITION SumPosToMove, POSITION SumPosToStay);
	CSummary* Xfer(CTocDoc* TargetToc, CSummary* SingleSum = NULL, BOOL CloseWindow = TRUE,
		BOOL Copy = FALSE, BOOL RedisplayTOCs = TRUE);
	BOOL ChangeName(const char* NewName, const char* NewFilename);
	void DoSwitchMessage(UINT Direction, POSITION OpenPos);
	void ClearDelSpace()
		{ m_DelSpace = 0L; }
//	void UpdateSizeInfo();

	void ClearUndoTo();
	void ClearUndoFrom();
	void ClearUndo()
		{ ClearUndoTo(); ClearUndoFrom(); }
	int InvolvedInUndo()
		{ return (m_UndoToToc && m_UndoFromToc); }

	static void UpdateAllServerStatus();

	void ReformatDateFields();

	// When the CTocFrame goes away we need to tell the CTocDoc, even
	// tho for In,Out,Trash the CTocDoc doesn't actually die. This is
	// where we will do IDS_INI_MAILBOX_SUPERCLOSE
	void NotifyFrameClosing();

	//
	// The following are Yet Another Kludge Hack From Hell(tm), courtesy 
	// of Willie.  The "previewable" summary is defined to be the Summary
	// object that *may* be previewed at some later time in the TocFrame's
	// preview pane.
	//
	BOOL UsingPreviewPane();
	void SetPreviewableSummary(CSummary* pPreviewableSummary)		// can be NULL
		{ m_pPreviewableSummary = pPreviewableSummary; }
	CSummary* GetPreviewableSummary() const
		{ return m_pPreviewableSummary; }
	void InvalidatePreviewableSummary(CSummary* pInvalidSummary);
	void MailboxChangedByHuman();

// IMAP4
	// major IMAP methods. (public)
	CSummary* ImapXfer(CTocDoc* TargetToc, CSummary* SingleSum = NULL, BOOL CloseWindow = TRUE,
		BOOL Copy = FALSE, BOOL RedisplayTOCs = TRUE);

	// IMAP4 public support functions.
	BOOL IsImapToc(){ return m_Type == MBT_IMAP_MAILBOX; }
	// Instantiate the CImapMailbox for this TOC.
	BOOL ImapInitialize (unsigned long AccountID);
	HRESULT		ImapDeleteMessages (CSummary *SingleSum = NULL, BOOL bExpunge = FALSE, BOOL bSilent = TRUE);
	HRESULT		ImapUnDeleteMessages (CSummary *SingleSum = NULL, BOOL bSilent = TRUE);
	HRESULT		ImapExpunge (BOOL bSilent = TRUE);
	BOOL ImapMergeTmpMbx (CPtrList *pExpungedUidList);
	BOOL ImapRemoveListedTocs (
				CUidMap *pUidMap,
				int& HighlightIndex,
				BOOL CloseWindow = TRUE,
				BOOL SetViewSelection = FALSE,
				BOOL bJustSetDeletedFlag = FALSE);

	BOOL ImapRemoveOrderedSubSums (
				CSumList *pOrderedSubSumList,
				int& HighlightIndex,
				BOOL CloseWindow = TRUE,
				BOOL SetViewSelection = FALSE,
				BOOL bJustSetDeletedFlag = FALSE,
				BOOL bUndelete = FALSE);

	void ImapRemoveAllSummaries ();

	CSummary*	ImapFetchMessages (
				CSummary *SingleSum = NULL,
				BOOL bDownloadAttachments = TRUE,
				BOOL bOnlyIfNotDownloaded = TRUE,
				BOOL bClearCacheOnly = FALSE);

	void ImapFilterMessages (CSummary *SingleSum = NULL);

	BOOL ImapSetMessageState (CSummary *pSum, char State, BOOL bUnset = FALSE);
	BOOL ImapSetStatusOfSelectedMessages (char State);
	BOOL ImapToggleStatusOfSelectedMessages ();

	// Entry point to an IMAP on-server search.
	// NOTE: pUidRange is either NULL or one of the foll:
	// (1). An range of UIDs of the form 234:444. First UID MUST be lower than second UID.
	// (2). Comma-separated UIDs', e.g., 5, 7, 45, 1, 23. UID's not necessarily in any order.
	// (3). Combination of (1) and (2)
	// If NULL, then a range of "1:*" is assumed (i.e. the whole mailbox).
	//

	BOOL ImapOnServerSearchAll (MultSearchCriteria* pMultiSearchCriteria, CSumList& slResults, unsigned long* pNmsgsMatchedUnknown);

	// Number of messages on the server for which we don't have summaries.
	unsigned long ImapNumberOfUnknownMsgs ();

	// Number of summaries for which we haven't downloaded bodies.
	unsigned long ImapNumberOfUnfetchedMsgs ();

	static BOOL ImapServerCanSearch (SearchCriteria *pCurSearchCriteria);


// END IMAP4 public.

	//
	// MAPI Support functions.
	//
	BOOL GetNextMessageId(unsigned long& messageId, BOOL sortByDate, BOOL unreadOnly);
	BOOL GetMessageById(CString& messageData, unsigned long messageId,
						BOOL bodyAsFile, BOOL envelopeOnly,
						BOOL markAsRead, BOOL wantAttachments);
	BOOL PutMessageById(const CString& messageData, unsigned long& messageId);
	BOOL DeleteMessageById(unsigned long messageId);

	BOOL GetProductUsedFlag( int flag );	// 0 .. 47
	void SetProductUsedFlag( int flag );	// 0 .. 47

// Implementation
public:
	BOOL OnSaveDocument(const char* pszPathName);
	BOOL SaveModified();
	BOOL CanCloseFrame(CFrameWnd* pFrame);
	void PreCloseFrame(CFrameWnd* pFrame);
	
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

protected:
	~CTocDoc();
	BOOL OnNewDocument();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTocDoc)
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateSwitchMessage(CCmdUI* pCmdUI);
	BOOL OnSwitchMessage(UINT Direction);
	DECLARE_MESSAGE_MAP()


// IMAP4 // Note: These are private methods.
private:
	CSummary* ImapXferMessages (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow = TRUE, BOOL Copy = FALSE);
	CSummary* ImapXferOnSameAccount (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow = TRUE, BOOL Copy = FALSE);
	CSummary* ImapXferBetweenAccounts (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow = TRUE, BOOL Copy = FALSE);
	CSummary* ImapXferFromImapToLocal (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow = TRUE, BOOL Copy = FALSE);
	CSummary* ImapXferFromLocalToImap (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow = TRUE, BOOL Copy = FALSE);
	BOOL	  ImapCopyMsgFromImapToLocal (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum);
	BOOL	  ImapCopyMsgFromLocalToImap (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum);
	BOOL	  ImapCopyMsgBetweenAccounts (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum);
	HRESULT	  ImapMoveMsgsToTrash (CSummary* SingleSum);
	HRESULT   ImapFlagDeleteMsgs (CSummary* SingleSum, BOOL bExpunge, BOOL bSilent);
	HRESULT	  ImapFlagMessages (CSummary *SingleSum = NULL, BOOL bUndelete = FALSE, BOOL bExpunge = FALSE);
	BOOL	  DoFancyExpunge (LPCSTR pUidlist);
	unsigned long ImapGetMaxSummaryUid ();
	unsigned long ImapGetMinSummaryUid ();
// IMAP4

public:
	enum { NumFields = 8 };
	enum { NumSortColumns = NumFields + 1 };
	SortType GetSortMethod(unsigned Column)
		{ return m_SortMethod[Column]; }
	void SetSortMethod(unsigned Column, SortType NewSortMethod)
		{ m_SortMethod[Column] = NewSortMethod; }
	void SetNeedsSorting()
		{ m_bNeedsSorting = TRUE; }
	BOOL SortNowIfNeeded();

private:
	void SetFieldSeparators();
	int Corrupt();
	int Salvage(BOOL Ask);
	                   
	CString m_MBFilename;
    enum { TocVersion = 0x30 };		// Current version is 3.00
	WORD m_Version;
	enum { UsedVersion = 6 };
	BYTE m_UsedVersion[ UsedVersion ];
	enum { UnusedDiskSpace = 7 };
	enum { NameSize = 32 };
	static const short DefaultFieldWidth[NumFields];
	short m_FieldWidth[NumFields];
	short m_TopSum, m_CurrentSum;
	int m_NumShowing;
	long m_TotalSpace, m_MesSpace, m_DelSpace;
	BOOL m_bGroupBySubject;
	BOOL m_bNeedsSorting;
	BOOL m_bNeedsSortingNOW;
	SortType m_SortMethod[NumSortColumns];
	long m_NextUniqueMessageId;
	long m_PeanutID;
	long m_PeanutProp;
	short m_SplitterPos;

	CSumList m_UndoXferTo, m_UndoXferFrom;
	CTocDoc* m_UndoToToc;
	CTocDoc* m_UndoFromToc;
	CSummary* m_pPreviewableSummary;		// Yet another Kludge Hack from Hell(tm), courtesy of WKS

	//
	// Support for MAPI support.
	//
	BOOL GetMessagesSortedByDate(CSumList& summaryList, BOOL unreadOnly);
	void AddAddressData(CString& messageData, char* pRawAddresses, const char* pKeyword);
	BOOL AddBodyAttachmentData(CString& messageData, const CString& bodyText);
	BOOL AddAttachmentData(CString& messageData, const CString& attachPathname, BOOL bMakeCopyOfFile);
	
public:
	UINT	GetSplitterPos() { return m_SplitterPos; }

	// Peanut Support
	BOOL ShowPeanutIcon();
	BOOL IsPeanutTagged() { return ( m_PeanutID != 0 ); }
	void SetPeanutID(unsigned long id) { m_PeanutID = id; }
	unsigned long GetPeanutID() { return m_PeanutID; }
	void SetPeanutTag(unsigned long tag) { m_PeanutProp = tag; }
	unsigned long GetPeanutTag() { return m_PeanutProp; }
};

class CTocList : public CObList
{
public:
	CTocDoc* GetAt(POSITION position) const
		{ return ((CTocDoc*)CObList::GetAt(position)); }
	CTocDoc*& GetAt(POSITION position)
		{ return ((CTocDoc*&)CObList::GetAt(position)); }
	CTocDoc*& GetHead()
		{ return ((CTocDoc*&)CObList::GetHead()); }
	CTocDoc* GetHead() const
		{ return ((CTocDoc*)CObList::GetHead()); }
	CTocDoc*& GetTail()
		{ return ((CTocDoc*&)CObList::GetTail()); }
	CTocDoc* GetTail() const
		{ return ((CTocDoc*)CObList::GetTail()); }
	CTocDoc*& GetNext(POSITION& rPosition)
		{ return ((CTocDoc*&)CObList::GetNext(rPosition)); }
	CTocDoc* GetNext(POSITION& rPosition) const
		{ return ((CTocDoc*)CObList::GetNext(rPosition)); }
	CTocDoc*& GetPrev(POSITION& rPosition)
		{ return ((CTocDoc*&)CObList::GetPrev(rPosition)); }
	CTocDoc* GetPrev(POSITION& rPosition) const
		{ return ((CTocDoc*)CObList::GetPrev(rPosition)); }
	CTocDoc* RemoveHead()
		{ return ((CTocDoc*)CObList::RemoveHead()); }
	CTocDoc* RemoveTail()
		{ return ((CTocDoc*)CObList::RemoveTail()); }
		
	CTocDoc* IndexData(int nIndex) const;
};


// Function Prototypes

CTocDoc* GetToc(const char* Filename, const char* Name = NULL, BOOL HeaderOnly = FALSE, BOOL OnlyIfLoaded = FALSE);
CTocDoc* GetToc(UINT nID, BOOL HeaderOnly = FALSE, BOOL OnlyIfLoaded = FALSE);
CTocDoc* GetInToc(BOOL HeaderOnly = FALSE);
CTocDoc* GetOutToc();
CTocDoc* GetTrashToc();
BOOL DisplayToc(const char* Filename, const char* Name);
BOOL DisplayToc(UINT nID);
void TocCleanup();

// Remove the TOCs that are currently in memory and not being used
//Call in OnIdle() loop
void ReclaimTOCs();

void UpdatePathnamesInOpenTocs(const CString& oldPathname,
								const CString& newPathname);

short	StripAttachmentName( 
LPCSTR		szSrc, 
CString&	szDest,
LPCSTR		szAttachmentDir );


#ifdef OLDSTUFF
class CUserMenu;
class CMailboxesMenuItem;

typedef enum
{
    CR_COMPACTED,
    CR_MILD_ERROR,
    CR_DRASTIC_ERROR
} CompactResultType;

CompactResultType CompactOneMailbox(LPCSTR szPathname, CTocDoc* TheTOC = NULL);
BOOL CompactMailboxes(CUserMenu* Menu = NULL);
#endif

BOOL EmptyTrash();
void DeleteEmbeddedObjects( CString EOHeader );
BOOL InEmbeddedDir( CString path );

//
// Used to avoid telling TocFrame about summaries.
//
// IMAP4	// IMAP4
BOOL ImapSumIsDeleted (CSummary *pSum);
BOOL ImapTocHasDeletedMsgs (CTocDoc *pTocDoc);
// END IMAP4


inline BOOL IsImapSum(CSummary *Sum){  return Sum->m_TheToc?Sum->m_TheToc->IsImapToc():FALSE; }

#endif



