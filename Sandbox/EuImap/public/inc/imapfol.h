// imapfol.h - declaration of the ImapFolder structure used for storing imap folder 
// hierarchies.
//

#ifndef __IMAPFOL_H
#define __IMAPFOL_H

#include "ImapExports.h"
#include "imap.h"
#include "imapjournal.h"
#include "imapresync.h"
#include "imaputil.h"

// Node of the mailbox tree. 
typedef struct imap_mailbox_node
{
	ImapMailboxType		Type;			// IMAP_NAMESPACE, IMAP_SERVICE or IMAP_MAILBOX.
	short				level;			// Relative level in the tree. NOTE: This is used to locate stuff
										// within the mailbox tree. HAS nothing to do with a mailbox level
										// on the server!! 
	LPSTR			pImapName;			// Complete imap name, NOT including machine name.
										// Also used as the "reference" for child fetches.
	LPSTR			pDirname;			// Local direcrtory for this mailbox.
	ACCOUNT_ID		AccountID;			// ID of owning account.
	TCHAR			Delimiter;			// For this mailbox hierarchy.
	BOOL			NoInferiors;		// If can never contain other mailboxes.
	BOOL			NoSelect;			// If cannot be selected.
	BOOL			Marked;
	BOOL			UnMarked;

	// Set to TRUE if this mailbox already exists locally.
	BOOL			bExists;

	// In case we need to move backwards.
	struct imap_mailbox_node *pParentMailbox;

	struct imap_mailbox_node 	*SiblingList;		// Ordered linked list of top level Mailboxes..
	struct imap_mailbox_node	*ChildList;			// Each mailbox may have a linked list of children.
} ImapMailboxNode;



// CPtrUidList below contains a list of objects of this type:
class CImapFlags
{
public:
	CImapFlags (unsigned long msgno = 0, unsigned long uid = 0,
				unsigned long Imflags = 0, BOOL IsNew = FALSE);

	~CImapFlags() {}

	// Data:
	unsigned long m_Uid;
	unsigned long m_Imflags;
	BOOL		  m_IsNew;
};


//=============== CPtrUidList interface ==========================//
class CPtrUidList  : public CPtrList
{
public:
	CPtrUidList();
	~CPtrUidList();

	// Insert in ascending order.

	// If we already have flags as bitflags.
	POSITION OrderedInsert(unsigned long uid, unsigned long Imflags, BOOL IsNew);
	// Have BOOLS.
	POSITION OrderedInsert(	unsigned long uid,
							BOOL seen,
							BOOL deleted,
							BOOL flagged,
							BOOL answered,
							BOOL draft,
							BOOL recent,
							BOOL IsNew);

	// Delete data plus do a RemoveAll().
	void DeleteAll();
};



//======================================================================//
//====================== CImapMailbox class interface ==================//

// Forward declarations
class  CIMAP;
class  CMBXWriter;
class  CImapJournaler;
class  CImapResyncer;
class  CSumList;
class  CImapChecker;		// Friend class.
class  CImapSumList;
class  CImapSum;
class  CImapSettings;
class	CTaskInfoMT;
class  SearchCriteria;

// CImapMailbox class.
class CImapMailbox : public CObject
{
public:
	CImapMailbox ();
	CImapMailbox (ACCOUNT_ID AccountID);
	~CImapMailbox();

private:
	void InitAttributes();

// Interface.
public:
	// Resets all attributes.
	void ResetAttributes();
	// Copy.
	void Copy (CImapMailbox *pImapMailbox);

	// Create local mailbox directory and internal files stubs.
	BOOL InitializeLocalStorage(BOOL Clean);

	// Manages the mailbox's info file.
	BOOL ReadImapInfo ();
	BOOL WriteImapInfo (BOOL Overwrite);

	// Initialize stuff in the main thread before we attempt to
	// connect to the server.
	//
	HRESULT GetLogin ();

	// Just open and select the mailbox.
	HRESULT OpenMailbox (BOOL bSilent = FALSE);
	HRESULT Close ();

	HRESULT  OpenOnDisplay (CTocDoc *pTocDoc);

	HRESULT CImapMailbox::DoManualResync (CTocDoc *pTocDoc,
						       BOOL bCheckMail = FALSE,
							   BOOL bDownloadedOnly = TRUE,
							   BOOL bInForeGround = FALSE);

	HRESULT  CheckNewMail (CTocDoc *pTocDoc, BOOL bSilent = FALSE);

	HRESULT	 OnPreviewSingleMessage (CTocDoc *pTocDoc, CSummary *pSum, BOOL bDownloadAttachments);

