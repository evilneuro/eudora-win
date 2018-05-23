// SUMMARY.H
//
// Routines for message summaries

#ifndef _SUMMARY_H_
#define _SUMMARY_H_


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

	BOOL Selected() const;
	int Select(BOOL bSelect = TRUE);

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
	void SetState(char State);
	void SetPriority(int Priority, BOOL bUpdateFrame = TRUE);
	void SetLabel(int Label);
	void SetServerStatus(UINT nID);
	void GleanPriority(const char* HeaderContents);
	void SetFrom(const char* NewFrom);
	void SetSubject(const char* NewSubject);
	void SetDate(time_t Seconds = 0L);
	void FormatDate(const char* GMTOffset = NULL);

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
	CMessageDoc* FindMessageDoc();
	CMessageDoc* GetMessageDoc();
	BOOL NukeMessageDocIfUnused();
	BOOL Display();
	BOOL DisplayBelowTopMostMDIChild();

	BOOL StripAttachFromBody(char* message, UINT tag);

	BOOL ComposeMessage(UINT MenuItemID, const char* DefaultTo = NULL, BOOL AutoSend = FALSE, const char* pszStationery = NULL, const char* pszPersona = NULL, const char* selectedText = NULL);
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

public:
	long m_Offset;
	long m_Length;
	short m_State;
	short m_Priority;
	short m_Label;
	long m_Seconds;
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
	
	static long m_lLastTime;
	static long m_lBegin;

#ifdef IMAP4
	// Public.
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
	void SetDateString(char* pszDateStr, int nSize, BOOL bDisplayLocalTime, BOOL bUseFixedFormat) const;
	
private:
	WORD			m_Flags;
	WORD			m_FlagsEx;

	CString			m_Title;
	unsigned long	m_Hash;
	unsigned long	m_UniqueMessageId;
	CSummary*		m_ResponseSum;
	CSummary*		m_RespondingToSum;
	short			m_RespondingToOldState;
	CString			m_Persona;
	unsigned long	m_PersonaHash;	// hash of m_Persona (save in TOC)
};

class CSumList : public CObList
{
public:
	CSumList() {}
	CSumList(const CSumList& copy);
	~CSumList();

	CSummary* GetAt(POSITION position) const
		{ return ((CSummary*)CObList::GetAt(position)); }
	CSummary*& GetAt(POSITION position)
		{ return ((CSummary*&)CObList::GetAt(position)); }
	CSummary*& GetHead()
		{ return ((CSummary*&)CObList::GetHead()); }
	CSummary* GetHead() const
		{ return ((CSummary*)CObList::GetHead()); }
	CSummary*& GetTail()
		{ return ((CSummary*&)CObList::GetTail()); }
	CSummary* GetTail() const
		{ return ((CSummary*)CObList::GetTail()); }
	CSummary*& GetNext(POSITION& rPosition)
		{ return ((CSummary*&)CObList::GetNext(rPosition)); }
	CSummary* GetNext(POSITION& rPosition) const
		{ return ((CSummary*)CObList::GetNext(rPosition)); }
	CSummary*& GetPrev(POSITION& rPosition)
		{ return ((CSummary*&)CObList::GetPrev(rPosition)); }
	CSummary* GetPrev(POSITION& rPosition) const
		{ return ((CSummary*)CObList::GetPrev(rPosition)); }
	CSummary* RemoveHead()
		{ return ((CSummary*)CObList::RemoveHead()); }
	CSummary* RemoveTail()
		{ return ((CSummary*)CObList::RemoveTail()); }
	int GetIndex(const CSummary* Sum) const;
	POSITION Find(CSummary* searchSum, POSITION startAfter = NULL) const;

	
	POSITION Add(CSummary* NewSummary)
		{ return (CObList::AddTail(NewSummary)); }

	BOOL	Print();

	//
	// MAPI Support methods.
	//
	POSITION FindNextByMessageId(unsigned long messageId, BOOL unreadOnly);
	CSummary* GetByMessageId(unsigned long messageId);

	// JOK.
	CSummary* GetByUid(unsigned long Uid);
};


// Function Prototypes

long GetTime(const char* Line, BOOL FromLine);
inline long IsFromLine(const char* Line) { return (GetTime(Line, TRUE)); }

// IMAP
BOOL IsNotDownloaded(CSummary *pSum);
HRESULT DownloadImapMessage (CSummary *pSum, BOOL bDownloadAttachments = FALSE);
#endif
