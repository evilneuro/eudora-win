// SUMMARY.H
//
// Routines for message summaries
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#ifndef _SUMMARY_H_
#define _SUMMARY_H_

#include "resource.h"
#include "rs.h"

#include <map>

// Data Types

// MesSummary flags (used with m_Flags)
const WORD MSF_ALT_SIGNATURE	= 0x0001;
const WORD MSF_USE_SIGNATURE	= 0x0002;
const WORD MSF_WORD_WRAP		= 0x0004;
const WORD MSF_TABS_IN_BODY		= 0x0008;
const WORD MSF_KEEP_COPIES		= 0x0010;
const WORD MSF_TEXT_AS_DOC		= 0x0020;
const WORD MSF_RETURN_RECEIPT	= 0x0040;
const WORD MSF_QUOTED_PRINTABLE	= 0x0080;
const WORD MSF_ENCODE0			= 0x0100;
const WORD MSF_ENCODE1			= 0x0200;
const WORD MSF_SHOW_ALL_HEADERS	= 0x0400;
const WORD MSF_SUB_PART			= 0x0800;
const WORD MSF_MAPI_MESSAGE		= 0x1000;
const WORD MSF_XRICH			= 0x2000;
const WORD MSF_READ_RECEIPT		= 0x4000;
const WORD MSF_HAS_ATTACHMENT	= 0x8000;
const WORD MSF_COMP_MOD_FLAGS	= 0x8FFF;
const WORD MSF_BINHEX			= 0;
const WORD MSF_MIME				= MSF_ENCODE0;
const WORD MSF_UUENCODE			= MSF_ENCODE1;

// MesSummary extended flags (used with m_FlagsEx)
const WORD MSFEX_AUTO_ATTACHED	= 0x0001;
const WORD MSFEX_HTML			= 0x0002;
const WORD MSFEX_MDN			= 0x0004;
const WORD MSFEX_MIME_ATTACHED	= 0x0008;
const WORD MSFEX_SEND_PLAIN		= 0x0010;
const WORD MSFEX_SEND_STYLED	= 0x0020;
const WORD MSFEX_FLOWED			= 0x0040;
const WORD MSFEX_INL_SIGNATURE  = 0x0080;
const WORD MSFEX_EMPTY_BODY		= 0x0100;

// MesSummary states
const char MS_UNREAD		= 0;
const char MS_READ			= 1;
const char MS_REPLIED		= 2;
const char MS_FORWARDED		= 3;
const char MS_REDIRECT		= 4;
const char MS_UNSENDABLE	= 5;
const char MS_SENDABLE		= 6;
const char MS_QUEUED		= 7;
const char MS_SENT			= 8;
const char MS_UNSENT		= 9;
const char MS_TIME_QUEUED	=10;
const char MS_SPOOLED		=11;
const char MS_RECOVERED		=12;

// MesSummary priorites
const short MSP_HIGHEST	= 1;
const short MSP_HIGH	= 2;
const short MSP_NORMAL	= 3;
const short MSP_LOW		= 4;
const short MSP_LOWEST	= 5;

// MesSummary Mood
const char MSM_MOOD_UNKNOWN = 0;
const char MSM_MOOD_CLEAN = 1;
const char MSM_MOOD_LOW = 2;
const char MSM_MOOD_MEDIUM = 3;
const char MSM_MOOD_HIGH = 4;

// Imap message flags :
const unsigned long IMFLAGS_SEEN		= 0x00000001;
const unsigned long IMFLAGS_ANSWERED	= 0x00000002;
const unsigned long IMFLAGS_FLAGGED		= 0x00000004;
const unsigned long IMFLAGS_DELETED		= 0x00000008;
const unsigned long IMFLAGS_DRAFT		= 0x00000010;
const unsigned long IMFLAGS_RECENT		= 0x00000020;
////////////////////////////////////////////////////////////////////////////////////////////
// Note:
// The TOC entry for a message is filling up fast. Try to conserve space by using
// bits 16 to 20 (5 bits) of m_ImFlags to indicate the number of un-downloaded attachments.
// This gives a maximum of 32 possible undownloaded attachments.
// The CSummary entry named "m_nUndownloadedAttachments" maps to these 5 bits.
// The foll. masks these bits.
//
const unsigned long IMFLAGS_UNDOWNLOADED_ATTACHMENTS = 0x001F0000;
const int UNDOWNLOADED_ATTACHMENTS_BIT_SHIFT = 16;
/////////////////////////////////////////////////////////////////////////////////////////////

