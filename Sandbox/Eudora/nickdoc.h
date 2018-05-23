#ifndef __NICKDOC_H__
#define __NICKDOC_H__

// NICKDOC.H
//
// Routines for handling the content of nicknames
//

#include "QICommandClient.h"
//
// User-defined message for forwarding property page
// resize message to parent form view after we switch
// to a new property page.
//
#define WM_USER_RESIZE_PAGE  WM_USER+42


//
// Status codes for results of analyzing LHS
// selection for nicknames that also on the
// quick recipient menu.
//
#define RL_NONE 0x0
#define RL_ON 0x1
#define RL_OFF 0x2
#define RL_BOTH (RL_ON | RL_OFF)

//
// Internally-used "field name" strings for the default "View By" combo
// box entries in the LHS view of the nicknames window.
//
#define NICKNAME_FIELD_NICKNAME "$Nickname$"
#define NICKNAME_FIELD_ADDRESSES "$Addresses$"


// see CNicknamesViewRight::OnUpdate()
enum NicknamesHintType
{
	NICKNAME_HINT_FIRST = 100,						// must always be first (not used)
	NICKNAME_HINT_DISPLAYNICKNAME,					// RHS should display data for singly-selected nickname
	NICKNAME_HINT_DISPLAYNICKNAME_READONLY,	 		// RHS should display read-only data for singly-selected nickname
	NICKNAME_HINT_MULTISELECT_RECIPLIST_DISABLE,	// RHS notified that we have multi-selection with no nicknames selected
	NICKNAME_HINT_MULTISELECT_RECIPLIST_CHECK,		// RHS notified that we have multi-selection with all nicknames on recipient list
	NICKNAME_HINT_MULTISELECT_RECIPLIST_UNCHECK,	// RHS notified that we have multi-selection with one or more nicknames not on recipient list
	NICKNAME_HINT_RECIPLIST_ADD,					// user wants selection added to recipient list
	NICKNAME_HINT_RECIPLIST_REMOVE,					// user wants selection removed from recipient list
	NICKNAME_HINT_RECIPLIST_CHANGED,				// notification of external recipient list change
	NICKNAME_HINT_MARK_NICKFILE_DIRTY,				// notification of edit change to selected nickname
	NICKNAME_HINT_REQUESTING_NICKNAME,				// RHS requesting current nickname from LHS
	NICKNAME_HINT_SENDING_NICKNAME,					// LHS sending current nickname to RHS (response to REQUESTING_NICKNAME)
	NICKNAME_HINT_NICKNAME_WAS_CHANGED,				// user changed an existing nickname, so update LHS view
	NICKNAME_HINT_ADDED_NEW_NICKNAME,				// notification of externally added new nickname
	NICKNAME_HINT_ADDED_NEW_NICKFILE,				// notification of externally added new nickname file
	NICKNAME_HINT_ACTIVATE_ADDRESS_PAGE,			// RHS should display Addresses page and set focus there
	NICKNAME_HINT_FETCH_SELECTED_NICKNAMES,			// generic request for LHS to return list of selected CNickname items
	NICKNAME_HINT_REGISTER_VIEWBY_ITEM,				// RHS sending custom View By item to LHS
	NICKNAME_HINT_RHS_CLOSED,						// RHS pane is closed
	NICKNAME_HINT_RHS_OPEN,							// RHS pane is open
	NICKNAME_HINT_REFRESH_LHS,						// refresh LHS tree display
	NICKNAME_HINT_LAST								// must always be last (not used)
};

class CNickname : public CObject
{
	DECLARE_DYNCREATE(CNickname)

public:
	enum
	{
		MAX_NAME_LENGTH = 32
	};

	CNickname(const char* Name = NULL);
	CNickname(const CNickname&);

