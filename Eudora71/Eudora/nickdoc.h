// nickdoc.h
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
// quick recipient menu & BP menu.
//
#define LIST_NONE 0x0
#define LIST_ON 0x1
#define LIST_OFF 0x2
#define LIST_BOTH (LIST_ON | LIST_OFF)

//
// Internally-used "field name" strings for the default "View By" combo
// box entries in the LHS view of the nicknames window.
//
#define NICKNAME_FIELD_NICKNAME "$Nickname$"
#define NICKNAME_FIELD_ADDRESSES "$Addresses$"


// see CNicknamesViewRight::OnUpdate()
enum NicknamesHintType
{
	NICKNAME_HINT_FIRST = 100,							// must always be first (not used)
	NICKNAME_HINT_DISPLAYNICKNAME,						// RHS should display data for singly-selected nickname
	NICKNAME_HINT_DISPLAYNICKNAME_READONLY,	 			// RHS should display read-only data for singly-selected nickname
	NICKNAME_HINT_NOSELECT_RECIPLIST_DISABLE,			// RHS notified that we have no nicknames selected
	NICKNAME_HINT_SINGLE_NICKFILE_SELECTED,				// RHS notified when selection is a single nickname file
	NICKNAME_HINT_MULTI_NICKFILE_SELECTED,				// RHS notified when multiple selections that may contains nickname files
	NICKNAME_HINT_MULTISELECT_RECIPLIST_CHECK,			// RHS notified that we have multi-selection with all nicknames on recipient list
	NICKNAME_HINT_MULTISELECT_BPLIST_CHECK,				// RHS notified that we have multi-selection with all nicknames on BP list
	NICKNAME_HINT_MULTISELECT_RECIPLIST_BPLIST_CHECK,	// RHS notified that we have multi-selection with all nicknames on recipient and BP list
	NICKNAME_HINT_MULTISELECT_RECIPLIST_BPLIST_UNCHECK,	// RHS notified that we have multi-selection with one or more nicknames not on recipient & BP list
	NICKNAME_HINT_RECIPLIST_ADD,						// user wants selection added to recipient list
	NICKNAME_HINT_RECIPLIST_REMOVE,						// user wants selection removed from recipient list
	NICKNAME_HINT_RECIPLIST_CHANGED,					// notification of external recipient list change
	NICKNAME_HINT_MARK_NICKFILE_DIRTY,					// notification of edit change to selected nickname
	NICKNAME_HINT_REQUESTING_NICKNAME,					// RHS requesting current nickname from LHS
	NICKNAME_HINT_SENDING_NICKNAME,						// LHS sending current nickname to RHS (response to REQUESTING_NICKNAME)
	NICKNAME_HINT_NICKNAME_WAS_CHANGED,					// user changed an existing nickname, so update LHS view
	NICKNAME_HINT_ADDED_NEW_NICKNAME,					// notification of externally added new nickname
	NICKNAME_HINT_ADDED_NEW_NICKFILE,					// notification of externally added new nickname file
	NICKNAME_HINT_ACTIVATE_ADDRESS_PAGE,				// RHS should display Addresses page and set focus there
	NICKNAME_HINT_FETCH_SELECTED_NICKNAMES,				// generic request for LHS to return list of selected CNickname items
	NICKNAME_HINT_REGISTER_VIEWBY_ITEM,					// RHS sending custom View By item to LHS
	NICKNAME_HINT_RHS_CLOSED,							// RHS pane is closed
	NICKNAME_HINT_RHS_OPEN,								// RHS pane is open
	NICKNAME_HINT_REFRESH_LHS,							// refresh LHS tree display
	NICKNAME_HINT_BP_ADD,								// user wants selection added to BP list
	NICKNAME_HINT_BP_REMOVE,							// user wants selection removed from BP list
	NICKNAME_HINT_LAST									// must always be last (not used)
};