// Set if the message's full header was downloaded from IMAP.
const unsigned long IMFLAGS_FULL_HEADER	= 0x80000000;
// Indicate that this message was not downloaded. Only a fetch of
// minimal headers was performed.
const unsigned long IMFLAGS_NOT_DOWNLOADED	= 0x40000000;

extern char StateFlag[];


// Some forward declarations
class JJFile;
class CTocDoc;
class CMessageDoc;
class CCompMessageDoc;
class CCompStationeryDoc;
class CMessageFrame;

class CSummary : public CObject
{
	CRect m_SavedPos;

	friend class CTocDoc;

	DECLARE_DYNAMIC(CSummary)
public:
	enum { kDisplayOrQueueImmediately = true, kDontDisplayOrQueue = false };

	CSummary();
	~CSummary();

	const CRect& GetSavedPos() { return m_SavedPos; }

	void SetSavedPos( const CRect& theRect );
	void ToggleDeleteFromServer();
	void ToggleDownloadFromServer();
	BOOL IsMessageIsOnServer();
	BOOL IsDeleteFromServerSet();
	BOOL IsDownloadFromServerSet();
	void SetMessageServerLeave();
	void SetMessageServerDelete();
	void SetMessageServerDownload();
	void SetMessageServerDownloadDelete();

	int CalculateMood(JJFile* pFile);
	BOOL Selected() const;
	int Select(BOOL bSelect = TRUE, bool in_bResultDirectUserAction = false);

	int Build(JJFile* in, BOOL Rebuild = FALSE);
	int Read(JJFile* in);
	int Write(JJFile* out);
	
	void Copy(CSummary* NewInfo);
	void SetOffset(long Offset) { m_Offset = Offset; }
	void SetFlag(WORD Flag) { m_Flags |= Flag; }
	void UnsetFlag(WORD Flag) { m_Flags &= ~Flag; }
	void ToggleFlag(WORD Flag) { m_Flags ^= Flag; }
	void CopyFlags(CSummary* Sum) { m_Flags = Sum->m_Flags; }
	WORD GetFlags() { return m_Flags; }

	void SetFlagEx(WORD Flag) { m_FlagsEx |= Flag; }
	void UnsetFlagEx(WORD Flag) { m_FlagsEx &= ~Flag; }
	void ToggleFlagEx(WORD Flag) { m_FlagsEx ^= Flag; }
	void CopyFlagsEx(CSummary* Sum) { m_FlagsEx = Sum->m_FlagsEx; }
	WORD GetFlagsEx() { return m_FlagsEx; }

	void RedisplayField(int field, BOOL SetDirty = TRUE);
	void SetState(char State, BOOL bRebuild = FALSE);
	void SetPriority(int Priority, BOOL bUpdateFrame = TRUE);
	void SetLabel(int Label);
	void SetServerStatus(UINT nID);
	static int ParsePriority(const char * szHeaderContents);
	static int ParseImportance(const char * szHeaderContents);
	void SetPriorityFromHeader(const char * szHeaderContents) { SetPriority( ParsePriority(szHeaderContents) ); }
	void SetPriorityFromImportanceHeader(const char * szHeaderContents) { SetPriority( ParseImportance(szHeaderContents) ); }
	void SetFrom(const char* NewFrom);
	void SetSubject(const char* NewSubject);
	void SetDate(time_t Seconds = 0L, BOOL bJunk = FALSE);
	static void SetDateString(char* pszDateBuf, int nSize, time_t nSeconds, int nTimeZoneMinutes, BOOL bDisplayLocalTime, BOOL bUseFixedFormat);
	void SetDateString(char* pszDateBuf, int nSize, BOOL bDisplayLocalTime, BOOL bUseFixedFormat) const
		{ SetDateString(pszDateBuf, nSize, m_Seconds, m_TimeZoneMinutes, bDisplayLocalTime, bUseFixedFormat); }
	void FormatDate(const char* GMTOffset = NULL);
	void SetMood(char NewMood);