	HRESULT  FetchAttachmentContentsToFile (IMAPUID uid, char *sequence, const char *Filename, short encoding, LPCSTR pSubtype = NULL);

// COPY/Move
	HRESULT  XferMessagesOnServer (LPCSTR pUidlist, LPCSTR pDestination, BOOL Copy, BOOL bSilent = FALSE);

// Append
	HRESULT  AppendMessageFromLocal (CSummary *pSum, unsigned long *pNewUid, BOOL bSilent = FALSE);
// Append
	HRESULT  AppendMessageAcrossRemotes (CSummary *pSum, unsigned long *pNewUid, BOOL bSilent = FALSE);

// Public interfaces to journal and resyncer object methods.	
	BOOL QueueRemoteXfer (IMAPUID sourceUID, ACCOUNT_ID DestAccountID, LPCSTR pDestImapMboxName, BOOL Copy);

// Access to account's trash management.
	BOOL GetTrashMailboxMbxName (CString& MbxFilePath, BOOL CreateRemote, BOOL bSilent = FALSE);

// Compare pathname to see if the given pathname is our trash.
	BOOL IsImapTrashMailbox (LPCSTR pPathname);

// Delete message on IMAP server.
	HRESULT DeleteMessagesFromServer (LPCSTR pUidList, BOOL Expunge, BOOL bSilent = FALSE);

// Undelete.
	HRESULT UnDeleteMessagesFromServer (LPCSTR pUidList, BOOL bSilent = FALSE);

	HRESULT MoveMsgsToTrash ( LPCSTR pUidlist, CUidMap& mapUidsActuallyRemoved);

	BOOL CImapMailbox::UseFancyTrash();

// Just expunge the mailbox.
	HRESULT Expunge();

// Expunge only specific messages.
	HRESULT UIDExpunge (LPCSTR pUidList, CUidMap& mapUidsActuallyRemoved);

	// Set/Reset message flags on the server.
	HRESULT ImapSetSeenFlag		  (LPCSTR pUidList,  BOOL bSilent = FALSE);
	HRESULT ImapUnsetSeenFlag     (LPCSTR pUidList,  BOOL bSilent = FALSE);
	HRESULT ImapSetAnsweredFlag   (LPCSTR pUidList,  BOOL bSilent = FALSE);
	HRESULT ImapUnsetAnsweredFlag (LPCSTR pUidList,  BOOL bSilent = FALSE);
	HRESULT ModifyRemoteFlags     (LPCSTR pUidList, unsigned ulFlags, BOOL bSet, BOOL bSilent = FALSE);

// Do a server search of all messages for a match.
	HRESULT FindAll (LPCSTR pSearchString, CString& szImapUidMatches);

	HRESULT UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
			LPCSTR pUidStr, CString& szResults);

	// On-server search using MultSearchCriteria.
	//
	HRESULT	DoOnServerSearch (MultSearchCriteria* pMultiSearchCriteria, CString& szResults, LPCSTR pUidRange = NULL);

	BOOL	DoOnServerSearchKnown (MultSearchCriteria* pMultiSearchCriteria, CString& szResults);


// Utility method:
	BOOL Matched (CString& theList, unsigned long ulUid);

// If the given file is one of our "stubs", download the real attachment's contents to the file.	
	HRESULT	DownloadAttachment (LPCSTR szAttachmentPathname);

	HRESULT DownloadAttachmentsBeforeCompose (CString& szAttach, UINT Optype);

// PUBLIC method to download a message. Can be called directly from outside, given a CSummary
	HRESULT  DownloadSingleMessage (CTocDoc *pTocDoc,
									CSummary *pSum,
									BOOL bDownloadAttachments,
									BOOL bToTmpMbx = FALSE);

// This is the CImapSum version:	
	HRESULT  DownloadSingleMessage (CTocDoc *pTocDoc,
									CImapSum *pSum,
									BOOL bDownloadAttachments,
									BOOL bToTmpMbx = FALSE);



	void FilterMessages (CTocDoc *pTocDoc, CSummary *SingleSum = NULL);

	void Ping();

// Utility stuff:
	void	SetFrameName (CTocDoc *pTocDoc);

	void	ShowLastImapError ();

	void	ResetUnseenFlags (CPtrUidList& NewUidList);

	BOOL	ParseHeaderInMemory (CSummary* pSum, LPCSTR pHeader);

	BOOL	VerifyCache (BOOL bSilent = FALSE);

// public static:
public:
	static BOOL ImapServerCanSearch (SearchCriteria *pCurSearchCriteria);

