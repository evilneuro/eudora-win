// TOCDOC.H
//
// Routines for handling the data of mailbox TOCs
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
	BY_JUNK,
	BY_PRIORITY,
	BY_ATTACHMENT,
	BY_LABEL,
	BY_SENDER,
	BY_DATE,
	BY_SIZE,
	BY_SERVERSTATUS,
	BY_MOOD,
	BY_SUBJECT,

	BY_REVERSE_STATUS = 129,
	BY_REVERSE_JUNK,
	BY_REVERSE_PRIORITY,
	BY_REVERSE_ATTACHMENT,
	BY_REVERSE_LABEL,
	BY_REVERSE_SENDER,
	BY_REVERSE_DATE,
	BY_REVERSE_SIZE,
	BY_REVERSE_SERVERSTATUS,
	BY_REVERSE_MOOD,
	BY_REVERSE_SUBJECT,
} SortType;

typedef enum
{
	FW_STATUS,
	FW_JUNK,
	FW_PRIORITY,
	FW_ATTACHMENT,
	FW_LABEL,
	FW_SENDER,
	FW_DATE,
	FW_SIZE,
	FW_SERVER,
	FW_MOOD,
	FW_SUBJECT,
	FW_UNUSED = FW_SUBJECT
} FieldWidthType;

enum  MailboxType
{
	MBT_IN,
	MBT_OUT,
	MBT_JUNK,
	MBT_TRASH,
	MBT_REGULAR,
	MBT_FOLDER,
	//used for Imap
	MBT_IMAP_ACCOUNT,
	MBT_IMAP_NAMESPACE,
	MBT_IMAP_MAILBOX
};


extern enum UnreadStatusType;

// Forward declarations
class CImapMailbox;
class CImapSum;
class CUidMap;
class QCMailboxCommand;
class MultSearchCriteria;
class SearchCriteria;


class CTocDoc : public CDoc
{
	// Friends
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

	const CString &			GetFileName() const { return m_strPathName; }
	const CString &			GetMBFileName() const { return m_MBFileName; }
	void					SetMBFileName(const char * szMBFileName) { m_MBFileName = szMBFileName; }
	void					SetFileName(const char * szFileName) { m_strPathName = szFileName; }

	CSumList &				GetSumList() { return m_Sums; }
	int						NumSums() const { return m_Sums.GetCount(); }
   	
	BOOL                m_ReadSums;     // Have the summaries been read from disk into mem?
	short				m_Type;			// Type of mailbox (In, Out, Trash, Junk, or regular)
	short				m_NeedsCompact;	// Whether or not we need to compact this mailbox
	CRect				m_SavedPos;		// Screen position when last saved
	UnreadStatusType	m_UnreadStatus;	// If this mailbox has unread messages
	BOOL				m_CanNuke;		// Can toc be deleted?
	int					m_iApproxSums;	// Approximate number of sums (for crude error checking)

// IMAP4
	CImapMailbox*		m_pImapMailbox;		// NULL for non-IMAP mailboxes.
	BOOL				m_bHideDeletedIMAPMsgs;	// TRUE if deleted IMAP messages are being hidden


// Operations
public:
	BOOL Build(long lMessageIDStart = 0L);
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
	void CheckNumSums();
	BOOL Display();
	BOOL DisplayBelowTopMostMDIChild();

	CTocView* GetView();
	BOOL IsOpen() const
		{ return (GetFirstViewPosition() != NULL); }
	void AddSum(CSummary* pSum, bool bSelectIt = false);
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
		{ ASSERT(m_TotalSpace >= m_DelSpace); m_TotalSpace -= m_DelSpace; m_DelSpace = 0L; }
//	void UpdateSizeInfo();

	void ClearUndoTo();
	void ClearUndoFrom();
	void ClearUndo()
		{ ClearUndoTo(); ClearUndoFrom(); }
	int InvolvedInUndo()
		{ return (m_UndoToToc || m_UndoFromToc); }

	static void UpdateAllServerStatus();

	void ReformatDateFields();

	// When the CTocFrame goes away we need to tell the CTocDoc, even
	// tho for In,Out,Trash,Junk the CTocDoc doesn't actually die. This is
	// where we will do IDS_INI_MAILBOX_SUPERCLOSE
	void NotifyFrameClosing();

	//
	// The following are Yet Another Kludge Hack From Hell(tm), courtesy 
	// of Willie.  The "previewable" summary is defined to be the Summary
	// object that *may* be previewed at some later time in the TocFrame's
	// preview pane.
	//
	BOOL UsingPreviewPane();