	void PrintSum();

	int GetSize(); //get size in KB
	short GetLabel(){ return m_Label; }
	const char* GetFrom();
	CString GetPriority() const;
	const char* GetSubject();
	const char* GetDate() const
		{ return (m_Date); }
	unsigned long GetHash()
		{ return (m_Hash); }
	unsigned long SetHash(unsigned long Hash)
		{ return (m_Hash = Hash); }
	const char* MakeTitle();
	const char* GetTitle()
		{ if (m_Title.IsEmpty()) MakeTitle();  return (m_Title); }
	void SetJunkScore(unsigned char ucJunkScore);
	unsigned char GetJunkScore() {return m_ucJunkScore;}
	void SetJunkPluginID(unsigned long ulJunkPluginID) {m_ulJunkPluginID = ulJunkPluginID;}
	unsigned long GetJunkPluginID() {return m_ulJunkPluginID;}
	void SetManuallyJunked(bool bManuallyJunked) {m_bManuallyJunked = bManuallyJunked;}
	bool IsManuallyJunked() {return m_bManuallyJunked;}
	void SaveJunkValues() {m_ucOldJunkScore = m_ucJunkScore;
						   m_ulOldJunkPluginID = m_ulJunkPluginID;
						   m_bOldManuallyJunked = m_bManuallyJunked;};
	void RevertJunkValues() {SetJunkScore(m_ucOldJunkScore);
							 m_ucOldJunkScore = 0;
							 m_ulJunkPluginID = m_ulOldJunkPluginID;
							 m_ulOldJunkPluginID = 0;
							 m_bManuallyJunked = m_bOldManuallyJunked;
							 m_bOldManuallyJunked = false;};
	void SetOldJunk(bool bOldJunk) {m_bOldJunk = bOldJunk;};
	bool IsOldJunk() {return m_bOldJunk;};
	void SetLastXferWasJunkAction(bool bWasJunkAction) {m_bLastXferWasJunkAction = bWasJunkAction;};
	bool LastXferWasJunkAction() {return m_bLastXferWasJunkAction;};

	CMessageDoc* FindMessageDoc();
	CMessageDoc* GetMessageDoc(bool * pbWasCreated = NULL);

	// Static more efficient version of NukeMessageDocIfUnused for when you know
	// which pMessageDoc is associated with pSum or when pSum is NULL
	static BOOL NukeMessageDocIfUnused(CSummary * pSum, CMessageDoc * pMessageDoc);

	// Convenient inline version of NukeMessageDocIfUnused compatible with past use
	BOOL NukeMessageDocIfUnused() { return NukeMessageDocIfUnused( this, FindMessageDoc() ); }

	// Another way of calling more efficient version of NukeMessageDocIfUnused
	// when you already know what pMessageDoc is associated with this summary
	BOOL NukeMessageDocIfUnused(CMessageDoc * pMessageDoc) { return NukeMessageDocIfUnused(this, pMessageDoc); }

	BOOL Display();
	BOOL DisplayBelowTopMostMDIChild();

