// IMAPHEADER.H
//
// Routines for looking at mail headers
//

#ifndef _IMAPHEADER_H_
#define _IMAPHEADER_H_


// Need this for the enumerated data types, interesting headers, etc.
#include "header.h"


// Forward Declaration
class CTLMime;
class CImapDownloader;

// Structure that describes a header
class ImapHeaderDesc
{
public:
	ImapHeaderDesc();
	~ImapHeaderDesc();

	void	Init ();

	int		Read (CImapDownloader *pRW);
	int		AddAttribute(const char* Value);
	AttributeElement* GetAttribute(const char* Name);

// Public data.
public:
	long				m_RemainingBytes;		// Approximately how much left to go
	CObList Attributes;							// Attributes from Content-Type header
	HeaderStateEnum		m_state;				// state of header converter
	InterestHeaderEnum	m_hFound;				// header we're working on now
	char				m_contentType[32];		// MIME content type
	char				m_contentSubType[32];	// MIME content subtype
	char				m_contentEnco[32];		// MIME content encoding
	char				m_receivedDate[32];		// Date part of first Received: header
	char				m_messageID[128];		// Message-ID: header
	char				m_contentID[128];		// Content-ID header
	char				m_contentLocation[128];	// Content-Location header
	char				m_contentBase[256];		// Content-Base header
	char				m_status[32];			// Status: header
	char				m_subj[32];				// subject
	char				m_who[32];				// sender
	char				m_attributeName[32];		// name of attribute being collected
	char				m_mimeVersion[32];		// mime version string

	// File data.
	JJFileMT*			m_pMbxFile;
	long				m_lDiskStart;			// where header starts on disk
	long				m_lDiskEnd;				// where header ends on disk

	BOOL				m_grokked;				// did we find understand it all?
	BOOL				m_isMIME;				// is MIME
	BOOL				m_isMHTML;				// is MHTML (can have embedded parts)
	BOOL				m_hasRich;				// has richtext
	BOOL				m_isPartial;				// is this a MIME-partial?
	BOOL				m_isDispNotifyTo;		// is this a Disposition-Notification-To ?
	CTLMime*			m_TLMime;			// Mime info for the translator

	// If this is non-zero, then this header is the top-level header for a message.
	unsigned long       m_Uid;
};

#endif