// Access
public:
	// Error functions.
	//
	HRESULT GetLastImapError (LPSTR pBuffer, short nBufferSize);
	HRESULT	GetLastServerMessage (LPSTR pBuffer, short nBufferSize);
	HRESULT	GetLastImapErrorAndType (LPSTR pBuffer, short nBufferSize, int *pType);

	// Set stuff.
	void SetImapType (ImapMailboxType Type)
			{ m_Type = Type; }
	void SetImapName (LPCSTR pImapName)
			{ m_ImapName = pImapName; }

	// Note: m_Dirname is ALWAYS the name of the directory housing all the files that
	// make up the mailbox.
	void SetDirname ( LPCSTR pDirname)
			{ m_Dirname = pDirname; }
	void SetAccountID (ACCOUNT_ID AccountID)
			{ m_AccountID = AccountID; }
	void SetDelimiter ( TCHAR Delimiter )
			{ m_Delimiter = Delimiter; }
	void SetHasUnread (BOOL bHasUnread)
			{ m_bHasUnread = bHasUnread; }
	void SetNoInferiors (BOOL bNoInferiors)
			{ m_bNoInferiors = bNoInferiors; }
	void SetNoSelect (BOOL bNoSelect)
			{ m_bNoSelect = bNoSelect; }
	void SetMarked (BOOL bMarked)
			{ m_bMarked = bMarked; }
	void SetUnMarked (BOOL bUnMarked)
			{ m_bUnMarked = bUnMarked; }

	void SetModifiedFlag (BOOL bState)
			{ m_bModified = bState; }

	void SetNeedsUpdateFromTemp (BOOL bNeedsUpdateFromTemp)
			{ m_bNeedsUpdateFromTemp = bNeedsUpdateFromTemp; }

	void SetReadOnly (BOOL bValue)
			{ m_bReadOnly = bValue; }


	// Access
	void SetUidvalidity(unsigned long ulNewV )
			{ m_Uidvalidity = ulNewV; }

	unsigned long GetUidvalidity()
			{ return m_Uidvalidity; }

	void SetNumberOfMessages (unsigned long nMsgs)
			{ m_NumberOfMessages = nMsgs; }

	unsigned long GetNumberOfMessages ()
			{ return m_NumberOfMessages; }

	unsigned long GetUIDHighest()
			{ return m_UIDHighest; }

	void SetUIDHighest( unsigned long UIDHighest )
			{ m_UIDHighest = UIDHighest; }

	BOOL IsModified ()
			{ return m_bModified; }

	LPCSTR	GetDirname ()
			{ return (LPCSTR) m_Dirname; }
	LPCSTR	GetImapName ()
			{ return (LPCSTR) m_ImapName; }
	ImapMailboxType GetImapType ()
			{ return m_Type; }
	unsigned long	GetAccountID ()
			{ return m_AccountID; }
	TCHAR	GetDelimiter ()
			{ return m_Delimiter; }
	BOOL	HasUnread ()
			{ return m_bHasUnread; }
	BOOL	IsNoInferiors ()
			{ return m_bNoInferiors; }
	BOOL	IsNoSelect ()
			{ return m_bNoSelect; }
	BOOL	IsMarked ()
			{ return m_bMarked; }
	BOOL	IsUnMarked ()
			{ return m_bUnMarked; }
	BOOL	NeedsUpdateFromTemp ()
			{ return m_bNeedsUpdateFromTemp; }
	BOOL	IsReadOnly ()
			{ return m_bReadOnly; }

	BOOL    IsSelected ();

	void	RecreateMessageMap ();

	BOOL	JustCheckedMail ()
			{ return m_bJustCheckedMail; }

	void	SetJustCheckedMail (BOOL Val)
			{ m_bJustCheckedMail = Val; }

	BOOL	AlreadyCheckingMail ()
			{ return m_bAlreadyCheckingMail; }

	void    SetAlreadyCheckingMail (BOOL Val)
			{ m_bAlreadyCheckingMail = Val; }

	void	SetTocJustRebuilt ()
			{ m_bTocWasJustRebuilt = TRUE; }

	BOOL	EscapePressed ()
			{ return m_bEscapePressed; }

	BOOL	PreviewAllowed ()
			{ return m_bPreviewAllowed; }

	void	SetPreviewAllowed (BOOL bVal)
			{ m_bPreviewAllowed = bVal; }

	// Dig into the HRESULT to see if it contains our "cancel" error code.
	//
	BOOL	DidUserCancel (HRESULT hResult);

	void	SetLoginBusy(BOOL bVal)
			{ m_bLoginBusy = bVal; }

	BOOL	IsLoginBusy()
			{ return m_bLoginBusy; }

	CIMAP*	GetImapAgent()
			{ return m_pImap; }

	CTocDoc *GetParenToc()
			{ return m_pParentToc; }

	void	SetParentToc (CTocDoc *pTocDoc)
			{ m_pParentToc = pTocDoc; }