	BOOL ComposeMessage(UINT MenuItemID, const char* DefaultTo = NULL, BOOL AutoSend = FALSE, 
		const char* pszStationery = NULL, const char* pszPersona = NULL, const char* selectedText = NULL,
		BOOL bTurboRedirectOK = TRUE);
	static CCompMessageDoc *		ComposeMessage(
											CTocDoc *				in_pOriginatingTOC,
											bool					in_bDisplay,
											const char *			in_szTo,
											const char *			in_szSubject,
											const char *			in_szCc,
											const char *			in_szAttach,
											const char *			in_szBody,
											const char *			in_szStationery,
											const char *			in_szPersona,
											char					in_cResponseType,
											const char *			in_szECHeaders,
											const char *			in_szPrecedence,
											int						in_nIsFancy,
											int						in_nPriority,
											bool					in_bDisplayOrQueueImmediately = kDisplayOrQueueImmediately);
	
	CCompMessageDoc* SendAgain(BOOL DisplayIt = TRUE, const char* pszPersona = NULL);
	
	char GetState() const { return (char)m_State; }
	BOOL IsComp() const
		{ return (m_State >= MS_UNSENDABLE && m_State!=MS_RECOVERED); }

	BOOL CantEdit() const
		{ return (m_State == MS_SENT || m_State == MS_UNSENT || m_State == MS_SPOOLED ||
		         ( (m_State == MS_QUEUED) && (m_FlagsEx & MSFEX_MIME_ATTACHED) ) ); }
	BOOL IsSendable() const
		{ return (m_State == MS_SENDABLE || m_State == MS_QUEUED || m_State == MS_TIME_QUEUED); }
	BOOL IsQueued() const
		{ return (m_State == MS_QUEUED || m_State == MS_TIME_QUEUED); }

	BOOL Encoding() const
		{ return (m_Flags & (MSF_ENCODE0 | MSF_ENCODE1)); }
	BOOL UseSignature() const
		{ return (m_Flags & (MSF_USE_SIGNATURE | MSF_ALT_SIGNATURE)); }
	BOOL WordWrap() const
		{ return ((m_Flags & MSF_WORD_WRAP)? TRUE : FALSE); }
	BOOL SendFormatFlowed() const
		{ return (WordWrap() && GetIniShort(IDS_INI_SEND_FORMAT_FLOWED)); }
	BOOL TabsInBody() const
		{ return ((m_Flags & MSF_TABS_IN_BODY)? TRUE : FALSE); }
	BOOL KeepCopies() const
		{ return ((m_Flags & MSF_KEEP_COPIES)? TRUE : FALSE); }
	BOOL TextAsDoc() const
		{ return ((m_Flags & MSF_TEXT_AS_DOC)? TRUE : FALSE); }
	BOOL UseQP() const
		{ return ((m_Flags & MSF_QUOTED_PRINTABLE)? TRUE : FALSE); }
	BOOL ReturnReceipt() const
		{ return ((m_Flags & MSF_RETURN_RECEIPT)? TRUE : FALSE); }
	BOOL ReadReceipt() const
		{ return ((m_Flags & MSF_READ_RECEIPT)? TRUE : FALSE); }
	BOOL ShowAllHeaders() const
		{ return ((m_Flags & MSF_SHOW_ALL_HEADERS)? TRUE : FALSE); }

	void SetShowAllHeaders(BOOL doIt)
		{ if (doIt) m_Flags |= MSF_SHOW_ALL_HEADERS;
			else m_Flags &= ~MSF_SHOW_ALL_HEADERS;	}

	BOOL HasAttachment() const
		{ return ((m_Flags & MSF_HAS_ATTACHMENT)? TRUE : FALSE); }
	BOOL IsSubPart() const
		{ return ((m_Flags & MSF_SUB_PART)? TRUE : FALSE); }
	BOOL IsMAPI() const
		{ return ((m_Flags & MSF_MAPI_MESSAGE)? TRUE : FALSE); }
	BOOL IsXRich() const
		{ return ((m_Flags & MSF_XRICH)? TRUE : FALSE); }
	BOOL IsHTML() const
		{ return ((m_FlagsEx & MSFEX_HTML)? TRUE : FALSE); }
	BOOL IsFlowed() const
		{ return ((m_FlagsEx & MSFEX_FLOWED)? TRUE : FALSE); }
	BOOL IsAutoAttached() const
		{ return ((m_FlagsEx & MSFEX_AUTO_ATTACHED)? TRUE : FALSE); }
	BOOL IsMimeAttached() const
		{ return ((m_FlagsEx & MSFEX_MIME_ATTACHED)? TRUE : FALSE); }
	BOOL IsMDN() const
		{ return ((m_FlagsEx & MSFEX_MDN)? TRUE : FALSE); }
	BOOL HasEmptyBody() const
		{ return ((m_FlagsEx & MSFEX_EMPTY_BODY)? TRUE : FALSE); }