	void InvalidateCachedPreviewSummary(CSummary * in_pInvalidSummary = NULL, bool in_bRefreshPreviewDisplay = false);

	void MailboxChangedByHuman();

	bool TrimJunk(CTocDoc *pTocDest,
				  CString &strTrimToName,
				  QCMailboxCommand *pCommandDest = NULL);
	bool IsJunk();

// IMAP4
	// major IMAP methods. (public)
	CSummary* ImapXfer(CTocDoc *pTargetToc,
					   CSumList *pSumList,
					   BOOL bCloseWindow = TRUE,
                       BOOL bCopy = FALSE,
					   BOOL bRedisplayTOCs = TRUE);

	// IMAP4 public support functions.
	BOOL IsImapToc(){ return m_Type == MBT_IMAP_MAILBOX; }
	// Instantiate the CImapMailbox for this TOC.
	BOOL ImapInitialize (unsigned long AccountID);
	HRESULT		ImapDeleteMessages (CSummary *SingleSum = NULL, BOOL bExpunge = FALSE, BOOL bSilent = TRUE);
	HRESULT		ImapUnDeleteMessages (CSummary *SingleSum = NULL, BOOL bSilent = TRUE);
	HRESULT		ImapJunkMessages(bool bJunk, CSummary *pSingleSum = NULL, BOOL bSilent = TRUE);
	HRESULT		ImapExpunge (BOOL bSilent = TRUE);
	void		ImapExpungeOnServer();
	void		ImapDoPostExpunge();
	void		SetHideDeletedIMAPMsgs(BOOL bHide, BOOL bUpdateView = FALSE);
	BOOL		HideDeletedIMAPMsgs() {return m_bHideDeletedIMAPMsgs;};
	BOOL ImapMergeTmpMbx (CPtrList *pExpungedUidList);
	CSummary	*ImapTransferSummaryData(CString &strUIDList,
										 CTocDoc *pTargetToc,
										 CDWordArray *pdwaNewUIDs,
										 CImapSum *pImapSum = NULL,
										 bool bFindNew = false);
	CSummary	*GetSummaryFromUID(unsigned long lUID);
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

	int		ImapFetchMessages(BOOL bDownloadAttachments = TRUE,
							  BOOL bOnlyIfNotDownloaded = TRUE,
							  BOOL bClearCacheOnly = FALSE,
							  BOOL bInvalidateCachedPreviewSums = TRUE,
							  BOOL bDoItNow = FALSE);
	bool	QueueImapFetch(CString strUidList,
						   BOOL bDownloadAttachments,
						   BOOL bOnlyIfNotDownloaded,
						   BOOL bClearCacheOnly,
						   BOOL bInvalidateCachedPreviewSums);
	int		ImapFetchMessagesFromServer(CSumList *pSumList,
										BOOL bDownloadAttachments,
										BOOL bOnlyIfNotDownloaded,
										BOOL bClearCacheOnly,
										BOOL bInvalidateCachedPreviewSums);

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

	BOOL ImapRemoveAttachments(CSummary *pSum);
	BOOL ImapMoveContents(CTocDoc* pTargetToc, CSummary *pSum);

// END IMAP4 public.

	//
	// MAPI Support functions.
	//
	BOOL GetNextMessageId(unsigned long& messageId, BOOL sortByDate, BOOL unreadOnly);
	BOOL GetMessageById(CString& messageData, unsigned long messageId,
						BOOL bodyAsFile, BOOL envelopeOnly,
						BOOL markAsRead, BOOL wantAttachments);
	BOOL PutMessageById(const char* messageData, unsigned long& messageId);
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
	BOOL				OnNewDocument();

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
	CSummary* ImapXferMessages(CTocDoc *pTargetToc,
							   CSumList *pSumList,
							   int &iHighlightIndex,
                               BOOL bCloseWindow = TRUE,
							   BOOL bCopy = FALSE);
	CSummary* ImapXferOnSameAccount(CTocDoc *pTargetToc,
									CSumList *pSumList,
									int &iHighlightIndex,
                                    BOOL bCloseWindow = TRUE,
									BOOL bCopy = FALSE);
	CSummary* ImapXferBetweenAccounts(CTocDoc *pTargetToc,
									  CSumList *pSumList,
									  int &iHighlightIndex,
                                      BOOL bCloseWindow = TRUE,
									  BOOL bCopy = FALSE);
	CSummary* ImapXferFromImapToLocal(CTocDoc *pTargetToc,
									  CSumList *pSumList,
									  int &iHighlightIndex,
                                      BOOL bCloseWindow = TRUE,
									  BOOL bCopy = FALSE);
	CSummary* ImapXferFromLocalToImap(CTocDoc *pTargetToc,
									  CSumList *pSumList,
									  int &iHighlightIndex,
                                      BOOL bCloseWindow = TRUE,
									  BOOL bCopy = FALSE);