	const CString& GetName() const;
	void SetName(const CString& newName);
	const CString& GetAddresses() const { return (m_Addresses); }
	CString GetElaboratedAddresses() const;
	void SetAddresses(const CString& newAddress) { m_Addresses = newAddress; }
	const CString& GetRawNotes() const { return (m_Notes); }
	void SetRawNotes(const CString& newNotes) { m_Notes = newNotes; }
	CString GetNotes() const;
	void SetNotes(const CString& newNotes);
	long GetRawNotesLength() const	{ return (m_NotesLength); }
	void SetRawNotesLength(long newLength) { m_NotesLength = newLength; }
	BOOL GetNotesFieldValue(const CString& fieldName, CString& fieldValue) const;
	BOOL SetNotesFieldValue(const CString& fieldName, const CString& fieldValue);
	BOOL IsRecipient() const { return (m_Recipient); }
	BOOL IsVisited() const { return (m_Visited); }
	void SetVisited(BOOL isVisited) { m_Visited = isVisited; }
	long GetAddressesOffset() const	{ return (m_AddressesOffset); }
	void SetAddressesOffset(long newOffset) { m_AddressesOffset = newOffset; }
	long GetNotesOffset() const { return (m_NotesOffset); }
	void SetNotesOffset(long newOffset)	{ m_NotesOffset = newOffset; }
	long GetAddressesLength() const	{ return (m_AddressesLength); }
	void SetAddressesLength(long newLength) { m_AddressesLength = newLength; }
	
	BOOL NeedsReadIn() { return ((m_AddressesLength > 0 && m_Addresses.IsEmpty()) || ( m_NotesLength > 0 && m_Notes.IsEmpty())); }
	void UpdateRecipientList();
	void ToggleRecipient()
		{ m_Recipient = !m_Recipient; UpdateRecipientList(); }
	BOOL HasValidName() const;
	void AddRecipient()
		{ m_Recipient = TRUE; UpdateRecipientList(); }
	void RemoveRecipient()
		{ m_Recipient = FALSE; UpdateRecipientList(); }
	void SetRecipient( BOOL bValue )
		{	m_Recipient = bValue; }

private:
	BOOL RemoveFieldsFromNotes(CString& notesString, CStringList& fieldList) const;
	BOOL UpdateFieldList(CStringList& fieldList, const CString& fieldName, const CString& fieldValue) const;

	CString		m_Name;					// Name
	CString		m_Addresses;			// Addresses
	CString		m_Notes;				// Notes
	BOOL		m_Recipient;			// Is it on the recipient list?
	BOOL		m_Visited;				// Used for checking for circular nicknames when expanding
	long		m_AddressesOffset;		// Offset of addresses section in nicknames file
	long		m_NotesOffset;			// Offset of notes section in nicknames file
	long		m_AddressesLength;		// Length of addresses section in nicknames file
	long		m_NotesLength;			// Length of notes section in nicknames file
};


class CNicknamesDoc;
class JJFile;

class CNicknameFile : public CObList
{
	DECLARE_DYNCREATE(CNicknameFile)

public:
	CNicknameFile(const char* Filename);
	~CNicknameFile();

	const CString& GetName() const	{ return m_Name;}
	CNickname* GetAt(POSITION position) const
		{ return ((CNickname*)CObList::GetAt(position)); }
	CNickname*& GetAt(POSITION position)
		{ return ((CNickname*&)CObList::GetAt(position)); }
	CNickname*& GetHead()
		{ return ((CNickname*&)CObList::GetHead()); }
	CNickname* GetHead() const
		{ return ((CNickname*)CObList::GetHead()); }
	CNickname*& GetTail()
		{ return ((CNickname*&)CObList::GetTail()); }
	CNickname* GetTail() const
		{ return ((CNickname*)CObList::GetTail()); }
	CNickname*& GetNext(POSITION& rPosition)
		{ return ((CNickname*&)CObList::GetNext(rPosition)); }
	CNickname* GetNext(POSITION& rPosition) const
		{ return ((CNickname*)CObList::GetNext(rPosition)); }
	CNickname*& GetPrev(POSITION& rPosition)
		{ return ((CNickname*&)CObList::GetPrev(rPosition)); }
	CNickname* GetPrev(POSITION& rPosition) const
		{ return ((CNickname*)CObList::GetPrev(rPosition)); }
	CNickname* RemoveHead()
		{ return ((CNickname*)CObList::RemoveHead()); }
	CNickname* RemoveTail()
		{ return ((CNickname*)CObList::RemoveTail()); }
	