	BOOL SendPlain() const
		{ return (((m_FlagsEx & MSFEX_SEND_PLAIN) || !SendStyled())? TRUE : FALSE); }
	BOOL SendStyled() const
		{ return ((m_FlagsEx & MSFEX_SEND_STYLED)? TRUE : FALSE); }
	BOOL SendPlainOnly() const
		{ return (!SendStyled()); }
	BOOL SendStyledOnly() const
		{ return (!SendPlain() && SendStyled()); }
	BOOL SendPlainAndStyled() const
		{ return (SendPlain() && SendStyled()); }
	

	void SetTranslators(const char *trans, BOOL bValidate = FALSE);
	
	CString GetTranslators()
		{ return  m_TransHdr; }

	void SetPersona(const char *persona);
	
	CString GetPersona();
	unsigned long GetPersonaHash() { return m_PersonaHash; }
	
	void SetPrecedence(const char *prec)
		{ m_Precedence = prec; }
	
	CString GetPrecedence()
		{ return  m_Precedence; }

	void SetSignature( const char *sig);

	void SetUniqueMessageId(long uniqueMessageId)
		{ m_UniqueMessageId = uniqueMessageId; }
	long GetUniqueMessageId() const
		{ return m_UniqueMessageId; }
	void ConvertAttachmentLine(char* buf);

	BOOL	IsResponse() const
	{	return ( m_RespondingToSum != NULL ); }
	void SetDeleteEmbedded(BOOL bDelete = TRUE) {m_DeleteEmbedded = bDelete;}
	BOOL CanDeleteEmbedded(){ return m_DeleteEmbedded;}

	bool		GetNotifySearchManager() const { return m_bNotifySearchManager; }
	void		SetNotifySearchManager(bool in_bNotify) { m_bNotifySearchManager = in_bNotify; }

	bool		ShouldBeAddedToIndex() const { return m_bShouldBeAddedToIndex; }
	void		SetShouldBeAddedToIndex(bool in_bShouldBeAddedToIndex) { m_bShouldBeAddedToIndex = in_bShouldBeAddedToIndex; }

public:
	long m_Offset;
	long m_Length;
	short m_State;
	unsigned char m_ucJunkScore;
	bool m_bManuallyJunked;
	unsigned long m_ulJunkPluginID;
	unsigned char m_ucOldJunkScore;
	bool m_bOldManuallyJunked;
	unsigned long m_ulOldJunkPluginID;
	bool m_bOldJunk;
	bool m_bLastXferWasJunkAction;
	char m_Priority;
	char m_nMood;
	short m_Label;
	long m_Seconds;
	long m_lArrivalSeconds;
	int m_TimeZoneMinutes;
	CTocDoc* m_TheToc;
	char m_Date[32];
	char m_From[64];
	char m_Subject[64];
	CMessageFrame* m_FrameWnd;
	int m_Temp2;
	DWORD m_Temp;
	CString m_TransHdr;
	CString m_Precedence;
	CString m_SigHdr;
	CString m_SigSelected;

	CString m_strFullHeaders;		// Short-lived data member for holding full headers used in filtering
									// IMAP messages downloaded with only minimal headers.

	static long m_lLastTime;
	static long m_lBegin;
	
	// The number of attachments that would be associated with this summary when the mail is being sent
	int	m_nAttachmentCount;

