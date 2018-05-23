/////////////////////////////////////////////////////////////////////////////
// 	File: IMAPGETS.H 
//
//  Purpose:  This file provides the declarations for the MAILGETS, MM_LIST and MM_LSUB
//				routines.   
//
//	Author:		J. King.
//	Created:    10/23/96
//  Modified:
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _IMAPGETS_H_
#define _IMAPGETS_H_

#include "QCUtils.h"
#include "Decode.h"
#include "ImapExports.h"


#define	BUFLEN	8192


// Forward declarations:
class CImapAccount;
class CPtrUidList;

// Declarations for the CStringWriter class, derived from CWriter (in imapdll).
// Note: Stores TEXT only!!
class CStringWriter : public CWriter
{
public:
	CStringWriter();
	~CStringWriter();

// We MUST override this.
	BOOL   Write   (readfn_t readfn, void * read_data, unsigned long size);

// Interface to caller:	
	BOOL	Write (const char *buffer, ULONG nBytes);
	void	RemoveTrailingBlankLine ();
	LPCSTR	GetString();
	unsigned long GetCurrentSize ()
		{ return m_CurrentSize; }

// State:
private:
	CString		m_Buffer;		// Fill this with the data.
	LONG		m_CurrentSize;
	LONG		m_FullSize;		// Size of all of data.

	// When m_pReadBuffer is created, set the buffer size also.
	long		m_nBufferSize;
	LPSTR		m_pReadBuffer;
};



// =================== CFileWriter class ========================/


// Declarations for the CFileWriter class, derived from CWriter (in imapdll).
// Note: Stores TEXT only!!
class CFileWriter : public CWriter
{
public:
	CFileWriter();
	CFileWriter(const char *Filename);
	~CFileWriter();

// We MUST override this:
	BOOL	Write (readfn_t readfn, void * read_data, unsigned long size);

// Interface to caller:	
	BOOL	Open ();
	BOOL	Close();
	void	SetFilename (const char *filename);
	LPCSTR	GetFilename();

	// IF we will be decoding before we write.
	void	SetDecoder (unsigned short encoding);
	void	ResetDecoder ();

	// Special!!
	//
	void	SetApplefileDecoder (unsigned short encoding);

	// Utility methods.
	BOOL	Write	(const char *buffer, ULONG nBytes);
	BOOL	WriteTextLine  (const char *buffer = NULL);

	// If to show progress bar:
	void	ShowProgress (BOOL bVal)
			{ m_bShowProgress = bVal; }

private:
	// Internal methods.


// State:
private:
	CString		m_Filename;					// Store contents in this file.
	LONG		m_CurrentSize;				// Length of data in m_Buffer.
	LONG		m_FullSize;					// Size of all of data.

	// Allocated buffer.
	// When m_pReadBuffer is created, set the buffer size also.
	long		m_nBufferSize;
	LPSTR		m_pReadBuffer;
	LPSTR		m_pWriteBuffer;

	// Can have one of these, or none, but not both.
	Decoder*	m_Decoder;

	// If decoder requires a line at a time:
	BOOL		m_bMustReadSingleLines;

	//IMAPFile	m_Jfile;				// Use this to store to file.
	JJFileMT	m_Jfile;				// Use this to store to file.
	int			m_fd;		// File handle. Initialized to -1.

	// If to show progress.
	BOOL m_bShowProgress;
};




//======================= CMboxLister class =============================/

class CMboxLister : public CLister
{
public:
	CMboxLister();
	CMboxLister(CImapAccount *pAccount);

	BOOL	AddMailbox (char *name, int delimiter, long attributes);

protected:
	CImapAccount *m_pAccount;
	
};





//======================= CImapNodeLister class =============================/
// This accumulates mailboxes as they come in into a tree of ImapMailboxNodes.

// Forward declarations
typedef struct imap_mailbox_node ImapMailboxNode;


class CImapNodeLister : public CLister
{
public:
	CImapNodeLister(ACCOUNT_ID AccountId);
	~CImapNodeLister();