class CNicknameFile;

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
	void SetName(const char* newName);
	const CString& GetAddresses() const { return (m_Addresses); }
	CString GetElaboratedAddresses() const;
	void SetAddresses(const char* newAddress) { m_Addresses = newAddress; }
	const CString& GetRawNotes() const { return (m_Notes); }
	void SetRawNotes(const char* newNotes) { m_Notes = newNotes; }
	CString GetNotes() const;
	void SetNotes(const char* newNotes);
	long GetRawNotesLength() const	{ return (m_NotesLength); }
	void SetRawNotesLength(long newLength) { m_NotesLength = newLength; }
	BOOL GetNotesFieldValue(const char* fieldName, CString& fieldValue) const;
	BOOL SetNotesFieldValue(const char* fieldName, const char* fieldValue);
	CString GetOtherAddresses() const;
	BOOL IsRecipient() const { return (m_Recipient); }
	BOOL IsVisited() const { return (m_Visited); }
	void SetVisited(BOOL isVisited) { m_Visited = isVisited; }
	long GetAddressesOffset() const	{ return (m_AddressesOffset); }
	void SetAddressesOffset(long newOffset) { m_AddressesOffset = newOffset; }
	long GetNotesOffset() const { return (m_NotesOffset); }
	void SetNotesOffset(long newOffset)	{ m_NotesOffset = newOffset; }
	long GetAddressesLength() const	{ return (m_AddressesLength); }
	void SetAddressesLength(long newLength) { m_AddressesLength = newLength; }
	void SetNickContainingFile(CString szFilename);
	CString GetNickContainingFile();
	CNicknameFile* GetNickFile();

	BOOL NeedsReadIn() { return ((m_AddressesLength > 0 && m_Addresses.IsEmpty()) || ( m_NotesLength > 0 && m_Notes.IsEmpty())); }
	void UpdateRecipientList();
	void ToggleRecipient()
		{ m_Recipient = !m_Recipient; UpdateRecipientList(); }
	BOOL HasValidName() const;
	void AddRecipient()
		{ m_Recipient = TRUE; UpdateRecipientList(); }
	void RemoveRecipient()
		{ m_Recipient = FALSE; UpdateRecipientList(); }
	void AddToBP();
	void RemoveFromBP();
	void SetRecipient( BOOL bValue )
		{	m_Recipient = bValue; }
	void SetBeingMoved ( BOOL bValue ) { m_BeingMoved = bValue; }
	BOOL IsBeingMoved () { return m_BeingMoved; }
	BOOL RemoveFieldsFromNotes(CString& notesString, CStringList& fieldList) const;
	CString CreateSimpleHTML();
	CString CreateSimpleCSV(const char* replace = " ");
	CString CreateSimpleHTMLTableRow(const char* fieldName= NULL,   const char *fieldValue= NULL, bool bOnlyOnValue = TRUE);
	CString ProcessCSVCell(const char* field, const char* replace = " ");
	void SetValidateMode(bool bValidate = true){ m_bValidate = bValidate;}
	bool GetValidateMode(){ return m_bValidate;}
    CString CreateValidName(const char*);
	static BOOL IsValidNickName(const char*);
	unsigned long GetHash() {return m_lHash;}

private:
	BOOL UpdateFieldList(CStringList& fieldList, const CString& fieldName, const CString& fieldValue) const;

	CString		m_Name;					// Name
	CString		m_Addresses;			// Addresses
	CString		m_Notes;				// Notes
	CString		m_szContainingFile;		// Which file is it in?
	BOOL		m_Recipient;			// Is it on the recipient list?
	BOOL		m_Visited;				// Used for checking for circular nicknames when expanding
	long		m_AddressesOffset;		// Offset of addresses section in nicknames file
	long		m_NotesOffset;			// Offset of notes section in nicknames file
	long		m_AddressesLength;		// Length of addresses section in nicknames file
	long		m_NotesLength;			// Length of notes section in nicknames file
	BOOL		m_BeingMoved;			// Set when we are moving nickname from one file to another
										//   so that we know not to delete it from the recipient list
	bool m_bValidate; //perform all nickname operations with or  without validating it.
	
	unsigned long	m_lHash;			// Hash of nickname
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

	void SetFileName(const char* Filename) { m_Filename = Filename; }
	
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
	CNickname* AddNickname(const CNickname& nickName, BOOL bAddToRecipientList = FALSE, BOOL bAddToBPList = FALSE);
	void RemoveAt(POSITION pos);
	void RemoveAll();
	
	CNickname* FindNickname(const char* Name);
	POSITION FindNicknameStartingWith(const char* Prefix, POSITION pos = NULL);

	BOOL ReadToc();
	BOOL ReadNickname(CNickname* nn, JJFile& in);
	BOOL WriteTxt();
	BOOL WriteToc();

	BOOL IsPluginNickFile();

	BOOL IsModified()
		{ return (m_IsModified); }
	void SetModified(BOOL bIsModified = TRUE, BOOL bForgetHashes = TRUE);

	CString m_Filename;
	CString m_Name;
	BOOL m_ReadOnly;

	// Peanut can create nickname files that should be show up in the UI
	BOOL m_Invisible;
	void Refresh();

	BOOL IsBPList()
		{ return (m_bBPList); }
	void SetBPList(BOOL bBPList = TRUE, BOOL bUpdateINISetting = TRUE);