	CNickname* CreateNewNickname(const char* Name);
	CNickname* AddNickname(const CNickname& nickName, BOOL bAddToRecipientList = FALSE);
	void RemoveAt(POSITION pos);
	void RemoveAll();
	
	CNickname* FindNickname(const char* Name);
	POSITION FindNicknameStartingWith(const char* Prefix, POSITION pos = NULL);

	BOOL ReadToc();
	BOOL ReadNickname(CNickname* nn);
	BOOL WriteTxt();
	BOOL WriteToc();

	BOOL IsModified()
		{ return (m_IsModified); }
	void SetModified(BOOL IsModified = TRUE);

	CString m_Filename;
	CString m_Name;
	BOOL m_ReadOnly;

	// Peanut can create nickname files that should be show up in the UI
	BOOL m_Invisible;
	void Refresh();
	
protected:
	enum { ADDRESSES, NOTES };
	enum { TocVersion = 0x2A };

	CNicknameFile() { };		// do not use, for MFC RTTI only
	
	BOOL ReadOld(JJFile& in);
	BOOL ReadMailrc(JJFile& in);
	BOOL RebuildToc();

	BOOL m_IsModified;
};


/////////////////////////////////////////////////////////////////////////////
// CNicknamesDoc document

class CNicknamesDoc : public CDoc, public QICommandClient
{
	DECLARE_DYNCREATE(CNicknamesDoc)
protected:
	CNicknamesDoc();

// Attributes
public:

// Operations
public:
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(const char* pszPathName);

	BOOL ReadTocs();
	void UpdateSharewareFeatureMode();

	CNicknameFile* GetFirstNicknameFile();
	CNicknameFile* GetNextNicknameFile();
	void UpdateNicknameFile(const CString& fileName);


	int GetNumWritableFiles() const;

	CNicknameFile* AddNicknameFile(const CString& fileName, BOOL bPrepend  = FALSE);
	BOOL RemoveNicknameFile(CNicknameFile* pNicknameFile);
	CNickname* Find(const char* Name, int Start = 0, BOOL OneFileOnly = FALSE);
	CNickname* ReadNickname(const char* Name, int Start = 0, BOOL OneFileOnly = FALSE);
	BOOL GetNicknames(CString& nickNames);
	
	void RegisterNicknameFieldName(const CString& fieldName);
	BOOL IsNicknameFieldName(const CString& fieldName);

	void Notify( 
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

// Implementation
protected:
	~CNicknamesDoc();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNicknamesDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void NukeAllData();
	BOOL ReadTocDir(const char* Dir);

private:
	CObList m_NicknameFiles;
	POSITION m_currentPos;
	CStringList m_NicknameFieldNameList;

	bool m_bSavedFeatureMode; // Save the feature mode
};


// Data Items
extern CNicknamesDoc* g_Nicknames;


// Function Prototypes

//FORNOWBOOL LoadNicknames();
char* ExpandAliases(const char* Addresses, BOOL domainQualify = TRUE, BOOL StripFCC = FALSE,
	BOOL bElaborateNicknames = TRUE, BOOL bDisplayError = FALSE);
void FinishNickname(CWnd* pWnd);
//void FinishNNAutoComplete(CWnd* pWnd);
void ConvertEndOfLinesToCommas(CString& itemText);

#endif // __NICKDOC_H__