	HRESULT	  ImapMoveMsgsToTrash (CSummary* SingleSum);
	HRESULT   ImapFlagDeleteMsgs (CSummary* SingleSum, BOOL bExpunge, BOOL bSilent);
	HRESULT	  ImapFlagMessages (CSummary *SingleSum = NULL, BOOL bUndelete = FALSE, BOOL bExpunge = FALSE);
	BOOL	  DoFancyExpunge (LPCSTR pUidlist);
	unsigned long ImapGetMaxSummaryUid ();
	unsigned long ImapGetMinSummaryUid ();
// IMAP4

public:
	BOOL	  ImapCopyMsgFromImapToLocal (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum);
	BOOL	  ImapCopyMsgFromLocalToImap (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum);
	BOOL	  ImapCopyMsgBetweenAccounts (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum);

	// Up to this point NumFields actually represented the number of separators
	// (one fewer than the number of fields).  Since I find this confusing I
	// changed NumFields to be the actual number of fields. -dwiggins
	enum { NumFields = 11 };
	enum { NumSortColumns = NumFields};
	SortType GetSortMethod(unsigned Column)
		{ return m_SortMethod[Column]; }
	void SetSortMethod(unsigned Column, SortType NewSortMethod)
		{ m_SortMethod[Column] = NewSortMethod; }
	void SetNeedsSorting()
		{ m_bNeedsSorting = TRUE; }
	void SetNeedsFumlub() { m_bNeedsFumlub = true; }
	BOOL SortNowIfNeeded();
	BOOL SortedDateDescending();
	BYTE GetAdFailure()
		{ return m_AdFailure; }
	void SetAdFailure( BYTE days )
		{ if ( m_AdFailure!=days ) { m_AdFailure=days; SetModifiedFlag( TRUE ); } }

	HRESULT	  ImapChangeMsgsJunkStatus(CSummary *pSingleSum, bool bJunk, bool bManual = false);
	HRESULT   ImapChangeMsgsJunkStatus(CImapSum *pImapSum, bool bJunk);
	HRESULT   ImapChangeMsgsJunkStatus(CString &strUIDList, bool bJunk, CImapSum *pImapSum = NULL);

	void				ImapRecheckMessageForJunk(CSummary *pSum, CObArray *poaABHashes);

	void				IncrementPreventAutoDelete()
								{
									if (m_nPreventAutoDelete++ == 0) m_bSaveAutoDelete = m_bAutoDelete;
									m_bAutoDelete = FALSE;
								}
	void				DecrementPreventAutoDelete()
								{
									if (--m_nPreventAutoDelete == 0) m_bAutoDelete = m_bSaveAutoDelete;
									ASSERT(m_nPreventAutoDelete >= 0);
									if (m_nPreventAutoDelete < 0) m_nPreventAutoDelete = 0;
								}

	bool				IsSingleSelectionProfileOverridden() { return (m_szSingleSelectionProfileName.IsEmpty() == FALSE); }
	bool				IsMultipleSelectionProfileOverridden() { return (m_szMultipleSelectionProfileName.IsEmpty() == FALSE); }
	const CString &		GetSingleSelectionProfileName() const { return m_szSingleSelectionProfileName; }
	const CString &		GetMultipleSelectionProfileName() const { return m_szMultipleSelectionProfileName; }
	void				SetSingleSelectionProfileName(const char * in_szProfileName) { m_szSingleSelectionProfileName = in_szProfileName; }
	void				SetMultipleSelectionProfileName(const char * in_szProfileName) { m_szMultipleSelectionProfileName = in_szProfileName; }
	void				ResetSingleSelectionProfileName() { m_szSingleSelectionProfileName.Empty(); }
	void				ResetMultipleSelectionProfileName() { m_szMultipleSelectionProfileName.Empty(); }

private:
	void SetFieldSeparators();
	int Corrupt();
	int Salvage(BOOL Ask);
	POSITION InternalAddSum(CSummary * pSum, bool bSelectIt);
	void AddExistingSum(CSummary * pSum);

	CSumList			m_Sums;			// Array of message summaries

