// HEADER.HPP
//
// Routines for looking at mail headers
//

#ifndef _HEADER_H_
#define _HEADER_H_


// Data Types

// States of the header converter
typedef enum
{
	ExpectHeaderName,		// next token shd be a header name
	ExpectColon,			// next token shd be a colon
	ExpectText,				// we're looking for an unstructured field
	ExpectType,				// content type
	ExpectSlash,			// separator
	ExpectSubType,			// content subtype
	ExpectSem,				// expecting semicolon (or end)
	ExpectAttribute,		// expecting attribute name
	ExpectEqual,			// expecting equal sign
	ExpectValue,			// expecting attribute value
	ExpectEnco,				// expecting encoding
	ExpectVersion			// expecting MIME version number
} HeaderStateEnum;

// "Interesting" headers
typedef enum
{
	hContentType,
	hContentEncoding,
	hContentDisposition,
	hContentDescription,
	hContentBase,
	hContentID,
	hContentLocation,
	hStatus,
	hMimeVersion,
	hReceived,
	hMessageID,
	hSubject,
	hWho,
	hDispNotifyTo,
	hLimit
} InterestHeaderEnum;

// Known attributes
typedef enum
{
	aUnknown,		// ???
	aName,			// filename
	aCharSet,		// character set
	aBoundary,		// multipart boundary
	aLimit
} AttributeEnum;

class AttributeElement : public CObject
{
public:
	CString m_Name;
	CString m_Value;

	AttributeElement(const char* Name, const char* Value)
		{ m_Name = Name; m_Value = Value; }

	void ChangeValue(const char *NewValue)
		{ m_Value = NewValue; }
};

// Forward Declaration
class CTLMime;
class LineReader;

// Structure that describes a header
class HeaderDesc
{
public:
	HeaderDesc(long estSize = 0L);
	~HeaderDesc();

	void Init();
	int Read(LineReader* pLR);
	int AddAttribute(const char* Value);
	AttributeElement* GetAttribute(const char* Name);

	long RemainingBytes;		// Approximately how much left to go
	CObList Attributes;			// Attributes from Content-Type header
	HeaderStateEnum state;		// state of header converter
	InterestHeaderEnum hFound;	// header we're working on now
	char contentType[32];		// MIME content type
	char contentSubType[32];	// MIME content subtype
	char contentEnco[32];		// MIME content encoding
	char receivedDate[32];		// Date part of first Received: header
	char messageID[128];		// Message-ID: header
	char contentID[128];		// Content-ID header
	char contentLocation[128];	// Content-Location header
	char contentBase[256];		// Content-Base header
	char status[32];			// Status: header
	char subj[32];				// subject
	char who[32];				// sender
	char attributeName[32];		// name of attribute being collected
	char mimeVersion[32];		// mime version string
	long lDiskStart;			// where header starts on disk
	long lDiskEnd;				// where header ends on disk
	BOOL grokked;				// did we find understand it all?
	BOOL isMIME;				// is MIME
	BOOL isMHTML;				// is MHTML (can have embedded parts)
	BOOL hasRich;				// has richtext
	BOOL isPartial;				// is this a MIME-partial?
	BOOL isDispNotifyTo;		// is this a Disposition-Notification-To ?
	CTLMime *m_TLMime;			// Mime info for the translator
};

#endif