	bool m_bReplyAll; //if the message was a result of hitting the Reply All

#ifdef IMAP4
	// Public.
	bool IsNotFullyIMAPDownloaded() const
		{ return ((m_Imflags & IMFLAGS_NOT_DOWNLOADED) != 0) || (m_nUndownloadedAttachments > 0); }
	bool IsNotIMAPDownloadedAtAll() const
		{ return ((m_Imflags & IMFLAGS_NOT_DOWNLOADED) != 0); }
	bool IsIMAPFullyDownloaded() const
		{ return ((m_Imflags & IMFLAGS_NOT_DOWNLOADED) == 0) && (m_nUndownloadedAttachments == 0); }
	bool IsIMAPMessageBodyDownloaded() const
		{ return ((m_Imflags & IMFLAGS_NOT_DOWNLOADED) == 0); }
	bool IMAPGetFromHeader(CString &strFrom);

	bool IsIMAPSeen() const			{ return (m_Imflags & IMFLAGS_SEEN) != 0; }
	bool IsIMAPAnswered() const		{ return (m_Imflags & IMFLAGS_ANSWERED) != 0; }
	bool IsIMAPFlagged() const		{ return (m_Imflags & IMFLAGS_FLAGGED) != 0; }
	bool IsIMAPDeleted() const		{ return (m_Imflags & IMFLAGS_DELETED) != 0; }
	bool IsIMAPDraft() const		{ return (m_Imflags & IMFLAGS_DRAFT) != 0; }
	bool IsIMAPRecent() const		{ return (m_Imflags & IMFLAGS_RECENT) != 0; }

	unsigned long m_Imflags;		// IMAP message flags - 4 bytes.
	//used for Imap
	unsigned short m_MsgSize;

	int m_nUndownloadedAttachments;	// Number of undownloaded attachments.
#endif

	BOOL m_rrDialogOverride;		// To prevent multiple RR dialogs -jdboyd 8/23/99
// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

private:
	void GetStatus(const char* Line);
	
private:
	WORD			m_Flags;
	WORD			m_FlagsEx;

	CString			m_Title;
	unsigned long	m_Hash;
	unsigned long	m_UniqueMessageId;
	CSummary*		m_ResponseSum;
	CSummary*		m_RespondingToSum;
	short			m_RespondingToOldState;
	unsigned long	m_PersonaHash;	// hash of m_Persona (save in TOC)
	long			m_DeleteEmbedded;
	bool			m_bNotifySearchManager;		//	Used to "gate" search manager notifications - ideally so that only occur when necessary
	bool			m_bShouldBeAddedToIndex;	//	Used by search manager to flag whether or not message is already indexed
};

class CSumList
{
  public:
	static const bool		kOwnsSums = true;
	static const bool		kDoesntOwnSums = false;
	static const bool		kUseMap = true;
	static const bool		kDontUseMap = false;


							CSumList(
								bool						in_bOwnsSums = kOwnsSums,
								bool						in_bUseMap = kUseMap);
							~CSumList();


	void					MoveSumsToOtherList(
								CSumList *					out_pDestination);

  // === Attributes (head and tail) ===
	// Count of elements
	INT_PTR					GetSize() const
								{ return m_ObList.GetSize(); }
	BOOL					IsEmpty() const
								{ return m_ObList.IsEmpty(); }

	// Peek at head or tail
	CSummary*&				GetHead()
								{ return reinterpret_cast<CSummary *&>( m_ObList.GetHead() ); }
	const CSummary*&		GetHead() const
								{ return reinterpret_cast<const CSummary *&>( m_ObList.GetHead() ); }
	CSummary*&				GetTail()
								{ return reinterpret_cast<CSummary *&>( m_ObList.GetHead() ); }
	const CSummary*&		GetTail() const
								{ return reinterpret_cast<const CSummary *&>( m_ObList.GetHead() ); }

  // === Operations ===
	// get head or tail (and remove it) - don't call on empty list!
	CSummary *				RemoveHead();
	CSummary *				RemoveTail();

	// add another list of elements before head or after tail
	POSITION				AddHead(CSummary * in_pNewSummary);
	POSITION				AddTail(CSummary * in_pNewSummary);