	CString m_MBFileName;
	BOOL m_bNeedMoodCheck;
    enum { TocVersion = 0x31 };		// Current version is 3.1
	WORD m_Version;
	enum { UsedVersion = 6 };
	BYTE m_UsedVersion[ UsedVersion ];
	enum { UnusedDiskSpace = 2 };
	enum { NameSize = 32 };
	static const short DefaultFieldWidth[NumFields - 1];	// These two are NumFields - 1 because the Subject column isn't
	short m_FieldWidth[NumFields - 1];						// technically sizeable, it just takes up the remaining space.
	short m_TopSum, m_CurrentSum;
	int m_NumShowing;
	long m_TotalSpace, m_MesSpace, m_DelSpace;
	BOOL m_bGroupBySubject;
	BOOL m_bNeedsSorting;
	BOOL m_bNeedsSortingNOW;
	bool m_bNeedsFumlub;
	SortType m_SortMethod[NumSortColumns];
	long m_NextUniqueMessageId;
	long m_PluginID;
	long m_PluginTag;
	BOOL m_ShowFileBrowser;
	int  m_FileBrowserViewState;
	short m_SplitterPos;

	CSumList m_UndoXferTo, m_UndoXferFrom;
	CTocDoc* m_UndoToToc;
	CTocDoc* m_UndoFromToc;
	
	BYTE m_AdFailure;

	long		m_nPreventAutoDelete;
	BOOL		m_bSaveAutoDelete;

	CString		m_szSingleSelectionProfileName;
	CString		m_szMultipleSelectionProfileName;

	//
	// Support for MAPI support.
	//
	BOOL GetMessagesSortedByDate(CSumList& summaryList, BOOL unreadOnly);
	void AddAddressData(CString& messageData, char* pRawAddresses, const char* pKeyword);
	BOOL AddBodyAttachmentData(CString& messageData, const CString& bodyText);
	BOOL AddAttachmentData(CString& messageData, const char* attachPathname, BOOL bMakeCopyOfFile);
	
public:
	UINT	GetSplitterPos() { return m_SplitterPos; }

	// Peanut Support
	BOOL			IsPeanutTagged();
	BOOL			IsPluginTagged() { return ( m_PluginID != 0 ); }
	void			SetPluginID(unsigned long id) { m_PluginID = id; }
	unsigned long	GetPluginID() { return m_PluginID; }
	void			SetPluginTag(unsigned long tag) { m_PluginTag = tag; }
	unsigned long	GetPluginTag() { return m_PluginTag; }
	BOOL			ShowFileBrowser() { return m_ShowFileBrowser; }
	void			SetShowFileBrowser(BOOL bNew) { if (bNew != m_ShowFileBrowser) { m_ShowFileBrowser = bNew; SetModifiedFlag(); } }
	int				GetFileBrowserViewState() { return m_FileBrowserViewState; }
	void			SetFileBrowserViewState(int NewState)  { if (NewState != m_FileBrowserViewState) { m_FileBrowserViewState = NewState; SetModifiedFlag(); } }
	BOOL			CalculateMood();
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

CTocDoc* GetToc(
			const char *			Filename,
			const char *			Name = NULL,
			BOOL					HeaderOnly = FALSE,
			BOOL					OnlyIfLoaded = FALSE,
			BOOL					bDisplayErrors = TRUE);
CTocDoc* GetInToc(BOOL HeaderOnly = FALSE);
CTocDoc* GetOutToc();
CTocDoc* GetTrashToc();
CTocDoc* GetJunkToc();
BOOL DisplayToc(const char* Filename, const char* Name);
void TocCleanup();

bool CloseTOCIfNotInUse(CTocDoc * pTocDoc);

// Remove the TOCs that are currently in memory and not being used
//Call in OnIdle() loop
void ReclaimTOCs();

void UpdatePathnamesInOpenTocs(const CString& oldPathname,
								const CString& newPathname);

short	StripAttachmentName( 
LPCSTR		szSrc, 
CString&	szDest,
LPCSTR		szAttachmentDir );


BOOL EmptyTrash();
bool TrimJunk(bool bUserRequested = false, bool bOnMailCheck = false);
bool TrimImapJunk(CString &strTrimToName);
void DeleteEmbeddedObjects( const char* EOHeader );
BOOL InEmbeddedDir( const char* path );

//
// Used to avoid telling TocFrame about summaries.
//
// IMAP4	// IMAP4
BOOL ImapSumIsDeleted (CSummary *pSum);
BOOL ImapTocHasDeletedMsgs (CTocDoc *pTocDoc);
// END IMAP4


inline BOOL IsImapSum(CSummary *Sum){  return Sum->m_TheToc?Sum->m_TheToc->IsImapToc():FALSE; }

#endif



