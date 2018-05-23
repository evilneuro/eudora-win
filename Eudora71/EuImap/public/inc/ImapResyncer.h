// ImapResyncer.h - declaration of the CImapResyncer class. This manages the resync options database
// and performs resync operations.
//

#ifndef __IMAPRESYNCER_H
#define __IMAPRESYNCER_H

#include "ImapExports.h"

#include "imap.h"


//======================================================================//

// Forward declarations
class  CImapConnection;
class  CImapMailbox;
class  CPtrUidList;

// CImapMailbox class.
class CImapResyncer
{

	// This is private:
	enum HeaderToDownload
	{
		ENVELOPE,
		FULL
	};

	enum BodyPartsToDownload
	{
		STRUCTURE_ONLY,			// Structure is always downloaded.
		STRUCTURE_AND_INLINE,
		STRUCTURE_AND_INLINE_AND
	};

public:
	CImapResyncer ();

//Interface
public:
	// Set stuff.
	void	SetMailbox (CImapMailbox *pImapMailbox)
				{ m_pImapMailbox = pImapMailbox; }
	BOOL	IsFullHeaderToBeDownloaded ()
				{ return m_bFullHeader; }

	BOOL	InitializeDatabase (BOOL Reset);
	void	SetDefaultOptions ();
	BOOL	SaveOptions();
	BOOL	IsPartToBeDownloaded (short Type, LPCSTR pSubtype);


// Internal functions:
// These may change if we ever use a dbm database to storing options so clients MUST
// not access these directly.
private:
	// Format pathname to the resync options file.
	BOOL	SetDatabaseFilePath ();


// Internal state:
private:
	CImapMailbox*	m_pImapMailbox;		// Referenced. Do not delete.

	CString			m_DBPathname;			// Pathname to the resync options database.

	// What to download:
	// Header:
	BOOL			m_bFullHeader;		// If TRUE, download full header, otherwise just envelope
										// and individual required headers such as priority, etc.
	// Body:
	BOOL			m_bInlineParts;		// We always download structure. If this is TRUE, download
										// in-line components also, if they are smaller that 
										// "MaxInlineSize" below.
	unsigned long   m_ulMaxInlineSize;  // Don't download inline parts larger than this.

	// Which MIME types to download.
	CString			m_TextSubtypesToDownload;	// This is a string of comma-separated Text Subtypes to download.
	CString			m_ImageSubtypesToDownload;
	CString			m_AudioSubtypesToDownload;
	CString			m_VideoSubtypesToDownload;
	CString			m_ApplicationSubtypesToDownload;

	unsigned long	m_ulMaxPartSize;		// For MIME types in the aboe list, don't download
											// parts larger than this.
};



#endif // __IMAPRESYNCER_H