	// remove all elements
	void					RemoveAll();

	// iteration
	POSITION				GetHeadPosition() const
								{ return m_ObList.GetHeadPosition(); }
	POSITION				GetTailPosition() const
								{ return m_ObList.GetTailPosition(); }
	CSummary *&				GetNext(POSITION& rPosition)
								{ return reinterpret_cast<CSummary *&>( m_ObList.GetNext(rPosition) ); }
	const CSummary *&		GetNext(POSITION& rPosition) const
								{ return reinterpret_cast<const CSummary *&>( m_ObList.GetNext(rPosition) ); }
	CSummary *&				GetPrev(POSITION& rPosition)
								{ return reinterpret_cast<CSummary *&>( m_ObList.GetPrev(rPosition) ); }
	const CSummary *&		GetPrev(POSITION& rPosition) const
								{ return reinterpret_cast<const CSummary *&>( m_ObList.GetPrev(rPosition) ); }

	// getting/modifying an element at a given position
	CSummary *&				GetAt(POSITION position)
								{ return reinterpret_cast<CSummary *&>( m_ObList.GetAt(position) ); }
	const CSummary *&		GetAt(POSITION position) const
								{ return reinterpret_cast<const CSummary *&>( m_ObList.GetAt(position) ); }
	void					SetAt(POSITION position, CSummary * in_pSummary);
	void					RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION				InsertBefore(POSITION in_position, CSummary * in_pSummary);
	POSITION				InsertAfter(POSITION in_position, CSummary * in_pSummary);

	// helper functions (note: O(n) speed)
	POSITION				Find(
								CSummary *					in_pFindSummary) const;
	POSITION				FindIndex(INT_PTR nIndex) const
								{ return m_ObList.FindIndex(nIndex); }

	int						GetIndex(
								const CSummary *			Sum,
								BOOL						bIgnoreDeletedIMAP = FALSE) const;
	int						GetCount(
								BOOL						bIgnoreDeletedIMAP = FALSE) const;

	BOOL					Print();

	//
	// MAPI Support methods.
	//
	POSITION				FindNextByMessageId(
								unsigned long				messageId,
								BOOL						unreadOnly);
	CSummary *				GetByMessageId(
								unsigned long				messageId);

	// JOK.
	CSummary *				GetByUid(
								unsigned long				Uid);

  private:
	// Typedefs
	typedef std::map<unsigned long, CSummary *>		MessageIDToSummaryMapT;
	typedef MessageIDToSummaryMapT::iterator		MessageIDMapIteratorT;
	typedef std::map<CSummary *, POSITION>			SummaryToPositionMapT;
	typedef SummaryToPositionMapT::iterator			SummaryMapIteratorT;
	typedef SummaryToPositionMapT::const_iterator	SummaryMapConstIteratorT;

	// Methods
	void					AddSummaryToMaps(
								POSITION					in_pos,
								CSummary *					in_pSummary);
	void					RemoveSummaryFromMaps(
								CSummary *					in_pSummary,
								POSITION					in_pos = NULL);

	// Data
	bool						m_bOwnsSums;	// false if destructor shouldn't delete summaries
	bool						m_bUseMaps;		// false if maps shouldn't be used to speed up operations
	CObList						m_ObList;
	MessageIDToSummaryMapT		m_mapMessageIDToSummary;
	SummaryToPositionMapT		m_mapSummaryToPosition;
};


class CTempSumList : public CSumList
{
  public:
							//	Construct CSumList telling it that it doesn't own the sums
							CTempSumList() : CSumList(CSumList::kDoesntOwnSums) {}
};


// Function Prototypes

long GetTime(const char* Line, BOOL FromLine);
inline long IsFromLine(const char* Line) { return (GetTime(Line, TRUE)); }

// IMAP
HRESULT DownloadImapMessage (CSummary *pSum, BOOL bDownloadAttachments = FALSE);
HRESULT DownloadFromHeader(CSummary *pSum, CString &strFrom);
#endif