// Interface to internal settings object.
//
public:
	short	GetSettingsShort	(UINT StringNum);
	long	GetSettingsLong		(UINT StringNum);
	LPCSTR	GetSettingsString	(UINT StringNum, char *Buffer = NULL, int len = -1);

// Internal functions:
private:

	HRESULT DoBackgroundResync (CTocDoc *pTocDoc,
								BOOL bCheckMail = FALSE,
								BOOL bDownloadedOnly = TRUE);

	HRESULT DoForegroundResync (CTocDoc *pTocDoc,
								BOOL bDownloadedOnly = TRUE);

	HRESULT GetAndFilterNewMessages (CTocDoc *pTocDoc, BOOL& bWasResynced, BOOL bDoFiltering = TRUE, BOOL bSilent = FALSE);

	HRESULT ResyncMailbox (CTocDoc *pTocDoc, BOOL bSilent = FALSE, 
						   IMAPUID ulMaxUid = 0xFFFFFFFF,
						   BOOL bDownloadedOnly = TRUE);

	HRESULT	CheckMail (CTocDoc *pTocDoc, BOOL bSilent = FALSE, BOOL bForceResync = FALSE);

	HRESULT DoFetchNewMessages (CTocDoc *pTocDoc, CUidMap& NewUidMap, BOOL bDoFiltering);

	HRESULT DownloadRemainingMessages (CTocDoc *pTocDoc, CUidMap& NewUidMap,
							 CImapSumList *pSumList, BOOL bToTmpMbx = FALSE, CTaskInfoMT* pTaskInfo = NULL);


	HRESULT DoMinimalDownload (CTocDoc *pTocDoc, IMAPUID Uid, CImapSum **ppSum);

	BOOL	ImapAppendSumList (CTocDoc* pTocDoc,
							  CImapSumList* pSumList,
							  CSumList* pRefSumList,
							  BOOL bFromTmpMbx = FALSE);

	void	UpdateOldSummaries (CTocDoc *pTocDoc, CPtrUidList& CurrentUidList);

	// MAP version:
	void	UpdateOldSummaries (CTocDoc *pTocDoc, CUidMap& CurrentUidMap);

	HRESULT	AcquireNetworkConnection ();

	HRESULT WriteOfflineMessage (CTocDoc* pTocDoc, CImapSum* pSum, BOOL bToTmpMbx = FALSE);

	void GrabSettings();

	HRESULT DoOnServerSearchInChunks_ (MultSearchCriteria* pMultiSearchCriteria, CString& szResults, LPCSTR pUidRange = NULL);
	LPSTR	ExpandUidRange (LPCSTR pUidRange);

// public members.
public:
	// A CImapMailbox contains an instance of a CIMAP which may or may NOT be open.
	CIMAP*		m_pImap; 


// Internal state:
private:
	ImapMailboxType		m_Type;			// MBT_IMAP_NAMESPACE, MBT_IMAP_SERVICE or MBT_IMAP_MAILBOX.
	unsigned long		m_Uidvalidity;	// unique ID, used in locating the folder in
										// the service tree structure.
	unsigned long		m_NumberOfMessages; 
	IMAPUID				m_UIDHighest;

	// We cache this information for easy access.
	CString			m_ImapName;			// Complete imap name, possibly including machine name.
										// Also used as the "reference" for child fetches.
	CString			m_Dirname;
	ACCOUNT_ID		m_AccountID;			// ID of owning account.
	TCHAR			m_Delimiter;			// For this folder hierarchy.
	BOOL			m_bHasUnread;			// If has unread mesages.
	BOOL			m_bPartial;				// Has at least 1 message with un-downloaded part. 
	BOOL			m_bNoInferiors;			// If can never contain other folders.
	BOOL			m_bNoSelect;			// If cannot be selected.
	BOOL			m_bMarked;
	BOOL			m_bUnMarked;
	BOOL			m_bReadOnly;

	// Each mailbox has associated journal and resyncer objects.
	// The mailbox CONTAINS" these, so initialize them when a mailbox object is
	// created, and clean them up when we are destroyed.

	CImapJournaler	m_Journaler;
	CImapResyncer   m_Resyncer;

	// internal Modified state.
	BOOL	m_bModified;

	BOOL	m_bNeedsUpdateFromTemp;

	BOOL	m_bEscapePressed;

	BOOL	m_bExpunged;

	// Set this to TRUE soon after we've done a check mail.
	BOOL	m_bJustCheckedMail;

	BOOL	m_bPreviewAllowed;

	// Used to prevent re-entrance problems.
	BOOL	m_bAlreadyCheckingMail;

	// For internal use.
	BOOL	m_bForceResync;

	// Client can set this to TRUE via "SetTocJustRebuilt()".
	// 
	BOOL	m_bTocWasJustRebuilt;

	BOOL	m_bUidValidityChanged;

	// If this is TRUE, then we're doing a mailcheck or something. Queue commands
	// while this is set.
	//
	BOOL	m_bLoginBusy;

	// Internally created settings object. We have out own.
	//
	CImapSettings*		m_pImapSettings;

	// Careful eith this!! Pointer to containing CTocDoc.
	//
	CTocDoc *m_pParentToc;

	// Name of this mailbox as displayed in the mbox tree.
	//
	CString m_FriendlyName;

	// This is an interface to checking mail, and needs to use
	// CImapMailbox to do the work.
	//
	friend CImapChecker;
};