protected:
	enum { ADDRESSES, NOTES };
	enum { TocVersion = 0x2A };

	CNicknameFile() { };		// do not use, for MFC RTTI only
	
	BOOL ReadOld(JJFile& in);
	BOOL ReadMailrc(JJFile& in);
	BOOL RebuildToc();

	BOOL m_IsModified;
	BOOL m_bBPList;
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
	void UpdateNicknameFile(const char* fileName);


	int GetNumWritableFiles() const;

	CNicknameFile* AddNicknameFile(const char* fileName, const CString& nickfileBPList, BOOL bPrepend  = FALSE);
	BOOL RemoveNicknameFile(CNicknameFile* pNicknameFile);
	CNickname* Find(const char* Name, int Start = 0, BOOL OneFileOnly = FALSE);
	CNickname* ReadNickname(const char* Name, JJFile& in, int Start = 0, BOOL OneFileOnly = FALSE);
	BOOL GetNicknames(CString& nickNames);
	BOOL GetNicknamesFromFile(CString file, CString& nickNames);
	
	void RegisterNicknameFieldName(const char* fieldName);
	BOOL IsNicknameFieldName(const char* fieldName);

	void Notify( 
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );
	bool WriteToHTML(const char* fileName);
	bool WriteToCSV(const char* fileName);
	CString ConvertToHTMLBasic(const char* Notes);
	void ChangeNickFileName(const char * OldFilename, const char * NewFilename);
	bool IsAddressOnBPList(const CString & address);
	void AddNickNameToBPList(CNickname* nickname);
	void RemoveNickNameFromBPList(CNickname* nickname);
	void AddABToBPList(CNicknameFile* nickfile);
	void RemoveABFromBPList(CNicknameFile* nickfile);

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
	BOOL ReadTocDir(const char* Dir, const CString &nickfileBPList);

private:
	void AddNickFileToBPList(const CString &nickfile);
	void AddRemoveNickNamesToBPList(const CString &name, bool bIsNickFile = true, bool bAdd = true );
	void AddAddressToBPList(const CString & address);
	void RemoveAddressFromBPList(const CString & address);
	void RebuildBPMap();

private:
	CObList m_NicknameFiles;
	POSITION m_currentPos;
	CStringList m_NicknameFieldNameList;
	//CStringList m_BPAddressList; // list of all addresses on the BP list. 
	CMap< CString, LPCSTR, int, int > m_BPAddressMap; // map between BP addresses to its counter 
	bool m_bSavedFeatureMode; // Save the feature mode
};


// Data Items
extern CNicknamesDoc* g_Nicknames;


// Function Prototypes

//FORNOWBOOL LoadNicknames();
char* ExpandAliases(const char* Addresses, BOOL domainQualify = TRUE, BOOL StripFCC = FALSE,
	BOOL bElaborateNicknames = TRUE, BOOL bDisplayError = FALSE, BOOL bUseOtherField = FALSE, JJFile* pABFile = NULL);
void FinishNickname(CWnd* pWnd);
//void FinishNNAutoComplete(CWnd* pWnd);
void ConvertEndOfLinesToCommas(CString& itemText);
void RenameABookExtensions(CString oldDBName, CString newDBName, CString fromFileExtn, CString toFileExtn);
void UpdateNicknameWithNewContainingNickfile();
bool DoesABookExtnExist(CString ABookName, CString ABookExtn);

#endif // __NICKDOC_H__