	BOOL	AddMailbox (char *name, int delimiter, long attributes);

public:
	ImapMailboxNode		*m_pTopMailbox;	// Found folders are added to this list. 

private:
int Insert(const char *mailbox, TCHAR delimiter, long attributes, ImapMailboxType Type);

private:
	// We fetch stuff for one account at a time.
	ACCOUNT_ID		m_AccountID;

	// Internal state used in fetching mailbox lists.
	short			m_curLevel;
	CString			m_curReference;			// Used in Insert().
	ImapMailboxNode *m_curParentMailbox;
	ImapMailboxNode	**m_curMailboxList;		// pointer to Current top node, initially m_pTopMailbox.
};






//==================== CImapLogin class ==============================/

class CImapLogin : public CIMLogin
{
public:
	CImapLogin ();

	BOOL Login (char *user, char *pwd, long trial);

	// Not inherited:
	void SetAccount (CImapAccount *pAccount)
			{ m_pAccount = pAccount; }

protected:
	CImapAccount *m_pAccount;

};





//======================================= CImapEltWriter class =========================//
class CImapEltWriter : public CEltWriter
{

public:
	CImapEltWriter  (CPtrUidList *pUidList);
	~CImapEltWriter ();

	// Must override this.
	BOOL  SetFlag (	unsigned long uid,
					unsigned int seen,
					unsigned int deleted,
					unsigned int flagged,
					unsigned int answered,
					unsigned int draft,
					unsigned int recent);


// Internal state.
private:
	// For setting flags. Contains by reference.
	CPtrUidList *m_pUidList;
};





//======================================= CImapEltWriterMap class =========================//
// Uses an STL "MAP" instead of a list.
//
class CUidMap;

class CImapEltWriterMap : public CEltWriter
{

public:
	CImapEltWriterMap  (CUidMap *pUidMap);
	~CImapEltWriterMap ();

	// Must override this.
	BOOL  SetFlag (	unsigned long uid,
					unsigned int seen,
					unsigned int deleted,
					unsigned int flagged,
					unsigned int answered,
					unsigned int draft,
					unsigned int recent);


// Internal state.
private:
	// For setting flags. Contains by reference.
	CUidMap *m_pUidMap;
};





//====================== CFileReader class ====================/
//====================== CFileReader class ====================/
// This class reads a file in chunks.
// NOTE: Don't use a IMAPFile to do the reading because it doesn't provide a decent
// function that will return the number of bytes read.
class CFileReader : public CReader
{
public:
	CFileReader(LPCSTR pFilepath);
	~CFileReader();	

	// Must override these.
	unsigned long	GetTotalSize ()
					{ return m_TotalSize; }

	long 			Read	(char *buffer, ULONG BufferSize);
	BOOL			Done	();

	// Additional:

	// Read into the internal buffer.
	long			Read();

	BOOL			Open();
	BOOL			Close ();

	// Note: If the file is open, then m_fd (below) is >= 0.
	BOOL			IsOpen ()
						{ return (m_fd >= 0); }

public:
	char m_buffer [BufferSize + 4];


private:
	// Size of the file.
	unsigned long m_TotalSize;

	int			m_fd;		// File handle. Initialized to -1.

	CString       m_Filepath;
};




//======================= CChunkReader ===========================//

class CChunkReader
{
public:
	CChunkReader (readfn_t readfn, void * read_data, long size,
							 BOOL bMustReadSingleLines);

	~CChunkReader ();

	BOOL			GetNextChunk (LPSTR *pBuf, long *inLen);

private:
	readfn_t			m_pReadfn;
	void*				m_pReadData;
	long				m_Size;
	// Caller
	BOOL				m_bMustReadSingleLines;

	LPSTR				m_pReadBuffer;
	long				m_lBufferSize;
	LPSTR				m_pStart, m_pNext;

	// Internal state:
	long				m_lBytesInBuffer;
	long				m_lBytesRead;
};

#endif  // #ifndef _IMAPGETS_H_