// ========= ImapMailboxNode management functions. =====================//
ImapMailboxNode *NewImapMailboxNode();
void DeleteFolderSubtree (ImapMailboxNode *pStartingFolder);
void DeleteMailboxNode (ImapMailboxNode* pFolder);
void DeleteChildList (ImapMailboxNode *pParentFolder);
void DeleteSiblingList (ImapMailboxNode *pGivenFolder);
void SetImapAttributesFromImapNode (CImapMailbox* pImapMbox, ImapMailboxNode *pNode);

// =============== Exported =====================/
void StripTrailingDelimiter (CString &myName);
BOOL CreateMailboxDirectory (LPCSTR pDirname);
BOOL CreateMbxFile (LPCSTR pMailboxDir);
BOOL CreateAttachDir (LPCSTR pPathname);
void GetAttachDirPath (LPCSTR pPathname, CString& FullPath);
BOOL GetTmpMbxFilePath (LPCSTR pMailboxDir, CString& TmpMbxFilePath);
BOOL GetMbxFilePath (LPCSTR pMailboxDir, CString& MbxFilePath);
BOOL MbxFilePathToMailboxDir (LPCSTR pMbxFilepath, CString& MailboxDir);
BOOL GetInfFilePath (LPCSTR pMailboxDir, CString& InfFilePath);
BOOL ChangeFileSize (LPCSTR pFilepath, long length);
BOOL CreateLocalFile (LPCSTR pFilepath, BOOL Truncate);
void DeleteLocalFile (LPCSTR pPathname);
BOOL RemoveLocalDirectory (LPCSTR pPathname, BOOL RemoveChildren);
void MakePath (LPCSTR pParent, LPCSTR pName, CString &Path);
LPSTR CopyString (LPCSTR buf);
BOOL ImapIsTmpMbx (LPCSTR pFilename);
BOOL ImapCopyAttachmentFile (LPCSTR pSourceAttachmentDir, LPCSTR pTargetMailboxDir, LPCSTR pFilename, CString& NewPathname);
BOOL GetUniqueTempFilepath (LPCSTR pParentDir, LPCSTR pBaseName, CString &NewName);
void FormulateFriendlyImapName (LPCSTR pImapName, TCHAR DelimiterChar, CString& FriendlyName);

BOOL RenameMboxDirAndContents (LPCSTR pOldDirpath, ImapMailboxNode *pNode, CString& szNewDirname, LPCSTR pNewParentDir = NULL);

BOOL RenameInternalMailboxFiles (LPCSTR pOldDirPathname, LPCSTR pNewDirPathname);
BOOL CreateDirectoryForMailbox (LPCSTR pParentDir, ImapMailboxNode *pImapNode, CString &NewDirName);
void  ShowLastImapError (CIMAP *pImap);
void ImapBitflagsToFlaglist (unsigned ulBitflags, CString& szFlaglist);
BOOL IsInbox (LPCSTR pName);
BOOL ExpungeAfterDelete (CTocDoc *pTocDoc);

unsigned long GetLocalHighestUid (CPtrUidList& CurrentUidList);
void GetCurrentUidList (CTocDoc *pTocDoc, CPtrUidList& CUrrentUidList);
void DeleteAllFromSumlist (CSumList* pSumlist);
int MyCompareRStringI(UINT StringNum, const char* String, int len = -1);
int MyCompareRString(UINT StringNum, const char* String, int len = -1);

void MergeUidListsMT (CPtrUidList& CurrentUidList, CPtrUidList& NewUidList, CPtrUidList& NewOldUidList);

void GetCurrentUids (CTocDoc *pTocDoc, CUidMap& UidMap);


#endif // __IMAPFOL_H